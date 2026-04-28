#include "mini_asset_bootstrap.h"

#include <stdlib.h>
#include <string.h>

#include "funcs.h"
#include "ida_types.h"
#include "mini_ppu_stub.h"
#include "samus_asset_bridge.h"
#include "sm_rtl.h"
#include "variables.h"

enum {
  kMiniRoomTilesVramSize = 0x5000,
  kMiniCreTilesVramByteDst = 0x5000,
  kMiniStdObjTilesVramDst = 0x6000,
  kMiniStdObjTilesVramSize = 0x2E00,
};

static bool g_mini_has_editor_tileset_assets;
static bool g_mini_has_editor_bg2_assets;
static int g_mini_editor_tileset_id;
static char g_mini_editor_bg2_variant_key[32];
static uint8 g_mini_editor_bg2_scroll_x;
static uint8 g_mini_editor_bg2_scroll_y;
static bool g_mini_has_editor_samus_palette_assets;
static MiniSamusSuit g_mini_editor_samus_initial_suit;
static uint8 g_mini_editor_tiles4bpp[kMiniEditorBridgeTiles4bppSize];
static uint16 g_mini_editor_metatile_words[kMiniEditorBridgeMetatileWordCount];
static uint16 g_mini_editor_palette[kMiniEditorBridgePaletteCount];
static uint16 g_mini_editor_bg2_tilemap_words[kMiniEditorBridgeBg2TilemapWordCount];
static uint16 g_mini_editor_samus_palette_power[kMiniEditorBridgeSamusPaletteCount];
static uint16 g_mini_editor_samus_palette_varia[kMiniEditorBridgeSamusPaletteCount];
static uint16 g_mini_editor_samus_palette_gravity[kMiniEditorBridgeSamusPaletteCount];
static MiniEditorRoomSpriteView *g_mini_editor_room_sprite_views;
static int g_mini_editor_room_sprite_count;

static void MiniClearEditorRoomSpriteAssets(void) {
  if (g_mini_editor_room_sprite_views != NULL) {
    for (int i = 0; i < g_mini_editor_room_sprite_count; i++) {
      free((void *)g_mini_editor_room_sprite_views[i].tile_data);
      free((void *)g_mini_editor_room_sprite_views[i].palette);
      free((void *)g_mini_editor_room_sprite_views[i].entries);
      free((void *)g_mini_editor_room_sprite_views[i].key);
      free((void *)g_mini_editor_room_sprite_views[i].label);
    }
  }
  free(g_mini_editor_room_sprite_views);
  g_mini_editor_room_sprite_views = NULL;
  g_mini_editor_room_sprite_count = 0;
}

static void MiniClearEditorSamusPaletteAssets(void) {
  g_mini_has_editor_samus_palette_assets = false;
  g_mini_editor_samus_initial_suit = kMiniSamusSuit_Power;
  memset(g_mini_editor_samus_palette_power, 0, sizeof(g_mini_editor_samus_palette_power));
  memset(g_mini_editor_samus_palette_varia, 0, sizeof(g_mini_editor_samus_palette_varia));
  memset(g_mini_editor_samus_palette_gravity, 0, sizeof(g_mini_editor_samus_palette_gravity));
}

static void MiniClearEditorTilesetAssets(void) {
  g_mini_has_editor_tileset_assets = false;
  g_mini_has_editor_bg2_assets = false;
  g_mini_editor_tileset_id = -1;
  g_mini_editor_bg2_variant_key[0] = '\0';
  g_mini_editor_bg2_scroll_x = 0;
  g_mini_editor_bg2_scroll_y = 0;
  memset(g_mini_editor_tiles4bpp, 0, sizeof(g_mini_editor_tiles4bpp));
  memset(g_mini_editor_metatile_words, 0, sizeof(g_mini_editor_metatile_words));
  memset(g_mini_editor_palette, 0, sizeof(g_mini_editor_palette));
  memset(g_mini_editor_bg2_tilemap_words, 0, sizeof(g_mini_editor_bg2_tilemap_words));
  MiniClearEditorRoomSpriteAssets();
}

static MiniSamusSuit MiniConvertEditorSuit(MiniEditorSamusSuit suit) {
  switch (suit) {
  case kMiniEditorSamusSuit_Varia:
    return kMiniSamusSuit_Varia;
  case kMiniEditorSamusSuit_Gravity:
    return kMiniSamusSuit_Gravity;
  default:
    return kMiniSamusSuit_Power;
  }
}

static void MiniInstallEditorSamusAssets(const MiniEditorRoom *room) {
  SamusAssetBridge_Reset();
  MiniClearEditorSamusPaletteAssets();
  g_mini_editor_samus_initial_suit = MiniConvertEditorSuit(room->initial_suit);
  if (room->has_samus_palette_assets) {
    memcpy(g_mini_editor_samus_palette_power, room->samus_palette_power, sizeof(g_mini_editor_samus_palette_power));
    memcpy(g_mini_editor_samus_palette_varia, room->samus_palette_varia, sizeof(g_mini_editor_samus_palette_varia));
    memcpy(g_mini_editor_samus_palette_gravity, room->samus_palette_gravity, sizeof(g_mini_editor_samus_palette_gravity));
    g_mini_has_editor_samus_palette_assets = true;
  }
  if (!room->has_samus_assets || room->samus_bank92 == NULL || room->samus_data == NULL ||
      room->samus_ranges == NULL || room->samus_range_count <= 0)
    return;
  SamusAssetBridge_Install(room->samus_bank92, kMiniEditorBridgeSamusBank92Size,
                           room->samus_data, room->samus_data_size,
                           (const SamusAssetBridgeRange *)room->samus_ranges,
                           room->samus_range_count);
}

static void MiniCopySuitPalette(void) {
  const uint16 *palette_words = NULL;
  uint32 palette_addr = 0;
  if ((equipped_items & 0x20) != 0) {
    palette_words = g_mini_has_editor_samus_palette_assets ? g_mini_editor_samus_palette_gravity : NULL;
    palette_addr = 0x9B9C40;
  } else if ((equipped_items & 1) != 0) {
    palette_words = g_mini_has_editor_samus_palette_assets ? g_mini_editor_samus_palette_varia : NULL;
    palette_addr = 0x9B9820;
  } else {
    palette_words = g_mini_has_editor_samus_palette_assets ? g_mini_editor_samus_palette_power : NULL;
    palette_addr = 0x9B9400;
  }
  if (palette_words != NULL) {
    memcpy(&target_palettes[192], palette_words, sizeof(uint16) * kMiniEditorBridgeSamusPaletteCount);
    memcpy(&palette_buffer[192], palette_words, sizeof(uint16) * kMiniEditorBridgeSamusPaletteCount);
    return;
  }
  const uint8 *palette = SamusAssetBridge_GetData(palette_addr, 32);
  if (palette != NULL) {
    memcpy(&target_palettes[192], palette, 32);
    memcpy(&palette_buffer[192], palette, 32);
  }
}

static void MiniInstallEditorTilesetAssets(const MiniEditorRoom *room) {
  MiniClearEditorTilesetAssets();
  if (!room->has_tileset_assets || room->tiles4bpp == NULL ||
      room->metatile_words == NULL || room->palette == NULL)
    return;

  memcpy(g_mini_editor_tiles4bpp, room->tiles4bpp, sizeof(g_mini_editor_tiles4bpp));
  memcpy(g_mini_editor_metatile_words, room->metatile_words, sizeof(g_mini_editor_metatile_words));
  memcpy(g_mini_editor_palette, room->palette, sizeof(g_mini_editor_palette));
  memcpy(target_palettes, g_mini_editor_palette, sizeof(g_mini_editor_palette));
  memcpy(palette_buffer, g_mini_editor_palette, sizeof(g_mini_editor_palette));
  MiniPpu_CopyVram(0x0000, g_mini_editor_tiles4bpp, kMiniRoomTilesVramSize);
  g_mini_editor_tileset_id = room->tileset;
  g_mini_has_editor_tileset_assets = true;
  if (room->has_bg2_assets && room->bg2_tilemap_words != NULL) {
    memcpy(g_mini_editor_bg2_tilemap_words, room->bg2_tilemap_words, sizeof(g_mini_editor_bg2_tilemap_words));
    snprintf(g_mini_editor_bg2_variant_key, sizeof(g_mini_editor_bg2_variant_key), "%s", room->bg2_variant_key);
    g_mini_editor_bg2_scroll_x = (uint8)(room->export_bg_scrolling & 0xFF);
    g_mini_editor_bg2_scroll_y = (uint8)(room->export_bg_scrolling >> 8);
    g_mini_has_editor_bg2_assets = true;
  }
}

static char *MiniDupString(const char *src) {
  size_t len = strlen(src) + 1;
  char *dst = (char *)malloc(len);
  if (dst != NULL)
    memcpy(dst, src, len);
  return dst;
}

static void MiniInstallEditorRoomSprites(const MiniEditorRoom *room) {
  MiniClearEditorRoomSpriteAssets();
  if (room->room_sprites == NULL || room->room_sprite_count <= 0)
    return;
  g_mini_editor_room_sprite_views = (MiniEditorRoomSpriteView *)calloc((size_t)room->room_sprite_count,
                                                                       sizeof(*g_mini_editor_room_sprite_views));
  if (g_mini_editor_room_sprite_views == NULL)
    return;
  g_mini_editor_room_sprite_count = room->room_sprite_count;
  for (int i = 0; i < room->room_sprite_count; i++) {
    const MiniEditorRoomSprite *src = &room->room_sprites[i];
    MiniEditorRoomSpriteView *dst = &g_mini_editor_room_sprite_views[i];
    dst->key = MiniDupString(src->key);
    dst->label = MiniDupString(src->label);
    dst->species_id = src->species_id;
    dst->x_pos = src->x_pos;
    dst->y_pos = src->y_pos;
    dst->entry_count = src->entry_count;
    if (src->tile_data_size != 0 && src->tile_data != NULL) {
      uint8 *tile_data = (uint8 *)malloc(src->tile_data_size);
      if (tile_data != NULL) {
        memcpy(tile_data, src->tile_data, src->tile_data_size);
        dst->tile_data = tile_data;
        dst->tile_data_size = src->tile_data_size;
      }
    }
    uint16 *palette = (uint16 *)malloc(sizeof(uint16) * kMiniEditorBridgeRoomSpritePaletteCount);
    if (palette != NULL) {
      memcpy(palette, src->palette, sizeof(uint16) * kMiniEditorBridgeRoomSpritePaletteCount);
      dst->palette = palette;
    }
    if (src->entry_count > 0 && src->entries != NULL) {
      MiniEditorRoomSpriteOamView *entries = (MiniEditorRoomSpriteOamView *)malloc(
          sizeof(*entries) * (size_t)src->entry_count);
      if (entries != NULL) {
        for (int j = 0; j < src->entry_count; j++) {
          entries[j] = (MiniEditorRoomSpriteOamView){
            .x_offset = src->entries[j].x_offset,
            .y_offset = src->entries[j].y_offset,
            .tile_num = src->entries[j].tile_num,
            .palette_row = src->entries[j].palette_row,
            .h_flip = src->entries[j].h_flip,
            .v_flip = src->entries[j].v_flip,
            .is_16x16 = src->entries[j].is_16x16,
          };
        }
        dst->entries = entries;
      }
    }
  }
}

static void MiniCopyMetatileWordsFromTileTable(uint16 *dst) {
  for (int i = 0; i < 1024; i++) {
    dst[i * 4 + 0] = tile_table.tables[i].top_left;
    dst[i * 4 + 1] = tile_table.tables[i].top_right;
    dst[i * 4 + 2] = tile_table.tables[i].bottom_left;
    dst[i * 4 + 3] = tile_table.tables[i].bottom_right;
  }
}

static void MiniLoadRoomFxStateFromRom(void) {
  InitializeSpecialEffectsForNewRoom();
  LoadRoomHeader();
  LoadStateHeader();
  LoadFXHeader();
}

void MiniAssetBootstrap_Reset(void) {
  MiniClearEditorTilesetAssets();
  MiniClearEditorSamusPaletteAssets();
  SamusAssetBridge_Reset();
}

void MiniAssetBootstrap_InstallEditorAssets(const MiniEditorRoom *room) {
  MiniInstallEditorTilesetAssets(room);
  MiniInstallEditorSamusAssets(room);
  MiniInstallEditorRoomSprites(room);
}

void MiniAssetBootstrap_SetSamusSuitState(MiniSamusSuit suit) {
  equipped_items &= ~(1 | 0x20);
  collected_items &= ~(1 | 0x20);
  switch (suit) {
  case kMiniSamusSuit_Gravity:
    equipped_items |= 0x20;
    collected_items |= 0x20;
    samus_suit_palette_index = 4;
    break;
  case kMiniSamusSuit_Varia:
    equipped_items |= 1;
    collected_items |= 1;
    samus_suit_palette_index = 2;
    break;
  default:
    samus_suit_palette_index = 0;
    break;
  }
}

MiniSamusSuit MiniAssetBootstrap_GetInitialSuit(void) {
  return g_mini_editor_samus_initial_suit;
}

bool MiniAssetBootstrap_HasEditorTilesetAssets(void) {
  return g_mini_has_editor_tileset_assets;
}

bool MiniAssetBootstrap_LoadSamusBaseTilesFromAssets(void) {
  const uint8 *obj_tiles = SamusAssetBridge_GetData(0x9AD200, kMiniStdObjTilesVramSize);
  if (obj_tiles == NULL)
    return false;
  MiniPpu_CopyVram(kMiniStdObjTilesVramDst, obj_tiles, kMiniStdObjTilesVramSize);
  MiniCopySuitPalette();
  return true;
}

void MiniAssetBootstrap_InstallRomSamusBaseTiles(void) {
  MiniPpu_CopyVram(kMiniStdObjTilesVramDst, RomFixedPtr(0x9AD200), kMiniStdObjTilesVramSize);
  MiniCopySuitPalette();
}

static void MiniTransferOriginalEnemyTilesToVramAndInit(void) {
  for (int i = 0; i < 10 && enemy_tile_vram_src != 0xFFFF; i++) {
    TransferEnemyTilesToVramAndInit();
    NMI_ProcessVramWriteQueue();
  }
}

void MiniAssetBootstrap_LoadCurrentRoomAssets(void) {
  MiniPpu_InitGameplay();
  LoadInitialPalette();
  MiniLoadRoomFxStateFromRom();
  ClearPLMs();
  ClearEprojs();
  ClearPaletteFXObjects();
  LoadLevelDataAndOtherThings();
  DecompressToMem(Load24(&tileset_tiles_pointer), (uint8 *)tilemap_stuff);
  DecompressToMem(Load24(&tileset_compr_palette_ptr), (uint8 *)target_palettes);
  memcpy(palette_buffer, target_palettes, 512);
  MiniPpu_CopyVram(0x0000, tilemap_stuff, 0x2000);
  MiniPpu_CopyVram(0x1000, (uint8 *)tilemap_stuff + 0x2000, 0x2000);
  MiniPpu_CopyVram(0x2000, (uint8 *)tilemap_stuff + 0x4000, 0x1000);
  DecompressToMem(0xb98000, MiniPpu_GetVram() + kMiniCreTilesVramByteDst);
  MiniAssetBootstrap_InstallRomSamusBaseTiles();
  LoadColorsForSpritesBeamsAndEnemies();
  LoadEnemies();
  MiniTransferOriginalEnemyTilesToVramAndInit();
  LoadRoomPlmGfx();
  EnableEprojs();
  EnablePLMs();
}

void MiniAssetBootstrap_PrimeEditorRoomFxAndMissingRomVisuals(const MiniEditorRoom *room,
                                                             bool load_tileset_visuals,
                                                             bool load_bg2_visuals) {
  if (room == NULL)
    return;

  MiniPpu_InitGameplay();
  LoadInitialPalette();
  MiniLoadRoomFxStateFromRom();

  if (load_tileset_visuals) {
    DecompressToMem(Load24(&tileset_tiles_pointer), (uint8 *)tilemap_stuff);
    DecompressToMem(Load24(&tileset_compr_palette_ptr), (uint8 *)target_palettes);
    memcpy(palette_buffer, target_palettes, 512);
    memcpy(g_mini_editor_tiles4bpp, tilemap_stuff, sizeof(g_mini_editor_tiles4bpp));
    MiniCopyMetatileWordsFromTileTable(g_mini_editor_metatile_words);
    memcpy(g_mini_editor_palette, target_palettes, sizeof(g_mini_editor_palette));
    g_mini_editor_tileset_id = room->tileset;
    g_mini_has_editor_tileset_assets = true;
  }

  LoadLibraryBackground();

  if (load_bg2_visuals) {
    memcpy(g_mini_editor_bg2_tilemap_words, ram4000.bg2_tilemap, sizeof(g_mini_editor_bg2_tilemap_words));
    snprintf(g_mini_editor_bg2_variant_key, sizeof(g_mini_editor_bg2_variant_key), "%s", "rom_fallback");
    g_mini_editor_bg2_scroll_x = (uint8)(room->export_bg_scrolling & 0xFF);
    g_mini_editor_bg2_scroll_y = (uint8)(room->export_bg_scrolling >> 8);
    g_mini_has_editor_bg2_assets = true;
  }
}

void MiniAssetBootstrap_GetEditorTilesetView(MiniEditorTilesetView *view) {
  *view = (MiniEditorTilesetView){
    .loaded = g_mini_has_editor_tileset_assets,
    .tileset_id = g_mini_editor_tileset_id,
    .tiles4bpp = g_mini_editor_tiles4bpp,
    .metatile_words = g_mini_editor_metatile_words,
    .palette = g_mini_editor_palette,
  };
}

void MiniAssetBootstrap_GetEditorBg2View(MiniEditorBg2View *view) {
  *view = (MiniEditorBg2View){
    .loaded = g_mini_has_editor_bg2_assets,
    .tilemap_words = g_mini_editor_bg2_tilemap_words,
    .scroll_x = g_mini_editor_bg2_scroll_x,
    .scroll_y = g_mini_editor_bg2_scroll_y,
    .variant_key = g_mini_editor_bg2_variant_key,
  };
}

int MiniAssetBootstrap_GetEditorRoomSpriteViews(const MiniEditorRoomSpriteView **sprites) {
  *sprites = g_mini_editor_room_sprite_views;
  return g_mini_editor_room_sprite_count;
}
