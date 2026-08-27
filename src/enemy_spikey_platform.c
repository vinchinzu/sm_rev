// Enemy AI - Spikey platform — peeled from Bank $A6
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"

void SpikeyPlatform_Init(void) {  // 0xA68B2F
  Enemy_SpikeyPlatform *E = Get_SpikeyPlatform(cur_enemy_index);
  E->base.current_instruction = addr_kSpikeyPlatform_Ilist_8B29;
  E->spm_var_A = FUNC16(SpikeyPlatform_Func_1);
  uint16 v1 = 8 * LOBYTE(E->spm_parameter_1);
  E->spm_var_02 = v1;
  int v2 = v1 >> 1;
  E->spm_var_E = kCommonEnemySpeeds_Linear[v2 + 1];
  E->spm_var_F = kCommonEnemySpeeds_Linear[v2];
  E->spm_var_D = E->base.y_pos + HIBYTE(E->spm_parameter_2);
  E->spm_var_C = E->base.y_pos;
  uint16 spm_parameter_2_low = LOBYTE(E->spm_parameter_2);
  E->spm_var_00 = spm_parameter_2_low;
  E->spm_var_B = spm_parameter_2_low;
}

void SpikeyPlatform2ndEnemy_Init(void) {  // 0xA68B85
  int v0 = cur_enemy_index >> 1;
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  v1->x_pos = enemy_drawing_queue[v0 + 91];
  v1->y_pos = enemy_drawing_queue[v0 + 93] + 12;
}

void SpikeyPlatform2ndEnemy_Main(void) {  // 0xA68B99
  int v0 = cur_enemy_index >> 1;
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  v1->x_pos = enemy_drawing_queue[v0 + 91];
  v1->y_pos = enemy_drawing_queue[v0 + 93] + 12;
}

void CallSpikeyPlatformFunc(uint32 ea, uint16 k) {
  switch (ea) {
  case fnSpikeyPlatform_Func_1: SpikeyPlatform_Func_1(); return;
  case fnSpikeyPlatform_Func_2: SpikeyPlatform_Func_2(); return;
  case fnSpikeyPlatform_Func_3: SpikeyPlatform_Func_3(k); return;
  case fnSpikeyPlatform_Func_4: SpikeyPlatform_Func_4(); return;
  default: Unreachable();
  }
}

void SpikeyPlatform_Main(void) {  // 0xA68BAD
  Enemy_SpikeyPlatform *E = Get_SpikeyPlatform(cur_enemy_index);
  CallSpikeyPlatformFunc(E->spm_var_A | 0xA60000, cur_enemy_index);
}

void SpikeyPlatform_Func_1(void) {  // 0xA68BB4
  Enemy_SpikeyPlatform *E = Get_SpikeyPlatform(cur_enemy_index);
  if (!--E->spm_var_B) {
    E->spm_var_B = E->spm_var_00;
    E->spm_var_02 = 8 * LOBYTE(E->spm_parameter_1);
    E->spm_var_A = FUNC16(SpikeyPlatform_Func_2);
  }
}

void SpikeyPlatform_Func_2(void) {  // 0xA68BDC
  Enemy_SpikeyPlatform *E = Get_SpikeyPlatform(cur_enemy_index);
  E->spm_var_01 = E->base.y_pos;
  int v1 = E->spm_var_02 >> 1;
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(kCommonEnemySpeeds_Linear[v1], kCommonEnemySpeeds_Linear[v1 + 1]));
  if ((int16)(E->base.y_pos - E->spm_var_D) >= 0) {
    E->spm_var_03 = 64;
    E->spm_var_A = FUNC16(SpikeyPlatform_Func_3);
    E->base.y_pos = E->spm_var_D;
    QueueSfx2_Max6(0x1B);
  }
  if (SpikeyPlatform_Func_5(cur_enemy_index))
    extra_samus_y_displacement += E->base.y_pos - E->spm_var_01;
  int16 v4 = E->spm_var_02 + 8;
  if (!sign16(E->spm_var_02 - 504))
    v4 = 512;
  E->spm_var_02 = v4;
}

void SpikeyPlatform_Func_3(uint16 k) {  // 0xA68C4A
  Enemy_SpikeyPlatform *E = Get_SpikeyPlatform(k);
  uint16 v2 = E->spm_var_03 - 1;
  E->spm_var_03 = v2;
  if (!v2)
    E->spm_var_A = FUNC16(SpikeyPlatform_Func_4);
}

void SpikeyPlatform_Func_4(void) {  // 0xA68C5D
  Enemy_SpikeyPlatform *E = Get_SpikeyPlatform(cur_enemy_index);
  E->spm_var_01 = E->base.y_pos;
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, -0x8000);
  if ((int16)(E->base.y_pos - E->spm_var_C) < 0) {
    E->spm_var_A = FUNC16(SpikeyPlatform_Func_1);
    E->base.y_pos = E->spm_var_C;
  }
  if (SpikeyPlatform_Func_5(cur_enemy_index))
    extra_samus_y_displacement += E->base.y_pos - E->spm_var_01;
}

uint16 SpikeyPlatform_Func_5(uint16 k) {  // 0xA68CA1
  Enemy_SpikeyPlatform *E = Get_SpikeyPlatform(k);
  uint16 v2 = abs16(samus_x_pos - E->base.x_pos);
  bool v3 = v2 < samus_x_radius;
  uint16 v4 = v2 - samus_x_radius;
  if (!v3 && v4 >= E->base.x_width)
    return 0;
  if ((int16)(samus_y_pos + 5 - E->base.y_pos) < 0) {
    uint16 v6 = E->base.y_pos - (samus_y_pos + 5);
    v3 = v6 < samus_y_radius;
    uint16 v7 = v6 - samus_y_radius;
    if (v3 || v7 == E->base.y_height || v7 < E->base.y_height)
      return -1;
  }
  return 0;
}
