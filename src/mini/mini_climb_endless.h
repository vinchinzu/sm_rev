#ifndef SM_MINI_CLIMB_ENDLESS_H_
#define SM_MINI_CLIMB_ENDLESS_H_

#include "mini_editor_bridge.h"
#include "mini_game.h"
#include "mini_room_adapter.h"
#include "types.h"

enum {
  kMiniClimbEndlessRoomId = 0x96BA,
};

typedef struct MiniClimbModeSnapshot {
  int virtual_floors;
  bool lava_enabled;
  int lava_floor_y;
  int ascent_pixels;
  int last_samus_y;
  bool has_score_anchor;
} MiniClimbModeSnapshot;

void MiniClimbEndless_SetActive(bool active);
bool MiniClimbEndless_IsActive(void);
const char *MiniClimbEndless_DefaultRoomExportPath(void);

void MiniClimbEndless_AssignRoomDefaults(MiniEditorRoom *room);
void MiniClimbEndless_ApplySpawnDefaults(MiniRoomInfo *room);
void MiniClimbEndless_InitAfterRoom(MiniRoomInfo *room);
void MiniClimbEndless_ApplySamusLoadout(void);
void MiniClimbEndless_Tick(MiniGameState *state);

void MiniClimbEndless_SaveSnapshot(MiniClimbModeSnapshot *snapshot);
void MiniClimbEndless_LoadSnapshot(const MiniClimbModeSnapshot *snapshot);

int MiniClimbEndless_VirtualFloors(void);
bool MiniClimbEndless_LavaEnabled(void);
int MiniClimbEndless_LavaFloorY(void);
int MiniClimbEndless_AscentPixels(void);

#endif  // SM_MINI_CLIMB_ENDLESS_H_
