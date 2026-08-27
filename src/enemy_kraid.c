// Enemy AI - Kraid boss runtime + shared Bank $A7 reaction wrappers — peeled from Bank $A7
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "enemy_types.h"
#include "sm_rtl.h"
#include "enemy_ai_canon.h"

#undef r18

enum {
  kKraidLintDelayTop = 0x120,
  kKraidLintDelayMiddle = 0xa0,
  kKraidLintDelayBottom = 0x40,
  kKraidWalkStepX = 3,
  kKraidFootMoveRightSpeed = 4,
  kKraidLintXSpeed = 3,
  kKraidLintXSubspeed = 0x8000,
  kKraidInstrTimer_MouthOpen = 0x0a,
  kKraidInstrTimer_EyeGlow = 0x05,
  kKraidInstrTimer_Death = 0x19,
  kKraidExplosion_Small = 6,
  kKraidExplosion_SuperMissile = 29,
  kKraidMusic_Boss = 5,
  kKraidMusic_Defeated = 3,
  kSfx2_KraidRoar = 0x2D,
  kSfx2_KraidDyingCry = 0x2E,
  kSfx2_KraidFootstep = 0x76,
  kSfx1_KraidShotExplosion = 0x3D,
  kSfx3_KraidRock = 0x1E,
  kSfx3_KraidLint = 0x1F,
  kProjectileDir_Hit = 0x10,
  kKraidShotCharged = 0x10,
};

typedef struct KraidWalkChoice {
  uint16 target_x;
  uint16 think_timer;
} KraidWalkChoice;
_Static_assert(sizeof(KraidWalkChoice) == 4, "walk choice is 4 bytes");

typedef struct KraidFingernailVel {
  uint16 x_subspeed;
  uint16 x_speed;
  uint16 y_subspeed;
  uint16 y_speed;
} KraidFingernailVel;
_Static_assert(sizeof(KraidFingernailVel) == 8, "fingernail vel is 8 bytes");

/* Leftover: RomPtr_A7 mouth instruction list / hitbox blobs walked via kraid_var_B. */

static const uint16 kKraidPalette2[16] = {
  0x0400, 0x2d6b, 0x2d6b, 0x2d6b, 0x1986, 0x14c2, 0x0840, 0x0400,
  0x31ed, 0x18c6, 0x10c3, 0x0420, 0x0252, 0x2d6b, 0x4a52, 0x0000,
};

static const uint16 kKraidBgTargetPalette3[16] = {
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0df6, 0x0d92, 0x0cec,
  0x0ca9, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

static const uint16 kKraidCeilingRockX[9] = {
  0x0068, 0x00d8, 0x0028, 0x00a8, 0x0058, 0x00c8, 0x0038, 0x00b8, 0x0048,
};

static const uint16 kKraidBodyHitbox[] = {
  0x03ff, 0xffd0, 0x0010, 0xffd0, 0x0000, 0xffe0, 0xffe0, 0xffe8,
  0xffd0, 0xfff8, 0xffb0, 0x0000, 0xff90, 0x0008, 0x8000, 0x0008,
};

static const uint16 kKraidBgHealthPalettes[9][16] = {
  { 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff },
  { 0x0000, 0x559d, 0x1816, 0x100d, 0x127a, 0x0e17, 0x0dd3, 0x0970, 0x092d, 0x04ca, 0x0486, 0x0023, 0x0443, 0x3e52, 0x4ad5, 0x0000 },
  { 0x3800, 0x559d, 0x1816, 0x100d, 0x1a9b, 0x1637, 0x11f3, 0x0d8f, 0x0d4c, 0x08e9, 0x04a6, 0x0043, 0x0443, 0x4273, 0x52f6, 0x0000 },
  { 0x3800, 0x559d, 0x1816, 0x100d, 0x22db, 0x1a77, 0x1a12, 0x15ae, 0x116c, 0x0d09, 0x08a6, 0x0444, 0x0423, 0x4a94, 0x5b38, 0x0000 },
  { 0x3800, 0x559d, 0x1816, 0x100d, 0x2afc, 0x2297, 0x1e32, 0x19cd, 0x158b, 0x1128, 0x08c6, 0x0464, 0x0423, 0x4eb5, 0x6359, 0x0000 },
  { 0x3800, 0x559d, 0x1816, 0x100d, 0x331d, 0x2ab7, 0x2a91, 0x1e0c, 0x19aa, 0x1148, 0x0ce5, 0x0484, 0x0023, 0x52b5, 0x677b, 0x0000 },
  { 0x3800, 0x559d, 0x1816, 0x100d, 0x3b3e, 0x32d7, 0x32b0, 0x222b, 0x1dc9, 0x1567, 0x0d05, 0x04a4, 0x0023, 0x56d6, 0x6f9c, 0x0000 },
  { 0x3800, 0x559d, 0x1816, 0x100d, 0x437e, 0x3717, 0x32b0, 0x2a4a, 0x21e9, 0x1987, 0x1105, 0x08a5, 0x0003, 0x5ef7, 0x77de, 0x0000 },
  { 0x3800, 0x559d, 0x1816, 0x100d, 0x4b9f, 0x3f37, 0x36d0, 0x2e69, 0x2608, 0x1da6, 0x1125, 0x08c5, 0x0003, 0x6318, 0x7fff, 0x0000 },
};

static const uint16 kKraidBgDeathPalette[16] = {
  0x3800, 0x0807, 0x0404, 0x0000, 0x127a, 0x0e17, 0x0dd3, 0x0970,
  0x092d, 0x04ca, 0x0486, 0x0023, 0x0443, 0x3e52, 0x4ad5, 0x0000,
};

static const uint16 kKraidSprHealthPalettes[9][16] = {
  { 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff },
  { 0x0000, 0x559d, 0x1816, 0x100d, 0x127a, 0x0e17, 0x0dd3, 0x0970, 0x092d, 0x04ca, 0x0486, 0x0023, 0x0443, 0x3e52, 0x4ad5, 0x0000 },
  { 0x3800, 0x559d, 0x1816, 0x100d, 0x1a9b, 0x1637, 0x11f3, 0x0d8f, 0x0d4c, 0x08e9, 0x04a6, 0x0043, 0x0443, 0x4273, 0x52f6, 0x0000 },
  { 0x3800, 0x559d, 0x1816, 0x100d, 0x22db, 0x1a77, 0x1a12, 0x15ae, 0x116c, 0x0d09, 0x08a6, 0x0444, 0x0423, 0x4a94, 0x5b38, 0x0000 },
  { 0x3800, 0x559d, 0x1816, 0x100d, 0x2afc, 0x2297, 0x1e32, 0x19cd, 0x158b, 0x1128, 0x08c6, 0x0464, 0x0423, 0x4eb5, 0x6359, 0x0000 },
  { 0x3800, 0x559d, 0x1816, 0x100d, 0x331d, 0x2ab7, 0x2a91, 0x1e0c, 0x19aa, 0x1148, 0x0ce5, 0x0484, 0x0023, 0x52b5, 0x677b, 0x0000 },
  { 0x3800, 0x559d, 0x1816, 0x100d, 0x3b3e, 0x32d7, 0x32b0, 0x222b, 0x1dc9, 0x1567, 0x0d05, 0x04a4, 0x0023, 0x56d6, 0x6f9c, 0x0000 },
  { 0x3800, 0x559d, 0x1816, 0x100d, 0x437e, 0x3717, 0x32b0, 0x2a4a, 0x21e9, 0x1987, 0x1105, 0x08a5, 0x0003, 0x5ef7, 0x77de, 0x0000 },
  { 0x3800, 0x559d, 0x1816, 0x100d, 0x4b9f, 0x3f37, 0x36d0, 0x2e69, 0x2608, 0x1da6, 0x1125, 0x08c5, 0x0003, 0x6318, 0x7fff, 0x0000 },
};

static const Hitbox kKraidLintHitbox = {
  .left = 0xffe8, .top = 0xfffc, .right = 0x0004, .bottom = 0x0006,
};

static const KraidWalkChoice kKraidWalkFromX_F0[5] = {
  { 0x0180, 0x0158 }, { 0x0180, 0x0158 }, { 0x00d0, 0x002c }, { 0x0170, 0x002c }, { 0x0170, 0x002c },
};
static const KraidWalkChoice kKraidWalkFromX_160[5] = {
  { 0x00f0, 0x0100 }, { 0x0180, 0x0158 }, { 0x00d0, 0x002c }, { 0x0140, 0x002c }, { 0x0170, 0x002c },
};
static const KraidWalkChoice kKraidWalkFromX_180[5] = {
  { 0x00f0, 0x0100 }, { 0x0160, 0x0158 }, { 0x00d0, 0x002c }, { 0x0140, 0x002c }, { 0x0170, 0x002c },
};
static const KraidWalkChoice kKraidWalkFromX_D0[5] = {
  { 0x00f0, 0x0100 }, { 0x0160, 0x0158 }, { 0x0180, 0x0158 }, { 0x0140, 0x002c }, { 0x0170, 0x002c },
};
static const KraidWalkChoice kKraidWalkFromX_140[5] = {
  { 0x00f0, 0x0100 }, { 0x0160, 0x0158 }, { 0x0180, 0x0158 }, { 0x00d0, 0x002c }, { 0x0170, 0x002c },
};
static const KraidWalkChoice kKraidWalkFromX_170[5] = {
  { 0x00f0, 0x0100 }, { 0x0180, 0x0158 }, { 0x0170, 0x0158 }, { 0x00d0, 0x002c }, { 0x0140, 0x002c },
};

static const struct {
  uint16 x_pos;
  const KraidWalkChoice *choices;
} kKraidWalkSlots[6] = {
  { 0x00f0, kKraidWalkFromX_F0 },
  { 0x0160, kKraidWalkFromX_160 },
  { 0x0180, kKraidWalkFromX_180 },
  { 0x00d0, kKraidWalkFromX_D0 },
  { 0x0140, kKraidWalkFromX_140 },
  { 0x0170, kKraidWalkFromX_170 },
};

static const uint16 kKraidSpitYSpeed[8] = {
  0xfc00, 0xfc40, 0xfb40, 0xfb80, 0xfb40, 0xfc00, 0xfb80, 0xfc40,
};

static const KraidFingernailVel kKraidFingernailVelDown[4] = {
  { 0x0000, 0xffff, 0x0000, 0x0001 },
  { 0x0000, 0xffff, 0x0000, 0x0001 },
  { 0x0000, 0xffff, 0x0000, 0x0001 },
  { 0x0000, 0xffff, 0x0000, 0x0001 },
};
static const KraidFingernailVel kKraidFingernailVelUp[4] = {
  { 0x0000, 0xffff, 0x0000, 0xffff },
  { 0x0000, 0xffff, 0x0000, 0xffff },
  { 0x0000, 0xffff, 0x0000, 0xffff },
  { 0x0000, 0xffff, 0x0000, 0xffff },
};

static const uint16 kKraidFingernailBodyHitbox[] = {
  0xffc0, 0x0010, 0xffd8, 0xffd8, 0xfff0, 0xffa0, 0x0008, 0xff80,
};

static const KraidSinkTable kKraidSinkEntry[] = {
  { 0x0130, 0x08a0, 0xc691 },
  { 0x0138, 0x0880, 0xc6a6 },
  { 0x0140, 0x0860, 0xc6a6 },
  { 0x0148, 0x0840, 0xc6a7 },
  { 0x0150, 0x0820, 0xc6a6 },
  { 0x0158, 0x0800, 0xc6a6 },
  { 0x0160, 0x03e0, 0xc6bd },
  { 0x0168, 0x03c0, 0xc6a6 },
  { 0x0170, 0x03a0, 0xc6a6 },
  { 0x0178, 0x0380, 0xc6a6 },
  { 0x0180, 0x0360, 0xc6d3 },
  { 0x0188, 0x0340, 0xc6a6 },
  { 0x0190, 0x0320, 0xc6a6 },
  { 0x0198, 0x0300, 0xc6e9 },
  { 0x01a0, 0x02e0, 0xc6a6 },
  { 0x01a8, 0x02c0, 0xc6a6 },
  { 0x01b0, 0x02a0, 0xc6ff },
  { 0x01b8, 0x0280, 0xc6a6 },
  { 0x01c0, 0x0260, 0xc6a6 },
  { 0x01c8, 0x0240, 0xc6a6 },
  { 0x01d0, 0x0220, 0xc6a6 },
  { 0x01d8, 0x0200, 0xc6a6 },
  { 0x01e0, 0x01e0, 0xc6a6 },
  { 0x01e8, 0x01c0, 0xc6a6 },
  { 0x01f0, 0x01a0, 0xc6a6 },
  { 0x01f8, 0x0180, 0xc6a6 },
  { 0x0200, 0x0160, 0xc6a6 },
  { 0x0208, 0x0140, 0xc6a6 },
  { 0xffff, 0, 0 },
};

void Kraid_SpawnExplosionEproj(uint16 k);
void Kraid_SpawnPlmToClearCeiling(void);
void Kraid_SpawnRandomQuakeProjs(void);
void Kraid_ClearSomeSpikes(void);
void Kraid_SetupGfxWithTilePrioClear(uint16 r18);
void Kraid_Shot_Mouth(void);
void Kraid_Death_UpdateBG2TilemapBottomHalf(void);
void Kraid_Death_UpdateBG2TilemapTopHalf(void);
void Kraid_Palette_Handling(void);
void Kraid_Shot_Body(void);
void Kraid_Enemy_Touch(void);
void Kraid_UpdateBG2TilemapBottomHalf(void);
void Kraid_UpdateBg2TilemapTopHalf(void);
void Kraid_HurtFlash_Handling(void);
void Kraid_HealthBasedPaletteHandling(void);
void Kraid_Shot_GlowHisEye(void);
void Kraid_Shot_MouthIsOpen(void);
void Kraid_Shot_UnglowEye(void);
void Kraid_HandleFirstPhase(void);
void Kraid_TransferTopHalfToVram(void);
void Kraid_RestrictSamusXtoFirstScreen_2(void);
void Kraid_EnemyTouch_Lint(uint16 k);
void CallKraidSinkTableFunc(uint32 ea);
void CallKraidFunc(uint32 ea);

void CallEnemyInstrExtFunc(uint32 ea, uint16 k) {
  switch (ea) {
  case fnPhantoon_Func_1: Phantoon_Func_1(); return;
  case fnPhantoon_Func_4: Phantoon_Func_4(k); return;
  case fnPhantoon_StartTrackingSamusAndInitEyeTimer: Phantoon_StartTrackingSamusAndInitEyeTimer(); return;
  case fnPhantoon_PickPatternForRound2: Phantoon_PickPatternForRound2(); return;
  default: Unreachable();
  }
}

const uint16 *EnemyInstr_Call_A7(uint16 k, const uint16 *jp) {  // 0xA7808A
  CallEnemyInstrExtFunc(jp[0] | 0xA70000, k);
  return jp + 1;
}


const uint16 *Kraid_Instr_9(uint16 k, const uint16 *jp) {  // 0xA78A8F
  Enemy_Kraid *E = Get_Kraid(0);
  if ((int16)(E->base.health - E->kraid_healths_8ths[3]) < 0) {
    Enemy_Kraid *E1 = Get_Kraid(0x40);
    if (sign16(E1->base.current_instruction + 0x75BF))
      return INSTR_RETURN_ADDR(addr_kKraid_Ilist_8A41);
  }
  return jp;
}

void Kraid_Touch_ArmFoot(void) {  // 0xA7948B
  NormalEnemyTouchAi();
}

void Kraid_Func_1(void) {  // 0xA794A4
  extra_samus_x_displacement = 4;
  extra_samus_y_displacement = -8;
}

void Kraid_Touch(void) {  // 0xA7949F
  NormalEnemyTouchAiSkipDeathAnim_CurEnemy();
}

void Kraid_Shot(void) {  // 0xA794B1
  NormalEnemyShotAi();
}

void KraidsArm_Touch(void) {  // 0xA79490
  if (!samus_invincibility_timer) {
    Kraid_Func_1();
    gEnemyData(0)[4].ai_var_A = FUNC16(KraidLint_FireLint);
    Kraid_Touch();
  }
}


void Kraid_Arm_Shot(uint16 j) {  // 0xA794B6
  Kraid_SpawnExplosionEproj(j);
  projectile_dir[j >> 1] |= kProjectileDir_Hit;
}

void sub_A7A92A(void) {  // 0xA7A92A
  Kraid_CheckIfDead();
}

uint16 Kraid_CheckIfDead(void) {  // 0xA7A92C
  return (*(uint16 *)&boss_bits_for_area[area_index] & 1) != 0;
}

void Kraid_SetEnemyPropsToDead(void) {  // 0xA7A943
  Enemy_Kraid *E = Get_Kraid(cur_enemy_index);
  E->base.properties = E->base.properties & 0x50FF | 0x700;
}

void Kraid_Init(void) {  // 0xA7A959
  VramWriteEntry *v5;
  uint16 j;

  unpause_hook.bank = -89;
  pause_hook.bank = -89;
  unpause_hook.addr = FUNC16(UnpauseHook_Kraid_IsAlive);
  pause_hook.addr = FUNC16(PauseHook_Kraid);
  if (Kraid_CheckIfDead()) {
    for (int i = 0; i < 16; i++)
      target_palettes[96 + i] = kKraidPalette2[i];
    for (int i = 2046; i >= 0; i -= 2)
      tilemap_stuff[i >> 1] = 824;
    Enemy_Kraid *E = Get_Kraid(0);
    E->field_4 = 0;
    uint16 v4 = vram_write_queue_tail;
    v5 = gVramWriteEntry(vram_write_queue_tail);
    v5->size = 512;
    v5->src.addr = addr_kKraidsRoomBg;
    v5->src.bank = -89;
    v5->vram_dst = ((reg_BG12NBA & 0xF) << 8) + 16128;
    vram_write_queue_tail = v4 + 7;
    Kraid_SpawnPlmToClearCeiling();
    Kraid_ClearSomeSpikes();
    E->kraid_var_A = FUNC16(Kraid_FadeInBg_ClearBg2TilemapTopHalf);
  } else {
    reg_BG2SC = 67;
    camera_distance_index = 2;
    *(uint16 *)scrolls = 0;
    *(uint16 *)&scrolls[2] = 1;
    Enemy_Kraid *E = Get_Kraid(0);
    E->kraid_min_y_pos_eject = 324;
    uint16 v7 = 0;
    uint16 v8 = E->base.health >> 3;
    uint16 r18 = v8;
    do {
      Get_Kraid(v7)->kraid_healths_8ths[0] = v8;
      v8 += r18;
      v7 += 2;
    } while ((int16)(v7 - 16) < 0);
    Kraid_SetupGfxWithTilePrioClear(0xDFFF);
    uint16 v10 = E->base.health >> 2;
    E->kraid_healths_4ths[0] = v10;
    uint16 v11 = E->kraid_healths_4ths[0] + v10;
    E->kraid_healths_4ths[1] = v11;
    uint16 v12 = E->kraid_healths_4ths[0] + v11;
    E->kraid_healths_4ths[2] = v12;
    E->kraid_healths_4ths[3] = E->kraid_healths_4ths[0] + v12;
    E->field_2E[8] = 0;
    E->base.x_pos = 176;
    E->base.y_pos = 592;
    E->base.properties |= kEnemyProps_Intangible;
    E->kraid_var_A = FUNC16(Kraid_RestrictSamusXtoFirstScreen);
    E->kraid_var_F = 300;
    E->kraid_next = FUNC16(Kraid_RaiseKraidThroughFloor);
    E->kraid_var_C = 64;
    DisableMinimapAndMarkBossRoomAsExplored();
    for (j = 62; !sign16(j); j -= 2)
      tilemap_stuff[(j >> 1) + 2016] = 824;
    earthquake_type = 5;
    for (int i = 0; i < 16; i++)
      target_palettes[176 + i] = kKraidBgTargetPalette3[i];
  }
}

void Kraid_SetupGfxWithTilePrioClear(uint16 r18) {  // 0xA7AAC6
  DecompressToMem(0xB9FA38, (uint8*)&ram4000);
  DecompressToMem(0xB9FE3E, (uint8*)tilemap_stuff);
  Enemy_Kraid *E = Get_Kraid(0);
  E->kraid_hurt_frame = 0;
  E->kraid_hurt_frame_timer = 0;
  uint16 v1 = 0;
  do {
    tilemap_stuff[(v1 >> 1) + 1024] = tilemap_stuff[v1 >> 1] & 0xDFFF;
    v1 += 2;
  } while ((int16)(v1 - 1536) < 0);
  uint16 v2 = 0;
  do {
    tilemap_stuff[v2] = r18 & ram4000.xray_tilemaps[v2];
    ++v2;
  } while ((int16)(v2 * 2 - 2048) < 0);
}

void KraidsArm_Init(void) {  // 0xA7AB43
  if (Kraid_CheckIfDead()) {
    Kraid_SetEnemyPropsToDead();
  } else {
    EnemyData *v0 = gEnemyData(0);
    v0[1].palette_index = v0->palette_index;
    v0[1].ai_var_A = FUNC16(nullsub_234);
    v0[1].current_instruction = addr_kKraid_Ilist_8AA4;
    v0[1].instruction_timer = 1;
    v0[1].ai_var_B = 0;
  }
}

void KraidsTopLint_Init(void) {  // 0xA7AB68
  if (Kraid_CheckIfDead()) {
    Kraid_SetEnemyPropsToDead();
  } else {
    EnemyData *v0 = gEnemyData(0);
    v0[2].palette_index = v0->palette_index;
    v0[2].instruction_timer = 0x7FFF;
    v0[2].current_instruction = addr_kKraid_Ilist_8AFE;
    v0[2].spritemap_pointer = addr_kKraid_Sprmap_A5DF;
    v0[2].ai_var_A = addr_locret_A7B831;
    v0[2].ai_preinstr = 0x7FFF;
    v0[2].ai_var_C = 0;
  }
}

void KraidsMiddleLint_Init(void) {  // 0xA7AB9C
  if (Kraid_CheckIfDead()) {
    Kraid_SetEnemyPropsToDead();
  } else {
    EnemyData *v0 = gEnemyData(0);
    v0[3].palette_index = v0->palette_index;
    v0[3].instruction_timer = 0x7FFF;
    v0[3].current_instruction = addr_kKraid_Ilist_8AFE;
    v0[3].spritemap_pointer = addr_kKraid_Sprmap_A5DF;
    v0[3].ai_var_A = addr_locret_A7B831;
    v0[3].ai_var_C = -16;
  }
}

void KraidsBottomLint_Init(void) {  // 0xA7ABCA
  if (Kraid_CheckIfDead()) {
    Kraid_SetEnemyPropsToDead();
  } else {
    EnemyData *v0 = gEnemyData(0);
    v0[4].palette_index = v0->palette_index;
    v0[4].instruction_timer = 0x7FFF;
    v0[4].current_instruction = addr_kKraid_Ilist_8AFE;
    v0[4].spritemap_pointer = addr_kKraid_Sprmap_A5DF;
    v0[4].ai_var_A = addr_locret_A7B831;
    v0[4].ai_var_C = -16;
  }
}

void KraidsFoot_Init(void) {  // 0xA7ABF8
  if (Kraid_CheckIfDead()) {
    Kraid_SetEnemyPropsToDead();
  } else {
    uint16 palette_index = Get_Kraid(0)->base.palette_index;
    Enemy_Kraid *E = Get_Kraid(0x140);
    E->base.palette_index = palette_index;
    E->base.current_instruction = addr_kKraid_Ilist_86E7;
    E->base.instruction_timer = 1;
    E->kraid_var_A = FUNC16(nullsub_234);
    E->kraid_next = 0;
  }
}

void Kraid_Main(void) {  // 0xA7AC21
  Kraid_Shot_Mouth();
  Kraid_Palette_Handling();
  Kraid_Shot_Body();
  Kraid_Enemy_Touch();
  Enemy_Kraid *E = Get_Kraid(0);
  reg_BG2HOFS = E->base.x_width + reg_BG1HOFS - bg1_x_offset - E->base.x_pos;
  reg_BG2VOFS = layer1_y_pos - E->base.y_pos + 152;
  CallKraidFunc(E->kraid_var_A | 0xA70000);
}

void Kraid_GetsBig_BreakCeilingPlatforms(void) {  // 0xA7AC4D
  uint16 kraid_var_F;

  if ((nmi_frame_counter_word & 7) == 0)
    Kraid_SpawnRandomQuakeProjs();
  uint16 dx = 1;
  Enemy_Kraid *E = Get_Kraid(0);
  if ((E->base.y_pos & 2) != 0)
    dx = -1;
  E->base.x_pos += dx;
  --E->base.y_pos;
  if ((E->base.y_pos & 3) != 0 || (kraid_var_F = E->kraid_var_F, (int16)(kraid_var_F - 18) >= 0)) {

  } else {
    SpawnEprojWithGfx(kKraidCeilingRockX[kraid_var_F >> 1], cur_enemy_index, addr_kEproj_RocksFallingKraidCeiling);
    switch (E->kraid_var_F >> 1) {
    case 0:
      SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x06, 0x12, 0xb7b3 });
      break;
    case 1:
      SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x0d, 0x12, 0xb7ab });
      break;
    case 2:
      SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x02, 0x12, 0xb7a3 });
      break;
    case 3:
      SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x0a, 0x12, 0xb7b3 });
      break;
    case 4:
      SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x05, 0x12, 0xb7ab });
      break;
    case 5:
      SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x0c, 0x12, 0xb7b3 });
      break;
    case 6:
      SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x03, 0x12, 0xb7ab });
      break;
    case 7:
      SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x0b, 0x12, 0xb7ab });
      break;
    case 8:
      SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x04, 0x12, 0xb7b3 });
      break;
    }
    E->kraid_var_F += 2;
  }
  if (sign16(E->base.y_pos - 296))
    E->kraid_var_A = FUNC16(Kraid_GetsBig_SetBG2TilemapPrioBits);
}

void Kraid_GetsBig_SetBG2TilemapPrioBits(void) {  // 0xA7AD3A
  uint16 i = 0;
  do {
    tilemap_stuff[i >> 1] |= 0x2000;
    i += 2;
  } while ((int16)(i - 4096) < 0);
  Enemy_Kraid *E = Get_Kraid(0x40);
  E->base.properties &= ~0x400;
  Get_Kraid(0)->kraid_var_A = FUNC16(Kraid_GetsBig_FinishUpdateBg2Tilemap);
  Kraid_UpdateBg2TilemapTopHalf();
}

void Kraid_GetsBig_FinishUpdateBg2Tilemap(void) {  // 0xA7AD61
  Get_Kraid(0)->kraid_var_A = FUNC16(Kraid_GetsBig_DrawRoomBg);
  Enemy_Kraid *E5 = Get_Kraid(0x140);
  E5->base.instruction_timer = 1;
  E5->base.current_instruction = addr_kKraid_Ilist_86ED;
  Enemy_Kraid *E2 = Get_Kraid(0x80);
  E2->base.current_instruction = addr_kKraid_Ilist_8B04;
  Enemy_Kraid *E3 = Get_Kraid(0xC0);
  E3->base.current_instruction = addr_kKraid_Ilist_8B04;
  Enemy_Kraid *E = Get_Kraid(0x100);
  E->base.current_instruction = addr_kKraid_Ilist_8B04;
  E2->base.spritemap_pointer = addr_kKraid_Sprmap_8C6C;
  E3->base.spritemap_pointer = addr_kKraid_Sprmap_8C6C;
  E->base.spritemap_pointer = addr_kKraid_Sprmap_8C6C;
  Kraid_UpdateBG2TilemapBottomHalf();
}

void Kraid_DrawRoomBg(void) {  // 0xA7AD9A
  VramWriteEntry *v3;

  for (int i = 0; i < 16; i++)
    target_palettes[96 + i] = kKraidPalette2[i];
  palette_change_num = 0;
  uint16 v2 = vram_write_queue_tail;
  v3 = gVramWriteEntry(vram_write_queue_tail);
  v3->size = 512;
  v3->src.addr = addr_kKraidsRoomBg;
  v3->src.bank = -89;
  v3->vram_dst = ((reg_BG12NBA & 0xF) << 8) + 16128;
  vram_write_queue_tail = v2 + 7;
}

void Kraid_GetsBig_DrawRoomBg(void) {  // 0xA7AD8E
  Enemy_Kraid *E = Get_Kraid(0);
  E->kraid_var_A = FUNC16(Kraid_GetsBig_FadeInRoomBg);
  E->kraid_var_E = 0;
  E->kraid_var_F = 0;
  Kraid_DrawRoomBg();
}

void Kraid_SetLintYAndRandomThinkTimer(void) {  // 0xA7ADEF
  int16 v1;

  Enemy_Kraid *E = Get_Kraid(0);
  Get_Kraid(0x80)->base.y_pos = E->base.y_pos - 20;
  Get_Kraid(0xC0)->base.y_pos = E->base.y_pos + 46;
  Get_Kraid(0x100)->base.y_pos = E->base.y_pos + 112;
  v1 = random_number & 7;
  if ((random_number & 7) == 0)
    v1 = 2;
  E->kraid_thinking = v1 << 6;
}

void Kraid_GetsBig_Thinking_Setup(void) {  // 0xA7ADE1
  Get_Kraid(0)->kraid_var_A = FUNC16(Kraid_GetsBig_Thinking);
  Kraid_SetLintYAndRandomThinkTimer();
}

void Kraid_Mainloop_Thinking_Setup(void) {  // 0xA7ADE9
  Get_Kraid(0)->kraid_var_A = FUNC16(Kraid_Mainloop_Thinking);
  Kraid_SetLintYAndRandomThinkTimer();
}

void Kraid_Lints_Enable(uint16 k, uint16 a) {  // 0xA7AE90
  Enemy_Kraid *E = Get_Kraid(k);
  E->kraid_var_F = a;
  E->kraid_var_A = FUNC16(Kraid_AlignEnemyToKraid);
  E->kraid_next = FUNC16(KraidLint_ProduceLint);
  E->kraid_var_B = 0;
}

void Kraid_GetsBig_FadeInRoomBg(void) {  // 0xA7AE23
  if (AdvancePaletteFade_BgPalette6() & 1) {
    Kraid_GetsBig_Thinking_Setup();
    Kraid_Lints_Enable(0x80, kKraidLintDelayTop);
    Kraid_Lints_Enable(0xc0, kKraidLintDelayMiddle);
    Kraid_Lints_Enable(0x100, kKraidLintDelayBottom);
    Enemy_Kraid *E6 = Get_Kraid(0x180);
    E6->kraid_next = FUNC16(KraidsFingernail_Init);
    Enemy_Kraid *E7 = Get_Kraid(0x1C0);
    E7->kraid_next = FUNC16(KraidsFingernail_Init);
    E6->kraid_var_A = FUNC16(KraidEnemy_HandleFunctionTimer);
    E7->kraid_var_A = FUNC16(KraidEnemy_HandleFunctionTimer);
    E6->kraid_var_F = 64;
    E7->kraid_var_F = 128;
    Get_Kraid(0x40)->kraid_var_C = 1;
    Enemy_Kraid *E0 = Get_Kraid(0);
    E0->kraid_var_B = addr_stru_A796DA;
    E0->kraid_target_x = 288;
    Enemy_Kraid *E5 = Get_Kraid(0x140);
    E5->kraid_var_A = FUNC16(KraidsFoot_SecondPhaseSetup_WalkToStartPt);
    E5->base.instruction_timer = 1;
    E5->base.current_instruction = addr_kKraid_Ilist_8887;
  }
}

void Kraid_Mainloop_Thinking(void) {  // 0xA7AEA4
  Enemy_Kraid *E = Get_Kraid(0);
  uint16 kraid_thinking = E->kraid_thinking;
  if (kraid_thinking) {
    uint16 v2 = kraid_thinking - 1;
    E->kraid_thinking = v2;
    if (!v2) {
      E->kraid_var_A = FUNC16(Kraid_Main_AttackWithMouthOpen);
      E->kraid_var_B = addr_stru_A796DA;
      E->kraid_var_C = kKraidInstrTimer_MouthOpen;
    }
  }
}

void Kraid_GetsBig_Thinking(void) {  // 0xA7AEC4
  Enemy_Kraid *E = Get_Kraid(0);
  uint16 kraid_thinking = E->kraid_thinking;
  if (kraid_thinking) {
    uint16 v2 = kraid_thinking - 1;
    E->kraid_thinking = v2;
    if (!v2) {
      E->kraid_var_A = FUNC16(Kraid_Shot_MouthIsOpen);
      E->kraid_var_B = addr_stru_A796DA;
      E->kraid_var_C = kKraidInstrTimer_MouthOpen;
    }
  }
}

void Kraid_Shot_MouthIsOpen(void) {  // 0xA7AEE4
  uint16 v2;
  Enemy_Kraid *E = Get_Kraid(0);

  if (Kraid_ProcessKraidInstr() == 0xFFFF) {
    E->kraid_var_A = FUNC16(Kraid_Mainloop_Thinking);
    E->kraid_var_C = 90;
    uint16 kraid_mouth_flags = E->kraid_mouth_flags;
    if ((kraid_mouth_flags & 4) != 0
        && (v2 = kraid_mouth_flags - 256, E->kraid_mouth_flags = v2, (v2 & 0xFF00) != 0)) {
      E->kraid_var_A = FUNC16(KraidEnemy_HandleFunctionTimer);
      E->kraid_var_F = 64;
      E->kraid_next = FUNC16(Kraid_InitEyeGlowing);
      E->field_2 = 2;
    } else {
      E->kraid_mouth_flags = 0;
    }
  }
}

uint16 Kraid_ProcessKraidInstr(void) {  // 0xA7AF32
  Enemy_Kraid *E = Get_Kraid(0);
  uint16 result = E->kraid_var_C;
  if (result) {
    if (E->kraid_var_C-- == 1)
      return Kraid_ExecuteInstr();
  }
  return result;
}

void KraidInstr_PlayRoarSfx(void) {  // 0xA7AF94
  QueueSfx2_Max6(kSfx2_KraidRoar);
}

void KraidInstr_PlayDyingSfx(void) {  // 0xA7AF9F
  QueueSfx2_Max15(kSfx2_KraidDyingCry);
}

void CallKraidInstr(uint32 ea) {
  switch (ea) {
  case fnKraidInstr_PlayRoarSfx: KraidInstr_PlayRoarSfx(); return;
  case fnKraidInstr_PlayDyingSfx: KraidInstr_PlayDyingSfx(); return;
  default: Unreachable();
  }
}

uint16 Kraid_ExecuteInstr(void) {  // 0xA7AF3D
  Enemy_Kraid *E = Get_Kraid(0);
  for (;;) {
    uint16 kraid_var_B = E->kraid_var_B;
    const uint16 *instr = (const uint16 *)RomPtr_A7(kraid_var_B);
    uint16 result = *instr;
    if (*instr == 0xFFFF)
      return result;
    if ((int16)(*instr + 1) < 0) {
      CallKraidInstr(result | 0xA70000);
      E->kraid_var_B += 2;
      continue;
    }
    E->kraid_var_C = result;
    E->kraid_var_B = kraid_var_B + 8;
    uint16 tilemap = instr[1];
    uint16 q = vram_write_queue_tail;
    gVramWriteEntry(vram_write_queue_tail)->size = 704;
    q += 2;
    gVramWriteEntry(q)->size = tilemap;
    q += 2;
    LOBYTE(gVramWriteEntry(q++)->size) = -89;
    gVramWriteEntry(q)->size = (reg_BG2SC & 0xFC) << 8;
    vram_write_queue_tail = q + 2;
    return 1;
  }
}

void Kraid_Shot_Mouth(void) {  // 0xA7AFAA
  Enemy_Kraid *E = Get_Kraid(0);
  if (!sign16(E->kraid_var_A + 0x3AC9))
    return;
  const uint8 *prev_instr = RomPtr_A7(E->kraid_var_B - 8);
  int16 mouth_hit = 0;
  if (GET_WORD(prev_instr + 6) != 0xFFFF) {
    uint16 hitbox_ptr = GET_WORD(prev_instr + 6);
    E->kraid_var_E = 1;
    const uint8 *hitbox = RomPtr_A7(hitbox_ptr);
    uint16 r22 = E->base.x_pos + GET_WORD(hitbox);
    uint16 r20 = E->base.y_pos + GET_WORD(hitbox + 2);
    uint16 r18 = E->base.y_pos + GET_WORD(hitbox + 6);
    if (projectile_counter) {
      uint16 proj = 2 * projectile_counter;
      while (!sign16(proj)) {
        int pi = proj >> 1;
        if (!sign16(projectile_y_pos[pi] - projectile_y_radius[pi] - 1 - r18)
            || sign16(projectile_y_radius[pi] + projectile_y_pos[pi] - r20)
            || sign16(projectile_x_radius[pi] + projectile_x_pos[pi] - r22)) {
          proj -= 2;
          continue;
        }
        uint16 type = projectile_type[pi];
        if ((type & kProjectileType_TypeMask) == 0) {
          if ((type & kKraidShotCharged) == 0) {
            proj -= 2;
            continue;
          }
          E->kraid_mouth_flags |= 1;
        }
        collision_detection_index = proj >> 1;
        NormalEnemyShotAiSkipDeathAnim_CurEnemy();
        projectile_dir[proj >> 1] |= kProjectileDir_Hit;
        mouth_hit = 1;
        break;
      }
    }
  }
  if (mouth_hit) {
    E->kraid_hurt_frame = 6;
    E->kraid_hurt_frame_timer = 2;
    uint16 kraid_mouth_flags = E->kraid_mouth_flags;
    if ((kraid_mouth_flags & 2) != 0)
      E->kraid_mouth_flags = kraid_mouth_flags | 4;
    if (sign16(E->base.health - 1) && sign16(E->kraid_var_A + 0x3CA0)) {
      E->kraid_var_A = FUNC16(Kraid_Death_Init);
      E->kraid_mouth_flags = 0;
      E->base.properties |= kEnemyProps_Intangible;
      Kraid_SetupGfxWithTilePrioClear(~0x2000);
      uint16 v12 = 0;
      do {
        Enemy_Kraid *EL = Get_Kraid(v12);
        EL->base.properties |= kEnemyProps_Intangible;
        v12 += 64;
      } while ((int16)(v12 - 384) < 0);
      if (sign16(E->kraid_var_B + 0x68F2))
        E->kraid_var_B += 60;
    }
  }
}

void Kraid_SpawnExplosionEproj(uint16 k) {  // 0xA7B0CB
  int v1 = k >> 1;
  eproj_spawn_pt = (Point16U){ projectile_x_pos[v1], projectile_y_pos[v1] };
  uint16 explosion = ((projectile_type[v1] & kProjectileType_SuperMissile) == 0) ? kKraidExplosion_Small : kKraidExplosion_SuperMissile;
  SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, explosion);
  QueueSfx1_Max6(kSfx1_KraidShotExplosion);
}

void Kraid_Enemy_Touch(void) {  // 0xA7B0F3
  int16 v1;

  Enemy_Kraid *E = Get_Kraid(0);
  if (sign16(E->kraid_var_A + 0x3CA0)) {
    uint16 r18 = samus_x_radius + samus_x_pos;
    v1 = samus_y_pos - E->base.y_pos;
    int i;
    for (i = 0; ; i += 4) {
      int v3 = i >> 1;
      if ((int16)(v1 - kKraidBodyHitbox[v3]) >= 0 || (int16)(v1 - kKraidBodyHitbox[v3 + 2]) >= 0)
        break;
    }
    if ((int16)(E->base.x_pos + kKraidBodyHitbox[(i >> 1) + 1] - r18) < 0) {
      if (!sign16(samus_x_pos - 40)) {
        samus_x_pos -= 8;
        samus_prev_x_pos = samus_x_pos;
      }
      uint16 kraid_min_y_pos_eject = samus_y_pos - 8;
      if ((int16)(samus_y_pos - 8 - E->kraid_min_y_pos_eject) < 0)
        kraid_min_y_pos_eject = E->kraid_min_y_pos_eject;
      samus_y_pos = kraid_min_y_pos_eject;
      samus_prev_y_pos = kraid_min_y_pos_eject;
      Kraid_Func_1();
      if (!samus_invincibility_timer)
        NormalEnemyTouchAi();
    }
  }
}

void Kraid_Shot_Body(void) {  // 0xA7B181
  Enemy_Kraid *E = Get_Kraid(0);
  if (sign16(E->kraid_var_A + 0x3AC9)) {
    E->kraid_var_E = 0;
    E->kraid_mouth_flags &= ~1;
    uint16 hits = 0;
    const uint8 *prev_instr = RomPtr_A7(E->kraid_var_B - 8);
    const uint8 *hitbox = RomPtr_A7(GET_WORD(prev_instr + 4));
    uint16 r22 = E->base.x_pos + GET_WORD(hitbox);
    uint16 r20 = E->base.y_pos + GET_WORD(hitbox + 2);
    uint16 r18 = E->base.y_pos + GET_WORD(hitbox + 6);
    if (projectile_counter) {
      for (int i = 2 * projectile_counter; i >= 0; i -= 2) {
        int pi = i >> 1;
        bool hit;
        if (sign16(projectile_y_pos[pi] - projectile_y_radius[pi] - 1 - r18)) {
          hit = !sign16(projectile_y_radius[pi] + projectile_y_pos[pi] - r20)
              && !sign16(projectile_x_radius[pi] + projectile_x_pos[pi] - r22);
        } else {
          r18 = projectile_x_radius[pi] + projectile_x_pos[pi];
          int16 rel_y = projectile_y_pos[pi] - E->base.y_pos;
          uint16 j;
          for (j = 0; ; j += 4) {
            int v13 = j >> 1;
            if ((int16)(rel_y - kKraidBodyHitbox[v13]) >= 0 || (int16)(rel_y - kKraidBodyHitbox[v13 + 2]) >= 0)
              break;
          }
          hit = (int16)(E->base.x_pos + kKraidBodyHitbox[(j >> 1) + 1] - r18) < 0;
        }
        if (hit) {
          Kraid_SpawnExplosionEproj(i);
          projectile_dir[pi] |= kProjectileDir_Hit;
          if ((projectile_type[pi] & kKraidShotCharged) != 0)
            E->kraid_mouth_flags |= 1;
          ++hits;
        }
      }
    }
    if (hits) {
      if (E->kraid_var_A == FUNC16(Kraid_Mainloop_Thinking)) {
        E->kraid_var_A = FUNC16(Kraid_InitEyeGlowing);
        uint16 kraid_mouth_flags = E->kraid_mouth_flags;
        if ((kraid_mouth_flags & 1) != 0)
          E->kraid_mouth_flags = kraid_mouth_flags | 0x302;
      }
    }
  }
}

void Kraid_Palette_Handling(void) {  // 0xA7B337
  Enemy_Kraid *E = Get_Kraid(0);
  if (sign16(E->base.health - 1)) {
    E->kraid_hurt_frame = E->base.health;
    Kraid_HurtFlash_Handling();
    Kraid_HealthBasedPaletteHandling();
    return;
  }
  if (E->kraid_hurt_frame) {
    uint16 v1 = E->kraid_hurt_frame_timer - 1;
    E->kraid_hurt_frame_timer = v1;
    if (!v1) {
      E->kraid_hurt_frame_timer = 2;
      --E->kraid_hurt_frame;
      Kraid_HurtFlash_Handling();
      Kraid_HealthBasedPaletteHandling();
    }
  }
}

void Kraid_HurtFlash_Handling(void) {  // 0xA7B371
  uint16 row = (Get_Kraid(0)->kraid_hurt_frame & 1) == 0 ? 1 : 0;
  for (int i = 0; i < 16; i++)
    palette_buffer[240 + i] = kKraidSprHealthPalettes[row][i];
}

void Kraid_HealthBasedPaletteHandling(void) {  // 0xA7B394
  uint16 row = 0;
  Enemy_Kraid *E = Get_Kraid(0);
  if ((E->kraid_hurt_frame & 1) == 0) {
    uint16 slot = 14;
    uint16 health = E->base.health;
    do {
      if ((int16)(health - Get_Kraid(slot)->kraid_healths_8ths[0]) >= 0)
        break;
      slot -= 2;
    } while (slot);
    row = (uint16)(slot + 2) >> 1;
  }
  for (int i = 0; i < 16; i++) {
    palette_buffer[112 + i] = kKraidBgHealthPalettes[row][i];
    palette_buffer[240 + i] = kKraidSprHealthPalettes[row][i];
  }
}

const uint16 *Kraid_Instr_1(uint16 k, const uint16 *jp) {  // 0xA7B633
  return jp;
}

const uint16 *Kraid_Instr_DecYpos(uint16 k, const uint16 *jp) {  // 0xA7B636
  Enemy_Kraid *E = Get_Kraid(0);
  --E->base.y_pos;
  return jp;
}

const uint16 *Kraid_Instr_IncrYpos_Shake(uint16 k, const uint16 *jp) {  // 0xA7B63C
  Enemy_Kraid *E = Get_Kraid(0);
  ++E->base.y_pos;
  earthquake_type = 1;
  earthquake_timer = 10;
  return jp;
}

const uint16 *Kraid_Instr_PlaySound_0x76(uint16 k, const uint16 *jp) {  // 0xA7B64E
  QueueSfx2_Max6(kSfx2_KraidFootstep);
  return jp;
}

const uint16 *Kraid_Instr_XposMinus3(uint16 k, const uint16 *jp) {  // 0xA7B65A
  Enemy_Kraid *E = Get_Kraid(0);
  E->base.x_pos -= kKraidWalkStepX;
  return jp;
}

const uint16 *Kraid_Instr_XposMinus3b(uint16 k, const uint16 *jp) {  // 0xA7B667
  Enemy_Kraid *E = Get_Kraid(0);
  E->base.x_pos -= kKraidWalkStepX;
  return jp;
}

const uint16 *Kraid_Instr_XposPlus3(uint16 k, const uint16 *jp) {  // 0xA7B674
  Enemy_Kraid *E = Get_Kraid(0);
  E->base.x_pos += kKraidWalkStepX;
  return jp;
}

const uint16 *Kraid_Instr_MoveHimRight(uint16 k, const uint16 *jp) {  // 0xA7B683
  uint16 v3;

  Enemy_Kraid *E = Get_Kraid(0);
  if (sign16(E->base.x_pos - 320) || (v3 = E->kraid_target_x - 1, (E->kraid_target_x = v3) == 0)) {
    if (Enemy_MoveRight_IgnoreSlopes(0, INT16_SHL16(kKraidFootMoveRightSpeed))) {
      earthquake_type = 0;
      earthquake_timer = 7;
      uint16 x_pos = Get_Kraid(0)->base.x_pos;
      Get_Kraid(0x140)->base.x_pos = x_pos;
    }
  }
  return jp;
}

void Kraid_InitEyeGlowing(void) {  // 0xA7B6BF
  Enemy_Kraid *E = Get_Kraid(0);
  E->kraid_var_A = FUNC16(Kraid_Shot_MouthIsOpen);
  E->kraid_var_A = FUNC16(Kraid_Shot_GlowHisEye);
  E->kraid_var_B = addr_stru_A7974A + 8;
  E->kraid_var_C = kKraidInstrTimer_EyeGlow;
  Kraid_Shot_GlowHisEye();
}

void Kraid_Shot_GlowHisEye(void) {  // 0xA7B6D7
  int16 saturated = 0;

  Kraid_ProcessKraidInstr();
  uint16 pal_i = 226;
  do {
    int dest = pal_i >> 1;
    uint16 red = (palette_buffer[dest] & 0x1F) + 1;
    if (!sign16((palette_buffer[dest] & 0x1F) - 30)) {
      ++saturated;
      red = 31;
    }
    uint16 green = (palette_buffer[dest] & 0x3E0) + 32;
    if (!sign16((palette_buffer[dest] & 0x3E0) - 960)) {
      ++saturated;
      green = 992;
    }
    palette_buffer[dest] = green | red | palette_buffer[dest] & 0xFC00;
    pal_i += 2;
  } while ((int16)(pal_i - 232) < 0);
  if ((int16)(saturated - 6) >= 0)
    Get_Kraid(0)->kraid_var_A = FUNC16(Kraid_Shot_UnglowEye);
}

void Kraid_Shot_UnglowEye(void) {  // 0xA7B73D
  Enemy_Kraid *E = Get_Kraid(0);
  uint16 slot = 14;
  uint16 health = E->base.health;
  do {
    if ((int16)(health - Get_Kraid(slot)->kraid_healths_8ths[0]) >= 0)
      break;
    slot -= 2;
  } while (slot);
  uint16 row = (uint16)(slot + 2) >> 1;
  uint16 pal_i = 226;
  uint16 changed = 0, component;
  uint16 color = 1;
  do {
    int dest = pal_i >> 1;
    component = palette_buffer[dest] & 0x1F;
    if ((kKraidBgHealthPalettes[row][color] & 0x1F) != component) {
      ++changed;
      --palette_buffer[dest];
    }
    component = palette_buffer[dest] & 0x3E0;
    if ((kKraidBgHealthPalettes[row][color] & 0x3E0) != component) {
      ++changed;
      palette_buffer[dest] -= 32;
    }
    pal_i += 2;
    color += 1;
  } while ((int16)(pal_i - 232) < 0);
  if (!changed) {
    E->kraid_var_A = FUNC16(Kraid_Shot_MouthIsOpen);
    E->kraid_var_B = addr_stru_A796DA;
    E->kraid_var_C = kKraidInstrTimer_MouthOpen;
  }
}

void KraidsArm_Main(void) {  // 0xA7B7BD
  uint16 r18 = layer1_y_pos + 224;
  Enemy_Kraid *E = Get_Kraid(0);
  uint16 v1 = E->base.y_pos - 44;
  Enemy_Kraid *E1 = Get_Kraid(0x40);
  E1->base.y_pos = v1;
  uint16 v3 = v1;
  uint16 v4 = E1->base.properties | 0x100;
  if ((int16)(v3 - layer1_y_pos) >= 0 && (int16)(v3 - r18) < 0)
    v4 = E1->base.properties & 0xFEFF;
  E1->base.properties = v4;
  E1->base.x_pos = E->base.x_pos;
  if (HIBYTE(E->kraid_mouth_flags))
    ++E1->base.instruction_timer;
}

void KraidLintCommon_Main(uint16 k) {  // 0xA7B822
  Kraid_EnemyTouch_Lint(k);
  // r18 = layer1_y_pos + 224;
  EnemyData *v1 = gEnemyData(k);
  CallKraidFunc(v1->ai_var_A | 0xA70000);
}

void KraidsTopLint_Main(void) {  // 0xA7B801
  gEnemyData(0x80)->instruction_timer = 0x7FFF;
  KraidLintCommon_Main(0x80);
}

void KraidsMiddleLint_Main(void) {  // 0xA7B80D
  gEnemyData(0xC0)->instruction_timer = 0x7FFF;
  KraidLintCommon_Main(0xC0);
}

void KraidsBottomLint_Main(void) {  // 0xA7B819
  gEnemyData(0x100)->instruction_timer = 0x7FFF;
  KraidLintCommon_Main(0x100);
}

void KraidLint_ProduceLint(uint16 k) {  // 0xA7B832
  EnemyData *v1 = gEnemyData(k);
  v1->properties &= 0xFAFF;
  v1->x_pos = v1->ai_var_C + gEnemyData(0)->x_pos - v1->ai_var_B;
  uint16 v2 = v1->ai_var_B + 1;
  v1->ai_var_B = v2;
  if (!sign16(v2 - 32)) {
    v1->ai_var_A = FUNC16(KraidLint_ChargeLint);
    v1->ai_preinstr = 30;
  }
}

void KraidLint_ChargeLint(uint16 k) {  // 0xA7B868
  EnemyData *E = gEnemyData(k);
  int16 v1 = 0;
  if ((E->ai_preinstr & 1) != 0)
    v1 = 3584;
  E->palette_index = v1;
  E->x_pos = E->ai_var_C + gEnemyData(0)->x_pos - E->ai_var_B;
  if (E->ai_preinstr-- == 1) {
    E->ai_var_A = FUNC16(KraidLint_FireLint);
    QueueSfx3_Max6(kSfx3_KraidLint);
  }
}

void KraidLint_FireLint(uint16 k) {  // 0xA7B89B
  Enemy_Kraid *E = Get_Kraid(k);
  AddToHiLo(&E->base.x_pos, &E->base.x_subpos, -IPAIR32(kKraidLintXSpeed, kKraidLintXSubspeed));
  if (sign16(E->base.x_pos - 56))
    E->base.properties |= kEnemyProps_Intangible;
  if (sign16(E->base.x_pos - 32)) {
    E->base.properties |= kEnemyProps_Invisible;
    E->kraid_var_A = FUNC16(Kraid_AlignEnemyToKraid);
    E->kraid_var_F = 300;
    E->kraid_next = FUNC16(KraidLint_ProduceLint);
    E->kraid_var_B = 0;
  }
  if (CheckIfEnemyTouchesSamus(k)) {
    AddToHiLo(&extra_samus_x_displacement, &extra_samus_x_subdisplacement, -IPAIR32(kKraidLintXSpeed, kKraidLintXSubspeed));
    if (sign16(extra_samus_x_displacement + 16))
      extra_samus_x_displacement = -16;
  }
}

void KraidFingernail_WaitForLintXpos(uint16 k) {  // 0xA7B907
  Enemy_Kraid *E2 = Get_Kraid(0x80);
  if (!sign16(E2->base.x_pos - 256)) {
    Enemy_Kraid *E = Get_Kraid(k);
    E->kraid_var_A = E->kraid_next;
    E->base.properties &= 0xFAFF;
  }
}

void KraidEnemy_HandleFunctionTimer(uint16 k) {  // 0xA7B92D
  Enemy_Kraid *E = Get_Kraid(k);
  if (E->kraid_var_F && E->kraid_var_F-- == 1)
    E->kraid_var_A = E->kraid_next;
}

void Kraid_AlignEnemyToKraid(uint16 k) {  // 0xA7B923
  uint16 x_pos = Get_Kraid(0)->base.x_pos;
  Enemy_Kraid *E = Get_Kraid(k);
  E->base.x_pos = x_pos - E->base.x_width;
  KraidEnemy_HandleFunctionTimer(k);
}

void KraidEnemy_DecrementEnemyFunctionTimer(void) {  // 0xA7B93F
  Enemy_Kraid *E = Get_Kraid(cur_enemy_index);
  if (E->kraid_var_F) {
    if (E->kraid_var_F-- == 1) {
      E->kraid_var_A = E->kraid_next;
      E->base.current_instruction = addr_kKraid_Ilist_8887;
      E->base.instruction_timer = 1;
    }
  }
}

void KraidFoot_FirstPhase_Thinking(uint16 k) {  // 0xA7B960
  Kraid_HandleFirstPhase();
  KraidEnemy_HandleFunctionTimer(k);
}

void KraidEnemy_ProcessInstrEnemyTimer(uint16 k) {  // 0xA7B965
  Kraid_ProcessKraidInstr();
  KraidEnemy_HandleFunctionTimer(k);
}

void Kraid_EnemyTouch_Lint(uint16 k) {  // 0xA7B96A
  Enemy_Kraid *E = Get_Kraid(k);
  if ((E->base.properties & kEnemyProps_Intangible) == 0 && !samus_invincibility_timer) {
    uint16 r18 = kKraidLintHitbox.left + E->base.x_pos - 2;
    if (!sign16(samus_x_radius + samus_x_pos - r18)) {
      if (sign16(samus_x_pos - samus_x_radius - r18)) {
        uint16 r22 = kKraidLintHitbox.top + E->base.y_pos + 2;
        if (!sign16(samus_y_radius + samus_y_pos - r22)) {
          uint16 r24 = kKraidLintHitbox.bottom + E->base.y_pos - 2;
          if (sign16(samus_y_pos - samus_y_radius - r24)) {
            uint16 v2 = extra_samus_x_displacement + ~(samus_x_radius + 16);
            if (!sign16(v2 - 16))
              v2 = 16;
            extra_samus_x_displacement = v2;
            NormalEnemyTouchAi();
            E->base.properties |= kEnemyProps_Intangible;
          }
        }
      }
    }
  }
}

void KraidsFoot_Main(void) {  // 0xA7B9F6
  int16 v3;

  EnemyData *v0 = gEnemyData(0);
  v0[5].x_pos = v0->x_pos;
  uint16 v1 = v0->y_pos + 100;
  v0[5].y_pos = v1;
  uint16 v2 = v1;
  v3 = v1 - 224;
  uint16 v4 = v0[5].properties & 0xFEFF;
  if ((int16)(v2 - layer1_y_pos) >= 0) {
    if ((int16)(v3 - layer1_y_pos) >= 0)
      v4 |= 0x100;
  } else {
    v4 |= 0x100;
  }
  v0[5].properties = v4;
  CallKraidFunc(v0[5].ai_var_A | 0xA70000);
}

void Kraid_SetWalkingBackwards(uint16 j, uint16 a) {  // 0xA7BB0D
  Get_Kraid(0)->kraid_target_x = a;
  Enemy_Kraid *E = Get_Kraid(0x140);
  E->kraid_next = j;
  E->kraid_var_A = FUNC16(KraidsFoot_SecondPhase_WalkingBackwards);
  E->base.instruction_timer = 1;
  E->base.current_instruction = addr_kKraid_Ilist_8887;
}

void Kraid_SetWalkingForwards(uint16 j, uint16 a) {  // 0xA7BB29
  Get_Kraid(0)->kraid_target_x = a;
  Enemy_Kraid *E = Get_Kraid(0x140);
  E->kraid_next = j;
  E->kraid_var_A = FUNC16(KraidsFoot_SecondPhase_WalkForward);
  E->base.instruction_timer = 1;
  E->base.current_instruction = addr_kKraid_Ilist_86F3;
}

void KraidsFoot_SecondPhase_Thinking(void) {  // 0xA7BA2E
  Enemy_Kraid *E5 = Get_Kraid(0x140);
  uint16 v1 = E5->kraid_next - 1;
  E5->kraid_next = v1;
  if (!v1) {
    Enemy_Kraid *E0 = Get_Kraid(0);
    const KraidWalkChoice *choices = kKraidWalkSlots[1].choices;
    for (int slot = 0; slot < 6; slot++) {
      if (E0->base.x_pos == kKraidWalkSlots[slot].x_pos) {
        choices = kKraidWalkSlots[slot].choices;
        break;
      }
    }
    int16 pick = random_number & 0x1C;
    if (!sign16(pick - 16))
      pick = 16;
    const KraidWalkChoice *choice = &choices[pick >> 2];
    if ((int16)(choice->target_x - E0->base.x_pos) >= 0)
      Kraid_SetWalkingBackwards(choice->think_timer, choice->target_x);
    else
      Kraid_SetWalkingForwards(choice->think_timer, choice->target_x);
  }
}

void KraidsFoot_SecondPhase_WalkingBackwards(void) {  // 0xA7BB45
  Enemy_Kraid *E0 = Get_Kraid(0);
  uint16 kraid_target_x = E0->kraid_target_x;
  if (kraid_target_x != E0->base.x_pos) {
    if ((int16)(kraid_target_x - E0->base.x_pos) >= 0)
      return;
    E0->base.x_pos = kraid_target_x;
  }
  Enemy_Kraid *E5 = Get_Kraid(0x140);
  if (!sign16(E5->base.current_instruction + 0x76C7)) {
    E5->kraid_var_A = FUNC16(KraidsFoot_SecondPhase_Thinking);
    E5->base.instruction_timer = 1;
    E5->base.current_instruction = addr_kKraid_Ilist_86ED;
  }
}

void KraidsFoot_SecondPhaseSetup_WalkToStartPt(void) {  // 0xA7BB6E
  Enemy_Kraid *E0 = Get_Kraid(0);
  uint16 kraid_target_x = E0->kraid_target_x;
  if (kraid_target_x != E0->base.x_pos) {
    if ((int16)(kraid_target_x - E0->base.x_pos) >= 0)
      return;
    E0->base.x_pos = kraid_target_x;
  }
  Enemy_Kraid *E5 = Get_Kraid(0x140);
  if (!sign16(E5->base.current_instruction + 0x76C7)) {
    E5->kraid_var_A = FUNC16(KraidEnemy_HandleFunctionTimer);
    E5->kraid_var_F = 180;
    E5->kraid_next = FUNC16(KraidsFoot_SecondPhase_Init);
    E5->base.instruction_timer = 1;
    E5->base.current_instruction = addr_kKraid_Ilist_86ED;
  }
}

void KraidsFoot_SecondPhase_Init(void) {  // 0xA7BBA4
  Kraid_SetWalkingBackwards(0xB4, 0x160);
}

void KraidsFoot_SecondPhase_WalkForward(void) {  // 0xA7BBAE
  Enemy_Kraid *E0 = Get_Kraid(0);
  uint16 kraid_target_x = E0->kraid_target_x;
  if ((int16)(kraid_target_x - E0->base.x_pos) < 0) {
    Enemy_Kraid *E5 = Get_Kraid(0x140);
    if (E5->base.current_instruction == (uint16)addr_off_A787BB) {
      E5->base.current_instruction = addr_kKraid_Ilist_86F3;
      E5->base.instruction_timer = 1;
    }
  } else {
    E0->base.x_pos = kraid_target_x;
    Enemy_Kraid *E5 = Get_Kraid(0x140);
    if (E5->base.current_instruction == (uint16)addr_off_A787BB) {
      E5->kraid_var_A = FUNC16(KraidsFoot_SecondPhase_Thinking);
      E5->base.instruction_timer = 1;
      E5->base.current_instruction = addr_kKraid_Ilist_86ED;
    }
  }
}

void Kraid_Main_AttackWithMouthOpen(void) {  // 0xA7BBEA
  uint16 v3;
  Enemy_Kraid *E = Get_Kraid(0);

  if (Kraid_ProcessKraidInstr() == 0xFFFF) {
    Kraid_Mainloop_Thinking_Setup();
    E->kraid_var_C = 90;
    uint16 kraid_mouth_flags = E->kraid_mouth_flags;
    if ((kraid_mouth_flags & 4) != 0 && (v3 = kraid_mouth_flags - 256, E->kraid_mouth_flags = v3, (v3 & 0xFF00) != 0)) {
      E->kraid_var_A = FUNC16(KraidEnemy_HandleFunctionTimer);
      E->kraid_var_F = 64;
      E->kraid_next = FUNC16(Kraid_InitEyeGlowing);
      E->field_2 = 2;
    } else {
      E->kraid_mouth_flags = 0;
    }
  } else {
    if (*((uint16 *)RomPtr_A7(E->kraid_var_B - 8) + 1) == addr_kKraidTilemaps_3 && (E->kraid_var_C & 0xF) == 0) {
      SpawnEprojWithGfx(kKraidSpitYSpeed[(random_number & 0xE) >> 1], random_number & 0xE, addr_kEproj_RocksKraidSpits);
      QueueSfx3_Max6(kSfx3_KraidRock);
    }
  }
}

void KraidsGoodFingernail_Touch(void) {  // 0xA7BCCF
  uint16 death_anim = 0;
  printf("X undefined\n");
  NormalEnemyTouchAi();
  EnemyDeathAnimation(cur_enemy_index, death_anim);
}

void KraidsBadFingernail_Touch(void) {  // 0xA7BCDE
  uint16 death_anim = 0;

  printf("X undefined\n");
  NormalEnemyTouchAi();
  EnemyDeathAnimation(cur_enemy_index, death_anim);
}

void KraidFingernailInit(uint16 k) {  // 0xA7BCF2
  uint16 palette_index = Get_Kraid(0)->base.palette_index;
  Enemy_Kraid *E = Get_Kraid(k);
  E->base.palette_index = palette_index;
  E->kraid_var_B = 40;
  E->base.properties |= kEnemyProps_Invisible;
  E->base.instruction_timer = 0x7FFF;
  E->base.current_instruction = addr_kKraid_Ilist_8B0A;
  E->base.spritemap_pointer = addr_kKraid_Sprmap_A617;
  E->kraid_next = FUNC16(KraidsFingernail_Init);
  E->kraid_var_A = FUNC16(KraidEnemy_HandleFunctionTimer);
  E->kraid_var_F = 64;
}

void KraidsGoodFingernail_Init(void) {  // 0xA7BCEF
  KraidFingernailInit(cur_enemy_index);
}

void KraidsBadFingernail_Init(void) {  // 0xA7BD2D
  KraidFingernailInit(cur_enemy_index);
}

void KraidsGoodFingernail_Main(void) {  // 0xA7BD32
  EnemyData *v0 = gEnemyData(0);
  if (sign16(v0->health - 1)) {
    EnemyData *v1 = gEnemyData(0);
    v1[6].properties |= kEnemyProps_Deleted | kEnemyProps_Invisible;
  } else {
    CallKraidFunc(v0[6].ai_var_A | 0xA70000);
  }
}

void KraidsBadFingernail_Main(void) {  // 0xA7BD49
  EnemyData *v0 = gEnemyData(0);
  if (sign16(v0->health - 1)) {
    EnemyData *v1 = gEnemyData(0);
    v1[7].properties |= kEnemyProps_Deleted | kEnemyProps_Invisible;
  } else {
    CallKraidFunc(v0[7].ai_var_A | 0xA70000);
  }
}

void KraidsFingernail_Init(void) {  // 0xA7BD60
  uint16 kraid_var_E = Get_Kraid(0x180)->kraid_var_E;
  if (cur_enemy_index == 384)
    kraid_var_E = Get_Kraid(0x1C0)->kraid_var_E;
  const KraidFingernailVel *vel = sign16(kraid_var_E)
      ? &kKraidFingernailVelDown[(random_number & 6) >> 1]
      : &kKraidFingernailVelUp[(random_number & 6) >> 1];
  Enemy_Kraid *E = Get_Kraid(cur_enemy_index);
  E->kraid_var_B = vel->x_subspeed;
  E->kraid_var_C = vel->x_speed;
  E->kraid_var_D = vel->y_subspeed;
  E->kraid_var_E = vel->y_speed;
  E->kraid_parameter_1 = 1;
  E->base.properties &= ~(kEnemyProps_Intangible | kEnemyProps_Invisible);
  E->base.instruction_timer = 1;
  E->base.current_instruction = addr_kKraid_Ilist_8B0A;
  E->kraid_var_A = FUNC16(KraidsFingernail_Fire);
  uint16 other_nail_from_mouth = Get_Kraid(0x180)->kraid_healths_8ths[1];
  if (cur_enemy_index != 448)
    other_nail_from_mouth = Get_Kraid(0x1C0)->kraid_healths_8ths[1];
  if ((random_number & 1) == 0 || other_nail_from_mouth == 1) {
    E->kraid_healths_8ths[1] = 0;
    Enemy_Kraid *E0 = Get_Kraid(0);
    E->base.x_pos = (E0->base.x_pos - E0->base.x_width - E->base.x_width) & 0xFFF0;
    E->base.y_pos = Get_Kraid(0x40)->base.y_pos + 128;
  } else {
    E->kraid_healths_8ths[1] = 1;
    E->base.x_pos = 50;
    E->base.y_pos = 240;
    E->kraid_var_B = 0;
    E->kraid_var_C = 1;
    E->kraid_var_D = 0;
    E->kraid_var_E = 0;
    E->kraid_var_A = FUNC16(KraidFingernail_WaitForLintXpos);
    E->kraid_next = FUNC16(KraidsFingernail_Fire);
    E->base.properties |= kEnemyProps_Intangible | kEnemyProps_Invisible;
  }
}

void KraidsFingernail_Fire(uint16 k) {  // 0xA7BE8E
  EnemyData *v1 = gEnemyData(k);
  if (Enemy_MoveRight_IgnoreSlopes(k, __PAIR32__(v1->ai_var_C, v1->ai_var_B))) {
    v1->ai_var_B = -v1->ai_var_B;
    v1->ai_var_C = -v1->ai_var_C;
  } else {
    int i;
    EnemyData *v3, *v4;
    for (i = 0; ; i += 4) {
      v3 = gEnemyData(0);
      v4 = gEnemyData(k);
      if ((int16)(kKraidFingernailBodyHitbox[(i >> 1) + 1] + v3->y_pos - v4->y_pos) < 0)
        break;
    }
    uint16 r18 = kKraidFingernailBodyHitbox[i >> 1] + v3->x_pos;
    if (!sign16(v4->x_width + v4->x_pos - r18) && !sign16(v4->ai_var_C)) {
      v4->ai_var_B = -v4->ai_var_B;
      v4->ai_var_C = -v4->ai_var_C;
    }
  }
  EnemyData *v5 = gEnemyData(k);
  if (Enemy_MoveDown(k, __PAIR32__(v5->ai_var_E, v5->ai_var_D)))
    SetHiLo(&v5->ai_var_E, &v5->ai_var_D, -IPAIR32(v5->ai_var_E, v5->ai_var_D));
}

void KraidsFoot_PrepareToLungeForward(void) {  // 0xA7BF2D
  Kraid_HandleFirstPhase();
  EnemyData *v0 = gEnemyData(0);
  if (!sign16(v0[1].current_instruction + 0x75C9)) {
    v0[1].current_instruction = addr_kKraid_Ilist_8AF0;
    v0[1].instruction_timer = 1;
    v0[5].instruction_timer = 1;
    v0[5].current_instruction = addr_kKraid_Ilist_87BD;
    v0[5].ai_var_A = FUNC16(KraidsFoot_FirstPhase_LungeForward);
    v0[5].ai_preinstr = 0;
  }
}

void KraidsFoot_FirstPhase_LungeForward(void) {  // 0xA7BF5D
  Kraid_HandleFirstPhase();
  Enemy_Kraid *E = Get_Kraid(0);
  if (sign16(E->base.x_pos - 92))
    E->base.x_pos = 92;
  Enemy_Kraid *E5 = Get_Kraid(0x140);
  if (E5->base.current_instruction == (uint16)addr_off_A78885) {
    if (E->base.x_pos == 92) {
      E5->kraid_next = FUNC16(KraidsFoot_FirstPhase_RetreatFromLunge);
      E5->kraid_var_A = FUNC16(KraidEnemy_DecrementEnemyFunctionTimer);
      E5->kraid_var_F = 1;
      E5->base.instruction_timer = 1;
      E5->base.current_instruction = addr_kKraid_Ilist_86ED;
    } else {
      E5->base.instruction_timer = 1;
      E5->base.current_instruction = addr_kKraid_Ilist_87BD;
    }
  }
}

void KraidsFoot_FirstPhase_RetreatFromLunge(void) {  // 0xA7BFAB
  Kraid_HandleFirstPhase();
  Enemy_Kraid *E0 = Get_Kraid(0);
  if (!sign16(E0->base.x_pos - 176))
    E0->base.x_pos = 176;
  Enemy_Kraid *E5 = Get_Kraid(0x140);
  if (!sign16(E5->base.current_instruction + 0x76C7)) {
    if (E0->base.x_pos == 176) {
      Enemy_Kraid *E1 = Get_Kraid(0x40);
      E1->base.current_instruction = addr_kKraid_Ilist_89F3;
      E1->base.instruction_timer = 1;
      E5->base.instruction_timer = 1;
      E5->base.current_instruction = addr_kKraid_Ilist_86ED;
      E5->kraid_var_A = FUNC16(KraidFoot_FirstPhase_Thinking);
      E5->kraid_var_F = 300;
      E5->kraid_next = FUNC16(KraidsFoot_PrepareToLungeForward);
    } else {
      printf("X undefined\n");
      Enemy_Kraid *E = Get_Kraid(cur_enemy_index);
      E->base.current_instruction = addr_kKraid_Ilist_8887;
      E->base.instruction_timer = 1;
    }
  }
}

void Kraid_HandleFirstPhase(void) {  // 0xA7C005
  Enemy_Kraid *E0 = Get_Kraid(0);
  if ((int16)(E0->base.health - E0->kraid_healths_8ths[6]) < 0) {
    E0->kraid_var_A = FUNC16(KraidEnemy_ProcessInstrEnemyTimer);
    E0->kraid_var_F = 180;
    E0->kraid_next = FUNC16(Kraid_GetsBig_ReleaseCamera);
    uint16 v2 = *((uint16 *)RomPtr_A7(E0->kraid_var_B) + 1);
    uint16 v3 = 50;
    if (v2 != addr_kKraidTilemaps_0) {
      v3 = 42;
      if (v2 != addr_kKraidTilemaps_1) {
        v3 = 34;
        if (v2 != addr_kKraidTilemaps_2)
          v3 = 26;
      }
    }
    E0->kraid_var_B = (uint16)(addr_stru_A796DA + v3);
    E0->kraid_var_C = GET_WORD(RomPtr_A7(addr_kKraid_MouthInstrList + v3));
    earthquake_type = 4;
    earthquake_timer = 340;
    Enemy_Kraid *E5 = Get_Kraid(0x140);
    E5->base.current_instruction = addr_kKraid_Ilist_86E7;
    E5->base.instruction_timer = 1;
    E5->kraid_var_A = FUNC16(nullsub_234);
    Enemy_Kraid *E1 = Get_Kraid(0x40);
    E1->base.current_instruction = addr_kKraid_Ilist_89F3;
    E1->base.instruction_timer = 1;
    Enemy_Kraid *E2 = Get_Kraid(0x80);
    E2->base.properties |= 0x100;
    Enemy_Kraid *E3 = Get_Kraid(0xC0);
    E3->base.properties |= 0x100;
    Enemy_Kraid *E4 = Get_Kraid(0x100);
    E4->base.properties |= 0x100;
    E1->base.properties |= 0x400;
  }
}

void Kraid_GetsBig_ReleaseCamera(void) {  // 0xA7C0A1
  Enemy_Kraid *E = Get_Kraid(0);
  E->kraid_var_A = FUNC16(Kraid_GetsBig_BreakCeilingPlatforms);
  *(uint16 *)scrolls = 514;
  *(uint16 *)&scrolls[2] = 257;
  E->kraid_min_y_pos_eject = 164;
}

void Kraid_SpawnPlmToClearCeiling(void) {  // 0xA7C168
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x02, 0x12, 0xb7b7 });
}

void Kraid_ClearSomeSpikes(void) {  // 0xA7C171
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x05, 0x1b, 0xb7bb });
}

CoroutineRet UnpauseHook_Kraid_IsDead(void) {  // 0xA7C1FB
  static const StartDmaCopy unk_A7C21E = { 1, 1, 0x18, LONGPTR(0xa7a716), 0x0200 };
  static const StartDmaCopy unk_A7C23E = { 1, 1, 0x18, LONGPTR(0x9ab200), 0x0800 };
  ScreenOff();
  WriteReg(VMADDL, 0);
  WriteReg(VMADDH, 16 * (reg_BG12NBA & 0xF) + 63);
  WriteReg(VMAIN, 0x80);
  SetupDmaTransfer(&unk_A7C21E);
  WriteReg(MDMAEN, 2);
  WriteReg(VMADDL, 0);
  WriteReg(VMADDH, 0x40);
  WriteReg(VMAIN, 0x80);
  SetupDmaTransfer(&unk_A7C23E);
  WriteReg(MDMAEN, 2);
  Kraid_TransferTopHalfToVram();
  return kCoroutineNone;
}

static const StartDmaCopy unk_A7C26B = { 1, 1, 0x18, LONGPTR(0x7e5000), 0x0400 };
static const StartDmaCopy unk_A7C28D = { 1, 1, 0x18, LONGPTR(0x7e2000), 0x0800 };

CoroutineRet UnpauseHook_Kraid_IsAlive(void) {  // 0xA7C24E
  ScreenOff();
  WriteReg(VMADDL, 0);
  WriteReg(VMADDH, reg_BG12NBA + 62);
  WriteReg(VMAIN, 0x80);
  SetupDmaTransfer(&unk_A7C26B);
  WriteReg(MDMAEN, 2);
  Kraid_TransferTopHalfToVram();
  return kCoroutineNone;
}

void Kraid_TransferTopHalfToVram(void) {  // 0xA7C278
  WriteReg(VMADDL, 0);
  WriteReg(VMADDH, reg_BG2SC & 0xFC);
  WriteReg(VMAIN, 0x80);
  SetupDmaTransfer(&unk_A7C28D);
  WriteReg(MDMAEN, 2);
  ScreenOn();
}

static const StartDmaCopy unk_A7C2BD = { 1, 1, 0x18, LONGPTR(0x7e5000), 0x0400 };

CoroutineRet Kraid_UnpauseHook_IsSinking(void) {  // 0xA7C2A0
  COROUTINE_BEGIN(coroutine_state_2, 0);
  ScreenOff();
  WriteReg(VMADDL, 0);
  WriteReg(VMADDH, reg_BG12NBA + 62);
  WriteReg(VMAIN, 0x80);
  SetupDmaTransfer(&unk_A7C2BD);
  WriteReg(MDMAEN, 2);
  if ((int16)(Get_Kraid(0)->base.y_pos - kKraidSinkEntry[0].field_0) >= 0) {
    my_counter = 0;
    my_counter2 = vram_write_queue_tail;
    while (kKraidSinkEntry[my_counter].field_0 != 0xFFFF
           && (int16)(Get_Kraid(0)->base.y_pos - kKraidSinkEntry[my_counter].field_0) >= 0) {
      VramWriteEntry *v2;
      v2 = gVramWriteEntry(my_counter2);
      v2->size = 64;
      v2->src.addr = 0x2FC0;
      v2->src.bank = 0x7E;
      v2->vram_dst = kKraidSinkEntry[my_counter].field_2 + ((reg_BG2SC & 0xFC) << 8);
      vram_write_queue_tail = my_counter2 + 7;
      COROUTINE_AWAIT(1, WaitForNMI_Async());
      my_counter++;
    }
  }
  COROUTINE_END(0);
}

void PauseHook_Kraid(void) {  // 0xA7C325
  unsigned int v1;

  uint16 q = vram_read_queue_tail;
  v1 = vram_read_queue_tail;
  *(uint16 *)((uint8 *)&vram_read_queue[0].vram_target + vram_read_queue_tail) = ((reg_BG12NBA & 0xFC) << 8) + 15872;
  *(uint16 *)((uint8 *)&vram_read_queue[0].dma_parameters + v1) = 129;
  *(uint16 *)((uint8 *)&vram_read_queue[0].dma_parameters + q + 1) = 57;
  *(VoidP *)((uint8 *)&vram_read_queue[0].src.addr + q) = 20480;
  *(uint16 *)(&vram_read_queue[0].src.bank + q) = 126;
  *(uint16 *)((uint8 *)&vram_read_queue[0].size + q) = 1024;
  vram_read_queue_tail = q + 9;
}

void Kraid_Death_Init(void) {  // 0xA7C360
  Enemy_Kraid *E0 = Get_Kraid(0);
  if (!E0->kraid_hurt_frame) {
    for (int i = 0; i < 16; i++)
      target_palettes[96 + i] = 0;
    for (int i = 0; i < 16; i++)
      palette_buffer[112 + i] = kKraidBgDeathPalette[i];
    Enemy_Kraid *E1 = Get_Kraid(0x40);
    E1->base.current_instruction = addr_kKraid_Ilist_8AF0;
    E1->base.instruction_timer = 1;
    E0->kraid_var_A = FUNC16(Kraid_Death_Fadeout);
    E0->kraid_var_B = addr_stru_A79764 + 8;
    E0->kraid_var_C = kKraidInstrTimer_Death;
    uint16 v4 = cur_enemy_index;
    uint16 v7 = cur_enemy_index;
    Enemy_Kraid *E6 = Get_Kraid(0x180);
    E6->base.properties &= ~0x4000;
    cur_enemy_index = 384;
    EnemyDeathAnimation(v4, 0x180);
    Enemy_Kraid *E7 = Get_Kraid(0x1C0);
    E7->base.properties &= ~0x4000;
    cur_enemy_index = 448;
    EnemyDeathAnimation(v4, 0x1C0);
    cur_enemy_index = 128;
    EnemyDeathAnimation(v4, 0x80);
    cur_enemy_index = 192;
    EnemyDeathAnimation(v4, 0xC0);
    cur_enemy_index = 256;
    EnemyDeathAnimation(v4, 0x100);
    cur_enemy_index = v7;
    SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x05, 0x1b, 0xb7bf });
  }
}

void Kraid_Death_Fadeout(void) {  // 0xA7C3F9
  VramWriteEntry *v3;

  Kraid_ProcessKraidInstr();
  if (AdvancePaletteFade_BgPalette6() & 1) {
    Enemy_Kraid *E = Get_Kraid(0);
    E->kraid_var_A = FUNC16(Kraid_Death_UpdateBG2TilemapTopHalf);
    E->kraid_hurt_frame_timer = 1;
    E->kraid_hurt_frame = 1;
    Kraid_Palette_Handling();
    uint16 v1 = 0;
    do {
      ram4000.xray_tilemaps[v1] = 0;
      ram4000.xray_tilemaps[v1 + 1] = 0;
      v1 += 2;
    } while ((int16)(v1 * 2 - 512) < 0);
    uint16 v2 = vram_write_queue_tail;
    v3 = gVramWriteEntry(vram_write_queue_tail);
    v3->size = 512;
    v3->src.addr = 0x4000;
    v3->src.bank = 126;
    v3->vram_dst = ((reg_BG12NBA & 0xF) << 8) + 16128;
    vram_write_queue_tail = v2 + 7;
  }
}

void Kraid_Death_UpdateBG2TilemapTopHalf(void) {  // 0xA7C4A4
  Kraid_ProcessKraidInstr();
  Get_Kraid(0)->kraid_var_A = FUNC16(Kraid_Death_UpdateBG2TilemapBottomHalf);
  Enemy_Kraid *E2 = Get_Kraid(0x80);
  E2->kraid_var_A = FUNC16(Kraid_AlignEnemyToKraid);
  E2->kraid_var_F = 0x7FFF;
  Enemy_Kraid *E3 = Get_Kraid(0xC0);
  E3->kraid_var_A = FUNC16(Kraid_AlignEnemyToKraid);
  E3->kraid_var_F = 0x7FFF;
  Enemy_Kraid *E4 = Get_Kraid(0x100);
  E4->kraid_var_A = FUNC16(Kraid_AlignEnemyToKraid);
  E4->kraid_var_F = 0x7FFF;
  Kraid_UpdateBg2TilemapTopHalf();
}

void Kraid_Death_UpdateBG2TilemapBottomHalf(void) {  // 0xA7C4C8
  Kraid_ProcessKraidInstr();
  unpause_hook.bank = -89;
  unpause_hook.addr = FUNC16(Kraid_UnpauseHook_IsSinking);
  Enemy_Kraid *E = Get_Kraid(0);
  E->kraid_var_A = FUNC16(Kraid_Death_SinkThroughFloor);
  kraid_unk9000 = 43;
  E->base.properties |= kEnemyProps_SolidToSamus;
  earthquake_type = 1;
  earthquake_timer = 256;
  Enemy_Kraid *E1 = Get_Kraid(0x40);
  E1->base.current_instruction = addr_kKraid_Ilist_8AA4;
  E1->base.instruction_timer = 1;
  Enemy_Kraid *E5 = Get_Kraid(0x140);
  E5->base.current_instruction = addr_kKraid_Ilist_86E7;
  E5->base.instruction_timer = 1;
  E5->kraid_var_A = FUNC16(nullsub_234);
  Kraid_UpdateBG2TilemapBottomHalf();
}

void Kraid_PlaySoundEveryHalfSecond(void) {  // 0xA7C51D
  /* Leftover WRAM overlay: kraid_unk9000 is the sinking SFX timer at $7E9000. */
  if (!--kraid_unk9000) {
    QueueSfx3_Max6(kSfx3_KraidRock);
    kraid_unk9000 = 30;
  }
}

void Kraid_HandleSinking(void) {  // 0xA7C59F
  int16 v1;
  VramWriteEntry *v3;

  for (int i = 0; ; ++i) {
    v1 = kKraidSinkEntry[i].field_0;
    if (v1 < 0)
      break;
    if (v1 == Get_Kraid(0)->base.y_pos) {
      if (!sign16(kKraidSinkEntry[i].field_2)) {
        uint16 v2 = vram_write_queue_tail;
        v3 = gVramWriteEntry(vram_write_queue_tail);
        v3->size = 64;
        v3->src.addr = 12224;
        v3->src.bank = 126;
        v3->vram_dst = kKraidSinkEntry[i].field_2 + ((reg_BG2SC & 0xFC) << 8);
        vram_write_queue_tail = v2 + 7;
      }
      CallKraidSinkTableFunc(kKraidSinkEntry[i].field_4 | 0xA70000);
      return;
    }
  }
}

void Kraid_Death_SinkThroughFloor(void) {  // 0xA7C537
  Kraid_ProcessKraidInstr();
  Kraid_PlaySoundEveryHalfSecond();
  Kraid_HandleSinking();
  Enemy_Kraid *E0 = Get_Kraid(0);
  if (!sign16(++E0->base.y_pos - 608)) {
    E0->base.properties &= ~kEnemyProps_Intangible;
    enemy_bg2_tilemap_size = 2;
    uint16 enemy_ptr = Get_Kraid(cur_enemy_index)->base.enemy_ptr;
    get_EnemyDef_A2(enemy_ptr)->shot_ai = FUNC16(nullsub_170_A7);
    RebindEnemyDefAi(enemy_ptr);
    Enemy_Kraid *E1 = Get_Kraid(0x40);
    uint16 v3 = E1->base.properties | kEnemyProps_Intangible | kEnemyProps_Deleted;
    E1->base.properties = v3;
    uint16 v4 = v3 & 0x51FF | 0x600;
    Get_Kraid(0x80)->base.properties = v4;
    Get_Kraid(0xC0)->base.properties = v4;
    Get_Kraid(0x100)->base.properties = v4;
    Get_Kraid(0x140)->base.properties = v4;
    Get_Kraid(0)->kraid_var_A = FUNC16(Kraid_FadeInBg_ClearBg2TilemapTopHalf);
    camera_distance_index = 0;
    Enemy_ItemDrop_Kraid(enemy_ptr);
    Kraid_DrawRoomBg();
  }
}

void Kraid_CrumbleLeftPlatform_Left(void) {  // 0xA7C691
  SpawnEprojWithGfx(0x70, cur_enemy_index, addr_kEproj_RocksFallingKraidCeiling);
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x07, 0x12, 0xb7a7 });
}

void Kraid_CrumbleRightPlatform_Middle(void) {  // 0xA7C6A7
  SpawnEprojWithGfx(0xF0, cur_enemy_index, addr_kEproj_RocksFallingKraidCeiling);
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x0f, 0x12, 0xb7a7 });
}

void Kraid_CrumbleRightPlatform_Left(void) {  // 0xA7C6BD
  SpawnEprojWithGfx(0xE0, cur_enemy_index, addr_kEproj_RocksFallingKraidCeiling);
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x0e, 0x12, 0xb7af });
}

void Kraid_CrumbleLeftPlatform_Right(void) {  // 0xA7C6D3
  SpawnEprojWithGfx(0x90, cur_enemy_index, addr_kEproj_RocksFallingKraidCeiling);
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x09, 0x12, 0xb7a7 });
}

void Kraid_CrumbleLeftPlatform_Middle(void) {  // 0xA7C6E9
  SpawnEprojWithGfx(0x80, cur_enemy_index, addr_kEproj_RocksFallingKraidCeiling);
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x08, 0x12, 0xb7af });
}

void Kraid_CrumbleRightPlatform_Right(void) {  // 0xA7C6FF
  SpawnEprojWithGfx(0x100, cur_enemy_index, addr_kEproj_RocksFallingKraidCeiling);
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x10, 0x12, 0xb7af });
}

void CallKraidSinkTableFunc(uint32 ea) {
  switch (ea) {
  case fnKraid_CrumbleLeftPlatform_Left: Kraid_CrumbleLeftPlatform_Left(); return;
  case fnnullsub_356: return;
  case fnKraid_CrumbleRightPlatform_Middle: Kraid_CrumbleRightPlatform_Middle(); return;
  case fnKraid_CrumbleRightPlatform_Left: Kraid_CrumbleRightPlatform_Left(); return;
  case fnKraid_CrumbleLeftPlatform_Right: Kraid_CrumbleLeftPlatform_Right(); return;
  case fnKraid_CrumbleLeftPlatform_Middle: Kraid_CrumbleLeftPlatform_Middle(); return;
  case fnKraid_CrumbleRightPlatform_Right: Kraid_CrumbleRightPlatform_Right(); return;
  default: Unreachable();
  }
}

void Kraid_FadeInBg_ClearBg2TilemapTopHalf(void) {  // 0xA7C715
  VramWriteEntry *v2;

  reg_BG2SC = 72;
  for (int i = 2046; i >= 0; i -= 2)
    tilemap_stuff[i >> 1] = 824;
  uint16 v1 = vram_write_queue_tail;
  v2 = gVramWriteEntry(vram_write_queue_tail);
  v2->size = 1024;
  v2->src.addr = 0x2000;
  *(uint16 *)&v2->src.bank = 126;
  v2->vram_dst = 18432;
  vram_write_queue_tail = v1 + 7;
  Get_Kraid(0)->kraid_var_A = FUNC16(Kraid_FadeInBg_ClearBg2TilemapBottomHalf);
}

void Kraid_FadeInBg_ClearBg2TilemapBottomHalf(void) {  // 0xA7C751
  VramWriteEntry *v1;

  uint16 q = vram_write_queue_tail;
  v1 = gVramWriteEntry(vram_write_queue_tail);
  v1->size = 1024;
  v1->src.addr = 0x2000;
  *(uint16 *)&v1->src.bank = 126;
  v1->vram_dst = 18944;
  vram_write_queue_tail = q + 7;
  Get_Kraid(0)->kraid_var_A = FUNC16(Kraid_FadeInBg_LoadBg3Tiles1of4);
}

void Kraid_FadeInBg_LoadBg3Tiles1of4(void) {  // 0xA7C777
  VramWriteEntry *v1;

  unpause_hook.addr = FUNC16(UnpauseHook_Kraid_IsDead);
  Get_Kraid(0)->kraid_var_A = FUNC16(Kraid_FadeInBg_LoadBg3Tiles2of4);
  uint16 q = vram_write_queue_tail;
  v1 = gVramWriteEntry(vram_write_queue_tail);
  v1->size = 1024;
  v1->src.addr = -19968;
  *(uint16 *)&v1->src.bank = 154;
  v1->vram_dst = 0x4000;
  vram_write_queue_tail = q + 7;
}

void Kraid_FadeInBg_LoadBg3Tiles2of4(void) {  // 0xA7C7A3
  VramWriteEntry *v1;

  Get_Kraid(0)->kraid_var_A = FUNC16(Kraid_FadeInBg_LoadBg3Tiles3of4);
  uint16 q = vram_write_queue_tail;
  v1 = gVramWriteEntry(vram_write_queue_tail);
  v1->size = 1024;
  v1->src.addr = -18944;
  *(uint16 *)&v1->src.bank = 154;
  v1->vram_dst = 16896;
  vram_write_queue_tail = q + 7;
}

void Kraid_FadeInBg_LoadBg3Tiles3of4(void) {  // 0xA7C7C9
  VramWriteEntry *v1;

  Get_Kraid(0)->kraid_var_A = FUNC16(Kraid_FadeInBg_LoadBg3Tiles4of4);
  uint16 q = vram_write_queue_tail;
  v1 = gVramWriteEntry(vram_write_queue_tail);
  v1->size = 1024;
  v1->src.addr = -17920;
  *(uint16 *)&v1->src.bank = 154;
  v1->vram_dst = 17408;
  vram_write_queue_tail = q + 7;
}

void Kraid_FadeInBg_LoadBg3Tiles4of4(void) {  // 0xA7C7EF
  VramWriteEntry *v1;

  Get_Kraid(0)->kraid_var_A = FUNC16(Kraid_FadeInBg_FadeInBp6);
  uint16 q = vram_write_queue_tail;
  v1 = gVramWriteEntry(vram_write_queue_tail);
  v1->size = 1024;
  v1->src.addr = -16896;
  *(uint16 *)&v1->src.bank = 154;
  v1->vram_dst = 17920;
  vram_write_queue_tail = q + 7;
}

void Kraid_FadeInBg_FadeInBp6(void) {  // 0xA7C815
  int16 boss_bits;

  if (AdvancePaletteFade_BgPalette6() & 1) {
    QueueMusic_Delayed8(kKraidMusic_Defeated);
    boss_bits = *(uint16 *)&boss_bits_for_area[area_index];
    if ((boss_bits & 1) != 0) {
      Get_Kraid(0)->kraid_var_A = FUNC16(Kraid_FadeInBg_SetEnemyDead_KraidWasDead);
    } else {
      *(uint16 *)&boss_bits_for_area[area_index] = boss_bits | 1;
      Get_Kraid(0)->kraid_var_A = FUNC16(Kraid_FadeInBg_SetEnemyDead_KraidWasAlive);
    }
  }
}

void Kraid_FadeInBg_SetEnemyDead_KraidWasAlive(void) {  // 0xA7C843
  if (!Kraid_CheckIfDead() || layer1_x_pos)
    Kraid_SetEnemyPropsToDead();
}

void Kraid_FadeInBg_SetEnemyDead_KraidWasDead(void) {  // 0xA7C851
  if (!Kraid_CheckIfDead() || layer1_x_pos)
    Kraid_SetEnemyPropsToDead();
  previous_layer1_x_block = -1;
}

void Kraid_RestrictSamusXtoFirstScreen(uint16 k) {  // 0xA7C865
  Kraid_RestrictSamusXtoFirstScreen_2();
  KraidEnemy_HandleFunctionTimer(k);
}

void Kraid_UpdateBg2TilemapTopHalf(void) {  // 0xA7C874
  VramWriteEntry *v1;

  uint16 q = vram_write_queue_tail;
  v1 = gVramWriteEntry(vram_write_queue_tail);
  v1->size = 2048;
  v1->src.addr = 0x2000;
  v1->src.bank = 126;
  v1->vram_dst = (reg_BG2SC & 0xFC) << 8;
  vram_write_queue_tail = q + 7;
}

void Kraid_UpdateBG2TilemapBottomHalf(void) {  // 0xA7C8B6
  VramWriteEntry *v1;

  uint16 q = vram_write_queue_tail;
  v1 = gVramWriteEntry(vram_write_queue_tail);
  v1->size = 2048;
  v1->src.addr = 10240;
  v1->src.bank = 126;
  v1->vram_dst = ((reg_BG2SC & 0xFC) << 8) + 2048;
  vram_write_queue_tail = q + 7;
}

void Kraid_RaiseKraidThroughFloor(uint16 k) {  // 0xA7C86B
  Kraid_RestrictSamusXtoFirstScreen_2();
  Get_Kraid(0)->kraid_var_A = FUNC16(Kraid_Raise_LoadTilemapBottomAndShake);
  Kraid_UpdateBg2TilemapTopHalf();
}

void Kraid_Raise_LoadTilemapBottomAndShake(void) {  // 0xA7C89A
  Kraid_RestrictSamusXtoFirstScreen_2();
  Enemy_Kraid *E = Get_Kraid(0);
  E->kraid_var_A = FUNC16(Kraid_Raise_SpawnRandomEarthquakeProjs16);
  E->kraid_var_F = 120;
  earthquake_timer = 496;
  QueueMusic_Delayed8(kKraidMusic_Boss);
  Kraid_UpdateBG2TilemapBottomHalf();
}

void Kraid_Raise_SpawnRandomEarthquakeProjs16(void) {  // 0xA7C8E0
  Kraid_RestrictSamusXtoFirstScreen_2();
  Enemy_Kraid *E = Get_Kraid(0);
  uint16 v1 = E->kraid_var_F - 1;
  E->kraid_var_F = v1;
  if (v1) {
    if ((v1 & 0xF) == 0)
      Kraid_SpawnRandomQuakeProjs();
  } else {
    E->kraid_var_A = FUNC16(Kraid_Raise_SpawnRandomEarthquakeProjs8);
    E->kraid_var_F = 96;
  }
}

void Kraid_Raise_SpawnRandomEarthquakeProjs8(void) {  // 0xA7C902
  Kraid_RestrictSamusXtoFirstScreen_2();
  Enemy_Kraid *E = Get_Kraid(0);
  uint16 v1 = E->kraid_var_F - 1;
  E->kraid_var_F = v1;
  if (v1) {
    if ((v1 & 7) == 0)
      Kraid_SpawnRandomQuakeProjs();
  } else {
    E->kraid_var_A = FUNC16(Kraid_Raise_Handler);
    E->kraid_var_F = 288;
  }
}

void Kraid_Raise_Handler(void) {  // 0xA7C924
  Kraid_RestrictSamusXtoFirstScreen_2();
  if ((earthquake_timer & 5) == 0)
    Kraid_SpawnRandomQuakeProjs();
  Enemy_Kraid *E0 = Get_Kraid(0);
  E0->base.x_pos += (E0->base.y_pos & 2) == 0 ? -1 : 1;
  AddToHiLo(&E0->base.y_pos, &E0->base.y_subpos, -0x8000);
  if (sign16(E0->base.y_pos - 457)) {
    E0->base.x_pos = 176;
    Enemy_Kraid *E5 = Get_Kraid(0x140);
    E5->kraid_var_A = FUNC16(KraidFoot_FirstPhase_Thinking);
    E5->kraid_var_F = 300;
    E5->kraid_next = FUNC16(KraidsFoot_PrepareToLungeForward);
    E0->kraid_var_B = addr_stru_A796DA;
    Kraid_Mainloop_Thinking_Setup();
    Enemy_Kraid *E1 = Get_Kraid(0x40);
    E1->base.current_instruction = addr_kKraid_Ilist_89F3;
    E1->base.instruction_timer = 1;
  }
}

void Kraid_SpawnRandomQuakeProjs(void) {  // 0xA7C995
  uint16 x_off = random_number & 0x3F;
  if ((random_number & 2) == 0)
    x_off = ~x_off;
  uint16 x = Get_Kraid(0)->base.x_pos + x_off;
  uint16 y = 448 - ((uint16)(random_number & 0x3F00) >> 8);
  CreateSpriteAtPos(x, y, 21, 0);
  uint16 v1 = ((random_number & 0x10) != 0) ? addr_kEproj_RocksWhenKraidRisesRight : addr_kEproj_RocksWhenKraidRisesLeft;
  SpawnEprojWithGfx(random_number & 0x3F0, cur_enemy_index, v1);
}

void Kraid_RestrictSamusXtoFirstScreen_2(void) {  // 0xA7C9EE
  if ((int16)(samus_x_pos - 256) >= 0) {
    samus_x_pos = 256;
    samus_prev_x_pos = 256;
  }
}

void CallKraidFunc(uint32 ea) {
  uint16 k = cur_enemy_index;
  switch (ea) {
  case fnKraid_GetsBig_BreakCeilingPlatforms: Kraid_GetsBig_BreakCeilingPlatforms(); return;  // 0x9bc35
  case fnKraid_GetsBig_SetBG2TilemapPrioBits: Kraid_GetsBig_SetBG2TilemapPrioBits(); return;  // 0x9bde6
  case fnKraid_GetsBig_FinishUpdateBg2Tilemap: Kraid_GetsBig_FinishUpdateBg2Tilemap(); return;  // 0x9be51
  case fnKraid_GetsBig_DrawRoomBg: Kraid_GetsBig_DrawRoomBg(); return;  // 0x9bee5
  case fnKraid_GetsBig_FadeInRoomBg: Kraid_GetsBig_FadeInRoomBg(); return;  // 0x9c08c
  case fnKraid_Mainloop_Thinking: Kraid_Mainloop_Thinking(); return;  // 0x9c1b8
  case fnKraid_GetsBig_Thinking: Kraid_GetsBig_Thinking(); return;  // 0x9c207
  case fnKraid_Shot_MouthIsOpen: Kraid_Shot_MouthIsOpen(); return;  // 0x9c256
  case fnKraid_InitEyeGlowing: Kraid_InitEyeGlowing(); return;  // 0x9cf64
  case fnKraid_Shot_GlowHisEye: Kraid_Shot_GlowHisEye(); return;  // 0x9cf9f
  case fnKraid_Shot_UnglowEye: Kraid_Shot_UnglowEye(); return;  // 0x9d0b4
  case fnKraidLint_ProduceLint: KraidLint_ProduceLint(k); return;  // 0x9d371
  case fnKraidLint_FireLint: KraidLint_FireLint(k); return;  // 0x9d44f
  case fnKraidFingernail_WaitForLintXpos: KraidFingernail_WaitForLintXpos(k); return;  // 0x9d556
  case fnKraid_AlignEnemyToKraid: Kraid_AlignEnemyToKraid(k); return;  // 0x9d5b5
  case fnKraidEnemy_HandleFunctionTimer: KraidEnemy_HandleFunctionTimer(k); return;  // 0x9d5e5
  case fnKraidEnemy_DecrementEnemyFunctionTimer: KraidEnemy_DecrementEnemyFunctionTimer(); return;  // 0x9d61c
  case fnKraidFoot_FirstPhase_Thinking: KraidFoot_FirstPhase_Thinking(k); return;  // 0x9d66a
  case fnKraidEnemy_ProcessInstrEnemyTimer: KraidEnemy_ProcessInstrEnemyTimer(k); return;  // 0x9d67d
  case fnKraidsFoot_SecondPhase_Thinking: KraidsFoot_SecondPhase_Thinking(); return;  // 0x9d8b2
  case fnKraidsFoot_SecondPhase_WalkingBackwards: KraidsFoot_SecondPhase_WalkingBackwards(); return;  // 0x9da2d
  case fnKraidsFoot_SecondPhaseSetup_WalkToStartPt: KraidsFoot_SecondPhaseSetup_WalkToStartPt(); return;  // 0x9daa2
  case fnKraidsFoot_SecondPhase_Init: KraidsFoot_SecondPhase_Init(); return;  // 0x9db2a
  case fnKraidsFoot_SecondPhase_WalkForward: KraidsFoot_SecondPhase_WalkForward(); return;  // 0x9db40
  case fnKraid_Main_AttackWithMouthOpen: Kraid_Main_AttackWithMouthOpen(); return;  // 0x9dbe0
  case fnKraidsFingernail_Init: KraidsFingernail_Init(); return;  // 0x9df94
  case fnKraidsFingernail_Fire: KraidsFingernail_Fire(k); return;  // 0x9e1ac
  case fnKraidsFoot_PrepareToLungeForward: KraidsFoot_PrepareToLungeForward(); return;  // 0x9e312
  case fnKraidsFoot_FirstPhase_RetreatFromLunge: KraidsFoot_FirstPhase_RetreatFromLunge(); return;  // 0x9e43a
  case fnKraid_GetsBig_ReleaseCamera: Kraid_GetsBig_ReleaseCamera(); return;  // 0x9e6a7
  case fnKraid_Death_Init: Kraid_Death_Init(); return;  // 0x9ec0b
  case fnKraid_Death_Fadeout: Kraid_Death_Fadeout(); return;  // 0x9ed75
  case fnKraid_Death_UpdateBG2TilemapTopHalf: Kraid_Death_UpdateBG2TilemapTopHalf(); return;  // 0x9ef19
  case fnKraid_Death_UpdateBG2TilemapBottomHalf: Kraid_Death_UpdateBG2TilemapBottomHalf(); return;  // 0x9ef92
  case fnKraid_Death_SinkThroughFloor: Kraid_Death_SinkThroughFloor(); return;  // 0x9f074
  case fnKraid_FadeInBg_ClearBg2TilemapTopHalf: Kraid_FadeInBg_ClearBg2TilemapTopHalf(); return;  // 0x9f369
  case fnKraid_FadeInBg_ClearBg2TilemapBottomHalf: Kraid_FadeInBg_ClearBg2TilemapBottomHalf(); return;  // 0x9f3ed
  case fnKraid_FadeInBg_LoadBg3Tiles1of4: Kraid_FadeInBg_LoadBg3Tiles1of4(); return;  // 0x9f448
  case fnKraid_FadeInBg_LoadBg3Tiles2of4: Kraid_FadeInBg_LoadBg3Tiles2of4(); return;  // 0x9f4ad
  case fnKraid_FadeInBg_LoadBg3Tiles3of4: Kraid_FadeInBg_LoadBg3Tiles3of4(); return;  // 0x9f507
  case fnKraid_FadeInBg_LoadBg3Tiles4of4: Kraid_FadeInBg_LoadBg3Tiles4of4(); return;  // 0x9f561
  case fnKraid_FadeInBg_FadeInBp6: Kraid_FadeInBg_FadeInBp6(); return;  // 0x9f5bb
  case fnKraid_FadeInBg_SetEnemyDead_KraidWasAlive: Kraid_FadeInBg_SetEnemyDead_KraidWasAlive(); return;  // 0x9f62d
  case fnKraid_FadeInBg_SetEnemyDead_KraidWasDead: Kraid_FadeInBg_SetEnemyDead_KraidWasDead(); return;  // 0x9f65b
  case fnKraid_RestrictSamusXtoFirstScreen: Kraid_RestrictSamusXtoFirstScreen(k); return;  // 0x9f694
  case fnKraid_RaiseKraidThroughFloor: Kraid_RaiseKraidThroughFloor(k); return;  // 0x9f6a7
  case fnKraid_Raise_LoadTilemapBottomAndShake: Kraid_Raise_LoadTilemapBottomAndShake(); return;  // 0x9f71f
  case fnKraid_Raise_SpawnRandomEarthquakeProjs16: Kraid_Raise_SpawnRandomEarthquakeProjs16(); return;  // 0x9f7b8
  case fnKraid_Raise_SpawnRandomEarthquakeProjs8: Kraid_Raise_SpawnRandomEarthquakeProjs8(); return;  // 0x9f805
  case fnKraid_Raise_Handler: Kraid_Raise_Handler(); return;  // 0x9f852
  case fnKraidLint_ChargeLint: KraidLint_ChargeLint(k); return;  // 0xa7b868
  case fnnullsub_234: return;  // 0xa7ba2d
  case fnKraidsFoot_FirstPhase_LungeForward: KraidsFoot_FirstPhase_LungeForward(); return;  // 0xa7bf5d
  case fnnullsub_347: return;
  default: Unreachable();
  }
}

