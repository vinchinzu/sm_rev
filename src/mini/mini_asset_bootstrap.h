#ifndef SM_MINI_ASSET_BOOTSTRAP_H_
#define SM_MINI_ASSET_BOOTSTRAP_H_

#include <stddef.h>

#include "mini_editor_bridge.h"
#include "mini_room_adapter.h"

typedef struct MiniEditorTilesetView {
  bool loaded;
  int tileset_id;
  const uint8 *tiles4bpp;
  const uint16 *metatile_words;
  const uint16 *palette;
} MiniEditorTilesetView;

typedef struct MiniEditorBg2View {
  bool loaded;
  const uint16 *tilemap_words;
  uint8 scroll_x;
  uint8 scroll_y;
  const char *variant_key;
} MiniEditorBg2View;

typedef struct MiniEditorRoomSpriteOamView {
  int16 x_offset;
  int16 y_offset;
  uint16 tile_num;
  uint8 palette_row;
  bool h_flip;
  bool v_flip;
  bool is_16x16;
} MiniEditorRoomSpriteOamView;

typedef struct MiniEditorRoomSpriteView {
  const char *key;
  const char *label;
  uint16 species_id;
  int x_pos;
  int y_pos;
  const uint8 *tile_data;
  size_t tile_data_size;
  const uint16 *palette;
  const MiniEditorRoomSpriteOamView *entries;
  int entry_count;
} MiniEditorRoomSpriteView;

typedef struct MiniEditorSamusRenderedFrameView {
  uint16 pose;
  uint16 anim_frame;
  uint32 data_offset;
  int16 origin_x;
  int16 origin_y;
} MiniEditorSamusRenderedFrameView;

typedef struct MiniEditorSamusRenderedSpritesView {
  bool loaded;
  const uint8 *rgba;
  size_t rgba_size;
  const MiniEditorSamusRenderedFrameView *frames;
  int frame_count;
  int frame_width;
  int frame_height;
} MiniEditorSamusRenderedSpritesView;

void MiniAssetBootstrap_Reset(void);
void MiniAssetBootstrap_InstallEditorAssets(const MiniEditorRoom *room);
void MiniAssetBootstrap_SetSamusSuitState(MiniSamusSuit suit);
MiniSamusSuit MiniAssetBootstrap_GetInitialSuit(void);
bool MiniAssetBootstrap_HasEditorTilesetAssets(void);
bool MiniAssetBootstrap_LoadSamusBaseTilesFromAssets(void);
void MiniAssetBootstrap_InstallRomSamusBaseTiles(void);
void MiniAssetBootstrap_LoadCurrentRoomAssets(void);
void MiniAssetBootstrap_PrimeEditorRoomFxAndMissingRomVisuals(const MiniEditorRoom *room,
                                                             bool load_tileset_visuals,
                                                             bool load_bg2_visuals);
void MiniAssetBootstrap_GetEditorTilesetView(MiniEditorTilesetView *view);
void MiniAssetBootstrap_GetEditorBg2View(MiniEditorBg2View *view);
int MiniAssetBootstrap_GetEditorRoomSpriteViews(const MiniEditorRoomSpriteView **sprites);
void MiniAssetBootstrap_GetEditorSamusRenderedSpritesView(MiniEditorSamusRenderedSpritesView *view);

#endif  // SM_MINI_ASSET_BOOTSTRAP_H_
