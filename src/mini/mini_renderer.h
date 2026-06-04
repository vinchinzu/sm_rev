#ifndef SM_MINI_RENDERER_H_
#define SM_MINI_RENDERER_H_

#include <stdint.h>

#include "mini_backdrop.h"
#include "mini_game.h"

void MiniRenderer_SetBackdropMode(MiniBackdropMode mode);
uint32_t MiniRenderer_ConvertBgr555(uint16 color);
void MiniRenderer_FillRect(uint32_t *pixels, int pitch_pixels, int left, int top,
                           int width, int height, uint32_t color);
void MiniRenderFrameToPixels(uint32_t *pixels, int pitch_pixels, const MiniGameState *state);
void MiniRenderFrameToPixelsWithCamera(uint32_t *pixels, int pitch_pixels,
                                       const MiniGameState *state,
                                       int camera_x, int camera_y);
bool MiniSaveScreenshot(const char *path, const MiniGameState *state);

#endif  // SM_MINI_RENDERER_H_
