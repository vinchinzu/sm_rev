#include "gameplay_frame.h"

#include "funcs.h"
#include "multi_samus.h"
#include "samus_projectile.h"
#include "variables.h"

void GameplayFrame_DetermineEnemies(void) {  // GameState_8 @ 0x828B44
  DetermineWhichEnemiesToProcess();
}

void GameplayFrame_SamusInputForAllPlayers(void) {  // GameState_8 @ 0x828B44
  int player_count = MultiSamus_GetNumSamus();
  if (player_count > 1)
    SamusProjectile_BeginSharedInputPass();
  for (int i = 0; i < player_count; i++) {
    MultiSamus_Switch(i);
    MultiSamus_NotifyPlayerInput(i);
    HandleControllerInputForGamePhysics();
    MultiSamus_NotifyPlayerInputEnd(i);
    if (!debug_disable_sprite_interact)
      SamusProjectileInteractionHandler();
  }
  MultiSamus_Switch(0);
  if (player_count > 1)
    SamusProjectile_EndSharedInputPass();
}

void GameplayFrame_SamusMovementForAllPlayers(void) {  // GameState_8 @ 0x828B44
  for (int i = 0, n = MultiSamus_GetNumSamus(); i < n; i++) {
    MultiSamus_Switch(i);
    HandleSamusMovementAndPause();
  }
  MultiSamus_Switch(0);
}

void GameplayFrame_Animtiles(void) {  // GameState_8 @ 0x828B44
  AnimtilesHandler();
}

void GameplayFrame_DecrementSamusTimersForAllPlayers(void) {  // GameState_8 @ 0x828B44
  for (int i = 0, n = MultiSamus_GetNumSamus(); i < n; i++) {
    MultiSamus_Switch(i);
    DecrementSamusTimers();
  }
  MultiSamus_Switch(0);
}
