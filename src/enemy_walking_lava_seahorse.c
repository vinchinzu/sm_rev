// Enemy AI - Walking Lava Seahorse — peeled from Bank $A8

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

static const uint16 g_word_A8DCC7 = 0x50;
static const uint16 g_word_A8DCCB = 0x70;

void WalkingLavaSeahorse_Init(void) {  // 0xA8DCCD
  Enemy_WalkingLavaSeahorse *E = Get_WalkingLavaSeahorse(cur_enemy_index);
  E->wlse_var_04 = 0;
  E->wlse_var_F = E->base.y_pos;
  E->wlse_var_02 = E->base.x_pos;
  E->base.properties |= kEnemyProps_ProcessInstructions;
  E->base.current_instruction = addr_kWalkingLavaSeahorse_Ilist_DBE7;
  E->wlse_var_A = FUNC16(WalkingLavaSeahorse_Func_3);
  WalkingLavaSeahorse_Func_1(cur_enemy_index);
  do {
    AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(E->wlse_var_B, E->wlse_var_C));
  } while ((WalkingLavaSeahorse_Func_2(cur_enemy_index) & 0x8000) != 0);
  while (1) {
    if (Enemy_MoveDown(cur_enemy_index, __PAIR32__(E->wlse_var_B, E->wlse_var_C)))
      break;
    WalkingLavaSeahorse_Func_2(cur_enemy_index);
  }
  E->wlse_var_03 = E->base.y_pos;
  E->base.y_pos = E->wlse_var_F;
}

void WalkingLavaSeahorse_Func_1(uint16 k) {  // 0xA8DD37
  Enemy_WalkingLavaSeahorse *E = Get_WalkingLavaSeahorse(k);
  E->wlse_var_B = -12;
  E->wlse_var_C = 0;
  E->wlse_var_D = 0;
  E->wlse_var_E = 0;
  E->wlse_var_00 = 0;
  E->wlse_var_01 = 0x8000;
}

uint16 WalkingLavaSeahorse_Func_2(uint16 k) {  // 0xA8DD55
  Enemy_WalkingLavaSeahorse *E = Get_WalkingLavaSeahorse(k);
  AddToHiLo(&E->wlse_var_B, &E->wlse_var_C, __PAIR32__(E->wlse_var_00, E->wlse_var_01));
  return E->wlse_var_B;
}

void CallWalkingLavaSeahorseFunc(uint32 ea, uint16 k) {
  switch (ea) {
  case fnnullsub_257: return;
  case fnWalkingLavaSeahorse_Func_3: WalkingLavaSeahorse_Func_3(k); return;
  case fnWalkingLavaSeahorse_Func_4: WalkingLavaSeahorse_Func_4(k); return;
  case fnWalkingLavaSeahorse_Func_6: WalkingLavaSeahorse_Func_6(k); return;
  case fnWalkingLavaSeahorse_Func_7: WalkingLavaSeahorse_Func_7(k); return;
  case fnWalkingLavaSeahorse_Func_8: WalkingLavaSeahorse_Func_8(k); return;
  case fnWalkingLavaSeahorse_Func_9: WalkingLavaSeahorse_Func_9(k); return;
  default: Unreachable();
  }
}

void WalkingLavaSeahorse_Main(void) {  // 0xA8DD6B
  Enemy_WalkingLavaSeahorse *E = Get_WalkingLavaSeahorse(cur_enemy_index);
  CallWalkingLavaSeahorseFunc(E->wlse_var_A | 0xA80000, cur_enemy_index);
}

void WalkingLavaSeahorse_Func_3(uint16 k) {  // 0xA8DD71
  uint16 v4;
  int16 v5;

  Enemy_WalkingLavaSeahorse *E = Get_WalkingLavaSeahorse(k);
  uint16 xxx;
  uint16 v3 = abs16(E->wlse_var_03 - samus_y_pos);
  if (sign16(v3 - 32) && (int16)(abs16((xxx = samus_x_pos - E->base.x_pos)) - g_word_A8DCC7) < 0) {
    WalkingLavaSeahorse_Func_1(k);
    v4 = addr_stru_A8DC4B;
    v5 = -2;
    if (!sign16(xxx)) {
      v4 = addr_stru_A8DCBB;
      v5 = 2;
    }
    E->wlse_var_D = v5;
    E->base.current_instruction = v4;
    E->base.instruction_timer = 1;
    E->wlse_var_A = FUNC16(WalkingLavaSeahorse_Func_4);
    QueueSfx2_Max6(0x5E);
  }
}

void WalkingLavaSeahorse_Func_4(uint16 k) {  // 0xA8DDC6
  Enemy_WalkingLavaSeahorse *E = Get_WalkingLavaSeahorse(k);
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(E->wlse_var_B, E->wlse_var_C));
  if ((WalkingLavaSeahorse_Func_2(k) & 0x8000) == 0)
    sub_A8DDDE(k);
}

void sub_A8DDDE(uint16 k) {  // 0xA8DDDE
  EnemyData *v1 = gEnemyData(k);
  v1->ai_var_A = FUNC16(WalkingLavaSeahorse_Func_6);
  uint16 v2 = addr_kWalkingLavaSeahorse_Ilist_DC51;
  if ((v1->ai_var_D & 0x8000) == 0)
    v2 = addr_kWalkingLavaSeahorse_Ilist_DCC1;
  v1->current_instruction = v2;
  v1->instruction_timer = 1;
}

void WalkingLavaSeahorse_Func_5(uint16 k) {  // 0xA8DDFA
  Enemy_WalkingLavaSeahorse *E = Get_WalkingLavaSeahorse(k);
  E->wlse_var_B = 0;
  E->wlse_var_C = 0;
  sub_A8DDDE(k);
}

void WalkingLavaSeahorse_Func_6(uint16 k) {  // 0xA8DE05
  int16 v3;

  Enemy_WalkingLavaSeahorse *E = Get_WalkingLavaSeahorse(k);
  if (Enemy_MoveDown(k, __PAIR32__(E->wlse_var_B, E->wlse_var_C))) {
    int t = samus_x_pos - E->base.x_pos;
    uint16 v2 = addr_kWalkingLavaSeahorse_Ilist_DBE7;
    v3 = -2;
    if (!sign16(t)) {
      v3 = 2;
      v2 = addr_kWalkingLavaSeahorse_Ilist_DC57;
    }
    E->wlse_var_D = v3;
    E->base.current_instruction = v2;
    E->base.instruction_timer = 1;
    E->wlse_var_A = FUNC16(WalkingLavaSeahorse_Func_7);
    E->wlse_var_04 = 1;
  } else {
    WalkingLavaSeahorse_Func_2(k);
  }
}

void WalkingLavaSeahorse_Func_7(uint16 k) {  // 0xA8DE4B
  Enemy_MoveDown(k, INT16_SHL16(2));
  Enemy_WalkingLavaSeahorse *E = Get_WalkingLavaSeahorse(k);
  if ((int16)(abs16(E->wlse_var_02 - E->base.x_pos) - g_word_A8DCCB) >= 0) {
    E->wlse_var_A = FUNC16(WalkingLavaSeahorse_Func_8);
    E->wlse_var_B = -4;
    E->wlse_var_C = 0;
    uint16 v4 = addr_kWalkingLavaSeahorse_Ilist_DC51;
    if ((E->wlse_var_D & 0x8000) == 0)
      v4 = addr_kWalkingLavaSeahorse_Ilist_DCC1;
    E->base.current_instruction = v4;
    E->base.instruction_timer = 1;
  } else {
    if (E->wlse_var_04)
      return;
    int t = samus_x_pos - E->base.x_pos;
    uint16 v3;
    if (!sign16(t)) {
      v3 = addr_kWalkingLavaSeahorse_Ilist_DC73;
      if ((E->wlse_var_D & 0x8000) != 0)
        return;
    } else {
      v3 = addr_kWalkingLavaSeahorse_Ilist_DC03;
      if ((E->wlse_var_D & 0x8000) == 0)
        return;
    }
    E->base.current_instruction = v3;
    E->base.instruction_timer = 1;
    E->wlse_var_A = FUNC16(nullsub_257);
  }
}

void WalkingLavaSeahorse_Func_8(uint16 k) {  // 0xA8DECD
  Enemy_WalkingLavaSeahorse *E = Get_WalkingLavaSeahorse(k);
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(E->wlse_var_B, E->wlse_var_C));
  if ((WalkingLavaSeahorse_Func_2(k) & 0x8000) == 0)
    E->wlse_var_A = FUNC16(WalkingLavaSeahorse_Func_9);
}

void WalkingLavaSeahorse_Func_9(uint16 k) {  // 0xA8DEEC
  Enemy_WalkingLavaSeahorse *E = Get_WalkingLavaSeahorse(k);
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(E->wlse_var_B, E->wlse_var_C));
  if ((int16)(E->base.y_pos - E->wlse_var_F) >= 0) {
    E->base.y_pos = E->wlse_var_F;
    E->base.x_pos = E->wlse_var_02;
    E->wlse_var_A = FUNC16(WalkingLavaSeahorse_Func_3);
  } else {
    WalkingLavaSeahorse_Func_2(k);
  }
}

const uint16 *WalkingLavaSeahorse_Instr_4(uint16 k, const uint16 *jp) {  // 0xA8DF1C
  WalkingLavaSeahorse_DF20(0);
  return jp;
}

void WalkingLavaSeahorse_DF20(uint16 a) {  // 0xA8DF20
  SpawnEprojWithGfx(a, cur_enemy_index, addr_loc_A89E90);
  QueueSfx2_Max6(0x3F);
}

const uint16 *WalkingLavaSeahorse_Instr_3(uint16 k, const uint16 *jp) {  // 0xA8DF33
  WalkingLavaSeahorse_DF20(2);
  return jp;
}

const uint16 *WalkingLavaSeahorse_Instr_5(uint16 k, const uint16 *jp) {  // 0xA8DF39
  WalkingLavaSeahorse_DF20(4);
  return jp;
}

const uint16 *WalkingLavaSeahorse_Instr_6(uint16 k, const uint16 *jp) {  // 0xA8DF3F
  int16 v3;

  Enemy_WalkingLavaSeahorse *E = Get_WalkingLavaSeahorse(cur_enemy_index);
  E->wlse_var_A = FUNC16(WalkingLavaSeahorse_Func_7);
  v3 = random_number & 3;
  if ((random_number & 3) == 0)
    v3 = 2;
  E->wlse_var_04 = v3;
  if ((E->wlse_var_D & 0x8000) == 0)
    return INSTR_RETURN_ADDR(addr_kWalkingLavaSeahorse_Ilist_DC57);
  return INSTR_RETURN_ADDR(addr_kWalkingLavaSeahorse_Ilist_DBE7);
}

const uint16 *WalkingLavaSeahorse_Instr_2(uint16 k, const uint16 *jp) {  // 0xA8DF63
  Enemy_WalkingLavaSeahorse *E = Get_WalkingLavaSeahorse(cur_enemy_index);
  uint16 wlse_var_04 = E->wlse_var_04;
  if (wlse_var_04)
    E->wlse_var_04 = wlse_var_04 - 1;
  return WalkingLavaSeahorse_Instr_1(cur_enemy_index, jp);
}

const uint16 *WalkingLavaSeahorse_Instr_1(uint16 k, const uint16 *jp) {  // 0xA8DF71
  int16 wlse_var_D;

  Enemy_WalkingLavaSeahorse *E = Get_WalkingLavaSeahorse(cur_enemy_index);
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, INT16_SHL16(E->wlse_var_D))) {
    jp = INSTR_RETURN_ADDR(addr_stru_A8DBE9);
    wlse_var_D = E->wlse_var_D;
    if (wlse_var_D < 0)
      jp = INSTR_RETURN_ADDR(addr_stru_A8DC59);
    E->wlse_var_D = -wlse_var_D;
  } else {
    EnemyFunc_C8AD(cur_enemy_index);
  }
  return jp;
}

void WalkingLavaSeahorse_Func_10(void) {  // 0xA8DF9D
  NormalEnemyShotAi();
}

