// Enemies
#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "enemy_types.h"

void EnemyCollisionHandler(void) {  // 0xA09758
  if ((gEnemyData(cur_enemy_index)->extra_properties & kEnemyExtraProps_MultiHitbox) != 0) {
    EprojCollHandler_Multibox();
    EnemyBombCollHandler_Multibox();
    EnemySamusCollHandler_Multibox();
  } else {
    EprojCollHandler();
    EnemyBombCollHandler();
    EnemySamusCollHandler();
  }
}

void func_nullsub_4(void) {
  ;
}

uint16 SuitDamageDivision(uint16 a) {  // 0xA0A45E
  if ((equipped_items & 0x20) != 0)
    return a >> 2;
  if (equipped_items & 1)
    return a >> 1;
  return a;
}
