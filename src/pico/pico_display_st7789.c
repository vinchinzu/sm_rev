#include "pico_display.h"

#include "hardware/structs/watchdog.h"
#include "pico/stdlib.h"

#include "pico_viewport.h"
#include "st7789_explorer.h"

/*
 * Watchdog display-phase crumbs. Values must match pico2_main.c kCrumbBegin
 * through kCrumbEnd so a hung Present still names the scanline over CDC.
 */
enum {
  kCrumbMagic = 0x5D4B0000u,
  kCrumbBegin = 0x05,
  kCrumbRaster = 0x06,
  kCrumbLine = 0x07,
  kCrumbEnd = 0x08
};

static void crumb(uint32_t phase) {
  watchdog_hw->scratch[0] = kCrumbMagic | (phase & 0xFFu);
}

void PicoDisplay_Init(void) {
  St7789Explorer_Init();
  /* Black, not magenta: a boot hang must not look like the old ship-1 fill. */
  St7789Explorer_Fill(0);
}

void PicoDisplay_Present(const PicoFramePacket *pkt, PicoDisplayStats *stats) {
  PicoPpuState ppu;
  /* static: ~1KB of line buffers would not fit the default 2KiB stack. */
  static uint16_t line256[kPicoScreenWidth];
  static uint16_t line240[kPicoPanelWidth];
  uint64_t t;
  uint32_t raster_us;
  uint32_t spi_us;
  int y;

  if (pkt == NULL)
    return;

  PicoFramePacket_ToPpu(pkt, &ppu);
  raster_us = 0;
  spi_us = 0;

  t = time_us_64();
  crumb(kCrumbBegin);
  St7789Explorer_BeginFrame();
  for (y = 0; y < kPicoViewportLetterboxY; y++)
    St7789Explorer_WriteSolidLine(0, kPicoPanelWidth);
  spi_us += (uint32_t)(time_us_64() - t);

  for (y = 0; y < kPicoScreenHeight; y++) {
    /* Scanline in the high half so a watchdog reboot names the exact line. */
    watchdog_hw->scratch[3] =
        (watchdog_hw->scratch[3] & 0xFFFFu) | ((uint32_t)y << 16);
    crumb(kCrumbRaster);
    t = time_us_64();
    /* sm_rev-k5q.7 item 6: the panel only ever shows columns 8..247, and
     * PicoViewport_CropLineRgb565 throws the other 16 away. Raster the 240
     * that survive. The columns outside the range keep whatever line256 held;
     * nothing reads them. Host renders still call PicoScanline_Mode1 and stay
     * 256 wide and byte-identical. */
    PicoScanline_Mode1Range(&ppu, y, line256, kPicoViewportCropX,
                            kPicoViewportCropX + kPicoPanelWidth);
    PicoViewport_CropLineRgb565(line256, line240);
    raster_us += (uint32_t)(time_us_64() - t);

    crumb(kCrumbLine);
    t = time_us_64();
    St7789Explorer_WriteRgb565Line(line240, kPicoPanelWidth);
    spi_us += (uint32_t)(time_us_64() - t);
  }

  t = time_us_64();
  crumb(kCrumbEnd);
  for (y = 0; y < kPicoViewportLetterboxY; y++)
    St7789Explorer_WriteSolidLine(0, kPicoPanelWidth);
  St7789Explorer_EndFrame();
  spi_us += (uint32_t)(time_us_64() - t);

  if (stats != NULL) {
    stats->raster_us = raster_us;
    stats->spi_us = spi_us;
    St7789Explorer_GetStalls(&stats->stall_dma, &stats->stall_abort,
                             &stats->stall_spi);
  }
}
