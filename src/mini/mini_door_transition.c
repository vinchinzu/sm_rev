#include "mini_door_transition.h"

#include "block_reaction.h"
#include "funcs.h"
#include "ida_types.h"
#include "mini_content_scope.h"
#include "mini_room_adapter.h"
#include "variables.h"

static int MiniDoorTransition_Clamp(int value, int min_value, int max_value) {
  if (value < min_value)
    return min_value;
  if (value > max_value)
    return max_value;
  return value;
}

static bool MiniDoorTransition_IsCeresEscapeState(void) {
  return game_state == kGameState_32_MadeItToCeresElevator
      || game_state == kGameState_33_BlackoutFromCeres
      || game_state == kGameState_34_CeresGoesBoom
      || game_state == kGameState_35_TimeUp
      || game_state == kGameState_36_WhitingOutFromTimeUp
      || game_state == kGameState_37_CeresGoesBoomWithSamus;
}

bool MiniDoorTransition_IsActive(void) {
  return game_state == kGameState_9_HitDoorBlock
      || game_state == kGameState_10_LoadingNextRoom
      || game_state == kGameState_11_LoadingNextRoom
      || MiniDoorTransition_IsCeresEscapeState();
}

void MiniDoorTransition_BeginFrame(void) {
  if (MiniDoorTransition_IsActive())
    return;
  coroutine_state_1 = 0;
  coroutine_state_2 = 0;
  coroutine_state_3 = 0;
  coroutine_state_4 = 0;
}

void MiniDoorTransition_RejectOutOfScope(void) {
  if (game_state != kGameState_9_HitDoorBlock)
    return;
  if (MiniContentScope_AllowsDoor(door_def_ptr))
    return;
  game_state = kGameState_8_MainGameplay;
  door_transition_function = FUNC16(DoorTransitionFunction_HandleElevator);
}

void MiniDoorTransition_PumpScrollIrq(void) {
  if (game_state != kGameState_11_LoadingNextRoom)
    return;
  if ((door_transition_flag & 0x8000) != 0)
    return;
  if (door_transition_function != FUNC16(DoorTransitionFunction_LoadMoreThings_Async))
    return;
  Irq_FollowDoorTransition();
}

void MiniDoorTransition_Dispatch(void) {
  switch (game_state) {
  case kGameState_9_HitDoorBlock:
    GameState_9_HitDoorBlock();
    break;
  case kGameState_10_LoadingNextRoom:
    GameState_10_LoadingNextRoom_Async();
    break;
  case kGameState_11_LoadingNextRoom:
    GameState_11_LoadingNextRoom_Async();
    break;
  case kGameState_32_MadeItToCeresElevator:
    GameState_32_MadeItToCeresElevator();
    break;
  case kGameState_33_BlackoutFromCeres:
    GameState_33_BlackoutFromCeres();
    break;
  case kGameState_34_CeresGoesBoom:
  case kGameState_37_CeresGoesBoomWithSamus:
    GameState_37_CeresGoesBoomWithSamus();
    break;
  case kGameState_35_TimeUp:
    GameState_35_TimeUp();
    break;
  case kGameState_36_WhitingOutFromTimeUp:
    GameState_36_WhitingOutFromTimeUp();
    break;
  default:
    if (game_state != kGameState_8_MainGameplay)
      game_state = kGameState_8_MainGameplay;
    GameState_8_MainGameplay();
    break;
  }
}

void MiniDoorTransition_SyncRoom(MiniGameState *state) {
  MiniRoomInfo info;

  if (!state->room.uses_original_gameplay_runtime)
    return;
  if (state->room.room_id == room_ptr)
    return;

  MiniStubs_RefreshRomRoomFromGlobals();
  MiniStubs_GetRoomInfo(&info);
  MiniApplyRoomInfo(state, &info);
}

bool MiniDoorTransition_TryAuthored(MiniGameState *state, int y_radius_fallback) {
  int y_radius = state->samus.y_radius != 0 ? state->samus.y_radius : y_radius_fallback;
  int block_x = state->samus.world_x >> kBlockPixelShift;
  int foot_y = state->samus.world_y + y_radius - 1;
  int block_y = foot_y >> kBlockPixelShift;
  int doorway_count;
  int max_x;
  int max_y;

  if (MiniStubs_GetCollisionMaterial(block_x, block_y) != kBlockType_Door)
    return false;

  doorway_count = state->room.doorway_count;
  if (doorway_count > kMiniDoorwayTransitionCapacity)
    doorway_count = kMiniDoorwayTransitionCapacity;
  for (int i = 0; i < doorway_count; i++) {
    const MiniDoorwayTransition *doorway = &state->room.doorways[i];
    if (!doorway->active ||
        doorway->source_block_x != block_x ||
        doorway->source_block_y != block_y) {
      continue;
    }

    state->samus.world_x = doorway->destination_x;
    state->samus.world_y = doorway->destination_y;
    state->samus.x_velocity = 0;
    state->samus.y_velocity = 0;
    max_x = state->room.room_width_blocks * kMiniBlockSize - state->viewport.width;
    max_y = state->room.room_height_blocks * kMiniBlockSize - state->viewport.height;
    layer1_x_pos = MiniDoorTransition_Clamp(doorway->camera_x, 0, max_x > 0 ? max_x : 0);
    layer1_y_pos = MiniDoorTransition_Clamp(doorway->camera_y, 0, max_y > 0 ? max_y : 0);
    layer1_x_subpos = 0;
    layer1_y_subpos = 0;
    return true;
  }
  return false;
}
