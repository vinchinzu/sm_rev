// HirisingSlowfalling extracted from sm_a2.c.
#include "ida_types.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "variables.h"
#include "enemy_ai_canon.h"

#define g_off_A2DF5E ((uint16*)RomFixedPtr(0xa2df5e))
#define g_off_A2DF6A ((uint16*)RomFixedPtr(0xa2df6a))


static uint16 HirisingSlowfalling_Func_1(uint16 k) {  // 0xA2DFCE
  Enemy_HirisingSlowfalling *E = Get_HirisingSlowfalling(k);
  return Mult8x8(E->hsg_parameter_1, E->hsg_var_F);
}

void HirisingSlowfalling_Init(void) {  // 0xA2DF76
  HirisingSlowfalling_Func_3(addr_kHirisingSlowfalling_Ilist_D82C);
  Enemy_HirisingSlowfalling *E = Get_HirisingSlowfalling(cur_enemy_index);
  E->hsg_var_A = FUNC16(HirisingSlowfalling_Func_4);
  E->hsg_var_00 = E->base.x_pos;
  E->hsg_var_01 = E->base.y_pos;
  int v2 = HIBYTE(E->hsg_parameter_1);
  E->hsg_var_E = g_off_A2DF5E[v2];
  E->hsg_var_F = *(uint16 *)RomPtr_A2(g_off_A2DF6A[v2]);
  uint16 r18 = HirisingSlowfalling_Func_1(cur_enemy_index);
  HirisingSlowfalling_Func_2(cur_enemy_index, r18);
  E->hsg_var_03 = E->hsg_var_01 - r18;
  E->hsg_var_02 = E->hsg_var_00;
  E->hsg_var_B = E->hsg_var_06;
}

void HirisingSlowfalling_Func_2(uint16 k, uint16 r18) {  // 0xA2DFE9
  uint32 v = 0;
  uint16 r24 = 0;
  do {
    r24 += 512;
    int v1 = 4 * (r24 >> 8);
    v += __PAIR32__(kCommonEnemySpeeds_Quadratic[v1 + 1], kCommonEnemySpeeds_Quadratic[v1]);
  } while (sign16((v >> 16) - r18));
  Get_HirisingSlowfalling(k)->hsg_var_06 = r24;
}

void HirisingSlowfalling_Func_3(uint16 a) {  // 0xA2E01E
  Enemy_HirisingSlowfalling *E = Get_HirisingSlowfalling(cur_enemy_index);
  E->base.current_instruction = a;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
}

void CallHirisingSlowfalling(uint32 ea) {
  switch (ea) {
  case fnHirisingSlowfalling_Func_4: HirisingSlowfalling_Func_4(); return;
  case fnHirisingSlowfalling_Func_5: HirisingSlowfalling_Func_5(); return;
  case fnHirisingSlowfalling_Func_6: HirisingSlowfalling_Func_6(); return;
  case fnHirisingSlowfalling_Func_7: HirisingSlowfalling_Func_7(); return;
  default: Unreachable();
  }
}

void HirisingSlowfalling_Main(void) {  // 0xA2E02E
  Enemy_HirisingSlowfalling *E = Get_HirisingSlowfalling(cur_enemy_index);
  CallHirisingSlowfalling(E->hsg_var_A | 0xA20000);
}

void HirisingSlowfalling_Func_4(void) {  // 0xA2E035
  if (IsSamusWithinEnemy_X(cur_enemy_index, 0x50)) {
    Enemy_HirisingSlowfalling *E = Get_HirisingSlowfalling(cur_enemy_index);
    E->hsg_var_07 = E->hsg_parameter_2;
    E->hsg_var_A = FUNC16(HirisingSlowfalling_Func_5);
  }
}

void HirisingSlowfalling_Func_5(void) {  // 0xA2E04F
  Enemy_HirisingSlowfalling *E = Get_HirisingSlowfalling(cur_enemy_index);
  int16 v1 = E->hsg_var_07 - 1;
  E->hsg_var_07 = v1;
  if (v1 < 0) {
    HirisingSlowfalling_Func_3(addr_kHirisingSlowfalling_Ilist_D834);
    E->hsg_var_A = FUNC16(HirisingSlowfalling_Func_6);
  }
}

void HirisingSlowfalling_Func_6(void) {  // 0xA2E06A
  Enemy_HirisingSlowfalling *E = Get_HirisingSlowfalling(cur_enemy_index);
  int v2 = 4 * HIBYTE(E->hsg_var_B);
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(kCommonEnemySpeeds_Quadratic[v2 + 3], kCommonEnemySpeeds_Quadratic[v2 + 2]));
  int16 v5 = E->hsg_var_B - 512;
  E->hsg_var_B = v5;
  if (v5 < 0) {
    E->base.x_pos = E->hsg_var_04 = E->hsg_var_02;
    E->base.y_pos = E->hsg_var_05 = E->hsg_var_03;
    E->hsg_var_C = 0;
    E->hsg_var_D = LOBYTE(E->hsg_parameter_1) - 1;
    HirisingSlowfalling_Func_3(addr_kHirisingSlowfalling_Ilist_D840);
    E->hsg_var_A = FUNC16(HirisingSlowfalling_Func_7);
  }
}

void HirisingSlowfalling_Func_7(void) {  // 0xA2E0CD
  Enemy_HirisingSlowfalling *E = Get_HirisingSlowfalling(cur_enemy_index);
  const uint8 *v1 = RomPtr_A2(E->hsg_var_E + 4 * HIBYTE(E->hsg_var_C));
  if (GET_WORD(v1) == 0x8000) {
    E->hsg_var_05 += E->hsg_var_F;
    E->hsg_var_C = 0;
    if ((--E->hsg_var_D & 0x8000) != 0) {
      E->hsg_var_B = E->hsg_var_06;
      E->base.x_pos = E->hsg_var_00;
      E->base.x_subpos = 0;
      E->base.y_pos = E->hsg_var_01;
      E->base.y_subpos = 0;
      HirisingSlowfalling_Func_3(addr_kHirisingSlowfalling_Ilist_D82C);
      E->hsg_var_A = FUNC16(HirisingSlowfalling_Func_4);
    }
  } else {
    E->base.x_pos = GET_WORD(v1) + E->hsg_var_04;
    E->base.y_pos = GET_WORD(v1 + 2) + E->hsg_var_05;
    E->hsg_var_C += 256;
  }
}
