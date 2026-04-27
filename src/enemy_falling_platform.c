// Enemy AI - Falling/Sinking platforms + shared A3 wrappers — peeled from Bank $A3
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"

void Enemy_GrappleReact_NoInteract_A3(void) {  // 0xA38000
  SwitchEnemyAiToMainAi();
}

void Enemy_GrappleReact_KillEnemy_A3(void) {  // 0xA3800A
  EnemyGrappleDeath();
}

void Enemy_GrappleReact_CancelBeam_A3(void) {  // 0xA3800F
  Enemy_SwitchToFrozenAi();
}

void Enemy_NormalTouchAI_A3(void) {  // 0xA38023
  NormalEnemyTouchAi();
}

void Enemy_NormalShotAI_A3(void) {  // 0xA3802D
  NormalEnemyShotAi();
}

void Enemy_NormalShotAI_SkipSomeParts_A3(void) {  // 0xA38032
  NormalEnemyShotAiSkipDeathAnim_CurEnemy();
}

void Enemy_NormalPowerBombAI_A3(void) {  // 0xA38037
  NormalEnemyPowerBombAi();
}

void Enemy_NormalFrozenAI_A3(void) {  // 0xA38041
  NormalEnemyFrozenAI();
}

const uint16 *PlatformThatFallsWithSamus_Instr_3(uint16 k, const uint16 *jp) {  // 0xA39C6B
  Get_PlatformThatFallsWithSamus(cur_enemy_index)->ptfwss_var_02 = 0;
  return jp;
}

const uint16 *PlatformThatFallsWithSamus_Instr_4(uint16 k, const uint16 *jp) {  // 0xA39C76
  Get_PlatformThatFallsWithSamus(cur_enemy_index)->ptfwss_var_02 = 1;
  return jp;
}

const uint16 *PlatformThatFallsWithSamus_Instr_1(uint16 k, const uint16 *jp) {  // 0xA39C81
  Get_PlatformThatFallsWithSamus(cur_enemy_index)->ptfwss_var_02 = 0;
  return jp;
}

const uint16 *PlatformThatFallsWithSamus_Instr_2(uint16 k, const uint16 *jp) {  // 0xA39C8C
  Get_PlatformThatFallsWithSamus(cur_enemy_index)->ptfwss_var_02 = 1;
  return jp;
}

void PlatformThatFallsWithSamus_Init(void) {  // 0xA39C9F
  Enemy_PlatformThatFallsWithSamus *E = Get_PlatformThatFallsWithSamus(cur_enemy_index);
  E->ptfwss_var_1F = -1;
  uint16 ptfwss_parameter_1 = E->ptfwss_parameter_1;
  E->ptfwss_var_02 = ptfwss_parameter_1;
  if (ptfwss_parameter_1)
    PlatformThatFalls_Init(cur_enemy_index, addr_kPlatformThatFallsWithSamus_Ilist_9BFD);
  else
    PlatformThatFalls_Init(cur_enemy_index, addr_kPlatformThatFallsWithSamus_Ilist_9BE7);
}

void FastMovingSlowSinkingPlatform_Init(void) {  // 0xA39CBA
  Enemy_PlatformThatFallsWithSamus *E = Get_PlatformThatFallsWithSamus(cur_enemy_index);
  uint16 ptfwss_parameter_1 = E->ptfwss_parameter_1;
  E->ptfwss_var_02 = ptfwss_parameter_1;
  if (ptfwss_parameter_1)
    PlatformThatFalls_Init(cur_enemy_index, addr_kPlatformThatFallsWithSamus_Ilist_9C55);
  else
    PlatformThatFalls_Init(cur_enemy_index, addr_kPlatformThatFallsWithSamus_Ilist_9C3F);
}

void PlatformThatFalls_Init(uint16 k, uint16 j) {  // 0xA39CCC
  Enemy_PlatformThatFallsWithSamus *E = Get_PlatformThatFallsWithSamus(k);
  E->base.current_instruction = j;
  int v3 = (8 * LOBYTE(E->ptfwss_parameter_2)) >> 1;
  E->ptfwss_var_C = kCommonEnemySpeeds_Linear[v3];
  E->ptfwss_var_B = kCommonEnemySpeeds_Linear[v3 + 1];
  E->ptfwss_var_E = kCommonEnemySpeeds_Linear[v3 + 2];
  E->ptfwss_var_D = kCommonEnemySpeeds_Linear[v3 + 3];
  E->ptfwss_var_00 = 0;
  E->ptfwss_var_03 = 0;
  E->ptfwss_var_04 = 0;
  E->ptfwss_var_A = E->base.y_pos + 1;
  E->ptfwss_var_F = 0;
  E->ptfwss_var_05 = HIBYTE(E->ptfwss_parameter_2);
}
static Func_V *const off_A39C97[2] = { PlatformThatFallsWithSamus_Func_1, PlatformThatFallsWithSamus_Func_2 };
static Func_V *const off_A39C9B[2] = { PlatformThatFallsWithSamus_Func_3, PlatformThatFallsWithSamus_Func_4 };
void PlatformThatFallsWithSamus_Main(void) {  // 0xA39D16
  Enemy_PlatformThatFallsWithSamus *E = Get_PlatformThatFallsWithSamus(cur_enemy_index);
  E->ptfwss_var_00 = 0;
  if (CheckIfEnemyTouchesSamus(cur_enemy_index))
    E->ptfwss_var_00 = 1;
  off_A39C97[E->ptfwss_var_02]();
  int v3 = Get_PlatformThatFallsWithSamus(cur_enemy_index)->ptfwss_var_00;
  off_A39C9B[v3]();
  if (E->ptfwss_var_00 != E->ptfwss_var_06)
    E->ptfwss_var_F = 0;
  E->ptfwss_var_06 = E->ptfwss_var_00;
}

void PlatformThatFallsWithSamus_Func_1(void) {  // 0xA39D5E
  Enemy_PlatformThatFallsWithSamus *E = Get_PlatformThatFallsWithSamus(cur_enemy_index);
  E->ptfwss_var_01 = E->base.x_pos;
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, __PAIR32__(E->ptfwss_var_E, E->ptfwss_var_D))) {
    E->ptfwss_var_02 = 1;
    PlatformThatFallsWithSamus_Func_8();
  }
}

void PlatformThatFallsWithSamus_Func_2(void) {  // 0xA39D83
  Enemy_PlatformThatFallsWithSamus *E = Get_PlatformThatFallsWithSamus(cur_enemy_index);
  E->ptfwss_var_01 = E->base.x_pos;
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, __PAIR32__(E->ptfwss_var_C, E->ptfwss_var_B))) {
    E->ptfwss_var_02 = 0;
    PlatformThatFallsWithSamus_Func_7();
  }
}

void PlatformThatFallsWithSamus_Func_3(void) {  // 0xA39DA8
  Enemy_PlatformThatFallsWithSamus *E = Get_PlatformThatFallsWithSamus(cur_enemy_index);
  if ((int16)(E->base.y_pos - E->ptfwss_var_A) < 0)
    goto LABEL_5;
  PlatformThatFallsWithSamus_Func_9();
  uint16 ptfwss_var_05;
  ptfwss_var_05 = ++E->ptfwss_var_F;
  if ((int16)(ptfwss_var_05 - E->ptfwss_var_05) >= 0) {
    ptfwss_var_05 = E->ptfwss_var_05;
    E->ptfwss_var_F = ptfwss_var_05;
  }
  int v3;
  v3 = (8 * ptfwss_var_05) >> 1;
  if (Enemy_MoveDown(cur_enemy_index, __PAIR32__(kCommonEnemySpeeds_Quadratic[v3 + 3], kCommonEnemySpeeds_Quadratic[v3 + 2]))) {
LABEL_5:
    E->ptfwss_var_F = 0;
    PlatformThatFallsWithSamus_Func_10();
  }
}

void PlatformThatFallsWithSamus_Func_4(void) {  // 0xA39DE4
  Enemy_PlatformThatFallsWithSamus *E = Get_PlatformThatFallsWithSamus(cur_enemy_index);
  if ((int16)(++E->ptfwss_var_F - E->ptfwss_var_05) >= 0)
    E->ptfwss_var_F = E->ptfwss_var_05;
  extra_samus_x_displacement += E->base.x_pos - E->ptfwss_var_01;
  E->ptfwss_var_01 = E->base.y_pos;
  int v2 = (8 * E->ptfwss_var_F) >> 1;
  if (Enemy_MoveDown(cur_enemy_index, __PAIR32__(kCommonEnemySpeeds_Quadratic[v2 + 1], kCommonEnemySpeeds_Quadratic[v2]))) {
    E->ptfwss_var_F = 0;
    PlatformThatFallsWithSamus_Func_10();
  }
  extra_samus_y_displacement += E->base.y_pos - E->ptfwss_var_01;
}

void PlatformThatFallsWithSamus_Func_5(void) {  // 0xA39E47
  Enemy_PlatformThatFallsWithSamus *E = Get_PlatformThatFallsWithSamus(cur_enemy_index);
  uint16 v0 = addr_kPlatformThatFallsWithSamus_Ilist_9C13;
  if ((E->ptfwss_var_1F & 0x8000) != 0)
    v0 = addr_kPlatformThatFallsWithSamus_Ilist_9BBB;
  E->base.current_instruction = v0;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
}

void PlatformThatFallsWithSamus_Func_6(void) {  // 0xA39E64
  Enemy_PlatformThatFallsWithSamus *E = Get_PlatformThatFallsWithSamus(cur_enemy_index);
  uint16 v0 = addr_kPlatformThatFallsWithSamus_Ilist_9C29;
  if ((E->ptfwss_var_1F & 0x8000) != 0)
    v0 = addr_kPlatformThatFallsWithSamus_Ilist_9BD1;
  E->base.current_instruction = v0;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
}

void PlatformThatFallsWithSamus_Func_7(void) {  // 0xA39E81
  Enemy_PlatformThatFallsWithSamus *E = Get_PlatformThatFallsWithSamus(cur_enemy_index);
  uint16 v0 = addr_kPlatformThatFallsWithSamus_Ilist_9C3F;
  if ((E->ptfwss_var_1F & 0x8000) != 0)
    v0 = addr_kPlatformThatFallsWithSamus_Ilist_9BE7;
  E->base.current_instruction = v0;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
}

void PlatformThatFallsWithSamus_Func_8(void) {  // 0xA39E9E
  Enemy_PlatformThatFallsWithSamus *E = Get_PlatformThatFallsWithSamus(cur_enemy_index);
  uint16 v0 = addr_kPlatformThatFallsWithSamus_Ilist_9C55;
  if ((E->ptfwss_var_1F & 0x8000) != 0)
    v0 = addr_kPlatformThatFallsWithSamus_Ilist_9BFD;
  E->base.current_instruction = v0;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
}

void PlatformThatFallsWithSamus_Func_9(void) {  // 0xA39EBB
  Enemy_PlatformThatFallsWithSamus *E = Get_PlatformThatFallsWithSamus(cur_enemy_index);
  if (!E->ptfwss_var_03) {
    E->ptfwss_var_03 = 1;
    if (E->ptfwss_var_02)
      PlatformThatFallsWithSamus_Func_6();
    else
      PlatformThatFallsWithSamus_Func_5();
  }
  E->ptfwss_var_04 = 0;
}

void PlatformThatFallsWithSamus_Func_10(void) {  // 0xA39EE1
  Enemy_PlatformThatFallsWithSamus *E = Get_PlatformThatFallsWithSamus(cur_enemy_index);
  if (!E->ptfwss_var_04) {
    E->ptfwss_var_04 = 1;
    if (E->ptfwss_var_02)
      PlatformThatFallsWithSamus_Func_8();
    else
      PlatformThatFallsWithSamus_Func_7();
  }
  E->ptfwss_var_03 = 0;
}

void FastMovingSlowSinkingPlatform_Shot(void) {  // 0xA39F08
  Enemy_NormalShotAI_A3();
  Enemy_PlatformThatFallsWithSamus *E = Get_PlatformThatFallsWithSamus(cur_enemy_index);
  if (E->base.frozen_timer) {
    if (E->ptfwss_var_02)
      E->base.spritemap_pointer = addr_kPlatformThatFallsWithSamus_Sprmap_A015;
    else
      E->base.spritemap_pointer = addr_kPlatformThatFallsWithSamus_Sprmap_A009;
  }
}

