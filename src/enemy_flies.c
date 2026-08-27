// Flies extracted from sm_a2.c.
#include "ida_types.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "variables.h"
#include "enemy_ai_canon.h"


void Flies_Init(void) {  // 0xA2B06B
  Enemy_Flies *E = Get_Flies(cur_enemy_index);
  E->flies_var_E = 0;
  E->flies_var_F = FUNC16(Flies_4);
  E->base.current_instruction = addr_kFlies_Ilist_B013;
  E->base.spritemap_pointer = addr_kSpritemap_Nothing_A2;
  E->base.instruction_timer = 1;
  E->base.spritemap_pointer = addr_kSpritemap_Nothing_A2;
}

bool CarryAdd16(uint16 *rv, uint16 v) {
  int i = *rv + v;
  *rv = i;
  return (i >> 16);
}

void Flies_1(uint16 k) {  // 0xA2B090
  Enemy_Flies *E = Get_Flies(k);
  int v2 = E->flies_var_E >> 1;
  bool carry = CarryAdd16(&E->base.x_subpos, LOBYTE(kSinCosTable8bit_Sext[v2 + 64]) << 8);
  E->base.x_pos += (int8)HIBYTE(kSinCosTable8bit_Sext[v2 + 64]) + carry;
  carry = CarryAdd16(&E->base.y_subpos, LOBYTE(kSinCosTable8bit_Sext[v2]) << 8);
  E->base.y_pos += (int8)HIBYTE(kSinCosTable8bit_Sext[v2]) + carry;
}

uint16 Flies_2(uint16 k) {  // 0xA2B0DC
  Enemy_Flies *E = Get_Flies(k);
  bool carry = CarryAdd16(&E->base.x_subpos, LOBYTE(E->flies_var_B) << 8);
  E->base.x_pos += (int8)HIBYTE(E->flies_var_B) + carry;
  carry = CarryAdd16(&E->base.y_subpos, LOBYTE(E->flies_var_C) << 8);
  uint16 result = E->base.y_pos + (int8)HIBYTE(E->flies_var_C) + carry;
  E->base.y_pos = result;
  return result;
}

void Flies_Main(void) {  // 0xA2B11F
  NextRandom();
  Enemy_Flies *E = Get_Flies(cur_enemy_index);
  EnemyRunPreInstr(E->flies_var_F);
}

void Flies_3(uint16 k) {  // 0xA2B129
  int v1 = CalculateAngleOfSamusFromEnemy(k);
  Enemy_Flies *E = Get_Flies(k);
  E->flies_var_B = 2 * kSinCosTable8bit_Sext[v1 + 64];
  E->flies_var_C = 4 * kSinCosTable8bit_Sext[v1];
  E->flies_var_D = samus_y_pos;
  E->flies_var_F = FUNC16(Flies_6);
}

void Flies_4(uint16 k) {  // 0xA2B14E
  Enemy_Flies *E = Get_Flies(k);
  uint16 flies_var_A = E->flies_var_A;
  if (flies_var_A) {
    E->flies_var_A = flies_var_A - 1;
  } else if (!(CompareDistToSamus_X(k, 0x70) & 1)) {
    Flies_3(k);
    return;
  }
  Flies_1(k);
  bool v3 = ((E->flies_var_E + 32) & 0x1FF) == 0;
  E->flies_var_E = (E->flies_var_E + 32) & 0x1FF;
  if (v3)
    E->flies_var_F = FUNC16(Flies_5);
}

void Flies_5(uint16 k) {  // 0xA2B17C
  Enemy_Flies *E = Get_Flies(k);
  uint16 flies_var_A = E->flies_var_A;
  if (flies_var_A) {
    E->flies_var_A = flies_var_A - 1;
  } else if (!(CompareDistToSamus_X(k, 0x70) & 1)) {
    Flies_3(k);
    return;
  }
  Flies_1(k);
  bool v3 = ((E->flies_var_E - 32) & 0x1FF) == 0;
  E->flies_var_E = (E->flies_var_E - 32) & 0x1FF;
  if (v3)
    E->flies_var_F = FUNC16(Flies_4);
}

void Flies_6(uint16 k) {  // 0xA2B1AA
  uint16 v1 = Flies_2(k);
  Enemy_Flies *E = Get_Flies(k);
  ++E->flies_var_A;
  if ((E->flies_var_C & 0x8000) == 0) {
    if (v1 < E->flies_var_D)
      return;
    goto LABEL_5;
  }
  if (v1 < E->flies_var_D) {
LABEL_5:
    E->flies_var_C = -E->flies_var_C;
    E->flies_var_F = FUNC16(Flies_7);
  }
}

void Flies_7(uint16 k) {  // 0xA2B1D2
  Flies_2(k);
  Enemy_Flies *E = Get_Flies(k);
  if ((--E->flies_var_A & 0x8000) != 0) {
    E->flies_var_A = 24;
    E->flies_var_F = FUNC16(Flies_4);
  }
}
