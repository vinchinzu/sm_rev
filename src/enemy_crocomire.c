// Enemy AI - Crocomire boss runtime — peeled from Bank $A4
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "enemy_types.h"

enum {
  kCrocomireBossId = 6,
  kCrocomireBossBit = 2,
  kCrocomireBg2TilemapFill = 0x338,
  kCrocomireMouthCloseDelay = 8,
  kCrocomireMouthCloseDelayDuringSpit = 8,
  kCrocomireStepsBackChargedBeam = 2,
  kCrocomireStepsBackMissile = 1,
  kCrocomireStepsBackSuperMissile = 3,
  kCrocomirePowerBombReactionEnable = 3,
  kCrocomireMouthOpenTimerUncharged = 8,
  kCrocomireNearSpikeWallX = 0x300,
  kCrocomireBridgeCollapseX = 0x640,
  kCrocomireRumbleTerminator = 0x8080,
  kCrocomireShotCharged = 0x10,
  kCrocomireMusic_Song0 = 5,
  kCrocomireMusic_Song1 = 6,
  kSfx3_CrocomireSpit = 0x1C,
  kSfx3_CrocomireAcidDamage = 0x22,
  kSfx2_ShotCrocomire = 0x54,
  kSfx2_DachoraShinespark = 0x3B,
  kSfx2_CrocomireCry = 0x74,
  kSfx2_BigExplosion = 0x25,
  kSfx2_CrocomireSkeletonCollapses = 0x75,
  kSfx2_Quake = 0x76,
  kSfx2_CrocomireMeltingCry = 0x77,
  kSfx2_CrocomireDyingCry = 0x2D,
  kSfx2_CrocomirePostDeathRumble = 0x2B,
  kSfx2_CrocomireWallExplodes = 0x29,
  kSfx2_CrocomireDestroysWall = 0x30,
};

static const uint16 kCrocomireHurtFlashPalette[16] = {
  0x0000, 0x7fff, 0x0dff, 0x08bf, 0x0895, 0x086c, 0x0447, 0x6b7e,
  0x571e, 0x3a58, 0x2171, 0x0ccb, 0x039f, 0x023a, 0x0176, 0x0000,
};

static const uint16 kCrocomireSpritePalette2[17] = {
  0x3800, 0x571e, 0x6318, 0x6318, 0x6318, 0x6318, 0x6318, 0x4a7b,
  0x1c90, 0x1469, 0x1424, 0x0008, 0x24bf, 0x2495, 0x1c6c, 0x1045,
  0x3800,
};

static const uint16 kCrocomireSpritePalette5[17] = {
  0x3800, 0x7f5a, 0x033b, 0x0216, 0x0113, 0x7c1d, 0x5814, 0x300a,
  0x3be0, 0x2680, 0x1580, 0x5294, 0x39ce, 0x2108, 0x2484, 0x03e0,
  0x3800,
};

static const uint16 kCrocomireSkeletonPalette[16] = {
  0x3800, 0x7fff, 0x6b7e, 0x571e, 0x3a58, 0x2171, 0x0ccb, 0x6b7e,
  0x571e, 0x3a58, 0x2171, 0x0ccb, 0x039f, 0x023a, 0x0176, 0x0000,
};

static const uint16 kCrocomireRumblePalette[16] = {
  0x3800, 0x02df, 0x01d7, 0x00ac, 0x5a73, 0x41ad, 0x2d08, 0x1863,
  0x0bb1, 0x48fb, 0x7fff, 0x0000, 0x7fff, 0x44e5, 0x7fff, 0x0000,
};

static const uint16 kCrocomireBg2ScrollSpritemaps[17] = {
  0xbfc4, 0xbff6, 0xc028, 0xc05a, 0xc08c, 0xc0be, 0xc0f0, 0xc122,
  0xc154, 0xc186, 0xc1b8, 0xc1ea, 0xc47a, 0xc4ac, 0xc4de, 0xc510,
  0xc542,
};

static const uint16 kCrocomireBridgeCrumbleX[11] = {
  0x0780, 0x0730, 0x0790, 0x0740, 0x07b0, 0x0760, 0x07a0, 0x0770,
  0x0710, 0x0750, 0x0720,
};

static const uint16 kCrocomireMelting1Tilemap[257] = {
  0x3c00, 0x3c20, 0x3c30, 0x3c40, 0x3c50, 0x3c01, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0x3c10, 0x3c11, 0x3c04, 0x3c33, 0x3c43, 0x3c23, 0x3c41, 0x3c51, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x3c02, 0x3c14, 0x3c05, 0x3c44, 0x3c54, 0x3c24, 0x3c34, 0x3c12, 0x3c22, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x0338, 0x3c53, 0x3c15, 0x3c06, 0x3c16, 0x3c26, 0x3c36, 0x3c46, 0x3c13, 0x3c52, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x0338, 0x0338, 0x3c25, 0x3c56, 0x3c07, 0x3c17, 0x3c27, 0x3c37, 0x3c47, 0x3c03, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x0338, 0x0338, 0x0338, 0x3c57, 0x3c08, 0x3c18, 0x3c28, 0x3c38, 0x3c48, 0x3c35, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x3c32, 0x3c21, 0x3c45, 0x3c58, 0x3c09, 0x3c19, 0x3c29, 0x3c29, 0x3c49, 0x3c59, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x3c42, 0x3c31, 0x3c55, 0x3c0a, 0x3c1a, 0x3c2a, 0x3c3a, 0x3c4a, 0x3c5a, 0x3c0b, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0xffff,
};

static const uint16 kCrocomireMelting2Tilemap[257] = {
  0x3c00, 0x3c10, 0x3c20, 0x3c30, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x3c00, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0x3c40, 0x3c11, 0x3c21, 0x3c50, 0x3c23, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x3c32, 0x3c42, 0x3c04, 0x3c54, 0x3c03, 0x0338, 0x0338, 0x3c33, 0x3c01, 0x3c53, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x0338, 0x0338, 0x3c34, 0x3c44, 0x3c05, 0x3c24, 0x3c14, 0x3c02, 0x3c12, 0x3c22, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x0338, 0x0338, 0x0338, 0x3c35, 0x3c45, 0x3c55, 0x3c06, 0x3c16, 0x3c26, 0x3c36, 0x3c3a, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x3c46, 0x3c56, 0x3c07, 0x3c17, 0x3c27, 0x3c37, 0x3c47, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x3c31, 0x3c41, 0x3c51, 0x3c57, 0x3c08, 0x3c18, 0x3c28, 0x3c38, 0x3c48, 0x3c58, 0x3c09, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x3c43, 0x3c52, 0x3c19, 0x3c13, 0x3c39, 0x3c49, 0x3c59, 0x3c0a, 0x3c1a, 0x3c2a, 0x3c3a, 0x0338, 0x0338, 0x0338, 0x0338,
  0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338, 0x0338,
  0xffff,
};

static const uint16 kCrocomireMeltingLoad[90] = {
  0x0058, 0x0030, 0x0200, 0x00a4, 0xa07d, 0x4000, 0xa27d, 0x4200,
  0xa47d, 0x4400, 0xa67d, 0x4600, 0xa87d, 0x4800, 0xaa7d, 0x4a00,
  0xffff, 0x0160, 0x0000, 0x007e, 0x4000, 0x0160, 0x0100, 0x007e,
  0x4200, 0x0160, 0x0200, 0x007e, 0x4400, 0x0160, 0x0300, 0x007e,
  0x4600, 0x0160, 0x0400, 0x007e, 0x4800, 0x0160, 0x0500, 0x007e,
  0x4a00, 0xffff, 0x0058, 0x0030, 0x0200, 0x00a4, 0xac7d, 0x4000,
  0xae7d, 0x4200, 0xb07d, 0x4400, 0xb27d, 0x4600, 0xb47d, 0x4800,
  0xb67d, 0x4a00, 0xb87d, 0x4c00, 0xffff, 0x0160, 0x0000, 0x007e,
  0x4000, 0x0160, 0x0100, 0x007e, 0x4200, 0x0160, 0x0200, 0x007e,
  0x4400, 0x0160, 0x0300, 0x007e, 0x4600, 0x0160, 0x0400, 0x007e,
  0x4800, 0x0160, 0x0500, 0x007e, 0x4a00, 0x0160, 0x0600, 0x007e,
  0x4c00, 0xffff,
};

static const uint8 kCrocomireMeltingVlineOrder[49] = {
  0x2b, 0x28, 0x21, 0x1f, 0x2c, 0x10, 0x16, 0x17, 0x0f, 0x00, 0x06, 0x07, 0x0b, 0x08, 0x01, 0x2a,
  0x0c, 0x24, 0x2e, 0x2d, 0x1a, 0x14, 0x1d, 0x23, 0x1e, 0x29, 0x25, 0x22, 0x13, 0x19, 0x15, 0x12,
  0x30, 0x03, 0x09, 0x02, 0x1b, 0x05, 0x18, 0x1c, 0x11, 0x0a, 0x04, 0x0d, 0x2f, 0x0e, 0x20, 0x26,
  0x27,
};

static const uint8 kCrocomireEraseLineMasks[8] = { 0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe };

static const uint16 kCrocomireRumble[29] = {
  0x0004, 0x0001, 0x0000, 0xffff, 0x0008, 0x0001, 0x0001, 0xffff,
  0x000c, 0x0001, 0x0001, 0xfffe, 0x0010, 0x0002, 0x0002, 0xfffe,
  0x0010, 0x0002, 0x0002, 0xfffc, 0x0008, 0x0001, 0x0001, 0xfffe,
  0x0003, 0x0001, 0x0001, 0xffff, 0x8080,
};

static const uint16 kCrocomireSkeletonVramDst[7] = {
  0x1600, 0x1700, 0x1800, 0x1900, 0x1e00, 0x1f00, 0xffff,
};

static const uint16 kCrocomireSkeletonVramSrc[6] = {
  0xa600, 0xa800, 0xaa00, 0xac00, 0xae00, 0xb000,
};

static uint16 CrocomireMeltingLoadWord(uint16 byte_off) {
  return kCrocomireMeltingLoad[byte_off >> 1];
}


void Crocomire_Hurt(void) {  // 0xA48687
  Crocomire_Func_27(cur_enemy_index);
  Crocomire_Func_31();
}

typedef const uint16 *CrocomireFunc(uint16 k, const uint16 *jp);

static CrocomireFunc *const kCrocomireFightAi[21] = {  // 0xA486A6
  Crocomire_Func_2,  Crocomire_Func_3,  Crocomire_Func_4,  Crocomire_Func_5,
  Crocomire_Func_7,  Crocomire_Func_8,  Crocomire_Func_9, Crocomire_Func_10,
  Crocomire_Func_11, Crocomire_Func_13, Crocomire_Func_14, Crocomire_Func_15,
  Crocomire_Func_16, Crocomire_Func_17, Crocomire_Func_18, Crocomire_Func_21,
  Crocomire_Func_22, Crocomire_Func_23, Crocomire_Func_24, Crocomire_Func_25,
  Crocomire_Func_26,
};

const uint16 *Crocomire_Instr_1(uint16 k, const uint16 *jp) {
  uint16 crocom_var_C = Get_Crocomire(cur_enemy_index)->crocom_var_C;
  return kCrocomireFightAi[crocom_var_C >> 1](crocom_var_C, jp);
}

const uint16 *Crocomire_Func_2(uint16 k, const uint16 *jp) {  // 0xA486DE
  Get_Crocomire(k)->base.instruction_timer = 1;
  return INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BADE);
}

const uint16 *Crocomire_Func_3(uint16 k, const uint16 *jp) {  // 0xA486E8
  Get_Crocomire(0)->crocom_var_C = 4;
  return INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BBCE);
}

const uint16 *Crocomire_Func_4(uint16 k, const uint16 *jp) {  // 0xA486F2
  Enemy_Crocomire *E = Get_Crocomire(0);
  uint16 v1 = abs16(E->base.x_pos - samus_x_pos);
  if (sign16(v1 - 224)) {
    E->crocom_var_B |= 0x8000;
    E->crocom_var_C = 18;
    return INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BC56);
  }
  return jp;
}

const uint16 *Crocomire_Func_5(uint16 k, const uint16 *jp) {  // 0xA48717
  Enemy_Crocomire *E = Get_Crocomire(0);
  if ((E->crocom_var_B & 0x800) != 0 && (E->crocom_var_B &= ~0x800, E->crocom_var_D)) {
    jp = INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BC30);
    E->crocom_var_C = 12;
  } else if ((int16)(E->base.x_pos - kCrocomireNearSpikeWallX) < 0) {
    jp = INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BE7E);
    E->crocom_var_C = 10;
  } else if ((int16)(INSTR_ADDR_TO_PTR(0, jp) - addr_kCrocomire_Ilist_BC34) >= 0) {
    return INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BBCE);
  }
  return jp;
}

const uint16 *Crocomire_Instr_14(uint16 k, const uint16 *jp) {  // 0xA48752
  if (sign16((random_number & 0xFFF) - 1024)) {
    Enemy_Crocomire *E = Get_Crocomire(0);
    E->crocom_var_C = 8;
    E->crocom_var_F = 0;
    return INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BB36);
  }
  return jp;
}

const uint16 *Crocomire_Func_7(uint16 k, const uint16 *jp) {  // 0xA4876C
  Enemy_Crocomire *E = Get_Crocomire(0);
  uint16 crocom_var_B = E->crocom_var_B;
  if ((crocom_var_B & 0x800) != 0) {
    E->crocom_var_B = crocom_var_B & 0xF7FF;
    jp = INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BC30);
    E->crocom_var_C = 12;
  } else {
    uint16 crocom_var_F = E->crocom_var_F;
    if (sign16(crocom_var_F - 18)) {
      E->crocom_var_F += 2;
      SpawnEprojWithGfx(crocom_var_F, cur_enemy_index, addr_stru_868F8F);
      QueueSfx3_Max6(kSfx3_CrocomireSpit);
    } else {
      jp = INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BBCA);
      E->crocom_var_C = 6;
    }
  }
  return jp;
}

const uint16 *Crocomire_Func_8(uint16 k, const uint16 *jp) {  // 0xA487B2
  Enemy_Crocomire *E = Get_Crocomire(0);
  uint16 crocom_var_B = E->crocom_var_B;
  if ((crocom_var_B & 0x800) != 0) {
    E->crocom_var_B = crocom_var_B & 0xF7FF;
    jp = INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BC30);
    E->crocom_var_C = 12;
  }
  return jp;
}

const uint16 *Crocomire_Func_9(uint16 k, const uint16 *jp) {  // 0xA487CA
  uint16 v2;

  Enemy_Crocomire *E = Get_Crocomire(0);
  uint16 crocom_var_D = E->crocom_var_D;
  if (crocom_var_D && (v2 = crocom_var_D - 1, (E->crocom_var_D = v2) != 0)) {
    E->crocom_var_C = 12;
    return INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BC34);
  } else {
    E->crocom_var_C = 6;
    return INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BBCE);
  }
}

const uint16 *Crocomire_Func_10(uint16 k, const uint16 *jp) {  // 0xA487E9
  Enemy_Crocomire *E = Get_Crocomire(0);
  if ((int16)(E->base.x_pos - kCrocomireNearSpikeWallX) >= 0) {
    E->crocom_var_C = 6;
    return INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BBCE);
  }
  return jp;
}

const uint16 *Crocomire_Func_11(uint16 k, const uint16 *jp) {  // 0xA487FB
  Get_Crocomire(0)->crocom_var_C = 6;
  return INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BD2A);
}

const uint16 *Crocomire_Func_12(uint16 k, const uint16 *jp) {  // 0xA48805
  Enemy_Crocomire *E = Get_Crocomire(0);
  E->crocom_var_B &= ~0x400;
  return INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BCD8);
}

const uint16 *Crocomire_Func_13(uint16 k, const uint16 *jp) {  // 0xA48812
  Enemy_Crocomire *E = Get_Crocomire(0);
  if ((E->crocom_var_B & 0x800) != 0) {
    E->crocom_var_B &= ~0x800;
    E->crocom_var_C = 20;
    return INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BC30);
  } else if ((int16)(INSTR_ADDR_TO_PTR(0, jp) - addr_kCrocomire_Ilist_BDA2) >= 0) {
    return INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BD2A);
  }
  return jp;
}

const uint16 *Crocomire_Func_14(uint16 k, const uint16 *jp) {  // 0xA48836
  Enemy_Crocomire *E = Get_Crocomire(0);
  if ((E->crocom_var_B & 0x800) != 0) {
    E->crocom_var_B &= ~0x800;
    E->crocom_var_C = 12;
    return INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BC30);
  } else if ((int16)(INSTR_ADDR_TO_PTR(0, jp) - addr_kCrocomire_Ilist_BDA2) >= 0) {
    return INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BD2A);
  }
  return jp;
}

const uint16 *Crocomire_Func_15(uint16 k, const uint16 *jp) {  // 0xA4885A
  Enemy_Crocomire *E = Get_Crocomire(0);
  if ((E->crocom_var_B & 0x800) != 0) {
    E->crocom_var_B &= ~0x800;
    E->crocom_var_C = 12;
    return INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BC30);
  } else if ((int16)(INSTR_ADDR_TO_PTR(0, jp) - addr_kCrocomire_Ilist_BDA2) >= 0) {
    return INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BD2A);
  }
  return jp;
}

const uint16 *Crocomire_Func_16(uint16 k, const uint16 *jp) {  // 0xA4887E
  Enemy_Crocomire *E = Get_Crocomire(0);
  uint16 v1 = E->crocom_var_D - 1;
  E->crocom_var_D = v1;
  if (sign16(v1 - 2)) {
    E->crocom_var_D = 0;
    E->crocom_var_C = 6;
    return INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BBCE);
  }
  return jp;
}

const uint16 *Crocomire_Func_17(uint16 k, const uint16 *jp) {  // 0xA4889A
  Enemy_Crocomire *E = Get_Crocomire(0);
  uint16 crocom_var_B = E->crocom_var_B;
  if ((crocom_var_B & 0x800) != 0) {
    E->crocom_var_B = crocom_var_B & 0x1F00 | 0xA000;
    E->crocom_var_D = 1;
    E->crocom_var_E = 10;
    E->crocom_var_C = 12;
    QueueSfx2_Max6(kSfx2_ShotCrocomire);
  } else {
    Get_Crocomire(cur_enemy_index)->crocom_var_C = 10;
    return INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BD8E);
  }
  return jp;
}

const uint16 *Crocomire_Func_18(uint16 k, const uint16 *jp) {  // 0xA488D2
  const uint16 *result = Crocomire_Func_2(cur_enemy_index, jp);
  Enemy_Crocomire *E = Get_Crocomire(0);
  E->crocom_var_B |= 0x200;
  E->crocom_var_D = 32;
  E->crocom_var_C = 30;
  return result;
}

const uint16 *Crocomire_Func_19(uint16 k, const uint16 *jp) {  // 0xA488EE
  bool v2; // zf

  jp = Crocomire_Func_2(cur_enemy_index, jp);
  Enemy_Crocomire *E = Get_Crocomire(0);
  if (!E->crocom_var_D || (v2 = E->crocom_var_D == 1, --E->crocom_var_D, v2)) {
    jp = Crocomire_Func_20(cur_enemy_index);
    E->crocom_var_C = 32;
  }
  return jp;
}

const uint16 *Crocomire_Func_20(uint16 k) {  // 0xA4890B
  Get_Crocomire(k)->crocom_var_C = 20;
  Enemy_Crocomire *E = Get_Crocomire(0);
  E->crocom_var_B = E->crocom_var_B;
  return INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BAEA);
}

const uint16 *Crocomire_Func_21(uint16 k, const uint16 *jp) {  // 0xA4891B
  Enemy_Crocomire *E = Get_Crocomire(0);
  if ((E->crocom_var_B & 0x100) != 0) {
    jp = Crocomire_Func_2(cur_enemy_index, jp);
    E->crocom_var_D = 16;
    E->crocom_var_C = 34;
  } else {
    jp = Crocomire_Func_20(cur_enemy_index);
    E->crocom_var_C = 32;
  }
  return jp;
}

const uint16 *Crocomire_Func_22(uint16 k, const uint16 *jp) {  // 0xA48940
  k = cur_enemy_index;
  jp = Crocomire_Func_2(k, jp);
  Enemy_Crocomire *E = Get_Crocomire(0);
  if (!E->crocom_var_D) {
    E->crocom_var_B |= 0x2000;
    jp = Crocomire_Func_3(k, jp);
    E->crocom_var_C = 36;
  }
  return jp;
}

const uint16 *Crocomire_Func_23(uint16 k, const uint16 *jp) {  // 0xA4895E
  k = cur_enemy_index;

  Enemy_Crocomire *E = Get_Crocomire(0);
  if (sign16(E->base.x_pos - 672)) {
    jp = Crocomire_Func_3(k, jp);
    E->crocom_var_C = 36;
    E->crocom_var_D = 3;
  } else {
    if ((E->crocom_var_B & 0x4000) == 0) {
      E->crocom_var_C = 38;
      jp = Crocomire_Func_12(k, jp);
    }
    if ((E->crocom_var_B & 0x4000) != 0) {
      E->crocom_var_D = 5;
      jp = INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BCD8);
      E->crocom_var_F = E->crocom_var_C;
      E->crocom_var_C = 42;
    }
  }
  return jp;
}

const uint16 *Crocomire_Func_24(uint16 k, const uint16 *jp) {  // 0xA489A8
  bool v2; // zf

  k = cur_enemy_index;
  Enemy_Crocomire *E = Get_Crocomire(0);
  if (!E->crocom_var_D || (v2 = E->crocom_var_D == 1, --E->crocom_var_D, v2)) {
    jp = Crocomire_Func_3(k, jp);
    E->crocom_var_C = 40;
    E->crocom_var_B &= ~0x400;
  } else {
    E->crocom_var_C = 36;
    Get_Crocomire(0x40)->crocom_var_D = 0;
    E->crocom_var_B |= 0x400;
    return INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BCD8);
  }
  return jp;
}

const uint16 *Crocomire_Func_25(uint16 k, const uint16 *jp) {  // 0xA489DE
  k = cur_enemy_index;
  Enemy_Crocomire *E = Get_Crocomire(0);
  uint16 crocom_var_B = E->crocom_var_B;
  if ((crocom_var_B & 0x2000) == 0)
    E->crocom_var_B = crocom_var_B & 0xFCFF;
  jp = Crocomire_Func_3(k, jp);
  E->crocom_var_C = 40;
  return jp;
}

const uint16 *Crocomire_Func_26(uint16 k, const uint16 *jp) {  // 0xA489F9
  Enemy_Crocomire *E = Get_Crocomire(0);
  if (E->crocom_var_D) {
    uint16 crocom_var_B = E->crocom_var_B;
    if ((crocom_var_B & 0x4000) != 0) {
      --E->crocom_var_D;
      QueueSfx2_Max6(kSfx2_DachoraShinespark);
      return INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BCD8);
    } else {
      E->crocom_var_B = crocom_var_B & 0xBFFF;
      E->crocom_var_C = 12;
    }
  } else {
    E->crocom_var_B &= ~0x4000;
    E->base.instruction_timer = 1;
    E->crocom_var_C = E->crocom_var_F;
    return INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BCD8);
  }
  return jp;
}

void Crocomire_Init(void) {  // 0xA48A5A
  VramWriteEntry *v7;

  boss_id = kCrocomireBossId;
  for (uint16 i = 0; (int16)(i - 4096) < 0; i += 2)
    tilemap_stuff[i >> 1] = kCrocomireBg2TilemapFill;
  if ((*(uint16 *)&boss_bits_for_area[area_index] & kCrocomireBossBit) != 0) {
    *(uint16 *)scrolls = 257;
    *(uint16 *)&scrolls[2] = 257;
    croco_target_0688 = 0;
    Enemy_Crocomire *E = Get_Crocomire(0);
    E->base.properties = (E->base.properties & ~(kEnemyProps_SolidToSamus | kEnemyProps_Intangible)) | kEnemyProps_Intangible;
    SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x20, 0x03, 0xb753 });
    SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x1e, 0x03, 0xb753 });
    SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x61, 0x0b, 0xb747 });
    E->crocom_var_A = 84;
    E->base.current_instruction = addr_kCrocomire_Ilist_E1CC;
    E->base.instruction_timer = 1;
    E->base.x_pos = 576;
    E->base.y_pos = 144;
    E->base.y_height = 28;
    E->base.x_width = 40;
    uint16 v6 = vram_write_queue_tail;
    v7 = gVramWriteEntry(vram_write_queue_tail);
    v7->size = 2048;
    v7->src.addr = 0x2000;
    *(uint16 *)&v7->src.bank = 126;
    v7->vram_dst = (reg_BG2SC & 0xFC) << 8;
    vram_write_queue_tail = v6 + 7;
  } else {
    DisableMinimapAndMarkBossRoomAsExplored();
    croco_word_7E069A = 0;
    Enemy_Crocomire *E = Get_Crocomire(cur_enemy_index);
    E->crocom_var_A = 0;
    E->crocom_var_E = 0;
    *(uint16 *)scrolls = 0;
    for (int i = 32; i >= 0; i -= 2) {
      int v3 = i >> 1;
      target_palettes[v3 + 160] = kCrocomireSpritePalette2[v3];
      target_palettes[v3 + 208] = kCrocomireSpritePalette5[v3];
    }
    E->crocom_var_C = 4;
    UNUSED_word_7E179E = 16;
    camera_distance_index = 2;
    enemy_bg2_tilemap_size = 1024;
    E->base.current_instruction = addr_kCrocomire_Ilist_BADE;
    E->base.extra_properties |= 4;
    E->base.instruction_timer = 1;
  }
}

void Crocomire_Func_27(uint16 k) {  // 0xA48B5B
  reg_BG2VOFS = 67 - Get_Crocomire(k)->base.y_pos;
  uint16 spritemap_i = 32;
  uint16 spritemap_pointer;
  while (1) {
    spritemap_pointer = Get_Crocomire(0)->base.spritemap_pointer;
    if (spritemap_pointer == kCrocomireBg2ScrollSpritemaps[spritemap_i >> 1])
      break;
    spritemap_i -= 2;
    if (sign16(spritemap_i)) {
      Crocomire_8BA4();
      return;
    }
  }
  reg_BG2VOFS += *((uint16 *)RomPtr_A4(spritemap_pointer) + 14);
  Crocomire_8BA4();
}

void Crocomire_8BA4(void) {  // 0xA48BA4
  Enemy_Crocomire *E0 = Get_Crocomire(cur_enemy_index);
  Enemy_Crocomire *E1 = Get_Crocomire(cur_enemy_index + 64);
  E1->base.x_pos = E1->crocom_var_A + E0->base.x_pos;
  E1->base.y_pos = E0->base.y_pos;
  uint16 x_pos = E0->base.x_pos;
  bool off_screen;
  if ((int16)(x_pos - layer1_x_pos) >= 0)
    off_screen = !sign16(E0->base.x_pos - 128 - (layer1_x_pos + 256));
  else
    off_screen = (int16)(x_pos + 128 - layer1_x_pos) < 0;
  if (off_screen) {
    reg_BG2HOFS = 256;
    return;
  }
  uint16 bg2_x = layer1_x_pos - E0->base.x_pos + 51;
  if (!sign16(abs16(bg2_x) - 284))
    bg2_x = 256;
  reg_BG2HOFS = bg2_x;
}

void nullsub_305(void) {}

static Func_V *const kCrocomireDeathSequence[45] = {  // 0xA48C04
  Crocomire_Func_28, Crocomire_Func_52, Crocomire_Func_36,    Crocomire_92CE, Crocomire_Func_54, Crocomire_Func_36,    Crocomire_92CE, Crocomire_Func_54,
  Crocomire_Func_57, Crocomire_Func_60, Crocomire_Func_62,    Crocomire_929E, Crocomire_Func_64, Crocomire_Func_65, Crocomire_Func_66, Crocomire_Func_51,
     Crocomire_8D47,    Crocomire_929E, Crocomire_Func_51,    Crocomire_8D47,    Crocomire_929E, Crocomire_Func_51, Crocomire_Func_59, Crocomire_Func_60,
  Crocomire_Func_62, Crocomire_Func_63, Crocomire_Func_56,    Crocomire_9506, Crocomire_Func_65, Crocomire_Func_66, Crocomire_Func_49, Crocomire_Func_68,
  Crocomire_Func_70, Crocomire_Func_71, Crocomire_Func_72, Crocomire_Func_73, Crocomire_Func_88,    Crocomire_9B65, Crocomire_Func_90, Crocomire_Func_89,
     Crocomire_9B86,       nullsub_305, Crocomire_Func_91, Crocomire_Func_29, Crocomire_Func_69,
};

void Crocomire_Main(void) {
  kCrocomireDeathSequence[Get_Crocomire(0)->crocom_var_A >> 1]();
  Crocomire_Func_30();
  Crocomire_Func_31();
}

void Crocomire_Func_28(void) {  // 0xA48C6E
  Crocomire_Func_37();
  *(uint16 *)&scrolls[4] = 257;
  if (!sign16(samus_x_pos - 1312))
    *(uint16 *)&scrolls[4] = 256;
  Crocomire_Func_27(cur_enemy_index);
}

void Crocomire_Func_29(void) {  // 0xA48C90
  reg_BG2HOFS = 0;
  reg_BG2VOFS = 0;
}

void Crocomire_Func_30(void) {  // 0xA48C95
  Enemy_Crocomire *E = Get_Crocomire(0);
  if (!E->crocom_var_A && (int16)(E->base.x_pos - E->base.x_width - samus_x_radius - samus_x_pos) < 0) {
    NormalEnemyTouchAi();
    samus_x_pos = E->base.x_pos - E->base.x_width - samus_x_radius;
    samus_prev_x_pos = samus_x_pos;
    extra_samus_x_displacement = -4;
    extra_samus_y_displacement = -1;
  }
}

void Crocomire_Func_31(void) {  // 0xA48CCB
  uint16 j;

  if (!door_transition_flag_enemies) {
    if (Get_Crocomire(0)->base.flash_timer && (random_enemy_counter & 2) != 0) {
      for (int i = 14; i >= 0; i -= 2)
        palette_buffer[(i >> 1) + 112] = 0x7FFF;
    } else {
      for (j = 14; !sign16(j); j -= 2)
        palette_buffer[(j >> 1) + 112] = kCrocomireHurtFlashPalette[j >> 1];
    }
  }
}

const uint16 *Crocomire_Instr_11(uint16 k, const uint16 *jp) {  // 0xA48CFB
  QueueSfx2_Max6(kSfx2_CrocomireCry);
  return jp;
}

const uint16 *Crocomire_Instr_7(uint16 k, const uint16 *jp) {  // 0xA48D07
  QueueSfx2_Max6(kSfx2_BigExplosion);
  return jp;
}

const uint16 *Crocomire_Instr_19(uint16 k, const uint16 *jp) {  // 0xA48D13
  QueueSfx2_Max6(kSfx2_CrocomireSkeletonCollapses);
  return jp;
}

void Crocomire_Func_35(void) {  // 0xA48D1F
  Enemy_Crocomire *E = Get_Crocomire(0);
  uint16 crocom_var_20 = E->crocom_var_20;
  if (crocom_var_20) {
    uint16 v2 = crocom_var_20 - 1;
    E->crocom_var_20 = v2;
    if (!v2) {
      E->crocom_var_20 = 32;
      QueueSfx3_Max6(kSfx3_CrocomireAcidDamage);
    }
  }
}

void Crocomire_Func_36(void) {  // 0xA48D3F
  Crocomire_Func_35();
  Crocomire_8BA4();
  Crocomire_8D47();
}

void Crocomire_8D47(void) {  // 0xA48D47
  Enemy_Crocomire *E = Get_Crocomire(0);
  uint16 crocom_var_D = E->crocom_var_D;
  if (crocom_var_D) {
    E->crocom_var_D = crocom_var_D - 1;
  } else {
    ++E->crocom_var_A;
    ++E->crocom_var_A;
    E->crocom_var_E = 768;
  }
}

void Crocomire_Func_37(void) {  // 0xA48D5E

  Enemy_Crocomire *E0 = Get_Crocomire(0);
  if (sign16(E0->base.x_pos - 1536)) {
    kraid_unk9000 = 0;
    g_word_7E9002 = 0;
    g_word_7E9006 = 0;
    g_word_7E900A = 0;
  } else if (sign16(E0->base.x_pos - 1552)) {
    if (!kraid_unk9000) {
      kraid_unk9000 = 1;
      eproj_spawn_pt = (Point16U){ 1536, 176 };
      SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 0x15);
    }
  } else if (sign16(E0->base.x_pos - 1552)) {
    g_word_7E9002 = 0;
    g_word_7E9006 = 0;
    g_word_7E900A = 0;
  } else if (sign16(E0->base.x_pos - 1568)) {
    if (!g_word_7E9002) {
      g_word_7E9002 = 1;
      SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x61, 0x0b, 0xb74b });
      eproj_spawn_pt = (Point16U){ 1568, 176 };
      SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 0x15);
    }
  } else {
    uint16 x_pos = E0->base.x_pos;
    if (sign16(x_pos - 1568)) {
      g_word_7E9006 = 0;
      g_word_7E900A = 0;
    } else if (sign16(x_pos - 1584)) {
      if (!g_word_7E9006) {
        g_word_7E9006 = 1;
        SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x62, 0x0b, 0xb74b });
        SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x63, 0x0b, 0xb74b });
        eproj_spawn_pt = (Point16U){ 1584, 176 };
        SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 0x15);
      }
    } else if ((int16)(x_pos - kCrocomireBridgeCollapseX) >= 0) {
      Crocomire_8EE5();
      E0->crocom_var_20 = 1;
      g_word_7E9018 = 1;
      uint16 v3 = cur_enemy_index;
      Enemy_Crocomire *EK = Get_Crocomire(cur_enemy_index);
      EK->crocom_var_A += 2;
      Get_Crocomire(0)->crocom_var_00 = 2;
      Get_Crocomire(0x40)->crocom_var_00 = 2;
      Get_Crocomire(0x80)->crocom_var_00 = 2;
      Get_Crocomire(0xC0)->crocom_var_00 = 2;
      Get_Crocomire(0x100)->crocom_var_00 = 2;
      Get_Crocomire(0x140)->crocom_var_00 = 2;
      Get_Crocomire(0)->crocom_var_01 = 0;
      Get_Crocomire(0x40)->crocom_var_01 = 0;
      Get_Crocomire(0x80)->crocom_var_01 = 0;
      Get_Crocomire(0xC0)->crocom_var_01 = 0;
      Get_Crocomire(0x100)->crocom_var_01 = 0;
      Get_Crocomire(0x140)->crocom_var_01 = 0;
      QueueSfx2_Max6(kSfx2_DachoraShinespark);
      *(uint16 *)((uint8 *)&g_word_7E9015 + 1) = 0;
      SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x4e, 0x03, 0xb757 });
      EK->base.current_instruction = addr_kCrocomire_Ilist_BFB0;
      EK->base.instruction_timer = 1;
      EK->base.properties |= kEnemyProps_Intangible;
      Enemy_Crocomire *E1 = Get_Crocomire(cur_enemy_index + 64);
      E1->base.instruction_timer = 0x7FFF;
      E1->base.current_instruction = addr_kCrocomire_Ilist_BF62;
      Get_Crocomire(0x40)->base.properties |= kEnemyProps_Invisible;
      EK->crocom_var_E = 0;
      EK->crocom_var_F = 0;
      EK->crocom_var_D = 2048;
      Get_Crocomire(0)->base.y_height = 16;
    }
  }
}

void Crocomire_8EE5(void) {  // 0xA48EE5
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x61, 0x0b, 0xb74f });
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x62, 0x0b, 0xb74f });
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x63, 0x0b, 0xb74f });
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x64, 0x0b, 0xb74f });
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x65, 0x0b, 0xb74f });
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x66, 0x0b, 0xb74f });
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x67, 0x0b, 0xb74f });
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x68, 0x0b, 0xb74f });
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x69, 0x0b, 0xb74f });
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x6a, 0x0b, 0xb74f });
  eproj_spawn_pt = (Point16U){ 1536, 176 };
  SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 0x15);
  eproj_spawn_pt = (Point16U){ 1552, 192 };
  SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 0x15);
  eproj_spawn_pt = (Point16U){ 1568, 176 };
  SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 0x15);
  eproj_spawn_pt = (Point16U){ 1584, 192 };
  SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 0x15);
  eproj_spawn_pt = (Point16U){ 1600, 192 };
  SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 0x15);
  eproj_spawn_pt = (Point16U){ 1616, 192 };
  SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 0x15);
  eproj_spawn_pt = (Point16U){ 1632, 192 };
  SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 0x15);
}

const uint16 *Crocomire_Instr_2(uint16 k, const uint16 *jp) {  // 0xA48FC7
  earthquake_type = 4;
  earthquake_timer = 5;
  QueueSfx2_Max6(kSfx2_Quake);
  return jp;
}

const uint16 *Crocomire_Instr_4(uint16 k, const uint16 *jp) {  // 0xA48FDF
  if ((Get_Crocomire(0)->crocom_var_B & 0x800) == 0)
    Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, INT16_SHL16(-4));
  return jp;
}

const uint16 *Crocomire_Instr_3(uint16 k, const uint16 *jp) {  // 0xA48FFA
  Crocomire_Func_43();
  return Crocomire_Instr_4(k, jp);
}

const uint16 *Crocomire_Instr_15(uint16 k, const uint16 *jp) {  // 0xA48FFF
  Crocomire_Func_43();
  return Crocomire_Instr_4(k, jp);
}

void Crocomire_Func_43(void) {  // 0xA49004
  uint16 v1 = random_number & 0x1F;
  if ((int16)(random_number - 4096) >= 0)
    v1 = -v1;
  Crocomire_Func_87(0, v1);
}

const uint16 *Crocomire_Instr_16(uint16 k, const uint16 *jp) {  // 0xA4901D
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, INT16_SHL16(-4))) {
    Get_Crocomire(0)->crocom_var_C = 14;
    return INSTR_RETURN_ADDR(addr_kCrocomire_Ilist_BF3C);
  } else {
    uint16 v2 = 32;
    if (!sign16(random_number - 2048))
      v2 = -32;
    return Crocomire_Func_87(jp, v2 + (random_number & 0xF));
  }
}

const uint16 *Crocomire_Instr_13(uint16 k, const uint16 *jp) {  // 0xA4905B
  Enemy_Crocomire *E = Get_Crocomire(0);
  if ((int16)(E->base.x_pos - E->base.x_width - 260 - layer1_x_pos) < 0)
    Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, INT16_SHL16(4));
  return jp;
}

const uint16 *Crocomire_Instr_18(uint16 k, const uint16 *jp) {  // 0xA4907F
  Enemy_MoveRight_IgnoreSlopes(k, INT16_SHL16(4));
  return jp;
}

const uint16 *Crocomire_Instr_12(uint16 k, const uint16 *jp) {  // 0xA4908F
  Crocomire_Func_43();
  return Crocomire_Instr_13(k, jp);
}

const uint16 *Crocomire_Instr_17(uint16 k, const uint16 *jp) {  // 0xA49094
  Crocomire_Func_43();
  return Crocomire_Instr_18(k, jp);
}

void Crocomire_Func_49(void) {  // 0xA49099
  Crocomire_Func_50();
  Crocomire_Func_53();
  Crocomire_Func_55();
  Enemy_Crocomire *E = Get_Crocomire(0);
  if (E->crocom_var_A == 62) {
    QueueMusic_Delayed8(kCrocomireMusic_Song1);
    E->crocom_var_A = 88;
    E->base.current_instruction = addr_kCrocomire_Ilist_E1D2;
    *(uint16 *)&scrolls[4] = 257;
    debug_disable_minimap = 0;
    Get_Crocomire(0x40)->base.properties |= kEnemyProps_Deleted;
    SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x4e, 0x03, 0xb753 });
    camera_distance_index = 0;
    croco_target_0688 = 0;
  }
}

void Crocomire_Func_50(void) {  // 0xA490DF
  Enemy_Crocomire *E = Get_Crocomire(0);
  uint16 y_pos = E->base.y_pos;
  uint16 v2 = addr_kCrocomire_Ilist_BF7E;
  if (sign16(y_pos - 280)) {
    v2 = addr_kCrocomire_Ilist_BF86;
    if (sign16(y_pos - 264)) {
      v2 = addr_kCrocomire_Ilist_BF8C;
      if (sign16(y_pos - 248))
        v2 = addr_kCrocomire_Ilist_BF92;
    }
  }
  E->base.current_instruction = v2;
  E->base.instruction_timer = 1;
}

void Crocomire_Func_51(void) {  // 0xA49108
  Crocomire_Func_53();
  Enemy_Crocomire *E = Get_Crocomire(0);
  uint16 y_pos = E->base.y_pos;
  uint16 v2 = addr_kCrocomire_Ilist_BF64;
  if (sign16(y_pos - 280)) {
    v2 = addr_kCrocomire_Ilist_BF6C;
    if (sign16(y_pos - 264)) {
      v2 = addr_kCrocomire_Ilist_BF72;
      if (sign16(y_pos - 248))
        v2 = addr_kCrocomire_Ilist_BF78;
    }
  }
  E->base.current_instruction = v2;
  E->base.instruction_timer = 1;
  Crocomire_Func_55();
}

void Crocomire_Func_52(void) {  // 0xA49136
  uint16 crumbling_idx = *(uint16 *)((uint8 *)&g_word_7E9015 + 1);
  if (sign16(crumbling_idx - 22)) {
    *(uint16 *)((uint8 *)&g_word_7E9015 + 1) = crumbling_idx + 2;
    SpawnEprojWithGfx(kCrocomireBridgeCrumbleX[crumbling_idx >> 1], crumbling_idx, addr_stru_868F9D);
  }
  Crocomire_Func_54();
}

void Crocomire_Func_53(void) {  // 0xA4916C
  if (!--g_word_7E9018) {
    g_word_7E9018 = 6;
    uint16 x_jitter = random_number & 0x3F;
    if ((random_number & 2) == 0)
      x_jitter = ~x_jitter;
    uint16 x = Get_Crocomire(0)->base.x_pos + x_jitter;
    uint16 y = lava_acid_y_pos + 16 - ((uint16)(random_number & 0x1F00) >> 8);
    CreateSpriteAtPos(x, y, 21, 0);
  }
}

void Crocomire_Func_54(void) {  // 0xA491BA
  Crocomire_Func_35();
  Crocomire_Func_53();
  Crocomire_Func_55();
}

void Crocomire_Func_55(void) {  // 0xA491C1
  int8 crocom_var_D;
  int8 v7;
  int8 v9;
  int8 crocom_var_E;
  int8 crocom_var_E_high;
  int8 v12;

  Crocomire_Func_58();
  Enemy_Crocomire *E0 = Get_Crocomire(0);
  E0->crocom_var_B &= ~0x800;
  Crocomire_Func_27(cur_enemy_index);
  Enemy_Crocomire *E = Get_Crocomire(cur_enemy_index);
  if (sign16(E->base.y_pos - 280)) {
    uint16 v3 = 8 * ((reg_BG2VOFS + 288) & 0xFFF8);
    int m = 32;
    do {
      tilemap_stuff[v3 >> 1] = kCrocomireBg2TilemapFill;
      v3 += 2;
    } while (--m);
    nmi_flag_bg2_enemy_vram_transfer = 1;
    E0->base.extra_properties &= ~0x8000;
    crocom_var_D = E->crocom_var_D;
    LOBYTE(E->crocom_var_D) = crocom_var_D + 0x80;
    v7 = __CFADD__uint8(crocom_var_D, 0x80) + HIBYTE(E->crocom_var_D) + 3;
    if (!sign8(v7 - 48))
      v7 = 48;
    HIBYTE(E->crocom_var_D) = v7;
    bool v8 = __CFADD__uint8(LOBYTE(E->crocom_var_E), v7);
    LOBYTE(E->crocom_var_E) += v7;
    v9 = v8 + HIBYTE(E->crocom_var_E);
    if (!sign8(v9 - 3))
      v9 = 3;
    HIBYTE(E->crocom_var_E) = v9;
    crocom_var_E = E->crocom_var_E;
    v8 = __CFADD__uint8(HIBYTE(E->crocom_var_F), crocom_var_E);
    HIBYTE(E->crocom_var_F) += crocom_var_E;
    crocom_var_E_high = HIBYTE(E->crocom_var_E);
    bool v13 = v8;
    v8 = __CFADD__uint8(v8, crocom_var_E_high);
    v12 = v13 + crocom_var_E_high;
    v8 |= __CFADD__uint8(LOBYTE(E->base.y_pos), v12);
    LOBYTE(E->base.y_pos) += v12;
    HIBYTE(E->base.y_pos) += v8;
  } else {
    ++E->crocom_var_A;
    ++E->crocom_var_A;
    E0->crocom_var_D = 48;
  }
}

void Crocomire_Func_56(void) {  // 0xA4926E
  Enemy_Crocomire *E = Get_Crocomire(cur_enemy_index);
  uint16 y_pos = E->base.y_pos;
  uint16 v2 = addr_kCrocomire_Ilist_BF7E;
  if (sign16(y_pos - 280)) {
    v2 = addr_kCrocomire_Ilist_BF86;
    if (sign16(y_pos - 264)) {
      v2 = addr_kCrocomire_Ilist_BF8C;
      if (sign16(y_pos - 248))
        v2 = addr_kCrocomire_Ilist_BF92;
    }
  }
  E->base.current_instruction = v2;
  E->base.instruction_timer = 1;
  Crocomire_Func_53();
  Crocomire_92D8();
}

void Crocomire_929E(void) {  // 0xA4929E
  Crocomire_Func_53();
  Enemy_Crocomire *E = Get_Crocomire(cur_enemy_index);
  uint16 y_pos = E->base.y_pos;
  uint16 v2 = addr_kCrocomire_Ilist_BF64;
  if (sign16(y_pos - 280)) {
    v2 = addr_kCrocomire_Ilist_BF6C;
    if (sign16(y_pos - 264)) {
      v2 = addr_kCrocomire_Ilist_BF72;
      if (sign16(y_pos - 248))
        v2 = addr_kCrocomire_Ilist_BF78;
    }
  }
  E->base.current_instruction = v2;
  E->base.instruction_timer = 1;
  Crocomire_92D8();
}

void Crocomire_92CE(void) {  // 0xA492CE
  Crocomire_Func_35();
  Crocomire_Func_53();
  Crocomire_92D8();
}

void Crocomire_92D8(void) {  // 0xA492D8
  int16 v3;
  int8 v6;

  Crocomire_Func_58();
  Enemy_Crocomire *EK = Get_Crocomire(cur_enemy_index);
  if (sign16(EK->base.y_pos - 218)) {
    ++EK->crocom_var_A;
    ++EK->crocom_var_A;
  } else {
    Crocomire_Func_27(cur_enemy_index);
    Enemy_Crocomire *E0 = Get_Crocomire(0);
    v3 = E0->crocom_var_D + 256;
    if (!sign16(E0->crocom_var_D - 7680))
      v3 = 7936;
    E0->crocom_var_D = v3;
    uint8 crocom_var_E = E0->crocom_var_E;
    bool v5 = crocom_var_E < HIBYTE(E0->crocom_var_D);
    LOBYTE(E0->crocom_var_E) = crocom_var_E - HIBYTE(E0->crocom_var_D);
    v6 = HIBYTE(E0->crocom_var_E) - v5;
    if (v6 < 0) {
      LOBYTE(E0->crocom_var_E) = -1;
      v6 = 0;
    }
    HIBYTE(E0->crocom_var_E) = v6;
    uint8 crocom_var_F_high = HIBYTE(E0->crocom_var_F);
    v5 = crocom_var_F_high < LOBYTE(E0->crocom_var_E);
    HIBYTE(E0->crocom_var_F) = crocom_var_F_high - LOBYTE(E0->crocom_var_E);
    uint8 y_pos = E0->base.y_pos;
    uint8 v9 = v5 + HIBYTE(E0->crocom_var_E);
    LOBYTE(E0->base.y_pos) = y_pos - v9;
    HIBYTE(E0->base.y_pos) -= y_pos < v9;
  }
}

void Crocomire_Func_57(void) {  // 0xA49341
  int i;

  croco_word_068C = 48;
  croco_target_0688 = 48;
  Enemy_Crocomire *E = Get_Crocomire(cur_enemy_index);
  ++E->crocom_var_A;
  ++E->crocom_var_A;
  E->base.current_instruction = addr_kCrocomire_Ilist_BF64;
  E->base.instruction_timer = 1;
  nmi_flag_bg2_enemy_vram_transfer = 1;
  Enemy_Crocomire *E1 = Get_Crocomire(cur_enemy_index + 64);
  E1->base.current_instruction = addr_kCrocomire_Ilist_BF98;
  E1->base.instruction_timer = 1;
  E1->base.properties = E1->base.properties & 0xD2FF | 0x2C00;
  E1->base.x_pos = E->base.x_pos;
  E1->base.y_pos = E->base.y_pos + 16;
  uint16 v3 = 0;
  do {
    int v4 = v3 >> 1;
    tilemap_stuff[v4] = kCrocomireBg2TilemapFill;
    tilemap_stuff[v4 + 1] = kCrocomireBg2TilemapFill;
    v3 += 4;
  } while ((int16)(v3 - 1024) < 0);
  for (i = 0; ; i += 2) {
    int v6 = i >> 1;
    if (kCrocomireMelting1Tilemap[v6] == 0xFFFF)
      break;
    tilemap_stuff[v6 + 32] = kCrocomireMelting1Tilemap[v6];
  }
  Crocomire_93BE(i + 1024);
}

void Crocomire_93BE(uint16 k) {  // 0xA493BE
  VramWriteEntry *v3;

  uint16 v2 = vram_write_queue_tail;
  v3 = gVramWriteEntry(vram_write_queue_tail);
  v3->size = k;
  v3->src.addr = 0x2000;
  *(uint16 *)&v3->src.bank = 126;
  v3->vram_dst = (reg_BG2SC & 0xFC) << 8;
  vram_write_queue_tail = v2 + 7;
}

void Crocomire_Func_58(void) {  // 0xA493DF
  uint16 bg2_vofs = reg_BG2VOFS;
  for (int i = 510; i >= 0; i -= 2)
    crocomire_bg2_scroll_hdma_data[i >> 1] = bg2_vofs;
}

void Crocomire_Func_59(void) {  // 0xA493ED
  int i;

  Crocomire_Func_58();
  Enemy_Crocomire *E = Get_Crocomire(cur_enemy_index);
  ++E->crocom_var_A;
  ++E->crocom_var_A;
  E->base.instruction_timer = 1;
  croco_word_068C = 48;
  croco_target_0688 = 48;
  E->base.current_instruction = addr_kCrocomire_Ilist_BF7E;
  uint16 v1 = 0;
  do {
    tilemap_stuff[v1 >> 1] = kCrocomireBg2TilemapFill;
    v1 += 2;
  } while ((int16)(v1 - 2048) < 0);
  for (i = 0; ; i += 2) {
    int v3 = i >> 1;
    if (kCrocomireMelting2Tilemap[v3] == 0xFFFF)
      break;
    tilemap_stuff[v3 + 32] = kCrocomireMelting2Tilemap[v3];
  }
  Crocomire_93BE(i + 1024);
}

void Crocomire_Func_60(void) {  // 0xA4943D
  int i;

  Enemy_Crocomire *E = Get_Crocomire(cur_enemy_index);
  E->crocom_var_A += 2;
  g_word_7E0692 = 256;
  croco_cur_vline_idx = 0;
  g_word_7E0698 = CrocomireMeltingLoadWord(croco_word_7E069A);
  g_word_7E0694 = g_word_7E0698;
  g_word_7E0696 = CrocomireMeltingLoadWord(croco_word_7E069A + 2);
  g_word_7E068E = CrocomireMeltingLoadWord(croco_word_7E069A + 4);
  uint8 bank = CrocomireMeltingLoadWord(croco_word_7E069A + 6);
  uint16 v6;
  for (i = croco_word_7E069A + 8; ; i = v6 + 4) {
    uint16 src = CrocomireMeltingLoadWord(i);
    if (src == 0xFFFF)
      break;
    v6 = i;
    uint16 dst = CrocomireMeltingLoadWord(i + 2);
    int n = g_word_7E068E;
    do {
      *(uint16 *)&g_ram[dst] = GET_WORD(RomPtrWithBank(bank, src));
      dst += 2, src += 2;
    } while (--n >= 0);
  }
  croco_word_7E069A = i + 2;
  g_word_7E068A = i + 2;
  for (int j = 128; j >= 0; j -= 2)
    *(uint16 *)&croco_vline_height[(uint16)j] = 0;
}

void Crocomire_Func_61(void) {  // 0xA494B2
  Crocomire_Func_62();
}

void Crocomire_Func_62(void) {  // 0xA494B6
  VramWriteEntry *v2;

  uint16 table_off = g_word_7E068A;
  uint16 vram_tail = vram_write_queue_tail;
  if (CrocomireMeltingLoadWord(g_word_7E068A) == 0xFFFF) {
    Enemy_Crocomire *E = Get_Crocomire(cur_enemy_index);
    ++E->crocom_var_A;
    ++E->crocom_var_A;
    g_word_7E068A = 0;
  } else {
    v2 = gVramWriteEntry(vram_write_queue_tail);
    v2->size = CrocomireMeltingLoadWord(g_word_7E068A);
    v2->src.addr = CrocomireMeltingLoadWord(table_off + 6);
    *(uint16 *)&v2->src.bank = CrocomireMeltingLoadWord(table_off + 4);
    v2->vram_dst = CrocomireMeltingLoadWord(table_off + 2);
    g_word_7E068A = table_off + 8;
    vram_write_queue_tail = vram_tail + 7;
  }
}

void Crocomire_Func_64(void) {  // 0xA494FB
  QueueSfx2_Max6(kSfx2_CrocomireMeltingCry);
  Crocomire_950F();
}

void Crocomire_9506(void) {  // 0xA49506
  QueueSfx2_Max6(kSfx2_CrocomireDyingCry);
  Crocomire_950F();
}

void Crocomire_950F(void) {  // 0xA4950F
  static const SpawnHdmaObject_Args unk_A49559 = { 0x42, 0x10, 0x9563 };
  Enemy_Crocomire *v4; // r10

  uint16 v1 = 2 * (Get_Crocomire(0x40)->base.y_pos - 72);
  uint16 v2 = reg_BG2VOFS;
  do {
    crocomire_bg2_scroll_hdma_data[v1 >> 1] = v2;
    v1 -= 2;
  } while ((v1 & 0x8000) == 0);
  Enemy_Crocomire *E = Get_Crocomire(cur_enemy_index);
  ++E->crocom_var_A;
  ++E->crocom_var_A;
  v4 = Get_Crocomire(0);
  Get_Crocomire(0x80)->crocom_var_D = v4->base.x_pos;
  *(uint16 *)crocomire_bg2_scroll_hdma_indirect = 255;
  *(uint16 *)&crocomire_bg2_scroll_hdma_indirect[1] = -13584;
  *(uint16 *)&crocomire_bg2_scroll_hdma_indirect[3] = 225;
  *(uint16 *)&crocomire_bg2_scroll_hdma_indirect[4] = -13330;
  *(uint16 *)&crocomire_bg2_scroll_hdma_indirect[6] = 0;
  v4->crocom_var_1F = SpawnHdmaObject(0xa4, &unk_A49559);
}

void Crocomire_Func_63(void) {  // 0xA49576
  Enemy_Crocomire *E = Get_Crocomire(cur_enemy_index);
  E->crocom_var_A += 2;
}

void Crocomire_Func_65(void) {  // 0xA49580
  Crocomire_Func_53();
  Enemy_Crocomire *E0 = Get_Crocomire(0);
  Enemy_Crocomire *E1 = Get_Crocomire(0x40);
  Enemy_Crocomire *E2 = Get_Crocomire(0x80);
  uint16 crocom_var_D = E2->crocom_var_D;
  --E1->crocom_var_D;
  if ((E1->crocom_var_D & 2) != 0)
    crocom_var_D += 4;
  E0->base.x_pos = crocom_var_D;
  bool finished = !Crocomire_Func_67();
  if (!finished) {
    Crocomire_Func_27(cur_enemy_index);
    uint16 dest_y = g_word_7E0694 - 3;
    if (sign16(g_word_7E0694 - 19)) {
      if ((int16)(g_word_7E0692 - 20480) >= 0)
        finished = true;
      else
        dest_y = 16;
    }
    if (!finished) {
      g_word_7E0694 = dest_y;
      uint16 v5 = g_word_7E0692 + 384;
      if (!sign16(g_word_7E0692 - 20096))
        v5 = 20480;
      g_word_7E0692 = v5;
      uint16 r18 = 0;
      uint16 v6 = 2 * (E1->base.y_pos - 72);
      uint16 v7 = g_word_7E0694;
      message_box_animation_y1 = g_word_7E0694;
      do {
        crocomire_bg2_scroll_hdma_data[v6 >> 1] = reg_BG2VOFS + v7 - message_box_animation_y1;
        bool v8 = __CFADD__uint16(g_word_7E0692, r18);
        r18 += g_word_7E0692;
        if (!v8)
          ++v7;
        message_box_animation_y1++;
        v6 += 2;
      } while ((int16)(v7 - g_word_7E0698) < 0);
      if ((int16)(v6 - 512) < 0) {
        uint16 v9 = reg_BG2VOFS;
        do {
          crocomire_bg2_scroll_hdma_data[v6 >> 1] = v9;
          v6 += 2;
        } while ((int16)(v6 - 512) < 0);
      }
      return;
    }
  }
  Enemy_Crocomire *EK = Get_Crocomire(cur_enemy_index);
  EK->crocom_var_A += 2;
  int i;
  for (i = croco_word_7E069A; CrocomireMeltingLoadWord(i) != 0xFFFF; i += 8)
    ;
  croco_word_7E069A = i + 2;
  hdma_object_channels_bitmask[E0->crocom_var_1F >> 1] = 0;
}

void Crocomire_Func_66(void) {  // 0xA49653
  VramWriteEntry *v3;

  Enemy_Crocomire *E = Get_Crocomire(cur_enemy_index);
  E->crocom_var_E = 0;
  E->crocom_var_F = 0;
  E->crocom_var_D = 2048;
  for (int i = 4094; i >= 0; i -= 2)
    tilemap_stuff[i >> 1] = kCrocomireBg2TilemapFill;
  uint16 v2 = vram_write_queue_tail;
  v3 = gVramWriteEntry(vram_write_queue_tail);
  v3->size = 2048;
  v3->src.addr = 0x2000;
  *(uint16 *)&v3->src.bank = 126;
  v3->vram_dst = (reg_BG2SC & 0xFE) << 8;
  vram_write_queue_tail = v2 + 7;
  Crocomire_9BB3();
}


//  Used when dissolving the crocomire thing
uint16 Crocomire_Func_67(void) {  // 0xA496C8
  int n = croco_word_068C;
//  R20 = 0;
  //R22_ = 0;
  while (1) {
    if (croco_cur_vline_idx > 48) // bugfix
      return 1;
    int rr = kCrocomireMeltingVlineOrder[croco_cur_vline_idx];
    if ((int8)(croco_vline_height[rr] - croco_target_0688) < 0)
      break;
    if (++croco_cur_vline_idx >= 128) {
      croco_cur_vline_idx = 0;
      return 0;
    }
  }
  assert(croco_cur_vline_idx <= 48);
  int vline_idx = kCrocomireMeltingVlineOrder[croco_cur_vline_idx];
  uint8 mask = kCrocomireEraseLineMasks[vline_idx & 7];
  do {
    int q = croco_vline_height[vline_idx];
    int j = 2 * (q & 7) + ((q & ~7) << 6) + 4 * (vline_idx & ~7);
    ram4000.backups.field_0[j + 0] &= mask;
    ram4000.backups.field_0[j + 1] &= mask;
    ram4000.backups.field_0[j + 16] &= mask;
    ram4000.backups.field_0[j + 17] &= mask;

    if (croco_vline_height[vline_idx] == 48)
      break;
    croco_vline_height[vline_idx]++;
  } while (--n);

  uint16 v7, v8;
  while (1) {
    v7 = g_word_7E068A + croco_word_7E069A;
    v8 = vram_write_queue_tail;
    if (CrocomireMeltingLoadWord(v7) != 0xFFFF)
      break;
    g_word_7E068A = 0;
  }
  VramWriteEntry *v9 = gVramWriteEntry(vram_write_queue_tail);
  v9->size = CrocomireMeltingLoadWord(v7);
  v9->src.addr = CrocomireMeltingLoadWord(v7 + 6);
  *(uint16 *)&v9->src.bank = CrocomireMeltingLoadWord(v7 + 4);
  v9->vram_dst = CrocomireMeltingLoadWord(v7 + 2);
  vram_write_queue_tail = v8 + 7;
  g_word_7E068A += 8;
  return 1;
}

void Crocomire_Func_68(void) {  // 0xA497D3
  if (sign16(samus_x_pos - 640)) {
    QueueMusic_Delayed8(kCrocomireMusic_Song0);
    *(uint16 *)&scrolls[3] = 256;
    SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x30, 0x03, 0xb757 });
    camera_distance_index = 6;
    Enemy_Crocomire *E0 = Get_Crocomire(0);
    E0->base.properties = (E0->base.properties & ~(kEnemyProps_SolidToSamus | kEnemyProps_Intangible)) | kEnemyProps_Intangible;
    Enemy_Crocomire *E1 = Get_Crocomire(0x40);
    E1->base.properties |= kEnemyProps_Invisible | kEnemyProps_Intangible;
    E0->crocom_var_D = 4;
    E1->crocom_var_D = 0;
    Get_Crocomire(0x80)->crocom_var_D = 10;
    Get_Crocomire(0xC0)->crocom_var_D = 1;
    E0->crocom_var_B = 0;
    E0->base.y_height = 56;
    Crocomire_9BB3();
  }
}

void Crocomire_Func_69(void) {  // 0xA49830
  Enemy_Crocomire *E = Get_Crocomire(0);
  uint16 v1 = E->base.x_pos - 2;
  E->base.x_pos = v1;
  if (sign16(v1 - 480)) {
    E->base.x_pos = 480;
    E->base.y_pos = 54;
    E->crocom_var_A = 62;
  } else {
    E->base.y_pos = 220;
  }
}

void Crocomire_Func_70(void) {  // 0xA49859
  Enemy_Crocomire *E0 = Get_Crocomire(0);
  Enemy_Crocomire *E1 = Get_Crocomire(0x40);
  Enemy_Crocomire *E2 = Get_Crocomire(0x80);
  Enemy_Crocomire *E3 = Get_Crocomire(0xc0);
  if (kCrocomireRumble[E0->crocom_var_D >> 1] == kCrocomireRumbleTerminator) {
    E1->crocom_var_D = -32640;
    E0->crocom_var_D = 128;
    for (int i = 30; i >= 0; i -= 2)
      palette_buffer[(i >> 1) + 176] = kCrocomireRumblePalette[i >> 1];
    Crocomire_9BB3();
  } else {
    uint16 crocom_var_D = E0->crocom_var_D;
    uint16 v3 = E1->crocom_var_D;
    int v4 = crocom_var_D >> 1;
    if (v3 == kCrocomireRumble[v4]) {
      if ((kCrocomireRumble[crocom_var_D >> 1] & 0x8000) != 0) {
        uint16 v7 = E2->crocom_var_D;
        if (v7) {
          E2->crocom_var_D = v7 - 1;
          E0->crocom_var_D = crocom_var_D - 2;
          QueueSfx2_Max6(kSfx2_CrocomirePostDeathRumble);
          return;
        }
        uint16 v8 = crocom_var_D + 2;
        E2->crocom_var_D = kCrocomireRumble[v8 >> 1];
        crocom_var_D = v8 + 2;
        E3->crocom_var_D = kCrocomireRumble[crocom_var_D >> 1];
      }
      E0->crocom_var_D = crocom_var_D + 2;
    } else {
      uint16 v5;
      if ((int16)(v3 - kCrocomireRumble[v4]) >= 0)
        v5 = v3 - E3->crocom_var_D;
      else
        v5 = E3->crocom_var_D + v3;
      E1->crocom_var_D = v5;
    }
  }
}

void Crocomire_Func_71(void) {  // 0xA4990A
  int16 v3;
  VramWriteEntry *v6;

  Enemy_Crocomire *E = Get_Crocomire(0);
  uint16 crocom_var_D = E->crocom_var_D;
  if (crocom_var_D) {
    E->crocom_var_D = crocom_var_D - 1;
    v3 = croco_target_0688;
    int v4 = croco_target_0688 >> 1;
    if (kCrocomireSkeletonVramDst[v4] != 0xFFFF) {
      uint16 v5 = vram_write_queue_tail;
      v6 = gVramWriteEntry(vram_write_queue_tail);
      v6->size = 512;
      v6->src.addr = kCrocomireSkeletonVramSrc[v4];
      *(uint16 *)&v6->src.bank = 173;
      v6->vram_dst = kCrocomireSkeletonVramDst[v4] + ((reg_OBSEL & 7) << 13);
      vram_write_queue_tail = v5 + 7;
      croco_target_0688 = v3 + 2;
    }
  } else {
    E->base.x_pos = 480;
    E->base.y_pos = 54;
    Get_Crocomire(0x80)->crocom_var_D = 80;
    E->crocom_var_E = 0;
    E->crocom_var_F = 0;
    SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x20, 0x03, 0xb753 });
    SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x1e, 0x03, 0xb757 });
    SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x70, 0x0b, 0xb747 });
    QueueSfx2_Max6(kSfx2_CrocomireWallExplodes);
    E->base.current_instruction = addr_kCrocomire_Ilist_E158;
    E->base.instruction_timer = 1;
    E->base.palette_index = 0;
    for (int i = 30; i >= 0; i -= 2)
      palette_buffer[(i >> 1) + 144] = kCrocomireSkeletonPalette[i >> 1];
    ClearEprojs();
    uint16 v8 = 8;
    int n = 8;
    do {
      SpawnEprojWithGfx(v8, cur_enemy_index, addr_kEproj_CrocomireSpikeWallPieces);
    } while (--n);
    QueueSfx2_Max6(kSfx2_CrocomireDestroysWall);
    Crocomire_9BB3();
  }
}

void Crocomire_Func_72(void) {  // 0xA499E5
  Enemy_Crocomire *E = Get_Crocomire(0);
  if (sign16(E->base.x_pos - 224)) {
    uint32 t = __PAIR32__(E->crocom_var_F, E->crocom_var_E) + 0x8000;
    if (!sign16((t >> 16) - 2))
      t = (t & 0xffff) | (2 << 16);
    SetHiLo(&E->crocom_var_F, &E->crocom_var_E, t);
    AddToHiLo(&E->base.x_pos, &E->base.x_subpos, t);
  }
  Enemy_Crocomire *E2 = Get_Crocomire(0x80);
  if (E2->crocom_var_D && E2->crocom_var_D-- == 1) {
    E->crocom_var_E = 0;
    E->base.current_instruction = addr_kCrocomire_Ilist_E14A;
    E->base.instruction_timer = 1;
    Crocomire_9BB3();
  }
}

void Crocomire_Func_73(void) {  // 0xA49A38
  Enemy_Crocomire *E = Get_Crocomire(0);
  uint32 t = __PAIR32__(E->crocom_var_F, E->crocom_var_E) + 2048;
  if (!sign16((t >> 16) - 5))
    t = (t & 0xffff) | (5 << 16);
  SetHiLo(&E->crocom_var_F, &E->crocom_var_E, t);
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, 0xe000);
  AddToHiLo(&E->base.x_pos, &E->base.x_subpos, t);
  if (!sign16(E->base.x_pos - 576)) {
    QueueSfx2_Max6(kSfx2_BigExplosion);
    Get_Crocomire(0x40)->base.palette_index = E->base.palette_index;
    E->base.current_instruction = addr_kCrocomire_Ilist_E158;
    E->base.instruction_timer = 1;
    Crocomire_9BB3();
  }
}

const uint16 *Crocomire_Instr_8(uint16 k, const uint16 *jp) {  // 0xA49A9B
  return Crocomire_Func_87(jp, 0xFFE0);
}

const uint16 *Crocomire_Instr_6(uint16 k, const uint16 *jp) {  // 0xA49AA0
  return Crocomire_Func_87(jp, 0);
}

const uint16 *Crocomire_Instr_9(uint16 k, const uint16 *jp) {  // 0xA49AA5
  return Crocomire_Func_87(jp, 0xFFF0);
}

const uint16 *Crocomire_Instr_5(uint16 k, const uint16 *jp) {  // 0xA49AAA
  return Crocomire_Func_87(jp, 0x10);
}

const uint16 *Crocomire_Instr_20(uint16 k, const uint16 *jp) {  // 0xA49AAF
  return Crocomire_Func_87(jp, 0);
}

const uint16 *Crocomire_Instr_21(uint16 k, const uint16 *jp) {  // 0xA49AB4
  return Crocomire_Func_87(jp, 8);
}

const uint16 *Crocomire_Instr_22(uint16 k, const uint16 *jp) {  // 0xA49AB9
  return Crocomire_Func_87(jp, 0x10);
}

const uint16 *Crocomire_Instr_23(uint16 k, const uint16 *jp) {  // 0xA49ABE
  return Crocomire_Func_87(jp, 0x18);
}

const uint16 *Crocomire_Instr_24(uint16 k, const uint16 *jp) {  // 0xA49AC3
  return Crocomire_Func_87(jp, 0x20);
}

const uint16 *Crocomire_Instr_10(uint16 k, const uint16 *jp) {  // 0xA49AC8
  return Crocomire_Func_87(jp, 0x28);
}

const uint16 *Crocomire_Instr_25(uint16 k, const uint16 *jp) {  // 0xA49ACD
  return Crocomire_Func_87(jp, 0x30);
}

const uint16 *Crocomire_Instr_26(uint16 k, const uint16 *jp) {  // 0xA49AD2
  return Crocomire_Func_87(jp, 0x38);
}

const uint16 *Crocomire_Instr_27(uint16 k, const uint16 *jp) {  // 0xA49AD7
  return Crocomire_Func_87(jp, 0x40);
}

const uint16 *Crocomire_Func_87(const uint16 *jp, uint16 a) {  // 0xA49ADA
  Enemy_Crocomire *E = Get_Crocomire(0);
  eproj_spawn_pt = (Point16U){ a + E->base.x_pos + (random_number & 7), E->base.y_height + E->base.y_pos - 16 };
  SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 0x15);
  return jp;
}

void Crocomire_Func_88(void) {  // 0xA49B06
  Enemy_Crocomire *E = Get_Crocomire(0);
  if (sign16(E->base.current_instruction + 0x1E3A)) {
    uint32 t = __PAIR32__(E->crocom_var_F, E->crocom_var_E) + 4096;
    if (!sign16((t >> 16) - 6))
      t = (t & 0xffff) | (6 << 16);
    SetHiLo(&E->crocom_var_F, &E->crocom_var_E, t);
  } else {
    E->base.current_instruction = addr_kCrocomire_Ilist_E1CC;
    E->base.instruction_timer = 1;
    E->base.x_pos += 64;
    E->base.y_pos += 21;
    E->base.y_height = 28;
    E->base.x_width = 40;
    SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x30, 0x03, 0xb753 });
    Enemy_ItemDrop_Crocomire(0);
    Crocomire_9BB3();
  }
}

void Crocomire_9B65(void) {  // 0xA49B65
  *(uint16 *)scrolls = 257;
  *(uint16 *)&scrolls[2] = 257;
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x1e, 0x03, 0xb753 });
  Crocomire_9BB3();
}

void Crocomire_Func_89(void) {  // 0xA49B7B
  Crocomire_9BB3();
}

void Crocomire_Func_90(void) {  // 0xA49B7D
  Enemy_Crocomire *E = Get_Crocomire(0);
  if (!sign16(E->base.current_instruction + 0x1E3A))
    Crocomire_9BB3();
}

void Crocomire_9B86(void) {  // 0xA49B86
  QueueMusic_Delayed8(kCrocomireMusic_Song1);
  camera_distance_index = 0;
  *(uint16 *)&boss_bits_for_area[area_index] |= kCrocomireBossBit;
  QueueMusic_Delayed8(kCrocomireMusic_Song1);
  Crocomire_Func_87(0, 0xFFF0);
  Crocomire_Func_87(0, 0x10);
  Crocomire_9BB3();
}

void Crocomire_9BB3(void) {  // 0xA49BB3
  Enemy_Crocomire *E = Get_Crocomire(0);
  ++E->crocom_var_A;
  ++E->crocom_var_A;
}

void Crocomire_Func_91(void) {  // 0xA49BBA
  Crocomire_9BB3();
}

void Crocomire_Func_92(void) {  // 0xA4B93D
  NormalEnemyTouchAi();
  Enemy_Crocomire *E = Get_Crocomire(0);
  E->crocom_var_B |= 0x4000;
  extra_samus_x_displacement = -4;
}

void Crocomire_Func_93(void) {  // 0xA4B951
  Enemy_Crocomire *E = Get_Crocomire(0);
  uint16 v1 = E->crocom_var_B & 0xF;
  if (sign16(v1 - 15))
    ++v1;
  E->crocom_var_B |= v1;
}

void Crocomire_Func_94(void) {  // 0xA4B968
  int proj = collision_detection_index;
  eproj_spawn_pt = (Point16U){ projectile_x_pos[proj], projectile_y_pos[proj] };
  uint16 explosion = ((projectile_type[proj] & kProjectileType_SuperMissile) == 0) ? 6 : 29;
  SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, explosion);
}

void Crocomire_Powerbomb(void) {  // 0xA4B992
  uint16 v2;

  Enemy_Crocomire *E = Get_Crocomire(0);
  if (!E->crocom_var_A) {
    if (kCrocomirePowerBombReactionEnable) {
      E->crocom_var_D = kCrocomirePowerBombReactionEnable;
      if (E->crocom_var_C != 24) {
        E->crocom_var_B = E->crocom_var_B & 0x3FF0 | 0x8000;
        E->crocom_var_E = 10;
        E->base.flash_timer += 4;
        E->base.ai_handler_bits |= 2;
        E->crocom_var_C = 24;
        uint16 spritemap_pointer = E->base.spritemap_pointer;
        uint16 v3;
        int n = *(uint16 *)RomPtr_A4(spritemap_pointer);
        while (1) {
          v2 = *((uint16 *)RomPtr_A4(spritemap_pointer) + 3);
          v3 = addr_kCrocomire_Ilist_BDAE;
          if (v2 == addr_kCrocomire_BigSprmap_D600)
            break;
          v3 = addr_kCrocomire_Ilist_BDB2;
          if (v2 == addr_kCrocomire_BigSprmap_D51C)
            break;
          spritemap_pointer += 8;
          if (!--n) {
            v3 = addr_kCrocomire_Ilist_BDB6;
            break;
          }
        }
        Enemy_Crocomire *E0 = Get_Crocomire(0);
        E0->base.current_instruction = v3;
        E0->base.instruction_timer = 1;
      }
    }
  }
}

void Crocomire_Func_95(void) {  // 0xA4BA05
  Enemy_Crocomire *E = Get_Crocomire(0);
  E->base.invincibility_timer = 0;
  bool apply_damage_reaction = (int16)(E->base.x_pos - E->base.x_width - 256 - layer1_x_pos) >= 0;
  if (!apply_damage_reaction) {
    uint16 type = projectile_type[collision_detection_index];
    uint16 steps = 0;
    uint16 kind = type & kProjectileType_TypeMask;
    if (kind != 0) {
      if (kind == kProjectileType_Missile)
        steps = kCrocomireStepsBackMissile;
      else if (kind == kProjectileType_SuperMissile)
        steps = kCrocomireStepsBackSuperMissile;
    } else {
      steps = kCrocomireStepsBackChargedBeam;
      if ((type & kCrocomireShotCharged) == 0) {
        E->base.instruction_timer = kCrocomireMouthOpenTimerUncharged;
        Crocomire_Func_1();
        return;
      }
    }
    if (steps) {
      E->crocom_var_D += steps;
      apply_damage_reaction = true;
    }
  }
  if (apply_damage_reaction) {
    uint16 v4 = E->crocom_var_B & 0xF;
    if (sign16(v4 - 15))
      ++v4;
    if ((E->crocom_var_B & 0x800) == 0) {
      uint16 v5 = kCrocomireMouthCloseDelay;
      if (E->crocom_var_C == 8)
        v5 = kCrocomireMouthCloseDelayDuringSpit;
      E->base.instruction_timer += v5;
    }
    E->crocom_var_B = v4 | E->crocom_var_B & 0xB7F0 | 0x800;
    E->crocom_var_E = 10;
  }
  E->base.flash_timer += 14;
  E->base.ai_handler_bits |= 2;
}

void Crocomire_Func_1(void) {  // 0xA4BAB4
  int proj = collision_detection_index;
  eproj_spawn_pt = (Point16U){ projectile_x_pos[proj], projectile_y_pos[proj] };
  uint16 explosion = ((projectile_type[proj] & kProjectileType_SuperMissile) == 0) ? 6 : 29;
  SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, explosion);
}

void CrocomireTongue_Init(void) {  // 0xA4F67A
  Enemy_Crocomire *E = Get_Crocomire(cur_enemy_index);
  if ((*(uint16 *)&boss_bits_for_area[area_index] & kCrocomireBossBit) != 0) {
    E->base.properties = E->base.properties & 0xDCFF | 0x300;
  } else {
    E->base.current_instruction = addr_kCrocomire_Ilist_BE56;
    E->base.extra_properties |= 0x404;
    E->base.instruction_timer = 1;
    E->crocom_var_A = 23;
    E->base.palette_index = 3584;
  }
}

void CrocomireTongue_Main(void) {  // 0xA4F6BB
  ;
}
