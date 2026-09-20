#include "pico_display.h"

#include <stdio.h>
#include <string.h>

#include "pico_viewport.h"

enum { kFramePx = kPicoScreenWidth * kPicoScreenHeight };

static uint16_t s_rgb565[kFramePx];
static uint16_t s_panel[kPicoPanelWidth * kPicoPanelHeight];
static PicoFramePacket s_last;
static int s_have_last;

void PicoDisplay_Init(void) {
  memset(s_rgb565, 0xFF, sizeof(s_rgb565));
  memset(s_panel, 0xFF, sizeof(s_panel));
  memset(&s_last, 0, sizeof(s_last));
  s_have_last = 0;
}

void PicoDisplay_Present(const PicoFramePacket *pkt, PicoDisplayStats *stats) {
  PicoPpuState ppu;
  int y;

  if (pkt == NULL)
    return;

  s_last = *pkt;
  s_have_last = 1;
  PicoFramePacket_ToPpu(pkt, &ppu);

  for (y = 0; y < kPicoScreenHeight; y++)
    PicoScanline_Mode1(&ppu, y, s_rgb565 + (size_t)y * kPicoScreenWidth);

  for (y = 0; y < kPicoViewportLetterboxY; y++) {
    memset(s_panel + (size_t)y * kPicoPanelWidth, 0,
           (size_t)kPicoPanelWidth * sizeof(uint16_t));
    memset(s_panel + (size_t)(kPicoPanelHeight - 1 - y) * kPicoPanelWidth, 0,
           (size_t)kPicoPanelWidth * sizeof(uint16_t));
  }
  for (y = 0; y < kPicoScreenHeight; y++) {
    PicoViewport_CropLineRgb565(
        s_rgb565 + (size_t)y * kPicoScreenWidth,
        s_panel + (size_t)(y + kPicoViewportLetterboxY) * kPicoPanelWidth);
  }

  if (stats != NULL) {
    stats->raster_us = 0;
    stats->spi_us = 0;
    stats->stall_dma = 0;
    stats->stall_abort = 0;
    stats->stall_spi = 0;
  }
}

const PicoFramePacket *PicoDisplayCapture_LastPacket(void) {
  return s_have_last ? &s_last : NULL;
}

const uint16_t *PicoDisplayCapture_Rgb565(void) {
  return s_rgb565;
}

const uint16_t *PicoDisplayCapture_PanelRgb565(void) {
  return s_panel;
}

int PicoDisplayCapture_WritePpm(const char *path) {
  FILE *f;
  int i;

  if (path == NULL || !s_have_last)
    return 0;
  f = fopen(path, "wb");
  if (f == NULL)
    return 0;
  fprintf(f, "P6\n%d %d\n255\n", kPicoScreenWidth, kPicoScreenHeight);
  for (i = 0; i < kFramePx; i++) {
    uint16_t c = s_rgb565[i];
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
