#include "mini_climb_endless.h"

#include <string.h>

#include "funcs.h"
#include "ida_types.h"
#include "mini_content_scope.h"
#include "mini_room_adapter.h"
#include "samus_env.h"
#include "variables.h"

enum {
  kMiniClimbSpawnX = 521,
  kMiniClimbSpawnY = 2187,
  kMiniClimbCameraX = 393,
  kMiniClimbCameraY = 2027,
  kMiniClimbWrapTriggerCameraY = 384,
  kMiniClimbWrapShiftY = 1792,
  kMiniClimbLavaEnableFrame = 3600,
  kMiniClimbLavaRisePerFrame = 0,
};

static bool g_climb_endless_active;
static int g_virtual_floors;
static bool g_lava_enabled;
static int g_lava_floor_y;

void MiniClimbEndless_SetActive(bool active) {
  g_climb_endless_active = active;
  g_virtual_floors = 0;
  g_lava_enabled = false;
  g_lava_floor_y = 0;
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
  room->camera_x = kMiniClimbCameraX;
  room->camera_y = kMiniClimbCameraY;
  room->spawn_x = kMiniClimbSpawnX;
  room->spawn_y = kMiniClimbSpawnY;
  room->initial_suit = kMiniEditorSamusSuit_Varia;
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

  if (layer1_y_pos >= kMiniClimbWrapTriggerCameraY)
    return;

  MiniClimbEndless_ShiftWorldY(kMiniClimbWrapShiftY);
  for (int player = 0; player < state->player_count; player++)
    state->players[player].samus.world_y += kMiniClimbWrapShiftY;
  state->samus.world_y = state->players[0].samus.world_y;
  g_virtual_floors++;
  MiniStubs_ClampCameraToRoom();
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
