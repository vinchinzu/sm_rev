#ifndef ENEMY_CONFIG_H
#define ENEMY_CONFIG_H

#include "types.h"

struct EnemyDef;
typedef struct EnemyDef EnemyDef;

void LoadEnemyConfig(void);
void CheckEnemyConfigReload(void);
EnemyDef *GetEnemyDefOverride(uint16 addr);

#endif
