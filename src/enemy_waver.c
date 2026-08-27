// Enemy AI - Waver — peeled from Bank $A3
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

#define g_off_A386DB ((uint16*)RomFixedPtr(0xa386db))

const uint16 *Waver_Instr_1(uint16 k, const uint16 *jp) {  // 0xA386E3
  Get_Waver(cur_enemy_index)->waver_var_E = 1;
  return jp;
}

void Waver_Init(void) {  // 0xA386ED
  Enemy_Waver *E = Get_Waver(cur_enemy_index);
  E->waver_var_B = 1;
  E->waver_var_A = 0x8000;
  if ((E->waver_parameter_1 & 1) == 0) {
    E->waver_var_B = SignExtend8(0xFE);
    E->waver_var_A = SignExtend8(0x8000);
  }
  E->waver_var_F = 0;
  E->waver_var_C = 0;
  E->waver_var_E = 0;
  E->base.current_instruction = addr_kWaver_Ilist_86A7;
  E->waver_var_F = E->waver_parameter_1 & 1;
  Waver_Func_1();
}

void Waver_Main(void) {  // 0xA3874C
  Enemy_Waver *E = Get_Waver(cur_enemy_index);
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, __PAIR32__(E->waver_var_B, E->waver_var_A))) {
    uint16 r18 = *(uint16 *)((uint8 *)&E->waver_var_A + 1);
    E->waver_var_B = SignExtend8((-r18 & 0xFF00) >> 8);
    E->waver_var_A = -(int8)r18 << 8;
    E->waver_var_F = (E->waver_var_F ^ 1) & 1;
    Waver_Func_1();
  } else {
    if (Enemy_MoveDown(cur_enemy_index, INT16_SHL16(SineMult8bit(LOBYTE(E->waver_var_D), 4)))) {
      E->waver_var_D = (uint8)(E->waver_var_D + 0x80);
    } else {
      E->waver_var_D += 2;
    }
  }
  if ((E->waver_var_D & 0x7F) == 56) {
    E->waver_var_F |= 2;
    Waver_Func_1();
  }
  if (E->waver_var_E) {
    E->waver_var_E = 0;
    E->waver_var_F = E->waver_var_F & 1;
    Waver_Func_1();
  }
}

void Waver_Func_1(void) {  // 0xA387FE
  Enemy_Waver *E = Get_Waver(cur_enemy_index);
  uint16 waver_var_F = E->waver_var_F;
  if (waver_var_F != E->waver_var_C) {
    E->waver_var_C = waver_var_F;
    E->base.current_instruction = g_off_A386DB[waver_var_F];
    E->base.instruction_timer = 1;
    E->base.timer = 0;
  }
}
