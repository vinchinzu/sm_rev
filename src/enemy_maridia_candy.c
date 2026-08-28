// Enemy AI - MaridiaRefillCandy — peeled from Bank $A3
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

enum {
  kMaridiaRefillCandyDetectX = 0x80,
  kMaridiaRefillCandyDir_Left = 1,
  kMaridiaRefillCandyDir_Right = 3,
  kMaridiaRefillCandyRiseSpeed = -0x8000,
};

static const uint16 kMaridiaRefillCandyIlists[4] = {
  addr_kMaridiaRefillCandy_Ilist_B3C1,
  addr_kMaridiaRefillCandy_Ilist_B3D7,
  addr_kMaridiaRefillCandy_Ilist_B3E7,
  addr_kMaridiaRefillCandy_Ilist_B3FD,
};

static const uint16 kMaridiaRefillCandyXSpeed[10] = {
  0x0000, 0x0000,
  0x0000, 0x8000,
  0x0000, 0xa000,
  0x0002, 0x0000,
  0x0000, 0x0000,
};

const uint16 *MaridiaRefillCandy_Instr_1(uint16 k, const uint16 *jp) {  // 0xA3B429
  Get_MaridiaRefillCandy(cur_enemy_index)->mrcy_var_00 = 4;
  return jp;
}

const uint16 *MaridiaRefillCandy_Instr_2(uint16 k, const uint16 *jp) {  // 0xA3B434
  Get_MaridiaRefillCandy(cur_enemy_index)->mrcy_var_00 = 8;
  return jp;
}

const uint16 *MaridiaRefillCandy_Instr_3(uint16 k, const uint16 *jp) {  // 0xA3B43F
  Get_MaridiaRefillCandy(cur_enemy_index)->mrcy_var_00 = 12;
  return jp;
}

void MaridiaRefillCandy_Init(void) {  // 0xA3B44A
  Enemy_MaridiaRefillCandy *E = Get_MaridiaRefillCandy(cur_enemy_index);
  E->mrcy_parameter_1 = FUNC16(MaridiaRefillCandy_Func_1);
  E->mrcy_var_D = 0;
  E->mrcy_var_E = 0;
  E->mrcy_var_00 = 0;
  E->base.current_instruction = addr_kMaridiaRefillCandy_Ilist_B3C1;
  E->base.properties |= kEnemyProps_Invisible;
  E->mrcy_var_B = E->base.x_pos;
  E->mrcy_var_C = E->base.y_pos;
}

void CallMaridiaRefillCandyFunc(uint32 ea, uint16 k) {
  switch (ea) {
  case fnMaridiaRefillCandy_Func_1: MaridiaRefillCandy_Func_1(); return;
  case fnMaridiaRefillCandy_Func_2: MaridiaRefillCandy_Func_2(k); return;
  case fnMaridiaRefillCandy_Func_3: MaridiaRefillCandy_Func_3(k); return;
  default: Unreachable();
  }
}

void MaridiaRefillCandy_Main(void) {  // 0xA3B47C
  Enemy_MaridiaRefillCandy *E = Get_MaridiaRefillCandy(cur_enemy_index);
  CallMaridiaRefillCandyFunc(E->mrcy_parameter_1 | 0xA30000, cur_enemy_index);
}

void MaridiaRefillCandy_Func_1(void) {  // 0xA3B482
  if (IsSamusWithinEnemy_X(cur_enemy_index, kMaridiaRefillCandyDetectX)) {
    uint16 dir = kMaridiaRefillCandyDir_Left;
    if (!sign16(GetSamusEnemyDelta_X(cur_enemy_index)))
      dir = kMaridiaRefillCandyDir_Right;
    Enemy_MaridiaRefillCandy *E = Get_MaridiaRefillCandy(cur_enemy_index);
    E->mrcy_var_D = dir;
    MaridiaRefillCandy_Func_4();
    E->mrcy_parameter_1 = FUNC16(MaridiaRefillCandy_Func_2);
  }
}

void MaridiaRefillCandy_Func_2(uint16 k) {  // 0xA3B4A8
  Enemy_MaridiaRefillCandy *E = Get_MaridiaRefillCandy(k);
  E->base.properties &= ~kEnemyProps_Invisible;
  if (sign16(GetSamusEnemyDelta_Y(k))) {
    AddToHiLo(&E->base.y_pos, &E->base.y_subpos, kMaridiaRefillCandyRiseSpeed);
  } else {
    --E->mrcy_var_D;
    MaridiaRefillCandy_Func_4();
    E->mrcy_var_F = 0;
    E->mrcy_parameter_1 = FUNC16(MaridiaRefillCandy_Func_3);
  }
}

void MaridiaRefillCandy_Func_3(uint16 k) {  // 0xA3B4D6
  Enemy_MaridiaRefillCandy *E = Get_MaridiaRefillCandy(k);
  int v = E->mrcy_var_00 >> 1;
  int32 speed = __PAIR32__(kMaridiaRefillCandyXSpeed[v], kMaridiaRefillCandyXSpeed[v + 1]);
  AddToHiLo(&E->base.x_pos, &E->base.x_subpos, E->mrcy_var_D ? speed : -speed);
  if (!CheckIfEnemyIsOnScreen()) {
    MaridiaRefillCandy_Func_4();
    return;
  }
  E->base.properties |= kEnemyProps_Invisible;
  E->base.x_pos = E->mrcy_var_B;
  E->base.y_pos = E->mrcy_var_C;
  E->mrcy_var_D = 0;
  MaridiaRefillCandy_Func_4();
  E->mrcy_parameter_1 = FUNC16(MaridiaRefillCandy_Func_1);
}

void MaridiaRefillCandy_Func_4(void) {  // 0xA3B537
  Enemy_MaridiaRefillCandy *E = Get_MaridiaRefillCandy(cur_enemy_index);
  uint16 dir = E->mrcy_var_D;
  if (dir != E->mrcy_var_E) {
    E->mrcy_var_E = dir;
    E->base.current_instruction = kMaridiaRefillCandyIlists[dir];
    E->base.instruction_timer = 1;
    E->base.timer = 0;
  }
}

void MaridiaRefillCandy_Func_5(void) {  // 0xA3B557
  ;
}

void MaridiaRefillCandy_Func_6(void) {  // 0xA3B55B
  ;
}
