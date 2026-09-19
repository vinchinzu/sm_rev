/*
 * Packed Landing Site tiles (no game kernel, no SDL, no sm.smc).
 *
 *   gcc -O2 -Werror -I. -Isrc/pico tests/test_pico_ls_assets.c \
 *       src/pico/pico_ls_assets.c src/pico/pico_frame_packet.c \
 *       src/pico/scanline_mode1.c \
 *       -o sm_rev_pico_ls_assets_test
 *   ./sm_rev_pico_ls_assets_test
 *
 * Do not pass -Isrc: it shadows glibc's <features.h> with src/features.h.
 */

#include <stdio.h>
#include <string.h>

#include "pico_ls_assets.h"

enum {
  kRomBytes = 3145728,
  kPackedBudget = 300000
};

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

static int count_nonzero_u8(const uint8_t *p, size_t n) {
  int c = 0;
  size_t i;
  for (i = 0; i < n; i++) {
    if (p[i] != 0)
      c++;
  }
  return c;
}

static int count_nonzero_u16(const uint16_t *p, size_t n) {
  int c = 0;
  size_t i;
  for (i = 0; i < n; i++) {
    if (p[i] != 0)
      c++;
  }
  return c;
}

int main(void) {
  static PicoFramePacket pkt;
  static PicoPpuState ppu;
  uint16_t line[kPicoScreenWidth];
  size_t packed = PicoLsAssets_PackedSize();
  size_t unpacked = PicoLsAssets_UnpackedSourceSize();
  int x;
  int non_black = 0;

  PicoFramePacket_InitLandingSiteExtracted(&pkt);
  PicoFramePacket_ToPpu(&pkt, &ppu);

  expect_u16("vsync token", pkt.vsync_token, (uint16_t)kPicoFramePacketVsync);
  expect_true("vram pointer", pkt.vram != NULL && pkt.vram_size == (size_t)kPicoVramSize);
  expect_true("full uploads", pkt.oam_full && pkt.cgram_full && pkt.vram_full);
  expect_u16("BGMODE Mode 1", pkt.bgmode, 9);
  expect_true("ToPpu vram", ppu.vram == pkt.vram);
  expect_true("ToPpu cgram", ppu.cgram == pkt.cgram);

  expect_true("vram has tiles", count_nonzero_u8(pkt.vram, 32768) > 0);
  expect_true("cgram has colors", count_nonzero_u16(pkt.cgram, kPicoCgramColors) > 0);

  PicoScanline_Mode1(&ppu, 100, line);
  for (x = 0; x < kPicoScreenWidth; x++) {
    if (line[x] != 0) {
      non_black = 1;
      break;
    }
  }
  expect_true("line 100 not all black", non_black);
  expect_u16("line 100 x0 BG2", line[0], 0x0903);
  expect_u16("line 100 x100 BG2", line[100], 0x1943);

  expect_true("packed well under 3MB", packed > 0 && packed < (size_t)kRomBytes);
  expect_true("packed under hundreds-of-KB budget", packed <= (size_t)kPackedBudget);
  expect_true("packed smaller than unpacked sources", packed < unpacked);

  printf("packed=%zu unpacked_source=%zu rom=%d rom_used=no\n", packed, unpacked, kRomBytes);

  if (g_failures) {
    fprintf(stderr, "%d failure(s)\n", g_failures);
    return 1;
  }
  puts("ok");
  return 0;
}
