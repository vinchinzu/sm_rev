#include <stdio.h>
#include <assert.h>

#include "mini/mini_game.h"
#include "mini/mini_enemy_runtime.h"

// Quick diagnostic to verify enemy runtime is hooked up
int main(void) {
  printf("Testing enemy runtime hookup...\n");
  
  MiniGameState *state = MiniCreate(320, 240);
  assert(state != NULL);
  
  MiniEnemyPolicy policy = MiniEnemyRuntime_PolicyForState(state);
  printf("Policy: %d (0=None, 1=RomOwned, 2=MiniSim)\n", policy);
  printf("uses_original_gameplay_runtime: %d\n", state->room.uses_original_gameplay_runtime);
  
  if (policy != kMiniEnemyPolicy_MiniSim) {
    printf("ERROR: Policy is not MiniSim, enemy runtime will not run\n");
    MiniDestroy(state);
    return 1;
  }
  
  // Manually spawn a roach enemy
  state->enemy_state.count = 1;
  state->enemy_state.active_count = 1;
  state->enemy_state.enemies[0] = (MiniEnemyRuntimeState){
    .active = true,
    .species_id = 0xD87F,
    .x = 100,
    .y = 100,
    .x_velocity = 5,
    .y_velocity = 0,
    .behavior = kMiniEnemyBehavior_Roach,
    .ai_state = 1,  // Already triggered
    .extra_parameter2 = 0x0050,
  };
  
  printf("Initial: enemy x=%d, ai_state=%d\n", 
         state->enemy_state.enemies[0].x, state->enemy_state.enemies[0].ai_state);
  
  // Step a few frames
  for (int i = 0; i < 10; i++) {
    MiniStepButtons(state, 0, false);
  }
  
  printf("After 10 frames: enemy x=%d\n", state->enemy_state.enemies[0].x);
  
  // Roach should have moved if AI is working
  bool moved = state->enemy_state.enemies[0].x != 100;
  printf("Enemy moved: %s\n", moved ? "YES" : "NO");
  
  if (!moved) {
    printf("ERROR: Roach AI not moving enemy\n");
    return 1;
  }
  
  printf("✓ Enemy runtime is working\n");
  MiniDestroy(state);
  return 0;
}
