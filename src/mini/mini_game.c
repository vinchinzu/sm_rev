#include "mini_game.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "funcs.h"
#include "gameplay_frame.h"
#include "ida_types.h"
#include "mini_audio.h"
#include "mini_authored_movement.h"
#include "mini_climb_endless.h"
#include "mini_climb_mode.h"
#include "mini_editor_camera.h"
#include "mini_enemy_runtime.h"
#include "mini_frame_step.h"
#include "mini_multiplayer_combat.h"
#include "mini_multiplayer_players.h"
#include "mini_multiplayer_weapons.h"
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
  kMiniSnapshotVersion = 13,
  kMiniRamSnapshotSize = 0x20000,
  kMiniSramSnapshotSize = 0x2000,
};

typedef struct MiniStateSnapshot {
  uint32 magic;
  uint32 version;
  MiniGameState game;
  MiniStubsSnapshot stubs;
  MiniPpuSnapshot ppu;
  MiniRunMode run_mode;
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

static void MiniSyncLegacyPublicFields(MiniGameState *state) {
  state->player_count =
      MiniMultiplayerPlayers_NormalizeCount(state->player_count);
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
  state->samus = MiniMultiplayerPlayers_CoreFromGlobals(state);
  if (state->room.uses_original_gameplay_runtime)
    MiniMultiplayerPlayers_SyncFromOriginalRuntime(state);
  else {
    state->players[0].samus = state->samus;
    MiniMultiplayerPlayers_UpdateScreenPositions(state);
  }
  MiniRefreshProjectileState(state);
  MiniSyncLegacyPublicFields(state);
}

static void MiniUpdateButtons(MiniGameState *state, const MiniInputState *input) {
  int input_player_count = input->player_count != 0 ? input->player_count : state->player_count;
  int old_player_count = state->player_count;
  state->player_count =
      MiniMultiplayerPlayers_NormalizeCount(input_player_count);
  if (state->player_count != old_player_count)
    MultiSamus_SetNumSamus(state->player_count);
  for (int i = 0; i < kMiniMaxPlayers; i++) {
    uint16 buttons =
        i < state->player_count
            ? MiniMultiplayerPlayers_InputButtons(input, i)
            : 0;
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

void MiniGameState_Init(MiniGameState *state, int viewport_width, int viewport_height) {
  MiniRoomInfo room;

  memset(state, 0, sizeof(*state));
  MiniSystem_Reset();
  MiniStubs_ConfigureWorld(viewport_width, viewport_height);
  MiniStubs_GetRoomInfo(&room);
  if (MiniRunMode_IsClimbRoom()) {
    MiniClimbEndless_ApplySpawnDefaults(&room);
    if (MiniRunMode_IsClimbEndless())
      MiniClimbEndless_InitAfterRoom(state, &room);
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
  button_config_itemswitch = kButton_Select;
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
  MiniMultiplayerPlayers_InitializePlayerOne(state);
  MiniMultiplayerPlayers_InitializePlayerTwo(state);
  MiniMultiplayerWeapons_Initialize(state);
  MiniEnemyRuntime_Initialize(state);
  MultiSamus_SetNumSamus(1);
  MiniSyncLegacyPublicFields(state);
}

static void MiniStepSharedSamusMultiplayerFrame(MiniGameState *state) {
  MiniFrameStep_RunSharedMultiplayerSamus(state);
  MiniMultiplayerPlayers_LoadRuntime(state, 0);
  state->samus = state->players[0].samus;
  MiniMultiplayerPlayers_ApplyJoypad(state, 0);
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

void MiniUpdate(MiniGameState *state, const MiniInputState *input) {
  bool music_already_ticked = false;

  if (input->quit_requested) {
    state->controls.quit_requested = true;
    state->quit_requested = true;
  }

  MiniUpdateButtons(state, input);
  MiniMultiplayerCombat_BeginFrame(state);
  MiniMultiplayerWeapons_BeginFrame(state);
  if (state->room.uses_original_gameplay_runtime) {
    state->original_oam_next_ptr = MiniFrameStep_RunOriginalGameplay();
  } else if (state->player_count > 1) {
    state->original_oam_next_ptr = 0;
    nmi_frame_counter_word++;
    HdmaObjectHandler();
    music_already_ticked = true;
    PaletteFxHandler();
    MiniStepSharedSamusMultiplayerFrame(state);
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
    MiniMultiplayerPlayers_RunPostMovementChecksFromMultiSamus(state);
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
    MiniMultiplayerPlayers_RunPostMovementChecksFromMultiSamus(state);
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
  MiniMultiplayerWeapons_EndFrame();
  MiniClimbMode_Tick(state);
  MiniSyncRenderState(state);
  MiniMultiplayerCombat_Update(state);
  MiniEnemyRuntime_Update(state);
  MiniRefreshProjectileState(state);
  MiniSyncLegacyPublicFields(state);
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
    hash = MiniHashByte(hash, (uint8)player->weapon.selected);
    hash = MiniHashUInt16(hash, player->weapon.missiles);
    hash = MiniHashUInt16(hash, player->weapon.missile_capacity);
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
  hash = MiniHashByte(hash, state->melee_hit_event_count);
  hash = MiniHashBytes(hash, &state->melee_hit_event_dropped_count,
                       sizeof(state->melee_hit_event_dropped_count));
  for (int i = 0; i < kMiniMeleeHitEventCapacity; i++) {
    const MiniMeleeHitEvent *event = &state->melee_hit_events[i];
    hash = MiniHashByte(hash, event->attacker_player);
    hash = MiniHashByte(hash, event->defender_player);
    hash = MiniHashUInt16(hash, event->projectile_slot);
    hash = MiniHashUInt16(hash, event->damage);
  }
  if (!state->room.uses_original_gameplay_runtime) {
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
  hash = MiniHashInt(hash, (int)MiniRunMode_Get());
  hash = MiniHashInt(hash, state->climb.virtual_floors);
  hash = MiniHashBool(hash, state->climb.lava_enabled);
  hash = MiniHashInt(hash, state->climb.lava_floor_y);
  hash = MiniHashInt(hash, state->climb.lava_rise_carry_q8);
  hash = MiniHashInt(hash, state->climb.lava_enable_frame);
  hash = MiniHashInt(hash, state->climb.lava_damage_cooldown);
  hash = MiniHashInt(hash, state->climb.run_start_frame);
  hash = MiniHashInt(hash, state->climb.deaths);
  hash = MiniHashInt(hash, state->climb.best_ascent_pixels);
  hash = MiniHashInt(hash, state->climb.ascent_pixels);
  hash = MiniHashInt(hash, state->climb.last_samus_y);
  hash = MiniHashBool(hash, state->climb.has_score_anchor);
  return hash;
}

void MiniInit(MiniGameState *state, int viewport_width, int viewport_height) {
  MiniGameState_Init(state, viewport_width, viewport_height);
}

void MiniSetPlayerCount(MiniGameState *state, int player_count) {
  if (state == NULL)
    return;
  state->player_count = MiniMultiplayerPlayers_NormalizeCount(player_count);
  MultiSamus_SetNumSamus(state->player_count);
  MiniMultiplayerPlayers_FaceEachOther(state);
  for (int i = state->player_count; i < kMiniMaxPlayers; i++) {
    state->player_inputs[i] = (MiniPlayerInputState){0};
  }
  MiniMultiplayerPlayers_UpdateScreenPositions(state);
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
    .player_count = MiniMultiplayerPlayers_NormalizeCount(player_count),
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
