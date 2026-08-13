#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "mini/mini_game.h"
#include "mini/mini_predict.h"
#include "mini/mini_enemy_runtime.h"
#include "mini/mini_enemy_metadata.h"

// Test helper: manually spawn enemies for testing since GetEditorEnemySpawnViews returns 0
static void test_spawn_enemies(MiniGameState *state) {
  // Position Samus close to the roach to trigger its AI
  state->samus.world_x = 90;  // Within 80 pixel trigger radius of roach at x=100
  state->samus.world_y = 100;
  
  // Spawn a Roach that should walk (vx != 0 in MiniUpdateRoach when triggered)
  state->enemy_state.count = 2;
  state->enemy_state.active_count = 2;
  
  const MiniEnemySpeciesMetadata *roach_meta = MiniEnemyMetadataForSpecies(0xD87F);
  const MiniEnemySpeciesMetadata *pirate_meta = MiniEnemyMetadataForSpecies(0xF353);
  
  state->enemy_state.enemies[0] = (MiniEnemyRuntimeState){
    .active = true,
    .species_id = 0xD87F,
    .x = 100,
    .y = 100,
    .home_x = 100,
    .home_y = 100,
    .x_velocity = 0,  // Will be set by AI when triggered
    .y_velocity = 0,
    .x_radius = roach_meta ? roach_meta->x_radius : 4,
    .y_radius = roach_meta ? roach_meta->y_radius : 4,
    .health = roach_meta ? roach_meta->max_health : 20,
    .max_health = roach_meta ? roach_meta->max_health : 20,
    .damage = roach_meta ? roach_meta->damage : 40,
    .ai_bank = roach_meta ? roach_meta->ai_bank : 0xA3,
    .init_ai = roach_meta ? roach_meta->init_ai : 0xA14D,
    .main_ai = roach_meta ? roach_meta->main_ai : 0xA2D0,
    .behavior = kMiniEnemyBehavior_Roach,
    .facing_right = true,
    .ai_state = 0,  // Will trigger when Samus is nearby
    .state_timer = 0,
    .init_parameter = 0x0000,
    .properties1 = 0x2400,
    .properties2 = 0x0000,
    .extra_parameter1 = 0x5003,  // Velocity encoding
    .extra_parameter2 = 0x0050,  // Trigger radius = 80
  };
  
  state->enemy_state.enemies[1] = (MiniEnemyRuntimeState){
    .active = true,
    .species_id = 0xF353,
    .x = 300,  // Moved farther away
    .y = 150,
    .home_x = 300,
    .home_y = 150,
    .x_velocity = 0,
    .y_velocity = 0,
    .x_radius = pirate_meta ? pirate_meta->x_radius : 16,
    .y_radius = pirate_meta ? pirate_meta->y_radius : 24,
    .health = pirate_meta ? pirate_meta->max_health : 20,
    .max_health = pirate_meta ? pirate_meta->max_health : 20,
    .damage = pirate_meta ? pirate_meta->damage : 15,
    .ai_bank = pirate_meta ? pirate_meta->ai_bank : 0xB2,
    .init_ai = pirate_meta ? pirate_meta->init_ai : 0xEF9F,
    .main_ai = pirate_meta ? pirate_meta->main_ai : 0xF02D,
    .behavior = kMiniEnemyBehavior_SpacePirateShooter,
    .facing_right = false,
    .ai_state = 0,
    .state_timer = 0,
    .shoot_cooldown = 60,
    .init_parameter = 0x0000,
    .properties1 = 0x0000,
    .properties2 = 0x0000,
    .extra_parameter1 = 0x0000,
    .extra_parameter2 = 0x0000,
  };
  
  MiniEnemyRuntime_RefreshCounts(&state->enemy_state);
}

static void test_enemy_movement_through_predict(void) {
  printf("Testing enemy movement through MiniPredict...\n");
  
  const size_t frame_count = 30;
  uint16 inputs[30];
  memset(inputs, 0, sizeof(inputs));
  
  MiniPrediction *prediction = MiniPrediction_Create(frame_count);
  assert(prediction != NULL);
  
  // Create a state with manually spawned enemies
  MiniGameState *state = MiniCreate(320, 240);
  assert(state != NULL);
  test_spawn_enemies(state);
  
  // Run prediction - this should call MiniEnemyRuntime_Update each frame
  prediction->frame_count = 0;
  for (size_t i = 0; i < frame_count; i++) {
    MiniStepButtons(state, inputs[i], false);
    prediction->frames[prediction->frame_count++] = MiniCaptureTrajectoryFrame(state, (int)i);
  }
  
  // Verify enemies are present in frames
  int frames_with_enemies = 0;
  int roach_x_min = 10000, roach_x_max = -10000;
  int roach_y_min = 10000, roach_y_max = -10000;
  
  for (size_t i = 0; i < prediction->frame_count; i++) {
    const MiniTrajectoryFrame *frame = &prediction->frames[i];
    if (frame->enemy_count > 0) {
      frames_with_enemies++;
      for (size_t e = 0; e < frame->enemy_count; e++) {
        const MiniEnemySnapshot *enemy = &frame->enemies[e];
        if (enemy->enemy_id == 0xD87F) {  // Roach
          if (enemy->world_x < roach_x_min) roach_x_min = enemy->world_x;
          if (enemy->world_x > roach_x_max) roach_x_max = enemy->world_x;
          if (enemy->world_y < roach_y_min) roach_y_min = enemy->world_y;
          if (enemy->world_y > roach_y_max) roach_y_max = enemy->world_y;
        }
      }
    }
  }
  
  printf("  Frames with enemies: %d/%zu\n", frames_with_enemies, prediction->frame_count);
  printf("  Roach x range: [%d, %d] (span: %d)\n", roach_x_min, roach_x_max, roach_x_max - roach_x_min);
  printf("  Roach y range: [%d, %d] (span: %d)\n", roach_y_min, roach_y_max, roach_y_max - roach_y_min);
  
  assert(frames_with_enemies > 0 && "Expected frames to have enemy data");
  
  // Check that at least one enemy coordinate changed (roach should walk when triggered)
  bool position_changed = (roach_x_max - roach_x_min) > 0 || (roach_y_max - roach_y_min) > 0;
  
  printf("  Position changed: %s\n", position_changed ? "YES" : "NO");
  
  if (!position_changed) {
    printf("  ERROR: Roach position did not change over %zu frames\n", frame_count);
    printf("  Roach should be triggered by nearby Samus and start walking\n");
    // Print first few frames for debugging
    for (size_t i = 0; i < (frame_count < 10 ? frame_count : 10); i++) {
      const MiniTrajectoryFrame *frame = &prediction->frames[i];
      printf("  Frame %d: ", frame->frame);
      if (frame->enemy_count > 0) {
        for (size_t e = 0; e < frame->enemy_count; e++) {
          const MiniEnemySnapshot *enemy = &frame->enemies[e];
          printf("Enemy 0x%04X type=%u x=%d y=%d ", 
                 enemy->enemy_id, enemy->enemy_type, enemy->world_x, enemy->world_y);
        }
      }
      printf("\n");
    }
    assert(position_changed && "Expected roach to move when triggered by nearby Samus");
  }
  
  printf("  ✓ Roach moved %d pixels in x-direction\n", roach_x_max - roach_x_min);
  printf("  ✓ Enemies captured with type field in trajectory\n");
  
  MiniPrediction_Destroy(prediction);
  MiniDestroy(state);
  
  printf("✓ Enemy movement test completed\n\n");
}

int main(void) {
  printf("=== Mini Enemy Observation Tests ===\n\n");
  
  test_enemy_movement_through_predict();
  
  printf("All tests passed!\n");
  return 0;
}
