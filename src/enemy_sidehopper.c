// Enemy AI - Sidehopper — peeled from Bank $A3
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

#define g_off_A3AAC2 ((uint16*)RomFixedPtr(0xa3aac2))
#define g_off_A3AACA ((uint16*)RomFixedPtr(0xa3aaca))
#define g_off_A3AAD2 ((uint16*)RomFixedPtr(0xa3aad2))
#define g_off_A3AADA ((uint16*)RomFixedPtr(0xa3aada))
#define g_off_A3AAE2 ((uint16*)RomFixedPtr(0xa3aae2))
#define g_word_A3AAE6 ((uint16*)RomFixedPtr(0xa3aae6))
#define g_word_A3AAEA ((uint16*)RomFixedPtr(0xa3aaea))
#define g_word_A3AAEE ((uint16*)RomFixedPtr(0xa3aaee))
#define g_word_A3AAF2 ((uint16*)RomFixedPtr(0xa3aaf2))
#define g_word_A3AAF6 ((uint16*)RomFixedPtr(0xa3aaf6))
#define g_word_A3AAFA ((uint16*)RomFixedPtr(0xa3aafa))

static void CallSidehopperFunc(uint32 ea);

const uint16 *Sidehopper_Func_1(uint16 k, const uint16 *jp) {  // 0xA3AA68
  QueueSfx2_Max3(*jp);
  return jp + 1;
}

const uint16 *Sidehopper_Instr_1(uint16 k, const uint16 *jp) {  // 0xA3AAFE
  Get_Sidehopper(cur_enemy_index)->sideh_var_04 = 1;
  return jp;
}

void Sidehopper_Init(void) {  // 0xA3AB09
  random_number = 37;
  NextRandom();
  Enemy_Sidehopper *E = Get_Sidehopper(cur_enemy_index);
  E->sideh_var_03 = 0;
  E->sideh_var_04 = 0;
  E->sideh_var_05 = 0;
  uint16 v2 = 2 * get_EnemyDef_A2(E->base.enemy_ptr)->field_2A;
  E->sideh_var_06 = v2;
  uint16 v4;
  if (E->sideh_parameter_1)
    v4 = g_off_A3AACA[E->sideh_var_06 >> 1];
  else
    v4 = g_off_A3AAC2[E->sideh_var_06 >> 1];
  E->sideh_var_00 = v4;
  Sidehopper_Func_3();
  if (get_EnemyDef_A2(E->base.enemy_ptr)->field_2A)
    E->sideh_var_05 = 2;
  int v6 = E->sideh_var_05 >> 1;
  E->sideh_var_01 = Sidehopper_Func_2(g_word_A3AAEE[v6], g_word_A3AAE6[v6]);
  int v8 = E->sideh_var_05 >> 1;
  E->sideh_var_02 = Sidehopper_Func_2(g_word_A3AAFA[v8], g_word_A3AAF2[v8]);
  E->sideh_var_B = FUNC16(Sidehopper_Func_4);
}

uint16 Sidehopper_Func_2(uint16 r22, uint16 r24) {  // 0xA3AB9D
  uint16 r18 = 0, r20 = 0;
  do {
    r18 += r24;
    r20 += *(uint16 *)((uint8 *)kCommonEnemySpeeds_Quadratic + (8 * r18) + 1);
  } while (sign16(r20 - r22));
  return r18;
}

void Sidehopper_Func_3(void) {  // 0xA3ABBB
  Enemy_Sidehopper *E = Get_Sidehopper(cur_enemy_index);
  E->base.current_instruction = E->sideh_var_00;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
}

void Sidehopper_Main(void) {  // 0xA3ABCF
  Enemy_Sidehopper *E = Get_Sidehopper(cur_enemy_index);
  CallSidehopperFunc(E->sideh_var_B | 0xA30000);
}

void Sidehopper_Func_4(uint16 k) {  // 0xA3ABD6
  uint16 v1 = g_off_A3AAE2[NextRandom() & 1];
  Get_Sidehopper(k)->sideh_var_B = v1;
}

void Sidehopper_Func_5(void) {  // 0xA3ABE6
  Enemy_Sidehopper *E = Get_Sidehopper(cur_enemy_index);
  int v1 = E->sideh_var_05 >> 1;
  E->sideh_var_E = g_word_A3AAE6[v1];
  E->sideh_var_D = g_word_A3AAEA[v1];
  E->sideh_var_C = E->sideh_var_01;
  E->sideh_var_B = FUNC16(Sidehopper_Func_7);
  if (E->sideh_parameter_1)
    E->sideh_var_B = FUNC16(Sidehopper_Func_8);
}

void Sidehopper_Func_6(void) {  // 0xA3AC13
  Enemy_Sidehopper *E = Get_Sidehopper(cur_enemy_index);
  int v1 = E->sideh_var_05 >> 1;
  E->sideh_var_E = g_word_A3AAF2[v1];
  E->sideh_var_D = g_word_A3AAF6[v1];
  E->sideh_var_C = E->sideh_var_02;
  E->sideh_var_B = FUNC16(Sidehopper_Func_7);
  if (E->sideh_parameter_1)
    E->sideh_var_B = FUNC16(Sidehopper_Func_8);
}

void Sidehopper_Func_7(void) {  // 0xA3AC40
  Enemy_Sidehopper *E = Get_Sidehopper(cur_enemy_index);
  E->sideh_var_B = FUNC16(Sidehopper_Func_9);
  if ((GetSamusEnemyDelta_X(cur_enemy_index) & 0x8000) == 0)
    E->sideh_var_B = FUNC16(Sidehopper_Func_10);
}

void Sidehopper_Func_8(void) {  // 0xA3AC56
  Enemy_Sidehopper *E = Get_Sidehopper(cur_enemy_index);
  E->sideh_var_B = FUNC16(Sidehopper_Func_11);
  if ((GetSamusEnemyDelta_X(cur_enemy_index) & 0x8000) == 0)
    E->sideh_var_B = FUNC16(Sidehopper_Func_12);
}

void Sidehopper_Func_9(void) {  // 0xA3AC6C
  Enemy_Sidehopper *E = Get_Sidehopper(cur_enemy_index);
  E->sideh_var_D = -E->sideh_var_D;
  E->sideh_var_00 = g_off_A3AAD2[E->sideh_var_06 >> 1];
  Sidehopper_Func_3();
  E->sideh_var_B = FUNC16(Sidehopper_Func_14);
}

void Sidehopper_Func_10(void) {  // 0xA3AC8F
  Enemy_Sidehopper *E = Get_Sidehopper(cur_enemy_index);
  E->sideh_var_00 = g_off_A3AAD2[E->sideh_var_06 >> 1];
  Sidehopper_Func_3();
  E->sideh_var_B = FUNC16(Sidehopper_Func_15);
}

void Sidehopper_Func_11(void) {  // 0xA3ACA8
  Enemy_Sidehopper *E = Get_Sidehopper(cur_enemy_index);
  E->sideh_var_D = -E->sideh_var_D;
  E->sideh_var_00 = g_off_A3AADA[E->sideh_var_06 >> 1];
  Sidehopper_Func_3();
  E->sideh_var_B = FUNC16(Sidehopper_Func_16);
}

void Sidehopper_Func_12(void) {  // 0xA3ACCB
  Enemy_Sidehopper *E = Get_Sidehopper(cur_enemy_index);
  E->sideh_var_00 = g_off_A3AADA[E->sideh_var_06 >> 1];
  Sidehopper_Func_3();
  E->sideh_var_B = FUNC16(Sidehopper_Func_17);
}

void Sidehopper_Func_13(void) {  // 0xA3ACE4
  Enemy_Sidehopper *E = Get_Sidehopper(cur_enemy_index);
  E->sideh_var_00 = g_off_A3AAC2[E->sideh_var_06 >> 1];
  if (E->sideh_parameter_1)
    E->sideh_var_00 = g_off_A3AACA[E->sideh_var_06 >> 1];
  Sidehopper_Func_3();
  E->sideh_var_B = FUNC16(Sidehopper_Func_18);
}

void Sidehopper_Func_14(void) {  // 0xA3AD0E
  if (Get_Sidehopper(cur_enemy_index)->sideh_var_03)
    Sidehopper_Func_20();
  else
    Sidehopper_Func_19();
}

void Sidehopper_Func_15(void) {  // 0xA3AD20
  if (Get_Sidehopper(cur_enemy_index)->sideh_var_03)
    Sidehopper_Func_20();
  else
    Sidehopper_Func_19();
}

void Sidehopper_Func_16(void) {  // 0xA3AD32
  if (Get_Sidehopper(cur_enemy_index)->sideh_var_03)
    Sidehopper_Func_22();
  else
    Sidehopper_Func_21();
}

void Sidehopper_Func_17(void) {  // 0xA3AD44
  if (Get_Sidehopper(cur_enemy_index)->sideh_var_03)
    Sidehopper_Func_22();
  else
    Sidehopper_Func_21();
}

void Sidehopper_Func_18(void) {  // 0xA3AD56
  Enemy_Sidehopper *E = Get_Sidehopper(cur_enemy_index);
  if (E->sideh_var_04) {
    E->sideh_var_04 = 0;
    E->sideh_var_B = FUNC16(Sidehopper_Func_4);
  }
}

static void CallSidehopperFunc(uint32 ea) {
  switch (ea) {
  case fnSidehopper_Func_4: Sidehopper_Func_4(cur_enemy_index); return;  // 0xa3abd6
  case fnSidehopper_Func_5: Sidehopper_Func_5(); return;  // 0xa3abe6
  case fnSidehopper_Func_6: Sidehopper_Func_6(); return;  // 0xa3ac13
  case fnSidehopper_Func_7: Sidehopper_Func_7(); return;  // 0xa3ac40
  case fnSidehopper_Func_8: Sidehopper_Func_8(); return;  // 0xa3ac56
  case fnSidehopper_Func_9: Sidehopper_Func_9(); return;  // 0xa3ac6c
  case fnSidehopper_Func_10: Sidehopper_Func_10(); return;  // 0xa3ac8f
  case fnSidehopper_Func_11: Sidehopper_Func_11(); return;  // 0xa3aca8
  case fnSidehopper_Func_12: Sidehopper_Func_12(); return;  // 0xa3accb
  case fnSidehopper_Func_13: Sidehopper_Func_13(); return;  // 0xa3ace4
  case fnSidehopper_Func_14: Sidehopper_Func_14(); return;  // 0xa3ad0e
  case fnSidehopper_Func_15: Sidehopper_Func_15(); return;  // 0xa3ad20
  case fnSidehopper_Func_16: Sidehopper_Func_16(); return;  // 0xa3ad32
  case fnSidehopper_Func_17: Sidehopper_Func_17(); return;  // 0xa3ad44
  case fnSidehopper_Func_18: Sidehopper_Func_18(); return;  // 0xa3ad56
  default: Unreachable();
  }
}

void Sidehopper_Func_19(void) {  // 0xA3AD6D
  Enemy_Sidehopper *E = Get_Sidehopper(cur_enemy_index);
  int v2 = (8 * E->sideh_var_C) >> 1;
  if (Enemy_MoveDown(cur_enemy_index, __PAIR32__(kCommonEnemySpeeds_Quadratic[v2 + 3], kCommonEnemySpeeds_Quadratic[v2 + 2]))) {
    E->sideh_var_D = -E->sideh_var_D;
    E->sideh_var_03 = 1;
  } else {
    if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, INT16_SHL16(E->sideh_var_D))) {
      E->sideh_var_D = -E->sideh_var_D;
      E->sideh_var_03 = 1;
    } else {
      int16 v3 = E->sideh_var_C - E->sideh_var_E;
      E->sideh_var_C = v3;
      if (v3 < 0) {
        E->sideh_var_03 = 1;
        E->sideh_var_C = 0;
      }
    }
  }
}

void Sidehopper_Func_20(void) {  // 0xA3ADD4
  Enemy_Sidehopper *E = Get_Sidehopper(cur_enemy_index);
  int v2 = (8 * E->sideh_var_C) >> 1;
  if (Enemy_MoveDown(cur_enemy_index, __PAIR32__(kCommonEnemySpeeds_Quadratic[v2 + 1], kCommonEnemySpeeds_Quadratic[v2]))) {
    E->sideh_var_03 = 0;
    E->sideh_var_B = FUNC16(Sidehopper_Func_13);
  } else {
    if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, INT16_SHL16(E->sideh_var_D)))
      E->sideh_var_D = -E->sideh_var_D;
    int16 v3 = E->sideh_var_E + E->sideh_var_C;
    if (!sign16(v3 - 64))
      v3 = 64;
    E->sideh_var_C = v3;
  }
}

void Sidehopper_Func_21(void) {  // 0xA3AE27
  Enemy_Sidehopper *E = Get_Sidehopper(cur_enemy_index);
  int v2 = (8 * E->sideh_var_C) >> 1;
  if (Enemy_MoveDown(cur_enemy_index, __PAIR32__(kCommonEnemySpeeds_Quadratic[v2 + 1], kCommonEnemySpeeds_Quadratic[v2]))) {
    E->sideh_var_D = -E->sideh_var_D;
    E->sideh_var_03 = 1;
  } else {
    if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, INT16_SHL16(E->sideh_var_D))) {
      E->sideh_var_D = -E->sideh_var_D;
      E->sideh_var_03 = 1;
    } else {
      int16 v3 = E->sideh_var_C - E->sideh_var_E;
      E->sideh_var_C = v3;
      if (v3 < 0) {
        E->sideh_var_03 = 1;
        E->sideh_var_C = 0;
      }
    }
  }
}

void Sidehopper_Func_22(void) {  // 0xA3AE8E
  int16 v3;

  Enemy_Sidehopper *E = Get_Sidehopper(cur_enemy_index);
  int v2 = (8 * E->sideh_var_C) >> 1;
  if (Enemy_MoveDown(cur_enemy_index, __PAIR32__(kCommonEnemySpeeds_Quadratic[v2 + 3], kCommonEnemySpeeds_Quadratic[v2 + 2]))) {
    E->sideh_var_03 = 0;
    E->sideh_var_B = FUNC16(Sidehopper_Func_13);
  } else {
    if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, INT16_SHL16(E->sideh_var_D)))
      E->sideh_var_D = -E->sideh_var_D;
    v3 = E->sideh_var_E + E->sideh_var_C;
    if (!sign16(v3 - 64))
      v3 = 64;
    E->sideh_var_C = v3;
  }
}
