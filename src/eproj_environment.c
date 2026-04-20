// Enemy projectile environment/facility families split out of sm_86.c.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "eproj_internal.h"

#define kCommonEnemySpeeds_Quadratic_Copy ((uint16*)RomFixedPtr(0xa0cbc7))
#define kCommonEnemySpeeds_Quadratic32 ((uint32*)RomFixedPtr(0xa0cbc7))

static void CallNorfairLavaquakeRocksFunc(uint32 ea, uint16 k) {
  switch (ea) {
  case fnEproj_NorfairLavaquakeRocks_Func1: Eproj_NorfairLavaquakeRocks_Func1(k); return;
  case fnEproj_NorfairLavaquakeRocks_Func2: Eproj_NorfairLavaquakeRocks_Func2(k); return;
  default: Unreachable();
  }
}

void EprojInit_NorfairLavaquakeRocks(uint16 j) {  // 0x86BBDB
  int v1 = j >> 1;
  eproj_instr_list_ptr[v1] = addr_word_86BBD5;
  eproj_E[v1] = FUNC16(Eproj_NorfairLavaquakeRocks_Func1);
  eproj_y_vel[v1] = eproj_init_param_1;
  eproj_x_vel[v1] = eproj_unk1995;
  EnemyData *v2 = gEnemyData(cur_enemy_index);
  eproj_x_pos[v1] = v2->x_pos;
  eproj_x_subpos[v1] = v2->x_subpos;
  eproj_y_pos[v1] = v2->y_pos;
  eproj_y_subpos[v1] = v2->y_subpos;
}

void EprojPreInstr_NorfairLavaquakeRocks(uint16 k) {  // 0x86BC0F
  CallNorfairLavaquakeRocksFunc(eproj_E[k >> 1] | 0x860000, k);
  EprojPreInstr_NorfairLavaquakeRocks_Inner(k);
}

void Eproj_NorfairLavaquakeRocks_Func1(uint16 k) {  // 0x86BC16
  int16 v2;
  int16 v3;

  int v1 = k >> 1;
  v2 = eproj_y_vel[v1] - 2;
  eproj_y_vel[v1] = v2;
  if (v2 >= 0) {
    int n = 2;
    do {
      v3 = n + eproj_y_vel[v1] - 1;
      if (v3 < 0)
        v3 = 0;
      int t = (8 * v3 + 4) >> 1;
      AddToHiLo(&eproj_y_pos[v1], &eproj_y_subpos[v1], kCommonEnemySpeeds_Quadratic32[t >> 1]);
      eproj_F[v1] = kCommonEnemySpeeds_Quadratic_Copy[t + 1];  // junk
    } while (--n);
    Eproj_NorfairLavaquakeRocks_Func3(k);
  } else {
    eproj_y_vel[v1] = 0;
    eproj_E[v1] = FUNC16(Eproj_NorfairLavaquakeRocks_Func2);
  }
}

void Eproj_NorfairLavaquakeRocks_Func2(uint16 k) {  // 0x86BC8F
  int v1 = k >> 1;
  uint16 v2 = eproj_y_vel[v1] + 2;
  eproj_y_vel[v1] = v2;
  if (!sign16(v2 - 64))
    eproj_y_vel[v1] = 64;
  int n = 2;
  do {
    int t = (8 * (eproj_y_vel[v1] - n + 1)) >> 1;
    AddToHiLo(&eproj_y_pos[v1], &eproj_y_subpos[v1], kCommonEnemySpeeds_Quadratic32[t >> 1]);
    eproj_F[v1] = kCommonEnemySpeeds_Quadratic_Copy[t + 1];  // junk
  } while (--n);
  Eproj_NorfairLavaquakeRocks_Func3(k);
}

void Eproj_NorfairLavaquakeRocks_Func3(uint16 k) {  // 0x86BCF4
  int v1 = k >> 1;
  AddToHiLo(&eproj_x_pos[v1], &eproj_x_subpos[v1], INT16_SHL8(eproj_x_vel[v1]));
}

uint16 EprojPreInstr_NorfairLavaquakeRocks_Inner2(uint16 k) {  // 0x86BD2A
  int v1 = k >> 1;
  return (int16)(eproj_x_pos[v1] - layer1_x_pos) < 0
    || (int16)(layer1_x_pos + 256 - eproj_x_pos[v1]) < 0
    || (int16)(eproj_y_pos[v1] - layer1_y_pos) < 0
    || (int16)(layer1_y_pos + 256 - eproj_y_pos[v1]) < 0;
}

void EprojPreInstr_NorfairLavaquakeRocks_Inner(uint16 k) {  // 0x86BD1E
  if (EprojPreInstr_NorfairLavaquakeRocks_Inner2(k))
    eproj_id[k >> 1] = 0;
}
