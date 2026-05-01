#ifndef SM_ENEMY_TORIZO_ATTACKS_H_
#define SM_ENEMY_TORIZO_ATTACKS_H_

#include "types.h"

struct Enemy_Torizo;

void TorizoAttacks_RunScheduledOpeningChozoOrbWaves(uint16 k, struct Enemy_Torizo *E);
bool TorizoAttacks_TryStartBombOpeningChozoOrbAttack(uint16 k, struct Enemy_Torizo *E);

#endif  // SM_ENEMY_TORIZO_ATTACKS_H_
