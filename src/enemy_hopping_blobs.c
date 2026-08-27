// ThinHoppingBlobs / TwinHoppingBlobs extracted from sm_a2.c.
#include "ida_types.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "variables.h"
#include "enemy_ai_canon.h"

#define g_word_A29A07 (*(uint16*)RomFixedPtr(0xa29a07))
#define g_word_A29A09 (*(uint16*)RomFixedPtr(0xa29a09))
#define g_word_A29A0B (*(uint16*)RomFixedPtr(0xa29a0b))
#define g_word_A29A0D (*(uint16*)RomFixedPtr(0xa29a0d))


void ThinHoppingBlobs_Init(void) {  // 0xA29A3F
  Enemy_ThinHoppingBlobs *E = Get_ThinHoppingBlobs(cur_enemy_index);
  E->base.spritemap_pointer = addr_kSpritemap_Nothing_A2;
  E->thbs_var_A = 0;
  sub_A29A6C(addr_kThinHoppingBlobs_Ilist_99AD);
  E->thbs_var_00 = 0;
  E->thbs_var_D = FUNC16(ThinHoppingBlobs_Func6);
  E->thbs_var_C = E->thbs_parameter_1;
  E->thbs_var_04 = 0;
}

void sub_A29A6C(uint16 a) {  // 0xA29A6C
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  v1->current_instruction = a;
  v1->instruction_timer = 1;
  v1->timer = 0;
}

void CallThinHoppingBlobs(uint32 ea) {
  switch (ea) {
  case fnThinHoppingBlobs_Func6: ThinHoppingBlobs_Func6(cur_enemy_index); return;
  case fnThinHoppingBlobs_Func7: ThinHoppingBlobs_Func7(); return;
  default: Unreachable();
  }
}

void ThinHoppingBlobs_Main(void) {  // 0xA29A7D
  Enemy_ThinHoppingBlobs *E = Get_ThinHoppingBlobs(cur_enemy_index);
  CallThinHoppingBlobs(E->thbs_var_D | 0xA20000);
}

void ThinHoppingBlobs_Func1(uint16 k) {  // 0xA29A84
  Enemy_ThinHoppingBlobs *E = Get_ThinHoppingBlobs(k);
  if (sign16(E->thbs_var_00 - 3))
    TwinHoppingBlobs_Func2();
  ThinHoppingBlobs_Func3();
  ThinHoppingBlobs_Func5();
}

void TwinHoppingBlobs_Func2(void) {  // 0xA29A9B
  Enemy_ThinHoppingBlobs *E = Get_ThinHoppingBlobs(cur_enemy_index);
  E->thbs_var_00 = IsSamusWithinEnemy_X(cur_enemy_index, E->thbs_parameter_2);
}

void ThinHoppingBlobs_Func3(void) {  // 0xA29AAA
  Enemy_ThinHoppingBlobs *E = Get_ThinHoppingBlobs(cur_enemy_index);
  E->thbs_var_02 = 1;
  if ((GetSamusEnemyDelta_X(cur_enemy_index) & 0x8000) == 0)
    E->thbs_var_02 = 0;
  if (E->thbs_var_04)
    E->thbs_var_02 = E->thbs_var_05;
  E->thbs_var_04 = 0;
  uint16 R28 = ThinHoppingBlobs_Func4();
  uint16 thbs_var_00 = E->thbs_var_00;
  if (sign16(thbs_var_00 - 3)) {
    if (!thbs_var_00)
      R28 &= 1;
    thbs_var_00 = R28;
    if (!sign16(R28 - 2))
      thbs_var_00 = 2;
  }
  uint16 v3 = 8 * thbs_var_00;
  E->thbs_var_F = v3;
  E->thbs_var_E = *(uint16 *)((uint8 *)&g_word_A29A0D + v3);
}

uint16 ThinHoppingBlobs_Func4(void) {  // 0xA29B06
  NextRandom();
  return (Get_ThinHoppingBlobs(cur_enemy_index)->base.frame_counter + random_number) & 7;
}

void ThinHoppingBlobs_Func5(void) {  // 0xA29B1A
  uint16 v2;
  Enemy_ThinHoppingBlobs *E = Get_ThinHoppingBlobs(cur_enemy_index);
  uint16 thbs_var_F = E->thbs_var_F;
  uint16 r22 = 0;
  uint16 r24 = 0;
  do {
    r22 += *(uint16 *)((uint8 *)&g_word_A29A09 + thbs_var_F);
    r24 += *(uint16 *)((uint8 *)kCommonEnemySpeeds_Quadratic + (8 * ((r22 & 0xFF00) >> 8)) + 1);
    thbs_var_F = E->thbs_var_F;
    v2 = swap16(*(uint16 *)((uint8 *)&g_word_A29A07 + thbs_var_F));
  } while (!sign16(v2 - r24));
  E->thbs_var_B = r22;
  E->thbs_var_03 = 0;
  uint16 v3 = r22 >> 1;
  E->thbs_var_07 = r22 >> 1;
  E->thbs_var_06 = E->thbs_var_07 + (v3 >> 1);
}

void ThinHoppingBlobs_Func6(uint16 k) {  // 0xA29B65
  Enemy_ThinHoppingBlobs *E = Get_ThinHoppingBlobs(k);
  if ((--E->thbs_var_C & 0x8000) != 0) {
    E->thbs_var_D = FUNC16(ThinHoppingBlobs_Func7);
    E->thbs_var_C = E->thbs_parameter_1;
    E->thbs_var_01 = 1;
    ThinHoppingBlobs_Func1(k);
  }
}

void CallThinHoppingBlobsVarE(uint32 ea) {
  switch (ea) {
  case fnThinHoppingBlobs_Func11: ThinHoppingBlobs_Func11(); return;
  case fnThinHoppingBlobs_Func12: ThinHoppingBlobs_Func12(); return;
  case fnThinHoppingBlobs_Func13: ThinHoppingBlobs_Func13(); return;
  case fnThinHoppingBlobs_Func14: ThinHoppingBlobs_Func14(); return;
  case fnThinHoppingBlobs_Func15: ThinHoppingBlobs_Func15(); return;
  case fnThinHoppingBlobs_Func16: ThinHoppingBlobs_Func16(); return;
  default: Unreachable();
  }
}

void ThinHoppingBlobs_Func7(void) {  // 0xA29B81
  Enemy_ThinHoppingBlobs *E = Get_ThinHoppingBlobs(cur_enemy_index);
  CallThinHoppingBlobsVarE(E->thbs_var_E | 0xA20000);
}

void ThinHoppingBlobs_Func8(void) {  // 0xA29B88
  Enemy_ThinHoppingBlobs *E = Get_ThinHoppingBlobs(cur_enemy_index);
  uint16 thbs_var_B = E->thbs_var_B;
  if (!sign16(thbs_var_B - 0x4000))
    thbs_var_B = 0x4000;
  uint16 v6 = 8 * (thbs_var_B >> 8);
  if (!E->thbs_var_03)
    v6 += 4;
  if (Enemy_MoveDown(cur_enemy_index, kCommonEnemySpeeds_Quadratic32[v6 >> 2])) {
    if (E->thbs_var_03) {
      E->thbs_var_D = FUNC16(ThinHoppingBlobs_Func6);
    } else {
      E->thbs_var_04 = 1;
      E->thbs_var_05 = E->thbs_var_02 ^ 1;
      E->thbs_var_00 = 4;
      E->thbs_var_E = FUNC16(ThinHoppingBlobs_Func15);
    }
    E->thbs_var_01 = 0;
  } else {
    uint16 thbs_var_F = E->thbs_var_F;
    if (E->thbs_var_03) {
      ThinHoppingBlobs_Func10();
      E->thbs_var_B += *(uint16 *)((uint8 *)&g_word_A29A0B + thbs_var_F);
    } else {
      ThinHoppingBlobs_Func9();
      E->thbs_var_B -= *(uint16 *)((uint8 *)&g_word_A29A0B + thbs_var_F);
    }
    if (sign16(E->thbs_var_B)) {
      E->thbs_var_03 = 1;
      E->thbs_var_B = 0;
    }
    uint16 R20 = (*(uint16 *)((uint8 *)&g_word_A29A09 + E->thbs_var_F) & 0xFF00) >> 8;
    if (E->thbs_var_02)
      R20 = -R20;
    if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, INT16_SHL16(R20))) {
      E->thbs_var_04 = 1;
      E->thbs_var_05 = E->thbs_var_02 ^ 1;
      E->thbs_var_01 = 0;
      E->thbs_var_00 = 4;
      E->thbs_var_E = FUNC16(ThinHoppingBlobs_Func15);
    }
  }
}

void ThinHoppingBlobs_Func9(void) {  // 0xA29C71
  Enemy_ThinHoppingBlobs *E = Get_ThinHoppingBlobs(cur_enemy_index);
  uint16 v2;
  if (E->thbs_var_02) {
    uint16 thbs_var_B = E->thbs_var_B;
    if ((int16)(thbs_var_B - E->thbs_var_06) >= 0) {
      v2 = addr_kThinHoppingBlobs_Ilist_9A01;
    } else if ((int16)(thbs_var_B - E->thbs_var_07) >= 0) {
      v2 = addr_kThinHoppingBlobs_Ilist_99FB;
    } else {
      v2 = addr_kThinHoppingBlobs_Ilist_99F5;
    }
  } else {
    uint16 v1 = E->thbs_var_B;
    if ((int16)(v1 - E->thbs_var_06) >= 0) {
      v2 = addr_kThinHoppingBlobs_Ilist_99E9;
    } else if ((int16)(v1 - E->thbs_var_07) >= 0) {
      v2 = addr_kThinHoppingBlobs_Ilist_99EF;
    } else {
      v2 = addr_kThinHoppingBlobs_Ilist_99F5;
    }
  }
  sub_A29A6C(v2);
}

void ThinHoppingBlobs_Func10(void) {  // 0xA29CBE
  Enemy_ThinHoppingBlobs *E = Get_ThinHoppingBlobs(cur_enemy_index);
  uint16 v2;
  if (E->thbs_var_02) {
    uint16 thbs_var_B = E->thbs_var_B;
    if ((int16)(thbs_var_B - E->thbs_var_07) < 0) {
      v2 = addr_kThinHoppingBlobs_Ilist_99F5;
    } else if ((int16)(thbs_var_B - E->thbs_var_06) < 0) {
      v2 = addr_kThinHoppingBlobs_Ilist_99EF;
    } else {
      v2 = addr_kThinHoppingBlobs_Ilist_99E9;
    }
  } else {
    uint16 v1 = E->thbs_var_B;
    if ((int16)(v1 - E->thbs_var_07) < 0) {
      v2 = addr_kThinHoppingBlobs_Ilist_99F5;
    } else if ((int16)(v1 - E->thbs_var_06) < 0) {
      v2 = addr_kThinHoppingBlobs_Ilist_99FB;
    } else {
      v2 = addr_kThinHoppingBlobs_Ilist_9A01;
    }
  }
  sub_A29A6C(v2);
}

void ThinHoppingBlobs_Func11(void) {  // 0xA29D0B

  ThinHoppingBlobs_Func8();
  Enemy_ThinHoppingBlobs *E = Get_ThinHoppingBlobs(cur_enemy_index);
  if (E->thbs_var_04 || !E->thbs_var_01) {
    E->thbs_var_01 = 0;
    sub_A29A6C(addr_kThinHoppingBlobs_Ilist_99D5);
  }
}

void ThinHoppingBlobs_Func12(void) {  // 0xA29D2B

  ThinHoppingBlobs_Func8();
  Enemy_ThinHoppingBlobs *E = Get_ThinHoppingBlobs(cur_enemy_index);
  if (E->thbs_var_04 || !E->thbs_var_01) {
    E->thbs_var_01 = 0;
    sub_A29A6C(addr_kThinHoppingBlobs_Ilist_99C1);
  }
}

void ThinHoppingBlobs_Func13(void) {  // 0xA29D4B

  ThinHoppingBlobs_Func8();
  Enemy_ThinHoppingBlobs *E = Get_ThinHoppingBlobs(cur_enemy_index);
  if (E->thbs_var_04 || !E->thbs_var_01) {
    E->thbs_var_01 = 0;
    sub_A29A6C(addr_kThinHoppingBlobs_Ilist_99AD);
  }
}

void ThinHoppingBlobs_Func14(void) {  // 0xA29D6B

  ThinHoppingBlobs_Func8();
  Enemy_ThinHoppingBlobs *E = Get_ThinHoppingBlobs(cur_enemy_index);
  if (!E->thbs_var_04) {
    if (E->thbs_var_01)
      return;
    E->thbs_var_00 = 0;
    E->thbs_var_D = FUNC16(ThinHoppingBlobs_Func6);
  }
  E->thbs_var_01 = 0;
  sub_A29A6C(addr_kThinHoppingBlobs_Ilist_99D5);
}

void ThinHoppingBlobs_Func15(void) {  // 0xA29D98
  Enemy_ThinHoppingBlobs *E = Get_ThinHoppingBlobs(cur_enemy_index);
  uint16 thbs_var_F = E->thbs_var_F;
  uint16 R20 = (*(uint16 *)((uint8 *)&g_word_A29A0B + thbs_var_F) & 0xFF00) >> 8;
  uint16 R18 = *((uint8 *)&g_word_A29A0B + thbs_var_F) << 8;
  if (Enemy_MoveDown(cur_enemy_index, __PAIR32__(R20, R18))) {
    uint16 R28 = ThinHoppingBlobs_Func4();
    E->thbs_var_00 = (R28 & 1) + 5;
    E->thbs_var_D = FUNC16(ThinHoppingBlobs_Func6);
  }
}

void ThinHoppingBlobs_Func16(void) {  // 0xA29DCD

  ThinHoppingBlobs_Func8();
  Enemy_ThinHoppingBlobs *E = Get_ThinHoppingBlobs(cur_enemy_index);
  if (!E->thbs_var_01) {
    E->thbs_var_01 = 0;
    E->thbs_var_00 = 3;
    E->thbs_var_D = FUNC16(ThinHoppingBlobs_Func6);
    sub_A29A6C(addr_kThinHoppingBlobs_Ilist_99D5);
  }
}
