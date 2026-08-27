// LavaSeahorse extracted from sm_a2.c.
#include "ida_types.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "variables.h"
#include "enemy_ai_canon.h"

#define g_off_A2E5EF ((uint16*)RomFixedPtr(0xa2e5ef))


const uint16 *LavaSeahorse_Instr_E5FB(uint16 k, const uint16 *jp) {  // 0xA2E5FB
  Get_LavaSeahorse(cur_enemy_index)->lse_var_02 = 1;
  return jp;
}

void LavaSeahorse_Init(void) {  // 0xA2E606
  Enemy_LavaSeahorse *E = Get_LavaSeahorse(cur_enemy_index);
  E->lse_var_02 = 0;
  if (E->lse_parameter_1) {
    E->lse_var_00 = 2;
    E->lse_var_01 = 2;
    E->base.current_instruction = addr_kLavaSeahorse_Ilist_E5A1;
    E->base.properties |= kEnemyProps_Intangible;
    E->lse_var_F = FUNC16(nullsub_196);
  } else {
    E->lse_var_00 = 0;
    E->lse_var_01 = 0;
    E->base.current_instruction = addr_kLavaSeahorse_Ilist_E59B;
    E->lse_var_F = FUNC16(LavaSeahorse_Func_1);
  }
}

void LavaSeahorse_Main(void) {  // 0xA2E64E
  Enemy_LavaSeahorse *E = Get_LavaSeahorse(cur_enemy_index);
  EnemyRunPreInstr(E->lse_var_F);
}

void LavaSeahorse_Func_1(uint16 k) {  // 0xA2E654
  Enemy_LavaSeahorse *E = Get_LavaSeahorse(k);
  if ((--E->lse_var_D & 0x8000) != 0) {
    E->lse_var_D = 48;
    E->lse_var_F = FUNC16(LavaSeahorse_Func_2);
    uint16 t = GetSamusEnemyDelta_X(k);
    E->lse_var_A = (E->lse_var_A & 0x7fff) | (t & 0x8000);
    if ((E->lse_var_A & 0x8000) != 0) {
      LOBYTE(E->lse_var_00) &= ~1;
      Enemy_LavaSeahorse *E1 = Get_LavaSeahorse(k + 64);
      LOBYTE(E1->lse_var_00) &= ~1;
    } else {
      LOBYTE(E->lse_var_00) |= 1;
      Enemy_LavaSeahorse *E1 = Get_LavaSeahorse(k + 64);
      LOBYTE(E1->lse_var_00) |= 1;
    }
    LavaSeahorse_Func_6();
    LavaSeahorse_Func_7();
  }
}

void LavaSeahorse_Func_2(uint16 k) {  // 0xA2E6AD
  Enemy_LavaSeahorse *E = Get_LavaSeahorse(k);
  if ((--E->lse_var_D & 0x8000) != 0) {
    E->lse_var_00 += 4;
    E->lse_var_D = 3;
    E->lse_var_F = FUNC16(LavaSeahorse_Func_3);
  }
  --E->base.y_pos;
  Enemy_LavaSeahorse *E1 = Get_LavaSeahorse(k + 64);
  --E1->base.y_pos;
}

void LavaSeahorse_Func_3(uint16 k) {  // 0xA2E6F1
  LavaSeahorse_Func_6();
  Enemy_LavaSeahorse *E = Get_LavaSeahorse(k);
  if (E->lse_var_02) {
    E->lse_var_02 = 0;
    E->lse_var_01 = -1;
    SpawnEprojWithGfx(0xFFFF, k, addr_kEproj_LavaSeahorseFireball);
    QueueSfx2_Max6(0x61);
    if (E->lse_var_D-- == 1) {
      E->lse_var_00 -= 4;
      E->lse_var_D = 96;
      E->lse_var_F = FUNC16(LavaSeahorse_Func_4);
    }
  }
}

void LavaSeahorse_Func_4(uint16 k) {  // 0xA2E734
  Enemy_LavaSeahorse *E = Get_LavaSeahorse(k);
  if (E->lse_var_D-- == 1) {
    E->lse_var_D = 48;
    E->lse_var_F = FUNC16(LavaSeahorse_Func_5);
    LavaSeahorse_Func_6();
  }
}

void LavaSeahorse_Func_5(uint16 k) {  // 0xA2E749
  Enemy_LavaSeahorse *E = Get_LavaSeahorse(k);
  if ((--E->lse_var_D & 0x8000) != 0) {
    E->lse_var_D = 128;
    E->lse_var_F = FUNC16(LavaSeahorse_Func_1);
  }
  ++E->base.y_pos;
  Enemy_LavaSeahorse *E1 = Get_LavaSeahorse(k + 64);
  ++E1->base.y_pos;
}

void LavaSeahorse_Func_6(void) {  // 0xA2E782
  Enemy_LavaSeahorse *E = Get_LavaSeahorse(cur_enemy_index);
  uint16 lse_var_00 = E->lse_var_00;
  if (lse_var_00 != E->lse_var_01) {
    E->lse_var_01 = lse_var_00;
    E->base.current_instruction = g_off_A2E5EF[lse_var_00];
    E->base.instruction_timer = 1;
    E->base.timer = 0;
  }
}

void LavaSeahorse_Func_7(void) {  // 0xA2E7A5
  Enemy_LavaSeahorse *E = Get_LavaSeahorse(cur_enemy_index + 64);
  uint16 lse_var_00 = E->lse_var_00;
  if (lse_var_00 != E->lse_var_01) {
    E->lse_var_01 = lse_var_00;
    E->base.current_instruction = g_off_A2E5EF[lse_var_00];
    E->base.instruction_timer = 1;
    E->base.timer = 0;
  }
}

void LavaSeahorse_Touch(void) {  // 0xA2E7C8
  NormalEnemyTouchAi();
  LavaSeahorse_E7DA();
}

void LavaSeahorse_Shot(void) {  // 0xA2E7CE
  NormalEnemyShotAi();
  LavaSeahorse_E7DA();
}

void LavaSeahorse_Powerbomb(void) {  // 0xA2E7D4
  NormalEnemyPowerBombAi();
  LavaSeahorse_E7DA();
}

void LavaSeahorse_E7DA(void) {  // 0xA2E7DA
  Enemy_LavaSeahorse *E = Get_LavaSeahorse(cur_enemy_index);
  if (E->base.health) {
    uint16 shake_timer = E->base.shake_timer;
    Enemy_LavaSeahorse *E1 = Get_LavaSeahorse(cur_enemy_index + 64);
    E1->base.shake_timer = shake_timer;
    E1->base.invincibility_timer = E->base.invincibility_timer;
    E1->base.flash_timer = E->base.flash_timer;
    E1->base.frozen_timer = E->base.frozen_timer;
    E1->base.ai_handler_bits = E->base.ai_handler_bits;
  } else {
    Enemy_LavaSeahorse *E1 = Get_LavaSeahorse(cur_enemy_index + 64);
    E1->base.properties |= 0x200;
  }
}
