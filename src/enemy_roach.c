// Enemy AI - Roach — peeled from Bank $A3
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

#define g_off_A3A111 ((uint16*)RomFixedPtr(0xa3a111))
#define g_off_A3A121 ((uint16*)RomFixedPtr(0xa3a121))

void Roach_Func_1(void) {  // 0xA3A12F
  Enemy_Roach *E = Get_Roach(cur_enemy_index);
  uint16 roach_var_24 = E->roach_var_24;
  if (roach_var_24 != E->roach_var_25) {
    E->roach_var_25 = roach_var_24;
    E->base.current_instruction = roach_var_24;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
  }
}

void Roach_Init(void) {  // 0xA3A14D
  Enemy_Roach *E = Get_Roach(cur_enemy_index);
  E->roach_var_25 = 0;
  E->roach_var_24 = 0;
  Roach_Func_2(cur_enemy_index);
  Roach_Func_4(cur_enemy_index);
  Roach_Func_5(cur_enemy_index);
  Roach_Func_6(cur_enemy_index);
  Roach_Func_7(cur_enemy_index);
  Roach_Func_8(cur_enemy_index);
  E->roach_var_24 = g_off_A3A111[E->roach_var_20 >> 1];
  Roach_Func_1();
  E->roach_var_B = FUNC16(Roach_Func_9);
}

void Roach_Func_2(uint16 k) {  // 0xA3A183
  Enemy_Roach *E = Get_Roach(k);
  Point32 pt = ConvertAngleToXy(HIBYTE(E->roach_parameter_1), LOBYTE(E->roach_parameter_1));
  SetHiLo(&E->roach_var_01, &E->roach_var_00, pt.x);
  SetHiLo(&E->roach_var_03, &E->roach_var_02, pt.y);
}

void Roach_Func_3(uint16 k) {  // 0xA3A1B0
  Enemy_Roach *E = Get_Roach(k);
  SetHiLo(&E->roach_var_01, &E->roach_var_00, CosineMult8bitFull(HIBYTE(E->roach_parameter_1), LOBYTE(E->roach_parameter_1)));
  SetHiLo(&E->roach_var_03, &E->roach_var_02, SineMult8bitFull(HIBYTE(E->roach_parameter_1), LOBYTE(E->roach_parameter_1)));
}

void Roach_Func_4(uint16 k) {  // 0xA3A1F3
  Enemy_Roach *E = Get_Roach(k);
  SetHiLo(&E->roach_var_05, &E->roach_var_04, CosineMult8bitFull((uint8)(HIBYTE(E->roach_parameter_1) - 32), LOBYTE(E->roach_parameter_1)));
  SetHiLo(&E->roach_var_07, &E->roach_var_06, SineMult8bitFull((uint8)(HIBYTE(E->roach_parameter_1) - 32), LOBYTE(E->roach_parameter_1)));
}

void Roach_Func_5(uint16 k) {  // 0xA3A23E
  Enemy_Roach *E = Get_Roach(k);
  SetHiLo(&E->roach_var_09, &E->roach_var_08, CosineMult8bitFull((uint8)(HIBYTE(E->roach_parameter_1) + 32), LOBYTE(E->roach_parameter_1)));
  SetHiLo(&E->roach_var_0B, &E->roach_var_0A, SineMult8bitFull((uint8)(HIBYTE(E->roach_parameter_1) + 32), LOBYTE(E->roach_parameter_1)));
}

void Roach_Func_6(uint16 k) {  // 0xA3A289
  Enemy_Roach *E = Get_Roach(k);
  E->roach_var_20 = 2 * ((uint8)(HIBYTE(E->roach_parameter_1) - 48) >> 5);
}

void Roach_Func_7(uint16 k) {  // 0xA3A29E
  Enemy_Roach *E = Get_Roach(k);
  E->roach_var_21 = 2 * ((uint8)(HIBYTE(E->roach_parameter_1) - 80) >> 5);
}

void Roach_Func_8(uint16 k) {  // 0xA3A2B7
  Enemy_Roach *E = Get_Roach(k);
  E->roach_var_22 = 2 * ((uint8)(HIBYTE(E->roach_parameter_1) - 48 + 32) >> 5);
}
void CallRoachFunc(uint32 ea) {
  switch (ea) {
  case fnRoach_Func_9: Roach_Func_9(); return;  // 0xa3a2d7
  case fnRoach_Func_10: Roach_Func_10(); return;  // 0xa3a301
  case fnRoach_Func_11: Roach_Func_11(); return;  // 0xa3a30b
  case fnRoach_Func_12: Roach_Func_12(); return;  // 0xa3a315
  case fnRoach_Func_13: Roach_Func_13(); return;  // 0xa3a325
  case fnRoach_Func_14: Roach_Func_14(); return;  // 0xa3a33b
  case fnRoach_Func_15: Roach_Func_15(); return;  // 0xa3a34b
  case fnRoach_Func_16: Roach_Func_16(); return;  // 0xa3a380
  case fnRoach_Func_19: Roach_Func_19(); return;  // 0xa3a407
  case fnRoach_Func_20: Roach_Func_20(); return;  // 0xa3a40e
  case fnRoach_Func_21: Roach_Func_21(); return;  // 0xa3a440
  case fnRoach_Func_22: Roach_Func_22(); return;  // 0xa3a447
  case fnRoach_Func_23: Roach_Func_23(); return;  // 0xa3a44e
  case fnRoach_Func_24: Roach_Func_24(); return;  // 0xa3a462
  case fnRoach_Func_25: Roach_Func_25(); return;  // 0xa3a476
  case fnRoach_Func_26: Roach_Func_26(); return;  // 0xa3a4b6
  case fnRoach_Func_27: Roach_Func_27(); return;  // 0xa3a4f0
  default: Unreachable();
  }
}


void Roach_Main(void) {  // 0xA3A2D0
  Enemy_Roach *E = Get_Roach(cur_enemy_index);
  CallRoachFunc(E->roach_var_B | 0xA30000);
}

void Roach_Func_9(void) {  // 0xA3A2D7
  Enemy_Roach *E = Get_Roach(cur_enemy_index);
  if (IsSamusWithinEnemy_X(cur_enemy_index, LOBYTE(E->roach_parameter_2))) {
    if (IsSamusWithinEnemy_Y(cur_enemy_index, LOBYTE(E->roach_parameter_2)))
      E->roach_var_B = g_off_A3A121[HIBYTE(E->roach_parameter_2)];
  }
}

void Roach_Func_10(void) {  // 0xA3A301
  Get_Roach(cur_enemy_index)->roach_var_B = FUNC16(Roach_Func_19);
}

void Roach_Func_11(void) {  // 0xA3A30B
  Get_Roach(cur_enemy_index)->roach_var_B = FUNC16(Roach_Func_20);
}

void Roach_Func_12(void) {  // 0xA3A315
  random_number = 11;
  Get_Roach(cur_enemy_index)->roach_var_B = FUNC16(Roach_Func_27);
}

void Roach_Func_13(void) {  // 0xA3A325
  Enemy_Roach *E = Get_Roach(cur_enemy_index);
  E->roach_var_A = 512;
  random_number = 11;
  E->roach_var_B = FUNC16(Roach_Func_26);
}

void Roach_Func_14(void) {  // 0xA3A33B
  Enemy_Roach *E = Get_Roach(cur_enemy_index);
  E->roach_var_F = 32;
  E->roach_var_B = FUNC16(Roach_Func_23);
}

void Roach_Func_15(void) {  // 0xA3A34B
  uint8 v1 = 64 - CalculateAngleOfSamusFromEnemy(cur_enemy_index);
  Enemy_Roach *E = Get_Roach(cur_enemy_index);
  E->roach_var_C = v1;
  Roach_Func_18(cur_enemy_index);
  Roach_Func_17(cur_enemy_index);
  E->roach_var_24 = g_off_A3A111[E->roach_var_23 >> 1];
  Roach_Func_1();
  E->roach_var_B = FUNC16(Roach_Func_21);
}

void Roach_Func_16(void) {  // 0xA3A380
  uint16 v1 = (uint8)(64 - CalculateAngleOfSamusFromEnemy(cur_enemy_index) + 0x80);
  Enemy_Roach *E = Get_Roach(cur_enemy_index);
  E->roach_var_C = v1;
  Roach_Func_18(cur_enemy_index);
  Roach_Func_17(cur_enemy_index);
  E->roach_var_24 = g_off_A3A111[E->roach_var_23 >> 1];
  Roach_Func_1();
  E->roach_var_B = FUNC16(Roach_Func_22);
}

void Roach_Func_17(uint16 k) {  // 0xA3A3B5
  Enemy_Roach *E = Get_Roach(k);
  E->roach_var_23 = 2 * ((uint8)(E->roach_var_C - 48) >> 5);
}

void Roach_Func_18(uint16 k) {  // 0xA3A3CA
  Enemy_Roach *E = Get_Roach(k);
  SetHiLo(&E->roach_var_0D, &E->roach_var_0C, CosineMult8bitFull(E->roach_var_C, LOBYTE(E->roach_parameter_1)));
  SetHiLo(&E->roach_var_0F, &E->roach_var_0E, SineMult8bitFull(E->roach_var_C, LOBYTE(E->roach_parameter_1)));
}

void Roach_Func_19(void) {  // 0xA3A407
  Roach_Func_29(cur_enemy_index);
}

void Roach_Func_20(void) {  // 0xA3A40E
  Enemy_Roach *E = Get_Roach(cur_enemy_index);
  if ((E->base.frame_counter & 0x10) != 0) {
    E->roach_var_24 = g_off_A3A111[E->roach_var_22 >> 1];
    Roach_Func_1();
    Roach_Func_31(cur_enemy_index);
  } else {
    E->roach_var_24 = g_off_A3A111[E->roach_var_21 >> 1];
    Roach_Func_1();
    Roach_Func_30(cur_enemy_index);
  }
}

void Roach_Func_21(void) {  // 0xA3A440
  Roach_Func_32(cur_enemy_index);
}

void Roach_Func_22(void) {  // 0xA3A447
  Roach_Func_32(cur_enemy_index);
}

void Roach_Func_23(void) {  // 0xA3A44E
  Enemy_Roach *E = Get_Roach(cur_enemy_index);
  if ((--E->roach_var_F & 0x8000) != 0)
    E->roach_var_B = FUNC16(Roach_Func_9);
  else
    Roach_Func_29(cur_enemy_index);
}

void Roach_Func_24(void) {  // 0xA3A462
  Enemy_Roach *E = Get_Roach(cur_enemy_index);
  if ((--E->roach_var_F & 0x8000) != 0)
    E->roach_var_B = FUNC16(Roach_Func_27);
  else
    Roach_Func_33(cur_enemy_index);
}

void Roach_Func_25(void) {  // 0xA3A476
  Enemy_Roach *E = Get_Roach(cur_enemy_index);
  bool v2 = (--E->roach_var_A & 0x8000) != 0;
  if (v2) {
LABEL_6:
    Roach_Func_32(cur_enemy_index);
    return;
  }
  v2 = (--E->roach_var_F & 0x8000) != 0;
  if (!v2) {
    uint16 v3 = Abs16(E->base.x_pos - samus_x_pos);
    if (!sign16(v3 - 96)) {
      uint16 v4 = Abs16(E->base.y_pos - samus_y_pos);
      if (!sign16(v4 - 96))
        Roach_Func_28();
    }
    goto LABEL_6;
  }
  E->roach_var_B = FUNC16(Roach_Func_26);
}

void Roach_Func_26(void) {  // 0xA3A4B6
  uint8 v1 = NextRandom() - 64;
  Enemy_Roach *E = Get_Roach(cur_enemy_index);
  E->roach_var_C += v1;
  Roach_Func_18(cur_enemy_index);
  Roach_Func_17(cur_enemy_index);
  E->roach_var_24 = g_off_A3A111[E->roach_var_23 >> 1];
  Roach_Func_1();
  E->roach_var_F = 32;
  E->roach_var_B = FUNC16(Roach_Func_25);
}

void Roach_Func_27(void) {  // 0xA3A4F0
  uint8 v1 = NextRandom() - 64;
  Enemy_Roach *E = Get_Roach(cur_enemy_index);
  E->roach_var_C += v1;
  Roach_Func_18(cur_enemy_index);
  Roach_Func_17(cur_enemy_index);
  E->roach_var_24 = g_off_A3A111[E->roach_var_23 >> 1];
  Roach_Func_1();
  E->roach_var_F = 32;
  E->roach_var_B = FUNC16(Roach_Func_24);
}

void Roach_Func_28(void) {  // 0xA3A52A
  Enemy_Roach *E = Get_Roach(cur_enemy_index);
  E->roach_var_0D = -E->roach_var_0D;
  E->roach_var_0C = -E->roach_var_0C;
  E->roach_var_0F = -E->roach_var_0F;
  E->roach_var_0E = -E->roach_var_0E;
  uint16 v1 = (E->roach_var_23 + 4) & 7;
  E->roach_var_23 = v1;
  E->roach_var_24 = g_off_A3A111[v1 >> 1];
  Roach_Func_1();
}

void Roach_Func_29(uint16 k) {  // 0xA3A578
  Enemy_Roach *E = Get_Roach(k);
  uint16 varE20 = HIBYTE(E->roach_parameter_1);
  EnemyFunc_B691(varE20, (Point32) { __PAIR32__(E->roach_var_01, E->roach_var_00), __PAIR32__(E->roach_var_03, E->roach_var_02) });
}

void Roach_Func_30(uint16 k) {  // 0xA3A5A3
  Enemy_Roach *E = Get_Roach(k);
  AddToHiLo(&E->base.x_pos, &E->base.x_subpos, __PAIR32__(E->roach_var_05, E->roach_var_04));
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(E->roach_var_07, E->roach_var_06));
}

void Roach_Func_31(uint16 k) {  // 0xA3A5DA
  Enemy_Roach *E = Get_Roach(k);
  AddToHiLo(&E->base.x_pos, &E->base.x_subpos, __PAIR32__(E->roach_var_09, E->roach_var_08));
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(E->roach_var_0B, E->roach_var_0A));
}

void Roach_Func_32(uint16 k) {  // 0xA3A611
  Enemy_Roach *E = Get_Roach(k);
  AddToHiLo(&E->base.x_pos, &E->base.x_subpos, __PAIR32__(E->roach_var_0D, E->roach_var_0C));
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(E->roach_var_0F, E->roach_var_0E));
}

void Roach_Func_33(uint16 k) {  // 0xA3A648
  Enemy_Roach *E = Get_Roach(k);
  if (Enemy_MoveRight_IgnoreSlopes(k, __PAIR32__(E->roach_var_0D, E->roach_var_0C))) {
    E->roach_var_B = FUNC16(Roach_Func_9);
  } else {
    if (Enemy_MoveDown(k, __PAIR32__(E->roach_var_0F, E->roach_var_0E)))
      E->roach_var_B = FUNC16(Roach_Func_9);
  }
}
