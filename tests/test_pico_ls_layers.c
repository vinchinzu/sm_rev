/*
 * Packed Landing Site layer isolation (no game kernel, no SDL, no sm.smc).
 * Rasters BG1-only / BG2-only / OBJ-only / composite and writes PPMs.
 * OBJ/composite plant packed pose-1 Samus; gunship is unhooked.
 *
 * PicoScanline_Mode1 does not read tm. Isolate layers by copying VRAM and
 * filling the other layer's tilemap with transparent air (word 0x0338,
 * tileset 00 tile 824 is color-0). Zeroing a map draws tile 0, which is
 * opaque CHR, not an empty layer.
 *
 *   make pico-ls-layers-test
 *
 * Do not pass -Isrc: it shadows glibc's <features.h> with src/features.h.
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "mini_asset_bootstrap.h"
#include "pico_ls_assets.h"
#include "pico_oam_from_samus.h"
#include "pico_oam_gunship.h"

enum {
  kBg1MapWord = 0x5000,
  kBg2MapWord = 0x4800,
  kMapWords = 2048,
  /* tileset 00 air: color-0 8x8. Map word 0 is opaque tile 0. */
  kTransparentMapWord = 0x0338,
  /* Boot framing, derived exactly as pico2_main does it, so this composite is
   * comparable to the panel: camera = clamp(spawn - viewport/2, window). */
  kBootCamX = kPicoLsRoomSpawnX - 256 / 2 < kPicoLsExtractCameraX
                  ? kPicoLsExtractCameraX
                  : kPicoLsRoomSpawnX - 256 / 2,
  kBootCamY = kPicoLsRoomSpawnY - 224 / 2 < kPicoLsExtractCameraY
                  ? kPicoLsExtractCameraY
                  : kPicoLsRoomSpawnY - 224 / 2,
  kBootHofs = kBootCamX - kPicoLsExtractCameraX,
  kBootVofs = kBootCamY - kPicoLsExtractCameraY,
  kSamusOriginX = kPicoLsRoomSpawnX - kBootCamX,
  kSamusOriginY = kPicoLsRoomSpawnY - kBootCamY - 6,
  kGunshipScreenX = 128,
  kGunshipScreenY = 136,
  kMagentaBgr555 = 0x7C1F,
  kFramePx = kPicoScreenWidth * kPicoScreenHeight,
  /* Stale BG2 VRAM fill: tile 0x0F, palette 3, and priority 1 so it paints
   * over BG1. sm_rev-k5q.5: the packer used to copy 1024 of these. */
  kBg2StaleFill = 0x2C0F,
  kBgPriorityBit = 0x2000,
  /* Between the gunship's underside and the terrain strip: the band that was
   * uniform wallpaper. */
  kBg2BandY0 = 157,
  kBg2BandY1 = 207,
  /* pico2_main clamps the camera to the packed window, so BG2 -- which runs at
   * half the BG1 rate -- travels this far. The second 32-tile copy of the
   * backdrop is on screen here, so this catches a fix that only repaired the
   * first screen. */
  kBg2MaxHofs = (512 - kPicoScreenWidth) / 2,
  kBg2ScreenWords = 32 * 32,
  kBg2FileWords = 2048
};

/* room_91F8.json backgroundAssets.defaultVariantKey. The packer reads this
 * same file; the test re-reads it to check the mini renderer agrees. */
#define kBg2SourceBin "assets/local_mini/backgrounds/bg2_91F8_door_896A.bin"

static int g_failures;

static void expect_true(const char *name, int cond) {
  if (!cond) {
    fprintf(stderr, "FAIL %s\n", name);
    g_failures++;
  }
}

static void fill_map_transparent(uint8_t *vram, uint16_t word_base) {
  size_t off = (size_t)word_base << 1;
  size_t i;
  uint8_t lo = (uint8_t)kTransparentMapWord;
  uint8_t hi = (uint8_t)(kTransparentMapWord >> 8);
  for (i = 0; i < (size_t)kMapWords; i++) {
    vram[off + i * 2u] = lo;
    vram[off + i * 2u + 1u] = hi;
  }
}

static void raster_packet(const PicoFramePacket *pkt, uint16_t *out) {
  PicoPpuState ppu;
  int y;

  PicoFramePacket_ToPpu(pkt, &ppu);
  for (y = 0; y < kPicoScreenHeight; y++)
    PicoScanline_Mode1(&ppu, y, out + (size_t)y * kPicoScreenWidth);
}

static int write_ppm(const char *path, const uint16_t *frame) {
  FILE *f;
  int i;
  int n = kFramePx;

  f = fopen(path, "wb");
  if (f == NULL)
    return 0;
  fprintf(f, "P6\n%d %d\n255\n", kPicoScreenWidth, kPicoScreenHeight);
  for (i = 0; i < n; i++) {
    uint16_t c = frame[i];
    uint8_t rgb[3];
    rgb[0] = (uint8_t)(((c >> 11) & 0x1F) * 255 / 31);
    rgb[1] = (uint8_t)(((c >> 5) & 0x3F) * 255 / 63);
    rgb[2] = (uint8_t)((c & 0x1F) * 255 / 31);
    if (fwrite(rgb, 1, 3, f) != 3) {
      fclose(f);
      return 0;
    }
  }
  fclose(f);
  return 1;
}

typedef struct ColorBox {
  uint16_t color;
  int count;
  int min_x;
  int max_x;
  int min_y;
  int max_y;
} ColorBox;

typedef struct LayerStats {
  uint16_t backdrop;
  int non_backdrop;
  int unique_non_backdrop;
  uint16_t filler;
  int filler_count;
  int terrain; /* non-backdrop, excluding full-frame repeating filler colors */
  int unique_terrain;
  int min_x;
  int max_x;
  int min_y;
  int max_y;
} LayerStats;

static int color_is_full_frame(const ColorBox *b) {
  int w;
  int h;
  if (b->count <= 0)
    return 0;
  w = b->max_x - b->min_x + 1;
  h = b->max_y - b->min_y + 1;
  /* Repeating 8x8 air/filler paints nearly the whole 256x224 viewport. */
  return w >= 240 && h >= 200;
}

static LayerStats analyze_layer(const uint16_t *frame, uint16_t backdrop) {
  ColorBox boxes[256];
  int box_n;
  LayerStats s;
  int x;
  int y;
  int j;
  uint16_t c;
  uint8_t is_filler[256];

  s.backdrop = backdrop;
  s.non_backdrop = 0;
  s.unique_non_backdrop = 0;
  s.filler = 0;
  s.filler_count = 0;
  s.terrain = 0;
  s.unique_terrain = 0;
  s.min_x = kPicoScreenWidth;
  s.max_x = -1;
  s.min_y = kPicoScreenHeight;
  s.max_y = -1;
  box_n = 0;
  memset(is_filler, 0, sizeof(is_filler));

  for (y = 0; y < kPicoScreenHeight; y++) {
    for (x = 0; x < kPicoScreenWidth; x++) {
      c = frame[y * kPicoScreenWidth + x];
      if (c == backdrop)
        continue;
      s.non_backdrop++;
      for (j = 0; j < box_n; j++) {
        if (boxes[j].color == c)
          break;
      }
      if (j == box_n) {
        if (box_n >= 256)
          continue;
        boxes[box_n].color = c;
        boxes[box_n].count = 0;
        boxes[box_n].min_x = x;
        boxes[box_n].max_x = x;
        boxes[box_n].min_y = y;
        boxes[box_n].max_y = y;
        j = box_n++;
      }
      boxes[j].count++;
      if (x < boxes[j].min_x)
        boxes[j].min_x = x;
      if (x > boxes[j].max_x)
        boxes[j].max_x = x;
      if (y < boxes[j].min_y)
        boxes[j].min_y = y;
      if (y > boxes[j].max_y)
        boxes[j].max_y = y;
    }
  }
  s.unique_non_backdrop = box_n;

  for (j = 0; j < box_n; j++) {
    if (boxes[j].count > s.filler_count) {
      s.filler_count = boxes[j].count;
      s.filler = boxes[j].color;
    }
    is_filler[j] = (uint8_t)color_is_full_frame(&boxes[j]);
  }

  for (j = 0; j < box_n; j++) {
    int w;
    int h;
    if (is_filler[j])
      continue;
    w = boxes[j].max_x - boxes[j].min_x + 1;
    h = boxes[j].max_y - boxes[j].min_y + 1;
    if (w < 16 && h < 8)
      continue;
    s.unique_terrain++;
    s.terrain += boxes[j].count;
    if (boxes[j].min_x < s.min_x)
      s.min_x = boxes[j].min_x;
    if (boxes[j].max_x > s.max_x)
      s.max_x = boxes[j].max_x;
    if (boxes[j].min_y < s.min_y)
      s.min_y = boxes[j].min_y;
    if (boxes[j].max_y > s.max_y)
      s.max_y = boxes[j].max_y;
  }
  return s;
}

/* Old LS gunship sat in extract space near (128, 136). Samus origin (40, 80)
 * does not use that OAM window. */
static int gunship_oam_present(const PicoFramePacket *pkt) {
  int i;
  for (i = 0; i < kPicoSpriteCount; i++) {
    uint8_t x = pkt->oam[(size_t)i * 4u];
    uint8_t y = pkt->oam[(size_t)i * 4u + 1u];
    if (y == 224)
      continue;
    if ((int)x >= 32 && (int)x <= 220 && (int)y >= 100 && (int)y <= 160)
      return 1;
  }
  return 0;
}

typedef struct ObjCluster {
  int count;
  int unique;
  int min_x;
  int max_x;
  int min_y;
  int max_y;
} ObjCluster;

/* Non-BG pixels in the old gunship body, away from the Samus plant origin. */
static ObjCluster gunship_cluster(const uint16_t *composite, const uint16_t *bg) {
  ObjCluster c;
  int x;
  int y;

  c.count = 0;
  c.unique = 0;
  c.min_x = kPicoScreenWidth;
  c.max_x = -1;
  c.min_y = kPicoScreenHeight;
  c.max_y = -1;
  for (y = 0; y < kPicoScreenHeight; y++) {
    for (x = 0; x < kPicoScreenWidth; x++) {
      int i = y * kPicoScreenWidth + x;
      if (composite[i] == bg[i])
        continue;
      if (x < 100 || x > 220)
        continue;
      if (y < 120 || y > 180)
        continue;
      c.count++;
      if (x < c.min_x)
        c.min_x = x;
      if (x > c.max_x)
        c.max_x = x;
      if (y < c.min_y)
        c.min_y = y;
      if (y > c.max_y)
        c.max_y = y;
    }
  }
  return c;
}

/* OBJ-only pixels vs backdrop over the WHOLE 256x224 frame, no window.
 * Used for Samus (multi-color, not a 16x16 solid) and for the gunship-only
 * pass, whose rows fall outside gunship_cluster()'s hard y clip. */
static ObjCluster obj_cluster_full(const uint16_t *obj, uint16_t backdrop) {
  uint16_t colors[64];
  int color_n;
  ObjCluster c;
  int x;
  int y;
  int j;

  c.count = 0;
  c.unique = 0;
  c.min_x = kPicoScreenWidth;
  c.max_x = -1;
  c.min_y = kPicoScreenHeight;
  c.max_y = -1;
  color_n = 0;
  for (y = 0; y < kPicoScreenHeight; y++) {
    for (x = 0; x < kPicoScreenWidth; x++) {
      uint16_t px = obj[y * kPicoScreenWidth + x];
      if (px == backdrop)
        continue;
      c.count++;
      if (x < c.min_x)
        c.min_x = x;
      if (x > c.max_x)
        c.max_x = x;
      if (y < c.min_y)
        c.min_y = y;
      if (y > c.max_y)
        c.max_y = y;
      for (j = 0; j < color_n; j++) {
        if (colors[j] == px)
          break;
      }
      if (j == color_n && color_n < 64)
        colors[color_n++] = px;
    }
  }
  c.unique = color_n;
  return c;
}

/* Distinct colours in a horizontal band, backdrop included. */
static int band_unique_colors(const uint16_t *frame, int y0, int y1) {
  uint16_t colors[256];
  int n = 0;
  int x;
  int y;
  int j;

  for (y = y0; y <= y1 && y < kPicoScreenHeight; y++) {
    for (x = 0; x < kPicoScreenWidth; x++) {
      uint16_t px = frame[y * kPicoScreenWidth + x];
      for (j = 0; j < n; j++) {
        if (colors[j] == px)
          break;
      }
      if (j == n && n < 256)
        colors[n++] = px;
    }
  }
  return n;
}

/* Words in a packed tilemap matching a mask/value, read straight out of VRAM. */
static int map_word_count(const uint8_t *vram, uint16_t word_base, uint16_t mask,
                          uint16_t value) {
  size_t off = (size_t)word_base << 1;
  int n = 0;
  int i;

  for (i = 0; i < kMapWords; i++) {
    uint16_t w = (uint16_t)(vram[off + (size_t)i * 2u] |
                            ((uint16_t)vram[off + (size_t)i * 2u + 1u] << 8));
    if ((w & mask) == value)
      n++;
  }
  return n;
}

/* Distinct tilemap words and tile indices in the visible 32x28 tile window,
 * i.e. what a 256x224 frame shows at scroll (0,0). */
static void map_window_unique(const uint8_t *vram, uint16_t word_base,
                              int *out_words, int *out_tiles) {
  uint16_t words[512];
  uint16_t tiles[512];
  int word_n = 0;
  int tile_n = 0;
  int tx;
  int ty;
  int j;

  for (ty = 0; ty < kPicoScreenHeight / 8; ty++) {
    for (tx = 0; tx < kPicoScreenWidth / 8; tx++) {
      size_t addr = (size_t)word_base + (size_t)ty * 32u + (size_t)tx;
      uint16_t w = (uint16_t)(vram[addr * 2u] |
                              ((uint16_t)vram[addr * 2u + 1u] << 8));
      uint16_t t = (uint16_t)(w & 0x3FF);
      for (j = 0; j < word_n; j++) {
        if (words[j] == w)
          break;
      }
      if (j == word_n && word_n < 512)
        words[word_n++] = w;
      for (j = 0; j < tile_n; j++) {
        if (tiles[j] == t)
          break;
      }
      if (j == tile_n && tile_n < 512)
        tiles[tile_n++] = t;
    }
  }
  *out_words = word_n;
  *out_tiles = tile_n;
}

/* SNES 64x32 tilemap address: two 32x32 screens, 0x400 words apart. */
static size_t snes_map_index(int tile_x, int tile_y) {
  size_t addr = (size_t)(tile_y & 31) * 32u + (size_t)(tile_x & 31);
  if (tile_x & 32)
    addr += 0x400u;
  return addr;
}

static uint16_t map_word_at(const uint8_t *vram, uint16_t word_base, size_t index) {
  size_t addr = ((size_t)word_base + index) * 2u;
  return (uint16_t)(vram[addr] | ((uint16_t)vram[addr + 1u] << 8));
}

static void map_word_set(uint8_t *vram, uint16_t word_base, size_t index, uint16_t w) {
  size_t addr = ((size_t)word_base + index) * 2u;
  vram[addr] = (uint8_t)w;
  vram[addr + 1u] = (uint8_t)(w >> 8);
}

/* The raw two-screen BG2 VRAM dump the packer and the mini both start from. */
static int read_bg2_source(uint16_t *out) {
  FILE *f = fopen(kBg2SourceBin, "rb");
  int i;
  if (f == NULL)
    return 0;
  for (i = 0; i < kBg2FileWords; i++) {
    int lo = fgetc(f);
    int hi = fgetc(f);
    if (lo < 0 || hi < 0) {
      fclose(f);
      return 0;
    }
    out[i] = (uint16_t)(lo | (hi << 8));
  }
  i = fgetc(f);
  fclose(f);
  return i == EOF;
}

/* Rows of the packed map that are a single word repeated across all 64
 * columns. The real backdrop's sky rows legitimately are, so only the band
 * under the ship is measured: mountains and foliage, never one flat tile. */
static int map_uniform_rows(const uint8_t *vram, uint16_t word_base, int first_row,
                            int last_row) {
  int ty;
  int tx;
  int n = 0;
  for (ty = first_row; ty <= last_row; ty++) {
    uint16_t first = map_word_at(vram, word_base, snes_map_index(0, ty));
    int uniform = 1;
    for (tx = 1; tx < 64; tx++) {
      if (map_word_at(vram, word_base, snes_map_index(tx, ty)) != first) {
        uniform = 0;
        break;
      }
    }
    n += uniform;
  }
  return n;
}

/* Park every sprite outside [first,last] so one gunship piece renders alone. */
static void keep_only_slots(PicoFramePacket *pkt, int first, int last) {
  int i;
  for (i = 0; i < kPicoSpriteCount; i++) {
    if (i < first || i > last)
      pkt->oam[(size_t)i * 4u + 1u] = 224;
  }
}

/* Pixels where two OBJ-only rasters both painted something. */
static int overlap_px(const uint16_t *a, const uint16_t *b, uint16_t backdrop) {
  int i;
  int n = 0;
  for (i = 0; i < kFramePx; i++) {
    if (a[i] != backdrop && b[i] != backdrop)
      n++;
  }
  return n;
}

static int bg1_terrain_on_screen(const LayerStats *s) {
  int bbox_w;
  int bbox_h;
  int variety;
  int spatial;

  if (s->terrain <= 0)
    return 0;
  bbox_w = s->max_x - s->min_x + 1;
  bbox_h = s->max_y - s->min_y + 1;
  variety = s->unique_terrain >= 3;
  spatial = bbox_w >= 16 && bbox_h >= 8;
  return variety || spatial;
}

static void print_layer_stats(const char *tag, const LayerStats *s) {
  int bbox_w = 0;
  int bbox_h = 0;
  if (s->terrain > 0) {
    bbox_w = s->max_x - s->min_x + 1;
    bbox_h = s->max_y - s->min_y + 1;
  }
  printf("%s non_bd=%d unique_non_bd=%d filler=0x%04X filler_n=%d "
         "terrain=%d unique_terrain=%d bbox=%dx%d\n",
         tag, s->non_backdrop, s->unique_non_backdrop, (unsigned)s->filler,
         s->filler_count, s->terrain, s->unique_terrain, bbox_w, bbox_h);
}

int main(void) {
  static PicoFramePacket pkt;
  static uint8_t master_vram[kPicoVramSize];
  static uint8_t work_vram[kPicoVramSize];
  static uint16_t frame_bg1[kPicoScreenHeight * kPicoScreenWidth];
  static uint16_t frame_bg1_vofs32[kPicoScreenHeight * kPicoScreenWidth];
  static uint16_t frame_bg2[kPicoScreenHeight * kPicoScreenWidth];
  static uint16_t frame_bg2_scrolled[kPicoScreenHeight * kPicoScreenWidth];
  static uint16_t frame_bg2_linear[kPicoScreenHeight * kPicoScreenWidth];
  static uint16_t bg2_src[kBg2FileWords];
  static uint16_t frame_obj[kPicoScreenHeight * kPicoScreenWidth];
  static uint16_t frame_composite[kPicoScreenHeight * kPicoScreenWidth];
  static uint16_t frame_bg[kPicoScreenHeight * kPicoScreenWidth];
  static uint16_t frame_ship[kPicoScreenHeight * kPicoScreenWidth];
  static uint16_t frame_ship_top[kPicoScreenHeight * kPicoScreenWidth];
  static uint16_t frame_ship_body[kPicoScreenHeight * kPicoScreenWidth];
  uint16_t backdrop;
  uint16_t magenta;
  LayerStats bg1s;
  LayerStats bg1s32;
  LayerStats bg2s;
  ObjCluster ship;
  ObjCluster ship_only;
  ObjCluster samus;
  int magenta_count;
  int samus_w;
  int samus_h;
  int ship_only_w;
  int ship_only_h;
  int ship_piece_overlap;
  int bg2_stale;
  int bg2_priority;
  int bg2_band_colors;
  int bg2_win_words;
  int bg2_win_tiles;
  int bg2_uniform_rows;
  int bg2_right_screen_mismatch;
  int bg2_scrolled_band;
  int bg2_mini_mismatch;
  int bg2_live_screen;
  int bg2_linear_band;
  int bg2_linear_uniform_rows;
  int tx;
  int ty;
  int i;

  PicoFramePacket_InitLandingSiteExtracted(&pkt);
  expect_true("ls vram", pkt.vram != NULL && pkt.vram_size == (size_t)kPicoVramSize);
  memcpy(master_vram, pkt.vram, kPicoVramSize);
  pkt.vram = master_vram;
  expect_true("plant samus gfx",
              PicoOam_PlantSamusFrame(&pkt, PicoOam_SamusFrameIndex(1, 0)));
  expect_true("plant gunship gfx", PicoOam_PlantGunshipGfx(&pkt));
  PicoOam_ParkAllSprites(&pkt);

  backdrop = PicoBgr555ToRgb565(pkt.cgram[0]);
  magenta = PicoBgr555ToRgb565((uint16_t)kMagentaBgr555);

  /* BG1-only at packet default scroll (0,0): BG2 map air, no OAM. */
  memcpy(work_vram, master_vram, kPicoVramSize);
  fill_map_transparent(work_vram, (uint16_t)kBg2MapWord);
  pkt.vram = work_vram;
  pkt.oam_full = 0;
  pkt.bg1hofs = 0;
  pkt.bg1vofs = 0;
  pkt.bg2hofs = 0;
  pkt.bg2vofs = 0;
  raster_packet(&pkt, frame_bg1);

  /* Diagnostic only: vofs=32 currently reveals packed ty 28-31. Not the pass. */
  pkt.bg1vofs = 32;
  raster_packet(&pkt, frame_bg1_vofs32);
  pkt.bg1vofs = 0;

  /* BG2-only: BG1 map air, no OAM. */
  memcpy(work_vram, master_vram, kPicoVramSize);
  fill_map_transparent(work_vram, (uint16_t)kBg1MapWord);
  pkt.vram = work_vram;
  pkt.oam_full = 0;
  raster_packet(&pkt, frame_bg2);

  /* OBJ-only: both maps air, stand pose frame 0 at spritemap origin (40,80). */
  memcpy(work_vram, master_vram, kPicoVramSize);
  fill_map_transparent(work_vram, (uint16_t)kBg1MapWord);
  fill_map_transparent(work_vram, (uint16_t)kBg2MapWord);
  pkt.vram = work_vram;
  PicoOam_WriteSamusFrame(&pkt, PicoOam_SamusFrameIndex(1, 0), kSamusOriginX,
                          kSamusOriginY);
  PicoOam_WriteGunship(&pkt, 0, 0);
  raster_packet(&pkt, frame_obj);

  /* Gunship-only OBJ pass (sm_rev-k5q.4). No Samus, both maps air, scroll 0,
   * bbox over the whole frame with NO window: gunship_cluster() hard-clips
   * y to [120,180], which pins the height at 38 whatever the ship does. */
  memcpy(work_vram, master_vram, kPicoVramSize);
  fill_map_transparent(work_vram, (uint16_t)kBg1MapWord);
  fill_map_transparent(work_vram, (uint16_t)kBg2MapWord);
  pkt.vram = work_vram;
  pkt.bg1hofs = 0;
  pkt.bg1vofs = 0;
  pkt.bg2hofs = 0;
  pkt.bg2vofs = 0;
  PicoOam_ParkAllSprites(&pkt);
  PicoOam_WriteGunship(&pkt, 0, 0);
  raster_packet(&pkt, frame_ship);
  /* Same pass twice more, one ship piece at a time: landing_ship_top owns
   * gunship slots 0..17, landing_ship_bottom_front 18..51. They must not
   * paint the same pixel. */
  PicoOam_ParkAllSprites(&pkt);
  PicoOam_WriteGunship(&pkt, 0, 0);
  keep_only_slots(&pkt, kPicoLsGunshipFirstSlot, kPicoLsGunshipFirstSlot + 17);
  raster_packet(&pkt, frame_ship_top);
  PicoOam_ParkAllSprites(&pkt);
  PicoOam_WriteGunship(&pkt, 0, 0);
  keep_only_slots(&pkt, kPicoLsGunshipFirstSlot + 18,
                  kPicoLsGunshipFirstSlot + 51);
  raster_packet(&pkt, frame_ship_body);

  /* Composite: both maps + Samus + the ship she stands on, at boot framing. */
  memcpy(work_vram, master_vram, kPicoVramSize);
  pkt.vram = work_vram;
  pkt.bg1hofs = kBootHofs;
  pkt.bg1vofs = kBootVofs;
  pkt.bg2hofs = kBootHofs;
  pkt.bg2vofs = kBootVofs;
  PicoOam_WriteSamusFrame(&pkt, PicoOam_SamusFrameIndex(1, 0), kSamusOriginX,
                          kSamusOriginY);
  PicoOam_WriteGunship(&pkt, -kBootHofs, -kBootVofs);
  expect_true("gunship OAM present", gunship_oam_present(&pkt));
  raster_packet(&pkt, frame_composite);
  {
    /* sm_rev-k5q.12: host-measurable raster time for one packed LS frame.
     * Warm (OAM span cache already built by the composite pass above). */
    enum { kBenchFrames = 50 };
    struct timespec t0;
    struct timespec t1;
    int b;
    unsigned us;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (b = 0; b < kBenchFrames; b++)
      raster_packet(&pkt, frame_composite);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    us = (unsigned)((t1.tv_sec - t0.tv_sec) * 1000000L +
                    (t1.tv_nsec - t0.tv_nsec) / 1000L);
    printf("raster_us=%u (packed LS composite, %d-frame avg %u us)\n", us,
           kBenchFrames, us / (unsigned)kBenchFrames);
  }
  pkt.oam_full = 0;
  raster_packet(&pkt, frame_bg);
  pkt.oam_full = 1;

  (void)mkdir("out", 0755);
  (void)write_ppm("out/pico_ls_bg1.ppm", frame_bg1);
  (void)write_ppm("out/pico_ls_bg2.ppm", frame_bg2);
  (void)write_ppm("out/pico_ls_obj.ppm", frame_obj);
  (void)write_ppm("out/pico_ls_composite.ppm", frame_composite);

  bg1s = analyze_layer(frame_bg1, backdrop);
  bg1s32 = analyze_layer(frame_bg1_vofs32, backdrop);
  bg2s = analyze_layer(frame_bg2, backdrop);
  print_layer_stats("bg1 vofs=0", &bg1s);
  print_layer_stats("bg1 vofs=32 (diag, not pass)", &bg1s32);
  print_layer_stats("bg2 vofs=0", &bg2s);
  bg2_stale = map_word_count(master_vram, (uint16_t)kBg2MapWord, 0xFFFF,
                             (uint16_t)kBg2StaleFill);
  bg2_priority = map_word_count(master_vram, (uint16_t)kBg2MapWord,
                                (uint16_t)kBgPriorityBit,
                                (uint16_t)kBgPriorityBit);
  bg2_band_colors = band_unique_colors(frame_bg2, kBg2BandY0, kBg2BandY1);
  map_window_unique(master_vram, (uint16_t)kBg2MapWord, &bg2_win_words,
                    &bg2_win_tiles);
  printf("bg2 stale_0x%04X=%d priority_words=%d window unique_words=%d "
         "unique_tiles=%d band y=%d..%d colors=%d\n",
         (unsigned)kBg2StaleFill, bg2_stale, bg2_priority, bg2_win_words,
         bg2_win_tiles, (int)kBg2BandY0, (int)kBg2BandY1, bg2_band_colors);

  /* Every packed row must be real backdrop. Uniform wallpaper -- one word
   * repeated across all 64 columns -- is exactly the failure this bead is
   * about, and the linear-64 control below scores 16 here. */
  bg2_uniform_rows = map_uniform_rows(master_vram, (uint16_t)kBg2MapWord,
                                      kBg2BandY0 / 8, kBg2BandY1 / 8);
  /* The packer tiles one 32x32 screen twice, so the right screen must repeat
   * the left. A fix that only repaired columns 0-31 fails here. */
  bg2_right_screen_mismatch = 0;
  for (ty = 0; ty < 32; ty++) {
    for (tx = 0; tx < 32; tx++) {
      if (map_word_at(master_vram, (uint16_t)kBg2MapWord, snes_map_index(tx, ty)) !=
          map_word_at(master_vram, (uint16_t)kBg2MapWord, snes_map_index(tx + 32, ty)))
        bg2_right_screen_mismatch++;
    }
  }

  /* BG2 at the far end of its travel (half the BG1 rate, vofs locked): the
   * second copy of the backdrop is on screen. */
  memcpy(work_vram, master_vram, kPicoVramSize);
  fill_map_transparent(work_vram, (uint16_t)kBg1MapWord);
  pkt.vram = work_vram;
  pkt.oam_full = 0;
  pkt.bg1hofs = 0;
  pkt.bg1vofs = 0;
  pkt.bg2hofs = (uint16_t)kBg2MaxHofs;
  pkt.bg2vofs = (uint16_t)kPicoLsBg2VerticalScroll;
  raster_packet(&pkt, frame_bg2_scrolled);
  (void)write_ppm("out/pico_ls_bg2_scrolled.ppm", frame_bg2_scrolled);
  bg2_scrolled_band = band_unique_colors(frame_bg2_scrolled, kBg2BandY0, kBg2BandY1);

  /* sm_rev-k5q.8: mini_renderer.c indexed this same .bin as a linear 64-wide
   * map. Rebuild the window the way MiniAssetBootstrap_Bg2Word now does and
   * require it to agree with the packer word for word, so a mini screenshot
   * and out/pico_ls_composite.ppm are the same oracle again. */
  expect_true("BG2 source bin readable", read_bg2_source(bg2_src));
  bg2_live_screen = MiniAssetBootstrap_Bg2LiveScreen(bg2_src);
  bg2_mini_mismatch = 0;
  for (ty = 0; ty < 32; ty++) {
    for (tx = 0; tx < 64; tx++) {
      if (MiniAssetBootstrap_Bg2Word(bg2_src, bg2_live_screen, tx, ty) !=
          map_word_at(master_vram, (uint16_t)kBg2MapWord, snes_map_index(tx, ty)))
        bg2_mini_mismatch++;
    }
  }

  /* Negative control: the old linear-64 reading, rastered and measured, so the
   * assertions above are known to reject the bug and not merely to pass. */
  memcpy(work_vram, master_vram, kPicoVramSize);
  fill_map_transparent(work_vram, (uint16_t)kBg1MapWord);
  for (ty = 0; ty < 32; ty++) {
    for (tx = 0; tx < 64; tx++)
      map_word_set(work_vram, (uint16_t)kBg2MapWord, snes_map_index(tx, ty),
                   bg2_src[ty * 64 + tx]);
  }
  pkt.vram = work_vram;
  pkt.oam_full = 0;
  pkt.bg1hofs = 0;
  pkt.bg1vofs = 0;
  pkt.bg2hofs = 0;
  pkt.bg2vofs = 0;
  raster_packet(&pkt, frame_bg2_linear);
  (void)write_ppm("out/pico_ls_bg2_linear64_bug.ppm", frame_bg2_linear);
  bg2_linear_band = band_unique_colors(frame_bg2_linear, kBg2BandY0, kBg2BandY1);
  bg2_linear_uniform_rows =
      map_uniform_rows(work_vram, (uint16_t)kBg2MapWord, kBg2BandY0 / 8, kBg2BandY1 / 8);
  /* Leave the packet as the composite pass left it: the run-frame check below
   * rasters OBJ from this state. */
  pkt.oam_full = 1;
  pkt.bg1hofs = (uint16_t)kBootHofs;
  pkt.bg1vofs = (uint16_t)kBootVofs;
  pkt.bg2hofs = (uint16_t)kBootHofs;
  pkt.bg2vofs = (uint16_t)kBootVofs;
  printf("bg2 band_uniform_rows=%d right_screen_mismatch=%d scrolled(hofs=%d) "
         "band_colors=%d\n",
         bg2_uniform_rows, bg2_right_screen_mismatch, (int)kBg2MaxHofs,
         bg2_scrolled_band);
  printf("bg2 mini live_screen=%d mismatch_vs_packed=%d | linear-64 control "
         "band_colors=%d uniform_rows=%d\n",
         bg2_live_screen, bg2_mini_mismatch, bg2_linear_band,
         bg2_linear_uniform_rows);

  magenta_count = 0;
  for (i = 0; i < kFramePx; i++) {
    if (frame_composite[i] == magenta)
      magenta_count++;
  }
  samus = obj_cluster_full(frame_obj, backdrop);
  samus_w = samus.count ? samus.max_x - samus.min_x + 1 : 0;
  samus_h = samus.count ? samus.max_y - samus.min_y + 1 : 0;
  printf("composite magenta=%d (want <16)\n", magenta_count);
  ship = gunship_cluster(frame_composite, frame_bg);
  printf("gunship cluster n=%d bbox=%dx%d origin=(%d,%d)\n", ship.count,
         ship.count ? ship.max_x - ship.min_x + 1 : 0,
         ship.count ? ship.max_y - ship.min_y + 1 : 0, kGunshipScreenX,
         kGunshipScreenY);
  printf("boot cam=%d,%d scroll=%d,%d samus origin=(%d,%d)\n", (int)kBootCamX,
         (int)kBootCamY, (int)kBootHofs, (int)kBootVofs, (int)kSamusOriginX,
         (int)kSamusOriginY);
  printf("samus cluster n=%d unique=%d bbox=%dx%d origin=(%d,%d)\n",
         samus.count, samus.unique, samus_w, samus_h, kSamusOriginX,
         kSamusOriginY);
  ship_only = obj_cluster_full(frame_ship, backdrop);
  ship_only_w = ship_only.count ? ship_only.max_x - ship_only.min_x + 1 : 0;
  ship_only_h = ship_only.count ? ship_only.max_y - ship_only.min_y + 1 : 0;
  ship_piece_overlap = overlap_px(frame_ship_top, frame_ship_body, backdrop);
  printf("gunship-only n=%d bbox=%dx%d y=%d..%d x=%d..%d top/body overlap=%d\n",
         ship_only.count, ship_only_w, ship_only_h, ship_only.min_y,
         ship_only.max_y, ship_only.min_x, ship_only.max_x,
         ship_piece_overlap);
  fflush(stdout);

  expect_true("BG2-only many non-backdrop pixels", bg2s.non_backdrop > 10000);
  /* sm_rev-k5q.5: the BG2 .bin is two SNES 32x32 screens, not a 64x32 linear
   * map. Reading it linearly mixed the live screen with the stale VRAM fill,
   * which was 1024 of the 2048 packed words and painted over BG1. */
  expect_true("BG2 has no stale VRAM fill", bg2_stale == 0);
  expect_true("BG2 has no priority-1 words", bg2_priority == 0);
  /* Measured before the fix: 53 words / 50 tiles / 4 colours in the band. */
  expect_true("BG2 window has >= 100 unique tilemap words", bg2_win_words >= 100);
  expect_true("BG2 window has >= 90 unique tile indices", bg2_win_tiles >= 90);
  expect_true("BG2 band under the ship is not wallpaper", bg2_band_colors >= 10);
  expect_true("no BG2 row under the ship is uniform wallpaper",
              bg2_uniform_rows == 0);
  expect_true("packed BG2 right screen repeats the left",
              bg2_right_screen_mismatch == 0);
  expect_true("BG2 is still real at the far end of its travel",
              bg2_scrolled_band >= 10);
  /* sm_rev-k5q.8 */
  expect_true("BG2 source has exactly one live 32x32 screen", bg2_live_screen == 0);
  expect_true("mini BG2 indexing matches the packed map", bg2_mini_mismatch == 0);
  expect_true("the linear-64 reading is wallpaper these checks reject",
              bg2_linear_band < 10 && bg2_linear_uniform_rows > 0);
  expect_true("composite magenta gone", magenta_count < 16);
  /* Word 0 is opaque tile 0. Air-fill isolation must not paint a full frame. */
  expect_true("BG1-only is not opaque tile-0 fill",
              bg1s.non_backdrop < kFramePx / 2);
  expect_true("BG1 unique terrain on screen at scroll 0",
              bg1_terrain_on_screen(&bg1s));
  /* The ship is the spawn floor: its collision blocks carry metatile 0xFF
   * (air graphics), so a zero cluster here means Samus stands on nothing. */
  expect_true("gunship cluster is drawn", ship.count > 200);
  /* sm_rev-k5q.4: landing_ship_top used to render 40 px low, on top of
   * landing_ship_bottom_front. Measured before the fix: 48 tall, 6906 px,
   * 3494 px of the two pieces overlapping. */
  expect_true("gunship-only bbox height is 80", ship_only_h == 80);
  expect_true("gunship-only bbox width is 188", ship_only_w == 188);
  expect_true("gunship-only non-backdrop px is 9668", ship_only.count == 9668);
  expect_true("gunship top and body do not overlap", ship_piece_overlap == 0);
  expect_true("samus unique OBJ colors >= 4", samus.unique >= 4);
  expect_true("samus bbox height >= 24", samus_h >= 24);
  expect_true("samus bbox width >= 12", samus_w >= 12);
  expect_true("samus pixel count > 80", samus.count > 80);
  expect_true("samus is not a solid 16x16",
              !(samus.count == 256 && samus_w == 16 && samus_h == 16 &&
                samus.unique <= 1));

  expect_true("stand pose packed", PicoOam_SamusPoseFrames(1) >= 1);
  expect_true("run-right pose packed", PicoOam_SamusPoseFrames(9) >= 4);
  expect_true("run-left pose packed", PicoOam_SamusPoseFrames(0x0A) >= 4);
  expect_true("turn-right pose packed", PicoOam_SamusPoseFrames(0x25) >= 2);
  expect_true("unpacked pose falls back, never invalid",
              PicoOam_SamusFrameIndex(0xFE, 3) >= 0);
  {
    /* Consecutive run frames must differ, or the walk cycle is a still. */
    int a = PicoOam_SamusFrameIndex(9, 0);
    int b = PicoOam_SamusFrameIndex(9, 1);
    int differs = 0;
    expect_true("run frames are distinct indices", a != b);
    memcpy(work_vram, master_vram, kPicoVramSize);
    pkt.vram = work_vram;
    PicoOam_PlantSamusFrame(&pkt, a);
    raster_packet(&pkt, frame_obj);
    {
      static uint16_t run_a[kFramePx];
      memcpy(run_a, frame_obj, sizeof(run_a));
      PicoOam_PlantSamusFrame(&pkt, b);
      raster_packet(&pkt, frame_obj);
      for (i = 0; i < kFramePx; i++)
        if (run_a[i] != frame_obj[i])
          differs++;
    }
    printf("run frame 0 vs 1 differing pixels: %d\n", differs);
    expect_true("run frame CHR actually changes", differs > 40);
  }

  if (g_failures) {
    fprintf(stderr, "%d failure(s)\n", g_failures);
    return 1;
  }
  puts("ok");
  return 0;
}
