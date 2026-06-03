#ifndef SM_MINI_ENEMY_H_
#define SM_MINI_ENEMY_H_

#include "types.h"

enum {
  kMiniEnemyCapacity = 16,
  kMiniEnemyShotCapacity = 16,
  kMiniEnemyNameCapacity = 64,
  kMiniEnemySourceLabelCapacity = 64,
  kMiniEnemySpecies_Roach = 0xD87F,
};

typedef enum MiniEnemyBehavior {
  kMiniEnemyBehavior_Passive = 0,
  kMiniEnemyBehavior_Roach = 1,
  kMiniEnemyBehavior_SpacePirateShooter = 2,
} MiniEnemyBehavior;

typedef struct MiniEnemyRuntimeState {
  bool active;
  uint16 species_id;
  char name[kMiniEnemyNameCapacity];
  char source_label[kMiniEnemySourceLabelCapacity];
  uint16 init_parameter;
  uint16 properties1;
  uint16 properties2;
  uint16 extra_parameter1;
  uint16 extra_parameter2;
  uint8 ai_bank;
  uint16 init_ai;
  uint16 main_ai;
  int x;
  int y;
  int home_x;
  int home_y;
  int x_velocity;
  int y_velocity;
  int x_radius;
  int y_radius;
  int health;
  int max_health;
  int damage;
  int shoot_cooldown;
  int invulnerable_frames;
  uint16 ai_state;
  uint16 state_timer;
  uint16 hit_count;
  bool facing_right;
  bool has_sprite_assets;
  MiniEnemyBehavior behavior;
} MiniEnemyRuntimeState;

typedef struct MiniEnemyShotState {
  bool active;
  int x;
  int y;
  int x_velocity;
  int y_velocity;
  int radius;
  int damage;
} MiniEnemyShotState;

typedef struct MiniEnemyState {
  int count;
  int active_count;
  int passive_count;
  int renderable_count;
  int shot_count;
  uint16 defeated_count;
  MiniEnemyRuntimeState enemies[kMiniEnemyCapacity];
  MiniEnemyShotState shots[kMiniEnemyShotCapacity];
} MiniEnemyState;

enum {
  kMiniPirateCapacity = kMiniEnemyCapacity,
  kMiniPirateShotCapacity = kMiniEnemyShotCapacity,
};

typedef MiniEnemyRuntimeState MiniPirateEnemyState;
typedef MiniEnemyShotState MiniPirateShotState;
typedef MiniEnemyState MiniPirateState;

#endif  // SM_MINI_ENEMY_H_
