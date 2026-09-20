#ifndef SM_PICO_VIEWPORT_H_
#define SM_PICO_VIEWPORT_H_

#include <stdint.h>

#include "scanline_mode1.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 256×224 SNES viewport on the Explorer 240×240 panel: crop 8px left/right,
 * letterbox 8px top/bottom, no scale.
 */
enum {
  kPicoViewportCropX = 8,
  kPicoViewportLetterboxY = 8,
  kPicoPanelWidth = 240,
  kPicoPanelHeight = 240
};

void PicoViewport_CropLineRgb565(const uint16_t *src256, uint16_t *dst240);

/* Packed LS maps start at (origin_x, origin_y). Camera below origin → 0. */
uint16_t PicoViewport_ExtractScroll(uint16_t camera, uint16_t origin);

#ifdef __cplusplus
}
#endif

#endif /* SM_PICO_VIEWPORT_H_ */
