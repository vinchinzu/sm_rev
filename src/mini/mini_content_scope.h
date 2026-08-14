#ifndef SM_MINI_CONTENT_SCOPE_H_
#define SM_MINI_CONTENT_SCOPE_H_

#include "types.h"

enum {
  kMiniContentScopeArea_Ceres = 6,
  kMiniContentScopeRoom_LandingSite = 0x91F8,
  kMiniContentScopeRoom_CeresElevator = 0xDF45,
  kMiniContentScopeRoom_CeresFallingTile = 0xDF8D,
  kMiniContentScopeRoom_CeresMagnetStairs = 0xDFD7,
  kMiniContentScopeRoom_CeresDeadScientist = 0xE021,
  kMiniContentScopeRoom_CeresEscape = 0xE06B,
  kMiniContentScopeRoom_CeresRidley = 0xE0B5,
};

bool MiniContentScope_AllowsRoom(uint16 room_id);
bool MiniContentScope_AllowsDoor(uint16 door_def_ptr);
uint16 MiniContentScope_DoorDestinationRoom(uint16 door_def_ptr);
const char *MiniContentScope_Name(void);
const char *MiniContentScope_RoomHandle(uint16 room_id);
const char *MiniContentScope_RoomName(uint16 room_id);

#endif  // SM_MINI_CONTENT_SCOPE_H_
