/*
 * Dummy game→display frame packet (no game kernel, no SDL).
 *
 *   gcc -O2 -Werror -I. -Isrc/pico tests/test_pico_frame_packet.c \
 *       src/pico/pico_frame_packet.c src/pico/scanline_mode1.c \
 *       -o sm_rev_pico_frame_packet_test
 *   ./sm_rev_pico_frame_packet_test
 *
 * Do not pass -Isrc: it shadows glibc's <features.h> with src/features.h.
 */

#include <stdio.h>
#include <string.h>

#include "pico_frame_packet.h"

static int g_failures;

static void expect_u16(const char *name, uint16_t got, uint16_t want) {
  if (got != want) {
    fprintf(stderr, "FAIL %s: got 0x%04X want 0x%04X\n", name, got, want);
    g_failures++;
  }
}

static void expect_true(const char *name, int cond) {
  if (!cond) {
    fprintf(stderr, "FAIL %s\n", name);
    g_failures++;
  }
}

static void raster_frame(const PicoPpuState *ppu, uint16_t *out) {
  for (int y = 0; y < kPicoScreenHeight; y++)
    PicoScanline_Mode1(ppu, y, out + (size_t)y * kPicoScreenWidth);
}

int main(void) {
  static PicoFramePacket pkt;
  static PicoPpuState ppu;
  static uint16_t frame_a[kPicoScreenHeight * kPicoScreenWidth];
  static uint16_t frame_b[kPicoScreenHeight * kPicoScreenWidth];
  uint16_t line[kPicoScreenWidth];

  PicoFramePacket_InitDummy(&pkt);
  expect_u16("vsync token", pkt.vsync_token, (uint16_t)kPicoFramePacketVsync);
  expect_true("frame_id", pkt.frame_id == 1);
  expect_true("vram pointer", pkt.vram != NULL && pkt.vram_size == (size_t)kPicoVramSize);
  expect_true("full uploads", pkt.oam_full && pkt.cgram_full && pkt.vram_full);
  expect_u16("BGMODE Mode 1", pkt.bgmode, 9);
  expect_u16("INIDISP full", pkt.inidisp, 0x0F);
  expect_u16("TM BG1+BG2+OBJ", pkt.tm, 0x13);

  PicoFramePacket_ToPpu(&pkt, &ppu);
  expect_true("ToPpu vram", ppu.vram == pkt.vram);
  expect_true("ToPpu cgram", ppu.cgram == pkt.cgram);
  expect_true("ToPpu oam", ppu.oam == pkt.oam && ppu.oam_hi == pkt.oam_hi);
  expect_u16("ToPpu BG1SC", ppu.bg1sc, pkt.bg1sc);
  expect_u16("ToPpu OBSEL", ppu.obsel, pkt.obsel);

  expect_true("line width 256", kPicoScreenWidth == 256);

  PicoScanline_Mode1(&ppu, 0, line);
  {
    int x;
    int non_black = 0;
    for (x = 0; x < kPicoScreenWidth; x++) {
      if (line[x] != 0) {
        non_black = 1;
        break;
      }
    }
    expect_true("y0 has a non-black pixel", non_black);
  }

  uint16_t green = PicoBgr555ToRgb565(0x03E0);
  uint16_t yellow = PicoBgr555ToRgb565(0x03FF);
  uint16_t red = PicoBgr555ToRgb565(0x001F);
  uint16_t white = PicoBgr555ToRgb565(0x7FFF);
  uint16_t blue = PicoBgr555ToRgb565(0x7C00);

  expect_u16("y0 x0 BG1 quadrant", line[0], green);
  expect_u16("y0 x4 BG1 quadrant", line[4], yellow);
  expect_u16("y0 x8 BG2 through hole", line[8], red);

  PicoScanline_Mode1(&ppu, 8, line);
  expect_u16("y8 x20 sprite marker", line[20], white);
  expect_u16("y8 x21 sprite body", line[21], blue);

  raster_frame(&ppu, frame_a);
  raster_frame(&ppu, frame_b);
  expect_true("second raster identical",
              memcmp(frame_a, frame_b, sizeof(frame_a)) == 0);

  {
    int i;
    int non_black = 0;
    for (i = 0; i < kPicoScreenHeight * kPicoScreenWidth; i++) {
      if (frame_a[i] != 0) {
        non_black = 1;
        break;
      }
    }
    expect_true("full frame has a non-black pixel", non_black);
  }

  if (g_failures) {
    fprintf(stderr, "%d failure(s)\n", g_failures);
    return 1;
  }
  puts("ok");
  return 0;
}
