// Enemy AI - Spikey platform — peeled from Bank $A6
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"

enum {
  kSpikeyPlatform2ndYOffset = 12,
  kSpikeyPlatformRiseWait = 64,
  kSpikeyPlatformMaxSpeedIndex = 504,
  kSpikeyPlatformTerminalSpeedIndex = 512,
  kSpikeyPlatformSpeedStep = 8,
  kSpikeyPlatformRiseSpeed = -0x8000,
  kSpikeyPlatformSamusYSlack = 5,
  kSfx2_SpikeyPlatformStop = 0x1B,
};

void SpikeyPlatform_Init(void) {  // 0xA68B2F
  Enemy_SpikeyPlatform *E = Get_SpikeyPlatform(cur_enemy_index);
  E->base.current_instruction = addr_kSpikeyPlatform_Ilist_8B29;
  E->spm_var_A = FUNC16(SpikeyPlatform_Func_1);
  uint16 speed_idx = 8 * LOBYTE(E->spm_parameter_1);
  E->spm_var_02 = speed_idx;
  int v = speed_idx >> 1;
  E->spm_var_E = kCommonEnemySpeeds_Linear[v + 1];
  E->spm_var_F = kCommonEnemySpeeds_Linear[v];
  E->spm_var_D = E->base.y_pos + HIBYTE(E->spm_parameter_2);
  E->spm_var_C = E->base.y_pos;
  uint16 wait = LOBYTE(E->spm_parameter_2);
  E->spm_var_00 = wait;
  E->spm_var_B = wait;
}

void SpikeyPlatform2ndEnemy_Init(void) {  // 0xA68B85
  int idx = cur_enemy_index >> 1;
  EnemyData *E = gEnemyData(cur_enemy_index);
  E->x_pos = enemy_drawing_queue[idx + 91];
  E->y_pos = enemy_drawing_queue[idx + 93] + kSpikeyPlatform2ndYOffset;
}

void SpikeyPlatform2ndEnemy_Main(void) {  // 0xA68B99
  int idx = cur_enemy_index >> 1;
  EnemyData *E = gEnemyData(cur_enemy_index);
  E->x_pos = enemy_drawing_queue[idx + 91];
  E->y_pos = enemy_drawing_queue[idx + 93] + kSpikeyPlatform2ndYOffset;
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
  int v = E->spm_var_02 >> 1;
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(kCommonEnemySpeeds_Linear[v], kCommonEnemySpeeds_Linear[v + 1]));
  if ((int16)(E->base.y_pos - E->spm_var_D) >= 0) {
    E->spm_var_03 = kSpikeyPlatformRiseWait;
    E->spm_var_A = FUNC16(SpikeyPlatform_Func_3);
    E->base.y_pos = E->spm_var_D;
    QueueSfx2_Max6(kSfx2_SpikeyPlatformStop);
  }
  if (SpikeyPlatform_Func_5(cur_enemy_index))
    extra_samus_y_displacement += E->base.y_pos - E->spm_var_01;
  int16 next_speed = E->spm_var_02 + kSpikeyPlatformSpeedStep;
  if (!sign16(E->spm_var_02 - kSpikeyPlatformMaxSpeedIndex))
    next_speed = kSpikeyPlatformTerminalSpeedIndex;
  E->spm_var_02 = next_speed;
}

void SpikeyPlatform_Func_3(uint16 k) {  // 0xA68C4A
  Enemy_SpikeyPlatform *E = Get_SpikeyPlatform(k);
  uint16 wait = E->spm_var_03 - 1;
  E->spm_var_03 = wait;
  if (!wait)
    E->spm_var_A = FUNC16(SpikeyPlatform_Func_4);
}

void SpikeyPlatform_Func_4(void) {  // 0xA68C5D
  Enemy_SpikeyPlatform *E = Get_SpikeyPlatform(cur_enemy_index);
  E->spm_var_01 = E->base.y_pos;
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, kSpikeyPlatformRiseSpeed);
  if ((int16)(E->base.y_pos - E->spm_var_C) < 0) {
    E->spm_var_A = FUNC16(SpikeyPlatform_Func_1);
    E->base.y_pos = E->spm_var_C;
  }
  if (SpikeyPlatform_Func_5(cur_enemy_index))
    extra_samus_y_displacement += E->base.y_pos - E->spm_var_01;
}

uint16 SpikeyPlatform_Func_5(uint16 k) {  // 0xA68CA1
  Enemy_SpikeyPlatform *E = Get_SpikeyPlatform(k);
  uint16 dx = abs16(samus_x_pos - E->base.x_pos);
  bool closer_x = dx < samus_x_radius;
  uint16 x_gap = dx - samus_x_radius;
  if (!closer_x && x_gap >= E->base.x_width)
    return 0;
  if ((int16)(samus_y_pos + kSpikeyPlatformSamusYSlack - E->base.y_pos) < 0) {
    uint16 dy = E->base.y_pos - (samus_y_pos + kSpikeyPlatformSamusYSlack);
    bool closer_y = dy < samus_y_radius;
    uint16 y_gap = dy - samus_y_radius;
    if (closer_y || y_gap == E->base.y_height || y_gap < E->base.y_height)
      return -1;
  }
  return 0;
}
