/*
 * Host-paired proof: MiniStepButtons Right over extracted Landing Site tiles.
 * Game side packs OAM+scrolls every frame (VRAM blob on frame 1 only).
 * Display side decodes and rasters Mode 1. Authored 16x16 OAM blob tracks Samus.
 *
 *   make pico-kernel
 *   gcc -O2 -fno-strict-aliasing -Werror -DSYSTEM_VOLUME_MIXER_AVAILABLE=0 \
 *       -I. -Isrc/pico -iquote src -iquote src/mini -DCURRENT_BUILD=BUILD_PICO \
 *       tests/test_pico_move_tileset.c src/pico/pico_oam_from_samus.c \
 *       src/pico/pico_frame_wire.c src/pico/pico_frame_packet.c \
 *       src/pico/pico_ls_assets.c src/pico/scanline_mode1.c \
 *       -o sm_rev_pico_move_tileset_test -L. -lsm_rev_pico_kernel -lm
 *   ./sm_rev_pico_move_tileset_test
 *
 * Do not pass -Isrc: it shadows glibc's <features.h> with src/features.h.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ida_types.h"
#include "mini/mini_game.h"
#include "variables.h"

#include "pico_frame_wire.h"
#include "pico_ls_assets.h"
#include "pico_oam_from_samus.h"

enum {
  kMoveFrames = 30,
  kViewportW = 256,
  kViewportH = 224,
  kFramebufferBytes = kPicoScreenWidth * kPicoScreenHeight * 2
};

static int g_failures;

static void expect_true(const char *name, int cond) {
  if (!cond) {
    fprintf(stderr, "FAIL %s\n", name);
    g_failures++;
  }
}

static uint16_t extract_scroll(uint16_t camera, uint16_t origin) {
  if (camera >= origin)
    return (uint16_t)(camera - origin);
  return camera;
}

static void pack_from_samus(PicoFramePacket *pkt, uint32_t frame_id) {
  int sx = (int)samus_x_pos - (int)layer1_x_pos - (int)samus_x_radius;
  int sy = (int)samus_y_pos - (int)layer1_y_pos - (int)samus_y_radius;
  uint16_t hofs = extract_scroll(layer1_x_pos, (uint16_t)kPicoLsExtractCameraX);
  uint16_t vofs = extract_scroll(layer1_y_pos, (uint16_t)kPicoLsExtractCameraY);

  pkt->frame_id = frame_id;
  pkt->vsync_token = (uint16_t)kPicoFramePacketVsync;
  pkt->joypad_echo = kButton_Right;
  pkt->bg1hofs = hofs;
  pkt->bg1vofs = vofs;
  pkt->bg2hofs = hofs;
  pkt->bg2vofs = vofs;
  PicoOam_WriteSamusBlob(pkt, sx, sy);
}

static void raster_packet(const PicoFramePacket *pkt, uint16_t *out) {
  PicoPpuState ppu;
  int y;

  PicoFramePacket_ToPpu(pkt, &ppu);
  for (y = 0; y < kPicoScreenHeight; y++)
    PicoScanline_Mode1(&ppu, y, out + (size_t)y * kPicoScreenWidth);
}

static void raster_bg_only(const PicoFramePacket *pkt, uint16_t *out) {
  PicoFramePacket bg = *pkt;
  bg.oam_full = 0;
  raster_packet(&bg, out);
}

typedef struct SpriteStats {
  int count;
  int min_x;
  int max_x;
  int min_y;
  int over_bg;
} SpriteStats;

static SpriteStats measure_sprite(const uint16_t *with_obj, const uint16_t *bg_only) {
  SpriteStats s;
  int y;
  int x;

  s.count = 0;
  s.min_x = kPicoScreenWidth;
  s.max_x = -1;
  s.min_y = kPicoScreenHeight;
  s.over_bg = 0;
  for (y = 0; y < kPicoScreenHeight; y++) {
    for (x = 0; x < kPicoScreenWidth; x++) {
      int i = y * kPicoScreenWidth + x;
      if (with_obj[i] == bg_only[i])
        continue;
      s.count++;
      if (x < s.min_x)
        s.min_x = x;
      if (x > s.max_x)
        s.max_x = x;
      if (y < s.min_y)
        s.min_y = y;
      if (bg_only[i] != 0)
        s.over_bg++;
    }
  }
  return s;
}

static int write_ppm(const char *path, const uint16_t *frame) {
  FILE *f;
  int i;
  int n = kPicoScreenWidth * kPicoScreenHeight;

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

int main(void) {
  static PicoFramePacket src;
  static PicoFramePacket dst;
  static uint8_t game_vram[kPicoVramSize];
  static uint8_t display_vram[kPicoVramSize];
  static uint8_t wire[kPicoFrameWireMaxSize];
  static uint16_t frame_start[kPicoScreenHeight * kPicoScreenWidth];
  static uint16_t frame_end[kPicoScreenHeight * kPicoScreenWidth];
  static uint16_t frame_bg[kPicoScreenHeight * kPicoScreenWidth];
  MiniGameState *state;
  uint16 start_x;
  uint16 end_x;
  size_t n_first;
  size_t n_live;
  SpriteStats start_spr;
  SpriteStats end_spr;
  int i;

  state = MiniCreate(kViewportW, kViewportH);
  if (state == NULL) {
    fprintf(stderr, "test_pico_move_tileset: MiniCreate failed\n");
    return 1;
  }

  start_x = samus_x_pos;
  PicoFramePacket_InitLandingSiteExtracted(&src);
  expect_true("ls vram", src.vram != NULL && src.vram_size == (size_t)kPicoVramSize);
  memcpy(game_vram, src.vram, src.vram_size);
  src.vram = game_vram;
  expect_true("plant samus blob gfx", PicoOam_PlantSamusBlobGfx(&src));

  pack_from_samus(&src, 1);
  n_first = PicoFrameWire_Encode(&src, wire, sizeof(wire), 1);
  expect_true("encode first with vram", n_first == PicoFrameWire_EncodedSize(&src, 1));
  expect_true("decode first",
              PicoFrameWire_Decode(wire, n_first, &dst, display_vram, sizeof(display_vram)));
  raster_packet(&dst, frame_start);
  raster_bg_only(&dst, frame_bg);
  start_spr = measure_sprite(frame_start, frame_bg);

  for (i = 0; i < kMoveFrames; i++) {
    MiniStepButtons(state, kButton_Right, false);
    pack_from_samus(&src, (uint32_t)(i + 2));
    n_live = PicoFrameWire_Encode(&src, wire, sizeof(wire), 0);
    expect_true("live packet is header+oam+cgram",
                n_live == (size_t)kPicoFrameWireBaseSize);
    expect_true("live packet smaller than framebuffer",
                n_live > 0 && n_live < (size_t)kFramebufferBytes);
    expect_true("decode live retained vram",
                PicoFrameWire_Decode(wire, n_live, &dst, display_vram, sizeof(display_vram)));
    expect_true("display kept vram pointer", dst.vram == display_vram);
  }

  end_x = samus_x_pos;
  raster_packet(&dst, frame_end);
  raster_bg_only(&dst, frame_bg);
  end_spr = measure_sprite(frame_end, frame_bg);

  expect_true("samus_x increased", end_x > start_x);
  expect_true("start sprite pixels", start_spr.count >= kPicoOamSamusBlobPx);
  expect_true("end sprite pixels", end_spr.count >= kPicoOamSamusBlobPx);
  expect_true("sprite moved right", end_spr.min_x > start_spr.min_x);
  expect_true("start sprite over non-black BG", start_spr.over_bg > 0);
  expect_true("end sprite over non-black BG", end_spr.over_bg > 0);
  expect_true("first packet carries vram blob",
              n_first > (size_t)kPicoFrameWireBaseSize);

  (void)write_ppm("out/pico_move_start.ppm", frame_start);
  (void)write_ppm("out/pico_move_end.ppm", frame_end);

  printf("samus_x before=%u after=%u dy=%d\n",
         (unsigned)start_x, (unsigned)end_x, (int)end_x - (int)start_x);
  printf("samus_y=%u camera=%u,%u radius=%u,%u\n",
         (unsigned)samus_y_pos, (unsigned)layer1_x_pos, (unsigned)layer1_y_pos,
         (unsigned)samus_x_radius, (unsigned)samus_y_radius);
  printf("packet first=%zu live=%zu framebuffer=%d\n",
         n_first, n_live, kFramebufferBytes);
  printf("sprite min_x before=%d after=%d count=%d over_bg=%d\n",
         start_spr.min_x, end_spr.min_x, end_spr.count, end_spr.over_bg);

  MiniDestroy(state);
  if (g_failures) {
    fprintf(stderr, "%d failure(s)\n", g_failures);
    return 1;
  }
  puts("ok");
  return 0;
}
