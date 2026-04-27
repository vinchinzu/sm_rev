#include "mini_content_scope.h"

bool MiniContentScope_AllowsRoom(uint16 room_id) {
  return room_id == kMiniContentScopeRoom_LandingSite;
}

const char *MiniContentScope_Name(void) {
  return "landing_site_only";
}

const char *MiniContentScope_RoomHandle(uint16 room_id) {
  return MiniContentScope_AllowsRoom(room_id) ? "landingSite" : "blockedRoom";
}

const char *MiniContentScope_RoomName(uint16 room_id) {
  return MiniContentScope_AllowsRoom(room_id) ? "Landing Site" : "Blocked Room";
}
