#include "mini_climb_endless.h"

#include <string.h>

#include "funcs.h"
#include "ida_types.h"
#include "mini_content_scope.h"
#include "mini_defs.h"
#include "mini_room_adapter.h"
#include "samus_env.h"
#include "variables.h"

enum {
  kMiniClimbFloorBlocksFromBottom = 6,
  kMiniClimbSpawnFootClearance = 16,
  kMiniClimbDownScroller = 160,
  kMiniClimbWrapTriggerCameraY = 384,
  kMiniClimbWrapShiftY = 1792,
  kMiniClimbLavaEnableFrame = 3600,
  kMiniClimbLavaRisePerFrame = 0,
};

static bool g_climb_endless_active;
static int g_virtual_floors;
static bool g_lava_enabled;
static int g_lava_floor_y;
static int g_wrap_target_camera_y;

void MiniClimbEndless_SetActive(bool active) {
  g_climb_endless_active = active;
  g_virtual_floors = 0;
  g_lava_enabled = false;
  g_lava_floor_y = 0;
  g_wrap_target_camera_y = 0;
  MiniContentScope_SetClimbEndlessMode(active);
}

bool MiniClimbEndless_IsActive(void) {
  return g_climb_endless_active;
}

const char *MiniClimbEndless_DefaultRoomExportPath(void) {
  return "assets/local_mini/room_96BA.json";
}

void MiniClimbEndless_AssignRoomDefaults(MiniEditorRoom *room) {
  if (room->room_id != kMiniClimbEndlessRoomId && strcmp(room->handle, "climb") != 0)
    return;

  int room_width_px = room->width_blocks * kMiniBlockSize;
  int floor_block_y = room->height_blocks - kMiniClimbFloorBlocksFromBottom;
  if (floor_block_y < 0)
    floor_block_y = 0;

  room->spawn_x = room_width_px / 2;
  room->spawn_y = floor_block_y * kMiniBlockSize - kMiniClimbSpawnFootClearance;
  room->camera_x = room->spawn_x - (kMiniGameWidth * room->camera_target_x_percent / 100);
  if (room->camera_target_x_percent <= 0)
    room->camera_x = room->spawn_x - kMiniGameWidth / 2;
  room->camera_y = room->spawn_y - kMiniClimbDownScroller;
  room->initial_suit = kMiniEditorSamusSuit_Varia;
  room->camera_target_y_percent = (kMiniClimbDownScroller * 100) / kMiniGameHeight;
  g_wrap_target_camera_y = room->camera_y;
}

void MiniClimbEndless_ApplySamusLoadout(void) {
  const uint16 climb_items = kSamusEquip_MorphBall | kSamusEquip_VariaSuit | kSamusEquip_GravitySuit;
  equipped_items = climb_items;
  collected_items = climb_items;
  equipped_beams = 0;
  collected_beams = 0;
  hyper_beam_flag = 0;
  samus_suit_palette_index = kSamusSuitPalette_Varia;

  if (!samus_max_health)
    samus_max_health = 99;
  samus_health = samus_max_health;
  samus_max_reserve_health = 0;
  samus_reserve_health = 0;
  reserve_health_mode = 0;
}

static void MiniClimbEndless_ShiftWorldY(int shift_y) {
  layer1_y_pos = (uint16)(layer1_y_pos + shift_y);
  ideal_layer1_ypos = (uint16)(ideal_layer1_ypos + shift_y);
  samus_y_pos = (uint16)(samus_y_pos + shift_y);
  samus_prev_y_pos = samus_y_pos;
}

void MiniClimbEndless_Tick(MiniGameState *state) {
  if (!g_climb_endless_active || state == NULL)
    return;

  if (!g_lava_enabled && state->frame >= kMiniClimbLavaEnableFrame) {
    g_lava_enabled = true;
    g_lava_floor_y = state->room.room_bottom - 64;
  }
  if (g_lava_enabled && kMiniClimbLavaRisePerFrame > 0)
    g_lava_floor_y -= kMiniClimbLavaRisePerFrame;

  if ((int)layer1_y_pos >= kMiniClimbWrapTriggerCameraY)
    return;

  int shift_y = g_wrap_target_camera_y - (int)layer1_y_pos;
  if (shift_y <= 0)
    shift_y = kMiniClimbWrapShiftY;

  MiniClimbEndless_ShiftWorldY(shift_y);
  for (int player = 0; player < state->player_count; player++)
    state->players[player].samus.world_y += shift_y;
  state->samus.world_y = state->players[0].samus.world_y;
  g_virtual_floors++;
  CalculateLayer2Xpos();
  CalculateLayer2Ypos();
  CalculateBgScrolls();
}

int MiniClimbEndless_VirtualFloors(void) {
  return g_virtual_floors;
}

bool MiniClimbEndless_LavaEnabled(void) {
  return g_lava_enabled;
}

int MiniClimbEndless_LavaFloorY(void) {
  return g_lava_floor_y;
}
