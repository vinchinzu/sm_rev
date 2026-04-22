#ifndef SM_MINI_ASSET_BOOTSTRAP_H_
#define SM_MINI_ASSET_BOOTSTRAP_H_

#include "mini_editor_bridge.h"
#include "stubs_mini.h"

void MiniAssetBootstrap_Reset(void);
void MiniAssetBootstrap_InstallEditorAssets(const MiniEditorRoom *room);
void MiniAssetBootstrap_SetSamusSuitState(MiniSamusSuit suit);
MiniSamusSuit MiniAssetBootstrap_GetInitialSuit(void);
bool MiniAssetBootstrap_HasEditorTilesetAssets(void);
bool MiniAssetBootstrap_LoadSamusBaseTilesFromAssets(void);
void MiniAssetBootstrap_InstallRomSamusBaseTiles(void);
void MiniAssetBootstrap_LoadCurrentRoomAssets(void);
int MiniAssetBootstrap_GetRoomSprites(const MiniRoomSprite **sprites);
void MiniAssetBootstrap_GetEditorTilesetView(MiniEditorTilesetView *view);
void MiniAssetBootstrap_GetEditorBg2View(MiniEditorBg2View *view);
int MiniAssetBootstrap_GetEditorRoomSpriteViews(const MiniEditorRoomSpriteView **sprites);

#endif  // SM_MINI_ASSET_BOOTSTRAP_H_
