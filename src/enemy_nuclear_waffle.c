// Enemy AI - Nuclear waffle — peeled from Bank $A6
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"

static PairU16 NuclearWaffle_Func_6(uint16 a);

static const uint16 g_word_A695F6[4] = { 0x190, 0xf0, 0xf0, 0x190 };

static const uint16 g_word_A695FE[4] = { 0xffe8, 0xfff4, 0x18, 0xc };

static const uint16 g_word_A69606[4] = { 0x180, 0x100, 0x100, 0x180 };

void NuclearWaffle_Init(void) {  // 0xA694C4
  Enemy_NuclearWaffle *E = Get_NuclearWaffle(cur_enemy_index);
  E->base.current_instruction = addr_kNuclearWaffle_Ilist_9490;
  E->nwe_var_C = LOBYTE(E->nwe_parameter_1);
  E->nwe_var_D = HIBYTE(E->nwe_parameter_1);
  E->nwe_var_E = LOBYTE(E->nwe_parameter_2);
  uint16 nwe_parameter_2_high = HIBYTE(E->nwe_parameter_2);
  E->nwe_var_F = nwe_parameter_2_high;
  E->nwe_var_B = nwe_parameter_2_high;
  E->nwe_var_A = FUNC16(NuclearWaffle_Func_1);
  int v2 = (uint16)(4 * E->nwe_var_E) >> 1;
  uint16 v3 = g_word_A695F6[v2];
  E->nwe_var_23 = v3;
  E->nwe_var_21 = v3;
  E->nwe_var_2E = g_word_A695F6[v2 + 1];
  E->nwe_var_2B = g_word_A695FE[v2];
  E->nwe_var_2C = g_word_A695FE[v2 + 1];
  E->nwe_var_30 = g_word_A69606[v2];
  E->nwe_var_2F = g_word_A69606[v2 + 1];
  uint16 v4 = 8 * E->nwe_var_C;
  if (!E->nwe_var_E)
    v4 += 4;
  int v5 = v4 >> 1;
  E->nwe_var_25 = kCommonEnemySpeeds_Linear[v5];
  E->nwe_var_24 = kCommonEnemySpeeds_Linear[v5 + 1];
  E->nwe_var_26 = E->base.x_pos;
  E->nwe_var_27 = E->base.y_pos;
  uint16 v6 = E->nwe_var_26 + CosineMult8bit(E->nwe_var_23, E->nwe_var_D);
  E->nwe_var_28 = v6;
  E->base.x_pos = v6;
  uint16 v7 = E->nwe_var_27 + SineMult8bit(E->nwe_var_23, E->nwe_var_D);
  E->nwe_var_29 = v7;
  E->base.y_pos = v7;
  uint16 v8 = 8;
  E->nwe_var_2A = 8;
  do {
    SpawnEprojWithGfx(v8, cur_enemy_index, addr_kEproj_NuclearWaffleBody);
    v8 = E->nwe_var_2A - 2;
    E->nwe_var_2A = v8;
  } while (v8);
  E->nwe_var_2A = 6;
  uint16 v15;
  do {
    uint16 v13 = E->base.vram_tiles_index | E->base.palette_index;
    E->nwe_var_34 = v13;
    uint16 r18 = CreateSpriteAtPos(E->base.x_pos, E->base.y_pos, 43, v13);
    Get_NuclearWaffle(cur_enemy_index + E->nwe_var_2A)->nwe_var_10 = r18;
    v15 = E->nwe_var_2A - 2;
    E->nwe_var_2A = v15;
  } while (v15);
}

void CallNuclearWaffleFunc(uint32 ea) {
  switch (ea) {
  case fnNuclearWaffle_Func_1: NuclearWaffle_Func_1(); return;
  case fnNuclearWaffle_Func_2: NuclearWaffle_Func_2(); return;
  default: Unreachable();
  }
}

void NuclearWaffle_Main(void) {  // 0xA6960E
  Enemy_NuclearWaffle *E = Get_NuclearWaffle(cur_enemy_index);
  CallNuclearWaffleFunc(E->nwe_var_A | 0xA60000);
}

void NuclearWaffle_Func_1(void) {  // 0xA69615
  Enemy_NuclearWaffle *E = Get_NuclearWaffle(cur_enemy_index);
  if ((--E->nwe_var_B & 0x8000) != 0) {
    E->nwe_var_B = E->nwe_var_F;
    E->nwe_var_21 = E->nwe_var_23;
    E->nwe_var_A = FUNC16(NuclearWaffle_Func_2);
    E->nwe_var_33 = 0;
    E->nwe_var_08 = 0;
    E->nwe_var_09 = 0;
    E->nwe_var_0A = 0;
    E->nwe_var_0B = 0;
    E->nwe_var_0C = 0;
    E->nwe_var_0D = 0;
    E->nwe_var_0E = 0;
    E->nwe_var_0F = 0;
    E->nwe_var_18 = 0;
    E->nwe_var_19 = 0;
    E->nwe_var_1A = 0;
    E->nwe_var_1B = 0;
    E->nwe_var_1C = 0;
    E->nwe_var_1D = 0;
    E->nwe_var_1E = 0;
    E->nwe_var_1F = 0;
    E->base.properties |= kEnemyProps_ProcessedOffscreen;
  }
}

void NuclearWaffle_Func_2(void) {  // 0xA69682
  Enemy_NuclearWaffle *E = Get_NuclearWaffle(cur_enemy_index);
  uint16 varE32 = E->nwe_var_D;
  PairU16 pair = NuclearWaffle_Func_6(E->nwe_var_21);
  uint16 v8 = pair.j;
  if (pair.j != E->nwe_var_33) {
    CreateSpriteAtPos(E->base.x_pos, E->base.y_pos, 46, E->nwe_var_34);
    CreateSpriteAtPos(E->base.x_pos, E->base.y_pos, pair.k + 44, E->nwe_var_34);
    NuclearWaffle_Func_7(pair.j);
  }
  E->nwe_var_33 = v8;
  uint16 tmp;
  uint16 v4 = NuclearWaffle_Func_5(E->nwe_var_21, &tmp);
  E->base.x_pos = E->nwe_var_26 + CosineMult8bit(v4, E->nwe_var_D);
  uint16 v5 = NuclearWaffle_Func_5(E->nwe_var_21, &tmp);
  E->base.y_pos = E->nwe_var_27 + SineMult8bit(v5, E->nwe_var_D);
  NuclearWaffle_Func_3(varE32);
  NuclearWaffle_Func_4(varE32);
  AddToHiLo(&E->nwe_var_21, &E->nwe_var_20, __PAIR32__(E->nwe_var_25, E->nwe_var_24));
}

void NuclearWaffle_Func_3(uint16 varE32) {  // 0xA69721
  Enemy_NuclearWaffle *E = Get_NuclearWaffle(cur_enemy_index);
  uint16 v12;
  E->nwe_var_2A = 8;
  uint16 R36 = E->nwe_var_2C + E->nwe_var_21;
  uint16 r28;
  do {
    R36 -= E->nwe_var_2B;
    Enemy_NuclearWaffle *ET = Get_NuclearWaffle(cur_enemy_index + E->nwe_var_2A);
    uint16 nwe_var_00 = ET->nwe_var_00;
    PairU16 pair = NuclearWaffle_Func_6(R36);
    uint16 v13 = pair.j;
    if (pair.j != ET->nwe_var_08) {
      int v5 = nwe_var_00 >> 1;
      CreateSpriteAtPos(eproj_x_pos[v5], eproj_y_pos[v5], 46, E->nwe_var_34);
      CreateSpriteAtPos(eproj_x_pos[v5], eproj_y_pos[v5], pair.k + 44, E->nwe_var_34);
      NuclearWaffle_Func_7(pair.j);
    }
    ET->nwe_var_08 = v13;
    uint16 v7 = NuclearWaffle_Func_5(R36, &r28);
    int v10 = nwe_var_00 >> 1;
    eproj_x_pos[v10] = E->nwe_var_26 + CosineMult8bit(v7, varE32);
    uint16 v11 = NuclearWaffle_Func_5(R36, &r28);
    eproj_y_pos[v10] = E->nwe_var_27 + SineMult8bit(v11, varE32);
    v12 = E->nwe_var_2A - 2;
    E->nwe_var_2A = v12;
  } while (v12);
  if (r28) {
    E->nwe_var_A = FUNC16(NuclearWaffle_Func_1);
    E->base.properties &= ~kEnemyProps_ProcessedOffscreen;
  }
}

void NuclearWaffle_Func_4(uint16 varE32) {  // 0xA697E9
  Enemy_NuclearWaffle *E = Get_NuclearWaffle(cur_enemy_index);
  E->nwe_var_2A = 6;
  uint16 R36 = E->nwe_var_21;
  uint16 v12;
  uint16 r28 = 0;
  do {
    R36 -= E->nwe_var_2B;
    Enemy_NuclearWaffle *ET = Get_NuclearWaffle(cur_enemy_index + E->nwe_var_2A);
    uint16 nwe_var_10 = ET->nwe_var_10;
    PairU16 pair = NuclearWaffle_Func_6(R36);
    uint16 v13 = pair.j;
    if (pair.j != ET->nwe_var_18) {
      uint16 r38 = E->nwe_var_34;
      int v5 = nwe_var_10 >> 1;
      CreateSpriteAtPos(sprite_x_pos[v5], sprite_y_pos[v5], 46, r38);
      CreateSpriteAtPos(sprite_x_pos[v5], sprite_y_pos[v5], pair.k + 44, r38);
      NuclearWaffle_Func_7(pair.j);
    }
    ET->nwe_var_18 = v13;
    uint16 v6 = NuclearWaffle_Func_5(R36, &r28);
    uint16 v7 = CosineMult8bit(v6, varE32);
    uint16 R32 = E->nwe_var_26 + v7;
    uint16 v9 = NuclearWaffle_Func_5(R36, &r28);
    uint16 R34 = E->nwe_var_27 + SineMult8bit(v9, varE32);
    sprite_x_pos[nwe_var_10 >> 1] = R32;
    sprite_y_pos[nwe_var_10 >> 1] = R34;
    v12 = E->nwe_var_2A - 2;
    E->nwe_var_2A = v12;
  } while (v12);
}

uint16 NuclearWaffle_Func_5(uint16 a, uint16 *r28_out) {  // 0xA698AD
  *r28_out = 0;
  Enemy_NuclearWaffle *E = Get_NuclearWaffle(cur_enemy_index);
  if (!E->nwe_var_E) {
    if ((int16)(a - E->nwe_var_2E) >= 0) {
      if ((int16)(a - E->nwe_var_23) < 0)
        return a;
      return E->nwe_var_23;
    }
LABEL_8:
    ++(*r28_out);
    return E->nwe_var_2E;
  }
  if ((int16)(a - E->nwe_var_2E) >= 0)
    goto LABEL_8;
  if ((int16)(a - E->nwe_var_23) < 0)
    return E->nwe_var_23;
  return a;
}

static PairU16 NuclearWaffle_Func_6(uint16 a) {  // 0xA698E7
  Enemy_NuclearWaffle *E = Get_NuclearWaffle(cur_enemy_index);
  if (E->nwe_var_E) {
    if ((int16)(a - E->nwe_var_2F) >= 0) {
      return (PairU16) { 1, 2 };
    } else if ((int16)(a - E->nwe_var_30) >= 0) {
      return (PairU16) { 0, 1 };
    } else {
      return (PairU16) { 0, 0 };
    }
  } else if ((int16)(a - E->nwe_var_2F) < 0) {
    return (PairU16) { 0, 2 };
  } else if ((int16)(a - E->nwe_var_30) < 0) {
    return (PairU16) { 1, 1 };
  } else {
    return (PairU16) { 0, 0 };
  }
}

void NuclearWaffle_Func_7(uint16 r30) {  // 0xA6993F
  if (r30 != 2)
    QueueSfx2_Max6(0x5E);
}
