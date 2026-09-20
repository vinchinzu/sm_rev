#include "pico_viewport.h"

#include <string.h>

void PicoViewport_CropLineRgb565(const uint16_t *src256, uint16_t *dst240) {
  memcpy(dst240, src256 + kPicoViewportCropX,
         (size_t)kPicoPanelWidth * sizeof(uint16_t));
}

uint16_t PicoViewport_ExtractScroll(uint16_t camera, uint16_t origin) {
  if (camera >= origin)
    return (uint16_t)(camera - origin);
  return 0;
}
