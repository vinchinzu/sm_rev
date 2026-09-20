#ifndef SM_MINI_AUTHORED_MOVEMENT_H_
#define SM_MINI_AUTHORED_MOVEMENT_H_

#include "mini_game.h"
#include "mini_room_adapter.h"

bool MiniAuthoredMovement_ShouldUseRoom(const MiniRoomInfo *room);
bool MiniAuthoredMovement_ShouldUseState(const MiniGameState *state);
void MiniAuthoredMovement_InitializeSamusGlobals(void);
/* Advances samus_anim_frame / samus_anim_frame_timer with the vanilla clock
 * (Samus_Animate + the bank 0x91 per-pose delay bytes). No-op until the packed
 * bank 0x91 window is installed. Called from MiniAuthoredMovement_Step. */
void MiniAuthoredMovement_StepAnimation(void);
void MiniAuthoredMovement_SyncGrounded(MiniGameState *state);
void MiniAuthoredMovement_Step(MiniGameState *state);

#endif  // SM_MINI_AUTHORED_MOVEMENT_H_
