#include "mini_game.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "funcs.h"
#include "ida_types.h"
#include "mini_authored_movement.h"
#include "mini_content_scope.h"
#include "mini_door_transition.h"
#include "mini_enemy_runtime.h"
#include "mini_multiplayer_combat.h"
#include "mini_ppu_stub.h"
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
  kMiniSnapshotVersion = 5,
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

void MiniApplyRoomInfo(MiniGameState *state, const MiniRoomInfo *room) {
  state->room = MiniRoomState_FromInfo(room);
  MiniStubs_GetCollisionMapView(&state->collision_map);
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

void MiniSyncPublicViews(MiniGameState *state) {
  state->player_count = MiniNormalizePlayerCount(state->player_count);
  if (state->players[0].samus.x_radius != 0 || state->players[0].samus.y_radius != 0)
    state->samus = state->players[0].samus;
}

void MiniRefreshProjectileState(MiniGameState *state) {
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
  MiniSyncPublicViews(state);
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

  if (room->uses_rom_room && room->room_id == kMiniContentScopeRoom_CeresElevator)
    loading_game_state = kLoadingGameState_1F_StartingAtCeres;

  Samus_Initialize();
  samus_x_pos = room->spawn_x;
  samus_y_pos = room->spawn_y;
  samus_prev_x_pos = samus_x_pos;
  samus_prev_y_pos = samus_y_pos;
  if (room->uses_rom_room && room->room_id == kMiniContentScopeRoom_CeresElevator) {
    CallSomeSamusCode(8);
    return;
  }

  samus_pose = MiniInitialPoseForRoom(room);
  samus_movement_type = kMovementType_00_Standing;
  samus_anim_frame_skip = 0;
  SamusFunc_F433();
  Samus_SetRadius();
  Samus_SetAnimationFrameIfPoseChanged();
  Samus_UpdatePreviousPose();
  CallSomeSamusCode(1);
}

void MiniGameState_Init(MiniGameState *state, int viewport_width, int viewport_height) {
  MiniRoomInfo room;

  memset(state, 0, sizeof(*state));
  MiniSystem_Reset();
  MiniStubs_ConfigureWorld(viewport_width, viewport_height);
  MiniStubs_GetRoomInfo(&room);

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
  MiniSyncPublicViews(state);

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
  EnablePaletteFx();
  EnableHdmaObjects();
  EnableAnimtiles();
  SetLiquidPhysicsType();
  if (!(room.uses_rom_room && room.room_id == kMiniContentScopeRoom_CeresElevator)) {
    samus_x_pos = room.spawn_x;
    samus_y_pos = room.spawn_y;
  }
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
  MultiSamus_SetNumSamus(1);
  MiniEnemyRuntime_Initialize(state);
  MiniSyncPublicViews(state);
}

static uint16 MiniStepOriginalGameplayFrame(MiniGameState *state) {
  MiniDoorTransition_BeginFrame();

  HdmaObjectHandler();
  NextRandom();
  ClearOamExt();
  oam_next_ptr = 0;
  nmi_copy_samus_halves = 0;
  nmi_copy_samus_top_half_src = 0;
  nmi_copy_samus_bottom_half_src = 0;

  MiniDoorTransition_RejectOutOfScope();
  MiniDoorTransition_PumpScrollIrq();
  MiniDoorTransition_Dispatch();
  HandleSoundEffects();
  uint16 original_oam_next_ptr = oam_next_ptr;
  ClearUnusedOam();

  waiting_for_nmi = 1;
  Vector_NMI();
  MiniDoorTransition_SyncRoom(state);
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
  MainScrollingRoutine();
  if (!state->room.uses_rom_room)
    MiniStubs_ClampCameraToRoom();
  CalculateLayer2PosAndScrollsWhenScrolling();
  AnimtilesHandler();
  NmiProcessAnimtilesVramTransfers();
  NMI_ProcessVramWriteQueue();
  MiniSyncRenderState(state);
}

static void MiniStepGameplay(MiniGameState *state) {
  if (state->room.uses_original_gameplay_runtime) {
    state->original_oam_next_ptr = MiniStepOriginalGameplayFrame(state);
    return;
  }

  state->original_oam_next_ptr = 0;
  nmi_frame_counter_word++;
  if (state->player_count > 1) {
    HdmaObjectHandler();
    PaletteFxHandler();
    MiniStepSharedSamusMultiplayerFrame(state);
    return;
  }
  if (MiniAuthoredMovement_ShouldUseState(state)) {
    MiniAuthoredMovement_Step(state);
    MiniStubs_ClampCameraToRoom();
    MiniEnemyRuntime_Update(state);
    return;
  }

  HdmaObjectHandler();
  PaletteFxHandler();
  HandleControllerInputForGamePhysics();
  HandleSamusMovementAndPause();
  MainScrollingRoutine();
  if (!state->room.uses_rom_room)
    MiniStubs_ClampCameraToRoom();
  MiniEnemyRuntime_Update(state);
  CalculateLayer2PosAndScrollsWhenScrolling();
  AnimtilesHandler();
  NmiProcessAnimtilesVramTransfers();
  NMI_ProcessVramWriteQueue();
}

void MiniUpdate(MiniGameState *state, const MiniInputState *input) {
  if (input->quit_requested)
    state->controls.quit_requested = true;

  MiniUpdateButtons(state, input);
  MiniStepGameplay(state);
  MiniSyncRenderState(state);
  MiniUpdateMultiplayerCombat(state);
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
  for (int i = state->player_count; i < kMiniMaxPlayers; i++) {
    state->player_inputs[i] = (MiniPlayerInputState){0};
  }
  MiniUpdatePlayerScreenPositions(state);
  MiniSyncPublicViews(state);
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
