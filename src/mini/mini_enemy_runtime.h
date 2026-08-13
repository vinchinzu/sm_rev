#ifndef SM_MINI_ENEMY_RUNTIME_H_
#define SM_MINI_ENEMY_RUNTIME_H_

#include "mini_game.h"

typedef enum MiniEnemyPolicy {
  kMiniEnemyPolicy_None = 0,
  kMiniEnemyPolicy_RomOwned = 1,
  kMiniEnemyPolicy_MiniSim = 2,
} MiniEnemyPolicy;

typedef struct MiniEnemyTelemetry {
  int count;
  int active_count;
  int passive_count;
  int renderable_count;
  int shot_count;
  uint16 defeated_count;
  bool has_first_enemy;
  MiniEnemyRuntimeState first_enemy;
  int pirate_count;
  int pirate_active_count;
  bool has_first_pirate;
  MiniEnemyRuntimeState first_pirate;
} MiniEnemyTelemetry;

MiniEnemyPolicy MiniEnemyRuntime_PolicyForState(const MiniGameState *state);
void MiniEnemyRuntime_Initialize(MiniGameState *state);
void MiniEnemyRuntime_Update(MiniGameState *state);
void MiniEnemyRuntime_RefreshCounts(MiniEnemyState *enemy_state);
void MiniEnemyRuntime_BuildTelemetry(const MiniGameState *state,
                                     MiniEnemyTelemetry *telemetry);

#endif  // SM_MINI_ENEMY_RUNTIME_H_
