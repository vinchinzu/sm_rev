#ifndef SM_PICO_VIEWPORT_H_
#define SM_PICO_VIEWPORT_H_

#include <stdint.h>

#include "pico_frame_packet.h"
#include "scanline_mode1.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 256×224 SNES viewport on the Explorer 240×240 panel: crop 8px left/right,
 * letterbox 8px top/bottom, no scale.
 *
 * Packed LS maps are a 64×32-tile (512×256 px) spawn window. The camera may
 * travel (map - viewport) inside that extract; past it, hofs clamps and Samus
 * walks to the screen edge. Follow-up: VRAM window / dirty upload of the rest
 * of the 144×80 room. Do not pack the whole room here.
 */
enum {
  kPicoViewportCropX = 8,
  kPicoViewportLetterboxY = 8,
  kPicoPanelWidth = 240,
  kPicoPanelHeight = 240,
  kPicoExtractMapWidth = 512,
  kPicoExtractMapHeight = 256,
  kPicoExtractMaxScrollX = kPicoExtractMapWidth - kPicoScreenWidth,
  kPicoExtractMaxScrollY = kPicoExtractMapHeight - kPicoScreenHeight
};

void PicoViewport_CropLineRgb565(const uint16_t *src256, uint16_t *dst240);

/* Packed LS maps start at (origin_x, origin_y). Camera below origin → 0. */
uint16_t PicoViewport_ExtractScroll(uint16_t camera, uint16_t origin);

/* ExtractScroll, then clamp to [0, max_scroll] (extract travel, not room). */
uint16_t PicoViewport_ExtractScrollMax(uint16_t camera, uint16_t origin,
                                       uint16_t max_scroll);

/*
 * Fill packet BG1/BG2 scrolls from the game camera (layer1_*).
 * BG2: bgScrolling 0x0181 → hofs = BG1/2, vofs locked at packed value.
 */
void PicoViewport_PackExtractScrolls(PicoFramePacket *pkt, uint16_t layer1_x,
                                     uint16_t layer1_y);

#ifdef __cplusplus
}
#endif

#endif /* SM_PICO_VIEWPORT_H_ */
