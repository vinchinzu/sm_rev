#ifndef SM_MINI_DOOR_TRANSITION_H_
#define SM_MINI_DOOR_TRANSITION_H_

#include "mini_game.h"

bool MiniDoorTransition_IsActive(void);
void MiniDoorTransition_BeginFrame(void);
void MiniDoorTransition_RejectOutOfScope(void);
void MiniDoorTransition_PumpScrollIrq(void);
void MiniDoorTransition_Dispatch(void);
void MiniDoorTransition_SyncRoom(MiniGameState *state);
bool MiniDoorTransition_TryAuthored(MiniGameState *state, int y_radius_fallback);

#endif  // SM_MINI_DOOR_TRANSITION_H_
