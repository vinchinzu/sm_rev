#ifndef SM_MINI_ROOM_FX_H_
#define SM_MINI_ROOM_FX_H_

#include <stdint.h>

#include "mini_game.h"

int MiniRoomFx_EditorBg2DriftX(const MiniGameState *state);
int MiniRoomFx_EditorBg2ScrollXForScanline(const MiniGameState *state, int screen_y, int base_scroll_x);
void MiniRoomFx_RenderEditorOverlay(uint32_t *pixels, int pitch_pixels, const MiniGameState *state);

#endif  // SM_MINI_ROOM_FX_H_
