#ifndef SM_MINI_AUTHORED_MOVEMENT_H_
#define SM_MINI_AUTHORED_MOVEMENT_H_

#include "mini_game.h"
#include "mini_room_adapter.h"

bool MiniAuthoredMovement_ShouldUseRoom(const MiniRoomInfo *room);
bool MiniAuthoredMovement_ShouldUseState(const MiniGameState *state);
void MiniAuthoredMovement_InitializeSamusGlobals(void);
void MiniAuthoredMovement_SyncGrounded(MiniGameState *state);
void MiniAuthoredMovement_Step(MiniGameState *state);

#endif  // SM_MINI_AUTHORED_MOVEMENT_H_
