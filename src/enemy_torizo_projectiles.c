// Enemy projectile handlers owned by the Torizo fights.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "torizo_config.h"

#define off_86B209 ((uint16*)RomFixedPtr(0x86b209))

static int16 TorizoEproj_RandomCenteredOffset(uint16 jitter_pixels) {
  uint16 span = jitter_pixels * 2 + 1;
  return (int16)(NextRandom() % span) - (int16)jitter_pixels;
}

static void TorizoEproj_ApplyChozoOrbSpawnJitter(uint16 eproj_idx) {
  uint16 jitter_pixels = TorizoConfig_ChozoOrbSpawnJitterPixels();
  if (jitter_pixels == 0)
    return;

  eproj_x_pos[eproj_idx] += TorizoEproj_RandomCenteredOffset(jitter_pixels);
  eproj_y_pos[eproj_idx] += TorizoEproj_RandomCenteredOffset(jitter_pixels);
}

static void TorizoEproj_ApplyChozoOrbMods(uint16 j) {
  int eproj_idx = j >> 1;
  uint16 properties = eproj_properties[eproj_idx];
  uint16 damage = TorizoConfig_ChozoOrbContactDamage(properties & 0xfff);
  eproj_properties[eproj_idx] = (properties & 0xf000) | damage;
  TorizoConfig_ApplyChozoOrbAppearance(eproj_idx);
  TorizoEproj_ApplyChozoOrbSpawnJitter(eproj_idx);
}

static void TorizoEproj_ApplyFinishExplosionJitter(uint16 eproj_idx) {
  uint16 jitter_pixels = TorizoConfig_FinishExplosionJitterPixels();
  if (jitter_pixels == 0)
    return;

  eproj_x_pos[eproj_idx] += TorizoEproj_RandomCenteredOffset(jitter_pixels);
  eproj_y_pos[eproj_idx] += TorizoEproj_RandomCenteredOffset(jitter_pixels);
  eproj_E[eproj_idx] = eproj_x_pos[eproj_idx];
  eproj_F[eproj_idx] = eproj_y_pos[eproj_idx];
}

static void TorizoEproj_InitXYVelRandom(uint16 j, uint16 init_args, Point16U origin) {  // 0x86ABAE
  const uint16 *args = (const uint16 *)RomPtr_86(init_args);
  int eproj_idx = j >> 1;
  eproj_instr_list_ptr[eproj_idx] = args[0];
  eproj_x_pos[eproj_idx] = args[1] + origin.x;
  eproj_x_vel[eproj_idx] = args[2] + (uint8)NextRandom() - 128;
  eproj_y_pos[eproj_idx] = args[3] + origin.y;
  eproj_y_vel[eproj_idx] = args[4] + (uint8)NextRandom() - 128;
}

void EprojInit_BombTorizoLowHealthExplode(uint16 j) {  // 0x86A81B
  static const int16 kBombTorizoLowHealthExplodeX[6] = { 0, 12, -12, 0, 16, -16 };
  static const int16 kBombTorizoLowHealthExplodeY[6] = { -8, -8, -8, -20, -20, -20 };

  EnemyData *E = gEnemyData(cur_enemy_index);
  int eproj_idx = j >> 1;
  eproj_x_pos[eproj_idx] = E->x_pos;
  eproj_y_pos[eproj_idx] = E->y_pos;
  if ((E->parameter_1 & 0x8000) == 0)
    eproj_init_param_1 += 2;
  eproj_init_param_1 += 2;
  int offset_idx = eproj_init_param_1 >> 1;
  eproj_x_pos[eproj_idx] += kBombTorizoLowHealthExplodeX[offset_idx];
  eproj_y_pos[eproj_idx] += kBombTorizoLowHealthExplodeY[offset_idx];
  eproj_E[eproj_idx] = eproj_x_pos[eproj_idx];
  eproj_F[eproj_idx] = eproj_y_pos[eproj_idx];
  TorizoEproj_ApplyFinishExplosionJitter(eproj_idx);
}

void EprojInit_BombTorizoDeathExplosion(uint16 j) {  // 0x86A871
  EnemyData *E = gEnemyData(cur_enemy_index);
  int eproj_idx = j >> 1;
  eproj_x_pos[eproj_idx] = E->x_pos;
  eproj_y_pos[eproj_idx] = E->y_pos;
  eproj_E[eproj_idx] = eproj_x_pos[eproj_idx];
  eproj_F[eproj_idx] = eproj_y_pos[eproj_idx];
  TorizoEproj_ApplyFinishExplosionJitter(eproj_idx);
}

void EprojInit_BombTorizosChozoOrbs(uint16 j) {  // 0x86ABEB
  EnemyData *E = gEnemyData(cur_enemy_index);
  uint16 init_args = (E->parameter_1 & 0x8000) != 0
      ? addr_kEprojInit_BombTorizosChozoOrbs_init0
      : addr_kEprojInit_BombTorizosChozoOrbs_init1;
  TorizoEproj_InitXYVelRandom(j, init_args, (Point16U) { E->x_pos, E->y_pos });
  TorizoEproj_ApplyChozoOrbMods(j);
}

void EprojInit_GoldenTorizosChozoOrbs(uint16 j) {  // 0x86AC7C
  EnemyData *E = gEnemyData(cur_enemy_index);
  uint16 init_args = (E->parameter_1 & 0x8000) != 0
      ? addr_kEprojInit_GoldenTorizosChozoOrbs_init0
      : addr_kEprojInit_GoldenTorizosChozoOrbs_init1;
  TorizoEproj_InitXYVelRandom(j, init_args, (Point16U) { E->x_pos, E->y_pos });
  TorizoEproj_ApplyChozoOrbMods(j);
}

void EprojPreInstr_BombTorizosChozoOrbs(uint16 k) {  // 0x86ACAD
  if (EprojBlockCollisition_Horiz(k)) {
    int eproj_idx = k >> 1;
    eproj_instr_list_ptr[eproj_idx] = addr_off_86AB25;
    eproj_instr_timers[eproj_idx] = 1;
    return;
  }

  uint8 carry = EprojBlockCollisition_Vertical(k);
  int eproj_idx = k >> 1;
  if ((eproj_y_vel[eproj_idx] & 0x8000) != 0 || !carry) {
    uint16 y_vel = eproj_y_vel[eproj_idx] + 18;
    eproj_y_vel[eproj_idx] = y_vel;
    if ((y_vel & 0xF000) == 4096)
      eproj_id[eproj_idx] = 0;
    return;
  }

  eproj_y_pos[eproj_idx] = (eproj_y_pos[eproj_idx] & 0xFFF0 | 8) - 2;
  eproj_instr_list_ptr[eproj_idx] = addr_off_86AB41;
  eproj_instr_timers[eproj_idx] = 1;
}

void EprojPreInstr_GoldenTorizosChozoOrbs(uint16 k) {  // 0x86ACFA
  int eproj_idx = k >> 1;
  if (EprojBlockCollisition_Horiz(k) & 1)
    eproj_x_vel[eproj_idx] = -eproj_x_vel[eproj_idx];

  if ((EprojBlockCollisition_Vertical(k) & 1) && (eproj_y_vel[eproj_idx] & 0x8000) == 0) {
    int16 x_vel = eproj_x_vel[eproj_idx];
    eproj_x_vel[eproj_idx] = x_vel >= 0 ? x_vel - 64 : x_vel + 64;
    uint16 y_vel = -(eproj_y_vel[eproj_idx] >> 1);
    eproj_y_vel[eproj_idx] = y_vel;
    if ((y_vel & 0xFF80) == 0xFF80) {
      eproj_y_pos[eproj_idx] = (eproj_y_pos[eproj_idx] & 0xFFF0 | 8) - 2;
      eproj_instr_list_ptr[eproj_idx] = addr_off_86AB41;
      eproj_instr_timers[eproj_idx] = 1;
      return;
    }
  }

  eproj_y_vel[eproj_idx] += 24;
}

const uint8 *EprojInstr_GotoDependingOnXDirection(uint16 k, const uint8 *epjp) {  // 0x86AD92
  int eproj_idx = k >> 1;
  AddToHiLo(&eproj_x_pos[eproj_idx], &eproj_x_subpos[eproj_idx], INT16_SHL8(eproj_x_vel[eproj_idx]));
  return INSTRB_RETURN_ADDR(GET_WORD(epjp + ((eproj_x_vel[eproj_idx] & 0x8000) == 0 ? 2 : 0)));
}

void EprojInit_TorizoSonicBoom(uint16 j) {  // 0x86AE15
  int y_offset = (NextRandom() & 1) != 0 ? -12 : 20;
  EnemyData *E = gEnemyData(cur_enemy_index);
  int eproj_idx = j >> 1;
  eproj_y_pos[eproj_idx] = E->y_pos + y_offset;
  eproj_y_vel[eproj_idx] = 0;
  if ((E->parameter_1 & 0x8000) != 0) {
    eproj_x_pos[eproj_idx] = E->x_pos + 32;
    eproj_x_vel[eproj_idx] = 624;
    eproj_instr_list_ptr[eproj_idx] = addr_off_86ADD2;
  } else {
    eproj_x_pos[eproj_idx] = E->x_pos - 32;
    eproj_x_vel[eproj_idx] = -624;
    eproj_instr_list_ptr[eproj_idx] = addr_off_86ADBF;
  }
}

void EprojPreInstr_TorizoSonicBoom(uint16 k) {  // 0x86AE6C
  int eproj_idx = k >> 1;
  if (EprojBlockCollisition_Horiz(k) & 1) {
    eproj_instr_list_ptr[eproj_idx] = addr_off_86ADE5;
    eproj_instr_timers[eproj_idx] = 1;
    eproj_E[eproj_idx] = eproj_x_pos[eproj_idx];
    eproj_F[eproj_idx] = eproj_y_pos[eproj_idx];
    return;
  }

  eproj_x_vel[eproj_idx] += sign16(eproj_x_vel[eproj_idx]) ? -16 : 16;
  if ((eproj_x_vel[eproj_idx] & 0xF000) == 4096)
    eproj_id[eproj_idx] = 0;
}

void EprojInit_TorizoLandingDustCloudLeftFoot(uint16 j) {  // 0x86AFCD
  EnemyData *E = gEnemyData(cur_enemy_index);
  int eproj_idx = j >> 1;
  eproj_y_pos[eproj_idx] = E->y_pos + 48;
  eproj_x_pos[eproj_idx] = E->x_pos - 24;
}

void EprojInit_GoldenTorizoEgg(uint16 j) {  // 0x86B001
  EnemyData *E = gEnemyData(cur_enemy_index);
  int eproj_idx = j >> 1;
  eproj_F[eproj_idx] = (0xe2 & 0x1F) + 64;  // Original bug.
  eproj_E[eproj_idx] = E->parameter_1;
  uint16 init_args = sign16(E->parameter_1)
      ? addr_kEprojInit_GoldenTorizoEgg0
      : addr_kEprojInit_GoldenTorizoEgg1;
  TorizoEproj_InitXYVelRandom(j, init_args, (Point16U) { E->x_pos, E->y_pos });
}

void EprojPreInstr_GoldenTorizoEgg(uint16 k) {  // 0x86B043
  int eproj_idx = k >> 1;
  if ((--eproj_F[eproj_idx] & 0x8000) != 0) {
    eproj_instr_list_ptr[eproj_idx] += 2;
    eproj_instr_timers[eproj_idx] = 1;
    eproj_x_vel[eproj_idx] = (eproj_E[eproj_idx] & 0x8000) != 0 ? 256 : -256;
    return;
  }

  if (EprojBlockCollisition_Horiz(k) & 1) {
    eproj_x_vel[eproj_idx] = -eproj_x_vel[eproj_idx];
    eproj_E[eproj_idx] ^= 0x8000;
  }
  if ((EprojBlockCollisition_Vertical(k) & 1) && (eproj_y_vel[eproj_idx] & 0x8000) == 0) {
    eproj_x_vel[eproj_idx] += sign16(eproj_x_vel[eproj_idx]) ? 32 : -32;
    eproj_y_vel[eproj_idx] = -eproj_y_vel[eproj_idx];
  }
  eproj_y_vel[eproj_idx] += 48;
  if ((eproj_y_vel[eproj_idx] & 0xF000) == 4096)
    eproj_id[eproj_idx] = 0;
}

void sub_86B0B9(uint16 k) {  // 0x86B0B9
  int eproj_idx = k >> 1;
  if (EprojBlockCollisition_Horiz(k) & 1) {
    eproj_pre_instr[eproj_idx] = 0xB0DD;
    eproj_y_vel[eproj_idx] = 0;
    return;
  }

  eproj_x_vel[eproj_idx] += (eproj_E[eproj_idx] & 0x8000) != 0 ? 48 : -48;
}

void sub_86B0DD(uint16 k) {  // 0x86B0DD
  int eproj_idx = k >> 1;
  if (EprojBlockCollisition_Vertical(k) & 1) {
    eproj_instr_list_ptr[eproj_idx] = (eproj_E[eproj_idx] & 0x8000) != 0
        ? addr_off_86B1A8
        : addr_off_86B190;
    eproj_instr_timers[eproj_idx] = 1;
  } else {
    eproj_y_vel[eproj_idx] += 48;
  }
}

const uint8 *sub_86B13E(uint16 k, const uint8 *epjp) {  // 0x86B13E
  return INSTRB_RETURN_ADDR((eproj_E[k >> 1] & 0x8000) != 0 ? addr_off_86B166 : addr_off_86B14B);
}

void EprojInit_GoldenTorizoSuperMissile(uint16 j) {  // 0x86B1CE
  static const int16 kGoldenTorizoSuperMissileXOffset[2] = { -0x1e, 0x1e };
  int eproj_idx = j >> 1;
  eproj_E[eproj_idx] = cur_enemy_index;
  EnemyData *E = gEnemyData(cur_enemy_index);
  int facing_idx = (E->parameter_1 & 0x8000) != 0 ? 1 : 0;
  eproj_x_pos[eproj_idx] = E->x_pos + kGoldenTorizoSuperMissileXOffset[facing_idx];
  eproj_y_pos[eproj_idx] = E->y_pos - 52;
  eproj_instr_list_ptr[eproj_idx] = off_86B209[facing_idx];
}

void EprojPreInstr_GoldenTorizoSuperMissile(uint16 k) {  // 0x86B20D
  int eproj_idx = k >> 1;
  EnemyData *E = gEnemyData(eproj_E[eproj_idx]);
  eproj_x_pos[eproj_idx] = E->x_pos + ((E->parameter_1 & 0x8000) != 0 ? 32 : -32);
  eproj_y_pos[eproj_idx] = E->y_pos - 52;
}

void EprojPreInstr_B237(uint16 k) {  // 0x86B237
  if (EprojBlockCollisition_Horiz(k)) {
    int eproj_idx = k >> 1;
    eproj_instr_list_ptr[eproj_idx] = addr_off_86B2EF;
    eproj_instr_timers[eproj_idx] = 1;
    return;
  }

  uint8 carry = EprojBlockCollisition_Vertical(k);
  int eproj_idx = k >> 1;
  if ((eproj_y_vel[eproj_idx] & 0x8000) == 0 && carry) {
    eproj_instr_list_ptr[eproj_idx] = addr_off_86B2EF;
    eproj_instr_timers[eproj_idx] = 1;
    return;
  }

  uint16 y_vel = eproj_y_vel[eproj_idx] + 16;
  eproj_y_vel[eproj_idx] = y_vel;
  if ((y_vel & 0xF000) == 4096)
    eproj_id[eproj_idx] = 0;
}

static void TorizoEproj_SetVelFromAngle(uint16 k, uint16 angle) {  // 0x86B279
  int eproj_idx = k >> 1;
  eproj_x_vel[eproj_idx] = 4 * kSinCosTable8bit_Sext[angle + 64];
  eproj_y_vel[eproj_idx] = 4 * kSinCosTable8bit_Sext[angle];
}

const uint8 *EprojInstr_SetVelTowardsSamus1(uint16 k, const uint8 *epjp) {  // 0x86B269
  TorizoEproj_SetVelFromAngle(k, CalculateAngleOfSamusFromEproj(k) & 0x7F);
  return epjp;
}

const uint8 *EprojInstr_SetVelTowardsSamus2(uint16 k, const uint8 *epjp) {  // 0x86B272
  TorizoEproj_SetVelFromAngle(k, CalculateAngleOfSamusFromEproj(k) | 0x80);
  return epjp;
}

void EprojInit_GoldenTorizoEyeBeam(uint16 j) {  // 0x86B328
  EnemyData *E = gEnemyData(cur_enemy_index);
  uint16 init_args = (E->parameter_1 & 0x8000) != 0 ? addr_stru_86B376 : addr_stru_86B380;
  TorizoEproj_InitXYVelRandom(j, init_args, (Point16U) { E->x_pos, E->y_pos });
  uint16 angle = (NextRandom() & 0x1E) - 16 + 192;
  if ((E->parameter_1 & 0x8000) == 0)
    angle += 128;
  int sincos_idx = angle >> 1;
  int eproj_idx = j >> 1;
  eproj_x_vel[eproj_idx] = 8 * kSinCosTable8bit_Sext[sincos_idx + 64];
  eproj_y_vel[eproj_idx] = 8 * kSinCosTable8bit_Sext[sincos_idx];
}

void EprojPreInstr_GoldenTorizoEyeBeam(uint16 k) {  // 0x86B38A
  int eproj_idx = k >> 1;
  if (EprojBlockCollisition_Horiz(k) & 1) {
    eproj_instr_list_ptr[eproj_idx] = addr_off_86B3CD;
    eproj_instr_timers[eproj_idx] = 1;
    return;
  }
  if (EprojBlockCollisition_Vertical(k) & 1) {
    eproj_y_pos[eproj_idx] = (eproj_y_pos[eproj_idx] & 0xFFF0 | 8) - 2;
    eproj_instr_list_ptr[eproj_idx] = addr_off_86B3E5;
    eproj_instr_timers[eproj_idx] = 1;
  }
}
