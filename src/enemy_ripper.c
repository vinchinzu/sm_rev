// Ripper / JetPowerRipper extracted from sm_a2.c.
#include "ida_types.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "variables.h"
#include "enemy_ai_canon.h"


void JetPowerRipper_Init(void) {  // 0xA2E318
  uint16 v0 = addr_kRipper_Ilist_E2E0;
  Enemy_JetPowerRipper *E = Get_JetPowerRipper(cur_enemy_index);
  if (E->jprr_parameter_2)
    v0 = addr_kRipper_Ilist_E2F4;
  E->base.current_instruction = v0;
  uint16 v2 = 8 * E->jprr_parameter_1;
  E->jprr_var_E = v2;
  int v3 = v2 >> 1;
  uint16 v4;
  if (E->jprr_parameter_2) {
    E->jprr_var_D = kCommonEnemySpeeds_Linear[v3];
    v4 = kCommonEnemySpeeds_Linear[v3 + 1];
  } else {
    E->jprr_var_D = kCommonEnemySpeeds_Linear[v3 + 2];
    v4 = kCommonEnemySpeeds_Linear[v3 + 3];
  }
  E->jprr_var_C = v4;
}

void JetPowerRipper_Main(void) {  // 0xA2E353
  int16 v3;

  Enemy_JetPowerRipper *E = Get_JetPowerRipper(cur_enemy_index);
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, __PAIR32__(E->jprr_var_D, E->jprr_var_C))) {
    int v2 = E->jprr_var_E >> 1;
    if ((E->jprr_var_D & 0x8000) == 0) {
      E->jprr_var_D = kCommonEnemySpeeds_Linear[v2 + 2];
      E->jprr_var_C = kCommonEnemySpeeds_Linear[v2 + 3];
      v3 = -7456;
    } else {
      E->jprr_var_D = kCommonEnemySpeeds_Linear[v2];
      E->jprr_var_C = kCommonEnemySpeeds_Linear[v2 + 1];
      v3 = -7436;
    }
    E->base.current_instruction = v3;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
  }
}

void Ripper_Func_1(void) {  // 0xA2E3A0
  NormalEnemyFrozenAI();
}

void JetPowerRipper_Shot(void) {  // 0xA2E3A9
  int16 v1;

  NormalEnemyShotAi();
  Enemy_JetPowerRipper *E = Get_JetPowerRipper(cur_enemy_index);
  if (E->base.frozen_timer) {
    v1 = -7105;
    if ((E->jprr_var_D & 0x8000) == 0)
      v1 = -7093;
    E->base.spritemap_pointer = v1;
  }
}

void Ripper_Init(void) {  // 0xA2E49F
  uint16 v0 = addr_kRipper_Ilist_E477;
  Enemy_Ripper *E = Get_Ripper(cur_enemy_index);
  if (!E->ripper_parameter_2)
    v0 = addr_kRipper_Ilist_E48B;
  E->base.current_instruction = v0;
  uint16 v2 = 8 * E->ripper_parameter_1;
  E->ripper_var_E = v2;
  int v3 = v2 >> 1;
  uint16 v4;
  if (E->ripper_parameter_2) {
    E->ripper_var_D = kCommonEnemySpeeds_Linear[v3];
    v4 = kCommonEnemySpeeds_Linear[v3 + 1];
  } else {
    E->ripper_var_D = kCommonEnemySpeeds_Linear[v3 + 2];
    v4 = kCommonEnemySpeeds_Linear[v3 + 3];
  }
  E->ripper_var_C = v4;
}

void Ripper_Main(void) {  // 0xA2E4DA
  int16 v3;

  Enemy_Ripper *E = Get_Ripper(cur_enemy_index);
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, __PAIR32__(E->ripper_var_D, E->ripper_var_C))) {
    int v2 = E->ripper_var_E >> 1;
    if ((E->ripper_var_D & 0x8000) == 0) {
      E->ripper_var_D = kCommonEnemySpeeds_Linear[v2 + 2];
      E->ripper_var_C = kCommonEnemySpeeds_Linear[v2 + 3];
      v3 = -7029;
    } else {
      E->ripper_var_D = kCommonEnemySpeeds_Linear[v2];
      E->ripper_var_C = kCommonEnemySpeeds_Linear[v2 + 1];
      v3 = -7049;
    }
    E->base.current_instruction = v3;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
  }
}
