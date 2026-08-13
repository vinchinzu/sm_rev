#include "mini_predict.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mini_game.h"
#include "mini_room_adapter.h"
#include "mini_wram_peek.h"
#include "variables.h"

MiniTrajectoryFrame MiniCaptureTrajectoryFrame(const MiniGameState *state, int frame) {
  MiniTrajectoryFrame result = {0};
  result.frame = frame;
  result.room_id = state->room.room_id;
  
  // Position: read from g_ram to ensure we get the authoritative post-MiniLoadState values
  // These include subpixels that public Mini fields don't expose
  result.samus_x = samus_x_pos;           // g_ram $0AF6
  result.samus_y = samus_y_pos;           // g_ram $0AFA
  result.samus_x_sub = samus_x_subpos;    // g_ram $0AF8
  result.samus_y_sub = samus_y_subpos;    // g_ram $0AFC
  
  // Velocity (mini tracks pixel velocity, subpixel always zero for authored movement)
  result.velocity_x = state->samus.x_velocity;
  result.velocity_y = state->samus.y_velocity;
  result.velocity_x_sub = 0;  // Authored movement doesn't track subpixel velocity
  result.velocity_y_sub = 0;
  
  // Momentum (speed booster - not in mini authored movement)
  result.momentum_x = 0;
  result.momentum_x_sub = 0;
  
  // Pose and movement: read from g_ram for post-MiniLoadState correctness
  result.pose = samus_pose;                    // g_ram $0A1C
  result.facing = (state->samus.x_velocity < 0) ? 0x04 : 0x08;
  result.movement_type = state->samus.movement_type;
  
  // Speed booster state (not in mini)
  result.speed_counter = 0;
  result.speed_flag = 0;
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

  if (input_count > prediction->capacity)
    return false;

  MiniGameState *state = NULL;
  bool created_state = false;

  if (state_snapshot != NULL && snapshot_size > 0) {
    state = MiniCreate(viewport_width, viewport_height);
    if (!state)
      return false;
    created_state = true;
    if (!MiniLoadState(state, state_snapshot, snapshot_size)) {
      MiniDestroy(state);
      return false;
    }
  } else {
    state = MiniCreate(viewport_width, viewport_height);
    if (!state)
      return false;
    created_state = true;
  }

  prediction->frame_count = 0;
  for (size_t i = 0; i < input_count; i++) {
    MiniStepButtons(state, input_buttons[i], false);
    prediction->frames[prediction->frame_count++] = MiniCaptureTrajectoryFrame(state, (int)i);
  }

  if (created_state)
    MiniDestroy(state);

  return true;
}
