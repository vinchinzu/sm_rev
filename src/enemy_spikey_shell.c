// MaridiaSpikeyShell extracted from sm_a2.c.
#include "ida_types.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "variables.h"
#include "enemy_ai_canon.h"

#define g_word_A2A3DD ((uint16*)RomFixedPtr(0xa2a3dd))
#define g_word_A2A3ED ((uint16*)RomFixedPtr(0xa2a3ed))


static Func_V *const off_A2A3D3[7] = {  // 0xA2A3F9
  MaridiaSpikeyShell_1, MaridiaSpikeyShell_2,

  MaridiaSpikeyShell_3, MaridiaSpikeyShell_4,
  MaridiaSpikeyShell_5, MaridiaSpikeyShell_6,
  MaridiaSpikeyShell_7,
};

void MaridiaSpikeyShell_Init(void) {

  Enemy_MaridiaSpikeyShell *E = Get_MaridiaSpikeyShell(cur_enemy_index);
  E->base.spritemap_pointer = addr_kSpritemap_Nothing_A2;
  uint16 mssl_parameter_1_low = LOBYTE(E->mssl_parameter_1);
  E->mssl_var_E = mssl_parameter_1_low;
  off_A2A3D3[mssl_parameter_1_low & 1]();
  int v3 = 4 * LOBYTE(E->mssl_parameter_2);
  E->mssl_var_B = kCommonEnemySpeeds_Linear[v3];
  E->mssl_var_A = kCommonEnemySpeeds_Linear[v3 + 1];
  E->mssl_var_D = kCommonEnemySpeeds_Linear[v3 + 2];
  E->mssl_var_C = kCommonEnemySpeeds_Linear[v3 + 3];
  int v4 = HIBYTE(E->mssl_parameter_2);
  E->mssl_var_02 = g_word_A2A3DD[v4] + E->base.x_pos;
  E->mssl_var_01 = E->base.x_pos - g_word_A2A3DD[v4];
  E->mssl_var_00 = g_word_A2A3ED[HIBYTE(E->mssl_parameter_1)];
  E->mssl_var_F = 0;
  if (E->mssl_var_E == 2) {
    E->mssl_var_F = 16;
    E->base.y_pos += 16;
  }
}

void MaridiaSpikeyShell_Main(void) {  // 0xA2A47E
  Enemy_MaridiaSpikeyShell *e = Get_MaridiaSpikeyShell(cur_enemy_index);
  off_A2A3D3[(int16)e->mssl_var_E + 2]();
}

void MaridiaSpikeyShell_1(void) {  // 0xA2A48A
  Enemy_MaridiaSpikeyShell *E = Get_MaridiaSpikeyShell(cur_enemy_index);
  E->base.current_instruction = addr_kMaridiaSpikeyShell_Ilist_A3AB;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
}

void MaridiaSpikeyShell_2(void) {  // 0xA2A49D
  Enemy_MaridiaSpikeyShell *E = Get_MaridiaSpikeyShell(cur_enemy_index);
  E->base.current_instruction = addr_kMaridiaSpikeyShell_Ilist_A3BD;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
}

void MaridiaSpikeyShell_3(void) {  // 0xA2A4B0
  Enemy_MaridiaSpikeyShell *E = Get_MaridiaSpikeyShell(cur_enemy_index);
  AddToHiLo(&E->base.x_pos, &E->base.x_subpos, __PAIR32__(E->mssl_var_D, E->mssl_var_C));
  if ((int16)(E->base.x_pos - E->mssl_var_01) < 0)
    --E->mssl_var_E;
  MaridiaSpikeyShell_8(cur_enemy_index);
}

void MaridiaSpikeyShell_4(void) {  // 0xA2A4D9
  Enemy_MaridiaSpikeyShell *E = Get_MaridiaSpikeyShell(cur_enemy_index);
  AddToHiLo(&E->base.x_pos, &E->base.x_subpos, __PAIR32__(E->mssl_var_B, E->mssl_var_A));
  if ((int16)(E->base.x_pos - E->mssl_var_02) >= 0)
    E->mssl_var_E = 0;
  MaridiaSpikeyShell_8(cur_enemy_index);
}

void MaridiaSpikeyShell_5(void) {  // 0xA2A502
  Enemy_MaridiaSpikeyShell *E = Get_MaridiaSpikeyShell(cur_enemy_index);
  uint16 v1 = E->mssl_var_00 - 1;
  E->mssl_var_00 = v1;
  if (!v1)
    E->mssl_var_E = 4;
}

void MaridiaSpikeyShell_6(void) {  // 0xA2A517
  Enemy_MaridiaSpikeyShell *E = Get_MaridiaSpikeyShell(cur_enemy_index);
  ++E->base.y_pos;
  if (!sign16(++E->mssl_var_F - 16)) {
    E->mssl_var_E = 2;
    E->mssl_var_00 = g_word_A2A3ED[HIBYTE(E->mssl_parameter_1)];
  }
}

void MaridiaSpikeyShell_7(void) {  // 0xA2A53E
  Enemy_MaridiaSpikeyShell *E = Get_MaridiaSpikeyShell(cur_enemy_index);
  --E->base.y_pos;
  if (E->mssl_var_F-- == 1)
    E->mssl_var_E = HIBYTE(random_number) & 1;
}

void MaridiaSpikeyShell_8(uint16 k) {  // 0xA2A553
  NextRandom();
  Enemy_MaridiaSpikeyShell *E = Get_MaridiaSpikeyShell(k);
  if (sign16((uint8)(LOBYTE(E->base.frame_counter) + random_number) - 6))
    E->mssl_var_E = 3;
}

const uint16 *MaridiaSpikeyShell_Instr_A56D(uint16 k, const uint16 *jp) {  // 0xA2A56D
  Get_MaridiaSpikeyShell(k)->mssl_var_E = 0;
  return jp;
}

const uint16 *MaridiaSpikeyShell_Instr_A571(uint16 k, const uint16 *jp) {  // 0xA2A571
  Get_MaridiaSpikeyShell(k)->mssl_var_E = 1;
  return jp;
}

void MaridiaSpikeyShell_Shot(void) {  // 0xA2A579
  Enemy_MaridiaSpikeyShell *E = Get_MaridiaSpikeyShell(cur_enemy_index);
  if (sign16(E->mssl_var_E - 1))
    NormalEnemyShotAi();
}
