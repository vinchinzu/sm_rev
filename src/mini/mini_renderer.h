#ifndef SM_MINI_RENDERER_H_
#define SM_MINI_RENDERER_H_

#include <stdint.h>

#include "mini_backdrop.h"
#include "mini_game.h"

void MiniRenderer_SetBackdropMode(MiniBackdropMode mode);
void MiniRenderFrameToPixels(uint32_t *pixels, int pitch_pixels, const MiniGameState *state);
bool MiniSaveScreenshot(const char *path, const MiniGameState *state);

#endif  // SM_MINI_RENDERER_H_
