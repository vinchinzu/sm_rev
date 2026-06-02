#ifndef SM_MINI_EDITOR_CAMERA_H_
#define SM_MINI_EDITOR_CAMERA_H_

#include "mini_game.h"

bool MiniEditorCamera_ShouldUseState(const MiniGameState *state);
void MiniEditorCamera_Follow(MiniGameState *state);

#endif  // SM_MINI_EDITOR_CAMERA_H_
