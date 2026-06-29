#include "mini_climb_endless.h"

#include <stdlib.h>
#include <string.h>

#include "funcs.h"
#include "ida_types.h"
#include "mini_defs.h"
#include "mini_editor_camera.h"
#include "mini_rom_bootstrap.h"
#include "mini_run_mode.h"
#include "mini_world_shift.h"
#include "samus_env.h"
#include "sm_rtl.h"
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
  // Lava pacing: a short grace window each run, then a rise that ramps with
  // every virtual floor. Speeds are Q8 pixels per frame.
  kMiniClimbLavaGraceFrames = 600,
  kMiniClimbLavaStartOffsetFromBottom = 16,
  kMiniClimbLavaBaseSpeedQ8 = 56,
  kMiniClimbLavaSpeedRampQ8 = 20,
  kMiniClimbLavaMaxSpeedQ8 = 480,
  // Keep the lava threatening: it never trails Samus by more than this.
  kMiniClimbLavaMaxGapBelowSamus = 360,
  kMiniClimbLavaDamagePeriod = 8,
  kMiniClimbLavaDamageBase = 6,
  kMiniClimbFloorsPerTier = 4,
  kMiniClimbMaxDifficultyTier = 4,
  kMiniClimbPirateBaseCooldown = 64,
  kMiniClimbPirateCooldownStepPerTier = 8,
  kMiniClimbPirateMinCooldown = 28,
};

// Authored shaft bands ordered easy -> hard; the high rows are the sparse,
// demanding platform layouts.
static const uint8 kMiniClimbWrapBandRows[] = {48, 64, 80, 96, 112};

void MiniClimbEndless_SetActive(bool active) {
  MiniRunMode_Set(active ? kMiniRunMode_ClimbEndless : kMiniRunMode_LandingSite);
}

bool MiniClimbEndless_IsActive(void) {
  return MiniRunMode_IsClimbEndless();
}

const char *MiniClimbEndless_DefaultRoomExportPath(void) {
  return "assets/local_mini/room_96BA.json";
}

void MiniClimbEndless_ApplySpawnDefaults(MiniRoomInfo *room) {
  if (room == NULL ||
      (room->room_id != kMiniClimbEndlessRoomId && strcmp(room->room_handle, "climb") != 0))
    return;

  int room_width_px = room->room_width_blocks * kMiniBlockSize;
  int floor_block_y = room->room_height_blocks - kMiniClimbFloorBlocksFromBottom;
  if (floor_block_y < 0)
    floor_block_y = 0;

  room->spawn_x = room_width_px / 2;
  room->spawn_y = floor_block_y * kMiniBlockSize - kMiniClimbSpawnFootClearance;
  room->camera_x = room->spawn_x - (kMiniGameWidth * room->camera_target_x_percent / 100);
  if (room->camera_target_x_percent <= 0)
    room->camera_x = room->spawn_x - kMiniGameWidth / 2;
  room->camera_y = room->spawn_y - kMiniClimbDownScroller;
  room->samus_suit = kMiniSamusSuit_Power;
  room->camera_target_y_percent = (kMiniClimbDownScroller * 100) / kMiniGameHeight;
}

static void MiniClimbEndless_ResetRunProgress(MiniGameState *state, int spawn_y) {
  int deaths = state->climb.deaths;
  int best_ascent = state->climb.best_ascent_pixels;
  state->climb = (MiniClimbState){0};
  state->climb.deaths = deaths;
  state->climb.best_ascent_pixels = best_ascent;
  state->climb.run_start_frame = state->frame;
  state->climb.lava_enable_frame = state->frame + kMiniClimbLavaGraceFrames;
  state->climb.last_samus_y = spawn_y;
  state->climb.has_score_anchor = true;
}

void MiniClimbEndless_InitAfterRoom(MiniGameState *state, MiniRoomInfo *room) {
  if (!MiniRunMode_IsClimbEndless() || state == NULL || room == NULL)
    return;

  MiniClimbEndless_ApplySpawnDefaults(room);
  state->climb = (MiniClimbState){0};
  MiniClimbEndless_ResetRunProgress(state, room->spawn_y);
}

void MiniClimbEndless_ApplySamusLoadout(void) {
  MiniRomBootstrap_ApplyPowerBeamLoadout(kSamusEquip_MorphBall, kSamusSuitPalette_Power);
}

// Mirror fields in MiniGameState must shift in lockstep with the globals
// touched by MiniWorldShift_ApplyY so a wrap is invisible to screen space.
static void MiniClimbEndless_ShiftMiniStateY(MiniGameState *state, int shift_y) {
  if (state->player_count < 1)
    state->player_count = 1;
  for (int player = 0; player < state->player_count; player++)
    state->players[player].samus.world_y += shift_y;
  state->players[0].samus.world_y = samus_y_pos;
  state->players[0].samus.world_x = samus_x_pos;
  if (state->climb.has_score_anchor)
    state->climb.last_samus_y += shift_y;
  if (state->climb.lava_enabled)
    state->climb.lava_floor_y += shift_y;
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

static uint32 MiniClimbEndless_Mix32(uint32 value) {
  value ^= value >> 16;
  value *= 0x7FEB352Du;
  value ^= value >> 15;
  value *= 0x846CA68Bu;
  value ^= value >> 16;
  return value;
}

int MiniClimbEndless_DifficultyTier(const MiniGameState *state) {
  int tier = state->climb.virtual_floors / kMiniClimbFloorsPerTier;
  return tier > kMiniClimbMaxDifficultyTier ? kMiniClimbMaxDifficultyTier : tier;
}

int MiniClimbEndless_LavaSpeedQ8(const MiniGameState *state) {
  if (!state->climb.lava_enabled)
    return 0;
  int speed_q8 = kMiniClimbLavaBaseSpeedQ8 +
                 state->climb.virtual_floors * kMiniClimbLavaSpeedRampQ8;
  return speed_q8 > kMiniClimbLavaMaxSpeedQ8 ? kMiniClimbLavaMaxSpeedQ8 : speed_q8;
}

int MiniClimbEndless_NextWrapTargetRow(const MiniGameState *state) {
  int band_count = (int)(sizeof(kMiniClimbWrapBandRows) / sizeof(kMiniClimbWrapBandRows[0]));
  // Deterministic per-floor shuffle whose window slides toward the harder
  // high bands as the lava speed tier rises, while always keeping at least
  // three bands in rotation so floors stay varied.
  int tier = MiniClimbEndless_DifficultyTier(state);
  int min_band = tier;
  if (min_band > band_count - 3)
    min_band = band_count - 3;
  uint32 roll = MiniClimbEndless_Mix32(
      (uint32)state->climb.virtual_floors + 0x9E3779B9u * (uint32)(tier + 1));
  int window = band_count - min_band;
  return kMiniClimbWrapBandRows[min_band + (int)(roll % (uint32)window)];
}

static int MiniClimbEndless_NextWrapShiftY(const MiniGameState *state) {
  int target_y = MiniClimbEndless_NextWrapTargetRow(state) * kMiniBlockSize;
  int shift_y = target_y - (int)layer1_y_pos;
  return shift_y > 0 ? shift_y : kMiniClimbFallbackWrapShiftY;
}

static void MiniClimbEndless_TrackAscent(MiniGameState *state) {
  int samus_y = state->players[0].samus.world_y;
  if (!state->climb.has_score_anchor) {
    state->climb.last_samus_y = samus_y;
    state->climb.has_score_anchor = true;
    return;
  }

  if (samus_y < state->climb.last_samus_y)
    state->climb.ascent_pixels += state->climb.last_samus_y - samus_y;
  state->climb.last_samus_y = samus_y;
  if (state->climb.ascent_pixels > state->climb.best_ascent_pixels)
    state->climb.best_ascent_pixels = state->climb.ascent_pixels;
}

static int MiniClimbEndless_SamusFeetY(const MiniGameState *state) {
  const MiniSamusCoreState *samus = &state->players[0].samus;
  return samus->world_y + samus->y_radius;
}

bool MiniClimbEndless_SamusInLava(const MiniGameState *state) {
  return state->climb.lava_enabled &&
         MiniClimbEndless_SamusFeetY(state) >= state->climb.lava_floor_y;
}

static void MiniClimbEndless_UpdateLava(MiniGameState *state) {
  MiniClimbState *climb = &state->climb;
  if (!climb->lava_enabled) {
    if (state->frame < climb->lava_enable_frame)
      return;
    climb->lava_enabled = true;
    climb->lava_floor_y = state->room.room_bottom - kMiniClimbLavaStartOffsetFromBottom;
    climb->lava_rise_carry_q8 = 0;
    return;
  }

  climb->lava_rise_carry_q8 += MiniClimbEndless_LavaSpeedQ8(state);
  climb->lava_floor_y -= climb->lava_rise_carry_q8 >> 8;
  climb->lava_rise_carry_q8 &= 0xFF;

  int max_floor_y = MiniClimbEndless_SamusFeetY(state) + kMiniClimbLavaMaxGapBelowSamus;
  if (climb->lava_floor_y > max_floor_y)
    climb->lava_floor_y = max_floor_y;
  if (climb->lava_floor_y > state->room.room_bottom)
    climb->lava_floor_y = state->room.room_bottom;
}

static void MiniClimbEndless_DrainSamusHealth(int damage) {
  samus_health = samus_health > damage ? (uint16)(samus_health - damage) : 0;
}

static void MiniClimbEndless_ApplyHazardDamage(MiniGameState *state) {
  // The original gameplay runtime applies enemy damage itself; the mini-sim
  // path only records it as pending combat damage, so settle it here.
  MiniPlayerCombatState *combat = &state->players[0].combat;
  if (!state->room.uses_original_gameplay_runtime && combat->pending_damage != 0) {
    MiniClimbEndless_DrainSamusHealth(combat->pending_damage);
    combat->pending_damage = 0;
  }

  if (state->climb.lava_damage_cooldown > 0)
    state->climb.lava_damage_cooldown--;
  if (!MiniClimbEndless_SamusInLava(state) || state->climb.lava_damage_cooldown > 0)
    return;
  MiniClimbEndless_DrainSamusHealth(kMiniClimbLavaDamageBase +
                                    MiniClimbEndless_DifficultyTier(state));
  state->climb.lava_damage_cooldown = kMiniClimbLavaDamagePeriod;
}

// A death restarts the run from the bottom platform: score and lava reset,
// the deaths counter and session-best ascent persist.
static void MiniClimbEndless_RestartRunAfterDeath(MiniGameState *state) {
  state->climb.deaths++;
  MiniClimbEndless_ResetRunProgress(state, state->room.spawn_y);

  game_state = kGameState_8_MainGameplay;
  time_is_frozen_flag = 0;
  samus_health = samus_max_health;
  samus_input_handler = FUNC16(Samus_InputHandler_E913);
  samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
  samus_draw_handler = FUNC16(SamusDrawHandler_Default);
  samus_x_pos = (uint16)state->room.spawn_x;
  samus_y_pos = (uint16)state->room.spawn_y;
  samus_prev_x_pos = samus_x_pos;
  samus_prev_y_pos = samus_y_pos;
  samus_y_speed = 0;
  samus_y_subspeed = 0;
  samus_knockback_timer = 0;
  samus_pose = kPose_01_FaceR_Normal;
  samus_movement_type = kMovementType_00_Standing;
  SamusFunc_F433();
  Samus_SetRadius();
  Samus_SetAnimationFrameIfPoseChanged();
  Samus_UpdatePreviousPose();
  CallSomeSamusCode(1);

  layer1_x_pos = (uint16)state->room.camera_x;
  layer1_y_pos = (uint16)state->room.camera_y;
  ideal_layer1_ypos = layer1_y_pos;
  CalculateLayer2Xpos();
  CalculateLayer2Ypos();
  CalculateBgScrolls();

  state->players[0].samus.world_x = samus_x_pos;
  state->players[0].samus.world_y = samus_y_pos;
  state->players[0].samus.x_velocity = 0;
  state->players[0].samus.y_velocity = 0;
  state->players[0].combat = (MiniPlayerCombatState){0};
  memset(state->enemy_state.shots, 0, sizeof(state->enemy_state.shots));
  MiniClimbEndless_SyncStateAfterWrap(state);
}

static void MiniClimbEndless_HandleSamusDown(MiniGameState *state) {
  // game_state leaving main gameplay means the original runtime started its
  // death sequence; fold both cases into the climb's own restart loop.
  if (samus_health != 0 && game_state == kGameState_8_MainGameplay)
    return;
  MiniClimbEndless_RestartRunAfterDeath(state);
}

void MiniClimbEndless_Tick(MiniGameState *state) {
  if (!MiniRunMode_IsClimbEndless() || state == NULL)
    return;

  MiniEditorCamera_Follow(state);
  MiniClimbEndless_TrackAscent(state);
  MiniClimbEndless_UpdateLava(state);
  MiniClimbEndless_ApplyHazardDamage(state);
  MiniClimbEndless_HandleSamusDown(state);

  if ((int)layer1_y_pos >= kMiniClimbWrapTriggerCameraY)
    return;

  int wraps = 0;
  while ((int)layer1_y_pos < kMiniClimbWrapTriggerCameraY &&
         wraps < kMiniClimbMaxWrapsPerFrame) {
    int shift_y = MiniClimbEndless_NextWrapShiftY(state);
    MiniWorldShift_ApplyY(shift_y);
    MiniClimbEndless_ShiftMiniStateY(state, shift_y);
    state->climb.virtual_floors++;
    wraps++;
  }
  state->samus.world_y = state->players[0].samus.world_y;
  CalculateLayer2Xpos();
  CalculateLayer2Ypos();
  CalculateBgScrolls();
  MiniClimbEndless_SyncStateAfterWrap(state);
}

int MiniClimbEndless_VirtualFloors(const MiniGameState *state) {
  return state->climb.virtual_floors;
}

bool MiniClimbEndless_LavaEnabled(const MiniGameState *state) {
  return state->climb.lava_enabled;
}

int MiniClimbEndless_LavaFloorY(const MiniGameState *state) {
  return state->climb.lava_floor_y;
}

int MiniClimbEndless_AscentPixels(const MiniGameState *state) {
  return state->climb.ascent_pixels;
}

int MiniClimbEndless_BestAscentPixels(const MiniGameState *state) {
  return state->climb.best_ascent_pixels;
}

int MiniClimbEndless_Deaths(const MiniGameState *state) {
  return state->climb.deaths;
}

int MiniClimbEndless_RunFrames(const MiniGameState *state) {
  int run_frames = state->frame - state->climb.run_start_frame;
  return run_frames > 0 ? run_frames : 0;
}

int MiniClimbEndless_PirateShotCooldownFrames(const MiniGameState *state) {
  if (!MiniRunMode_IsClimbEndless())
    return kMiniClimbPirateBaseCooldown;
  int cooldown = kMiniClimbPirateBaseCooldown -
                 MiniClimbEndless_DifficultyTier(state) * kMiniClimbPirateCooldownStepPerTier;
  return cooldown < kMiniClimbPirateMinCooldown ? kMiniClimbPirateMinCooldown : cooldown;
}
