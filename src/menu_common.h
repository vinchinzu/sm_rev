#ifndef SM_MENU_COMMON_H_
#define SM_MENU_COMMON_H_

#include "types.h"

void ClearMenuTilemap(void);
void LoadFileSelectPalettes(void);
void LoadInitialMenuTiles(void);
void LoadMenuExitTilemap(void);
void LoadMenuPalettes(void);
void LoadMenuTilemap(uint16 k, uint16 j);
void MapVramForMenu(void);
void QueueTransferOfMenuTilemapToVramBG1(void);

#endif  // SM_MENU_COMMON_H_
