// Flies extracted from sm_a2.c.
#include "ida_types.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "variables.h"
#include "enemy_ai_canon.h"

enum {
  kFliesDetectX = 0x70,
  kFliesIdleAngleStep = 32,
  kFliesIdleAngleMask = 0x1FF,
  kFliesCooldown = 24,
};

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
  int v = E->flies_var_E >> 1;
  bool carry = CarryAdd16(&E->base.x_subpos, LOBYTE(kSinCosTable8bit_Sext[v + 64]) << 8);
  E->base.x_pos += (int8)HIBYTE(kSinCosTable8bit_Sext[v + 64]) + carry;
  carry = CarryAdd16(&E->base.y_subpos, LOBYTE(kSinCosTable8bit_Sext[v]) << 8);
  E->base.y_pos += (int8)HIBYTE(kSinCosTable8bit_Sext[v]) + carry;
}

uint16 Flies_2(uint16 k) {  // 0xA2B0DC
  Enemy_Flies *E = Get_Flies(k);
  bool carry = CarryAdd16(&E->base.x_subpos, LOBYTE(E->flies_var_B) << 8);
  E->base.x_pos += (int8)HIBYTE(E->flies_var_B) + carry;
  carry = CarryAdd16(&E->base.y_subpos, LOBYTE(E->flies_var_C) << 8);
  uint16 y = E->base.y_pos + (int8)HIBYTE(E->flies_var_C) + carry;
  E->base.y_pos = y;
  return y;
}

void Flies_Main(void) {  // 0xA2B11F
  NextRandom();
  Enemy_Flies *E = Get_Flies(cur_enemy_index);
  EnemyRunPreInstr(E->flies_var_F);
}

void Flies_3(uint16 k) {  // 0xA2B129
  int angle = CalculateAngleOfSamusFromEnemy(k);
  Enemy_Flies *E = Get_Flies(k);
  E->flies_var_B = 2 * kSinCosTable8bit_Sext[angle + 64];
  E->flies_var_C = 4 * kSinCosTable8bit_Sext[angle];
  E->flies_var_D = samus_y_pos;
  E->flies_var_F = FUNC16(Flies_6);
}

void Flies_4(uint16 k) {  // 0xA2B14E
  Enemy_Flies *E = Get_Flies(k);
  uint16 cooldown = E->flies_var_A;
  if (cooldown) {
    E->flies_var_A = cooldown - 1;
  } else if (!(CompareDistToSamus_X(k, kFliesDetectX) & 1)) {
    Flies_3(k);
    return;
  }
  Flies_1(k);
  bool wrapped = ((E->flies_var_E + kFliesIdleAngleStep) & kFliesIdleAngleMask) == 0;
  E->flies_var_E = (E->flies_var_E + kFliesIdleAngleStep) & kFliesIdleAngleMask;
  if (wrapped)
    E->flies_var_F = FUNC16(Flies_5);
}

void Flies_5(uint16 k) {  // 0xA2B17C
  Enemy_Flies *E = Get_Flies(k);
  uint16 cooldown = E->flies_var_A;
  if (cooldown) {
    E->flies_var_A = cooldown - 1;
  } else if (!(CompareDistToSamus_X(k, kFliesDetectX) & 1)) {
    Flies_3(k);
    return;
  }
  Flies_1(k);
  bool wrapped = ((E->flies_var_E - kFliesIdleAngleStep) & kFliesIdleAngleMask) == 0;
  E->flies_var_E = (E->flies_var_E - kFliesIdleAngleStep) & kFliesIdleAngleMask;
  if (wrapped)
    E->flies_var_F = FUNC16(Flies_4);
}

void Flies_6(uint16 k) {  // 0xA2B1AA
  uint16 y = Flies_2(k);
  Enemy_Flies *E = Get_Flies(k);
  ++E->flies_var_A;
  bool passed_target;
  if (sign16(E->flies_var_C))
    passed_target = y < E->flies_var_D;
  else
    passed_target = y >= E->flies_var_D;
  if (passed_target) {
    E->flies_var_C = -E->flies_var_C;
    E->flies_var_F = FUNC16(Flies_7);
  }
}

void Flies_7(uint16 k) {  // 0xA2B1D2
  Flies_2(k);
  Enemy_Flies *E = Get_Flies(k);
  if (sign16(--E->flies_var_A)) {
    E->flies_var_A = kFliesCooldown;
    E->flies_var_F = FUNC16(Flies_4);
  }
}
