// Ripper / JetPowerRipper extracted from sm_a2.c.
#include "ida_types.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "variables.h"
#include "enemy_ai_canon.h"

void JetPowerRipper_Init(void) {  // 0xA2E318
  uint16 ilist = addr_kRipper_Ilist_E2E0;
  Enemy_JetPowerRipper *E = Get_JetPowerRipper(cur_enemy_index);
  if (E->jprr_parameter_2)
    ilist = addr_kRipper_Ilist_E2F4;
  E->base.current_instruction = ilist;
  uint16 speed_idx = 8 * E->jprr_parameter_1;
  E->jprr_var_E = speed_idx;
  int v = speed_idx >> 1;
  uint16 subspeed;
  if (E->jprr_parameter_2) {
    E->jprr_var_D = kCommonEnemySpeeds_Linear[v];
    subspeed = kCommonEnemySpeeds_Linear[v + 1];
  } else {
    E->jprr_var_D = kCommonEnemySpeeds_Linear[v + 2];
    subspeed = kCommonEnemySpeeds_Linear[v + 3];
  }
  E->jprr_var_C = subspeed;
}

void JetPowerRipper_Main(void) {  // 0xA2E353
  Enemy_JetPowerRipper *E = Get_JetPowerRipper(cur_enemy_index);
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, __PAIR32__(E->jprr_var_D, E->jprr_var_C))) {
    int v = E->jprr_var_E >> 1;
    uint16 ilist;
    if (!sign16(E->jprr_var_D)) {
      E->jprr_var_D = kCommonEnemySpeeds_Linear[v + 2];
      E->jprr_var_C = kCommonEnemySpeeds_Linear[v + 3];
      ilist = addr_kRipper_Ilist_E2E0;
    } else {
      E->jprr_var_D = kCommonEnemySpeeds_Linear[v];
      E->jprr_var_C = kCommonEnemySpeeds_Linear[v + 1];
      ilist = addr_kRipper_Ilist_E2F4;
    }
    E->base.current_instruction = ilist;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
  }
}

void Ripper_Func_1(void) {  // 0xA2E3A0
  NormalEnemyFrozenAI();
}

void JetPowerRipper_Shot(void) {  // 0xA2E3A9
  NormalEnemyShotAi();
  Enemy_JetPowerRipper *E = Get_JetPowerRipper(cur_enemy_index);
  if (E->base.frozen_timer) {
    uint16 spritemap = addr_kGripper_Sprmap_E43F;
    if (!sign16(E->jprr_var_D))
      spritemap = addr_kGripper_Sprmap_E44B;
    E->base.spritemap_pointer = spritemap;
  }
}

void Ripper_Init(void) {  // 0xA2E49F
  uint16 ilist = addr_kRipper_Ilist_E477;
  Enemy_Ripper *E = Get_Ripper(cur_enemy_index);
  if (!E->ripper_parameter_2)
    ilist = addr_kRipper_Ilist_E48B;
  E->base.current_instruction = ilist;
  uint16 speed_idx = 8 * E->ripper_parameter_1;
  E->ripper_var_E = speed_idx;
  int v = speed_idx >> 1;
  uint16 subspeed;
  if (E->ripper_parameter_2) {
    E->ripper_var_D = kCommonEnemySpeeds_Linear[v];
    subspeed = kCommonEnemySpeeds_Linear[v + 1];
  } else {
    E->ripper_var_D = kCommonEnemySpeeds_Linear[v + 2];
    subspeed = kCommonEnemySpeeds_Linear[v + 3];
  }
  E->ripper_var_C = subspeed;
}

void Ripper_Main(void) {  // 0xA2E4DA
  Enemy_Ripper *E = Get_Ripper(cur_enemy_index);
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, __PAIR32__(E->ripper_var_D, E->ripper_var_C))) {
    int v = E->ripper_var_E >> 1;
    uint16 ilist;
    if (!sign16(E->ripper_var_D)) {
      E->ripper_var_D = kCommonEnemySpeeds_Linear[v + 2];
      E->ripper_var_C = kCommonEnemySpeeds_Linear[v + 3];
      ilist = addr_kRipper_Ilist_E48B;
    } else {
      E->ripper_var_D = kCommonEnemySpeeds_Linear[v];
      E->ripper_var_C = kCommonEnemySpeeds_Linear[v + 1];
      ilist = addr_kRipper_Ilist_E477;
    }
    E->base.current_instruction = ilist;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
  }
}
