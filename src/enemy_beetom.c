// Enemy AI - Beetom — peeled from Bank $A8

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

#define kBeetom_Ilist_B74E ((uint16*)RomFixedPtr(0xa8b74e))

const uint16 *Beetom_Instr_1(uint16 k, const uint16 *jp) {  // 0xA8B75E
  return jp;
}

void Beetom_Func_1(void) {  // 0xA8B762
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  E->base.current_instruction = E->beetom_var_00;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
}

void Beetom_Init(void) {  // 0xA8B776
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  E->beetom_var_D = 0;
  E->beetom_var_C = 0;
  E->beetom_var_05 = 0;
  E->beetom_var_08 = 0;
  E->beetom_var_A = 0;
  E->beetom_var_E = 64;
  E->beetom_var_F = joypad1_lastkeys;
  random_number = 23;
  E->beetom_var_02 = Beetom_Func_2(12288, 4);
  E->beetom_var_03 = Beetom_Func_2(0x4000, 5);
  E->beetom_var_04 = Beetom_Func_2(12288, 3);
  E->beetom_var_00 = addr_kBeetom_Ilist_B6F2;
  if ((GetSamusEnemyDelta_X(cur_enemy_index) & 0x8000) != 0)
    E->beetom_var_00 = addr_kBeetom_Ilist_B696;
  Beetom_Func_1();
  E->beetom_var_C = FUNC16(Beetom_Func_3);
}

uint16 Beetom_Func_2(uint16 r22, uint16 r24) {  // 0xA8B7EF
  uint16 r18 = 0, r20 = 0;
  do {
    r18 += r24;
    r20 += *(uint16 *)((uint8 *)kCommonEnemySpeeds_Quadratic + (8 * r18) + 1);
  } while (sign16(r20 - r22));
  return r18;
}

void Beetom_Func_3(void) {  // 0xA8B814
  if (IsSamusWithinEnemy_X(cur_enemy_index, 0x60))
    Get_Beetom(cur_enemy_index)->beetom_var_C = FUNC16(Beetom_Func_12);
  else
    Get_Beetom(cur_enemy_index)->beetom_var_C = FUNC16(Beetom_Func_4);
}

void Beetom_Func_4(void) {  // 0xA8B82F
  NextRandom();
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  E->beetom_var_C = kBeetom_Ilist_B74E[random_number & 7];
  E->beetom_var_09 = random_number & 1;
}

void Beetom_Func_5(void) {  // 0xA8B84F
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  E->beetom_var_D = 32;
  E->beetom_var_C = FUNC16(Beetom_Func_17);
}

void Beetom_Func_6(void) {  // 0xA8B85F
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  E->beetom_var_C = FUNC16(Beetom_Func_18);
  E->beetom_var_00 = addr_kBeetom_Ilist_B696;
  Beetom_Func_1();
}

void Beetom_Func_7(void) {  // 0xA8B873
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  E->beetom_var_C = FUNC16(Beetom_Func_19);
  E->beetom_var_00 = addr_kBeetom_Ilist_B6F2;
  Beetom_Func_1();
}

void Beetom_Func_8(void) {  // 0xA8B887
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  E->beetom_var_B = E->beetom_var_02;
  E->beetom_var_C = FUNC16(Beetom_Func_20);
  E->beetom_var_05 = 0;
  E->beetom_var_00 = addr_kBeetom_Ilist_B6AC;
  Beetom_Func_1();
}

void Beetom_Func_9(void) {  // 0xA8B8A9
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  E->beetom_var_B = E->beetom_var_02;
  E->beetom_var_C = FUNC16(Beetom_Func_21);
  E->beetom_var_05 = 0;
  E->beetom_var_00 = addr_kBeetom_Ilist_B708;
  Beetom_Func_1();
}

void Beetom_Func_10(void) {  // 0xA8B8CB
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  E->beetom_var_B = E->beetom_var_03;
  E->beetom_var_C = FUNC16(Beetom_Func_24);
  E->beetom_var_05 = 0;
  E->beetom_var_00 = addr_kBeetom_Ilist_B6AC;
  Beetom_Func_1();
}

void Beetom_Func_11(void) {  // 0xA8B8ED
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  E->beetom_var_B = E->beetom_var_03;
  E->beetom_var_C = FUNC16(Beetom_Func_25);
  E->beetom_var_05 = 0;
  E->beetom_var_00 = addr_kBeetom_Ilist_B708;
  Beetom_Func_1();
}

void Beetom_Func_12(void) {  // 0xA8B90F
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  E->beetom_var_B = E->beetom_var_04;
  E->beetom_var_00 = addr_kBeetom_Ilist_B708;
  E->beetom_var_C = FUNC16(Beetom_Func_29);
  E->beetom_var_09 = 1;
  if ((GetSamusEnemyDelta_X(cur_enemy_index) & 0x8000) != 0) {
    E->beetom_var_00 = addr_kBeetom_Ilist_B6AC;
    E->beetom_var_C = FUNC16(Beetom_Func_28);
    E->beetom_var_09 = 0;
  }
  Beetom_Func_1();
  E->beetom_var_05 = 0;
}

void Beetom_Func_13(void) {  // 0xA8B952
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  E->beetom_var_00 = addr_kBeetom_Ilist_B6CC;
  Beetom_Func_1();
  E->beetom_var_C = FUNC16(Beetom_Func_32);
}

void Beetom_Func_14(void) {  // 0xA8B966
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  E->beetom_var_00 = addr_kBeetom_Ilist_B728;
  Beetom_Func_1();
  E->beetom_var_C = FUNC16(Beetom_Func_33);
}

void Beetom_Func_15(void) {  // 0xA8B97A
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  E->beetom_var_00 = addr_kBeetom_Ilist_B696;
  if (E->beetom_var_09)
    E->beetom_var_00 = addr_kBeetom_Ilist_B6F2;
  Beetom_Func_1();
  E->beetom_var_05 = 0;
  E->beetom_var_C = FUNC16(Beetom_Func_35);
}

void Beetom_Func_16(void) {  // 0xA8B9A2
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  E->beetom_var_B = 0;
  E->beetom_var_C = FUNC16(Beetom_Func_36);
}

void Beetom_Func_17(void) {  // 0xA8B9B2
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  if ((--E->beetom_var_D & 0x8000) != 0)
    E->beetom_var_C = FUNC16(Beetom_Func_3);
}

void Beetom_Func_18(void) {  // 0xA8B9C1
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  if ((--E->beetom_var_D & 0x8000) != 0) {
    E->beetom_var_D = 64;
    E->beetom_var_C = FUNC16(Beetom_Func_3);
  } else {
    E->base.x_pos -= 8;
    if (Enemy_MoveDown(cur_enemy_index, INT16_SHL16(1))) {
      E->base.x_pos += 8;
      if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, -16384))
        E->beetom_var_C = FUNC16(Beetom_Func_7);
    } else {
      E->beetom_var_C = FUNC16(Beetom_Func_7);
      --E->base.y_pos;
      E->base.x_pos += 8;
    }
  }
}

void Beetom_Func_19(void) {  // 0xA8BA24
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  if ((--E->beetom_var_D & 0x8000) != 0) {
    E->beetom_var_D = 64;
    E->beetom_var_C = FUNC16(Beetom_Func_3);
  } else {
    E->base.x_pos += 8;
    if (Enemy_MoveDown(cur_enemy_index, INT16_SHL16(1))) {
      E->base.x_pos -= 8;
      if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, 0x4000))
        E->beetom_var_C = FUNC16(Beetom_Func_6);
    } else {
      E->beetom_var_C = FUNC16(Beetom_Func_6);
      --E->base.y_pos;
      E->base.x_pos -= 8;
    }
  }
}

void Beetom_Func_22(uint16 k) {  // 0xA8BAE7
  Enemy_Beetom *E = Get_Beetom(k);
  int v2 = (8 * E->beetom_var_B) >> 1;
  if (Enemy_MoveDown(k, kCommonEnemySpeeds_Quadratic32[(v2 + 2) >> 1])) {
    E->beetom_var_C = FUNC16(Beetom_Func_15);
  } else {
    E->beetom_var_B -= 4;
    if (sign16(E->beetom_var_B)) {
      E->beetom_var_B = 0;
      E->beetom_var_05 = 1;
    }
  }
}

void Beetom_Func_23(uint16 k) {  // 0xA8BB20
  Enemy_Beetom *E = Get_Beetom(k);
  int v2 = (8 * E->beetom_var_B) >> 1;
  if (Enemy_MoveDown(k, kCommonEnemySpeeds_Quadratic32[v2 >> 1])) {
    E->beetom_var_C = FUNC16(Beetom_Func_3);
  } else {
    E->beetom_var_B += 4;
    if (!sign16(E->beetom_var_B - 64))
      E->beetom_var_B = 64;
  }
}

void Beetom_Func_20(void) {  // 0xA8BA84
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  if (E->beetom_var_05)
    Beetom_Func_23(cur_enemy_index);
  else
    Beetom_Func_22(cur_enemy_index);
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, -16384)) {
    E->beetom_var_09 ^= 1;
    E->beetom_var_C = FUNC16(Beetom_Func_15);
  }
}

void Beetom_Func_21(void) {  // 0xA8BAB7
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  if (E->beetom_var_05)
    Beetom_Func_23(cur_enemy_index);
  else
    Beetom_Func_22(cur_enemy_index);
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, 0x4000)) {
    E->beetom_var_09 ^= 1;
    E->beetom_var_C = FUNC16(Beetom_Func_15);
  }
}

void Beetom_Func_26(uint16 k) {  // 0xA8BBB8
  Enemy_Beetom *E = Get_Beetom(k);
  int v2 = (8 * E->beetom_var_B) >> 1;
  if (Enemy_MoveDown(k, kCommonEnemySpeeds_Quadratic32[(v2 + 2) >> 1])) {
    E->beetom_var_C = FUNC16(Beetom_Func_15);
  } else {
    E->beetom_var_B -= 5;
    if (sign16(E->beetom_var_B)) {
      E->beetom_var_B = 0;
      E->beetom_var_05 = 1;
    }
  }
}

void Beetom_Func_27(uint16 k) {  // 0xA8BBF1
  Enemy_Beetom *E = Get_Beetom(k);
  int v2 = (8 * E->beetom_var_B) >> 1;
  if (Enemy_MoveDown(k, kCommonEnemySpeeds_Quadratic32[v2 >> 1])) {
    E->beetom_var_C = FUNC16(Beetom_Func_3);
  } else {
    E->beetom_var_B += 5;
    if (!sign16(E->beetom_var_B - 64))
      E->beetom_var_B = 64;
  }
}

void Beetom_Func_24(void) {  // 0xA8BB55
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  if (E->beetom_var_05)
    Beetom_Func_27(cur_enemy_index);
  else
    Beetom_Func_26(cur_enemy_index);
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, -16384)) {
    E->beetom_var_09 ^= 1;
    E->beetom_var_C = FUNC16(Beetom_Func_15);
  }
}

void Beetom_Func_25(void) {  // 0xA8BB88
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  if (E->beetom_var_05)
    Beetom_Func_27(cur_enemy_index);
  else
    Beetom_Func_26(cur_enemy_index);
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, 0x4000)) {
    E->beetom_var_09 ^= 1;
    E->beetom_var_C = FUNC16(Beetom_Func_15);
  }
}


void Beetom_Func_30(uint16 k) {  // 0xA8BC8A
  Enemy_Beetom *E = Get_Beetom(k);
  int v2 = (8 * E->beetom_var_B) >> 1;
  if (Enemy_MoveDown(k, kCommonEnemySpeeds_Quadratic32[(v2 + 2) >> 1])) {
    E->beetom_var_C = FUNC16(Beetom_Func_15);
  } else {
    int16 v3 = E->beetom_var_B - 5;
    E->beetom_var_B = v3;
    if (v3 < 0) {
      E->beetom_var_B = 0;
      E->beetom_var_05 = 1;
    }
  }
}

void Beetom_Func_31(uint16 k) {  // 0xA8BCC3
  Enemy_Beetom *E = Get_Beetom(k);
  int v2 = (8 * E->beetom_var_B) >> 1;
  if (Enemy_MoveDown(k, kCommonEnemySpeeds_Quadratic32[v2 >> 1])) {
    E->beetom_var_C = FUNC16(Beetom_Func_3);
  } else {
    uint16 v3 = E->beetom_var_B + 3;
    E->beetom_var_B = v3;
    if (!sign16(v3 - 64))
      E->beetom_var_B = 64;
  }
}
void Beetom_Func_28(void) {  // 0xA8BC26
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  if (E->beetom_var_05)
    Beetom_Func_31(cur_enemy_index);
  else
    Beetom_Func_30(cur_enemy_index);
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, __PAIR32__(-3, 0))) {
    E->beetom_var_09 ^= 1;
    E->beetom_var_C = FUNC16(Beetom_Func_15);
  }
}

void Beetom_Func_29(void) {  // 0xA8BC5A
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  if (E->beetom_var_05)
    Beetom_Func_31(cur_enemy_index);
  else
    Beetom_Func_30(cur_enemy_index);
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, __PAIR32__(3, 0))) {
    E->beetom_var_09 ^= 1;
    E->beetom_var_C = FUNC16(Beetom_Func_15);
  }
}

void Beetom_Func_34(void) {  // 0xA8BD8C
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  if (joypad1_lastkeys != E->beetom_var_F) {
    E->beetom_var_F = joypad1_lastkeys;
    --E->beetom_var_E;
  }
}

void Beetom_Func_32(void) {  // 0xA8BCF8
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  if (E->beetom_var_E) {
    E->base.x_pos = samus_x_pos;
    E->base.y_pos = samus_y_pos - 4;
    Beetom_Func_34();
  } else {
    if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, __PAIR32__(16, 0))) {
      E->beetom_var_09 = 1;
      Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, __PAIR32__(-32, 0));
    }
    E->beetom_var_08 = 0;
    E->beetom_var_C = FUNC16(Beetom_Func_16);
  }
}

void Beetom_Func_33(void) {  // 0xA8BD42
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  if (E->beetom_var_E) {
    E->base.x_pos = samus_x_pos;
    E->base.y_pos = samus_y_pos - 4;
    Beetom_Func_34();
  } else {
    if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, __PAIR32__(-16, 0))) {
      E->beetom_var_09 = 0;
      Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, __PAIR32__(32, 0));
    }
    E->beetom_var_08 = 0;
    E->beetom_var_C = FUNC16(Beetom_Func_16);
  }
}

void Beetom_Func_35(void) {  // 0xA8BD9D
  if (Enemy_MoveDown(cur_enemy_index, __PAIR32__(3, 0))) {
    Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
    if (E->beetom_var_09)
      E->beetom_var_C = FUNC16(Beetom_Func_7);
    else
      E->beetom_var_C = FUNC16(Beetom_Func_6);
  }
}

void Beetom_Func_37(uint16 k) {  // 0xA8BDCC
  Enemy_Beetom *E = Get_Beetom(k);
  int v2 = (8 * E->beetom_var_B) >> 1;
  if (Enemy_MoveDown(k, kCommonEnemySpeeds_Quadratic32[v2 >> 1])) {
    E->beetom_var_C = FUNC16(Beetom_Func_3);
  } else {
    uint16 v3 = E->beetom_var_B + 1;
    E->beetom_var_B = v3;
    if (!sign16(v3 - 64))
      E->beetom_var_B = 64;
  }
  uint16 v4 = (E->beetom_var_09) ? -2 : 2;
  if (Enemy_MoveRight_IgnoreSlopes(k, __PAIR32__(v4, 0))) {
    E->beetom_var_09 ^= 1;
    E->beetom_var_C = FUNC16(Beetom_Func_15);
  }
}

void Beetom_Func_36(void) {  // 0xA8BDC5
  Beetom_Func_37(cur_enemy_index);
}

void CallBeetomFunc(uint32 ea) {
  switch (ea) {
  case fnBeetom_Func_3: Beetom_Func_3(); return;  // 0xa8b814
  case fnBeetom_Func_4: Beetom_Func_4(); return;  // 0xa8b82f
  case fnBeetom_Func_5: Beetom_Func_5(); return;  // 0xa8b84f
  case fnBeetom_Func_6: Beetom_Func_6(); return;  // 0xa8b85f
  case fnBeetom_Func_7: Beetom_Func_7(); return;  // 0xa8b873
  case fnBeetom_Func_8: Beetom_Func_8(); return;  // 0xa8b887
  case fnBeetom_Func_9: Beetom_Func_9(); return;  // 0xa8b8a9
  case fnBeetom_Func_10: Beetom_Func_10(); return;  // 0xa8b8cb
  case fnBeetom_Func_11: Beetom_Func_11(); return;  // 0xa8b8ed
  case fnBeetom_Func_12: Beetom_Func_12(); return;  // 0xa8b90f
  case fnBeetom_Func_13: Beetom_Func_13(); return;  // 0xa8b952
  case fnBeetom_Func_14: Beetom_Func_14(); return;  // 0xa8b966
  case fnBeetom_Func_15: Beetom_Func_15(); return;  // 0xa8b97a
  case fnBeetom_Func_16: Beetom_Func_16(); return;  // 0xa8b9a2
  case fnBeetom_Func_17: Beetom_Func_17(); return;  // 0xa8b9b2
  case fnBeetom_Func_18: Beetom_Func_18(); return;  // 0xa8b9c1
  case fnBeetom_Func_19: Beetom_Func_19(); return;  // 0xa8ba24
  case fnBeetom_Func_20: Beetom_Func_20(); return;  // 0xa8ba84
  case fnBeetom_Func_21: Beetom_Func_21(); return;  // 0xa8bab7
  case fnBeetom_Func_24: Beetom_Func_24(); return;  // 0xa8bb55
  case fnBeetom_Func_25: Beetom_Func_25(); return;  // 0xa8bb88
  case fnBeetom_Func_28: Beetom_Func_28(); return;  // 0xa8bc26
  case fnBeetom_Func_29: Beetom_Func_29(); return;  // 0xa8bc5a
  case fnBeetom_Func_32: Beetom_Func_32(); return;  // 0xa8bcf8
  case fnBeetom_Func_33: Beetom_Func_33(); return;  // 0xa8bd42
  case fnBeetom_Func_35: Beetom_Func_35(); return;  // 0xa8bd9d
  case fnBeetom_Func_36: Beetom_Func_36(); return;  // 0xa8bdc5
  default: Unreachable();
  }
}

void Beetom_Main(void) {  // 0xA8B80D
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  CallBeetomFunc(E->beetom_var_C | 0xA80000);
}

void Beetom_Touch(void) {  // 0xA8BE2E
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  if (!E->beetom_var_08) {
    if (E->beetom_var_09)
      E->beetom_var_C = FUNC16(Beetom_Func_14);
    else
      E->beetom_var_C = FUNC16(Beetom_Func_13);
    E->beetom_var_08 = 1;
    E->beetom_var_E = 64;
    E->base.layer = 2;
    E->beetom_var_06 = samus_x_pos - E->base.x_pos;
    E->beetom_var_07 = samus_y_pos - E->base.y_pos;
  }
  if (samus_contact_damage_index)
    goto LABEL_11;
  if ((random_enemy_counter & 7) == 7 && !sign16(samus_health - 30))
    QueueSfx3_Max6(0x2D);
  if ((E->base.frame_counter & 0x3F) == 63) {
LABEL_11:
    NormalEnemyTouchAi();
    samus_invincibility_timer = 0;
    samus_knockback_timer = 0;
  }
}

void Beetom_Shot(void) {  // 0xA8BEAC
  NormalEnemyShotAi();
  Enemy_Beetom *E = Get_Beetom(cur_enemy_index);
  if (E->base.frozen_timer
      && (E->beetom_var_C == FUNC16(Beetom_Func_33)
          || E->beetom_var_C == FUNC16(Beetom_Func_32))) {
    E->beetom_var_C = FUNC16(Beetom_Func_15);
  }
  E->beetom_var_08 = 0;
}

