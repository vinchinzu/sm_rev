// LavaquakeRocks extracted from sm_a2.c.
#include "ida_types.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "variables.h"
#include "enemy_ai_canon.h"

#define g_word_A2B520 ((uint16*)RomFixedPtr(0xa2b520))
#define g_word_A2B530 ((uint16*)RomFixedPtr(0xa2b530))
#define g_word_A2B550 ((uint16*)RomFixedPtr(0xa2b550))


void LavaquakeRocks_Init(void) {  // 0xA2B570
  Enemy_LavaquakeRocks *E = Get_LavaquakeRocks(cur_enemy_index);
  E->base.current_instruction = addr_kNorfairErraticFireball_Ilist_B51A;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->lrs_var_A = FUNC16(LavaquakeRocks_1);
  random_number = 17;
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
  if (IsSamusWithinEnemy_X(cur_enemy_index, 0x40)) {
    if (IsSamusWithinEnemy_Y(cur_enemy_index, 0x40))
      Get_LavaquakeRocks(cur_enemy_index)->lrs_var_A = FUNC16(LavaquakeRocks_2);
  }
}

void LavaquakeRocks_2(void) {  // 0xA2B5B2
  eproj_unk1995 = g_word_A2B550[(NextRandom() & 0x1E) >> 1];
  int v1 = (NextRandom() & 0x1E) >> 1;
  SpawnEprojWithGfx(g_word_A2B530[v1], cur_enemy_index, addr_stru_86BD5A);
  Enemy_LavaquakeRocks *E = Get_LavaquakeRocks(cur_enemy_index);
  E->lrs_var_A = FUNC16(LavaquakeRocks_3);
  E->lrs_var_B = g_word_A2B520[(uint16)(NextRandom() & 0xE) >> 1];
}

void LavaquakeRocks_3(void) {  // 0xA2B5EA
  Enemy_LavaquakeRocks *E = Get_LavaquakeRocks(cur_enemy_index);
  if ((--E->lrs_var_B & 0x8000) != 0)
    E->lrs_var_A = FUNC16(LavaquakeRocks_1);
}
