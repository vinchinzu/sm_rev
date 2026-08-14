#include "mini_content_scope.h"

#include <stddef.h>

#include "ida_types.h"
#include "sm_rtl.h"

typedef struct MiniScopedRoom {
  uint16 room_id;
  const char *handle;
  const char *name;
} MiniScopedRoom;

static const MiniScopedRoom kMiniScopedRooms[] = {
  { kMiniContentScopeRoom_CeresElevator, "ceresElevator", "Ceres Elevator" },
  { kMiniContentScopeRoom_CeresFallingTile, "ceresFallingTile", "Ceres Falling Tile" },
  { kMiniContentScopeRoom_CeresMagnetStairs, "ceresMagnetStairs", "Ceres Magnet Stairs" },
  { kMiniContentScopeRoom_CeresDeadScientist, "ceresDeadScientist", "Ceres Dead Scientist" },
  { kMiniContentScopeRoom_CeresEscape, "ceresEscape", "Ceres Escape" },
  { kMiniContentScopeRoom_CeresRidley, "ceresRidley", "Ceres Ridley" },
  { kMiniContentScopeRoom_LandingSite, "landingSite", "Landing Site" },
};

static const MiniScopedRoom *MiniContentScope_FindRoom(uint16 room_id) {
  for (size_t i = 0; i < sizeof(kMiniScopedRooms) / sizeof(kMiniScopedRooms[0]); i++) {
    if (kMiniScopedRooms[i].room_id == room_id)
      return &kMiniScopedRooms[i];
  }
  return NULL;
}

bool MiniContentScope_AllowsRoom(uint16 room_id) {
  if (MiniContentScope_FindRoom(room_id) != NULL)
    return true;
  if (g_rom == NULL)
    return false;
  RoomDefHeader *header = get_RoomDefHeader(room_id);
  return header != NULL && header->area_index_ == kMiniContentScopeArea_Ceres;
}

uint16 MiniContentScope_DoorDestinationRoom(uint16 door_def_ptr) {
  if (door_def_ptr == 0 || g_rom == NULL)
    return 0;
  return get_DoorDef(door_def_ptr)->room_definition_ptr;
}

bool MiniContentScope_AllowsDoor(uint16 door_def_ptr) {
  uint16 destination = MiniContentScope_DoorDestinationRoom(door_def_ptr);
  if (destination == 0)
    return false;
  if ((destination & 0x8000) == 0)
    return true;
  return MiniContentScope_AllowsRoom(destination);
}

const char *MiniContentScope_Name(void) {
  return "ceres";
}

const char *MiniContentScope_RoomHandle(uint16 room_id) {
  const MiniScopedRoom *room = MiniContentScope_FindRoom(room_id);
  return room != NULL ? room->handle : "blockedRoom";
}

const char *MiniContentScope_RoomName(uint16 room_id) {
  const MiniScopedRoom *room = MiniContentScope_FindRoom(room_id);
  return room != NULL ? room->name : "Blocked Room";
}
