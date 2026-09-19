/*
 * Host-to-host PicoFramePacket wire (no UART, no SPI/PIO, no SDL).
 *
 *   gcc -O2 -Werror -I. -Isrc/pico tests/test_pico_frame_wire.c \
 *       src/pico/pico_frame_wire.c src/pico/pico_frame_packet.c \
 *       src/pico/pico_ls_assets.c src/pico/scanline_mode1.c \
 *       -o sm_rev_pico_frame_wire_test
 *   ./sm_rev_pico_frame_wire_test
 *
 * Do not pass -Isrc: it shadows glibc's <features.h> with src/features.h.
 */

#include <stdio.h>
#include <string.h>

#include "pico_frame_wire.h"
#include "pico_ls_assets.h"

static int g_failures;

static void expect_true(const char *name, int cond) {
  if (!cond) {
    fprintf(stderr, "FAIL %s\n", name);
    g_failures++;
  }
}

static void expect_size(const char *name, size_t got, size_t want) {
  if (got != want) {
    fprintf(stderr, "FAIL %s: got %zu want %zu\n", name, got, want);
    g_failures++;
  }
}

static int fields_equal(const PicoFramePacket *a, const PicoFramePacket *b) {
  if (a->frame_id != b->frame_id)
    return 0;
  if (a->vsync_token != b->vsync_token)
    return 0;
  if (a->joypad_echo != b->joypad_echo)
    return 0;
  if (a->bg1hofs != b->bg1hofs || a->bg1vofs != b->bg1vofs)
    return 0;
  if (a->bg2hofs != b->bg2hofs || a->bg2vofs != b->bg2vofs)
    return 0;
  if (a->inidisp != b->inidisp || a->obsel != b->obsel || a->bgmode != b->bgmode)
    return 0;
  if (a->bg1sc != b->bg1sc || a->bg2sc != b->bg2sc || a->bg12nba != b->bg12nba)
    return 0;
  if (a->tm != b->tm)
    return 0;
  if (a->oam_full != b->oam_full || a->cgram_full != b->cgram_full ||
      a->vram_full != b->vram_full)
    return 0;
  if (memcmp(a->oam, b->oam, sizeof(a->oam)) != 0)
    return 0;
  if (memcmp(a->oam_hi, b->oam_hi, sizeof(a->oam_hi)) != 0)
    return 0;
  if (memcmp(a->cgram, b->cgram, sizeof(a->cgram)) != 0)
    return 0;
  if (a->vram_size != b->vram_size)
    return 0;
  if (a->vram_size > 0) {
    if (a->vram == NULL || b->vram == NULL)
      return 0;
    if (memcmp(a->vram, b->vram, a->vram_size) != 0)
      return 0;
  }
  return 1;
}

static void raster_frame(const PicoFramePacket *pkt, uint16_t *out) {
  PicoPpuState ppu;
  int y;

  PicoFramePacket_ToPpu(pkt, &ppu);
  for (y = 0; y < kPicoScreenHeight; y++)
    PicoScanline_Mode1(&ppu, y, out + (size_t)y * kPicoScreenWidth);
}

static int frame_has_non_black(const uint16_t *frame) {
  int i;
  int n = kPicoScreenHeight * kPicoScreenWidth;
  for (i = 0; i < n; i++) {
    if (frame[i] != 0)
      return 1;
  }
  return 0;
}

static int line_non_black_and_equal(const uint16_t *a, const uint16_t *b, int y) {
  const uint16_t *la = a + (size_t)y * kPicoScreenWidth;
  const uint16_t *lb = b + (size_t)y * kPicoScreenWidth;
  int x;
  int non_black = 0;

  if (memcmp(la, lb, (size_t)kPicoScreenWidth * sizeof(uint16_t)) != 0)
    return 0;
  for (x = 0; x < kPicoScreenWidth; x++) {
    if (la[x] != 0) {
      non_black = 1;
      break;
    }
  }
  return non_black;
}

int main(void) {
  static PicoFramePacket src;
  static PicoFramePacket dst;
  static uint8_t wire[kPicoFrameWireMaxSize];
  static uint8_t vram_dst[kPicoVramSize];
  static uint16_t frame_src[kPicoScreenHeight * kPicoScreenWidth];
  static uint16_t frame_dst[kPicoScreenHeight * kPicoScreenWidth];
  size_t n;
  uint8_t bad[kPicoFrameWireBaseSize];

  /* Dummy packet with non-zero scrolls / vsync fields. */
  PicoFramePacket_InitDummy(&src);
  src.frame_id = 42;
  src.joypad_echo = 0x0F00;
  src.bg1hofs = 8;
  src.bg2vofs = 4;

  expect_size("base size constant", (size_t)kPicoFrameWireBaseSize, 1092u);
  expect_size("encoded no-blob", PicoFrameWire_EncodedSize(&src, 0),
              (size_t)kPicoFrameWireBaseSize);
  expect_size("encoded with-blob", PicoFrameWire_EncodedSize(&src, 1),
              (size_t)kPicoFrameWireBaseSize + (size_t)kPicoVramSize);
  expect_true("no-blob smaller than framebuffer",
              PicoFrameWire_EncodedSize(&src, 0) <
                  (size_t)kPicoScreenWidth * kPicoScreenHeight * 2u);

  n = PicoFrameWire_Encode(&src, wire, sizeof(wire), 1);
  expect_true("encode dummy blob", n == PicoFrameWire_EncodedSize(&src, 1));
  expect_true("magic PFR1", wire[0] == 'P' && wire[1] == 'F' && wire[2] == 'R' &&
                                wire[3] == '1');

  memset(vram_dst, 0xA5, sizeof(vram_dst));
  expect_true("decode dummy blob",
              PicoFrameWire_Decode(wire, n, &dst, vram_dst, sizeof(vram_dst)));
  expect_true("dummy fields round-trip", fields_equal(&src, &dst));
  expect_true("decoded vram is dest", dst.vram == vram_dst);
  expect_true("vsync token", dst.vsync_token == (uint16_t)kPicoFramePacketVsync);

  raster_frame(&src, frame_src);
  raster_frame(&dst, frame_dst);
  expect_true("dummy raster identical",
              memcmp(frame_src, frame_dst, sizeof(frame_src)) == 0);
  expect_true("dummy raster non-black", frame_has_non_black(frame_src));
  expect_true("dummy line 0 identical non-black",
              line_non_black_and_equal(frame_src, frame_dst, 0));

  /* Pointer-only: display Pico already has VRAM. */
  n = PicoFrameWire_Encode(&src, wire, sizeof(wire), 0);
  expect_size("encode dummy no-blob bytes", n, (size_t)kPicoFrameWireBaseSize);
  expect_true("decode dummy no-blob",
              PicoFrameWire_Decode(wire, n, &dst, (uint8_t *)src.vram, src.vram_size));
  expect_true("no-blob fields (shared vram)", fields_equal(&src, &dst));
  raster_frame(&dst, frame_dst);
  expect_true("no-blob raster identical",
              memcmp(frame_src, frame_dst, sizeof(frame_src)) == 0);

  /* Extracted Landing Site tiles. */
  PicoFramePacket_InitLandingSiteExtracted(&src);
  n = PicoFrameWire_Encode(&src, wire, sizeof(wire), 1);
  expect_true("encode LS blob", n == PicoFrameWire_EncodedSize(&src, 1));
  expect_true("decode LS blob",
              PicoFrameWire_Decode(wire, n, &dst, vram_dst, sizeof(vram_dst)));
  expect_true("LS fields round-trip", fields_equal(&src, &dst));
  raster_frame(&src, frame_src);
  raster_frame(&dst, frame_dst);
  expect_true("LS raster identical",
              memcmp(frame_src, frame_dst, sizeof(frame_src)) == 0);
  expect_true("LS raster non-black", frame_has_non_black(frame_src));
  expect_true("LS line 100 identical non-black",
              line_non_black_and_equal(frame_src, frame_dst, 100));

  /* Error paths. */
  expect_true("encode NULL pkt", PicoFrameWire_Encode(NULL, wire, sizeof(wire), 0) == 0);
  expect_true("decode short", PicoFrameWire_Decode(wire, 8, &dst, vram_dst, sizeof(vram_dst)) == 0);
  memcpy(bad, wire, sizeof(bad));
  bad[0] ^= 0xFF;
  expect_true("decode bad magic",
              PicoFrameWire_Decode(bad, sizeof(bad), &dst, vram_dst, sizeof(vram_dst)) == 0);

  if (g_failures) {
    fprintf(stderr, "%d failure(s)\n", g_failures);
    return 1;
  }
  puts("ok");
  return 0;
}
