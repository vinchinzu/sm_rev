// LavaquakeRocks extracted from sm_a2.c.
#include "ida_types.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "variables.h"
#include "enemy_ai_canon.h"

enum {
  kLavaquakeRocksProximity = 0x40,
  kLavaquakeRocksDelayMask = 0xE,
  kLavaquakeRocksSpeedMask = 0x1E,
  kLavaquakeRocksRandomSeed = 17,
};

static const uint16 kLavaquakeRocksCooldown[8] = {
  0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40, 0x48,
};
static const uint16 kLavaquakeRocksEprojParam[16] = {
  0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
  0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
};
static const int16 kLavaquakeRocksYSpeed[16] = {
  0x60, 0x70, 0x80, 0x90, 0xa0, 0xb0, 0xc0, 0xd0,
  -0x60, -0x70, -0x80, -0x90, -0xa0, -0xb0, -0xc0, -0xd0,
};

void LavaquakeRocks_Init(void) {  // 0xA2B570
  Enemy_LavaquakeRocks *E = Get_LavaquakeRocks(cur_enemy_index);
  E->base.current_instruction = addr_kNorfairErraticFireball_Ilist_B51A;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->lrs_var_A = FUNC16(LavaquakeRocks_1);
  random_number = kLavaquakeRocksRandomSeed;
}

void CallLavaQuakeRocksFunc(uint32 ea) {
  switch (ea) {
  case fnLavaquakeRocks_1: LavaquakeRocks_1(); return;
  case fnLavaquakeRocks_2: LavaquakeRocks_2(); return;
  case fnLavaquakeRocks_3: LavaquakeRocks_3(); return;
  default: Unreachable();
  }
}

void LavaquakeRocks_Main(void) {  // 0xA2B58F
  Enemy_LavaquakeRocks *E = Get_LavaquakeRocks(cur_enemy_index);
  CallLavaQuakeRocksFunc(E->lrs_var_A | 0xA20000);
}

void LavaquakeRocks_1(void) {  // 0xA2B596
  if (IsSamusWithinEnemy_X(cur_enemy_index, kLavaquakeRocksProximity)) {
    if (IsSamusWithinEnemy_Y(cur_enemy_index, kLavaquakeRocksProximity))
      Get_LavaquakeRocks(cur_enemy_index)->lrs_var_A = FUNC16(LavaquakeRocks_2);
  }
}

void LavaquakeRocks_2(void) {  // 0xA2B5B2
  eproj_unk1995 = kLavaquakeRocksYSpeed[(NextRandom() & kLavaquakeRocksSpeedMask) >> 1];
  int param_index = (NextRandom() & kLavaquakeRocksSpeedMask) >> 1;
  SpawnEprojWithGfx(kLavaquakeRocksEprojParam[param_index], cur_enemy_index, addr_stru_86BD5A);
  Enemy_LavaquakeRocks *E = Get_LavaquakeRocks(cur_enemy_index);
  E->lrs_var_A = FUNC16(LavaquakeRocks_3);
  E->lrs_var_B = kLavaquakeRocksCooldown[(uint16)(NextRandom() & kLavaquakeRocksDelayMask) >> 1];
}

void LavaquakeRocks_3(void) {  // 0xA2B5EA
  Enemy_LavaquakeRocks *E = Get_LavaquakeRocks(cur_enemy_index);
  if (sign16(--E->lrs_var_B))
    E->lrs_var_A = FUNC16(LavaquakeRocks_1);
}
