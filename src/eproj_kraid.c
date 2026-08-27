// Kraid and Mini-Kraid enemy-projectile families split out of sm_86.c.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "eproj_internal.h"

void sub_869DA5(uint16 k);

void EprojInit_RocksKraidSpits(uint16 j) {  // 0x869CA3
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  int v2 = j >> 1;
  eproj_x_pos[v2] = v1->x_pos + 16;
  eproj_y_pos[v2] = v1->y_pos - 96;
  eproj_y_subpos[v2] = 0;
  eproj_x_subpos[v2] = 0;
  eproj_x_vel[v2] = eproj_init_param_1;
  eproj_y_vel[v2] = -1024;
  eproj_gfx_idx[v2] = 1536;
}

void EprojInit_RocksFallingKraidCeiling(uint16 j) {  // 0x869CD8
  int v1 = j >> 1;
  eproj_x_pos[v1] = eproj_init_param_1;
  eproj_y_pos[v1] = 312;
  eproj_y_subpos[v1] = 0;
  eproj_x_subpos[v1] = 0;
  eproj_x_vel[v1] = 0;
  eproj_y_vel[v1] = (random_number & 0x3F) + 64;
  eproj_gfx_idx[v1] = 1536;
}

void EprojInit_RocksWhenKraidRises(uint16 j) {  // 0x869D0C
  int16 v1;

  v1 = random_number & 0x3F;
  if ((random_number & 1) == 0)
    v1 = ~v1;
  int v2 = j >> 1;
  eproj_x_pos[v2] = gEnemyData(cur_enemy_index)->x_pos + v1;
  eproj_y_pos[v2] = 432;
  eproj_y_subpos[v2] = 0;
  eproj_x_subpos[v2] = 0;
  eproj_x_vel[v2] = eproj_init_param_1;
  eproj_y_vel[v2] = -1280;
  eproj_gfx_idx[v2] = 1536;
  QueueSfx3_Max6(0x1E);
}

void EprojPreInstr_KraidRocks(uint16 k) {  // 0x869D56
  if (EprojBlockCollisition_Horiz(k) & 1 || EprojBlockCollisition_Vertical(k) & 1) {
    eproj_id[k >> 1] = 0;
  } else {
    int v1 = k >> 1;
    uint16 v2 = eproj_x_vel[v1] + 8;
    eproj_x_vel[v1] = v2;
    if (!sign16(v2 - 256))
      v2 = -256;
    eproj_x_vel[v1] = v2;
    eproj_y_vel[v1] += 64;
  }
}

void EprojPreInstr_RocksFallingKraidCeiling(uint16 k) {  // 0x869D89
  if (EprojBlockCollisition_Vertical(k) & 1)
    eproj_id[k >> 1] = 0;
  else
    eproj_y_vel[k >> 1] = (eproj_y_vel[k >> 1] + 24) & 0x3FFF;
}

void sub_869DA5(uint16 k) {  // 0x869DA5
  eproj_gfx_idx[k >> 1] = 0;
}

void EprojInit_MiniKraidSpit(uint16 j) {  // 0x869DEC
  ExtraEnemyRam7800 *v2;

  EnemyData *v1 = gEnemyData(cur_enemy_index);
  v2 = gExtraEnemyRam7800(cur_enemy_index);
  int v3 = j >> 1;
  eproj_x_pos[v3] = v2->kraid.field_4 + v1->x_pos;
  eproj_y_pos[v3] = v1->y_pos - 16;
  eproj_y_subpos[v3] = 0;
  eproj_x_subpos[v3] = 0;
  eproj_x_vel[v3] = v2->kraid.kraid_next;
  eproj_y_vel[v3] = v2->kraid.field_2;
}

void EprojPreInit_MiniKraidSpit(uint16 k) {  // 0x869E1E
  int16 v2;

  if (EprojBlockCollisition_Horiz(k) & 1 || EprojBlockCollisition_Vertical(k) & 1) {
    eproj_id[k >> 1] = 0;
  } else {
    int v1 = k >> 1;
    v2 = eproj_y_vel[v1] + 64;
    if (v2 >= 0 && !sign16(eproj_y_vel[v1] - 960))
      v2 = 1024;
    eproj_y_vel[v1] = v2;
  }
}

static const int16 kEprojInit_MiniKraidSpikes_Tab0[3] = { -2, 12, 24 };

static void EprojInit_MiniKraidSpikes(uint16 j, uint16 a) {  // 0x869E4E
  int v2 = j >> 1;
  eproj_x_vel[v2] = a;
  uint16 r18 = kEprojInit_MiniKraidSpikes_Tab0[gExtraEnemyRam7800(cur_enemy_index)->kraid.kraid_healths_8ths[0] >> 1];
  EnemyData *v3 = gEnemyData(cur_enemy_index);
  eproj_x_pos[v2] = v3->x_pos;
  eproj_y_pos[v2] = r18 + v3->y_pos;
  eproj_y_subpos[v2] = 0;
  eproj_x_subpos[v2] = 0;
  eproj_y_vel[v2] = 0;
}

void EprojInit_MiniKraidSpikesLeft(uint16 j) {  // 0x869E46
  EprojInit_MiniKraidSpikes(j, -0x200);
}

void EprojInit_MiniKraidSpikesRight(uint16 j) {  // 0x869E4B
  EprojInit_MiniKraidSpikes(j, 0x200);
}

void EprojPreInstr_MiniKraidSpikes(uint16 k) {  // 0x869E83
  if (EprojBlockCollisition_Horiz(k) & 1)
    eproj_id[k >> 1] = 0;
}
