// Crocomire enemy-projectile family split out of sm_86.c.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "eproj_internal.h"

#define word_869105 ((uint16*)RomFixedPtr(0x869105))
#define g_word_869059 ((uint16*)RomFixedPtr(0x869059))

void sub_8690B3(uint16 k);

void EprojInit_CrocomireProjectile(uint16 j) {  // 0x869023
  int v2 = j >> 1;
  eproj_x_vel[v2] = -512;
  eproj_y_vel[v2] = 1;
  EnemyData *v3 = gEnemyData(cur_enemy_index);
  eproj_x_pos[v2] = v3->x_pos - 32;
  eproj_y_pos[v2] = v3->y_pos - 16;
  eproj_timers[v2] = 0;
  eproj_x_subpos[v2] = 0;
  eproj_y_subpos[v2] = 0;
  eproj_gfx_idx[v2] = 2560;
}

static const int16 word_869059[9] = { -16, 0, 32, -16, 0, 32, -16, 0, 32 }; // bug: oob read
void EprojPreInstr_CrocomireProjectile(uint16 k) {  // 0x86906B
  EprojBlockCollisition_Horiz(k);
  eproj_gfx_idx[0] = 2560;
  eproj_timers[k >> 1] += eproj_x_vel[k >> 1];
  uint16 x = -64;
  uint16 y = g_word_869059[enemy_data[0].ai_preinstr >> 1]; // bug: out of bounds read...
  int v1 = CalculateAngleFromXY(x, y);
  int v2 = k >> 1;
  eproj_x_vel[v2] = 4 * kSinCosTable8bit_Sext[v1 + 64];
  eproj_y_vel[v2] = 4 * kSinCosTable8bit_Sext[v1];
  eproj_pre_instr[v2] = FUNC16(sub_8690B3);
}

void sub_8690B3(uint16 k) {  // 0x8690B3
  if (EprojBlockCollisition_Horiz(k) & 1 || EprojBlockCollisition_Vertical(k) & 1)
    eproj_id[k >> 1] = 0;
}

void EprojInit_CrocomireSpikeWallPieces(uint16 j) {  // 0x8690CF
  int v1 = j >> 1;
  eproj_y_pos[v1] = word_869105[(uint16)(j - 20) >> 1];
  eproj_x_pos[v1] = 528;
  eproj_x_vel[v1] = 0;
  eproj_y_vel[v1] = 0;
  eproj_E[v1] = 0;
  eproj_F[v1] = 0;
  eproj_x_subpos[v1] = 0;
  eproj_y_subpos[v1] = 0;
  eproj_y_vel[v1] = -5;
  eproj_F[v1] = -30720;
}

static const uint16 CrocomireSpikeWallPieces_Tab2[18] = {  // 0x869115
      0,     0, 0xff0, 0xee0, 0xcc0, 0xaa0, 0x880, 0x660, 0x440, 0x220, 0xff0, 0xee0,
  0xcc0, 0xaa0, 0x880, 0x660, 0x440, 0x220,
};
static const uint16 CrocomireSpikeWallPieces_Tab1[18] = {
       0,      0, 0xff00, 0xee00, 0xcc00, 0xaa00, 0x8800, 0x6600, 0x4400, 0x2200, 0xff00, 0xee00,
  0xcc00, 0xaa00, 0x8800, 0x6600, 0x4400, 0x2200,
};
static const int8 CrocomireSpikeWallPieces_Tab3[36] = {
  0, 0, 0, 0, 4, 0, 4, 0, 3, 0, 3, 0,
  2, 0, 2, 0, 1, 0, 1, 0, 6, 0, 5, 0,
  4, 0, 3, 0, 2, 0, 2, 0, 1, 0, 1, 0,
};

void EprojPreInstr_CrocomireSpikeWallPieces(uint16 k) {
  int8 v3;
  int8 v6;
  int8 v7;
  int8 v8;

  int v1 = k >> 1;
  uint16 v2 = eproj_E[v1];
  if (v2 != CrocomireSpikeWallPieces_Tab1[v1]) {
    v2 += CrocomireSpikeWallPieces_Tab2[v1];
    if (v2 >= CrocomireSpikeWallPieces_Tab1[v1])
      v2 = CrocomireSpikeWallPieces_Tab1[v1];
  }
  eproj_E[v1] = v2;
  v3 = *((uint8 *)eproj_E + k + 1);
  bool v4 = __CFADD__uint8(*((uint8 *)eproj_x_vel + k), v3);
  *((uint8 *)eproj_x_vel + k) += v3;
  uint8 v5 = v4 + *((uint8 *)eproj_x_vel + k + 1);
  if ((int8)(v5 - CrocomireSpikeWallPieces_Tab3[k]) >= 0)
    v5 = CrocomireSpikeWallPieces_Tab3[k];
  *((uint8 *)eproj_x_vel + k + 1) = v5;
  v6 = *((uint8 *)eproj_x_vel + k);
  v4 = __CFADD__uint8(*((uint8 *)eproj_x_subpos + k + 1), v6);
  *((uint8 *)eproj_x_subpos + k + 1) += v6;
  v7 = *((uint8 *)eproj_x_vel + k + 1);
  bool v9 = v4;
  v4 = __CFADD__uint8(v4, v7);
  v8 = v9 + v7;
  v4 |= __CFADD__uint8(*((uint8 *)eproj_x_pos + k), v8);
  *((uint8 *)eproj_x_pos + k) += v8;
  *((uint8 *)eproj_x_pos + k + 1) += v4;
  AddToHiLo(&eproj_y_vel[v1], &eproj_F[v1], 0x3000);
  AddToHiLo(&eproj_y_pos[v1], &eproj_y_subpos[v1], __PAIR32__(eproj_y_vel[v1], eproj_F[v1]));

  if (eproj_y_pos[v1] >= 0xA8) {
    eproj_id[v1] = 0;
    if ((k & 2) == 0)
      QueueSfx2_Max6(0x29);
    int v12 = k >> 1;
    eproj_spawn_pt = (Point16U) { eproj_x_pos[v12], eproj_y_pos[v12] };
    SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 0x15);
    QueueSfx2_Max6(0x25);
  }
}

const uint8 *EprojInstr_9270(uint16 k, const uint8 *epjp) {  // 0x869270
  int v1 = k >> 1;
  eproj_spawn_pt = (Point16U) { eproj_x_pos[v1], eproj_y_pos[v1] };
  SpawnEnemyDrops(addr_kEnemyDef_DDBF, k, 0);
  return epjp;
}

void EprojInit_CrocomireBridgeCrumbling(uint16 j) {  // 0x869286
  int v1 = j >> 1;
  eproj_x_pos[v1] = eproj_init_param_1;
  eproj_y_pos[v1] = 187;
  eproj_y_subpos[v1] = 0;
  eproj_x_subpos[v1] = 0;
  eproj_x_vel[v1] = 0;
  eproj_y_vel[v1] = (random_number & 0x3F) + 64;
  eproj_gfx_idx[v1] = 1024;
}

void EprojPreInstr_CrocomireBridgeCrumbling(uint16 k) {  // 0x8692BA
  if (EprojBlockCollisition_Vertical(k) & 1)
    eproj_id[k >> 1] = 0;
  else
    eproj_y_vel[k >> 1] = (eproj_y_vel[k >> 1] + 24) & 0x3FFF;
}
