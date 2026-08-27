// Enemy AI - Boulder — peeled from Bank $A6
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"

static const uint16 g_word_A686F1[2] = { 0x1000, 0x1800 };

void Boulder_Init(void) {  // 0xA686F5
  int16 boulder_parameter_2_high;

  Enemy_Boulder *E = Get_Boulder(cur_enemy_index);
  E->boulder_var_C = 0;
  E->boulder_var_B = 0;
  E->boulder_var_D = 2;
  E->boulder_var_A = FUNC16(Boulder_Func_1);
  E->boulder_var_04 = E->base.x_pos;
  uint16 y_pos = E->base.y_pos;
  E->boulder_var_05 = y_pos;
  E->boulder_var_03 = y_pos;
  E->boulder_var_02 = E->base.y_subpos;
  boulder_parameter_2_high = HIBYTE(E->boulder_parameter_2);
  if (!HIBYTE(E->boulder_parameter_2)) {
    boulder_parameter_2_high = 1;
    E->boulder_var_07 = 1;
  }
  E->base.y_pos = E->boulder_var_05 - boulder_parameter_2_high;
  E->boulder_var_00 = -LOBYTE(E->boulder_parameter_2);
  E->boulder_var_06 = LOBYTE(E->base.current_instruction);
  E->base.current_instruction = addr_kBoulder_Ilist_86A7;
  uint16 boulder_parameter_1_high = HIBYTE(E->boulder_parameter_1);
  E->boulder_var_E = boulder_parameter_1_high;
  if (!boulder_parameter_1_high) {
    E->boulder_var_00 = -E->boulder_var_00;
    E->base.current_instruction = addr_kBoulder_Ilist_86CB;
  }
  E->boulder_var_01 = 2;
  if (LOBYTE(E->boulder_parameter_1))
    E->boulder_var_01 = 5;
}

void CallBoulderFunc(uint32 ea) {
  switch (ea) {
  case fnBoulder_Func_1: Boulder_Func_1(); return;
  case fnBoulder_Func_2: Boulder_Func_2(); return;
  case fnBoulder_Func_3: Boulder_Func_3(); return;
  case fnBoulder_Func_4: Boulder_Func_4(); return;
  case fnBoulder_Func_5: Boulder_Func_5(); return;
  case fnBoulder_Func_6: Boulder_Func_6(); return;
  default: Unreachable();
  }
}

void Boulder_Main(void) {  // 0xA68793
  Enemy_Boulder *E = Get_Boulder(cur_enemy_index);
  CallBoulderFunc(E->boulder_var_A | 0xA60000);
}

void Boulder_Func_1(void) {  // 0xA6879A
  int16 SamusEnemyDelta_Y;
  int16 v3;
  int16 SamusEnemyDelta_X;

  SamusEnemyDelta_Y = GetSamusEnemyDelta_Y(cur_enemy_index);
  if (SamusEnemyDelta_Y >= 0) {
    Enemy_Boulder *E = Get_Boulder(cur_enemy_index);
    if ((int16)(SamusEnemyDelta_Y - E->boulder_var_06) < 0) {
      if (E->boulder_var_E) {
        SamusEnemyDelta_X = GetSamusEnemyDelta_X(cur_enemy_index);
        if (SamusEnemyDelta_X < 0 && (int16)(SamusEnemyDelta_X - E->boulder_var_00) >= 0) {
          E->boulder_var_A = FUNC16(Boulder_Func_2);
          if (E->boulder_var_07)
            E->boulder_var_A = FUNC16(Boulder_Func_5);
        }
      } else {
        v3 = GetSamusEnemyDelta_X(cur_enemy_index);
        if (v3 >= 0 && (int16)(v3 - E->boulder_var_00) < 0) {
          E->boulder_var_A = FUNC16(Boulder_Func_2);
          if (E->boulder_var_07)
            E->boulder_var_A = FUNC16(Boulder_Func_5);
        }
      }
    }
  }
}

void Boulder_Func_2(void) {  // 0xA687ED
  Enemy_Boulder *E = Get_Boulder(cur_enemy_index);
  Boulder_Func_9(8 * HIBYTE(E->boulder_var_C));
  if ((int16)(E->base.y_pos - E->boulder_var_05) < 0) {
    uint16 v1 = E->boulder_var_C + 256;
    E->boulder_var_C = v1;
    if (!sign16(v1 - 20480))
      E->boulder_var_C = 20480;
  } else {
    E->base.y_pos = E->boulder_var_05;
    E->boulder_var_A = FUNC16(Boulder_Func_3);
    E->boulder_var_C = 0x2000;
  }
}

void Boulder_Func_3(void) {  // 0xA68832
  int16 v1;

  Enemy_Boulder *E = Get_Boulder(cur_enemy_index);
  Boulder_Func_9(8 * HIBYTE(E->boulder_var_C) + 4);
  v1 = E->boulder_var_C - 256;
  E->boulder_var_C = v1;
  if (v1 >= 0) {
    uint16 v2 = 8 * HIBYTE(E->boulder_var_B);
    if (E->boulder_var_E)
      v2 += 4;
    Boulder_Func_7(v2);
    uint16 v3 = E->boulder_var_B + 32;
    E->boulder_var_B = v3;
    if (!sign16(v3 - 20480))
      E->boulder_var_B = 20480;
  } else {
    E->boulder_var_C = 0;
    E->boulder_var_A = FUNC16(Boulder_Func_4);
  }
}

void Boulder_Func_4(void) {  // 0xA6888B
  Enemy_Boulder *E = Get_Boulder(cur_enemy_index);
  int v1 = (8 * HIBYTE(E->boulder_var_C)) >> 1;
  if (Enemy_MoveDown(cur_enemy_index, kCommonEnemySpeeds_Quadratic32[v1 >> 1])) {
    QueueSfx2_Max6(0x42);
    if (E->boulder_var_E == 2) {
      E->base.properties |= kEnemyProps_Deleted;
      eproj_spawn_pt = (Point16U){ E->base.x_pos, E->base.y_pos };
      SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 0x11);
      QueueSfx2_Max6(0x43);
    } else {
      E->boulder_var_A = FUNC16(Boulder_Func_3);
      E->boulder_var_C = g_word_A686F1[E->boulder_var_D - 1];
      if ((--E->boulder_var_D & 0x8000) != 0) {
        E->boulder_var_03 = E->base.y_pos;
        E->boulder_var_02 = E->base.y_subpos;
        E->boulder_var_A = FUNC16(Boulder_Func_5);
      }
    }
  } else {
    E->boulder_var_C += 256;
    uint16 v3 = 8 * HIBYTE(E->boulder_var_B);
    if (E->boulder_var_E)
      v3 += 4;
    Boulder_Func_7(v3);
    uint16 v4 = E->boulder_var_B + 32;
    E->boulder_var_B = v4;
    if (!sign16(v4 - 20480))
      E->boulder_var_B = 20480;
  }
}

void Boulder_Func_5(void) {  // 0xA68942
  Enemy_Boulder *E = Get_Boulder(cur_enemy_index);
  uint16 v2 = 8 * HIBYTE(E->boulder_var_B);
  int v3 = v2 >> 1;
  Enemy_MoveDown(cur_enemy_index, kCommonEnemySpeeds_Quadratic32[v3 >> 1] + (E->boulder_var_01 << 16));
  E->base.y_pos -= E->boulder_var_01;
  if (E->boulder_var_E)
    v2 += 4;
  int v4 = v2 >> 1;
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, kCommonEnemySpeeds_Quadratic32[v4 >> 1])) {
    E->base.properties |= 0x300;
    E->boulder_var_A = FUNC16(Boulder_Func_6);
    QueueSfx2_Max6(0x42);
    eproj_spawn_pt = (Point16U){ E->base.x_pos, E->base.y_pos };
    SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 0x11);
    QueueSfx2_Max6(0x43);
  } else {
    uint16 v5 = E->boulder_var_B + 64;
    E->boulder_var_B = v5;
    if (!sign16(v5 - 0x4000))
      E->boulder_var_B = 0x4000;
    if (E->base.y_pos == E->boulder_var_03 && E->base.y_subpos == E->boulder_var_02)
      E->boulder_var_01 = 0;
  }
  E->boulder_var_03 = E->base.y_pos;
  E->boulder_var_02 = E->base.y_subpos;
}

void Boulder_Func_6(void) {  // 0xA689FC
  ;
}

void Boulder_Func_7(uint16 j) {  // 0xA68A00
  Enemy_Boulder *E = Get_Boulder(cur_enemy_index);
  int v3 = j >> 1;
  AddToHiLo(&E->base.x_pos, &E->base.x_subpos, __PAIR32__(kCommonEnemySpeeds_Quadratic[v3 + 1], kCommonEnemySpeeds_Quadratic[v3]));
}

void Boulder_Func_8(uint16 j) {  // 0xA68A1D
  Enemy_Boulder *E = Get_Boulder(cur_enemy_index);
  int v3 = j >> 1;
  AddToHiLo(&E->base.x_pos, &E->base.x_subpos, __PAIR32__(kCommonEnemySpeeds_Linear[v3 + 1], kCommonEnemySpeeds_Linear[v3]));
}

void Boulder_Func_9(uint16 j) {  // 0xA68A3A
  Enemy_Boulder *E = Get_Boulder(cur_enemy_index);
  int v3 = j >> 1;
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(kCommonEnemySpeeds_Quadratic[v3 + 1], kCommonEnemySpeeds_Quadratic[v3]));
}
