// Enemy projectile combat, boss, and pickup clusters split out of sm_86.c.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "eproj_internal.h"

#define kAlignPos_Tab1 ((uint8*)RomFixedPtr(0x94892b))
#define off_868A75 ((uint16*)RomFixedPtr(0x868a75))
#define word_869105 ((uint16*)RomFixedPtr(0x869105))
#define off_86A64D ((uint16*)RomFixedPtr(0x86a64d))
#define off_86BB1E ((uint16*)RomFixedPtr(0x86bb1e))
#define kCommonEnemySpeeds_Quadratic_Copy ((uint16*)RomFixedPtr(0xa0cbc7))
#define kCommonEnemySpeeds_Quadratic32 ((uint32*)RomFixedPtr(0xa0cbc7))
#define off_86C040 ((uint16*)RomFixedPtr(0x86c040))
#define g_word_869059 ((uint16*)RomFixedPtr(0x869059))
#define off_86A2E2 ((uint16*)RomFixedPtr(0x86a2e2))
#define kEprojInit_BombTorizoStatueBreaking_InstrList ((uint16*)RomFixedPtr(0x86a7ab))
#define kEprojInit_N00bTubeShards_InstrPtrs ((uint16*)RomFixedPtr(0x86d760))
#define off_86D96A ((uint16*)RomFixedPtr(0x86d96a))
#define kSporeMovementData ((uint8*)RomFixedPtr(0x86dd6c))
#define word_86DEB6 ((uint16*)RomFixedPtr(0x86deb6))
#define off_86E42C ((uint16*)RomFixedPtr(0x86e42c))
#define word_86E47E ((uint16*)RomFixedPtr(0x86e47e))
#define kEprojInit_BotwoonsBody_InstrLists ((uint16*)RomFixedPtr(0x86e9f1))
#define off_86EF04 ((uint16*)RomFixedPtr(0x86ef04))
#define off_86EFD5 ((uint16*)RomFixedPtr(0x86efd5))
#define kEnemyDef_F3D3 (*(EnemyDef_B2*)RomFixedPtr(0xa0f3d3))
#define g_word_86F3D4 ((uint16*)RomFixedPtr(0x86f3d4))  // bug:: oob read

void sub_8690B3(uint16 k);
void sub_869DA5(uint16 k);
void sub_86A301(uint16 j);
void sub_86A887(uint16 k);
void sub_86A91A(uint16 v0);
void sub_86BB30(uint16 j);
void sub_86CFD5(uint16 k);
void sub_86CFE6(uint16 k);
void sub_86CFF8(uint16 k);
void sub_86D7BF(uint16 k);
void sub_86D7DE(uint16 k);
void sub_86D83D(uint16 k);
void sub_86D89F(uint16 k);
void sub_86D8DF(uint16 k);
void sub_86D992(uint16 k);
void sub_86DCC3(uint16 j);
void sub_86EC0C(uint16 k);

static void Eproj_Init_0x8aaf(uint16 j) {  // 0x868A39
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

static void Eproj_Init_0x8bc2_SkreeDownRight(uint16 j) {  // 0x868ACD
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  int v2 = j >> 1;
  eproj_y_pos[v2] = v1->y_pos;
  eproj_y_vel[v2] = -769;
  eproj_x_pos[v2] = v1->x_pos + 6;
  eproj_x_vel[v2] = 320;
}

static void Eproj_Init_0x8bd0_SkreeUpRight(uint16 j) {  // 0x868AF1
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  int v2 = j >> 1;
  eproj_y_pos[v2] = v1->y_pos;
  eproj_y_vel[v2] = -1025;
  eproj_x_pos[v2] = v1->x_pos + 6;
  eproj_x_vel[v2] = 96;
}

static void Eproj_Init_0x8bde_SkreeDownLeft(uint16 j) {  // 0x868B15
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  int v2 = j >> 1;
  eproj_y_pos[v2] = v1->y_pos;
  eproj_y_vel[v2] = -769;
  eproj_x_pos[v2] = v1->x_pos - 6;
  eproj_x_vel[v2] = -320;
}

static void Eproj_Init_0x8bec_SkreeUpLeft(uint16 j) {  // 0x868B39
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

const uint8 *EprojInstr_SpawnEnemyDropsWithDraygonsEyeDrops(uint16 k, const uint8 *epjp) {  // 0x868C68
  int v1 = k >> 1;
  eproj_spawn_pt = (Point16U) { eproj_x_pos[v1], eproj_y_pos[v1] };
  SpawnEnemyDrops(addr_kEnemyDef_DE7F, k, 0);
  return epjp;
}

const uint8 *EprojInstr_SetPreInstrA(uint16 k, const uint8 *epjp) {  // 0x868CF6
  eproj_pre_instr[k >> 1] = FUNC16(EprojPreInstr_DraygonsTurret_8DFF);
  return epjp;
}

const uint8 *EprojInstr_SetPreInstrB(uint16 k, const uint8 *epjp) {  // 0x868CFD
  eproj_pre_instr[k >> 1] = FUNC16(EprojPreInstr_8DCA);
  return epjp;
}

static void EprojInit_DraygonsGunk(uint16 j) {  // 0x868D04
  int v1 = j >> 1;
  eproj_x_pos[v1] = eproj_spawn_pt.x;
  eproj_y_pos[v1] = eproj_spawn_pt.y;
  g_word_7E97DC[v1] = eproj_unk1995;
  Point32 pt = ConvertAngleToXy(eproj_unk1995, eproj_init_param_1);
  eproj_x_vel[v1] = pt.x >> 16;
  eproj_E[v1] = pt.x;
  eproj_y_vel[v1] = pt.y >> 16;
  eproj_F[v1] = pt.y;
  eproj_gfx_idx[v1] = 1024;
}

static void EprojInit_DraygonsWallTurretProjs(uint16 j) {  // 0x868D40
  Eproj_AngleToSamus(j, eproj_spawn_pt.x, eproj_spawn_pt.y);
  int v1 = j >> 1;
  eproj_gfx_idx[v1] = 2560;
  eproj_pre_instr[v1] = FUNC16(nullsub_84);
}

void EprojPowerBombCollision(uint16 k) {  // 0x868D5C
  uint16 r18 = HIBYTE(power_bomb_explosion_radius);
  if (HIBYTE(power_bomb_explosion_radius)) {
    uint16 r20 = (uint16)(r18 + (HIBYTE(power_bomb_explosion_radius) & 1) + (power_bomb_explosion_radius >> 9)) >> 1;
    int v1 = k >> 1;
    if (abs16(power_bomb_explosion_x_pos - eproj_x_pos[v1]) < r18
        && abs16(power_bomb_explosion_y_pos - eproj_y_pos[v1]) < r20) {
      eproj_id[v1] = 0;
      samus_x_speed_divisor = 0;
    }
  }
}

const uint8 *EprojInstr_868D99(uint16 k, const uint8 *epjp) {  // 0x868D99
  EprojPowerBombCollision(k);
  uint16 v2 = samus_x_speed_divisor + 1;
  if (sign16(samus_x_speed_divisor - 5)) {
    ++samus_x_speed_divisor;
    int v3 = k >> 1;
    eproj_F[v3] = v2;
    eproj_E[v3] = 256;
    eproj_pre_instr[v3] = FUNC16(EprojPreInstr_8DCA);
    eproj_properties[v3] = eproj_properties[v3] & 0x5FFF | 0x2000;
    samus_invincibility_timer = 0;
    samus_knockback_timer = 0;
  }
  return epjp;
}

void EprojPreInstr_8DCA(uint16 k) {  // 0x868DCA
  int v1;

  EprojPowerBombCollision(k);
  if (samus_contact_damage_index
      || (v1 = k >> 1,
          eproj_x_pos[v1] = samus_x_pos,
          eproj_y_pos[v1] = samus_y_pos + 4 * eproj_F[v1] - 12,
          --eproj_E[v1],
          !eproj_E[v1])) {
    eproj_id[k >> 1] = 0;
    if ((--samus_x_speed_divisor & 0x8000) != 0)
      samus_x_speed_divisor = 0;
  }
}

void EprojPreInstr_DraygonsTurret_8DFF(uint16 k) {  // 0x868DFF
  int16 v1;

  EprojPowerBombCollision(k);
  Eproj_FuncE73E_MoveXY(k);
  v1 = Eproj_FuncE722(k);
  if (v1)
    eproj_id[k >> 1] = 0;
}

void EprojPreInstr_DraygonsGunk_8E0F(uint16 k) {  // 0x868E0F
  uint16 v3;

  EprojPowerBombCollision(k);
  Eproj_FuncE73E_MoveXY(k);
  int v1 = k >> 1;
  uint16 v2 = abs16(samus_x_pos - eproj_x_pos[v1]);
  if (sign16(v2 - 16) && (v3 = abs16(samus_y_pos - eproj_y_pos[v1]), sign16(v3 - 20))) {
    eproj_instr_list_ptr[v1] = 0x8C38;
    eproj_instr_timers[v1] = 1;
  } else {
    if (Eproj_FuncE722(k))
      eproj_id[v1] = 0;
  }
}

static void EprojInit_CrocomireProjectile(uint16 j) {  // 0x869023
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

static void EprojInit_CrocomireSpikeWallPieces(uint16 j) {  // 0x8690CF
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

static void EprojInit_CrocomireBridgeCrumbling(uint16 j) {  // 0x869286
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

static const int16 kEprojInit_9634_Xvel[4] = { -0x200, -0x1f0, -0x1bc, -0x16a };
static const int16 kEprojInit_9634_Yvel[4] = { 0, 0x88, 0xfc, 0x16a };

static void EprojInit_9634(uint16 j) {  // 0x86934D
  int v1 = j >> 1;
  eproj_E[v1] = 0;
  eproj_F[v1] = 0;
  eproj_x_pos[v1] = enemy_data[0].x_pos - 29;
  eproj_y_pos[v1] = enemy_data[0].y_pos - 35;
  eproj_gfx_idx[v1] = 2560;
  int v2 = enemy_data[0].parameter_1;
  eproj_x_vel[v1] = kEprojInit_9634_Xvel[v2];
  eproj_y_vel[v1] = kEprojInit_9634_Yvel[v2];
}

void EprojPreInstr_9634(uint16 k) {  // 0x869392
  int v1 = k >> 1;
  if (eproj_E[v1] >= 8) {
    MoveEprojWithVelocity(k);
    if (EprojBlockCollisition_Vertical(k) & 1) {
      eproj_instr_list_ptr[v1] = addr_off_869574;
      ++eproj_E[v1];
      eproj_instr_timers[v1] = 1;
      eproj_x_vel[v1] = 0;
      eproj_y_vel[v1] = 0;
      eproj_gfx_idx[v1] = 2560;
      QueueSfx2_Max6(0x2B);
    }
  } else {
    ++eproj_E[v1];
  }
}

static void EprojInit_9642_RidleysFireball(uint16 j) {  // 0x8693CA
  int v1 = j >> 1;
  eproj_F[v1] = eproj_unk1995;
  int16 v2 = eproj_init_param_1 ? 25 : -25;
  eproj_x_pos[v1] = enemy_data[0].x_pos + v2;
  eproj_y_pos[v1] = enemy_data[0].y_pos - 43;
  eproj_gfx_idx[v1] = 2560;

  Ram7800_Default *v4 = gRam7800_Default(0);
  eproj_x_vel[v1] = v4->var_19;
  eproj_y_vel[v1] = v4->var_1A;
  SetAreaDependentEprojPropertiesEx(addr_kRidleysFireball_Tab0, j);
}

void EprojPreInstr_9642_RidleysFireball(uint16 k) {  // 0x86940E
  uint16 v1;
  if (EprojBlockCollisition_Horiz(k) & 1) {
    v1 = -27042;
  } else {
    if (!(EprojBlockCollisition_Vertical(k) & 1))
      return;
    v1 = -27056;
  }
  int v2 = k >> 1;
  eproj_id[v2] = 0;
  if (!eproj_F[v2]) {
    eproj_spawn_pt = (Point16U){ eproj_x_pos[v2], eproj_y_pos[v2] };
    SpawnEprojWithRoomGfx(v1, 3);
    QueueSfx2_Max6(0x2B);
  }
}

const uint8 *EprojInstr_DisableCollisionsWithSamus(uint16 k, const uint8 *epjp) {  // 0x869475
  eproj_properties[k >> 1] |= 0x2000;
  return epjp;
}

static void EprojInit_9660_FireballExplosion(uint16 j) {  // 0x86947F
  int v1 = j >> 1;
  eproj_gfx_idx[v1] = 2560;
  eproj_x_pos[v1] = eproj_spawn_pt.x;
  eproj_y_pos[v1] = eproj_spawn_pt.y;
  eproj_E[v1] = eproj_init_param_1;
  eproj_F[v1] = 0;
  eproj_x_vel[v1] = 0;
  eproj_y_vel[v1] = 0;
}

static void EprojInit_Common(uint16 j, uint16 k, uint16 a) {  // 0x8694EE
  int v3 = j >> 1;
  eproj_F[v3] = a;
  int v4 = k >> 1;
  eproj_x_pos[v3] = eproj_x_pos[v4];
  eproj_y_pos[v3] = eproj_y_pos[v4];
  eproj_E[v3] = eproj_E[v4];
  eproj_gfx_idx[v3] = 2560;
  SetAreaDependentEprojProperties(j);
}

static void EprojInit_9688(uint16 j) {  // 0x8694A0
  uint16 v1 = eproj_init_param_1;
  int v2 = j >> 1;
  eproj_x_vel[v2] = 0;
  eproj_y_vel[v2] = -3584;
  EprojInit_Common(j, v1, 0x9688);
}

static void EprojInit_9696(uint16 j) {  // 0x8694B4
  uint16 v1 = eproj_init_param_1;
  int v2 = j >> 1;
  eproj_x_vel[v2] = 0;
  eproj_y_vel[v2] = 3584;
  EprojInit_Common(j, v1, 0x9696);
}

static void EprojInit_966C(uint16 j) {  // 0x8694C8
  uint16 v1 = eproj_init_param_1;
  int v2 = j >> 1;
  eproj_x_vel[v2] = 3584;
  eproj_y_vel[v2] = 0;
  EprojInit_Common(j, v1, 0x966C);
}

static void EprojInit_967A(uint16 j) {  // 0x8694DC
  uint16 v1 = eproj_init_param_1;
  int v2 = j >> 1;
  eproj_x_vel[v2] = -3584;
  eproj_y_vel[v2] = 0;
  EprojInit_Common(j, v1, 0x967A);
}

void EprojPreInstr_966C(uint16 k) {  // 0x86950D
  MoveEprojWithVelocityX(k);
  if (EprojBlockCollisition_Vertical(k) & 1) {
    int v1 = k >> 1;
    eproj_instr_list_ptr[v1] = addr_off_869574;
    eproj_instr_timers[v1] = 1;
  }
}

void EprojPreInstr_9688(uint16 k) {  // 0x869522
  MoveEprojWithVelocityY(k);
  if (EprojBlockCollisition_Horiz(k) & 1) {
    int v1 = k >> 1;
    eproj_instr_list_ptr[v1] = addr_off_869574;
    eproj_instr_timers[v1] = 1;
  }
}

void EprojPreInstr_96A4(uint16 k) {  // 0x869537
  if (EprojBlockCollisition_Horiz(k) & 1)
    eproj_id[k >> 1] = 0;
}

void EprojPreInstr_96C0(uint16 k) {  // 0x869540
  if (EprojBlockCollisition_Vertical(k) & 1)
    eproj_id[k >> 1] = 0;
}

void EprojPreInstr_96CE(uint16 k) {  // 0x869549
  if (EprojBlockCollisition_Vertical(k) & 1)
    eproj_id[k >> 1] = 0;
}

const uint8 *EprojInstr_95BA(uint16 k, const uint8 *epjp) {  // 0x8695BA
  eproj_unk1995 = 0;
  SpawnEprojWithRoomGfx(addr_stru_86966C, k);
  eproj_unk1995 = 0;
  SpawnEprojWithRoomGfx(addr_stru_86967A, k);
  return epjp;
}

const uint8 *EprojInstr_95ED(uint16 k, const uint8 *epjp) {  // 0x8695ED
  eproj_unk1995 = 0;
  SpawnEprojWithRoomGfx(addr_stru_869688, k);
  eproj_unk1995 = 0;
  SpawnEprojWithRoomGfx(addr_stru_869696, k);
  return epjp;
}

const uint8 *EprojInstr_9620(uint16 k, const uint8 *epjp) {  // 0x869620
  if ((int8)-- * ((uint8 *)eproj_E + k) >= 0)
    SpawnEprojWithRoomGfx(eproj_F[k >> 1], k);
  return epjp;
}

static void EprojInit_9734_CeresFallingDebris(uint16 j) {  // 0x8696DC
  int v1 = j >> 1;
  eproj_E[v1] = 0;
  eproj_F[v1] = 0;
  eproj_x_vel[v1] = 0;
  eproj_gfx_idx[v1] = 3584;
  eproj_x_pos[v1] = eproj_init_param_1;
  eproj_y_pos[v1] = 42;
  eproj_y_vel[v1] = 16;
}

void EprojPreInstr_9734_CeresFallingDebris(uint16 k) {  // 0x869701
  int v1 = k >> 1;
  eproj_y_vel[v1] += 16;
  if (EprojBlockCollisition_Vertical(k) & 1) {
    eproj_id[v1] = 0;
    eproj_spawn_pt = (Point16U) { eproj_x_pos[v1], eproj_y_pos[v1] };
    SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 9);
    QueueSfx2_Max6(0x6D);
  }
}

const uint8 *EprojInstr_980E(uint16 k, const uint8 *epjp) {  // 0x86980E
  int v2 = k >> 1;
  eproj_spawn_pt = (Point16U) { eproj_x_pos[v2], eproj_y_pos[v2] };
  SpawnEnemyDrops(addr_kEnemyDef_E4FF, k, 0);
  return epjp;
}

static const uint8 byte_8698B4[16] = { 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0xf0, 0xe0, 0xd0, 0xc0, 0xb0, 0xa0, 0x90, 0x80 };
static const uint8 byte_8698F7[9] = { 0x30, 0x44, 0x58, 0x6c, 0x80, 0x94, 0xa8, 0xbc, 0xd0 };
static const uint8 byte_869979[8] = { 0, 0x20, 0x40, 0x60, 0x80, 0xa0, 0xc0, 0xe0 };


static void EprojInit_PhantoonDestroyableFireballs(uint16 j) {  // 0x869824
  int8 v1;

  v1 = (uint16)(eproj_init_param_1 & 0xFF00) >> 8;
  switch (v1) {
  case 0: {
    int v2 = j >> 1;
    eproj_x_subpos[v2] = 0;
    eproj_y_subpos[v2] = 0;
    eproj_x_vel[v2] = 0;
    eproj_y_vel[v2] = 0;
    eproj_x_pos[v2] = enemy_data[0].x_pos;
    eproj_y_pos[v2] = enemy_data[0].y_pos + 32;
    eproj_instr_list_ptr[v2] = addr_off_8697B4;
    eproj_properties[v2] = eproj_properties[v2] & 0xFFF | 0x2000;
    break;
  }
  case 2: {
    int v3 = j >> 1;
    eproj_x_subpos[v3] = 0;
    eproj_y_subpos[v3] = 0;
    eproj_y_vel[v3] = 0;
    uint16 v4 = (uint8)eproj_init_param_1;
    if ((int16)((uint8)eproj_init_param_1 - 8) >= 0)
      eproj_x_vel[v3] = -2;
    else
      eproj_x_vel[v3] = 2;
    eproj_E[v3] = byte_8698B4[v4];
    eproj_x_pos[v3] = enemy_data[0].x_pos;
    eproj_y_pos[v3] = enemy_data[0].y_pos + 32;
    eproj_pre_instr[v3] = FUNC16(EprojPreInstr_PhantoonDestroyableFireballs);
    break;
  }
  case 4: {
    int v5 = j >> 1;
    eproj_x_subpos[v5] = 0;
    eproj_y_subpos[v5] = 0;
    eproj_y_vel[v5] = 0;
    eproj_x_vel[v5] = (eproj_init_param_1 & 0xF0) >> 1;
    eproj_x_pos[v5] = byte_8698F7[eproj_init_param_1 & 0xF];
    eproj_y_pos[v5] = 40;
    eproj_pre_instr[v5] = FUNC16(EprojPreInstr_PhantoonDestroyableFireballs_2);
    break;
  }
  case 6: {
    int v7 = j >> 1;
    eproj_x_subpos[v7] = 0;
    eproj_y_subpos[v7] = 0;
    eproj_y_vel[v7] = 0;
    eproj_x_vel[v7] = 128;
    eproj_E[v7] = byte_869979[(uint8)eproj_init_param_1];
    eproj_x_pos[v7] = enemy_data[0].x_pos;
    eproj_y_pos[v7] = enemy_data[0].y_pos + 16;
    eproj_pre_instr[v7] = FUNC16(EprojPreInstr_PhantoonDestroyableFireballs_3);
    break;
  }
  default:
    Unreachable();
  }
}

static void EprojInit_PhantoonStartingFireballs(uint16 j) {  // 0x86993A
  int v1 = j >> 1;
  eproj_x_subpos[v1] = 0;
  eproj_y_subpos[v1] = 0;
  eproj_x_vel[v1] = 0;
  eproj_y_vel[v1] = 0;
  uint16 v2 = byte_869979[eproj_init_param_1];
  eproj_E[v1] = v2;
  Point16U pt = Eproj_PhantomFireballs_Func1(v2, 0x30);
  eproj_x_pos[v1] = pt.x + enemy_data[0].x_pos;
  eproj_y_pos[v1] = pt.y + enemy_data[0].y_pos + 16;
}

void EprojPreInstr_PhantoonStartingFireballs(uint16 k) {  // 0x869981
  int v1 = k >> 1;
  eproj_y_vel[v1] += 16;
  if (EprojBlockCollisition_Vertical(k) & 1) {
    eproj_properties[v1] = eproj_properties[v1] & 0xFFF | 0x8000;
    eproj_pre_instr[v1] = FUNC16(EprojPreInstr_PhantoonStartingFireballs2);
    eproj_instr_list_ptr[v1] = addr_word_86976C;
    eproj_instr_timers[v1] = 1;
    eproj_E[v1] = 8;
    eproj_y_pos[v1] += 8;
  }
}

void EprojPreInstr_PhantoonStartingFireballs2(uint16 k) {  // 0x8699BF
  int v1 = k >> 1;
  bool v2 = eproj_E[v1] == 1;
  bool v3 = (--eproj_E[v1] & 0x8000) != 0;
  if (v2 || v3) {
    eproj_pre_instr[v1] = FUNC16(EprojPreInstr_PhantoonStartingFireballs3);
    eproj_instr_list_ptr[v1] = addr_word_869772;
    eproj_instr_timers[v1] = 1;
    eproj_y_pos[v1] -= 8;
    eproj_y_vel[v1] = -768;
    eproj_E[v1] = 0;
    if ((nmi_frame_counter_word & 1) != 0)
      eproj_x_vel[v1] = -128;
    else
      eproj_x_vel[v1] = 128;
  }
}

static const uint16 word_869A3E[3] = { 0xfd00, 0xfe00, 0xff00 };

void EprojPreInstr_PhantoonStartingFireballs3(uint16 k) {  // 0x869A01
  int v1 = k >> 1;
  eproj_y_vel[v1] += 16;
  if (EprojBlockCollisition_Vertical(k) & 1) {
    uint16 v2 = eproj_E[v1] + 1;
    eproj_E[v1] = v2;
    if (sign16(v2 - 3)) {
      eproj_y_vel[v1] = word_869A3E[v2];
      return;
    }
    goto LABEL_6;
  }
  if (EprojBlockCollisition_Horiz(k) & 1) {
LABEL_6:
    eproj_instr_list_ptr[v1] = addr_word_869782;
    eproj_instr_timers[v1] = 1;
    eproj_pre_instr[v1] = FUNC16(nullsub_86);
  }
}

void EprojPreInstr_PhantoonDestroyableFireballs(uint16 k) {  // 0x869A45
  int16 v4;
  uint16 v5;
  int v1 = k >> 1;
  eproj_y_vel[v1] += 4;
  uint16 v2 = (uint8)(LOBYTE(eproj_x_vel[v1]) + eproj_E[v1]);
  eproj_E[v1] = v2;
  Point16U pt = Eproj_PhantomFireballs_Func1(v2, eproj_y_vel[v1]);
  bool v3 = (int16)(pt.x + enemy_data[0].x_pos) < 0;
  v4 = pt.x + enemy_data[0].x_pos;
  eproj_x_pos[v1] = pt.x + enemy_data[0].x_pos;
  if (v3
      || !sign16(v4 - 256)
      || (v3 = (int16)(pt.y + enemy_data[0].y_pos + 16) < 0,
          v5 = pt.y + enemy_data[0].y_pos + 16,
          eproj_y_pos[v1] = v5,
          v3)
      || !sign16(v5 - 256)) {
    eproj_instr_list_ptr[v1] = addr_off_8697F8;
    eproj_instr_timers[v1] = 1;
  }
}

void EprojPreInstr_PhantoonDestroyableFireballs_2(uint16 k) {  // 0x869A94
  int v1 = k >> 1;
  if (!eproj_x_vel[v1])
    goto LABEL_5;
  bool v2, v3;
  v2 = eproj_x_vel[v1] == 1;
  v3 = (--eproj_x_vel[v1] & 0x8000) != 0;
  if (v2 || v3) {
    QueueSfx3_Max6(0x1D);
LABEL_5:
    eproj_y_vel[v1] += 16;
    if (EprojBlockCollisition_Vertical(k) & 1) {
      eproj_instr_list_ptr[v1] = addr_word_8697AC;
      eproj_instr_timers[v1] = 1;
      eproj_y_pos[v1] += 8;
      eproj_pre_instr[v1] = FUNC16(nullsub_86);
      QueueSfx3_Max6(0x1D);
    }
  }
}

void EprojPreInstr_PhantoonDestroyableFireballs_3(uint16 k) {  // 0x869ADA
  int16 v4;
  uint16 v5;
  int v1 = k >> 1;
  eproj_y_vel[v1] += 2;
  uint16 v2 = (uint8)(eproj_E[v1] + 2);
  eproj_E[v1] = v2;
  Point16U pt = Eproj_PhantomFireballs_Func1(v2, eproj_y_vel[v1]);
  bool v3 = (int16)(pt.x + enemy_data[0].x_pos) < 0;
  v4 = pt.x + enemy_data[0].x_pos;
  eproj_x_pos[v1] = pt.x + enemy_data[0].x_pos;
  if (v3
      || !sign16(v4 - 256)
      || (v3 = (int16)(pt.y + enemy_data[0].y_pos + 16) < 0,
          v5 = pt.y + enemy_data[0].y_pos + 16,
          eproj_y_pos[v1] = v5,
          v3)
      || !sign16(v5 - 256)) {
    eproj_instr_list_ptr[v1] = addr_off_8697F8;
    eproj_instr_timers[v1] = 1;
  }
}

void EprojPreInstr_PhantoonStartingFireballsB(uint16 k) {  // 0x869B29
  if (enemy_data[0].ai_var_B) {
    int v1 = k >> 1;
    eproj_pre_instr[v1] = FUNC16(EprojPreInstr_PhantoonStartingFireballsB_2);
    eproj_x_vel[v1] = 180;
    eproj_y_vel[v1] = 48;
  }
}

void EprojPreInstr_PhantoonStartingFireballsB_2(uint16 k) {  // 0x869B41
  int v1 = k >> 1;
  uint16 v2 = eproj_x_vel[v1];
  if (v2) {
    eproj_x_vel[v1] = v2 - 1;
LABEL_5:;
    uint8 v4 = eproj_E[v1] + 1;
    eproj_E[v1] = v4;
    Point16U pt = Eproj_PhantomFireballs_Func1(v4, eproj_y_vel[v1]);
    eproj_x_pos[v1] = pt.x + enemy_data[0].x_pos;
    eproj_y_pos[v1] = pt.y + enemy_data[0].y_pos + 16;
    return;
  }
  if ((nmi_frame_counter_word & 1) == 0)
    goto LABEL_5;
  uint16 v3 = eproj_y_vel[v1] - 1;
  eproj_y_vel[v1] = v3;
  if (v3)
    goto LABEL_5;
  eproj_x_pos[v1] = enemy_data[0].x_pos;
  eproj_y_pos[v1] = enemy_data[0].y_pos + 16;
  eproj_instr_timers[v1] = 1;
  eproj_instr_list_ptr[v1] = addr_off_8697F8;
}

Point16U Eproj_PhantomFireballs_Func1(uint16 j, uint16 a) {  // 0x869BA2
  int16 v3;
  uint16 v2, v4;

  uint16 r24 = a;
  uint16 r26 = j;
  if (sign16(j - 128))
    v2 = Eproj_PhantomFireballs_Func2(2 * j, r24);
  else
    v2 = -Eproj_PhantomFireballs_Func2(2 * (uint8)(j + 0x80), r24);
  uint16 r20 = v2;
  v3 = (uint8)(r26 - 64);
  if (sign16(v3 - 128))
    v4 = Eproj_PhantomFireballs_Func2(2 * v3, r24);
  else
    v4 = -Eproj_PhantomFireballs_Func2(2 * (uint8)(v3 + 0x80), r24);
  uint16 r22 = v4;
  return (Point16U) { r20, r22 };
}


uint16 Eproj_PhantomFireballs_Func2(uint16 k, uint16 r24) {  // 0x869BF3
  uint16 r18 = Mult8x8(*((uint8 *)&kSinCosTable8bit_Sext[64] + k), r24) >> 8;
  return r18 + Mult8x8(*((uint8 *)&kSinCosTable8bit_Sext[64] + k + 1), r24);
}

static void EprojInit_RocksKraidSpits(uint16 j) {  // 0x869CA3
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

static void EprojInit_RocksFallingKraidCeiling(uint16 j) {  // 0x869CD8
  int v1 = j >> 1;
  eproj_x_pos[v1] = eproj_init_param_1;
  eproj_y_pos[v1] = 312;
  eproj_y_subpos[v1] = 0;
  eproj_x_subpos[v1] = 0;
  eproj_x_vel[v1] = 0;
  eproj_y_vel[v1] = (random_number & 0x3F) + 64;
  eproj_gfx_idx[v1] = 1536;
}

static void EprojInit_RocksWhenKraidRises(uint16 j) {  // 0x869D0C
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

static void EprojInit_MiniKraidSpit(uint16 j) {  // 0x869DEC
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

static void EprojInit_MiniKraidSpikesLeft(uint16 j) {  // 0x869E46
  EprojInit_MiniKraidSpikes(j, -0x200);
}

static void EprojInit_MiniKraidSpikesRight(uint16 j) {  // 0x869E4B
  EprojInit_MiniKraidSpikes(j, 0x200);
}

void EprojPreInstr_MiniKraidSpikes(uint16 k) {  // 0x869E83
  if (EprojBlockCollisition_Horiz(k) & 1)
    eproj_id[k >> 1] = 0;
}

static void EprojInit_WalkingLavaSeahorseFireball(uint16 j) {  // 0x869EB2
  static const int16 word_869EF9[3] = { -0x100, 0, 0x100 };

  EnemyData *v1 = gEnemyData(cur_enemy_index);
  int v2 = j >> 1;
  eproj_y_pos[v2] = v1->y_pos - 12;
  eproj_x_vel[v2] = -1024;
  eproj_x_pos[v2] = v1->x_pos - 16;
  if ((v1->ai_var_D & 0x8000) == 0) {
    eproj_x_vel[v2] = 1024;
    eproj_x_pos[v2] = v1->x_pos + 16;
  }
  eproj_y_vel[v2] = word_869EF9[eproj_init_param_1 >> 1];
  eproj_y_subpos[v2] = 0;
  eproj_x_subpos[v2] = 0;
}

void EprojPreInstr_WalkingLavaSeahorseFireball(uint16 k) {  // 0x869EFF
  uint16 v2;
  if (EprojBlockCollisition_Vertical(k) & 1 || EprojBlockCollisition_Horiz(k) & 1) {
    eproj_id[k >> 1] = 0;
  } else {
    int v1 = k >> 1;
    if ((eproj_x_vel[v1] & 0x8000) == 0) {
      v2 = eproj_x_vel[v1] - 64;
      eproj_x_vel[v1] = v2;
      if (sign16(v2 - 512))
        v2 = 512;
    } else {
      v2 = eproj_x_vel[v1] + 64;
      eproj_x_vel[v1] = v2;
      if (!sign16(v2 + 512))
        v2 = -512;
    }
    eproj_x_vel[v1] = v2;
  }
}

static void EprojInit_PirateMotherBrainLaser(uint16 j) {  // 0x86A009
  int v2 = j >> 1;
  eproj_instr_list_ptr[v2] = eproj_spawn_r22 ? addr_word_869F7D : addr_word_869F41;
  eproj_pre_instr[v2] = FUNC16(nullsub_87);
  eproj_x_pos[v2] = eproj_spawn_pt.x;
  eproj_y_pos[v2] = eproj_spawn_pt.y;
  EnemyData *v3 = gEnemyData(cur_enemy_index);
  eproj_properties[v2] = *((uint16 *)RomPtr_A0(v3->enemy_ptr) + 3) | 0x1000;
  eproj_E[v2] = v3->parameter_1;
  QueueSfx2_Max6(0x67);
}

const uint8 *EprojInstr_SetPreInstrAndRun(uint16 k, const uint8 *epjp) {  // 0x86A050
  eproj_pre_instr[k >> 1] = GET_WORD(epjp);
  return epjp;
}

void EprojPreInstr_PirateMotherBrainLaser_MoveLeft(uint16 k) {  // 0x86A05C
  int v1 = k >> 1;
  eproj_x_pos[v1] -= 2;
  if ((eproj_E[v1] & 0x8000) == 0)
    eproj_x_pos[v1] -= 2;
  if (CheckIfEprojIsOffScreen(k))
    eproj_id[v1] = 0;
}

void EprojPreInstr_PirateMotherBrainLaser_MoveRight(uint16 k) {  // 0x86A07A
  int v1 = k >> 1;
  eproj_x_pos[v1] += 2;
  if ((eproj_E[v1] & 0x8000) == 0)
    eproj_x_pos[v1] += 2;
  if (CheckIfEprojIsOffScreen(k))
    eproj_id[v1] = 0;
}

static void EprojInit_PirateClaw(uint16 j) {  // 0x86A098
  Rect16U r = eproj_spawn_rect;
  int v1 = j >> 1;
  eproj_y_pos[v1] = r.y + r.h;
  eproj_x_pos[v1] = r.x + r.w;
  eproj_instr_list_ptr[v1] = eproj_init_param_1 ? addr_off_869FE1 : addr_off_869FB9;
  eproj_pre_instr[v1] = FUNC16(nullsub_87);
  eproj_E[v1] = 2048;
  eproj_F[v1] = 1;
}

void EprojPreInstr_PirateClawThrownLeft(uint16 k) {  // 0x86A0D1
  int v1 = k >> 1;
  if (eproj_F[v1]) {
    eproj_x_pos[v1] -= HIBYTE(eproj_E[v1]);
    uint16 v2 = eproj_E[v1] - 32;
    eproj_E[v1] = v2;
    if (!v2)
      eproj_F[v1] = 0;
  } else {
    eproj_x_pos[v1] += HIBYTE(eproj_E[v1]);
    eproj_E[v1] += 32;
  }
  ++eproj_y_pos[v1];
  if (CheckIfEprojIsOffScreen(k))
    eproj_id[v1] = 0;
}

void EprojPreInstr_PirateClawThrownRight(uint16 k) {  // 0x86A124
  int v1 = k >> 1;
  if (eproj_F[v1]) {
    eproj_x_pos[v1] += HIBYTE(eproj_E[v1]);
    uint16 v2 = eproj_E[v1] - 32;
    eproj_E[v1] = v2;
    if (!v2)
      eproj_F[v1] = 0;
  } else {
    eproj_x_pos[v1] -= HIBYTE(eproj_E[v1]);
    eproj_E[v1] += 32;
  }
  ++eproj_y_pos[v1];
  if (CheckIfEprojIsOffScreen(k))
    eproj_id[v1] = 0;
}

static const int16 word_86A2D6[6] = { 64, 72, 80, -64, -72, -80 };

static void EprojInit_A379(uint16 j) {  // 0x86A2A1
  int v1 = j >> 1;
  eproj_x_subpos[v1] = 0;
  eproj_y_subpos[v1] = 0;
  eproj_x_vel[v1] = 0;
  eproj_y_vel[v1] = 0;
  uint16 v2 = eproj_init_param_1;
  int v3 = eproj_init_param_1 >> 1;
  eproj_x_pos[v1] = word_86A2D6[v3] + samus_x_pos;
  eproj_y_pos[v1] = samus_y_pos + 80;
  eproj_instr_list_ptr[v1] = off_86A2E2[v3];
  eproj_E[v1] = v2;
}

static void EprojInit_CeresElevatorPad(uint16 j) {  // 0x86A2EE
  int v1 = j >> 1;
  eproj_y_pos[v1] = samus_y_pos + 28;
  eproj_E[v1] = 60;
  sub_86A301(j);
}

void sub_86A301(uint16 j) {  // 0x86A301
  int v1 = j >> 1;
  eproj_x_subpos[v1] = 0;
  eproj_y_subpos[v1] = 0;
  eproj_x_vel[v1] = 0;
  eproj_y_vel[v1] = 0;
  eproj_gfx_idx[v1] = 0;
  eproj_x_pos[v1] = samus_x_pos;
}


static void EprojInit_CeresElevatorPlatform(uint16 j) {  // 0x86A31B
  eproj_y_pos[j >> 1] = 97;
  sub_86A301(j);
}


void EprojPreInstr_CeresElevatorPad(uint16 k) {  // 0x86A328
  bool v2; // zf
  bool v3; // sf

  int v1 = k >> 1;
  if (!eproj_E[v1]
      || (v2 = eproj_E[v1] == 1, v3 = (int16)(eproj_E[v1] - 1) < 0, --eproj_E[v1], v2)
      || v3) {
    eproj_y_pos[v1] = samus_y_pos + 28;
    if (!sign16(++samus_y_pos - 73)) {
      samus_y_pos = 72;
      eproj_instr_timers[v1] = 1;
      eproj_instr_list_ptr[v1] = addr_off_86A28B;
      CallSomeSamusCode(0xE);
    }
  }
}

void EprojPreInstr_CeresElevatorPlatform(uint16 k) {  // 0x86A364
  if (samus_y_pos == 72) {
    int v1 = k >> 1;
    eproj_instr_timers[v1] = 1;
    eproj_instr_list_ptr[v1] = addr_off_86A28B;
  }
}

void EprojPreInstr_PrePhantomRoom(uint16 j) {  // 0x86A3A3
  bg2_y_scroll = 0;
}

const uint8 *EprojInstr_A3BE(uint16 k, const uint8 *epjp) {  // 0x86A3BE
  int v1 = k >> 1;
  eproj_x_pos[v1] = eproj_E[v1];
  eproj_y_pos[v1] = eproj_F[v1];
  return epjp;
}

const uint8 *EprojInstr_GotoWithProbability25(uint16 k, const uint8 *epjp) {  // 0x86A456
  if ((NextRandom() & 0xC000) == 0xC000)
    return INSTRB_RETURN_ADDR(GET_WORD(epjp));
  else
    return epjp + 2;
}

static void EprojInit_BombTorizoLowHealthDrool(uint16 j) {  // 0x86A5D3
  int16 parameter_1; // dx
  uint16 v4, v5, v8;
  int v1 = j >> 1;
  eproj_gfx_idx[v1] = 0;
  eproj_instr_list_ptr[v1] = off_86A64D[(uint16)((NextRandom() >> 1) & 0xE) >> 1];
  NextRandom();
  EnemyData *v2 = gEnemyData(cur_enemy_index);
  eproj_y_pos[v1] = v2->y_pos - 5;
  parameter_1 = v2->parameter_1;
  if ((parameter_1 & 0x4000) != 0) {
    v4 = random_number & 0x1FE;
  } else {
    if (parameter_1 < 0)
      v5 = 32;
    else
      v5 = 224;
    v4 = 2 * (v5 + (random_number & 0xF) - 8);
  }
  int v6 = v4 >> 1;
  eproj_x_vel[v1] = kSinCosTable8bit_Sext[v6 + 64];
  eproj_y_vel[v1] = kSinCosTable8bit_Sext[v6];
  EnemyData *v7 = gEnemyData(cur_enemy_index);
  if ((v7->parameter_1 & 0x8000) != 0)
    v8 = v7->x_pos + 8;
  else
    v8 = v7->x_pos - 8;
  eproj_x_pos[v1] = v8;
}

static void EprojInit_BombTorizoLowHealthInitialDrool(uint16 j) {  // 0x86A65D
  int16 v3;
  int16 parameter_1; // dx

  int v1 = j >> 1;
  eproj_gfx_idx[v1] = 0;
  NextRandom();
  EnemyData *v2 = gEnemyData(cur_enemy_index);
  eproj_y_pos[v1] = v2->y_pos + (random_number & 3) - 5;
  eproj_y_vel[v1] = (random_number & 0x1F) + 48;
  NextRandom();
  v3 = random_number & 3;
  parameter_1 = v2->parameter_1;
  if ((parameter_1 & 0x4000) != 0) {
    eproj_x_pos[v1] = v2->x_pos + v3;
    eproj_x_vel[v1] = 0;
  } else {
    if (parameter_1 < 0)
      eproj_x_pos[v1] = v2->x_pos + v3 + 8;
    else
      eproj_x_pos[v1] = v2->x_pos + v3 - 8;
    eproj_x_vel[v1] = 0;
  }
}

static void EprojInit_A977(uint16 j) {  // 0x86A6C7
  int v1 = j >> 1;
  eproj_gfx_idx[v1] = 0;
  CalculatePlmBlockCoords(plm_id);
  eproj_x_pos[v1] = 16 * plm_x_block;
  eproj_y_pos[v1] = 16 * plm_y_block - 4;
  eproj_x_subpos[v1] = 0;
  eproj_y_subpos[v1] = 0;
}

static const int16 kEprojInit_BombTorizoExplosiveSwipe_Tab0[11] = { -30, -40, -47, -31, -21, -1, -28, -43, -48, -31, -21 };
static const int16 kEprojInit_BombTorizoExplosiveSwipe_Tab1[11] = { -52, -28, -11, 9, 21, 20, -52, -27, -10, 9, 20 };
static const int16 kEprojInit_BombTorizoStatueBreaking_Xpos[16] = { 8, 0x18, -8, 8, 0x18, -8, 8, 0x18, 8, -8, 0x18, 8, -8, 0x18, 8, -8 };
static const int16 kEprojInit_BombTorizoStatueBreaking_Ypos[8] = { -8, -8, 8, 8, 8, 0x18, 0x18, 0x18 };
static const int16 kEprojInit_BombTorizoStatueBreaking_Yvel[8] = { 256, 256, 256, 256, 256, 256, 256, 256 };
static const int16 kEprojInit_BombTorizoStatueBreaking_F[8] = { 16, 16, 16, 16, 16, 16, 16, 16 };
static void EprojInit_BombTorizoExplosiveSwipe(uint16 j) {  // 0x86A6F6
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  if ((v1->parameter_1 & 0x8000) != 0) {
    int v4 = eproj_init_param_1 >> 1;
    int v5 = j >> 1;
    eproj_x_pos[v5] = v1->x_pos - kEprojInit_BombTorizoExplosiveSwipe_Tab0[v4];
    eproj_y_pos[v5] = v1->y_pos + kEprojInit_BombTorizoExplosiveSwipe_Tab1[v4];
  } else {
    int v2 = eproj_init_param_1 >> 1;
    int v3 = j >> 1;
    eproj_x_pos[v3] = v1->x_pos + kEprojInit_BombTorizoExplosiveSwipe_Tab0[v2];
    eproj_y_pos[v3] = v1->y_pos + kEprojInit_BombTorizoExplosiveSwipe_Tab1[v2];
  }
}

static void EprojInit_BombTorizoStatueBreaking(uint16 j) {  // 0x86A764
  int8 v1;

  CalculatePlmBlockCoords(plm_id);
  v1 = eproj_init_param_1;
  int v2 = eproj_init_param_1 >> 1;
  int v3 = j >> 1;
  eproj_instr_list_ptr[v3] = kEprojInit_BombTorizoStatueBreaking_InstrList[v2];
  eproj_x_pos[v3] = kEprojInit_BombTorizoStatueBreaking_Xpos[v2] + 16 * plm_x_block;
  int v4 = (v1 & 0xF) >> 1;
  eproj_y_pos[v3] = kEprojInit_BombTorizoStatueBreaking_Ypos[v4] + 16 * plm_y_block;
  eproj_y_vel[v3] = kEprojInit_BombTorizoStatueBreaking_Yvel[v4];
  eproj_F[v3] = kEprojInit_BombTorizoStatueBreaking_F[v4];
  eproj_properties[v3] |= 0x1000;
}

void sub_86A887(uint16 v0) {  // 0x86A887
  int16 v2;
  int16 v3;
  int16 v4;

  if (EprojBlockCollisition_Horiz(v0) & 1) {
    int v6 = v0 >> 1;
    eproj_instr_list_ptr[v6] = 0xA48A;
    eproj_instr_timers[v6] = 1;
  } else {
    int v1 = v0 >> 1;
    v2 = eproj_x_vel[v1];
    if (v2 >= 0) {
      v4 = v2 - 4;
      if (v4 < 0)
        v4 = 3;
      eproj_x_vel[v1] = v4;
    } else {
      v3 = v2 + 4;
      if (v3 >= 0)
        v3 = 3;
      eproj_x_vel[v1] = v3;
    }
    uint8 carry = EprojBlockCollisition_Vertical(v0);
    if ((eproj_y_vel[v1] & 0x8000) != 0 || !carry) {
      uint16 v5 = eproj_y_vel[v1] + 16;
      eproj_y_vel[v1] = v5;
      if ((v5 & 0xF000) == 4096)
        eproj_id[v1] = 0;
    } else {
      int v7 = v0 >> 1;
      eproj_y_pos[v7] -= 3;
      eproj_instr_list_ptr[v7] = 0xA48E;
      eproj_instr_timers[v7] = 1;
    }
  }
}

void EprojPreInstr_A977(uint16 k) {  // 0x86A8EF
  uint8 carry = EprojBlockCollisition_Vertical(k);
  int v1 = k >> 1;
  if ((eproj_y_vel[v1] & 0x8000) != 0 || !carry) {
    uint16 v2 = eproj_F[0] + eproj_y_vel[v1];
    eproj_y_vel[v1] = v2;
    if ((v2 & 0xF000) == 4096)
      eproj_y_vel[v1] = 4096;
  } else {
    eproj_pre_instr[v1] = 0xA918;
  }
}

void sub_86A91A(uint16 v0) {  // 0x86A91A
  int v1 = v0 >> 1;
  eproj_x_vel[v1] = 0;
  if ((joypad2_last & 0x100) != 0)
    eproj_x_vel[v1] = 256;
  if ((joypad2_last & 0x200) != 0)
    eproj_x_vel[v1] = -256;
  EprojBlockCollisition_Horiz(v0);
  eproj_y_vel[v1] = 0;
  if ((joypad2_last & 0x400) != 0)
    eproj_y_vel[v1] = 256;
  if ((joypad2_last & 0x800) != 0)
    eproj_y_vel[v1] = -256;
  EprojBlockCollisition_Vertical(v0);
}

static void EprojInit_AB07(uint16 j) {  // 0x86AA3D
  uint16 v1 = vram_write_queue_tail;
  VramWriteEntry *v2 = gVramWriteEntry(vram_write_queue_tail);
  v2->size = 64;
  v2->src.addr = addr_kEprojInit_AB07_Tile0;
  *(uint16 *)&v2->src.bank = 134;
  v2->vram_dst = 28160;
  v1 += 7;
  VramWriteEntry *v3 = gVramWriteEntry(v1);
  v3->size = 64;
  v3->src.addr = addr_kEprojInit_AB07_Tile1;
  *(uint16 *)&v3->src.bank = 134;
  v3->vram_dst = 28416;
  vram_write_queue_tail = v1 + 7;
  int v4 = j >> 1;
  eproj_x_pos[v4] = samus_x_pos;
  eproj_y_pos[v4] = samus_y_pos - 36;
}

void EprojPreInstr_AB07(uint16 k) {  // 0x86AA8C
  int v1 = k >> 1;
  int32 amt = INT16_SHL8(8 * (samus_x_pos - eproj_x_pos[v1]));
  AddToHiLo(&eproj_x_pos[v1], &eproj_x_subpos[v1], amt);
  
  amt = INT16_SHL8(8 * (samus_y_pos - 36 - eproj_y_pos[v1]));
  AddToHiLo(&eproj_y_pos[v1], &eproj_y_subpos[v1], amt);
}

const uint8 *EprojInstr_SpawnEnemyDrops(uint16 k, const uint8 *epjp) {  // 0x86AB8A
  int v2 = k >> 1;
  eproj_spawn_pt = (Point16U) { eproj_x_pos[v2], eproj_y_pos[v2] };
  SpawnEnemyDrops(area_index ? GET_WORD(epjp + 2) : GET_WORD(epjp), k, 0);
  return epjp + 4;
}

static void EprojInit_WreckedShipChozoSpikeFootsteps(uint16 j) {  // 0x86AEFC
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  int v2 = j >> 1;
  eproj_x_pos[v2] = eproj_init_param_1 + v1->x_pos;
  eproj_y_pos[v2] = v1->y_pos + 28;
}

const uint8 *EprojInstr_ResetXYpos1(uint16 k, const uint8 *epjp) {  // 0x86AF36
  int v2 = k >> 1;
  eproj_x_pos[v2] = eproj_E[v2];
  eproj_y_pos[v2] = eproj_F[v2];
  return epjp;
}

static void EprojInit_TourianStatueDustClouds(uint16 j) {  // 0x86AF43
  int v1 = j >> 1;
  eproj_E[v1] = 128;
  eproj_F[v1] = 188;
}

static void EprojInit_TourianLandingDustCloudsRightFoot(uint16 j) {  // 0x86AF50
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  int v2 = j >> 1;
  eproj_y_pos[v2] = v1->y_pos + 48;
  eproj_x_pos[v2] = v1->x_pos + 24;
}

const uint8 *EprojInstr_MoveY_Minus4(uint16 k, const uint8 *epjp) {  // 0x86AF92
  eproj_y_pos[k >> 1] -= 4;
  return epjp;
}

const uint8 *EprojInstr_GotoIfFunc1(uint16 k, const uint8 *epjp) {  // 0x86B3B8
  if ((gExtraEnemyRam7800(eproj_F[k >> 1])->kraid.kraid_healths_8ths[0] & 0x8000) == 0)
    return INSTRB_RETURN_ADDR(GET_WORD(epjp));
  else
    return epjp + 2;
}

const uint8 *EprojInstr_ResetXYpos2(uint16 k, const uint8 *epjp) {  // 0x86B436
  int v2 = k >> 1;
  eproj_x_pos[v2] = eproj_E[v2];
  eproj_y_pos[v2] = eproj_F[v2];
  return epjp;
}

const uint8 *EprojInstr_SpawnTourianStatueUnlockingParticle(uint16 k, const uint8 *epjp) {  // 0x86B7EA
  SpawnEprojWithRoomGfx(addr_kEproj_TourianStatueUnlockingParticle, k);
  return epjp;
}

void sub_86BB30(uint16 j) {  // 0x86BB30
  ExtraEnemyRam8000 *v1 = gExtraEnemyRam8000(cur_enemy_index);
  int v2 = j >> 1;
  eproj_x_pos[v2] = *(uint16 *)&v1->pad[34];
  eproj_y_pos[v2] = *(uint16 *)&v1->pad[36];
  eproj_instr_list_ptr[v2] = off_86BB1E[eproj_init_param_1];
}

static const int16 kEprojInit_ShaktoolAttackMiddleBackCircle_X[8] = { 0, 12, 16, 12, 0, -12, -16, -12 };
static const int16 kEprojInit_ShaktoolAttackMiddleBackCircle_Y[8] = { -16, -12, 0, 12, 16, 12, 0, -12 };

static void EprojInit_BDA2(uint16 j) {  // 0x86BDA2
  int v1 = j >> 1;
  EnemyData *v2 = gEnemyData(cur_enemy_index);
  eproj_x_pos[v1] = v2->x_pos;
  eproj_y_pos[v1] = v2->y_pos;
  uint16 v3 = 2 * LOBYTE(v2->ai_var_D);
  int v4 = v3 >> 1;
  eproj_x_vel[v1] = kSinCosTable8bit_Sext[v4 + 64];
  eproj_y_vel[v1] = kSinCosTable8bit_Sext[v4];
  int v5 = (uint16)(v3 >> 5) >> 1;
  eproj_x_pos[v1] += kEprojInit_ShaktoolAttackMiddleBackCircle_X[v5];
  eproj_y_pos[v1] += kEprojInit_ShaktoolAttackMiddleBackCircle_Y[v5];
}

static void EprojInit_ShaktoolAttackMiddleBackCircle(uint16 j) {  // 0x86BD9C
  eproj_E[j >> 1] = eproj_init_param_1;
  EprojInit_BDA2(j);
}

static void EprojInit_ShaktoolAttackFrontCircle(uint16 v1) {  // 0x86BE03
  if (EprojBlockCollisition_Horiz(v1) & 1 || EprojBlockCollisition_Vertical(v1) & 1)
    eproj_id[v1 >> 1] = 0;
}

void EprojPreInstr_BE12(uint16 k) {  // 0x86BE12
  int v1 = k >> 1;
  if (eproj_id[eproj_E[v1] >> 1]) {
    EprojBlockCollisition_Horiz(k);
    EprojBlockCollisition_Vertical(k);
  } else {
    eproj_id[v1] = 0;
  }
}

static void EprojInit_KiHunterAcidSpitCommon(uint16 k, uint16 j) {  // 0x86CFBA
  int v2 = j >> 1;
  eproj_y_vel[v2] = 0;
  eproj_y_pos[v2] = gEnemyData(k)->y_pos - 16;
  eproj_y_subpos[v2] = 0;
  eproj_x_subpos[v2] = 0;
}

static void EprojInit_KiHunterAcidSpitLeft(uint16 j) {  // 0x86CF90
  int v2 = j >> 1;
  eproj_x_vel[v2] = -768;
  eproj_x_pos[v2] = gEnemyData(cur_enemy_index)->x_pos - 22;
  EprojInit_KiHunterAcidSpitCommon(cur_enemy_index, j);
}

static void EprojInit_KiHunterAcidSpitRight(uint16 j) {  // 0x86CFA6
  int v2 = j >> 1;
  eproj_x_vel[v2] = 768;
  eproj_x_pos[v2] = gEnemyData(cur_enemy_index)->x_pos + 22;
  EprojInit_KiHunterAcidSpitCommon(cur_enemy_index, j);
}

void sub_86CFD5(uint16 k) {  // 0x86CFD5
  int v1 = k >> 1;
  eproj_pre_instr[v1] = FUNC16(sub_86CFF8);
  eproj_x_pos[v1] -= 19;
}

void sub_86CFE6(uint16 k) {  // 0x86CFE6
  int v1 = k >> 1;
  eproj_pre_instr[v1] = FUNC16(sub_86CFF8);
  eproj_x_pos[v1] += 19;
}

void sub_86CFF8(uint16 k) {  // 0x86CFF8
  int v1 = k >> 1;
  if (EprojBlockCollisition_Vertical(k) & 1) {
    eproj_instr_list_ptr[v1] = addr_off_86CF56;
    eproj_instr_timers[v1] = 1;
  } else if (EprojBlockCollisition_Horiz(k) & 1) {
    eproj_x_vel[k >> 1] = 0;
  } else {
    uint16 v2 = eproj_y_vel[v1] + 16;
    eproj_y_vel[v1] = v2;
    if (!sign16(v2 - 512))
      v2 = 512;
    eproj_y_vel[v1] = v2;
  }
}

static void EprojInit_KagosBugs(uint16 j) {  // 0x86D088
  int v2 = j >> 1;
  eproj_F[v2] = cur_enemy_index;
  EnemyData *v3 = gEnemyData(cur_enemy_index);
  eproj_x_pos[v2] = v3->x_pos;
  eproj_y_pos[v2] = v3->y_pos;
  uint16 v4 = (random_number & 7) + 1;
  eproj_G[v2] = v4;
  eproj_E[v2] = v4 + 4;
  eproj_pre_instr[v2] = FUNC16(EprojPreInstr_KagosBugs);
}

void EprojPreInstr_KagosBugs_Func1(uint16 k) {  // 0x86D0B3
  int v1 = k >> 1;
  uint16 v2 = eproj_E[v1];
  if (v2) {
    uint16 v3 = v2 - 1;
    eproj_E[v1] = v3;
    if (!v3)
      QueueSfx2_Max6(0x6C);
  }
}

void EprojPreInstr_KagosBugs_Func2(uint16 k) {  // 0x86D1E4
  int v1 = k >> 1;
  EnemyData *v2 = gEnemyData(eproj_F[v1]);
  uint16 v3 = abs16(v2->x_pos - eproj_x_pos[v1]);
  if (!sign16(v3 - 23))
    eproj_properties[v1] |= 0x8000;
}

void EprojPreInstr_KagosBugs(uint16 k) {  // 0x86D0CA
  EprojPreInstr_KagosBugs_Func1(k);
  EprojPreInstr_KagosBugs_Func2(k);
  int v1 = k >> 1;
  if (eproj_G[v1]) {
    --eproj_G[v1];
  } else {
    eproj_instr_list_ptr[v1] = addr_word_86D052;
    eproj_instr_timers[v1] = 1;
    eproj_pre_instr[v1] = FUNC16(nullsub_302);
  }
}

static const uint16 word_86D082 = 0xe0;

void EprojPreInstr_D0EC(uint16 k) {  // 0x86D0EC
  EprojPreInstr_KagosBugs_Func1(k);
  EprojPreInstr_KagosBugs_Func2(k);
  if (EprojBlockCollisition_Horiz(k)) {
    eproj_x_vel[k >> 1] = 0;
    goto LABEL_6;
  }
  if (EprojBlockCollisition_Vertical(k)) {
LABEL_6:
    eproj_y_vel[k >> 1] = 256;
LABEL_7:;
    eproj_pre_instr[k >> 1] = FUNC16(EprojPreInstr_D128);
    eproj_instr_list_ptr[k >> 1] = addr_word_86D04A;
    eproj_instr_timers[k >> 1] = 1;
    return;
  }
  uint16 v2 = eproj_y_vel[k >> 1];
  bool v3 = (int16)(word_86D082 + v2) < 0;
  eproj_y_vel[k >> 1] = word_86D082 + v2;
  if (!v3) {
    goto LABEL_7;
  }
}

void EprojPreInstr_D128(uint16 v0) {  // 0x86D128
  EprojPreInstr_KagosBugs_Func1(v0);
  EprojPreInstr_KagosBugs_Func2(v0);
  if (EprojBlockCollisition_Horiz(v0) & 1) {
    eproj_x_vel[v0 >> 1] = 0;
  } else if (EprojBlockCollisition_Vertical(v0) & 1) {
    int v1 = v0 >> 1;
    eproj_pre_instr[v1] = FUNC16(nullsub_302);
    eproj_instr_list_ptr[v1] = addr_word_86D03C;
    eproj_instr_timers[v1] = 1;
  } else {
    eproj_y_vel[v0 >> 1] += word_86D082;
  }
}

static const uint16 g_word_86D086 = 0x200;
static const uint16 g_word_86D084 = 0x30;

const uint8 *EprojInstr_D15C(uint16 k, const uint8 *epjp) {  // 0x86D15C
  EprojPreInstr_KagosBugs_Func1(k);
  EprojPreInstr_KagosBugs_Func2(k);
  int v2 = k >> 1;

  uint16 t = (random_number & 0x300) + 2048;
  eproj_y_vel[v2] = -t;
  EnemyData *v3 = gEnemyData(eproj_F[v2]);
  uint16 t2 = v3->x_pos - eproj_x_pos[v2];

  if ((int16)(abs16(t2) - g_word_86D084) >= 0)
    t = sign16(t2) ? -1 : 0;

  uint16 v4 = g_word_86D086;
  if (t & 0x100)
    v4 = -v4;
  eproj_x_vel[v2] = v4;
  eproj_pre_instr[v2] = FUNC16(EprojPreInstr_D0EC);
  return epjp;
}

const uint8 *EprojInstr_D1B6(uint16 k, const uint8 *epjp) {  // 0x86D1B6
  int v1 = k >> 1;
  eproj_G[v1] = (random_number & 0x1F) + 1;
  eproj_pre_instr[v1] = FUNC16(EprojPreInstr_KagosBugs);
  return epjp;
}

const uint8 *EprojInstr_D1C7(uint16 k, const uint8 *epjp) {  // 0x86D1C7
  eproj_gfx_idx[k >> 1] = 0;
  return epjp;
}

const uint8 *EprojInstr_D1CE(uint16 k, const uint8 *epjp) {  // 0x86D1CE
  eproj_spawn_pt = (Point16U){ eproj_x_pos[k >> 1], eproj_y_pos[k >> 1] };
  SpawnEnemyDrops(addr_kEnemyDef_E7FF, k, 0);
  return epjp;
}

static void EprojInit_MaridiaFloatersSpikes(uint16 j) {  // 0x86D23A
  int v1 = j >> 1;
  eproj_x_subpos[v1] = 0;
  eproj_y_subpos[v1] = 0;
  eproj_x_vel[v1] = 0;
  eproj_y_vel[v1] = 0;
  EnemyData *v2 = gEnemyData(cur_enemy_index);
  eproj_x_pos[v1] = v2->x_pos;
  eproj_y_pos[v1] = v2->y_pos;
  eproj_E[v1] = eproj_init_param_1;
}

static const int16 word_86D21A[8] = { 0, 32, 32, 32, 0, -32, -32, -32 };
static const int16 word_86D22A[8] = { -32, -32, 0, 32, 32, 32, 0, -32 };

void EprojPreInstr_MaridiaFloatersSpikes(uint16 k) {  // 0x86D263
  int v1 = k >> 1;
  eproj_x_vel[v1] += word_86D21A[eproj_E[v1]];
  if (EprojBlockCollisition_Horiz(k) & 1
      || (eproj_y_vel[v1] += word_86D22A[eproj_E[v1]],  EprojBlockCollisition_Vertical(k) & 1)) {
    eproj_instr_list_ptr[v1] = addr_off_86D218;
    eproj_instr_timers[v1] = 1;
  }
}

static void EprojInit_WreckedShipRobotLaserCommon(uint16 k, uint16 j) {  // 0x86D35B
  EnemyData *v2 = gEnemyData(k);
  int v3 = j >> 1;
  eproj_y_pos[v3] = v2->y_pos - 16;
  eproj_x_pos[v3] = v2->x_pos + (sign16(eproj_x_vel[v3]) ? -4 : 4);
  eproj_y_subpos[v3] = 0;
  eproj_x_subpos[v3] = 0;
  if ((int16)(v2->x_width + v2->x_pos - layer1_x_pos) >= 0
    && (int16)(v2->x_pos - v2->x_width - 257 - layer1_x_pos) < 0
    && (int16)(v2->y_height + v2->y_pos - layer1_y_pos) >= 0
    && (int16)(v2->y_pos - v2->y_height - 224) < 0) {
    QueueSfx2_Max6(0x67);
  }
}

static void EprojInit_WreckedShipRobotLaserDown(uint16 j) {  // 0x86D30C
  int v2 = j >> 1;
  eproj_x_vel[v2] = gEnemyData(cur_enemy_index)->ai_var_A;
  eproj_y_vel[v2] = 128;
  eproj_gfx_idx[v2] = 0;
  EprojInit_WreckedShipRobotLaserCommon(cur_enemy_index, j);
}

static void EprojInit_WreckedShipRobotLaserHorizontal(uint16 j) {  // 0x86D32E
  int v2 = j >> 1;
  eproj_x_vel[v2] = gEnemyData(cur_enemy_index)->ai_var_A;
  eproj_y_vel[v2] = 0;
  EprojInit_WreckedShipRobotLaserCommon(cur_enemy_index, j);
}

static void EprojInit_WreckedShipRobotLaserUp(uint16 j) {  // 0x86D341
  int v2 = j >> 1;
  eproj_x_vel[v2] = gEnemyData(cur_enemy_index)->ai_var_A;
  eproj_y_vel[v2] = -128;
  EprojInit_WreckedShipRobotLaserCommon(cur_enemy_index, j);
}

void EprojPreInstr_WreckedShipRobotLaser(uint16 k) {  // 0x86D3BF
  int v1 = k >> 1;
  eproj_gfx_idx[v1] = 0;
  if (EprojBlockCollisition_Horiz(k) & 1 || EprojBlockCollisition_Vertical(k) & 1)
    eproj_id[v1] = 0;
}

const uint8 *EprojInstr_AssignNewN00bTubeShardVelocity(uint16 k, const uint8 *epjp) {  // 0x86D5E1
  NextRandom();
  int v2 = k >> 1;
  eproj_x_vel[v2] = *(uint16 *)((uint8 *)&random_number + 1);
  eproj_y_vel[v2] = 192;
  return epjp;
}

const uint8 *EprojInstr_SetN00bTubeShardX(uint16 k, const uint8 *epjp) {  // 0x86D5F2
  int v2 = k >> 1;
  if (nmi_frame_counter_word & 1) {
    eproj_x_pos[v2] = eproj_F[v2];
    eproj_spritemap_ptr[v2] = GET_WORD(epjp);
  } else {
    eproj_x_pos[v2] = 128 - eproj_F[v2] + 128;
    eproj_spritemap_ptr[v2] = GET_WORD(epjp + 2);
  }
  eproj_instr_list_ptr[v2] = epjp - RomBankBase(0x86) + 4;
  eproj_instr_timers[v2] = 1;
  return 0;
}

const uint8 *EprojInstr_D62A(uint16 k, const uint8 *epjp) {  // 0x86D62A
  if (nmi_frame_counter_word & 1)
    eproj_x_pos[k >> 1] = eproj_F[k >> 1];
  else
    eproj_x_pos[k >> 1] = -4608;
  int v2 = k >> 1;
  eproj_spritemap_ptr[v2] = GET_WORD(epjp);
  eproj_instr_list_ptr[v2] = epjp - RomBankBase(0x86) + 2;
  eproj_instr_timers[v2] = 1;
  return 0;
}

const uint8 *EprojInstr_SetXvelRandom(uint16 k, const uint8 *epjp) {  // 0x86D69A
  NextRandom();
  eproj_x_vel[k >> 1] = *(uint16 *)((uint8 *)&random_number + 1);
  return epjp;
}

static void EprojInit_N00bTubeCrack(uint16 j) {  // 0x86D6A5
  CalculatePlmBlockCoords(plm_id);
  int v1 = j >> 1;
  eproj_x_pos[v1] = 16 * plm_x_block + 96;
  eproj_y_pos[v1] = 16 * plm_y_block + 48;
}

static const int16 kEprojInit_N00bTubeShards_X[10] = { -56, -64, -20, -40, -64, -48, -24, -40, 0, -8 };
static const int16 kEprojInit_N00bTubeShards_Y[10] = { 8, -12, -26, -24, -32, 28, 16, -8, -24, 16 };
static const int16 kEprojInit_N00bTubeShards_Xvel[10] = { -384, -384, -160, -288, -288, -320, -96, -352, 0, -64 };
static const int16 kEprojInit_N00bTubeShards_Yvel[10] = { 320, -256, -416, -288, -288, 448, 576, -96, -288, 384 };

static void EprojInit_N00bTubeShards(uint16 j) {  // 0x86D6C9
  CalculatePlmBlockCoords(plm_id);
  int v1 = eproj_init_param_1 >> 1;
  int v2 = j >> 1;
  eproj_F[v2] = kEprojInit_N00bTubeShards_X[v1] + 16 * plm_x_block + 96;
  eproj_E[v2] = 0;
  eproj_y_pos[v2] = kEprojInit_N00bTubeShards_Y[v1] + 16 * plm_y_block + 48;
  eproj_instr_list_ptr[v2] = kEprojInit_N00bTubeShards_InstrPtrs[v1];
  eproj_x_vel[v2] = kEprojInit_N00bTubeShards_Xvel[v1];
  eproj_y_vel[v2] = kEprojInit_N00bTubeShards_Yvel[v1];
}

static const uint16 kEprojInit_N00bTubeReleasedAirBubbles_X[6] = { 40, 80, 104, 120, 152, 184 };
static const uint16 kEprojInit_N00bTubeReleasedAirBubbles_Y[6] = { 80, 72, 84, 32, 64, 84 };

static void EprojInit_N00bTubeReleasedAirBubbles(uint16 j) {  // 0x86D774
  CalculatePlmBlockCoords(plm_id);
  int v1 = eproj_init_param_1 >> 1;
  int v2 = j >> 1;
  eproj_F[v2] = kEprojInit_N00bTubeReleasedAirBubbles_X[v1] + 16 * plm_x_block;
  eproj_E[v2] = 0;
  eproj_y_pos[v2] = kEprojInit_N00bTubeReleasedAirBubbles_Y[v1] + 16 * plm_y_block;
  eproj_y_vel[v2] = -1280;
}

void sub_86D7BF(uint16 k) {  // 0x86D7BF
  int v1 = k >> 1;
  if (eproj_x_pos[v1] != 0xEE00)
    eproj_E[v1] = eproj_x_pos[v1];
  if (nmi_frame_counter_word & 1)
    eproj_x_pos[v1] = -4608;
  else
    eproj_x_pos[v1] = eproj_E[v1];
}

void sub_86D7DE(uint16 k) {  // 0x86D7DE
  int v1 = k >> 1;
  int32 amt = INT16_SHL8(192);
  AddToHiLo(&eproj_y_pos[v1], &eproj_y_subpos[v1], amt);
}

void EprojPreInstr_N00bTubeShards(uint16 k) {  // 0x86D7FD
  int v1 = k >> 1;
  int32 amt = INT16_SHL8(eproj_x_vel[v1]);
  AddToHiLo(&eproj_F[v1], &eproj_E[v1], amt);
  amt = INT16_SHL8(eproj_y_vel[v1]);
  AddToHiLo(&eproj_y_pos[v1], &eproj_y_subpos[v1], amt);
  Eproj_DeleteIfYposOutside(k);
}

void sub_86D83D(uint16 k) {  // 0x86D83D
  int v2 = (eproj_x_vel[k >> 1] & 0x17E | 0x80) >> 1;
  int v6 = eproj_index >> 1;
  int32 amt = INT16_SHL8((int16)kSinCosTable8bit_Sext[v2 + 64] >> 2);
  AddToHiLo(&eproj_F[v6], &eproj_E[v6], amt);
  eproj_x_vel[v6] += 2;
  amt = INT16_SHL8(eproj_y_vel[v6]);
  AddToHiLo(&eproj_y_pos[v6], &eproj_y_subpos[v6], amt);
  Eproj_DeleteIfYposOutside(eproj_index);
}

void sub_86D89F(uint16 k) {  // 0x86D89F
  uint16 v1 = eproj_x_vel[k >> 1] & 0x17E | 0x80;
  int v2 = v1 >> 1;
  int32 amt = INT16_SHL8((int16)kSinCosTable8bit_Sext[v2 + 64] >> 2);
  int v5 = eproj_index >> 1;
  AddToHiLo(&eproj_F[v5], &eproj_E[v5], amt);
  eproj_x_vel[v5] += 4;
  sub_86D8DF(eproj_index);
}

void sub_86D8DF(uint16 k) {  // 0x86D8DF
  int v5 = k >> 1;
  int32 amt = INT16_SHL8(eproj_y_vel[v5]);
  AddToHiLo(&eproj_y_pos[v5], &eproj_y_subpos[v5], amt);
  eproj_x_pos[v5] = eproj_F[v5];
}

void sub_86D992(uint16 v0) {  // 0x86D992
  uint16 v1 = eproj_init_param_1;
  int v2 = v0 >> 1;
  eproj_E[v2] = eproj_init_param_1;
  eproj_instr_list_ptr[v2] = off_86D96A[v1 >> 1];
  EnemyData *v3 = gEnemyData(cur_enemy_index);
  eproj_x_pos[v2] = v3->x_pos;
  eproj_x_subpos[v2] = v3->x_subpos;
  eproj_y_pos[v2] = v3->y_pos;
  eproj_y_subpos[v2] = v3->y_subpos;
  eproj_y_vel[v2] = -512;
  eproj_x_vel[v2] = 512;
  if (!sign16(eproj_init_param_1 - 12)) {
    eproj_y_vel[v2] = -384;
    eproj_x_vel[v2] = 384;
  }
}

void EprojPreInstr_SpikeShootingPlantSpikes_MoveY1(uint16 k) {  // 0x86D9E6
  int v1 = k >> 1;
  AddToHiLo(&eproj_y_pos[v1], &eproj_y_subpos[v1], INT16_SHL8(eproj_y_vel[v1]));
}

void EprojPreInstr_SpikeShootingPlantSpikes_MoveY2(uint16 k) {  // 0x86DA10
  int v1 = k >> 1;
  AddToHiLo(&eproj_y_pos[v1], &eproj_y_subpos[v1], INT16_SHL8(eproj_x_vel[v1]));
}

void EprojPreInstr_SpikeShootingPlantSpikes_MoveX1(uint16 k) {  // 0x86DA3A
  int v1 = k >> 1;
  AddToHiLo(&eproj_x_pos[v1], &eproj_x_subpos[v1], INT16_SHL8(eproj_y_vel[v1]));
}

void EprojPreInstr_SpikeShootingPlantSpikes_MoveX2(uint16 k) {  // 0x86DA64
  int v1 = k >> 1;
  AddToHiLo(&eproj_x_pos[v1], &eproj_x_subpos[v1], INT16_SHL8(eproj_x_vel[v1]));
}

void EprojPreInstr_SpikeShootingPlantSpikes_0_MoveX1(uint16 k) {  // 0x86DA8E
  EprojPreInstr_SpikeShootingPlantSpikes_MoveX1(k);
}

void EprojPreInstr_SpikeShootingPlantSpikes_2_MoveX2(uint16 k) {  // 0x86DA93
  EprojPreInstr_SpikeShootingPlantSpikes_MoveX2(k);
}

void EprojPreInstr_SpikeShootingPlantSpikes_1_MoveY1(uint16 k) {  // 0x86DA98
  EprojPreInstr_SpikeShootingPlantSpikes_MoveY1(k);
}

void EprojPreInstr_SpikeShootingPlantSpikes_4_MoveY2(uint16 k) {  // 0x86DA9D
  EprojPreInstr_SpikeShootingPlantSpikes_MoveY2(k);
}

void EprojPreInstr_SpikeShootingPlantSpikes_6_MoveX1Y1(uint16 k) {  // 0x86DAA2
  EprojPreInstr_SpikeShootingPlantSpikes_MoveX1(k);
  EprojPreInstr_SpikeShootingPlantSpikes_MoveY1(k);
}

void EprojPreInstr_SpikeShootingPlantSpikes_8_MoveX1Y2(uint16 k) {  // 0x86DAAA
  EprojPreInstr_SpikeShootingPlantSpikes_MoveX1(k);
  EprojPreInstr_SpikeShootingPlantSpikes_MoveY2(k);
}

void EprojPreInstr_SpikeShootingPlantSpikes_7_MoveX2Y1(uint16 k) {  // 0x86DAB2
  EprojPreInstr_SpikeShootingPlantSpikes_MoveX2(k);
  EprojPreInstr_SpikeShootingPlantSpikes_MoveY1(k);
}

void EprojPreInstr_SpikeShootingPlantSpikes_9_MoveX2Y2(uint16 k) {  // 0x86DABA
  EprojPreInstr_SpikeShootingPlantSpikes_MoveX2(k);
  EprojPreInstr_SpikeShootingPlantSpikes_MoveY2(k);
}

static Func_Y_V *const kEprojPreInstr_SpikeShootingPlantSpikes[10] = {  // 0x86D9DB
  EprojPreInstr_SpikeShootingPlantSpikes_0_MoveX1,
  EprojPreInstr_SpikeShootingPlantSpikes_1_MoveY1,
  EprojPreInstr_SpikeShootingPlantSpikes_2_MoveX2,
  EprojPreInstr_SpikeShootingPlantSpikes_0_MoveX1,
  EprojPreInstr_SpikeShootingPlantSpikes_4_MoveY2,
  EprojPreInstr_SpikeShootingPlantSpikes_2_MoveX2,
  EprojPreInstr_SpikeShootingPlantSpikes_6_MoveX1Y1,
  EprojPreInstr_SpikeShootingPlantSpikes_7_MoveX2Y1,
  EprojPreInstr_SpikeShootingPlantSpikes_8_MoveX1Y2,
  EprojPreInstr_SpikeShootingPlantSpikes_9_MoveX2Y2,
};

void EprojPreInstrHelper_SpikeShootingPlantSpikes_Func1(uint16 k) {  // 0x86DAC2
  if (EprojPreInstrHelper_SpikeShootingPlantSpikes_Func2(k))
    eproj_id[k >> 1] = 0;
}

void EprojPreInstr_SpikeShootingPlantSpikes(uint16 k) {
  kEprojPreInstr_SpikeShootingPlantSpikes[eproj_E[k >> 1] >> 1](k);
  EprojPreInstrHelper_SpikeShootingPlantSpikes_Func1(k);
}

uint16 EprojPreInstrHelper_SpikeShootingPlantSpikes_Func2(uint16 k) {  // 0x86DACE
  int v1 = k >> 1;
  return (int16)(eproj_x_pos[v1] - layer1_x_pos) < 0
    || (int16)(layer1_x_pos + 256 - eproj_x_pos[v1]) < 0
    || (int16)(eproj_y_pos[v1] - layer1_y_pos) < 0
    || (int16)(layer1_y_pos + 256 - eproj_y_pos[v1]) < 0;
}

static void EprojInit_DBF2(uint16 j) {  // 0x86DB18
  int v2 = j >> 1;
  eproj_instr_list_ptr[v2] = addr_word_86DB0C;
  eproj_E[v2] = FUNC16(EprojPreInstr_DBF2_MoveX1);
  if (eproj_init_param_1)
    eproj_E[v2] = FUNC16(EprojPreInstr_DBF2_MoveX2);
  EnemyData *v3 = gEnemyData(cur_enemy_index);
  eproj_x_pos[v2] = v3->x_pos;
  eproj_x_subpos[v2] = v3->x_subpos;
  eproj_y_pos[v2] = v3->y_pos + 2;
  eproj_y_subpos[v2] = v3->y_subpos;
  eproj_y_vel[v2] = -256;
  eproj_x_vel[v2] = 256;
}

void EprojPreInstr_DBF2_Func1(uint16 k) {  // 0x86DBB6
  if (EprojPreInstrHelper_DBF2_Func2(k))
    eproj_id[k >> 1] = 0;
}

void EprojPreInstr_DBF2(uint16 k) {  // 0x86DB5B
  CallEprojPreInstr(eproj_E[k >> 1] | 0x860000, k);
  EprojPreInstr_DBF2_Func1(k);
}

void EprojPreInstr_DBF2_MoveX1(uint16 k) {  // 0x86DB62
  int v1 = k >> 1;
  AddToHiLo(&eproj_x_pos[v1], &eproj_x_subpos[v1], INT16_SHL8(eproj_y_vel[v1]));
}

void EprojPreInstr_DBF2_MoveX2(uint16 k) {  // 0x86DB8C
  int v1 = k >> 1;
  AddToHiLo(&eproj_x_pos[v1], &eproj_x_subpos[v1], INT16_SHL8(eproj_x_vel[v1]));
}

uint16 EprojPreInstrHelper_DBF2_Func2(uint16 k) {  // 0x86DBC2
  int v1 = k >> 1;
  return (int16)(eproj_x_pos[v1] - layer1_x_pos) < 0
    || (int16)(layer1_x_pos + 256 - eproj_x_pos[v1]) < 0
    || (int16)(eproj_y_pos[v1] - layer1_y_pos) < 0
    || (int16)(layer1_y_pos + 256 - eproj_y_pos[v1]) < 0;
}

const uint8 *EprojInstr_DC5A(uint16 k, const uint8 *epjp) {  // 0x86DC5A
  eproj_properties[k >> 1] = 12288;
  return epjp;
}

const uint8 *EprojInstr_SpawnEnemyDrops_0(uint16 k, const uint8 *epjp) {  // 0x86DC61
  int v2 = k >> 1;
  eproj_spawn_pt = (Point16U){ eproj_x_pos[v2], eproj_y_pos[v2] };
  SpawnEnemyDrops(addr_kEnemyDef_DF7F, k, 0);
  return epjp;
}

const uint8 *EprojInstr_SpawnSporesEproj(uint16 k, const uint8 *epjp) {  // 0x86DC77
  int v2 = k >> 1;
  eproj_spawn_pt = (Point16U) { eproj_x_pos[v2], eproj_y_pos[v2] };
  SpawnEprojWithRoomGfx(addr_kEproj_Spores, 0);
  return epjp;
}

static void EprojInit_Spores(uint16 j) {  // 0x86DC8D
  int v2 = j >> 1;
  eproj_F[v2] = eproj_x_pos[v2] = eproj_spawn_pt.x;
  eproj_y_pos[v2] = eproj_spawn_pt.y;
  eproj_gfx_idx[v2] = 512;
}

static const int16 word_86DCB9[5] = { -64, -56, -48, -40, -32 };

static void EprojInit_SporeSpawnStalk(uint16 j) {  // 0x86DCA3
  int v1 = j >> 1;
  eproj_y_pos[v1] = enemy_data[0].y_pos + word_86DCB9[eproj_init_param_1];
  eproj_x_pos[v1] = enemy_data[0].x_pos;
}

void sub_86DCC3(uint16 v0) {  // 0x86DCC3
  int v1 = v0 >> 1;
  eproj_y_pos[v1] = enemy_data[0].y_pos - 96;
  eproj_x_pos[v1] = enemy_data[0].x_pos;
}

static const int16 kEprojInit_SporeSpawners_X[4] = { 0x20, 0x60, 0xa0, 0xe0 };

static void EprojInit_SporeSpawners(uint16 j) {  // 0x86DCD4
  int v1 = j >> 1;
  eproj_x_pos[v1] = kEprojInit_SporeSpawners_X[eproj_init_param_1];
  eproj_y_pos[v1] = 520;
}

void EprojPreInstr_Spores(uint16 k) {  // 0x86DCEE
  int v1 = k >> 1;
  uint16 v2 = LOBYTE(eproj_E[v1]);
  uint16 r18 = SignExtend8(kSporeMovementData[LOBYTE(eproj_E[v1])]);
  if ((eproj_F[v1] & 0x80) != 0)
    r18 = -r18;
  eproj_x_pos[v1] += r18;
  r18 = SignExtend8(kSporeMovementData[v2 + 1]);
  uint16 v3 = r18 + eproj_y_pos[v1] + r18;
  eproj_y_pos[v1] = v3;
  if (!sign16(v3 - 768))
    eproj_id[v1] = 0;
  eproj_E[v1] = (uint8)(LOBYTE(eproj_E[v1]) + 2);
}

void EprojPreInstr_SporeSpawners(uint16 k) {  // 0x86DD46
  if (!kraid_unk9000) {
    int v1 = k >> 1;
    if (!eproj_F[v1]) {
      eproj_instr_list_ptr[v1] = addr_word_86DC06;
      eproj_instr_timers[v1] = 1;
      eproj_F[v1] = NextRandom() & 0x1FF;
    }
    --eproj_F[v1];
  }
}

const uint8 *EprojInstr_DFEA(uint16 k, const uint8 *epjp) {  // 0x86DFEA
  int v1 = k >> 1;
  eproj_spawn_pt = (Point16U){ eproj_x_pos[v1], eproj_y_pos[v1] };
  SpawnEnemyDrops(addr_kEnemyDef_E83F, k, 0);
  return epjp;
}

const uint8 *EprojInstr_SetYVel(uint16 k, const uint8 *epjp) {  // 0x86E533
  eproj_y_vel[k >> 1] = GET_WORD(epjp);
  return epjp + 2;
}

uint16 Eproj_FuncE722(uint16 k) {  // 0x86E722
  int16 v2;
  int16 v3;

  int v1 = k >> 1;
  v2 = eproj_x_pos[v1];
  uint16 result = 1;
  if (v2 >= 0) {
    if (sign16(v2 - 512)) {
      v3 = eproj_y_pos[v1];
      if (v3 >= 0) {
        if (sign16(v3 - 512))
          return 0;
      }
    }
  }
  return result;
}

void Eproj_FuncE73E_MoveXY(uint16 k) {  // 0x86E73E
  int v1 = k >> 1;
  if (((g_word_7E97DC[v1] + 64) & 0x80) != 0) {
    AddToHiLo(&eproj_x_pos[v1], &eproj_x_subpos[v1], -IPAIR32(eproj_x_vel[v1], eproj_E[v1]));
  } else {
    AddToHiLo(&eproj_x_pos[v1], &eproj_x_subpos[v1], __PAIR32__(eproj_x_vel[v1], eproj_E[v1]));
  }
  if (((g_word_7E97DC[v1] + 128) & 0x80) != 0) {
    AddToHiLo(&eproj_y_pos[v1], &eproj_y_subpos[v1], -IPAIR32(eproj_y_vel[v1], eproj_F[v1]));
  } else {
    AddToHiLo(&eproj_y_pos[v1], &eproj_y_subpos[v1], __PAIR32__(eproj_y_vel[v1], eproj_F[v1]));
  }
}

static void EprojInit_BotwoonsBody(uint16 j) {  // 0x86EA31
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

static void EprojInit_BotwoonsSpit(uint16 j) {  // 0x86EBC6
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

static void EprojInit_YappingMawsBody(uint16 j) {  // 0x86EC62
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

const uint8 *EprojInstr_ECE3(uint16 k, const uint8 *epjp) {  // 0x86ECE3
  int v2 = k >> 1;
  uint16 x = eproj_x_pos[v2] + (NextRandom() & 0x3F) - 32;
  uint16 y = eproj_y_pos[v2] + ((uint16)(random_number & 0x3F00) >> 8) - 32;
  CreateSpriteAtPos(x, y, GET_WORD(epjp), 0);
  return epjp + 2;
}

const uint8 *EprojInstr_ED17(uint16 k, const uint8 *epjp) {  // 0x86ED17
  int v2 = k >> 1;
  uint16 x = eproj_x_pos[v2] + (NextRandom() & 0x1F) - 16;
  uint16 y = eproj_y_pos[v2] + ((uint16)(random_number & 0x1F00) >> 8) - 16;
  CreateSpriteAtPos(x, y, GET_WORD(epjp), 0);
  return epjp + 2;
}

const uint8 *EprojInstr_QueueSfx2_9(uint16 k, const uint8 *epjp) {  // 0x86EE8B
  QueueSfx2_Max1(9);
  return epjp;
}

const uint8 *EprojInstr_QueueSfx2_24(uint16 k, const uint8 *epjp) {  // 0x86EE97
  QueueSfx2_Max1(0x24);
  return epjp;
}

const uint8 *EprojInstr_QueueSfx2_B(uint16 k, const uint8 *epjp) {  // 0x86EEA3
  QueueSfx2_Max1(0xB);
  return epjp;
}

const uint8 *EprojInstr_EEAF(uint16 k, const uint8 *epjp) {  // 0x86EEAF
  uint16 v2 = RandomDropRoutine(k);
  if (k != 0 && sign16(v2 - 6)) {
    uint16 v3 = 2 * v2;
    int v4 = k >> 1;
    eproj_E[v4] = v3;
    eproj_instr_list_ptr[v4] = off_86EF04[v3 >> 1];
    eproj_instr_timers[v4] = 1;
    eproj_F[v4] = 400;
    eproj_pre_instr[v4] = FUNC16(EprojPreInstr_Pickup);
    eproj_properties[v4] &= ~0x4000;
    return INSTRB_RETURN_ADDR(eproj_instr_list_ptr[v4]);
  } else {
    int v6 = k >> 1;
    eproj_instr_timers[v6] = 1;
    eproj_properties[v6] = 12288;
    eproj_pre_instr[v6] = FUNC16(EprojPreInstr_Empty);
    eproj_instr_list_ptr[v6] = 0xECA3;
    return INSTRB_RETURN_ADDR(0xECA3);
  }
}

const uint8 *EprojInstr_HandleRespawningEnemy(uint16 k, const uint8 *epjp) {  // 0x86EF10
  if ((int16)eproj_killed_enemy_index[k >> 1] <= -2)
    RespawnEnemy(eproj_killed_enemy_index[k >> 1] & 0x7fff);
  return epjp;
}

static void EprojInit_F337(uint16 j) {  // 0x86EF29
  int v2 = j >> 1;
  eproj_x_pos[v2] = eproj_spawn_pt.x;
  eproj_y_pos[v2] = eproj_spawn_pt.y;
  eproj_gfx_idx[v2] = 0;
  eproj_enemy_header_ptr[v2] = eproj_spawn_varE24; // this is X?!
  uint16 v3 = RandomDropRoutine(j);
  if (v3 != 0 && sign16(v3 - 6)) { // bug, why does it compare x here.
    uint16 v4 = 2 * v3;
    eproj_E[v2] = v4;
    eproj_instr_list_ptr[v2] = off_86EF04[v4 >> 1];
    eproj_instr_timers[v2] = 1;
    eproj_F[v2] = 400;
    eproj_killed_enemy_index[j >> 1] = -1;
  } else {
    int v5 = j >> 1;
    eproj_instr_list_ptr[v5] = addr_word_86ECA3;
    eproj_instr_timers[v5] = 1;
    eproj_properties[v5] = 12288;
    eproj_pre_instr[v5] = FUNC16(EprojPreInstr_Empty);
  }
}


static void EprojInit_EnemyDeathExplosion(uint16 j) {  // 0x86EF89
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  int v2 = j >> 1;
  eproj_x_pos[v2] = v1->x_pos;
  eproj_y_pos[v2] = v1->y_pos;
  eproj_killed_enemy_index[v2] = cur_enemy_index;
  if ((v1->properties & 0x4000) != 0)
    eproj_killed_enemy_index[v2] = cur_enemy_index | 0x8000;
  eproj_enemy_header_ptr[v2] = v1->enemy_ptr;
  eproj_gfx_idx[v2] = 0;
  eproj_instr_list_ptr[v2] = off_86EFD5[eproj_init_param_1];
  eproj_instr_timers[v2] = 1;
}

static Func_V *const off_86F0AD[7] = {  // 0x86EFE0
  0,
  Eproj_Pickup_SmallHealth,
  Eproj_Pickup_BigHealth,
  Eproj_Pickup_PowerBombs,
  Eproj_Pickup_Missiles,
  Eproj_Pickup_SuperMissiles,
  0,
};

void EprojPreInstr_Pickup(uint16 k) {
  int v1 = k >> 1;
  if (!--eproj_F[v1])
    goto LABEL_7;
  if (CallSomeSamusCode(0xD)) {
    if (sign16(eproj_F[v1] - 384)) {
      uint16 v2, v3;
      v2 = abs16(eproj_x_pos[v1] - grapple_beam_end_x_pos);
      if (sign16(v2 - 16)) {
        v3 = abs16(eproj_y_pos[v1] - grapple_beam_end_y_pos);
        if (sign16(v3 - 16)) {
          off_86F0AD[eproj_E[v1] >> 1]();
LABEL_7:;
          int v4 = k >> 1;
          eproj_instr_list_ptr[v4] = addr_word_86ECA3;
          eproj_instr_timers[v4] = 1;
          eproj_properties[v4] = 12288;
          eproj_pre_instr[v4] = FUNC16(EprojPreInstr_Empty);
          return;
        }
      }
    }
  }
  int v5 = k >> 1;
  uint16 varE20 = LOBYTE(eproj_radius[v5]);
  uint16 varE22 = HIBYTE(eproj_radius[v5]);
  uint16 v6 = abs16(samus_x_pos - eproj_x_pos[v5]);
  bool v7 = v6 < samus_x_radius;
  uint16 v8 = v6 - samus_x_radius;
  if (v7 || v8 < varE20) {
    uint16 v9 = abs16(samus_y_pos - eproj_y_pos[v5]);
    v7 = v9 < samus_y_radius;
    uint16 v10 = v9 - samus_y_radius;
    if (v7 || v10 < varE22) {
      off_86F0AD[eproj_E[v5] >> 1]();
      int v11 = k >> 1;
      eproj_instr_list_ptr[v11] = addr_word_86ECA3;
      eproj_instr_timers[v11] = 1;
      eproj_properties[v11] = 12288;
      eproj_pre_instr[v11] = FUNC16(EprojPreInstr_Empty);
    }
  }
}

void Eproj_Pickup_SmallHealth(void) {  // 0x86F0BB
  Samus_RestoreHealth(5);
  QueueSfx2_Max1(1);
}

void Eproj_Pickup_BigHealth(void) {  // 0x86F0CA
  Samus_RestoreHealth(0x14);
  QueueSfx2_Max1(2);
}

void Eproj_Pickup_PowerBombs(void) {  // 0x86F0D9
  Samus_RestorePowerBombs(1);
  QueueSfx2_Max1(5);
}

void Eproj_Pickup_Missiles(void) {  // 0x86F0E8
  Samus_RestoreMissiles(2);
  QueueSfx2_Max1(3);
}

void Eproj_Pickup_SuperMissiles(void) {  // 0x86F0F7
  Samus_RestoreSuperMissiles(1);
  QueueSfx2_Max1(4);
}

static const uint8 byte_86F25E[6] = { 1, 2, 4, 6, 5, 3 };

uint16 RandomDropRoutine(uint16 k) {  // 0x86F106
  int8 v9; // cf
  uint8 r18, r22, r20;
  uint16 r24;

  int v1 = k >> 1;
  //varE2A = eproj_killed_enemy_index[v1] & 0x7FFF;
  uint16 varE28 = eproj_enemy_header_ptr[v1];
  if (varE28 == 0)
    goto LABEL_30;
  uint16 v2;
  v2 = get_EnemyDef_A2(varE28)->item_drop_chances_ptr;
  if (!v2)
    goto LABEL_30;
  uint8 Random;
  do
    Random = NextRandom();
  while (!Random);
  r20 = 255;
  r24 = 0;
  uint16 v5;
  v5 = 1;
  if ((uint16)(samus_reserve_health + samus_health) >= 0x1E) {
    if ((uint16)(samus_reserve_health + samus_health) < 0x32)
      goto LABEL_7;
    v5 = 0;
  }
  health_drop_bias_flag = v5;
LABEL_7:;
  const uint8 *v7 = RomPtr_B4(v2);
  if ((uint8)health_drop_bias_flag) {
    r18 = v7[1] + *v7;
    r22 = 3;
  } else {
    r18 = v7[3];
    r22 = 8;
    if (samus_health != samus_max_health || samus_reserve_health != samus_max_reserve_health) {
      r18 = v7[1] + *v7 + r18;
      r22 = r22 | 3;
    }
    if (samus_missiles != samus_max_missiles) {
      r18 = v7[2] + r18;
      r22 = r22 | 4;
    }
    if (samus_super_missiles != samus_max_super_missiles) {
      r20 = r20 - v7[4];
      r22 = r22 | 0x10;
    }
    if (samus_power_bombs != samus_max_power_bombs) {
      r20 = r20 - v7[5];
      r22 = r22 | 0x20;
    }
  }
  int i;
  for (i = 0; i != 4; ++i) {
    if (!(uint8)r18) {
      r22 = (uint8)r22 >> 4;
      v2 += 4;
      i = 4;
      goto LABEL_26;
    }
    v9 = r22 & 1;
    r22 = (uint8)r22 >> 1;
    if (v9) {
      const uint8 *v10 = RomPtr_B4(v2);
      uint16 RegWord = Mult8x8(r20, *v10);
      int divved = SnesDivide(RegWord, r18);
      uint16 v12 = r24;
      uint16 v13 = divved + v12;
      if (v13 >= Random)
        return byte_86F25E[i];
      r24 = v13;
    }
    ++v2;
  }
  do {
LABEL_26:
    v9 = r22 & 1;
    r22 = (uint8)r22 >> 1;
    if (v9) {
      const uint8 *v14 = RomPtr_B4(v2);
      if ((uint16)(r24 + *v14) >= Random)
        return byte_86F25E[i];
      r24 += *v14;
    }
    ++v2;
    ++i;
  } while (i != 6);
LABEL_30:
  i = 3;
  return byte_86F25E[i];
}

void RespawnEnemy(uint16 v0) {  // 0x86F264
  cur_enemy_index = v0;
  const uint8 *v1 = RomPtr_A1(room_enemy_population_ptr + (v0 >> 2));
  EnemyData *E = gEnemyData(v0);
  E->enemy_ptr = GET_WORD(v1);
  E->x_pos = GET_WORD(v1 + 2);
  E->y_pos = GET_WORD(v1 + 4);
  E->current_instruction = GET_WORD(v1 + 6);
  E->properties = GET_WORD(v1 + 8);
  E->extra_properties = GET_WORD(v1 + 10);
  E->parameter_1 = GET_WORD(v1 + 12);
  E->parameter_2 = GET_WORD(v1 + 14);
  EnemySpawnData *ESD = gEnemySpawnData(v0);
  E->palette_index = ESD->palette_index;
  E->vram_tiles_index = ESD->vram_tiles_index;
  E->frozen_timer = 0;
  E->flash_timer = 0;
  E->invincibility_timer = 0;
  E->timer = 0;
  E->frame_counter = 0;
  E->ai_var_A = 0;
  E->ai_var_B = 0;
  E->ai_var_C = 0;
  E->ai_var_D = 0;
  E->ai_var_E = 0;
  E->ai_preinstr = 0;
  E->instruction_timer = 1;
  EnemyDef *Edef = get_EnemyDef_A2(E->enemy_ptr);
  E->x_width = Edef->x_radius;
  E->y_height = Edef->y_radius;
  E->health = Edef->health;
  E->layer = Edef->layer;
  *(uint16 *)&E->bank = *(uint16 *)&Edef->bank;
  CallEnemyAi(Edef->bank << 16 | Edef->ai_init);
}


static void EprojInit_Sparks(uint16 j) {  // 0x86F391
  static const uint16 word_86F3D4[14] = { 0xffff, 0xb800, 0xffff, 0xc000, 0xffff, 0xe000, 0xffff, 0xff00, 0, 0x100, 0, 0x2000, 0, 0x4000 };
  int v2 = j >> 1;
  eproj_instr_list_ptr[v2] = addr_kEnemyDef_F353;
  EnemyData *v3 = gEnemyData(cur_enemy_index);
  eproj_x_pos[v2] = v3->x_pos;
  eproj_x_subpos[v2] = v3->x_subpos;
  eproj_y_pos[v2] = v3->y_pos + 8;
  eproj_y_subpos[v2] = v3->y_subpos;
  eproj_x_vel[v2] = 0;
  eproj_y_vel[v2] = 0;
  uint16 v4 = (NextRandom() & 0x1C) >> 1;
  eproj_F[v2] = g_word_86F3D4[v4 + 0];
  eproj_E[v2] = g_word_86F3D4[v4 + 1];
}

void EprojPreInstr_Sparks(uint16 k) {  // 0x86F3F0
  int v1 = k >> 1;
  if ((eproj_y_vel[v1] & 0x8000) == 0) {
    if (EprojBlockCollisition_Vertical(k) & 1) {
      eproj_instr_list_ptr[v1] = 0xF363;
      eproj_instr_timers[v1] = 1;

      eproj_F[v1] = eproj_F[v1] * 2 + (eproj_E[v1] >> 15);
      eproj_E[v1] *= 2;

      eproj_F[v1] = eproj_F[v1] * 2 + (eproj_E[v1] >> 15);
      eproj_E[v1] *= 2;

      eproj_x_vel[v1] = 0x8000;
      eproj_y_vel[v1] = -1;
      eproj_y_pos[v1] -= 2;
      return;
    }
    uint16 v2 = eproj_x_vel[v1];
    eproj_x_vel[v1] = v2 + 0x4000;
    uint16 v3 = __CFADD__uint16(v2, 0x4000) + eproj_y_vel[v1];
    if (v3 < 4)
      eproj_y_vel[v1] = v3;
  }
  AddToHiLo(&eproj_y_pos[v1], &eproj_y_subpos[v1], __PAIR32__(eproj_y_vel[v1], eproj_x_vel[v1]));
  AddToHiLo(&eproj_x_pos[v1], &eproj_x_subpos[v1], __PAIR32__(eproj_F[v1], eproj_E[v1]));
  if ((nmi_frame_counter_byte & 3) == 0)
    CreateSpriteAtPos(eproj_x_pos[v1], eproj_y_pos[v1], 48, eproj_gfx_idx[v1]);
}

void CallEprojInit(uint32 ea, uint16 j) {
  switch (ea) {
  case fnEproj_Init_0x8aaf: Eproj_Init_0x8aaf(j); return;
  case fnEproj_Init_0x8bc2_SkreeDownRight: Eproj_Init_0x8bc2_SkreeDownRight(j); return;
  case fnEproj_Init_0x8bd0_SkreeUpRight: Eproj_Init_0x8bd0_SkreeUpRight(j); return;
  case fnEproj_Init_0x8bde_SkreeDownLeft: Eproj_Init_0x8bde_SkreeDownLeft(j); return;
  case fnEproj_Init_0x8bec_SkreeUpLeft: Eproj_Init_0x8bec_SkreeUpLeft(j); return;
  case fnEprojInit_DraygonsGunk: EprojInit_DraygonsGunk(j); return;
  case fnEprojInit_DraygonsWallTurretProjs: EprojInit_DraygonsWallTurretProjs(j); return;
  case fnEprojInit_CrocomireProjectile: EprojInit_CrocomireProjectile(j); return;
  case fnEprojInit_CrocomireSpikeWallPieces: EprojInit_CrocomireSpikeWallPieces(j); return;
  case fnEprojInit_CrocomireBridgeCrumbling: EprojInit_CrocomireBridgeCrumbling(j); return;
  case fnEprojInit_9634: EprojInit_9634(j); return;
  case fnEprojInit_9642_RidleysFireball: EprojInit_9642_RidleysFireball(j); return;
  case fnEprojInit_9660_FireballExplosion: EprojInit_9660_FireballExplosion(j); return;
  case fnEprojInit_9688: EprojInit_9688(j); return;
  case fnEprojInit_9696: EprojInit_9696(j); return;
  case fnEprojInit_966C: EprojInit_966C(j); return;
  case fnEprojInit_967A: EprojInit_967A(j); return;
  case fnEprojInit_9734_CeresFallingDebris: EprojInit_9734_CeresFallingDebris(j); return;
  case fnEprojInit_PhantoonDestroyableFireballs: EprojInit_PhantoonDestroyableFireballs(j); return;
  case fnEprojInit_PhantoonStartingFireballs: EprojInit_PhantoonStartingFireballs(j); return;
  case fnEprojInit_RocksKraidSpits: EprojInit_RocksKraidSpits(j); return;
  case fnEprojInit_RocksFallingKraidCeiling: EprojInit_RocksFallingKraidCeiling(j); return;
  case fnEprojInit_RocksWhenKraidRises: EprojInit_RocksWhenKraidRises(j); return;
  case fnEprojInit_MiniKraidSpit: EprojInit_MiniKraidSpit(j); return;
  case fnEprojInit_MiniKraidSpikesLeft: EprojInit_MiniKraidSpikesLeft(j); return;
  case fnEprojInit_MiniKraidSpikesRight: EprojInit_MiniKraidSpikesRight(j); return;
  case fnEprojInit_WalkingLavaSeahorseFireball: EprojInit_WalkingLavaSeahorseFireball(j); return;
  case fnEprojInit_PirateMotherBrainLaser: EprojInit_PirateMotherBrainLaser(j); return;
  case fnEprojInit_PirateClaw: EprojInit_PirateClaw(j); return;
  case fnEprojInit_A379: EprojInit_A379(j); return;
  case fnEprojInit_CeresElevatorPad: EprojInit_CeresElevatorPad(j); return;
  case fnEprojInit_CeresElevatorPlatform: EprojInit_CeresElevatorPlatform(j); return;
  case fnEprojPreInstr_PrePhantomRoom: EprojPreInstr_PrePhantomRoom(j); return;
  case fnEprojInit_BombTorizoLowHealthDrool: EprojInit_BombTorizoLowHealthDrool(j); return;
  case fnEprojInit_BombTorizoLowHealthInitialDrool: EprojInit_BombTorizoLowHealthInitialDrool(j); return;
  case fnEprojInit_A977: EprojInit_A977(j); return;
  case fnEprojInit_BombTorizoExplosiveSwipe: EprojInit_BombTorizoExplosiveSwipe(j); return;
  case fnEprojInit_BombTorizoStatueBreaking: EprojInit_BombTorizoStatueBreaking(j); return;
  case fnEprojInit_BombTorizoLowHealthExplode: EprojInit_BombTorizoLowHealthExplode(j); return;
  case fnEprojInit_BombTorizoDeathExplosion: EprojInit_BombTorizoDeathExplosion(j); return;
  case fnEprojInit_AB07: EprojInit_AB07(j); return;
  case fnEprojInit_BombTorizosChozoOrbs: EprojInit_BombTorizosChozoOrbs(j); return;
  case fnEprojInit_GoldenTorizosChozoOrbs: EprojInit_GoldenTorizosChozoOrbs(j); return;
  case fnEprojInit_TorizoSonicBoom: EprojInit_TorizoSonicBoom(j); return;
  case fnEprojInit_WreckedShipChozoSpikeFootsteps: EprojInit_WreckedShipChozoSpikeFootsteps(j); return;
  case fnEprojInit_TourianStatueDustClouds: EprojInit_TourianStatueDustClouds(j); return;
  case fnEprojInit_TourianLandingDustCloudsRightFoot: EprojInit_TourianLandingDustCloudsRightFoot(j); return;
  case fnEprojInit_TorizoLandingDustCloudLeftFoot: EprojInit_TorizoLandingDustCloudLeftFoot(j); return;
  case fnEprojInit_GoldenTorizoEgg: EprojInit_GoldenTorizoEgg(j); return;
  case fnEprojInit_GoldenTorizoSuperMissile: EprojInit_GoldenTorizoSuperMissile(j); return;
  case fnEprojInit_GoldenTorizoEyeBeam: EprojInit_GoldenTorizoEyeBeam(j); return;
  case fnEprojInit_TourianEscapeShaftFakeWallExplode: EprojInit_TourianEscapeShaftFakeWallExplode(j); return;
  case fnEprojInit_LavaSeahorseFireball: EprojInit_LavaSeahorseFireball(j); return;
  case fnEprojInit_EyeDoorProjectile: EprojInit_EyeDoorProjectile(j); return;
  case fnEprojInit_EyeDoorSweat: EprojInit_EyeDoorSweat(j); return;
  case fnEprojInit_TourianStatueUnlockingParticleWaterSplash: EprojInit_TourianStatueUnlockingParticleWaterSplash(j); return;
  case fnEprojInit_TourianStatueEyeGlow: EprojInit_TourianStatueEyeGlow(j); return;
  case fnEprojInit_TourianStatueUnlockingParticle: EprojInit_TourianStatueUnlockingParticle(j); return;
  case fnEprojIni_TourianStatueUnlockingParticleTail: EprojIni_TourianStatueUnlockingParticleTail(j); return;
  case fnEprojInit_TourianStatueSoul: EprojInit_TourianStatueSoul(j); return;
  case fnEprojInit_TourianStatueBaseDecoration: EprojInit_TourianStatueBaseDecoration(j); return;
  case fnEprojInit_TourianStatueRidley: EprojInit_TourianStatueRidley(j); return;
  case fnEprojInit_TourianStatuePhantoon: EprojInit_TourianStatuePhantoon(j); return;
  case fnsub_86BB30: sub_86BB30(j); return;
  case fnEprojInit_NuclearWaffleBody: EprojInit_NuclearWaffleBody(j); return;
  case fnEprojInit_NorfairLavaquakeRocks: EprojInit_NorfairLavaquakeRocks(j); return;
  case fnEprojInit_ShaktoolAttackMiddleBackCircle: EprojInit_ShaktoolAttackMiddleBackCircle(j); return;
  case fnEprojInit_BDA2: EprojInit_BDA2(j); return;
  case fnEprojInit_MotherBrainRoomTurrets: EprojInit_MotherBrainRoomTurrets(j); return;
  case fnEprojInit_MotherBrainRoomTurretBullets: EprojInit_MotherBrainRoomTurretBullets(j); return;
  case fnEproj_MotherBrainsBlueRingLasers: Eproj_MotherBrainsBlueRingLasers(j); return;
  case fnEprojInit_MotherBrainBomb: EprojInit_MotherBrainBomb(j); return;
  case fnsub_86C605: sub_86C605(j); return;
  case fnEprojInit_MotherBrainDeathBeemFired: EprojInit_MotherBrainDeathBeemFired(j); return;
  case fnEprojInit_MotherBrainRainbowBeam: EprojInit_MotherBrainRainbowBeam(j); return;
  case fnEprojInit_MotherBrainsDrool: EprojInit_MotherBrainsDrool(j); return;
  case fnEprojInit_MotherBrainsDeathExplosion: EprojInit_MotherBrainsDeathExplosion(j); return;
  case fnEprojInit_MotherBrainsRainbowBeamExplosion: EprojInit_MotherBrainsRainbowBeamExplosion(j); return;
  case fnEprojInit_MotherBrainEscapeDoorParticles: EprojInit_MotherBrainEscapeDoorParticles(j); return;
  case fnEprojInit_MotherBrainPurpleBreathBig: EprojInit_MotherBrainPurpleBreathBig(j); return;
  case fnEprojInit_MotherBrainPurpleBreathSmall: EprojInit_MotherBrainPurpleBreathSmall(j); return;
  case fnEprojInit_TimeBombSetJapaneseText: EprojInit_TimeBombSetJapaneseText(j); return;
  case fnEprojInit_MotherBrainTubeFalling: EprojInit_MotherBrainTubeFalling(j); return;
  case fnEprojInit_MotherBrainGlassShatteringShard: EprojInit_MotherBrainGlassShatteringShard(j); return;
  case fnEprojInit_MotherBrainGlassShatteringSparkle: EprojInit_MotherBrainGlassShatteringSparkle(j); return;
  case fnEprojInit_KiHunterAcidSpitLeft: EprojInit_KiHunterAcidSpitLeft(j); return;
  case fnEprojInit_KiHunterAcidSpitRight: EprojInit_KiHunterAcidSpitRight(j); return;
  case fnEprojInit_KagosBugs: EprojInit_KagosBugs(j); return;
  case fnEprojInit_MaridiaFloatersSpikes: EprojInit_MaridiaFloatersSpikes(j); return;
  case fnEprojInit_WreckedShipRobotLaserDown: EprojInit_WreckedShipRobotLaserDown(j); return;
  case fnEprojInit_WreckedShipRobotLaserHorizontal: EprojInit_WreckedShipRobotLaserHorizontal(j); return;
  case fnEprojInit_WreckedShipRobotLaserUp: EprojInit_WreckedShipRobotLaserUp(j); return;
  case fnEprojInit_N00bTubeCrack: EprojInit_N00bTubeCrack(j); return;
  case fnEprojInit_N00bTubeShards: EprojInit_N00bTubeShards(j); return;
  case fnEprojInit_N00bTubeReleasedAirBubbles: EprojInit_N00bTubeReleasedAirBubbles(j); return;
  case fnsub_86D992: sub_86D992(j); return;
  case fnEprojInit_DBF2: EprojInit_DBF2(j); return;
  case fnEprojInit_Spores: EprojInit_Spores(j); return;
  case fnEprojInit_SporeSpawnStalk: EprojInit_SporeSpawnStalk(j); return;
  case fnEprojInit_SporeSpawners: EprojInit_SporeSpawners(j); return;
  case fnEprojInit_NamiFuneFireball: EprojInit_NamiFuneFireball(j); return;
  case fnEprojInit_LavaThrownByLavaman: EprojInit_LavaThrownByLavaman(j); return;
  case fnEprojInit_DustCloudOrExplosion: EprojInit_DustCloudOrExplosion(j); return;
  case fnEprojInit_EyeDoorSmoke: EprojInit_EyeDoorSmoke(j); return;
  case fnEprojInit_SpawnedShotGate: EprojInit_SpawnedShotGate(j); return;
  case fnEprojInit_ClosedDownwardsShotGate: EprojInit_ClosedDownwardsShotGate(j); return;
  case fnEprojInit_ClosedUpwardsShotGate: EprojInit_ClosedUpwardsShotGate(j); return;
  case fnEprojInit_SaveStationElectricity: EprojInit_SaveStationElectricity(j); return;
  case fnEprojInit_BotwoonsBody: EprojInit_BotwoonsBody(j); return;
  case fnEprojInit_BotwoonsSpit: EprojInit_BotwoonsSpit(j); return;
  case fnEprojInit_YappingMawsBody: EprojInit_YappingMawsBody(j); return;
  case fnEprojInit_F337: EprojInit_F337(j); return;
  case fnEprojInit_EnemyDeathExplosion: EprojInit_EnemyDeathExplosion(j); return;
  case fnEprojInit_Sparks: EprojInit_Sparks(j); return;
  default: Unreachable();
  }
}

void CallEprojPreInstr(uint32 ea, uint16 k) {
  switch (ea) {
  case fnnullsub_352: return;
  case fnEprojPreInstr_nullsub_297: return;
  case fnEprojPreInstr_nullsub_83: return;
  case fnEproj_PreInit_0x8aaf: Eproj_PreInit_0x8aaf(k); return;
  case fnEproj_PreInstr_SkreeParticle: Eproj_PreInstr_SkreeParticle(k); return;
  case fnnullsub_84: return;
  case fnEprojPreInstr_8DCA: EprojPreInstr_8DCA(k); return;
  case fnEprojPreInstr_DraygonsTurret_8DFF: EprojPreInstr_DraygonsTurret_8DFF(k); return;
  case fnEprojPreInstr_DraygonsGunk_8E0F: EprojPreInstr_DraygonsGunk_8E0F(k); return;
  case fnEprojPreInstr_CrocomireProjectile: EprojPreInstr_CrocomireProjectile(k); return;
  case fnsub_8690B3: sub_8690B3(k); return;
  case fnEprojPreInstr_CrocomireSpikeWallPieces: EprojPreInstr_CrocomireSpikeWallPieces(k); return;
  case fnEprojPreInstr_CrocomireBridgeCrumbling: EprojPreInstr_CrocomireBridgeCrumbling(k); return;
  case fnEprojPreInstr_9634: EprojPreInstr_9634(k); return;
  case fnEprojPreInstr_9642_RidleysFireball: EprojPreInstr_9642_RidleysFireball(k); return;
  case fnnullsub_85: return;
  case fnEprojPreInstr_966C: EprojPreInstr_966C(k); return;
  case fnEprojPreInstr_9688: EprojPreInstr_9688(k); return;
  case fnEprojPreInstr_96A4: EprojPreInstr_96A4(k); return;
  case fnEprojPreInstr_96C0: EprojPreInstr_96C0(k); return;
  case fnEprojPreInstr_96CE: EprojPreInstr_96CE(k); return;
  case fnEprojPreInstr_9734_CeresFallingDebris: EprojPreInstr_9734_CeresFallingDebris(k); return;
  case fnEprojPreInstr_PhantoonStartingFireballs: EprojPreInstr_PhantoonStartingFireballs(k); return;
  case fnEprojPreInstr_PhantoonStartingFireballs2: EprojPreInstr_PhantoonStartingFireballs2(k); return;
  case fnEprojPreInstr_PhantoonStartingFireballs3: EprojPreInstr_PhantoonStartingFireballs3(k); return;
  case fnnullsub_86: return;
  case fnEprojPreInstr_PhantoonDestroyableFireballs: EprojPreInstr_PhantoonDestroyableFireballs(k); return;
  case fnEprojPreInstr_PhantoonDestroyableFireballs_2: EprojPreInstr_PhantoonDestroyableFireballs_2(k); return;
  case fnEprojPreInstr_PhantoonDestroyableFireballs_3: EprojPreInstr_PhantoonDestroyableFireballs_3(k); return;
  case fnEprojPreInstr_PhantoonStartingFireballsB: EprojPreInstr_PhantoonStartingFireballsB(k); return;
  case fnEprojPreInstr_PhantoonStartingFireballsB_2: EprojPreInstr_PhantoonStartingFireballsB_2(k); return;
  case fnEprojPreInstr_KraidRocks: EprojPreInstr_KraidRocks(k); return;
  case fnEprojPreInstr_RocksFallingKraidCeiling: EprojPreInstr_RocksFallingKraidCeiling(k); return;
  case fnsub_869DA5: sub_869DA5(k); return;
  case fnEprojPreInit_MiniKraidSpit: EprojPreInit_MiniKraidSpit(k); return;
  case fnEprojPreInstr_MiniKraidSpikes: EprojPreInstr_MiniKraidSpikes(k); return;
  case fnEprojPreInstr_WalkingLavaSeahorseFireball: EprojPreInstr_WalkingLavaSeahorseFireball(k); return;
  case fnnullsub_87: return;
  case fnEprojPreInstr_PirateMotherBrainLaser_MoveLeft: EprojPreInstr_PirateMotherBrainLaser_MoveLeft(k); return;
  case fnEprojPreInstr_PirateMotherBrainLaser_MoveRight: EprojPreInstr_PirateMotherBrainLaser_MoveRight(k); return;
  case fnEprojPreInstr_PirateClawThrownLeft: EprojPreInstr_PirateClawThrownLeft(k); return;
  case fnEprojPreInstr_PirateClawThrownRight: EprojPreInstr_PirateClawThrownRight(k); return;
  case fnnullsub_88: return;
  case fnEprojPreInstr_CeresElevatorPad: EprojPreInstr_CeresElevatorPad(k); return;
  case fnEprojPreInstr_CeresElevatorPlatform: EprojPreInstr_CeresElevatorPlatform(k); return;
  case fnEprojPreInstr_PrePhantomRoom: EprojPreInstr_PrePhantomRoom(k); return;
  case fnsub_86A887: sub_86A887(k); return;
  case fnEprojPreInstr_A977: EprojPreInstr_A977(k); return;
  case fnnullsub_89: return;
  case fnEprojPreInstr_AB07: EprojPreInstr_AB07(k); return;
  case fnEprojPreInstr_BombTorizosChozoOrbs: EprojPreInstr_BombTorizosChozoOrbs(k); return;
  case fnEprojPreInstr_GoldenTorizosChozoOrbs: EprojPreInstr_GoldenTorizosChozoOrbs(k); return;
  case fnEprojPreInstr_TorizoSonicBoom: EprojPreInstr_TorizoSonicBoom(k); return;
  case fnEprojPreInstr_GoldenTorizoEgg: EprojPreInstr_GoldenTorizoEgg(k); return;
  case fnsub_86B0B9: sub_86B0B9(k); return;
  case fnsub_86B0DD: sub_86B0DD(k); return;
  case fnEprojPreInstr_GoldenTorizoSuperMissile: EprojPreInstr_GoldenTorizoSuperMissile(k); return;
  case fnEprojPreInstr_B237: EprojPreInstr_B237(k); return;
  case fnEprojPreInstr_GoldenTorizoEyeBeam: EprojPreInstr_GoldenTorizoEyeBeam(k); return;
  case fnnullsub_90: return;
  case fnsub_86B535: sub_86B535(k); return;
  case fnEprojPreInstr_EyeDoorProjectile: EprojPreInstr_EyeDoorProjectile(k); return;
  case fnEprojPreInstr_EyeDoorSweat: EprojPreInstr_EyeDoorSweat(k); return;
  case fnEprojPreInstr_TourianStatueUnlockingParticleWaterSplash: EprojPreInstr_TourianStatueUnlockingParticleWaterSplash(k); return;
  case fnEprojPreInstr_TourianStatueUnlockingParticle: EprojPreInstr_TourianStatueUnlockingParticle(k); return;
  case fnEprojPreInstr_TourianStatueSoul: EprojPreInstr_TourianStatueSoul(k); return;
  case fnEprojPreInstr_TourianStatueStuff: EprojPreInstr_TourianStatueStuff(k); return;
  case fnEprojPreInstr_BA42: EprojPreInstr_BA42(k); return;
  case fnnullsub_91: return;
  case fnnullsub_92: return;
  case fnEprojPreInstr_NorfairLavaquakeRocks: EprojPreInstr_NorfairLavaquakeRocks(k); return;
  case fnEprojPreInstr_NorfairLavaquakeRocks_Inner: EprojPreInstr_NorfairLavaquakeRocks_Inner(k); return;
  case fnEprojPreInstr_NorfairLavaquakeRocks_Inner2: EprojPreInstr_NorfairLavaquakeRocks_Inner2(k); return;
  case fnEprojInit_ShaktoolAttackFrontCircle: EprojInit_ShaktoolAttackFrontCircle(k); return;
  case fnEprojPreInstr_BE12: EprojPreInstr_BE12(k); return;
  case fnEprojPreInstr_MotherBrainRoomTurrets: EprojPreInstr_MotherBrainRoomTurrets(k); return;
  case fnEprojPreInstr_MotherBrainRoomTurretBullets: EprojPreInstr_MotherBrainRoomTurretBullets(k); return;
  case fnEproj_MoveToBlueRingSpawnPosition: Eproj_MoveToBlueRingSpawnPosition(k); return;
  case fnEprojPreInstr_MotherBrainBomb: EprojPreInstr_MotherBrainBomb(k); return;
  case fnEprojPreInstr_MotherBrainRainbowBeam: EprojPreInstr_MotherBrainRainbowBeam(k); return;
  case fnEprojPreInstr_C84D: EprojPreInstr_C84D(k); return;
  case fnEprojInit_MotherBrainsDrool_Falling: EprojInit_MotherBrainsDrool_Falling(k); return;
  case fnEprojPreInit_MotherBrainsDeathExplosion_0: EprojPreInit_MotherBrainsDeathExplosion_0(k); return;
  case fnEprojPreInstr_MotherBrainsRainbowBeamExplosion: EprojPreInstr_MotherBrainsRainbowBeamExplosion(k); return;
  case fnEprojPreInstr_MotherBrainsExplodedDoorParticles: EprojPreInstr_MotherBrainsExplodedDoorParticles(k); return;
  case fnnullsub_94: return;
  case fnEprojPreInstr_TimeBombSetJapaneseText: EprojPreInstr_TimeBombSetJapaneseText(k); return;
  case fnEprojPreInstr_MotherBrainTubeFalling: EprojPreInstr_MotherBrainTubeFalling(k); return;
  case fnEprojPreInstr_MotherBrainGlassShatteringShard: EprojPreInstr_MotherBrainGlassShatteringShard(k); return;
  case fnsub_86CFD5: sub_86CFD5(k); return;
  case fnsub_86CFE6: sub_86CFE6(k); return;
  case fnnullsub_95: return;
  case fnsub_86CFF8: sub_86CFF8(k); return;
  case fnEprojPreInstr_KagosBugs_Func1: EprojPreInstr_KagosBugs_Func1(k); return;
  case fnEprojPreInstr_KagosBugs: EprojPreInstr_KagosBugs(k); return;
  case fnnullsub_302: return;
  case fnEprojPreInstr_D0EC: EprojPreInstr_D0EC(k); return;
  case fnEprojPreInstr_D128: EprojPreInstr_D128(k); return;
  case fnEprojPreInstr_KagosBugs_Func2: EprojPreInstr_KagosBugs_Func2(k); return;
  case fnEprojPreInstr_MaridiaFloatersSpikes: EprojPreInstr_MaridiaFloatersSpikes(k); return;
  case fnEprojPreInstr_WreckedShipRobotLaser: EprojPreInstr_WreckedShipRobotLaser(k); return;
  case fnsub_86D7BF: sub_86D7BF(k); return;
  case fnsub_86D7DE: sub_86D7DE(k); return;
  case fnEprojPreInstr_N00bTubeShards: EprojPreInstr_N00bTubeShards(k); return;
  case fnsub_86D83D: sub_86D83D(k); return;
  case fnsub_86D89F: sub_86D89F(k); return;
  case fnsub_86D8DF: sub_86D8DF(k); return;
  case fnEprojPreInstr_SpikeShootingPlantSpikes: EprojPreInstr_SpikeShootingPlantSpikes(k); return;
  case fnEprojPreInstr_SpikeShootingPlantSpikes_MoveY1: EprojPreInstr_SpikeShootingPlantSpikes_MoveY1(k); return;
  case fnEprojPreInstr_SpikeShootingPlantSpikes_MoveY2: EprojPreInstr_SpikeShootingPlantSpikes_MoveY2(k); return;
  case fnEprojPreInstr_SpikeShootingPlantSpikes_MoveX1: EprojPreInstr_SpikeShootingPlantSpikes_MoveX1(k); return;
  case fnEprojPreInstr_SpikeShootingPlantSpikes_MoveX2: EprojPreInstr_SpikeShootingPlantSpikes_MoveX2(k); return;
  case fnEprojPreInstr_SpikeShootingPlantSpikes_0_MoveX1: EprojPreInstr_SpikeShootingPlantSpikes_0_MoveX1(k); return;
  case fnEprojPreInstr_SpikeShootingPlantSpikes_2_MoveX2: EprojPreInstr_SpikeShootingPlantSpikes_2_MoveX2(k); return;
  case fnEprojPreInstr_SpikeShootingPlantSpikes_1_MoveY1: EprojPreInstr_SpikeShootingPlantSpikes_1_MoveY1(k); return;
  case fnEprojPreInstr_SpikeShootingPlantSpikes_4_MoveY2: EprojPreInstr_SpikeShootingPlantSpikes_4_MoveY2(k); return;
  case fnEprojPreInstr_SpikeShootingPlantSpikes_6_MoveX1Y1: EprojPreInstr_SpikeShootingPlantSpikes_6_MoveX1Y1(k); return;
  case fnEprojPreInstr_SpikeShootingPlantSpikes_8_MoveX1Y2: EprojPreInstr_SpikeShootingPlantSpikes_8_MoveX1Y2(k); return;
  case fnEprojPreInstr_SpikeShootingPlantSpikes_7_MoveX2Y1: EprojPreInstr_SpikeShootingPlantSpikes_7_MoveX2Y1(k); return;
  case fnEprojPreInstr_SpikeShootingPlantSpikes_9_MoveX2Y2: EprojPreInstr_SpikeShootingPlantSpikes_9_MoveX2Y2(k); return;
  case fnEprojPreInstr_DBF2: EprojPreInstr_DBF2(k); return;
  case fnEprojPreInstr_DBF2_MoveX1: EprojPreInstr_DBF2_MoveX1(k); return;
  case fnEprojPreInstr_DBF2_MoveX2: EprojPreInstr_DBF2_MoveX2(k); return;
  case fnEprojPreInstr_DBF2_Func1: EprojPreInstr_DBF2_Func1(k); return;
  case fnEprojPreInstr_Spores: EprojPreInstr_Spores(k); return;
  case fnnullsub_96: return;
  case fnEprojPreInstr_SporeSpawners: EprojPreInstr_SporeSpawners(k); return;
  case fnEprojPreInstr_NamiFuneFireball: EprojPreInstr_NamiFuneFireball(k); return;
  case fnsub_86E049: sub_86E049(k); return;
  case fnEprojPreInstr_DustCloudOrExplosion: EprojPreInstr_DustCloudOrExplosion(k); return;
  case fnEprojPreInstr_nullsub_98: return;
  case fnEprojPreInstr_nullsub_99: return;
  case fnEprojPreInstr_E605: EprojPreInstr_E605(k); return;
  case fnnullsub_100: return;
  case fnEprojPreInstr_BotwoonsBody: EprojPreInstr_BotwoonsBody(k); return;
  case fnEprojPreInstr_BotwoonsSpit: EprojPreInstr_BotwoonsSpit(k); return;
  case fnEprojPreInstr_Empty2: return;
  case fnEprojPreInstr_Empty: return;
  case fnEprojPreInstr_Pickup: EprojPreInstr_Pickup(k); return;
  case fnEprojPreInstr_Sparks: EprojPreInstr_Sparks(k); return;
  case fnnullsub_366: return;
  default: Unreachable();
  }
}

const uint8 *CallEprojInstr(uint32 ea, uint16 k, const uint8 *j) {
  switch (ea) {
  case fnEprojInstr_Delete: return EprojInstr_Delete(k, j);
  case fnEprojInstr_Sleep: return EprojInstr_Sleep(k, j);
  case fnEprojInstr_SetPreInstr_: return EprojInstr_SetPreInstr_(k, j);
  case fnEprojInstr_ClearPreInstr: return EprojInstr_ClearPreInstr(k, j);
  case fnEprojInstr_CallFunc: return EprojInstr_CallFunc(k, j);
  case fnEprojInstr_Goto: return EprojInstr_Goto(k, j);
  case fnEprojInstr_GotoRel: return EprojInstr_GotoRel(k, j);
  case fnEprojInstr_DecTimerAndGotoIfNonZero: return EprojInstr_DecTimerAndGotoIfNonZero(k, j);
  case fnEprojInstr_DecTimerAndGotoRelIfNonZero: return EprojInstr_DecTimerAndGotoRelIfNonZero(k, j);
  case fnEprojInstr_SetTimer: return EprojInstr_SetTimer(k, j);
  case fnEprojInstr_MoveRandomlyWithinRadius: return EprojInstr_MoveRandomlyWithinRadius(k, j);
  case fnEprojInstr_SetProjectileProperties: return EprojInstr_SetProjectileProperties(k, j);
  case fnEprojInstr_ClearProjectileProperties: return EprojInstr_ClearProjectileProperties(k, j);
  case fnEprojInstr_EnableCollisionWithSamusProj: return EprojInstr_EnableCollisionWithSamusProj(k, j);
  case fnEprojInstr_DisableCollisionWithSamusProj: return EprojInstr_DisableCollisionWithSamusProj(k, j);
  case fnEprojInstr_DisableCollisionWithSamus: return EprojInstr_DisableCollisionWithSamus(k, j);
  case fnEprojInstr_EnableCollisionWithSamus: return EprojInstr_EnableCollisionWithSamus(k, j);
  case fnEprojInstr_SetToNotDieOnContact: return EprojInstr_SetToNotDieOnContact(k, j);
  case fnEprojInstr_SetToDieOnContact: return EprojInstr_SetToDieOnContact(k, j);
  case fnEprojInstr_SetLowPriority: return EprojInstr_SetLowPriority(k, j);
  case fnEprojInstr_SetHighPriority: return EprojInstr_SetHighPriority(k, j);
  case fnEprojInstr_SetXyRadius: return EprojInstr_SetXyRadius(k, j);
  case fnEprojInstr_SetXyRadiusZero: return EprojInstr_SetXyRadiusZero(k, j);
  case fnEprojInstr_CalculateDirectionTowardsSamus: return EprojInstr_CalculateDirectionTowardsSamus(k, j);
  case fnEprojInstr_WriteColorsToPalette: return EprojInstr_WriteColorsToPalette(k, j);
  case fnEprojInstr_QueueMusic: return EprojInstr_QueueMusic(k, j);
  case fnEprojInstr_QueueSfx1_Max6: return EprojInstr_QueueSfx1_Max6(k, j);
  case fnEprojInstr_QueueSfx2_Max6: return EprojInstr_QueueSfx2_Max6(k, j);
  case fnEprojInstr_QueueSfx3_Max6: return EprojInstr_QueueSfx3_Max6(k, j);
  case fnEprojInstr_QueueSfx1_Max15: return EprojInstr_QueueSfx1_Max15(k, j);
  case fnEprojInstr_QueueSfx2_Max15: return EprojInstr_QueueSfx2_Max15(k, j);
  case fnEprojInstr_QueueSfx3_Max15: return EprojInstr_QueueSfx3_Max15(k, j);
  case fnEprojInstr_QueueSfx1_Max3: return EprojInstr_QueueSfx1_Max3(k, j);
  case fnEprojInstr_QueueSfx2_Max3: return EprojInstr_QueueSfx2_Max3(k, j);
  case fnEprojInstr_QueueSfx3_Max3: return EprojInstr_QueueSfx3_Max3(k, j);
  case fnEprojInstr_QueueSfx1_Max9: return EprojInstr_QueueSfx1_Max9(k, j);
  case fnEprojInstr_QueueSfx2_Max9: return EprojInstr_QueueSfx2_Max9(k, j);
  case fnEprojInstr_QueueSfx3_Max9: return EprojInstr_QueueSfx3_Max9(k, j);
  case fnEprojInstr_QueueSfx1_Max1: return EprojInstr_QueueSfx1_Max1(k, j);
  case fnEprojInstr_QueueSfx2_Max1: return EprojInstr_QueueSfx2_Max1(k, j);
  case fnEprojInstr_QueueSfx3_Max1: return EprojInstr_QueueSfx3_Max1(k, j);
  case fnEprojInstr_SpawnEnemyDropsWithDraygonsEyeDrops: return EprojInstr_SpawnEnemyDropsWithDraygonsEyeDrops(k, j);
  case fnEprojInstr_868D99: return EprojInstr_868D99(k, j);
  case fnEprojInstr_DisableCollisionsWithSamus: return EprojInstr_DisableCollisionsWithSamus(k, j);
  case fnEprojInstr_95BA: return EprojInstr_95BA(k, j);
  case fnEprojInstr_95ED: return EprojInstr_95ED(k, j);
  case fnEprojInstr_9620: return EprojInstr_9620(k, j);
  case fnEprojInstr_980E: return EprojInstr_980E(k, j);
  case fnEprojInstr_SetPreInstrAndRun: return EprojInstr_SetPreInstrAndRun(k, j);
  case fnEprojInstr_GotoWithProbability25: return EprojInstr_GotoWithProbability25(k, j);
  case fnEprojInstr_SpawnEnemyDrops: return EprojInstr_SpawnEnemyDrops(k, j);
  case fnEprojInstr_GotoDependingOnXDirection: return EprojInstr_GotoDependingOnXDirection(k, j);
  case fnEprojInstr_ResetXYpos1: return EprojInstr_ResetXYpos1(k, j);
  case fnEprojInstr_MoveY_Minus4: return EprojInstr_MoveY_Minus4(k, j);
  case fnEprojInstr_SetVelTowardsSamus1: return EprojInstr_SetVelTowardsSamus1(k, j);
  case fnEprojInstr_SetVelTowardsSamus2: return EprojInstr_SetVelTowardsSamus2(k, j);
  case fnEprojInstr_GotoIfFunc1: return EprojInstr_GotoIfFunc1(k, j);
  case fnEprojInstr_ResetXYpos2: return EprojInstr_ResetXYpos2(k, j);
  case fnEprojInstr_SpawnTourianStatueUnlockingParticle: return EprojInstr_SpawnTourianStatueUnlockingParticle(k, j);
  case fnEprojInstr_Earthquake: return EprojInstr_Earthquake(k, j);
  case fnEprojInstr_SpawnTourianStatueUnlockingParticleTail: return EprojInstr_SpawnTourianStatueUnlockingParticleTail(k, j);
  case fnEprojInstr_AddToYpos: return EprojInstr_AddToYpos(k, j);
  case fnEprojInstr_SwitchJump: return EprojInstr_SwitchJump(k, j);
  case fnEprojInstr_UserPalette0: return EprojInstr_UserPalette0(k, j);
  case fnEprojInstr_Add12ToY: return EprojInstr_Add12ToY(k, j);
  case fnEprojInstr_MotherBrainPurpleBreathIsActive: return EprojInstr_MotherBrainPurpleBreathIsActive(k, j);
  case fnEprojInstr_D15C: return EprojInstr_D15C(k, j);
  case fnEprojInstr_D1B6: return EprojInstr_D1B6(k, j);
  case fnEprojInstr_D1C7: return EprojInstr_D1C7(k, j);
  case fnEprojInstr_D1CE: return EprojInstr_D1CE(k, j);
  case fnEprojInstr_AssignNewN00bTubeShardVelocity: return EprojInstr_AssignNewN00bTubeShardVelocity(k, j);
  case fnEprojInstr_SetN00bTubeShardX: return EprojInstr_SetN00bTubeShardX(k, j);
  case fnEprojInstr_D62A: return EprojInstr_D62A(k, j);
  case fnEprojInstr_SetXvelRandom: return EprojInstr_SetXvelRandom(k, j);
  case fnEprojInstr_DC5A: return EprojInstr_DC5A(k, j);
  case fnEprojInstr_SpawnEnemyDrops_0: return EprojInstr_SpawnEnemyDrops_0(k, j);
  case fnEprojInstr_SpawnSporesEproj: return EprojInstr_SpawnSporesEproj(k, j);
  case fnEprojInstr_DFEA: return EprojInstr_DFEA(k, j);
  case fnEprojInstr_SetYVel: return EprojInstr_SetYVel(k, j);
  case fnEprojInstr_ECE3: return EprojInstr_ECE3(k, j);
  case fnEprojInstr_ED17: return EprojInstr_ED17(k, j);
  case fnEprojInstr_QueueSfx2_9: return EprojInstr_QueueSfx2_9(k, j);
  case fnEprojInstr_QueueSfx2_24: return EprojInstr_QueueSfx2_24(k, j);
  case fnEprojInstr_QueueSfx2_B: return EprojInstr_QueueSfx2_B(k, j);
  case fnEprojInstr_EEAF: return EprojInstr_EEAF(k, j);
  case fnEprojInstr_HandleRespawningEnemy: return EprojInstr_HandleRespawningEnemy(k, j);
  case fnEprojInstr_SetPreInstrA: return EprojInstr_SetPreInstrA(k, j);
  case fnEprojInstr_SetPreInstrB: return EprojInstr_SetPreInstrB(k, j);
  case fnEprojPreInstr_PirateMotherBrainLaser_MoveRight: EprojPreInstr_PirateMotherBrainLaser_MoveRight(k); return j;
  case fnEprojPreInstr_PirateMotherBrainLaser_MoveLeft: EprojPreInstr_PirateMotherBrainLaser_MoveLeft(k); return j;
  case fnEprojInstr_A3BE: return EprojInstr_A3BE(k, j);
  case fnEprojInstr_9270: return EprojInstr_9270(k, j);
  case fnnullsub_82: return j; // really j
  case fnsub_86B13E: return sub_86B13E(k, j);
  default: Unreachable(); return NULL;
  }
}
