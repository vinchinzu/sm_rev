// Enemy projectile Tourian and Mother Brain families split out of sm_86.c.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "eproj_internal.h"

#define off_86C929 ((uint16*)RomFixedPtr(0x86c929))
#define kEproj_MotherBrainRoomTurrets_DirectionIndexes ((uint16*)RomFixedPtr(0x86bee1))
#define kEproj_MotherBrainRoomTurrets_AllowedRotations ((uint16*)RomFixedPtr(0x86bec9))
#define kEproj_MotherBrainRoomTurrets_InstrLists ((uint16*)RomFixedPtr(0x86beb9))
#define g_off_86C040 ((uint16*)RomFixedPtr(0x86c040))
#define kEprojInit_MotherBrainGlassShatteringShard_InstrPtrs ((uint16*)RomFixedPtr(0x86ce41))

static const uint16 kEprojInit_TourianStatueEyeGlow_X[4] = { 0x84, 0x7a, 0x9e, 0x68 };
static const uint16 kEprojInit_TourianStatueEyeGlow_Y[4] = { 0x90, 0x51, 0x80, 0x72 };
static const uint16 kEprojInit_TourianStatueEyeGlow_Colors[16] = {
  0x6bff, 0x33b, 0x216, 0x173, 0x7f5f, 0x7c1f, 0x5816, 0x300c,
  0x7f5a, 0x7ec0, 0x6de0, 0x54e0, 0x6bfa, 0x3be0, 0x2680, 0x1580,
};
static const int16 kEprojInit_MotherBrainRoomTurrets_X[12] = {
  0x398, 0x348, 0x328, 0x2d8, 0x288, 0x268, 0x218, 0x1c8, 0x1a8, 0x158, 0x108, 0xe8,
};
static const int16 kEprojInit_MotherBrainRoomTurrets_Y[12] = {
  0x30, 0x40, 0x40, 0x30, 0x40, 0x40, 0x30, 0x40, 0x40, 0x30, 0x40, 0x40,
};
static const int16 kEproj_MotherBrainRoomTurretBullets_X[8] = { -17, -12, 0, 12, 17, 12, 0, -12 };
static const int16 kEproj_MotherBrainRoomTurretBullets_Y[8] = { -9, 3, 7, 3, -9, -19, -21, -19 };
static const int16 kEproj_MotherBrainRoomTurretBullets_Xvel[8] = { -704, -498, 0, 498, 704, 498, 0, -498 };
static const int16 kEproj_MotherBrainRoomTurretBullets_Yvel[8] = { 0, 498, 704, 498, 0, -498, -704, -498 };
static const int16 kMotherBrainsBomb_Yaccel[10] = { 7, 0x10, 0x20, 0x40, 0x70, 0xb0, 0xf0, 0x130, 0x170, 0 };
static const int16 kEprojInit_MotherBrainsDrool[12] = { 6, 0x14, 0xe, 0x12, 8, 0x17, 0xa, 0x13, 0xb, 0x19, 0xc, 0x12 };
static const int16 kEprojInit_MotherBrainGlassShatteringShard_X[3] = { 8, -40, -16 };
static const int16 kEprojInit_MotherBrainGlassShatteringShard_Y[3] = { 32, 32, 32 };

static Rect16U Eproj_GetCollDetectRect(uint16 k);
uint8 Eproj_MotherBrainRoomTurretBullets_CheckIfTurretOnScreen(uint16 k);
void Eproj_MotherBrainRoomTurretBullets_Func2(uint16 k);
void Eproj_SetXvelRandom(uint16 k);
void Eproj_SetYvelRandom(uint16 k);
static uint8 Eproj_CheckForBombCollisionWithRect(Rect16U rect);
static uint8 Eproj_CheckForEnemyCollisionWithRect(uint16 k, Rect16U rect);
static uint8 Eproj_CheckForCollisionWithSamus(uint16 k);
static void sub_86C320(uint16 k);
uint8 CheckForCollisionWithShitroid_DoubleRet(uint16 k);
uint8 CheckForBlueRingCollisionWithRoom(uint16 k);
void Eproj_Earthqhake5(uint16 k);
void BlueRingContactExplosion(uint16 k);
static void sub_86C42E(uint16 k);
uint8 MotherBrainBomb_Bomb_CollDetect_DoubleRet(uint16 k);
uint8 MoveMotherBrainBomb(uint16 k, uint16 a);
static void CallMotherBrainTubeFallingFunc(uint32 ea, uint16 k);

const uint8 *EprojInstr_Earthquake(uint16 k, const uint8 *epjp) {  // 0x86B7F5
  earthquake_type = 1;
  earthquake_timer |= 0x20;
  return epjp;
}

const uint8 *EprojInstr_SpawnTourianStatueUnlockingParticleTail(uint16 k, const uint8 *epjp) {  // 0x86B818
  SpawnEprojWithRoomGfx(addr_kEproj_TourianStatueUnlockingParticleTail, k);
  return epjp;
}

const uint8 *EprojInstr_AddToYpos(uint16 k, const uint8 *epjp) {  // 0x86B841
  eproj_y_pos[k >> 1] += GET_WORD(epjp);
  return epjp + 2;
}

void EprojInit_TourianStatueUnlockingParticleWaterSplash(uint16 j) {  // 0x86B87A
  int v1 = j >> 1;
  eproj_x_pos[v1] = eproj_x_pos[eproj_init_param_1 >> 1];
  eproj_y_pos[v1] = fx_y_pos - 4;
}

void EprojInit_TourianStatueEyeGlow(uint16 j) {  // 0x86B88E
  uint16 v1 = eproj_init_param_1;
  int v2 = eproj_init_param_1 >> 1;
  int v3 = j >> 1;
  eproj_x_pos[v3] = kEprojInit_TourianStatueEyeGlow_X[v2];
  eproj_y_pos[v3] = kEprojInit_TourianStatueEyeGlow_Y[v2];
  uint16 v4 = 4 * v1;
  for (int i = 498; i != 506; i += 2) {
    palette_buffer[i >> 1] = kEprojInit_TourianStatueEyeGlow_Colors[v4 >> 1];
    v4 += 2;
  }
}

void EprojInit_TourianStatueUnlockingParticle(uint16 j) {  // 0x86B8B5
  int v1 = eproj_init_param_1 >> 1;
  int v2 = j >> 1;
  eproj_x_pos[v2] = eproj_x_pos[v1];
  eproj_y_pos[v2] = eproj_y_pos[v1];
  uint16 v3 = 2 * (uint8)((NextRandom() & 0x3F) - 32);
  eproj_E[v2] = v3;
  int v4 = v3 >> 1;
  eproj_x_vel[v2] = kSinCosTable8bit_Sext[v4 + 64];
  eproj_y_vel[v2] = 4 * kSinCosTable8bit_Sext[v4];
}

void EprojIni_TourianStatueUnlockingParticleTail(uint16 v0) {  // 0x86B8E8
  int v1 = eproj_init_param_1 >> 1;
  int v2 = v0 >> 1;
  eproj_x_pos[v2] = eproj_x_pos[v1];
  eproj_y_pos[v2] = eproj_y_pos[v1];
}

void EprojInit_TourianStatueSoul(uint16 j) {  // 0x86B8F8
  int v1 = eproj_init_param_1 >> 1;
  int v2 = j >> 1;
  eproj_x_pos[v2] = kEprojInit_TourianStatueEyeGlow_X[v1];
  eproj_y_pos[v2] = kEprojInit_TourianStatueEyeGlow_Y[v1];
  eproj_y_vel[v2] = -1024;
}

void EprojInit_TourianStatueBaseDecoration(uint16 j) {  // 0x86B93E
  int v1 = j >> 1;
  eproj_E[v1] = 120;
  eproj_x_pos[v1] = 120;
  eproj_F[v1] = 184;
  eproj_y_pos[v1] = 184;
}

void EprojInit_TourianStatueRidley(uint16 j) {  // 0x86B951
  int v1 = j >> 1;
  eproj_E[v1] = 142;
  eproj_x_pos[v1] = 142;
  eproj_F[v1] = 85;
  eproj_y_pos[v1] = 85;
}

void EprojInit_TourianStatuePhantoon(uint16 j) {  // 0x86B964
  int v1 = j >> 1;
  eproj_E[v1] = 132;
  eproj_x_pos[v1] = 132;
  eproj_F[v1] = 136;
  eproj_y_pos[v1] = 136;
}

void EprojPreInstr_TourianStatueUnlockingParticleWaterSplash(uint16 k) {  // 0x86B977
  eproj_y_pos[k >> 1] = fx_y_pos - 4;
}

void EprojPreInstr_TourianStatueUnlockingParticle(uint16 k) {  // 0x86B982
  int v1 = k >> 1;
  int32 amt = INT16_SHL8(eproj_x_vel[v1]);
  AddToHiLo(&eproj_x_pos[v1], &eproj_x_subpos[v1], amt);
  uint16 v7 = fx_y_pos - eproj_y_pos[v1];
  amt = INT16_SHL8(eproj_y_vel[v1]);
  AddToHiLo(&eproj_y_pos[v1], &eproj_y_subpos[v1], amt);
  if (((v7 ^ (fx_y_pos - eproj_y_pos[v1])) & 0x8000) != 0)
    SpawnEprojWithRoomGfx(addr_stru_86BA5C, k);
  if ((eproj_y_pos[v1] & 0xFF00) == 256) {
    eproj_instr_list_ptr[v1] = addr_off_86B79F;
    eproj_instr_timers[v1] = 1;
  } else {
    eproj_y_vel[v1] += 16;
  }
}

void EprojPreInstr_TourianStatueSoul(uint16 k) {  // 0x86B9FD
  int v1 = k >> 1;
  int32 amt = INT16_SHL8(eproj_y_vel[v1]);
  AddToHiLo(&eproj_y_pos[v1], &eproj_y_subpos[v1], amt);
  if ((eproj_y_pos[v1] & 0x100) != 0) {
    eproj_instr_list_ptr[v1] = addr_off_86B79F;
    eproj_instr_timers[v1] = 1;
  }
  eproj_y_vel[v1] -= 128;
}

void EprojPreInstr_BA42(uint16 k) {  // 0x86BA42
  int v1 = k >> 1;
  eproj_x_pos[v1] = eproj_E[v1];
  eproj_y_pos[v1] = eproj_F[v1] + layer1_y_pos - *(uint16 *)&hdma_window_1_left_pos[0].field_0;
}

void EprojPreInstr_TourianStatueStuff(uint16 k) {  // 0x86BA37
  if (!tourian_entrance_statue_animstate)
    tourian_entrance_statue_finished |= 0x8000;
  EprojPreInstr_BA42(k);
}

void EprojInit_MotherBrainRoomTurrets(uint16 j) {  // 0x86BE4F
  int v1 = j >> 1;
  eproj_gfx_idx[v1] = 1024;
  uint16 v2 = 2 * eproj_init_param_1;
  uint16 v3 = kEproj_MotherBrainRoomTurrets_DirectionIndexes[eproj_init_param_1] | 0x100;
  eproj_y_subpos[v1] = v3;
  eproj_instr_list_ptr[v1] = kEproj_MotherBrainRoomTurrets_InstrLists[(uint16)(2 * (uint8)v3) >> 1];
  int v4 = v2 >> 1;
  eproj_x_pos[v1] = kEprojInit_MotherBrainRoomTurrets_X[v4];
  eproj_y_pos[v1] = kEprojInit_MotherBrainRoomTurrets_Y[v4];
  eproj_x_subpos[v1] = kEproj_MotherBrainRoomTurrets_AllowedRotations[v4];
  Eproj_SetXvelRandom(j);
  Eproj_SetYvelRandom(j);
}

void EprojInit_MotherBrainRoomTurretBullets(uint16 j) {  // 0x86BF59
  int v1 = j >> 1;
  eproj_F[v1] = 0;
  eproj_gfx_idx[v1] = 1024;
  uint16 v2 = 2 * LOBYTE(eproj_y_subpos[eproj_init_param_1 >> 1]);
  eproj_E[v1] = v2;
  int v3 = v2 >> 1;
  Point16U pt = { kEproj_MotherBrainRoomTurretBullets_X[v3], kEproj_MotherBrainRoomTurretBullets_Y[v3] };
  eproj_x_vel[v1] = kEproj_MotherBrainRoomTurretBullets_Xvel[v3];
  eproj_y_vel[v1] = kEproj_MotherBrainRoomTurretBullets_Yvel[v3];
  int v4 = eproj_init_param_1 >> 1;
  eproj_x_pos[v1] = pt.x + eproj_x_pos[v4];
  eproj_y_pos[v1] = pt.y + eproj_y_pos[v4];
}

void EprojPreInstr_MotherBrainRoomTurrets(uint16 k) {  // 0x86BFDF
  if (Eproj_MotherBrainRoomTurretBullets_CheckIfTurretOnScreen(k) & 1) {
    if (gRam7800_Default(0)->var_1D)
      *(uint16 *)((uint8 *)eproj_id + k) = 0;
  } else if (gRam7800_Default(0)->var_1D) {
    *(uint16 *)((uint8 *)eproj_id + k) = 0;
    int v5 = k >> 1;
    eproj_spawn_pt = (Point16U){ eproj_x_pos[v5], eproj_y_pos[v5] };
    SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 0xC);
  } else {
    int v1 = k >> 1;
    bool v2 = eproj_x_vel[v1]-- == 1;
    if (v2) {
      Eproj_SetXvelRandom(k);
      Eproj_MotherBrainRoomTurretBullets_Func2(k);
      int v3 = k >> 1;
      eproj_instr_list_ptr[v3] = g_off_86C040[LOBYTE(eproj_y_subpos[v3])];
      eproj_instr_timers[v3] = 1;
    }
    int v4 = k >> 1;
    v2 = eproj_y_vel[v4]-- == 1;
    if (v2) {
      Eproj_SetYvelRandom(k);
      SpawnEprojWithRoomGfx(addr_kEproj_MotherBrainRoomTurretBullets, k);
    }
  }
}

void Eproj_MotherBrainRoomTurretBullets_Func2(uint16 v0) {  // 0x86C050
  uint8 r20 = (*((uint8 *)eproj_y_subpos + v0 + 1) + *((uint8 *)eproj_y_subpos + v0)) & 7;
  uint16 r18 = eproj_x_subpos[v0 >> 1];
  if (RomPtr_86(r18)[r20 & 7]) {
    *((uint8 *)eproj_y_subpos + v0) = r20;
  } else {
    int8 v1 = -*((uint8 *)eproj_y_subpos + v0 + 1);
    *((uint8 *)eproj_y_subpos + v0 + 1) = v1;
    *((uint8 *)eproj_y_subpos + v0) += v1;
  }
}

void Eproj_SetXvelRandom(uint16 k) {  // 0x86C08E
  uint16 random = (uint8)NextRandom();
  if (sign16((uint8)random - 32))
    random = 32;
  eproj_x_vel[k >> 1] = random;
}

void Eproj_SetYvelRandom(uint16 k) {  // 0x86C0A1
  uint16 random = (uint8)NextRandom();
  if (sign16((uint8)random - 128))
    random = 128;
  eproj_y_vel[k >> 1] = random;
}

uint8 Eproj_MotherBrainRoomTurretBullets_CheckIfTurretOnScreen(uint16 k) {  // 0x86C0B4
  int16 v2;
  int16 v3;
  int16 v4;
  int16 v5;

  int v1 = k >> 1;
  v2 = eproj_y_pos[v1];
  uint8 result = 1;
  if (v2 >= 0) {
    v3 = v2 + 16 - layer1_y_pos;
    if (v3 >= 0) {
      if (sign16(v3 - 256)) {
        v4 = eproj_x_pos[v1];
        if (v4 >= 0) {
          v5 = v4 + 4 - layer1_x_pos;
          if (v5 >= 0) {
            if (sign16(v5 - 264))
              return 0;
          }
        }
      }
    }
  }
  return result;
}

void EprojPreInstr_MotherBrainRoomTurretBullets(uint16 k) {  // 0x86C0E0
  int v1 = k >> 1;
  eproj_properties[v1] ^= 0x8000;
  MoveEprojWithVelocity(k);
  if (Ridley_Func_103(eproj_x_pos[v1], eproj_y_pos[v1]) & 1)
    eproj_id[eproj_index >> 1] = 0;
}

void EprojPreInstr_MotherBrainBomb(uint16 k) {  // 0x86C4C8
  int16 v2;

  if (MotherBrainBomb_Bomb_CollDetect_DoubleRet(k))
    return;
  int v1 = k >> 1;
  if (eproj_F[v1]) {
    int v4 = k >> 1;
    uint16 v5 = kMotherBrainsBomb_Yaccel[eproj_F[v4] >> 1];
    if (!v5) {
      eproj_x_vel[v4] = 0;
      eproj_y_vel[v4] = 0;
      --enemy_ram7800[1].kraid.kraid_mouth_flags;
      eproj_id[v4] = 0;
      eproj_spawn_pt = (Point16U){ eproj_x_pos[v4], eproj_y_pos[v4] };
      SpawnEprojWithRoomGfx(addr_stru_869650, LOBYTE(eproj_x_subpos[v4]));
      eproj_spawn_pt = (Point16U){ eproj_x_pos[v4], eproj_y_pos[v4] };
      SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 3);
      QueueSfx3_Max6(0x13);
      return;
    }
    if (MoveMotherBrainBomb(k, v5) & 1)
      goto LABEL_5;
  } else {
    v2 = abs16(eproj_x_vel[v1]) - 2;
    if (v2 < 0)
      v2 = 0;
    eproj_x_vel[v1] = sign16(eproj_x_vel[v1]) ? -v2 : v2;
    if (MoveMotherBrainBomb(k, 7) & 1) {
LABEL_5:
      int v3 = k >> 1;
      eproj_F[v3] += 2;
    }
  }
}

const uint8 *EprojInstr_SwitchJump(uint16 k, const uint8 *epjp) {  // 0x86C173
  return INSTRB_RETURN_ADDR(GET_WORD(epjp + eproj_E[k >> 1]));
}

const uint8 *EprojInstr_UserPalette0(uint16 k, const uint8 *epjp) {  // 0x86C1B4
  eproj_gfx_idx[k >> 1] = 0;
  return epjp;
}

static uint8 Eproj_CheckForBombCollisionWithRect(Rect16U rect) {  // 0x86C1B8
  if (!bomb_counter)
    return 0;
  for (int v1 = 10; v1 < 20; v1 += 2) {
    int v2 = v1 >> 1;
    if ((projectile_type[v2] & 0xF00) == 1280 && !projectile_variables[v2]) {
      uint16 x = abs16(projectile_x_pos[v2] - rect.x);
      if (x < projectile_x_radius[v2] || (uint16)(x - projectile_x_radius[v2]) < rect.w) {
        uint16 y = abs16(projectile_y_pos[v2] - rect.y);
        if (y < projectile_y_radius[v2] || (uint16)(y - projectile_y_radius[v2]) < rect.h)
          return 1;
      }
    }
  }
  return 0;
}

static uint8 Eproj_CheckForEnemyCollisionWithRect(uint16 k, Rect16U rect) {  // 0x86C209
  EnemyData *v1 = gEnemyData(k);
  return (abs16(v1->x_pos - rect.x) - v1->x_width) < rect.w &&
         (abs16(v1->y_pos - rect.y) - v1->y_height) < rect.h;
}

static uint8 Eproj_CheckForCollisionWithSamus(uint16 k) {  // 0x86C239
  Rect16U rect = Eproj_GetCollDetectRect(k);
  uint16 v1 = abs16(samus_x_pos - rect.x);
  bool v2 = v1 < samus_x_radius;
  uint16 v3 = v1 - samus_x_radius;
  uint8 result = 0;
  if (v2 || v3 < rect.w) {
    uint16 v4 = abs16(samus_y_pos - rect.y);
    v2 = v4 < samus_y_radius;
    uint16 v5 = v4 - samus_y_radius;
    if (v2 || v5 < rect.h)
      return 1;
  }
  return result;
}

void Eproj_MotherBrainsBlueRingLasers(uint16 j) {  // 0x86C2F3
  int v1 = j >> 1;
  eproj_E[v1] = 8;
  eproj_F[v1] = 0;
  eproj_gfx_idx[v1] = 1024;
  uint16 r18 = eproj_init_param_1;
  eproj_x_vel[v1] = Math_MultBySin(0x450, r18);
  eproj_y_vel[v1] = Math_MultByCos(0x450, r18);
  eproj_x_pos[v1] = enemy_data[1].x_pos + 10;
  eproj_y_pos[v1] = enemy_data[1].y_pos + 16;
  sub_86C320(j);
}

static void sub_86C320(uint16 k) {  // 0x86C320
  int v1 = k >> 1;
  eproj_x_pos[v1] = enemy_data[1].x_pos + 10;
  eproj_y_pos[v1] = enemy_data[1].y_pos + 16;
}

void Eproj_MoveToBlueRingSpawnPosition(uint16 k) {  // 0x86C335
  int16 v5;

  int v1 = k >> 1;
  if (eproj_E[v1]) {
    --eproj_E[v1];
    sub_86C320(k);
  } else {
    MoveEprojWithVelocity(k);
    uint8 t = CheckForCollisionWithShitroid_DoubleRet(k);
    if (t & 0x80)
      return;
    if (t) {
      ++enemy_ram7800[0].kraid.field_28;
      BlueRingContactExplosion(k);
      uint16 v3 = enemy_ram7800[1].kraid.kraid_healths_8ths[4];
      gExtraEnemyRam7800(enemy_ram7800[1].kraid.kraid_healths_8ths[4])->kraid.kraid_healths_8ths[0] = 16;
      EnemyData *v4 = gEnemyData(v3);
      v5 = v4->health - 80;
      if (v5 < 0)
        v5 = 0;
      v4->health = v5;
    } else if (Eproj_CheckForCollisionWithSamus(k) & 1) {
      BlueRingContactExplosion(k);
      Samus_DealDamage(SuitDamageDivision(0x50));
      samus_invincibility_timer = 96;
      samus_knockback_timer = 5;
      knockback_x_dir = (int16)(samus_x_pos - eproj_x_pos[k >> 1]) >= 0;
    } else if (CheckForBlueRingCollisionWithRoom(k) & 1) {
      Eproj_Earthqhake5(k);
    }
  }
}

uint8 CheckForCollisionWithShitroid_DoubleRet(uint16 k) {  // 0x86C3A9
  if (!enemy_ram7800[1].kraid.kraid_healths_8ths[4])
    return 0;
  if (gEnemyData(enemy_ram7800[1].kraid.kraid_healths_8ths[4])->health) {
    Rect16U rect = Eproj_GetCollDetectRect(k);
    return Eproj_CheckForEnemyCollisionWithRect(enemy_ram7800[1].kraid.kraid_healths_8ths[4], rect);
  }
  eproj_id[k >> 1] = 0;
  return 0xff;
}

uint8 CheckForBlueRingCollisionWithRoom(uint16 k) {  // 0x86C3C9
  int16 v2;
  int16 v3;

  int v1 = k >> 1;
  uint8 result = 1;
  if (!sign16(eproj_y_pos[v1] - 32) && eproj_y_pos[v1] < 0xD8) {
    v2 = eproj_x_pos[v1];
    if (v2 >= 0) {
      v3 = v2 - layer1_x_pos;
      if (v3 >= 0) {
        if (sign16(v3 - 248))
          return 0;
      }
    }
  }
  return result;
}

static Rect16U Eproj_GetCollDetectRect(uint16 k) {  // 0x86C3E9
  int v1 = k >> 1;
  Rect16U rect = {
    eproj_x_pos[v1], eproj_y_pos[v1],
    LOBYTE(eproj_radius[v1]), HIBYTE(eproj_radius[v1]),
  };
  return rect;
}

void Eproj_Earthqhake5(uint16 k) {  // 0x86C404
  earthquake_timer = 10;
  earthquake_type = 5;
  BlueRingContactExplosion(k);
}

void BlueRingContactExplosion(uint16 k) {  // 0x86C410
  int v1 = k >> 1;
  eproj_id[v1] = 0;
  eproj_spawn_pt = (Point16U){ eproj_x_pos[v1], eproj_y_pos[v1] };
  SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 3);
  QueueSfx3_Max6(0x13);
  sub_86C42E(k);
}

static void sub_86C42E(uint16 k) {  // 0x86C42E
  eproj_gfx_idx[k >> 1] = 0;
}

void EprojInit_MotherBrainBomb(uint16 j) {  // 0x86C482
  *((uint8 *)eproj_x_subpos + j) = eproj_init_param_1;
  int v1 = j >> 1;
  eproj_y_vel[v1] = 256;
  eproj_x_vel[v1] = 224;
  eproj_x_pos[v1] = enemy_data[1].x_pos + 12;
  eproj_y_pos[v1] = enemy_data[1].y_pos + 16;
  eproj_gfx_idx[v1] = 1024;
  eproj_E[v1] = 112;
  eproj_F[v1] = 0;
  ++enemy_ram7800[1].kraid.kraid_mouth_flags;
}

uint8 MotherBrainBomb_Bomb_CollDetect_DoubleRet(uint16 k) {  // 0x86C564
  int v1 = k >> 1;
  Rect16U rect = {
    eproj_x_pos[v1], eproj_y_pos[v1],
    LOBYTE(eproj_radius[v1]), HIBYTE(eproj_radius[v1]),
  };
  if (!Eproj_CheckForBombCollisionWithRect(rect))
    return 0;

  --enemy_ram7800[1].kraid.kraid_mouth_flags;
  eproj_x_vel[v1] = 0;
  eproj_y_vel[v1] = 0;
  eproj_id[v1] = 0;
  eproj_spawn_pt = (Point16U){ eproj_x_pos[v1], eproj_y_pos[v1] };
  SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 9);
  eproj_spawn_pt = (Point16U){ eproj_x_pos[v1], eproj_y_pos[v1] };
  SpawnEnemyDrops(addr_kEnemyDef_EC3F, k, 0);
  return 1;
}

uint8 MoveMotherBrainBomb(uint16 k, uint16 a) {  // 0x86C5C2
  int v2 = k >> 1;
  eproj_y_vel[v2] += a;
  MoveEprojWithVelocity(k);
  if (!sign16(eproj_x_pos[v2] - 240))
    eproj_x_vel[v2] = -eproj_x_vel[v2];
  if (sign16(eproj_y_pos[v2] - 208))
    return 0;
  eproj_y_pos[v2] = 208;
  eproj_x_vel[v2] = sign16(eproj_x_vel[v2]) ? -eproj_E[v2] : eproj_E[v2];
  eproj_y_vel[v2] = -512;
  return 1;
}

void sub_86C605(uint16 j) {  // 0x86C605
  int v1 = j >> 1;
  eproj_E[v1] = 0;
  eproj_F[v1] = 0;
  eproj_x_vel[v1] = 0;
  eproj_y_vel[v1] = 0;
  eproj_x_subpos[v1] = 0;
  eproj_y_subpos[v1] = 0;
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->mbn_var_23 = 0;
  E->mbn_var_25 = 0;
  eproj_gfx_idx[v1] = 1024;
  E->mbn_var_24 = eproj_x_pos[v1] = E->base.x_pos + 64;
  uint16 x = samus_x_pos - E->mbn_var_24;
  E->mbn_var_26 = eproj_y_pos[v1] = E->base.y_pos - 48;
  uint16 y = samus_y_pos - E->mbn_var_26;
  int r18 = (uint8)-(CalculateAngleFromXY(x, y) + 0x80);
  E->mbn_var_29 = r18;
  E->mbn_var_27 = Math_MultBySin(0xC00, r18);
  E->mbn_var_28 = Math_MultByCos(0xC00, r18);
}

void EprojInit_MotherBrainDeathBeemFired(uint16 j) {  // 0x86C684
  int v1 = j >> 1;

  Enemy_MotherBrain *E = Get_MotherBrain(0);
  eproj_x_pos[v1] = E->mbn_var_24;
  eproj_x_subpos[v1] = E->mbn_var_23;
  eproj_y_pos[v1] = E->mbn_var_26;
  eproj_y_subpos[v1] = E->mbn_var_25;
  eproj_x_vel[v1] = E->mbn_var_27;
  eproj_y_vel[v1] = E->mbn_var_28;
  MoveEprojWithVelocity(j);
  E->mbn_var_24 = eproj_x_pos[v1];
  E->mbn_var_23 = eproj_x_subpos[v1];
  E->mbn_var_26 = eproj_y_pos[v1];
  E->mbn_var_25 = eproj_y_subpos[v1];
  uint16 r18 = (uint8)(LOBYTE(E->mbn_var_29) + NextRandom());
  uint16 rv = NextRandom();
  eproj_x_vel[v1] = Math_MultBySin(rv & 0x700, r18);
  eproj_y_vel[v1] = Math_MultByCos(random_number & 0x700, r18);
  MoveEprojWithVelocity(j);
  if (sign16(eproj_y_pos[v1] - 34)
      || !sign16(eproj_y_pos[v1] - 206)
      || sign16(eproj_x_pos[v1] - 2)
      || !sign16(eproj_x_pos[v1] - 238)) {
    eproj_id[v1] = 0;
    eproj_spawn_pt = (Point16U){ eproj_x_pos[v1], eproj_y_pos[v1] };
    SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 0x1D);
    QueueSfx3_Max6(0x13);
    earthquake_timer = 10;
    earthquake_type = 5;
  } else {
    eproj_E[v1] = (eproj_E[v1] + 1) & 3;
    eproj_F[v1] = 0;
    eproj_x_vel[v1] = 0;
    eproj_y_vel[v1] = 0;
  }
}

void SpawnMotherBrainDeathBeam(uint16 x) {  // 0x86C7FB
  SpawnEprojWithRoomGfx(addr_kEproj_MotherBrainDeathBeamFired, eproj_E[x >> 1]);
}

void EprojPreInstr_MotherBrainRainbowBeam(uint16 k) {  // 0x86C814
  int v2 = k >> 1;
  eproj_x_pos[v2] = enemy_data[1].x_pos;
  eproj_y_pos[v2] = enemy_data[1].y_pos;
}

void EprojInit_MotherBrainRainbowBeam(uint16 j) {  // 0x86C80A
  int v1 = j >> 1;
  eproj_gfx_idx[v1] = 0;
  eproj_x_vel[v1] = 0;
  eproj_y_vel[v1] = 0;
  EprojPreInstr_MotherBrainRainbowBeam(j);
}

void EprojPreInstr_C84D(uint16 k) {  // 0x86C84D
  int v1 = k >> 1;
  int v2 = (uint16)(4 * eproj_E[v1]) >> 1;
  eproj_x_pos[v1] = enemy_data[1].x_pos + kEprojInit_MotherBrainsDrool[v2];
  eproj_y_pos[v1] = enemy_data[1].y_pos + kEprojInit_MotherBrainsDrool[v2 + 1];
  eproj_x_vel[v1] = 0;
  eproj_y_vel[v1] = 0;
}

void EprojInit_MotherBrainsDrool(uint16 j) {  // 0x86C843
  int v1 = j >> 1;
  eproj_gfx_idx[v1] = 0;
  eproj_E[v1] = eproj_init_param_1;
  EprojPreInstr_C84D(j);
}

void EprojInit_MotherBrainsDrool_Falling(uint16 k) {  // 0x86C886
  int v1 = k >> 1;
  eproj_y_vel[v1] += 12;
  if (MoveEprojWithVelocityY(k) >= 0xD7) {
    eproj_y_pos[v1] -= 4;
    eproj_instr_list_ptr[v1] = addr_off_86C8E1;
    eproj_instr_timers[v1] = 1;
  }
}

const uint8 *EprojInstr_Add12ToY(uint16 k, const uint8 *epjp) {  // 0x86C8D0
  eproj_y_pos[k >> 1] += 12;
  return epjp;
}

void EprojPreInit_MotherBrainsDeathExplosion_0(uint16 k) {  // 0x86C914
  int v2 = k >> 1;
  eproj_x_pos[v2] = enemy_data[0].x_pos + eproj_x_vel[v2];
  eproj_y_pos[v2] = enemy_data[0].y_pos + eproj_y_vel[v2];
}

void EprojInit_MotherBrainsDeathExplosion(uint16 j) {  // 0x86C8F5
  int v1 = j >> 1;
  eproj_instr_list_ptr[v1] = off_86C929[eproj_init_param_1];
  eproj_instr_timers[v1] = 1;
  eproj_gfx_idx[v1] = 0;
  eproj_x_vel[v1] = eproj_spawn_pt.x;
  eproj_y_vel[v1] = eproj_spawn_pt.y;
  EprojPreInit_MotherBrainsDeathExplosion_0(j);
}

void EprojInit_MotherBrainsRainbowBeamExplosion(uint16 j) {  // 0x86C92F
  int v1 = j >> 1;
  eproj_gfx_idx[v1] = 0;
  eproj_x_vel[v1] = eproj_spawn_pt.x;
  eproj_x_pos[v1] = samus_x_pos + eproj_spawn_pt.x;
  eproj_y_vel[v1] = eproj_spawn_pt.y;
  eproj_y_pos[v1] = samus_y_pos + eproj_spawn_pt.y;
}

void EprojPreInstr_MotherBrainsRainbowBeamExplosion(uint16 k) {  // 0x86C94C
  int v1 = k >> 1;
  eproj_x_pos[v1] = samus_x_pos + eproj_x_vel[v1];
  eproj_y_pos[v1] = samus_y_pos + eproj_y_vel[v1];
}

void EprojInit_MotherBrainEscapeDoorParticles(uint16 j) {  // 0x86C961
  static const int16 kEprojInit_MotherBrainEscapeDoorParticles_X[16] = {
    0, -0x20, 0, -0x18, 0, -0x10, 0, -8, 0, 0, 0, 8, 0, 0x10, 0, 0x18,
  };
  static const int16 kEprojInit_MotherBrainEscapeDoorParticles_Xvel[16] = {
    0x500, -0x200, 0x500, -0x100, 0x500, -0x100, 0x500, -0x80,
    0x500, -0x80, 0x500, 0x80, 0x500, -0x100, 0x500, 0x200,
  };

  int v1 = j >> 1;
  eproj_gfx_idx[v1] = 0;
  int v2 = (4 * eproj_init_param_1) >> 1;
  eproj_x_pos[v1] = kEprojInit_MotherBrainEscapeDoorParticles_X[v2] + 16;
  eproj_y_pos[v1] = kEprojInit_MotherBrainEscapeDoorParticles_X[v2 + 1] + 128;
  eproj_x_vel[v1] = kEprojInit_MotherBrainEscapeDoorParticles_Xvel[v2];
  eproj_y_vel[v1] = kEprojInit_MotherBrainEscapeDoorParticles_Xvel[v2 + 1];
  eproj_E[v1] = 32;
}

void EprojPreInstr_MotherBrainsExplodedDoorParticles(uint16 k) {  // 0x86C9D2
  int v1 = k >> 1;
  int16 v2 = abs16(eproj_x_vel[v1]) - 16;
  if (v2 < 0)
    v2 = 0;
  eproj_x_vel[v1] = sign16(eproj_x_vel[v1]) ? -v2 : v2;
  eproj_y_vel[v1] += 32;
  MoveEprojWithVelocity(k);
  if ((--eproj_E[v1] & 0x8000) != 0) {
    eproj_id[v1] = 0;
    eproj_y_pos[v1] -= 4;
    eproj_spawn_pt = (Point16U){ eproj_x_pos[v1], eproj_y_pos[v1] };
    SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 9);
  }
}

void EprojInit_MotherBrainPurpleBreathBig(uint16 j) {  // 0x86CA6A
  int v1 = j >> 1;
  eproj_gfx_idx[v1] = 0;
  eproj_x_pos[v1] = enemy_data[1].x_pos + 6;
  eproj_y_pos[v1] = enemy_data[1].y_pos + 16;
}

void EprojInit_MotherBrainPurpleBreathSmall(uint16 j) {  // 0x86CA83
  int v1 = j >> 1;
  eproj_gfx_idx[v1] = 0;
  eproj_x_pos[v1] = enemy_data[1].x_pos + 6;
  eproj_y_pos[v1] = enemy_data[1].y_pos + 16;
  enemy_ram7800[1].kraid.kraid_hurt_frame = 1;
}

const uint8 *EprojInstr_MotherBrainPurpleBreathIsActive(uint16 k, const uint8 *epjp) {  // 0x86CAEE
  enemy_ram7800[1].kraid.kraid_hurt_frame = 0;
  return epjp;
}

void EprojPreInstr_TimeBombSetJapaneseText(uint16 k) {  // 0x86CAFA
  int v1 = k >> 1;
  eproj_x_vel[v1] = 0;
  eproj_y_vel[v1] = 0;
  eproj_x_pos[v1] = 128;
  eproj_y_pos[v1] = 192;
}

void EprojInit_TimeBombSetJapaneseText(uint16 j) {  // 0x86CAF6
  eproj_gfx_idx[j >> 1] = 0;
  EprojPreInstr_TimeBombSetJapaneseText(j);
}

void EprojInit_MotherBrainTubeFalling(uint16 j) {  // 0x86CBC9
  int v1 = j >> 1;
  eproj_gfx_idx[v1] = 3584;
  eproj_x_vel[v1] = 0;
  eproj_y_vel[v1] = 0;
  eproj_x_pos[v1] = eproj_spawn_pt.x;
  eproj_y_pos[v1] = eproj_spawn_pt.y;
  eproj_E[v1] = FUNC16(MotherBrainTubeFallingFunc_GenerateExplosion);
}

static void CallMotherBrainTubeFallingFunc(uint32 ea, uint16 k) {
  switch (ea) {
  case fnMotherBrainTubeFallingFunc_GenerateExplosion: MotherBrainTubeFallingFunc_GenerateExplosion(k); return;
  case fnMotherBrainTubeFallingFunc_Falling: MotherBrainTubeFallingFunc_Falling(k); return;
  default: Unreachable();
  }
}

void EprojPreInstr_MotherBrainTubeFalling(uint16 k) {  // 0x86CBE7
  CallMotherBrainTubeFallingFunc(eproj_E[k >> 1] | 0x860000, k);
}

void MotherBrainTubeFallingFunc_GenerateExplosion(uint16 k) {  // 0x86CBEA
  int v1 = k >> 1;
  eproj_spawn_pt = (Point16U){ eproj_x_pos[v1], eproj_y_pos[v1] + 8 };
  SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 9);
  eproj_E[v1] = FUNC16(MotherBrainTubeFallingFunc_Falling);
  MotherBrainTubeFallingFunc_Falling(k);
}

void MotherBrainTubeFallingFunc_Falling(uint16 k) {  // 0x86CC08
  int16 v2;

  int v1 = k >> 1;
  eproj_y_vel[v1] += 6;
  v2 = MoveEprojWithVelocityY(k);
  if (!sign16(v2 - 208)) {
    eproj_id[v1] = 0;
    eproj_spawn_pt = (Point16U){ eproj_x_pos[v1], eproj_y_pos[v1] };
    SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 0xC);
  }
}

void EprojInit_MotherBrainGlassShatteringShard(uint16 j) {  // 0x86CDC5
  uint16 v1 = (2 * NextRandom()) & 0x1FE;
  int v2 = j >> 1;
  eproj_E[v2] = v1;
  int v3 = v1 >> 1;
  eproj_x_vel[v2] = kSinCosTable8bit_Sext[v3 + 64];
  eproj_y_vel[v2] = 4 * kSinCosTable8bit_Sext[v3];
  eproj_instr_list_ptr[v2] = kEprojInit_MotherBrainGlassShatteringShard_InstrPtrs[(uint16)((v1 >> 4) & 0x1E) >> 1];
  eproj_gfx_idx[v2] = 1600;
  CalculatePlmBlockCoords(plm_id);
  int v4 = eproj_init_param_1 >> 1;
  eproj_x_pos[v2] = kEprojInit_MotherBrainGlassShatteringShard_X[v4] + 16 * plm_x_block;
  eproj_y_pos[v2] = kEprojInit_MotherBrainGlassShatteringShard_Y[v4] + 16 * plm_y_block;
  eproj_x_pos[v2] += (NextRandom() & 0xF) - 8;
  eproj_y_pos[v2] += (NextRandom() & 0xF) - 8;
}

void EprojInit_MotherBrainGlassShatteringSparkle(uint16 j) {  // 0x86CE6D
  int v1 = eproj_init_param_1 >> 1;
  int v2 = j >> 1;
  eproj_x_pos[v2] = eproj_x_pos[v1] + (NextRandom() & 0x1F) - 16;
  eproj_y_pos[v2] = eproj_y_pos[v1] + (NextRandom() & 0x1F) - 16;
  eproj_gfx_idx[v2] = 1600;
}

void EprojPreInstr_MotherBrainGlassShatteringShard(uint16 k) {  // 0x86CE9B
  int v1 = k >> 1;
  int32 amt = INT16_SHL8(eproj_x_vel[v1]);
  AddToHiLo(&eproj_x_pos[v1], &eproj_x_subpos[v1], amt);
  amt = INT16_SHL8(eproj_y_vel[v1]);
  AddToHiLo(&eproj_y_pos[v1], &eproj_y_subpos[v1], amt);
  uint16 v6 = eproj_y_subpos[v1];
  if ((eproj_y_pos[v1] & 0xFF00) != 0) {
    eproj_id[v1] = 0;
  } else {
    eproj_y_vel[v1] += 32;
    if ((NextRandom() & 0x420) == 0)
      SpawnEprojWithRoomGfx(addr_kEproj_MotherBrainGlassShatteringSparkle, k);
  }
}
