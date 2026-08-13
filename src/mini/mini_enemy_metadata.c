#include "mini_enemy_metadata.h"

#include <stddef.h>

static const MiniEnemySpeciesMetadata kMiniEnemySpeciesMetadata[] = {
  {
    .species_id = kMiniEnemySpecies_Roach,
    .canonical_name = "Roach",
    .max_health = 20,
    .damage = 40,
    .x_radius = 4,
    .y_radius = 4,
    .ai_bank = 0xA3,
    .init_ai = 0xA14D,
    .main_ai = 0xA2D0,
    .behavior = kMiniEnemyBehavior_Roach,
  },
  {
    .species_id = kMiniEnemySpecies_SpacePirate,
    .canonical_name = "Space Pirate",
    .max_health = 20,
    .damage = 15,
    .x_radius = 16,
    .y_radius = 24,
    .ai_bank = 0xB2,
    .init_ai = 0xEF9F,
    .main_ai = 0xF02D,
    .behavior = kMiniEnemyBehavior_SpacePirateShooter,
  },
};

static const MiniKnownEnemyPopulation kMiniKnownClimbEnemyPopulation[] = {
  {kMiniEnemySpecies_Roach, 276, 76, 0x0000, 0x2400, 0x0000, 0x5003, 0x0050},
  {kMiniEnemySpecies_Roach, 272, 88, 0x0000, 0x2400, 0x0000, 0x9002, 0x0050},
  {kMiniEnemySpecies_Roach, 269, 114, 0x0000, 0x2400, 0x0000, 0xAC03, 0x0050},
  {kMiniEnemySpecies_Roach, 491, 150, 0x0000, 0x2400, 0x0000, 0xC804, 0x0050},
  {kMiniEnemySpecies_Roach, 499, 154, 0x0000, 0x2400, 0x0000, 0xC303, 0x0050},
  {kMiniEnemySpecies_Roach, 277, 294, 0x0000, 0x2400, 0x0000, 0x9203, 0x0050},
  {kMiniEnemySpecies_Roach, 276, 291, 0x0000, 0x2400, 0x0000, 0x6003, 0x0050},
  {kMiniEnemySpecies_Roach, 273, 296, 0x0000, 0x2400, 0x0000, 0x9C02, 0x0050},
  {kMiniEnemySpecies_Roach, 494, 535, 0x0000, 0x2400, 0x0000, 0xF004, 0x0050},
  {kMiniEnemySpecies_Roach, 278, 1721, 0x0000, 0x2400, 0x0000, 0xBC02, 0x0050},
};

const MiniEnemySpeciesMetadata *MiniEnemyMetadataForSpecies(uint16 species_id) {
  for (size_t i = 0; i < sizeof(kMiniEnemySpeciesMetadata) / sizeof(kMiniEnemySpeciesMetadata[0]); i++) {
    if (kMiniEnemySpeciesMetadata[i].species_id == species_id)
      return &kMiniEnemySpeciesMetadata[i];
  }
  return NULL;
}

bool MiniKnownClimbPopulationForSpawn(const MiniEditorEnemySpawnView *spawn,
                                      MiniKnownEnemyPopulation *population) {
  if (spawn == NULL || population == NULL)
    return false;
  for (size_t i = 0; i < sizeof(kMiniKnownClimbEnemyPopulation) / sizeof(kMiniKnownClimbEnemyPopulation[0]); i++) {
    const MiniKnownEnemyPopulation *candidate = &kMiniKnownClimbEnemyPopulation[i];
    if (candidate->species_id == spawn->species_id &&
        candidate->x == spawn->x_pos &&
        candidate->y == spawn->y_pos) {
      *population = *candidate;
      return true;
    }
  }
  return false;
}

bool MiniEnemyTakesProjectileDamage(const MiniEnemyRuntimeState *enemy) {
  return enemy->behavior == kMiniEnemyBehavior_Roach ||
         enemy->behavior == kMiniEnemyBehavior_SpacePirateShooter;
}

bool MiniEnemyDoesTouchDamage(const MiniEnemyRuntimeState *enemy) {
  return enemy->behavior == kMiniEnemyBehavior_Roach ||
         enemy->behavior == kMiniEnemyBehavior_SpacePirateShooter;
}

const char *MiniEnemyBehaviorName(MiniEnemyBehavior behavior) {
  switch (behavior) {
  case kMiniEnemyBehavior_Passive: return "passive";
  case kMiniEnemyBehavior_Roach: return "roach";
  case kMiniEnemyBehavior_SpacePirateShooter: return "space_pirate_shooter";
  default: return "unknown";
  }
}
