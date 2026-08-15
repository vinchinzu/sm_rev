#include "mini_predict.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mini_game.h"
#include "mini_room_adapter.h"
#include "mini_wram_peek.h"
#include "variables.h"
#include "wram_obs.h"

MiniTrajectoryFrame MiniCaptureTrajectoryFrame(const MiniGameState *state, int frame) {
  MiniTrajectoryFrame result = {0};
  WramSamusObs obs = Wram_PeekSamus();
  result.frame = frame;
  result.room_id = obs.room != 0 ? obs.room : state->room.room_id;
  result.samus_x = samus_x_pos;
  result.samus_y = samus_y_pos;
  result.samus_x_sub = samus_x_subpos;
  result.samus_y_sub = samus_y_subpos;
  result.velocity_x = (int16)samus_x_base_speed;
  result.velocity_y = (int16)samus_y_speed;
  result.velocity_x_sub = (int16)samus_x_base_subspeed;
  result.velocity_y_sub = (int16)samus_y_subspeed;
  result.momentum_x = (int16)samus_x_extra_run_speed;
  result.momentum_x_sub = (int16)samus_x_extra_run_subspeed;
  result.pose = samus_pose;
  result.facing = obs.facing != 0 ? obs.facing : 0x08;
  result.movement_type = samus_movement_type;
  result.speed_counter = speed_boost_counter;
  result.speed_flag = samus_has_momentum_flag ? 1 : 0;
  result.shinespark_timer = 0;
  
  // Energy: read from g_ram $09C2
  result.energy = samus_health;                // g_ram $09C2
  
  // Death and game over state: not yet implemented in Mini
  // These fields are NOT emitted in JSON output (see predict_cli.c)
  result.is_dead = false;
  result.is_game_over = false;
  
  // Frame counters: read from g_ram (both uint16 per locked M-E map)
  result.frame_counter_1 = *(uint16*)(g_ram + kWramAddr_FrameCounter1);
  result.frame_counter_2 = *(uint16*)(g_ram + kWramAddr_FrameCounter2);
  
  // Enemy tracking: capture active enemies from MiniSim (pixel x/y only)
  result.enemy_count = 0;
  result.enemies = NULL;
  
  if (state->enemy_state.active_count > 0) {
    result.enemies = (MiniEnemySnapshot *)malloc(state->enemy_state.active_count * sizeof(MiniEnemySnapshot));
    if (result.enemies) {
      for (int i = 0; i < state->enemy_state.count; i++) {
        const MiniEnemyRuntimeState *enemy = &state->enemy_state.enemies[i];
        if (enemy->active) {
          MiniEnemySnapshot *snapshot = &result.enemies[result.enemy_count++];
          snapshot->enemy_id = enemy->species_id;
          snapshot->enemy_type = enemy->behavior;
          snapshot->world_x = enemy->x;
          snapshot->world_y = enemy->y;
          snapshot->x_velocity = enemy->x_velocity;
          snapshot->y_velocity = enemy->y_velocity;
        }
      }
    }
  }
  
  return result;
}

MiniPrediction *MiniPrediction_Create(size_t capacity) {
  MiniPrediction *prediction = (MiniPrediction *)malloc(sizeof(MiniPrediction));
  if (!prediction)
    return NULL;

  prediction->frames = (MiniTrajectoryFrame *)malloc(capacity * sizeof(MiniTrajectoryFrame));
  if (!prediction->frames) {
    free(prediction);
    return NULL;
  }

  prediction->frame_count = 0;
  prediction->capacity = capacity;
  return prediction;
}

void MiniPrediction_Destroy(MiniPrediction *prediction) {
  if (!prediction)
    return;
  // Free enemy snapshots for each frame
  for (size_t i = 0; i < prediction->frame_count; i++) {
    if (prediction->frames[i].enemies) {
      free(prediction->frames[i].enemies);
      prediction->frames[i].enemies = NULL;
    }
  }
  free(prediction->frames);
  free(prediction);
}

bool MiniPredict(MiniPrediction *prediction,
                 const void *state_snapshot,
                 size_t snapshot_size,
                 const uint16 *input_buttons,
                 size_t input_count,
                 int viewport_width,
                 int viewport_height) {
  if (!prediction || !input_buttons || input_count == 0)
    return false;

  // When loading a snapshot, we capture frame 0 BEFORE stepping (pre-step state),
  // then frames 1..N after each input. Without a snapshot, we only capture post-step frames.
  size_t required_capacity = (state_snapshot != NULL) ? (input_count + 1) : input_count;
  if (required_capacity > prediction->capacity)
    return false;

  MiniGameState *state = NULL;
  bool created_state = false;

  prediction->frame_count = 0;

  if (state_snapshot != NULL && snapshot_size > 0) {
    state = MiniCreate(viewport_width, viewport_height);
    if (!state)
      return false;
    created_state = true;
    if (!MiniLoadState(state, state_snapshot, snapshot_size)) {
      MiniDestroy(state);
      return false;
    }
    
    // REQUIRED A: Capture frame 0 BEFORE any MiniStepButtons.
    // This is the loaded state from g_ram, before any simulation steps.
    prediction->frames[prediction->frame_count++] = MiniCaptureTrajectoryFrame(state, 0);
  } else {
    state = MiniCreate(viewport_width, viewport_height);
    if (!state)
      return false;
    created_state = true;
  }

  // Step through inputs and capture post-step frames.
  // For snapshot loads: frames[1..N] are post-step for inputs[0..N-1]
  // Without snapshot: frames[0..N-1] are post-step for inputs[0..N-1]
  for (size_t i = 0; i < input_count; i++) {
    MiniStepButtons(state, input_buttons[i], false);
    int frame_num = (state_snapshot != NULL) ? (int)(i + 1) : (int)i;
    prediction->frames[prediction->frame_count++] = MiniCaptureTrajectoryFrame(state, frame_num);
  }

  if (created_state)
    MiniDestroy(state);

  return true;
}
