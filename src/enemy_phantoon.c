// Enemy AI - Phantoon boss + Wrecked Ship critters (Etecoon/Dachora) — peeled from Bank $A7
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "enemy_types.h"
#include "sm_rtl.h"
#include "enemy_ai_canon.h"

#undef r18

enum {
  kPhantoonBg2TilemapFill = 0x338,
  kPhantoonBg2TilemapSize = 0x360,
  kPhantoonLayer_SemiTransparent = 0x4000,
  kPhantoonHealthStep = 312,
  kPhantoonEnrageDamage = 300,
  kPhantoonFig8PathLenRight = 533,
  kPhantoonFig8PathLenLeft = 534,
  kPhantoonFig8SlowAccelSub = 0x600,
  kPhantoonFig8SlowAccel = 0,
  kPhantoonFig8FastAccelSub = 0x1000,
  kPhantoonFig8FastAccel = 0,
  kPhantoonFig8Stage0MaxSpeed = 2,
  kPhantoonFig8Stage1MaxSpeed = 7,
  kPhantoonFig8Stage2MinSpeed = 0,
  kPhantoonFig8RevStage0MaxSpeed = 0xfffe,
  kPhantoonFig8RevStage1MaxSpeed = 0xfff9,
  kPhantoonFig8RevStage2MinSpeed = 0,
  kPhantoonWavyIntroAmpDelta = 0x40,
  kPhantoonWavyIntroMaxAmp = 0xc00,
  kPhantoonWavyDyingAmpDelta = 0x100,
  kPhantoonWavyDyingMaxAmp = 0xf000,
  kPhantoonWavyPhaseDelta = 8,
  kPhantoonMusic_Boss = 5,
  kPhantoonMusic_Defeated = 3,
  kSfx2_PhantoonMaterialise1 = 0x79,
  kSfx2_PhantoonMaterialise2 = 0x7A,
  kSfx2_PhantoonMaterialise3 = 0x7B,
  kSfx2_SmallExplosion = 0x24,
  kSfx2_PhantoonExploding = 0x2B,
  kSfx2_PhantoonDyingCry = 0x7E,
  kSfx2_PhantoonCry = 0x73,
  kSfx2_EtecoonTheme = 0x35,
  kSfx2_EtecoonCry = 0x33,
  kSfx2_EtecoonWallJump = 0x32,
  kSfx2_DachoraCry = 0x1D,
  kSfx2_Silence = 0x71,
  kSfx2_DachoraStoredShinespark = 0x3D,
  kSfx2_DachoraSpeedBooster = 0x39,
  kSfx2_DachoraShinespark = 0x3B,
  kSfx2_DachoraShinesparkEnded = 0x3C,
  kSfx3_PhantoonFlame = 0x1D,
  kSfx3_PhantoonMaterialisesAttack = 0x28,
  kSfx3_PhantoonSuperMissileAttack = 0x29,
  kEtecoonHopYSpeed = 0xfffd,
  kEtecoonHopYSubspeed = 0,
  kEtecoonJumpYSpeed = 0xfffc,
  kEtecoonJumpYSubspeed = 0,
  kEtecoonXSpeedRight = 2,
  kEtecoonXSubspeedRight = 0,
  kEtecoonXSpeedLeft = 0xfffe,
  kEtecoonXSubspeedLeft = 0,
  kEtecoonSamusXThreshold = 0x40,
  kDachoraSamusXThreshold = 0x60,
  kDachoraBlinkDuration = 0x78,
  kDachoraShinesparkDelay = 0x3c,
  kDachoraEchoUpdateInterval = 1,
  kDachoraEchoLifetime = 8,
  kDachoraMaxXSpeed = 8,
  kDachoraMaxXSubspeed = 0,
  kDachoraXAccel = 0,
  kDachoraXSubaccel = 0x1000,
};

typedef struct PhantoonFlameRainPos {
  uint16 move_index;
  uint16 x;
  uint16 y;
  uint16 pad;
} PhantoonFlameRainPos;
_Static_assert(sizeof(PhantoonFlameRainPos) == 8, "flame rain pos is 8 bytes");

typedef struct PhantoonExplosion {
  int8 x;
  int8 y;
  uint8 type;
  uint8 timer;
} PhantoonExplosion;
_Static_assert(sizeof(PhantoonExplosion) == 4, "explosion entry is 4 bytes");

/* Leftover: debug write into ROM extended spritemap at $A7902D. */
#define g_stru_A7902D ((ExtendedSpriteMap*)RomFixedPtr(0xa7902d))

static const uint16 kPhantoonPartIlists[4] = {
  addr_kPhantoon_Ilist_BodyInvulnerable,
  addr_kPhantoon_Ilist_EyeClosed,
  addr_kPhantoon_Ilist_Tentacles,
  addr_kPhantoon_Ilist_MouthInitial,
};

static const uint16 kPhantoonMaterialisationSfx[3] = {
  kSfx2_PhantoonMaterialise1, kSfx2_PhantoonMaterialise2, kSfx2_PhantoonMaterialise3,
};

static const uint8 kPhantoonFlameRainGap[8] = { 5, 7, 0, 7, 5, 3, 1, 3 };

static const uint16 kPhantoonFlamePattern0[] = { 5, 0xb4, 0x20, 0x20, 0x20, 0x20, 0x20 };
static const uint16 kPhantoonFlamePattern1[] = { 3, 0xb4, 0x10, 0x10, 0x10 };
static const uint16 kPhantoonFlamePattern2[] = { 7, 0xb4, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30 };
static const uint16 kPhantoonFlamePattern3[] = { 7, 0xb4, 0x10, 0x40, 0x20, 0x40, 0x20, 0x10, 0x20 };
static const uint16 *const kPhantoonFlamePatterns[4] = {
  kPhantoonFlamePattern0, kPhantoonFlamePattern1, kPhantoonFlamePattern2, kPhantoonFlamePattern3,
};

static const uint16 kPhantoonFig8VulnerableTimers[8] = {
  0x003c, 0x001e, 0x000f, 0x001e, 0x003c, 0x001e, 0x000f, 0x003c,
};
static const uint16 kPhantoonEyeClosedTimers[8] = {
  0x02d0, 0x003c, 0x0168, 0x02d0, 0x0168, 0x003c, 0x0168, 0x02d0,
};
static const uint16 kPhantoonFlameRainHideTimers[8] = {
  0x003c, 0x0078, 0x001e, 0x003c, 0x001e, 0x003c, 0x001e, 0x001e,
};

static const uint16 kPhantoonEyeIlists[10] = {
  addr_kPhantoon_Ilist_EyeballUp,
  addr_kPhantoon_Ilist_EyeballUpRight,
  addr_kPhantoon_Ilist_EyeballRight,
  addr_kPhantoon_Ilist_EyeballDownRight,
  addr_kPhantoon_Ilist_EyeballDown,
  addr_kPhantoon_Ilist_EyeballDown,
  addr_kPhantoon_Ilist_EyeballDownLeft,
  addr_kPhantoon_Ilist_EyeballLeft,
  addr_kPhantoon_Ilist_EyeballUpLeft,
  addr_kPhantoon_Ilist_EyeballUp,
};

static const uint8 kPhantoonShotUnused0FEA[8] = { 6, 6, 8, 8, 6, 8, 6, 8 };

static const PhantoonFlameRainPos kPhantoonFlameRainPos[8] = {
  { 0x0001, 0x0080, 0x0060, 0 },
  { 0x0047, 0x00a8, 0x0040, 0 },
  { 0x0088, 0x00d0, 0x0060, 0 },
  { 0x00c9, 0x00a8, 0x0080, 0 },
  { 0x0001, 0x0080, 0x0060, 0 },
  { 0x014e, 0x0058, 0x0040, 0 },
  { 0x018f, 0x0030, 0x0060, 0 },
  { 0x01d1, 0x0058, 0x0080, 0 },
};

static const PhantoonExplosion kPhantoonExplosions[13] = {
  { 0x00, 0x00, 0x1d, 0x10 },
  { 0x20, -0x20, 0x1d, 0x10 },
  { -0x20, 0x20, 0x1d, 0x10 },
  { -0x20, -0x20, 0x1d, 0x10 },
  { 0x20, 0x20, 0x1d, 0x20 },
  { -0x20, -0x08, 0x1d, 0x08 },
  { 0x00, 0x00, 0x03, 0x08 },
  { 0x20, -0x08, 0x1d, 0x08 },
  { 0x00, 0x00, 0x03, 0x08 },
  { 0x00, 0x18, 0x03, 0x08 },
  { 0x00, 0x30, 0x1d, 0x08 },
  { -0x18, 0x18, 0x03, 0x08 },
  { 0x18, 0x18, 0x03, 0x08 },
};

static const uint16 kPhantoonFadeOutPalette[16] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const uint16 kWreckedShipPowerOnPalettes[112] = {
  0x0000, 0x02df, 0x01d7, 0x00ac, 0x5ebb, 0x3db3, 0x292e, 0x1486, 0x0bb1, 0x48fb, 0x7fff, 0x0000, 0x7fff, 0x44e5, 0x7fff, 0x0000,
  0x2003, 0x0bb1, 0x1ea9, 0x0145, 0x5ebb, 0x3db3, 0x292e, 0x1486, 0x6318, 0x1ce7, 0x1084, 0x0000, 0x7fff, 0x02df, 0x001f, 0x0000,
  0x2003, 0x72bc, 0x48fb, 0x1816, 0x6318, 0x6318, 0x6318, 0x6318, 0x6318, 0x6318, 0x6318, 0x6318, 0x6318, 0x6318, 0x7fff, 0x0000,
  0x0000, 0x72b2, 0x71c7, 0x4d03, 0x6318, 0x6318, 0x6318, 0x6318, 0x6318, 0x6318, 0x6318, 0x6318, 0x6318, 0x6318, 0x7fff, 0x0000,
  0x0000, 0x3ef4, 0x262e, 0x0d68, 0x3e94, 0x25ce, 0x0d08, 0x1484, 0x6e94, 0x55ce, 0x3d08, 0x2c84, 0x1ea9, 0x0bb1, 0x7fff, 0x0000,
  0x0000, 0x1246, 0x0145, 0x00c0, 0x3652, 0x1d8c, 0x04c6, 0x0484, 0x6231, 0x496b, 0x30a5, 0x2021, 0x0015, 0x025b, 0x7fff, 0x0000,
  0x0000, 0x6318, 0x6318, 0x6318, 0x6318, 0x6318, 0x6318, 0x6318, 0x6318, 0x6318, 0x6318, 0x6318, 0x6318, 0x6318, 0x7fff, 0x0000,
};

static const uint16 kPhantoonHealthPalettes[8][16] = {
  { 0x000e, 0x25dc, 0x1538, 0x0071, 0x0030, 0x1d79, 0x1117, 0x08b4, 0x0092, 0x421f, 0x1d79, 0x0cf5, 0x0493, 0x1c1d, 0x0c19, 0x0013 },
  { 0x000c, 0x2a1c, 0x1957, 0x0070, 0x002e, 0x2199, 0x1136, 0x08d3, 0x0091, 0x4a5f, 0x1d99, 0x0cf4, 0x04b1, 0x201d, 0x0c18, 0x0012 },
  { 0x000a, 0x2e5c, 0x1d96, 0x008e, 0x004c, 0x25d8, 0x1555, 0x08f1, 0x00af, 0x529f, 0x21d8, 0x1113, 0x04d0, 0x241d, 0x1017, 0x0011 },
  { 0x0008, 0x327c, 0x21b5, 0x008c, 0x004a, 0x29f7, 0x1574, 0x0d10, 0x00ae, 0x5adf, 0x25f7, 0x1131, 0x04ce, 0x281d, 0x1016, 0x000f },
  { 0x0006, 0x36bb, 0x21d4, 0x008b, 0x0048, 0x2e17, 0x1993, 0x0d0f, 0x00cc, 0x631f, 0x2a17, 0x1150, 0x04ed, 0x2c1d, 0x1016, 0x000e },
  { 0x0004, 0x3afb, 0x25f3, 0x00a9, 0x0046, 0x3256, 0x1db2, 0x0d2d, 0x00eb, 0x6b5f, 0x2e56, 0x156f, 0x04eb, 0x301d, 0x1415, 0x000c },
  { 0x0002, 0x3f3b, 0x2a12, 0x00a7, 0x0044, 0x3675, 0x1df1, 0x0d4c, 0x04e9, 0x77bf, 0x3275, 0x158e, 0x050a, 0x341d, 0x1414, 0x000b },
  { 0x0000, 0x477b, 0x2e52, 0x00c6, 0x0063, 0x3ab5, 0x2210, 0x116b, 0x0508, 0x7fff, 0x36b5, 0x19ad, 0x0929, 0x381d, 0x1814, 0x000a },
};

static const uint16 kDachoraPalette[16] = {
  0x3800, 0x3f57, 0x2e4d, 0x00e2, 0x0060, 0x3ab0, 0x220b, 0x1166,
  0x0924, 0x57ff, 0x42f7, 0x2610, 0x158c, 0x7c1f, 0x6018, 0x300a,
};
static const uint16 kDachoraSpeedBoostPalettes[4][16] = {
  { 0x1580, 0x3f57, 0x2e4d, 0x00e2, 0x0060, 0x3ab0, 0x220b, 0x1166, 0x0924, 0x57ff, 0x42f7, 0x2610, 0x158c, 0x7c1f, 0x6018, 0x300a },
  { 0x3800, 0x5357, 0x424d, 0x14e2, 0x1460, 0x4eb0, 0x360b, 0x2566, 0x1d24, 0x63ff, 0x56f7, 0x3a10, 0x298c, 0x7c1f, 0x6818, 0x440a },
  { 0x3800, 0x6b57, 0x5a4d, 0x2ce2, 0x2c60, 0x66b0, 0x4e0b, 0x3d66, 0x3524, 0x73ff, 0x6af7, 0x5210, 0x418c, 0x7c1f, 0x7418, 0x5c0a },
  { 0x3800, 0x7f57, 0x6e4d, 0x40e2, 0x4060, 0x7ab0, 0x620b, 0x5166, 0x4924, 0x7fff, 0x7ef7, 0x6610, 0x558c, 0x7c1f, 0x7c18, 0x700a },
};
static const uint16 kDachoraShinePalettes[4][16] = {
  { 0x3800, 0x3f57, 0x2e4d, 0x00e2, 0x0060, 0x3ab0, 0x220b, 0x1166, 0x0924, 0x57ff, 0x42f7, 0x2610, 0x158c, 0x7c1f, 0x6018, 0x300a },
  { 0x3800, 0x4b78, 0x3e90, 0x1967, 0x1906, 0x46f3, 0x326f, 0x25eb, 0x1da9, 0x5fff, 0x4f18, 0x3673, 0x29ef, 0x7cdf, 0x64d9, 0x3cce },
  { 0x3800, 0x579a, 0x4ef4, 0x320d, 0x31cc, 0x5336, 0x46d3, 0x3a70, 0x362e, 0x67ff, 0x5b5a, 0x46d6, 0x3e73, 0x7d9f, 0x699a, 0x4d92 },
  { 0x3800, 0x63bb, 0x5f37, 0x4ab3, 0x4a72, 0x6379, 0x5737, 0x52f5, 0x4ed4, 0x6fff, 0x677b, 0x5b39, 0x52f7, 0x7e5f, 0x725c, 0x5e56 },
};
static const uint16 kDachoraPaletteDest[8] = {
  0xc100, 0xc120, 0xc140, 0xc160, 0xc180, 0xc1a0, 0xc1c0, 0xc1e0,
};

void Phantoon_Init(void) {  // 0xA7CDF3
  int16 j;
  uint16 k;

  for (int i = 4094; i >= 0; i -= 2)
    tilemap_stuff[i >> 1] = kPhantoonBg2TilemapFill;
  for (j = 2046; j >= 0; j -= 2)
    *(uint16 *)((uint8 *)&kraid_unk9000 + (uint16)j) = 0;
  for (k = 30; !sign16(k); k -= 2)
    target_palettes[(k >> 1) + 112] = 0;
  enemy_bg2_tilemap_size = kPhantoonBg2TilemapSize;
  DisableMinimapAndMarkBossRoomAsExplored();
  Enemy_Phantoon *E = Get_Phantoon(cur_enemy_index);
  E->phant_var_E = 120;
  E->phant_var_A = 0;
  E->phant_var_B = 0;
  g_word_7E9032 = 0;
  Enemy_Phantoon *E1 = Get_Phantoon(0x40);
  E1->phant_parameter_1 = 0;
  E1->phant_parameter_2 = 0;
  Enemy_Phantoon *E0 = Get_Phantoon(0);
  E0->base.properties |= kEnemyProps_Intangible;
  static const SpawnHdmaObject_Args kPhantoonSemiTransparencyHdma = { 0x01, 0x14, 0xce96 };
  SpawnHdmaObject(0xa7, &kPhantoonSemiTransparencyHdma);
  Phantoon2_Init();
}

void Phantoon2_Init(void) {  // 0xA7CE55
  EnemyData *v6 = gEnemyData(cur_enemy_index);
  v6->spritemap_pointer = addr_kSpritemap_Nothing_A7;
  v6->instruction_timer = 1;
  v6->timer = 0;
  EnemyData *v7 = gEnemyData(0);
  v6->palette_index = v7->palette_index;
  v6->vram_tiles_index = v7->vram_tiles_index;
  v6->current_instruction = kPhantoonPartIlists[v6->parameter_2];
  v6->ai_preinstr = FUNC16(Phantoon_Spawn8FireballsInCircleAtStart);
  v7[3].parameter_1 = 0;
  v7[3].ai_var_C = -1;
}

void Phantoon_Main(void) {  // 0xA7CEA6
  Phantoon_Func_2(cur_enemy_index);
  Enemy_Phantoon *EK = Get_Phantoon(cur_enemy_index);
  EnemyRunPreInstr(EK->phant_var_F);
  if (cur_enemy_index == 0) { // code bug: X is overwritten
    Enemy_Phantoon *E0 = Get_Phantoon(0);
    Enemy_Phantoon *E1 = Get_Phantoon(0x40);
    Enemy_Phantoon *E2 = Get_Phantoon(0x80);
    Enemy_Phantoon *E3 = Get_Phantoon(0xC0);
    uint16 x_pos = E0->base.x_pos;
    E1->base.x_pos = x_pos;
    E2->base.x_pos = x_pos;
    E3->base.x_pos = x_pos;
    uint16 y_pos = E0->base.y_pos;
    E1->base.y_pos = y_pos;
    E2->base.y_pos = y_pos;
    E3->base.y_pos = y_pos;
    if (!E1->phant_parameter_1) {
      reg_BG2HOFS = layer1_x_pos - E0->base.x_pos + 40;
      reg_BG2VOFS = layer1_y_pos - E0->base.y_pos + 40;
    }
  }



}

void Phantoon_Func_1(void) {  // 0xA7CEED
  QueueSfx2_Max6(kPhantoonMaterialisationSfx[g_word_7E9032]);
  uint16 next = g_word_7E9032 + 1;
  if (!sign16(g_word_7E9032 - 2))
    next = 0;
  g_word_7E9032 = next;
}

void Phantoon_Func_2(uint16 k) {  // 0xA7CF0C
  if (!k && (joypad1_newkeys & kButton_Y) != 0)
    *(uint16 *)((uint8 *)&g_stru_A7902D[0].ypos + 1) = *(uint16 *)((uint8 *)&g_stru_A7902D[0].ypos + 1) == 0;
}

uint8 Phantoon_Func_3(int32 amt) {  // 0xA7CF27
  Enemy_Phantoon *E = Get_Phantoon(0xC0);
  uint16 r18 = amt, r20 = amt >> 16;
  if (E->phant_var_E) {
    uint16 phant_var_D = E->phant_var_D;
    bool v4 = phant_var_D < r18;
    E->phant_var_D = phant_var_D - r18;
    if (v4) {
      E->phant_var_D = 0;
      return 1;
    }
  } else {
    uint16 r22 = (r20 & 0xFF00) >> 8;
    uint16 v1 = r18 + E->phant_var_D;
    E->phant_var_D = v1;
    v1 = (v1 & 0xFF00) >> 8;
    if (!sign16(v1 - r22))
      E->phant_var_D = r20;
  }
  return 0;
}

void Phantoon_Func_4(uint16 k) {  // 0xA7CF5E
  SpawnEprojWithGfx(0, k, addr_kEproj_DestroyableFireballs);
  QueueSfx3_Max6(kSfx3_PhantoonFlame);
}

void Phantoon_Func_5(uint16 k) {  // 0xA7CF70
  for (int i = 7; i >= 0; --i)
    SpawnEprojWithGfx(i | 0x600, k, addr_kEproj_DestroyableFireballs);
  QueueSfx3_Max6(kSfx3_PhantoonMaterialisesAttack);
}

void Phantoon_Func_6(uint16 k, uint16 a) {  // 0xA7CF8B
  int16 v2;
  int16 v3;

  v2 = kPhantoonFlameRainGap[a];
  int n = 7;
  int r20 = 16;
  do {
    v3 = v2;
    SpawnEprojWithGfx(r20 | v2++ | 0x400, k, addr_kEproj_DestroyableFireballs);
    if ((int16)(v3 - 8) >= 0)
      v2 = 0;
    r20 += 16;
  } while (--n >= 0);
}

void Phantoon_Func_7(uint16 k) {  // 0xA7CFCA
  Enemy_Phantoon *E = Get_Phantoon(k + 192);
  bool v3 = E->phant_var_B == 1;
  bool v4 = (--E->phant_var_B & 0x8000) != 0;
  if (v3 || v4) {
    if (!sign16(E->phant_var_C)) {
      v3 = E->phant_var_C == 1;
      bool v8 = (--E->phant_var_C & 0x8000) != 0;
      const uint16 *pat = kPhantoonFlamePatterns[E->phant_var_A];
      if (v3 || v8) {
        E->phant_var_C = -1;
        E->phant_var_B = pat[1];
      } else {
        E->phant_var_B = pat[E->phant_var_C + 1];
      }
      E->base.instruction_timer = 1;
      E->base.current_instruction = addr_kPhantoon_Ilist_MouthSpawnFlame;
    } else {
      uint16 v5 = NextRandom() & 3;
      E->phant_var_A = v5;
      const uint16 *pat = kPhantoonFlamePatterns[v5];
      uint16 v7 = pat[0];
      E->phant_var_C = v7;
      E->phant_var_B = pat[v7 + 1];
    }
  }
}

void Phantoon_StartTrackingSamusAndInitEyeTimer(void) {  // 0xA7D03F
  Get_Phantoon(0x80)->phant_var_A = 0;
  Enemy_Phantoon *E = Get_Phantoon(0);
  E->base.instruction_timer = 1;
  E->base.current_instruction = addr_kPhantoon_Ilist_BodyEyeHitbox;
  E->base.properties &= ~kEnemyProps_Intangible;
  E->phant_var_E = kPhantoonFig8VulnerableTimers[NextRandom() & 7];
  E->phant_var_F = FUNC16(Phantoon_EyeFollowsSamusUntilTimerRunsOut);

  Enemy_Phantoon *E1 = Get_Phantoon(0x40);
  E1->base.instruction_timer = 1;
  E1->base.current_instruction = addr_kPhantoon_Ilist_EyeballCentred;
}

void Phantoon_PickPatternForRound2(void) {  // 0xA7D076
  int16 v4;

  Enemy_Phantoon *E = Get_Phantoon(0);
  E->phant_var_E = 60;
  uint16 v1 = kPhantoonEyeClosedTimers[NextRandom() & 7];
  Enemy_Phantoon *E1 = Get_Phantoon(0x40);
  E1->phant_var_A = v1;
  if ((nmi_frame_counter_word & 1) != 0) {
    if (!E1->phant_var_C) {
      v4 = E->phant_var_A - 1;
      E->phant_var_A = v4;
      if (v4 < 0)
        E->phant_var_A = kPhantoonFig8PathLenRight;
    }
    E->phant_var_C = 0;
    E->phant_var_B = 0;
    E->phant_var_D = 0;
    E1->phant_var_C = 1;
  } else {
    if (E1->phant_var_C) {
      uint16 v3 = E->phant_var_A + 1;
      E->phant_var_A = v3;
      if (!sign16(v3 - kPhantoonFig8PathLenLeft))
        E->phant_var_A = 0;
    }
    E->phant_var_C = 1;
    E->phant_var_B = 0;
    E->phant_var_D = 0;
    E1->phant_var_C = 0;
  }
  if (E->phant_parameter_2) {
    E1->phant_var_F = 0;
    E->phant_var_F = FUNC16(Phantoon_FadeOutBeforeFirstFireballRain);
  } else {
    E->phant_var_F = FUNC16(Phantoon_MovePhantoonInFigure8ThenOpenEye);
  }
}

void Phantoon_AdjustSpeedAndMoveInFigure8(void) {  // 0xA7D0F1
  if (Get_Phantoon(0x40)->phant_var_C) {
    Phantoon_AdjustSpeedRightSideClockwise();
    Phantoon_MoveInFigure8_RightSideClockwise(addr_kPhantoonMoveData, kPhantoonFig8PathLenRight);
  } else {
    Phantoon_AdjustSpeedLeftSideClockwise();
    Phantoon_MoveInFigure8_LeftSideClockwise(addr_kPhantoonMoveData, kPhantoonFig8PathLenLeft);
  }
}

void Phantoon_AdjustSpeedLeftSideClockwise(void) {  // 0xA7D114
  Enemy_Phantoon *E = Get_Phantoon(0);
  uint16 phant_var_D = E->phant_var_D;
  if (phant_var_D) {
    if ((phant_var_D & 1) != 0) {
      AddToHiLo(&E->phant_var_C, &E->phant_var_B, __PAIR32__(kPhantoonFig8FastAccel, kPhantoonFig8FastAccelSub));
      if ((int16)(E->phant_var_C - kPhantoonFig8Stage1MaxSpeed) >= 0) {
        E->phant_var_C = kPhantoonFig8Stage1MaxSpeed;
        E->phant_var_B = 0;
        ++E->phant_var_D;
      }
    } else {
      AddToHiLo(&E->phant_var_C, &E->phant_var_B, -IPAIR32(kPhantoonFig8FastAccel, kPhantoonFig8FastAccelSub));
      if (E->phant_var_C == kPhantoonFig8Stage2MinSpeed || (int16)(E->phant_var_C - kPhantoonFig8Stage2MinSpeed) < 0) {
        E->phant_var_C = kPhantoonFig8Stage2MinSpeed + 1;
        E->phant_var_B = 0;
        E->phant_var_D = 0;
      }
    }
  } else {
    AddToHiLo(&E->phant_var_C, &E->phant_var_B, __PAIR32__(kPhantoonFig8SlowAccel, kPhantoonFig8SlowAccelSub));
    if ((int16)(E->phant_var_C - kPhantoonFig8Stage0MaxSpeed) >= 0) {
      E->phant_var_C = kPhantoonFig8Stage0MaxSpeed - 1;
      E->phant_var_B = 0;
      ++E->phant_var_D;
    }
  }
}

void Phantoon_AdjustSpeedRightSideClockwise(void) {  // 0xA7D193
  Enemy_Phantoon *E = Get_Phantoon(0);
  uint16 phant_var_D = E->phant_var_D;
  if (phant_var_D) {
    if ((phant_var_D & 1) != 0) {
      AddToHiLo(&E->phant_var_C, &E->phant_var_B, -IPAIR32(kPhantoonFig8FastAccel, kPhantoonFig8FastAccelSub));
      if (E->phant_var_C == kPhantoonFig8RevStage1MaxSpeed || (int16)(E->phant_var_C - kPhantoonFig8RevStage1MaxSpeed) < 0) {
        E->phant_var_C = kPhantoonFig8RevStage1MaxSpeed + 1;
        E->phant_var_B = 0;
        ++E->phant_var_D;
      }
    } else {
      AddToHiLo(&E->phant_var_C, &E->phant_var_B, __PAIR32__(kPhantoonFig8FastAccel, kPhantoonFig8FastAccelSub));
      if ((int16)(E->phant_var_C - kPhantoonFig8RevStage2MinSpeed) >= 0) {
        E->phant_var_C = kPhantoonFig8RevStage2MinSpeed;
        E->phant_var_B = 0;
        E->phant_var_D = 0;
      }
    }
  } else {
    AddToHiLo(&E->phant_var_C, &E->phant_var_B, -IPAIR32(kPhantoonFig8SlowAccel, kPhantoonFig8SlowAccelSub));
    if (E->phant_var_C == kPhantoonFig8RevStage0MaxSpeed || (int16)(E->phant_var_C - kPhantoonFig8RevStage0MaxSpeed) < 0) {
      E->phant_var_C = (uint16)(kPhantoonFig8RevStage0MaxSpeed + 2);
      E->phant_var_B = 0;
      ++E->phant_var_D;
    }
  }
}

void Phantoon_MoveInFigure8_LeftSideClockwise(uint16 j, uint16 r20) {  // 0xA7D215
  Enemy_Phantoon *E = Get_Phantoon(0);
  for (int n = E->phant_var_C; n; --n) {
    const uint8 *v2 = RomPtr_A7(2 * E->phant_var_A + j);
    E->base.x_pos += (int8)v2[0];
    E->base.y_pos += (int8)v2[1];
    if (!sign16(++E->phant_var_A - r20))
      E->phant_var_A = 0;
  }
}

void Phantoon_MoveInFigure8_RightSideClockwise(uint16 j, uint16 r20) {  // 0xA7D271
  Enemy_Phantoon *E = Get_Phantoon(0);
  for(int n = (uint16)-E->phant_var_C; n; n--) {
    const uint8 *v4 = RomPtr_A7(2 * E->phant_var_A + j);
    E->base.x_pos -= (int8)v4[0];
    E->base.y_pos -= (int8)v4[1];
    if (sign16(--E->phant_var_A))
      E->phant_var_A = r20;
  }
}

void Phantoon_MoveInSwoopingPattern(uint16 k) {  // 0xA7D2D1
  int16 phant_var_E;
  uint16 v4;
  uint16 v5;
  uint16 v6;
  int16 v13;

  Enemy_Phantoon *E2 = Get_Phantoon(0x80);
  phant_var_E = E2->phant_var_E;
  if (phant_var_E < 0) {
    v6 = phant_var_E - 2;
    E2->phant_var_E = v6;
    v4 = v6 & 0x7FFF;
    if (!v4) {
      v4 = 0;
      E2->phant_var_E = 0;
    }
  } else {
    v4 = phant_var_E + 2;
    E2->phant_var_E = v4;
    if (!sign16(v4 - 256)) {
      v5 = v4 | 0x8000;
      E2->phant_var_E = v5;
      v4 = v5 & 0x7FFF;
    }
  }
  Enemy_Phantoon *E0 = Get_Phantoon(0);
  if ((int16)(v4 - E0->base.x_pos) < 0) {
    if (!sign16(E2->phant_var_C + 2047))
      E2->phant_var_C -= 32;
  } else if (sign16(E2->phant_var_C - 2048)) {
    E2->phant_var_C += 32;
  }
  AddToHiLo(&E0->base.x_pos, &E0->base.x_subpos, INT16_SHL8(E2->phant_var_C));
  if (sign16(E0->base.x_pos + 64)) {
    E0->base.x_pos = -64;
  } else if (!sign16(E0->base.x_pos - 448)) {
    E0->base.x_pos = 448;
  }
  if (Get_Phantoon(k)->phant_var_F == FUNC16(Phantoon_CompleteSwoopAfterFatalShot))
    v13 = 112;
  else
    v13 = samus_y_pos - 48;
  if ((int16)(v13 - E0->base.y_pos) < 0) {
    if (!sign16(E2->phant_var_D + 1535))
      E2->phant_var_D -= 64;
  } else if (sign16(E2->phant_var_D - 1536)) {
    E2->phant_var_D += 64;
  }
  AddToHiLo(&E0->base.y_pos, &E0->base.y_subpos, INT16_SHL8(E2->phant_var_D));
  if (sign16(E0->base.y_pos - 64)) {
    E0->base.y_pos = 64;
  } else if (!sign16(E0->base.y_pos - 216)) {
    E0->base.y_pos = 216;
  }
}

void Phantoon_BeginSwoopingPattern(uint16 k) {  // 0xA7D3E1
  Enemy_Phantoon *E2 = Get_Phantoon(0x80);
  E2->phant_var_C = 1024;
  E2->phant_var_D = 1024;
  E2->phant_var_E = 0;
  Enemy_Phantoon *E = Get_Phantoon(k);
  E->phant_var_F = FUNC16(Phantoon_IsSwooping);
  E->phant_var_E = 360;
}

void Phantoon_ChangeEyeSpriteBasedOnSamusDist(void) {  // 0xA7D3FA
  uint16 dir = DetermineDirectionOfSamusFromEnemy();
  Enemy_Phantoon *E = Get_Phantoon(0x40);
  E->base.instruction_timer = 1;
  E->base.current_instruction = kPhantoonEyeIlists[dir];
}

void Phantoon_StartDeathSequence(uint16 k) {  // 0xA7D421
  int16 v3;

  Enemy_Phantoon *E = Get_Phantoon(k);
  if (E->phant_var_F == FUNC16(Phantoon_IsSwooping)
      || E->phant_var_F == FUNC16(Phantoon_FadeoutWithSwoop)) {
    E->phant_var_F = FUNC16(Phantoon_CompleteSwoopAfterFatalShot);
  } else {
    E->phant_var_F = FUNC16(Phantoon_DyingPhantoonFadeInOut);
  }
  Enemy_Phantoon *E1 = Get_Phantoon(0x40);
  E1->phant_var_C = 0;
  E1->phant_var_F = 0;
  Phantoon_ChangeEyeSpriteBasedOnSamusDist();
  v3 = 510;
  uint16 v4 = reg_BG2HOFS;
  do {
    *(uint16 *)((uint8 *)&g_word_7E9100 + (uint16)v3) = v4;
    v3 -= 2;
  } while (v3 >= 0);
  phantom_related_layer_flag |= kPhantoonLayer_SemiTransparent;
  Get_Phantoon(0xC0)->phant_parameter_2 = 1;
}

void Phantoon_FadeOut(uint16 a) {  // 0xA7D464
  if ((nmi_frame_counter_word & 1) == 0) {
    Enemy_Phantoon *E = Get_Phantoon(0x40);
    if (!E->phant_var_F) {
      E->phant_var_D = a;
      if (Phantoon_Func_8() & 1)
        E->phant_var_F = 1;
    }
  }
}

void Phantoon_FadeIn(uint16 a) {  // 0xA7D486
  if ((nmi_frame_counter_word & 1) == 0) {
    Enemy_Phantoon *E = Get_Phantoon(0x40);
    if (!E->phant_var_F) {
      E->phant_var_D = a;
      if (Phantoon_SetColorBasedOnHp() & 1)
        E->phant_var_F = 1;
    }
  }
}

void Phantoon_Spawn8FireballsInCircleAtStart(uint16 k) {  // 0xA7D4A9
  Enemy_Phantoon *EK = Get_Phantoon(k);
  bool v2 = EK->phant_var_E == 1;
  bool v3 = (--EK->phant_var_E & 0x8000) != 0;
  if (v2 || v3) {
    Enemy_Phantoon *E0 = Get_Phantoon(0);
    SpawnEprojWithGfx(E0->phant_var_A, k, addr_kEproj_StartingFireballs);
    QueueSfx3_Max6(kSfx3_PhantoonFlame);
    EK->phant_var_E = 30;
    uint16 v5 = E0->phant_var_A + 1;
    E0->phant_var_A = v5;
    if (!sign16(v5 - 8)) {
      E0->phant_var_A = 0;
      Get_Phantoon(k + 128)->phant_var_B = 0;
      EK->phant_var_F = FUNC16(Phantoon_WaitBetweenSpawningAndSpinningFireballs);
      EK->phant_var_E = 30;
      SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x00, 0x06, 0xb781 });
    }
  }
}

void Phantoon_WaitBetweenSpawningAndSpinningFireballs(uint16 k) {  // 0xA7D4EE
  Enemy_Phantoon *E = Get_Phantoon(k);
  bool v2 = E->phant_var_E == 1;
  bool v3 = (--E->phant_var_E & 0x8000) != 0;
  if (v2 || v3) {
    E->phant_var_E = 240;
    E->phant_var_B = 1;
    E->phant_var_F = FUNC16(Phantoon_SpawnFireballsBeforeFight);
  }
}

void Phantoon_SpawnFireballsBeforeFight(uint16 k) {  // 0xA7D508
  Enemy_Phantoon *EK = Get_Phantoon(k);
  bool v3 = EK->phant_var_E == 1;
  bool v4 = (--EK->phant_var_E & 0x8000) != 0;
  if (v3 || v4) {
    EK->phant_var_B = 0;
    phantom_related_layer_flag |= kPhantoonLayer_SemiTransparent;
    EK->phant_var_F = FUNC16(Phantoon_WavyFadeIn);
    Enemy_Phantoon *E3 = Get_Phantoon(0xC0);
    E3->phant_parameter_1 = -32767;
    EK->phant_var_E = 120;
    sub_88E487(2, kPhantoonWavyPhaseDelta);
    E3->phant_var_D = kPhantoonWavyIntroMaxAmp;
    Get_Phantoon(0x40)->phant_var_F = 0;
    QueueMusic_Delayed8(kPhantoonMusic_Boss);
  }
}

void Phantoon_WavyFadeIn(uint16 k) {  // 0xA7D54A
  Phantoon_FadeIn(0xC);
  if (Phantoon_Func_3(__PAIR32__(kPhantoonWavyIntroMaxAmp, kPhantoonWavyIntroAmpDelta))) {
    Enemy_Phantoon *E = Get_Phantoon(k);
    E->phant_var_F = FUNC16(Phantoon_PickPatternForRound1);
    Get_Phantoon(0xC0)->phant_parameter_1 = 1;
    E->phant_var_E = 30;
  } else {
    Enemy_Phantoon *E = Get_Phantoon(k);
    bool v2 = E->phant_var_E == 1;
    bool v3 = (--E->phant_var_E & 0x8000) != 0;
    if (v2 || v3)
      Get_Phantoon(0xC0)->phant_var_E = 1;
  }
}

void Phantoon_PickPatternForRound1(uint16 k) {  // 0xA7D596
  Enemy_Phantoon *EK = Get_Phantoon(k);
  bool v2 = EK->phant_var_E == 1;
  bool v3 = (--EK->phant_var_E & 0x8000) != 0;
  if (v2 || v3) {
    Enemy_Phantoon *E1 = Get_Phantoon(0x40);
    E1->phant_parameter_1 = 0;
    EK->phant_var_F = FUNC16(Phantoon_MovePhantoonInFigure8ThenOpenEye);
    E1->phant_var_A = kPhantoonEyeClosedTimers[(nmi_frame_counter_word >> 1) & 3];
    if ((NextRandom() & 1) != 0) {
      Enemy_Phantoon *E = Get_Phantoon(0);
      E->phant_var_C = 0;
      E->phant_var_B = 0;
      E->phant_var_D = 0;
      E1->phant_var_C = 1;
      E->phant_var_A = kPhantoonFig8PathLenRight;
    } else {
      Enemy_Phantoon *E = Get_Phantoon(0);
      E->phant_var_C = 1;
      E->phant_var_B = 0;
      E->phant_var_D = 0;
      E1->phant_var_C = 0;
      E->phant_var_A = 0;
    }
  }
}

void Phantoon_MovePhantoonInFigure8ThenOpenEye(uint16 k) {  // 0xA7D5E7
  Phantoon_AdjustSpeedAndMoveInFigure8();
  Phantoon_Func_7(k);
  Enemy_Phantoon *E = Get_Phantoon(0x40);
  bool v2 = E->phant_var_A == 1;
  bool v3 = (--E->phant_var_A & 0x8000) != 0;
  if (v2 || v3) {
    Get_Phantoon(k)->phant_var_F = FUNC16(nullsub_237);
    E->base.instruction_timer = 1;
    E->base.current_instruction = addr_kPhantoon_Ilist_EyeOpen;
    Get_Phantoon(0)->phant_parameter_2 = 0;
    Phantoon_Func_5(k);
  }
}

void Phantoon_EyeFollowsSamusUntilTimerRunsOut(uint16 k) {  // 0xA7D60D
  Enemy_Phantoon *EK = Get_Phantoon(k);
  bool v2 = EK->phant_var_E == 1;
  bool v3 = (--EK->phant_var_E & 0x8000) != 0;
  if (v2 || v3) {
    Get_Phantoon(k + 128)->phant_var_B = 0;
    Enemy_Phantoon *E2 = Get_Phantoon(0x80);
    if (!E2->phant_var_A) {
      EK->phant_var_F = FUNC16(nullsub_237);
      Enemy_Phantoon *E0 = Get_Phantoon(0);
      E0->base.instruction_timer = 1;
      Enemy_Phantoon *E1 = Get_Phantoon(0x40);
      E1->base.instruction_timer = 1;
      E0->base.current_instruction = addr_kPhantoon_Ilist_BodyInvulnerable;
      E1->base.current_instruction = addr_kPhantoon_Ilist_EyeCloseAndPickPattern;
      E0->base.properties |= kEnemyProps_Intangible;
      E0->phant_parameter_2 = 1;
      return;
    }
    E2->phant_var_A = 0;
    EK->phant_var_E = 60;
    EK->phant_var_F = FUNC16(Phantoon_BecomesSolidAndBodyVuln);
  }
  Phantoon_ChangeEyeSpriteBasedOnSamusDist();
}

void Phantoon_BecomesSolidAndBodyVuln(uint16 k) {  // 0xA7D65C
  Phantoon_ChangeEyeSpriteBasedOnSamusDist();
  phantom_related_layer_flag &= ~kPhantoonLayer_SemiTransparent;
  Phantoon_BeginSwoopingPattern(k);
  Enemy_Phantoon *E = Get_Phantoon(0);
  E->base.instruction_timer = 1;
  E->base.current_instruction = addr_kPhantoon_Ilist_BodyFullHitbox;
}

void Phantoon_IsSwooping(uint16 k) {  // 0xA7D678
  Phantoon_ChangeEyeSpriteBasedOnSamusDist();
  Phantoon_MoveInSwoopingPattern(k);
  Enemy_Phantoon *EK = Get_Phantoon(k);
  bool v2 = EK->phant_var_E == 1;
  bool v3 = (--EK->phant_var_E & 0x8000) != 0;
  if (v2 || v3) {
    EK->phant_var_F = FUNC16(Phantoon_FadeoutWithSwoop);
    phantom_related_layer_flag |= kPhantoonLayer_SemiTransparent;
    Enemy_Phantoon *E0 = Get_Phantoon(0);
    E0->base.instruction_timer = 1;
    Enemy_Phantoon *E1 = Get_Phantoon(0x40);
    E1->base.instruction_timer = 1;
    E0->base.current_instruction = addr_kPhantoon_Ilist_BodyInvulnerable;
    E1->base.current_instruction = addr_kPhantoon_Ilist_EyeClose;
    E0->base.properties |= kEnemyProps_Intangible;
    E1->phant_var_F = 0;
    Get_Phantoon(k + 128)->phant_var_B = 0;
  }
}

void Phantoon_FadeoutWithSwoop(uint16 k) {  // 0xA7D6B9
  Phantoon_MoveInSwoopingPattern(k);
  Phantoon_FadeOut(0xC);
  if (Get_Phantoon(0x40)->phant_var_F) {
    Enemy_Phantoon *E = Get_Phantoon(k);
    E->phant_var_F = FUNC16(Phantoon_WaitAfterFadeOut);
    E->phant_var_E = 120;
  }
}

void Phantoon_WaitAfterFadeOut(uint16 k) {  // 0xA7D6D4
  Enemy_Phantoon *E = Get_Phantoon(k);
  bool v2 = E->phant_var_E == 1;
  bool v3 = (--E->phant_var_E & 0x8000) != 0;
  if (v2 || v3)
    E->phant_var_F = FUNC16(Phantoon_MoveLeftOrRightAndPickEyeOpenPatt);
}

void Phantoon_MoveLeftOrRightAndPickEyeOpenPatt(uint16 k) {  // 0xA7D6E2
  Enemy_Phantoon *E = Get_Phantoon(0);
  if ((NextRandom() & 1) != 0) {
    E->phant_var_A = 136;
    E->base.x_pos = 208;
  } else {
    E->phant_var_A = 399;
    E->base.x_pos = 48;
  }
  E->base.y_pos = 96;
  Enemy_Phantoon *E1 = Get_Phantoon(0x40);
  E1->phant_var_C = 0;
  E->phant_var_C = 1;
  E->phant_var_B = 0;
  E->phant_parameter_2 = 0;
  Phantoon_PickPatternForRound2();
  Get_Phantoon(k)->phant_var_F = FUNC16(Phantoon_FadeInBeforeFigure8);
  E1->phant_var_F = 0;
}

void Phantoon_FadeInBeforeFigure8(uint16 k) {  // 0xA7D72D
  Phantoon_FadeIn(0xC);
  if (Get_Phantoon(0x40)->phant_var_F)
    Get_Phantoon(0)->phant_var_F = FUNC16(Phantoon_MovePhantoonInFigure8ThenOpenEye);
}

void Phantoon_BecomeSolidAfterRainingFireballs(uint16 k) {  // 0xA7D73F
  Enemy_Phantoon *E1 = Get_Phantoon(0x40);
  E1->phant_var_F = 0;
  phantom_related_layer_flag &= ~kPhantoonLayer_SemiTransparent;
  Enemy_Phantoon *E = Get_Phantoon(0);
  E->base.instruction_timer = 1;
  E1->base.instruction_timer = 1;
  E->base.current_instruction = addr_kPhantoon_Ilist_BodyFullHitbox;
  E1->base.current_instruction = addr_kPhantoon_Ilist_EyeballCentred;
  Get_Phantoon(k)->phant_var_F = FUNC16(Phantoon_FadeInDuringFireballRain);
}

void Phantoon_FadeInDuringFireballRain(uint16 k) {  // 0xA7D767
  Phantoon_FadeIn(1);
  if (Get_Phantoon(0x40)->phant_var_F) {
    Enemy_Phantoon *E = Get_Phantoon(0);
    E->base.properties &= ~kEnemyProps_Intangible;
    Get_Phantoon(k)->phant_var_F = FUNC16(Phantoon_FollowSamusWithEyeDuringFireballRain);
    E->phant_var_E = 90;
  }
}

void Phantoon_FollowSamusWithEyeDuringFireballRain(uint16 k) {  // 0xA7D788
  Enemy_Phantoon *EK = Get_Phantoon(k);
  bool v2 = EK->phant_var_E == 1;
  bool v3 = (--EK->phant_var_E & 0x8000) != 0;
  if (v2 || v3) {
    Get_Phantoon(k + 128)->phant_var_B = 0;
    Enemy_Phantoon *E2 = Get_Phantoon(0x80);
    if (E2->phant_var_A) {
      E2->phant_var_A = 0;
      Get_Phantoon(0)->phant_parameter_2 = 1;
      Phantoon_BeginSwoopingPattern(k);
    } else {
      EK->phant_var_F = FUNC16(Phantoon_FadeOutDuringFireballRain);
      Enemy_Phantoon *E1 = Get_Phantoon(0x40);
      E1->phant_var_F = 0;
      Enemy_Phantoon *E0 = Get_Phantoon(0);
      E0->base.instruction_timer = 1;
      E1->base.instruction_timer = 1;
      E0->base.current_instruction = addr_kPhantoon_Ilist_BodyInvulnerable;
      E1->base.current_instruction = addr_kPhantoon_Ilist_EyeClose;
      E0->base.properties |= kEnemyProps_Intangible;
      phantom_related_layer_flag |= kPhantoonLayer_SemiTransparent;
    }
  }
}

void Phantoon_FadeOutDuringFireballRain(uint16 k) {  // 0xA7D7D5
  Phantoon_FadeOut(0xC);
  if (Get_Phantoon(0x40)->phant_var_F) {
    Enemy_Phantoon *E = Get_Phantoon(k);
    E->phant_var_F = FUNC16(Phantoon_SpawnRainingFireballs);
    E->phant_var_E = kPhantoonFlameRainHideTimers[NextRandom() & 7];
  }
}

void Phantoon_SpawnRainingFireballs(uint16 k) {  // 0xA7D7F7
  Enemy_Phantoon *EK = Get_Phantoon(k);
  bool v2 = EK->phant_var_E == 1;
  bool v3 = (--EK->phant_var_E & 0x8000) != 0;
  if (v2 || v3) {
    uint16 a = NextRandom() & 7;
    Enemy_Phantoon *E0 = Get_Phantoon(0);
    const PhantoonFlameRainPos *pos = &kPhantoonFlameRainPos[a];
    E0->phant_var_A = pos->move_index;
    E0->base.x_pos = pos->x;
    E0->base.y_pos = pos->y;
    Get_Phantoon(0x40)->phant_var_C = 0;
    EK->phant_var_F = FUNC16(Phantoon_BecomeSolidAfterRainingFireballs);
    Phantoon_Func_6(k, a);
  }
}

void Phantoon_FadeOutBeforeFirstFireballRain(uint16 k) {  // 0xA7D82A
  Phantoon_FadeOut(0xC);
  Phantoon_AdjustSpeedAndMoveInFigure8();
  Phantoon_Func_7(k);
  Enemy_Phantoon *E = Get_Phantoon(0x40);
  bool v2 = E->phant_var_A == 1;
  bool v3 = (--E->phant_var_A & 0x8000) != 0;
  if (v2 || v3) {
    Get_Phantoon(0x80)->phant_var_A = 0;
    Get_Phantoon(k)->phant_var_F = FUNC16(Phantoon_BecomeSolidAfterRainingFireballs);
    if (sign16(Get_Phantoon(0)->base.x_pos - 128))
      Phantoon_Func_6(k, 0);
    else
      Phantoon_Func_6(k, 2);
  }
}

void Phantoon_FadeOutBeforeEnrage(uint16 k) {  // 0xA7D85C
  Phantoon_FadeOut(0xC);
  if (Get_Phantoon(0x40)->phant_var_F) {
    Enemy_Phantoon *E = Get_Phantoon(k);
    E->phant_var_F = FUNC16(Phantoon_MoveEnragedPhantoonToTopCenter);
    E->phant_var_E = 120;
  }
}

void Phantoon_MoveEnragedPhantoonToTopCenter(uint16 k) {  // 0xA7D874
  Enemy_Phantoon *E = Get_Phantoon(k);
  bool v2 = E->phant_var_E == 1;
  bool v3 = (--E->phant_var_E & 0x8000) != 0;
  if (v2 || v3) {
    E->phant_var_F = FUNC16(Phantoon_FadeInEnragedPhantoon);
    Enemy_Phantoon *E = Get_Phantoon(0);
    E->base.x_pos = 128;
    E->base.y_pos = 32;
    Get_Phantoon(0x40)->phant_var_F = 0;
  }
}

void Phantoon_FadeInEnragedPhantoon(uint16 k) {  // 0xA7D891
  Phantoon_FadeIn(0xC);
  Enemy_Phantoon *E = Get_Phantoon(0x40);
  if (E->phant_var_F) {
    Get_Phantoon(0)->phant_var_F = FUNC16(Phantoon_Enraged);
    Get_Phantoon(k)->phant_var_E = 4;
    E->phant_var_F = 0;
  }
}

void Phantoon_Enraged(uint16 k) {  // 0xA7D8AC
  int16 v5;
  int16 v8;

  Enemy_Phantoon *E = Get_Phantoon(k);
  bool v2 = E->phant_var_E == 1;
  bool v3 = (--E->phant_var_E & 0x8000) != 0;
  if (v2 || v3) {
    if ((Get_Phantoon(0x40)->phant_var_F & 1) != 0) {
      v5 = 15;
      do {
        v8 = v5;
        SpawnEprojWithGfx(v5-- | 0x200, k, addr_kEproj_DestroyableFireballs);
      } while ((int16)(v8 - 9) >= 0);
    } else {
      for (int i = 6; i >= 0; --i)
        SpawnEprojWithGfx(i | 0x200, k, addr_kEproj_DestroyableFireballs);
    }
    QueueSfx3_Max6(kSfx3_PhantoonSuperMissileAttack);
    Enemy_Phantoon *E1 = Get_Phantoon(0x40);
    uint16 v7 = E1->phant_var_F + 1;
    E1->phant_var_F = v7;
    if (sign16(v7 - 8)) {
      Get_Phantoon(k)->phant_var_E = 128;
    } else {
      E1->base.instruction_timer = 1;
      E1->base.current_instruction = addr_kPhantoon_Ilist_EyeClose;
      E1->phant_var_F = 0;
      Get_Phantoon(k)->phant_var_F = FUNC16(Phantoon_FadeoutAfterEnrage);
    }
  }
}

void Phantoon_FadeoutAfterEnrage(uint16 k) {  // 0xA7D916
  Phantoon_FadeOut(0xC);
  if (Get_Phantoon(0x40)->phant_var_F) {
    Enemy_Phantoon *E = Get_Phantoon(k);
    E->phant_var_F = FUNC16(Phantoon_WaitAfterFadeOut);
    E->phant_var_E = 120;
  }
}

void Phantoon_CompleteSwoopAfterFatalShot(uint16 k) {  // 0xA7D92E
  Phantoon_ChangeEyeSpriteBasedOnSamusDist();
  Phantoon_MoveInSwoopingPattern(k);
  uint16 x_pos = Get_Phantoon(0)->base.x_pos;
  if (!sign16(x_pos - 96)) {
    if (sign16(x_pos - 160))
      Get_Phantoon(k)->phant_var_F = FUNC16(Phantoon_DyingPhantoonFadeInOut);
  }
}

void Phantoon_DyingPhantoonFadeInOut(uint16 k) {  // 0xA7D948
  Enemy_Phantoon *E1 = Get_Phantoon(0x40);
  if ((E1->phant_var_C & 1) != 0) {
    Phantoon_FadeIn(0xC);
    if (!E1->phant_var_F)
      return;
  } else {
    Phantoon_FadeOut(0xC);
    if (!E1->phant_var_F)
      return;
  }
  E1->phant_var_F = 0;
  uint16 v2 = E1->phant_var_C + 1;
  E1->phant_var_C = v2;
  if (!sign16(v2 - 10)) {
    Enemy_Phantoon *E = Get_Phantoon(k);
    E->phant_var_F = FUNC16(Phantoon_DyingPhantoonExplosions);
    E->phant_var_E = 15;
    Get_Phantoon(0x80)->phant_var_F = 0;
    Get_Phantoon(0)->phant_var_A = 0;
  }
}

void Phantoon_DyingPhantoonExplosions(uint16 k) {  // 0xA7D98B
  Enemy_Phantoon *EK = Get_Phantoon(k);
  bool v2 = EK->phant_var_E == 1;
  bool v3 = (--EK->phant_var_E & 0x8000) != 0;
  if (v2 || v3) {
    Enemy_Phantoon *E2 = Get_Phantoon(0x80);
    const PhantoonExplosion *ex = &kPhantoonExplosions[E2->phant_var_F];
    Enemy_Phantoon *E0 = Get_Phantoon(0);
    eproj_spawn_pt = (Point16U){ ex->x + E0->base.x_pos, ex->y + E0->base.y_pos };
    uint16 v11 = ex->type;
    SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, v11);
    if (v11 == 29)
      QueueSfx2_Max6(kSfx2_SmallExplosion);
    else
      QueueSfx2_Max6(kSfx2_PhantoonExploding);
    EK->phant_var_E = ex->timer;
    uint16 v9 = E2->phant_var_F + 1;
    E2->phant_var_F = v9;
    if (!sign16(v9 - 13)) {
      E2->phant_var_F = 5;
      uint16 v10 = E0->phant_var_A + 1;
      E0->phant_var_A = v10;
      if (!sign16(v10 - 3))
        EK->phant_var_F = FUNC16(Phantoon_WavyDyingPhantoonAndCry);
    }
  }
}

void Phantoon_WavyDyingPhantoonAndCry(uint16 k) {  // 0xA7DA51
  Enemy_Phantoon *Phantoon; // r10

  sub_88E487(1, kPhantoonWavyPhaseDelta);
  Get_Phantoon(0xC0)->phant_var_D = 0;
  Get_Phantoon(k)->phant_var_F = FUNC16(Phantoon_DyingFadeOut);
  Phantoon = Get_Phantoon(0x40);
  Phantoon->phant_var_C = 2;
  uint16 v2 = Get_Phantoon(0)->base.properties & ~(kEnemyProps_ProcessInstructions | kEnemyProps_Intangible | kEnemyProps_Invisible) | kEnemyProps_Intangible | kEnemyProps_Invisible;
  Phantoon->base.properties = v2;
  Get_Phantoon(0x80)->base.properties = v2;
  Get_Phantoon(0xC0)->base.properties = v2;
  QueueSfx2_Max6(kSfx2_PhantoonDyingCry);
}

void Phantoon_DyingFadeOut(uint16 k) {  // 0xA7DA86
  Phantoon_Func_3(__PAIR32__(kPhantoonWavyDyingMaxAmp, kPhantoonWavyDyingAmpDelta));
  Enemy_Phantoon *E = Get_Phantoon(0x40);
  if (E->phant_var_C == 0xFFFF) {
    Phantoon_FadeOut(0xC);
    if (E->phant_var_F)
      Get_Phantoon(k)->phant_var_F = FUNC16(Phantoon_AlmostDead);
  } else if ((nmi_frame_counter_word & 0xF) == 0) {
    if (LOBYTE(E->phant_var_C) == 0xF2) {
      E->phant_var_C = -1;
      E->phant_var_F = 0;
    } else {
      uint8 v2 = LOBYTE(E->phant_var_C) + 16;
      LOBYTE(E->phant_var_C) = v2;
      reg_MOSAIC = v2;
    }
  }
}

void Phantoon_AlmostDead(uint16 k) {  // 0xA7DAD7
  reg_MOSAIC = 0;
  Enemy_Phantoon *E1 = Get_Phantoon(0x40);
  E1->phant_parameter_1 = 0;
  phantom_related_layer_flag &= ~kPhantoonLayer_SemiTransparent;
  Get_Phantoon(0xC0)->phant_parameter_1 = -1;
  Enemy_Phantoon *EK = Get_Phantoon(k);
  EK->phant_var_F = FUNC16(Phantoon_Dead);
  EK->phant_var_E = 60;
  E1->phant_var_F = 0;
  Enemy_Phantoon *E0 = Get_Phantoon(0);
  E0->base.x_pos = 384;
  E0->base.y_pos = 128;
  for (int i = 1022; i >= 0; i -= 2)
    tilemap_stuff[i >> 1] = kPhantoonBg2TilemapFill;
  uint16 v5 = vram_write_queue_tail;
  gVramWriteEntry(vram_write_queue_tail)->size = 1024;
  v5 += 2;
  gVramWriteEntry(v5)->size = 0x2000;
  v5 += 2;
  LOBYTE(gVramWriteEntry(v5++)->size) = 126;
  gVramWriteEntry(v5)->size = 18432;
  vram_write_queue_tail = v5 + 2;
}

void Phantoon_Dead(uint16 k) {  // 0xA7DB3D
  Enemy_Phantoon *EK = Get_Phantoon(k);
  if (EK->phant_var_E) {
    --EK->phant_var_E;
  } else if ((nmi_frame_counter_word & 3) == 0) {
    Enemy_Phantoon *E1 = Get_Phantoon(0x40);
    E1->phant_var_D = 12;
    if (Phantoon_Func_9() & 1) {
      reg_TM |= 2;
      Enemy_ItemDrop_Phantoon(k);
      Enemy_Phantoon *E0 = Get_Phantoon(0);
      uint16 v4 = E0->base.properties | kEnemyProps_Deleted;
      E0->base.properties = v4;
      E1->base.properties = v4;
      Get_Phantoon(0x80)->base.properties = v4;
      Get_Phantoon(0xC0)->base.properties = v4;
      *(uint16 *)&boss_bits_for_area[area_index] |= 1;
      SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x00, 0x06, 0xb78b });
      QueueMusic_Delayed8(kPhantoonMusic_Defeated);
    }
  }
}

uint8 Phantoon_Func_8(void) {  // 0xA7DB9A
  Enemy_Phantoon *E = Get_Phantoon(0x40);
  if ((uint16)(E->phant_var_D + 1) >= E->phant_var_E) {
    for (int i = 0; i < 0x20; i += 2) {
      palette_buffer[(i >> 1) + 112] =
        Phantoon_Func_10_CalculateNthTransitionColorFromXtoY(E->phant_var_E,
          palette_buffer[(i >> 1) + 112],
          kPhantoonFadeOutPalette[i >> 1]);
    }
    ++E->phant_var_E;
    return 0;
  } else {
    E->phant_var_E = 0;
    return 1;
  }
}

uint8 Phantoon_SetColorBasedOnHp(void) {  // 0xA7DBD5
  PairU16 Entry;

  Enemy_Phantoon *E = Get_Phantoon(0x40);
  if ((uint16)(E->phant_var_D + 1) >= E->phant_var_E) {
    uint16 v2 = 0, v7;
    do {
      v7 = v2;
      Entry = Phantoon_SetColorBasedOnHp_FindEntry(v2);
      uint16 j = Entry.j;
      uint16 v5 = palette_buffer[(Entry.k >> 1) + 112];
      palette_buffer[(v7 >> 1) + 112] = Phantoon_Func_10_CalculateNthTransitionColorFromXtoY(E->phant_var_E, v5, j);
      v2 = v7 + 2;
    } while ((uint16)(v7 + 2) < 0x20);
    ++E->phant_var_E;
    return 0;
  } else {
    E->phant_var_E = 0;
    return 1;
  }
}

PairU16 Phantoon_SetColorBasedOnHp_FindEntry(uint16 k) {  // 0xA7DC0F
  uint16 r24 = k;
  uint16 r18 = kPhantoonHealthStep, r20 = kPhantoonHealthStep;
  uint16 r22 = 0;
  do {
    if ((int16)(r20 - Get_Phantoon(cur_enemy_index)->base.health) >= 0)
      break;
    r20 += r18;
    ++r22;
  } while (sign16(r22 - 7));
  return MakePairU16(r24, kPhantoonHealthPalettes[r22][r24 >> 1]);
}

uint8 Phantoon_Func_9(void) {  // 0xA7DC5A
  Enemy_Phantoon *E = Get_Phantoon(0x40);
  if ((uint16)(E->phant_var_D + 1) >= E->phant_var_E) {
    for (int i = 0; i < 0xE0; i += 2) {
      palette_buffer[i >> 1] = Phantoon_Func_10_CalculateNthTransitionColorFromXtoY(
        E->phant_var_E,
        palette_buffer[i >> 1],
        kWreckedShipPowerOnPalettes[i >> 1]);
    }
    ++E->phant_var_E;
    return 0;
  } else {
    E->phant_var_E = 0;
    return 1;
  }
}

uint16 Phantoon_Func_10_CalculateNthTransitionColorFromXtoY(uint16 a, uint16 k, uint16 j) {  // 0xA7DC95
  return Phantoon_CalculateNthTransitionColorComponentFromXtoY(a, k & 0x1F, j & 0x1F) + 
         (Phantoon_CalculateNthTransitionColorComponentFromXtoY(a, (k >> 5) & 0x1F, (j >> 5) & 0x1F) << 5) +
         (Phantoon_CalculateNthTransitionColorComponentFromXtoY(a, (k >> 10) & 0x1F, (j >> 10) & 0x1F) << 10);
}

uint16 Phantoon_CalculateNthTransitionColorComponentFromXtoY(uint16 a, uint16 k, uint16 j) {  // 0xA7DCF1
  if (!a)
    return k;
  uint16 v4 = a - 1;
  Enemy_Phantoon *E = Get_Phantoon(0x40);
  if (v4 == E->phant_var_D)
    return j;
  uint16 r20 = v4 + 1;
  uint16 r18 = j - k;
  uint8 v6 = abs16(r18);
  uint16 RegWord = SnesDivide(v6 << 8, LOBYTE(E->phant_var_D) - r20 + 1);
  r18 = sign16(r18) ? -RegWord : RegWord;
  return (uint16)(r18 + swap16(k)) >> 8;
}

static void Phantoon_RestoreHealthPalette(void) {
  uint16 v1 = 30;
  PairU16 Entry;
  do {
    Entry = Phantoon_SetColorBasedOnHp_FindEntry(v1);
    palette_buffer[(Entry.k >> 1) + 112] = Entry.j;
    v1 = Entry.k - 2;
  } while ((int16)(Entry.k - 2) >= 0);
  Enemy_Phantoon *E3 = Get_Phantoon(0x80);
  E3->phant_parameter_2 = LOBYTE(E3->phant_parameter_2);
}

void Phantoon_Hurt(void) {  // 0xA7DD3F
  Enemy_Phantoon *E0 = Get_Phantoon(0);
  if (E0->base.flash_timer == 8) {
    Phantoon_RestoreHealthPalette();
    return;
  }
  if ((E0->base.frame_counter & 2) == 0) {
    if (HIBYTE(Get_Phantoon(0x80)->phant_parameter_2))
      Phantoon_RestoreHealthPalette();
    return;
  }
  if (!HIBYTE(Get_Phantoon(0x80)->phant_parameter_2)) {
    for (int i = 30; i >= 0; i -= 2)
      palette_buffer[(i >> 1) + 112] = 0x7FFF;
    Enemy_Phantoon *E3 = Get_Phantoon(0x80);
    E3->phant_parameter_2 |= 0x100;
  }
}

void Phantoon_Touch(void) {  // 0xA7DD95
  NormalEnemyTouchAiSkipDeathAnim_CurEnemy();
}

static void Phantoon_CloseEyeAfterHeavyShot(uint16 k) {
  Enemy_Phantoon *EK = Get_Phantoon(k);
  Enemy_Phantoon *E0 = Get_Phantoon(0);
  EK->phant_var_E = 0;
  Get_Phantoon(0x80)->phant_var_A = 0;
  Get_Phantoon(k + 0x80)->phant_var_B = 0;
  phantom_related_layer_flag |= kPhantoonLayer_SemiTransparent;
  E0->base.instruction_timer = 1;
  Enemy_Phantoon *E1 = Get_Phantoon(0x40);
  E1->base.instruction_timer = 1;
  E0->base.current_instruction = addr_kPhantoon_Ilist_BodyInvulnerable;
  E1->base.current_instruction = addr_kPhantoon_Ilist_EyeClose;
  E0->base.properties |= kEnemyProps_Intangible;
  E1->phant_var_F = 0;
}

void Phantoon_Shot(void) {  // 0xA7DD9B
  Enemy_Phantoon *E0 = Get_Phantoon(0);
  if (!sign16(E0->phant_var_F + 0x26B8))
    return;
  uint16 k = cur_enemy_index;
  Enemy_Phantoon *EK = Get_Phantoon(cur_enemy_index);
  uint16 health = EK->base.health;
  NormalEnemyShotAiSkipDeathAnim_CurEnemy();
  if (!EK->base.health) {
    QueueSfx2_Max6(kSfx2_PhantoonCry);
    Get_Phantoon(0x80)->phant_parameter_2 = 1;
    E0->base.properties |= kEnemyProps_Intangible;
    Phantoon_StartDeathSequence(k);
    return;
  }
  if ((EK->base.ai_handler_bits & 2) == 0)
    return;
  QueueSfx2_Max6(kSfx2_PhantoonCry);
  uint16 phanto_var_F = EK->phant_var_F;
  bool eye_open = phanto_var_F == FUNC16(Phantoon_EyeFollowsSamusUntilTimerRunsOut)
      || phanto_var_F == FUNC16(Phantoon_FollowSamusWithEyeDuringFireballRain);
  bool swooping = phanto_var_F == FUNC16(Phantoon_IsSwooping);
  if (!eye_open && !swooping) {
    Get_Phantoon(0x80)->phant_parameter_2 = 2;
    return;
  }
  health -= EK->base.health;
  bool super_missile = (projectile_type[collision_detection_index] & kProjectileType_TypeMask) == kProjectileType_SuperMissile;
  bool heavy = !sign16(health - kPhantoonEnrageDamage) && super_missile;
  if (swooping && !heavy) {
    Enemy_Phantoon *E2 = Get_Phantoon(k + 0x80);
    uint16 v7 = health + E2->phant_var_B;
    E2->phant_var_B = v7;
    if (!sign16(v7 - kPhantoonEnrageDamage))
      EK->phant_var_E = 1;
    Get_Phantoon(0x80)->phant_parameter_2 = 2;
    return;
  }
  if (heavy) {
    EK->phant_var_F = FUNC16(Phantoon_FadeOutBeforeEnrage);
    Phantoon_CloseEyeAfterHeavyShot(k);
    Get_Phantoon(0x80)->phant_parameter_2 = 2;
    return;
  }
  Enemy_Phantoon *E2 = Get_Phantoon(k + 0x80);
  uint16 v9 = health + E2->phant_var_B;
  E2->phant_var_B = v9;
  if (!sign16(v9 - kPhantoonEnrageDamage)) {
    EK->phant_var_F = FUNC16(Phantoon_FadeoutWithSwoop);
    Phantoon_CloseEyeAfterHeavyShot(k);
    Get_Phantoon(0x80)->phant_parameter_2 = 2;
    return;
  }
  uint16 v10 = NextRandom() & 7;
  Get_Phantoon(0x40)->phant_var_B = kPhantoonShotUnused0FEA[v10];
  Get_Phantoon(0xC0)->phant_parameter_2 = v10;
  Enemy_Phantoon *E2x = Get_Phantoon(0x80);
  E2x->phant_parameter_2 = 1;
  if (!E2x->phant_var_A) {
    E2x->phant_var_A = 1;
    if (!sign16(EK->phant_var_E - 16))
      EK->phant_var_E = 16;
  }
}

void Etecoon_Init(void) {  // 0xA7E912
  Enemy_Etecoon *E = Get_Etecoon(cur_enemy_index);
  E->base.properties |= kEnemyProps_ProcessInstructions;
  E->base.spritemap_pointer = addr_kSpritemap_Nothing_A7;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->base.current_instruction = addr_kEtecoon_Ilist_E8CE;
  E->etecoon_var_F = FUNC16(Etecoon_Func_4);
  E->etecoon_var_E = -1;
}

void Etecoon_Main(void) {  // 0xA7E940
  Enemy_Etecoon *E = Get_Etecoon(cur_enemy_index);
  if (HIBYTE(E->etecoon_parameter_2))
    E->etecoon_parameter_2 -= 256;
  else
    EnemyRunPreInstr(E->etecoon_var_F);
}

void Etecoon_Func_1(uint16 k) {  // 0xA7E958
  if (earthquake_timer) {
    Enemy_Etecoon *E = Get_Etecoon(k);
    E->etecoon_parameter_2 = LOBYTE(E->etecoon_parameter_2) | 0x8000;
    E->base.instruction_timer += 128;
  }
}

uint8 Etecoon_Func_2(uint16 k) {  // 0xA7E974
  Enemy_Etecoon *E = Get_Etecoon(k);
  return Enemy_MoveRight_IgnoreSlopes(k, __PAIR32__(E->etecoon_var_C, E->etecoon_var_D));
}

uint8 Etecoon_Func_3(uint16 k) {  // 0xA7E983
  Enemy_Etecoon *E = Get_Etecoon(k);
  int32 amt = __PAIR32__(E->etecoon_var_A, E->etecoon_var_B);
  if (sign16(E->etecoon_var_A - 5))
    AddToHiLo(&E->etecoon_var_A, &E->etecoon_var_B, __PAIR32__(samus_y_accel, samus_y_subaccel));
  return Enemy_MoveDown(k, amt);
}

void Etecoon_Func_4(uint16 k) {  // 0xA7E9AF
  if (!door_transition_flag_enemies) {
    Enemy_Etecoon *E = Get_Etecoon(k);
    if ((E->etecoon_var_E & 0x8000) == 0) {
      bool v2 = E->etecoon_var_E == 1;
      bool v3 = (--E->etecoon_var_E & 0x8000) != 0;
      if (v2 || v3) {
        E->base.current_instruction = addr_kEtecoon_Ilist_E854;
        E->etecoon_var_F = FUNC16(Etecoon_Func_5);
        E->etecoon_var_E = 11;
      }
    } else if (IsSamusWithinEnemy_Y(k, 0x80)) {
      if ((E->etecoon_parameter_2 & 3) == 0)
        QueueSfx2_Max15(kSfx2_EtecoonTheme);
      E->base.instruction_timer = 1;
      E->base.current_instruction = addr_kEtecoon_Ilist_E8D6;
      E->etecoon_var_E = 256;
    }
  }
}

void Etecoon_Func_5(uint16 k) {  // 0xA7EA00
  Enemy_Etecoon *E = Get_Etecoon(k);
  bool v2 = E->etecoon_var_E == 1;
  bool v3 = (--E->etecoon_var_E & 0x8000) != 0;
  if (v2 || v3) {
    E->etecoon_var_A = kEtecoonHopYSpeed;
    E->etecoon_var_B = kEtecoonHopYSubspeed;
    E->base.current_instruction += 2;
    E->base.instruction_timer = 1;
    E->etecoon_var_F = FUNC16(Etecoon_Func_6);
    if (!sign16(samus_x_pos - 256))
      QueueSfx2_Max6(kSfx2_EtecoonCry);
  }
}

void Etecoon_Func_6(uint16 k) {  // 0xA7EA37
  if (Etecoon_Func_3(k) & 1) {
    Enemy_Etecoon *E = Get_Etecoon(k);
    if ((E->etecoon_var_A & 0x8000) == 0) {
      if (IsSamusWithinEnemy_Y(k, 0x40) && IsSamusWithinEnemy_X(k, kEtecoonSamusXThreshold)) {
        uint16 v2 = DetermineDirectionOfSamusFromEnemy();
        if (sign16(v2 - 5)) {
          E->base.current_instruction = addr_kEtecoon_Ilist_E81E;
          E->etecoon_parameter_1 = 0;
        } else {
          E->base.current_instruction = addr_kEtecoon_Ilist_E876;
          E->etecoon_parameter_1 = 1;
        }
        E->etecoon_var_E = 32;
        E->base.instruction_timer = 1;
        E->etecoon_var_F = FUNC16(Etecoon_Func_7);
      } else {
        E->etecoon_var_E = 11;
        E->etecoon_var_F = FUNC16(Etecoon_Func_5);
        E->base.instruction_timer = 1;
        E->base.current_instruction = addr_kEtecoon_Ilist_E854;
      }
    } else {
      E->etecoon_var_A = 0;
      E->etecoon_var_B = 0;
      E->base.instruction_timer = 3;
      E->base.current_instruction = addr_stru_A7E862;
    }
  }
}

void Etecoon_Func_7(uint16 k) {  // 0xA7EAB5
  Enemy_Etecoon *E = Get_Etecoon(k);
  bool v2 = E->etecoon_var_E == 1;
  bool v3 = (--E->etecoon_var_E & 0x8000) != 0;
  if (v2 || v3) {
    E->base.current_instruction += 2;
    E->base.instruction_timer = 1;
    if (E->etecoon_parameter_1) {
      E->etecoon_var_C = kEtecoonXSpeedRight;
      E->etecoon_var_D = kEtecoonXSubspeedRight;
      E->etecoon_var_F = FUNC16(Etecoon_Func_9);
    } else {
      E->etecoon_var_C = kEtecoonXSpeedLeft;
      E->etecoon_var_D = kEtecoonXSubspeedLeft;
      E->etecoon_var_F = FUNC16(Etecoon_Func_8);
    }
    E->etecoon_var_A = kEtecoonHopYSpeed;
    E->etecoon_var_B = kEtecoonHopYSubspeed;
  }
}

void Etecoon_Func_8(uint16 k) {  // 0xA7EB02
  if (Etecoon_Func_2(k) & 1) {
    Enemy_Etecoon *E = Get_Etecoon(k);
    E->etecoon_var_C = kEtecoonXSpeedRight;
    E->etecoon_var_D = kEtecoonXSubspeedRight;
    E->etecoon_var_F = FUNC16(Etecoon_Func_9);
    E->base.instruction_timer = 1;
    E->base.current_instruction = addr_stru_A7E880;
    E->etecoon_parameter_1 = 1;
  }
}

void Etecoon_Func_9(uint16 k) {  // 0xA7EB2C
  if (EnemyFunc_BBBF(k, __PAIR32__(32, 0))) {
    Enemy_Etecoon *E = Get_Etecoon(k);
    E->base.instruction_timer = 1;
    E->base.current_instruction = addr_stru_A7E898;
    E->etecoon_var_F = FUNC16(Etecoon_Func_10);
  } else {
    Etecoon_Func_2(k);
  }
}

void Etecoon_Func_10(uint16 k) {  // 0xA7EB50
  Etecoon_Func_1(k);
  if (Etecoon_Func_2(k) & 1) {
    Enemy_Etecoon *E = Get_Etecoon(k);
    if (E->etecoon_parameter_1) {
      E->base.current_instruction = addr_kEtecoon_Ilist_E870;
      E->etecoon_parameter_1 = 0;
    } else {
      E->base.current_instruction = addr_kEtecoon_Ilist_E8C8;
      E->etecoon_parameter_1 = 1;
    }
    E->base.instruction_timer = 1;
    E->etecoon_var_F = FUNC16(Etecoon_Func_11);
    E->etecoon_var_E = 8;
    if (!sign16(samus_x_pos - 256))
      QueueSfx2_Max6(kSfx2_EtecoonWallJump);
  } else if (Etecoon_Func_3(k) & 1) {
    Enemy_Etecoon *E = Get_Etecoon(k);
    if (E->etecoon_parameter_1)
      E->base.current_instruction = addr_kEtecoon_Ilist_E854;
    else
      E->base.current_instruction = addr_kEtecoon_Ilist_E8AC;
    E->base.instruction_timer = 1;
    E->etecoon_var_E = 11;
    E->etecoon_var_F = FUNC16(Etecoon_Func_12);
    E->etecoon_var_A = kEtecoonHopYSpeed;
    E->etecoon_var_B = kEtecoonHopYSubspeed;
  }
}

void Etecoon_Func_11(uint16 k) {  // 0xA7EBCD
  Etecoon_Func_1(k);
  Enemy_Etecoon *E = Get_Etecoon(k);
  bool v2 = E->etecoon_var_E == 1;
  bool v3 = (--E->etecoon_var_E & 0x8000) != 0;
  if (v2 || v3) {
    if (E->etecoon_parameter_1) {
      E->base.current_instruction = addr_kEtecoon_Ilist_E894;
      E->etecoon_var_C = kEtecoonXSpeedRight;
      E->etecoon_var_D = kEtecoonXSubspeedRight;
    } else {
      E->base.current_instruction = addr_kEtecoon_Ilist_E83C;
      E->etecoon_var_C = kEtecoonXSpeedLeft;
      E->etecoon_var_D = kEtecoonXSubspeedLeft;
    }
    E->base.instruction_timer = 1;
    E->etecoon_var_F = FUNC16(Etecoon_Func_10);
    E->etecoon_var_A = kEtecoonHopYSpeed;
    E->etecoon_var_B = kEtecoonHopYSubspeed;
  }
}

static Func_Y_V *const funcs_A2A79[3] = { Etecoon_Func_13, Etecoon_Func_14, Etecoon_Func_15 };

void Etecoon_Func_12(uint16 k) {  // 0xA7EC1B
  Enemy_Etecoon *E = Get_Etecoon(k);
  bool v2 = E->etecoon_var_E == 1;
  bool v3 = (--E->etecoon_var_E & 0x8000) != 0;
  if (v2 || v3) {
    E->etecoon_var_A = kEtecoonHopYSpeed;
    E->etecoon_var_B = kEtecoonHopYSubspeed;
    funcs_A2A79[LOBYTE(E->etecoon_parameter_2)](k);
    Get_Etecoon(k)->base.instruction_timer = 1;
  }
}

void Etecoon_Func_13(uint16 j) {  // 0xA7EC47
  Enemy_Etecoon *E = Get_Etecoon(j);
  E->etecoon_var_F = FUNC16(Etecoon_Func_16);
  E->base.current_instruction = addr_stru_A7E828;
  E->etecoon_var_C = kEtecoonXSpeedLeft;
  E->etecoon_var_D = kEtecoonXSubspeedLeft;
}

void Etecoon_Func_14(uint16 j) {  // 0xA7EC61
  Enemy_Etecoon *E = Get_Etecoon(j);
  E->etecoon_var_F = FUNC16(Etecoon_Func_17);
  E->base.current_instruction = addr_stru_A7E880;
  E->etecoon_var_C = kEtecoonXSpeedRight;
  E->etecoon_var_D = kEtecoonXSubspeedRight;
}

void Etecoon_Func_15(uint16 j) {  // 0xA7EC7B
  Enemy_Etecoon *E = Get_Etecoon(j);
  E->etecoon_var_F = FUNC16(Etecoon_Func_22);
  E->base.current_instruction += 2;
  E->etecoon_var_C = kEtecoonXSpeedRight;
  E->etecoon_var_D = kEtecoonXSubspeedRight;
}

void Etecoon_Func_16(uint16 k) {  // 0xA7EC97
  Etecoon_Func_2(k);
  Enemy_Etecoon *E = Get_Etecoon(k);
  if (sign16(E->base.x_pos - 537)) {
    E->etecoon_var_E = 11;
    E->etecoon_var_F = FUNC16(Etecoon_Func_23);
    E->base.instruction_timer = 1;
    E->base.current_instruction = addr_kEtecoon_Ilist_E854;
  }
}

void Etecoon_Func_17(uint16 k) {  // 0xA7ECBB
  Etecoon_Func_2(k);
  Enemy_Etecoon *E = Get_Etecoon(k);
  if (!sign16(E->base.x_pos - 600)) {
    E->etecoon_var_E = 11;
    E->etecoon_var_F = FUNC16(Etecoon_Func_23);
    E->base.instruction_timer = 1;
    E->base.current_instruction = addr_kEtecoon_Ilist_E854;
  }
}

void Etecoon_Func_18(uint16 k) {  // 0xA7ECDF
  Etecoon_Func_2(k);
  Enemy_Etecoon *E = Get_Etecoon(k);
  if (!sign16(E->base.x_pos - 600)) {
    E->etecoon_var_F = FUNC16(Etecoon_Func_19);
    E->etecoon_var_A = kEtecoonJumpYSpeed;
    E->etecoon_var_B = kEtecoonJumpYSubspeed;
    E->base.instruction_timer = 1;
    E->base.current_instruction = addr_stru_A7E898;
  }
}

void Etecoon_Func_19(uint16 k) {  // 0xA7ED09
  Etecoon_Func_2(k);
  Etecoon_Func_3(k);
  Enemy_Etecoon *E = Get_Etecoon(k);
  if (!sign16(E->base.x_pos - 680)) {
    E->base.instruction_timer = 1;
    E->base.current_instruction = addr_stru_A7E880;
    E->etecoon_var_F = FUNC16(Etecoon_Func_20);
  }
}

void Etecoon_Func_20(uint16 k) {  // 0xA7ED2A
  Etecoon_Func_2(k);
  Enemy_Etecoon *E = Get_Etecoon(k);
  if (!sign16(E->base.x_pos - 840)) {
    E->base.instruction_timer = 1;
    E->base.current_instruction = addr_stru_A7E898;
    E->etecoon_var_F = FUNC16(Etecoon_Func_21);
    E->etecoon_var_A = -1;
    E->etecoon_var_B = kEtecoonJumpYSubspeed;
  }
}

void Etecoon_Func_21(uint16 k) {  // 0xA7ED54
  Etecoon_Func_2(k);
  if (Etecoon_Func_3(k) & 1) {
    Enemy_Etecoon *E = Get_Etecoon(k);
    E->etecoon_var_E = 11;
    E->base.instruction_timer = 1;
    E->base.current_instruction = addr_kEtecoon_Ilist_E854;
    E->etecoon_var_F = FUNC16(Etecoon_Func_23);
  }
}

void Etecoon_Func_22(uint16 k) {  // 0xA7ED75
  Etecoon_Func_1(k);
  if (Etecoon_Func_3(k) & 1) {
    Enemy_Etecoon *E = Get_Etecoon(k);
    if ((E->etecoon_var_A & 0x8000) == 0) {
      E->etecoon_var_E = 11;
      E->base.instruction_timer = 1;
      E->base.current_instruction = addr_kEtecoon_Ilist_E854;
      if ((E->etecoon_parameter_2 & 2) != 0 && (sign16(E->base.x_pos - 832)))
        E->etecoon_var_F = FUNC16(Etecoon_Func_24);
      else
        E->etecoon_var_F = FUNC16(Etecoon_Func_23);
    } else {
      E->etecoon_var_A = 0;
      E->etecoon_var_B = 0;
      E->base.instruction_timer = 3;
      E->base.current_instruction = addr_stru_A7E862;
    }
  }
}

void Etecoon_Func_23(uint16 k) {  // 0xA7EDC7
  Etecoon_Func_1(k);
  Enemy_Etecoon *E = Get_Etecoon(k);
  bool v2 = E->etecoon_var_E == 1;
  bool v3 = (--E->etecoon_var_E & 0x8000) != 0;
  if (v2 || v3) {
    uint16 v4 = E->etecoon_parameter_1 + 256;
    E->etecoon_parameter_1 = v4;
    if (sign16((v4 & 0xFF00) - 1024)
        || (E->etecoon_parameter_1 = LOBYTE(E->etecoon_parameter_1), (E->etecoon_parameter_2 & 2) != 0)) {
      E->etecoon_var_F = FUNC16(Etecoon_Func_22);
      E->base.current_instruction += 2;
    } else {
      E->etecoon_var_F = FUNC16(Etecoon_Func_25);
      E->base.current_instruction = addr_stru_A7E880;
      E->etecoon_var_C = kEtecoonXSpeedRight;
      E->etecoon_var_D = kEtecoonXSubspeedRight;
    }
    E->etecoon_var_A = kEtecoonHopYSpeed;
    E->etecoon_var_B = kEtecoonHopYSubspeed;
    E->base.instruction_timer = 1;
    if (!sign16(samus_x_pos - 256))
      QueueSfx2_Max6(kSfx2_EtecoonCry);
  }
}

void Etecoon_Func_24(uint16 k) {  // 0xA7EE3E
  Etecoon_Func_1(k);
  Enemy_Etecoon *E = Get_Etecoon(k);
  bool v2 = E->etecoon_var_E == 1;
  bool v3 = (--E->etecoon_var_E & 0x8000) != 0;
  if (v2 || v3) {
    if (IsSamusWithinEnemy_Y(k, 0x40) && IsSamusWithinEnemy_X(k, 0x30)) {
      E->base.current_instruction = addr_stru_A7E880;
      E->etecoon_var_F = FUNC16(Etecoon_Func_18);
    } else {
      E->etecoon_var_A = kEtecoonHopYSpeed;
      E->etecoon_var_B = kEtecoonHopYSubspeed;
      E->base.current_instruction += 2;
      E->etecoon_var_F = FUNC16(Etecoon_Func_22);
      if (!sign16(samus_x_pos - 256))
        QueueSfx2_Max6(kSfx2_EtecoonCry);
    }
    E->base.instruction_timer = 1;
  }
}

void Etecoon_Func_25(uint16 k) {  // 0xA7EE9A
  Etecoon_Func_2(k);
  Enemy_Etecoon *E = Get_Etecoon(k);
  if (!sign16(E->base.x_pos - 600)) {
    E->etecoon_var_F = FUNC16(Etecoon_Func_26);
    E->base.instruction_timer = 1;
    E->base.current_instruction = addr_stru_A7E898;
  }
}

void Etecoon_Func_26(uint16 k) {  // 0xA7EEB8
  Etecoon_Func_2(k);
  if (Etecoon_Func_3(k) & 1) {
    Enemy_Etecoon *E = Get_Etecoon(k);
    E->etecoon_var_C = kEtecoonXSpeedLeft;
    E->etecoon_var_D = kEtecoonXSubspeedLeft;
    E->etecoon_var_F = FUNC16(Etecoon_Func_8);
    E->etecoon_var_A = kEtecoonHopYSpeed;
    E->etecoon_var_B = kEtecoonHopYSubspeed;
    E->base.instruction_timer = 1;
    E->base.current_instruction = addr_stru_A7E828;
  }
}

void Dachora_Init(void) {  // 0xA7F4DD
  int16 dachor_parameter_1;

  Enemy_Dachora *E = Get_Dachora(cur_enemy_index);
  E->base.properties |= kEnemyProps_ProcessInstructions;
  E->base.spritemap_pointer = addr_kSpritemap_Nothing_A7;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  dachor_parameter_1 = E->dachor_parameter_1;
  if (dachor_parameter_1 < 0) {
    if ((dachor_parameter_1 & 1) != 0)
      E->base.current_instruction = addr_kDachora_Ilist_F4B9;
    else
      E->base.current_instruction = addr_kDachora_Ilist_F3F7;
    E->dachor_var_F = FUNC16(Dachora_Func_12);
  } else {
    if (dachor_parameter_1)
      E->base.current_instruction = addr_kDachora_Ilist_F45B;
    else
      E->base.current_instruction = addr_kDachora_Ilist_F399;
    E->dachor_var_F = FUNC16(Dachora_Func_2);
  }
}

void Dachora_Main(void) {  // 0xA7F52E
  Enemy_Dachora *E = Get_Dachora(cur_enemy_index);
  EnemyRunPreInstr(E->dachor_var_F);
}
void Dachora_Func_1(const uint16 *src, uint16 k) {  // 0xA7F535
  Enemy_Dachora *E = Get_Dachora(k);
  uint16 v3 = swap16(E->base.palette_index);
  memcpy(g_ram + kDachoraPaletteDest[v3 >> 1], src, 32);
}

void Dachora_Func_2(uint16 k) {  // 0xA7F570
  Enemy_MoveDown(k, __PAIR32__(1, 0));
  if (IsSamusWithinEnemy_Y(k, 0x40) && IsSamusWithinEnemy_X(k, kDachoraSamusXThreshold)) {
    Enemy_Dachora *E = Get_Dachora(k);
    if (E->dachor_parameter_1)
      E->base.current_instruction = addr_kDachora_Ilist_F48B;
    else
      E->base.current_instruction = addr_kDachora_Ilist_F3C9;
    E->base.instruction_timer = 1;
    E->dachor_var_F = FUNC16(Dachora_Func_3);
    E->dachor_var_A = kDachoraBlinkDuration;
    QueueSfx2_Max15(kSfx2_DachoraCry);
  }
}

void Dachora_Func_3(uint16 k) {  // 0xA7F5BC
  Enemy_Dachora *E = Get_Dachora(k);
  if (E->dachor_var_A-- == 1) {
    if (E->dachor_parameter_1) {
      E->base.current_instruction = addr_kDachora_Ilist_F407;
      E->dachor_var_F = FUNC16(Dachora_Func_5);
    } else {
      E->base.current_instruction = addr_kDachora_Ilist_F345;
      E->dachor_var_F = FUNC16(Dachora_Func_4);
    }
    E->base.instruction_timer = 1;
    E->dachor_var_E = 1;
  }
}

void Dachora_Func_4(uint16 k) {  // 0xA7F5ED
  int32 amt = -Dachora_Func_6(k);
  Enemy_Dachora *E = Get_Dachora(k);
  if (Enemy_MoveRight_IgnoreSlopes(k, amt) || (EnemyFunc_C8AD(k), sign16(E->base.x_pos - 96))) {
    E->base.current_instruction = addr_kDachora_Ilist_F407;
    E->dachor_var_F = FUNC16(Dachora_Func_5);
    E->dachor_var_E = 1;
    E->dachor_parameter_1 = 1;
    E->base.instruction_timer = 1;
    E->dachor_var_A = 0;
    E->dachor_var_B = 0;
    Dachora_Func_1(kDachoraPalette, k);
  }
}

void Dachora_Func_5(uint16 k) {  // 0xA7F65E
  int32 amt = Dachora_Func_6(k);
  Enemy_Dachora *E = Get_Dachora(k);
  if (Enemy_MoveRight_IgnoreSlopes(k, amt)) {
    QueueSfx2_Max15(kSfx2_Silence);
    E->base.current_instruction = addr_kDachora_Ilist_F345;
    E->dachor_var_F = FUNC16(Dachora_Func_4);
    E->dachor_parameter_1 = 0;
    E->dachor_var_A = 0;
    Dachora_Func_1(kDachoraPalette, k);
    E->dachor_var_E = 0;
    E->dachor_var_B = 0;
    E->base.instruction_timer = 1;
    return;
  }
  EnemyFunc_C8AD(k);
  if (!sign16(E->base.x_pos - 1152)) {
    E->base.current_instruction = addr_kDachora_Ilist_F4B3;
    E->dachor_var_F = FUNC16(Dachora_Func_7);
    E->dachor_var_A = kDachoraShinesparkDelay;
    E->base.y_pos += 8;
    QueueSfx2_Max6(kSfx2_DachoraStoredShinespark);
    E->dachor_var_E = 0;
    E->dachor_var_B = 0;
    E->base.instruction_timer = 1;
  }
}

int32 Dachora_Func_6(uint16 k) {  // 0xA7F6D5
  Enemy_Dachora *E = Get_Dachora(k);
  if ((int16)(E->dachor_var_A - kDachoraMaxXSpeed) >= 0) {
    if (E->dachor_var_E == 1)
      QueueSfx2_Max6(kSfx2_DachoraSpeedBooster);
    uint16 v2 = E->dachor_var_E - 1;
    E->dachor_var_E = v2;
    if (!(uint8)v2) {
      Dachora_Func_1(kDachoraSpeedBoostPalettes[HIBYTE(E->dachor_var_E)], k);
      uint16 v3 = E->dachor_var_E + 272;
      E->dachor_var_E = v3;
      if (!sign16(v3 - 1040))
        E->dachor_var_E = 784;
    }
  }
  Enemy_MoveDown(k, INT16_SHL16(1));
  if ((int16)(E->dachor_var_A - kDachoraMaxXSpeed) >= 0 && (int16)(E->dachor_var_B - kDachoraMaxXSubspeed) >= 0) {
    E->dachor_var_A = kDachoraMaxXSpeed;
    E->dachor_var_B = kDachoraMaxXSubspeed;
    return __PAIR32__(kDachoraMaxXSpeed, kDachoraMaxXSubspeed);
  }
  AddToHiLo(&E->dachor_var_A, &E->dachor_var_B, __PAIR32__(kDachoraXAccel, kDachoraXSubaccel));
  if ((E->dachor_var_A == 4 || E->dachor_var_A == 8) && !E->dachor_var_B)
    E->base.current_instruction += 28;
  return __PAIR32__(E->dachor_var_A, E->dachor_var_B);
}

void Dachora_Func_7(uint16 k) {  // 0xA7F78F
  Dachora_Func_10(k);
  Enemy_Dachora *E = Get_Dachora(k);
  if (E->dachor_var_A-- == 1) {
    E->base.current_instruction += 2;
    E->base.instruction_timer = 1;
    E->dachor_var_F = FUNC16(Dachora_Func_8);
    Enemy_Dachora *E1 = Get_Dachora(k + 64);
    E1->dachor_var_A = 0;
    E1->dachor_var_D = 0;
    Get_Dachora(k + 128)->dachor_var_D = 0;
    Get_Dachora(k + 192)->dachor_var_D = 0;
    Get_Dachora(k + 256)->dachor_var_D = 0;
    E->dachor_var_C = 0;
    E->dachor_var_D = 0;
    E->base.y_pos -= 8;
    QueueSfx2_Max6(kSfx2_DachoraShinespark);
    if (E->dachor_parameter_1) {
      Get_Dachora(k + 64)->base.current_instruction = addr_kDachora_Ilist_F4B9;
      Get_Dachora(k + 128)->base.current_instruction = addr_kDachora_Ilist_F4B9;
      Get_Dachora(k + 192)->base.current_instruction = addr_kDachora_Ilist_F4B9;
      Get_Dachora(k + 256)->base.current_instruction = addr_kDachora_Ilist_F4B9;
    } else {
      Get_Dachora(k + 64)->base.current_instruction = addr_kDachora_Ilist_F3F7;
      Get_Dachora(k + 128)->base.current_instruction = addr_kDachora_Ilist_F3F7;
      Get_Dachora(k + 192)->base.current_instruction = addr_kDachora_Ilist_F3F7;
      Get_Dachora(k + 256)->base.current_instruction = addr_kDachora_Ilist_F3F7;
    }
    Get_Dachora(k + 64)->base.instruction_timer = 1;
    Get_Dachora(k + 128)->base.instruction_timer = 1;
    Get_Dachora(k + 192)->base.instruction_timer = 1;
    Get_Dachora(k + 256)->base.instruction_timer = 1;
  }
}

void Dachora_Func_8(uint16 k) {  // 0xA7F806
  Dachora_Func_10(k);
  Dachora_Func_9(k);
  Enemy_Dachora *E = Get_Dachora(k);
  AddToHiLo(&E->dachor_var_C, &E->dachor_var_D, __PAIR32__(samus_y_accel, samus_y_subaccel));
  AddToHiLo(&E->dachor_var_A, &E->dachor_var_B, __PAIR32__(E->dachor_var_C, E->dachor_var_D));
  uint16 v6 = E->dachor_var_A;
  if (!sign16(v6 - 15))
    v6 = 15;
  int32 amt = __PAIR32__(v6, E->dachor_var_B);
  if (Enemy_MoveDown(k, -amt)) {
    if (E->dachor_parameter_1) {
      E->base.current_instruction = addr_kDachora_Ilist_F3FF;
      E->dachor_parameter_1 = 0;
    } else {
      E->base.current_instruction = addr_kDachora_Ilist_F4C1;
      E->dachor_parameter_1 = 1;
    }
    E->dachor_var_F = FUNC16(Dachora_Func_11);
    E->base.instruction_timer = 1;
    E->dachor_var_A = 0;
    E->dachor_var_B = 0;
    E->dachor_var_E = 0;
    Dachora_Func_1(kDachoraPalette, k);
    QueueSfx2_Max6(kSfx2_DachoraShinesparkEnded);
  }
}

void Dachora_Func_9(uint16 k) {  // 0xA7F89A

  Enemy_Dachora *EK = Get_Dachora(k);
  Enemy_Dachora *E1 = Get_Dachora(k + 64);
  uint16 dachor_var_A = E1->dachor_var_A;
  if (dachor_var_A) {
    E1->dachor_var_A = dachor_var_A - 1;
  } else {
    E1->dachor_var_A = kDachoraEchoUpdateInterval;
    if (E1->dachor_var_D) {
      Enemy_Dachora *E2 = Get_Dachora(k + 128);
      if (E2->dachor_var_D) {
        Enemy_Dachora *E3 = Get_Dachora(k + 192);
        if (E3->dachor_var_D) {
          Enemy_Dachora *E4 = Get_Dachora(k + 256);
          if (!E4->dachor_var_D) {
            E4->base.x_pos = EK->base.x_pos;
            E4->base.y_pos = EK->base.y_pos;
            E4->dachor_var_D = kDachoraEchoLifetime;
          }
        } else {
          E3->base.x_pos = EK->base.x_pos;
          E3->base.y_pos = EK->base.y_pos;
          E3->dachor_var_D = kDachoraEchoLifetime;
        }
      } else {
        E2->base.x_pos = EK->base.x_pos;
        E2->base.y_pos = EK->base.y_pos;
        E2->dachor_var_D = kDachoraEchoLifetime;
      }
    } else {
      E1->base.x_pos = EK->base.x_pos;
      E1->base.y_pos = EK->base.y_pos;
      E1->dachor_var_D = kDachoraEchoLifetime;
    }
  }
}

void Dachora_Func_10(uint16 k) {  // 0xA7F90A
  Enemy_Dachora *E = Get_Dachora(k);
  Dachora_Func_1(kDachoraShinePalettes[HIBYTE(E->dachor_var_E)], k);
  uint16 v2 = E->dachor_var_E + 256;
  E->dachor_var_E = v2;
  if (!sign16(v2 - 1024))
    E->dachor_var_E = 0;
}

void Dachora_Func_11(uint16 k) {  // 0xA7F935
  Enemy_Dachora *E = Get_Dachora(k);
  AddToHiLo(&E->dachor_var_A, &E->dachor_var_B, __PAIR32__(samus_y_accel, samus_y_subaccel));
  int32 amt = __PAIR32__(E->dachor_var_A, E->dachor_var_B);
  if (!sign16((amt >> 16) - 10))
    amt = INT16_SHL16(10);
  if (Enemy_MoveDown(k, amt)) {
    if (E->dachor_parameter_1) {
      E->base.current_instruction = addr_kDachora_Ilist_F407;
      E->dachor_var_F = FUNC16(Dachora_Func_5);
    } else {
      E->base.current_instruction = addr_kDachora_Ilist_F345;
      E->dachor_var_F = FUNC16(Dachora_Func_4);
    }
    E->base.instruction_timer = 1;
    E->dachor_var_A = 0;
    E->dachor_var_B = 0;
  }
}

void Dachora_Func_12(uint16 k) {  // 0xA7F98C
  Enemy_Dachora *E = Get_Dachora(k);
  uint16 dachor_var_D = E->dachor_var_D;
  bool hide;
  if (!dachor_var_D) {
    hide = true;
  } else {
    E->dachor_var_D = dachor_var_D - 1;
    if ((k & 0x40) != 0)
      hide = (nmi_frame_counter_word & 1) == 0;
    else
      hide = (nmi_frame_counter_word & 1) != 0;
  }
  if (hide)
    E->base.properties |= kEnemyProps_Invisible;
  else
    E->base.properties &= ~kEnemyProps_Invisible;
}
