#include "scanline_mode1.h"

enum {
  kTileBytes4bpp = 32,
  kScrollMask = 0x3FF
};

static const uint8_t kObjSizePx[8][2] = {
    {8, 16},  {8, 32},  {8, 64}, {16, 32},
    {16, 64}, {32, 64}, {16, 32}, {16, 32},
};

uint16_t PicoBgr555ToRgb565(uint16_t bgr555) {
  uint16_t r = (uint16_t)(bgr555 & 0x1F);
  uint16_t g = (uint16_t)((bgr555 >> 5) & 0x1F);
  uint16_t b = (uint16_t)((bgr555 >> 10) & 0x1F);
  uint16_t g6 = (uint16_t)((g << 1) | (g >> 4));
  return (uint16_t)((r << 11) | (g6 << 5) | b);
}

static size_t vram_bytes(const PicoPpuState *ppu) {
  return ppu->vram_size ? ppu->vram_size : (size_t)kPicoVramSize;
}

static uint16_t read_vram_word(const PicoPpuState *ppu, uint16_t word_addr) {
  size_t offset = (size_t)word_addr << 1;
  size_t n = vram_bytes(ppu);
  if (ppu->vram == NULL || offset + 1 >= n)
    return 0;
  return (uint16_t)(ppu->vram[offset] | ((uint16_t)ppu->vram[offset + 1] << 8));
}

static uint16_t lookup_cgram(const PicoPpuState *ppu, int index) {
  if (ppu->cgram == NULL || (unsigned)index >= (unsigned)kPicoCgramColors)
    return 0;
  return PicoBgr555ToRgb565(ppu->cgram[index]);
}

static uint8_t decode_4bpp(const PicoPpuState *ppu, size_t tile_byte, int px, int py) {
  size_t n = vram_bytes(ppu);
  if (ppu->vram == NULL || tile_byte >= n || n - tile_byte < (size_t)kTileBytes4bpp)
    return 0;
  const uint8_t *tile = ppu->vram + tile_byte;
  uint8_t mask = (uint8_t)(0x80u >> px);
  uint8_t p0 = (tile[py * 2] & mask) != 0;
  uint8_t p1 = (tile[py * 2 + 1] & mask) != 0;
  uint8_t p2 = (tile[16 + py * 2] & mask) != 0;
  uint8_t p3 = (tile[16 + py * 2 + 1] & mask) != 0;
  return (uint8_t)(p0 | (p1 << 1) | (p2 << 2) | (p3 << 3));
}

static uint16_t bg_tilemap_word(const PicoPpuState *ppu, uint8_t bg_sc, int tile_x, int tile_y) {
  int tm_w = (bg_sc & 1) ? 64 : 32;
  int tm_h = (bg_sc & 2) ? 64 : 32;
  tile_x &= tm_w - 1;
  tile_y &= tm_h - 1;
  uint16_t addr = (uint16_t)((bg_sc & 0xFC) << 8);
  addr = (uint16_t)(addr + ((tile_y & 31) << 5) + (tile_x & 31));
  if ((tile_x & 32) && (bg_sc & 1))
    addr = (uint16_t)(addr + 0x400);
  if ((tile_y & 32) && (bg_sc & 2))
    addr = (uint16_t)(addr + ((bg_sc & 1) ? 0x800 : 0x400));
  return read_vram_word(ppu, addr);
}

static uint16_t bg_chr_word(const PicoPpuState *ppu, int layer) {
  if (layer == 1)
    return (uint16_t)((ppu->bg12nba & 0x0F) << 12);
  return (uint16_t)((ppu->bg12nba & 0xF0) << 8);
}

static void blit_bg(const PicoPpuState *ppu, int layer, int pri, int y, uint16_t *out) {
  uint8_t bg_sc = (layer == 1) ? ppu->bg1sc : ppu->bg2sc;
  int hofs = (layer == 1 ? ppu->bg1hofs : ppu->bg2hofs) & kScrollMask;
  int vofs = (layer == 1 ? ppu->bg1vofs : ppu->bg2vofs) & kScrollMask;
  uint16_t chr = bg_chr_word(ppu, layer);
  int world_y = y + vofs;
  int tile_y = world_y >> 3;
  int row = world_y & 7;
  int x = 0;
  int world_x = hofs;

  while (x < kPicoScreenWidth) {
    int tile_x = world_x >> 3;
    int col0 = world_x & 7;
    int n_pix = 8 - col0;
    if (n_pix > kPicoScreenWidth - x)
      n_pix = kPicoScreenWidth - x;
    uint16_t attr = bg_tilemap_word(ppu, bg_sc, tile_x, tile_y);
    if (((attr >> 13) & 1) == pri) {
      int tile_index = attr & 0x3FF;
      int pal_base = ((attr >> 10) & 7) * 16;
      int x_flip = (attr & 0x4000) != 0;
      int y_flip = (attr & 0x8000) != 0;
      int sy = y_flip ? (7 - row) : row;
      size_t tile_byte = ((size_t)chr << 1) + (size_t)tile_index * kTileBytes4bpp;
      for (int i = 0; i < n_pix; i++) {
        int col = col0 + i;
        int sx = x_flip ? (7 - col) : col;
        uint8_t color = decode_4bpp(ppu, tile_byte, sx, sy);
        if (color != 0)
          out[x + i] = lookup_cgram(ppu, pal_base + color);
      }
    }
    x += n_pix;
    world_x += n_pix;
  }
}

static int obj_name_base(const PicoPpuState *ppu, int name_select) {
  int base = (ppu->obsel & 7) << 14;
  if (name_select)
    base += (((ppu->obsel & 0x18) + 8) << 10);
  return base;
}

static int obj_tile_byte(int charnum, int tile_x, int tile_y) {
  int tile = charnum & 0xFF;
  int packed = ((((tile >> 4) + tile_y) << 4) | (((tile & 0xF) + tile_x) & 0xF));
  return packed * kTileBytes4bpp;
}

static int sprite_on_line(int sprite_y, int size, int y) {
  int y1 = sprite_y + size;
  if (y >= sprite_y && y < y1)
    return 1;
  if (y1 > 256 && y < (y1 - 256))
    return 1;
  return 0;
}

static void blit_obj(const PicoPpuState *ppu, int pri, int y, uint16_t *out) {
  if (ppu->oam == NULL)
    return;
  int count = ppu->sprite_count;
  if (count <= 0 || count > kPicoSpriteCount)
    count = kPicoSpriteCount;
  int size_sel = (ppu->obsel >> 5) & 7;
  int small = kObjSizePx[size_sel][0];
  int large = kObjSizePx[size_sel][1];

  /* Lower OAM index wins on overlap; walk 127..0 so sprite 0 is on top. */
  for (int i = count - 1; i >= 0; i--) {
    const uint8_t *ent = ppu->oam + (size_t)i * 4;
    uint8_t hi = 0;
    if (ppu->oam_hi != NULL)
      hi = (uint8_t)((ppu->oam_hi[i >> 2] >> ((i & 3) * 2)) & 3);
    uint8_t attr = ent[3];
    int obj_pri = (attr >> 4) & 3;
    if (obj_pri != pri)
      continue;

    int size = (hi & 2) ? large : small;
    int sprite_y = ent[1];
    if (!sprite_on_line(sprite_y, size, y))
      continue;

    int sprite_x = ent[0] | ((hi & 1) << 8);
    if (sprite_x >= 256)
      sprite_x -= 512;

    int charnum = ent[2] | ((attr & 1) << 8);
    int pal_base = 128 + ((attr >> 1) & 7) * 16;
    int x_flip = (attr & 0x40) != 0;
    int y_flip = (attr & 0x80) != 0;
    int name_base = obj_name_base(ppu, (charnum & 0x100) != 0);
    int sy = y - sprite_y;
    if (sy < 0)
      sy += 256;
    int src_y = y_flip ? (size - 1 - sy) : sy;
    int ty = src_y >> 3;
    int row = src_y & 7;

    for (int sx = 0; sx < size; sx++) {
      int screen_x = sprite_x + sx;
      if ((unsigned)screen_x >= (unsigned)kPicoScreenWidth)
        continue;
      int src_x = x_flip ? (size - 1 - sx) : sx;
      int tx = src_x >> 3;
      int col = src_x & 7;
      size_t tile_byte = (size_t)name_base + (size_t)obj_tile_byte(charnum, tx, ty);
      uint8_t color = decode_4bpp(ppu, tile_byte, col, row);
      if (color != 0)
        out[screen_x] = lookup_cgram(ppu, pal_base + color);
    }
  }
}

void PicoScanline_Mode1(const PicoPpuState *ppu, int y, uint16_t *out256) {
  if (ppu == NULL || out256 == NULL || y < 0 || y >= kPicoScreenHeight)
    return;

  uint16_t backdrop = lookup_cgram(ppu, 0);
  for (int x = 0; x < kPicoScreenWidth; x++)
    out256[x] = backdrop;

  /* Mode 1 back-to-front without BG3. Sprite 32-tile/line cap is not applied. */
  blit_bg(ppu, 2, 0, y, out256);
  blit_obj(ppu, 0, y, out256);
  blit_bg(ppu, 1, 0, y, out256);
  blit_obj(ppu, 1, y, out256);
  blit_bg(ppu, 2, 1, y, out256);
  blit_obj(ppu, 2, y, out256);
  blit_bg(ppu, 1, 1, y, out256);
  blit_obj(ppu, 3, y, out256);
}
