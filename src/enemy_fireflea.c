// Enemy AI - Fireflea — peeled from Bank $A3
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

#define g_word_A38D1D ((uint16*)RomFixedPtr(0xa38d1d))

void Fireflea_Init(void) {  // 0xA38D2D
  Enemy_FireFlea *E = Get_FireFlea(cur_enemy_index);
  E->base.current_instruction = addr_kFireflea_Ilist_8C2F;
  if ((E->ffa_parameter_1 & 2) != 0) {
    Fireflea_Func_1(cur_enemy_index);
    Fireflea_Func_2(cur_enemy_index);
    Fireflea_Func_3(cur_enemy_index);
    Fireflea_Func_4(cur_enemy_index);
    Fireflea_Func_5(cur_enemy_index);
  } else {
    Fireflea_Func_3(cur_enemy_index);
    Fireflea_Func_4(cur_enemy_index);
    Fireflea_Func_6(cur_enemy_index);
  }
}

void Fireflea_Func_1(uint16 k) {  // 0xA38D5D
  Enemy_FireFlea *E = Get_FireFlea(k);
  E->ffa_var_E = E->base.x_pos;
  E->ffa_var_F = E->base.y_pos;
}

void Fireflea_Func_2(uint16 k) {  // 0xA38D6A
  Enemy_FireFlea *E = Get_FireFlea(k);
  E->ffa_var_D = HIBYTE(E->ffa_parameter_1) << 8;
}

void Fireflea_Func_3(uint16 k) {  // 0xA38D75
  Enemy_FireFlea *E = Get_FireFlea(k);
  uint16 v2 = 8 * LOBYTE(E->ffa_parameter_2);
  if ((E->ffa_parameter_1 & 1) == 0)
    v2 += 4;
  E->ffa_var_02 = v2;
  int v3 = v2 >> 1;
  E->ffa_var_B = kCommonEnemySpeeds_Linear[v3];
  E->ffa_var_A = kCommonEnemySpeeds_Linear[v3 + 1];
}

void Fireflea_Func_4(uint16 k) {  // 0xA38D9C
  Enemy_FireFlea *E = Get_FireFlea(k);
  E->ffa_var_C = LOBYTE(g_word_A38D1D[HIBYTE(E->ffa_parameter_2)]);
}

void Fireflea_Func_5(uint16 k) {  // 0xA38DAE
  Enemy_FireFlea *E = Get_FireFlea(k);
  E->base.x_pos = E->ffa_var_E + CosineMult8bit(E->ffa_var_D, E->ffa_var_C);
  E->base.y_pos = E->ffa_var_F + SineMult8bit(E->ffa_var_D, E->ffa_var_C);
}

void Fireflea_Func_6(uint16 k) {  // 0xA38DD7
  Enemy_FireFlea *E = Get_FireFlea(k);
  E->ffa_var_00 = E->base.y_pos - E->ffa_var_C;
  E->ffa_var_01 = E->ffa_var_C + E->base.y_pos;
}

void Fireflea_Main(void) {  // 0xA38DEE
  Enemy_FireFlea *E = Get_FireFlea(cur_enemy_index);
  if ((E->ffa_parameter_1 & 2) != 0) {
    E->base.x_pos = E->ffa_var_E + CosineMult8bit(HIBYTE(E->ffa_var_D), E->ffa_var_C);
    E->base.y_pos = E->ffa_var_F + SineMult8bit(HIBYTE(E->ffa_var_D), E->ffa_var_C);
    E->ffa_var_D += *(uint16 *)((uint8 *)&E->ffa_var_A + 1);
  } else {
    int v2 = E->ffa_var_02 >> 1;
    AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(kCommonEnemySpeeds_Linear[v2], kCommonEnemySpeeds_Linear[v2 + 1]));
    if ((int16)(E->base.y_pos - E->ffa_var_00) < 0
        || (int16)(E->base.y_pos - E->ffa_var_01) >= 0) {
      E->ffa_var_02 ^= 4;
    }
  }
}

void Fireflea_Touch(uint16 k) {  // 0xA38E6B
  uint16 v1 = 0;

  NormalEnemyTouchAi();
  EnemyDeathAnimation(k, v1);
  if (sign16(fireflea_darkness_level - 12))
    fireflea_darkness_level += 2;
}

void Fireflea_Powerbomb(void) {  // 0xA38E83
  NormalEnemyPowerBombAi();
  Fireflea_Common();
}

void Fireflea_Shot(void) {  // 0xA38E89
  NormalEnemyShotAi();
  Fireflea_Common();
}

void Fireflea_Common(void) {  // 0xA38E8D
  if (!Get_FireFlea(cur_enemy_index)->base.health) {
    if (sign16(fireflea_darkness_level - 12))
      fireflea_darkness_level += 2;
  }
}
