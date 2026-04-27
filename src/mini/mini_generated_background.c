#include "mini_generated_background.h"

#include <stddef.h>

#include "mini_defs.h"

#include "mini_generated_background_data.inc"

void MiniGeneratedBackground_Render(uint32_t *pixels, int pitch_pixels) {
  if (kMiniAiLandingSiteRgb_len != kMiniGameWidth * kMiniGameHeight * 3)
    return;

  const unsigned char *src = kMiniAiLandingSiteRgb;
  for (int y = 0; y < kMiniGameHeight; y++) {
    uint32_t *row = pixels + y * pitch_pixels;
    for (int x = 0; x < kMiniGameWidth; x++) {
      uint32_t r = src[0];
      uint32_t g = src[1];
      uint32_t b = src[2];
      row[x] = 0xFF000000u | (r << 16) | (g << 8) | b;
      src += 3;
    }
  }
}
