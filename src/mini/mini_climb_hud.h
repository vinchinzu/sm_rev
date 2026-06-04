#ifndef SM_MINI_CLIMB_HUD_H_
#define SM_MINI_CLIMB_HUD_H_

#include "types.h"

struct MiniGameState;

void MiniClimbHud_Render(uint32_t *pixels, int pitch_pixels, const struct MiniGameState *state);

#endif  // SM_MINI_CLIMB_HUD_H_
