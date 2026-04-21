#include "mini_game.h"

#include <string.h>

#include "funcs.h"
#include "ida_types.h"
#include "mini_runtime.h"
#include "physics_config.h"
#include "stubs_mini.h"
#include "variables.h"

enum {
  kMiniItem_VariaSuit = 1,
  kMiniItem_GravitySuit = 0x20,
};

static void MiniSyncRenderState(MiniGameState *state) {
  state->camera_x = layer1_x_pos;
  state->camera_y = layer1_y_pos;
  state->samus_x = samus_x_pos - state->camera_x - samus_x_radius;
  state->samus_y = samus_y_pos - state->camera_y - samus_y_radius;
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
  state->ground_y = room.room_bottom;
  state->last_buttons = 0;
  state->has_room = room.has_room;
  state->uses_rom_room = room.uses_rom_room;
  state->has_editor_room_visuals = room.has_editor_room_visuals;
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
  samus_x_pos = room.spawn_x;
  samus_y_pos = room.spawn_y;
  samus_prev_x_pos = samus_x_pos;
  samus_prev_y_pos = samus_y_pos;
  MiniSyncRenderState(state);
}

void MiniUpdate(MiniGameState *state, const MiniInputState *input) {
  if (input->quit_requested) {
    state->quit_requested = true;
  }

  nmi_frame_counter_word++;
  MiniUpdateButtons(state, input);
  HandleControllerInputForGamePhysics();
  HandleSamusMovementAndPause();
  MainScrollingRoutine();
  if (!state->uses_rom_room)
    MiniStubs_ClampCameraToRoom();
  CalculateLayer2PosAndScrollsWhenScrolling();
  NmiProcessAnimtilesVramTransfers();
  NMI_ProcessVramWriteQueue();
  MiniSyncRenderState(state);
  state->frame++;
}
