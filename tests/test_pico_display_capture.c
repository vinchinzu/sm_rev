/*
 * Host capture adapter at the PicoDisplay seam (no game kernel, no SDL,
 * no panel). Packs a dummy and an extracted Landing Site packet, Presents
 * them, and memcmps the 256×224 raster against PicoScanline_Mode1 of the
 * same packet — the pico-ls-layers reference algorithm. The 240×240 panel
 * buffer is the ST7789 letterbox+crop of that raster.
 *
 *   make pico-display-test
 *
 * Do not pass -Isrc: it shadows glibc's <features.h> with src/features.h.
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "pico_display.h"
#include "pico_ls_assets.h"
#include "pico_viewport.h"

enum { kFramePx = kPicoScreenWidth * kPicoScreenHeight };

static int g_failures;

static void expect_true(const char *name, int cond) {
  if (!cond) {
    fprintf(stderr, "FAIL %s\n", name);
    g_failures++;
  }
}

static void expect_u16(const char *name, uint16_t got, uint16_t want) {
  if (got != want) {
    fprintf(stderr, "FAIL %s: got 0x%04X want 0x%04X\n", name, got, want);
    g_failures++;
  }
}

static void raster_mode1(const PicoFramePacket *pkt, uint16_t *out) {
  PicoPpuState ppu;
  int y;

  PicoFramePacket_ToPpu(pkt, &ppu);
  for (y = 0; y < kPicoScreenHeight; y++)
    PicoScanline_Mode1(&ppu, y, out + (size_t)y * kPicoScreenWidth);
}

static int count_nonzero(const uint16_t *px, int n) {
  int i;
  int c = 0;
  for (i = 0; i < n; i++) {
    if (px[i] != 0)
      c++;
  }
  return c;
}

static int row_is_zero(const uint16_t *row, int n) {
  int x;
  for (x = 0; x < n; x++) {
    if (row[x] != 0)
      return 0;
  }
  return 1;
}

static void check_panel_letterbox(const uint16_t *rgb, const uint16_t *panel,
                                  const char *tag) {
  int y;
  char name[96];
  const uint16_t *dst;
  const uint16_t *src;

  for (y = 0; y < kPicoViewportLetterboxY; y++) {
    snprintf(name, sizeof(name), "%s top letterbox y=%d", tag, y);
    expect_true(name, row_is_zero(panel + (size_t)y * kPicoPanelWidth,
                                  kPicoPanelWidth));
    snprintf(name, sizeof(name), "%s bottom letterbox y=%d", tag,
             kPicoPanelHeight - 1 - y);
    expect_true(name, row_is_zero(panel + (size_t)(kPicoPanelHeight - 1 - y) *
                                              kPicoPanelWidth,
                                  kPicoPanelWidth));
  }
  for (y = 0; y < kPicoScreenHeight; y++) {
    src = rgb + (size_t)y * kPicoScreenWidth + kPicoViewportCropX;
    dst = panel + (size_t)(y + kPicoViewportLetterboxY) * kPicoPanelWidth;
    snprintf(name, sizeof(name), "%s crop y=%d", tag, y);
    expect_true(name, memcmp(dst, src, (size_t)kPicoPanelWidth * sizeof(uint16_t)) ==
                          0);
  }
}

static void present_and_check(const PicoFramePacket *pkt, const char *tag) {
  static uint16_t ref[kFramePx];
  PicoDisplayStats stats;
  const PicoFramePacket *last;
  const uint16_t *rgb;
  const uint16_t *panel;
  uint32_t frame_id = pkt->frame_id;
  uint16_t vsync = pkt->vsync_token;
  const uint8_t *vram = pkt->vram;

  memset(&stats, 0xA5, sizeof(stats));
  PicoDisplay_Present(pkt, &stats);

  expect_true("Present leaves packet vsync", pkt->vsync_token == vsync);
  expect_true("Present leaves packet frame_id", pkt->frame_id == frame_id);
  expect_true("Present leaves packet vram", pkt->vram == vram);

  last = PicoDisplayCapture_LastPacket();
  expect_true("last packet recorded", last != NULL);
  if (last != NULL) {
    expect_true("last vsync", last->vsync_token == vsync);
    expect_true("last frame_id", last->frame_id == frame_id);
    expect_true("last vram pointer", last->vram == vram);
    expect_u16("last bg1hofs", last->bg1hofs, pkt->bg1hofs);
    expect_u16("last bg1vofs", last->bg1vofs, pkt->bg1vofs);
  }

  rgb = PicoDisplayCapture_Rgb565();
  panel = PicoDisplayCapture_PanelRgb565();
  expect_true("rgb buffer", rgb != NULL);
  expect_true("panel buffer", panel != NULL);
  if (rgb == NULL || panel == NULL)
    return;

  raster_mode1(pkt, ref);
  {
    char name[64];
    snprintf(name, sizeof(name), "%s memcmp Mode1", tag);
    expect_true(name, memcmp(rgb, ref, sizeof(ref)) == 0);
  }
  expect_true("stats spi_us is 0 on capture", stats.spi_us == 0);
  expect_true("stats stalls are 0 on capture",
              stats.stall_dma == 0 && stats.stall_abort == 0 &&
                  stats.stall_spi == 0);

  check_panel_letterbox(rgb, panel, tag);
}

int main(void) {
  static PicoFramePacket dummy;
  static PicoFramePacket ls;
  const uint16_t *rgb;
  uint16_t green;
  uint16_t yellow;
  uint16_t red;

  PicoDisplay_Init();
  expect_true("no last packet before Present",
              PicoDisplayCapture_LastPacket() == NULL);

  PicoFramePacket_InitDummy(&dummy);
  present_and_check(&dummy, "dummy");

  rgb = PicoDisplayCapture_Rgb565();
  expect_true("dummy rgb", rgb != NULL);
  if (rgb != NULL) {
    green = PicoBgr555ToRgb565(0x03E0);
    yellow = PicoBgr555ToRgb565(0x03FF);
    red = PicoBgr555ToRgb565(0x001F);
    expect_u16("dummy y0 x0 BG1 quadrant", rgb[0], green);
    expect_u16("dummy y0 x4 BG1 quadrant", rgb[4], yellow);
    expect_u16("dummy y0 x8 BG2 through hole", rgb[8], red);
    expect_true("dummy has non-black pixels", count_nonzero(rgb, kFramePx) > 0);
  }

  PicoFramePacket_InitLandingSiteExtracted(&ls);
  expect_true("ls vram", ls.vram != NULL && ls.vram_size == (size_t)kPicoVramSize);
  present_and_check(&ls, "ls");
  rgb = PicoDisplayCapture_Rgb565();
  expect_true("ls has non-black pixels",
              rgb != NULL && count_nonzero(rgb, kFramePx) > 0);

  mkdir("out", 0755);
  expect_true("write capture ppm",
              PicoDisplayCapture_WritePpm("out/pico_display_capture.ppm"));

  if (g_failures) {
    fprintf(stderr, "%d failure(s)\n", g_failures);
    return 1;
  }
  puts("ok");
  return 0;
}
