// Botwoon and Yapping Maw enemy-projectile families split out of sm_86.c.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "eproj_internal.h"

#define kCommonEnemySpeeds_Quadratic32 ((uint32*)RomFixedPtr(0xa0cbc7))
#define kEprojInit_BotwoonsBody_InstrLists ((uint16*)RomFixedPtr(0x86e9f1))

void sub_86EC0C(uint16 k);

void EprojInit_BotwoonsBody(uint16 j) {  // 0x86EA31
  int v1 = j >> 1;
  eproj_x_pos[v1] = enemy_data[0].x_pos;
  eproj_y_pos[v1] = enemy_data[0].y_pos;
  eproj_y_vel[v1] = 0;
  uint16 v2 = enemy_data[0].ai_var_A ? 16 : 48;
  uint16 v3 = kEprojInit_BotwoonsBody_InstrLists[v2 >> 1];
  eproj_instr_list_ptr[v1] = v3;
  eproj_F[v1] = v3;
  eproj_E[v1] = v2;
  eproj_x_vel[v1] = FUNC16(Eproj_BotwoonsBody_Main);
  ExtraEnemyRam7800 *Ex = gExtraEnemyRam7800(enemy_data[0].ai_var_A);
  Ex->kraid.kraid_next = j;
  eproj_flags[v1] = 2;
  Ex->kraid.kraid_healths_4ths[0] = 1;
}

void CallBotwoonEprojFunc(uint32 ea, uint16 k) {
  switch (ea) {
  case fnEproj_BotwoonsBody_Main: Eproj_BotwoonsBody_Main(k); return;
  case fnEproj_BotwonsBodyFunction_Dying: Eproj_BotwonsBodyFunction_Dying(k); return;
  case fnEproj_BotwonsBodyFunction_Dying2: Eproj_BotwonsBodyFunction_Dying2(k); return;
  case fnEproj_BotwonsBodyFunction_DyingFalling: Eproj_BotwonsBodyFunction_DyingFalling(k); return;
  case fnnullsub_101: return;
  default: Unreachable();
  }
}

void EprojPreInstr_BotwoonsBody(uint16 k) {  // 0x86EA80
  if (*(uint16 *)&extra_enemy_ram8000[0].pad[32]) {
    int v1 = k >> 1;
    if (eproj_x_vel[v1] == FUNC16(Eproj_BotwoonsBody_Main))
      eproj_x_vel[v1] = FUNC16(Eproj_BotwonsBodyFunction_Dying);
  }
  CallBotwoonEprojFunc(eproj_x_vel[k >> 1] | 0x860000, k);
}

void Eproj_BotwoonsBody_Main(uint16 k) {  // 0x86EA98
  int v1 = k >> 1;
  uint16 v2 = kEprojInit_BotwoonsBody_InstrLists[eproj_E[v1] >> 1];
  if (v2 != eproj_F[v1]) {
    eproj_instr_list_ptr[v1] = v2;
    eproj_F[v1] = v2;
    eproj_instr_timers[v1] = 1;
  }
  Eproj_BotwoonsBodyHurtFlashHandling1(k);
}

void Eproj_BotwoonsBodyHurtFlashHandling1(uint16 k) {  // 0x86EAB4
  int v1 = k >> 1;
  eproj_gfx_idx[v1] |= 0xE00;
  if (enemy_data[0].flash_timer) {
    if ((random_enemy_counter & 2) != 0)
      eproj_gfx_idx[v1] &= 0xF1FF;
  }
}

void Eproj_BotwoonsBodyHurtFlashHandling2(uint16 j) {  // 0x86EAD4
  int v1 = j >> 1;
  eproj_gfx_idx[v1] |= 0xE00;
  if (enemy_data[0].flash_timer) {
    if ((random_enemy_counter & 2) != 0)
      eproj_gfx_idx[v1] &= 0xF1FF;
  }
}

void Eproj_BotwonsBodyFunction_Dying(uint16 v0) {  // 0x86EAF4
  int v1 = v0 >> 1;
  eproj_E[v1] = 4 * v0 + 96;
  eproj_x_vel[v1] = 0xEB04;
  Eproj_BotwonsBodyFunction_Dying2(v0);
}

void Eproj_BotwonsBodyFunction_Dying2(uint16 v0) {  // 0x86EB04
  int v1 = v0 >> 1;
  if (!sign16(++eproj_E[v1] - 256))
    eproj_x_vel[v1] = FUNC16(Eproj_BotwonsBodyFunction_DyingFalling);
  eproj_instr_timers[v1] = 0;
  Eproj_BotwoonsBodyHurtFlashHandling1(v0);
}

void Eproj_BotwonsBodyFunction_DyingFalling(uint16 v0) {  // 0x86EB1F
  int v1 = v0 >> 1;
  int v3 = (8 * ((eproj_y_vel[v1] & 0xFF00) >> 8)) >> 1;
  AddToHiLo(&eproj_y_pos[v1], &eproj_y_subpos[v1], kCommonEnemySpeeds_Quadratic32[v3 >> 1]);

  if (sign16(eproj_y_pos[v1] - 200)) {
    int v7 = v0 >> 1;
    eproj_y_vel[v7] += 192;
    eproj_instr_timers[v7] = 0;
    Eproj_BotwoonsBodyHurtFlashHandling2(v0);
  } else {
    eproj_y_pos[v1] = 200;
    eproj_x_vel[v1] = FUNC16(nullsub_101);
    eproj_instr_list_ptr[v1] = addr_word_86E208;
    eproj_gfx_idx[v1] = 2560;
    eproj_instr_timers[v1] = 1;
    QueueSmallExplosionSfx();
    if (v0 == 10)
      *(uint16 *)&extra_enemy_ram8800[0].pad[62] = 1;
  }
}

void QueueSmallExplosionSfx(void) {  // 0x86EB94
  QueueSfx2_Max6(0x24);
}

void EprojInit_BotwoonsSpit(uint16 j) {  // 0x86EBC6
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  int v2 = j >> 1;
  eproj_x_pos[v2] = v1->x_pos;
  eproj_y_pos[v2] = v1->y_pos;
  eproj_instr_list_ptr[v2] = addr_word_86EBAE;
  g_word_7E97DC[v2] = eproj_init_param_3;
  Point32 pt = ConvertAngleToXy(eproj_init_param_3, eproj_init_param_1);
  eproj_x_vel[v2] = pt.x >> 16;
  eproj_E[v2] = pt.x;
  eproj_y_vel[v2] = pt.y >> 16;
  eproj_F[v2] = pt.y;
}

void EprojPreInstr_BotwoonsSpit(uint16 k) {  // 0x86EC05
  Eproj_FuncE73E_MoveXY(k);
  sub_86EC0C(k);
}

void sub_86EC0C(uint16 k) {  // 0x86EC0C
  if (sub_86EC18(k))
    eproj_id[k >> 1] = 0;
}

uint16 sub_86EC18(uint16 k) {  // 0x86EC18
  int v1 = k >> 1;
  return (int16)(eproj_x_pos[v1] - layer1_x_pos) < 0
    || (int16)(layer1_x_pos + 256 - eproj_x_pos[v1]) < 0
    || (int16)(eproj_y_pos[v1] - layer1_y_pos) < 0
    || (int16)(layer1_y_pos + 256 - eproj_y_pos[v1]) < 0;
}

void EprojInit_YappingMawsBody(uint16 j) {  // 0x86EC62
  EnemyData *v2 = gEnemyData(cur_enemy_index);
  int v3 = j >> 1;
  eproj_x_pos[v3] = v2->x_pos;
  eproj_y_pos[v3] = v2->y_pos;
  eproj_instr_list_ptr[v3] = addr_word_86EC5C;
  if (!v2->parameter_2)
    eproj_instr_list_ptr[v3] = addr_word_86EC56;
  *(uint16 *)&extra_enemy_ram8800[0].pad[(uint16)(cur_enemy_index
                                                 + 2 * *(uint16 *)&extra_enemy_ram8800[0].pad[cur_enemy_index + 8])] = j;
}
