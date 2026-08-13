#include "mini_frame_step.h"

#include "funcs.h"
#include "mini_multiplayer_players.h"
#include "multi_samus.h"
#include "samus_projectile.h"
#include "variables.h"

uint16 MiniFrameStep_RunOriginalGameplay(void) {
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

static MiniControlState MiniControlStateForPlayer(const MiniGameState *state,
                                                  int player_index) {
  return (MiniControlState){
    .buttons = state->player_inputs[player_index].buttons,
    .previous_buttons = state->player_inputs[player_index].previous_buttons,
    .new_buttons = state->player_inputs[player_index].new_buttons,
    .quit_requested = state->controls.quit_requested,
  };
}

void MiniFrameStep_RunSharedMultiplayerSamus(MiniGameState *state) {
  SamusProjectile_BeginSharedInputPass();
  for (int i = 0; i < state->player_count; i++) {
    state->samus = state->players[i].samus;
    state->controls = MiniControlStateForPlayer(state, i);
    MiniMultiplayerPlayers_LoadRuntime(state, i);
    MiniMultiplayerPlayers_ApplyJoypad(state, i);
    MultiSamus_SetProjectileSpawnPlayer(i);
    MultiSamus_NotifyPlayerInput(i);
    HandleControllerInputForGamePhysics();
    MultiSamus_NotifyPlayerInputEnd(i);
    HandleSamusMovementAndPause();
    state->players[i].samus = MiniMultiplayerPlayers_CoreFromGlobals(state);
    MiniMultiplayerPlayers_SaveRuntime(state, i);
  }
  MultiSamus_SetProjectileSpawnPlayer(-1);
  MiniMultiplayerPlayers_LoadRuntime(state, 0);
  SamusProjectile_EndSharedInputPass();

  MiniMultiplayerPlayers_RunPostMovementChecksForStored(state);
}
