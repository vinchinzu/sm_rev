#ifndef SM_MINI_CLIMB_ENDLESS_H_
#define SM_MINI_CLIMB_ENDLESS_H_

#include "mini_game.h"
#include "mini_room_adapter.h"
#include "types.h"

enum {
  kMiniClimbEndlessRoomId = 0x96BA,
};

void MiniClimbEndless_SetActive(bool active);
bool MiniClimbEndless_IsActive(void);
const char *MiniClimbEndless_DefaultRoomExportPath(void);

void MiniClimbEndless_ApplySpawnDefaults(MiniRoomInfo *room);
void MiniClimbEndless_InitAfterRoom(MiniGameState *state, MiniRoomInfo *room);
void MiniClimbEndless_ApplySamusLoadout(void);
void MiniClimbEndless_Tick(MiniGameState *state);

int MiniClimbEndless_VirtualFloors(const MiniGameState *state);
bool MiniClimbEndless_LavaEnabled(const MiniGameState *state);
int MiniClimbEndless_LavaFloorY(const MiniGameState *state);
int MiniClimbEndless_AscentPixels(const MiniGameState *state);
int MiniClimbEndless_BestAscentPixels(const MiniGameState *state);
int MiniClimbEndless_Deaths(const MiniGameState *state);
int MiniClimbEndless_RunFrames(const MiniGameState *state);
int MiniClimbEndless_LavaSpeedQ8(const MiniGameState *state);
int MiniClimbEndless_DifficultyTier(const MiniGameState *state);
int MiniClimbEndless_NextWrapTargetRow(const MiniGameState *state);
bool MiniClimbEndless_SamusInLava(const MiniGameState *state);
int MiniClimbEndless_PirateShotCooldownFrames(const MiniGameState *state);

#endif  // SM_MINI_CLIMB_ENDLESS_H_
