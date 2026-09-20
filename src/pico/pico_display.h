#ifndef SM_PICO_DISPLAY_H_
#define SM_PICO_DISPLAY_H_

#include "pico_frame_packet.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Display-driver seam: the game loop packs a PicoFramePacket (PPU state —
 * scrolls, OAM, CGRAM, a VRAM pointer — not a 256×224 framebuffer) and this
 * module Presents it. Two adapters ship today; a vtable with one impl would
 * be a hypothetical seam.
 *
 *   pico_display_st7789.c  — Explorer panel (letterbox 8px, Mode1Range
 *                            columns 8..248, crop, SPI). Same pins as
 *                            St7789Explorer_* (GP18/19/17/16, BL PWM GP20,
 *                            window (0,0,240,240)).
 *   pico_display_capture.c — host test / 2nd-Pico dry run: raster the same
 *                            packet into a buffer or PPM and keep the last
 *                            packet.
 *
 * Next adapter, not a rewrite: pico_display_wire.c = PicoFrameWire_Encode
 * the same packet (~1.1 KB live) over UART/SPI; the display Pico rasters.
 * Do not invent a third packet. Do not stream pixels. HSTX is a later panel
 * adapter behind this same Present(), not a new contract.
 */
typedef struct PicoDisplayStats {
  uint32_t raster_us;    /* Mode 1 scanline time this frame */
  uint32_t spi_us;       /* panel transfer time this frame; 0 on capture */
  uint32_t stall_dma;    /* ST7789 bounded-wait totals since Init */
  uint32_t stall_abort;
  uint32_t stall_spi;
} PicoDisplayStats;

void PicoDisplay_Init(void);
void PicoDisplay_Present(const PicoFramePacket *pkt, PicoDisplayStats *stats);

/*
 * Host capture adapter extras. Linked only with pico_display_capture.c.
 * The ST7789 adapter does not define these.
 */
const PicoFramePacket *PicoDisplayCapture_LastPacket(void);
const uint16_t *PicoDisplayCapture_Rgb565(void);      /* 256×224 */
const uint16_t *PicoDisplayCapture_PanelRgb565(void); /* 240×240 letterbox+crop */
int PicoDisplayCapture_WritePpm(const char *path);

#ifdef __cplusplus
}
#endif

#endif /* SM_PICO_DISPLAY_H_ */
