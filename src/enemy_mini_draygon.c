// Enemy AI - Mini-Draygon — peeled from Bank $A8

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

const uint16 *MiniDraygon_Instr_2(uint16 k, const uint16 *jp) {  // 0xA8878F
  QueueSfx2_Max6(0x5E);
  return jp;
}

const uint16 *MiniDraygon_Instr_1(uint16 k, const uint16 *jp) {  // 0xA8879B
  if (gEnemySpawnData(cur_enemy_index)[30].cause_of_death == addr_kMiniDraygon_Ilist_870B)
    Get_MiniDraygon(cur_enemy_index)->mdn_var_F = -8;
  else
    Get_MiniDraygon(cur_enemy_index)->mdn_var_F = 8;
  return jp;
}

const uint16 *MiniDraygon_Instr_3(uint16 k, const uint16 *jp) {  // 0xA887B6
  Enemy_MiniDraygon *E = Get_MiniDraygon(cur_enemy_index);
  if (gEnemySpawnData(cur_enemy_index)[30].cause_of_death == addr_kMiniDraygon_Ilist_870B) {
    ++E->mdn_var_F;
  } else {
    --E->mdn_var_F;
  }
  return jp;
}

const uint16 *MiniDraygon_Instr_4(uint16 k, const uint16 *jp) {  // 0xA887CB
  Enemy_MiniDraygon *E = Get_MiniDraygon(cur_enemy_index);
  E->mdn_var_0C = 0;
  E->mdn_var_0B = 0;
  E->mdn_var_C = FUNC16(MiniDraygon_Func_8);
  return jp;
}

void MiniDraygon_Init(void) {  // 0xA887E0
  Enemy_MiniDraygon *E = Get_MiniDraygon(cur_enemy_index);
  if (E->mdn_parameter_1) {
    MiniDraygon_Func_2();
    E->base.layer = 4;
  } else {
    MiniDraygon_Func_1();
    uint16 v2 = 8 * LOBYTE(E->mdn_parameter_2);
    int v3 = v2 >> 1;
    E->mdn_var_04 = kCommonEnemySpeeds_Linear[v3];
    E->mdn_var_03 = kCommonEnemySpeeds_Linear[v3 + 1];
    E->mdn_var_06 = kCommonEnemySpeeds_Linear[v3 + 2];
    E->mdn_var_05 = kCommonEnemySpeeds_Linear[v3 + 3];
    E->mdn_var_E = HIBYTE(Get_MiniDraygon(v2)->mdn_parameter_2) >> 1;
  }
  E->mdn_var_00 = 0;
  E->mdn_var_01 = 0;
  E->mdn_var_C = FUNC16(MiniDraygon_Func_4);
}

void MiniDraygon_Func_1(void) {  // 0xA88838
  Get_MiniDraygon(cur_enemy_index)->mdn_var_B = (GetSamusEnemyDelta_X(cur_enemy_index) & 0x8000) == 0;
  Enemy_MiniDraygon *E = Get_MiniDraygon(cur_enemy_index);
  E->mdn_var_02 = addr_kMiniDraygon_Ilist_86A7;
  if (E->mdn_var_B)
    E->mdn_var_02 = addr_kMiniDraygon_Ilist_870B;
  MiniDraygon_Func_12();
}

void MiniDraygon_Func_2(void) {  // 0xA88866
  int v0 = cur_enemy_index >> 1;
  uint16 v1 = enemy_drawing_queue_sizes[v0 + 1];
  Enemy_MiniDraygon *E = Get_MiniDraygon(cur_enemy_index);
  E->mdn_var_B = v1;
  if (v1) {
    E->base.x_pos = enemy_drawing_queue[v0 + 91] + 4;
    E->base.y_pos = enemy_drawing_queue[v0 + 93] + 10;
    E->mdn_var_02 = addr_kMiniDraygon_Ilist_8727;
  } else {
    E->base.x_pos = enemy_drawing_queue[v0 + 91] - 4;
    E->base.y_pos = enemy_drawing_queue[v0 + 93] + 10;
    E->mdn_var_02 = addr_kMiniDraygon_Ilist_86C3;
  }
  MiniDraygon_Func_12();
}

void MiniDraygon_Func_3(void) {  // 0xA888E5
  int v0 = cur_enemy_index >> 1;
  uint16 v1 = enemy_drawing_queue[v0 + 83];
  Enemy_MiniDraygon *E = Get_MiniDraygon(cur_enemy_index);
  E->mdn_var_B = v1;
  if (v1)
    E->base.x_pos = enemy_drawing_queue[v0 + 59] + 4;
  else
    E->base.x_pos = enemy_drawing_queue[v0 + 59] - 4;
  E->base.y_pos = enemy_drawing_queue[v0 + 61] + 18;
}


void CallMiniDraygonFunc(uint32 ea) {
  switch (ea) {
  case fnMiniDraygon_Func_4: MiniDraygon_Func_4(); return;
  case fnMiniDraygon_Func_8: MiniDraygon_Func_8(); return;
  case fnMiniDraygon_Func_9: MiniDraygon_Func_9(); return;
  case fnMiniDraygon_Func_10: MiniDraygon_Func_10(); return;
  default: Unreachable();
  }
}
void MiniDraygon_Main(void) {  // 0xA8891B
  Enemy_MiniDraygon *E = Get_MiniDraygon(cur_enemy_index);
  CallMiniDraygonFunc(E->mdn_var_C | 0xA80000);
}

void MiniDraygon_Func_4(void) {  // 0xA88922
  if (Get_MiniDraygon(cur_enemy_index)->mdn_parameter_1)
    MiniDraygon_Func_6();
  else
    MiniDraygon_Func_5();
}

void MiniDraygon_Func_5(void) {  // 0xA88933
  if (!Get_MiniDraygon(cur_enemy_index + 128)->mdn_var_0C)
    MiniDraygon_Func_1();
  Enemy_MiniDraygon *E = Get_MiniDraygon(cur_enemy_index);
  if (E->mdn_var_00) {
    AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(E->mdn_var_04, E->mdn_var_03));
  } else {
    AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(E->mdn_var_06, E->mdn_var_05));
  }
  if ((--E->mdn_var_E & 0x8000) != 0) {
    E->mdn_var_E = HIBYTE(E->mdn_parameter_2);
    E->mdn_var_00 ^= 1;
  }
}

void MiniDraygon_Func_6(void) {  // 0xA88997
  MiniDraygon_Func_2();
}


void MiniDraygon_Func_7(void) {  // 0xA889D4
  if (IsSamusWithinEnemy_X(cur_enemy_index - 128, 0x80)) {
    uint16 r22 = -(uint8)(CalculateAngleOfSamusFromEnemy(cur_enemy_index) - 64);
    Enemy_MiniDraygon *E = Get_MiniDraygon(cur_enemy_index);
    SetHiLo(&E->mdn_var_07, &E->mdn_var_08, CosineMult8bitFull(r22, 4));
    SetHiLo(&E->mdn_var_09, &E->mdn_var_0A, SineMult8bitFull(r22, 4));
    E->mdn_var_02 = addr_kMiniDraygon_Ilist_876F;
    MiniDraygon_Func_12();
    E->mdn_var_0B = 1;
    E->mdn_var_C = FUNC16(MiniDraygon_Func_9);
  }
}

void MiniDraygon_Func_8(void) {  // 0xA88A34
  MiniDraygon_Func_3();
}

void MiniDraygon_Func_9(void) {  // 0xA88A3B
  MiniDraygon_Func_11();
  Enemy_MiniDraygon *E = Get_MiniDraygon(cur_enemy_index);
  AddToHiLo(&E->base.x_pos, &E->base.x_subpos, __PAIR32__(E->mdn_var_07, E->mdn_var_08));
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(E->mdn_var_09, E->mdn_var_0A));
}

void MiniDraygon_Func_10(void) {  // 0xA88A78
  if (!enemy_drawing_queue[(cur_enemy_index >> 1) + 77]) {
    Enemy_MiniDraygon *E = Get_MiniDraygon(cur_enemy_index);
    if (E->mdn_var_0C) {
      MiniDraygon_Func_3();
      E->base.x_pos += E->mdn_var_F;
    } else {
      E->mdn_var_02 = addr_kMiniDraygon_Ilist_876F;
      MiniDraygon_Func_12();
      E->mdn_var_0C = 0;
      E->mdn_var_0B = 0;
      E->mdn_var_C = FUNC16(MiniDraygon_Func_8);
    }
  }
}

void MiniDraygon_Func_11(void) {  // 0xA88AB1
  if (EnemyFunc_ADA3(0x100)) {
    if (!enemy_drawing_queue[(cur_enemy_index >> 1) + 77]) {
      Enemy_MiniDraygon *E = Get_MiniDraygon(cur_enemy_index);
      E->mdn_var_0B = 0;
      E->mdn_var_0C = 1;
      E->mdn_var_C = FUNC16(MiniDraygon_Func_10);
      E->mdn_var_0C = 1;
      E->mdn_var_02 = addr_kMiniDraygon_Ilist_8775;
      MiniDraygon_Func_12();
    }
  }
}

void MiniDraygon_Func_12(void) {  // 0xA88AE8
  Enemy_MiniDraygon *E = Get_MiniDraygon(cur_enemy_index);
  uint16 mdn_var_02 = E->mdn_var_02;
  if (mdn_var_02 != E->mdn_var_01) {
    E->base.current_instruction = mdn_var_02;
    E->mdn_var_01 = mdn_var_02;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
  }
}

void MiniDraygon_Touch(void) {  // 0xA88B06
  NormalEnemyTouchAi();
  MiniDraygon_Func_13();
}

void MiniDraygon_Powerbomb(void) {  // 0xA88B0C
  NormalEnemyPowerBombAi();
  MiniDraygon_Func_13();
}

void MiniDraygon_Shot(void) {  // 0xA88B12
  NormalEnemyShotAi();
  MiniDraygon_Func_13();
}

void MiniDraygon_Func_13(void) {  // 0xA88B16
  Enemy_MiniDraygon *E = Get_MiniDraygon(cur_enemy_index);
  Enemy_MiniDraygon *E1 = Get_MiniDraygon(cur_enemy_index + 64);
  Enemy_MiniDraygon *E2 = Get_MiniDraygon(cur_enemy_index + 128);
  if (!E->base.health) {
    E1->base.properties |= 0x200;
    E2->base.properties |= 0x200;
  }
  uint16 frozen_timer = E->base.frozen_timer;
  if (frozen_timer) {
    E1->base.frozen_timer = frozen_timer;
    E1->base.ai_handler_bits |= 4;
    if (E2->mdn_var_C != FUNC16(MiniDraygon_Func_9)) {
      E2->base.ai_handler_bits |= 4;
      E2->base.frozen_timer = E->base.frozen_timer;
    }
  }
}

