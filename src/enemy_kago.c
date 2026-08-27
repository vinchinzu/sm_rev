// Enemy AI - Kago — peeled from Bank $A8

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

void Kago_Init(void) {  // 0xA8AB46
  Enemy_Kago *E = Get_Kago(cur_enemy_index);
  E->base.properties |= kEnemyProps_ProcessInstructions;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->kago_var_E = 0;
  E->base.current_instruction = addr_kKago_Ilist_AB1E;
  E->kago_var_A = FUNC16(Kago_Func_1);
  E->kago_var_F = 0;
  E->kago_var_04 = E->kago_parameter_1;
}

void CallKagoFunc(uint32 ea, uint16 k) {
  switch (ea) {
  case fnKago_Func_1: Kago_Func_1(k); return;
  case fnnullsub_306: return;
  default: Unreachable();
  }
}

void Kago_Main(void) {  // 0xA8AB75
  Enemy_Kago *E = Get_Kago(cur_enemy_index);
  CallKagoFunc(E->kago_var_A | 0xA80000, cur_enemy_index);
}

void Kago_Func_1(uint16 k) {  // 0xA8AB7B
  Get_Kago(k)->kago_var_A = FUNC16(nullsub_306);
}

void Kago_Shot(void) {  // 0xA8AB83
  int16 v2;

  NormalEnemyShotAi();
  earthquake_type = 2;
  earthquake_timer = 16;
  Enemy_Kago *E = Get_Kago(cur_enemy_index);
  if (!E->kago_var_B) {
    E->kago_var_B = 1;
    E->base.current_instruction = addr_kKago_Ilist_AB32;
    E->base.instruction_timer = 1;
  }
  v2 = E->kago_var_04 - 1;
  E->kago_var_04 = v2;
  if (v2 < 0) {
    EnemyDeathAnimation(cur_enemy_index, 4);
    E->kago_var_F = 1;
  }
  SpawnEprojWithGfx(E->base.y_pos, cur_enemy_index, addr_loc_A8D02E);
}

