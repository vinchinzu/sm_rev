// NorfairErraticFireball / NorfairLavajumpingEnemy extracted from sm_a2.c.
#include "ida_types.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "variables.h"
#include "enemy_ai_canon.h"

#define g_word_A2BE86 ((uint16*)RomFixedPtr(0xa2be86))


void NorfairErraticFireball_Init(void) {  // 0xA2B3E0
  Enemy_NorfairErraticFireball *E = Get_NorfairErraticFireball(cur_enemy_index);
  Point32 pt = ConvertAngleToXy(E->nefl_parameter_1, LOBYTE(E->nefl_parameter_2));
  E->nefl_var_C = pt.x >> 16;
  E->nefl_var_D = pt.x;
  E->nefl_var_E = pt.y >> 16;
  E->nefl_var_F = pt.y;
  E->base.current_instruction = addr_kNorfairErraticFireball_Ilist_B2DC;
}

void NorfairErraticFireball_Main(void) {  // 0xA2B40F
  Enemy_NorfairErraticFireball *E = Get_NorfairErraticFireball(cur_enemy_index);
  Point32 pt = ConvertAngleToXy(E->nefl_parameter_1, LOBYTE(E->nefl_parameter_2));
  E->nefl_var_C = pt.x >> 16;
  E->nefl_var_D = pt.x;
  E->nefl_var_E = pt.y >> 16;
  E->nefl_var_F = pt.y;
  if (((E->nefl_parameter_1 + 64) & 0x80) == 0) {
    E->nefl_var_C = ~E->nefl_var_C;
    E->nefl_var_D = -E->nefl_var_D;
  }
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, __PAIR32__(E->nefl_var_C, E->nefl_var_D))) {
    E->nefl_parameter_1 ^= 0x40;
  } else {
    if (((E->nefl_parameter_1 + 128) & 0x80) == 0) {
      E->nefl_var_E = ~E->nefl_var_E;
      E->nefl_var_F = -E->nefl_var_F;
    }
    if (Enemy_MoveDown(cur_enemy_index, __PAIR32__(E->nefl_var_E, E->nefl_var_F)))
      E->nefl_parameter_1 ^= 0xC0;
  }
}

const uint16 *NorfairLavajumpingEnemy_Instr_BE8E(uint16 k, const uint16 *jp) {  // 0xA2BE8E
  Get_NorfairLavajumpingEnemy(cur_enemy_index)->nley_var_00 = 1;
  return jp;
}

void NorfairLavajumpingEnemy_Init(void) {  // 0xA2BE99
  Enemy_NorfairLavajumpingEnemy *E = Get_NorfairLavajumpingEnemy(cur_enemy_index);
  E->nley_var_00 = 0;
  E->nley_var_01 = 0;
  if ((E->nley_parameter_1 & 0x8000) != 0) {
    E->base.current_instruction = addr_kNorfairLavajumpingEnemy_Ilist_BE62;
    E->nley_var_F = FUNC16(NorfairLavajumpingEnemy_Func_1);
  } else {
    E->nley_var_D = E->base.x_pos;
    E->nley_var_E = E->base.y_pos;
    E->base.current_instruction = addr_kNorfairLavajumpingEnemy_Ilist_BE3C;
    E->nley_var_F = FUNC16(NorfairLavajumpingEnemy_Func_2);
  }
}

void NorfairLavajumpingEnemy_Main(void) {  // 0xA2BED2
  NextRandom();
  Enemy_NorfairLavajumpingEnemy *E = Get_NorfairLavajumpingEnemy(cur_enemy_index);
  EnemyRunPreInstr(E->nley_var_F);
}

void NorfairLavajumpingEnemy_Func_1(uint16 k) {  // 0xA2BEDC
  Enemy_NorfairLavajumpingEnemy *E = Get_NorfairLavajumpingEnemy(k);
  int v1 = k >> 1;
  if (enemy_drawing_queue[v1 + 100]) {
    uint16 v3 = enemy_drawing_queue[v1 + 109];
    E->base.frozen_timer = v3;
    if (v3 || (enemy_drawing_queue_sizes[v1 + 2] & 0x8000) == 0) {
      E->base.properties |= kEnemyProps_Invisible;
    } else {
      E->base.properties &= ~kEnemyProps_Invisible;
      E->base.y_pos = enemy_drawing_queue[v1 + 93];
    }
  } else {
    E->base.properties |= kEnemyProps_Deleted;
  }
}

void NorfairLavajumpingEnemy_Func_2(uint16 k) {  // 0xA2BF1A
  Enemy_NorfairLavajumpingEnemy *E = Get_NorfairLavajumpingEnemy(k);
  E->nley_var_C = g_word_A2BE86[(uint16)(HIBYTE(random_number) & 6) >> 1];
  E->nley_var_F = FUNC16(NorfairLavajumpingEnemy_Func_3);
  E->base.properties |= kEnemyProps_ProcessedOffscreen;
  QueueSfx2_Max6(0xD);
}

void NorfairLavajumpingEnemy_Func_3(uint16 k) {  // 0xA2BF3E
  Enemy_NorfairLavajumpingEnemy *E = Get_NorfairLavajumpingEnemy(k);
  int t = E->base.y_subpos + (LOBYTE(E->nley_var_C) << 8);
  E->base.y_subpos = t;
  E->base.y_pos += (int8)HIBYTE(E->nley_var_C) + (t >> 16);
  uint16 v3 = E->nley_var_C + 56;
  E->nley_var_C = v3;
  if (v3 >= 0xFC00) {
    NorfairLavajumpingEnemy_Func_6(addr_kNorfairLavajumpingEnemy_Ilist_BE42);
    E->nley_var_F = FUNC16(NorfairLavajumpingEnemy_Func_4);
  }
}

void NorfairLavajumpingEnemy_Func_4(uint16 k) {  // 0xA2BF7C
  Enemy_NorfairLavajumpingEnemy *E = Get_NorfairLavajumpingEnemy(k);
  int t = E->base.y_subpos + (LOBYTE(E->nley_var_C) << 8);
  E->base.y_subpos = t;
  E->base.y_pos += (int8)HIBYTE(E->nley_var_C) + (t >> 16);
  E->nley_var_C += 56;
  if (E->nley_var_00) {
    E->nley_var_00 = 0;
    E->nley_var_F = FUNC16(NorfairLavajumpingEnemy_Func_5);
  }
}

void NorfairLavajumpingEnemy_Func_5(uint16 k) {  // 0xA2BFBC
  Enemy_NorfairLavajumpingEnemy *E = Get_NorfairLavajumpingEnemy(k);
  int t = E->base.y_subpos + (LOBYTE(E->nley_var_C) << 8);
  E->base.y_subpos = t;
  E->base.y_pos = E->base.y_pos + (int8)HIBYTE(E->nley_var_C) + (t >> 16);
  if ((E->base.y_pos & 0xF0) == 240) {
    E->base.x_pos = E->nley_var_D;
    E->base.y_pos = E->nley_var_E;
    NorfairLavajumpingEnemy_Func_6(addr_kNorfairLavajumpingEnemy_Ilist_BE3C);
    E->nley_var_F = FUNC16(NorfairLavajumpingEnemy_Func_2);
    E->base.properties &= ~kEnemyProps_ProcessedOffscreen;
  } else {
    E->nley_var_C += 56;
  }
}

void NorfairLavajumpingEnemy_Func_6(uint16 a) {  // 0xA2C012
  Enemy_NorfairLavajumpingEnemy *E = Get_NorfairLavajumpingEnemy(cur_enemy_index);
  if (a != E->nley_var_01) {
    E->nley_var_01 = a;
    E->base.current_instruction = a;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
  }
}
