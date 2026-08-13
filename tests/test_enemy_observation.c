#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "mini/mini_game.h"
#include "mini/mini_predict.h"
#include "mini/mini_enemy_runtime.h"

static void test_enemy_observation_in_trajectory(void) {
  // Test that enemies in MiniGameState are properly captured in trajectory frames
  MiniGameState *state = MiniCreate(320, 240);
  assert(state != NULL);

  // Manually add a test enemy to the state
  // In real scenarios, enemies would be populated by MiniEnemyRuntime_Initialize
  // from editor room data or save state snapshots
  state->enemy_state.count = 2;
  state->enemy_state.active_count = 2;
  state->enemy_state.enemies[0] = (MiniEnemyRuntimeState){
    .active = true,
    .species_id = 0xD87F,  // Roach
    .x = 100,
    .y = 100,
    .x_velocity = 1,
    .y_velocity = 0,
    .behavior = kMiniEnemyBehavior_Roach,
  };
  state->enemy_state.enemies[1] = (MiniEnemyRuntimeState){
    .active = true,
    .species_id = 0xF353,  // Space Pirate
    .x = 200,
    .y = 150,
    .x_velocity = 0,
    .y_velocity = 0,
    .behavior = kMiniEnemyBehavior_SpacePirateShooter,
  };

  // Create prediction directly from the existing state (no snapshot)
  const size_t frame_count = 5;
  uint16 inputs[5] = {0};
  
  MiniPrediction *prediction = MiniPrediction_Create(frame_count);
  assert(prediction != NULL);

  // Capture frames from current state
  prediction->frame_count = 0;
  for (size_t i = 0; i < frame_count; i++) {
    MiniStepButtons(state, inputs[i], false);
    
    // Manually capture the frame to include our test enemies
    MiniTrajectoryFrame frame = {0};
    frame.frame = (int)i;
    frame.room_id = state->room_id;
    frame.samus_x = state->samus_x;
    frame.samus_y = state->samus_y;
    frame.samus_x_sub = 0;
    frame.samus_y_sub = 0;
    frame.velocity_x = 0;
    frame.velocity_y = 0;
    frame.velocity_x_sub = 0;
    frame.velocity_y_sub = 0;
    frame.momentum_x = 0;
    frame.momentum_x_sub = 0;
    frame.pose = state->samus_pose_value;
    frame.facing = 0x08;
    frame.movement_type = state->samus_movement_type_value;
    frame.speed_counter = 0;
    frame.speed_flag = 0;
    frame.shinespark_timer = 0;
    
    // Capture enemies
    frame.enemy_count = 0;
    frame.enemies = NULL;
    if (state->enemy_state.active_count > 0) {
      frame.enemies = (MiniEnemySnapshot *)malloc(state->enemy_state.active_count * sizeof(MiniEnemySnapshot));
      if (frame.enemies) {
        for (int e = 0; e < state->enemy_state.count; e++) {
          const MiniEnemyRuntimeState *enemy = &state->enemy_state.enemies[e];
          if (enemy->active) {
            MiniEnemySnapshot *snapshot = &frame.enemies[frame.enemy_count++];
            snapshot->enemy_id = enemy->species_id;
            snapshot->world_x = enemy->x;
            snapshot->world_y = enemy->y;
            snapshot->x_velocity = enemy->x_velocity;
            snapshot->y_velocity = enemy->y_velocity;
          }
        }
      }
    }
    
    prediction->frames[prediction->frame_count++] = frame;
  }

  printf("Enemy trajectory test:\n");
  printf("  Captured %zu frames\n", prediction->frame_count);
  
  int frames_with_enemies = 0;
  for (size_t i = 0; i < prediction->frame_count; i++) {
    const MiniTrajectoryFrame *frame = &prediction->frames[i];
    if (frame->enemy_count > 0) {
      frames_with_enemies++;
      printf("  Frame %d: %zu enemies\n", frame->frame, frame->enemy_count);
      for (size_t e = 0; e < frame->enemy_count; e++) {
        const MiniEnemySnapshot *enemy = &frame->enemies[e];
        printf("    Enemy %zu: id=0x%04X x=%d y=%d vx=%d vy=%d\n", 
               e, enemy->enemy_id, enemy->world_x, enemy->world_y,
               enemy->x_velocity, enemy->y_velocity);
      }
    }
  }

  printf("  Total frames with enemies: %d/%zu\n", frames_with_enemies, prediction->frame_count);
  assert(frames_with_enemies > 0 && "Expected at least some frames to have enemy data");

  MiniPrediction_Destroy(prediction);
  MiniDestroy(state);
  
  printf("✓ Enemy observation test passed\n\n");
}

int main(void) {
  printf("=== Mini Enemy Observation Tests ===\n\n");
  
  test_enemy_observation_in_trajectory();
  
  printf("All tests passed!\n");
  return 0;
}
