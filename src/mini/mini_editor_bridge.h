#ifndef SM_MINI_EDITOR_BRIDGE_H_
#define SM_MINI_EDITOR_BRIDGE_H_

#include <stddef.h>

#include "types.h"

enum {
  kMiniEditorBridgeRoomId_LandingSite = 0x91F8,
  kMiniEditorBridgeSamusBank92Size = 0x8000,
  kMiniEditorBridgeTiles4bppSize = 1024 * 32,
  kMiniEditorBridgeMetatileWordCount = 1024 * 4,
  kMiniEditorBridgeBg2TilemapWordCount = 64 * 32,
  kMiniEditorBridgePaletteCount = 8 * 16,
  kMiniEditorBridgeSamusPaletteCount = 16,
  kMiniEditorBridgeRoomSpritePaletteCount = 16,
};

typedef enum MiniEditorSamusSuit {
  kMiniEditorSamusSuit_Power = 0,
  kMiniEditorSamusSuit_Varia = 1,
  kMiniEditorSamusSuit_Gravity = 2,
} MiniEditorSamusSuit;

typedef struct MiniEditorSamusAssetRange {
  uint32 snes_address;
  uint32 size;
  uint32 data_offset;
} MiniEditorSamusAssetRange;

typedef struct MiniEditorRoomSpriteOamEntry {
  int16 x_offset;
  int16 y_offset;
  uint16 tile_num;
  uint8 palette_row;
  bool h_flip;
  bool v_flip;
  bool is_16x16;
} MiniEditorRoomSpriteOamEntry;

typedef struct MiniEditorRoomSprite {
  char key[32];
  char label[64];
  uint16 species_id;
  int x_pos;
  int y_pos;
  uint8 *tile_data;
  size_t tile_data_size;
  uint16 palette[kMiniEditorBridgeRoomSpritePaletteCount];
  MiniEditorRoomSpriteOamEntry *entries;
  int entry_count;
} MiniEditorRoomSprite;

typedef struct MiniEditorRoom {
  uint16 room_id;
  char handle[32];
  char name[64];
  int width_screens;
  int height_screens;
  int width_blocks;
  int height_blocks;
  int spawn_x;
  int spawn_y;
  int camera_x;
  int camera_y;
  int tileset;
  int cre_bitflag;
  int export_up_scroller;
  int export_down_scroller;
  int export_bg_scrolling;
  bool has_tileset_assets;
  bool has_bg2_assets;
  bool has_samus_assets;
  bool has_samus_palette_assets;
  MiniEditorSamusSuit initial_suit;
  uint16 *block_words;
  uint8 *collision_types;
  uint8 *bts;
  uint8 *scroll_values;
  uint8 *tiles4bpp;
  uint16 *metatile_words;
  uint16 *palette;
  uint16 *bg2_tilemap_words;
  char bg2_variant_key[32];
  uint16 samus_palette_power[kMiniEditorBridgeSamusPaletteCount];
  uint16 samus_palette_varia[kMiniEditorBridgeSamusPaletteCount];
  uint16 samus_palette_gravity[kMiniEditorBridgeSamusPaletteCount];
  uint8 *samus_bank92;
  uint8 *samus_data;
  MiniEditorSamusAssetRange *samus_ranges;
  int samus_range_count;
  size_t samus_data_size;
  MiniEditorRoomSprite *room_sprites;
  int room_sprite_count;
} MiniEditorRoom;

void MiniEditorBridge_SetRoomExportPath(const char *path);
const char *MiniEditorBridge_GetResolvedPath(void);
bool MiniEditorBridge_LoadRoom(MiniEditorRoom *room);
void MiniEditorBridge_FreeRoom(MiniEditorRoom *room);

#endif  // SM_MINI_EDITOR_BRIDGE_H_
