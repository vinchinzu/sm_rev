#include "pico_viewport.h"

#include <string.h>

#include "pico_ls_assets.h"

void PicoViewport_CropLineRgb565(const uint16_t *src256, uint16_t *dst240) {
  memcpy(dst240, src256 + kPicoViewportCropX,
         (size_t)kPicoPanelWidth * sizeof(uint16_t));
}

uint16_t PicoViewport_ExtractScroll(uint16_t camera, uint16_t origin) {
  if (camera >= origin)
    return (uint16_t)(camera - origin);
  return 0;
}

uint16_t PicoViewport_ExtractScrollMax(uint16_t camera, uint16_t origin,
                                       uint16_t max_scroll) {
  uint16_t scroll = PicoViewport_ExtractScroll(camera, origin);
  if (scroll > max_scroll)
    return max_scroll;
  return scroll;
}

void PicoViewport_PackExtractScrolls(PicoFramePacket *pkt, uint16_t layer1_x,
                                     uint16_t layer1_y) {
  uint16_t hofs;
  uint16_t vofs;

  if (pkt == NULL)
    return;
  hofs = PicoViewport_ExtractScrollMax(layer1_x, (uint16_t)kPicoLsExtractCameraX,
                                       (uint16_t)kPicoExtractMaxScrollX);
  vofs = PicoViewport_ExtractScrollMax(layer1_y, (uint16_t)kPicoLsExtractCameraY,
                                       (uint16_t)kPicoExtractMaxScrollY);
  pkt->bg1hofs = hofs;
  pkt->bg1vofs = vofs;
  pkt->bg2hofs = (uint16_t)(hofs >> 1);
  pkt->bg2vofs = (uint16_t)kPicoLsBg2VerticalScroll;
}
