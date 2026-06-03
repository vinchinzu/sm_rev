#ifndef GAMEPLAY_FRAME_H_
#define GAMEPLAY_FRAME_H_

// Shared main-gameplay frame slices extracted from GameState_8_MainGameplay
// (Bank $82, 0x828B44). Mini and the full build call these instead of
// reimplementing the Samus input/movement path.

void GameplayFrame_DetermineEnemies(void);
void GameplayFrame_SamusInputForAllPlayers(void);
void GameplayFrame_SamusMovementForAllPlayers(void);
void GameplayFrame_Animtiles(void);
void GameplayFrame_DecrementSamusTimersForAllPlayers(void);

#endif  // GAMEPLAY_FRAME_H_
