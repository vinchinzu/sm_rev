#ifndef SM_MINI_ENEMY_METADATA_H_
#define SM_MINI_ENEMY_METADATA_H_

#include "mini_asset_bootstrap.h"
#include "mini_enemy.h"
#include "types.h"

typedef struct MiniEnemySpeciesMetadata {
  uint16 species_id;
  const char *canonical_name;
  int max_health;
  int damage;
  int x_radius;
  int y_radius;
  uint8 ai_bank;
  uint16 init_ai;
  uint16 main_ai;
  MiniEnemyBehavior behavior;
} MiniEnemySpeciesMetadata;

typedef struct MiniKnownEnemyPopulation {
  uint16 species_id;
  int x;
  int y;
  uint16 init_parameter;
  uint16 properties1;
  uint16 properties2;
  uint16 extra_parameter1;
  uint16 extra_parameter2;
} MiniKnownEnemyPopulation;

const MiniEnemySpeciesMetadata *MiniEnemyMetadataForSpecies(uint16 species_id);
bool MiniKnownClimbPopulationForSpawn(const MiniEditorEnemySpawnView *spawn,
                                      MiniKnownEnemyPopulation *population);
bool MiniEnemyTakesProjectileDamage(const MiniEnemyRuntimeState *enemy);
bool MiniEnemyDoesTouchDamage(const MiniEnemyRuntimeState *enemy);
const char *MiniEnemyBehaviorName(MiniEnemyBehavior behavior);

#endif  // SM_MINI_ENEMY_METADATA_H_
