#include "mini_game.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "funcs.h"
#include "gameplay_frame.h"
#include "ida_types.h"
#include "mini_audio.h"
#include "mini_asset_bootstrap.h"
#include "mini_authored_movement.h"
#include "mini_climb_endless.h"
#include "mini_editor_camera.h"
#include "mini_ppu_stub.h"
#include "mini_run_mode.h"
#include "mini_system.h"
#include "multi_samus.h"
#include "physics_config.h"
#include "samus_projectile.h"
#include "samus_projectile_view.h"
#include "sm_rtl.h"
#include "variables.h"

enum {
  kMiniItem_VariaSuit = 1,
  kMiniItem_GravitySuit = 0x20,
  kMiniSnapshotMagic = 0x4D53534D,
  kMiniSnapshotVersion = 8,
  kMiniRamSnapshotSize = 0x20000,
  kMiniSramSnapshotSize = 0x2000,
  kMiniPlayerRuntimePreProjectileOffset = 0xA94,
  kMiniPlayerRuntimePostProjectileOffset = 0xCCC,
  kMiniPlayerTwoSpawnOffsetX = 48,
};

typedef struct MiniStateSnapshot {
  uint32 magic;
  uint32 version;
  MiniGameState game;
  MiniStubsSnapshot stubs;
  MiniPpuSnapshot ppu;
  MiniRunMode run_mode;
  MiniClimbModeSnapshot climb_mode;
  uint8 ram[kMiniRamSnapshotSize];
  uint8 sram[kMiniSramSnapshotSize];
  bool use_my_apu_code;
  bool host_debug_flag;
  int snes_frame_counter;
  uint16 installed_bug_fix_counter;
} MiniStateSnapshot;

static uint64_t MiniHashBytes(uint64_t hash, const void *data, size_t size) {
  static const uint64_t kFnvPrime = UINT64_C(1099511628211);
  const uint8 *bytes = (const uint8 *)data;
  for (size_t i = 0; i < size; i++) {
    hash ^= bytes[i];
    hash *= kFnvPrime;
  }
  return hash;
}

static uint64_t MiniHashInt(uint64_t hash, int value) {
  int32_t normalized = value;
  return MiniHashBytes(hash, &normalized, sizeof(normalized));
}

static uint64_t MiniHashUInt16(uint64_t hash, uint16 value) {
  return MiniHashBytes(hash, &value, sizeof(value));
}

static uint64_t MiniHashByte(uint64_t hash, uint8 value) {
  return MiniHashBytes(hash, &value, sizeof(value));
}

static uint64_t MiniHashBool(uint64_t hash, bool value) {
  uint8 normalized = value ? 1 : 0;
  return MiniHashBytes(hash, &normalized, sizeof(normalized));
}

static int MiniClampInt(int value, int min_value, int max_value) {
  if (value < min_value)
    return min_value;
  if (value > max_value)
    return max_value;
  return value;
}

static void MiniInitializeEnemies(MiniGameState *state);
static void MiniUpdateEnemies(MiniGameState *state);

static MiniRoomState MiniRoomState_FromInfo(const MiniRoomInfo *room) {
  MiniRoomState state = {
    .has_room = room->has_room,
    .uses_rom_room = room->uses_rom_room,
    .booted_from_save_slot = room->booted_from_save_slot,
    .has_editor_room_visuals = room->has_editor_room_visuals,
    .uses_original_gameplay_runtime = room->uses_original_gameplay_runtime,
    .has_original_enemies = room->has_original_enemies,
    .has_original_plms = room->has_original_plms,
    .samus_suit = room->samus_suit,
    .room_id = room->room_id,
    .room_source = room->room_source,
    .room_left = room->room_left,
    .room_top = room->room_top,
    .room_right = room->room_right,
    .room_bottom = room->room_bottom,
    .room_width_blocks = room->room_width_blocks,
    .room_height_blocks = room->room_height_blocks,
    .camera_x = room->camera_x,
    .camera_y = room->camera_y,
    .spawn_x = room->spawn_x,
    .spawn_y = room->spawn_y,
    .camera_target_x_percent = room->camera_target_x_percent,
    .camera_target_y_percent = room->camera_target_y_percent,
    .doorway_count = room->doorway_count,
  };
  memcpy(state.room_handle, room->room_handle, sizeof(state.room_handle));
  memcpy(state.room_name, room->room_name, sizeof(state.room_name));
  memcpy(state.doorways, room->doorways, sizeof(state.doorways));
  return state;
}

static int MiniNormalizePlayerCount(int player_count) {
  if (player_count < 1)
    return 1;
  if (player_count > kMiniMaxPlayers)
    return kMiniMaxPlayers;
  return player_count;
}

static uint16 MiniInputButtonsForPlayer(const MiniInputState *input, int player_index) {
  if (player_index == 0 && input->buttons != 0)
    return input->buttons;
  return input->player_buttons[player_index];
}

static MiniSamusCoreState MiniSamusCoreFromGlobals(const MiniGameState *state) {
  return (MiniSamusCoreState){
    .world_x = samus_x_pos,
    .world_y = samus_y_pos,
    .x_velocity = state->samus.x_velocity,
    .y_velocity = state->samus.y_velocity,
    .screen_x = samus_x_pos - state->viewport.camera_x - samus_x_radius,
    .screen_y = samus_y_pos - state->viewport.camera_y - samus_y_radius,
    .x_radius = samus_x_radius,
    .y_radius = samus_y_radius,
    .pose = samus_pose,
    .movement_type = samus_movement_type,
    .suit = state->room.samus_suit,
    .on_ground = state->samus.on_ground,
  };
}

static MiniSamusCoreState MiniSamusCoreWithScreen(const MiniGameState *state,
                                                  MiniSamusCoreState samus) {
  samus.screen_x = samus.world_x - state->viewport.camera_x - samus.x_radius;
  samus.screen_y = samus.world_y - state->viewport.camera_y - samus.y_radius;
  samus.suit = state->room.samus_suit;
  return samus;
}

static void MiniSavePlayerRuntime(MiniGameState *state, int player_index) {
  memcpy(state->player_runtime_pre_projectile[player_index],
         g_ram + kMiniPlayerRuntimePreProjectileOffset,
         kMiniPlayerRuntimePreProjectileSize);
  memcpy(state->player_runtime_post_projectile[player_index],
         g_ram + kMiniPlayerRuntimePostProjectileOffset,
         kMiniPlayerRuntimePostProjectileSize);
}

static void MiniLoadPlayerRuntime(const MiniGameState *state, int player_index) {
  memcpy(g_ram + kMiniPlayerRuntimePreProjectileOffset,
         state->player_runtime_pre_projectile[player_index],
         kMiniPlayerRuntimePreProjectileSize);
  memcpy(g_ram + kMiniPlayerRuntimePostProjectileOffset,
         state->player_runtime_post_projectile[player_index],
         kMiniPlayerRuntimePostProjectileSize);
}

static void MiniCopySamusCoreToGlobals(const MiniSamusCoreState *samus) {
  samus_x_pos = (uint16)samus->world_x;
  samus_y_pos = (uint16)samus->world_y;
  samus_prev_x_pos = samus_x_pos;
  samus_prev_y_pos = samus_y_pos;
  samus_x_radius = samus->x_radius;
  samus_y_radius = samus->y_radius;
  samus_pose = samus->pose;
  samus_movement_type = samus->movement_type;
}

static void MiniUpdatePlayerScreenPositions(MiniGameState *state) {
  for (int i = 0; i < kMiniMaxPlayers; i++)
    state->players[i].samus = MiniSamusCoreWithScreen(state, state->players[i].samus);
  state->samus = state->players[0].samus;
}

static void MiniSyncPlayersFromOriginalRuntime(MiniGameState *state) {
  int active_players = state->player_count;
  if (active_players > MultiSamus_GetNumSamus())
    active_players = MultiSamus_GetNumSamus();
  if (active_players > kMiniMaxPlayers)
    active_players = kMiniMaxPlayers;
  if (active_players < 1)
    active_players = 1;

  for (int i = 0; i < active_players; i++) {
    MultiSamus_Switch(i);
    state->players[i].samus = MiniSamusCoreFromGlobals(state);
    MiniSavePlayerRuntime(state, i);
  }
  MultiSamus_Switch(0);
  state->samus = state->players[0].samus;
  MiniUpdatePlayerScreenPositions(state);
}

static void MiniInitializePlayerOneFromCurrentSamus(MiniGameState *state) {
  state->players[0].samus = state->samus;
  state->players[0].combat = (MiniPlayerCombatState){0};
  MiniSavePlayerRuntime(state, 0);
}

static void MiniInitializePlayerTwoFromPlayerOne(MiniGameState *state) {
  state->players[1] = state->players[0];
  state->players[1].samus.world_x = state->players[0].samus.world_x + kMiniPlayerTwoSpawnOffsetX;
  int min_x = state->room.room_left + state->players[1].samus.x_radius;
  int max_x = state->room.room_right - state->players[1].samus.x_radius - 1;
  if (max_x >= min_x)
    state->players[1].samus.world_x = MiniClampInt(state->players[1].samus.world_x, min_x, max_x);
  state->players[1].combat = (MiniPlayerCombatState){0};

  MiniLoadPlayerRuntime(state, 0);
  MiniCopySamusCoreToGlobals(&state->players[1].samus);
  MiniSavePlayerRuntime(state, 1);
  MiniLoadPlayerRuntime(state, 0);
  MiniUpdatePlayerScreenPositions(state);
}

static void MiniSyncLegacyPublicFields(MiniGameState *state) {
  state->player_count = MiniNormalizePlayerCount(state->player_count);
  if (state->players[0].samus.x_radius != 0 || state->players[0].samus.y_radius != 0)
    state->samus = state->players[0].samus;
  state->viewport_width = state->viewport.width;
  state->viewport_height = state->viewport.height;
  state->camera_x = state->viewport.camera_x;
  state->camera_y = state->viewport.camera_y;

  state->room_id = state->room.room_id;
  state->room_width_blocks = state->room.room_width_blocks;
  state->room_height_blocks = state->room.room_height_blocks;
  state->room_left = state->room.room_left;
  state->room_top = state->room.room_top;
  state->room_right = state->room.room_right;
  state->room_bottom = state->room.room_bottom;
  state->ground_y = state->room.room_bottom;
  state->has_room = state->room.has_room;
  state->uses_rom_room = state->room.uses_rom_room;
  state->has_editor_room_visuals = state->room.has_editor_room_visuals;
  state->uses_original_gameplay_runtime = state->room.uses_original_gameplay_runtime;
  state->has_original_enemies = state->room.has_original_enemies;
  state->has_original_plms = state->room.has_original_plms;
  state->samus_suit = state->room.samus_suit;
  state->room_source = state->room.room_source;
  memcpy(state->room_handle, state->room.room_handle, sizeof(state->room_handle));
  memcpy(state->room_name, state->room.room_name, sizeof(state->room_name));

  state->samus_x = state->samus.screen_x;
  state->samus_y = state->samus.screen_y;
  state->samus_pose_value = state->samus.pose;
  state->samus_movement_type_value = state->samus.movement_type;

  state->last_buttons = state->controls.buttons;
  state->quit_requested = state->controls.quit_requested;
  state->projectile_count = state->projectile_state.count;
  memcpy(state->projectiles, state->projectile_state.views, sizeof(state->projectiles));
}

static void MiniRefreshProjectileState(MiniGameState *state) {
  state->projectile_state.count = SamusProjectile_GetActiveViews(
      state->projectile_state.views, kMiniProjectileViewCapacity);
}

static void MiniSyncRenderState(MiniGameState *state) {
  int x_velocity = state->samus.x_velocity;
  int y_velocity = state->samus.y_velocity;
  bool on_ground = state->samus.on_ground;
  state->viewport.camera_x = layer1_x_pos;
  state->viewport.camera_y = layer1_y_pos;
  state->samus.x_velocity = x_velocity;
  state->samus.y_velocity = y_velocity;
  state->samus.on_ground = on_ground;
  state->samus = MiniSamusCoreFromGlobals(state);
  if (state->room.uses_original_gameplay_runtime)
    MiniSyncPlayersFromOriginalRuntime(state);
  else {
    state->players[0].samus = state->samus;
    MiniUpdatePlayerScreenPositions(state);
  }
  MiniRefreshProjectileState(state);
  MiniSyncLegacyPublicFields(state);
}

static void MiniUpdateButtons(MiniGameState *state, const MiniInputState *input) {
  int input_player_count = input->player_count != 0 ? input->player_count : state->player_count;
  int old_player_count = state->player_count;
  state->player_count = MiniNormalizePlayerCount(input_player_count);
  if (state->player_count != old_player_count)
    MultiSamus_SetNumSamus(state->player_count);
  for (int i = 0; i < kMiniMaxPlayers; i++) {
    uint16 buttons = i < state->player_count ? MiniInputButtonsForPlayer(input, i) : 0;
    state->player_inputs[i].previous_buttons = state->player_inputs[i].buttons;
    state->player_inputs[i].buttons = buttons;
    state->player_inputs[i].new_buttons =
        buttons & (state->player_inputs[i].previous_buttons ^ buttons);
  }

  state->controls.previous_buttons = state->player_inputs[0].previous_buttons;
  state->controls.buttons = state->player_inputs[0].buttons;
  joypad1_prev = state->controls.previous_buttons;
  joypad1_lastkeys = state->controls.buttons;
  joypad1_newkeys = state->player_inputs[0].new_buttons;
  joypad1_newkeys2_UNUSED = joypad1_newkeys;
  joypad2_prev = state->player_inputs[1].previous_buttons;
  joypad2_last = state->player_inputs[1].buttons;
  joypad2_new_keys = state->player_inputs[1].new_buttons;
  joypad2_newkeys2 = joypad2_new_keys;
  state->controls.new_buttons = joypad1_newkeys;
  state->last_buttons = state->controls.buttons;
}

static void MiniApplyPlayerJoypadState(const MiniGameState *state, int player_index) {
  joypad1_prev = state->player_inputs[player_index].previous_buttons;
  joypad1_lastkeys = state->player_inputs[player_index].buttons;
  joypad1_newkeys = state->player_inputs[player_index].new_buttons;
  joypad1_newkeys2_UNUSED = joypad1_newkeys;
}

static uint16 MiniInitialPoseForRoom(const MiniRoomInfo *room) {
  if (!room->uses_rom_room && !room->has_editor_room_visuals)
    return kPose_01_FaceR_Normal;
  if (room->room_id != 0x91F8)
    return kPose_01_FaceR_Normal;
  return (equipped_items & (kMiniItem_GravitySuit | kMiniItem_VariaSuit)) != 0
             ? kPose_9B_FaceF_VariaGravitySuit
             : kPose_00_FaceF_Powersuit;
}

static bool MiniPoseIsFaceForward(uint16 pose) {
  return pose == kPose_00_FaceF_Powersuit ||
         pose == kPose_9B_FaceF_VariaGravitySuit;
}

static void MiniRefreshSamusRuntimePose(void) {
  SamusFunc_F433();
  Samus_SetRadius();
  Samus_SetAnimationFrameIfPoseChanged();
  Samus_UpdatePreviousPose();
  CallSomeSamusCode(1);
}

static void MiniInitializeSamusRuntime(const MiniRoomInfo *room) {
  if (!samus_max_health)
    samus_max_health = 99;
  if (!samus_health)
    samus_health = samus_max_health;

  game_state = kGameState_8_MainGameplay;
  debug_disable_minimap = room->uses_rom_room ? 0 : 1;
  time_is_frozen_flag = 0;
  elevator_status = 0;
  samus_input_handler = FUNC16(Samus_InputHandler_E913);
  samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
  samus_draw_handler = FUNC16(SamusDrawHandler_Default);

  Samus_Initialize();
  if (MiniRunMode_IsClimbEndless())
    MiniClimbEndless_ApplySamusLoadout();
  samus_pose = MiniInitialPoseForRoom(room);
  samus_movement_type = kMovementType_00_Standing;
  samus_anim_frame_skip = 0;
  SamusFunc_F433();
  Samus_SetRadius();
  Samus_SetAnimationFrameIfPoseChanged();
  Samus_UpdatePreviousPose();
  CallSomeSamusCode(1);
}

static void MiniSavePlayerCoreIntoRuntime(MiniGameState *state, int player_index) {
  MiniLoadPlayerRuntime(state, player_index);
  MiniCopySamusCoreToGlobals(&state->players[player_index].samus);
  MiniRefreshSamusRuntimePose();
  state->players[player_index].samus = MiniSamusCoreFromGlobals(state);
  MiniSavePlayerRuntime(state, player_index);
}

static void MiniFaceMultiplayerPlayersAtEachOther(MiniGameState *state) {
  if (state->player_count < 2)
    return;

  bool changed = false;
  if (MiniPoseIsFaceForward(state->players[0].samus.pose)) {
    state->players[0].samus.pose = kPose_01_FaceR_Normal;
    changed = true;
  }
  if (MiniPoseIsFaceForward(state->players[1].samus.pose)) {
    state->players[1].samus.pose = kPose_02_FaceL_Normal;
    changed = true;
  }
  if (!changed)
    return;

  for (int player = 0; player < state->player_count; player++) {
    MultiSamus_Switch(player);
    MiniSavePlayerCoreIntoRuntime(state, player);
  }
  MultiSamus_Switch(0);
  MiniLoadPlayerRuntime(state, 0);
  state->samus = state->players[0].samus;
  MiniUpdatePlayerScreenPositions(state);
}

void MiniGameState_Init(MiniGameState *state, int viewport_width, int viewport_height) {
  MiniRoomInfo room;

  memset(state, 0, sizeof(*state));
  MiniSystem_Reset();
  MiniStubs_ConfigureWorld(viewport_width, viewport_height);
  MiniStubs_GetRoomInfo(&room);
  if (MiniRunMode_IsClimbEndless()) {
    MiniClimbEndless_InitAfterRoom(&room);
    MiniStubs_UpdateRoomInfo(&room);
  }

  state->frame = 0;
  state->viewport = (MiniViewportState){
    .width = viewport_width,
    .height = viewport_height,
    .camera_x = room.camera_x,
    .camera_y = room.camera_y,
  };
  state->room = MiniRoomState_FromInfo(&room);
  MiniStubs_GetCollisionMapView(&state->collision_map);
  state->original_oam_next_ptr = 0;
  state->controls = (MiniControlState){0};
  state->player_count = 1;
  state->samus.suit = room.samus_suit;
  MiniSyncLegacyPublicFields(state);

  button_config_left = kButton_Left;
  button_config_right = kButton_Right;
  button_config_jump_a = kButton_A;
  button_config_run_b = kButton_B;
  button_config_shoot_x = kButton_X;
  button_config_itemcancel_y = kButton_Y;
  button_config_aim_down_L = kButton_L;
  button_config_aim_up_R = kButton_R;

  LoadPhysicsConfig();
  MiniInitializeSamusRuntime(&room);
  MiniAudio_BootRoomMusic();
  EnablePaletteFx();
  EnableHdmaObjects();
  EnableAnimtiles();
  SetLiquidPhysicsType();
  samus_x_pos = room.spawn_x;
  samus_y_pos = room.spawn_y;
  if (MiniAuthoredMovement_ShouldUseRoom(&room))
    MiniAuthoredMovement_InitializeSamusGlobals();
  samus_prev_x_pos = samus_x_pos;
  samus_prev_y_pos = samus_y_pos;
  MiniSyncRenderState(state);
  if (MiniAuthoredMovement_ShouldUseState(state)) {
    MiniAuthoredMovement_SyncGrounded(state);
    state->players[0].samus = state->samus;
  }
  MiniInitializePlayerOneFromCurrentSamus(state);
  MiniInitializePlayerTwoFromPlayerOne(state);
  MiniInitializeEnemies(state);
  MultiSamus_SetNumSamus(1);
  MiniSyncLegacyPublicFields(state);
}

static uint16 MiniStepOriginalGameplayFrame(void) {
  coroutine_state_1 = 0;
  coroutine_state_2 = 0;
  coroutine_state_3 = 0;
  coroutine_state_4 = 0;

  HdmaObjectHandler();
  NextRandom();
  ClearOamExt();
  oam_next_ptr = 0;
  nmi_copy_samus_halves = 0;
  nmi_copy_samus_top_half_src = 0;
  nmi_copy_samus_bottom_half_src = 0;

  (void)GameState_8_MainGameplay();
  HandleSoundEffects();
  uint16 original_oam_next_ptr = oam_next_ptr;
  ClearUnusedOam();

  waiting_for_nmi = 1;
  Vector_NMI();
  return original_oam_next_ptr;
}

static MiniControlState MiniControlStateForPlayer(const MiniGameState *state, int player_index) {
  return (MiniControlState){
    .buttons = state->player_inputs[player_index].buttons,
    .previous_buttons = state->player_inputs[player_index].previous_buttons,
    .new_buttons = state->player_inputs[player_index].new_buttons,
    .quit_requested = state->controls.quit_requested,
  };
}

static bool MiniProjectileSlotActive(int slot) {
  if ((unsigned)slot >= kSamusProjectileSlotCount)
    return false;
  return projectile_type[slot] != 0 ||
         projectile_damage[slot] != 0 ||
         projectile_bomb_instruction_ptr[slot] != 0;
}

static void MiniPruneInactiveProjectileOwners(MiniGameState *state) {
  for (int slot = 0; slot < kSamusProjectileSlotCount; slot++) {
    if (!MiniProjectileSlotActive(slot))
      state->projectile_state.owner_by_slot[slot] = 0;
  }
}

static int MiniProjectileOwnerScore(const SamusProjectileView *projectile,
                                    const MiniSamusCoreState *samus) {
  int projectile_x = projectile->x_pos;
  int projectile_y = projectile->y_pos;
  int score = abs(projectile_x - samus->world_x) + 2 * abs(projectile_y - samus->world_y);
  int direction = projectile->direction & 0xF;
  bool moving_right = direction == 1 || direction == 2 || direction == 3;
  bool moving_left = direction == 6 || direction == 7 || direction == 8;
  if (moving_right && samus->world_x > projectile_x)
    score += 10000;
  if (moving_left && samus->world_x < projectile_x)
    score += 10000;
  return score;
}

static void MiniAssignUnownedProjectiles(MiniGameState *state) {
  for (int i = 0; i < state->projectile_state.count; i++) {
    const SamusProjectileView *projectile = &state->projectile_state.views[i];
    if (projectile->slot_index >= kSamusProjectileSlotCount)
      continue;
    if (state->projectile_state.owner_by_slot[projectile->slot_index] != 0)
      continue;

    int best_player = -1;
    int best_score = 0;
    for (int player = 0; player < state->player_count; player++) {
      int score = MiniProjectileOwnerScore(projectile, &state->players[player].samus);
      if (best_player < 0 || score < best_score) {
        best_player = player;
        best_score = score;
      }
    }
    if (best_player >= 0)
      state->projectile_state.owner_by_slot[projectile->slot_index] = (uint8)(best_player + 1);
  }
}

static bool MiniRectsOverlap(int ax, int ay, int arx, int ary,
                             int bx, int by, int brx, int bry) {
  return abs(ax - bx) <= arx + brx && abs(ay - by) <= ary + bry;
}

static void MiniClearProjectileSlot(MiniGameState *state, uint16 slot_index);

typedef struct MiniEnemySpeciesMetadata {
  uint16 species_id;
  const char *canonical_name;
  int max_health;
  int damage;
  int x_radius;
  int y_radius;
  uint8 ai_bank;
  uint16 init_ai;
  uint16 main_ai;
  MiniEnemyBehavior behavior;
} MiniEnemySpeciesMetadata;

typedef struct MiniKnownEnemyPopulation {
  uint16 species_id;
  int x;
  int y;
  uint16 init_parameter;
  uint16 properties1;
  uint16 properties2;
  uint16 extra_parameter1;
  uint16 extra_parameter2;
} MiniKnownEnemyPopulation;

static const MiniEnemySpeciesMetadata kMiniEnemySpeciesMetadata[] = {
  {
    .species_id = kMiniEnemySpecies_Roach,
    .canonical_name = "Roach",
    .max_health = 20,
    .damage = 40,
    .x_radius = 4,
    .y_radius = 4,
    .ai_bank = 0xA3,
    .init_ai = 0xA14D,
    .main_ai = 0xA2D0,
    .behavior = kMiniEnemyBehavior_Roach,
  },
  {
    .species_id = kMiniEnemySpecies_SpacePirate,
    .canonical_name = "Space Pirate",
    .max_health = 60,
    .damage = 30,
    .x_radius = 16,
    .y_radius = 24,
    .ai_bank = 0xB2,
    .init_ai = 0xEF9F,
    .main_ai = 0xEF9F,
    .behavior = kMiniEnemyBehavior_SpacePirateShooter,
  },
};

static const MiniKnownEnemyPopulation kMiniKnownClimbEnemyPopulation[] = {
  {kMiniEnemySpecies_Roach, 276, 76, 0x0000, 0x2400, 0x0000, 0x5003, 0x0050},
  {kMiniEnemySpecies_Roach, 272, 88, 0x0000, 0x2400, 0x0000, 0x9002, 0x0050},
  {kMiniEnemySpecies_Roach, 269, 114, 0x0000, 0x2400, 0x0000, 0xAC03, 0x0050},
  {kMiniEnemySpecies_Roach, 491, 150, 0x0000, 0x2400, 0x0000, 0xC804, 0x0050},
  {kMiniEnemySpecies_Roach, 499, 154, 0x0000, 0x2400, 0x0000, 0xC303, 0x0050},
  {kMiniEnemySpecies_Roach, 277, 294, 0x0000, 0x2400, 0x0000, 0x9203, 0x0050},
  {kMiniEnemySpecies_Roach, 276, 291, 0x0000, 0x2400, 0x0000, 0x6003, 0x0050},
  {kMiniEnemySpecies_Roach, 273, 296, 0x0000, 0x2400, 0x0000, 0x9C02, 0x0050},
  {kMiniEnemySpecies_Roach, 494, 535, 0x0000, 0x2400, 0x0000, 0xF004, 0x0050},
  {kMiniEnemySpecies_Roach, 278, 1721, 0x0000, 0x2400, 0x0000, 0xBC02, 0x0050},
};

static const MiniEnemySpeciesMetadata *MiniEnemyMetadataForSpecies(uint16 species_id) {
  for (size_t i = 0; i < sizeof(kMiniEnemySpeciesMetadata) / sizeof(kMiniEnemySpeciesMetadata[0]); i++) {
    if (kMiniEnemySpeciesMetadata[i].species_id == species_id)
      return &kMiniEnemySpeciesMetadata[i];
  }
  return NULL;
}

static bool MiniKnownClimbPopulationForSpawn(const MiniEditorEnemySpawnView *spawn,
                                             MiniKnownEnemyPopulation *population) {
  if (spawn == NULL || population == NULL)
    return false;
  for (size_t i = 0; i < sizeof(kMiniKnownClimbEnemyPopulation) / sizeof(kMiniKnownClimbEnemyPopulation[0]); i++) {
    const MiniKnownEnemyPopulation *candidate = &kMiniKnownClimbEnemyPopulation[i];
    if (candidate->species_id == spawn->species_id &&
        candidate->x == spawn->x_pos &&
        candidate->y == spawn->y_pos) {
      *population = *candidate;
      return true;
    }
  }
  return false;
}

static bool MiniEnemyTakesProjectileDamage(const MiniEnemyRuntimeState *enemy) {
  return enemy->behavior == kMiniEnemyBehavior_Roach ||
         enemy->behavior == kMiniEnemyBehavior_SpacePirateShooter;
}

static bool MiniEnemyDoesTouchDamage(const MiniEnemyRuntimeState *enemy) {
  return enemy->behavior == kMiniEnemyBehavior_Roach ||
         enemy->behavior == kMiniEnemyBehavior_SpacePirateShooter;
}

static void MiniComputeSpriteViewOriginDelta(const MiniEditorRoomSpriteView *sprite,
                                             int16 *origin_dx, int16 *origin_dy) {
  int min_x = 0;
  int max_x = 0;
  int min_y = 0;
  int max_y = 0;
  if (sprite->entry_count > 0) {
    min_x = max_x = sprite->entries[0].x_offset;
    min_y = max_y = sprite->entries[0].y_offset;
    for (int entry_index = 0; entry_index < sprite->entry_count; entry_index++) {
      const MiniEditorRoomSpriteOamView *entry = &sprite->entries[entry_index];
      int size = entry->is_16x16 ? 16 : 8;
      if (entry->x_offset < min_x)
        min_x = entry->x_offset;
      if (entry->y_offset < min_y)
        min_y = entry->y_offset;
      if (entry->x_offset + size > max_x)
        max_x = entry->x_offset + size;
      if (entry->y_offset + size > max_y)
        max_y = entry->y_offset + size;
    }
  }
  *origin_dx = (int16)((min_x + max_x) / 2);
  *origin_dy = (int16)((min_y + max_y) / 2);
}

static void MiniEnemyBindSpriteView(MiniEnemyRuntimeState *enemy) {
  const MiniEditorRoomSpriteView *sprites = NULL;
  int sprite_view_count = MiniAssetBootstrap_GetEditorRoomSpriteViews(&sprites);
  enemy->sprite_view_index = kMiniEnemyNoSpriteView;
  enemy->sprite_origin_dx = 0;
  enemy->sprite_origin_dy = 0;
  if (sprite_view_count <= 0 || sprites == NULL)
    return;

  int best_index = kMiniEnemyNoSpriteView;
  int best_distance = 0x7FFFFFFF;
  for (int i = 0; i < sprite_view_count; i++) {
    const MiniEditorRoomSpriteView *sprite = &sprites[i];
    if (sprite->species_id != enemy->species_id || sprite->tile_data == NULL)
      continue;
    int dx = sprite->x_pos - enemy->home_x;
    int dy = sprite->y_pos - enemy->home_y;
    int distance = dx * dx + dy * dy;
    if (distance < best_distance) {
      best_distance = distance;
      best_index = i;
    }
  }
  if (best_index < 0)
    return;
  const MiniEditorRoomSpriteView *bound = &sprites[best_index];
  MiniComputeSpriteViewOriginDelta(bound, &enemy->sprite_origin_dx, &enemy->sprite_origin_dy);
  enemy->sprite_view_index = (int16)best_index;
  if (best_distance <= 64 * 64) {
    enemy->x = bound->x_pos + enemy->sprite_origin_dx;
    enemy->y = bound->y_pos + enemy->sprite_origin_dy;
  }
}

static bool MiniClimbEndlessUsesRecycledEnemyBand(const MiniGameState *state) {
  return !state->room.uses_original_gameplay_runtime &&
         MiniRunMode_IsClimbEndless() &&
         state->room.room_id == kMiniClimbEndlessRoomId &&
         strcmp(state->room.room_handle, "climb") == 0;
}

static void MiniRecycleClimbRoachIntoCameraBand(MiniGameState *state, int enemy_index,
                                                MiniEnemyRuntimeState *enemy) {
  enum {
    kClimbEnemyTopMargin = 24,
    kClimbEnemyBottomMargin = 24,
    kClimbEnemyPatternStride = 49,
  };

  if (!MiniClimbEndlessUsesRecycledEnemyBand(state) ||
      enemy->behavior != kMiniEnemyBehavior_Roach)
    return;
  if ((unsigned)enemy_index >=
      sizeof(kMiniKnownClimbEnemyPopulation) / sizeof(kMiniKnownClimbEnemyPopulation[0])) {
    return;
  }

  int camera_y = state->viewport.camera_y;
  int min_y = camera_y + kClimbEnemyTopMargin;
  int max_y = camera_y + state->viewport.height - kClimbEnemyBottomMargin;
  if (enemy->y >= min_y && enemy->y <= max_y)
    return;

  const MiniKnownEnemyPopulation *population = &kMiniKnownClimbEnemyPopulation[enemy_index];
  int band_height = max_y - min_y + 1;
  if (band_height <= 0)
    band_height = state->viewport.height;
  if (band_height <= 0)
    band_height = 1;
  int pattern_y = (population->y + enemy_index * kClimbEnemyPatternStride) % band_height;
  enemy->x = population->x;
  enemy->y = min_y + pattern_y;
  enemy->home_x = enemy->x;
  enemy->home_y = enemy->y;
  enemy->x_velocity = 0;
  enemy->y_velocity = 0;
  enemy->ai_state = 0;
  enemy->state_timer = 0;
  enemy->facing_right = state->players[0].samus.world_x > enemy->x;
}

static void MiniRecycleClimbEnemiesIntoCameraBand(MiniGameState *state) {
  if (!MiniClimbEndlessUsesRecycledEnemyBand(state))
    return;
  for (int i = 0; i < state->enemy_state.count; i++) {
    MiniEnemyRuntimeState *enemy = &state->enemy_state.enemies[i];
    if (enemy->active)
      MiniRecycleClimbRoachIntoCameraBand(state, i, enemy);
  }
}

static MiniEnemyRuntimeState *MiniReserveEnemy(MiniGameState *state) {
  if (state->enemy_state.count >= kMiniEnemyCapacity)
    return NULL;
  return &state->enemy_state.enemies[state->enemy_state.count++];
}

static void MiniApplyPopulationWords(MiniEnemyRuntimeState *enemy,
                                     const MiniEditorEnemySpawnView *spawn) {
  if (spawn->has_population_words) {
    enemy->init_parameter = spawn->init_parameter;
    enemy->properties1 = spawn->properties1;
    enemy->properties2 = spawn->properties2;
    enemy->extra_parameter1 = spawn->extra_parameter1;
    enemy->extra_parameter2 = spawn->extra_parameter2;
    return;
  }

  MiniKnownEnemyPopulation population;
  if (MiniKnownClimbPopulationForSpawn(spawn, &population)) {
    enemy->init_parameter = population.init_parameter;
    enemy->properties1 = population.properties1;
    enemy->properties2 = population.properties2;
    enemy->extra_parameter1 = population.extra_parameter1;
    enemy->extra_parameter2 = population.extra_parameter2;
  }
}

static void MiniInitializeOriginalExportEnemies(MiniGameState *state) {
  const MiniEditorEnemySpawnView *spawns = NULL;
  int spawn_count = MiniAssetBootstrap_GetEditorEnemySpawnViews(&spawns);
  if (spawn_count <= 0)
    return;
  for (int i = 0; i < spawn_count; i++) {
    MiniEnemyRuntimeState *enemy = MiniReserveEnemy(state);
    if (enemy == NULL)
      return;
    const MiniEnemySpeciesMetadata *metadata =
        MiniEnemyMetadataForSpecies(spawns[i].species_id);
    int max_health = metadata != NULL ? metadata->max_health : 20;
    *enemy = (MiniEnemyRuntimeState){
      .active = true,
      .species_id = spawns[i].species_id,
      .x = spawns[i].x_pos,
      .y = spawns[i].y_pos,
      .home_x = spawns[i].x_pos,
      .home_y = spawns[i].y_pos,
      .x_radius = metadata != NULL ? metadata->x_radius : 12,
      .y_radius = metadata != NULL ? metadata->y_radius : 12,
      .health = max_health,
      .max_health = max_health,
      .damage = metadata != NULL ? metadata->damage : 0,
      .ai_bank = metadata != NULL ? metadata->ai_bank : 0,
      .init_ai = metadata != NULL ? metadata->init_ai : 0,
      .main_ai = metadata != NULL ? metadata->main_ai : 0,
      .sprite_view_index = kMiniEnemyNoSpriteView,
      .behavior = metadata != NULL ? metadata->behavior : kMiniEnemyBehavior_Passive,
    };
    MiniApplyPopulationWords(enemy, &spawns[i]);
    snprintf(enemy->source_label, sizeof(enemy->source_label), "%s", spawns[i].name);
    snprintf(enemy->name, sizeof(enemy->name), "%s",
             metadata != NULL ? metadata->canonical_name : spawns[i].name);
    MiniEnemyBindSpriteView(enemy);
    if (enemy->behavior == kMiniEnemyBehavior_SpacePirateShooter)
      enemy->shoot_cooldown = 24 + i * 9;
  }
}

static void MiniRefreshEnemyCounts(MiniGameState *state) {
  int active_count = 0;
  int passive_count = 0;
  int renderable_count = 0;
  int shot_count = 0;
  for (int i = 0; i < state->enemy_state.count; i++) {
    const MiniEnemyRuntimeState *enemy = &state->enemy_state.enemies[i];
    if (enemy->active)
      active_count++;
    if (enemy->active && enemy->behavior == kMiniEnemyBehavior_Passive)
      passive_count++;
    if (MiniEnemy_IsRuntimeRenderable(enemy))
      renderable_count++;
  }
  for (int i = 0; i < kMiniEnemyShotCapacity; i++) {
    if (state->enemy_state.shots[i].active)
      shot_count++;
  }
  state->enemy_state.active_count = active_count;
  state->enemy_state.passive_count = passive_count;
  state->enemy_state.renderable_count = renderable_count;
  state->enemy_state.shot_count = shot_count;
}

static void MiniInitializeEnemies(MiniGameState *state) {
  memset(&state->enemy_state, 0, sizeof(state->enemy_state));
  if (state->room.uses_original_gameplay_runtime)
    return;
  MiniInitializeOriginalExportEnemies(state);
  MiniRefreshEnemyCounts(state);
}

static void MiniSpawnPirateShot(MiniGameState *state, const MiniEnemyRuntimeState *pirate) {
  for (int i = 0; i < kMiniEnemyShotCapacity; i++) {
    MiniEnemyShotState *shot = &state->enemy_state.shots[i];
    if (shot->active)
      continue;
    int dir = pirate->facing_right ? 1 : -1;
    *shot = (MiniEnemyShotState){
      .active = true,
      .x = pirate->x + dir * (pirate->x_radius + 6),
      .y = pirate->y - pirate->y_radius / 3,
      .x_velocity = dir * 4,
      .y_velocity = 0,
      .radius = 5,
      .damage = 20,
    };
    return;
  }
}

static void MiniDamagePlayer(MiniGameState *state, int player, int damage) {
  if (player < 0 || player >= state->player_count)
    return;
  MiniPlayerCombatState *combat = &state->players[player].combat;
  if (combat->invulnerable_frames != 0)
    return;
  combat->hit_count++;
  combat->pending_damage = (uint16)damage;
  combat->hitstun_frames = 8;
  combat->invulnerable_frames = 24;
  combat->last_hit_by_player = 0;
}

static void MiniUpdatePirateShots(MiniGameState *state) {
  for (int i = 0; i < kMiniEnemyShotCapacity; i++) {
    MiniEnemyShotState *shot = &state->enemy_state.shots[i];
    if (!shot->active)
      continue;
    shot->x += shot->x_velocity;
    shot->y += shot->y_velocity;
    if (shot->x < state->room.room_left - 32 || shot->x > state->room.room_right + 32 ||
        shot->y < state->room.room_top - 32 || shot->y > state->room.room_bottom + 32) {
      shot->active = false;
      continue;
    }
    for (int player = 0; player < state->player_count; player++) {
      const MiniSamusCoreState *samus = &state->players[player].samus;
      if (!MiniRectsOverlap(shot->x, shot->y, shot->radius, shot->radius,
                            samus->world_x, samus->world_y,
                            samus->x_radius, samus->y_radius)) {
        continue;
      }
      MiniDamagePlayer(state, player, shot->damage);
      shot->active = false;
      break;
    }
  }
}

static void MiniHandleSamusProjectilesVsEnemies(MiniGameState *state) {
  for (int projectile_view_index = 0; projectile_view_index < state->projectile_state.count; projectile_view_index++) {
    const SamusProjectileView *projectile = &state->projectile_state.views[projectile_view_index];
    if (projectile->slot_index >= kSamusProjectileSlotCount)
      continue;
    int projectile_rx = projectile->x_radius != 0 ? projectile->x_radius : 4;
    int projectile_ry = projectile->y_radius != 0 ? projectile->y_radius : 4;
    for (int enemy_index = 0; enemy_index < state->enemy_state.count; enemy_index++) {
      MiniEnemyRuntimeState *enemy = &state->enemy_state.enemies[enemy_index];
      if (!enemy->active || !MiniEnemyTakesProjectileDamage(enemy) ||
          enemy->invulnerable_frames != 0)
        continue;
      if (!MiniRectsOverlap(projectile->x_pos, projectile->y_pos, projectile_rx, projectile_ry,
                            enemy->x, enemy->y, enemy->x_radius, enemy->y_radius)) {
        continue;
      }
      int damage = projectile->damage != 0 ? projectile->damage : 20;
      enemy->health -= damage;
      enemy->hit_count++;
      enemy->invulnerable_frames = 4;
      MiniClearProjectileSlot(state, projectile->slot_index);
      if (enemy->health <= 0) {
        enemy->active = false;
        enemy->health = 0;
        state->enemy_state.defeated_count++;
      }
      break;
    }
  }
}

static void MiniHandleEnemyTouchDamage(MiniGameState *state) {
  for (int enemy_index = 0; enemy_index < state->enemy_state.count; enemy_index++) {
    const MiniEnemyRuntimeState *enemy = &state->enemy_state.enemies[enemy_index];
    if (!enemy->active || !MiniEnemyDoesTouchDamage(enemy))
      continue;
    for (int player = 0; player < state->player_count; player++) {
      const MiniSamusCoreState *samus = &state->players[player].samus;
      if (MiniRectsOverlap(enemy->x, enemy->y, enemy->x_radius, enemy->y_radius,
                           samus->world_x, samus->world_y, samus->x_radius, samus->y_radius)) {
        MiniDamagePlayer(state, player, enemy->damage);
      }
    }
  }
}

static void MiniRoachVelocityFromPopulation(const MiniEnemyRuntimeState *enemy,
                                            int *x_velocity, int *y_velocity) {
  static const int kOctantX[8] = {1, 1, 0, -1, -1, -1, 0, 1};
  static const int kOctantY[8] = {0, -1, -1, -1, 0, 1, 1, 1};
  uint8 angle = (uint8)(enemy->extra_parameter1 >> 8);
  int speed = enemy->extra_parameter1 & 0xFF;
  if (speed <= 0)
    speed = 2;
  speed = MiniClampInt(speed, 1, 4);
  int octant = ((int)angle + 16) >> 5;
  octant &= 7;
  *x_velocity = kOctantX[octant] * speed;
  *y_velocity = kOctantY[octant] * speed;
}

static void MiniUpdateRoach(MiniGameState *state, MiniEnemyRuntimeState *enemy) {
  int trigger_radius = enemy->extra_parameter2 & 0xFF;
  if (trigger_radius <= 0)
    trigger_radius = 80;
  const MiniSamusCoreState *samus = &state->players[0].samus;
  if (enemy->ai_state == 0) {
    if (abs(enemy->x - samus->world_x) > trigger_radius ||
        abs(enemy->y - samus->world_y) > trigger_radius) {
      return;
    }
    enemy->ai_state = 1;
    MiniRoachVelocityFromPopulation(enemy, &enemy->x_velocity, &enemy->y_velocity);
  }

  enemy->state_timer++;
  if ((enemy->state_timer & 31) == 0) {
    MiniRoachVelocityFromPopulation(enemy, &enemy->x_velocity, &enemy->y_velocity);
    if (((enemy->state_timer >> 5) & 1) != 0) {
      enemy->x_velocity = -enemy->x_velocity;
    }
  }

  enemy->x += enemy->x_velocity;
  enemy->y += enemy->y_velocity;
  if (enemy->x < state->room.room_left + enemy->x_radius ||
      enemy->x > state->room.room_right - enemy->x_radius) {
    enemy->x = MiniClampInt(enemy->x,
                            state->room.room_left + enemy->x_radius,
                            state->room.room_right - enemy->x_radius);
    enemy->x_velocity = -enemy->x_velocity;
  }
  if (enemy->y < state->room.room_top + enemy->y_radius ||
      enemy->y > state->room.room_bottom - enemy->y_radius) {
    enemy->y = MiniClampInt(enemy->y,
                            state->room.room_top + enemy->y_radius,
                            state->room.room_bottom - enemy->y_radius);
    enemy->y_velocity = -enemy->y_velocity;
  }
}

static void MiniUpdateEnemies(MiniGameState *state) {
  if (state->room.uses_original_gameplay_runtime || state->enemy_state.count == 0)
    return;
  for (int i = 0; i < state->enemy_state.count; i++) {
    MiniEnemyRuntimeState *enemy = &state->enemy_state.enemies[i];
    if (!enemy->active)
      continue;
    if (enemy->invulnerable_frames != 0)
      enemy->invulnerable_frames--;
    if (enemy->behavior == kMiniEnemyBehavior_Roach) {
      MiniUpdateRoach(state, enemy);
      continue;
    }
    if (enemy->behavior != kMiniEnemyBehavior_SpacePirateShooter)
      continue;
    enemy->facing_right = state->players[0].samus.world_x > enemy->x;
    if (enemy->shoot_cooldown > 0) {
      enemy->shoot_cooldown--;
    } else {
      MiniSpawnPirateShot(state, enemy);
      enemy->shoot_cooldown = 64;
    }
  }
  MiniHandleSamusProjectilesVsEnemies(state);
  MiniHandleEnemyTouchDamage(state);
  MiniUpdatePirateShots(state);
  MiniRecycleClimbEnemiesIntoCameraBand(state);
  MiniRefreshEnemyCounts(state);
  MiniRefreshProjectileState(state);
  MiniSyncLegacyPublicFields(state);
}

static void MiniClearProjectileSlot(MiniGameState *state, uint16 slot_index) {
  if (slot_index >= kSamusProjectileSlotCount)
    return;
  ClearProjectile((uint16)(slot_index * 2));
  state->projectile_state.owner_by_slot[slot_index] = 0;
}

static void MiniTickCombatTimers(MiniGameState *state) {
  for (int player = 0; player < state->player_count; player++) {
    MiniPlayerCombatState *combat = &state->players[player].combat;
    if (combat->hitstun_frames != 0)
      combat->hitstun_frames--;
    if (combat->invulnerable_frames != 0)
      combat->invulnerable_frames--;
  }
}

static void MiniHandleProjectileHits(MiniGameState *state) {
  for (int i = 0; i < state->projectile_state.count; i++) {
    const SamusProjectileView *projectile = &state->projectile_state.views[i];
    if (projectile->slot_index >= kSamusProjectileSlotCount)
      continue;
    uint8 owner_plus_one = state->projectile_state.owner_by_slot[projectile->slot_index];
    if (owner_plus_one == 0)
      continue;
    int owner = owner_plus_one - 1;
    if (owner < 0 || owner >= state->player_count)
      continue;

    int projectile_rx = projectile->x_radius != 0 ? projectile->x_radius : 4;
    int projectile_ry = projectile->y_radius != 0 ? projectile->y_radius : 4;
    for (int player = 0; player < state->player_count; player++) {
      if (player == owner)
        continue;
      MiniPlayerCombatState *combat = &state->players[player].combat;
      if (combat->invulnerable_frames != 0)
        continue;
      const MiniSamusCoreState *samus = &state->players[player].samus;
      if (!MiniRectsOverlap(projectile->x_pos, projectile->y_pos,
                            projectile_rx, projectile_ry,
                            samus->world_x, samus->world_y,
                            samus->x_radius, samus->y_radius)) {
        continue;
      }

      combat->hit_count++;
      combat->pending_damage = projectile->damage != 0 ? projectile->damage : 20;
      combat->hitstun_frames = 6;
      combat->invulnerable_frames = 10;
      combat->last_hit_by_player = owner_plus_one;
      MiniClearProjectileSlot(state, projectile->slot_index);
      break;
    }
  }
}

static void MiniUpdateMultiplayerCombat(MiniGameState *state) {
  if (state->player_count < 2)
    return;
  MiniPruneInactiveProjectileOwners(state);
  MiniAssignUnownedProjectiles(state);
  MiniTickCombatTimers(state);
  MiniHandleProjectileHits(state);
  MiniRefreshProjectileState(state);
  MiniPruneInactiveProjectileOwners(state);
  MiniSyncLegacyPublicFields(state);
}

static void MiniStepSharedSamusMultiplayerFrame(MiniGameState *state) {
  for (int i = 0; i < state->player_count; i++) {
    state->samus = state->players[i].samus;
    state->controls = MiniControlStateForPlayer(state, i);
    MiniLoadPlayerRuntime(state, i);
    MiniApplyPlayerJoypadState(state, i);
    HandleControllerInputForGamePhysics();
    HandleSamusMovementAndPause();
    state->players[i].samus = MiniSamusCoreFromGlobals(state);
    MiniSavePlayerRuntime(state, i);
  }

  MiniLoadPlayerRuntime(state, 0);
  state->samus = state->players[0].samus;
  MiniApplyPlayerJoypadState(state, 0);
  if (MiniEditorCamera_ShouldUseState(state)) {
    MiniEditorCamera_Follow(state);
  } else {
    MainScrollingRoutine();
    if (!state->room.uses_rom_room)
      MiniStubs_ClampCameraToRoom();
    CalculateLayer2PosAndScrollsWhenScrolling();
  }
  AnimtilesHandler();
  NmiProcessAnimtilesVramTransfers();
  NMI_ProcessVramWriteQueue();
  MiniSyncRenderState(state);
}

static void MiniRunSamusPostMovementChecksForStoredPlayers(MiniGameState *state) {
  for (int i = 0; i < state->player_count; i++) {
    MiniLoadPlayerRuntime(state, i);
    MiniCopySamusCoreToGlobals(&state->players[i].samus);
    MiniApplyPlayerJoypadState(state, i);
    Samus_JumpCheck();
    Samus_ShootCheck();
    state->players[i].samus = MiniSamusCoreFromGlobals(state);
    MiniSavePlayerRuntime(state, i);
  }
  MiniLoadPlayerRuntime(state, 0);
  MiniCopySamusCoreToGlobals(&state->players[0].samus);
  MiniApplyPlayerJoypadState(state, 0);
  state->samus = state->players[0].samus;
  MiniUpdatePlayerScreenPositions(state);
}

static void MiniRunSamusPostMovementChecksFromMultiSamus(MiniGameState *state) {
  int active_players = state->player_count;
  if (active_players > MultiSamus_GetNumSamus())
    active_players = MultiSamus_GetNumSamus();
  if (active_players > kMiniMaxPlayers)
    active_players = kMiniMaxPlayers;
  if (active_players < 1)
    active_players = 1;

  for (int i = 0; i < active_players; i++) {
    MultiSamus_Switch(i);
    MiniApplyPlayerJoypadState(state, i);
    Samus_JumpCheck();
    Samus_ShootCheck();
    state->players[i].samus = MiniSamusCoreFromGlobals(state);
    MiniSavePlayerRuntime(state, i);
  }
  MultiSamus_Switch(0);
  state->samus = state->players[0].samus;
  MiniUpdatePlayerScreenPositions(state);
}

void MiniUpdate(MiniGameState *state, const MiniInputState *input) {
  bool music_already_ticked = false;

  if (input->quit_requested) {
    state->controls.quit_requested = true;
    state->quit_requested = true;
  }

  MiniUpdateButtons(state, input);
  if (state->room.uses_original_gameplay_runtime) {
    state->original_oam_next_ptr = MiniStepOriginalGameplayFrame();
  } else if (state->player_count > 1) {
    state->original_oam_next_ptr = 0;
    nmi_frame_counter_word++;
    HdmaObjectHandler();
    music_already_ticked = true;
    PaletteFxHandler();
    MiniStepSharedSamusMultiplayerFrame(state);
    MiniRunSamusPostMovementChecksForStoredPlayers(state);
  } else if (MiniAuthoredMovement_ShouldUseState(state)) {
    state->original_oam_next_ptr = 0;
    nmi_frame_counter_word++;
    MiniAuthoredMovement_Step(state);
    MiniStubs_ClampCameraToRoom();
  } else if (MiniEditorCamera_ShouldUseState(state)) {
    state->original_oam_next_ptr = 0;
    nmi_frame_counter_word++;
    HdmaObjectHandler();
    music_already_ticked = true;
    PaletteFxHandler();
    GameplayFrame_SamusInputForAllPlayers();
    GameplayFrame_SamusMovementForAllPlayers();
    MiniRunSamusPostMovementChecksFromMultiSamus(state);
    MiniEditorCamera_Follow(state);
    GameplayFrame_Animtiles();
    NmiProcessAnimtilesVramTransfers();
    NMI_ProcessVramWriteQueue();
  } else {
    state->original_oam_next_ptr = 0;
    nmi_frame_counter_word++;
    HdmaObjectHandler();
    music_already_ticked = true;
    PaletteFxHandler();
    GameplayFrame_SamusInputForAllPlayers();
    GameplayFrame_SamusMovementForAllPlayers();
    MiniRunSamusPostMovementChecksFromMultiSamus(state);
    MainScrollingRoutine();
    if (!state->room.uses_rom_room)
      MiniStubs_ClampCameraToRoom();
    CalculateLayer2PosAndScrollsWhenScrolling();
    GameplayFrame_Animtiles();
    NmiProcessAnimtilesVramTransfers();
    NMI_ProcessVramWriteQueue();
  }
  if (!state->room.uses_original_gameplay_runtime)
    MiniAudio_TickQueues(music_already_ticked);
  MiniClimbEndless_Tick(state);
  MiniSyncRenderState(state);
  MiniUpdateMultiplayerCombat(state);
  MiniUpdateEnemies(state);
  state->frame++;
}

uint64_t MiniGameState_ComputeHash(const MiniGameState *state) {
  static const uint64_t kFnvOffsetBasis = UINT64_C(14695981039346656037);
  uint64_t hash = kFnvOffsetBasis;

  hash = MiniHashInt(hash, state->frame);
  hash = MiniHashInt(hash, state->viewport.width);
  hash = MiniHashInt(hash, state->viewport.height);
  hash = MiniHashInt(hash, state->viewport.camera_x);
  hash = MiniHashInt(hash, state->viewport.camera_y);
  hash = MiniHashInt(hash, state->samus.world_x);
  hash = MiniHashInt(hash, state->samus.world_y);
  hash = MiniHashInt(hash, state->samus.x_velocity);
  hash = MiniHashInt(hash, state->samus.y_velocity);
  hash = MiniHashInt(hash, state->samus.screen_x);
  hash = MiniHashInt(hash, state->samus.screen_y);
  hash = MiniHashUInt16(hash, state->samus.x_radius);
  hash = MiniHashUInt16(hash, state->samus.y_radius);
  hash = MiniHashUInt16(hash, state->samus.pose);
  hash = MiniHashUInt16(hash, state->samus.movement_type);
  hash = MiniHashByte(hash, (uint8)state->samus.suit);
  hash = MiniHashBool(hash, state->samus.on_ground);
  hash = MiniHashInt(hash, state->player_count);
  for (int i = 0; i < kMiniMaxPlayers; i++) {
    const MiniPlayerInputState *input = &state->player_inputs[i];
    const MiniPlayerState *player = &state->players[i];
    hash = MiniHashUInt16(hash, input->buttons);
    hash = MiniHashUInt16(hash, input->previous_buttons);
    hash = MiniHashUInt16(hash, input->new_buttons);
    hash = MiniHashInt(hash, player->samus.world_x);
    hash = MiniHashInt(hash, player->samus.world_y);
    hash = MiniHashInt(hash, player->samus.x_velocity);
    hash = MiniHashInt(hash, player->samus.y_velocity);
    hash = MiniHashInt(hash, player->samus.screen_x);
    hash = MiniHashInt(hash, player->samus.screen_y);
    hash = MiniHashUInt16(hash, player->samus.x_radius);
    hash = MiniHashUInt16(hash, player->samus.y_radius);
    hash = MiniHashUInt16(hash, player->samus.pose);
    hash = MiniHashUInt16(hash, player->samus.movement_type);
    hash = MiniHashByte(hash, (uint8)player->samus.suit);
    hash = MiniHashBool(hash, player->samus.on_ground);
    hash = MiniHashUInt16(hash, player->combat.hit_count);
    hash = MiniHashUInt16(hash, player->combat.pending_damage);
    hash = MiniHashUInt16(hash, player->combat.hitstun_frames);
    hash = MiniHashUInt16(hash, player->combat.invulnerable_frames);
    hash = MiniHashByte(hash, player->combat.last_hit_by_player);
    hash = MiniHashBytes(hash, state->player_runtime_pre_projectile[i],
                         sizeof(state->player_runtime_pre_projectile[i]));
    hash = MiniHashBytes(hash, state->player_runtime_post_projectile[i],
                         sizeof(state->player_runtime_post_projectile[i]));
  }
  hash = MiniHashInt(hash, state->projectile_state.count);
  for (int i = 0; i < kMiniProjectileViewCapacity; i++) {
    const SamusProjectileView *projectile = &state->projectile_state.views[i];
    hash = MiniHashBool(hash, projectile->active);
    hash = MiniHashBool(hash, projectile->is_beam);
    hash = MiniHashBool(hash, projectile->is_basic_beam);
    hash = MiniHashUInt16(hash, projectile->slot_index);
    hash = MiniHashUInt16(hash, projectile->type);
    hash = MiniHashUInt16(hash, projectile->direction);
    hash = MiniHashUInt16(hash, projectile->x_pos);
    hash = MiniHashUInt16(hash, projectile->y_pos);
    hash = MiniHashUInt16(hash, projectile->x_radius);
    hash = MiniHashUInt16(hash, projectile->y_radius);
    hash = MiniHashUInt16(hash, projectile->damage);
  }
  for (int i = 0; i < kSamusProjectileSlotCount; i++)
    hash = MiniHashByte(hash, state->projectile_state.owner_by_slot[i]);
  hash = MiniHashInt(hash, state->enemy_state.count);
  hash = MiniHashInt(hash, state->enemy_state.active_count);
  hash = MiniHashInt(hash, state->enemy_state.passive_count);
  hash = MiniHashInt(hash, state->enemy_state.renderable_count);
  hash = MiniHashInt(hash, state->enemy_state.shot_count);
  hash = MiniHashUInt16(hash, state->enemy_state.defeated_count);
  for (int i = 0; i < kMiniEnemyCapacity; i++) {
    const MiniEnemyRuntimeState *enemy = &state->enemy_state.enemies[i];
    hash = MiniHashBool(hash, enemy->active);
    hash = MiniHashUInt16(hash, enemy->species_id);
    hash = MiniHashBytes(hash, enemy->name, sizeof(enemy->name));
    hash = MiniHashBytes(hash, enemy->source_label, sizeof(enemy->source_label));
    hash = MiniHashUInt16(hash, enemy->init_parameter);
    hash = MiniHashUInt16(hash, enemy->properties1);
    hash = MiniHashUInt16(hash, enemy->properties2);
    hash = MiniHashUInt16(hash, enemy->extra_parameter1);
    hash = MiniHashUInt16(hash, enemy->extra_parameter2);
    hash = MiniHashByte(hash, enemy->ai_bank);
    hash = MiniHashUInt16(hash, enemy->init_ai);
    hash = MiniHashUInt16(hash, enemy->main_ai);
    hash = MiniHashInt(hash, enemy->x);
    hash = MiniHashInt(hash, enemy->y);
    hash = MiniHashInt(hash, enemy->home_x);
    hash = MiniHashInt(hash, enemy->home_y);
    hash = MiniHashInt(hash, enemy->x_velocity);
    hash = MiniHashInt(hash, enemy->y_velocity);
    hash = MiniHashInt(hash, enemy->x_radius);
    hash = MiniHashInt(hash, enemy->y_radius);
    hash = MiniHashInt(hash, enemy->health);
    hash = MiniHashInt(hash, enemy->max_health);
    hash = MiniHashInt(hash, enemy->damage);
    hash = MiniHashInt(hash, enemy->shoot_cooldown);
    hash = MiniHashInt(hash, enemy->invulnerable_frames);
    hash = MiniHashUInt16(hash, enemy->ai_state);
    hash = MiniHashUInt16(hash, enemy->state_timer);
    hash = MiniHashUInt16(hash, enemy->hit_count);
    hash = MiniHashBool(hash, enemy->facing_right);
    hash = MiniHashBool(hash, MiniEnemy_HasSpriteView(enemy));
    hash = MiniHashByte(hash, (uint8)enemy->behavior);
  }
  for (int i = 0; i < kMiniEnemyShotCapacity; i++) {
    const MiniEnemyShotState *shot = &state->enemy_state.shots[i];
    hash = MiniHashBool(hash, shot->active);
    hash = MiniHashInt(hash, shot->x);
    hash = MiniHashInt(hash, shot->y);
    hash = MiniHashInt(hash, shot->x_velocity);
    hash = MiniHashInt(hash, shot->y_velocity);
    hash = MiniHashInt(hash, shot->radius);
    hash = MiniHashInt(hash, shot->damage);
  }
  hash = MiniHashUInt16(hash, state->controls.buttons);
  hash = MiniHashUInt16(hash, state->controls.previous_buttons);
  hash = MiniHashUInt16(hash, state->controls.new_buttons);
  hash = MiniHashBool(hash, state->controls.quit_requested);
  hash = MiniHashBool(hash, state->room.has_room);
  hash = MiniHashBool(hash, state->room.uses_rom_room);
  hash = MiniHashBool(hash, state->room.booted_from_save_slot);
  hash = MiniHashBool(hash, state->room.has_editor_room_visuals);
  hash = MiniHashBool(hash, state->room.uses_original_gameplay_runtime);
  hash = MiniHashBool(hash, state->room.has_original_enemies);
  hash = MiniHashBool(hash, state->room.has_original_plms);
  hash = MiniHashByte(hash, (uint8)state->room.samus_suit);
  hash = MiniHashUInt16(hash, state->room.room_id);
  hash = MiniHashByte(hash, (uint8)state->room.room_source);
  hash = MiniHashInt(hash, state->room.room_left);
  hash = MiniHashInt(hash, state->room.room_top);
  hash = MiniHashInt(hash, state->room.room_right);
  hash = MiniHashInt(hash, state->room.room_bottom);
  hash = MiniHashInt(hash, state->room.room_width_blocks);
  hash = MiniHashInt(hash, state->room.room_height_blocks);
  hash = MiniHashInt(hash, state->room.camera_x);
  hash = MiniHashInt(hash, state->room.camera_y);
  hash = MiniHashInt(hash, state->room.spawn_x);
  hash = MiniHashInt(hash, state->room.spawn_y);
  hash = MiniHashInt(hash, state->room.camera_target_x_percent);
  hash = MiniHashInt(hash, state->room.camera_target_y_percent);
  hash = MiniHashBytes(hash, state->room.room_handle, sizeof(state->room.room_handle));
  hash = MiniHashBytes(hash, state->room.room_name, sizeof(state->room.room_name));
  hash = MiniHashInt(hash, state->room.doorway_count);
  for (int i = 0; i < kMiniDoorwayTransitionCapacity; i++) {
    const MiniDoorwayTransition *doorway = &state->room.doorways[i];
    hash = MiniHashBool(hash, doorway->active);
    hash = MiniHashInt(hash, doorway->source_block_x);
    hash = MiniHashInt(hash, doorway->source_block_y);
    hash = MiniHashInt(hash, doorway->destination_x);
    hash = MiniHashInt(hash, doorway->destination_y);
    hash = MiniHashInt(hash, doorway->camera_x);
    hash = MiniHashInt(hash, doorway->camera_y);
  }
  hash = MiniHashInt(hash, state->collision_map.block_size);
  hash = MiniHashInt(hash, state->collision_map.width_blocks);
  hash = MiniHashInt(hash, state->collision_map.height_blocks);
  hash = MiniHashInt(hash, state->collision_map.world_left);
  hash = MiniHashInt(hash, state->collision_map.world_top);
  hash = MiniHashInt(hash, state->collision_map.world_right);
  hash = MiniHashInt(hash, state->collision_map.world_bottom);
  hash = MiniHashBytes(hash, g_ram, sizeof(g_ram));
  hash = MiniHashBytes(hash, g_sram, 0x2000);
  hash = MiniHashBytes(hash, MiniPpu_GetVram(), kMiniPpuVramSize);
  if (MiniRunMode_IsClimbEndless()) {
    MiniClimbModeSnapshot climb_mode;
    MiniClimbEndless_SaveSnapshot(&climb_mode);
    hash = MiniHashInt(hash, (int)MiniRunMode_Get());
    hash = MiniHashInt(hash, climb_mode.virtual_floors);
    hash = MiniHashBool(hash, climb_mode.lava_enabled);
    hash = MiniHashInt(hash, climb_mode.lava_floor_y);
    hash = MiniHashInt(hash, climb_mode.ascent_pixels);
    hash = MiniHashInt(hash, climb_mode.last_samus_y);
    hash = MiniHashBool(hash, climb_mode.has_score_anchor);
  }
  return hash;
}

void MiniInit(MiniGameState *state, int viewport_width, int viewport_height) {
  MiniGameState_Init(state, viewport_width, viewport_height);
}

void MiniSetPlayerCount(MiniGameState *state, int player_count) {
  if (state == NULL)
    return;
  state->player_count = MiniNormalizePlayerCount(player_count);
  MultiSamus_SetNumSamus(state->player_count);
  MiniFaceMultiplayerPlayersAtEachOther(state);
  for (int i = state->player_count; i < kMiniMaxPlayers; i++) {
    state->player_inputs[i] = (MiniPlayerInputState){0};
  }
  MiniUpdatePlayerScreenPositions(state);
  MiniSyncLegacyPublicFields(state);
}

void MiniStep(MiniGameState *state, const MiniInputState *input) {
  MiniUpdate(state, input);
}

void MiniStepButtons(MiniGameState *state, uint16 buttons, bool quit_requested) {
  MiniInputState input = {
    .buttons = buttons,
    .player_buttons = { buttons, 0 },
    .player_count = 1,
    .quit_requested = quit_requested,
  };
  MiniStep(state, &input);
}

void MiniStepPlayers(MiniGameState *state, const uint16 *player_buttons,
                     int player_count, bool quit_requested) {
  MiniInputState input = {
    .player_count = MiniNormalizePlayerCount(player_count),
    .quit_requested = quit_requested,
  };
  for (int i = 0; i < input.player_count; i++)
    input.player_buttons[i] = player_buttons != NULL ? player_buttons[i] : 0;
  input.buttons = input.player_buttons[0];
  MiniStep(state, &input);
}

uint64_t MiniStateHash(const MiniGameState *state) {
  return MiniGameState_ComputeHash(state);
}

size_t MiniSaveStateSize(void) {
  return sizeof(MiniStateSnapshot);
}

bool MiniSaveState(const MiniGameState *state, void *buffer, size_t buffer_size) {
  if (state == NULL || buffer == NULL || buffer_size < sizeof(MiniStateSnapshot))
    return false;

  MiniStateSnapshot *snapshot = (MiniStateSnapshot *)buffer;
  memset(snapshot, 0, sizeof(*snapshot));
  snapshot->magic = kMiniSnapshotMagic;
  snapshot->version = kMiniSnapshotVersion;
  snapshot->game = *state;
  snapshot->run_mode = MiniRunMode_Get();
  MiniClimbEndless_SaveSnapshot(&snapshot->climb_mode);
  MiniStubs_SaveSnapshot(&snapshot->stubs);
  MiniPpu_SaveSnapshot(&snapshot->ppu);
  memcpy(snapshot->ram, g_ram, sizeof(snapshot->ram));
  memcpy(snapshot->sram, g_sram, sizeof(snapshot->sram));
  snapshot->use_my_apu_code = g_use_my_apu_code;
  snapshot->host_debug_flag = g_debug_flag;
  snapshot->snes_frame_counter = snes_frame_counter;
  snapshot->installed_bug_fix_counter = currently_installed_bug_fix_counter;
  return true;
}

bool MiniLoadState(MiniGameState *state, const void *buffer, size_t buffer_size) {
  if (state == NULL || buffer == NULL || buffer_size < sizeof(MiniStateSnapshot))
    return false;

  const MiniStateSnapshot *snapshot = (const MiniStateSnapshot *)buffer;
  if (snapshot->magic != kMiniSnapshotMagic || snapshot->version != kMiniSnapshotVersion)
    return false;

  *state = snapshot->game;
  MiniRunMode_Set(snapshot->run_mode);
  MiniClimbEndless_LoadSnapshot(&snapshot->climb_mode);
  MiniStubs_LoadSnapshot(&snapshot->stubs);
  MiniPpu_LoadSnapshot(&snapshot->ppu);
  memcpy(g_ram, snapshot->ram, sizeof(snapshot->ram));
  memcpy(g_sram, snapshot->sram, sizeof(snapshot->sram));
  g_use_my_apu_code = snapshot->use_my_apu_code;
  g_debug_flag = snapshot->host_debug_flag;
  snes_frame_counter = snapshot->snes_frame_counter;
  currently_installed_bug_fix_counter = snapshot->installed_bug_fix_counter;
  return true;
}

MiniGameState *MiniCreate(int viewport_width, int viewport_height) {
  MiniGameState *state = (MiniGameState *)calloc(1, sizeof(*state));
  if (state != NULL)
    MiniInit(state, viewport_width, viewport_height);
  return state;
}

void MiniDestroy(MiniGameState *state) {
  free(state);
}
