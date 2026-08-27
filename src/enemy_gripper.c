// Gripper extracted from sm_a2.c.
#include "ida_types.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "variables.h"
#include "enemy_ai_canon.h"


void Gripper_Init(void) {  // 0xA2E1D3
  Enemy_Gripper *E = Get_Gripper(cur_enemy_index);
  uint16 v1 = 8 * LOBYTE(E->base.current_instruction);
  E->gripper_var_E = v1;
  int v2 = v1 >> 1;
  uint16 v3;
  if ((E->base.current_instruction & 0xFEFF) != 0) {
    E->gripper_var_D = kCommonEnemySpeeds_Linear[v2];
    v3 = kCommonEnemySpeeds_Linear[v2 + 1];
  } else {
    E->gripper_var_D = kCommonEnemySpeeds_Linear[v2 + 2];
    v3 = kCommonEnemySpeeds_Linear[v2 + 3];
  }
  E->gripper_var_C = v3;
  uint16 v4 = addr_kGripper_Ilist_E19B;
  if ((E->gripper_var_D & 0x8000) == 0)
    v4 = addr_kGripper_Ilist_E1AF;
  E->base.current_instruction = v4;
  E->gripper_var_A = E->gripper_parameter_1;
  E->gripper_var_B = E->gripper_parameter_2;
}

void Gripper_Main(void) {  // 0xA2E221
  Enemy_Gripper *E = Get_Gripper(cur_enemy_index);
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, __PAIR32__(E->gripper_var_D, E->gripper_var_C)) ||
      Gripper_Func_1(cur_enemy_index) & 1 || Gripper_Func_2(cur_enemy_index) & 1) {
    int v3 = E->gripper_var_E >> 1;
    uint16 v4;
    if ((E->gripper_var_D & 0x8000) == 0) {
      E->gripper_var_D = kCommonEnemySpeeds_Linear[v3 + 2];
      E->gripper_var_C = kCommonEnemySpeeds_Linear[v3 + 3];
      v4 = addr_kGripper_Ilist_E19B;
    } else {
      E->gripper_var_D = kCommonEnemySpeeds_Linear[v3];
      E->gripper_var_C = kCommonEnemySpeeds_Linear[v3 + 1];
      v4 = addr_kGripper_Ilist_E1AF;
    }
    E->base.current_instruction = v4;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
  }
}

uint8 Gripper_Func_1(uint16 k) {  // 0xA2E279
  Enemy_Gripper *E = Get_Gripper(k);
  return (E->gripper_var_D & 0x8000) != 0 && (int16)(E->base.x_pos - E->gripper_var_A) < 0;
}

uint8 Gripper_Func_2(uint16 k) {  // 0xA2E28A
  Enemy_Gripper *E = Get_Gripper(k);
  return (E->gripper_var_D & 0x8000) == 0 && (int16)(E->base.x_pos - E->gripper_var_B) >= 0;
}

void Gripper_Func_3(void) {  // 0xA2E29B
  NormalEnemyFrozenAI();
}

void Gripper_Func_4(void) {  // 0xA2E2A4
  NormalEnemyShotAi();
  Enemy_Gripper *E = Get_Gripper(cur_enemy_index);
  if (E->base.frozen_timer) {
    uint16 v1 = addr_kGripper_Sprmap_E43F;
    if ((E->gripper_var_D & 0x8000) == 0)
      v1 = addr_kGripper_Sprmap_E44B;
    E->base.spritemap_pointer = v1;
  }
}
