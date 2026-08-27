// Enemy AI - Maridia fish — peeled from Bank $A3
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

const uint16 *MaridiaFish_Instr_3(uint16 k, const uint16 *jp) {  // 0xA39096
  Get_MaridiaFish(cur_enemy_index)->base.layer = 6;
  return jp;
}

const uint16 *MaridiaFish_Instr_1(uint16 k, const uint16 *jp) {  // 0xA390A0
  Get_MaridiaFish(cur_enemy_index)->base.layer = 2;
  return jp;
}

const uint16 *MaridiaFish_Instr_2(uint16 k, const uint16 *jp) {  // 0xA390AA
  Get_MaridiaFish(cur_enemy_index)->mfh_var_01 = 1;
  return jp;
}

void MaridiaFish_Init(void) {  // 0xA390B5
  Enemy_MaridiaFish *E = Get_MaridiaFish(cur_enemy_index);
  E->base.current_instruction = addr_kMaridiaFish_Ilist_902A;
  E->mfh_var_A = FUNC16(MaridiaFish_Func_1);
  if (!HIBYTE(E->mfh_parameter_1)) {
    E->base.current_instruction = addr_kMaridiaFish_Ilist_9060;
    E->mfh_var_A = FUNC16(MaridiaFish_Func_2);
  }
  int v1 = (8 * LOBYTE(E->mfh_parameter_1)) >> 1;
  E->mfh_var_C = kCommonEnemySpeeds_Linear[v1];
  E->mfh_var_B = kCommonEnemySpeeds_Linear[v1 + 1];
  E->mfh_var_E = kCommonEnemySpeeds_Linear[v1 + 2];
  E->mfh_var_D = kCommonEnemySpeeds_Linear[v1 + 3];
  E->mfh_var_00 = LOBYTE(E->mfh_parameter_2);
  E->mfh_var_02 = HIBYTE(E->mfh_parameter_2);
  E->mfh_var_F = 0;
  E->mfh_var_01 = 0;
  E->mfh_var_03 = SineMult8bit(E->mfh_var_F, E->mfh_var_00);
}

void MaridiaFish_Func_1(void) {  // 0xA39132
  Enemy_MaridiaFish *E = Get_MaridiaFish(cur_enemy_index);
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, __PAIR32__(E->mfh_var_E, E->mfh_var_D))) {
    E->mfh_var_A = FUNC16(MaridiaFish_Func_3);
    E->base.instruction_timer = 1;
    E->base.timer = 0;
    E->base.current_instruction = addr_kMaridiaFish_Ilist_903C;
  } else {
    uint16 v2 = SineMult8bit(E->mfh_var_F, E->mfh_var_00);
    E->mfh_var_04 = v2;
    if (Enemy_MoveDown(cur_enemy_index, INT16_SHL16(v2 - E->mfh_var_03))) {
      E->mfh_var_A = FUNC16(MaridiaFish_Func_3);
      E->base.instruction_timer = 1;
      E->base.timer = 0;
      E->base.current_instruction = addr_kMaridiaFish_Ilist_903C;
    } else {
      E->mfh_var_F = (uint8)(LOBYTE(E->mfh_var_02) + E->mfh_var_F);
    }
  }
  E->mfh_var_03 = E->mfh_var_04;
}

void MaridiaFish_Func_2(void) {  // 0xA391AB
  Enemy_MaridiaFish *E = Get_MaridiaFish(cur_enemy_index);
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, __PAIR32__(E->mfh_var_C, E->mfh_var_B))) {
    E->mfh_var_A = FUNC16(MaridiaFish_Func_4);
    E->base.instruction_timer = 1;
    E->base.timer = 0;
    E->base.current_instruction = addr_kMaridiaFish_Ilist_9072;
  } else {
    uint16 v2 = SineMult8bit(E->mfh_var_F, E->mfh_var_00);
    E->mfh_var_04 = v2;
    if (Enemy_MoveDown(cur_enemy_index, INT16_SHL16(v2 - E->mfh_var_03))) {
      E->mfh_var_A = FUNC16(MaridiaFish_Func_4);
      E->base.instruction_timer = 1;
      E->base.timer = 0;
      E->base.current_instruction = addr_kMaridiaFish_Ilist_9072;
    } else {
      E->mfh_var_F = (uint8)(LOBYTE(E->mfh_var_02) + E->mfh_var_F);
    }
  }
  E->mfh_var_03 = E->mfh_var_04;
}

void MaridiaFish_Func_3(void) {  // 0xA39224
  Enemy_MaridiaFish *E = Get_MaridiaFish(cur_enemy_index);
  if (E->mfh_var_01) {
    E->mfh_var_01 = 0;
    E->mfh_var_A = FUNC16(MaridiaFish_Func_2);
    E->mfh_var_02 = -E->mfh_var_02;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
    E->base.current_instruction = addr_kMaridiaFish_Ilist_9060;
  }
}

void MaridiaFish_Func_4(void) {  // 0xA39256
  Enemy_MaridiaFish *E = Get_MaridiaFish(cur_enemy_index);
  if (E->mfh_var_01) {
    E->mfh_var_01 = 0;
    E->mfh_var_A = FUNC16(MaridiaFish_Func_1);
    E->mfh_var_02 = -E->mfh_var_02;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
    E->base.current_instruction = addr_kMaridiaFish_Ilist_902A;
  }
}

void CallMaridiaFishFunc(uint32 ea) {
  switch (ea) {
  case fnMaridiaFish_Func_1: MaridiaFish_Func_1(); return;
  case fnMaridiaFish_Func_2: MaridiaFish_Func_2(); return;
  case fnMaridiaFish_Func_3: MaridiaFish_Func_3(); return;
  case fnMaridiaFish_Func_4: MaridiaFish_Func_4(); return;
  default: Unreachable();
  }
}

void MaridiaFish_Main(void) {  // 0xA3912B
  Enemy_MaridiaFish *E = Get_MaridiaFish(cur_enemy_index);
  CallMaridiaFishFunc(E->mfh_var_A | 0xA30000);
}
