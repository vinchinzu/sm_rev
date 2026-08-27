// TimedShutter / HorizontalShootableShutter / ShootableShutter / RisingFallingPlatform extracted from sm_a2.c.
#include "ida_types.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "variables.h"
#include "enemy_ai_canon.h"

#define g_word_A2EA56 ((uint16*)RomFixedPtr(0xa2ea56))
#define g_off_A2EA4E ((uint16*)RomFixedPtr(0xa2ea4e))
#define g_off_A2EC3A ((uint16*)RomFixedPtr(0xa2ec3a))
#define g_off_A2EDFB ((uint16*)RomFixedPtr(0xa2edfb))
#define g_off_A2F107 ((uint16*)RomFixedPtr(0xa2f107))


void TimedShutter_Init(void) {  // 0xA2E9DA
  Enemy_TimedShutter *E = Get_TimedShutter(cur_enemy_index);
  E->tsr_var_A = g_off_A2EA4E[E->base.current_instruction + 2 * E->base.extra_properties];
  uint16 y_pos = E->base.y_pos;
  uint16 v3;
  if (E->base.extra_properties) {
    E->tsr_var_B = y_pos;
    uint16 v2 = y_pos - 8;
    E->tsr_var_C = v2;
    v2 -= 8;
    E->tsr_var_D = v2;
    v3 = v2 - 8;
  } else {
    E->tsr_var_B = y_pos;
    uint16 v4 = y_pos + 8;
    E->tsr_var_C = v4;
    v4 += 8;
    E->tsr_var_D = v4;
    v3 = v4 + 8;
  }
  E->tsr_var_E = v3;
  E->base.extra_properties = 0;
  E->tsr_var_F = 0;
  E->base.current_instruction = addr_kTimedShutter_Ilist_E998;
  int v5 = (uint16)(4 * LOBYTE(E->tsr_parameter_2)) >> 1;
  E->tsr_var_00 = g_word_A2EA56[v5];
  E->tsr_var_01 = g_word_A2EA56[v5 + 1];
}

void CallTimedShutterFunc(uint32 ea, uint16 k) {
  switch (ea) {
  case fnTimedShutter_Func_1: TimedShutter_Func_1(k); return;
  case fnTimedShutter_Func_2: TimedShutter_Func_2(k); return;
  case fnTimedShutter_Func_3: TimedShutter_Func_3(k); return;
  case fnTimedShutter_Func_4: TimedShutter_Func_4(k); return;
  case fnTimedShutter_Func_5: TimedShutter_Func_5(k); return;
  case fnTimedShutter_Func_10: TimedShutter_Func_10(k); return;
  default: Unreachable();
  }
}

void TimedShutter_Main(void) {  // 0xA2EAB6
  Enemy_TimedShutter *E = Get_TimedShutter(cur_enemy_index);
  CallTimedShutterFunc(E->tsr_var_A | 0xA20000, cur_enemy_index);
}

void TimedShutter_Func_1(uint16 k) {  // 0xA2EABD
  Enemy_TimedShutter *E = Get_TimedShutter(k);
  uint16 tsr_parameter_1 = E->tsr_parameter_1;
  if (tsr_parameter_1) {
    E->tsr_parameter_1 = tsr_parameter_1 - 1;
  } else {
    RisingFallingPlatform_Func_8();
    E->tsr_var_A = FUNC16(TimedShutter_Func_10);
  }
}

void TimedShutter_Func_2(uint16 k) {  // 0xA2EAD1
  Enemy_TimedShutter *E = Get_TimedShutter(k);
  if (IsSamusWithinEnemy_X(k, E->tsr_parameter_1)) {
    RisingFallingPlatform_Func_8();
    E->tsr_var_A = FUNC16(TimedShutter_Func_10);
  }
}

void TimedShutter_Func_3(uint16 k) {  // 0xA2EAE7
  Enemy_TimedShutter *E = Get_TimedShutter(k);
  if (IsSamusWithinEnemy_X(k, E->tsr_parameter_1)) {
    RisingFallingPlatform_Func_8();
    E->tsr_var_A = FUNC16(TimedShutter_Func_5);
  }
}

void TimedShutter_Func_4(uint16 k) {  // 0xA2EAFD
  Enemy_TimedShutter *E = Get_TimedShutter(k);
  uint16 tsr_parameter_1 = E->tsr_parameter_1;
  if (tsr_parameter_1) {
    E->tsr_parameter_1 = tsr_parameter_1 - 1;
  } else {
    RisingFallingPlatform_Func_8();
    E->tsr_var_A = FUNC16(TimedShutter_Func_5);
  }
}

void TimedShutter_Func_Null(void) {
  ;
}


static Func_V *const off_A2EB1A[5] = { TimedShutter_Func_6, TimedShutter_Func_7, TimedShutter_Func_8, TimedShutter_Func_9, TimedShutter_Func_Null };

void TimedShutter_Func_5(uint16 k) {  // 0xA2EB11
  int v1 = Get_TimedShutter(k)->tsr_var_F;
  off_A2EB1A[v1]();
}

void TimedShutter_Func_6(void) {  // 0xA2EB25
  Enemy_TimedShutter *E = Get_TimedShutter(cur_enemy_index);
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(E->tsr_var_00, E->tsr_var_01));
  if ((int16)(E->tsr_var_B + 16 - E->base.y_pos) < 0) {
    E->base.y_pos = E->tsr_var_B + 9;
    ++E->tsr_var_F;
    E->base.instruction_timer = 1;
    E->base.current_instruction = addr_kTimedShutter_Ilist_E99E;
    E->base.y_height = 16;
  }
}

void TimedShutter_Func_7(void) {  // 0xA2EB66
  Enemy_TimedShutter *E = Get_TimedShutter(cur_enemy_index);
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(E->tsr_var_00, E->tsr_var_01));
  if ((int16)(E->tsr_var_C + 16 - E->base.y_pos) < 0) {
    E->base.y_pos = E->tsr_var_C + 9;
    ++E->tsr_var_F;
    E->base.instruction_timer = 1;
    E->base.current_instruction = addr_kTimedShutter_Ilist_E9A4;
    E->base.y_height = 24;
  }
}

void TimedShutter_Func_8(void) {  // 0xA2EBA7
  Enemy_TimedShutter *E = Get_TimedShutter(cur_enemy_index);
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(E->tsr_var_00, E->tsr_var_01));
  if ((int16)(E->tsr_var_D + 16 - E->base.y_pos) < 0) {
    E->base.y_pos = E->tsr_var_D + 9;
    ++E->tsr_var_F;
    E->base.instruction_timer = 1;
    E->base.current_instruction = addr_kTimedShutter_Ilist_E9AA;
    E->base.y_height = 32;
  }
}

void TimedShutter_Func_9(void) {  // 0xA2EBE8
  Enemy_TimedShutter *E = Get_TimedShutter(cur_enemy_index);
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(E->tsr_var_00, E->tsr_var_01));
  uint16 v3 = E->tsr_var_E + 16;
  if ((int16)(v3 - E->base.y_pos) < 0) {
    E->base.y_pos = v3;
    ++E->tsr_var_F;
  }
}

void TimedShutter_Func_Null2(void) {

}

static Func_V *const off_A2EC3A[5] = {  // 0xA2EC13
  TimedShutter_Func_11,
  TimedShutter_Func_12,
  TimedShutter_Func_13,
  TimedShutter_Func_14,
  TimedShutter_Func_Null2,
};

void TimedShutter_Func_10(uint16 k) {
  Enemy_TimedShutter *E = Get_TimedShutter(k);
  E->tsr_var_40 = E->base.y_pos;
  uint16 v2 = 2 * E->tsr_var_F;
  off_A2EC3A[v2 >> 1]();
  if (CheckIfEnemyTouchesSamus(k)) {
    int16 v4 = E->base.y_pos - E->tsr_var_40;
    if (v4 < 0)
      extra_samus_y_displacement += v4;
  }
}

void TimedShutter_Func_11(void) {  // 0xA2EC45
  Enemy_TimedShutter *E = Get_TimedShutter(cur_enemy_index);
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, -IPAIR32(E->tsr_var_00, E->tsr_var_01));
  int16 v3 = E->tsr_var_B - 16;
  if ((int16)(v3 - E->base.y_pos) >= 0) {
    E->base.y_pos = v3 + 7;
    ++E->tsr_var_F;
    E->base.instruction_timer = 1;
    E->base.current_instruction = addr_kTimedShutter_Ilist_E99E;
    E->base.y_height = 16;
  }
}

void TimedShutter_Func_12(void) {  // 0xA2EC86
  Enemy_TimedShutter *E = Get_TimedShutter(cur_enemy_index);
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, -IPAIR32(E->tsr_var_00, E->tsr_var_01));
  int16 v3 = E->tsr_var_C - 16;
  if ((int16)(v3 - E->base.y_pos) >= 0) {
    E->base.y_pos = v3 + 7;
    ++E->tsr_var_F;
    E->base.instruction_timer = 1;
    E->base.current_instruction = addr_kTimedShutter_Ilist_E9A4;
    E->base.y_height = 24;
  }
}

void TimedShutter_Func_13(void) {  // 0xA2ECC7
  Enemy_TimedShutter *E = Get_TimedShutter(cur_enemy_index);
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, -IPAIR32(E->tsr_var_00, E->tsr_var_01));
  int16 v3 = E->tsr_var_D - 16;
  if ((int16)(v3 - E->base.y_pos) >= 0) {
    E->base.y_pos = v3 + 7;
    ++E->tsr_var_F;
    E->base.instruction_timer = 1;
    E->base.current_instruction = addr_kTimedShutter_Ilist_E9AA;
    E->base.y_height = 32;
  }
}

void TimedShutter_Func_14(void) {  // 0xA2ED08
  Enemy_TimedShutter *E = Get_TimedShutter(cur_enemy_index);
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, -IPAIR32(E->tsr_var_00, E->tsr_var_01));
  uint16 v3 = E->tsr_var_E - 16;
  if ((int16)(v3 - E->base.y_pos) >= 0) {
    E->base.y_pos = v3;
    ++E->tsr_var_F;
  }
}

void TimedShutter_Func_15(void) {  // 0xA2ED33
  ;
}

void RisingFallingPlatform_Init(void) {  // 0xA2EE05
  RisingFallingPlatform_Func_1(cur_enemy_index);
  Get_RisingFallingPlatform(cur_enemy_index)->base.current_instruction = addr_kRisingFallingPlatform_Ilist_EDE7;
}

void ShootableShutter_Init(void) {  // 0xA2EE12
  RisingFallingPlatform_Func_1(cur_enemy_index);
  gEnemyData(cur_enemy_index)->current_instruction = addr_kTimedShutter_Ilist_E9AA;
}

void RisingFallingPlatform_Func_1(uint16 k) {  // 0xA2EE1F
  Enemy_RisingFallingPlatform *E = Get_RisingFallingPlatform(k);
  uint16 current_instruction_low = LOBYTE(E->base.current_instruction);
  E->rfpm_var_00 = current_instruction_low;
  int v3 = (8 * current_instruction_low) >> 1;
  E->rfpm_var_D = kCommonEnemySpeeds_Linear[v3];
  E->rfpm_var_C = kCommonEnemySpeeds_Linear[v3 + 1];
  E->rfpm_var_F = kCommonEnemySpeeds_Linear[v3 + 2];
  E->rfpm_var_E = kCommonEnemySpeeds_Linear[v3 + 3];
  uint16 current_instruction_high = HIBYTE(E->base.current_instruction);
  E->rfpm_var_01 = current_instruction_high;
  E->rfpm_var_20 = current_instruction_high;
  uint16 extra_properties_low = LOBYTE(E->base.extra_properties);
  E->rfpm_var_02 = extra_properties_low;
  E->rfpm_var_08 = 16 * extra_properties_low;
  uint16 extra_properties_high = HIBYTE(E->base.extra_properties);
  E->rfpm_var_03 = extra_properties_high;
  E->rfpm_var_09 = 16 * extra_properties_high;
  uint16 rfpm_parameter_1_low = LOBYTE(E->rfpm_parameter_1);
  E->rfpm_var_04 = rfpm_parameter_1_low;
  E->rfpm_var_07 = 2 * rfpm_parameter_1_low;
  E->rfpm_var_05 = HIBYTE(E->rfpm_parameter_1);
  uint16 rfpm_parameter_2 = E->rfpm_parameter_2;
  E->rfpm_var_06 = rfpm_parameter_2;
  E->rfpm_var_B = rfpm_parameter_2;
  uint16 y_pos = E->base.y_pos;
  E->rfpm_var_0F = y_pos;
  E->rfpm_var_10 = E->rfpm_var_05 + y_pos;
  if (!E->rfpm_var_01) {
    uint16 v10 = E->base.y_pos;
    E->rfpm_var_10 = v10;
    E->rfpm_var_0F = v10 - E->rfpm_var_05;
  }
  E->rfpm_var_A = FUNC16(RisingFallingPlatform_Func_2);
  E->base.extra_properties = 0;
  E->rfpm_var_0A = 0;
}

void CallRisingFallingPlatformFunc(uint32 ea) {
  switch (ea) {
  case fnRisingFallingPlatform_Func_2: RisingFallingPlatform_Func_2(); return;
  case fnRisingFallingPlatform_Func_4: RisingFallingPlatform_Func_4(); return;
  case fnRisingFallingPlatform_Func_9: RisingFallingPlatform_Func_9(); return;
  case fnRisingFallingPlatform_Func_10: RisingFallingPlatform_Func_10(); return;
  case fnRisingFallingPlatform_Func_11: RisingFallingPlatform_Func_11(); return;
  case fnRisingFallingPlatform_Func_12: RisingFallingPlatform_Func_12(); return;
  case fnRisingFallingPlatform_Func_13: RisingFallingPlatform_Func_13(); return;
  case fnHorizontalShootableShutter_Func_2: HorizontalShootableShutter_Func_2(); return;
  case fnHorizontalShootableShutter_Func_4: HorizontalShootableShutter_Func_4(); return;
  case fnHorizontalShootableShutter_Func_8: HorizontalShootableShutter_Func_8(); return;
  case fnHorizontalShootableShutter_Func_9: HorizontalShootableShutter_Func_9(); return;
  case fnHorizontalShootableShutter_Func_12: HorizontalShootableShutter_Func_12(); return;
  case fnHorizontalShootableShutter_Func_13: HorizontalShootableShutter_Func_13(); return;
  case fnHorizontalShootableShutter_Func_14: HorizontalShootableShutter_Func_14(); return;
  default: Unreachable();
  }
}

void RisingFallingPlatform_Main(void) {  // 0xA2EED1
  Enemy_RisingFallingPlatform *E = Get_RisingFallingPlatform(cur_enemy_index);
  CallRisingFallingPlatformFunc(E->rfpm_var_A | 0xA20000);
  if (E->rfpm_var_A != FUNC16(RisingFallingPlatform_Func_9)
      && E->rfpm_var_A != FUNC16(RisingFallingPlatform_Func_10)
      && (enemy_index_colliding_dirs[3] & (uint16)(enemy_index_colliding_dirs[2] & enemy_index_colliding_dirs[1] & enemy_index_colliding_dirs[0])) != 0xFFFF
      && (enemy_index_colliding_dirs[3] & (uint16)(enemy_index_colliding_dirs[2] & enemy_index_colliding_dirs[1] & enemy_index_colliding_dirs[0])) == cur_enemy_index) {
    if (samus_contact_damage_index)
      RisingFallingPlatform_Powerbomb();
  }
}

static Func_V *const off_A2EDFB[5] = {  // 0xA2EF09
  RisingFallingPlatform_Func_3,
  RisingFallingPlatform_Func_4,
  RisingFallingPlatform_Func_5,
  RisingFallingPlatform_Func_6,
  RisingFallingPlatform_Func_6,
};

void RisingFallingPlatform_Func_2(void) {

  int v0 = Get_RisingFallingPlatform(cur_enemy_index)->rfpm_var_07 >> 1;
  off_A2EDFB[v0]();
}

void RisingFallingPlatform_Func_3(void) {  // 0xA2EF15
  Enemy_RisingFallingPlatform *E = Get_RisingFallingPlatform(cur_enemy_index);
  if (E->rfpm_var_B-- == 1) {
    E->rfpm_var_B = E->rfpm_var_06;
    RisingFallingPlatform_Func_7(cur_enemy_index);
  }
}

void RisingFallingPlatform_Func_4(void) {  // 0xA2EF28
  Enemy_RisingFallingPlatform *E = Get_RisingFallingPlatform(cur_enemy_index);
  if (IsSamusWithinEnemy_X(cur_enemy_index, E->rfpm_var_06))
    RisingFallingPlatform_Func_7(cur_enemy_index);
}

void RisingFallingPlatform_Func_5(void) {  // 0xA2EF39
  RisingFallingPlatform_Func_7(cur_enemy_index);
}

void RisingFallingPlatform_Func_6(void) {  // 0xA2EF40
  ;
}

void RisingFallingPlatform_Func_7(uint16 k) {  // 0xA2EF44
  Enemy_RisingFallingPlatform *E = Get_RisingFallingPlatform(k);
  E->rfpm_var_A = FUNC16(RisingFallingPlatform_Func_9);
  if (E->rfpm_var_01)
    E->rfpm_var_A = FUNC16(RisingFallingPlatform_Func_10);
  RisingFallingPlatform_Func_8();
}

void RisingFallingPlatform_Func_8(void) {  // 0xA2EF5A
  if (!CheckIfEnemyIsOnScreen())
    QueueSfx3_Max6(0xE);
}

void RisingFallingPlatform_Func_9(void) {  // 0xA2EF68
  Enemy_RisingFallingPlatform *E = Get_RisingFallingPlatform(cur_enemy_index);
  E->rfpm_var_0E = E->base.y_pos;
  E->rfpm_var_0A = 0;
  if (CheckIfEnemyTouchesSamus(cur_enemy_index))
    E->rfpm_var_0A = 1;
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(E->rfpm_var_F, E->rfpm_var_E));
  if (E->rfpm_var_0A)
    extra_samus_y_displacement = E->base.y_pos - E->rfpm_var_0E;
  if ((int16)(E->rfpm_var_0F - E->base.y_pos) >= 0) {
    if (E->rfpm_var_08 == 4080) {
      E->rfpm_var_A = FUNC16(RisingFallingPlatform_Func_13);
    } else {
      E->rfpm_var_B = E->rfpm_var_08;
      E->rfpm_var_A = FUNC16(RisingFallingPlatform_Func_11);
    }
  }
}

void RisingFallingPlatform_Func_10(void) {  // 0xA2EFD4
  Enemy_RisingFallingPlatform *E = Get_RisingFallingPlatform(cur_enemy_index);
  E->rfpm_var_0E = E->base.y_pos;
  E->rfpm_var_0A = 0;
  if (CheckIfEnemyTouchesSamus(cur_enemy_index))
    E->rfpm_var_0A = 1;
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(E->rfpm_var_D, E->rfpm_var_C));
  if (E->rfpm_var_0A)
    extra_samus_y_displacement = E->base.y_pos - E->rfpm_var_0E;
  if ((int16)(E->base.y_pos - E->rfpm_var_10) >= 0) {
    if (E->rfpm_var_09 == 4080) {
      E->rfpm_var_A = FUNC16(RisingFallingPlatform_Func_13);
    } else {
      E->rfpm_var_B = E->rfpm_var_09;
      E->rfpm_var_A = FUNC16(RisingFallingPlatform_Func_12);
    }
  }
}

void RisingFallingPlatform_Func_11(void) {  // 0xA2F040
  Enemy_RisingFallingPlatform *E = Get_RisingFallingPlatform(cur_enemy_index);
  if ((--E->rfpm_var_B & 0x8000) != 0) {
    E->rfpm_var_A = FUNC16(RisingFallingPlatform_Func_10);
    if (E->rfpm_var_04 == 1 && E->rfpm_var_01)
      E->rfpm_var_A = FUNC16(RisingFallingPlatform_Func_4);
    if (Get_RisingFallingPlatform(cur_enemy_index)->base.enemy_ptr != 0xD83F)
      RisingFallingPlatform_Func_8();
  }
}

void RisingFallingPlatform_Func_12(void) {  // 0xA2F072
  Enemy_RisingFallingPlatform *E = Get_RisingFallingPlatform(cur_enemy_index);
  if ((--E->rfpm_var_B & 0x8000) != 0) {
    RisingFallingPlatform_Func_8();
    E->rfpm_var_A = FUNC16(RisingFallingPlatform_Func_9);
    if (E->rfpm_var_04 == 1 && !E->rfpm_var_01)
      E->rfpm_var_A = FUNC16(RisingFallingPlatform_Func_4);
  }
}

void RisingFallingPlatform_Func_13(void) {  // 0xA2F099
  ;
}

void RisingFallingPlatform_Touch(void) {  // 0xA2F09D
  RisingFallingPlatform_Powerbomb();
}

void RisingFallingPlatform_Shot(void) {  // 0xA2F0A2
  RisingFallingPlatform_Powerbomb();
}

void ShootableShutter_Shot(void) {  // 0xA2F0AA
  NormalEnemyShotAi();
  RisingFallingPlatform_Powerbomb();
}

void RisingFallingPlatform_Powerbomb(void) {  // 0xA2F0B6
  Enemy_RisingFallingPlatform *E = Get_RisingFallingPlatform(cur_enemy_index);
  if (sign16(E->rfpm_var_07 - 6)) {
LABEL_10:
    RisingFallingPlatform_Func_8();
    return;
  }
  if (E->rfpm_var_07 != 8) {
    if (E->rfpm_var_0C)
      return;
    E->rfpm_var_0C = 1;
  }
  if (E->rfpm_var_A != FUNC16(RisingFallingPlatform_Func_9)
      && E->rfpm_var_A != FUNC16(RisingFallingPlatform_Func_10)) {
    E->rfpm_var_A = FUNC16(RisingFallingPlatform_Func_9);
    if (E->rfpm_var_20)
      E->rfpm_var_A = FUNC16(RisingFallingPlatform_Func_10);
    E->rfpm_var_20 ^= 1;
    goto LABEL_10;
  }
}

void HorizontalShootableShutter_Init(void) {  // 0xA2F111
  HorizontalShootableShutter_Func_1(cur_enemy_index);
  Get_HorizontalShootableShutter(cur_enemy_index)->base.current_instruction = addr_kTimedShutter_Ilist_E9D4;
}

void HorizontalShootableShutter_Func_1(uint16 k) {  // 0xA2F11E
  Enemy_RisingFallingPlatform *E = Get_RisingFallingPlatform(k);
  uint16 current_instruction_low = LOBYTE(E->base.current_instruction);
  E->rfpm_var_00 = current_instruction_low;
  int v3 = (8 * current_instruction_low) >> 1;
  E->rfpm_var_D = kCommonEnemySpeeds_Linear[v3];
  E->rfpm_var_C = kCommonEnemySpeeds_Linear[v3 + 1];
  E->rfpm_var_F = kCommonEnemySpeeds_Linear[v3 + 2];
  E->rfpm_var_E = kCommonEnemySpeeds_Linear[v3 + 3];
  uint16 current_instruction_high = HIBYTE(E->base.current_instruction);
  E->rfpm_var_01 = current_instruction_high;
  E->rfpm_var_20 = current_instruction_high ^ 1;
  uint16 extra_properties_low = LOBYTE(E->base.extra_properties);
  E->rfpm_var_02 = extra_properties_low;
  E->rfpm_var_08 = 16 * extra_properties_low;
  uint16 extra_properties_high = HIBYTE(E->base.extra_properties);
  E->rfpm_var_03 = extra_properties_high;
  E->rfpm_var_09 = 16 * extra_properties_high;
  uint16 rfpm_parameter_1_low = LOBYTE(E->rfpm_parameter_1);
  E->rfpm_var_04 = rfpm_parameter_1_low;
  E->rfpm_var_07 = 2 * rfpm_parameter_1_low;
  E->rfpm_var_05 = HIBYTE(E->rfpm_parameter_1);
  uint16 rfpm_parameter_2 = E->rfpm_parameter_2;
  E->rfpm_var_06 = rfpm_parameter_2;
  E->rfpm_var_B = rfpm_parameter_2;
  uint16 x_pos = E->base.x_pos;
  E->rfpm_var_11 = x_pos;
  E->rfpm_var_12 = E->rfpm_var_05 + x_pos;
  if (!E->rfpm_var_01) {
    uint16 v10 = E->base.x_pos;
    E->rfpm_var_12 = v10;
    E->rfpm_var_11 = v10 - E->rfpm_var_05;
  }
  E->rfpm_var_A = FUNC16(HorizontalShootableShutter_Func_2);
  E->rfpm_var_15 = samus_x_pos;
  E->base.extra_properties = 0;
  E->rfpm_var_0A = 0;
  E->rfpm_var_0B = 0;
}

void HorizontalShootableShutter_Main(void) {  // 0xA2F1DE
  Enemy_RisingFallingPlatform *E = Get_RisingFallingPlatform(cur_enemy_index);
  CallRisingFallingPlatformFunc(E->rfpm_var_A | 0xA20000);
  if (E->rfpm_var_A != FUNC16(HorizontalShootableShutter_Func_8)
      && E->rfpm_var_A != FUNC16(HorizontalShootableShutter_Func_9)
      && (enemy_index_colliding_dirs[3] & (uint16)(enemy_index_colliding_dirs[2] & enemy_index_colliding_dirs[1] & enemy_index_colliding_dirs[0])) != 0xFFFF
      && (enemy_index_colliding_dirs[3] & (uint16)(enemy_index_colliding_dirs[2] & enemy_index_colliding_dirs[1] & enemy_index_colliding_dirs[0])) == cur_enemy_index
      && samus_contact_damage_index) {
    HorizontalShootableShutter_Powerbomb();
  }
  E->rfpm_var_15 = samus_x_pos;
  E->rfpm_var_16 = samus_x_subpos;
}

static Func_V *const off_A2F107[5] = {  // 0xA2F224
  HorizontalShootableShutter_Func_3,
  HorizontalShootableShutter_Func_4,
  HorizontalShootableShutter_Func_5,
  HorizontalShootableShutter_Func_6,
  HorizontalShootableShutter_Func_6,
};
void HorizontalShootableShutter_Func_2(void) {

  int v0 = Get_RisingFallingPlatform(cur_enemy_index)->rfpm_var_07 >> 1;
  off_A2F107[v0]();
}

void HorizontalShootableShutter_Func_3(void) {  // 0xA2F230
  Enemy_RisingFallingPlatform *E = Get_RisingFallingPlatform(cur_enemy_index);
  if (E->rfpm_var_B-- == 1) {
    E->rfpm_var_B = E->rfpm_var_06;
    HorizontalShootableShutter_Func_7(cur_enemy_index);
  }
}

void HorizontalShootableShutter_Func_4(void) {  // 0xA2F243
  Enemy_RisingFallingPlatform *E = Get_RisingFallingPlatform(cur_enemy_index);
  if (IsSamusWithinEnemy_X(cur_enemy_index, E->rfpm_var_06))
    HorizontalShootableShutter_Func_7(cur_enemy_index);
}

void HorizontalShootableShutter_Func_5(void) {  // 0xA2F254
  HorizontalShootableShutter_Func_7(cur_enemy_index);
}

void HorizontalShootableShutter_Func_6(void) {  // 0xA2F25B
  ;
}

void HorizontalShootableShutter_Func_7(uint16 k) {  // 0xA2F25F
  Enemy_RisingFallingPlatform *E = Get_RisingFallingPlatform(k);
  E->rfpm_var_A = FUNC16(HorizontalShootableShutter_Func_8);
  if (E->rfpm_var_01)
    E->rfpm_var_A = FUNC16(HorizontalShootableShutter_Func_9);
}

void HorizontalShootableShutter_Func_8(void) {  // 0xA2F272
  Enemy_RisingFallingPlatform *E = Get_RisingFallingPlatform(cur_enemy_index);
  E->rfpm_var_0D = E->base.x_pos;
  E->rfpm_var_13 = 0;
  if (EnemyFunc_AC67(cur_enemy_index) && (int16)(samus_x_pos - E->base.x_pos) < 0)
    E->rfpm_var_13 = 1;
  AddToHiLo(&E->base.x_pos, &E->base.x_subpos, __PAIR32__(E->rfpm_var_F, E->rfpm_var_E));
  if (E->rfpm_var_13) {
    extra_samus_x_subdisplacement = E->rfpm_var_E;
    extra_samus_x_displacement = E->rfpm_var_F;
    HorizontalShootableShutter_Func_10(cur_enemy_index);
  }
  if ((int16)(E->rfpm_var_11 - E->base.x_pos) >= 0) {
    if (E->rfpm_var_08 == 4080) {
      E->rfpm_var_A = FUNC16(HorizontalShootableShutter_Func_14);
    } else {
      E->rfpm_var_B = E->rfpm_var_08;
      E->rfpm_var_A = FUNC16(HorizontalShootableShutter_Func_12);
    }
  }
}

void HorizontalShootableShutter_Func_9(void) {  // 0xA2F2E4
  Enemy_RisingFallingPlatform *E = Get_RisingFallingPlatform(cur_enemy_index);
  E->rfpm_var_0D = E->base.x_pos;
  E->rfpm_var_13 = 0;
  if (EnemyFunc_AC67(cur_enemy_index) && (int16)(samus_x_pos - E->base.x_pos) >= 0)
    E->rfpm_var_13 = 1;
  AddToHiLo(&E->base.x_pos, &E->base.x_subpos, __PAIR32__(E->rfpm_var_D, E->rfpm_var_C));
  if (E->rfpm_var_13) {
    extra_samus_x_subdisplacement = E->rfpm_var_C;
    extra_samus_x_displacement = E->rfpm_var_D;
    HorizontalShootableShutter_Func_11(cur_enemy_index);
  }
  if ((int16)(E->base.x_pos - E->rfpm_var_12) >= 0) {
    if (E->rfpm_var_09 == 4080) {
      E->rfpm_var_A = FUNC16(HorizontalShootableShutter_Func_14);
    } else {
      E->rfpm_var_B = E->rfpm_var_09;
      E->rfpm_var_A = FUNC16(HorizontalShootableShutter_Func_13);
    }
  }
}

void HorizontalShootableShutter_Func_10(uint16 k) {  // 0xA2F356
  if (Get_RisingFallingPlatform(k)->rfpm_var_13) {
    if ((joypad1_lastkeys & 0x100) != 0) {
      extra_samus_x_displacement -= 4;
      extra_samus_y_subdisplacement = 0;
    }
  }
}

void HorizontalShootableShutter_Func_11(uint16 k) {  // 0xA2F371
  if (Get_RisingFallingPlatform(k)->rfpm_var_13) {
    if ((joypad1_lastkeys & 0x200) != 0) {
      extra_samus_x_displacement += 4;
      extra_samus_y_subdisplacement = 0;
    }
  }
}

void HorizontalShootableShutter_Func_12(void) {  // 0xA2F38C
  Enemy_RisingFallingPlatform *E = Get_RisingFallingPlatform(cur_enemy_index);
  if ((--E->rfpm_var_B & 0x8000) != 0) {
    E->rfpm_var_A = FUNC16(HorizontalShootableShutter_Func_9);
    if (E->rfpm_var_04 == 1) {
      if (E->rfpm_var_01)
        E->rfpm_var_A = FUNC16(HorizontalShootableShutter_Func_4);
    }
  }
}

void HorizontalShootableShutter_Func_13(void) {  // 0xA2F3B0
  Enemy_RisingFallingPlatform *E = Get_RisingFallingPlatform(cur_enemy_index);
  if ((--E->rfpm_var_B & 0x8000) != 0) {
    E->rfpm_var_A = FUNC16(HorizontalShootableShutter_Func_8);
    if (E->rfpm_var_04 == 1 && !E->rfpm_var_01)
      E->rfpm_var_A = FUNC16(HorizontalShootableShutter_Func_4);
  }
}

void HorizontalShootableShutter_Func_14(void) {  // 0xA2F3D4
  ;
}

void HorizontalShootableShutter_Touch(void) {  // 0xA2F3D8
  Enemy_HorizontalShootableShutter *E = Get_HorizontalShootableShutter(cur_enemy_index);
  if (E->hssr_var_A == FUNC16(HorizontalShootableShutter_Func_14)) {
    if ((int16)(samus_x_pos - E->base.x_pos) >= 0) {
      if ((joypad1_lastkeys & 0x200) != 0) {
        extra_samus_x_displacement = 4;
        extra_samus_y_subdisplacement = 0;
      }
    } else if ((joypad1_lastkeys & 0x100) != 0) {
      extra_samus_x_displacement = -4;
      extra_samus_y_subdisplacement = 0;
    }
  }
}

void HorizontalShootableShutter_Shot(void) {  // 0xA2F40E
  NormalEnemyShotAiSkipDeathAnim_CurEnemy();
  HorizontalShootableShutter_Powerbomb();
}

void HorizontalShootableShutter_Powerbomb(void) {  // 0xA2F41A
  Enemy_RisingFallingPlatform *E = Get_RisingFallingPlatform(cur_enemy_index);
  if (!sign16(E->rfpm_var_07 - 6)) {
    if (E->rfpm_var_07 != 8) {
      if (E->rfpm_var_0C)
        return;
      E->rfpm_var_0C = 1;
    }
    if (E->rfpm_var_A != FUNC16(HorizontalShootableShutter_Func_8)
        && E->rfpm_var_A != FUNC16(HorizontalShootableShutter_Func_9)) {
      E->rfpm_var_20 ^= 1;
      E->rfpm_var_A = FUNC16(HorizontalShootableShutter_Func_8);
      if (E->rfpm_var_20)
        E->rfpm_var_A = FUNC16(HorizontalShootableShutter_Func_9);
    }
  }
}
