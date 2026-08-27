// Enemy AI - Ki-Hunter — peeled from Bank $A8

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

enum {
  kKiHunterSwoopXProximity = 0x60,
  kKiHunterFallingAccel = 0xe000,
  kKiHunterWingsArcRadius = 0x30,
  kSfx2_KiHunterSpit = 0x4C,
};

static const uint16 kKiHunterIlists[4] = {
  addr_kKiHunter_Ilist_E9FA, addr_kKiHunter_Ilist_EA4E,
  addr_kKiHunter_Ilist_EA24, addr_kKiHunter_Ilist_EA5E,
};

static void KiHunter_SetFacingIlists(uint16 k, int idx) {
  Enemy_KiHunter *E = Get_KiHunter(k);
  Enemy_KiHunter *wings = Get_KiHunter(k + 64);
  E->base.current_instruction = kKiHunterIlists[idx];
  wings->base.current_instruction = kKiHunterIlists[idx + 1];
  E->base.instruction_timer = 1;
  wings->base.instruction_timer = 1;
}

void KiHunter_Init(void) {  // 0xA8F188
  Enemy_KiHunter *E = Get_KiHunter(cur_enemy_index);
  E->base.properties |= kEnemyProps_ProcessInstructions;
  E->khr_var_14 = 0;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->base.current_instruction = addr_kKiHunter_Ilist_E9FA;
  E->khr_var_F = 0;
  E->khr_var_A = FUNC16(KiHunter_Func_1);
  E->khr_var_08 = 0;
  E->khr_var_09 = 1;
  E->khr_var_06 = 0;
  E->khr_var_07 = -1;
  uint16 idle_top = E->base.y_pos - 16;
  E->khr_var_0A = idle_top;
  E->khr_var_0B = idle_top + 32;
  E->khr_var_0C = E->base.x_pos;
  E->khr_var_0D = E->base.y_pos;
  if (sign16(E->khr_parameter_1)) {
    E->khr_var_14 = 1;
    E->khr_var_A = FUNC16(KiHunter_Func_4);
    E->khr_var_08 = 0;
    E->khr_var_09 = 1;
  }
}

void KiHunterWings_Init(void) {  // 0xA8F214
  Enemy_KiHunter *E = Get_KiHunter(cur_enemy_index);
  E->base.properties |= kEnemyProps_ProcessInstructions;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->base.current_instruction = addr_kKiHunter_Ilist_EA4E;
  int slot = cur_enemy_index >> 1;
  E->base.y_pos = enemy_drawing_queue[slot + 93];
  E->base.x_pos = enemy_drawing_queue[slot + 91];
  E->khr_var_A = FUNC16(KiHunter_Func_9);
  E->base.palette_index = enemy_drawing_queue[slot + 105];
  E->base.vram_tiles_index = enemy_drawing_queue[slot + 106];
  if (sign16(enemy_drawing_queue_sizes[slot + 6]))
    E->base.properties |= kEnemyProps_Deleted;
}

void CallKiHunterFunc(uint32 ea, uint16 k) {
  switch (ea) {
  case fnKiHunter_Func_1: KiHunter_Func_1(k); return;  // 0xa8f268
  case fnKiHunter_Func_2: KiHunter_Func_2(k); return;  // 0xa8f3b8
  case fnKiHunter_Func_3: KiHunter_Func_3(k); return;  // 0xa8f4ed
  case fnKiHunter_Func_4: KiHunter_Func_4(k); return;  // 0xa8f55a
  case fnKiHunter_Func_5: KiHunter_Func_5(k); return;  // 0xa8f58b
  case fnKiHunter_Func_6: KiHunter_Func_6(k); return;  // 0xa8f5f0
  case fnKiHunter_Func_7: KiHunter_Func_7(k); return;  // 0xa8f68b
  case fnKiHunter_Func_8: KiHunter_Func_8(k); return;
  case fnKiHunter_Func_9: KiHunter_Func_9(k); return;  // 0xa8f6f3
  case fnKiHunter_Func_10: KiHunter_Func_10(k); return;  // 0xa8f7cf
  case fnnullsub_346: return;
  default: Unreachable();
  }
}

void KiHunter_Main(void) {  // 0xA8F25C
  Enemy_KiHunter *E = Get_KiHunter(cur_enemy_index);
  CallKiHunterFunc(E->khr_var_A | 0xA80000, cur_enemy_index);
}

void KiHunterWings_Main(void) {  // 0xA8F262
  Enemy_KiHunter *E = Get_KiHunter(cur_enemy_index);
  CallKiHunterFunc(E->khr_var_A | 0xA80000, cur_enemy_index);
}

void KiHunter_Func_1(uint16 k) {  // 0xA8F268
  Enemy_KiHunter *E = Get_KiHunter(k);
  if (Enemy_MoveDown(k, __PAIR32__(E->khr_var_09, E->khr_var_08))) {
    E->khr_var_09 = -E->khr_var_09;
  } else if ((int16)(E->base.y_pos - E->khr_var_0A) < 0) {
    E->khr_var_09 = 1;
  } else if ((int16)(E->base.y_pos - E->khr_var_0B) >= 0) {
    E->khr_var_09 = -1;
  }
  if (Enemy_MoveRight_IgnoreSlopes(k, __PAIR32__(E->khr_var_07, E->khr_var_06))) {
    E->khr_var_07 = -E->khr_var_07;
    KiHunter_SetFacingIlists(k, sign16(E->khr_var_07) ? 0 : 2);
  }
  EnemyFunc_C8AD(k);
  uint16 dx = samus_x_pos - E->base.x_pos;
  uint16 abs_dx = abs16(dx);
  if ((int16)(abs_dx - kKiHunterSwoopXProximity) < 0 && !sign16(samus_y_pos - E->base.y_pos - 32)) {
    uint16 dy = samus_y_pos - E->base.y_pos;
    if (sign16(dx)) {
      E->khr_var_00 = -2;
      E->khr_var_02 = 0;
      E->khr_var_03 = 0;
      E->khr_var_04 = -1;
      E->khr_var_07 = -1;
      E->khr_var_05 = -8192;
      E->khr_var_F = 255;
      E->khr_var_0E = 240;
      KiHunter_SetFacingIlists(k, 0);
    } else {
      E->khr_var_00 = 2;
      E->khr_var_02 = 0;
      E->khr_var_03 = 0;
      E->khr_var_04 = 0;
      E->khr_var_05 = 0x2000;
      E->khr_var_F = 128;
      E->khr_var_07 = 1;
      E->khr_var_0E = 144;
      KiHunter_SetFacingIlists(k, 2);
    }
    E->khr_var_B = dx + E->base.x_pos;
    E->khr_var_C = E->base.y_pos;
    E->khr_var_A = FUNC16(KiHunter_Func_2);
    E->khr_var_12 = dy;
    E->khr_var_11 = abs_dx;
    E->khr_var_10 = 0;
  }
}

void KiHunter_Func_2(uint16 k) {  // 0xA8F3B8
  Enemy_KiHunter *E = Get_KiHunter(k);
  bool past_swipe_angle = sign16(E->khr_var_04)
      ? (int16)(E->khr_var_F - E->khr_var_0E) < 0
      : (int16)(E->khr_var_F - E->khr_var_0E) >= 0;
  if (past_swipe_angle && !E->khr_var_10) {
    E->khr_var_10 = 1;
    E->base.current_instruction = sign16(E->khr_var_04)
        ? addr_kKiHunter_Ilist_EA08
        : addr_kKiHunter_Ilist_EA32;
    E->base.instruction_timer = 1;
  }
  if (!sign16(E->khr_var_04)) {
    AddToHiLo(&E->khr_var_02, &E->khr_var_03, __PAIR32__(E->khr_var_04, E->khr_var_05));
    if ((int16)(E->khr_var_02 - E->khr_var_00) >= 0)
      E->khr_var_02 = E->khr_var_00;
    E->khr_var_F += E->khr_var_02;
    if (!sign16(E->khr_var_F - 256)) {
      E->khr_var_A = FUNC16(KiHunter_Func_1);
      return;
    }
  } else {
    AddToHiLo(&E->khr_var_02, &E->khr_var_03, __PAIR32__(E->khr_var_04, E->khr_var_05));
    if ((int16)(E->khr_var_02 - E->khr_var_00) < 0)
      E->khr_var_02 = E->khr_var_00;
    E->khr_var_F += E->khr_var_02;
    if (sign16(E->khr_var_F - 128)) {
      E->khr_var_A = FUNC16(KiHunter_Func_1);
      return;
    }
  }
  bool collided = false;
  uint16 r20 = E->khr_var_B + CosineMult8bit(E->khr_var_F, E->khr_var_11) - E->base.x_pos;
  if (Enemy_MoveRight_IgnoreSlopes(k, INT16_SHL16(r20))) {
    if (!sign16(E->khr_var_04)) {
      E->khr_var_06 = 0;
      E->khr_var_07 = -1;
    } else {
      E->khr_var_06 = 0;
      E->khr_var_07 = 1;
    }
    collided = true;
  } else {
    EnemyFunc_C8AD(k);
    r20 = E->khr_var_C + SineMult8bit(E->khr_var_F, E->khr_var_12) - E->base.y_pos;
    if (Enemy_MoveDown(k, INT16_SHL16(r20)))
      collided = true;
  }
  if (collided) {
    E->khr_var_A = FUNC16(KiHunter_Func_3);
    E->khr_var_08 = 0;
    E->khr_var_09 = -1;
  }
}

void KiHunter_Func_3(uint16 k) {  // 0xA8F4ED
  Enemy_KiHunter *E = Get_KiHunter(k);
  if (Enemy_MoveRight_IgnoreSlopes(k, __PAIR32__(E->khr_var_07, E->khr_var_06))
      || (EnemyFunc_C8AD(k), Enemy_MoveDown(k, __PAIR32__(E->khr_var_09, E->khr_var_08)))
      || (int16)(E->base.y_pos - E->khr_var_0D) < 0) {
    E->khr_var_A = FUNC16(KiHunter_Func_1);
  }
}

const uint16 *KiHunter_Instr_1(uint16 k, const uint16 *jp) {  // 0xA8F526
  Enemy_KiHunter *E = Get_KiHunter(k);
  int idx = sign16(E->khr_var_07) ? 0 : 2;
  E->base.current_instruction = kKiHunterIlists[idx + 1];
  E->base.instruction_timer = 1;
  Enemy_KiHunter *E1 = Get_KiHunter(k + 64);
  if (E1->khr_var_A == FUNC16(KiHunter_Func_9)) {
    E1->base.current_instruction = kKiHunterIlists[idx + 1];
    E1->base.instruction_timer = 1;
  }
  return INSTR_RETURN_ADDR(kKiHunterIlists[idx]);
}

void KiHunter_Func_4(uint16 k) {  // 0xA8F55A
  Enemy_KiHunter *E = Get_KiHunter(k);
  if (Enemy_MoveDown(k, __PAIR32__(E->khr_var_09, E->khr_var_08))) {
    E->khr_var_A = FUNC16(KiHunter_Func_5);
  } else {
    AddToHiLo(&E->khr_var_09, &E->khr_var_08, kKiHunterFallingAccel);
  }
}

void KiHunter_Func_5(uint16 k) {  // 0xA8F58B
  Enemy_KiHunter *E = Get_KiHunter(k);
  E->khr_var_A = addr_locret_A8F5E3;
  E->khr_var_08 = 0;
  E->khr_var_09 = (random_number & 1) - 8;
  if ((int16)(E->base.x_pos - samus_x_pos) >= 0) {
    E->khr_var_06 = 0;
    E->khr_var_07 = -2;
    E->base.current_instruction = addr_kKiHunter_Ilist_EA8A;
  } else {
    E->khr_var_06 = 0;
    E->khr_var_07 = 2;
    E->base.current_instruction = addr_kKiHunter_Ilist_EAA6;
  }
  E->base.instruction_timer = 1;
}

const uint16 *KiHunter_Instr_2(uint16 k, const uint16 *jp) {  // 0xA8F5E4
  Get_KiHunter(cur_enemy_index)->khr_var_A = FUNC16(KiHunter_Func_6);
  return jp;
}

void KiHunter_Func_6(uint16 k) {  // 0xA8F5F0
  Enemy_KiHunter *E = Get_KiHunter(k);
  if (Enemy_MoveDown(k, __PAIR32__(E->khr_var_09, E->khr_var_08))) {
    if (sign16(E->khr_var_09)) {
      E->khr_var_09 = 1;
    } else {
      E->khr_var_08 = 0;
      E->khr_var_09 = -4;
      E->khr_var_A = addr_locret_A8F5E3;
      E->khr_var_0F = 12;
      uint16 landed_ilist = addr_kKiHunter_Ilist_EAC2;
      if (!sign16(E->base.current_instruction + 0x155A))
        landed_ilist = addr_kKiHunter_Ilist_EADA;
      E->base.current_instruction = landed_ilist;
      E->base.instruction_timer = 1;
    }
  } else {
    if (Enemy_MoveRight_IgnoreSlopes(k, __PAIR32__(E->khr_var_07, E->khr_var_06))) {
      E->khr_var_07 = -E->khr_var_07;
    } else {
      EnemyFunc_C8AD(k);
      AddToHiLo(&E->khr_var_09, &E->khr_var_08, kKiHunterFallingAccel);
    }
  }
}

const uint16 *KiHunter_Instr_3(uint16 k, const uint16 *jp) {  // 0xA8F67F
  Get_KiHunter(cur_enemy_index)->khr_var_A = FUNC16(KiHunter_Func_7);
  return jp;
}

void KiHunter_Func_7(uint16 k) {  // 0xA8F68B
  Enemy_KiHunter *E = Get_KiHunter(k);
  uint16 think_timer = E->khr_var_0F - 1;
  E->khr_var_0F = think_timer;
  if (!think_timer) {
    uint16 next_func = FUNC16(KiHunter_Func_5);
    uint16 dx = abs16(E->base.x_pos - samus_x_pos);
    if (sign16(dx - kKiHunterSwoopXProximity))
      next_func = FUNC16(KiHunter_Func_8);
    E->khr_var_A = next_func;
  }
}

void KiHunter_Func_8(uint16 k) {  // 0xA8F6B3
  Enemy_KiHunter *E = Get_KiHunter(k);
  uint16 spit_ilist = addr_kKiHunter_Ilist_EAF2;
  if ((int16)(E->base.x_pos - samus_x_pos) < 0)
    spit_ilist = addr_kKiHunter_Ilist_EB10;
  E->base.current_instruction = spit_ilist;
  E->base.instruction_timer = 1;
  E->khr_var_A = addr_locret_A8F5E3;
}

const uint16 *KiHunter_Instr_4(uint16 k, const uint16 *jp) {  // 0xA8F6D2
  sub_A8F6DC(k, addr_loc_A8CF18);
  return jp;
}

const uint16 *KiHunter_Instr_5(uint16 k, const uint16 *jp) {  // 0xA8F6D8
  sub_A8F6DC(k, addr_loc_A8CF26);
  return jp;
}

void sub_A8F6DC(uint16 k, uint16 j) {  // 0xA8F6DC
  QueueSfx2_Max6(kSfx2_KiHunterSpit);
  SpawnEprojWithGfx(0, cur_enemy_index, j);
  Get_KiHunter(cur_enemy_index)->khr_var_0F = 24;
}

void KiHunter_Func_9(uint16 k) {  // 0xA8F6F3
  int slot = k >> 1;
  Enemy_KiHunter *E = Get_KiHunter(k);
  E->base.x_pos = enemy_drawing_queue[slot + 91];
  E->base.y_pos = enemy_drawing_queue[slot + 93];
}

void KiHunter_Shot(void) {  // 0xA8F701
  NormalEnemyShotAi();
  uint16 old_cur_enemy_index = cur_enemy_index;
  Enemy_KiHunter *EK = Get_KiHunter(cur_enemy_index);
  Enemy_KiHunter *E1 = Get_KiHunter(cur_enemy_index + 64);
  uint16 health = EK->base.health;
  if (health) {
    if (health == E1->khr_parameter_1 || (int16)(health - E1->khr_parameter_1) < 0) {
      if (!EK->khr_var_14) {
        EK->khr_var_14 = 1;
        EK->khr_var_A = FUNC16(KiHunter_Func_4);
        EK->khr_var_08 = 0;
        EK->khr_var_09 = 1;
        cur_enemy_index = old_cur_enemy_index + 64;
        if (E1->khr_var_A != FUNC16(KiHunter_Func_10)) {
          E1->khr_var_07 = E1->base.y_pos;
          E1->khr_var_08 = E1->base.x_pos;
          KiHunter_Func_17();
          KiHunter_Func_12();
          KiHunter_Func_13();
          E1->khr_var_F = -8192;
          E1->khr_var_A = FUNC16(KiHunter_Func_10);
          E1->khr_var_00 = FUNC16(KiHunter_Func_11);
          E1->khr_var_06 = E1->khr_var_07 - E1->khr_var_0B;
          E1->khr_var_05 = E1->base.x_pos;
          E1->khr_var_B = E1->khr_var_0A;
          E1->base.current_instruction = addr_kKiHunter_Ilist_EA7E;
          E1->base.spritemap_pointer = addr_kSpritemap_Nothing_A8;
          E1->base.instruction_timer = 1;
          E1->base.timer = 0;
          E1->base.properties |= kEnemyProps_ProcessedOffscreen;
        }
        cur_enemy_index = old_cur_enemy_index;
      }
    } else {
      E1->base.ai_handler_bits = EK->base.ai_handler_bits;
      E1->base.frozen_timer = EK->base.frozen_timer;
      E1->base.invincibility_timer = EK->base.invincibility_timer;
      E1->base.flash_timer = EK->base.flash_timer;
    }
  } else {
    E1->base.properties = kEnemyProps_Deleted;
  }
}

void CallKiHunterBFunc(uint32 ea) {
  switch (ea) {
  case fnKiHunter_Func_11: KiHunter_Func_11(); return;
  case fnKiHunter_Func_14: KiHunter_Func_14(); return;
  default: Unreachable();
  }
}
void KiHunter_Func_10(uint16 k) {  // 0xA8F7CF
  uint16 r18 = Get_KiHunter(cur_enemy_index)->khr_var_00;
  CallKiHunterBFunc(r18 | 0xA80000);
}

void KiHunter_Func_11(void) {  // 0xA8F7DB
  int16 next_arc;

  Enemy_KiHunter *E = Get_KiHunter(cur_enemy_index);
  E->khr_var_F += *(uint16 *)((uint8 *)&kCommonEnemySpeeds_Quadratic[2] + (8 * HIBYTE(E->khr_var_B)) + 1);
  E->base.y_pos = E->khr_var_06 + SineMult8bit(HIBYTE(E->khr_var_F), kKiHunterWingsArcRadius) - E->khr_var_04;
  E->base.x_pos = E->khr_var_05 + CosineMult8bit(HIBYTE(E->khr_var_F), kKiHunterWingsArcRadius) - E->khr_var_03;
  if (sign16(E->khr_var_F + 0x4000)) {
    KiHunter_Func_16(cur_enemy_index);
  } else {
    next_arc = E->khr_var_B - 384;
    if (next_arc < 0)
      next_arc = 256;
    E->khr_var_B = next_arc;
  }
}

void KiHunter_Func_12(void) {  // 0xA8F851
  uint16 v1 = CosineMult8bit(0xE0, kKiHunterWingsArcRadius);
  Enemy_KiHunter *E = Get_KiHunter(cur_enemy_index);
  E->khr_var_03 = v1;
  E->khr_var_04 = SineMult8bit(0xE0, kKiHunterWingsArcRadius);
}

void KiHunter_Func_13(void) {  // 0xA8F87F
  uint16 v1 = CosineMult8bit(0xA0, kKiHunterWingsArcRadius);
  Enemy_KiHunter *E = Get_KiHunter(cur_enemy_index);
  E->khr_var_01 = v1;
  E->khr_var_02 = SineMult8bit(0xA0, kKiHunterWingsArcRadius);
}

void KiHunter_Func_14(void) {  // 0xA8F8AD
  int16 next_arc;

  Enemy_KiHunter *E = Get_KiHunter(cur_enemy_index);
  E->khr_var_F += *(uint16 *)((uint8 *)kCommonEnemySpeeds_Quadratic + (8 * HIBYTE(E->khr_var_B)) + 1);
  uint16 r20 = E->khr_var_06 + SineMult8bit(HIBYTE(E->khr_var_F), kKiHunterWingsArcRadius) - E->khr_var_02 - E->base.y_pos;
  if (Enemy_MoveDown(cur_enemy_index, INT16_SHL16(r20))) {
    E->base.properties |= kEnemyProps_Deleted;
    E->base.x_pos = E->khr_var_08;
    next_arc = E->khr_var_07;
    E->base.y_pos = next_arc;
  } else {
    E->base.x_pos = E->khr_var_05 + CosineMult8bit(HIBYTE(E->khr_var_F), kKiHunterWingsArcRadius) - E->khr_var_01;
    if (!sign16(E->khr_var_F + 0x4000)) {
      KiHunter_Func_15(cur_enemy_index);
      return;
    }
    next_arc = E->khr_var_B - 384;
    if (next_arc < 0)
      next_arc = 256;
  }
  Get_KiHunter(cur_enemy_index)->khr_var_B = next_arc;
}

void KiHunter_Func_15(uint16 k) {  // 0xA8F947
  Enemy_KiHunter *E = Get_KiHunter(k);
  E->khr_var_00 = FUNC16(KiHunter_Func_11);
  E->khr_var_B = E->khr_var_0A;
  E->khr_var_F = -8192;
  E->khr_var_05 = E->base.x_pos;
  E->khr_var_06 = E->base.y_pos;
}

void KiHunter_Func_16(uint16 k) {  // 0xA8F96A
  Enemy_KiHunter *E = Get_KiHunter(k);
  E->khr_var_00 = FUNC16(KiHunter_Func_14);
  E->khr_var_B = E->khr_var_0A;
  E->khr_var_F = -24576;
  E->khr_var_05 = E->base.x_pos;
  E->khr_var_06 = E->base.y_pos;
}

void KiHunter_Func_17(void) {  // 0xA8F98D
  uint16 accumulated;

  Enemy_KiHunter *E = Get_KiHunter(cur_enemy_index);
  E->khr_var_0A = 0;
  E->khr_var_B = 0;
  do {
    uint16 step = E->khr_var_0A + 384;
    E->khr_var_0A = step;
    step >>= 8;
    accumulated = *(uint16 *)((uint8 *)kCommonEnemySpeeds_Quadratic + (8 * step) + 1) + E->khr_var_D;
    E->khr_var_D = accumulated;
  } while (sign16(accumulated - 0x2000));
}
