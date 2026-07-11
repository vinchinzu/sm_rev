#include "mini_multiplayer_players.h"

#include <string.h>

#include "funcs.h"
#include "ida_types.h"
#include "multi_samus.h"
#include "variables.h"

enum {
  kMiniPlayerRuntimePreProjectileOffset = 0xA94,
  kMiniPlayerRuntimePostProjectileOffset = 0xCCC,
  kMiniPlayerTwoSpawnOffsetX = 48,
};

static int MiniClampInt(int value, int min_value, int max_value) {
  if (value < min_value)
    return min_value;
  if (value > max_value)
    return max_value;
  return value;
}

static MiniSamusCoreState MiniSamusCoreWithScreen(const MiniGameState *state,
                                                  MiniSamusCoreState samus) {
  samus.screen_x = samus.world_x - state->viewport.camera_x - samus.x_radius;
  samus.screen_y = samus.world_y - state->viewport.camera_y - samus.y_radius;
  samus.suit = state->room.samus_suit;
  return samus;
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

static void MiniSavePlayerCoreIntoRuntime(MiniGameState *state,
                                          int player_index) {
  MiniMultiplayerPlayers_LoadRuntime(state, player_index);
  MiniMultiplayerPlayers_CopyCoreToGlobals(&state->players[player_index].samus);
  MiniRefreshSamusRuntimePose();
  state->players[player_index].samus =
      MiniMultiplayerPlayers_CoreFromGlobals(state);
  MiniMultiplayerPlayers_SaveRuntime(state, player_index);
}

int MiniMultiplayerPlayers_NormalizeCount(int player_count) {
  if (player_count < 1)
    return 1;
  if (player_count > kMiniMaxPlayers)
    return kMiniMaxPlayers;
  return player_count;
}

uint16 MiniMultiplayerPlayers_InputButtons(const MiniInputState *input,
                                           int player_index) {
  if (player_index == 0 && input->buttons != 0)
    return input->buttons;
  return input->player_buttons[player_index];
}

MiniSamusCoreState MiniMultiplayerPlayers_CoreFromGlobals(
    const MiniGameState *state) {
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

void MiniMultiplayerPlayers_CopyCoreToGlobals(const MiniSamusCoreState *samus) {
  samus_x_pos = (uint16)samus->world_x;
  samus_y_pos = (uint16)samus->world_y;
  samus_prev_x_pos = samus_x_pos;
  samus_prev_y_pos = samus_y_pos;
  samus_x_radius = samus->x_radius;
  samus_y_radius = samus->y_radius;
  samus_pose = samus->pose;
  samus_movement_type = samus->movement_type;
}

void MiniMultiplayerPlayers_SaveRuntime(MiniGameState *state, int player_index) {
  memcpy(state->player_runtime_pre_projectile[player_index],
         g_ram + kMiniPlayerRuntimePreProjectileOffset,
         kMiniPlayerRuntimePreProjectileSize);
  memcpy(state->player_runtime_post_projectile[player_index],
         g_ram + kMiniPlayerRuntimePostProjectileOffset,
         kMiniPlayerRuntimePostProjectileSize);
}

void MiniMultiplayerPlayers_LoadRuntime(const MiniGameState *state,
                                        int player_index) {
  memcpy(g_ram + kMiniPlayerRuntimePreProjectileOffset,
         state->player_runtime_pre_projectile[player_index],
         kMiniPlayerRuntimePreProjectileSize);
  memcpy(g_ram + kMiniPlayerRuntimePostProjectileOffset,
         state->player_runtime_post_projectile[player_index],
         kMiniPlayerRuntimePostProjectileSize);
}

void MiniMultiplayerPlayers_UpdateScreenPositions(MiniGameState *state) {
  for (int i = 0; i < kMiniMaxPlayers; i++)
    state->players[i].samus = MiniSamusCoreWithScreen(state, state->players[i].samus);
  state->samus = state->players[0].samus;
}

void MiniMultiplayerPlayers_SyncFromOriginalRuntime(MiniGameState *state) {
  int active_players = state->player_count;
  if (active_players > MultiSamus_GetNumSamus())
    active_players = MultiSamus_GetNumSamus();
  if (active_players > kMiniMaxPlayers)
    active_players = kMiniMaxPlayers;
  if (active_players < 1)
    active_players = 1;

  for (int i = 0; i < active_players; i++) {
    MultiSamus_Switch(i);
    state->players[i].samus = MiniMultiplayerPlayers_CoreFromGlobals(state);
    MiniMultiplayerPlayers_SaveRuntime(state, i);
  }
  MultiSamus_Switch(0);
  state->samus = state->players[0].samus;
  MiniMultiplayerPlayers_UpdateScreenPositions(state);
}

void MiniMultiplayerPlayers_InitializePlayerOne(MiniGameState *state) {
  state->players[0].samus = state->samus;
  state->players[0].combat = (MiniPlayerCombatState){0};
  MiniMultiplayerPlayers_SaveRuntime(state, 0);
}

void MiniMultiplayerPlayers_InitializePlayerTwo(MiniGameState *state) {
  state->players[1] = state->players[0];
  state->players[1].samus.world_x =
      state->players[0].samus.world_x + kMiniPlayerTwoSpawnOffsetX;
  int min_x = state->room.room_left + state->players[1].samus.x_radius;
  int max_x = state->room.room_right - state->players[1].samus.x_radius - 1;
  if (max_x >= min_x)
    state->players[1].samus.world_x =
        MiniClampInt(state->players[1].samus.world_x, min_x, max_x);
  state->players[1].combat = (MiniPlayerCombatState){0};

  MiniMultiplayerPlayers_LoadRuntime(state, 0);
  MiniMultiplayerPlayers_CopyCoreToGlobals(&state->players[1].samus);
  MiniMultiplayerPlayers_SaveRuntime(state, 1);
  MiniMultiplayerPlayers_LoadRuntime(state, 0);
  MiniMultiplayerPlayers_UpdateScreenPositions(state);
}

void MiniMultiplayerPlayers_ApplyJoypad(const MiniGameState *state,
                                        int player_index) {
  joypad1_prev = state->player_inputs[player_index].previous_buttons;
  joypad1_lastkeys = state->player_inputs[player_index].buttons;
  joypad1_newkeys = state->player_inputs[player_index].new_buttons;
  joypad1_newkeys2_UNUSED = joypad1_newkeys;
}

void MiniMultiplayerPlayers_FaceEachOther(MiniGameState *state) {
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
  MiniMultiplayerPlayers_LoadRuntime(state, 0);
  state->samus = state->players[0].samus;
  MiniMultiplayerPlayers_UpdateScreenPositions(state);
}

void MiniMultiplayerPlayers_RunPostMovementChecksForStored(
    MiniGameState *state) {
  for (int i = 0; i < state->player_count; i++) {
    MiniMultiplayerPlayers_LoadRuntime(state, i);
    MiniMultiplayerPlayers_CopyCoreToGlobals(&state->players[i].samus);
    MiniMultiplayerPlayers_ApplyJoypad(state, i);
    Samus_JumpCheck();
    Samus_ShootCheck();
    state->players[i].samus = MiniMultiplayerPlayers_CoreFromGlobals(state);
    MiniMultiplayerPlayers_SaveRuntime(state, i);
  }
  MiniMultiplayerPlayers_LoadRuntime(state, 0);
  MiniMultiplayerPlayers_CopyCoreToGlobals(&state->players[0].samus);
  MiniMultiplayerPlayers_ApplyJoypad(state, 0);
  state->samus = state->players[0].samus;
  MiniMultiplayerPlayers_UpdateScreenPositions(state);
}

void MiniMultiplayerPlayers_RunPostMovementChecksFromMultiSamus(
    MiniGameState *state) {
  int active_players = state->player_count;
  if (active_players > MultiSamus_GetNumSamus())
    active_players = MultiSamus_GetNumSamus();
  if (active_players > kMiniMaxPlayers)
    active_players = kMiniMaxPlayers;
  if (active_players < 1)
    active_players = 1;

  for (int i = 0; i < active_players; i++) {
    MultiSamus_Switch(i);
    MiniMultiplayerPlayers_ApplyJoypad(state, i);
    Samus_JumpCheck();
    Samus_ShootCheck();
    state->players[i].samus = MiniMultiplayerPlayers_CoreFromGlobals(state);
    MiniMultiplayerPlayers_SaveRuntime(state, i);
  }
  MultiSamus_Switch(0);
  state->samus = state->players[0].samus;
  MiniMultiplayerPlayers_UpdateScreenPositions(state);
}
