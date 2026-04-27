#ifndef SM_MINI_ROM_BOOTSTRAP_H_
#define SM_MINI_ROM_BOOTSTRAP_H_

#include "stubs_mini.h"
#include "types.h"

void MiniRomBootstrap_Reset(void);
bool MiniRomBootstrap_LoadAnyRom(void);
void MiniRomBootstrap_TryLoadRoomHeaderMetadata(uint16 room_id);
bool MiniRomBootstrap_TryConfigureSaveSlotRoom(MiniRoomInfo *info);
bool MiniRomBootstrap_TryConfigureDemoRoom(MiniRoomInfo *info);

#endif  // SM_MINI_ROM_BOOTSTRAP_H_
