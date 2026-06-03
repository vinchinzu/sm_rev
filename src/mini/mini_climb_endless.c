#include "mini_climb_endless.h"

#include <string.h>

#include "mini_defs.h"
#include "mini_editor_camera.h"
#include "mini_rom_bootstrap.h"
#include "mini_run_mode.h"
#include "mini_world_shift.h"
#include "samus_env.h"
#include "variables.h"

enum {
  kMiniClimbFloorBlocksFromBottom = 6,
  kMiniClimbSpawnFootClearance = 16,
  kMiniClimbDownScroller = 160,
  // Wrap into authored 16-row bands so collision, foreground, and BG2 stay
  // aligned while the ascent cycles through more of the original room.
  kMiniClimbWrapTriggerCameraY = 32 * kMiniBlockSize,
  kMiniClimbFallbackWrapShiftY = 16 * kMiniBlockSize,
  kMiniClimbMaxWrapsPerFrame = 8,
  kMiniClimbLavaEnableFrame = 3600,
  kMiniClimbLavaRisePerFrame = 0,
};

static const uint8 kMiniClimbWrapTargetRows[] = {
  48, 96, 64, 112, 80, 96, 112, 64,
};

static int g_virtual_floors;
static bool g_lava_enabled;
static int g_lava_floor_y;

static void MiniClimbEndless_ResetProgress(void) {
  g_virtual_floors = 0;
  g_lava_enabled = false;
  g_lava_floor_y = 0;
}

void MiniClimbEndless_SetActive(bool active) {
  MiniRunMode_Set(active ? kMiniRunMode_ClimbEndless : kMiniRunMode_LandingSite);
  MiniClimbEndless_ResetProgress();
}

bool MiniClimbEndless_IsActive(void) {
  return MiniRunMode_IsClimbEndless();
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
  room->initial_suit = kMiniEditorSamusSuit_Power;
  room->camera_target_y_percent = (kMiniClimbDownScroller * 100) / kMiniGameHeight;
}

void MiniClimbEndless_InitAfterRoom(MiniRoomInfo *room) {
  if (!MiniRunMode_IsClimbEndless() || room == NULL)
    return;

  room->has_original_enemies = false;
  room->has_original_plms = false;
  room->uses_original_gameplay_runtime = false;
}

void MiniClimbEndless_ApplySamusLoadout(void) {
  MiniRomBootstrap_ApplyPowerBeamLoadout(kSamusEquip_MorphBall, kSamusSuitPalette_Power);
}

static void MiniClimbEndless_ShiftMiniStateY(MiniGameState *state, int shift_y) {
  for (int player = 0; player < state->player_count; player++)
    state->players[player].samus.world_y += shift_y;
  for (int i = 0; i < state->projectile_state.count; i++)
    state->projectile_state.views[i].y_pos += shift_y;
  for (int i = 0; i < state->projectile_count; i++)
    state->projectiles[i].y_pos += shift_y;
  for (int i = 0; i < state->enemy_state.count; i++) {
    state->enemy_state.enemies[i].y += shift_y;
    state->enemy_state.enemies[i].home_y += shift_y;
  }
  for (int i = 0; i < kMiniEnemyShotCapacity; i++) {
    if (state->enemy_state.shots[i].active)
      state->enemy_state.shots[i].y += shift_y;
  }
}

static void MiniClimbEndless_SyncStateAfterWrap(MiniGameState *state) {
  state->viewport.camera_x = layer1_x_pos;
  state->viewport.camera_y = layer1_y_pos;
  state->camera_x = state->viewport.camera_x;
  state->camera_y = state->viewport.camera_y;
  for (int player = 0; player < state->player_count; player++) {
    MiniSamusCoreState *samus = &state->players[player].samus;
    samus->screen_x = samus->world_x - state->viewport.camera_x - samus->x_radius;
    samus->screen_y = samus->world_y - state->viewport.camera_y - samus->y_radius;
  }
  state->samus = state->players[0].samus;
  state->samus_x = state->samus.screen_x;
  state->samus_y = state->samus.screen_y;
}

static int MiniClimbEndless_NextWrapShiftY(void) {
  int target_count = (int)(sizeof(kMiniClimbWrapTargetRows) / sizeof(kMiniClimbWrapTargetRows[0]));
  int difficulty_ramp = g_virtual_floors / 4;
  int target_index = (g_virtual_floors * 5 + difficulty_ramp * 3) % target_count;
  if (difficulty_ramp >= 2 && (g_virtual_floors & 3) == 0)
    target_index = 6;

  int target_y = kMiniClimbWrapTargetRows[target_index] * kMiniBlockSize;
  int shift_y = target_y - (int)layer1_y_pos;
  return shift_y > 0 ? shift_y : kMiniClimbFallbackWrapShiftY;
}

void MiniClimbEndless_Tick(MiniGameState *state) {
  if (!MiniRunMode_IsClimbEndless() || state == NULL)
    return;

  MiniEditorCamera_Follow(state);

  if (!g_lava_enabled && state->frame >= kMiniClimbLavaEnableFrame) {
    g_lava_enabled = true;
    g_lava_floor_y = state->room.room_bottom - 64;
  }
  if (g_lava_enabled && kMiniClimbLavaRisePerFrame > 0)
    g_lava_floor_y -= kMiniClimbLavaRisePerFrame;

  if ((int)layer1_y_pos >= kMiniClimbWrapTriggerCameraY)
    return;

  int wraps = 0;
  while ((int)layer1_y_pos < kMiniClimbWrapTriggerCameraY &&
         wraps < kMiniClimbMaxWrapsPerFrame) {
    int shift_y = MiniClimbEndless_NextWrapShiftY();
    MiniWorldShift_ApplyY(shift_y);
    MiniClimbEndless_ShiftMiniStateY(state, shift_y);
    wraps++;
  }
  state->samus.world_y = state->players[0].samus.world_y;
  g_virtual_floors += wraps;
  CalculateLayer2Xpos();
  CalculateLayer2Ypos();
  CalculateBgScrolls();
  MiniClimbEndless_SyncStateAfterWrap(state);
}

void MiniClimbEndless_SaveSnapshot(MiniClimbModeSnapshot *snapshot) {
  snapshot->virtual_floors = g_virtual_floors;
  snapshot->lava_enabled = g_lava_enabled;
  snapshot->lava_floor_y = g_lava_floor_y;
}

void MiniClimbEndless_LoadSnapshot(const MiniClimbModeSnapshot *snapshot) {
  g_virtual_floors = snapshot->virtual_floors;
  g_lava_enabled = snapshot->lava_enabled;
  g_lava_floor_y = snapshot->lava_floor_y;
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
