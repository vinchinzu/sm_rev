// Samus projectile runtime: projectile state reset/cleanup, pre-instruction
// dispatch, movement/collision stepping, speed initialization, and projectile
// trail rendering. Extracted from sm_90.c.

#include "ida_types.h"
#include "variables.h"
#include "variables_extra.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "samus_projectile.h"

#define kBeamTilePtrs ((uint16*)RomFixedPtr(0x90c3b1))
#define kBeamPalettePtrs ((uint16*)RomFixedPtr(0x90c3c9))
#define off_90B5BB ((uint16*)RomFixedPtr(0x90b5bb))
#define off_90B609 ((uint16*)RomFixedPtr(0x90b609))
#define kFlareAnimDelays ((uint16*)RomFixedPtr(0x90c481))

// ROM pointers for ProjectileTrail_Func5 (from sm_9b.c)
#define g_off_9BA4B3 ((uint16*)RomFixedPtr(0x9ba4b3))
#define g_off_9BA4CB ((uint16*)RomFixedPtr(0x9ba4cb))
#define g_off_9BA4E3 ((uint16*)RomFixedPtr(0x9ba4e3))

#define kProjectileData_UnchargedBeams ((uint16*)RomFixedPtr(0x9383c1))
#define kProjectileData_ChargedBeams ((uint16*)RomFixedPtr(0x9383d9))
#define kProjectileData_NonBeams ((uint16*)RomFixedPtr(0x9383f1))
#define kShinesparkEchoSpazer_ProjectileData ((uint16*)RomFixedPtr(0x938403))
#define kRunInstrForSuperMissile ((uint16*)RomFixedPtr(0x93842b))
#define g_stru_938691 (*(ProjectileDamagesAndInstrPtr*)RomFixedPtr(0x938691))
#define g_stru_938679 (*(ProjectileDamagesAndInstrPtr*)RomFixedPtr(0x938679))
#define kProjInstrList_Explosion (*(ProjectileDamagesAndInstrPtr*)RomFixedPtr(0x938681))
#define g_off_938413 ((uint16*)RomFixedPtr(0x938413))

static const uint16 kUnchargedProjectile_Sfx[12] = { 0xb, 0xd, 0xc, 0xe, 0xf, 0x12, 0x10, 0x11, 0x13, 0x16, 0x14, 0x15 };
static const uint16 kChargedProjectile_Sfx[12] = { 0x17, 0x19, 0x18, 0x1a, 0x1b, 0x1e, 0x1c, 0x1d, 0x1f, 0x22, 0x20, 0x21 };
static const uint16 kNonBeamProjectile_Sfx[9] = { 0, 3, 4, 0, 0, 0, 0, 0, 0 };
static const uint8 kProjectileCooldown_Uncharged[38] = {
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 12, 15, 0, 0, 0, 0,
  30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 0, 0, 0, 0,
   0,  0,  0,  0,  0,  0,
};
static const uint8 kNonBeamProjectileCooldowns[9] = { 0, 0xa, 0x14, 0x28, 0, 0x10, 0, 0, 0 };
static const uint8 kBeamAutoFireCooldowns[12] = { 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19 };

#ifdef SAMUS_PROJECTILE_BEAM_IMPL

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
  if (hud_item_index != 2) {
    if (!sign16(projectile_counter - 5))
      return 0;
LABEL_3:
    if (!(uint8)cooldown_timer) {
      cooldown_timer = 1;
      ++projectile_counter;
      return 1;
    }
    return 0;
  }
  if (sign16(projectile_counter - 4))
    goto LABEL_3;
  return 0;
}

void UpdateBeamTilesAndPalette(void) {  // 0x90AC8D
  uint16 v0 = 2 * (equipped_beams & 0xFFF);
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
  WriteBeamPalette_Y(2 * (a & 0xFFF));
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
  uint16 r0 = kBeamPalettePtrs[a & 0xFFF];
  uint16 v1 = 0;
  uint16 v2 = 0;
  do {
    palette_buffer[(v2 >> 1) + 224] = GET_WORD(RomPtr_90(r0 + v1));
    v2 += 2;
    v1 += 2;
  } while ((int16)(v1 - 32) < 0);
}

#endif

#ifdef SAMUS_PROJECTILE_CORE_IMPL

void ResetProjectileData(void) {  // 0x90AD22
  uint16 v0 = 0;
  do {
    int v1 = v0 >> 1;
    projectile_timers[v1] = 0;
    projectile_x_pos[v1] = 0;
    projectile_y_pos[v1] = 0;
    projectile_dir[v1] = 0;
    projectile_bomb_x_speed[v1] = 0;
    projectile_bomb_y_speed[v1] = 0;
    projectile_x_radius[v1] = 0;
    projectile_y_radius[v1] = 0;
    projectile_type[v1] = 0;
    projectile_damage[v1] = 0;
    projectile_bomb_instruction_ptr[v1] = 0;
    projectile_bomb_instruction_timers[v1] = 0;
    projectile_variables[v1] = 0;
    projectile_spritemap_pointers[v1] = 0;
    projectile_bomb_pre_instructions[v1] = FUNC16(ProjPreInstr_Empty);
    v0 += 2;
  } while ((int16)(v0 - 20) < 0);
  bomb_counter = 0;
  cooldown_timer = 0;
  projectile_counter = 0;
  power_bomb_flag = 0;
  if (hud_auto_cancel_flag) {
    hud_item_index = 0;
    samus_auto_cancel_hud_item_index = 0;
  }
  speed_echo_xpos[0] = 0;
  speed_echo_xpos[1] = 0;
  speed_echo_xpos[2] = 0;
  speed_echo_xpos[3] = 0;
  speed_echo_ypos[0] = 0;
  speed_echo_ypos[1] = 0;
  speed_echo_ypos[2] = 0;
  speed_echo_ypos[3] = 0;
  speed_echo_xspeed[0] = 0;
  speed_echo_xspeed[1] = 0;
  speed_echo_xspeed[2] = 0;
  speed_echo_xspeed[3] = 0;
  speed_echoes_index = 0;
  if (samus_special_super_palette_flags) {
    samus_special_super_palette_flags = 0;
    Samus_LoadSuitTargetPalette();
  }
  if (hyper_beam_flag)
    SpawnPalfxObject(addr_stru_8DE1F0);
}

void ClearProjectile(uint16 k) {  // 0x90ADB7
  int v1 = k >> 1;
  projectile_x_pos[v1] = 0;
  projectile_y_pos[v1] = 0;
  projectile_bomb_x_subpos[v1] = 0;
  projectile_bomb_y_subpos[v1] = 0;
  projectile_dir[v1] = 0;
  projectile_bomb_x_speed[v1] = 0;
  projectile_bomb_y_speed[v1] = 0;
  projectile_x_radius[v1] = 0;
  projectile_y_radius[v1] = 0;
  projectile_type[v1] = 0;
  projectile_damage[v1] = 0;
  projectile_bomb_instruction_ptr[v1] = 0;
  projectile_bomb_instruction_timers[v1] = 0;
  projectile_variables[v1] = 0;
  projectile_spritemap_pointers[v1] = 0;
  projectile_bomb_pre_instructions[v1] = FUNC16(ProjPreInstr_Empty);
  if ((int16)(k - 10) >= 0) {
    if ((--bomb_counter & 0x8000) != 0)
      bomb_counter = 0;
  } else if ((--projectile_counter & 0x8000) != 0) {
    projectile_counter = 0;
  }
}

static Func_Y_V *const kKillProjectileFuncs[10] = {  // 0x90AE06
  KillProjectileFunc_0,
  KillProjectileFunc_1,
  KillProjectileFunc_2,
  KillProjectileFunc_3,
  KillProjectileFunc_4,
  KillProjectileFunc_4,
  KillProjectileFunc_6,
  KillProjectileFunc_7,
  KillProjectileFunc_8,
  KillProjectileFunc_0,
};

void KillProjectile(uint16 k) {
  int v1 = k >> 1;
  if ((projectile_type[v1] & 0xF00) != 0) {
    if (!sign16((HIBYTE(projectile_type[v1]) & 0xF) - 3)) {
      ClearProjectile(k);
      return;
    }
  } else {
    kKillProjectileFuncs[projectile_dir[v1] & 0xF](k);
  }
  KillProjectileInner(k);
  projectile_bomb_pre_instructions[k >> 1] = FUNC16(ProjPreInstr_Empty);
}

void KillProjectileFunc_0(uint16 j) {  // 0x90AE4E
  projectile_y_pos[j >> 1] -= projectile_y_radius[j >> 1];
}

void KillProjectileFunc_1(uint16 j) {  // 0x90AE59
  int v1 = j >> 1;
  projectile_x_pos[v1] += projectile_x_radius[v1];
  projectile_y_pos[v1] -= projectile_y_radius[v1];
}

void KillProjectileFunc_2(uint16 j) {  // 0x90AE6E
  projectile_x_pos[j >> 1] += projectile_x_radius[j >> 1];
}

void KillProjectileFunc_3(uint16 j) {  // 0x90AE79
  int v1 = j >> 1;
  projectile_x_pos[v1] += projectile_x_radius[v1];
  projectile_y_pos[v1] += projectile_y_radius[v1];
}

void KillProjectileFunc_4(uint16 j) {  // 0x90AE8E
  projectile_y_pos[j >> 1] += projectile_y_radius[j >> 1];
}

void KillProjectileFunc_6(uint16 j) {  // 0x90AE99
  int v1 = j >> 1;
  projectile_x_pos[v1] -= projectile_x_radius[v1];
  projectile_y_pos[v1] += projectile_y_radius[v1];
}

void KillProjectileFunc_7(uint16 j) {  // 0x90AEAE
  projectile_x_pos[j >> 1] -= projectile_x_radius[j >> 1];
}

void KillProjectileFunc_8(uint16 j) {  // 0x90AEB9
  int v1 = j >> 1;
  projectile_x_pos[v1] -= projectile_x_radius[v1];
  projectile_y_pos[v1] -= projectile_y_radius[v1];
}

void CallProjPreInstr(uint32 ea, uint16 k) {
  switch (ea) {
  case fnProjPreInstr_Empty: return;
  case fnProjPreInstr_Beam_NoWaveBeam: ProjPreInstr_Beam_NoWaveBeam(k); return;
  case fnProjPreInstr_Dir0459: ProjPreInstr_Dir0459(k); return;
  case fnProjPreInstr_Dir1368: ProjPreInstr_Dir1368(k); return;
  case fnProjPreInstr_Dir27: ProjPreInstr_Dir27(k); return;
  case fnProjPreInstr_Missile: ProjPreInstr_Missile(k); return;
  case fnProjPreInstr_Missile_Func0459: ProjPreInstr_Missile_Func0459(k); return;
  case fnProjPreInstr_Missile_Func1368: ProjPreInstr_Missile_Func1368(k); return;
  case fnProjPreInstr_Missile_Func27: ProjPreInstr_Missile_Func27(k); return;
  case fnProjPreInstr_SuperMissile: ProjPreInstr_SuperMissile(k); return;
  case fnProjPreInstr_SuperMissile_Func0459: ProjPreInstr_SuperMissile_Func0459(k); return;
  case fnProjPreInstr_SuperMissile_Func1368: ProjPreInstr_SuperMissile_Func1368(k); return;
  case fnProjPreInstr_SuperMissile_Func27: ProjPreInstr_SuperMissile_Func27(k); return;
  case fnProjPreInstr_Func1: ProjPreInstr_Func1(k); return;
  case fnProjPreInstr_Bomb: ProjPreInstr_Bomb(k); return;
  case fnProjPreInstr_PowerBomb: ProjPreInstr_PowerBomb(k); return;
  case fnProjPreInstr_WavePlasmaEtc: ProjPreInstr_WavePlasmaEtc(k); return;
  case fnProjPreInstr_BeamOrIceWave: ProjPreInstr_BeamOrIceWave(k); return;
  case fnProjPreInstr_Wave_Shared: ProjPreInstr_Wave_Shared(k); return;
  case fnProjPreInstr_WavePlasmaEtc_0459: ProjPreInstr_WavePlasmaEtc_0459(k); return;
  case fnProjPreInstr_WavePlasmaEtc_1368: ProjPreInstr_WavePlasmaEtc_1368(k); return;
  case fnProjPreInstr_WavePlasmaEtc_27: ProjPreInstr_WavePlasmaEtc_27(k); return;
  case fnProjPreInstr_HyperBeam: ProjPreInstr_HyperBeam(k); return;
  case fnProjPreInstr_IceSba: ProjPreInstr_IceSba(k); return;
  case fnProjPreInstr_IceSba2: ProjPreInstr_IceSba2(k); return;
  case fnProjPreInstr_SpeedEcho: ProjPreInstr_SpeedEcho(k); return;
  case fnProjPreInstr_PlasmaSba: ProjPreInstr_PlasmaSba(k); return;
  case fnProjPreInstr_PlasmaSbaFunc_0: ProjPreInstr_PlasmaSbaFunc_0(k); return;
  case fnProjPreInstr_PlasmaSbaFunc_1: ProjPreInstr_PlasmaSbaFunc_1(k); return;
  case fnProjPreInstr_PlasmaSbaFunc_2: ProjPreInstr_PlasmaSbaFunc_2(k); return;
  case fnProjPreInstr_SpreadBomb: ProjPreInstr_SpreadBomb(k); return;
  case fnProjPreInstr_WaveSba: ProjPreInstr_WaveSba(k); return;
  case fnProjPreInstr_SpazerSba: ProjPreInstr_SpazerSba(k); return;
  case fnProjPreInstr_EndOfSpazerSba: ProjPreInstr_EndOfSpazerSba(k); return;
  case fnProjPreInstr_UnknownProj8027: ProjPreInstr_UnknownProj8027(k); return;
  default: Unreachable();
  }
}

#endif

#ifdef SAMUS_PROJECTILE_BEAM_IMPL

void ProjPreInstr_UnknownProj8027(uint16 k) {  // 0x90EFD3
  static const int16 kProjPreInstr_UnknownProj8027_X[4] = { -4, -4, 4, 4 };
  static const int16 kProjPreInstr_UnknownProj8027_Y[4] = { 4, -4, -4, 4 };
  static const int16 kProjPreInstr_UnknownProj8027_X2[4] = { 0x80, 0x80, -0x80, -0x80 };
  static const int16 kProjPreInstr_UnknownProj8027_Y2[4] = { -0x80, 0x80, 0x80, -0x80 };

  int v1 = k >> 1;
  projectile_x_pos[v1] += kProjPreInstr_UnknownProj8027_X[v1];
  uint16 v2 = kProjPreInstr_UnknownProj8027_Y[v1] + projectile_y_pos[v1];
  projectile_y_pos[v1] = v2;
  if (v2 == samus_y_pos) {
    if (projectile_variables[v1] == 1) {
      if (k == 6)
        samus_movement_handler = FUNC16(Samus_Func25_ShineSpark);
      ClearProjectile(k);
    } else {
      int v3 = k >> 1;
      ++projectile_variables[v3];
      samus_shine_timer = 180;
      timer_for_shine_timer = 1;
      special_samus_palette_frame = 0;
      projectile_x_pos[v3] = kProjPreInstr_UnknownProj8027_X2[v3] + samus_x_pos;
      projectile_y_pos[v3] = kProjPreInstr_UnknownProj8027_Y2[v3] + samus_y_pos;
    }
  }
}

#endif

#ifdef SAMUS_PROJECTILE_CORE_IMPL

void HandleProjectile(void) {  // 0x90AECE
  projectile_index = 18;
  for (int i = 18; i >= 0; projectile_index = i) {
    int v1 = i >> 1;
    if (projectile_bomb_instruction_ptr[v1]) {
      CallProjPreInstr(projectile_bomb_pre_instructions[v1] | 0x900000, i);
      RunProjectileInstructions();
      i = projectile_index;
    }
      i -= 2;
  }
}

#endif

#ifdef SAMUS_PROJECTILE_BEAM_IMPL

static const int16 kDirToVelMult16_X[10] = { 0, 16, 16, 16, 0, 0, -16, -16, -16, 0 };
static const int16 kDirToVelMult16_Y[10] = { -16, -16, 0, 16, 16, 16, 16, 0, -16, -16 };

static Func_Y_V *const kProjPreInstr_Beam_Funcs[10] = {  // 0x90AEF3
  ProjPreInstr_Dir0459,
  ProjPreInstr_Dir1368,
  ProjPreInstr_Dir27,
  ProjPreInstr_Dir1368,
  ProjPreInstr_Dir0459,
  ProjPreInstr_Dir0459,
  ProjPreInstr_Dir1368,
  ProjPreInstr_Dir27,
  ProjPreInstr_Dir1368,
  ProjPreInstr_Dir0459,
};

void ProjPreInstr_Beam_NoWaveBeam(uint16 k) {
  int v1 = k >> 1;
  if ((projectile_dir[v1] & 0xF0) != 0) {
    ClearProjectile(k);
  } else {
    if (projectile_timers[v1]-- == 1) {
      projectile_timers[v1] = 4;
      SpawnProjectileTrail(k);
      k = projectile_index;
    }
    int v3 = k >> 1;
    uint16 v4 = 2 * (projectile_dir[v3] & 0xF);
    int v5 = v4 >> 1;
    projectile_bomb_x_speed[v3] += kDirToVelMult16_X[v5];
    projectile_bomb_y_speed[v3] += kDirToVelMult16_Y[v5];
    kProjPreInstr_Beam_Funcs[v5](v4);
    DeleteProjectileIfFarOffScreen();
  }
}

void ProjPreInstr_Dir0459(uint16 k) {  // 0x90AF4A
  BlockCollNoWaveBeamVert(projectile_index);
}

void ProjPreInstr_Dir1368(uint16 k) {  // 0x90AF52
  uint16 v1 = projectile_index;
  if (!(BlockCollNoWaveBeamHoriz(projectile_index) & 1))
    BlockCollNoWaveBeamVert(v1);
}

void ProjPreInstr_Dir27(uint16 k) {  // 0x90AF60
  BlockCollNoWaveBeamHoriz(projectile_index);
}

static Func_Y_V *const kProjPreInstr_Missile_Funcs[10] = {  // 0x90AF68
  ProjPreInstr_Missile_Func0459,
  ProjPreInstr_Missile_Func1368,
  ProjPreInstr_Missile_Func27,
  ProjPreInstr_Missile_Func1368,
  ProjPreInstr_Missile_Func0459,
  ProjPreInstr_Missile_Func0459,
  ProjPreInstr_Missile_Func1368,
  ProjPreInstr_Missile_Func27,
  ProjPreInstr_Missile_Func1368,
  ProjPreInstr_Missile_Func0459,
};

void ProjPreInstr_Missile(uint16 k) {
  int v1 = k >> 1;
  if ((projectile_dir[v1] & 0xF0) != 0) {
    ClearProjectile(k);
  } else {
    if (projectile_timers[v1]-- == 1) {
      projectile_timers[v1] = 4;
      SpawnProjectileTrail(k);
      k = projectile_index;
    }
    int v3 = k >> 1;
    uint16 v4 = 2 * (projectile_dir[v3] & 0xF);
    int v5 = v4 >> 1;
    projectile_bomb_x_speed[v3] += kDirToVelMult16_X[v5];
    projectile_bomb_y_speed[v3] += kDirToVelMult16_Y[v5];
    Missile_Func1(k);
    kProjPreInstr_Missile_Funcs[v4 >> 1](v4);
    DeleteProjectileIfFarOffScreen();
  }
}

void ProjPreInstr_Missile_Func0459(uint16 k) {  // 0x90AFC7
  BlockCollMissileVert(projectile_index);
}

void ProjPreInstr_Missile_Func1368(uint16 k) {  // 0x90AFCF
  uint16 v1 = projectile_index;
  if (!BlockCollMissileHoriz(projectile_index))
    BlockCollMissileVert(v1);
}

void ProjPreInstr_Missile_Func27(uint16 k) {  // 0x90AFDD
  BlockCollMissileHoriz(projectile_index);
}

static Func_Y_V *const kProjPreInstr_SuperMissile_Funcs[10] = {  // 0x90AFE5
  ProjPreInstr_SuperMissile_Func0459,
  ProjPreInstr_SuperMissile_Func1368,
  ProjPreInstr_SuperMissile_Func27,
  ProjPreInstr_SuperMissile_Func1368,
  ProjPreInstr_SuperMissile_Func0459,
  ProjPreInstr_SuperMissile_Func0459,
  ProjPreInstr_SuperMissile_Func1368,
  ProjPreInstr_SuperMissile_Func27,
  ProjPreInstr_SuperMissile_Func1368,
  ProjPreInstr_SuperMissile_Func0459,
};

void ProjPreInstr_SuperMissile(uint16 k) {
  int16 v3;

  int v1 = k >> 1;
  if ((projectile_dir[v1] & 0xF0) != 0) {
    ClearProjectile(k);
LABEL_7:
    for (int i = 8; i >= 0; i -= 2) {
      if ((projectile_type[i >> 1] & 0xFFF) == 512)
        ClearProjectile(i);
    }
    return;
  }
  if (projectile_timers[v1]-- == 1) {
    projectile_timers[v1] = 2;
    SpawnProjectileTrail(k);
    k = projectile_index;
  }
  v3 = projectile_dir[k >> 1] & 0xF;
  Missile_Func1(k);
  kProjPreInstr_SuperMissile_Funcs[v3](v3);
  if (DeleteProjectileIfFarOffScreen() & 1)
    goto LABEL_7;
}

void ProjPreInstr_SuperMissile_Func0459(uint16 k) {  // 0x90B047
  BlockCollMissileVert(projectile_index);
  SuperMissileBlockCollDetect_Y();
}

void ProjPreInstr_SuperMissile_Func1368(uint16 k) {  // 0x90B052
  uint16 v1 = projectile_index;
  if (BlockCollMissileHoriz(projectile_index)) {
    SuperMissileBlockCollDetect_X();
  } else {
    SuperMissileBlockCollDetect_X();
    BlockCollMissileVert(v1);
    SuperMissileBlockCollDetect_Y();
  }
}

void ProjPreInstr_SuperMissile_Func27(uint16 k) {  // 0x90B06A
  BlockCollMissileHoriz(projectile_index);
  SuperMissileBlockCollDetect_X();
}

void ProjPreInstr_Func1(uint16 k) {  // 0x90B075
  if ((projectile_dir[k >> 1] & 0xF0) != 0) {
    ClearProjectile(k);
    for (int i = 8; i >= 0; i -= 2) {
      if ((projectile_type[i >> 1] & 0xFFF) == 512)
        ClearProjectile(i);
    }
  }
}

void ProjPreInstr_Bomb(uint16 k) {  // 0x90B099
  if ((projectile_dir[k >> 1] & 0xF0) != 0) {
    ClearProjectile(k);
  } else {
    Bomb_Func2();
    BombOrPowerBomb_Func1(k);
  }
}

void ProjPreInstr_PowerBomb(uint16 k) {  // 0x90B0AE
  if ((projectile_dir[k >> 1] & 0xF0) != 0) {
    ClearProjectile(k);
  } else {
    PowerBomb_Func3();
    BombOrPowerBomb_Func1(k);
  }
}

void ProjPreInstr_WavePlasmaEtc(uint16 k) {  // 0x90B0C3
  int v1 = k >> 1;
  if ((projectile_dir[v1] & 0xF0) != 0) {
    ClearProjectile(k);
  } else if (projectile_timers[v1]-- == 1) {
    projectile_timers[v1] = 4;
    SpawnProjectileTrail(k);
    ProjPreInstr_Wave_Shared(projectile_index);
  } else {
    ProjPreInstr_Wave_Shared(k);
  }
}

void ProjPreInstr_BeamOrIceWave(uint16 k) {  // 0x90B0E4
  int v1 = k >> 1;
  if ((projectile_dir[v1] & 0xF0) != 0) {
    ClearProjectile(k);
  } else if (projectile_timers[v1]-- == 1) {
    projectile_timers[v1] = 3;
    SpawnProjectileTrail(k);
    ProjPreInstr_Wave_Shared(projectile_index);
  } else {
    ProjPreInstr_Wave_Shared(k);
  }
}

static Func_Y_V *const kProjPreInstr_WavePlasmaEtcFuncs[10] = {  // 0x90B103
  ProjPreInstr_WavePlasmaEtc_0459,
  ProjPreInstr_WavePlasmaEtc_1368,
  ProjPreInstr_WavePlasmaEtc_27,
  ProjPreInstr_WavePlasmaEtc_1368,
  ProjPreInstr_WavePlasmaEtc_0459,
  ProjPreInstr_WavePlasmaEtc_0459,
  ProjPreInstr_WavePlasmaEtc_1368,
  ProjPreInstr_WavePlasmaEtc_27,
  ProjPreInstr_WavePlasmaEtc_1368,
  ProjPreInstr_WavePlasmaEtc_0459,
};

void ProjPreInstr_Wave_Shared(uint16 k) {
  int v1 = k >> 1;
  uint16 v2 = 2 * (projectile_dir[v1] & 0xF);
  int v3 = v2 >> 1;
  projectile_bomb_x_speed[v1] += kDirToVelMult16_X[v3];
  projectile_bomb_y_speed[v1] += kDirToVelMult16_Y[v3];
  kProjPreInstr_WavePlasmaEtcFuncs[v3](v2);
  DeleteProjectileIfFarOffScreen();
}

void ProjPreInstr_WavePlasmaEtc_0459(uint16 k) {  // 0x90B13B
  BlockCollWaveBeamVert(projectile_index);
}

void ProjPreInstr_WavePlasmaEtc_1368(uint16 k) {  // 0x90B143
  uint16 v1 = projectile_index;
  if (!(BlockCollWaveBeamHoriz(projectile_index) & 1))
    BlockCollWaveBeamVert(v1);
}

void ProjPreInstr_WavePlasmaEtc_27(uint16 k) {  // 0x90B151
  BlockCollWaveBeamHoriz(projectile_index);
}

void ProjPreInstr_HyperBeam(uint16 k) {  // 0x90B159
  if ((projectile_dir[k >> 1] & 0xF0) != 0)
    ClearProjectile(k);
  else
    ProjPreInstr_Wave_Shared(k);
}

uint8 DeleteProjectileIfFarOffScreen(void) {  // 0x90B16A
  int16 v0;
  int16 v2;

  v0 = projectile_x_pos[projectile_index >> 1] - layer1_x_pos;
  if (!sign16(v0 + 64)) {
    if (sign16(v0 - 320)) {
      v2 = projectile_y_pos[projectile_index >> 1] - layer1_y_pos;
      if (!sign16(v2 + 64)) {
        if (sign16(v2 - 320))
          return 0;
      }
    }
  }
  ClearProjectile(projectile_index);
  return 1;
}

static const uint16 kInitializeProjectileSpeed_XY_Diag[24] = {  // 0x90B197
  0x400, 0x2ab,
  0x400, 0x2ab,
  0x400, 0x2ab,
  0x400, 0x2ab,
  0x400, 0x2ab,
  0x400, 0x2ab,
  0x400, 0x2ab,
  0x400, 0x2ab,
  0x400, 0x2ab,
  0x400, 0x2ab,
  0x400, 0x2ab,
  0x400, 0x2ab,
};

void SetInitialProjectileSpeed(uint16 r20) {
  int v0 = r20 >> 1;
  uint16 v1 = 4 * (projectile_type[v0] & 0xF);
  uint16 v2 = 2 * (projectile_dir[v0] & 0xF);
  uint16 r22;
  if (!v2 || v2 == 4 || v2 == 8 || v2 == 10 || v2 == 14 || v2 == 18) {
    r22 = kInitializeProjectileSpeed_XY_Diag[v1 >> 1];
  } else {
    if (v2 != 2 && v2 != 6 && v2 != 12 && v2 != 16)
      Unreachable();
    r22 = kInitializeProjectileSpeed_XY_Diag[(v1 >> 1) + 1];
  }
  InitializeProjectileSpeed(r20, r22);
}

void InitializeProjectileSpeedOfType(uint16 r20) {  // 0x90B1DD
  InitializeProjectileSpeed(r20, 0);
}

void InitializeProjectileSpeed(uint16 k, uint16 r22) {  // 0x90B1F3
  uint16 r18;

  int kh = k >> 1;
  projectile_bomb_x_subpos[kh] = 0;
  projectile_bomb_y_subpos[kh] = 0;
  switch (2 * (projectile_dir[kh] & 0xF)) {
  case 0:
  case 18: {
    if ((uint8)projectile_init_speed_samus_moved_up)
      r18 = (*(uint16 *)((uint8 *)&projectile_init_speed_samus_moved_right_fract + 1) >> 2) | 0xC000;
    else
      r18 = 0;
    projectile_bomb_y_speed[kh] = r18 - r22;
    projectile_bomb_x_speed[kh] = 0;
    break;
  }
  case 2: {
    if ((uint8)projectile_init_speed_samus_moved_up)
      r18 = (*(uint16 *)((uint8 *)&projectile_init_speed_samus_moved_right_fract + 1) >> 2) | 0xC000;
    else
      r18 = 0;
    projectile_bomb_y_speed[kh] = r18 - r22;
    projectile_bomb_x_speed[kh] = *(uint16 *)((uint8 *)&projectile_init_speed_samus_moved_left_fract + 1) + r22;
    break;
  }
  case 4: {
    projectile_bomb_y_speed[kh] = 0;
    projectile_bomb_x_speed[kh] = *(uint16 *)((uint8 *)&projectile_init_speed_samus_moved_left_fract + 1) + r22;
    break;
  }
  case 6: {
    projectile_bomb_y_speed[kh] = *(uint16 *)((uint8 *)&projectile_init_speed_samus_moved_up_fract + 1) + r22;
    projectile_bomb_x_speed[kh] = *(uint16 *)((uint8 *)&projectile_init_speed_samus_moved_left_fract + 1) + r22;
    break;
  }
  case 8:
  case 10: {
    projectile_bomb_y_speed[kh] = *(uint16 *)((uint8 *)&projectile_init_speed_samus_moved_up_fract + 1) + r22;
    projectile_bomb_x_speed[kh] = 0;
    break;
  }
  case 12: {
    projectile_bomb_y_speed[kh] = *(uint16 *)((uint8 *)&projectile_init_speed_samus_moved_up_fract + 1) + r22;
    projectile_bomb_x_speed[kh] = *(uint16 *)((uint8 *)&absolute_moved_last_frame_y_fract + 1) - r22;
    break;
  }
  case 14: {
    projectile_bomb_y_speed[kh] = 0;
    projectile_bomb_x_speed[kh] = *(uint16 *)((uint8 *)&absolute_moved_last_frame_y_fract + 1) - r22;
    break;
  }
  case 16: {
    if ((uint8)projectile_init_speed_samus_moved_up)
      r18 = (*(uint16 *)((uint8 *)&projectile_init_speed_samus_moved_right_fract + 1) >> 2) | 0xC000;
    else
      r18 = 0;
    projectile_bomb_y_speed[kh] = r18 - r22;
    projectile_bomb_x_speed[kh] = *(uint16 *)((uint8 *)&absolute_moved_last_frame_y_fract + 1) - r22;
    break;
  }
  default:
    Unreachable();
  }
}

void Missile_Func1(uint16 k) {  // 0x90B2F6
  static const uint16 word_90C301 = 0x100;
  uint16 v3;

  int v1 = k >> 1;
  if ((projectile_variables[v1] & 0xFF00) != 0) {
    if ((projectile_type[v1] & 0x200) != 0)
      v3 = addr_kSuperMissileAccelerations2;
    else
      v3 = addr_kSuperMissileAccelerations;
    const uint8 *v4 = RomPtr_90(v3 + 4 * (projectile_dir[v1] & 0xF));
    projectile_bomb_x_speed[v1] += GET_WORD(v4);
    projectile_bomb_y_speed[v1] += GET_WORD(v4 + 2);
  } else {
    uint16 v2 = word_90C301 + projectile_variables[v1];
    projectile_variables[v1] = v2;
    if ((v2 & 0xFF00) != 0) {
      InitializeProjectileSpeed(k, projectile_variables[v1]);
      if ((projectile_type[v1] & 0x200) != 0)
        Missile_Func2();
    }
  }
}

void SuperMissileBlockCollDetect_Y(void) {  // 0x90B366
  int v0 = projectile_index >> 1;
  if ((projectile_type[v0] & 0xF00) == 512 || (projectile_type[v0] & 0xF00) == 2048) {
    uint8 v5 = projectile_variables[v0];
    if (*((uint8 *)projectile_variables + projectile_index + 1)) {
      uint16 v1 = abs16(projectile_bomb_y_speed[v0]) & 0xFF00;
      if (sign16(v1 - 2816)) {
        int v4 = projectile_index >> 1;
        if ((projectile_type[v4] & 0xF00) != 2048) {
          projectile_y_pos[v5 >> 1] = projectile_y_pos[v4];
          return;
        }
      } else {
        uint16 r18 = (v1 >> 8) - 10;
        if ((projectile_bomb_y_speed[v0] & 0x8000) == 0) {
          uint16 v6 = projectile_index;
          projectile_y_pos[v5 >> 1] = projectile_y_pos[v0] - r18;
          projectile_index = v5;
          BlockCollMissileVert(v5);
          projectile_index = v6;
          if ((projectile_type[v6 >> 1] & 0xF00) == 2048)
            ClearProjectile(v5);
          return;
        }
        uint16 v7 = projectile_index;
        projectile_y_pos[v5 >> 1] = r18 + projectile_y_pos[projectile_index >> 1];
        projectile_index = v5;
        BlockCollMissileVert(v5);
        projectile_index = v7;
        if ((projectile_type[v7 >> 1] & 0xF00) != 2048)
          return;
      }
      ClearProjectile(v5);
    }
  }
}

void Projectile_Func4(uint16 k) {  // 0x90B4A6
  int v1 = k >> 1;
  if (*((uint8 *)projectile_variables + k + 1)) {
    if ((projectile_type[v1] & 0xF00) == 2048)
      ClearProjectile((uint8)projectile_variables[v1]);
  }
}

void SuperMissileBlockCollDetect_X(void) {  // 0x90B406
  int v0 = projectile_index >> 1;
  if ((projectile_type[v0] & 0xF00) == 512 || (projectile_type[v0] & 0xF00) == 2048) {
    uint8 v5 = projectile_variables[v0];
    if (*((uint8 *)projectile_variables + projectile_index + 1)) {
      uint16 v1 = abs16(projectile_bomb_x_speed[v0]) & 0xFF00;
      if (sign16(v1 - 2816)) {
        int v4 = projectile_index >> 1;
        if ((projectile_type[v4] & 0xF00) != 2048) {
          projectile_x_pos[v5 >> 1] = projectile_x_pos[v4];
          return;
        }
      } else {
        uint16 r18 = (v1 >> 8) - 10;
        if ((projectile_bomb_x_speed[v0] & 0x8000) == 0) {
          uint16 v6 = projectile_index;
          projectile_x_pos[v5 >> 1] = projectile_x_pos[v0] - r18;
          projectile_index = v5;
          BlockCollMissileHoriz(v5);
          projectile_index = v6;
          if ((projectile_type[v6 >> 1] & 0xF00) == 2048)
            ClearProjectile(v5);
          return;
        }
        uint16 v7 = projectile_index;
        projectile_x_pos[v5 >> 1] = r18 + projectile_x_pos[projectile_index >> 1];
        projectile_index = v5;
        BlockCollMissileHoriz(v5);
        projectile_index = v7;
        if ((projectile_type[v7 >> 1] & 0xF00) != 2048)
          return;
      }
      ClearProjectile(v5);
    }
  }
}

#endif

#ifdef SAMUS_PROJECTILE_CORE_IMPL

void ProjInstr_MoveLeftProjectileTrailDown(uint16 j) {  // 0x90B525
  ++projectiletrail_left_y_pos[j >> 1];
}

void ProjInstr_MoveRightProjectileTrailDown(uint16 j) {  // 0x90B587
  ++projectiletrail_right_y_pos[j >> 1];
}

void ProjInstr_MoveLeftProjectileTrailUp(uint16 j) {  // 0x90B5B3
  --projectiletrail_left_y_pos[j >> 1];
}

void SpawnProjectileTrail(uint16 k) {  // 0x90B657
  int16 v2;

  uint16 v1 = projectile_type[k >> 1];
  if ((v1 & 0xF00) != 0) {
    uint16 v3 = HIBYTE(v1) & 0xF;
    if (v3 >= 3)
      return;
    v2 = v3 + 31;
  } else {
    v2 = projectile_type[k >> 1] & 0x3F;
  }
  uint16 v4 = 34;
  while (projectiletrail_left_instr_timer[v4 >> 1]) {
    v4 -= 2;
    if ((v4 & 0x8000) != 0)
      return;
  }
  int v5 = v4 >> 1;
  projectiletrail_left_instr_timer[v5] = 1;
  projectiletrail_right_instr_timer[v5] = 1;
  int v6 = v2;
  projectiletrail_left_instr_list_ptr[v5] = off_90B5BB[v6];
  projectiletrail_right_instr_list_ptr[v5] = off_90B609[v6];
  ProjectileTrail_Func5(projectile_index, v4);
}

void ProjectileTrail_Func5(uint16 k, uint16 j) {  // 0x9BA3CC
  uint16 R22 = ProjectileInsts_GetValue(k);
  uint16 r18, r20;
  if ((ceres_status & 0x8000) == 0) {
    int v2 = k >> 1;
    r18 = projectile_x_pos[v2];
    r20 = projectile_y_pos[v2];
  } else {
    Point16U pt = CalcExplosion_Mode7(k);
    r18 = layer1_x_pos + pt.x;
    r20 = layer1_y_pos + pt.y;
  }
  int v3 = k >> 1;
  uint16 v4 = projectile_type[v3], v5;
  if ((v4 & 0x20) != 0) {
    v5 = g_off_9BA4E3[projectile_type[v3] & 0xF] + 2 * (projectile_dir[v3] & 0xF);
  } else if ((v4 & 0x10) != 0) {
    v5 = g_off_9BA4CB[projectile_type[v3] & 0xF] + 2 * (projectile_dir[v3] & 0xF);
  } else {
    v5 = g_off_9BA4B3[projectile_type[v3] & 0xF] + 2 * (projectile_dir[v3] & 0xF);
  }
  uint16 v6 = *(uint16 *)RomPtr_9B(v5) + 4 * R22;
  const uint8 *p = RomPtr_9B(v6);
  int v7 = j >> 1;
  projectiletrail_left_y_pos[v7] = r20 + (int8)p[1] - 4;
  projectiletrail_left_x_pos[v7] = r18 + (int8)p[0] - 4;
  projectiletrail_right_y_pos[v7] = r20 + (int8)p[3] - 4;
  projectiletrail_right_x_pos[v7] = r18 + (int8)p[2] - 4;
}

void CallProjInstr(uint32 ea, uint16 j) {
  switch (ea) {
  case fnProjInstr_MoveLeftProjectileTrailDown: ProjInstr_MoveLeftProjectileTrailDown(j); return;
  case fnProjInstr_MoveRightProjectileTrailDown: ProjInstr_MoveRightProjectileTrailDown(j); return;
  case fnProjInstr_MoveLeftProjectileTrailUp: ProjInstr_MoveLeftProjectileTrailUp(j); return;
  default: Unreachable();
  }
}

void HandleProjectileTrails(void) {  // 0x90B6A9
  int i;
  int16 v10;
  OamEnt *v11;
  int16 v12;
  uint16 j;
  int16 v22;
  OamEnt *v23;
  int16 v24;
  uint16 k;
  int16 v28;
  OamEnt *v29;
  int16 v30;
  int16 v33;
  OamEnt *v34;
  int16 v35;
  uint16 v6, v18;
  int v7;

  if (!time_is_frozen_flag) {
    uint16 v0 = 34;
    while (1) {
      uint16 v2;
      int v1;
      v1 = v0 >> 1;
      v2 = projectiletrail_left_instr_timer[v1];
      if (v2) {
        uint16 v3 = v2 - 1;
        projectiletrail_left_instr_timer[v1] = v3;
        if (v3)
          goto LABEL_10;
        for (i = projectiletrail_left_instr_list_ptr[v1]; ; i += 2) {
          const uint16 *v5 = (const uint16 *)RomPtr_90(i);
          v6 = *v5;
          if ((*v5 & 0x8000) == 0)
            break;
          CallProjInstr(v6 | 0x900000, v0);
        }
        v7 = v0 >> 1;
        projectiletrail_left_instr_timer[v7] = v6;
        if (v6)
          break;
      }
LABEL_14:
      ;
      int v13 = v0 >> 1;
      uint16 v14 = projectiletrail_right_instr_timer[v13];
      if (v14) {
        uint16 v15 = v14 - 1;
        projectiletrail_right_instr_timer[v13] = v15;
        if (v15)
          goto LABEL_21;
        for (j = projectiletrail_right_instr_list_ptr[v13]; ; j += 2) {
          const uint16 *v17 = (const uint16 *)RomPtr_90(j);
          v18 = *v17;
          if ((*v17 & 0x8000) == 0)
            break;
          CallProjInstr(v18 | 0x900000, v0);
        }
        int v19 = v0 >> 1;
        projectiletrail_right_instr_timer[v19] = v18;
        if (v18) {
          projectiletrail_right_tile_and_attribs[v19] = *((uint16 *)RomPtr_90(j) + 1);
          projectiletrail_right_instr_list_ptr[v19] = j + 4;
LABEL_21:
          ;
          uint16 v20 = oam_next_ptr;
          if ((int16)(oam_next_ptr - 512) < 0) {
            int v21 = v0 >> 1;
            v22 = projectiletrail_right_x_pos[v21] - layer1_x_pos;
            if ((v22 & 0xFF00) == 0) {
              v23 = gOamEnt(oam_next_ptr);
              *(uint16 *)&v23->xcoord = v22;
              v24 = projectiletrail_right_y_pos[v21] - layer1_y_pos;
              if ((v24 & 0xFF00) == 0) {
                *(uint16 *)&v23->ycoord = v24;
                *(uint16 *)&v23->charnum = projectiletrail_right_tile_and_attribs[v21];
                oam_next_ptr = v20 + 4;
              }
            }
          }
        }
      }
      v0 -= 2;
      if ((v0 & 0x8000) != 0)
        return;
    }
    projectiletrail_left_tile_and_attribs[v7] = *((uint16 *)RomPtr_90(i) + 1);
    projectiletrail_left_instr_list_ptr[v7] = i + 4;
LABEL_10:
    ;
    uint16 v8 = oam_next_ptr;
    if ((int16)(oam_next_ptr - 512) < 0) {
      int v9 = v0 >> 1;
      v10 = projectiletrail_left_x_pos[v9] - layer1_x_pos;
      if ((v10 & 0xFF00) == 0) {
        v11 = gOamEnt(oam_next_ptr);
        *(uint16 *)&v11->xcoord = v10;
        v12 = projectiletrail_left_y_pos[v9] - layer1_y_pos;
        if ((v12 & 0xFF00) == 0) {
          *(uint16 *)&v11->ycoord = v12;
          *(uint16 *)&v11->charnum = projectiletrail_left_tile_and_attribs[v9];
          oam_next_ptr = v8 + 4;
        }
      }
    }
    goto LABEL_14;
  }
  for (k = 34; (k & 0x8000) == 0; k -= 2) {
    uint16 v26 = oam_next_ptr;
    if ((int16)(oam_next_ptr - 512) < 0) {
      int v27 = k >> 1;
      if (projectiletrail_left_instr_timer[v27]) {
        v28 = projectiletrail_left_x_pos[v27] - layer1_x_pos;
        if ((v28 & 0xFF00) == 0) {
          v29 = gOamEnt(oam_next_ptr);
          *(uint16 *)&v29->xcoord = v28;
          v30 = projectiletrail_left_y_pos[v27] - layer1_y_pos;
          if ((v30 & 0xFF00) == 0) {
            *(uint16 *)&v29->ycoord = v30;
            *(uint16 *)&v29->charnum = projectiletrail_left_tile_and_attribs[v27];
            oam_next_ptr = v26 + 4;
          }
        }
      }
    }
    uint16 v31 = oam_next_ptr;
    if ((int16)(oam_next_ptr - 512) < 0) {
      int v32 = k >> 1;
      if (projectiletrail_right_instr_timer[v32]) {
        v33 = projectiletrail_right_x_pos[v32] - layer1_x_pos;
        if ((v33 & 0xFF00) == 0) {
          v34 = gOamEnt(oam_next_ptr);
          *(uint16 *)&v34->xcoord = v33;
          v35 = projectiletrail_right_y_pos[v32] - layer1_y_pos;
          if ((v35 & 0xFF00) == 0) {
            *(uint16 *)&v34->ycoord = v35;
            *(uint16 *)&v34->charnum = projectiletrail_right_tile_and_attribs[v32];
            oam_next_ptr = v31 + 4;
          }
        }
      }
    }
  }
}

#endif

#ifdef SAMUS_PROJECTILE_BEAM_IMPL

void HudSelectionHandler_NothingOrPowerBombs(void) {  // 0x90B80D
  prev_beam_charge_counter = flare_counter;
  if (hyper_beam_flag || (equipped_beams & 0x1000) == 0) {
    if ((button_config_shoot_x & joypad1_lastkeys) != 0)
      FireUnchargedBeam();
    return;
  }
  if (new_projectile_direction_changed_pose) {
    if (sign16(flare_counter - 60)) {
LABEL_14:
      flare_counter = 0;
      ClearFlareAnimationState();
      FireUnchargedBeam();
      return;
    }
LABEL_15:
    flare_counter = 0;
    ClearFlareAnimationState();
    FireChargedBeam();
    return;
  }
  if ((button_config_shoot_x & joypad1_lastkeys) == 0) {
    if (!flare_counter)
      return;
    if (sign16(flare_counter - 60))
      goto LABEL_14;
    goto LABEL_15;
  }
  if (sign16(flare_counter - 120)) {
    if (++flare_counter == 1) {
      ClearFlareAnimationState();
      FireUnchargedBeam();
    }
  } else if (FireSba() & 1) {
    flare_counter = 0;
    ClearFlareAnimationState();
    Samus_LoadSuitPalette();
  }
}

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

void FireUnchargedBeam(void) {
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
      projectile_type[v2] = equipped_beams | 0x8000;
      QueueSfx1_Max15(kUnchargedProjectile_Sfx[v3 & 0xF]);
      play_resume_charging_beam_sfx = 0;
      InitializeProjectile(v1);
      if ((equipped_beams & 0x1000) != 0
          || (button_config_shoot_x & joypad1_newkeys) != 0
          || (button_config_shoot_x & joypad1_newinput_samusfilter) != 0) {
        uint16 v6 = projectile_type[v2];
        cooldown_timer = kProjectileCooldown_Uncharged[v6 & 0x3F];
        if ((v6 & 1) == 0)
          goto LABEL_17;
      } else {
        uint16 v5 = projectile_type[v2];
        cooldown_timer = kBeamAutoFireCooldowns[v5 & 0x3F];
        if ((v5 & 1) == 0) {
LABEL_17:
          projectile_bomb_x_speed[v2] = 0;
          projectile_bomb_y_speed[v2] = 0;
          projectile_index = v1;
          CheckBeamCollByDir(v1);
          v1 = projectile_index;
          if ((projectile_type[projectile_index >> 1] & 0xF00) != 0)
            return;
          goto LABEL_20;
        }
      }
      int v4 = v1 >> 1;
      projectile_bomb_x_speed[v4] = 0;
      projectile_bomb_y_speed[v4] = 0;
      projectile_index = v1;
      WaveBeam_CheckColl(v1);
LABEL_20:
      projectile_bomb_pre_instructions[v1 >> 1] = kProjectileBombPreInstr[projectile_type[v1 >> 1] & 0xF];
      SetInitialProjectileSpeed(v1);
      return;
    }
  }
  if (!sign16(prev_beam_charge_counter - 16)) {
    play_resume_charging_beam_sfx = 0;
    QueueSfx1_Max15(2);
  }
}

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

void FireChargedBeam(void) {
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
      uint16 v3 = equipped_beams & 0x100F | 0x8010;
      projectile_type[v2] = v3;
      QueueSfx1_Max15(kChargedProjectile_Sfx[v3 & 0xF]);
      play_resume_charging_beam_sfx = 0;
      InitializeProjectile(v1);
      uint16 v5 = projectile_type[v2];
      cooldown_timer = kProjectileCooldown_Uncharged[v5 & 0x3F];
      if ((v5 & 1) != 0) {
        int v4 = v1 >> 1;
        projectile_bomb_x_speed[v4] = 0;
        projectile_bomb_y_speed[v4] = 0;
        projectile_index = v1;
        WaveBeam_CheckColl(v1);
      } else {
        projectile_bomb_x_speed[v2] = 0;
        projectile_bomb_y_speed[v2] = 0;
        projectile_index = v1;
        CheckBeamCollByDir(v1);
        v1 = projectile_index;
        if ((projectile_type[projectile_index >> 1] & 0xF00) != 0) {
LABEL_14:
          charged_shot_glow_timer = 4;
          return;
        }
      }
      r20 = v1;
      projectile_bomb_pre_instructions[v1 >> 1] = kFireChargedBeam_Funcs[projectile_type[v1 >> 1] & 0xF];
      SetInitialProjectileSpeed(r20);
      goto LABEL_14;
    }
  }
  if (!sign16(prev_beam_charge_counter - 16)) {
    play_resume_charging_beam_sfx = 0;
    QueueSfx1_Max15(2);
  }
}

static const int16 kProjectileOriginOffsets3_X[10] = { 2, 13, 11, 13, 2, -5, -14, -11, -19, -2 };
static const int16 kProjectileOriginOffsets3_Y[10] = { -8, -13, 1, 4, 13, 13, 4, 1, -19, -8 };
static const int16 kProjectileOriginOffsets4_X[10] = { 2, 15, 15, 13, 2, -5, -13, -13, -15, -2 };
static const int16 kProjectileOriginOffsets4_Y[10] = { -8, -16, -2, 1, 13, 13, 1, -2, -16, -8 };

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

void HandleChargingBeamGfxAudio(void) {  // 0x90BAFC
  int16 v1;
  int16 v4;
  uint16 v9;

  if (hyper_beam_flag) {
    if (flare_counter) {
      for (int i = 4; i >= 0; i -= 2) {
        bool v7 = *(uint16 *)((uint8 *)&flare_animation_timer + i) == 1;
        bool v8 = (-- * (uint16 *)((uint8 *)&flare_animation_timer + i) & 0x8000) != 0;
        if (v7 || v8) {
          v7 = (*(uint16 *)((uint8 *)&flare_animation_frame + i))-- == 1;
          if (v7) {
            if (i == 4)
              flare_counter = 0;
          } else {
            *(uint16 *)((uint8 *)&flare_animation_timer + i) = 3;
          }
        }
        DrawFlareAnimationComponent(i);
      }
    }
  } else if ((int16)flare_counter > 0) {
    if (flare_counter == 1) {
      flare_animation_frame = 0;
      flare_slow_sparks_anim_frame = 0;
      flare_fast_sparks_anim_frame = 0;
      flare_animation_timer = 3;
      flare_slow_sparks_anim_timer = 5;
      flare_fast_sparks_anim_timer = 4;
    }
    if (!sign16(flare_counter - 15)) {
      if (flare_counter == 16)
        QueueSfx1_Max9(8);
      uint16 v0 = 0;
      do {
        v1 = *(uint16 *)((uint8 *)&flare_animation_timer + v0) - 1;
        *(uint16 *)((uint8 *)&flare_animation_timer + v0) = v1;
        if (v1 < 0) {
          uint16 v2 = *(uint16 *)((uint8 *)&flare_animation_frame + v0) + 1;
          *(uint16 *)((uint8 *)&flare_animation_frame + v0) = v2;
          uint16 v3 = v2;
          const uint8 *r0 = RomPtr_90(kFlareAnimDelays[v0 >> 1]);
          v4 = r0[v2];
          if (v4 == 255) {
            *(uint16 *)((uint8 *)&flare_animation_frame + v0) = 0;
            v3 = 0;
          } else if (v4 == 254) {
            uint16 v5 = *(uint16 *)((uint8 *)&flare_animation_frame + v0) - r0[v3 + 1];
            *(uint16 *)((uint8 *)&flare_animation_frame + v0) = v5;
            v3 = v5;
          }
          *(uint16 *)((uint8 *)&flare_animation_timer + v0) = r0[v3];
        }
        v9 = v0;
        DrawFlareAnimationComponent(v0);
        if (sign16(flare_counter - 30))
          break;
        v0 += 2;
      } while ((int16)(v9 - 4) < 0);
    }
  }
}

static const int16 kProjectileOriginOffsets_X[13] = { 2, 18, 15, 17, 3, -4, -17, -15, -18, -2, -4, -4, -4 };
static const int16 kProjectileOriginOffsets_Y[13] = { -28, -19, 1, 6, 17, 17, 6, 1, -20, -28, -20, -2, 8 };
static const int16 kProjectileOriginOffsets2_X[10] = { 2, 19, 20, 18, 3, -4, -18, -20, -19, -2 };
static const int16 kProjectileOriginOffsets2_Y[10] = { -32, -22, -3, 6, 25, 25, 6, -3, -20, -32 };

void DrawFlareAnimationComponent(uint16 k) {  // 0x90BBE1
  static const uint16 word_93A225[3] = { 0, 0x1e, 0x24 };
  static const uint16 word_93A22B[3] = { 0, 0x2a, 0x30 };
  int16 v2;
  uint16 v1;
  uint16 r20;
  uint16 r18 = *((uint8 *)&flare_animation_frame + k);
  if (samus_pose_x_dir == 4)
    v1 = word_93A22B[k >> 1];
  else
    v1 = word_93A225[k >> 1];
  uint16 r22 = r18 + v1;
  uint16 r24 = kPoseParams[samus_pose].y_offset_to_gfx;
  v2 = kPoseParams[samus_pose].direction_shots_fired;
  if (v2 != 255 && v2 != 16) {
    uint16 v3 = 2 * (v2 & 0xF);
    uint16 old_x = samus_x_pos, old_y = samus_y_pos;
    if ((ceres_status & 0x8000) != 0)
      Samus_CalcPos_Mode7();
    int v4 = v3 >> 1;
    uint16 v5;
    if (samus_movement_type == 1) {
      r20 = samus_x_pos + kProjectileOriginOffsets2_X[v4] - layer1_x_pos;
      v5 = samus_y_pos + kProjectileOriginOffsets2_Y[v4] - r24 - layer1_y_pos;
    } else {
      r20 = samus_x_pos + kProjectileOriginOffsets_X[v4] - layer1_x_pos;
      v5 = samus_y_pos + kProjectileOriginOffsets_Y[v4] - r24 - layer1_y_pos;
    }
    r18 = v5;
    if ((v5 & 0xFF00) == 0)
      DrawBeamGrappleSpritemap(r22, r20, r18);
    if ((ceres_status & 0x8000) != 0)
      samus_y_pos = old_y, samus_x_pos = old_x;
  }
}

void ClearFlareAnimationState(void) {  // 0x90BCBE
  flare_animation_frame = 0;
  flare_slow_sparks_anim_frame = 0;
  flare_fast_sparks_anim_frame = 0;
  flare_animation_timer = 0;
  flare_slow_sparks_anim_timer = 0;
  flare_fast_sparks_anim_timer = 0;
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
      projectile_type[r20 >> 1] = -28648;
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

static Func_V *const kCheckBeamCollByDir[10] = {  // 0x90BD64
  CheckBeamCollByDir_0459,
  CheckBeamCollByDir_1368,
  CheckBeamCollByDir_2,
  CheckBeamCollByDir_1368,
  CheckBeamCollByDir_0459,
  CheckBeamCollByDir_0459,
  CheckBeamCollByDir_1368,
  CheckBeamCollByDir_7,
  CheckBeamCollByDir_1368,
  CheckBeamCollByDir_0459,
};

void CheckBeamCollByDir(uint16 k) {
  kCheckBeamCollByDir[projectile_dir[k >> 1] & 0xF]();
}

void CheckBeamCollByDir_0459(void) {  // 0x90BD86
  BlockCollNoWaveBeamVert(projectile_index);
}

void CheckBeamCollByDir_1368(void) {  // 0x90BD8E
  uint16 v0 = projectile_index;
  if (!(BlockCollNoWaveBeamHoriz(projectile_index) & 1))
    BlockCollNoWaveBeamVert(v0);
}

void CheckBeamCollByDir_2(void) {  // 0x90BD9C
  BlockCollNoWaveBeamHoriz(projectile_index);
}

void CheckBeamCollByDir_7(void) {  // 0x90BDA4
  uint16 v0 = projectile_index;
  projectile_bomb_x_speed[projectile_index >> 1] = -1;
  BlockCollNoWaveBeamHoriz(v0);
}

static Func_V *const kWaveBeam_CheckColl_Funcs[10] = {  // 0x90BDB2
  WaveBeam_CheckColl_0459,
  WaveBeam_CheckColl_1368,
  WaveBeam_CheckColl_2,
  WaveBeam_CheckColl_1368,
  WaveBeam_CheckColl_0459,
  WaveBeam_CheckColl_0459,
  WaveBeam_CheckColl_1368,
  WaveBeam_CheckColl_7,
  WaveBeam_CheckColl_1368,
  WaveBeam_CheckColl_0459,
};

void WaveBeam_CheckColl(uint16 k) {
  kWaveBeam_CheckColl_Funcs[projectile_dir[k >> 1] & 0xF]();
}

void WaveBeam_CheckColl_0459(void) {  // 0x90BDD4
  BlockCollWaveBeamVert(projectile_index);
}

void WaveBeam_CheckColl_1368(void) {  // 0x90BDDC
  uint16 v0 = projectile_index;
  if (!(BlockCollWaveBeamHoriz(projectile_index) & 1))
    BlockCollWaveBeamVert(v0);
}

void WaveBeam_CheckColl_2(void) {  // 0x90BDEA
  BlockCollWaveBeamHoriz(projectile_index);
}

void WaveBeam_CheckColl_7(void) {  // 0x90BDF2
  uint16 v0 = projectile_index;
  projectile_bomb_x_speed[projectile_index >> 1] = -1;
  BlockCollWaveBeamHoriz(v0);
}

void ProjectileReflection(uint16 r20) {  // 0x90BE00
  uint16 v0 = r20;
  int v1 = r20 >> 1;
  uint16 v2 = projectile_type[v1];
  if ((v2 & 0x100) != 0) {
    InitializeProjectile(r20);
    projectile_bomb_pre_instructions[v1] = FUNC16(ProjPreInstr_Missile);
    projectile_variables[v1] = 240;
  } else if ((v2 & 0x200) != 0) {
    uint16 k = r20;
    ClearProjectile(LOBYTE(projectile_variables[v1]));
    InitializeProjectile(k);
    int v3 = k >> 1;
    projectile_bomb_pre_instructions[v3] = FUNC16(ProjPreInstr_SuperMissile);
    projectile_variables[v3] = 240;
  } else {
    SetInitialProjectileSpeed(r20);
    InitializeProjectile(v0);
    projectile_bomb_pre_instructions[v1] = kFireChargedBeam_Funcs[projectile_type[v1] & 0xF];
  }
}

void HudSelectionHandler_MissilesOrSuperMissiles(void) {  // 0x90BE62
  if ((button_config_shoot_x & joypad1_newkeys) == 0 && (button_config_shoot_x & joypad1_newinput_samusfilter) == 0
      || !(Samus_CanFireSuperMissile() & 1)) {
    return;
  }
  if (hud_item_index != 2) {
    if (samus_missiles)
      goto LABEL_10;
LABEL_5:
    --projectile_counter;
    return;
  }
  if (!samus_super_missiles)
    goto LABEL_5;
LABEL_10:
  ;
  uint16 v0 = 0;
  while (projectile_damage[v0 >> 1]) {
    v0 += 2;
    if ((int16)(v0 - 10) >= 0)
      goto LABEL_5;
  }
  uint16 r20 = v0;
  if (!(InitProjectilePositionDirection(r20) & 1)) {
    projectile_invincibility_timer = 20;
    if (hud_item_index == 2)
      --samus_super_missiles;
    else
      --samus_missiles;
    int v1 = r20 >> 1;
    projectile_timers[v1] = 4;
    uint16 v3 = hud_item_index;
    uint16 r18 = swap16(hud_item_index);
    projectile_type[v1] |= r18 | 0x8000;
    uint16 v4 = 2 * (v3 & 0xF);
    if (!cinematic_function)
      QueueSfx1_Max6(kNonBeamProjectile_Sfx[v4 >> 1]);
    InitializeProjectileSpeedOfType(r20);
    InitializeProjectile(r20);
    uint16 v7 = projectile_type[v1];
    if ((v7 & 0x200) != 0)
      projectile_bomb_pre_instructions[v1] = FUNC16(ProjPreInstr_SuperMissile);
    else
      projectile_bomb_pre_instructions[v1] = FUNC16(ProjPreInstr_Missile);
    cooldown_timer = kNonBeamProjectileCooldowns[HIBYTE(v7) & 0xF];
    if (samus_auto_cancel_hud_item_index) {
      hud_item_index = 0;
      samus_auto_cancel_hud_item_index = 0;
      return;
    }
    if (hud_item_index == 2) {
      if (samus_super_missiles)
        return;
    } else if (samus_missiles) {
      return;
    }
    hud_item_index = 0;
  }
}

void Missile_Func2(void) {  // 0x90BF46
  uint16 v0 = 0;
  while (projectile_damage[v0 >> 1]) {
    v0 += 2;
    if ((int16)(v0 - 10) >= 0)
      return;
  }
  uint16 r20 = v0;
  int v1 = v0 >> 1;
  projectile_type[v1] |= 0x8200;
  int v2 = projectile_index >> 1;
  projectile_x_pos[v1] = projectile_x_pos[v2];
  projectile_y_pos[v1] = projectile_y_pos[v2];
  projectile_dir[v1] = projectile_dir[v2];
  InitProjectilePositionDirection(r20);
  InitializeInstrForSuperMissile(v0);
  projectile_bomb_pre_instructions[v1] = FUNC16(ProjPreInstr_Func1);
  projectile_variables[projectile_index >> 1] = v0 + (projectile_variables[projectile_index >> 1] & 0xFF00);
  ++projectile_counter;
}

void HudSelectionHandler_MorphBall(void) {  // 0x90BF9D
  if ((button_config_shoot_x & joypad1_lastkeys) != 0) {
    if (hud_item_index == 3) {
      if ((power_bomb_flag & 0x8000) == 0) {
        if (HudSelectionHandler_MorphBall_Helper2() & 1) {
          if (samus_power_bombs) {
            if ((--samus_power_bombs & 0x8000) == 0) {
              power_bomb_flag = -1;
              uint16 v2 = 10;
              while (projectile_type[v2 >> 1]) {
                v2 += 2;
                if ((int16)(v2 - 20) >= 0) {
                  v2 -= 2;
                  break;
                }
              }
              uint16 r18 = swap16(hud_item_index);
              int v4 = v2 >> 1;
              uint16 v5 = r18 | projectile_type[v4];
              projectile_type[v4] = v5;
              uint8 v6 = HIBYTE(v5);
              projectile_dir[v4] = 0;
              projectile_x_pos[v4] = samus_x_pos;
              projectile_y_pos[v4] = samus_y_pos;
              projectile_variables[v4] = 60;
              InitializeInstrForMissile(v2);
              projectile_bomb_pre_instructions[v4] = FUNC16(ProjPreInstr_PowerBomb);
              cooldown_timer = kNonBeamProjectileCooldowns[v6 & 0xF];
              if (samus_auto_cancel_hud_item_index) {
                hud_item_index = 0;
                samus_auto_cancel_hud_item_index = 0;
              } else if (hud_item_index == 3 && !samus_power_bombs) {
                hud_item_index = 0;
              }
            }
          }
        }
      }
    } else if (HudSelectionHandler_MorphBall_Helper() & 1) {
      uint16 v0 = 10;
      while (projectile_type[v0 >> 1]) {
        v0 += 2;
        if ((int16)(v0 - 20) >= 0) {
          v0 -= 2;
          break;
        }
      }
      int v1 = v0 >> 1;
      projectile_type[v1] = 1280;
      projectile_dir[v1] = 0;
      projectile_x_pos[v1] = samus_x_pos;
      projectile_y_pos[v1] = samus_y_pos;
      projectile_variables[v1] = 60;
      InitializeInstrForMissile(v0);
      projectile_bomb_pre_instructions[v1] = FUNC16(ProjPreInstr_Bomb);
      cooldown_timer = kNonBeamProjectileCooldowns[5];
    }
  } else if (flare_counter) {
    QueueSfx1_Max9(2);
    flare_counter = 0;
    ClearFlareAnimationState();
    Samus_LoadSuitPalette();
  }
}

uint8 HudSelectionHandler_MorphBall_Helper(void) {  // 0x90C0AB
  if ((equipped_items & 0x1000) != 0) {
    if (sign16(flare_counter - 60) || bomb_counter)
      return HudSelectionHandler_MorphBall_Helper2();
    if ((joypad1_lastkeys & kButton_Down) != 0 && sign16((bomb_spread_charge_timeout_counter & 0xC0) - 192)) {
      ++bomb_spread_charge_timeout_counter;
    } else {
      BombSpread();
      Samus_LoadSuitPalette();
      QueueSfx1_Max9(2);
    }
  }
  return 0;
}

uint8 HudSelectionHandler_MorphBall_Helper2(void) {  // 0x90C0E7
  if ((button_config_shoot_x & joypad1_newkeys) == 0
      || bomb_counter && (!sign16(bomb_counter - 5) || (uint8)cooldown_timer)) {
    if (flare_counter) {
      QueueSfx1_Max9(2);
      flare_counter = 0;
      ClearFlareAnimationState();
      Samus_LoadSuitPalette();
    }
    return 0;
  } else {
    ++cooldown_timer;
    ++bomb_counter;
    return 1;
  }
}

void Bomb_Func2(void) {  // 0x90C128
  uint16 v0 = projectile_index;
  int v1 = projectile_index >> 1;
  uint16 v2 = projectile_variables[v1];
  if (v2) {
    uint16 v3 = v2 - 1;
    projectile_variables[v1] = v3;
    if (v3) {
      if (v3 == 15)
        projectile_bomb_instruction_ptr[v1] += 28;
    } else {
      QueueSfx2_Max6(8);
      InitializeBombExplosion(v0);
    }
  }
}

void PowerBomb_Func3(void) {  // 0x90C157
  uint16 v0 = projectile_index;
  int v1 = projectile_index >> 1;
  uint16 v2 = projectile_variables[v1];
  if (v2) {
    uint16 v3 = v2 - 1;
    projectile_variables[v1] = v3;
    if (v3) {
      if (v3 == 15)
        projectile_bomb_instruction_ptr[v1] += 28;
    } else {
      power_bomb_explosion_x_pos = projectile_x_pos[v1];
      power_bomb_explosion_y_pos = projectile_y_pos[v1];
      EnableHdmaObjects();
      SpawnPowerBombExplosion();
      projectile_variables[v0 >> 1] = -1;
    }
  } else if (!power_bomb_flag) {
    ClearProjectile(projectile_index);
  }
}

static Func_U8 *const kRunSwitchedToHudHandler[6] = {  // 0x90C4B5
  SwitchToHudHandler_Nothing,
  SwitchToHudHandler_Missiles,
  SwitchToHudHandler_SuperMissiles,
  SwitchToHudHandler_PowerBombs,
  SwitchToHudHandler_Grapple,
  SwitchToHudHandler_Xray,
};

void HandleSwitchingHudSelection(void) {
  uint16 v0;
  uint16 r22 = 0;
  uint16 r18 = hud_item_index;
  if ((button_config_itemcancel_y & joypad1_newkeys) != 0) {
    samus_auto_cancel_hud_item_index = 0;
LABEL_5:
    v0 = 0;
    goto LABEL_6;
  }
  r22 = (button_config_itemcancel_y & joypad1_lastkeys) != 0;
  if ((button_config_itemswitch & joypad1_newkeys) == 0)
    goto LABEL_13;
  v0 = hud_item_index + 1;
  if (!sign16(hud_item_index - 5))
    goto LABEL_5;
LABEL_6:
  hud_item_index = v0;
  while (kRunSwitchedToHudHandler[v0]() & 1) {
    v0 = hud_item_index + 1;
    hud_item_index = v0;
    if (!sign16(v0 - 6)) {
      v0 = 0;
      hud_item_index = 0;
    }
  }
  if (r22)
    samus_auto_cancel_hud_item_index = hud_item_index;
  else
    samus_auto_cancel_hud_item_index = 0;
LABEL_13:
  if (hud_item_index == r18) {
    uint16 v1 = hud_item_changed_this_frame + 1;
    if (!sign16(hud_item_changed_this_frame - 2))
      v1 = 2;
    hud_item_changed_this_frame = v1;
  } else {
    hud_item_changed_this_frame = 1;
  }
}

uint8 SwitchToHudHandler_Nothing(void) {  // 0x90C545
  flare_counter = 0;
  ClearFlareAnimationState();
  Samus_LoadSuitPalette();
  return 0;
}

uint8 SwitchToHudHandler_Missiles(void) {  // 0x90C551
  if (!samus_missiles)
    return 1;
  flare_counter = 0;
  ClearFlareAnimationState();
  Samus_LoadSuitPalette();
  return 0;
}

uint8 SwitchToHudHandler_SuperMissiles(void) {  // 0x90C564
  if (!samus_super_missiles)
    return 1;
  flare_counter = 0;
  ClearFlareAnimationState();
  Samus_LoadSuitPalette();
  return 0;
}

uint8 SwitchToHudHandler_PowerBombs(void) {  // 0x90C577
  if (!samus_power_bombs)
    return 1;
  flare_counter = 0;
  ClearFlareAnimationState();
  Samus_LoadSuitPalette();
  return 0;
}

uint8 SwitchToHudHandler_Grapple(void) {  // 0x90C58A
  if ((equipped_items & 0x4000) == 0)
    return 1;
  if (grapple_beam_function == FUNC16(GrappleBeamFunc_Inactive)) {
    Samus_LoadSuitPalette();
    flare_counter = 0;
    ClearFlareAnimationState();
    grapple_beam_function = FUNC16(GrappleBeamFunc_Inactive);
  }
  return 0;
}

uint8 SwitchToHudHandler_Xray(void) {  // 0x90C5AE
  if ((equipped_items & 0x8000) == 0)
    return 1;
  flare_counter = 0;
  ClearFlareAnimationState();
  Samus_LoadSuitPalette();
  return 0;
}

static uint16 Projectile_SinLookup_Inner(uint16 k, uint16 r24) {  // 0x90CC8A
  uint16 prod = Mult8x8(*((uint8 *)&kSinCosTable8bit_Sext[64] + k), r24);
  uint16 r18 = prod >> 8;
  prod = Mult8x8(*((uint8 *)&kSinCosTable8bit_Sext[64] + k + 1), r24);
  return r18 + prod;
}

Point16U Projectile_SinLookup(uint16 j, uint16 a) {  // 0x90CC39
  uint16 v2, v4;
  uint16 r24 = a;
  uint16 r26 = j;
  if (sign16(j - 128))
    v2 = Projectile_SinLookup_Inner(2 * j, r24);
  else
    v2 = -Projectile_SinLookup_Inner(2 * (uint8)(j + 0x80), r24);
  int16 v3 = (uint8)(r26 - 64);
  if (sign16(v3 - 128))
    v4 = Projectile_SinLookup_Inner(2 * v3, r24);
  else
    v4 = -Projectile_SinLookup_Inner(2 * (uint8)(v3 + 0x80), r24);
  return (Point16U) { v2, v4 };
}

static const uint16 kCostOfSbaInPowerBombs[12] = {  // 0x90CCC0
  0, 1, 1, 0, 1, 0, 0, 0,
  1, 0, 0, 0,
};

static Func_V_A *const kFireSbaFuncs[12] = {
  FireSba_ClearCarry,
  FireSba_FireWave,
  FireSba_FireIce,
  FireSba_ClearCarry,
  FireSba_FireSpazer,
  FireSba_ClearCarry,
  FireSba_ClearCarry,
  FireSba_ClearCarry,
  FireSba_FirePlasma,
  FireSba_ClearCarry,
  FireSba_ClearCarry,
  FireSba_ClearCarry,
};

uint8 FireSba(void) {
  int16 v2;

  if (hud_item_index != 3)
    return 0;
  uint16 v1 = 2 * (equipped_beams & 0xF);
  v2 = samus_power_bombs - kCostOfSbaInPowerBombs[v1 >> 1];
  if (v2 < 0)
    v2 = 0;
  samus_power_bombs = v2;
  uint8 rv = kFireSbaFuncs[v1 >> 1]();
  if (!samus_power_bombs) {
    hud_item_index = 0;
    samus_auto_cancel_hud_item_index = 0;
  }
  return rv;
}

uint8 FireSba_ClearCarry(void) {  // 0x90CD18
  return 0;
}

uint8 FireSba_FireWave(void) {  // 0x90CD1A
  static const int16 kFireSba_FireWave_X[4] = { 128, 128, -128, -128 };
  static const int16 kFireSba_FireWave_Y[4] = { 128, -128, -128, 128 };

  for (int i = 6; i >= 0; i -= 2) {
    int v1 = i >> 1;
    projectile_timers[v1] = 4;
    projectile_type[v1] = equipped_beams & 0x100F | 0x8010;
    projectile_dir[v1] = 0;
    projectile_bomb_pre_instructions[v1] = FUNC16(ProjPreInstr_WaveSba);
    projectile_bomb_y_speed[v1] = 600;
    projectile_bomb_x_speed[v1] = 0;
    projectile_variables[v1] = 0;
    projectile_bomb_x_subpos[v1] = 0;
    projectile_bomb_y_subpos[v1] = 0;
    projectile_x_pos[v1] = kFireSba_FireWave_X[v1] + samus_x_pos;
    projectile_y_pos[v1] = kFireSba_FireWave_Y[v1] + samus_y_pos;
    InitializeSbaProjectile(i);
  }
  projectile_counter = 4;
  cooldown_timer = kProjectileCooldown_Uncharged[projectile_type[0] & 0x3F];
  used_for_sba_attacksB60 = 4;
  QueueSfx1_Max6(0x28);
  return 1;
}

uint8 FireSba_FireIce(void) {  // 0x90CD9B
  static const uint16 kIcePlasmaSbaProjectileOriginAngles[8] = { 0, 0x40, 0x80, 0xc0, 0x20, 0x60, 0xa0, 0xe0 };

  if (projectile_bomb_pre_instructions[0] == FUNC16(ProjPreInstr_IceSba2)
      || projectile_bomb_pre_instructions[0] == FUNC16(ProjPreInstr_IceSba)) {
    return 0;
  }
  for (int i = 6; i >= 0; i -= 2) {
    int v2 = i >> 1;
    projectile_timers[v2] = 4;
    projectile_type[v2] = equipped_beams & 0x100F | 0x8010;
    projectile_dir[v2] = 0;
    projectile_bomb_pre_instructions[v2] = FUNC16(ProjPreInstr_IceSba);
    projectile_variables[v2] = kIcePlasmaSbaProjectileOriginAngles[v2];
    projectile_bomb_y_speed[v2] = 600;
    InitializeProjectile(i);
  }
  projectile_counter = 4;
  cooldown_timer = kProjectileCooldown_Uncharged[projectile_type[0] & 0x3F];
  if (samus_pose_x_dir == 4)
    used_for_sba_attacksB60 = -4;
  else
    used_for_sba_attacksB60 = 4;
  QueueSfx1_Max6(0x23);
  return 1;
}

uint8 FireSba_FireSpazer(void) {  // 0x90CE14
  static const int16 kFireSpazer_Timer[4] = { 0, 0, 4, 4 };
  static const int16 kFireSpazer_Yspeed[4] = { 4, -4, 4, -4 };

  for (int i = 6; i >= 0; i -= 2) {
    int v1 = i >> 1;
    projectile_timers[v1] = kFireSpazer_Timer[v1];
    projectile_dir[v1] = 5;
    projectile_bomb_pre_instructions[v1] = FUNC16(ProjPreInstr_SpazerSba);
    projectile_bomb_x_speed[v1] = 40;
    projectile_bomb_y_speed[v1] = kFireSpazer_Yspeed[v1];
    projectile_variables[v1] = 0;
    projectile_unk_A[v1] = 0;
    projectile_bomb_x_subpos[v1] = 0;
    projectile_bomb_y_subpos[v1] = 0;
    if ((int16)(i - 4) >= 0) {
      projectile_type[i >> 1] = -32732;
      InitializeShinesparkEchoOrSpazerSba(i);
    } else {
      projectile_type[i >> 1] = equipped_beams & 0x100F | 0x8010;
      InitializeSbaProjectile(i);
    }
  }
  projectile_counter = 4;
  cooldown_timer = kProjectileCooldown_Uncharged[projectile_type[0] & 0x3F];
  used_for_sba_attacksB60 = 0;
  QueueSfx1_Max6(0x25);
  return 1;
}

uint8 FireSba_FirePlasma(void) {  // 0x90CE98
  static const uint16 kIcePlasmaSbaProjectileOriginAngles[8] = { 0, 0x40, 0x80, 0xc0, 0x20, 0x60, 0xa0, 0xe0 };

  if (projectile_bomb_pre_instructions[0] == FUNC16(ProjPreInstr_PlasmaSba))
    return 0;
  for (int i = 6; i >= 0; i -= 2) {
    int v2 = i >> 1;
    projectile_type[v2] = equipped_beams & 0x100F | 0x8010;
    projectile_dir[v2] = 0;
    projectile_bomb_pre_instructions[v2] = FUNC16(ProjPreInstr_PlasmaSba);
    projectile_variables[v2] = kIcePlasmaSbaProjectileOriginAngles[v2];
    projectile_bomb_x_speed[v2] = 40;
    projectile_bomb_y_speed[v2] = 0;
    InitializeSbaProjectile(i);
  }
  projectile_counter = 4;
  cooldown_timer = kProjectileCooldown_Uncharged[projectile_type[0] & 0x3F];
  if (samus_pose_x_dir == 4)
    used_for_sba_attacksB60 = -4;
  else
    used_for_sba_attacksB60 = 4;
  QueueSfx1_Max6(0x27);
  return 1;
}

void ProjPreInstr_IceSba(uint16 k) {  // 0x90CF09
  int v1 = k >> 1;
  if ((projectile_dir[v1] & 0xF0) != 0) {
    QueueSfx1_Max6(0x24);
    ClearProjectile(k);
  } else {
    bool v2 = projectile_timers[v1]-- == 1;
    if (v2) {
      projectile_timers[v1] = 4;
      SpawnProjectileTrail(k);
      k = projectile_index;
    }
    int v3 = k >> 1;
    Point16U pt = Projectile_SinLookup(projectile_variables[v3], 0x20);
    projectile_x_pos[v3] = pt.x + samus_x_pos;
    projectile_y_pos[v3] = pt.y + samus_y_pos;
    projectile_variables[v3] = (uint8)(used_for_sba_attacksB60 + projectile_variables[v3]);
    v2 = projectile_bomb_y_speed[v3]-- == 1;
    if (v2) {
      projectile_bomb_pre_instructions[v3] = FUNC16(ProjPreInstr_IceSba2);
      projectile_bomb_x_speed[v3] = 40;
      QueueSfx1_Max6(0x24);
    }
    cooldown_timer = 2;
    flare_counter = 0;
  }
}

void ProjPreInstr_IceSba2(uint16 k) {  // 0x90CF7A
  int16 v5;
  int16 v6;
  int16 v7;

  int v1 = k >> 1;
  if ((projectile_dir[v1] & 0xF0) != 0)
    goto LABEL_9;
  if (projectile_timers[v1]-- == 1) {
    projectile_timers[v1] = 4;
    SpawnProjectileTrail(k);
    k = projectile_index;
  }
  int v3 = k >> 1;
  Point16U pt = Projectile_SinLookup(projectile_variables[v3], projectile_bomb_x_speed[v3]);
  pt.x += samus_x_pos;
  projectile_x_pos[v3] = pt.x;
  v5 = pt.x - layer1_x_pos;
  if (sign16(v5 + 32)
      || !sign16(v5 - 288)
      || (v6 = pt.y + samus_y_pos, projectile_y_pos[v3] = pt.y + samus_y_pos, v7 = v6 - layer1_y_pos, sign16(v7 - 16))
      || !sign16(v7 - 256)) {
LABEL_9:
    ClearProjectile(k);
  } else {
    projectile_variables[v3] = (uint8)(used_for_sba_attacksB60 + projectile_variables[v3]);
    projectile_bomb_x_speed[v3] = (uint8)(projectile_bomb_x_speed[v3] + 8);
    cooldown_timer = 2;
    flare_counter = 0;
  }
}

void ProjPreInstr_PlasmaSba(uint16 k) {  // 0x90D793
  int16 v3;

  int v1 = k >> 1;
  if ((projectile_dir[v1] & 0xF0) != 0) {
    ClearProjectile(k);
  } else {
    cooldown_timer = 2;
    flare_counter = 0;
    Point16U pt = Projectile_SinLookup(projectile_variables[v1], projectile_bomb_x_speed[v1]);
    projectile_x_pos[v1] = pt.x + samus_x_pos;
    projectile_y_pos[v1] = pt.y + samus_y_pos;
    projectile_variables[v1] = (uint8)(used_for_sba_attacksB60 + projectile_variables[v1]);
    v3 = 2 * projectile_bomb_y_speed[v1];
    if (v3) {
      if (v3 == 2) {
        ProjPreInstr_PlasmaSbaFunc_1(k);
      } else {
        if (v3 != 4) {
          Unreachable();
          while (1)
            ;
        }
        ProjPreInstr_PlasmaSbaFunc_2(k);
      }
    } else {
      ProjPreInstr_PlasmaSbaFunc_0(k);
    }
  }
}

void ProjPreInstr_PlasmaSbaFunc_0(uint16 j) {  // 0x90D7E1
  int v1 = j >> 1;
  uint16 v2 = (uint8)(projectile_bomb_x_speed[v1] + 4);
  projectile_bomb_x_speed[v1] = v2;
  if (!sign16(v2 - 192))
    projectile_bomb_y_speed[v1] = 1;
}

void ProjPreInstr_PlasmaSbaFunc_1(uint16 j) {  // 0x90D7FA
  int v1 = j >> 1;
  uint16 v2 = (uint8)(projectile_bomb_x_speed[v1] - 4);
  projectile_bomb_x_speed[v1] = v2;
  if (sign16(v2 - 45))
    projectile_bomb_y_speed[v1] = 2;
}

void ProjPreInstr_PlasmaSbaFunc_2(uint16 j) {  // 0x90D813
  int v1 = j >> 1;
  if (sign16(projectile_x_pos[v1] - layer1_x_pos + 32)
      || !sign16(projectile_x_pos[v1] - layer1_x_pos - 288)
      || sign16(projectile_y_pos[v1] - layer1_y_pos - 16)
      || !sign16(projectile_y_pos[v1] - layer1_y_pos - 256)) {
    ClearProjectile(j);
  } else {
    projectile_bomb_x_speed[v1] = (uint8)(projectile_bomb_x_speed[v1] + 4);
  }
}

static const uint16 kBombSpread_Tab0[5] = { 0x78, 0x6e, 0x64, 0x6e, 0x78 };
static const uint16 kBombSpread_Tab1[5] = { 0x8100, 0x8080, 0, 0x80, 0x100 };
static const uint16 kBombSpread_Tab2[5] = { 0, 1, 2, 1, 0 };
static const uint16 kBombSpread_Tab3[5] = { 0, 0, 0x8000, 0, 0 };

void ProjPreInstr_SpreadBomb(uint16 k) {  // 0x90D8F7
  int v1 = k >> 1;
  if ((projectile_dir[v1] & 0xF0) != 0) {
    ClearProjectile(k);
    return;
  }
  Bomb_Func2();
  if (projectile_variables[v1]) {
    AddToHiLo(&projectile_bomb_y_speed[v1], &projectile_timers[v1], __PAIR32__(samus_y_accel, samus_y_subaccel));
    AddToHiLo(&projectile_y_pos[v1], &projectile_bomb_y_subpos[v1], __PAIR32__(projectile_bomb_y_speed[v1], projectile_timers[v1]));
    if (BlockCollSpreadBomb(k) & 1) {
      AddToHiLo(&projectile_y_pos[v1], &projectile_bomb_y_subpos[v1], -IPAIR32(projectile_bomb_y_speed[v1], projectile_timers[v1]));
      if ((projectile_bomb_y_speed[v1] & 0x8000) != 0) {
        projectile_bomb_y_speed[v1] = 0;
        projectile_y_radius[v1] = 0;
      } else {
        projectile_timers[v1] = kBombSpread_Tab3[v1 - 5];
        projectile_bomb_y_speed[v1] = projectile_unk_A[v1];
      }
      return;
    }
    uint16 t = projectile_bomb_x_speed[v1];
    if (t & 0x8000)
      AddToHiLo(&projectile_x_pos[v1], &projectile_bomb_x_subpos[v1], -INT16_SHL8(t & 0x7fff));
    else
      AddToHiLo(&projectile_x_pos[v1], &projectile_bomb_x_subpos[v1], INT16_SHL8(t));
  }
  if (BlockCollSpreadBomb(k) & 1) {
    uint16 t = projectile_bomb_x_speed[v1];
    projectile_bomb_x_speed[v1] ^= 0x8000;
    if (!(t & 0x8000))
      AddToHiLo(&projectile_x_pos[v1], &projectile_bomb_x_subpos[v1], -INT16_SHL8(t));
    else
      AddToHiLo(&projectile_x_pos[v1], &projectile_bomb_x_subpos[v1], INT16_SHL8(t & 0x7fff));
  }
}

void ProjPreInstr_WaveSba(uint16 k) {  // 0x90DA08
  int v1 = k >> 1;
  bool v2, v3;
  if ((projectile_dir[v1] & 0xF0) != 0
      || (v2 = projectile_bomb_y_speed[v1] == 1,
          v3 = (int16)(projectile_bomb_y_speed[v1] - 1) < 0,
          --projectile_bomb_y_speed[v1],
          v2)
      || v3) {
    QueueSfx1_Max6(0x29);
    ClearProjectile(k);
    return;
  }
  uint16 R34 = projectile_bomb_x_speed[v1];
  v2 = projectile_timers[v1]-- == 1;
  if (v2) {
    projectile_timers[v1] = 4;
    SpawnProjectileTrail(k);
    k = projectile_index;
  }
  if ((int16)(samus_x_pos - projectile_x_pos[v1]) < 0) {
    if (!sign16(projectile_bomb_x_speed[v1] + 2047))
      projectile_bomb_x_speed[v1] -= 64;
  } else if (sign16(projectile_bomb_x_speed[v1] - 2048)) {
    projectile_bomb_x_speed[v1] += 64;
  }
  AddToHiLo(&projectile_x_pos[v1], &projectile_bomb_x_subpos[v1], INT16_SHL8(projectile_bomb_x_speed[v1]));

  if ((int16)(samus_y_pos - projectile_y_pos[v1]) < 0) {
    if (!sign16(projectile_variables[v1] + 2047))
      projectile_variables[v1] -= 64;
  } else if (sign16(projectile_variables[v1] - 2048)) {
    projectile_variables[v1] += 64;
  }
  AddToHiLo(&projectile_y_pos[v1], &projectile_bomb_y_subpos[v1], INT16_SHL8(projectile_variables[v1]));
  if (k == 6) {
    if ((projectile_bomb_x_speed[3] & 0x8000) != 0) {
      if ((R34 & 0x8000) == 0)
        QueueSfx1_Max6(0x28);
    } else if ((R34 & 0x8000) != 0) {
      QueueSfx1_Max6(0x28);
    }
  }
  cooldown_timer = 2;
  flare_counter = 0;
}

void BombSpread(void) {  // 0x90D849
  if (bomb_functions[0] != FUNC16(ProjPreInstr_SpreadBomb)) {
    uint16 v0 = 10;
    do {
      int v1 = v0 >> 1;
      projectile_type[v1] = -31488;
      projectile_dir[v1] = 0;
      projectile_bomb_pre_instructions[v1] = FUNC16(ProjPreInstr_SpreadBomb);
      InitializeInstrForMissile(v0);
      projectile_x_pos[v1] = samus_x_pos;
      projectile_bomb_x_subpos[v1] = 0;
      projectile_y_pos[v1] = samus_y_pos;
      projectile_bomb_y_subpos[v1] = 0;
      int v2 = (uint16)(v0 - 10) >> 1;
      projectile_bomb_x_speed[v1] = kBombSpread_Tab1[v2];
      projectile_timers[v1] = kBombSpread_Tab3[v2];
      uint16 v3 = -(int16)(kBombSpread_Tab2[v2] + ((bomb_spread_charge_timeout_counter >> 6) & 3));
      projectile_bomb_y_speed[v1] = v3;
      projectile_unk_A[v1] = v3;
      projectile_variables[v1] = kBombSpread_Tab0[v2];
      v0 += 2;
    } while ((int16)(v0 - 20) < 0);
    bomb_counter = 5;
    cooldown_timer = kNonBeamProjectileCooldowns[HIBYTE(projectile_type[5]) & 0xF];
    bomb_spread_charge_timeout_counter = 0;
    flare_counter = 0;
  }
}

typedef void Func_SpazerSba_V(uint16 j, uint16 r22);

static Func_SpazerSba_V *const kProjPreInstr_SpazerSba_FuncsB[3] = {  // 0x90DB06
  ProjPreInstr_SpazerSba_FuncB_0,
  ProjPreInstr_SpazerSba_FuncB_1,
  ProjPreInstr_SpazerSba_FuncB_2,
};

static Func_Y_V *const kProjPreInstr_SpazerSba_FuncsA[4] = {
  ProjPreInstr_SpazerSba_FuncA_0,
  ProjPreInstr_SpazerSba_FuncA_1,
  ProjPreInstr_SpazerSba_FuncA_2,
  ProjPreInstr_SpazerSba_FuncA_3,
};

void ProjPreInstr_SpazerSba(uint16 k) {
  int v1 = k >> 1;
  if ((projectile_dir[v1] & 0xF0) != 0) {
    kProjPreInstr_SpazerSba_FuncsA[v1](k);
  } else {
    int v2 = k >> 1;
    if (projectile_timers[v2]-- == 1) {
      projectile_timers[v2] = 4;
      SpawnProjectileTrail(k);
      k = projectile_index;
    }
    int v4 = k >> 1;
    Point16U pt = Projectile_SinLookup(projectile_variables[v4], projectile_bomb_x_speed[v4]);
    projectile_x_pos[v4] = pt.x + samus_x_pos;
    kProjPreInstr_SpazerSba_FuncsB[projectile_unk_A[v4] >> 1](k, pt.y);
    cooldown_timer = 2;
    flare_counter = 0;
  }
}

void ProjPreInstr_SpazerSba_FuncA_0(uint16 k) {  // 0x90DB57
  ClearProjectile(k);
  ClearProjectile(4);
}

void ProjPreInstr_SpazerSba_FuncA_1(uint16 k) {  // 0x90DB66
  ClearProjectile(k);
  ClearProjectile(6);
}

void ProjPreInstr_SpazerSba_FuncA_2(uint16 k) {  // 0x90DB75
  ClearProjectile(k);
  ClearProjectile(0);
}

void ProjPreInstr_SpazerSba_FuncA_3(uint16 k) {  // 0x90DB84
  ClearProjectile(k);
  ClearProjectile(2);
}

void ProjPreInstr_SpazerSba_FuncB_0(uint16 j, uint16 r22) {  // 0x90DB93
  static const int16 kProjPreInstr_SpazerSba_Yspeed[4] = { 2, -2, 2, -2 };

  int v1 = j >> 1;
  projectile_y_pos[v1] = r22 + samus_y_pos;
  uint16 v2 = (uint8)(LOBYTE(projectile_bomb_y_speed[v1]) + LOBYTE(projectile_variables[v1]));
  projectile_variables[v1] = v2;
  if (v2 == 128) {
    projectile_bomb_x_speed[v1] = 160;
    projectile_bomb_y_speed[v1] = kProjPreInstr_SpazerSba_Yspeed[v1];
    projectile_dir[v1] = 0;
    projectile_unk_A[v1] = 2;
  }
}

void ProjPreInstr_SpazerSba_FuncB_1(uint16 j, uint16 r22) {  // 0x90DBCF
  static const int16 kSpazerProjectileYSpeed[4] = { -2, 2, -2, 2 };

  uint16 v1 = r22 + samus_y_pos - 114;
  int v2 = j >> 1;
  projectile_y_pos[v2] = v1;
  if (sign16(v1 - layer1_y_pos - 16)) {
    FireEndOfSpazerSba(j);
  } else {
    projectile_variables[v2] = (uint8)(LOBYTE(projectile_bomb_y_speed[v2]) + LOBYTE(projectile_variables[v2]));
    uint16 v3 = projectile_bomb_x_speed[v2] - 5;
    projectile_bomb_x_speed[v2] = v3;
    if (!v3) {
      projectile_bomb_y_speed[v2] = kSpazerProjectileYSpeed[v2];
      projectile_variables[v2] = (uint8)(projectile_variables[v2] + 0x80);
      projectile_unk_A[v2] = 4;
      if (!j)
        QueueSfx1_Max6(0x26);
    }
  }
}

void ProjPreInstr_SpazerSba_FuncB_2(uint16 j, uint16 r22) {  // 0x90DC30
  int v2;
  uint16 v3;

  uint16 v1 = r22 + samus_y_pos - 114;
  projectile_y_pos[j >> 1] = v1;
  if (sign16(v1 - layer1_y_pos - 16)
      || (v2 = j >> 1,
          projectile_variables[v2] = (uint8)(LOBYTE(projectile_bomb_y_speed[v2])
                                             + LOBYTE(projectile_variables[v2])),
          v3 = projectile_bomb_x_speed[v2] + 5,
          projectile_bomb_x_speed[v2] = v3,
          !sign16(v3 - 96))) {
    FireEndOfSpazerSba(j);
  }
}

void FireEndOfSpazerSba(uint16 j) {  // 0x90DC67
  static const int16 kFireEndOfSpazerSba[4] = { 16, 16, -16, -16 };

  int v1 = j >> 1;
  projectile_x_pos[v1] += kFireEndOfSpazerSba[v1];
  projectile_dir[v1] = 5;
  projectile_timers[v1] = 4;
  projectile_bomb_pre_instructions[v1] = FUNC16(ProjPreInstr_EndOfSpazerSba);
  if ((int16)(j - 4) < 0) {
    projectile_type[j >> 1] = -32732;
    InitializeShinesparkEchoOrSpazerSba(j);
  }
}

void ProjPreInstr_EndOfSpazerSba(uint16 k) {  // 0x90DC9C
  int v1 = k >> 1;
  if ((projectile_dir[v1] & 0xF0) != 0)
    goto LABEL_2;
  if (projectile_timers[v1]-- == 1) {
    projectile_timers[v1] = 4;
    SpawnProjectileTrail(k);
    k = projectile_index;
  }
  int v3 = k >> 1;
  uint16 v4 = projectile_y_pos[v3] + 8;
  projectile_y_pos[v3] = v4;
  if (!sign16(v4 - layer1_y_pos - 248)) {
LABEL_2:
    ClearProjectile(k);
  } else {
    cooldown_timer = 2;
    flare_counter = 0;
  }
}

static Func_V *kHandleHudSpecificBehaviorAndProjs[28] = {  // 0x90DCDD
  HudSelectionHandler_Normal,
  HudSelectionHandler_Normal,
  HudSelectionHandler_Normal,
  HudSelectionHandler_JumpEtc,
  HudSelectionHandler_MorphBall,
  HudSelectionHandler_Normal,
  HudSelectionHandler_Normal,
  HudSelectionHandler_MorphBall,
  HudSelectionHandler_MorphBall,
  HudSelectionHandler_MorphBall,
  HudSelectionHandler_JumpEtc,
  HudSelectionHandler_Grappling,
  HudSelectionHandler_Grappling,
  HudSelectionHandler_JumpEtc,
  HudSelectionHandler_TurningAround,
  HudSelectionHandler_CrouchEtcTrans,
  HudSelectionHandler_Normal,
  HudSelectionHandler_MorphBall,
  HudSelectionHandler_MorphBall,
  HudSelectionHandler_MorphBall,
  HudSelectionHandler_JumpEtc,
  HudSelectionHandler_Normal,
  HudSelectionHandler_Grappling,
  HudSelectionHandler_TurningAround,
  HudSelectionHandler_TurningAround,
  HudSelectionHandler_JumpEtc,
  HudSelectionHandler_GrabbedByDraygon,
  HudSelectionHandler_JumpEtc,
};

void Samus_HandleHudSpecificBehaviorAndProjs(void) {
  if (samus_pose == kPose_00_FaceF_Powersuit
      || samus_pose == kPose_9B_FaceF_VariaGravitySuit
      || (Samus_HandleCooldown(),
          HandleSwitchingHudSelection(),
          kHandleHudSpecificBehaviorAndProjs[samus_movement_type](),
          !time_is_frozen_flag)) {
    HandleProjectile();
  }
}

void HudSelectionHandler_Normal(void) {  // 0x90DD3D
  static Func_V *const kHudSelectionHandler_Normal[7] = {
    HudSelectionHandler_NothingOrPowerBombs,
    HudSelectionHandler_MissilesOrSuperMissiles,
    HudSelectionHandler_MissilesOrSuperMissiles,
    HudSelectionHandler_NothingOrPowerBombs,
    HudSelectionHandler_Grappling,
    HudSelectionHandler_Xray,
    HudSelectionHandler_TurningAround,
  };
  uint16 v0;
  if (grapple_beam_function == FUNC16(GrappleBeamFunc_Inactive)) {
    if (time_is_frozen_flag)
      v0 = 10;
    else
      v0 = 2 * hud_item_index;
  } else {
    v0 = 8;
  }
  kHudSelectionHandler_Normal[v0 >> 1]();
}

void HudSelectionHandler_Grappling(void) {  // 0x90DD6F
  GrappleBeamHandler();
}

void HudSelectionHandler_TurningAround(void) {  // 0x90DD74
  if (new_projectile_direction_changed_pose) {
    HudSelectionHandler_Normal();
  } else if (grapple_beam_function != FUNC16(GrappleBeamFunc_Inactive)) {
    grapple_beam_function = FUNC16(GrappleBeamFunc_Cancel);
  }
}

void HudSelectionHandler_CrouchEtcTrans(void) {  // 0x90DD8C
  static const uint8 byte_90DDAA[12] = { 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1 };
  if (!sign16(samus_pose - kPose_F1_FaceR_CrouchTrans_AimU))
    goto LABEL_4;
  if (!sign16(samus_pose - kPose_DB))
    return;
  if (byte_90DDAA[(uint16)(samus_pose - 53)]) {
    if (grapple_beam_function != FUNC16(GrappleBeamFunc_Inactive)) {
      grapple_beam_function = FUNC16(GrappleBeamFunc_Cancel);
      HudSelectionHandler_Normal();
    }
  } else {
LABEL_4:
    HudSelectionHandler_Normal();
  }
}

void HudSelectionHandler_JumpEtc(void) {  // 0x90DDB6
  if (grapple_beam_function != FUNC16(GrappleBeamFunc_Inactive)) {
    grapple_beam_function = FUNC16(GrappleBeamFunc_Cancel);
    HudSelectionHandler_Normal();
  }
}

void HudSelectionHandler_Xray(void) {  // 0x90DDC8
  if ((button_config_run_b & joypad1_lastkeys) != 0)
    XrayRunHandler();
  else
    HudSelectionHandler_NothingOrPowerBombs();
}

void HudSelectionHandler_GrabbedByDraygon(void) {  // 0x90DDD8
  if (samus_pose == (kPose_DB | kPose_04_FaceL_AimU))
    HudSelectionHandler_MorphBall();
  else
    HudSelectionHandler_Normal();
}

#endif

#ifdef SAMUS_PROJECTILE_CORE_IMPL

void InitializeProjectile(uint16 k) {  // 0x938000
  int v1 = k >> 1;
  int r18 = (projectile_dir[v1] & 0xF);
  uint16 v2 = projectile_type[v1], v3;
  if ((v2 & 0xF00) != 0) {
    v3 = kProjectileData_NonBeams[HIBYTE(v2) & 0xF];
  } else if ((v2 & 0x10) != 0) {
    v3 = kProjectileData_ChargedBeams[projectile_type[v1] & 0xF];
  } else {
    v3 = kProjectileData_UnchargedBeams[projectile_type[v1] & 0xF];
  }
  ProjectileDataTable *PD = get_ProjectileDataTable(v3);
  if (sign16(PD->damage))
    InvalidInterrupt_Crash();
  projectile_damage[v1] = PD->damage;
  uint16 v7 = PD->instr_ptrs[r18];
  projectile_bomb_instruction_ptr[v1] = v7;
  const uint8 *v8 = RomPtr_93(v7);
  projectile_x_radius[v1] = v8[4];
  projectile_y_radius[v1] = v8[5];
  projectile_bomb_instruction_timers[v1] = 1;
}

void InitializeInstrForSuperMissile(uint16 v0) {  // 0x938071
  int v1 = v0 >> 1;
  const uint8 *v3 = RomPtr_93(kRunInstrForSuperMissile[HIBYTE(projectile_type[v1]) & 0xF]);
  uint16 v4 = GET_WORD(v3);
  projectile_damage[v1] = v4;
  if (sign16(v4))
    InvalidInterrupt_Crash();
  projectile_bomb_instruction_ptr[v1] = GET_WORD(v3 + 2);
  projectile_bomb_instruction_timers[v1] = 1;
}

void InitializeInstrForMissile(uint16 v0) {  // 0x9380A0
  int v1 = v0 >> 1;
  const uint8 *v3 = RomPtr_93(kProjectileData_NonBeams[HIBYTE(projectile_type[v1]) & 0xF]);
  uint16 v4 = GET_WORD(v3);
  projectile_damage[v1] = GET_WORD(v3);
  if (sign16(v4))
    InvalidInterrupt_Crash();
  projectile_bomb_instruction_ptr[v1] = GET_WORD(v3 + 2);
  projectile_bomb_instruction_timers[v1] = 1;
}

void KillProjectileInner(uint16 k) {  // 0x9380CF
  int v1 = k >> 1;
  if ((projectile_type[v1] & 0xF00) != 0) {
    if (!cinematic_function)
      QueueSfx2_Max6(7);
    uint16 v2 = projectile_type[v1];
    projectile_type[v1] = v2 & 0xF0FF | 0x800;
    if ((v2 & 0x200) != 0) {
      projectile_bomb_instruction_ptr[v1] = HIWORD(g_stru_938691.damages);
      earthquake_type = 20;
      earthquake_timer = 30;
    } else {
      projectile_bomb_instruction_ptr[v1] = *(&g_stru_938679.instr_ptr + 1);
    }
    if (!sign16(cooldown_timer - 21))
      cooldown_timer = 20;
  } else {
    projectile_type[v1] = projectile_type[v1] & 0xF0FF | 0x700;
    projectile_bomb_instruction_ptr[v1] = HIWORD(g_stru_938679.damages);
    QueueSfx2_Max6(0xC);
  }
  projectile_bomb_instruction_timers[v1] = 1;
  projectile_damage[v1] = 8;
}

void InitializeBombExplosion(uint16 k) {  // 0x93814E
  int v1 = k >> 1;
  projectile_bomb_instruction_ptr[v1] = HIWORD(kProjInstrList_Explosion.damages);
  projectile_bomb_instruction_timers[v1] = 1;
}

void InitializeShinesparkEchoOrSpazerSba(uint16 k) {  // 0x938163
  int v1 = k >> 1;
  int r18 = projectile_dir[v1] & 0xF;
  ProjectileDataTable *PD = get_ProjectileDataTable(kShinesparkEchoSpazer_ProjectileData[LOBYTE(projectile_type[v1]) - 34]);
  projectile_damage[v1] = PD->damage;
  if (sign16(PD->damage))
    InvalidInterrupt_Crash();
  projectile_bomb_instruction_ptr[v1] = PD->instr_ptrs[r18];
  projectile_bomb_instruction_timers[v1] = 1;
}

void InitializeSbaProjectile(uint16 k) {  // 0x9381A4
  int v1 = k >> 1;
  const uint8 *v2 = RomPtr_93(g_off_938413[projectile_type[v1] & 0xF]);
  uint16 v3 = GET_WORD(v2);
  projectile_damage[v1] = v3;
  if (sign16(v3))
    InvalidInterrupt_Crash();
  projectile_bomb_instruction_ptr[v1] = GET_WORD(v2 + 2);
  projectile_bomb_instruction_timers[v1] = 1;
}

uint16 ProjectileInsts_GetValue(uint16 k) {  // 0x9381D1
  int ip = projectile_bomb_instruction_ptr[k >> 1];
  int delta = (projectile_bomb_instruction_timers[k >> 1] == 1 && !sign16(get_ProjectileInstr(ip)->timer)) ? 0 : -8;
  return get_ProjectileInstr(ip + delta)->field_6;
}

uint16 CallProj93Instr(uint32 ea, uint16 j, uint16 k) {
  switch (ea) {
  case fnProj93Instr_Delete: return Proj93Instr_Delete(j, k);
  case fnProj93Instr_Goto: return Proj93Instr_Goto(j, k);
  default: return Unreachable();
  }
}

void RunProjectileInstructions(void) {  // 0x9381E9
  ProjectileInstr *PI;

  uint16 v0 = projectile_index;
  int v1 = projectile_index >> 1;
  if (projectile_bomb_instruction_timers[v1]-- == 1) {
    uint16 v3 = projectile_bomb_instruction_ptr[v1];
    while (1) {
      PI = get_ProjectileInstr(v3);
      if ((PI->timer & 0x8000) == 0)
        break;
      v3 = CallProj93Instr(PI->timer | 0x930000, v0, v3 + 2);
      if (!v3)
        return;
    }
    projectile_bomb_instruction_timers[v1] = PI->timer;
    projectile_spritemap_pointers[v1] = PI->spritemap_ptr;
    projectile_x_radius[v1] = PI->x_radius;
    projectile_y_radius[v1] = PI->y_radius;
    projectile_bomb_instruction_ptr[v1] = v3 + 8;
  }
}

uint16 Proj93Instr_Delete(uint16 k, uint16 j) {  // 0x93822F
  ClearProjectile(k);
  return 0;
}

uint16 Proj93Instr_Goto(uint16 k, uint16 j) {  // 0x938239
  return *(uint16 *)RomPtr_93(j);
}

void DrawPlayerExplosions2(void) {  // 0x938254
  uint16 v0 = 8;
  uint16 r18, r20;
  projectile_index = 8;
  do {
    int v1 = v0 >> 1;
    if (!projectile_bomb_instruction_ptr[v1])
      goto LABEL_25;
    uint16 v2, v3;
    v2 = projectile_type[v1];
    if ((v2 & (kProjectileType_TypeMask | 0x10)) != 0) {
      if (!sign16((v2 & 0xF00) - 768))
        goto LABEL_25;
    } else if ((v2 & 0xC) != 0) {
      if ((v0 & 2) != 0) {
        if ((nmi_frame_counter_word & 2) == 0)
          goto LABEL_25;
      } else if ((nmi_frame_counter_word & 2) != 0) {
        goto LABEL_25;
      }
    } else if ((v0 & 2) != 0) {
      if ((nmi_frame_counter_word & 1) != 0)
        goto LABEL_25;
    } else if ((nmi_frame_counter_word & 1) == 0) {
      goto LABEL_25;
    }
    if ((ceres_status & 0x8000) == 0) {
      r20 = projectile_x_pos[v1] - layer1_x_pos;
      v3 = projectile_y_pos[v1] - layer1_y_pos;
      r18 = v3;
    } else {
      Point16U pt = CalcExplosion_Mode7(v0);
      v3 = pt.y;
      r20 = pt.x;
    }
    if ((v3 & 0xFF00) == 0 && (projectile_spritemap_pointers[v1] & 0x8000) != 0) {
      DrawProjectileSpritemap(v0, r20, r18);
    }
    v0 = projectile_index;
LABEL_25:
    v0 -= 2;
    projectile_index = v0;
  } while ((v0 & 0x8000) == 0);
  Samus_DrawShinesparkCrashEchoProjectiles();
  HandleProjectileTrails();
}

void sub_9382FD(void) {  // 0x9382FD
  uint16 v0 = 8;
  projectile_index = 8;
  do {
    int v1 = v0 >> 1;
    if (projectile_bomb_instruction_ptr[v1]) {
      uint16 r20 = projectile_x_pos[v1] - layer1_x_pos;
      uint16 r18 = projectile_y_pos[v1] - 8 - layer1_y_pos;
      if ((r18 & 0xFF00) != 0) {
      } else if ((projectile_spritemap_pointers[v1] & 0x8000) != 0) {
        DrawProjectileSpritemap(v0, r20, r18);
      }
      v0 = projectile_index;
    }
    v0 -= 2;
    projectile_index = v0;
  } while ((v0 & 0x8000) == 0);
  HandleProjectileTrails();
}

void DrawBombAndProjectileExplosions(void) {  // 0x93834D
  uint16 v0 = 18, v2;
  uint16 r18, r20;
  projectile_index = 18;
  do {
    int v1 = v0 >> 1;
    if (!projectile_bomb_instruction_ptr[v1] || (int16)((projectile_type[v1] & 0xF00) - 768) < 0)
      goto LABEL_16;
    if ((projectile_type[v1] & 0xF00) == 768) {
      if (!projectile_variables[v1])
        goto LABEL_16;
LABEL_9:;
      uint16 v3 = projectile_x_pos[v1] - layer1_x_pos;
      r20 = v3;
      if (!sign16(v3 - 304) || sign16(v3 + 48))
        goto LABEL_16;
      v2 = projectile_y_pos[v1] - layer1_y_pos;
      r18 = v2;
      goto LABEL_12;
    }
    if ((projectile_type[v1] & 0xF00) == 1280 || (ceres_status & 0x8000) == 0)
      goto LABEL_9;
    CalcExplosion_Mode7(v0);
    v2 = r18;
LABEL_12:
    if ((v2 & 0xFF00) != 0)
      ;
    else
      DrawProjectileSpritemap(v0, r20, r18);
    v0 = projectile_index;
LABEL_16:
    v0 -= 2;
    projectile_index = v0;
  } while ((v0 & 0x8000) == 0);
}

#endif
