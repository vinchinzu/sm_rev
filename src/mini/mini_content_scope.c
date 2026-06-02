#include "mini_content_scope.h"

static bool g_climb_endless_scope;

void MiniContentScope_SetClimbEndlessMode(bool enabled) {
  g_climb_endless_scope = enabled;
}

bool MiniContentScope_AllowsRoom(uint16 room_id) {
  if (g_climb_endless_scope)
    return room_id == kMiniContentScopeRoom_Climb;
  return room_id == kMiniContentScopeRoom_LandingSite;
}

const char *MiniContentScope_Name(void) {
  return g_climb_endless_scope ? "climb_endless" : "landing_site_only";
}

const char *MiniContentScope_RoomHandle(uint16 room_id) {
  if (g_climb_endless_scope)
    return room_id == kMiniContentScopeRoom_Climb ? "climb" : "blockedRoom";
  return room_id == kMiniContentScopeRoom_LandingSite ? "landingSite" : "blockedRoom";
}

const char *MiniContentScope_RoomName(uint16 room_id) {
  if (g_climb_endless_scope)
    return room_id == kMiniContentScopeRoom_Climb ? "The Climb" : "Blocked Room";
  return room_id == kMiniContentScopeRoom_LandingSite ? "Landing Site" : "Blocked Room";
}
