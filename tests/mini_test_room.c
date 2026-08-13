#include "mini_test_room.h"

#include <string.h>
#include "funcs.h"
#include "ida_types.h"
#include "mini/mini_defs.h"
#include "mini/mini_room_adapter.h"
#include "physics.h"
#include "types.h"
#include "variables.h"

// Mini room adapter capacity (from mini_room_adapter.c)
enum { kMiniLevelDataCapacity = (0x16402 - 0x10002) / 2 };

extern void MiniGameState_Init(MiniGameState *state, int viewport_width, int viewport_height);
extern void MiniAuthoredMovement_InitializeSamusGlobals(void);
extern void MiniAuthoredMovement_SyncGrounded(MiniGameState *state);
extern void LoadPhysicsConfig(void);

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

MiniGameState *MiniTestRoom_CreateWithFloor(int viewport_width, int viewport_height) {
  MiniGameState *state = (MiniGameState *)calloc(1, sizeof(*state));
  if (state == NULL)
    return NULL;

  // Initialize minimal room dimensions
  const int kTestRoomWidthBlocks = (viewport_width + kMiniBlockSize - 1) / kMiniBlockSize;
  const int kTestRoomHeightBlocks = (viewport_height + kMiniBlockSize - 1) / kMiniBlockSize;
  const int kFloorBlockY = kTestRoomHeightBlocks - 3;  // Floor 3 blocks from bottom

  // Set up collision geometry manually (bypass ROM bootstrap)
  room_width_in_blocks = kTestRoomWidthBlocks;
  room_height_in_blocks = kTestRoomHeightBlocks;
  
  // Clear level data
  memset(level_data, 0, sizeof(uint16) * kMiniLevelDataCapacity);
  memset(BTS, 0, kMiniLevelDataCapacity);
  
  // Create a solid floor at the bottom
  for (int x = 0; x < kTestRoomWidthBlocks; x++) {
    for (int y = kFloorBlockY; y < kTestRoomHeightBlocks; y++) {
      int index = y * kTestRoomWidthBlocks + x;
      level_data[index] = BlockTileWithTypeIndex(0, kBlockType_Solid);
      BTS[index] = 0;
    }
  }

  // Create a 1-tile platform for jump test (at mid-height)
  int platform_y = kFloorBlockY - 4;
  int platform_x_start = kTestRoomWidthBlocks / 2 - 1;
  for (int x = platform_x_start; x < platform_x_start + 2; x++) {
    int index = platform_y * kTestRoomWidthBlocks + x;
    level_data[index] = BlockTileWithTypeIndex(0, kBlockType_Solid);
    BTS[index] = 0;
  }

  // Set up world bounds
  const int world_left = 32;
  const int world_right = viewport_width - 32;
  const int world_ceiling = 32;
  const int world_floor = kTestRoomHeightBlocks * kMiniBlockSize;

  // Configure room info for authored movement
  MiniRoomInfo test_room = {
    .has_room = true,
    .uses_rom_room = false,
    .booted_from_save_slot = false,
    .has_editor_room_visuals = false,
    .uses_original_gameplay_runtime = false,
    .has_original_enemies = false,
    .has_original_plms = false,
    .samus_suit = kMiniSamusSuit_Power,
    .room_id = 0xFFFF,  // Test room ID
    .room_source = kMiniRoomSource_EditorExport,  // Enable authored movement
    .room_left = world_left,
    .room_top = world_ceiling,
    .room_right = world_right,
    .room_bottom = world_floor,
    .room_width_blocks = kTestRoomWidthBlocks,
    .room_height_blocks = kTestRoomHeightBlocks,
    .camera_x = 0,
    .camera_y = 0,
    .spawn_x = viewport_width / 2,
    .spawn_y = kFloorBlockY * kMiniBlockSize - 32,  // Spawn above floor
    .camera_target_x_percent = 50,
    .camera_target_y_percent = 50,
  };
  snprintf(test_room.room_handle, sizeof(test_room.room_handle), "test_room");
  snprintf(test_room.room_name, sizeof(test_room.room_name), "Test Room with Floor");

  // Save to global state
  MiniStubs_SaveSnapshot(&(MiniStubsSnapshot){
    .world_left = world_left,
    .world_right = world_right,
    .world_ceiling = world_ceiling,
    .world_floor = world_floor,
    .explicit_room_export_path = false,
    .room_info = test_room,
  });

  // Initialize game state
  memset(state, 0, sizeof(*state));
  state->frame = 0;
  state->viewport = (MiniViewportState){
    .width = viewport_width,
    .height = viewport_height,
    .camera_x = test_room.camera_x,
    .camera_y = test_room.camera_y,
  };
  state->room = MiniRoomState_FromInfo(&test_room);
  MiniStubs_GetCollisionMapView(&state->collision_map);
  state->original_oam_next_ptr = 0;
  state->controls = (MiniControlState){0};
  state->player_count = 1;
  state->samus.suit = test_room.samus_suit;

  // Initialize button config
  button_config_left = kButton_Left;
  button_config_right = kButton_Right;
  button_config_jump_a = kButton_A;
  button_config_run_b = kButton_B;
  button_config_shoot_x = kButton_X;
  button_config_itemcancel_y = kButton_Y;
  button_config_aim_down_L = kButton_L;
  button_config_aim_up_R = kButton_R;

  // Initialize physics
  LoadPhysicsConfig();
  
  // Initialize Samus for authored movement
  samus_max_health = 99;
  samus_health = samus_max_health;
  samus_input_handler = FUNC16(Samus_InputHandler_E913);
  samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
  samus_draw_handler = FUNC16(SamusDrawHandler_Default);
  samus_pose = kPose_01_FaceR_Normal;
  samus_movement_type = kMovementType_00_Standing;
  samus_x_pos = test_room.spawn_x;
  samus_y_pos = test_room.spawn_y;
  samus_prev_x_pos = samus_x_pos;
  samus_prev_y_pos = samus_y_pos;

  // Initialize authored movement globals
  MiniAuthoredMovement_InitializeSamusGlobals();
  
  // Sync Samus state to MiniGameState
  state->samus.world_x = samus_x_pos;
  state->samus.world_y = samus_y_pos;
  state->samus.x_velocity = 0;
  state->samus.y_velocity = 0;
  state->samus.x_radius = 6;
  state->samus.y_radius = 16;
  state->samus.pose = samus_pose;
  state->samus.movement_type = samus_movement_type;
  
  // Set Samus on ground
  MiniAuthoredMovement_SyncGrounded(state);
  
  // Initialize player state
  state->players[0].samus = state->samus;
  state->players[0].combat = (MiniPlayerCombatState){0};

  return state;
}
