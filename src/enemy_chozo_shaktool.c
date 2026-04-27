// Enemy AI - Tourian Entrance Statue, Shaktool, Chozo Statue — peeled from Bank $AA
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "enemy_types.h"


#define g_off_AAD810 ((uint16*)RomFixedPtr(0xaad810))
#define g_word_AAD765 ((uint16*)RomFixedPtr(0xaad765))
#define g_word_AAD785 ((uint16*)RomFixedPtr(0xaad785))
#define g_off_AADF13 ((uint16*)RomFixedPtr(0xaadf13))
#define g_off_AADF21 ((uint16*)RomFixedPtr(0xaadf21))
#define kSine ((uint16*)RomFixedPtr(0xaae0bd))
#define kCosine ((uint16*)RomFixedPtr(0xaae13d))
#define kNegativeSine ((uint16*)RomFixedPtr(0xaae1bd))
#define kNegativeCosine_0 ((uint16*)RomFixedPtr(0xaae23d))
#define kNegativeCosine ((uint16*)RomFixedPtr(0xaae03d))
#define g_off_AADD15 ((uint16*)RomFixedPtr(0xaadd15))
#define g_word_AADE95 ((uint16*)RomFixedPtr(0xaade95))
#define g_word_AADEA3 ((uint16*)RomFixedPtr(0xaadea3))
#define g_word_AADEB1 ((uint16*)RomFixedPtr(0xaadeb1))
#define g_off_AADEDB ((uint16*)RomFixedPtr(0xaadedb))
#define g_word_AADEF7 ((uint16*)RomFixedPtr(0xaadef7))
#define g_word_AADECD ((uint16*)RomFixedPtr(0xaadecd))
#define g_word_AAE630 ((uint16*)RomFixedPtr(0xaae630))
#define g_word_AAE670 ((uint16*)RomFixedPtr(0xaae670))
#define g_word_AAE6B0 ((uint16*)RomFixedPtr(0xaae6b0))
#define kN00bTubeCracks_Palette2 ((uint16*)RomFixedPtr(0xaae2dd))
#define g_off_AAE7A2 ((uint16*)RomFixedPtr(0xaae7a2))
static const uint16 g_word_AADEE9[7] = { 0, 0x20, 0x60, 0xc0, 0x140, 0x1a0, 0x1e0 };
static const uint16 kChozoStatue_Palette[16] = { 0x3800, 0x633f, 0x4a9f, 0x2ddf, 0x6739, 0x4e73, 0x318c, 0x18c6, 0x27ff, 0x1af7, 0xdce, 0xc6, 0x3fff, 0x2b39, 0x7fff, 0 };
static const int16 kChozoStatue_Palettes[16] = {
  0x3800, 0x633f, 0x4a9f, 0x2ddf, 0x4210, 0x318c, 0x2108, 0x1084,
  0x27ff, 0x1af7,  0xdce,   0xc6, 0x3fff, 0x2b39, 0x5294,      0,
};
static const int16 kChozoStatue_Palettes2[16] = {
  0x3800, 0x633f, 0x4a9f, 0x2ddf, 0x2f7c, 0x2295, 0x118d, 0x8e8,
  0x27ff, 0x1af7,  0xdce,   0xc6, 0x3fff, 0x2b39, 0x73df,  0x43,
};
static const int16 kChozoStatue_Palettes3[16] = {
  0x3800, 0x633f, 0x4a9f, 0x2ddf, 0x2295, 0x118d,  0x8e8, 0x85,
  0x27ff, 0x1af7,  0xdce,   0xc6, 0x3fff, 0x2b39, 0x5294,    1,
};

void Enemy_GrappleReact_CancelBeam_AA(void) {  // 0xAA800F
  Enemy_SwitchToFrozenAi();
}

void Enemy_NormalFrozenAI_AA(void) {  // 0xAA8041
  NormalEnemyFrozenAI();
}

const uint16 *Enemy_ClearAiPreInstr_AA(uint16 k, const uint16 *jp) {  // 0xAA8074
  gEnemyData(k)->ai_preinstr = FUNC16(nullsub_171_AA);
  return jp;
}

const uint16 *Enemy_SetAiPreInstr_AA(uint16 k, const uint16 *jp) {  // 0xAA806B
  gEnemyData(k)->ai_preinstr = jp[0];
  return jp + 1;
}

void TourianEntranceStatue_Init(void) {  // 0xAAD7C8
  EnemyData *v0 = gEnemyData(cur_enemy_index);
  v0->palette_index = 0;
  v0->instruction_timer = 1;
  v0->timer = 0;
  uint16 v1 = g_off_AAD810[v0->parameter_1 >> 1];
  v0->current_instruction = v1;
  if (!v0->parameter_1) {
    SpawnEprojWithRoomGfx(addr_kEproj_TourianStatueBaseDecoration, 0);
    SpawnEprojWithRoomGfx(addr_kEproj_TourianStatueRidley, 0);
    SpawnEprojWithRoomGfx(addr_kEproj_TourianStatuePhantoon, 0);
  }
  for (int i = 30; i >= 0; i -= 2) {
    int v5 = i >> 1;
    target_palettes[v5 + 240] = g_word_AAD785[v5];
    target_palettes[v5 + 160] = g_word_AAD765[v5];
  }
}

const uint16 *Shaktool_Instr_2(uint16 k, const uint16 *jp) {  // 0xAAD931
  Enemy_Shaktool *E = Get_Shaktool(k);
  Enemy_Shaktool *EX = Get_Shaktool(E->shakt_var_E + 192);
  return Shaktool_D956(k, jp, EX->shakt_var_D ^ 0x80);
}

const uint16 *Shaktool_Instr_3(uint16 k, const uint16 *jp) {  // 0xAAD93F
  Enemy_Shaktool *E = Get_Shaktool(k);
  Enemy_Shaktool *EX = Get_Shaktool(E->shakt_var_E + 192);
  return Shaktool_D956(k, jp, EX->shakt_var_D);
}

const uint16 *Shaktool_Instr_4(uint16 k, const uint16 *jp) {  // 0xAAD94A
  Enemy_Shaktool *E = Get_Shaktool(k);
  return Shaktool_D956(k, jp, *(uint16 *)((uint8 *)&E->shakt_var_A + 1) ^ 0x80);
}

const uint16 *Shaktool_Instr_5(uint16 k, const uint16 *jp) {  // 0xAAD953
  Enemy_Shaktool *E = Get_Shaktool(k);
  return Shaktool_D956(k, jp, *(uint16 *)((uint8 *)&E->shakt_var_A + 1));
}

const uint16 *Shaktool_D956(uint16 k, const uint16 *jp, uint16 a) {  // 0xAAD956
  Enemy_Shaktool *E = Get_Shaktool(k);
  int v3 = (uint8)a;
  AddToHiLo(&E->base.x_pos, &E->base.x_subpos, INT16_SHL8(kSinCosTable8bit_Sext[v3 + 64]));
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, INT16_SHL8(kSinCosTable8bit_Sext[v3]));
  return jp;
}

const uint16 *Shaktool_Instr_6(uint16 k, const uint16 *jp) {  // 0xAAD99F
  return jp;
}

void Shaktool_Func_2(uint16 k) {  // 0xAAD9A0
  SpawnEprojWithGfx(0, k, addr_kEproj_ShaktoolAttackFrontCircle);
  SpawnEprojWithGfx(0, k, addr_kEproj_ShaktoolAttackMiddleCircle);
  SpawnEprojWithGfx(0, k, addr_kEproj_ShaktoolAttackBackCircle);
}

const uint16 *Shaktool_Instr_1(uint16 k, const uint16 *jp) {  // 0xAAD9BA
  uint16 shakt_var_E = Get_Shaktool(k)->shakt_var_E;
  Get_Shaktool(shakt_var_E)->shakt_var_F = g_off_AADEDB[0];
  Get_Shaktool(shakt_var_E + 64)->shakt_var_F = g_off_AADEDB[1];
  Get_Shaktool(shakt_var_E + 128)->shakt_var_F = g_off_AADEDB[2];
  Get_Shaktool(shakt_var_E + 192)->shakt_var_F = g_off_AADEDB[3];
  Get_Shaktool(shakt_var_E + 256)->shakt_var_F = g_off_AADEDB[4];
  Get_Shaktool(shakt_var_E + 320)->shakt_var_F = g_off_AADEDB[5];
  Get_Shaktool(shakt_var_E + 384)->shakt_var_F = g_off_AADEDB[6];
  return jp;
}

void Shaktool_DAE5(uint16 k) {  // 0xAADAE5
  if ((NextRandom() & 0x8431) == 0) {
    for (int i = 12; i >= 0; i -= 2) {
      Enemy_Shaktool *E = Get_Shaktool(k);
      E->shakt_var_F = FUNC16(nullsub_274);
      E->base.current_instruction = g_off_AADF21[i >> 1];
      E->base.instruction_timer = 1;
      k -= 64;
    }
  }
}

void Shaktool_DB0E(uint16 k, uint16 a) {  // 0xAADB0E
  uint16 shakt_var_E = Get_Shaktool(k)->shakt_var_E;
  Get_Shaktool(shakt_var_E)->shakt_parameter_1 = a;
  Get_Shaktool(shakt_var_E + 64)->shakt_parameter_1 = a;
  Get_Shaktool(shakt_var_E + 128)->shakt_parameter_1 = a;
  Get_Shaktool(shakt_var_E + 192)->shakt_parameter_1 = a;
  Get_Shaktool(shakt_var_E + 256)->shakt_parameter_1 = a;
  Get_Shaktool(shakt_var_E + 320)->shakt_parameter_1 = a;
  Get_Shaktool(shakt_var_E + 384)->shakt_parameter_1 = a;
}

void Shaktool_DB27(uint16 k, uint16 a) {  // 0xAADB27
  uint16 shakto_var_E = Get_Shaktool(k)->shakt_var_E;
  Get_Shaktool(shakto_var_E)->shakt_var_A = a;
  Get_Shaktool(shakto_var_E + 64)->shakt_var_A = a;
  Get_Shaktool(shakto_var_E + 128)->shakt_var_A = a;
  Get_Shaktool(shakto_var_E + 192)->shakt_var_A = a;
  Get_Shaktool(shakto_var_E + 256)->shakt_var_A = a;
  Get_Shaktool(shakto_var_E + 320)->shakt_var_A = a;
  Get_Shaktool(shakto_var_E + 384)->shakt_var_A = a;
}

void Shaktool_DB40(uint16 k, uint16 a) {  // 0xAADB40
  uint16 shakto_var_E = Get_Shaktool(k)->shakt_var_E;
  Get_Shaktool(shakto_var_E)->shakt_var_B = a;
  Get_Shaktool(shakto_var_E + 64)->shakt_var_B = a;
  Get_Shaktool(shakto_var_E + 128)->shakt_var_B = a;
  Get_Shaktool(shakto_var_E + 192)->shakt_var_B = a;
  Get_Shaktool(shakto_var_E + 256)->shakt_var_B = a;
  Get_Shaktool(shakto_var_E + 320)->shakt_var_B = a;
  Get_Shaktool(shakto_var_E + 384)->shakt_var_B = a;
}

void Shaktool_DB59(uint16 k) {  // 0xAADB59
  uint16 shakto_var_E;
  uint16 shakto_var_B;
  uint16 v7;
  uint16 v10;
  uint16 x_subpos;
  uint16 y_subpos;
  uint16 x_pos;
  uint16 y_pos;

  shakto_var_E = Get_Shaktool(k)->shakt_var_E;
  Enemy_Shaktool *Shaktool = Get_Shaktool(shakto_var_E + 192);
  Shaktool->shakt_var_D ^= 0x8000;
  Enemy_Shaktool *v3 = Get_Shaktool(shakto_var_E + 384);
  shakto_var_B = v3->shakt_var_B;
  Enemy_Shaktool *v5 = Get_Shaktool(shakto_var_E + 64);
  v3->shakt_var_B = (v5->shakt_var_B ^ 0x8000) & 0xFF00;
  v5->shakt_var_B = (shakto_var_B ^ 0x8000) & 0xFF00;
  Enemy_Shaktool *v6 = Get_Shaktool(shakto_var_E + 320);
  v7 = v6->shakt_var_B;
  Enemy_Shaktool *v8 = Get_Shaktool(shakto_var_E + 128);
  v6->shakt_var_B = (v8->shakt_var_B ^ 0x8000) & 0xFF00;
  v8->shakt_var_B = (v7 ^ 0x8000) & 0xFF00;
  Enemy_Shaktool *v9 = Get_Shaktool(shakto_var_E + 256);
  v10 = v9->shakt_var_B;
  Enemy_Shaktool *v11 = Get_Shaktool(shakto_var_E + 192);
  v9->shakt_var_B = (v11->shakt_var_B ^ 0x8000) & 0xFF00;
  v11->shakt_var_B = (v10 ^ 0x8000) & 0xFF00;
  Enemy_Shaktool *v12 = Get_Shaktool(shakto_var_E + 384);
  x_subpos = v12->base.x_subpos;
  Enemy_Shaktool *v14 = Get_Shaktool(shakto_var_E);
  v12->base.x_subpos = v14->base.x_subpos;
  v14->base.x_subpos = x_subpos;
  y_subpos = v12->base.y_subpos;
  v12->base.y_subpos = v14->base.y_subpos;
  v14->base.y_subpos = y_subpos;
  x_pos = v12->base.x_pos;
  v12->base.x_pos = v14->base.x_pos;
  v14->base.x_pos = x_pos;
  y_pos = v12->base.y_pos;
  v12->base.y_pos = v14->base.y_pos;
  v14->base.y_pos = y_pos;
  Get_Shaktool(shakto_var_E + 64)->base.x_subpos = 0x8000;
  Get_Shaktool(shakto_var_E + 128)->base.x_subpos = 0x8000;
  Get_Shaktool(shakto_var_E + 192)->base.x_subpos = 0x8000;
  Get_Shaktool(shakto_var_E + 256)->base.x_subpos = 0x8000;
  Get_Shaktool(shakto_var_E + 320)->base.x_subpos = 0x8000;
  Get_Shaktool(shakto_var_E + 64)->base.y_subpos = 0x8000;
  Get_Shaktool(shakto_var_E + 128)->base.y_subpos = 0x8000;
  Get_Shaktool(shakto_var_E + 192)->base.y_subpos = 0x8000;
  Get_Shaktool(shakto_var_E + 256)->base.y_subpos = 0x8000;
  Get_Shaktool(shakto_var_E + 320)->base.y_subpos = 0x8000;
}

void Shaktool_DC07(uint16 k) {  // 0xAADC07
  Enemy_Shaktool *E = Get_Shaktool(k);
  uint16 v2;
  if ((E->shakt_parameter_1 & 0x8000) != 0)
    v2 = E->shakt_var_B - E->shakt_var_A;
  else
    v2 = E->shakt_var_A - E->shakt_var_B;
  E->shakt_var_C = 4 * HIBYTE(v2);
}

void Shaktool_DC2A(uint16 k) {  // 0xAADC2A
  Enemy_Shaktool *E = Get_Shaktool(k);
  int v3 = HIBYTE(E->shakt_var_B);
  int v5 = k >> 1;
  SetHiLo(&E->base.x_pos, &E->base.x_subpos, INT16_SHL8(kSine[v3]) +
      __PAIR32__(enemy_drawing_queue[v5 + 91], enemy_drawing_queue[v5 + 92]));
  SetHiLo(&E->base.y_pos, &E->base.y_subpos, INT16_SHL8(kNegativeCosine[v3]) +
      __PAIR32__(enemy_drawing_queue[v5 + 93], enemy_drawing_queue[v5 + 94]));
}

void Shaktool_DC6F(uint16 k) {  // 0xAADC6F
  Enemy_Shaktool *E = Get_Shaktool(k);
  Shaktool_DB40(k, E->shakt_var_A);
  uint16 shakto_var_E = E->shakt_var_E;
  Get_Shaktool(shakto_var_E)->shakt_var_C = g_word_AADEE9[0];
  Get_Shaktool(shakto_var_E + 64)->shakt_var_C = g_word_AADEE9[1];
  Get_Shaktool(shakto_var_E + 128)->shakt_var_C = g_word_AADEE9[2];
  Get_Shaktool(shakto_var_E + 192)->shakt_var_C = g_word_AADEE9[3];
  Get_Shaktool(shakto_var_E + 256)->shakt_var_C = g_word_AADEE9[4];
  Get_Shaktool(shakto_var_E + 320)->shakt_var_C = g_word_AADEE9[5];
  Get_Shaktool(shakto_var_E + 384)->shakt_var_C = g_word_AADEE9[6];
}

void Shaktool_Hurt(void) {  // 0xAADCA3
  Enemy_Shaktool *E = Get_Shaktool(cur_enemy_index);
  CallEnemyPreInstr(E->shakt_var_F | 0xAA0000);
}

void Shaktool_DCAC(uint16 k) {  // 0xAADCAC
  Shaktool_DC2A(k);
  Enemy_Shaktool *E = Get_Shaktool(k);
  uint16 shakto_var_C;
  if ((E->shakt_parameter_1 & 0x4000) != 0) {
    E->shakt_var_A += 256;
    shakto_var_C = 256;
  } else {
    shakto_var_C = E->shakt_var_C;
  }
  E->shakt_var_B += sign16(E->shakt_parameter_1) ? -shakto_var_C : shakto_var_C;
}

void Shaktool_DCD7(uint16 k) {  // 0xAADCD7
  Shaktool_DCAC(k);
  Enemy_Shaktool *E = Get_Shaktool(k);
  uint16 r18 = E->shakt_var_B ^ 0x8000;
  int16 v2 = r18 + ((uint16)(Get_Shaktool(k + 64)->shakt_var_B - r18) >> 1);
  if ((E->shakt_var_D & 0x8000) != 0)
    HIBYTE(v2) ^= 0x80;
  uint16 v3 = (HIBYTE(v2) + 8) & 0xE0;
  LOBYTE(E->shakt_var_D) = v3;
  E->base.current_instruction = g_off_AADD15[(uint16)(v3 >> 4) >> 1];
  E->base.instruction_timer = 1;
}

void Shaktool_DD25(uint16 k) {  // 0xAADD25
  int16 v7;

  Enemy_Shaktool *E = Get_Shaktool(k);
  uint16 y_pos = E->base.y_pos;
  uint16 x_pos = E->base.x_pos;
  Shaktool_DCAC(k);
  uint16 v12 = E->base.y_pos;
  uint16 v11 = E->base.x_pos;
  E->base.x_pos = x_pos;
  E->base.y_pos = y_pos;
  if (Enemy_MoveRight_IgnoreSlopes(k, INT16_SHL16(v11 - x_pos))
      || (E->base.y_pos = y_pos, Enemy_MoveDown(k, INT16_SHL16(v12 - y_pos)))) {
    if ((E->shakt_parameter_1 & 0x2000) != 0) {
      Shaktool_DB0E(k, (E->shakt_parameter_1 ^ 0x8000) & 0x8FFF);
    } else {
      E->base.x_pos = x_pos;
      E->base.y_pos = y_pos;
      Shaktool_DB59(k);
      Shaktool_DB0E(cur_enemy_index, E->shakt_parameter_1 | 0x2000);
      Shaktool_DB0E(cur_enemy_index, E->shakt_parameter_1 & 0xBFFF);
    }
    E->shakt_var_D = 0;
    uint16 v5 = CalculateAngleOfEnemyXfromEnemyY(k, E->shakt_var_E);
    v7 = v5 << 8;
    uint16 v8;
    if ((E->shakt_parameter_1 & 0x8000) != 0)
      v8 = v7 - 0x4000;
    else
      v8 = v7 + 0x4000;
    Shaktool_DB27(k, v8);
    for (int i = 12; i >= 0; i -= 2) {
      Shaktool_DC07(k);
      Enemy_Shaktool *EK = Get_Shaktool(k);
      EK->shakt_var_F = FUNC16(nullsub_274);
      EK->base.current_instruction = g_off_AADF13[i >> 1];
      EK->base.instruction_timer = 1;
      k -= 64;
    }
  } else {
    E->base.x_pos = v11;
    E->base.y_pos = v12;
    if ((E->shakt_parameter_1 & 0x4000) != 0) {
      E->shakt_var_A += 256;
    } else {
      if (((E->shakt_var_A ^ E->shakt_var_B) & 0xFF00) == 0) {
        Shaktool_DC6F(k);
        E->shakt_var_D = 30720;
        Shaktool_DB0E(k, E->shakt_parameter_1 & 0xDFFF);
        E->shakt_var_D = HIBYTE(E->shakt_var_D) << 8;
      }
      uint16 v2 = E->shakt_var_D + E->shakt_var_C;
      E->shakt_var_D = v2;
      if (v2 >= 0xF000)
        Shaktool_DB0E(k, E->shakt_parameter_1 | 0x4000);
    }
  }
}

void Shaktool_Init(void) {  // 0xAADE43
  Enemy_Shaktool *E = Get_Shaktool(cur_enemy_index);
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->shakt_var_A = 0;
  E->shakt_var_D = 0;
  uint16 shakto_parameter_2 = E->shakt_parameter_2;
  int v3 = shakto_parameter_2 >> 1;
  E->base.properties |= g_word_AADE95[v3];
  E->shakt_var_E = cur_enemy_index - g_word_AADEA3[v3];
  E->shakt_var_F = *(uint16 *)((uint8 *)g_off_AADEDB + shakto_parameter_2);
  E->shakt_var_C = *(uint16 *)((uint8 *)g_word_AADEE9 + shakto_parameter_2) - g_word_AADEF7[v3];
  E->shakt_var_B = g_word_AADEB1[v3];
  E->base.current_instruction = g_word_AADEB1[v3 + 7];
  E->base.layer = g_word_AADECD[v3];
  if (shakto_parameter_2)
    Shaktool_DC2A(cur_enemy_index);
}

void Shaktool_Touch(void) {  // 0xAADF2F
  NormalEnemyTouchAi();
}

void Shaktool_Shot(void) {  // 0xAADF34
  NormalEnemyShotAi();
  Enemy_Shaktool *E = Get_Shaktool(cur_enemy_index);
  if (!E->base.health) {
    uint16 shakto_var_E = E->shakt_var_E;
    Get_Shaktool(shakto_var_E)->base.properties = 512;
    Get_Shaktool(shakto_var_E + 64)->base.properties = 512;
    Get_Shaktool(shakto_var_E + 128)->base.properties = 512;
    Get_Shaktool(shakto_var_E + 192)->base.properties = 512;
    Get_Shaktool(shakto_var_E + 256)->base.properties = 512;
    Get_Shaktool(shakto_var_E + 320)->base.properties = 512;
    Get_Shaktool(shakto_var_E + 384)->base.properties = 512;
  }
}

const uint16 *Shaktool_Instr_9(uint16 k, const uint16 *jp) {  // 0xAAE429
  fx_timer = 32;
  fx_y_vel = 64;
  return jp;
}

const uint16 *Shaktool_Instr_11(uint16 k, const uint16 *jp) {  // 0xAAE436
  fx_base_y_pos = 722;
  return jp;
}

const uint16 *Shaktool_Instr_10(uint16 k, const uint16 *jp) {  // 0xAAE43D
  CallSomeSamusCode(1);
  return jp;
}

void sub_AAE445(uint16 k) {  // 0xAAE445
  EnemyData *v1 = gEnemyData(k);
  if (v1->parameter_1) {
    v1->current_instruction = addr_kShaktool_Ilist_E3A7;
    v1->instruction_timer = 1;
  }
}

const uint16 *Shaktool_Instr_8(uint16 k, const uint16 *jp) {  // 0xAAE57F
  QueueSfx2_Max6(0x1C);
  return jp;
}

const uint16 *Shaktool_Instr_13(uint16 k, const uint16 *jp) {  // 0xAAE587
  QueueSfx2_Max6(0x4B);
  return jp;
}

const uint16 *Shaktool_Instr_12(uint16 k, const uint16 *jp) {  // 0xAAE58F
  Enemy_Shaktool *e = Get_Shaktool(k);
  uint16 arg = jp[0];

  CalculateBlockContainingPixelPos(e->base.x_pos + arg, e->base.y_pos + 28);
  if ((level_data[room_width_in_blocks + cur_block_index] & 0xF000) == 0xA000) {
    SpawnPLM(addr_kPlmHeader_D113);
    SpawnEprojWithRoomGfx(addr_kEproj_WreckedShipChozoSpikeFootsteps_1, arg);
  }
  return jp + 1;
}

const uint16 *Shaktool_Instr_7(uint16 k, const uint16 *jp) {  // 0xAAE5D8
  Enemy_Shaktool *E = Get_Shaktool(k);
  E->shakt_var_C = jp[0];
  Enemy_MoveRight_IgnoreSlopes(k, INT16_SHL8(g_word_AAE630[E->shakt_var_C >> 1]));
  Enemy_MoveDown(k, INT16_SHL8(abs16(g_word_AAE630[E->shakt_var_C >> 1])));
  EnemyFunc_C8AD(k);
  int v6 = E->shakt_var_C >> 1;
  samus_x_pos = g_word_AAE670[v6] + E->base.x_pos;
  samus_y_pos = g_word_AAE6B0[v6] + E->base.y_pos;
  return jp + 1;
}

const uint16 *Shaktool_Instr_14(uint16 k, const uint16 *jp) {  // 0xAAE6F0
  CallSomeSamusCode(1);
  *(uint16 *)&scrolls[6] = 0;
  *(uint16 *)&scrolls[8] = 0;
  *(uint16 *)&scrolls[9] = 0;
  *(uint16 *)&scrolls[13] = 1;
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x17, 0x1d, 0xd6fc });
  return jp;
}

void N00bTubeCracks_Init(void) {  // 0xAAE716
  for (int i = 62; i >= 0; i -= 2)
    target_palettes[(i >> 1) + 144] = kN00bTubeCracks_Palette2[i >> 1];
}

void ChozoStatue_Init(void) {  // 0xAAE725
  EnemyData *v0 = gEnemyData(cur_enemy_index);
  v0->properties |= kEnemyProps_DisableSamusColl | kEnemyProps_ProcessedOffscreen | 0x8000;
  v0->spritemap_pointer = addr_kSpritemap_Nothing_AA;
  v0->instruction_timer = 1;
  v0->timer = 0;
  v0->ai_preinstr = FUNC16(nullsub_276);
  v0->parameter_1 = 0;
  v0->palette_index = 0;
  gEnemyData(0)->layer = 0;
  uint16 parameter_2 = v0->parameter_2;
  v0->current_instruction = g_off_AAE7A2[parameter_2 >> 1];
  if (parameter_2) {
    sub_AAE784();
  } else {
    for (int i = 30; i >= 0; i -= 2) {
      int v3 = i >> 1;
      target_palettes[v3 + 160] = kChozoStatue_Palettes[v3];
      target_palettes[v3 + 144] = kChozoStatue_Palette[v3];
    }
    SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x4a, 0x17, 0xd6ee });
    SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x17, 0x1d, 0xd6fc });
  }
}

void sub_AAE784(void) {  // 0xAAE784
  for (int i = 30; i >= 0; i -= 2) {
    int v1 = i >> 1;
    target_palettes[v1 + 160] = kChozoStatue_Palettes3[v1];
    target_palettes[v1 + 144] = kChozoStatue_Palettes2[v1];
  }
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x0c, 0x1d, 0xd6d6 });
}

void ChozoStatue_Main(void) {  // 0xAAE7A7
  EnemyData *v0 = gEnemyData(cur_enemy_index);
  CallEnemyPreInstr(v0->ai_preinstr | 0xAA0000);
}

void Shaktool_PreInstr_0(uint16 k) {  // 0xAAE7AE
  if ((*(uint16 *)&boss_bits_for_area[area_index] & 1) != 0) {
    Enemy_Shaktool *E = Get_Shaktool(k);
    if (E->shakt_parameter_1) {
      E->base.current_instruction = addr_kShaktool_Ilist_E461;
      E->base.instruction_timer = 1;
      E->shakt_var_A = -256;
      E->shakt_var_B = 256;
    }
  }
}
