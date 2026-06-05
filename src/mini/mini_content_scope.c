#include "mini_content_scope.h"

#include "mini_run_mode.h"
#include "types.h"

bool MiniContentScope_AllowsRoom(uint16 room_id) {
  if (MiniRunMode_IsClimbRoom())
    return room_id == kMiniContentScopeRoom_Climb;
  return room_id == kMiniContentScopeRoom_LandingSite;
}

const char *MiniContentScope_Name(void) {
  if (MiniRunMode_IsClimbEndless())
    return "climb_endless";
  if (MiniRunMode_IsClimbRoom())
    return "climb_original";
  return "landing_site_only";
}

const char *MiniContentScope_RoomHandle(uint16 room_id) {
  if (MiniRunMode_IsClimbRoom())
    return room_id == kMiniContentScopeRoom_Climb ? "climb" : "blockedRoom";
  return room_id == kMiniContentScopeRoom_LandingSite ? "landingSite" : "blockedRoom";
}

const char *MiniContentScope_RoomName(uint16 room_id) {
  if (MiniRunMode_IsClimbRoom())
    return room_id == kMiniContentScopeRoom_Climb ? "The Climb" : "Blocked Room";
  return room_id == kMiniContentScopeRoom_LandingSite ? "Landing Site" : "Blocked Room";
}
