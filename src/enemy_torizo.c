// Enemy AI - Torizo
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "enemy_types.h"

#define g_off_AAC967 ((uint16*)RomFixedPtr(0xaac967))

static const uint16 g_word_AAB096 = 6;
static const uint16 g_word_AAB098 = 5;
static const uint16 g_word_AAB09A = 3;

static const int16 g_word_AAC3EE[16] = { -9, -6, -7, 5, -16, -7, 0, 0, 9, 6, 7, -5, 16, 7, 0, 0 };
static const int16 g_word_AAC40E[8] = { 0, -6, -6, -7, 0, 0, 0, 0 };
static const int16 g_word_AAC440[16] = { -9, -6, -7, 5, -16, -7, 0, 0, 9, 6, 7, -5, 16, 7, 0, 0 };
static const int16 g_word_AAC460[8] = { 0, -6, -6, -7, 0, 0, 0, 0 };
static const int16 g_word_AAC4BD[20] = {
  -5, 0, -5, -19, -16, -7, 0, -7, -17, -18, 5, 0, 5, 19, 16, 7,
   0, 7, 17,  18,
};
static const int16 g_word_AAC532[20] = {
  -5, 0,   -5, -0x13, -0x10, -7, 0, -7, -0x11, -0x12, 5, 0, 5, 0x13, 0x10, 7,
   0, 7, 0x11,  0x12,
};
static const int16 g_word_AAC95F[2] = { 0xdb, 0x1a8 };
static const int16 g_word_AAC963[2] = { 0xb3, 0x90 };
static const int16 g_word_AAC96B[2] = { 0x2800, 0x2800 };
static const int16 g_word_AAC96F[2] = { 0x12, 0x12 };
static const int16 g_word_AAC973[2] = { 0x30, 0x29 };

static const uint16 kTorizo_Palette[16] = { 0x3800, 0x3ff, 0x33b, 0x216, 0x113, 0x6b1e, 0x4a16, 0x3591, 0x20e9, 0x1580, 0x1580, 0x1580, 0x1580, 0x1580, 0x1580, 0x1580 };
static const uint16 kTorizo_Palettes_1[16] = { 0x3800, 0x2df, 0x1d7, 0xac, 0x5a73, 0x41ad, 0x2d08, 0x1863, 0x1486, 0x145, 0x145, 0x145, 0x7fff, 0x145, 0x145, 0 };
static const uint16 kTorizo_Palettes_2[16] = { 0x3800, 0x679f, 0x5299, 0x252e, 0x14aa, 0x5efc, 0x4657, 0x35b2, 0x2d70, 0x5b7f, 0x3df8, 0x2d0e, 0x5f5f, 0x5e1a, 0x5d35, 0xc63 };
static const uint16 kTorizo_Palettes_3[16] = { 0x3800, 0x4aba, 0x35b2, 0x847, 3, 0x4215, 0x2970, 0x18cb, 0x1089, 0x463a, 0x28b3, 0x1809, 0x6f7f, 0x51fd, 0x4113, 0xc63 };
static const uint16 kTorizo_Palettes_4[16] = { 0x3800, 0x56ba, 0x41b2, 0x1447, 0x403, 0x4e15, 0x3570, 0x24cb, 0x1868, 0x6f7f, 0x51f8, 0x410e, 0x31f, 0x1da, 0xf5, 0xc63 };
static const uint16 kTorizo_Palettes_5[16] = { 0x3800, 0x4215, 0x2d0d, 2, 0, 0x3970, 0x20cb, 0xc26, 0x403, 0x463a, 0x28b3, 0x1809, 0x6f7f, 0x51fd, 0x4113, 0xc63 };
static const uint16 kTorizo_Palettes_6[16] = { 0x3800, 0x6ab5, 0x49b0, 0x1c45, 0xc01, 0x5613, 0x416d, 0x2cc9, 0x2066, 0x5714, 0x31cc, 0x14e3, 0x5630, 0x3569, 0x1883, 0xc66 };
static const uint16 kTorizo_Palettes_7[16] = { 0x3800, 0x5610, 0x350b, 0x800, 0, 0x416e, 0x2cc8, 0x1823, 0xc01, 0x6a31, 0x4caa, 0x2406, 0x7f7b, 0x75f4, 0x4d10, 0xc63 };
static const uint16 kTorizo_Palettes_8[16] = { 0x3800, 0x4bbe, 0x6b9, 0xa8, 0, 0x173a, 0x276, 0x1f2, 0x14d, 0x73e0, 0x4f20, 0x2a20, 0x7fe0, 0x5aa0, 0x5920, 0x43 };
static const uint16 kTorizo_Palettes_10[16] = { 0x3800, 0x3719, 0x214, 3, 0, 0x295, 0x1d1, 0x14d, 0xa8, 0x4b40, 0x25e0, 0xe0, 0x6b40, 0x4600, 0x4480, 0 };
static const int16 g_word_AAD59A[20] = {
  -5, 0, -5, -19, -16, -7, 0, -7, -17, -18, 5, 0, 5, 19, 16, 7,
   0, 7, 17,  18,
};

static void CallTorizoFunc(uint32 ea, uint16 k);

const uint16 *Torizo_Instr_3(uint16 k, const uint16 *jp) {  // 0xAAB09C
  Get_Torizo(k)->toriz_var_E = jp[0];
  return jp + 1;
}

const uint16 *Torizo_Instr_31(uint16 k, const uint16 *jp) {  // 0xAAB11D
  Enemy_Torizo *E = Get_Torizo(k);
  E->toriz_parameter_2 = E->toriz_parameter_2 | 0x8000;
  SpawnEprojWithGfx(0, k, addr_stru_86A95B);
  SpawnEprojWithGfx(0, k, addr_stru_86A95B);
  SpawnEprojWithGfx(0, k, addr_stru_86A95B);
  SpawnEprojWithGfx(0, k, addr_stru_86A95B);
  SpawnEprojWithGfx(0, k, addr_stru_86A95B);
  SpawnEprojWithGfx(0, k, addr_stru_86A95B);
  return jp;
}

const uint16 *Torizo_Instr_33(uint16 k, const uint16 *jp) {  // 0xAAB1BE
  Enemy_Torizo *E = Get_Torizo(k);
  E->toriz_parameter_2 |= 0x4000;
  return jp;
}

const uint16 *Torizo_Instr_36(uint16 k, const uint16 *jp) {  // 0xAAB224
  Enemy_Torizo *E = Get_Torizo(k);
  E->base.properties &= ~kEnemyProps_Invisible;
  return jp;
}

const uint16 *Torizo_Instr_37(uint16 k, const uint16 *jp) {  // 0xAAB22E
  Enemy_Torizo *E = Get_Torizo(k);
  E->base.properties |= kEnemyProps_Invisible;
  return jp;
}

const uint16 *Torizo_Instr_35(uint16 k, const uint16 *jp) {  // 0xAAB238
  for (int i = 30; i >= 0; i -= 2) {
    int v3 = i >> 1;
    target_palettes[v3 + 160] = 0;
    target_palettes[v3 + 144] = 0;
  }
  return jp;
}

const uint16 *Torizo_Instr_38(uint16 k, const uint16 *jp) {  // 0xAAB24D
  SetBossBitForCurArea(4);
  QueueMusic_Delayed8(g_word_AAB09A);
  if (area_index)
    Enemy_ItemDrop_GoldenTorizo(k);
  else
    Enemy_ItemDrop_BombTorizo(k);
  return jp;
}

const uint16 *Torizo_Instr_6(uint16 k, const uint16 *jp) {  // 0xAAB271
  sub_82DAF7(0x600);
  return jp;
}

const uint16 *Torizo_Instr_5(uint16 k, const uint16 *jp) {  // 0xAAB94D
  Torizo_C268();
  return jp;
}

const uint16 *Torizo_Instr_9(uint16 k, const uint16 *jp) {  // 0xAAB951
  QueueMusic_Delayed8(g_word_AAB098);
  SpawnPalfxObject(addr_stru_8DF759);
  return jp;
}

void Torizo_C20A(uint16 k) {  // 0xAAC20A
  int16 v2;

  Enemy_Torizo *E = Get_Torizo(k);
  if ((E->toriz_parameter_1 & 0x8000) != 0)
    v2 = 512;
  else
    v2 = -512;
  E->toriz_var_A = v2;
  E->toriz_var_B = -1472;
  E->toriz_var_C = 40;
  E->base.instruction_timer = 1;
}

void Torizo_C22D(uint16 k) {  // 0xAAC22D
  int16 v2;

  Enemy_Torizo *E = Get_Torizo(k);
  if ((E->toriz_parameter_1 & 0x8000) != 0)
    v2 = -768;
  else
    v2 = 768;
  E->toriz_var_A = v2;
  E->toriz_var_B = -1152;
  E->toriz_var_C = 40;
  E->base.instruction_timer = 1;
}

void Torizo_C250(void) {  // 0xAAC250
  for (int i = 30; i >= 0; i -= 2) {
    int v1 = i >> 1;
    target_palettes[v1 + 160] = kTorizo_Palettes_3[v1];
    target_palettes[v1 + 144] = kTorizo_Palettes_2[v1];
  }
}

void Torizo_C268(void) {  // 0xAAC268
  for (int i = 30; i >= 0; i -= 2) {
    int v1 = i >> 1;
    target_palettes[v1 + 160] = kTorizo_Palettes_5[v1];
    target_palettes[v1 + 144] = kTorizo_Palettes_4[v1];
  }
}

void Torizo_C280(void) {  // 0xAAC280
  for (int i = 30; i >= 0; i -= 2) {
    int v1 = i >> 1;
    target_palettes[v1 + 160] = kTorizo_Palettes_7[v1];
    target_palettes[v1 + 144] = kTorizo_Palettes_6[v1];
  }
}

void Torizo_C298(void) {  // 0xAAC298
  for (int i = 30; i >= 0; i -= 2) {
    int v1 = i >> 1;
    target_palettes[v1 + 160] = kTorizo_Palettes_10[v1];
    target_palettes[v1 + 144] = kTorizo_Palettes_8[v1];
  }
}

void Torizo_C2B0(void) {  // 0xAAC2B0
  for (int i = 30; i >= 0; i -= 2) {
    int v1 = i >> 1;
    palette_buffer[v1 + 160] = kTorizo_Palettes_5[v1];
    palette_buffer[v1 + 144] = kTorizo_Palettes_4[v1];
  }
}

const uint16 *Torizo_Instr_7(uint16 k, const uint16 *jp) {  // 0xAAC2C8
  return jp;
}

const uint16 *Torizo_Instr_2(uint16 k, const uint16 *jp) {  // 0xAAC2C9
  Get_Torizo(k)->toriz_var_04 = 30583;
  return jp;
}

const uint16 *Torizo_Instr_8(uint16 k, const uint16 *jp) {  // 0xAAC2D1
  Get_Torizo(k)->toriz_var_04 = 0;
  return jp;
}

const uint16 *Torizo_Instr_25(uint16 k, const uint16 *jp) {  // 0xAAC2D9
  if ((Get_Torizo(k)->toriz_parameter_2 & 0x4000) != 0)
    return INSTR_RETURN_ADDR(jp[0]);
  if (area_index)
    return INSTR_RETURN_ADDR(jp[1]);
  else
    return jp + 2;
}

const uint16 *Torizo_Instr_22(uint16 k, const uint16 *jp) {  // 0xAAC2ED
  Get_Torizo(k)->toriz_var_00 = jp[0];
  return jp + 1;
}

const uint16 *Torizo_Instr_19(uint16 k, const uint16 *jp) {  // 0xAAC2F7
  return INSTR_RETURN_ADDR(Get_Torizo(k)->toriz_var_00);
}

const uint16 *Torizo_Instr_32(uint16 k, const uint16 *jp) {  // 0xAAC2FD
  return INSTR_RETURN_ADDR(Get_Torizo(k)->toriz_var_01);
}

const uint16 *Torizo_Instr_30(uint16 k, const uint16 *jp) {  // 0xAAC303
  uint16 a = jp[0];
  for (int i = 5; i >= 0; --i)
    SpawnEprojWithRoomGfx(addr_kEproj_BombTorizoLowHealthExplode, a);
  Enemy_Torizo *E = Get_Torizo(k);
  E->base.current_instruction = INSTR_ADDR_TO_PTR(k, jp + 1);
  E->base.flash_timer = 40;
  E->base.instruction_timer = 40;
  return 0;
}

const uint16 *Torizo_Instr_34(uint16 k, const uint16 *jp) {  // 0xAAC32F
  SpawnEprojWithRoomGfx(addr_kEproj_BombTorizoDeathExplosion, 0);
  Enemy_Torizo *E = Get_Torizo(k);
  E->base.current_instruction = INSTR_ADDR_TO_PTR(k, jp);
  E->base.flash_timer = 1;
  E->base.instruction_timer = 1;
  return 0;
}

const uint16 *Torizo_Instr_24(uint16 k, const uint16 *jp) {  // 0xAAC34A
  SpawnEprojWithRoomGfx(addr_kEproj_TourianLandingDustCloudsRightFoot, 0);
  SpawnEprojWithRoomGfx(addr_stru_86AFF3, 0);
  return jp;
}

const uint16 *Torizo_Instr_12(uint16 k, const uint16 *jp) {  // 0xAAC35B
  uint16 health = Get_Torizo(k)->base.health;
  if (health < 0x15E)
    SpawnEprojWithGfx(health, k, addr_kEproj_BombTorizoLowHealthInitialDrool);
  return jp;
}

const uint16 *Torizo_Instr_10(uint16 k, const uint16 *jp) {  // 0xAAC36D
  Enemy_Torizo *E = Get_Torizo(k);
  E->toriz_parameter_1 |= 0x4000;
  return jp;
}

const uint16 *Torizo_Instr_11(uint16 k, const uint16 *jp) {  // 0xAAC377
  Enemy_Torizo *E = Get_Torizo(k);
  E->toriz_parameter_1 &= 0x1FFF;
  ++E->toriz_var_09;
  return jp;
}

const uint16 *Torizo_Instr_29(uint16 k, const uint16 *jp) {  // 0xAAC38A
  Enemy_Torizo *E = Get_Torizo(k);
  E->toriz_parameter_1 = E->toriz_parameter_1 & 0x1FFF | 0x8000;
  ++E->toriz_var_09;
  return jp;
}

const uint16 *Torizo_Instr_1(uint16 k, const uint16 *jp) {  // 0xAAC3A0
  Enemy_Torizo *E = Get_Torizo(k);
  E->toriz_parameter_1 = E->toriz_parameter_1 & 0x1FFF | 0x2000;
  ++E->toriz_var_09;
  return jp;
}

const uint16 *Torizo_Instr_28(uint16 k, const uint16 *jp) {  // 0xAAC3B6
  Enemy_Torizo *E = Get_Torizo(k);
  E->toriz_parameter_1 = E->toriz_parameter_1 & 0x1FFF | 0xA000;
  ++E->toriz_var_09;
  return jp;
}

const uint16 *Torizo_Instr_4(uint16 k, const uint16 *jp) {  // 0xAAC3CC
  uint16 v2 = jp[0];
  Enemy_Torizo *E = Get_Torizo(k);
  E->base.x_pos += g_word_AAC3EE[v2 >> 1];
  E->base.y_pos += g_word_AAC40E[(v2 & 0xF) >> 1];
  return jp + 1;
}

const uint16 *Torizo_Instr_40(uint16 k, const uint16 *jp) {  // 0xAAC41E
  uint16 v2 = jp[0];
  Enemy_Torizo *E = Get_Torizo(k);
  E->base.x_pos -= g_word_AAC440[v2 >> 1];
  E->base.y_pos -= g_word_AAC460[(v2 & 0xF) >> 1];
  return jp + 1;
}

const uint16 *Torizo_Instr_16(uint16 k, const uint16 *jp) {  // 0xAAC470
  Enemy_Torizo *E = Get_Torizo(k);
  E->toriz_var_A = g_word_AAC4BD[jp[0] >> 1];
  if (Enemy_MoveRight_IgnoreSlopes(k, INT16_SHL16(E->toriz_var_A))) {
    E->toriz_var_03 = 0;
    if ((E->toriz_parameter_1 & 0x8000) != 0)
      return INSTR_RETURN_ADDR(addr_kTorizo_Ilist_B962);
    else
      return INSTR_RETURN_ADDR(addr_kTorizo_Ilist_BDD8);
  } else {
    EnemyFunc_C8AD(k);
    if (((E->toriz_parameter_1 ^ (samus_x_pos - E->base.x_pos)) & 0x8000) == 0) {
      if (!E->toriz_var_03)
        E->toriz_var_03 = 72;
      return jp + 1;
    } else {
      return jp + 1;
    }
  }
}

const uint16 *Torizo_Instr_27(uint16 k, const uint16 *jp) {  // 0xAAC4E5
  Enemy_Torizo *E = Get_Torizo(k);
  E->toriz_var_A = g_word_AAC532[jp[0] >> 1];
  if (Enemy_MoveRight_IgnoreSlopes(k, INT16_SHL16(E->toriz_var_A))) {
    E->toriz_var_03 = 0;
    if ((E->toriz_parameter_1 & 0x8000) != 0)
      return INSTR_RETURN_ADDR(addr_kTorizo_Ilist_BD0E);
    else
      return INSTR_RETURN_ADDR(addr_kTorizo_Ilist_C188);
  } else {
    EnemyFunc_C8AD(k);
    if (((E->toriz_parameter_1 ^ (samus_x_pos - E->base.x_pos)) & 0x8000) == 0) {
      if (!E->toriz_var_03)
        E->toriz_var_03 = 72;
      return jp + 1;
    } else {
      return jp + 1;
    }
  }
}

const uint16 *Torizo_Instr_23(uint16 k, const uint16 *jp) {  // 0xAAC55A
  if ((Get_Torizo(k)->toriz_var_B & 0x8000) != 0)
    return INSTR_RETURN_ADDR(jp[0]);
  else
    return jp + 1;
}

const uint16 *Torizo_Instr_14(uint16 k, const uint16 *jp) {  // 0xAAC567
  if (CompareDistToSamus_X(k, 0x38) & 1)
    return jp + 1;
  Enemy_Torizo *E = Get_Torizo(k);
  if (((E->toriz_parameter_1 ^ (samus_x_pos - E->base.x_pos)) & 0x8000) == 0)
    return jp + 1;
  E->toriz_var_00 = INSTR_ADDR_TO_PTR(k, jp + 1);
  return INSTR_RETURN_ADDR(jp[0]);
}

const uint16 *Torizo_Instr_15(uint16 k, const uint16 *jp) {  // 0xAAC58B
  if (CompareDistToSamus_X(k, 0x20) & 1 || (Torizo_Func_12(k) & 0x8000) != 0)
    return jp + 1;
  Torizo_C22D(k);
  return INSTR_RETURN_ADDR(jp[0]);
}

const uint16 *Torizo_Instr_26(uint16 k, const uint16 *jp) {  // 0xAAC5A4
  Enemy_Torizo *E = Get_Torizo(k);
  E->toriz_var_00 = INSTR_ADDR_TO_PTR(k, jp + 2);
  if (samus_missiles < 5 || ((nmi_frame_counter_word + (samus_x_pos & 1) + (samus_x_pos >> 1)) & 8) != 0)
    return INSTR_RETURN_ADDR(jp[0]);
  else
    return INSTR_RETURN_ADDR(jp[1]);
}

const uint16 *Torizo_Instr_18(uint16 k, const uint16 *jp) {  // 0xAAC5CB
  SpawnEprojWithGfx(0, k, addr_kEproj_BombTorizosChozoOrbs);
  SpawnEprojWithGfx(0, k, addr_kEproj_BombTorizosChozoOrbs);
  SpawnEprojWithGfx(0, k, addr_kEproj_BombTorizosChozoOrbs);
  return jp;
}

const uint16 *Torizo_Instr_20(uint16 k, const uint16 *jp) {  // 0xAAC5E3
  SpawnEprojWithGfx(jp[0], k, addr_kEproj_BombTorizoSonicBoom);
  return jp + 1;
}

const uint16 *Torizo_Instr_44(uint16 k, const uint16 *jp) {  // 0xAAC5F2
  SpawnEprojWithGfx(jp[0], k, addr_kEproj_GoldenTorizoSonicBoom);
  return jp + 1;
}

const uint16 *Torizo_Instr_21(uint16 k, const uint16 *jp) {  // 0xAAC601
  SpawnEprojWithRoomGfx(addr_kEproj_BombTorizoExplosiveSwipe, jp[0]);
  return jp + 1;
}

const uint16 *Torizo_Instr_17(uint16 k, const uint16 *jp) {  // 0xAAC610
  QueueSfx2_Max6(0x27);
  return jp;
}

const uint16 *Torizo_Instr_13(uint16 k, const uint16 *jp) {  // 0xAAC618
  QueueSfx2_Max6(0x4B);
  return jp;
}

void Torizo_C620(uint16 k) {  // 0xAAC620
  if (!area_index && (random_number & 0x8142) == 0) {
    uint16 health = Get_Torizo(k)->base.health;
    if (health && health < 0x15E)
      SpawnEprojWithGfx(health, k, addr_kEproj_BombTorizoLowHealthInitialDrool);
  }
}

void Torizo_C643(uint16 k) {  // 0xAAC643
  Enemy_Torizo *E = Get_Torizo(k);
  if (Enemy_MoveDown(k, INT16_SHL8(E->toriz_var_B))) {
    int16 v3 = E->toriz_var_B;
    if (v3 >= 0 && v3 != 256) {
      earthquake_type = 4;
      earthquake_timer = 32;
      E->toriz_var_B = 256;
    }
  } else {
    E->toriz_var_B += 40;
  }
}

void Torizo_Hurt(void) {  // 0xAAC67E
  Torizo_C620(cur_enemy_index);
  if (Get_Torizo(cur_enemy_index)->base.flash_timer & 1) {
    for (int i = 30; i >= 0; i -= 2) {
      int v2 = i >> 1;
      palette_buffer[v2 + 160] = 0x7FFF;
      palette_buffer[v2 + 144] = 0x7FFF;
    }
  } else {
    Torizo_C2B0();
  }
}

static void CallTorizoFunc(uint32 ea, uint16 k) {
  switch (ea) {
  case fnnullsub_270: return;
  case fnTorizo_Func_3: Torizo_Func_3(k); return;
  case fnTorizo_Func_4: Torizo_Func_4(k); return;
  case fnTorizo_Func_1: Torizo_Func_1(k); return;
  case fnTorizo_Func_9: Torizo_Func_9(k); return;
  case fnTorizo_Func_10: Torizo_Func_10(k); return;
  case fnTorizo_D5E6: Torizo_D5E6(k); return;
  default: Unreachable();
  }
}

void Torizo_Main(void) {  // 0xAAC6A4
  Enemy_Torizo *E = Get_Torizo(cur_enemy_index);
  CallTorizoFunc(E->toriz_var_E | 0xAA0000, cur_enemy_index);
}

void Torizo_Func_2(uint16 k) {  // 0xAAC6AC
  Torizo_C643(k);
  if (!(sub_82DAF7(0x600) & 1))
    Get_Torizo(k)->toriz_var_E = FUNC16(nullsub_270);
}

void Torizo_Func_3(uint16 k) {  // 0xAAC6BF
  Torizo_C620(k);
  Torizo_C643(k);
}

void Torizo_Func_4(uint16 k) {  // 0xAAC6C6
  Enemy_Torizo *E = Get_Torizo(k);
  E->base.properties |= kEnemyProps_Tangible;
  uint16 v2 = 78;
  while (plm_header_ptr[v2 >> 1] != addr_kPlmHeader_D6EA) {
    v2 -= 2;
    if ((v2 & 0x8000) != 0) {
      QueueMusic_Delayed8(g_word_AAB096);
      E->base.properties &= ~kEnemyProps_Tangible;
      E->base.current_instruction += 2;
      E->base.instruction_timer = 1;
      return;
    }
  }
}

void Torizo_Func_1(uint16 k) {  // 0xAAC6FF
  Torizo_C620(k);
  Enemy_Torizo *E = Get_Torizo(k);
  if ((E->toriz_parameter_2 & 0x8000) != 0 || E->base.health >= 0x15E) {
    if ((E->toriz_parameter_2 & 0x4000) != 0 || E->base.health >= 0x64) {
      CallEnemyPreInstr(E->toriz_var_F | 0xAA0000);
    } else {
      uint16 v2;
      if ((E->toriz_parameter_1 & 0x8000) != 0)
        v2 = addr_kTorizo_Ilist_BD0E;
      else
        v2 = addr_kTorizo_Ilist_C188;
      E->toriz_var_00 = v2;
      E->base.current_instruction = addr_kTorizo_Ilist_B155;
      E->base.instruction_timer = 1;
    }
  } else {
    E->toriz_var_01 = E->base.current_instruction;
    E->base.current_instruction = addr_kTorizo_Ilist_B0E5;
    E->base.instruction_timer = 1;
  }
}

void Torizo_Func_5(uint16 k) {  // 0xAAC752
  uint16 v3;
  uint16 v8;

  Enemy_Torizo *E = Get_Torizo(k);
  if ((E->toriz_parameter_2 & 0x4000) != 0) {
    uint16 torizo_var_03 = E->toriz_var_03;
    if (!torizo_var_03 || (v8 = torizo_var_03 - 1, (E->toriz_var_03 = v8) != 0)) {
      uint16 v10 = abs16(E->toriz_var_A) + 1;
      if (v10 >= 0x10)
        v10 = 15;
      if (!(Enemy_MoveDown(k, INT16_SHL16(v10)))) {
        uint16 v11;
        if ((E->toriz_parameter_1 & 0x8000) != 0)
          v11 = addr_off_AAC0F2;
        else
          v11 = addr_off_AABC78;
        E->base.current_instruction = v11;
        E->base.instruction_timer = 1;
        E->toriz_var_B = 256;
        E->toriz_var_A = 0;
      }
    } else {
      E->toriz_var_03 = 0;
      uint16 v9;
      if ((E->toriz_parameter_1 & 0x8000) != 0)
        v9 = addr_kTorizo_Ilist_BD0E;
      else
        v9 = addr_kTorizo_Ilist_C188;
      E->base.current_instruction = v9;
      E->base.instruction_timer = 1;
    }
  } else {
    uint16 v2 = E->toriz_var_03;
    if (!v2 || (v3 = v2 - 1, (E->toriz_var_03 = v3) != 0)) {
      uint16 v5 = abs16(E->toriz_var_A) + 1;
      if (v5 >= 0x10)
        v5 = 15;
      if (!(Enemy_MoveDown(k, INT16_SHL16(v5)))) {
        uint16 v6;
        if ((E->toriz_parameter_1 & 0x8000) != 0)
          v6 = addr_off_AAC0F2;
        else
          v6 = addr_off_AABC78;
        E->base.current_instruction = v6;
        E->base.instruction_timer = 1;
        E->toriz_var_B = 256;
        E->toriz_var_A = 0;
      }
    } else {
      E->toriz_var_03 = 0;
      uint16 v4;
      if ((E->toriz_parameter_1 & 0x8000) != 0)
        v4 = addr_kTorizo_Ilist_B962;
      else
        v4 = addr_kTorizo_Ilist_BDD8;
      E->base.current_instruction = v4;
      E->base.instruction_timer = 1;
    }
  }
}

void Torizo_Func_6(uint16 k) {  // 0xAAC828
  Torizo_C643(k);
}

void Torizo_Func_7(uint16 k) {  // 0xAAC82C
  Enemy_Torizo *E = Get_Torizo(k);
  Enemy_MoveRight_IgnoreSlopes(k, INT16_SHL8(E->toriz_var_A));
  EnemyFunc_C8AD(k);
  if (Enemy_MoveDown(k, INT16_SHL8(E->toriz_var_B))) {
    E->base.current_instruction = E->toriz_var_00;
    E->base.instruction_timer = 1;
    E->toriz_var_B = 256;
    earthquake_type = 4;
    earthquake_timer = 32;
  } else {
    E->toriz_var_B += E->toriz_var_C;
  }
}

void Torizo_Init(void) {  // 0xAAC87F
  Enemy_Torizo *E = Get_Torizo(cur_enemy_index);
  if (CheckBossBitForCurArea(4) & 1) {
    E->base.properties |= kEnemyProps_Deleted;
  } else {
    int v2 = area_index >> 1;
    E->base.properties |= g_word_AAC96B[v2];
    E->base.extra_properties |= 4;
    E->base.x_width = g_word_AAC96F[v2];
    E->base.y_height = g_word_AAC973[v2];
    E->toriz_var_E = FUNC16(Torizo_Func_3);
    E->base.instruction_timer = 1;
    E->base.timer = 0;
    E->base.palette_index = 0;
    E->toriz_var_F = addr_locret_AAC95E;
    E->base.current_instruction = g_off_AAC967[v2];
    E->base.spritemap_pointer = addr_kTorizo_ExtSprmap_87D0;
    E->base.x_pos = g_word_AAC95F[v2];
    E->base.y_pos = g_word_AAC963[v2];
    E->toriz_var_A = 0;
    E->toriz_var_B = 256;
    for (int i = 30; i >= 0; i -= 2) {
      int v4 = i >> 1;
      target_palettes[v4 + 240] = kTorizo_Palettes_1[v4];
      target_palettes[v4 + 176] = kTorizo_Palette[v4];
    }
    if (area_index) {
      Torizo_C280();
      if (joypad1_lastkeys == 0xC0C0) {
        samus_health = 700;
        samus_max_health = 700;
        samus_max_reserve_health = 300;
        samus_reserve_health = 300;
        samus_missiles = 100;
        samus_max_missiles = 100;
        samus_super_missiles = 20;
        samus_max_super_missiles = 20;
        samus_power_bombs = 20;
        samus_max_power_bombs = 20;
        equipped_items = -3273;
        collected_items = -3273;
        equipped_beams = 4111;
        collected_beams = 4111;
      }
    } else {
      Torizo_C250();
      SpawnBombTorizoHaze();
    }
  }
}

void GoldTorizo_Touch(void) {  // 0xAAC977
  NormalEnemyTouchAiSkipDeathAnim_CurEnemy();
}

void Torizo_Shot(void) {  // 0xAAC97C
  if (area_index) {
    GoldTorizo_Shot();
  } else {
    Enemy_Torizo *E = Get_Torizo(cur_enemy_index);
    if (!(E->toriz_var_04 | E->base.flash_timer)) {
      NormalEnemyShotAiSkipDeathAnim_CurEnemy();
      if (!E->base.health) {
        E->toriz_var_E = FUNC16(nullsub_270);
        E->base.current_instruction = addr_kTorizo_Ilist_B1C8;
        E->base.instruction_timer = 1;
        E->toriz_parameter_2 |= 0xC000;
        E->base.properties |= kEnemyProps_Tangible;
      }
    }
  }
}

void Torizo_Func_8(void) {  // 0xAAC9C2
  if (area_index)
    Torizo_D658();
}

const uint16 *Torizo_Instr_39(uint16 k, const uint16 *jp) {  // 0xAACACE
  if (Get_Torizo(k)->base.y_pos == 375)
    return jp + 1;
  else
    return INSTR_RETURN_ADDR(jp[0]);
}

const uint16 *Torizo_Instr_41(uint16 k, const uint16 *jp) {  // 0xAACADE
  Torizo_C298();
  return jp;
}

const uint16 *Torizo_Instr_42(uint16 k, const uint16 *jp) {  // 0xAACAE2
  QueueMusic_Delayed8(5);
  Enemy_Torizo *E = Get_Torizo(k);
  E->base.x_width = 18;
  E->base.y_height = 48;
  SpawnPalfxObject(addr_stru_8DF75D);
  return jp;
}

const uint16 *Torizo_Instr_48(uint16 k, const uint16 *jp) {  // 0xAACDD7
  Enemy_Torizo *E = Get_Torizo(k);
  E->toriz_parameter_2 &= ~0x1000;
  return jp;
}

const uint16 *Torizo_Instr_57(uint16 k, const uint16 *jp) {  // 0xAAD0E9
  SpawnEprojWithRoomGfx(addr_stru_86B1C0, 0);
  return jp;
}

const uint16 *Torizo_Instr_58(uint16 k, const uint16 *jp) {  // 0xAAD0F3
  int16 v2;

  v2 = 34;
  while (*(uint16 *)((uint8 *)eproj_id + (uint16)v2) != 0xB1C0) {
    v2 -= 2;
    if (v2 < 0)
      return jp + 1;
  }
  return INSTR_RETURN_ADDR(jp[0]);
}

const uint16 *Torizo_Instr_59(uint16 k, const uint16 *jp) {  // 0xAAD17B
  Enemy_Torizo *E = Get_Torizo(k);
  E->toriz_var_06 &= ~0x8000;
  return jp;
}

const uint16 *Torizo_Instr_62(uint16 k, const uint16 *jp) {  // 0xAAD187
  Enemy_Torizo *E = Get_Torizo(k);
  E->toriz_var_06 |= 0x8000;
  return jp;
}

const uint16 *Torizo_Instr_63(uint16 k, const uint16 *jp) {  // 0xAAD1E7
  Enemy_Torizo *E = Get_Torizo(k);
  E->toriz_parameter_2 &= ~0x2000;
  return jp;
}

void GoldTorizo_Main(void) {  // 0xAAD369
  Enemy_Torizo *E = Get_Torizo(cur_enemy_index);

  if (samus_pose == kPose_1B_FaceR_SpaceJump || samus_pose == kPose_1C_FaceL_SpaceJump) {
    ++E->toriz_var_07;
  } else {
    E->toriz_var_07 = 0;
  }
  CallTorizoFunc(E->toriz_var_E | 0xAA0000, cur_enemy_index);
}

const uint16 *Torizo_Instr_56(uint16 k, const uint16 *jp) {  // 0xAAD38F
  QueueSfx2_Max6(0x34);
  return jp;
}

const uint16 *Torizo_Instr_60(uint16 k, const uint16 *jp) {  // 0xAAD397
  QueueSfx2_Max6(0x67);
  return jp;
}

const uint16 *Torizo_Instr_46(uint16 k, const uint16 *jp) {  // 0xAAD39F
  QueueSfx2_Max6(0x48);
  return jp;
}

uint16 Torizo_Func_12(uint16 k) {  // 0xAAD3A7
  Enemy_Torizo *E = Get_Torizo(k);
  return E->toriz_parameter_1 ^ (E->base.x_pos - samus_x_pos);
}

void Torizo_Func_11(uint16 k) {  // 0xAAD3B2
  Enemy_Torizo *E = Get_Torizo(k);
  SetGoldenTorizoPalette(E->base.health);
}

void GoldTorizo_Hurt(void) {  // 0xAAD3BA
  Torizo_C620(cur_enemy_index);
  if (gEnemyData(cur_enemy_index)->flash_timer & 1) {
    for (int i = 30; i >= 0; i -= 2) {
      int v2 = i >> 1;
      palette_buffer[v2 + 160] = 0x7FFF;
      palette_buffer[v2 + 144] = 0x7FFF;
    }
  } else {
    Torizo_Func_11(cur_enemy_index);
  }
}

const uint16 *Torizo_Instr_47(uint16 k, const uint16 *jp) {  // 0xAAD3E0
  SpawnEprojWithRoomGfx(addr_kEproj_GoldenTorizoSuperMissile, 0);
  return jp;
}

const uint16 *Torizo_Instr_49(uint16 k, const uint16 *jp) {  // 0xAAD3EA
  if ((Torizo_Func_12(k) & 0x8000) == 0
      || !(CompareDistToSamus_X(k, 4) & 1)
      || CompareDistToSamus_X(k, 0x28) & 1
      || samus_pose != kPose_1D_FaceR_Morphball_Ground
      && samus_pose != kPose_1E_MoveR_Morphball_Ground
      && samus_pose != kPose_1F_MoveL_Morphball_Ground
      && samus_pose != kPose_79_FaceR_Springball_Ground
      && samus_pose != kPose_7A_FaceL_Springball_Ground
      && samus_pose != kPose_7B_MoveR_Springball_Ground
      && samus_pose != kPose_7C_MoveL_Springball_Ground) {
    return jp + 1;
  }
  Get_Torizo(k)->toriz_var_09 = 0;
  return INSTR_RETURN_ADDR(jp[0]);
}

const uint16 *Torizo_Instr_61(uint16 k, const uint16 *jp) {  // 0xAAD436
  SpawnEprojWithRoomGfx(addr_kEproj_GoldenTorizoEyeBeam, jp[0]);
  return jp + 1;
}

const uint16 *Torizo_Instr_53(uint16 k, const uint16 *jp) {  // 0xAAD445
  if ((Torizo_Func_12(k) & 0x8000) != 0
      || !(CompareDistToSamus_X(k, 0x20) & 1)
      || CompareDistToSamus_X(k, 0x60) & 1
      || (NextRandom() & 0x110) != 0) {
    return jp + 1;
  }
  Get_Torizo(k)->toriz_var_00 = INSTR_ADDR_TO_PTR(k, jp + 1);
  return INSTR_RETURN_ADDR(jp[0]);
}

const uint16 *Torizo_Instr_55(uint16 k, const uint16 *jp) {  // 0xAAD474
  Enemy_Torizo *E = Get_Torizo(k);
  if (E->base.health > 0x788 || (NextRandom() & 0x102) != 0)
    return INSTR_RETURN_ADDR(jp + 1);
  E->toriz_var_09 = 0;
  E->toriz_var_00 = INSTR_ADDR_TO_PTR(k, jp + 1);
  return INSTR_RETURN_ADDR(jp[0]);
}

const uint16 *Torizo_Instr_52(uint16 k, const uint16 *jp) {  // 0xAAD49B
  Enemy_Torizo *E = Get_Torizo(k);
  if (E->base.health <= 0x2A30 || (E->toriz_parameter_2 & 0x2000) == 0)
    return jp + 1;
  E->toriz_var_00 = INSTR_ADDR_TO_PTR(k, jp + 1);
  return INSTR_RETURN_ADDR(jp[0]);
}

const uint16 *Torizo_Instr_50(uint16 k, const uint16 *jp) {  // 0xAAD4BA
  if (!(CompareDistToSamus_X(k, 0x70) & 1))
    return jp + 1;
  if ((Torizo_Func_12(k) & 0x8000) != 0)
    return jp + 1;
  Enemy_Torizo *E = Get_Torizo(k);
  if (E->toriz_var_07 <= 0x168 && ((joypad1_lastkeys & 0x300) == 0 || (NextRandom() & 0x101) == 0))
    return jp + 1;
  E->toriz_var_09 = 0;
  Torizo_C20A(k);
  return INSTR_RETURN_ADDR(jp[0]);
}

const uint16 *Torizo_Instr_43(uint16 k, const uint16 *jp) {  // 0xAAD4F3
  SpawnEprojWithGfx(0, k, addr_kEproj_GoldenTorizosChozoOrbs);
  return jp;
}

const uint16 *Torizo_Instr_51(uint16 k, const uint16 *jp) {  // 0xAAD4FD
  Enemy_Torizo *E = Get_Torizo(k);
  if (E->toriz_var_09 < 8 && (CompareDistToSamus_X(k, 0x20) & 1 || (Torizo_Func_12(k) & 0x8000) != 0))
    return jp + 1;
  E->toriz_var_09 = 0;
  Torizo_C22D(k);
  return INSTR_RETURN_ADDR(jp[0]);
}

const uint16 *Torizo_Instr_45(uint16 k, const uint16 *jp) {  // 0xAAD526
  Get_Torizo(k)->toriz_var_00 = INSTR_ADDR_TO_PTR(k, jp + 2);
  if (samus_missiles < 0x20
      || ((nmi_frame_counter_word + (samus_x_pos & 1) + (samus_x_pos >> 1)) & 8) != 0) {
    return INSTR_RETURN_ADDR(jp[0]);
  } else {
    return INSTR_RETURN_ADDR(jp[1]);
  }
}

const uint16 *Torizo_Instr_54(uint16 k, const uint16 *jp) {  // 0xAAD54D
  Enemy_Torizo *E = Get_Torizo(k);
  E->toriz_var_A = g_word_AAD59A[jp[0] >> 1];
  if (Enemy_MoveRight_IgnoreSlopes(k, INT16_SHL16(E->toriz_var_A))) {
    E->toriz_var_03 = 0;
    if ((E->toriz_parameter_1 & 0x8000) != 0)
      return INSTR_RETURN_ADDR(addr_kTorizo_Ilist_D203);
    else
      return INSTR_RETURN_ADDR(addr_kTorizo_Ilist_D2BF);
  } else {
    EnemyFunc_C8AD(k);
    if (((E->toriz_parameter_1 ^ (samus_x_pos - E->base.x_pos)) & 0x8000) == 0) {
      if (!E->toriz_var_03)
        E->toriz_var_03 = 16;
      return jp + 1;
    } else {
      return jp + 1;
    }
  }
}

void Torizo_Func_9(uint16 k) {  // 0xAAD5C2
  Enemy_Torizo *E = Get_Torizo(cur_enemy_index);
  if (samus_y_pos > 0x140 && samus_x_pos > 0x170) {
    ++E->base.current_instruction;
    ++E->base.current_instruction;
    E->base.instruction_timer = 1;
  }
}

void Torizo_Func_10(uint16 k) {  // 0xAAD5DF
  Torizo_C620(k);
  Torizo_C643(k);
}

void Torizo_D5E6(uint16 k) {  // 0xAAD5E6
  Torizo_C620(k);
  Enemy_Torizo *E = Get_Torizo(k);
  CallEnemyPreInstr(E->toriz_var_F | 0xAA0000);
}

void Torizo_D5ED(uint16 k) {  // 0xAAD5ED
  Torizo_C643(k);
}

void Torizo_D5F1(uint16 k) {  // 0xAAD5F1
  uint16 v3;

  Enemy_Torizo *E = Get_Torizo(k);
  uint16 torizo_var_03 = E->toriz_var_03;
  if (!torizo_var_03 || (v3 = torizo_var_03 - 1, (E->toriz_var_03 = v3) != 0)) {
    uint16 v5 = abs16(E->toriz_var_A) + 1;
    if (v5 >= 0x10)
      v5 = 15;
    if (!(Enemy_MoveDown(k, INT16_SHL16(v5)))) {
      uint16 v6;
      if ((E->toriz_parameter_1 & 0x8000) != 0)
        v6 = addr_off_AAC0F2;
      else
        v6 = addr_off_AABC78;
      E->base.current_instruction = v6;
      E->base.instruction_timer = 1;
      E->toriz_var_B = 256;
      E->toriz_var_A = 0;
    }
  } else {
    E->toriz_var_03 = 0;
    uint16 v4;
    if ((E->toriz_parameter_1 & 0x8000) != 0)
      v4 = addr_kTorizo_Ilist_D203;
    else
      v4 = addr_kTorizo_Ilist_D2BF;
    E->base.current_instruction = v4;
    E->base.instruction_timer = 1;
  }
}

void Torizo_D658(void) {  // 0xAAD658
  Enemy_Torizo *E = Get_Torizo(cur_enemy_index);
  if (!E->base.flash_timer && !E->toriz_var_04)
    Torizo_D6A6();
}

void GoldTorizo_Shot(void) {  // 0xAAD667
  Enemy_Torizo *E = Get_Torizo(cur_enemy_index);
  if (!E->base.flash_timer) {
    if (E->toriz_var_04) {
      Torizo_D6A6();
      return;
    }
    if ((E->toriz_parameter_2 & 0x1000) != 0)
      goto LABEL_11;
    uint16 v2, v3;
    v2 = 2 * collision_detection_index;
    v3 = projectile_type[collision_detection_index] & 0xF00;
    E->toriz_var_05 = v3;
    if (v3 == 256) {
      Torizo_D6D1(cur_enemy_index, v2);
      return;
    }
    if (v3 != 512) {
LABEL_11:
      E->toriz_parameter_2 |= 0x2000;
      Torizo_D6A6();
    } else {
      Torizo_D6F7(cur_enemy_index, v2);
    }
  }
}

void Torizo_D6A6(void) {  // 0xAAD6A6
  NormalEnemyShotAiSkipDeathAnim_CurEnemy();
  Enemy_Torizo *E = Get_Torizo(cur_enemy_index);
  if (!E->base.health) {
    E->base.current_instruction = addr_kTorizo_Ilist_B1C8;
    E->base.instruction_timer = 1;
    E->toriz_parameter_2 |= 0xC000;
    E->base.properties |= kEnemyProps_Tangible;
  }
}

void Torizo_D6D1(uint16 k, uint16 j) {  // 0xAAD6D1
  projectile_dir[j >> 1] &= ~0x10;
  Enemy_Torizo *E = Get_Torizo(k);
  E->toriz_var_E = FUNC16(nullsub_270);
  E->base.instruction_timer = 1;
  uint16 v3;
  if ((E->toriz_parameter_1 & 0x8000) != 0)
    v3 = addr_kTorizo_Ilist_D2AD;
  else
    v3 = addr_kTorizo_Ilist_D1F1;
  E->base.current_instruction = v3;
}

void Torizo_D6F7(uint16 k, uint16 j) {  // 0xAAD6F7
  Enemy_Torizo *E = Get_Torizo(cur_enemy_index);

  if ((Torizo_Func_12(k) & 0x8000) == 0) {
    E->toriz_parameter_2 |= 0x1000;
    E->toriz_var_E = FUNC16(nullsub_270);
    projectile_dir[j >> 1] |= 0x10;
    E->base.instruction_timer = 1;
    uint16 v3;
    if ((E->toriz_parameter_1 & 0x2000) != 0) {
      if ((E->toriz_parameter_1 & 0x8000) != 0)
        v3 = addr_kTorizo_Ilist_CEFF;
      else
        v3 = addr_kTorizo_Ilist_CE43;
    } else if ((E->toriz_parameter_1 & 0x8000) != 0) {
      v3 = addr_kTorizo_Ilist_CEA5;
    } else {
      v3 = addr_kTorizo_Ilist_CDE1;
    }
    E->base.current_instruction = v3;
  } else {
    Torizo_D6A6();
  }
}
