#ifndef SM_STUBS_MINI_H_
#define SM_STUBS_MINI_H_

#include <stddef.h>

#include "types.h"

enum {
  kMiniBlockSize = 16,
  kMiniSolidBlock = 0x8000,
};

typedef enum MiniRoomSource {
  kMiniRoomSource_Fallback = 0,
  kMiniRoomSource_EditorExport = 1,
  kMiniRoomSource_RomSave = 2,
  kMiniRoomSource_RomDemo = 3,
} MiniRoomSource;

typedef enum MiniSamusSuit {
  kMiniSamusSuit_Power = 0,
  kMiniSamusSuit_Varia = 1,
  kMiniSamusSuit_Gravity = 2,
} MiniSamusSuit;

typedef struct MiniRoomInfo {
  bool has_room;
  bool uses_rom_room;
  bool booted_from_save_slot;
  bool has_editor_room_visuals;
  MiniSamusSuit samus_suit;
  uint16 room_id;
  char room_handle[32];
  char room_name[64];
  MiniRoomSource room_source;
  int room_left;
  int room_top;
  int room_right;
  int room_bottom;
  int room_width_blocks;
  int room_height_blocks;
  int camera_x;
  int camera_y;
  int spawn_x;
  int spawn_y;
} MiniRoomInfo;

typedef struct MiniRoomSprite {
  bool active;
  uint8 bank;
  uint16 spritemap;
  uint16 x_pos;
  uint16 y_pos;
  uint16 palette_index;
  uint16 vram_tiles_index;
} MiniRoomSprite;

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

void MiniStubs_Reset(void);
void MiniStubs_SetRoomExportPath(const char *path);
void MiniStubs_ConfigureWorld(int viewport_width, int viewport_height);
void MiniStubs_GetRoomInfo(MiniRoomInfo *info);
int MiniStubs_GetRoomSprites(const MiniRoomSprite **sprites);
void MiniStubs_GetEditorTilesetView(MiniEditorTilesetView *view);
void MiniStubs_GetEditorBg2View(MiniEditorBg2View *view);
int MiniStubs_GetEditorRoomSpriteViews(const MiniEditorRoomSpriteView **sprites);
uint16 MiniStubs_GetLevelBlock(int block_x, int block_y);
uint8 MiniStubs_GetBts(int block_x, int block_y);
int MiniStubs_GetFloorY(void);
const char *MiniStubs_RoomSourceName(MiniRoomSource source);
const char *MiniStubs_SamusSuitName(MiniSamusSuit suit);
void MiniStubs_ClampCameraToRoom(void);

#endif  // SM_STUBS_MINI_H_
