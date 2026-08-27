// Skree enemy-projectile family split out of sm_86.c.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "eproj_internal.h"

#define off_868A75 ((uint16*)RomFixedPtr(0x868a75))

void Eproj_Init_0x8aaf(uint16 j) {  // 0x868A39
  NextRandom();
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  uint16 v2 = v1->y_pos + 12;
  int v3 = j >> 1;
  eproj_y_pos[v3] = v2;
  eproj_timers[v3] = v2 + 72;
  eproj_y_vel[v3] = v1->ai_var_D;
  eproj_x_pos[v3] = v1->x_pos + (random_number & 0x1F) - 16;
  eproj_instr_list_ptr[v3] = off_868A75[(uint16)(HIBYTE(eproj_y_vel[v3]) & 6) >> 1];
}

void Eproj_PreInit_0x8aaf(uint16 k) {  // 0x868A7D
  int v1 = k >> 1;
  AddToHiLo(&eproj_y_pos[v1], &eproj_y_subpos[v1], INT16_SHL8(eproj_y_vel[v1]));

  if (eproj_y_pos[v1] >= eproj_timers[v1])
    eproj_id[v1] = 0;
}

void Eproj_Init_0x8bc2_SkreeDownRight(uint16 j) {  // 0x868ACD
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  int v2 = j >> 1;
  eproj_y_pos[v2] = v1->y_pos;
  eproj_y_vel[v2] = -769;
  eproj_x_pos[v2] = v1->x_pos + 6;
  eproj_x_vel[v2] = 320;
}

void Eproj_Init_0x8bd0_SkreeUpRight(uint16 j) {  // 0x868AF1
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  int v2 = j >> 1;
  eproj_y_pos[v2] = v1->y_pos;
  eproj_y_vel[v2] = -1025;
  eproj_x_pos[v2] = v1->x_pos + 6;
  eproj_x_vel[v2] = 96;
}

void Eproj_Init_0x8bde_SkreeDownLeft(uint16 j) {  // 0x868B15
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  int v2 = j >> 1;
  eproj_y_pos[v2] = v1->y_pos;
  eproj_y_vel[v2] = -769;
  eproj_x_pos[v2] = v1->x_pos - 6;
  eproj_x_vel[v2] = -320;
}

void Eproj_Init_0x8bec_SkreeUpLeft(uint16 j) {  // 0x868B39
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  int v2 = j >> 1;
  eproj_y_pos[v2] = v1->y_pos;
  eproj_y_vel[v2] = -1025;
  eproj_x_pos[v2] = v1->x_pos - 6;
  eproj_x_vel[v2] = -96;
}

void Eproj_PreInstr_SkreeParticle(uint16 k) {  // 0x868B5D
  int v1 = k >> 1;
  
  AddToHiLo(&eproj_x_pos[v1], &eproj_x_subpos[v1], INT16_SHL8(eproj_x_vel[v1]));
  AddToHiLo(&eproj_y_pos[v1], &eproj_y_subpos[v1], INT16_SHL8(eproj_y_vel[v1]));

  eproj_y_vel[v1] += 80;
  if (CheckIfEprojIsOffScreen(k))
    eproj_id[v1] = 0;
}
