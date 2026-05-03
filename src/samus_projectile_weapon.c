// Samus projectile weapon setup: beam palette loading, cooldown gates,
// and beam fire/origin initialization.

#include "samus_projectile.h"

#include "funcs.h"
#include "ida_types.h"
#include "sm_rtl.h"
#include "variables.h"
#include "variables_extra.h"

#define kBeamTilePtrs ((uint16*)RomFixedPtr(0x90c3b1))
#define kBeamPalettePtrs ((uint16*)RomFixedPtr(0x90c3c9))

enum {
  kBeamEquipmentMask = 0x0fff,
  kBeamCombinationMask = 0x003f,
  kBeamVariantMask = 0x000f,
  kBeamChargedFlag = 0x0010,
  kHyperBeamProjectileType = 0x9018,
  kProjectileLimit_Normal = 5,
  kProjectileLimit_SuperMissile = 4,
};

static const uint16 kUnchargedProjectile_Sfx[12] = {  // 0x90C3E1
  0xb, 0xd, 0xc, 0xe, 0xf, 0x12, 0x10, 0x11, 0x13, 0x16, 0x14, 0x15,
};

static const uint16 kChargedProjectile_Sfx[12] = {  // 0x90C3F9
  0x17, 0x19, 0x18, 0x1a, 0x1b, 0x1e, 0x1c, 0x1d, 0x1f, 0x22, 0x20, 0x21,
};

static const uint8 kProjectileCooldown_Uncharged[38] = {  // 0x90C411
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 12, 15, 0, 0, 0, 0,
  30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 0, 0, 0, 0,
   0,  0,  0,  0,  0,  0,
};

static const uint8 kBeamAutoFireCooldowns[12] = {  // 0x90C43B
  0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19,
};

static const uint16 kProjectileBombPreInstr[12] = {  // 0x90B887
  (uint16)fnProjPreInstr_Beam_NoWaveBeam,
  (uint16)fnProjPreInstr_BeamOrIceWave,
  (uint16)fnProjPreInstr_Beam_NoWaveBeam,
  (uint16)fnProjPreInstr_BeamOrIceWave,
  (uint16)fnProjPreInstr_Beam_NoWaveBeam,
  (uint16)fnProjPreInstr_WavePlasmaEtc,
  (uint16)fnProjPreInstr_Beam_NoWaveBeam,
  (uint16)fnProjPreInstr_WavePlasmaEtc,
  (uint16)fnProjPreInstr_Beam_NoWaveBeam,
  (uint16)fnProjPreInstr_WavePlasmaEtc,
  (uint16)fnProjPreInstr_Beam_NoWaveBeam,
  (uint16)fnProjPreInstr_WavePlasmaEtc,
};

static const uint16 kFireChargedBeam_Funcs[12] = {  // 0x90B986
  (uint16)fnProjPreInstr_Beam_NoWaveBeam,
  (uint16)fnProjPreInstr_WavePlasmaEtc,
  (uint16)fnProjPreInstr_Beam_NoWaveBeam,
  (uint16)fnProjPreInstr_WavePlasmaEtc,
  (uint16)fnProjPreInstr_Beam_NoWaveBeam,
  (uint16)fnProjPreInstr_WavePlasmaEtc,
  (uint16)fnProjPreInstr_Beam_NoWaveBeam,
  (uint16)fnProjPreInstr_WavePlasmaEtc,
  (uint16)fnProjPreInstr_Beam_NoWaveBeam,
  (uint16)fnProjPreInstr_WavePlasmaEtc,
  (uint16)fnProjPreInstr_Beam_NoWaveBeam,
  (uint16)fnProjPreInstr_WavePlasmaEtc,
};

static const int16 kProjectileOriginOffsets3_X[10] = { 2, 13, 11, 13, 2, -5, -14, -11, -19, -2 };
static const int16 kProjectileOriginOffsets3_Y[10] = { -8, -13, 1, 4, 13, 13, 4, 1, -19, -8 };
static const int16 kProjectileOriginOffsets4_X[10] = { 2, 15, 15, 13, 2, -5, -13, -13, -15, -2 };
static const int16 kProjectileOriginOffsets4_Y[10] = { -8, -16, -2, 1, 13, 13, 1, -2, -16, -8 };

static uint16 SamusProjectile_GetUnchargedBeamPreInstr(uint16 type) {
  return kProjectileBombPreInstr[type & kBeamVariantMask];
}

static uint16 SamusProjectile_GetChargedBeamPreInstr(uint16 type) {
  return kFireChargedBeam_Funcs[type & kBeamVariantMask];
}

uint8 SamusProjectile_GetUnchargedBeamCooldown(uint16 type) {
  return kProjectileCooldown_Uncharged[type & kBeamCombinationMask];
}

static uint8 SamusProjectile_GetBeamAutoFireCooldown(uint16 type) {
  return kBeamAutoFireCooldowns[type & kBeamCombinationMask];
}

static uint16 SamusProjectile_GetSuperMissileLimit(void) {
  return hud_item_index == 2 ? kProjectileLimit_SuperMissile : kProjectileLimit_Normal;
}

static bool SamusProjectile_RunBeamCollision(uint16 *projectile_slot, bool is_wave_beam) {
  uint16 slot = *projectile_slot;
  int slot_index = slot >> 1;
  projectile_bomb_x_speed[slot_index] = 0;
  projectile_bomb_y_speed[slot_index] = 0;
  projectile_index = slot;

  if (is_wave_beam) {
    WaveBeam_CheckColl(slot);
    return true;
  }

  CheckBeamCollByDir(slot);
  *projectile_slot = projectile_index;
  return (projectile_type[projectile_index >> 1] & kProjectileType_TypeMask) == 0;
}

static void SamusProjectile_FinishUnchargedBeam(uint16 projectile_slot) {
  projectile_bomb_pre_instructions[projectile_slot >> 1] =
      SamusProjectile_GetUnchargedBeamPreInstr(projectile_type[projectile_slot >> 1]);
  SetInitialProjectileSpeed(projectile_slot);
}

static void SamusProjectile_FinishChargedBeam(uint16 projectile_slot) {
  projectile_bomb_pre_instructions[projectile_slot >> 1] =
      SamusProjectile_GetChargedBeamPreInstr(projectile_type[projectile_slot >> 1]);
  SetInitialProjectileSpeed(projectile_slot);
  charged_shot_glow_timer = 4;
}

void Samus_HandleCooldown(void) {  // 0x90AC1C
  if (time_is_frozen_flag) {
    cooldown_timer = 32;
  } else if (cooldown_timer) {
    if ((cooldown_timer & 0x8000) != 0 || (--cooldown_timer, (cooldown_timer & 0x8000) != 0))
      cooldown_timer = 0;
  }
}

uint8 Samus_CanFireBeam(void) {  // 0x90AC39
  if (!sign16(projectile_counter - 5) || (uint8)cooldown_timer)
    return 0;
  cooldown_timer = 1;
  ++projectile_counter;
  return 1;
}

uint8 Samus_CanFireSuperMissile(void) {  // 0x90AC5A
  if (!sign16(projectile_counter - SamusProjectile_GetSuperMissileLimit()))
    return 0;
  if ((uint8)cooldown_timer)
    return 0;
  cooldown_timer = 1;
  ++projectile_counter;
  return 1;
}

void UpdateBeamTilesAndPalette(void) {  // 0x90AC8D
  uint16 v0 = 2 * (equipped_beams & kBeamEquipmentMask);
  uint16 v1 = vram_write_queue_tail;
  gVramWriteEntry(vram_write_queue_tail)->size = 256;
  v1 += 2;
  gVramWriteEntry(v1)->size = kBeamTilePtrs[v0 >> 1];
  v1 += 2;
  LOBYTE(gVramWriteEntry(v1++)->size) = -102;
  gVramWriteEntry(v1)->size = addr_unk_606300;
  vram_write_queue_tail = v1 + 2;
  WriteBeamPalette_Y(v0);
}

void WriteBeamPalette_A(uint16 a) {  // 0x90ACC2
  WriteBeamPalette_Y(2 * (a & kBeamEquipmentMask));
}

void WriteBeamPalette_Y(uint16 j) {  // 0x90ACCD
  uint16 r0 = kBeamPalettePtrs[j >> 1];
  uint16 v1 = 0;
  uint16 v2 = 0;
  do {
    palette_buffer[(v2 >> 1) + 224] = GET_WORD(RomPtr_90(r0 + v1));
    v2 += 2;
    v1 += 2;
  } while ((int16)(v1 - 32) < 0);
}

void LoadProjectilePalette(uint16 a) {  // 0x90ACFC
  uint16 r0 = kBeamPalettePtrs[a & kBeamEquipmentMask];
  uint16 v1 = 0;
  uint16 v2 = 0;
  do {
    palette_buffer[(v2 >> 1) + 224] = GET_WORD(RomPtr_90(r0 + v1));
    v2 += 2;
    v1 += 2;
  } while ((int16)(v1 - 32) < 0);
}

void FireUnchargedBeam(void) {  // 0x90B8D6
  int8 v3;

  if (hyper_beam_flag) {
    FireHyperBeam();
    return;
  }
  if (Samus_CanFireBeam() & 1) {
    uint16 v0 = 0;
    while (projectile_damage[v0 >> 1]) {
      v0 += 2;
      if ((int16)(v0 - 10) >= 0) {
        v0 -= 2;
        break;
      }
    }
    if (!(InitProjectilePositionDirection(v0) & 1)) {
      projectile_invincibility_timer = 10;
      uint16 v1 = v0;
      int v2 = v0 >> 1;
      projectile_timers[v2] = 4;
      v3 = equipped_beams;
      projectile_type[v2] = equipped_beams | kProjectileType_DontInteractWithSamus;
      QueueSfx1_Max15(kUnchargedProjectile_Sfx[v3 & kBeamVariantMask]);
      play_resume_charging_beam_sfx = 0;
      InitializeProjectile(v1);
      bool manual_fire =
          (equipped_beams & 0x1000) != 0 ||
          (button_config_shoot_x & joypad1_newkeys) != 0 ||
          (button_config_shoot_x & joypad1_newinput_samusfilter) != 0;
      uint16 beam_type = projectile_type[v2];
      cooldown_timer = manual_fire
          ? SamusProjectile_GetUnchargedBeamCooldown(beam_type)
          : SamusProjectile_GetBeamAutoFireCooldown(beam_type);
      if (!SamusProjectile_RunBeamCollision(&v1, (beam_type & 1) != 0))
        return;
      SamusProjectile_FinishUnchargedBeam(v1);
      return;
    }
  }
  if (!sign16(prev_beam_charge_counter - 16)) {
    play_resume_charging_beam_sfx = 0;
    QueueSfx1_Max15(2);
  }
}

void FireChargedBeam(void) {  // 0x90B99E
  if (Samus_CanFireBeam() & 1) {
    uint16 v0 = 0;
    while (projectile_damage[v0 >> 1]) {
      v0 += 2;
      if ((int16)(v0 - 10) >= 0) {
        v0 -= 2;
        break;
      }
    }
    uint16 r20 = v0;
    if (!(InitProjectilePositionDirection(r20) & 1)) {
      projectile_invincibility_timer = 10;
      uint16 v1 = r20;
      int v2 = r20 >> 1;
      projectile_timers[v2] = 4;
      uint16 v3 = equipped_beams & 0x100F | kProjectileType_DontInteractWithSamus | kBeamChargedFlag;
      projectile_type[v2] = v3;
      QueueSfx1_Max15(kChargedProjectile_Sfx[v3 & kBeamVariantMask]);
      play_resume_charging_beam_sfx = 0;
      InitializeProjectile(v1);
      uint16 v5 = projectile_type[v2];
      cooldown_timer = SamusProjectile_GetUnchargedBeamCooldown(v5);
      if (!SamusProjectile_RunBeamCollision(&v1, (v5 & 1) != 0)) {
        charged_shot_glow_timer = 4;
        return;
      }
      SamusProjectile_FinishChargedBeam(v1);
      return;
    }
  }
  if (!sign16(prev_beam_charge_counter - 16)) {
    play_resume_charging_beam_sfx = 0;
    QueueSfx1_Max15(2);
  }
}

uint8 InitProjectilePositionDirection(uint16 r20) {  // 0x90BA56
  uint16 v0 = samus_pose;
  uint16 direction_shots_fired;

  if (new_projectile_direction_changed_pose) {
    direction_shots_fired = (uint8)new_projectile_direction_changed_pose;
    new_projectile_direction_changed_pose = 0;
  } else {
    direction_shots_fired = kPoseParams[v0].direction_shots_fired;
    if ((direction_shots_fired & 0xF0) != 0) {
      if (direction_shots_fired != 16
          || (LOBYTE(direction_shots_fired) = kPoseParams[samus_last_different_pose].direction_shots_fired,
              (direction_shots_fired & 0xF0) != 0)) {
        --projectile_counter;
        return 1;
      }
      direction_shots_fired = (uint8)direction_shots_fired;
      v0 = samus_pose;
    }
  }
  int v2 = r20 >> 1;
  projectile_dir[v2] = direction_shots_fired;
  uint16 r22 = kPoseParams[v0].y_offset_to_gfx;
  uint16 v3 = 2 * (projectile_dir[v2] & 0xF);
  if (samus_pose == kPose_75_FaceL_Moonwalk_AimUL
      || samus_pose == kPose_76_FaceR_Moonwalk_AimUR
      || samus_movement_type == 1) {
    int v6 = v3 >> 1;
    projectile_x_pos[v2] = samus_x_pos + kProjectileOriginOffsets4_X[v6];
    projectile_y_pos[v2] = samus_y_pos + kProjectileOriginOffsets4_Y[v6] - r22;
    return 0;
  } else {
    int v4 = v3 >> 1;
    projectile_x_pos[v2] = samus_x_pos + kProjectileOriginOffsets3_X[v4];
    projectile_y_pos[v2] = samus_y_pos + kProjectileOriginOffsets3_Y[v4] - r22;
    return 0;
  }
}

void FireHyperBeam(void) {  // 0x90BCD1
  if (Samus_CanFireBeam() & 1) {
    uint16 v0 = 0;
    while (projectile_damage[v0 >> 1]) {
      v0 += 2;
      if ((int16)(v0 - 10) >= 0) {
        v0 -= 2;
        break;
      }
    }
    uint16 r20 = v0;
    if (!(InitProjectilePositionDirection(r20) & 1)) {
      projectile_invincibility_timer = 10;
      uint16 k = r20;
      projectile_type[r20 >> 1] = kHyperBeamProjectileType;
      QueueSfx1_Max15(kChargedProjectile_Sfx[8]);
      play_resume_charging_beam_sfx = 0;
      InitializeProjectile(k);
      int v1 = k >> 1;
      projectile_bomb_x_speed[v1] = 0;
      projectile_bomb_y_speed[v1] = 0;
      projectile_index = k;
      WaveBeam_CheckColl(k);
      uint16 v2 = projectile_index;
      int v3 = projectile_index >> 1;
      projectile_damage[v3] = 1000;
      projectile_bomb_pre_instructions[v3] = FUNC16(ProjPreInstr_HyperBeam);
      r20 = v2;
      SetInitialProjectileSpeed(r20);
      cooldown_timer = 21;
      charged_shot_glow_timer = -32748;
      flare_animation_frame = 29;
      flare_slow_sparks_anim_frame = 5;
      flare_fast_sparks_anim_frame = 5;
      flare_animation_timer = 3;
      flare_slow_sparks_anim_timer = 3;
      flare_fast_sparks_anim_timer = 3;
      flare_counter = 0x8000;
    }
  }
}

void ProjectileReflection(uint16 r20) {  // 0x90BE00
  uint16 v0 = r20;
  int v1 = r20 >> 1;
  uint16 v2 = projectile_type[v1];
  if ((v2 & kProjectileType_Missile) != 0) {
    InitializeProjectile(r20);
    projectile_bomb_pre_instructions[v1] = FUNC16(ProjPreInstr_Missile);
    projectile_variables[v1] = 240;
  } else if ((v2 & kProjectileType_SuperMissile) != 0) {
    uint16 k = r20;
    ClearProjectile(LOBYTE(projectile_variables[v1]));
    InitializeProjectile(k);
    int v3 = k >> 1;
    projectile_bomb_pre_instructions[v3] = FUNC16(ProjPreInstr_SuperMissile);
    projectile_variables[v3] = 240;
  } else {
    SetInitialProjectileSpeed(r20);
    InitializeProjectile(v0);
    projectile_bomb_pre_instructions[v1] = SamusProjectile_GetChargedBeamPreInstr(projectile_type[v1]);
  }
}
