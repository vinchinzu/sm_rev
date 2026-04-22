#include "stubs_mini.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "funcs.h"
#include "ida_types.h"
#include "enemy_types.h"
#include "mini_defs.h"
#include "mini_asset_bootstrap.h"
#include "mini_editor_bridge.h"
#include "mini_ppu_stub.h"
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
  kMiniRoomTilesVramSize = 0x5000,
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
  MiniPpu_InitGameplay();
  MiniAssetBootstrap_InstallEditorAssets(&room);
  MiniAssetBootstrap_SetSamusSuitState(MiniAssetBootstrap_GetInitialSuit());
  MiniInitializeScrollState(room.width_blocks, room.height_blocks);
  MiniTryLoadRoomHeaderMetadata(room.room_id);
  MiniApplyEditorScrollState(&room);
  if (!MiniAssetBootstrap_LoadSamusBaseTilesFromAssets() && MiniLoadAnyRom())
    MiniAssetBootstrap_InstallRomSamusBaseTiles();
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
    .has_editor_room_visuals = MiniAssetBootstrap_HasEditorTilesetAssets(),
    .samus_suit = MiniAssetBootstrap_GetInitialSuit(),
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
  MiniAssetBootstrap_Reset();
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
  MiniAssetBootstrap_LoadCurrentRoomAssets();
  MiniLoadRoomTilemaps();
  MiniConfigureRoomInfo(true, samus_x_pos, samus_y_pos);
  return true;
}

static bool MiniTryConfigureDemoRoom(void) {
  MiniAssetBootstrap_Reset();
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
  MiniAssetBootstrap_LoadCurrentRoomAssets();
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
  MiniPpu_Reset();
  MiniAssetBootstrap_Reset();
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
  return MiniAssetBootstrap_GetRoomSprites(sprites);
}

void MiniStubs_GetEditorTilesetView(MiniEditorTilesetView *view) {
  MiniAssetBootstrap_GetEditorTilesetView(view);
}

void MiniStubs_GetEditorBg2View(MiniEditorBg2View *view) {
  MiniAssetBootstrap_GetEditorBg2View(view);
}

int MiniStubs_GetEditorRoomSpriteViews(const MiniEditorRoomSpriteView **sprites) {
  return MiniAssetBootstrap_GetEditorRoomSpriteViews(sprites);
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
