#include "mini_room_adapter.h"

#include <stdio.h>
#include <string.h>

#include "funcs.h"
#include "features.h"
#include "mini_asset_bootstrap.h"
#include "mini_content_scope.h"
#include "mini_defs.h"
#include "mini_editor_bridge.h"
#include "mini_ppu_stub.h"
#include "mini_rom_bootstrap.h"
#include "variables.h"

enum {
  kMiniWorldPadding = 32,
  kMiniGroundOffset = 48,
  kMiniDefaultUpScroller = 112,
  kMiniDefaultDownScroller = 160,
  kMiniLevelDataCapacity = (0x16402 - 0x10002) / 2,
};

static int g_mini_world_left;
static int g_mini_world_right;
static int g_mini_world_ceiling;
static int g_mini_world_floor;
static bool g_mini_explicit_room_export_path;
static MiniRoomInfo g_mini_room_info;

static void MiniClampCamera(void);

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

static void MiniApplyRoomInfoWorld(void) {
  g_mini_world_left = g_mini_room_info.room_left;
  g_mini_world_right = g_mini_room_info.room_right;
  g_mini_world_ceiling = g_mini_room_info.room_top;
  g_mini_world_floor = g_mini_room_info.room_bottom;
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

static bool MiniTryConfigureEditorRoom(void) {
  MiniEditorRoom room;
  if (!MiniEditorBridge_LoadRoom(&room))
    return false;
  if (!MiniContentScope_AllowsRoom(room.room_id)) {
    fprintf(stderr, "mini: refusing editor room 0x%04X outside %s scope\n",
            room.room_id, MiniContentScope_Name());
    MiniEditorBridge_FreeRoom(&room);
    return false;
  }
  if ((size_t)room.width_blocks * room.height_blocks > kMiniLevelDataCapacity) {
    MiniEditorBridge_FreeRoom(&room);
    return false;
  }

  room_ptr = room.room_id;
  MiniPpu_InitGameplay();
  MiniAssetBootstrap_InstallEditorAssets(&room);
  MiniAssetBootstrap_SetSamusSuitState(MiniAssetBootstrap_GetInitialSuit());
  MiniInitializeScrollState(room.width_blocks, room.height_blocks);
  if (MiniRomBootstrap_LoadAnyRom()) {
    MiniAssetBootstrap_PrimeEditorRoomFxAndMissingRomVisuals(
        &room,
        !room.has_tileset_assets,
        !room.has_bg2_assets);
  }
  MiniRomBootstrap_TryLoadRoomHeaderMetadata(room.room_id);
  MiniApplyEditorScrollState(&room);
  if (!MiniAssetBootstrap_LoadSamusBaseTilesFromAssets() && MiniRomBootstrap_LoadAnyRom())
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
                                              : BlockTileWithTypeIndex(0, room.collision_types[index]);
      MiniWriteBlock(x, y, level, room.bts[index]);
    }
  }

  g_mini_room_info = (MiniRoomInfo){
    .has_room = true,
    .uses_rom_room = false,
    .booted_from_save_slot = false,
    .has_editor_room_visuals = MiniAssetBootstrap_HasEditorTilesetAssets(),
    .uses_original_gameplay_runtime = false,
    .has_original_enemies = false,
    .has_original_plms = false,
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
    .camera_target_x_percent = room.camera_target_x_percent,
    .camera_target_y_percent = room.camera_target_y_percent,
  };
  g_mini_room_info.doorway_count = room.doorway_count;
  memcpy(g_mini_room_info.doorways, room.doorways, sizeof(g_mini_room_info.doorways));
  MiniSetRoomLabel(&g_mini_room_info, room.handle, room.name);
  MiniApplyRoomInfoWorld();
  MiniClampCamera();
  MiniEditorBridge_FreeRoom(&room);
  return true;
}

static bool MiniTryConfigureSaveSlotRoom(void) {
  if (!MiniRomBootstrap_TryConfigureSaveSlotRoom(&g_mini_room_info))
    return false;
  MiniApplyRoomInfoWorld();
  return true;
}

static bool MiniTryConfigureDemoRoom(void) {
  if (!MiniRomBootstrap_TryConfigureDemoRoom(&g_mini_room_info))
    return false;
  MiniApplyRoomInfoWorld();
  return true;
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

void MiniStubs_SetRoomExportPath(const char *path) {
  g_mini_explicit_room_export_path = path != NULL && path[0] != '\0';
  MiniEditorBridge_SetRoomExportPath(path);
}

void MiniStubs_ConfigureWorld(int viewport_width, int viewport_height) {
  if (BUILD_IS_MODDABLE) {
    if (MiniTryConfigureEditorRoom())
      return;
  } else if (g_mini_explicit_room_export_path) {
    if (MiniTryConfigureEditorRoom() || MiniTryConfigureSaveSlotRoom() || MiniTryConfigureDemoRoom())
      return;
  } else if (MiniTryConfigureSaveSlotRoom() || MiniTryConfigureDemoRoom() || MiniTryConfigureEditorRoom()) {
    return;
  }

  g_mini_world_left = kMiniWorldPadding;
  g_mini_world_right = viewport_width - kMiniWorldPadding;
  g_mini_world_ceiling = kMiniWorldPadding;
  g_mini_world_floor = AlignUpToBlock(viewport_height - kMiniGroundOffset);

  g_mini_room_info = (MiniRoomInfo){
    .has_room = true,
    .uses_rom_room = false,
    .booted_from_save_slot = false,
    .has_editor_room_visuals = false,
    .uses_original_gameplay_runtime = false,
    .has_original_enemies = false,
    .has_original_plms = false,
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
    .camera_target_x_percent = kMiniCameraFollowDefaultTargetPercent,
    .camera_target_y_percent = kMiniCameraFollowDefaultTargetPercent,
  };
  MiniSetRoomLabel(&g_mini_room_info, "fallbackRoom", "Fallback Room");
  MiniBuildCollisionMap(viewport_width, viewport_height);
}

void MiniStubs_GetRoomInfo(MiniRoomInfo *info) {
  *info = g_mini_room_info;
}

void MiniStubs_GetCollisionMapView(MiniCollisionMapView *view) {
  *view = (MiniCollisionMapView){
    .block_size = kMiniBlockSize,
    .width_blocks = room_width_in_blocks,
    .height_blocks = room_height_in_blocks,
    .world_left = g_mini_world_left,
    .world_top = g_mini_world_ceiling,
    .world_right = g_mini_world_right,
    .world_bottom = g_mini_world_floor,
  };
}

void MiniStubs_SaveSnapshot(MiniStubsSnapshot *snapshot) {
  snapshot->world_left = g_mini_world_left;
  snapshot->world_right = g_mini_world_right;
  snapshot->world_ceiling = g_mini_world_ceiling;
  snapshot->world_floor = g_mini_world_floor;
  snapshot->explicit_room_export_path = g_mini_explicit_room_export_path;
  snapshot->room_info = g_mini_room_info;
}

void MiniStubs_LoadSnapshot(const MiniStubsSnapshot *snapshot) {
  g_mini_world_left = snapshot->world_left;
  g_mini_world_right = snapshot->world_right;
  g_mini_world_ceiling = snapshot->world_ceiling;
  g_mini_world_floor = snapshot->world_floor;
  g_mini_explicit_room_export_path = snapshot->explicit_room_export_path;
  g_mini_room_info = snapshot->room_info;
}

uint16 MiniStubs_GetLevelBlock(int block_x, int block_y) {
  if ((unsigned)block_x >= room_width_in_blocks || (unsigned)block_y >= room_height_in_blocks)
    return kMiniSolidBlock;
  return level_data[block_y * room_width_in_blocks + block_x];
}

BlockType MiniStubs_GetCollisionMaterial(int block_x, int block_y) {
  return (BlockType)BlockTypeFromTile(MiniStubs_GetLevelBlock(block_x, block_y));
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
