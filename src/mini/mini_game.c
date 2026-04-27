#include "mini_game.h"

#include <stdint.h>
#include <string.h>

#include "funcs.h"
#include "ida_types.h"
#include "mini_ppu_stub.h"
#include "physics_config.h"
#include "samus_projectile_view.h"
#include "sm_rtl.h"
#include "stubs_mini.h"
#include "variables.h"

enum {
  kMiniItem_VariaSuit = 1,
  kMiniItem_GravitySuit = 0x20,
};

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

static void MiniSyncRenderState(MiniGameState *state) {
  state->camera_x = layer1_x_pos;
  state->camera_y = layer1_y_pos;
  state->samus_x = samus_x_pos - state->camera_x - samus_x_radius;
  state->samus_y = samus_y_pos - state->camera_y - samus_y_radius;
  state->samus_pose_value = samus_pose;
  state->samus_movement_type_value = samus_movement_type;
  state->projectile_count = SamusProjectile_GetActiveViews(
      state->projectiles, kMiniProjectileViewCapacity);
}

static void MiniUpdateButtons(MiniGameState *state, const MiniInputState *input) {
  joypad1_prev = state->last_buttons;
  joypad1_lastkeys = input->buttons;
  joypad1_newkeys = input->buttons & (joypad1_prev ^ input->buttons);
  joypad1_newkeys2_UNUSED = joypad1_newkeys;
  joypad2_last = 0;
  joypad2_new_keys = 0;
  joypad2_newkeys2 = 0;
  state->last_buttons = input->buttons;
}

static uint16 MiniInitialPoseForRoom(const MiniRoomInfo *room) {
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
  samus_input_handler = FUNC16(Samus_InputHandler_E913);
  samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
  samus_draw_handler = FUNC16(SamusDrawHandler_Default);

  Samus_Initialize();
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

  MiniStubs_Reset();
  MiniStubs_ConfigureWorld(viewport_width, viewport_height);
  MiniStubs_GetRoomInfo(&room);

  state->frame = 0;
  state->viewport_width = viewport_width;
  state->viewport_height = viewport_height;
  state->room_id = room.room_id;
  state->room_width_blocks = room.room_width_blocks;
  state->room_height_blocks = room.room_height_blocks;
  state->room_left = room.room_left;
  state->room_top = room.room_top;
  state->room_right = room.room_right;
  state->room_bottom = room.room_bottom;
  state->camera_x = room.camera_x;
  state->camera_y = room.camera_y;
  state->original_oam_next_ptr = 0;
  state->ground_y = room.room_bottom;
  state->last_buttons = 0;
  state->has_room = room.has_room;
  state->uses_rom_room = room.uses_rom_room;
  state->has_editor_room_visuals = room.has_editor_room_visuals;
  state->uses_original_gameplay_runtime = room.uses_original_gameplay_runtime;
  state->has_original_enemies = room.has_original_enemies;
  state->has_original_plms = room.has_original_plms;
  state->quit_requested = false;
  state->samus_suit = room.samus_suit;
  state->room_source = room.room_source;
  memcpy(state->room_handle, room.room_handle, sizeof(state->room_handle));
  memcpy(state->room_name, room.room_name, sizeof(state->room_name));

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
  samus_x_pos = room.spawn_x;
  samus_y_pos = room.spawn_y;
  samus_prev_x_pos = samus_x_pos;
  samus_prev_y_pos = samus_y_pos;
  MiniSyncRenderState(state);
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

void MiniUpdate(MiniGameState *state, const MiniInputState *input) {
  if (input->quit_requested) {
    state->quit_requested = true;
  }

  MiniUpdateButtons(state, input);
  if (state->uses_original_gameplay_runtime) {
    state->original_oam_next_ptr = MiniStepOriginalGameplayFrame();
  } else {
    state->original_oam_next_ptr = 0;
    nmi_frame_counter_word++;
    HdmaObjectHandler();
    PaletteFxHandler();
    HandleControllerInputForGamePhysics();
    HandleSamusMovementAndPause();
    MainScrollingRoutine();
    if (!state->uses_rom_room)
      MiniStubs_ClampCameraToRoom();
    CalculateLayer2PosAndScrollsWhenScrolling();
    AnimtilesHandler();
    NmiProcessAnimtilesVramTransfers();
    NMI_ProcessVramWriteQueue();
  }
  MiniSyncRenderState(state);
  state->frame++;
}

uint64_t MiniGameState_ComputeHash(const MiniGameState *state) {
  static const uint64_t kFnvOffsetBasis = UINT64_C(14695981039346656037);
  MiniRoomInfo room;
  uint64_t hash = kFnvOffsetBasis;

  MiniStubs_GetRoomInfo(&room);
  hash = MiniHashInt(hash, state->frame);
  hash = MiniHashInt(hash, state->viewport_width);
  hash = MiniHashInt(hash, state->viewport_height);
  hash = MiniHashInt(hash, state->camera_x);
  hash = MiniHashInt(hash, state->camera_y);
  hash = MiniHashInt(hash, state->samus_x);
  hash = MiniHashInt(hash, state->samus_y);
  hash = MiniHashUInt16(hash, state->samus_pose_value);
  hash = MiniHashUInt16(hash, state->samus_movement_type_value);
  hash = MiniHashInt(hash, state->projectile_count);
  for (int i = 0; i < kMiniProjectileViewCapacity; i++) {
    const SamusProjectileView *projectile = &state->projectiles[i];
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
  hash = MiniHashUInt16(hash, state->last_buttons);
  hash = MiniHashBool(hash, state->quit_requested);
  hash = MiniHashBool(hash, room.has_room);
  hash = MiniHashBool(hash, room.uses_rom_room);
  hash = MiniHashBool(hash, room.booted_from_save_slot);
  hash = MiniHashBool(hash, room.has_editor_room_visuals);
  hash = MiniHashBool(hash, room.uses_original_gameplay_runtime);
  hash = MiniHashBool(hash, room.has_original_enemies);
  hash = MiniHashBool(hash, room.has_original_plms);
  hash = MiniHashByte(hash, (uint8)room.samus_suit);
  hash = MiniHashUInt16(hash, room.room_id);
  hash = MiniHashByte(hash, (uint8)room.room_source);
  hash = MiniHashInt(hash, room.room_left);
  hash = MiniHashInt(hash, room.room_top);
  hash = MiniHashInt(hash, room.room_right);
  hash = MiniHashInt(hash, room.room_bottom);
  hash = MiniHashInt(hash, room.room_width_blocks);
  hash = MiniHashInt(hash, room.room_height_blocks);
  hash = MiniHashInt(hash, room.camera_x);
  hash = MiniHashInt(hash, room.camera_y);
  hash = MiniHashInt(hash, room.spawn_x);
  hash = MiniHashInt(hash, room.spawn_y);
  hash = MiniHashBytes(hash, room.room_handle, sizeof(room.room_handle));
  hash = MiniHashBytes(hash, room.room_name, sizeof(room.room_name));
  hash = MiniHashBytes(hash, g_ram, sizeof(g_ram));
  hash = MiniHashBytes(hash, g_sram, 0x2000);
  hash = MiniHashBytes(hash, MiniPpu_GetVram(), 0x10000);
  return hash;
}
