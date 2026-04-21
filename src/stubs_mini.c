#include "stubs_mini.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "funcs.h"
#include "ida_types.h"
#include "enemy_types.h"
#include "mini_editor_bridge.h"
#include "mini_runtime.h"
#include "samus_asset_bridge.h"
#include "physics_config.h"
#include "sm_rtl.h"
#include "variables.h"

enum {
  kMiniWorldPadding = 32,
  kMiniGroundOffset = 48,
  kMiniGroundSpeed = 3,
  kMiniAirSpeed = 2,
  kMiniDefaultUpScroller = 112,
  kMiniDefaultDownScroller = 160,
  kMiniLevelDataCapacity = (0x16402 - 0x10002) / 2,
  kMiniRomCapacity = 0x400000,
  kMiniLandingSiteRoom = 0x91F8,
  kMiniLandingSiteCameraX = 1024,
  kMiniLandingSiteCameraY = 976,
  kMiniLandingSiteSamusX = 1153,
  kMiniLandingSiteSamusY = 1088,
  kMiniBg2Tiles = 64 * 32,
  kMiniRoomSpriteCount = 3,
  kMiniVramSize = 0x10000,
  kMiniRoomTilesVramSize = 0x5000,
  kMiniCreTilesVramDst = 0x5000,
  kMiniStdObjTilesVramDst = 0x6000,
  kMiniStdObjTilesVramSize = 0x2E00,
  kMiniEnemyObjTilesVramDst = 0x6C00,
  kMiniEnemyTileRamSize = 0x2800,
};

uint8 g_ram[0x20000];

static uint8 g_mini_sram[0x2000];
static uint8 g_mini_rom[0x400000];

uint8 *g_sram = g_mini_sram;
const uint8 *g_rom = g_mini_rom;
bool g_use_my_apu_code;
bool g_debug_flag;
int snes_frame_counter;
SpcPlayer *g_spc_player;
uint16 currently_installed_bug_fix_counter;

static int g_mini_world_left;
static int g_mini_world_right;
static int g_mini_world_ceiling;
static int g_mini_world_floor;
static MiniRoomInfo g_mini_room_info;
static bool g_mini_has_editor_tileset_assets;
static bool g_mini_has_editor_bg2_assets;
static bool g_mini_editor_bg2_uses_rom_vram;
static int g_mini_editor_tileset_id;
static char g_mini_editor_bg2_variant_key[32];
static uint8 g_mini_editor_bg2_scroll_x;
static uint8 g_mini_editor_bg2_scroll_y;
static bool g_mini_has_editor_samus_palette_assets;
static MiniSamusSuit g_mini_editor_samus_initial_suit;
static uint16 g_mini_bg2_tilemap[kMiniBg2Tiles];
static uint8 g_mini_vram[kMiniVramSize];
static uint8 g_mini_editor_tiles4bpp[kMiniEditorBridgeTiles4bppSize];
static uint16 g_mini_editor_metatile_words[kMiniEditorBridgeMetatileWordCount];
static uint16 g_mini_editor_palette[kMiniEditorBridgePaletteCount];
static uint16 g_mini_editor_bg2_tilemap_words[kMiniEditorBridgeBg2TilemapWordCount];
static uint16 g_mini_editor_samus_palette_power[kMiniEditorBridgeSamusPaletteCount];
static uint16 g_mini_editor_samus_palette_varia[kMiniEditorBridgeSamusPaletteCount];
static uint16 g_mini_editor_samus_palette_gravity[kMiniEditorBridgeSamusPaletteCount];
static MiniRoomSprite g_mini_room_sprites[kMiniRoomSpriteCount];
static MiniEditorRoomSpriteView *g_mini_editor_room_sprite_views;
static int g_mini_editor_room_sprite_count;
static struct {
  uint16 vram_addr;
  uint8 vmain;
  uint16 a1t1;
  uint8 a1b1;
  uint16 das1;
  uint8 dmap1;
  uint8 bbad1;
  uint8 cgadd;
  uint8 cgram_latch;
  bool cgram_low_pending;
} g_mini_ppu;

static const char *const kMiniRomCandidates[] = {
  "sm.smc",
  "../sm/sm.smc",
  "../roms/rom.sfc",
};

static const char *const kMiniSaveCandidates[] = {
  "saves/sm.srm",
  "../sm/saves/sm.srm",
};

static void MiniClampCamera(void);
static bool MiniLoadAnyRom(void);

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
  g_mini_editor_bg2_uses_rom_vram = false;
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

static void MiniSetSamusSuitState(MiniSamusSuit suit) {
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

static int AlignUpToBlock(int value) {
  return (value + kMiniBlockSize - 1) & ~(kMiniBlockSize - 1);
}

static void MiniSetRoomLabel(MiniRoomInfo *info, const char *handle, const char *name) {
  snprintf(info->room_handle, sizeof(info->room_handle), "%s", handle != NULL ? handle : "");
  snprintf(info->room_name, sizeof(info->room_name), "%s", name != NULL ? name : "");
}

static void MiniWriteBlock(int block_x, int block_y, uint16 level, uint8 bts) {
  int index = block_y * room_width_in_blocks + block_x;
  level_data[index] = level;
  BTS[index] = bts;
}

static void MiniInitializeScrollState(int width_blocks, int height_blocks) {
  room_width_in_scrolls = width_blocks / 16;
  room_height_in_scrolls = height_blocks / 16;
  room_width_in_blocks = width_blocks;
  room_height_in_blocks = height_blocks;
  room_size_in_blocks = 2 * width_blocks * height_blocks;
  up_scroller = kMiniDefaultUpScroller;
  down_scroller = kMiniDefaultDownScroller;
  camera_distance_index = 0;
  ideal_layer1_xpos = layer1_x_pos;
  ideal_layer1_ypos = layer1_y_pos;
  memset(scrolls, 0, 256);
  for (int i = 0; i < room_width_in_scrolls * room_height_in_scrolls; i++)
    scrolls[i] = 2;
}

static void MiniApplyEditorScrollState(const MiniEditorRoom *room) {
  if (room->scroll_values == NULL)
    return;
  up_scroller = room->export_up_scroller;
  down_scroller = room->export_down_scroller;
  memset(scrolls, 0, 256);
  int scroll_count = room->width_screens * room->height_screens;
  if (scroll_count > 256)
    scroll_count = 256;
  memcpy(scrolls, room->scroll_values, scroll_count);
}

static void MiniTryLoadRoomHeaderMetadata(uint16 room_id) {
  if (!MiniLoadAnyRom())
    return;
  RoomDefHeader *room_header = get_RoomDefHeader(room_id);
  room_index = room_header->semiunique_room_number;
  area_index = room_header->area_index_;
  room_x_coordinate_on_map = room_header->x_coordinate_on_map;
  room_y_coordinate_on_map = room_header->y_coordinate_on_map;
  room_width_in_scrolls = room_header->width;
  room_height_in_scrolls = room_header->height;
  room_width_in_blocks = 16 * room_width_in_scrolls;
  room_height_in_blocks = 16 * room_height_in_scrolls;
  room_size_in_blocks = 2 * room_width_in_blocks * room_height_in_blocks;
  up_scroller = room_header->up_scroller_;
  down_scroller = room_header->down_scroller_;
  door_list_pointer = room_header->ptr_to_doorout;
}

static uint16 MiniParseDoorVariantPtr(const char *variant_key) {
  if (variant_key == NULL || strncmp(variant_key, "door_", 5) != 0)
    return 0;
  char *end = NULL;
  unsigned long value = strtoul(variant_key + 5, &end, 16);
  if (end == NULL || *end != '\0' || value > 0xFFFF)
    return 0;
  return (uint16)value;
}

static bool MiniMaybeLoadEditorRoomRomBackground(const MiniEditorRoom *room) {
  if ((room->room_id & 0x8000) == 0)
    return false;
  if (!MiniLoadAnyRom())
    return false;

  room_ptr = room->room_id;
  LoadRoomHeader();
  LoadStateHeader();

  uint16 variant_door_ptr = MiniParseDoorVariantPtr(room->bg2_variant_key);
  if (variant_door_ptr != 0)
    door_def_ptr = variant_door_ptr;

  LoadLibraryBackground();
  g_mini_editor_bg2_uses_rom_vram = true;
  return true;
}

static void MiniBuildCollisionMap(int viewport_width, int viewport_height) {
  MiniInitializeScrollState((viewport_width + kMiniBlockSize - 1) / kMiniBlockSize,
                            (viewport_height + kMiniBlockSize - 1) / kMiniBlockSize);

  for (int i = 0; i < kMiniLevelDataCapacity; i++) {
    level_data[i] = kMiniSolidBlock;
    BTS[i] = 0;
  }

  for (int y = 0; y < room_height_in_blocks; y++) {
    int tile_top = y * kMiniBlockSize;
    for (int x = 0; x < room_width_in_blocks; x++) {
      int tile_left = x * kMiniBlockSize;
      bool outside_world = tile_left < g_mini_world_left
                        || tile_left >= g_mini_world_right
                        || tile_top < g_mini_world_ceiling
                        || tile_top >= g_mini_world_floor;
      MiniWriteBlock(x, y, outside_world ? kMiniSolidBlock : 0, 0);
    }
  }
}

static void MiniClearBg2Tilemap(void) {
  for (int i = 0; i < kMiniBg2Tiles; i++)
    g_mini_bg2_tilemap[i] = 0x2C0F;
}

static bool MiniLoadRomFile(const char *path) {
  FILE *f = fopen(path, "rb");
  if (f == NULL)
    return false;

  if (fseek(f, 0, SEEK_END) != 0) {
    fclose(f);
    return false;
  }
  long length = ftell(f);
  if (length <= 0) {
    fclose(f);
    return false;
  }
  rewind(f);

  size_t rom_offset = ((size_t)length & 0x7fff) == 0x200 ? 0x200 : 0;
  size_t rom_size = (size_t)length - rom_offset;
  if (rom_size > kMiniRomCapacity) {
    fclose(f);
    return false;
  }

  memset(g_mini_rom, 0, sizeof(g_mini_rom));
  if (rom_offset != 0)
    fseek(f, (long)rom_offset, SEEK_SET);
  bool ok = fread(g_mini_rom, 1, rom_size, f) == rom_size;
  fclose(f);
  return ok;
}

static bool MiniLoadAnyRom(void) {
  for (size_t i = 0; i < sizeof(kMiniRomCandidates) / sizeof(kMiniRomCandidates[0]); i++) {
    if (MiniLoadRomFile(kMiniRomCandidates[i]))
      return true;
  }
  return false;
}

static bool MiniLoadSaveFile(const char *path) {
  FILE *f = fopen(path, "rb");
  if (f == NULL)
    return false;
  bool ok = fread(g_mini_sram, 1, sizeof(g_mini_sram), f) == sizeof(g_mini_sram);
  fclose(f);
  return ok;
}

static bool MiniLoadAnySave(void) {
  for (size_t i = 0; i < sizeof(kMiniSaveCandidates) / sizeof(kMiniSaveCandidates[0]); i++) {
    if (MiniLoadSaveFile(kMiniSaveCandidates[i]))
      return true;
  }
  return false;
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

static void MiniVramCopy(uint16 vram_dst, const void *src, size_t size) {
  size_t dst = (size_t)vram_dst << 1;
  if (dst >= kMiniVramSize || size == 0)
    return;
  if (size > kMiniVramSize - dst)
    size = kMiniVramSize - dst;
  memcpy(g_mini_vram + dst, src, size);
}

static const uint8 *MiniBusPtr(uint8 bank, uint16 addr) {
  if (bank == 0x7E || bank == 0x7F)
    return g_ram + (((size_t)bank - 0x7E) << 16) + addr;
  return RomPtr(((uint32)bank << 16) | addr);
}

static uint16 MiniVramIncrement(void) {
  switch (g_mini_ppu.vmain & 3) {
  case 0:
    return 1;
  case 1:
    return 32;
  default:
    return 128;
  }
}

static void MiniAdvanceVramAddr(bool accessed_high) {
  if (((g_mini_ppu.vmain & 0x80) != 0) == accessed_high)
    g_mini_ppu.vram_addr += MiniVramIncrement();
}

static void MiniWriteVramData(bool high, uint8 value) {
  size_t offset = ((size_t)g_mini_ppu.vram_addr << 1) + (high ? 1 : 0);
  if (offset < kMiniVramSize)
    g_mini_vram[offset] = value;
  MiniAdvanceVramAddr(high);
}

static uint8 MiniReadVramData(bool high) {
  size_t offset = ((size_t)g_mini_ppu.vram_addr << 1) + (high ? 1 : 0);
  uint8 value = offset < kMiniVramSize ? g_mini_vram[offset] : 0;
  MiniAdvanceVramAddr(high);
  return value;
}

static void MiniWriteCgData(uint8 value) {
  if (!g_mini_ppu.cgram_low_pending) {
    g_mini_ppu.cgram_latch = value;
    g_mini_ppu.cgram_low_pending = true;
    return;
  }
  target_palettes[g_mini_ppu.cgadd & 0xFF] = g_mini_ppu.cgram_latch | (value << 8);
  g_mini_ppu.cgadd++;
  g_mini_ppu.cgram_low_pending = false;
}

static void MiniWriteBbus(uint8 reg, uint8 value) {
  switch (reg) {
  case 0x18:
    MiniWriteVramData(false, value);
    break;
  case 0x19:
    MiniWriteVramData(true, value);
    break;
  case 0x22:
    MiniWriteCgData(value);
    break;
  default:
    break;
  }
}

static void MiniRunDmaChannel1(void) {
  const uint8 *src = MiniBusPtr(g_mini_ppu.a1b1, g_mini_ppu.a1t1);
  bool fixed_source = (g_mini_ppu.dmap1 & 8) != 0;
  switch (g_mini_ppu.dmap1 & 7) {
  case 0:
    for (uint16 i = 0; i < g_mini_ppu.das1; i++) {
      MiniWriteBbus(g_mini_ppu.bbad1, *src);
      if (!fixed_source)
        src++;
    }
    break;
  case 1:
    for (uint16 i = 0; i < g_mini_ppu.das1; i++) {
      MiniWriteBbus(g_mini_ppu.bbad1 + (i & 1), *src);
      if (!fixed_source)
        src++;
    }
    break;
  default:
    break;
  }
}

static void MiniInitGameplayPpu(void) {
  reg_OBSEL = 3;
  reg_BGMODE = 9;
  reg_BG1SC = 0x51;
  reg_BG2SC = 0x49;
  reg_BG3SC = 0x5A;
  reg_BG4SC = 0;
  reg_BG12NBA = 0;
  reg_BG34NBA = 4;
  reg_BG1HOFS = layer1_x_pos;
  reg_BG1VOFS = layer1_y_pos;
  reg_BG2HOFS = layer1_x_pos;
  reg_BG2VOFS = layer1_y_pos;
}

static void MiniClearRoomSprites(void) {
  memset(g_mini_room_sprites, 0, sizeof(g_mini_room_sprites));
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
  MiniVramCopy(0x0000, g_mini_editor_tiles4bpp, kMiniRoomTilesVramSize);
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

static bool MiniLoadSamusBaseTilesFromAssets(void) {
  const uint8 *obj_tiles = SamusAssetBridge_GetData(0x9AD200, kMiniStdObjTilesVramSize);
  if (obj_tiles == NULL)
    return false;
  MiniVramCopy(kMiniStdObjTilesVramDst, obj_tiles, kMiniStdObjTilesVramSize);
  MiniCopySuitPalette();
  return true;
}

static void MiniPrepareEnemyTiles(void) {
  ProcessEnemyTilesets();
  LoadEnemyTileData();
  MiniVramCopy(kMiniEnemyObjTilesVramDst, enemy_spawn_data, kMiniEnemyTileRamSize);
}

static uint16 MiniGetEnemyInitialSpritemap(const EnemyData *enemy) {
  if (enemy->current_instruction == 0)
    return enemy->spritemap_pointer;

  const uint16 *instr = (const uint16 *)RomPtrWithBank(enemy->bank, enemy->current_instruction);
  if ((instr[0] & 0x8000) != 0)
    return enemy->spritemap_pointer;
  return instr[1];
}

static void MiniAddRoomSprite(int slot, uint8 bank, uint16 spritemap, uint16 x_pos, uint16 y_pos,
                              uint16 palette_index, uint16 vram_tiles_index) {
  if ((unsigned)slot >= kMiniRoomSpriteCount)
    return;
  g_mini_room_sprites[slot] = (MiniRoomSprite){
    .active = true,
    .bank = bank,
    .spritemap = spritemap,
    .x_pos = x_pos,
    .y_pos = y_pos,
    .palette_index = palette_index,
    .vram_tiles_index = vram_tiles_index,
  };
}

static bool MiniBootstrapGunshipEnemy(int slot, uint16 population_ptr) {
  uint16 enemy_index = slot * 64;
  EnemyPopulation *ep = get_EnemyPopulation(0xA1, population_ptr);
  EnemyDef *ed = get_EnemyDef_A2(ep->enemy_ptr);
  EnemyData *enemy = gEnemyData(enemy_index);
  EnemySpawnData *spawn = gEnemySpawnData(enemy_index);

  memset(enemy, 0, sizeof(*enemy));
  memset(spawn, 0, sizeof(*spawn));

  enemy->enemy_ptr = ep->enemy_ptr;
  enemy->x_pos = ep->x_pos;
  enemy->y_pos = ep->y_pos;
  enemy->x_width = ed->x_radius;
  enemy->y_height = ed->y_radius;
  enemy->health = ed->health;
  enemy->layer = ed->layer;
  enemy->current_instruction = ep->init_param;
  enemy->properties = ep->properties;
  enemy->extra_properties = ep->extra_properties;
  enemy->parameter_1 = ep->parameter1;
  enemy->parameter_2 = ep->parameter2;
  enemy->instruction_timer = 1;
  enemy->bank = ed->bank;

  LoadEnemyGfxIndexes(population_ptr, enemy_index);

  cur_enemy_index = enemy_index;
  if (ep->enemy_ptr == 0xD07F) {
    GunshipTop_Init();
  } else if (ep->enemy_ptr == 0xD0BF) {
    GunshipBottom_Init();
  } else {
    return false;
  }

  uint16 spritemap = MiniGetEnemyInitialSpritemap(enemy);
  if (spritemap == 0)
    return false;

  MiniAddRoomSprite(slot, enemy->bank, spritemap, enemy->x_pos, enemy->y_pos,
                    enemy->palette_index, enemy->vram_tiles_index);
  return true;
}

static void MiniBootstrapLandingSiteGunship(void) {
  if (room_ptr != kMiniLandingSiteRoom || room_enemy_population_ptr == 0 || room_enemy_tilesets_ptr == 0)
    return;

  MiniPrepareEnemyTiles();
  memset(enemy_data, 0, 2048);
  memset(enemy_spawn_data, 0, 0x2800);

  for (int i = 0, slot = 0; slot < kMiniRoomSpriteCount; i++, slot++) {
    EnemyPopulation *ep = get_EnemyPopulation(0xA1, room_enemy_population_ptr + i * sizeof(EnemyPopulation));
    if (ep->enemy_ptr == 0xFFFF)
      break;
    MiniBootstrapGunshipEnemy(slot, room_enemy_population_ptr + i * sizeof(EnemyPopulation));
  }
}

static void MiniLoadCurrentRoomAssets(void) {
  MiniInitGameplayPpu();
  LoadInitialPalette();
  LoadRoomHeader();
  LoadStateHeader();
  LoadLevelDataAndOtherThings();
  DecompressToMem(Load24(&tileset_tiles_pointer), (uint8 *)tilemap_stuff);
  DecompressToMem(Load24(&tileset_compr_palette_ptr), (uint8 *)target_palettes);
  memcpy(palette_buffer, target_palettes, 512);
  MiniVramCopy(0x0000, tilemap_stuff, 0x2000);
  MiniVramCopy(0x1000, (uint8 *)tilemap_stuff + 0x2000, 0x2000);
  MiniVramCopy(0x2000, (uint8 *)tilemap_stuff + 0x4000, 0x1000);
  DecompressToMem(0xb98000, g_mini_vram + ((size_t)kMiniCreTilesVramDst << 1));
  MiniVramCopy(kMiniStdObjTilesVramDst, RomFixedPtr(0x9AD200), kMiniStdObjTilesVramSize);
  LoadColorsForSpritesBeamsAndEnemies();
  MiniCopySuitPalette();
  MiniClearRoomSprites();
  MiniBootstrapLandingSiteGunship();
}

static void MiniLoadRoomTilemaps(void) {
  LoadLibraryBackground();
  DisplayViewablePartOfRoom();
  CalculateLayer2Xpos();
  CalculateLayer2Ypos();
  CalculateBgScrolls();
}

static void MiniConfigureRoomInfo(bool booted_from_save_slot, int spawn_x, int spawn_y) {
  g_mini_room_info = (MiniRoomInfo){
    .has_room = true,
    .uses_rom_room = true,
    .booted_from_save_slot = booted_from_save_slot,
    .has_editor_room_visuals = false,
    .samus_suit = (equipped_items & 0x20) != 0 ? kMiniSamusSuit_Gravity
                 : (equipped_items & 1) != 0 ? kMiniSamusSuit_Varia
                 : kMiniSamusSuit_Power,
    .room_id = room_ptr,
    .room_source = booted_from_save_slot ? kMiniRoomSource_RomSave : kMiniRoomSource_RomDemo,
    .room_left = 0,
    .room_top = 0,
    .room_right = room_width_in_blocks * kMiniBlockSize,
    .room_bottom = room_height_in_blocks * kMiniBlockSize,
    .room_width_blocks = room_width_in_blocks,
    .room_height_blocks = room_height_in_blocks,
    .camera_x = layer1_x_pos,
    .camera_y = layer1_y_pos,
    .spawn_x = spawn_x,
    .spawn_y = spawn_y,
  };
  MiniSetRoomLabel(&g_mini_room_info, room_ptr == kMiniLandingSiteRoom ? "landingSite" : "romRoom",
                   room_ptr == kMiniLandingSiteRoom ? "Landing Site" : "ROM Room");
  g_mini_world_left = g_mini_room_info.room_left;
  g_mini_world_right = g_mini_room_info.room_right;
  g_mini_world_ceiling = g_mini_room_info.room_top;
  g_mini_world_floor = g_mini_room_info.room_bottom;
}

static bool MiniTryConfigureEditorRoom(void) {
  MiniEditorRoom room;
  if (!MiniEditorBridge_LoadRoom(&room))
    return false;
  if ((size_t)room.width_blocks * room.height_blocks > kMiniLevelDataCapacity) {
    MiniEditorBridge_FreeRoom(&room);
    return false;
  }

  room_ptr = room.room_id;
  MiniInitGameplayPpu();
  MiniInstallEditorTilesetAssets(&room);
  MiniInstallEditorSamusAssets(&room);
  MiniInstallEditorRoomSprites(&room);
  MiniSetSamusSuitState(g_mini_editor_samus_initial_suit);
  MiniInitializeScrollState(room.width_blocks, room.height_blocks);
  MiniTryLoadRoomHeaderMetadata(room.room_id);
  MiniApplyEditorScrollState(&room);
  MiniMaybeLoadEditorRoomRomBackground(&room);
  if (!MiniLoadSamusBaseTilesFromAssets() && MiniLoadAnyRom()) {
    MiniVramCopy(kMiniStdObjTilesVramDst, RomFixedPtr(0x9AD200), kMiniStdObjTilesVramSize);
    MiniCopySuitPalette();
  }
  MiniClearRoomSprites();
  layer1_x_pos = room.camera_x;
  layer1_y_pos = room.camera_y;
  layer1_x_subpos = 0;
  layer1_y_subpos = 0;
  ideal_layer1_xpos = layer1_x_pos;
  ideal_layer1_ypos = layer1_y_pos;
  *(uint16 *)&layer2_scroll_x = (uint16)room.export_bg_scrolling;
  CalculateLayer2Xpos();
  CalculateLayer2Ypos();
  bg2_x_scroll = 0;
  bg2_y_scroll = 0;
  CalculateBgScrolls();
  samus_x_pos = room.spawn_x;
  samus_y_pos = room.spawn_y;

  memset(level_data, 0, sizeof(uint16) * kMiniLevelDataCapacity);
  memset(BTS, 0, kMiniLevelDataCapacity);
  for (int y = 0; y < room.height_blocks; y++) {
    for (int x = 0; x < room.width_blocks; x++) {
      size_t index = (size_t)y * room.width_blocks + x;
      uint16 level = room.block_words != NULL ? room.block_words[index]
                                              : (uint16)room.collision_types[index] << 12;
      MiniWriteBlock(x, y, level, room.bts[index]);
    }
  }

  g_mini_room_info = (MiniRoomInfo){
    .has_room = true,
    .uses_rom_room = false,
    .booted_from_save_slot = false,
    .has_editor_room_visuals = g_mini_has_editor_tileset_assets,
    .samus_suit = g_mini_editor_samus_initial_suit,
    .room_id = room.room_id,
    .room_source = kMiniRoomSource_EditorExport,
    .room_left = 0,
    .room_top = 0,
    .room_right = room.width_blocks * kMiniBlockSize,
    .room_bottom = room.height_blocks * kMiniBlockSize,
    .room_width_blocks = room.width_blocks,
    .room_height_blocks = room.height_blocks,
    .camera_x = room.camera_x,
    .camera_y = room.camera_y,
    .spawn_x = room.spawn_x,
    .spawn_y = room.spawn_y,
  };
  MiniSetRoomLabel(&g_mini_room_info, room.handle, room.name);
  g_mini_world_left = g_mini_room_info.room_left;
  g_mini_world_right = g_mini_room_info.room_right;
  g_mini_world_ceiling = g_mini_room_info.room_top;
  g_mini_world_floor = g_mini_room_info.room_bottom;
  MiniClampCamera();
  MiniEditorBridge_FreeRoom(&room);
  return true;
}

static bool MiniTryConfigureSaveSlotRoom(void) {
  MiniClearEditorTilesetAssets();
  MiniClearEditorSamusPaletteAssets();
  SamusAssetBridge_Reset();
  if (!MiniLoadAnyRom() || !MiniLoadAnySave())
    return false;
  if (LoadFromSram(0) != 0)
    return false;

  selected_save_slot = 0;
  LoadFromLoadStation();
  if (room_ptr == kMiniLandingSiteRoom) {
    layer1_x_pos = kMiniLandingSiteCameraX;
    layer1_y_pos = kMiniLandingSiteCameraY;
    samus_x_pos = kMiniLandingSiteSamusX;
    samus_y_pos = kMiniLandingSiteSamusY;
  }
  MiniLoadCurrentRoomAssets();
  MiniLoadRoomTilemaps();
  MiniConfigureRoomInfo(true, samus_x_pos, samus_y_pos);
  return true;
}

static bool MiniTryConfigureDemoRoom(void) {
  MiniClearEditorTilesetAssets();
  MiniClearEditorSamusPaletteAssets();
  SamusAssetBridge_Reset();
  if (!MiniLoadAnyRom())
    return false;

  const uint16 *demo_sets = (const uint16 *)RomFixedPtr(0x82876c);
  DemoRoomData *drd = get_DemoRoomData(demo_sets[0]);
  room_ptr = drd->room_ptr_;
  layer1_x_pos = drd->screen_x_pos;
  layer1_y_pos = drd->screen_y_pos;
  int spawn_x = layer1_x_pos + 128 + drd->samus_y_offs;
  int spawn_y = layer1_y_pos + drd->samus_x_offs;
  if (room_ptr == kMiniLandingSiteRoom) {
    layer1_x_pos = kMiniLandingSiteCameraX;
    layer1_y_pos = kMiniLandingSiteCameraY;
    spawn_x = kMiniLandingSiteSamusX;
    spawn_y = kMiniLandingSiteSamusY;
  }
  MiniLoadCurrentRoomAssets();
  MiniLoadRoomTilemaps();
  MiniConfigureRoomInfo(false, spawn_x, spawn_y);
  return true;
}

static int32 MiniHorizontalStep(void) {
  return INT16_SHL16(kMiniAirSpeed);
}

static int MiniClamp(int value, int min_value, int max_value) {
  if (value < min_value)
    return min_value;
  if (value > max_value)
    return max_value;
  return value;
}

static void MiniClampCamera(void) {
  int max_x = room_width_in_blocks * kMiniBlockSize - kMiniGameWidth;
  int max_y = room_height_in_blocks * kMiniBlockSize - kMiniGameHeight;
  layer1_x_pos = MiniClamp(layer1_x_pos, 0, max_x > 0 ? max_x : 0);
  layer1_y_pos = MiniClamp(layer1_y_pos, 0, max_y > 0 ? max_y : 0);
  layer1_x_subpos = 0;
  layer1_y_subpos = 0;
}

void MiniStubs_Reset(void) {
  memset(g_ram, 0, sizeof(g_ram));
  memset(g_mini_sram, 0, sizeof(g_mini_sram));
  memset(g_mini_rom, 0, sizeof(g_mini_rom));
  memset(g_mini_bg2_tilemap, 0, sizeof(g_mini_bg2_tilemap));
  memset(g_mini_vram, 0, sizeof(g_mini_vram));
  memset(&g_mini_ppu, 0, sizeof(g_mini_ppu));
  MiniClearRoomSprites();
  MiniClearEditorTilesetAssets();
  MiniClearEditorSamusPaletteAssets();
  SamusAssetBridge_Reset();
  g_sram = g_mini_sram;
  g_rom = g_mini_rom;
}

void MiniStubs_SetRoomExportPath(const char *path) {
  MiniEditorBridge_SetRoomExportPath(path);
}

void MiniStubs_ConfigureWorld(int viewport_width, int viewport_height) {
  if (MiniTryConfigureEditorRoom() || MiniTryConfigureSaveSlotRoom() || MiniTryConfigureDemoRoom())
    return;

  g_mini_world_left = kMiniWorldPadding;
  g_mini_world_right = viewport_width - kMiniWorldPadding;
  g_mini_world_ceiling = kMiniWorldPadding;
  g_mini_world_floor = AlignUpToBlock(viewport_height - kMiniGroundOffset);

  g_mini_room_info = (MiniRoomInfo){
    .has_room = true,
    .uses_rom_room = false,
    .booted_from_save_slot = false,
    .has_editor_room_visuals = false,
    .samus_suit = kMiniSamusSuit_Power,
    .room_id = 0,
    .room_source = kMiniRoomSource_Fallback,
    .room_left = g_mini_world_left,
    .room_top = g_mini_world_ceiling,
    .room_right = g_mini_world_right,
    .room_bottom = g_mini_world_floor,
    .room_width_blocks = (viewport_width + kMiniBlockSize - 1) / kMiniBlockSize,
    .room_height_blocks = (viewport_height + kMiniBlockSize - 1) / kMiniBlockSize,
    .camera_x = 0,
    .camera_y = 0,
    .spawn_x = g_mini_world_left + 3 * kMiniBlockSize,
    .spawn_y = g_mini_world_floor,
  };
  MiniSetRoomLabel(&g_mini_room_info, "fallbackRoom", "Fallback Room");
  MiniBuildCollisionMap(viewport_width, viewport_height);
}

void MiniStubs_GetRoomInfo(MiniRoomInfo *info) {
  *info = g_mini_room_info;
}

int MiniStubs_GetRoomSprites(const MiniRoomSprite **sprites) {
  *sprites = g_mini_room_sprites;
  return kMiniRoomSpriteCount;
}

void MiniStubs_GetEditorTilesetView(MiniEditorTilesetView *view) {
  *view = (MiniEditorTilesetView){
    .loaded = g_mini_has_editor_tileset_assets,
    .tileset_id = g_mini_editor_tileset_id,
    .tiles4bpp = g_mini_editor_tiles4bpp,
    .metatile_words = g_mini_editor_metatile_words,
    .palette = g_mini_editor_palette,
  };
}

void MiniStubs_GetEditorBg2View(MiniEditorBg2View *view) {
  *view = (MiniEditorBg2View){
    .loaded = g_mini_has_editor_bg2_assets,
    .uses_rom_vram = g_mini_editor_bg2_uses_rom_vram,
    .tilemap_words = g_mini_editor_bg2_tilemap_words,
    .scroll_x = g_mini_editor_bg2_scroll_x,
    .scroll_y = g_mini_editor_bg2_scroll_y,
    .variant_key = g_mini_editor_bg2_variant_key,
  };
}

int MiniStubs_GetEditorRoomSpriteViews(const MiniEditorRoomSpriteView **sprites) {
  *sprites = g_mini_editor_room_sprite_views;
  return g_mini_editor_room_sprite_count;
}

uint16 MiniStubs_GetLevelBlock(int block_x, int block_y) {
  if ((unsigned)block_x >= room_width_in_blocks || (unsigned)block_y >= room_height_in_blocks)
    return kMiniSolidBlock;
  return level_data[block_y * room_width_in_blocks + block_x];
}

uint8 MiniStubs_GetBts(int block_x, int block_y) {
  if ((unsigned)block_x >= room_width_in_blocks || (unsigned)block_y >= room_height_in_blocks)
    return 0;
  return BTS[block_y * room_width_in_blocks + block_x];
}

uint16 MiniStubs_GetBg2Tile(int tile_x, int tile_y) {
  uint16 base = (reg_BG2SC & 0xFC) << 8;
  uint16 addr = base + (((tile_y & 31) << 5) | (tile_x & 31));
  if ((tile_x & 32) && (reg_BG2SC & 1))
    addr += 0x400;
  if ((tile_y & 32) && (reg_BG2SC & 2))
    addr += (reg_BG2SC & 1) ? 0x800 : 0x400;
  size_t offset = (size_t)addr << 1;
  return offset + 1 < kMiniVramSize ? GET_WORD(g_mini_vram + offset) : 0;
}

uint8 *MiniStubs_GetVram(void) {
  return g_mini_vram;
}

int MiniStubs_GetFloorY(void) {
  return g_mini_world_floor;
}

void MiniStubs_ClampCameraToRoom(void) {
  MiniClampCamera();
  CalculateLayer2Xpos();
  CalculateLayer2Ypos();
  CalculateBgScrolls();
}

const char *MiniStubs_RoomSourceName(MiniRoomSource source) {
  switch (source) {
  case kMiniRoomSource_EditorExport:
    return "editor_export";
  case kMiniRoomSource_RomSave:
    return "rom_save";
  case kMiniRoomSource_RomDemo:
    return "rom_demo";
  case kMiniRoomSource_Fallback:
  default:
    return "fallback";
  }
}

const char *MiniStubs_SamusSuitName(MiniSamusSuit suit) {
  switch (suit) {
  case kMiniSamusSuit_Varia:
    return "varia";
  case kMiniSamusSuit_Gravity:
    return "gravity";
  default:
    return "power";
  }
}

void MemCpy(void *dst, const void *src, int size) {
  memcpy(dst, src, size);
}

void RtlApplyPhysicsParams(void) {
  samus_y_accel = g_physics_params.gravity_accel;
  samus_y_subaccel = g_physics_params.gravity_subaccel;
}

void mov24(struct LongPtr *dst, uint32 src) {
  dst->addr = src;
  dst->bank = src >> 16;
}

uint32 Load24(const LongPtr *src) {
  return src->addr | (src->bank << 16);
}

PairU16 MakePairU16(uint16 k, uint16 j) {
  PairU16 pair = { .k = k, .j = j };
  return pair;
}

const uint8 *RomPtr(uint32_t addr) {
  return &g_rom[(((addr >> 16) << 15) | (addr & 0x7fff)) & 0x3fffff];
}

uint16 Mult8x8(uint8 a, uint8 b) {
  return a * b;
}

uint16 SnesDivide(uint16 a, uint8 b) {
  return (b == 0) ? 0xffff : a / b;
}

uint16 SnesModulus(uint16 a, uint8 b) {
  return (b == 0) ? a : a % b;
}

void WriteReg(uint16 reg, uint8 value) {
  switch (reg) {
  case VMAIN:
    g_mini_ppu.vmain = value;
    break;
  case VMADDL:
    g_mini_ppu.vram_addr = (g_mini_ppu.vram_addr & 0xFF00) | value;
    break;
  case VMADDH:
    g_mini_ppu.vram_addr = (g_mini_ppu.vram_addr & 0x00FF) | (value << 8);
    break;
  case VMDATAL:
    MiniWriteVramData(false, value);
    break;
  case VMDATAH:
    MiniWriteVramData(true, value);
    break;
  case DMAP1:
    g_mini_ppu.dmap1 = value;
    break;
  case BBAD1:
    g_mini_ppu.bbad1 = value;
    break;
  case A1T1L:
    g_mini_ppu.a1t1 = (g_mini_ppu.a1t1 & 0xFF00) | value;
    break;
  case A1T1H:
    g_mini_ppu.a1t1 = (g_mini_ppu.a1t1 & 0x00FF) | (value << 8);
    break;
  case A1B1:
    g_mini_ppu.a1b1 = value;
    break;
  case DAS1L:
    g_mini_ppu.das1 = (g_mini_ppu.das1 & 0xFF00) | value;
    break;
  case DAS1H:
    g_mini_ppu.das1 = (g_mini_ppu.das1 & 0x00FF) | (value << 8);
    break;
  case CGADD:
    g_mini_ppu.cgadd = value;
    g_mini_ppu.cgram_low_pending = false;
    break;
  case CGDATA:
    MiniWriteCgData(value);
    break;
  case MDMAEN:
    if (value & 2)
      MiniRunDmaChannel1();
    break;
  default:
    break;
  }
}

uint8 ReadReg(uint16 reg) {
  switch (reg) {
  case RDVRAML:
    return MiniReadVramData(false);
  case RDVRAMH:
    return MiniReadVramData(true);
  default:
    return 0;
  }
}

uint16 ReadRegWord(uint16 reg) {
  uint16 value = ReadReg(reg);
  value |= (uint16)ReadReg(reg + 1) << 8;
  return value;
}

void WriteRegWord(uint16 reg, uint16 value) {
  WriteReg(reg, (uint8)value);
  WriteReg(reg + 1, value >> 8);
}

bool Unreachable(void) {
  Die("Unreachable\n");
}

NORETURN void Die(const char *error) {
  fprintf(stderr, "%s", error);
  abort();
}

void Warning(const char *error) {
  fprintf(stderr, "%s", error);
}
