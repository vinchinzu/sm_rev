// Enemy AI - Norfair Lava Man — peeled from Bank $A8

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

#define kNorfairLavaMan_Palette ((uint16*)RomFixedPtr(0xa8ac1c))
#define g_word_A8AF79 ((uint16*)RomFixedPtr(0xa8af79))
#define g_word_A8AF55 ((uint16*)RomFixedPtr(0xa8af55))
#define g_off_A8AF67 ((uint16*)RomFixedPtr(0xa8af67))

const uint16 *NorfairLavaMan_Instr_1(uint16 k, const uint16 *jp) {  // 0xA8AE12
  if (!CheckIfEnemyIsOnScreen()) {
    QueueSfx2_Max6(jp[0]);
  }
  return jp + 1;
}

const uint16 *NorfairLavaMan_Instr_8(uint16 k, const uint16 *jp) {  // 0xA8AE26
  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
  ++E->base.y_pos;
  ++E->base.y_pos;
  return jp;
}

const uint16 *NorfairLavaMan_Instr_14(uint16 k, const uint16 *jp) {  // 0xA8AE30
  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
  --E->base.y_pos;
  --E->base.y_pos;
  return jp;
}

const uint16 *NorfairLavaMan_Instr_2(uint16 k, const uint16 *jp) {  // 0xA8AE3A
  Get_NorfairLavaMan(cur_enemy_index)->nlmn_var_01 = 1;
  return jp;
}

const uint16 *NorfairLavaMan_Instr_7(uint16 k, const uint16 *jp) {  // 0xA8AE45
  Get_NorfairLavaMan(cur_enemy_index)->nlmn_var_01 = 0;
  return jp;
}

const uint16 *NorfairLavaMan_Instr_10(uint16 k, const uint16 *jp) {  // 0xA8AE50
  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
  --E->base.y_pos;
  Enemy_NorfairLavaMan *E1 = Get_NorfairLavaMan(cur_enemy_index + 64);
  --E1->base.y_pos;
  return jp;
}

const uint16 *NorfairLavaMan_Instr_12(uint16 k, const uint16 *jp) {  // 0xA8AE5A
  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
  ++E->base.y_pos;
  Enemy_NorfairLavaMan *E1 = Get_NorfairLavaMan(cur_enemy_index + 64);
  ++E1->base.y_pos;
  return jp;
}

const uint16 *NorfairLavaMan_Instr_9(uint16 k, const uint16 *jp) {  // 0xA8AE64
  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
  uint16 v4 = E->nlmn_var_03 + 24;
  E->base.y_pos = v4;
  Enemy_NorfairLavaMan *E1 = Get_NorfairLavaMan(cur_enemy_index + 64);
  E1->base.y_pos = v4;
  E1->base.properties &= ~0x100;
  Enemy_NorfairLavaMan *E2 = Get_NorfairLavaMan(cur_enemy_index + 128);
  E2->base.properties &= ~0x100;
  return jp;
}

const uint16 *NorfairLavaMan_Instr_11(uint16 k, const uint16 *jp) {  // 0xA8AE88
  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
  uint16 nlmn_var_03 = E->nlmn_var_03;
  E->base.y_pos = nlmn_var_03;
  Get_NorfairLavaMan(cur_enemy_index + 64)->base.y_pos = nlmn_var_03;
  return jp;
}

const uint16 *NorfairLavaMan_Instr_13(uint16 k, const uint16 *jp) {  // 0xA8AE96
  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
  uint16 v4 = E->nlmn_var_03 + 4;
  E->base.y_pos = v4;
  Enemy_NorfairLavaMan *E1 = Get_NorfairLavaMan(cur_enemy_index + 64);
  E1->base.y_pos = v4;
  E1->base.properties |= 0x100;
  Enemy_NorfairLavaMan *E2 = Get_NorfairLavaMan(cur_enemy_index + 128);
  E2->base.properties |= 0x100;
  return jp;
}

const uint16 *NorfairLavaMan_Instr_5(uint16 k, const uint16 *jp) {  // 0xA8AEBA
  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
  SpawnEprojWithGfx(E->nlmn_var_B, cur_enemy_index, addr_kEproj_LavaThrownByLavaman);
  return jp;
}

const uint16 *NorfairLavaMan_Instr_15(uint16 k, const uint16 *jp) {  // 0xA8AECA
  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
  E->base.x_pos = E->nlmn_var_12 + 8;
  E->base.y_pos = E->nlmn_var_13 - 4;
  return jp;
}

const uint16 *NorfairLavaMan_Instr_4(uint16 k, const uint16 *jp) {  // 0xA8AEE4
  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
  E->base.x_pos = E->nlmn_var_12 - 8;
  E->base.y_pos = E->nlmn_var_13 - 4;
  return jp;
}

const uint16 *NorfairLavaMan_Instr_16(uint16 k, const uint16 *jp) {  // 0xA8AEFE
  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
  E->base.x_pos = E->nlmn_var_12 + 8;
  E->base.y_pos = E->nlmn_var_13 - 8;
  return jp;
}

const uint16 *NorfairLavaMan_Instr_6(uint16 k, const uint16 *jp) {  // 0xA8AF18
  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
  E->base.x_pos = E->nlmn_var_12 - 8;
  E->base.y_pos = E->nlmn_var_13 - 4;
  return jp;
}

void sub_A8AF32(void) {  // 0xA8AF32
  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
  E->base.x_pos = E->nlmn_var_12;
  E->base.y_pos = E->nlmn_var_13;
}

const uint16 *NorfairLavaMan_Instr_3(uint16 k, const uint16 *jp) {  // 0xA8AF44
  Get_NorfairLavaMan(cur_enemy_index)->nlmn_var_04 = 256;
  return jp;
}
static Func_V *const g_off_A8AF4F[3] = { NorfairLavaMan_Func_1, NorfairLavaMan_Func_2, NorfairLavaMan_Func_3 };
void NorfairLavaMan_Init(void) {  // 0xA8AF8B
  uint16 v0 = 2 * Get_NorfairLavaMan(cur_enemy_index)->nlmn_parameter_1;
  g_off_A8AF4F[v0 >> 1]();
  NorfairLavaMan_Func_4();
  NorfairLavaMan_Func_5(cur_enemy_index);
}

void NorfairLavaMan_Func_1(void) {  // 0xA8AF9D
  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
  E->nlmn_var_C = 0;
  E->nlmn_var_00 = 0;
  E->nlmn_var_02 = 0;
  E->nlmn_var_03 = E->base.y_pos;
  if ((GetSamusEnemyDelta_X(cur_enemy_index) & 0x8000) != 0)
    E->nlmn_var_00 = 1;
  E->nlmn_var_D = addr_kNorfairLavaMan_Ilist_AC9C;
  if (!E->nlmn_var_00)
    E->nlmn_var_D = addr_kNorfairLavaMan_Ilist_AD3C;
  NorfairLavaMan_Func_20();
  E->nlmn_var_F = FUNC16(NorfairLavaMan_Func_7);
}

void NorfairLavaMan_Func_2(void) {  // 0xA8AFE2
  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
  E->nlmn_var_03 = E->base.y_pos;
  E->nlmn_var_C = 0;
  E->nlmn_var_0C = 0;
  E->nlmn_var_0D = 1;
  E->nlmn_var_D = addr_kNorfairLavaMan_Ilist_ADDC;
  NorfairLavaMan_Func_20();
  E->base.y_pos += 32;
  E->nlmn_var_F = FUNC16(NorfairLavaMan_Func_10);
  E->base.properties |= kEnemyProps_Invisible;
}

void NorfairLavaMan_Func_3(void) {  // 0xA8B020
  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
  E->nlmn_var_03 = E->base.y_pos;
  E->nlmn_var_11 = E->base.x_pos;
  E->nlmn_var_C = 0;
  E->nlmn_var_04 = 0;
  E->nlmn_var_D = addr_kNorfairLavaMan_Ilist_AE0C;
  NorfairLavaMan_Func_20();
  E->base.y_pos += 32;
  E->nlmn_var_F = FUNC16(NorfairLavaMan_Func_15);
  E->base.properties |= kEnemyProps_Invisible;
}

void NorfairLavaMan_Func_4(void) {  // 0xA8B05E
  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
  int v1 = (8 * HIBYTE(E->nlmn_parameter_2)) >> 1;
  E->nlmn_var_08 = kCommonEnemySpeeds_Linear[v1];
  E->nlmn_var_07 = kCommonEnemySpeeds_Linear[v1 + 1];
  E->nlmn_var_0A = kCommonEnemySpeeds_Linear[v1 + 2];
  E->nlmn_var_09 = kCommonEnemySpeeds_Linear[v1 + 3];
}

void NorfairLavaMan_Func_5(uint16 k) {  // 0xA8B088
  enemy_gfx_drawn_hook.addr = FUNC16(NorfairLavaMan_Func_6);
  *(uint16 *)&enemy_gfx_drawn_hook.bank = 168;
  variables_for_enemy_graphics_drawn_hook[0] = ((uint16)(Get_NorfairLavaMan(k)->base.palette_index & 0xE00) >> 4)
    + 256;
  variables_for_enemy_graphics_drawn_hook[2] = 8;
  variables_for_enemy_graphics_drawn_hook[1] = 0;
}

void NorfairLavaMan_Func_6(void) {  // 0xA8B0B2
  if (!door_transition_flag_enemies && !--variables_for_enemy_graphics_drawn_hook[2]) {
    variables_for_enemy_graphics_drawn_hook[2] = 8;
    ++variables_for_enemy_graphics_drawn_hook[1];
    int v0 = (uint16)(32 * (variables_for_enemy_graphics_drawn_hook[1] & 3)) >> 1;
    uint16 r18 = kNorfairLavaMan_Palette[v0 + 9];
    uint16 r20 = kNorfairLavaMan_Palette[v0 + 10];
    uint16 r22 = kNorfairLavaMan_Palette[v0 + 11];
    uint16 r24 = kNorfairLavaMan_Palette[v0 + 12];
    int v1 = variables_for_enemy_graphics_drawn_hook[0] >> 1;
    palette_buffer[v1 + 9] = r18;
    palette_buffer[v1 + 10] = r20;
    palette_buffer[v1 + 11] = r22;
    palette_buffer[v1 + 12] = r24;
  }
}

void NorfairLavaMan_Main(void) {  // 0xA8B10A
  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
  --E->nlmn_var_04;
  EnemyRunPreInstr(E->nlmn_var_F);
}

void NorfairLavaMan_Func_7(uint16 k) {  // 0xA8B11A
  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
  E->nlmn_var_00 = 0;
  E->nlmn_var_D = addr_kNorfairLavaMan_Ilist_AC9C;
  if ((GetSamusEnemyDelta_X(cur_enemy_index) & 0x8000) == 0) {
    E->nlmn_var_D = addr_kNorfairLavaMan_Ilist_AD3C;
    E->nlmn_var_00 = 1;
  }
  NorfairLavaMan_Func_20();
  Enemy_NorfairLavaMan *E2 = Get_NorfairLavaMan(cur_enemy_index + 128);
  if ((E2->nlmn_var_04 & 0x8000) != 0) {
    E2->nlmn_var_04 = 0;
    if (IsSamusWithinEnemy_X(cur_enemy_index, LOBYTE(E->nlmn_parameter_2))) {
      E->nlmn_var_D = addr_kNorfairLavaMan_Ilist_ACDE;
      if (E->nlmn_var_00)
        E->nlmn_var_D = addr_kNorfairLavaMan_Ilist_AD7E;
      NorfairLavaMan_Func_20();
      E->nlmn_var_F = FUNC16(NorfairLavaMan_Func_8);
    }
  }
}

void NorfairLavaMan_Func_8(uint16 k) {  // 0xA8B175
  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
   if (!E->nlmn_var_01) {
    E->nlmn_var_02 = 1;
    Get_NorfairLavaMan(cur_enemy_index + 64)->nlmn_var_0D = 0;
    E->nlmn_var_F = FUNC16(sub_A8B193);
  }
}

void sub_A8B193(uint16 k) {  // 0xA8B193
  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
  if (!E->nlmn_var_02) {
    E->nlmn_var_D = addr_kNorfairLavaMan_Ilist_AD0C;
    if ((GetSamusEnemyDelta_X(cur_enemy_index) & 0x8000) == 0)
      E->nlmn_var_D = addr_kNorfairLavaMan_Ilist_ADAC;
    NorfairLavaMan_Func_20();
    E->nlmn_var_F = FUNC16(NorfairLavaMan_Func_9);
  }
}

void NorfairLavaMan_Func_9(uint16 k) {  // 0xA8B1B8
  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
  if (!E->nlmn_var_01) {
    E->nlmn_var_D = addr_kNorfairLavaMan_Ilist_AC9C;
    if ((GetSamusEnemyDelta_X(cur_enemy_index) & 0x8000) == 0)
      E->nlmn_var_D = addr_kNorfairLavaMan_Ilist_AD3C;
    NorfairLavaMan_Func_20();
    E->nlmn_var_F = FUNC16(NorfairLavaMan_Func_7);
  }
}

void NorfairLavaMan_Func_10(uint16 k) {  // 0xA8B1DD
  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
  if (!E->nlmn_var_0D) {
    E->nlmn_var_F = FUNC16(NorfairLavaMan_Func_11);
    E->nlmn_var_E = 0;
    E->nlmn_var_0D = 0;
    E->nlmn_var_B = 2;
    E->base.y_pos = E->nlmn_var_03;
  }
}

void NorfairLavaMan_Func_11(uint16 k) {  // 0xA8B204
  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
  EnemySpawnData *v3 = gEnemySpawnData(cur_enemy_index);

  uint32 v = v3[31].ypos2 + E->base.y_subpos;
  E->base.y_subpos = v;
  E->base.y_pos += v3[31].field_14 + (v >> 16);
  E->nlmn_var_E += v3[31].field_14 + (v >> 16);
  NorfairLavaMan_Func_12(cur_enemy_index);
}

void NorfairLavaMan_Func_12(uint16 k) {  // 0xA8B230
  int v3;

  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(k);
  if (!sign16(-E->nlmn_var_E - 108))
    printf("Y undefined %d!\n", -(int16)E->nlmn_var_E);
  uint16 nlmn_var_B = E->nlmn_var_B;

  if (!sign16(-E->nlmn_var_E - 108)
      || (v3 = nlmn_var_B >> 1,
          (int16)(E->base.y_pos - g_word_A8AF79[v3] - samus_y_pos) < 0)) {
    E->nlmn_var_F = FUNC16(sub_A8B291);
    E->nlmn_var_0C = 1;
    if ((int16)(-E->nlmn_var_E - g_word_A8AF55[nlmn_var_B >> 1]) < 0)
      return;
    goto LABEL_6;
  }
  if ((int16)(-E->nlmn_var_E - g_word_A8AF55[v3]) >= 0) {
LABEL_6:
    ++E->nlmn_var_B;
    ++E->nlmn_var_B;
    E->base.y_pos += 8;
    E->nlmn_var_D = g_off_A8AF67[E->nlmn_var_B >> 1];
    NorfairLavaMan_Func_20();
  }
}

void sub_A8B291(uint16 k) {  // 0xA8B291
  ;
}

void NorfairLavaMan_Func_13(void) {  // 0xA8B295
  EnemySpawnData *v2;

  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
  v2 = gEnemySpawnData(cur_enemy_index);
  bool v4 = __CFADD__uint16(v2[31].field_E, E->base.y_subpos);
  E->base.y_subpos = v2[31].field_E + E->base.y_subpos;
  E->base.y_pos += v2[31].xpos2 + v4;
  E->nlmn_var_E += v2[31].xpos2 + v4;
  NorfairLavaMan_Func_14(cur_enemy_index);
}

void NorfairLavaMan_Func_14(uint16 k) {  // 0xA8B2C5
  int16 nlmn_var_E;

  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(k);
  nlmn_var_E = E->nlmn_var_E;
  if (nlmn_var_E >= 0) {
    E->nlmn_var_0D = 1;
    gEnemySpawnData(k)[31].field_4 = 0;
    E->nlmn_var_F = FUNC16(NorfairLavaMan_Func_10);
  } else if ((int16)(-nlmn_var_E - g_word_A8AF55[(uint16)(E->nlmn_var_B - 2) >> 1]) < 0) {
    E->nlmn_var_B -= 2;
    E->base.y_pos -= 8;
    E->nlmn_var_D = g_off_A8AF67[E->nlmn_var_B >> 1];
    NorfairLavaMan_Func_20();
  }
}

void NorfairLavaMan_Func_15(uint16 k) {  // 0xA8B30D
  if (enemy_drawing_queue[(cur_enemy_index >> 1) + 87] == FUNC16(NorfairLavaMan_Func_8))
    Get_NorfairLavaMan(cur_enemy_index)->nlmn_var_F = FUNC16(NorfairLavaMan_Func_17);
}

void NorfairLavaMan_Func_16(uint16 k) {  // 0xA8B31F
  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
  if (!E->nlmn_var_01) {
    E->nlmn_var_D = addr_kNorfairLavaMan_Ilist_AE0C;
    NorfairLavaMan_Func_20();
    enemy_drawing_queue_sizes[(cur_enemy_index >> 1) + 5] = FUNC16(NorfairLavaMan_Func_13);
    E->nlmn_var_F = FUNC16(NorfairLavaMan_Func_18);
    gEnemySpawnData(cur_enemy_index)[31].field_18 = 0;
    E->base.x_pos = E->nlmn_var_12;
    E->base.y_pos = E->nlmn_var_13;
  }
  NorfairLavaMan_Func_19(cur_enemy_index);
}

void NorfairLavaMan_Func_17(uint16 k) {  // 0xA8B356
  if (gEnemySpawnData(cur_enemy_index)[31].field_18) {
    Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
    E->nlmn_var_D = addr_kNorfairLavaMan_Ilist_ACB0;
    E->nlmn_var_B = 0;
    if ((GetSamusEnemyDelta_X(cur_enemy_index) & 0x8000) == 0) {
      E->nlmn_var_D = addr_kNorfairLavaMan_Ilist_AD50;
      E->nlmn_var_B = 1;
    }
    NorfairLavaMan_Func_20();
    E->nlmn_var_F = FUNC16(NorfairLavaMan_Func_16);
    E->nlmn_var_12 = E->base.x_pos;
    E->nlmn_var_13 = E->base.y_pos;
  } else {
    Get_NorfairLavaMan(cur_enemy_index)->base.y_pos = enemy_drawing_queue[(cur_enemy_index >> 1) + 93]
      - g_word_A8AF79[enemy_drawing_queue_sizes[(cur_enemy_index >> 1) + 1] >> 1];
  }
  NorfairLavaMan_Func_19(cur_enemy_index);
}

void NorfairLavaMan_Func_18(uint16 k) {  // 0xA8B3A7
  int v1 = cur_enemy_index >> 1;
  if (enemy_drawing_queue[v1 + 87] == FUNC16(NorfairLavaMan_Func_7))
    Get_NorfairLavaMan(cur_enemy_index)->nlmn_var_F = FUNC16(NorfairLavaMan_Func_15);
  else
    Get_NorfairLavaMan(cur_enemy_index)->base.y_pos = enemy_drawing_queue[v1 + 93]
    - g_word_A8AF79[enemy_drawing_queue_sizes[v1 + 1] >> 1];
  NorfairLavaMan_Func_19(cur_enemy_index);
}

void NorfairLavaMan_Func_19(uint16 k) {  // 0xA8B3CB
  int v1 = k >> 1;
  uint16 v2 = enemy_drawing_queue[v1 + 61] - Get_NorfairLavaMan(k)->base.y_pos + 2;
  enemy_drawing_queue[v1 + 64] = v2;
  if (sign16(v2 - 8))
    enemy_drawing_queue[v1 + 64] = 8;
}

void NorfairLavaMan_Func_20(void) {  // 0xA8B3E5
  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
  uint16 nlmn_var_D = E->nlmn_var_D;
  if (nlmn_var_D != E->nlmn_var_C) {
    E->base.current_instruction = nlmn_var_D;
    E->nlmn_var_C = nlmn_var_D;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
  }
}

void NorfairLavaMan_Powerbomb(void) {  // 0xA8B400
  NormalEnemyPowerBombAi();
  NorfairLavaMan_Common();
}

void NorfairLavaMan_Touch(void) {  // 0xA8B406
  NormalEnemyTouchAi();
  NorfairLavaMan_Common();
}

void NorfairLavaMan_Shot(void) {  // 0xA8B40C
  NormalEnemyShotAi();
  NorfairLavaMan_Common();
}

void NorfairLavaMan_Common(void) {  // 0xA8B410
  Enemy_NorfairLavaMan *E = Get_NorfairLavaMan(cur_enemy_index);
  Enemy_NorfairLavaMan *E1 = Get_NorfairLavaMan(cur_enemy_index + 64);
  Enemy_NorfairLavaMan *E2 = Get_NorfairLavaMan(cur_enemy_index + 128);
  if (!E->base.health) {
    E1->base.properties |= 0x200;
    E2->base.properties |= 0x200;
  }
  uint16 frozen_timer = E->base.frozen_timer;
  if (frozen_timer) {
    E1->base.frozen_timer = frozen_timer;
    E1->base.ai_handler_bits |= 4;
    E2->base.frozen_timer = frozen_timer;
    E2->base.ai_handler_bits |= 4;
  }
}

