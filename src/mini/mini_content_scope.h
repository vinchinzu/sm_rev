#ifndef SM_MINI_CONTENT_SCOPE_H_
#define SM_MINI_CONTENT_SCOPE_H_

#include "types.h"

enum {
  kMiniContentScopeRoom_LandingSite = 0x91F8,
};

bool MiniContentScope_AllowsRoom(uint16 room_id);
const char *MiniContentScope_Name(void);
const char *MiniContentScope_RoomHandle(uint16 room_id);
const char *MiniContentScope_RoomName(uint16 room_id);

#endif  // SM_MINI_CONTENT_SCOPE_H_
