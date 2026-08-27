// Enemy AI - Yapping Maw — peeled from Bank $A8

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

#define g_word_A8A0A7 (*(uint16*)RomFixedPtr(0xa8a0a7))
#define g_word_A8A0A9 (*(uint16*)RomFixedPtr(0xa8a0a9))
#define g_word_A8A0AB (*(uint16*)RomFixedPtr(0xa8a0ab))
#define g_word_A8A0AD (*(uint16*)RomFixedPtr(0xa8a0ad))
#define g_word_A8A0B3 (*(uint16*)RomFixedPtr(0xa8a0b3))
#define g_word_A8A0B5 (*(uint16*)RomFixedPtr(0xa8a0b5))
#define g_word_A8A0B7 (*(uint16*)RomFixedPtr(0xa8a0b7))
#define g_word_A8A0B9 (*(uint16*)RomFixedPtr(0xa8a0b9))
#define g_word_A8A0BB (*(uint16*)RomFixedPtr(0xa8a0bb))
#define g_word_A8A0BD (*(uint16*)RomFixedPtr(0xa8a0bd))
#define g_word_A8A0C3 (*(uint16*)RomFixedPtr(0xa8a0c3))
#define g_word_A8A0C5 (*(uint16*)RomFixedPtr(0xa8a0c5))
#define g_off_A8A097 ((uint16*)RomFixedPtr(0xa8a097))

const uint16 *YappingMaw_Instr_2(uint16 k, const uint16 *jp) {  // 0xA8A0C7
  Enemy_YappingMaw *E = Get_YappingMaw(cur_enemy_index);
  E->ymw_var_32 = g_word_A8A0AB;
  E->ymw_var_33 = g_word_A8A0AD;
  return jp;
}

const uint16 *YappingMaw_Instr_4(uint16 k, const uint16 *jp) {  // 0xA8A0D9
  Enemy_YappingMaw *E = Get_YappingMaw(cur_enemy_index);
  E->ymw_var_32 = g_word_A8A0C3;
  E->ymw_var_33 = g_word_A8A0C5;
  return jp;
}

const uint16 *YappingMaw_Instr_5(uint16 k, const uint16 *jp) {  // 0xA8A0EB
  Enemy_YappingMaw *E = Get_YappingMaw(cur_enemy_index);
  E->ymw_var_32 = g_word_A8A0B3;
  E->ymw_var_33 = g_word_A8A0B5;
  return jp;
}

const uint16 *YappingMaw_Instr_7(uint16 k, const uint16 *jp) {  // 0xA8A0FD
  Enemy_YappingMaw *E = Get_YappingMaw(cur_enemy_index);
  E->ymw_var_32 = g_word_A8A0BB;
  E->ymw_var_33 = g_word_A8A0BD;
  return jp;
}

const uint16 *YappingMaw_Instr_3(uint16 k, const uint16 *jp) {  // 0xA8A10F
  Enemy_YappingMaw *E = Get_YappingMaw(cur_enemy_index);
  E->ymw_var_32 = g_word_A8A0A7;
  E->ymw_var_33 = g_word_A8A0A9;
  return jp;
}

const uint16 *YappingMaw_Instr_6(uint16 k, const uint16 *jp) {  // 0xA8A121
  Enemy_YappingMaw *E = Get_YappingMaw(cur_enemy_index);
  E->ymw_var_32 = g_word_A8A0B7;
  E->ymw_var_33 = g_word_A8A0B9;
  return jp;
}

const uint16 *YappingMaw_Instr_1(uint16 k, const uint16 *jp) {  // 0xA8A133
  if (!Get_YappingMaw(cur_enemy_index)->ymw_var_36)
    QueueSfx2_Max6(0x2F);
  return jp;
}

void YappingMaw_Init(void) {  // 0xA8A148
  int16 v1;

  Enemy_YappingMaw *E = Get_YappingMaw(cur_enemy_index);
  E->ymw_var_0C = E->base.x_pos;
  E->ymw_var_0D = E->base.y_pos;
  E->ymw_var_00 = 0;
  E->ymw_var_01 = 0;
  E->ymw_var_02 = 0;
  E->ymw_var_03 = 0;
  E->ymw_var_04 = 0;
  E->ymw_var_05 = 0;
  E->ymw_var_06 = 0;
  E->ymw_var_07 = 0;
  E->ymw_var_30 = 0;
  E->ymw_var_F = E->ymw_parameter_1;
  E->ymw_var_E = 64;
  E->base.current_instruction = addr_kYappingMaw_Ilist_9F6F;
  uint16 R36 = 57;
  uint16 R34 = 8;
  if (!E->ymw_parameter_2) {
    E->base.current_instruction = addr_kYappingMaw_Ilist_9FC7;
    R36 = 56;
    R34 = -8;
  }
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->ymw_var_A = FUNC16(YappingMaw_Func_1);
  E->ymw_var_45 = E->base.palette_index & 0xE00;
  v1 = 3;
  E->ymw_var_44 = 3;
  do {
    SpawnEprojWithGfx(v1, cur_enemy_index, addr_kEproj_YappingMawsBody);
    v1 = E->ymw_var_44 - 1;
    E->ymw_var_44 = v1;
  } while (v1 >= 0);
  uint16 v3 = E->base.vram_tiles_index | E->base.palette_index;
  E->ymw_var_47 = v3;
  E->ymw_var_46 = CreateSpriteAtPos(E->base.x_pos, R34 + E->base.y_pos, R36, v3);
}

void CallYappingMawFunc(uint32 ea) {
  switch (ea) {
  case fnYappingMaw_Func_1: YappingMaw_Func_1(); return;  // 0xa8a235
  case fnYappingMaw_Func_2: YappingMaw_Func_2(); return;  // 0xa8a28c
  case fnYappingMaw_Func_8: YappingMaw_Func_8(); return;  // 0xa8a445
  case fnYappingMaw_Func_11: YappingMaw_Func_11(); return;  // 0xa8a68a
  default: Unreachable();
  }
}

void YappingMaw_Main(void) {  // 0xA8A211
  Enemy_YappingMaw *E = Get_YappingMaw(cur_enemy_index);
  --E->ymw_var_35;
  E->ymw_var_36 = CheckIfEnemyIsOnScreen();
  CallYappingMawFunc(E->ymw_var_A | 0xA80000);
  YappingMaw_Func_15();
  YappingMaw_Func_14();
  YappingMaw_Func_13();
  YappingMaw_Func_12();
}

void YappingMaw_Func_1(void) {  // 0xA8A235
  Enemy_YappingMaw *E = Get_YappingMaw(cur_enemy_index);
  PairU16 pair = EnemyFunc_ACA8((Point16U) { E->base.x_pos, E->base.y_pos}, (Point16U) { samus_x_pos, samus_y_pos });
  uint16 v2 = Abs16(pair.k);
  E->ymw_var_08 = v2;
  if (sign16(v2 - 32)) {
    E->ymw_var_35 = 48;
  } else if ((int16)(v2 - E->ymw_var_F) < 0) {
    if (!sign16(E->ymw_var_08 - 64))
      E->ymw_var_08 = 64;
    E->ymw_var_0A = pair.j;
    E->ymw_var_A = FUNC16(YappingMaw_Func_2);
  }
}

void YappingMaw_Func_2(void) {  // 0xA8A28C
  Enemy_YappingMaw *E = Get_YappingMaw(cur_enemy_index);
  E->ymw_var_B = 0;
  E->ymw_var_C = 0;
  E->ymw_var_D = 0;
  E->ymw_var_09 = E->ymw_var_08 >> 1;
  uint16 v1 = (uint8)(64 - E->ymw_var_0A);
  E->ymw_var_0B = v1;
  if (sign16(v1 - 128))
    E->ymw_var_2F = 0;
  else
    E->ymw_var_2F = 1;
  YappingMaw_Func_3();
  uint16 v2 = 2 * ((uint8)(E->ymw_var_0A + 16) >> 5);
  E->ymw_var_34 = v2;
  E->base.current_instruction = g_off_A8A097[v2 >> 1];
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->ymw_var_32 = *(uint16 *)((uint8 *)&g_word_A8A0A7 + (uint16)(2 * v2));
  E->ymw_var_33 = *(uint16 *)((uint8 *)&g_word_A8A0A9 + (uint16)(2 * v2));
  E->ymw_var_A = FUNC16(YappingMaw_Func_8);
}

void YappingMaw_Func_3(void) {  // 0xA8A310
  Enemy_YappingMaw *E = Get_YappingMaw(cur_enemy_index);
  E->ymw_var_0E = YappingMaw_Func_16(0x80, E->ymw_var_09);
  E->ymw_var_0F = YappingMaw_Func_17(0x80, E->ymw_var_09 >> 1);
}

void YappingMaw_Func_4(void) {  // 0xA8A339
  Enemy_YappingMaw *E = Get_YappingMaw(cur_enemy_index);
  uint16 varE32 = E->ymw_var_00;
  E->ymw_var_10 = YappingMaw_Func_16(0, varE32);
  E->ymw_var_11 = YappingMaw_Func_17(0, varE32);
  E->ymw_var_27 = YappingMaw_Func_16(E->ymw_var_0B, varE32) - E->ymw_var_10;
  E->ymw_var_28 = YappingMaw_Func_17(E->ymw_var_0B, varE32) - E->ymw_var_11;
}

void YappingMaw_Func_5(void) {  // 0xA8A37C
  Enemy_YappingMaw *E = Get_YappingMaw(cur_enemy_index);
  uint16 varE32 = E->ymw_var_02;
  E->ymw_var_10 = YappingMaw_Func_16(0, varE32);
  E->ymw_var_11 = YappingMaw_Func_17(0, varE32);
  E->ymw_var_29 = YappingMaw_Func_16(E->ymw_var_0B, varE32) - E->ymw_var_10;
  E->ymw_var_2A = YappingMaw_Func_17(E->ymw_var_0B, varE32) - E->ymw_var_11;
}

void YappingMaw_Func_6(void) {  // 0xA8A3BF
  Enemy_YappingMaw *E = Get_YappingMaw(cur_enemy_index);
  uint16 varE32 = E->ymw_var_04;
  E->ymw_var_10 = YappingMaw_Func_16(0, varE32);
  E->ymw_var_11 = YappingMaw_Func_17(0, varE32);
  E->ymw_var_2B = YappingMaw_Func_16(E->ymw_var_0B, varE32) - E->ymw_var_10;
  E->ymw_var_2C = YappingMaw_Func_17(E->ymw_var_0B, varE32) - E->ymw_var_11;
}

void YappingMaw_Func_7(void) {  // 0xA8A402
  Enemy_YappingMaw *E = Get_YappingMaw(cur_enemy_index);
  uint16 varE32 = E->ymw_var_06;
  E->ymw_var_10 = YappingMaw_Func_16(0, varE32);
  E->ymw_var_11 = YappingMaw_Func_17(0, varE32);
  E->ymw_var_2D = YappingMaw_Func_16(E->ymw_var_0B, varE32) - E->ymw_var_10;
  E->ymw_var_2E = YappingMaw_Func_17(E->ymw_var_0B, varE32) - E->ymw_var_11;
}

void YappingMaw_Func_8(void) {  // 0xA8A445
  Enemy_YappingMaw *E = Get_YappingMaw(cur_enemy_index);
  uint16 r20 = E->ymw_var_B >> 2;
  if (E->ymw_var_2F) {
    E->ymw_var_21 = 128 + r20;
    E->ymw_var_22 = 128 + r20 * 2;
    E->ymw_var_23 = 128 + r20 * 3;
    E->ymw_var_24 = 128 + r20 * 4;
  } else {
    E->ymw_var_21 = 128 - r20;
    E->ymw_var_22 = 128 - r20 * 2;
    E->ymw_var_23 = 128 - r20 * 3;
    E->ymw_var_24 = 128 - r20 * 4;
  }
  uint16 varE32 = E->ymw_var_09;
  E->ymw_var_00 = YappingMaw_Func_16(E->ymw_var_21, varE32) - E->ymw_var_0E;
  E->ymw_var_02 = YappingMaw_Func_16(E->ymw_var_22, varE32) - E->ymw_var_0E;
  E->ymw_var_04 = YappingMaw_Func_16(E->ymw_var_23, varE32) - E->ymw_var_0E;
  E->ymw_var_06 = YappingMaw_Func_16(E->ymw_var_24, varE32) - E->ymw_var_0E;
  varE32 = E->ymw_var_09 >> 1;
  E->ymw_var_01 = YappingMaw_Func_17(E->ymw_var_21, varE32) - E->ymw_var_0F;
  E->ymw_var_03 = YappingMaw_Func_17(E->ymw_var_22, varE32) - E->ymw_var_0F;
  E->ymw_var_05 = YappingMaw_Func_17(E->ymw_var_23, varE32) - E->ymw_var_0F;
  E->ymw_var_07 = YappingMaw_Func_17(E->ymw_var_24, varE32) - E->ymw_var_0F;
  YappingMaw_Func_4();
  YappingMaw_Func_5();
  YappingMaw_Func_6();
  YappingMaw_Func_7();
  E->ymw_var_00 += E->ymw_var_27;
  E->ymw_var_01 += E->ymw_var_28;
  E->ymw_var_02 += E->ymw_var_29;
  E->ymw_var_03 += E->ymw_var_2A;
  E->ymw_var_04 += E->ymw_var_2B;
  E->ymw_var_05 += E->ymw_var_2C;
  E->ymw_var_06 += E->ymw_var_2D;
  E->ymw_var_07 += E->ymw_var_2E;
  E->base.x_pos = E->ymw_var_06 + E->ymw_var_0C;
  E->base.y_pos = E->ymw_var_07 + E->ymw_var_0D;
  YappingMaw_Func_9(cur_enemy_index);
  if (!sign16(E->ymw_var_B)) {
    if (!sign16(E->ymw_var_B - 128)) {
      E->ymw_var_B = 128;
      E->ymw_var_C = 0;
      E->ymw_var_D += 4;
    }
LABEL_19:
    if (E->ymw_var_30)
      YappingMaw_Func_10();
    return;
  }
  E->ymw_var_A = FUNC16(YappingMaw_Func_11);
  E->ymw_var_35 = 48;
  if (E->ymw_var_34 == 4) {
    E->base.current_instruction = addr_kYappingMaw_Ilist_A01F;
  } else if (E->ymw_var_34 == 12) {
    E->base.current_instruction = addr_kYappingMaw_Ilist_A03D;
  } else {
    E->base.current_instruction = addr_off_A8A025;
  }
  if (!E->ymw_parameter_2) {
    if (E->ymw_var_34 == 4) {
      E->base.current_instruction = addr_kYappingMaw_Ilist_A05B;
    } else {
      if (E->ymw_var_34 != 12) {
        E->base.current_instruction = addr_off_A8A061;
        return;
      }
      E->base.current_instruction = addr_kYappingMaw_Ilist_A079;
    }
    goto LABEL_19;
  }
}

void YappingMaw_Func_9(uint16 k) {  // 0xA8A63E
  Enemy_YappingMaw *E = Get_YappingMaw(k);
  int v2 = E->ymw_var_D >> 1;
  AddToHiLo(&E->ymw_var_B, &E->ymw_var_C, __PAIR32__(kCommonEnemySpeeds_Quadratic[v2 + 1], kCommonEnemySpeeds_Quadratic[v2]));
  E->ymw_var_D += 8;
}

void YappingMaw_Func_10(void) {  // 0xA8A665
  CallSomeSamusCode(3);
  Enemy_YappingMaw *E = Get_YappingMaw(cur_enemy_index);
  samus_x_pos = E->ymw_var_32 + E->base.x_pos;
  samus_y_pos = E->ymw_var_33 + E->base.y_pos;
  EnemyFunc_B7A1();
}

void YappingMaw_Func_11(void) {  // 0xA8A68A
  Enemy_YappingMaw *E = Get_YappingMaw(cur_enemy_index);
  if (E->ymw_var_30)
    YappingMaw_Func_10();
  if ((--E->ymw_var_E & 0x8000) != 0 && samus_input_handler != FUNC16(Samus_InputHandler_E91D)) {
    samus_input_handler = FUNC16(Samus_InputHandler_E913);
    E->ymw_var_30 = 0;
    E->ymw_var_35 = 48;
    E->ymw_var_E = 64;
    E->ymw_var_A = FUNC16(YappingMaw_Func_1);
  }
}

void YappingMaw_Func_12(void) {  // 0xA8A6C4
  Enemy_YappingMaw *E = Get_YappingMaw(cur_enemy_index);
  int v1 = E->ymw_var_40 >> 1;
  eproj_x_pos[v1] = E->ymw_var_0C;
  eproj_y_pos[v1] = E->ymw_var_0D;
}

void YappingMaw_Func_13(void) {  // 0xA8A6DB
  Enemy_YappingMaw *E = Get_YappingMaw(cur_enemy_index);
  int v1 = E->ymw_var_41 >> 1;
  eproj_x_pos[v1] = E->ymw_var_00 + E->ymw_var_0C;
  eproj_y_pos[v1] = E->ymw_var_01 + E->ymw_var_0D;
}

void YappingMaw_Func_14(void) {  // 0xA8A6FC
  Enemy_YappingMaw *E = Get_YappingMaw(cur_enemy_index);
  int v1 = E->ymw_var_42 >> 1;
  eproj_x_pos[v1] = E->ymw_var_02 + E->ymw_var_0C;
  eproj_y_pos[v1] = E->ymw_var_03 + E->ymw_var_0D;
}

void YappingMaw_Func_15(void) {  // 0xA8A71D
  Enemy_YappingMaw *E = Get_YappingMaw(cur_enemy_index);
  int v1 = E->ymw_var_43 >> 1;
  eproj_x_pos[v1] = E->ymw_var_04 + E->ymw_var_0C;
  eproj_y_pos[v1] = E->ymw_var_05 + E->ymw_var_0D;
}

uint16 YappingMaw_Func_16(uint16 a, uint16 varE32) {  // 0xA8A73E
  return YappingMaw_Func_17(a - 64, varE32);
}

uint16 YappingMaw_Func_17(uint16 a, uint16 varE32) {  // 0xA8A742
  int16 v1;
  uint16 r20 = 0;

  uint16 r22 = 0;
  v1 = kSine16bit[(uint8)-a];
  if (v1 < 0) {
    v1 = -v1;
    ++r20;
  }
  WriteReg(WRMPYA, HIBYTE(v1));
  if ((uint8)varE32) {
    uint16 prod = Mult8x8(HIBYTE(v1), varE32);
    r22 = 2 * ((prod & 0xFF00) >> 8);
    if (r20)
      r22 = (2 * ((uint16)-prod >> 8)) | 0xFF00;
  }
  return r22;
}

void YappingMaw_Touch(void) {  // 0xA8A799
  Enemy_YappingMaw *E = Get_YappingMaw(cur_enemy_index);
  if ((E->ymw_var_35 & 0x8000) != 0 && !E->ymw_var_30) {
    E->ymw_var_35 = 0;
    E->ymw_var_30 = 1;
    samus_input_handler = FUNC16(nullsub_152);
  }
}

void YappingMaw_Shot(void) {  // 0xA8A7BD
  NormalEnemyShotAi();
  Enemy_YappingMaw *E = Get_YappingMaw(cur_enemy_index);
  if (E->base.health) {
    if (E->base.frozen_timer) {
      if (samus_input_handler != FUNC16(Samus_InputHandler_E91D))
        samus_input_handler = FUNC16(Samus_InputHandler_E913);
      E->ymw_var_30 = 0;
    }
  } else {
    *(uint16 *)((uint8 *)eproj_id + E->ymw_var_40) = 0;
    *(uint16 *)((uint8 *)eproj_id + E->ymw_var_41) = 0;
    *(uint16 *)((uint8 *)eproj_id + E->ymw_var_42) = 0;
    *(uint16 *)((uint8 *)eproj_id + E->ymw_var_43) = 0;
    sprite_instr_list_ptrs[E->ymw_var_46 >> 1] = 0;
    if (samus_input_handler != FUNC16(Samus_InputHandler_E91D))
      samus_input_handler = FUNC16(Samus_InputHandler_E913);
    Get_YappingMaw(cur_enemy_index)->ymw_var_30 = 0;
  }
}

void YappingMaw_Frozen(void) {  // 0xA8A835
  NormalEnemyFrozenAI();
  Enemy_YappingMaw *E = Get_YappingMaw(cur_enemy_index);
  YappingMaw_Func_18(E->ymw_var_40);
  YappingMaw_Func_18(E->ymw_var_41);
  YappingMaw_Func_18(E->ymw_var_42);
  YappingMaw_Func_18(E->ymw_var_43);
  YappingMaw_Func_19();
}

void YappingMaw_Func_18(uint16 j) {  // 0xA8A85D
  int v1 = j >> 1;
  Enemy_YappingMaw *E = Get_YappingMaw(cur_enemy_index);
  eproj_gfx_idx[v1] = E->ymw_var_45 | eproj_gfx_idx[v1] & 0xF1FF;
  if (E->base.frozen_timer) {
    eproj_gfx_idx[v1] = eproj_gfx_idx[v1] & 0xF1FF | 0xC00;
    uint16 frozen_timer = E->base.frozen_timer;
    if (sign16(frozen_timer - 90)) {
      if ((frozen_timer & 2) == 0)
        eproj_gfx_idx[v1] = E->ymw_var_45 | eproj_gfx_idx[v1] & 0xF1FF;
    }
  }
}

void YappingMaw_Func_19(void) {  // 0xA8A899
  Enemy_YappingMaw *E = Get_YappingMaw(cur_enemy_index);
  uint16 r18 = E->ymw_var_45;
  uint16 r20 = E->ymw_var_46;
  sprite_palettes[r20 >> 1] = r18 | sprite_palettes[r20 >> 1] & 0xF1FF;
  if (E->base.frozen_timer) {
    sprite_palettes[r20 >> 1] = sprite_palettes[r20 >> 1] & 0xF1FF | 0xC00;
    uint16 frozen_timer = E->base.frozen_timer;
    if (sign16(frozen_timer - 90)) {
      if ((frozen_timer & 2) == 0)
        sprite_palettes[r20 >> 1] = r18 | sprite_palettes[r20 >> 1] & 0xF1FF;
    }
  }
}

