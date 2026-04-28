#include "mini_rom_bootstrap.h"

#include <stdio.h>
#include <string.h>

#include "funcs.h"
#include "ida_types.h"
#include "mini_asset_bootstrap.h"
#include "mini_content_scope.h"
#include "samus_env.h"
#include "sm_rtl.h"
#include "variables.h"

enum {
  kMiniRomCapacity = 0x400000,
  kMiniSramSize = 0x2000,
  kMiniLandingSiteRoom = 0x91F8,
  kMiniLandingSiteCameraX = 1024,
  kMiniLandingSiteCameraY = 976,
  kMiniLandingSiteSamusX = 1153,
  kMiniLandingSiteSamusY = 1088,
  kMiniDemoRoomSetTable = 0x82876c,
};

static uint8 g_mini_sram[kMiniSramSize];
static uint8 g_mini_rom[kMiniRomCapacity];

uint8 *g_sram = g_mini_sram;
const uint8 *g_rom = g_mini_rom;

static const char *const kMiniRomCandidates[] = {
  "sm.smc",
  "../sm/sm.smc",
  "../roms/rom.sfc",
};

static const char *const kMiniSaveCandidates[] = {
  "saves/sm.srm",
  "../sm/saves/sm.srm",
};

static void MiniRomBootstrap_SetRoomLabel(MiniRoomInfo *info, const char *handle, const char *name) {
  snprintf(info->room_handle, sizeof(info->room_handle), "%s", handle != NULL ? handle : "");
  snprintf(info->room_name, sizeof(info->room_name), "%s", name != NULL ? name : "");
}

static bool MiniRomBootstrap_LoadRomFile(const char *path) {
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

bool MiniRomBootstrap_LoadAnyRom(void) {
  for (size_t i = 0; i < sizeof(kMiniRomCandidates) / sizeof(kMiniRomCandidates[0]); i++) {
    if (MiniRomBootstrap_LoadRomFile(kMiniRomCandidates[i]))
      return true;
  }
  return false;
}

static bool MiniRomBootstrap_LoadSaveFile(const char *path) {
  FILE *f = fopen(path, "rb");
  if (f == NULL)
    return false;
  bool ok = fread(g_mini_sram, 1, sizeof(g_mini_sram), f) == sizeof(g_mini_sram);
  fclose(f);
  return ok;
}

static bool MiniRomBootstrap_LoadAnySave(void) {
  for (size_t i = 0; i < sizeof(kMiniSaveCandidates) / sizeof(kMiniSaveCandidates[0]); i++) {
    if (MiniRomBootstrap_LoadSaveFile(kMiniSaveCandidates[i]))
      return true;
  }
  return false;
}

static void MiniRomBootstrap_LoadRoomTilemaps(void) {
  LoadLibraryBackground();
  DisplayViewablePartOfRoom();
  RefreshFxVisualsAfterLoad();
  CalculateLayer2Xpos();
  CalculateLayer2Ypos();
  CalculateBgScrolls();
}

static void MiniRomBootstrap_FillRoomInfo(MiniRoomInfo *info, bool booted_from_save_slot,
                                          int spawn_x, int spawn_y) {
  *info = (MiniRoomInfo){
    .has_room = true,
    .uses_rom_room = true,
    .booted_from_save_slot = booted_from_save_slot,
    .has_editor_room_visuals = false,
    .uses_original_gameplay_runtime = MiniContentScope_AllowsRoom(room_ptr),
    .has_original_enemies = num_enemies_in_room != 0,
    .has_original_plms = (plm_flag & 0x8000) != 0,
    .samus_suit = Samus_HasEquip(kSamusEquip_GravitySuit) ? kMiniSamusSuit_Gravity
                 : Samus_HasEquip(kSamusEquip_VariaSuit) ? kMiniSamusSuit_Varia
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
  MiniRomBootstrap_SetRoomLabel(info, MiniContentScope_RoomHandle(room_ptr),
                                MiniContentScope_RoomName(room_ptr));
}

static void MiniRomBootstrap_ApplyLandingSiteDefaults(int *spawn_x, int *spawn_y) {
  if (room_ptr != kMiniLandingSiteRoom)
    return;
  layer1_x_pos = kMiniLandingSiteCameraX;
  layer1_y_pos = kMiniLandingSiteCameraY;
  *spawn_x = kMiniLandingSiteSamusX;
  *spawn_y = kMiniLandingSiteSamusY;
}

void MiniRomBootstrap_Reset(void) {
  memset(g_mini_sram, 0, sizeof(g_mini_sram));
  memset(g_mini_rom, 0, sizeof(g_mini_rom));
  g_sram = g_mini_sram;
  g_rom = g_mini_rom;
}

void MiniRomBootstrap_TryLoadRoomHeaderMetadata(uint16 room_id) {
  if (!MiniRomBootstrap_LoadAnyRom())
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

bool MiniRomBootstrap_TryConfigureSaveSlotRoom(MiniRoomInfo *info) {
  MiniAssetBootstrap_Reset();
  if (!MiniRomBootstrap_LoadAnyRom() || !MiniRomBootstrap_LoadAnySave())
    return false;
  if (LoadFromSram(0) != 0)
    return false;

  selected_save_slot = 0;
  LoadFromLoadStation();
  if (!MiniContentScope_AllowsRoom(room_ptr))
    return false;
  int spawn_x = samus_x_pos;
  int spawn_y = samus_y_pos;
  MiniRomBootstrap_ApplyLandingSiteDefaults(&spawn_x, &spawn_y);
  samus_x_pos = spawn_x;
  samus_y_pos = spawn_y;
  MiniAssetBootstrap_LoadCurrentRoomAssets();
  MiniRomBootstrap_LoadRoomTilemaps();
  MiniRomBootstrap_FillRoomInfo(info, true, samus_x_pos, samus_y_pos);
  return true;
}

bool MiniRomBootstrap_TryConfigureDemoRoom(MiniRoomInfo *info) {
  MiniAssetBootstrap_Reset();
  if (!MiniRomBootstrap_LoadAnyRom())
    return false;

  const uint16 *demo_sets = (const uint16 *)RomFixedPtr(kMiniDemoRoomSetTable);
  DemoRoomData *drd = get_DemoRoomData(demo_sets[0]);
  room_ptr = drd->room_ptr_;
  door_def_ptr = drd->door_ptr;
  if (!MiniContentScope_AllowsRoom(room_ptr))
    return false;
  layer1_x_pos = drd->screen_x_pos;
  layer1_y_pos = drd->screen_y_pos;
  int spawn_x = layer1_x_pos + 128 + drd->samus_y_offs;
  int spawn_y = layer1_y_pos + drd->samus_x_offs;
  MiniRomBootstrap_ApplyLandingSiteDefaults(&spawn_x, &spawn_y);
  MiniAssetBootstrap_LoadCurrentRoomAssets();
  MiniRomBootstrap_LoadRoomTilemaps();
  MiniRomBootstrap_FillRoomInfo(info, false, spawn_x, spawn_y);
  return true;
}
