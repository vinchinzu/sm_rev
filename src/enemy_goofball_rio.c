// BouncingGoofball / Rio / NorfairRio / LowerNorfairRio extracted from sm_a2.c.
#include "ida_types.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "variables.h"
#include "enemy_ai_canon.h"

#define kEnemyInit_BouncingGoofball_Tab0 ((uint16*)RomFixedPtr(0xa286df))
#define kEnemyInit_BouncingGoofball_Tab1 ((uint16*)RomFixedPtr(0xa286ef))
#define kBouncingGoofball_Tab0 ((uint8*)RomFixedPtr(0xa28701))
#define g_word_A2BBBB (*(uint16*)RomFixedPtr(0xa2bbbb))
#define g_word_A2BBBF (*(uint16*)RomFixedPtr(0xa2bbbf))
#define g_word_A2C1C1 ((uint16*)RomFixedPtr(0xa2c1c1))
#define g_word_A2C1C5 (*(uint16*)RomFixedPtr(0xa2c1c5))
#define g_word_A2C6CA (*(uint16*)RomFixedPtr(0xa2c6ca))
#define g_word_A2C6CE (*(uint16*)RomFixedPtr(0xa2c6ce))


void BouncingGoofball_Init(void) {  // 0xA2871C
  Enemy_BouncingGoofball *E = Get_BouncingGoofball(cur_enemy_index);
  E->base.spritemap_pointer = addr_kSpritemap_Nothing_A2;
  BouncingGoofball_Func2();
  E->bgl_var_01 = 1;
  E->bgl_var_A = kEnemyInit_BouncingGoofball_Tab0[(uint16)(2
                                                           * LOBYTE(E->bgl_parameter_1)) >> 1];
  E->bgl_var_C = kEnemyInit_BouncingGoofball_Tab1[(uint16)(2
                                                           * HIBYTE(E->bgl_parameter_1)) >> 1];
  E->bgl_var_D = 0;
  E->bgl_var_E = 0;
  E->bgl_var_F = 0;
}

void BouncingGoofball_Func1(uint16 k) {  // 0xA28755
  Enemy_BouncingGoofball *E = Get_BouncingGoofball(k);
  uint16 v5;
  do {
    uint16 bgl_var_E = E->bgl_var_E;
    uint8 v3 = kBouncingGoofball_Tab0[bgl_var_E];
    if ((int16)(bgl_var_E - 23) >= 0)
      v3 = -1;
    uint16 prod = Mult8x8(v3, E->bgl_var_A);
    uint16 RegWord = prod;
    E->bgl_var_B = RegWord;
    v5 = E->bgl_var_D + RegWord;
    E->bgl_var_D = v5;
    ++E->bgl_var_E;
  } while ((int16)(v5 - E->bgl_var_C) < 0);
  E->bgl_var_00 = E->bgl_var_E;
  E->bgl_var_02 = 1;
  E->bgl_var_04 = 1;
}

static Func_V *const off_A28718[2] = { BouncingGoofball_State0, BouncingGoofball_State1 };

void BouncingGoofball_Main(void) {  // 0xA2879C
  Enemy_BouncingGoofball *E = Get_BouncingGoofball(cur_enemy_index);
  if (!E->bgl_var_05) {
    BouncingGoofball_Func1(cur_enemy_index);
    E->bgl_var_05 = 1;
    return;
  }
  E->bgl_var_03 = E->bgl_var_A;
  if (BouncingGoofball_SamusCloseX()) {
    E->bgl_var_02 = 0;
    E->bgl_var_04 = 0;
    if (!E->bgl_var_F) {
      E->bgl_var_F = 1;
      BouncingGoofball_Func3();
    }
    goto LABEL_9;
  }
  if (!E->bgl_var_02) {
LABEL_9:
    E->bgl_var_A = E->bgl_var_03;
    off_A28718[E->bgl_var_01]();
    return;
  }
  if (!E->bgl_var_04) {
    E->bgl_var_04 = 1;
    BouncingGoofball_Func2();
  }
}

void BouncingGoofball_State0(void) {  // 0xA28801
  Enemy_BouncingGoofball *E = Get_BouncingGoofball(cur_enemy_index);
  uint16 v1 = ++E->bgl_var_E;
  uint8 v2 = kBouncingGoofball_Tab0[v1];
  if ((int16)(v1 - 23) >= 0)
    v2 = -1;
  uint16 RegWord = Mult8x8(v2, E->bgl_var_A);
  E->bgl_var_B = RegWord;
  E->base.y_pos += HIBYTE(RegWord);
  if ((int16)(E->bgl_var_E - E->bgl_var_00) >= 0) {
    E->bgl_var_01 = 1;
    E->bgl_var_02 = 1;
    E->bgl_var_F = 0;
  }
}

void BouncingGoofball_State1(void) {  // 0xA28850
  Enemy_BouncingGoofball *E = Get_BouncingGoofball(cur_enemy_index);
  uint16 bgl_var_E = E->bgl_var_E;
  uint8 v2 = kBouncingGoofball_Tab0[bgl_var_E];
  if ((int16)(bgl_var_E - 23) >= 0)
    v2 = -1;
  uint16 RegWord = Mult8x8(v2, E->bgl_var_A);
  E->bgl_var_B = RegWord;
  E->base.y_pos -= HIBYTE(RegWord);
  if ((--E->bgl_var_E & 0x8000) != 0)
    E->bgl_var_01 = 0;
}

uint16 BouncingGoofball_SamusCloseX(void) {  // 0xA28894
  Enemy_BouncingGoofball *E = Get_BouncingGoofball(cur_enemy_index);
  return IsSamusWithinEnemy_X(cur_enemy_index, E->bgl_parameter_2);
}

void BouncingGoofball_Func2(void) {  // 0xA2889F
  Enemy_BouncingGoofball *E = Get_BouncingGoofball(cur_enemy_index);
  E->base.current_instruction = addr_kBouncingGoofball_Ilist_86A7;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
}

void BouncingGoofball_Func3(void) {  // 0xA288B2
  Enemy_BouncingGoofball *E = Get_BouncingGoofball(cur_enemy_index);
  E->base.current_instruction = addr_kBouncingGoofball_Ilist_86BF;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
}

const uint16 *BouncingGoofball_Instr_88C5(uint16 k, const uint16 *jp) {  // 0xA288C5
  return jp;
}

const uint16 *BouncingGoofball_Instr_88C6(uint16 k, const uint16 *jp) {  // 0xA288C6
  Get_BouncingGoofball(cur_enemy_index)->bgl_var_02 = 0;
  QueueSfx2_Max6(0xE);
  return jp;
}

const uint16 *EnemyInstr_Rio_Instr_1(uint16 k, const uint16 *jp) {  // 0xA2BBC3
  Get_Rio(cur_enemy_index)->rio_var_E = 1;
  return jp;
}

void Rio_Init(void) {  // 0xA2BBCD
  Enemy_Rio *E = Get_Rio(cur_enemy_index);
  E->rio_var_E = 0;
  E->rio_var_F = 0;
  E->base.current_instruction = addr_kRio_Ilist_BB4B;
  E->rio_var_B = FUNC16(Rio_1);
}

void CallRioFunc(uint32 ea) {
  uint16 k = cur_enemy_index;
  switch (ea) {
  case fnRio_1: Rio_1(k); return;  // 0x7c30a
  case fnRio_2: Rio_2(k); return;  // 0x7c396
  case fnRio_3: Rio_3(k); return;  // 0x7c3d4
  case fnRio_4: Rio_4(k); return;  // 0x7c4dc
  case fnRio_5: Rio_5(k); return;  // 0x7c599
  default: Unreachable();
  }
}

void Rio_Main(void) {  // 0xA2BBE3
  NextRandom();
  Enemy_Rio *E = Get_Rio(cur_enemy_index);
  CallRioFunc(E->rio_var_B | 0xA20000);
}

void Rio_1(uint16 k) {  // 0xA2BBED
  if (!(CompareDistToSamus_X(k, 0xA0) & 1)) {
    Enemy_Rio *E = Get_Rio(k);
    E->rio_var_C = g_word_A2BBBB;
    E->rio_var_D = g_word_A2BBBF;
    if ((int16)(samus_x_pos - E->base.x_pos) < 0)
      E->rio_var_D = -E->rio_var_D;
    Rio_6(addr_kRio_Ilist_BB7F);
    E->rio_var_B = FUNC16(Rio_3);
    if (!CheckIfEnemyIsOnScreen())
      QueueSfx2_Max6(0x65);
  }
}

void Rio_2(uint16 k) {  // 0xA2BC32
  Enemy_Rio *E = Get_Rio(k);
  if (E->rio_var_E) {
    E->rio_var_E = 0;
    Rio_6(addr_stru_A2BB53);
    E->rio_var_B = FUNC16(Rio_1);
  }
}

void Rio_3(uint16 k) {  // 0xA2BC48
  int16 v4;

  Enemy_Rio *E = Get_Rio(k);
  ;
  if (Enemy_MoveRight_IgnoreSlopes(k, INT16_SHL8(E->rio_var_D))) {
    E->rio_var_D = -E->rio_var_D;
LABEL_13:
    E->rio_var_C = -E->rio_var_C;
    E->rio_var_B = FUNC16(Rio_4);
    return;
  }
  if (Enemy_MoveDown(k, INT16_SHL8(E->rio_var_C)))
    goto LABEL_13;
  v4 = E->rio_var_C - 24;
  E->rio_var_C = v4;
  if (v4 < 0) {
    E->rio_var_A = E->rio_var_D;
    E->rio_var_D = 0;
    E->rio_var_C = 0;
    E->rio_var_B = FUNC16(Rio_5);
  } else if (E->rio_var_E) {
    E->rio_var_E = 0;
    Rio_6(addr_kRio_Ilist_BB97);
  }
}

void Rio_4(uint16 k) {  // 0xA2BCB7
  Enemy_Rio *E = Get_Rio(k);
  if (Enemy_MoveRight_IgnoreSlopes(k, INT16_SHL8(E->rio_var_D)))
    E->rio_var_D = -E->rio_var_D;
  if (Enemy_MoveDown(k, INT16_SHL8(E->rio_var_C))) {
    Rio_6(addr_kRio_Ilist_BBA3);
    E->rio_var_B = FUNC16(Rio_2);
  } else {
    E->rio_var_C -= 24;
  }
}

void Rio_5(uint16 k) {  // 0xA2BCFF
  Enemy_Rio *E = Get_Rio(k);
  if ((int16)(Get_Rio(k)->base.y_pos - samus_y_pos) >= 0) {
    E->rio_var_D = E->rio_var_A;
    E->rio_var_C = -1;
    E->rio_var_B = FUNC16(Rio_4);
  } else {
    int v1 = CalculateAngleOfSamusFromEnemy(k);
    E->rio_var_D = kSinCosTable8bit_Sext[v1 + 64];
    E->rio_var_C = kSinCosTable8bit_Sext[v1];
    Enemy_MoveRight_IgnoreSlopes(k, INT16_SHL8(E->rio_var_D));
    Enemy_MoveDown(k, INT16_SHL8(E->rio_var_C));
  }
}

void Rio_6(uint16 a) {  // 0xA2BD54
  Enemy_Rio *E = Get_Rio(cur_enemy_index);
  if (a != E->rio_var_F) {
    E->rio_var_F = a;
    E->base.current_instruction = a;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
  }
}

const uint16 *NorfairRio_Instr_C1C9(uint16 k, const uint16 *jp) {  // 0xA2C1C9
  Get_NorfairRio(cur_enemy_index)->nro_var_01 = 1;
  return jp;
}

const uint16 *NorfairRio_Instr_C1D4(uint16 k, const uint16 *jp) {  // 0xA2C1D4
  Get_NorfairRio(cur_enemy_index)->nro_var_02 = 8;
  return jp;
}

const uint16 *NorfairRio_Instr_C1DF(uint16 k, const uint16 *jp) {  // 0xA2C1DF
  Get_NorfairRio(cur_enemy_index)->nro_var_02 = 8;
  return jp;
}

const uint16 *NorfairRio_Instr_C1EA(uint16 k, const uint16 *jp) {  // 0xA2C1EA
  Get_NorfairRio(cur_enemy_index)->nro_var_02 = 12;
  return jp;
}

const uint16 *NorfairRio_Instr_C1F5(uint16 k, const uint16 *jp) {  // 0xA2C1F5
  Get_NorfairRio(cur_enemy_index)->nro_var_02 = -12;
  return jp;
}

const uint16 *NorfairRio_Instr_C200(uint16 k, const uint16 *jp) {  // 0xA2C200
  Get_NorfairRio(cur_enemy_index)->nro_var_02 = 4;
  return jp;
}

const uint16 *NorfairRio_Instr_C20B(uint16 k, const uint16 *jp) {  // 0xA2C20B
  Get_NorfairRio(cur_enemy_index)->nro_var_02 = 0;
  return jp;
}

const uint16 *NorfairRio_Instr_C216(uint16 k, const uint16 *jp) {  // 0xA2C216
  Get_NorfairRio(cur_enemy_index)->nro_var_02 = -4;
  return jp;
}

const uint16 *NorfairRio_Instr_C221(uint16 k, const uint16 *jp) {  // 0xA2C221
  Get_NorfairRio(cur_enemy_index)->nro_var_02 = -12;
  return jp;
}

const uint16 *NorfairRio_Instr_C22C(uint16 k, const uint16 *jp) {  // 0xA2C22C
  Get_NorfairRio(cur_enemy_index)->nro_var_02 = -16;
  return jp;
}

const uint16 *NorfairRio_Instr_C237(uint16 k, const uint16 *jp) {  // 0xA2C237
  Get_NorfairRio(cur_enemy_index)->nro_var_02 = 12;
  return jp;
}

void NorfairRio_Init(void) {  // 0xA2C242
  Enemy_NorfairRio *E = Get_NorfairRio(cur_enemy_index);
  E->nro_var_01 = 0;
  E->nro_var_02 = 0;
  if ((E->nro_parameter_1 & 0x8000) != 0) {
    E->nro_var_00 = addr_kNorfairRio_Ilist_C18F;
    E->base.current_instruction = addr_kNorfairRio_Ilist_C18F;
    E->nro_var_F = FUNC16(NorfairRio_Func_1);
  } else {
    E->nro_var_00 = addr_kNorfairRio_Ilist_C0F1;
    E->base.current_instruction = addr_kNorfairRio_Ilist_C0F1;
    E->nro_var_F = FUNC16(NorfairRio_Func_2);
  }
}

void NorfairRio_Main(void) {  // 0xA2C277
  NextRandom();
  Enemy_NorfairRio *E = Get_NorfairRio(cur_enemy_index);
  EnemyRunPreInstr(E->nro_var_F);
}

void NorfairRio_Func_1(uint16 k) {  // 0xA2C281
  Enemy_NorfairRio *E = Get_NorfairRio(k);
  EnemySpawnData *v6;

  int v1 = k >> 1;
  if (enemy_drawing_queue[v1 + 100]) {
    uint16 v3 = enemy_drawing_queue[v1 + 109];
    E->base.frozen_timer = v3;
    if (v3
        || (E->base.properties &= ~kEnemyProps_Invisible,
            gEnemySpawnData(k)[31].some_flag == addr_kNorfairRio_Ilist_C0F1)) {
      E->base.properties |= kEnemyProps_Invisible;
    } else {
      uint16 r18 = addr_kNorfairRio_Ilist_C18F;
      v6 = gEnemySpawnData(k);
      if ((v6[31].field_4 & 0x8000) != 0)
        r18 = addr_kNorfairRio_Ilist_C1A3;
      NorfairRio_Func_7(r18);
      E->base.properties &= ~kEnemyProps_Invisible;
      E->base.x_pos = enemy_drawing_queue[v1 + 91];
      E->base.y_pos = v6[31].field_4 + enemy_drawing_queue[v1 + 93];
    }
  } else {
    E->base.properties |= kEnemyProps_Deleted;
  }
}

void NorfairRio_Func_2(uint16 k) {  // 0xA2C2E7
  Enemy_NorfairRio *E = Get_NorfairRio(k);
  if ((random_number & 0x101) != 0 && !(CompareDistToSamus_X(k, 0xC0) & 1)) {
    E->nro_var_A = g_word_A2C1C1[(uint16)((random_number >> 1) & 2) >> 1];
    E->nro_var_B = g_word_A2C1C5;
    if ((int16)(samus_x_pos - E->base.x_pos) < 0)
      E->nro_var_B = -E->nro_var_B;
    NorfairRio_Func_7(addr_kNorfairRio_Ilist_C107);
    E->nro_var_F = FUNC16(NorfairRio_Func_3);
  } else {
    if (E->nro_var_01) {
      E->nro_var_01 = 0;
      NorfairRio_Func_7(addr_kNorfairRio_Ilist_C0F1);
    }
  }
}

void NorfairRio_Func_3(uint16 k) {  // 0xA2C33F
  Enemy_NorfairRio *E = Get_NorfairRio(k);
  if (E->nro_var_01) {
    E->nro_var_01 = 0;
    NorfairRio_Func_7(addr_kNorfairRio_Ilist_C12F);
    E->nro_var_F = FUNC16(NorfairRio_Func_4);
    QueueSfx2_Max6(0x65);
  }
}

void NorfairRio_Func_4(uint16 k) {  // 0xA2C361
  int16 v4;

  Enemy_NorfairRio *E = Get_NorfairRio(k);
  if (Enemy_MoveRight_IgnoreSlopes(k, INT16_SHL8(E->nro_var_B)))
    E->nro_var_B = -E->nro_var_B;
  if (Enemy_MoveDown(k, INT16_SHL8(E->nro_var_A)) || (v4 = E->nro_var_A - 32, E->nro_var_A = v4, v4 < 0)) {
    E->nro_var_A = -1;
    NorfairRio_Func_7(addr_kNorfairRio_Ilist_C145);
    E->nro_var_F = FUNC16(NorfairRio_Func_5);
  }
}

void NorfairRio_Func_5(uint16 k) {  // 0xA2C3B1
  Enemy_NorfairRio *E = Get_NorfairRio(k);
  if (Enemy_MoveRight_IgnoreSlopes(k, INT16_SHL8(E->nro_var_B)))
    E->nro_var_B = -E->nro_var_B;
  if (Enemy_MoveDown(k, INT16_SHL8(E->nro_var_A))) {
    E->nro_var_F = FUNC16(NorfairRio_Func_6);
  } else {
    E->nro_var_A -= 32;
    if (E->nro_var_01) {
      E->nro_var_01 = 0;
      NorfairRio_Func_7(addr_kNorfairRio_Ilist_C179);
    }
  }
}

void NorfairRio_Func_6(uint16 k) {  // 0xA2C406
  Get_NorfairRio(k)->nro_var_F = FUNC16(NorfairRio_Func_2);
}

void NorfairRio_Func_7(uint16 a) {  // 0xA2C40D
  Enemy_NorfairRio *E = Get_NorfairRio(cur_enemy_index);
  if (a != E->nro_var_00) {
    E->nro_var_00 = a;
    E->base.current_instruction = a;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
  }
}

const uint16 *LowerNorfairRio_Instr_C6D2(uint16 k, const uint16 *jp) {  // 0xA2C6D2
  Get_NorfairRio(cur_enemy_index)->nro_var_01 = 1;
  return jp;
}

const uint16 *LowerNorfairRio_Instr_C6DD(uint16 k, const uint16 *jp) {  // 0xA2C6DD
  Get_NorfairRio(cur_enemy_index)->nro_var_02 = 0;
  return jp;
}

const uint16 *LowerNorfairRio_Instr_C6E8(uint16 k, const uint16 *jp) {  // 0xA2C6E8
  Get_NorfairRio(cur_enemy_index)->nro_var_02 = 1;
  return jp;
}

void LowerNorfairRio_Init(void) {  // 0xA2C6F3
  Enemy_NorfairRio *E = Get_NorfairRio(cur_enemy_index);
  E->nro_var_01 = 0;
  if ((E->nro_parameter_1 & 0x8000) == 0) {
    E->nro_var_F = FUNC16(LowerNorfairRio_Func_2);
    E->nro_var_00 = addr_kLowerNorfairRio_Ilist_C61A;
    E->base.current_instruction = addr_kLowerNorfairRio_Ilist_C61A;
  } else {
    E->nro_var_F = FUNC16(LowerNorfairRio_Func_1);
    E->nro_var_00 = addr_kLowerNorfairRio_Ilist_C6B0;
    E->base.current_instruction = addr_kLowerNorfairRio_Ilist_C6B0;
  }
}

void LowerNorfairRio_Main(void) {  // 0xA2C724
  NextRandom();
  Enemy_LowerNorfairRio *E = Get_LowerNorfairRio(cur_enemy_index);
  EnemyRunPreInstr(E->lnro_var_F);
}

void LowerNorfairRio_Func_1(uint16 k) {  // 0xA2C72E
  Enemy_LowerNorfairRio *E = Get_LowerNorfairRio(k);

  int v1 = k >> 1;
  if (enemy_drawing_queue[v1 + 100]) {
    uint16 v3 = enemy_drawing_queue[v1 + 109];
    E->base.frozen_timer = v3;
    if (!v3 && gEnemySpawnData(k)[31].field_4) {
      E->base.properties &= ~kEnemyProps_Invisible;
      E->base.x_pos = enemy_drawing_queue[v1 + 91];
      E->base.y_pos = enemy_drawing_queue[v1 + 93] + 12;
    } else {
      E->base.properties |= kEnemyProps_Invisible;
    }
  } else {
    E->base.properties |= kEnemyProps_Deleted;
  }
}

void LowerNorfairRio_Func_2(uint16 k) {  // 0xA2C771
  Enemy_NorfairRio *E = Get_NorfairRio(k);
  if ((random_number & 0x101) != 0 && !(CompareDistToSamus_X(k, 0x70) & 1)) {
    E->nro_var_C = g_word_A2C6CA;
    E->nro_var_D = g_word_A2C6CE;
    if ((int16)(samus_x_pos - E->base.x_pos) < 0)
      E->nro_var_D = -E->nro_var_D;
    LowerNorfairRio_Func_7(addr_kLowerNorfairRio_Ilist_C630);
    E->nro_var_F = FUNC16(LowerNorfairRio_Func_3);
  } else {
    E->nro_var_01 = 0;
    LowerNorfairRio_Func_7(addr_kLowerNorfairRio_Ilist_C61A);
  }
}

void LowerNorfairRio_Func_3(uint16 k) {  // 0xA2C7BB
  Enemy_NorfairRio *E = Get_NorfairRio(k);
  if (E->nro_var_01) {
    E->nro_var_01 = 0;
    LowerNorfairRio_Func_7(addr_kLowerNorfairRio_Ilist_C65A);
    E->nro_var_F = FUNC16(LowerNorfairRio_Func_4);
  }
}

void LowerNorfairRio_Func_4(uint16 k) {  // 0xA2C7D6
  int16 v4;

  Enemy_LowerNorfairRio *E = Get_LowerNorfairRio(k);
  if (Enemy_MoveRight_IgnoreSlopes(k, INT16_SHL8(E->lnro_var_D)))
    E->lnro_var_D = -E->lnro_var_D;
  if (Enemy_MoveDown(k, INT16_SHL8(E->lnro_var_C)) || (v4 = E->lnro_var_C - 32, E->lnro_var_C = v4, v4 < 0)) {
    E->lnro_var_C = -1;
    LowerNorfairRio_Func_7(addr_kLowerNorfairRio_Ilist_C662);
    E->lnro_var_F = FUNC16(LowerNorfairRio_Func_5);
    QueueSfx2_Max6(0x64);
  }
}

void LowerNorfairRio_Func_5(uint16 k) {  // 0xA2C82D
  Enemy_NorfairRio *E = Get_NorfairRio(k);
  if (Enemy_MoveRight_IgnoreSlopes(k, INT16_SHL8(E->nro_var_D)))
    E->nro_var_D = -E->nro_var_D;
  if (Enemy_MoveDown(k, INT16_SHL8(E->nro_var_C))) {
    LowerNorfairRio_Func_7(addr_kLowerNorfairRio_Ilist_C686);
    E->nro_var_F = FUNC16(LowerNorfairRio_Func_6);
  } else {
    E->nro_var_C -= 32;
    if (E->nro_var_01) {
      E->nro_var_01 = 0;
      LowerNorfairRio_Func_7(addr_kLowerNorfairRio_Ilist_C674);
    }
  }
}

void LowerNorfairRio_Func_6(uint16 k) {  // 0xA2C888
  Enemy_NorfairRio *E = Get_NorfairRio(k);
  if (E->nro_var_01) {
    E->nro_var_01 = 0;
    LowerNorfairRio_Func_7(addr_kLowerNorfairRio_Ilist_C686);
    E->nro_var_F = FUNC16(LowerNorfairRio_Func_2);
  }
}

void LowerNorfairRio_Func_7(uint16 a) {  // 0xA2C8A3
  Enemy_NorfairRio *E = Get_NorfairRio(cur_enemy_index);
  if (a != E->nro_var_00) {
    E->nro_var_00 = a;
    E->base.current_instruction = a;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
  }
}
