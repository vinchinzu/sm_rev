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

typedef struct MiniEditorEnemySpawnView {
  char name[kMiniEditorBridgeEnemyNameCapacity];
  uint16 species_id;
  uint16 init_parameter;
  uint16 properties1;
  uint16 properties2;
  uint16 extra_parameter1;
  uint16 extra_parameter2;
  int x_pos;
  int y_pos;
  int block_x;
  int block_y;
  bool has_population_words;
} MiniEditorEnemySpawnView;

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

/*
 * sm_rev-k5q.8. The editor BG2 buffer is a raw image of BG2 VRAM
 * 0x4800..0x4FFF. reg_BG2SC is 0x49 in gameplay (mini_ppu_stub.c): a 64x32
 * map, which on the SNES is TWO 32x32 screens laid out sequentially (0x4800 =
 * columns 0-31, 0x4C00 = columns 32-63), NOT a 64-wide linear array. Reading
 * it linearly interleaves the two halves.
 *
 * Nothing fills both for the Landing Site: the bgdata record list at $8F:B76A
 * is six door-dependent entries, each a single 0x800-byte (32x32) DMA -- five
 * to 0x4800, door $89B2 to 0x4C00. The screen nobody wrote still holds
 * game_init's memset of 0x2C0F (tile 0x0F, palette 3, priority 1), the uniform
 * grey "wallpaper" that covered the bottom of the frame and painted over BG1.
 */
enum {
  kMiniEditorBg2ScreenWords = 32 * 32
};

/* Word offset of the one live 32x32 screen (a screen nobody DMA'd is a single
 * repeated word), or -1 when both carry content and the buffer really is a
 * 64x32 map. */
int MiniAssetBootstrap_Bg2LiveScreen(const uint16 *words);

/* One BG2 tilemap word, wrapped. With a single live screen the 256x256 px
 * backdrop repeats every 32 tiles in both axes, which is what vanilla shows;
 * with a full 64x32 map, address it as the SNES does. */
uint16 MiniAssetBootstrap_Bg2Word(const uint16 *words, int live_screen, int tile_x,
                                  int tile_y);
int MiniAssetBootstrap_GetEditorRoomSpriteViews(const MiniEditorRoomSpriteView **sprites);
int MiniAssetBootstrap_GetEditorEnemySpawnViews(const MiniEditorEnemySpawnView **enemies);
void MiniAssetBootstrap_GetEditorSamusRenderedSpritesView(MiniEditorSamusRenderedSpritesView *view);

#endif  // SM_MINI_ASSET_BOOTSTRAP_H_
