#include "scanline_mode1.h"

#include <string.h>

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

/*
 * sm_rev-k5q.7 item 2: CGRAM -> RGB565 once, not once per pixel.
 *
 * The renderer's only entry point is per scanline, so it has no frame hook to
 * hang a "build the palette now" call on. The cache is therefore keyed on the
 * CGRAM *contents*: one 512-byte memcmp per line is far cheaper than 256
 * BGR555->RGB565 conversions, and unlike a pointer key it cannot go stale when
 * a caller mutates the palette in place between frames (the host tests do
 * exactly that). In the steady state the palette is converted once per frame
 * because only the first line of a frame sees a changed CGRAM.
 *
 * Single-rasteriser assumption: if a later bead rasters on both cores this
 * must become per-core or caller-owned storage.
 *
 * Bounds: lookup_cgram() used to reject index >= 256, but every call site
 * feeds it pal_base + color with pal_base <= 240 and color <= 15, so the index
 * was always in range and the check only ever cost cycles. The NULL-CGRAM case
 * (all colors black) is kept, as an all-zero palette.
 */
static uint16_t s_pal_rgb565[kPicoCgramColors];
static uint16_t s_pal_src[kPicoCgramColors];
static int s_pal_state; /* 0 = empty, 1 = from s_pal_src, 2 = NULL CGRAM */

static const uint16_t *palette_for(const PicoPpuState *ppu) {
  int i;
  if (ppu->cgram == NULL) {
    if (s_pal_state != 2) {
      memset(s_pal_rgb565, 0, sizeof s_pal_rgb565);
      s_pal_state = 2;
    }
    return s_pal_rgb565;
  }
  if (s_pal_state != 1 ||
      memcmp(s_pal_src, ppu->cgram, sizeof s_pal_src) != 0) {
    memcpy(s_pal_src, ppu->cgram, sizeof s_pal_src);
    for (i = 0; i < kPicoCgramColors; i++)
      s_pal_rgb565[i] = PicoBgr555ToRgb565(s_pal_src[i]);
    s_pal_state = 1;
  }
  return s_pal_rgb565;
}

/*
 * sm_rev-k5q.7 items 1 and 3: decode a whole 4bpp tile row at a time.
 *
 * The old decode_4bpp() re-read all four bitplane bytes and re-ran the VRAM
 * bounds check for every pixel: 32 loads and 8 range tests per 8-pixel row.
 * This does 4 loads and one range test. Out-of-range VRAM yields all-zero,
 * which is exactly what the old per-pixel guard returned.
 */
static void decode_row_4bpp(const PicoPpuState *ppu, size_t tile_byte, int py,
                            uint8_t *px8) {
  size_t n = vram_bytes(ppu);
  const uint8_t *t;
  uint32_t b0, b1, b2, b3;
  int i;

  if (ppu->vram == NULL || tile_byte >= n ||
      n - tile_byte < (size_t)kTileBytes4bpp) {
    memset(px8, 0, 8);
    return;
  }
  t = ppu->vram + tile_byte + (size_t)py * 2u;
  b0 = t[0];
  b1 = t[1];
  b2 = t[16];
  b3 = t[17];
  for (i = 0; i < 8; i++) {
    px8[i] = (uint8_t)(((b0 >> 7) & 1) | (((b1 >> 7) & 1) << 1) |
                       (((b2 >> 7) & 1) << 2) | (((b3 >> 7) & 1) << 3));
    b0 <<= 1;
    b1 <<= 1;
    b2 <<= 1;
    b3 <<= 1;
  }
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

/*
 * base != 0 makes this pass opaque: every pixel in [x0,x1) is written, with
 * the backdrop where the tile is transparent or the wrong priority. That is
 * sm_rev-k5q.7 item 5 -- it is bit-identical to "fill 256 px with the backdrop,
 * then blit BG2 priority 0 over it", minus the 256 stores BG2 overwrote.
 */
static void blit_bg(const PicoPpuState *ppu, int layer, int pri, int y,
                    uint16_t *out, const uint16_t *pal, int x0, int x1,
                    int base, uint16_t backdrop) {
  uint8_t bg_sc = (layer == 1) ? ppu->bg1sc : ppu->bg2sc;
  int hofs = (layer == 1 ? ppu->bg1hofs : ppu->bg2hofs) & kScrollMask;
  int vofs = (layer == 1 ? ppu->bg1vofs : ppu->bg2vofs) & kScrollMask;
  uint16_t chr = bg_chr_word(ppu, layer);
  int world_y = y + vofs;
  int tile_y = world_y >> 3;
  int row = world_y & 7;
  int x = x0;
  int world_x = hofs + x0;
  uint8_t px8[8];

  while (x < x1) {
    int tile_x = world_x >> 3;
    int col0 = world_x & 7;
    int n_pix = 8 - col0;
    uint16_t attr;
    int i;
    if (n_pix > x1 - x)
      n_pix = x1 - x;
    attr = bg_tilemap_word(ppu, bg_sc, tile_x, tile_y);
    if (((attr >> 13) & 1) == pri) {
      int tile_index = attr & 0x3FF;
      const uint16_t *cpal = pal + ((attr >> 10) & 7) * 16;
      int x_flip = (attr & 0x4000) != 0;
      int y_flip = (attr & 0x8000) != 0;
      int sy = y_flip ? (7 - row) : row;
      int idx = x_flip ? (7 - col0) : col0;
      int step = x_flip ? -1 : 1;
      size_t tile_byte =
          ((size_t)chr << 1) + (size_t)tile_index * kTileBytes4bpp;
      decode_row_4bpp(ppu, tile_byte, sy, px8);
      if (base) {
        for (i = 0; i < n_pix; i++, idx += step) {
          uint8_t color = px8[idx];
          out[x + i] = color ? cpal[color] : backdrop;
        }
      } else {
        for (i = 0; i < n_pix; i++, idx += step) {
          uint8_t color = px8[idx];
          if (color != 0)
            out[x + i] = cpal[color];
        }
      }
    } else if (base) {
      for (i = 0; i < n_pix; i++)
        out[x + i] = backdrop;
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

/*
 * sm_rev-k5q.7 item 4: one pass over the 128 OAM slots per scanline instead of
 * four (one per priority). Slots are appended walking 127..0, so each list is
 * already in the descending-index order the old blit_obj() drew in and sprite 0
 * still lands on top. Sprites that fall entirely outside [x0,x1) are dropped
 * here, which is exactly the set the old per-pixel screen_x check discarded.
 */
static uint8_t s_obj_list[4][kPicoSpriteCount];
static uint8_t s_obj_n[4];

static void build_obj_lists(const PicoPpuState *ppu, int y, int x0, int x1) {
  int count;
  int size_sel;
  int small;
  int large;
  int i;

  s_obj_n[0] = 0;
  s_obj_n[1] = 0;
  s_obj_n[2] = 0;
  s_obj_n[3] = 0;
  if (ppu->oam == NULL)
    return;

  count = ppu->sprite_count;
  if (count <= 0 || count > kPicoSpriteCount)
    count = kPicoSpriteCount;
  size_sel = (ppu->obsel >> 5) & 7;
  small = kObjSizePx[size_sel][0];
  large = kObjSizePx[size_sel][1];

  for (i = count - 1; i >= 0; i--) {
    const uint8_t *ent = ppu->oam + (size_t)i * 4;
    uint8_t hi = 0;
    int obj_pri;
    int size;
    int sprite_x;
    if (ppu->oam_hi != NULL)
      hi = (uint8_t)((ppu->oam_hi[i >> 2] >> ((i & 3) * 2)) & 3);
    size = (hi & 2) ? large : small;
    if (!sprite_on_line(ent[1], size, y))
      continue;
    sprite_x = ent[0] | ((hi & 1) << 8);
    if (sprite_x >= 256)
      sprite_x -= 512;
    if (sprite_x >= x1 || sprite_x + size <= x0)
      continue;
    obj_pri = (ent[3] >> 4) & 3;
    s_obj_list[obj_pri][s_obj_n[obj_pri]++] = (uint8_t)i;
  }
}

static void blit_obj(const PicoPpuState *ppu, int pri, int y, uint16_t *out,
                     const uint16_t *pal, int x0, int x1) {
  int size_sel = (ppu->obsel >> 5) & 7;
  int small = kObjSizePx[size_sel][0];
  int large = kObjSizePx[size_sel][1];
  int n = s_obj_n[pri];
  int k;

  for (k = 0; k < n; k++) {
    int i = s_obj_list[pri][k];
    const uint8_t *ent = ppu->oam + (size_t)i * 4;
    uint8_t hi = 0;
    uint8_t attr = ent[3];
    int size;
    int sprite_y;
    int sprite_x;
    int charnum;
    const uint16_t *cpal;
    int x_flip;
    int y_flip;
    int name_base;
    int sy;
    int src_y;
    int ty;
    int row;
    int sx;
    int sx_lo;
    int sx_hi;
    int cur_tx = -1;
    uint8_t px8[8];

    if (ppu->oam_hi != NULL)
      hi = (uint8_t)((ppu->oam_hi[i >> 2] >> ((i & 3) * 2)) & 3);
    size = (hi & 2) ? large : small;
    sprite_y = ent[1];
    sprite_x = ent[0] | ((hi & 1) << 8);
    if (sprite_x >= 256)
      sprite_x -= 512;

    charnum = ent[2] | ((attr & 1) << 8);
    cpal = pal + 128 + ((attr >> 1) & 7) * 16;
    x_flip = (attr & 0x40) != 0;
    y_flip = (attr & 0x80) != 0;
    name_base = obj_name_base(ppu, (charnum & 0x100) != 0);
    sy = y - sprite_y;
    if (sy < 0)
      sy += 256;
    src_y = y_flip ? (size - 1 - sy) : sy;
    ty = src_y >> 3;
    row = src_y & 7;

    /* Hoisted out of the pixel loop: the visible span of this sprite. */
    sx_lo = x0 - sprite_x;
    if (sx_lo < 0)
      sx_lo = 0;
    sx_hi = x1 - sprite_x;
    if (sx_hi > size)
      sx_hi = size;

    for (sx = sx_lo; sx < sx_hi; sx++) {
      int src_x = x_flip ? (size - 1 - sx) : sx;
      int tx = src_x >> 3;
      uint8_t color;
      /* src_x is monotonic in sx, so this re-decodes once per 8 pixels. */
      if (tx != cur_tx) {
        cur_tx = tx;
        decode_row_4bpp(ppu,
                        (size_t)name_base + (size_t)obj_tile_byte(charnum, tx, ty),
                        row, px8);
      }
      color = px8[src_x & 7];
      if (color != 0)
        out[sprite_x + sx] = cpal[color];
    }
  }
}

void PicoScanline_Mode1Range(const PicoPpuState *ppu, int y, uint16_t *out256,
                             int x0, int x1) {
  const uint16_t *pal;
  uint16_t backdrop;

  if (ppu == NULL || out256 == NULL || y < 0 || y >= kPicoScreenHeight)
    return;
  if (x0 < 0)
    x0 = 0;
  if (x1 > kPicoScreenWidth)
    x1 = kPicoScreenWidth;
  if (x0 >= x1)
    return;

  pal = palette_for(ppu);
  backdrop = pal[0];
  build_obj_lists(ppu, y, x0, x1);

  /* Mode 1 back-to-front without BG3. The explicit backdrop fill is fused into
   * the first pass. Sprite 32-tile/line cap is not applied. */
  blit_bg(ppu, 2, 0, y, out256, pal, x0, x1, 1, backdrop);
  blit_obj(ppu, 0, y, out256, pal, x0, x1);
  blit_bg(ppu, 1, 0, y, out256, pal, x0, x1, 0, backdrop);
  blit_obj(ppu, 1, y, out256, pal, x0, x1);
  blit_bg(ppu, 2, 1, y, out256, pal, x0, x1, 0, backdrop);
  blit_obj(ppu, 2, y, out256, pal, x0, x1);
  blit_bg(ppu, 1, 1, y, out256, pal, x0, x1, 0, backdrop);
  blit_obj(ppu, 3, y, out256, pal, x0, x1);
}

void PicoScanline_Mode1(const PicoPpuState *ppu, int y, uint16_t *out256) {
  PicoScanline_Mode1Range(ppu, y, out256, 0, kPicoScreenWidth);
}
