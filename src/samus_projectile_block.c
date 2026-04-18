// Projectile-vs-block collision and block-reaction helpers extracted from
// bank 94: bombs, power bombs, beams, missiles, and spread bomb block tests.

#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "samus_projectile.h"
#include "block_reaction.h"

#define kBlockBombedReactSpecialPlm ((uint16 *)RomPtr_94(0x9da4))
#define kBlockBombedReactRegionPlmPtrs ((uint16 *)RomPtr_94(0x9e44))
#define kBlockShotBombedReactionShootablePlm ((uint16 *)RomPtr_94(0x9ea6))
#define kShootableBlockRegionPlmPtrs ((uint16 *)RomPtr_94(0x9fc6))
#define kBombableBlockReactionPlm ((uint16 *)RomPtr_94(0xa012))
#define kAlignYPos_Tab0 ((uint8*)RomFixedPtr(0x948b2b))
#define kTab948E54 ((uint8*)RomFixedPtr(0x948e54))

static uint16 BlockFunc_9C73(uint16 k);
static void BlockFunc_A06A(uint16 k);
static void BlockFunc_A0F4(uint16 a, uint16 r22, uint16 r24, uint16 r26, uint16 r28);
static void BlockFunc_A11A(uint16 a, uint16 r24, uint16 r26, uint16 r28);
static void BlockBombedReact(CollInfo *ci, uint16 v0);
static void BlockColl_BombExplosion(CollInfo *ci, uint16 k);
static uint8 BlockShotReactHoriz_Slope_Square(CollInfo *ci, uint16 a, uint16 k);
static uint8 BlockShotReactVert_Slope_Square(CollInfo *ci, uint16 a, uint16 k);
static uint8 BlockShotReactHoriz_Slope_NonSquare(CollInfo *ci);
static uint8 BlockShotReactVert_Slope_NonSquare(CollInfo *ci);

static const uint8 kBlockFunc_9C73_Tab0[36] = {  // 0x949C73
   3,  20, 0,
  10,  40, 1,
   5,  30, 0,
  10,  40, 1,
  20,  80, 0,
  20,  80, 1,
  20,  80, 0,
  20,  80, 1,
  30, 100, 0,
  30, 100, 1,
  30, 100, 0,
  30, 100, 1,
};

static const int8 kBlockFunc_9C73_Tab1[18] = {
   0,  0,
  10,  0,
  50,  0,
  50,  3,
   0, -1,
  10,  2,
   0, -1,
   0, -1,
   0, -1,
};

static uint16 BlockFunc_9C73(uint16 v0) {
  int v1 = v0 >> 1;
  if ((projectile_type[v1] & 0xF00) != 0) {
    return kBlockFunc_9C73_Tab1[2 * (HIBYTE(projectile_type[v1]) & 0xF) + 1];
  } else {
    int r18 = projectile_type[v1] & 0xF;
    return kBlockFunc_9C73_Tab0[3 * r18 + 2];
  }
}

void BombOrPowerBomb_Func1(uint16 v0) {  // 0x949CAC
  int16 v2;
  int16 v3;

  uint16 temp_collision_DD2 = BlockFunc_9C73(v0);
  int v1 = v0 >> 1;
  v2 = projectile_x_pos[v1];
  if (v2 >= 0) {
    uint16 r26 = projectile_x_pos[v1];
    if ((int16)(HIBYTE(v2) - room_width_in_scrolls) < 0) {
      v3 = projectile_y_pos[v1];
      if (v3 >= 0) {
        uint16 R28 = projectile_y_pos[v1];
        if ((int16)(HIBYTE(v3) - room_height_in_scrolls) < 0) {
          cur_block_index = 0;
          CalculateBlockAt(r26, R28, 0, 0);
          CollInfo ci = { .ci_r24 = 0, .ci_r26 = r26, .ci_r28 = R28 };
          if (temp_collision_DD2 == 2)
            BlockColl_BombExplosion(&ci, v0);
          else
            BlockFunc_A06A(v0);
        }
      }
    }
  }
}

static uint16 BlockColl_GetBlockIndexAbove(uint16 k) {  // 0x949D34
  return k - room_width_in_blocks - room_width_in_blocks;
}

static uint16 BlockColl_GetBlockIndexRight(uint16 k) {  // 0x949D3E
  return room_width_in_blocks * 2 + k + 1 + 1;
}

static uint16 BlockColl_GetBlockIndexLeft(uint16 k) {  // 0x949D49
  return k - 4;
}

static uint16 BlockColl_GetBlockIndexBelow(uint16 k) {  // 0x949D4E
  return room_width_in_blocks * 2 + k + 1 + 1;
}

static void BlockColl_BombExplosion(CollInfo *ci, uint16 k) {  // 0x949CF4
  int16 v2;

  int v1 = k >> 1;
  if (!projectile_variables[v1]) {
    v2 = projectile_type[v1];
    if ((v2 & 1) == 0) {
      projectile_type[v1] = v2 | 1;
      if (cur_block_index != 0xFFFF) {
        uint16 v3 = 2 * cur_block_index;
        BlockBombedReact(ci, 2 * cur_block_index);
        uint16 idx = BlockColl_GetBlockIndexAbove(v3);
        BlockBombedReact(ci, idx);
        uint16 idx2 = BlockColl_GetBlockIndexRight(idx);
        BlockBombedReact(ci, idx2);
        idx = BlockColl_GetBlockIndexLeft(idx2);
        BlockBombedReact(ci, idx);
        idx = BlockColl_GetBlockIndexBelow(idx);
        BlockBombedReact(ci, idx);
      }
    }
  }
}

static uint8 ClearCarry_9(CollInfo *ci) {  // 0x949D59
  return 0;
}

static uint8 SetCarry_0(CollInfo *ci) {  // 0x949D5B
  return 1;
}

static uint8 BlockSpreadBombReact_Slope(CollInfo *ci) {  // 0x949D5D
  if ((BTS[cur_block_index] & 0x1F) < 5)
    return 1;
  else
    return BlockShotReactVert_Slope_NonSquare(ci);
}

static uint8 BlockBombedReact_Special(CollInfo *ci) {  // 0x949D71
  uint8 v0 = BTS[cur_block_index];
  if ((v0 & 0x80) != 0) {
    const uint16 *v2 = (uint16 *)RomPtr_94(kBlockBombedReactRegionPlmPtrs[area_index]);
    SpawnPLM(v2[v0 & 0x7F]);
  } else {
    SpawnPLM(kBlockBombedReactSpecialPlm[v0]);
  }
  return 1;
}

uint8 BlockReact_ShootableAir(CollInfo *ci) {  // 0x949E55
  uint8 v0 = BTS[cur_block_index];
  if ((v0 & 0x80) == 0) {
    uint16 t = kBlockShotBombedReactionShootablePlm[v0];
    if (t == 0xb974)
      ci->ci_r38 = 0, ci->ci_r40 = 0xFFFF;
    SpawnPLM(t);
  }
  return 0;
}

uint8 BlockReact_Shootable(CollInfo *ci) {  // 0x949E73
  uint8 v0 = BTS[cur_block_index];
  if ((v0 & 0x80) != 0) {
    uint16 *v2 = (uint16 *)RomPtr_94(kShootableBlockRegionPlmPtrs[area_index]);
    SpawnPLM(v2[v0 & 0x7F]);
  } else {
    uint16 t = kBlockShotBombedReactionShootablePlm[v0];
    if (t == 0xb974)
      ci->ci_r38 = 0, ci->ci_r40 = 0xFFFF;
    SpawnPLM(t);
  }
  return 1;
}

uint8 BlockReact_BombableAir(CollInfo *ci) {  // 0x949FD6
  uint8 v0 = BTS[cur_block_index];
  if ((v0 & 0x80) == 0)
    SpawnPLM(kBombableBlockReactionPlm[v0]);
  return 0;
}

uint8 BlockReact_BombableBlock(CollInfo *ci) {  // 0x949FF4
  uint8 v0 = BTS[cur_block_index];
  if ((v0 & 0x80) == 0)
    SpawnPLM(kBombableBlockReactionPlm[v0]);
  return 1;
}

static Func_CollInfo_U8 *const kBlockBombedReact[16] = {  // 0x94A052
  ClearCarry_9,
  ClearCarry_9,
  ClearCarry_9,
  ClearCarry_9,
  BlockReact_ShootableAir,
  BlockReact_HorizExt,
  ClearCarry_9,
  BlockReact_BombableAir,
  SetCarry_0,
  SetCarry_0,
  SetCarry_0,
  BlockBombedReact_Special,
  BlockReact_Shootable,
  BlockReact_VertExt,
  SetCarry_0,
  BlockReact_BombableBlock,
};

static void BlockBombedReact(CollInfo *ci, uint16 v0) {
  cur_block_index = v0 >> 1;
  while (kBlockBombedReact[(level_data[cur_block_index] & 0xF000) >> 12](ci) & 0x80) {
  }
}

static void BlockFunc_A06A(uint16 k) {  // 0x94A06A
  int16 v2;
  int16 v3;

  int v1 = k >> 1;
  v2 = projectile_variables[v1];
  if (v2) {
    if (v2 < 0)
      projectile_variables[v1] = 0;
  } else {
    uint16 r18 = HIBYTE(power_bomb_explosion_radius);
    uint16 r20 = (uint16)(3 * HIBYTE(power_bomb_explosion_radius)) >> 2;
    v3 = power_bomb_explosion_x_pos - HIBYTE(power_bomb_explosion_radius);
    if (v3 < 0)
      v3 = 0;
    uint16 r22 = (uint16)v3 >> 4;
    uint16 v4 = (uint16)(r18 + power_bomb_explosion_x_pos) >> 4;
    if (v4 >= room_width_in_blocks)
      v4 = room_width_in_blocks - 1;
    uint16 r24 = v4;
    uint16 v5 = power_bomb_explosion_y_pos - r20;
    if ((int16)v5 < 0)
      v5 = 0;
    uint16 r26 = v5 >> 4;
    uint16 v6 = (uint16)(r20 + power_bomb_explosion_y_pos) >> 4;
    if (v6 >= room_height_in_blocks)
      v6 = room_height_in_blocks - 1;
    uint16 R28 = v6;
    uint16 a = r26 | (r22 << 8);
    BlockFunc_A0F4(a, r22, r24, r26, R28);
    BlockFunc_A11A(a, r24, r26, R28);
    BlockFunc_A0F4(R28 | (r22 << 8), r22, r24, r26, R28);
    BlockFunc_A11A(r26 | (r24 << 8), r24, r26, R28);
  }
}

static void BlockFunc_A0F4(uint16 a, uint16 r22, uint16 r24, uint16 r26, uint16 r28) {  // 0x94A0F4
  uint16 RegWord = Mult8x8(a, room_width_in_blocks);
  uint16 v3 = 2 * (RegWord + HIBYTE(a));
  int16 v4 = r24 - r22;
  CollInfo ci = { .ci_r24 = r24, .ci_r26 = r26, .ci_r28 = r28 };
  do {
    BlockBombedReact(&ci, v3);
    v3 += 2;
  } while (--v4 >= 0);
}

static void BlockFunc_A11A(uint16 a, uint16 r24, uint16 r26, uint16 r28) {  // 0x94A11A
  uint16 RegWord = Mult8x8(a, room_width_in_blocks);
  uint16 v3 = 2 * (RegWord + HIBYTE(a));
  int16 v4 = r28 - r26;
  CollInfo ci = { .ci_r24 = r24, .ci_r26 = r26, .ci_r28 = r28 };
  do {
    BlockBombedReact(&ci, v3);
    v3 += room_width_in_blocks * 2;
  } while (--v4 >= 0);
}

static uint8 BlockShotReactHoriz_Slope(CollInfo *ci) {  // 0x94A147
  if ((BTS[cur_block_index] & 0x1F) < 5)
    return BlockShotReactVert_Slope_Square(ci, BTS[cur_block_index] & 0x1F, cur_block_index);
  else
    return BlockShotReactVert_Slope_NonSquare(ci);
}

static uint8 BlockShotReactVert_Slope(CollInfo *ci) {  // 0x94A15E
  if ((BTS[cur_block_index] & 0x1F) < 5)
    return BlockShotReactHoriz_Slope_Square(ci, BTS[cur_block_index] & 0x1F, cur_block_index);
  else
    return BlockShotReactHoriz_Slope_NonSquare(ci);
}

static uint8 BlockShotReactHoriz(CollInfo *ci, uint16 k) {  // 0x94A1B5
  static Func_CollInfo_U8 *const kBlockShotReactHoriz[16] = {
    ClearCarry_9,
    BlockShotReactHoriz_Slope,
    ClearCarry_9,
    ClearCarry_9,
    BlockReact_ShootableAir,
    BlockReact_HorizExt,
    ClearCarry_9,
    BlockReact_BombableAir,
    SetCarry_0,
    SetCarry_0,
    SetCarry_0,
    SetCarry_0,
    BlockReact_Shootable,
    BlockReact_VertExt,
    SetCarry_0,
    BlockReact_BombableBlock,
  };
  uint8 v1;
  if (k >= room_size_in_blocks)
    return 1;
  cur_block_index = k >> 1;
  do {
    v1 = kBlockShotReactHoriz[(level_data[cur_block_index] & 0xF000) >> 12](ci);
  } while (v1 & 0x80);
  if (v1)
    ci->ci_r40--;
  return v1;
}

static uint8 BlockShotReactVert(CollInfo *ci, uint16 k) {  // 0x94A1D6
  static Func_CollInfo_U8 *const kBlockShotReactVert[16] = {
    ClearCarry_9,
    BlockShotReactVert_Slope,
    ClearCarry_9,
    ClearCarry_9,
    BlockReact_ShootableAir,
    BlockReact_HorizExt,
    ClearCarry_9,
    BlockReact_BombableAir,
    SetCarry_0,
    SetCarry_0,
    SetCarry_0,
    SetCarry_0,
    BlockReact_Shootable,
    BlockReact_VertExt,
    SetCarry_0,
    BlockReact_BombableBlock,
  };
  uint8 v1;
  if (k >= room_size_in_blocks)
    return 1;
  cur_block_index = k >> 1;
  do {
    v1 = kBlockShotReactVert[(level_data[cur_block_index] & 0xF000) >> 12](ci);
  } while (v1 & 0x80);
  if (v1)
    --ci->ci_r40;
  return v1;
}

static uint16 BlockGetYSpanInBlocks(uint16 k) {  // 0x94A1F7
  int v1 = k >> 1;
  uint16 r38 = (projectile_y_pos[v1] - projectile_y_radius[v1]) & 0xFFF0;
  return (uint16)(projectile_y_radius[v1] + projectile_y_pos[v1] - 1 - r38) >> 4;
}

static uint16 BlockGetXSpanInBlocks(uint16 k) {  // 0x94A219
  int v1 = k >> 1;
  uint16 r38 = (projectile_x_pos[v1] - projectile_x_radius[v1]) & 0xFFF0;
  return (uint16)(projectile_x_radius[v1] + projectile_x_pos[v1] - 1 - r38) >> 4;
}

uint8 BlockCollNoWaveBeamHoriz(uint16 k) {  // 0x94A23B
  uint16 v7;

  int v1 = k >> 1;
  int32 amt = INT16_SHL8(projectile_bomb_x_speed[v1]);
  uint16 some_pos = BlockGetYSpanInBlocks(k);
  uint16 prod = Mult8x8((uint16)(projectile_y_pos[v1] - projectile_y_radius[v1]) >> 4, room_width_in_blocks);

  AddToHiLo(&projectile_x_pos[v1], &projectile_bomb_x_subpos[v1], amt);
  uint16 r24 = projectile_x_pos[v1];
  if (amt < 0)
    v7 = projectile_x_pos[v1] - projectile_x_radius[v1];
  else
    v7 = projectile_x_radius[v1] + projectile_x_pos[v1] - 1;
  uint16 R28 = v7;
  uint16 v8 = 2 * (prod + (v7 >> 4));
  if (!sign16(some_pos - 16) || (int16)(HIBYTE(R28) - room_width_in_scrolls) >= 0)
    return 0;
  CollInfo ci = { .ci_r18_r20 = amt, .ci_r24 = r24, .ci_r26 = some_pos, .ci_r28 = R28, .ci_r30 = 0, .ci_r38 = some_pos, .ci_r40 = some_pos };
  do {
    BlockShotReactHoriz(&ci, v8);
    v8 += room_width_in_blocks * 2;
  } while ((--ci.ci_r38 & 0x8000) == 0);
  if ((ci.ci_r40 & 0x8000) == 0)
    return 0;
  KillProjectile(k);
  return 1;
}

uint8 BlockCollNoWaveBeamVert(uint16 k) {  // 0x94A2CA
  uint16 v7;

  int v1 = k >> 1;
  int32 amt = INT16_SHL8(projectile_bomb_y_speed[v1]);
  uint16 some_pos = BlockGetXSpanInBlocks(k);
  AddToHiLo(&projectile_y_pos[v1], &projectile_bomb_y_subpos[v1], amt);
  uint16 r24 = projectile_y_pos[v1];
  if (amt < 0)
    v7 = projectile_y_pos[v1] - projectile_y_radius[v1];
  else
    v7 = projectile_y_radius[v1] + projectile_y_pos[v1] - 1;
  uint16 R28 = v7;
  uint16 v8 = (uint16)(projectile_x_pos[v1] - projectile_x_radius[v1]) >> 4;
  uint16 v9 = 2 * (Mult8x8(v7 >> 4, room_width_in_blocks) + v8);
  if (!sign16(some_pos - 16) || (int16)(HIBYTE(R28) - room_height_in_scrolls) >= 0)
    return 0;
  CollInfo ci = { .ci_r18_r20 = amt, .ci_r24 = r24, .ci_r26 = some_pos, .ci_r28 = R28, .ci_r30 = 0, .ci_r38 = some_pos, .ci_r40 = some_pos };
  do {
    BlockShotReactVert(&ci, v9);
    v9 += 2;
  } while ((--ci.ci_r38 & 0x8000) == 0);
  if ((ci.ci_r40 & 0x8000) == 0)
    return 0;
  KillProjectile(k);
  return 1;
}

uint8 BlockCollWaveBeamHoriz(uint16 k) {  // 0x94A352
  uint16 v7;

  int v1 = k >> 1;
  int32 amt = INT16_SHL8(projectile_bomb_x_speed[v1]);
  uint16 some_pos = BlockGetYSpanInBlocks(k);
  uint16 prod = Mult8x8((uint16)(projectile_y_pos[v1] - projectile_y_radius[v1]) >> 4, room_width_in_blocks);
  AddToHiLo(&projectile_x_pos[v1], &projectile_bomb_x_subpos[v1], amt);
  uint16 r24 = projectile_x_pos[v1];
  if (amt < 0)
    v7 = projectile_x_pos[v1] - projectile_x_radius[v1];
  else
    v7 = projectile_x_radius[v1] + projectile_x_pos[v1] - 1;
  uint16 R28 = v7;
  uint16 v8 = 2 * (prod + (v7 >> 4));
  if (!sign16(some_pos - 16))
    return 0;

  CollInfo ci = { .ci_r18_r20 = amt, .ci_r24 = r24, .ci_r26 = some_pos, .ci_r28 = R28, .ci_r30 = 0, .ci_r38 = some_pos, .ci_r40 = some_pos };
  uint8 v9 = HIBYTE(projectile_y_pos[v1]);
  if (!sign8(v9) && (int16)(v9 - room_height_in_scrolls) < 0 && (int16)(HIBYTE(R28) - room_width_in_scrolls) < 0) {
    do {
      BlockShotReactHoriz(&ci, v8);
      v8 += room_width_in_blocks * 2;
    } while ((--ci.ci_r38 & 0x8000) == 0);
  }
  return 0;
}

uint8 BlockCollWaveBeamVert(uint16 k) {  // 0x94A3E4
  uint16 v7;

  int v1 = k >> 1;
  int32 amt = INT16_SHL8(projectile_bomb_y_speed[v1]);
  uint16 some_pos = BlockGetXSpanInBlocks(k);
  AddToHiLo(&projectile_y_pos[v1], &projectile_bomb_y_subpos[v1], amt);
  uint16 r24 = projectile_y_pos[v1];
  if (amt < 0)
    v7 = projectile_y_pos[v1] - projectile_y_radius[v1];
  else
    v7 = projectile_y_radius[v1] + projectile_y_pos[v1] - 1;
  uint16 R28 = v7;
  uint16 prod = Mult8x8(v7 >> 4, room_width_in_blocks);
  uint16 v8 = (uint16)(projectile_x_pos[v1] - projectile_x_radius[v1]) >> 4;
  uint16 v9 = 2 * (prod + v8);
  if (!sign16(some_pos - 16))
    return 0;
  uint8 v10 = HIBYTE(projectile_x_pos[v1]);
  CollInfo ci = { .ci_r18_r20 = amt, .ci_r24 = r24, .ci_r26 = some_pos, .ci_r28 = R28, .ci_r30 = 0, .ci_r38 = some_pos, .ci_r40 = some_pos };
  if (!sign8(v10) && (int16)(v10 - room_width_in_scrolls) < 0 && (int16)(HIBYTE(R28) - room_height_in_scrolls) < 0) {
    do {
      BlockShotReactVert(&ci, v9);
      v9 += 2;
    } while ((--ci.ci_r38 & 0x8000) == 0);
  }
  return 0;
}

uint8 BlockCollMissileHoriz(uint16 k) {  // 0x94A46F
  int v1 = k >> 1;
  int32 amt = INT16_SHL8(projectile_bomb_x_speed[v1]);
  uint16 prod = Mult8x8(projectile_y_pos[v1] >> 4, room_width_in_blocks);
  AddToHiLo(&projectile_x_pos[v1], &projectile_bomb_x_subpos[v1], amt);
  uint16 r24 = projectile_x_pos[v1];
  uint16 r28 = r24;
  uint16 v7 = 2 * (prod + (projectile_x_pos[v1] >> 4));
  CollInfo ci = { .ci_r18_r20 = amt, .ci_r24 = r24, .ci_r26 = 0, .ci_r28 = r28, .ci_r30 = 1, .ci_r38 = 0 };
  if ((int16)(HIBYTE(r24) - room_width_in_scrolls) >= 0 || !BlockShotReactHoriz(&ci, v7))
    return 0;
  KillProjectile(k);
  return 1;
}

uint8 BlockCollMissileVert(uint16 k) {  // 0x94A4D9
  int v1 = k >> 1;
  int32 amt = INT16_SHL8(projectile_bomb_y_speed[v1]);
  AddToHiLo(&projectile_y_pos[v1], &projectile_bomb_y_subpos[v1], amt);
  uint16 r24 = projectile_y_pos[v1];
  uint16 r28 = r24;
  uint16 prod = Mult8x8(projectile_y_pos[v1] >> 4, room_width_in_blocks);
  uint16 v7 = projectile_x_pos[v1] >> 4;
  uint16 v8 = 2 * (prod + v7);
  CollInfo ci = { .ci_r18_r20 = amt, .ci_r24 = r24, .ci_r26 = 0, .ci_r28 = r28, .ci_r30 = 1, .ci_r38 = 0 };
  if ((int16)(HIBYTE(r24) - room_height_in_scrolls) >= 0 || !BlockShotReactVert(&ci, v8))
    return 0;
  KillProjectile(k);
  return 1;
}

static uint8 BlockShotReact_Slope_NonSquare(CollInfo *ci, uint16 j, uint16 k) {  // 0x94A58F
  uint16 v2 = (projectile_x_pos[j >> 1] & 0xF) ^ ((BTS[k] & 0x40) != 0 ? 0xF : 0);
  uint16 v4 = (projectile_y_pos[j >> 1] & 0xF) ^ ((BTS[k] & 0x80) != 0 ? 0xF : 0);
  uint16 v5 = kAlignYPos_Tab0[16 * (BTS[k] & 0x1F) + (v2 & 0xF)] & 0x1F;
  if ((int16)(v5 - v4) <= 0) {
    ci->ci_r38 = 0;
    ci->ci_r40 = 0;
    return 1;
  } else {
    return 0;
  }
}

static uint8 BlockShotReactVert_Slope_NonSquare(CollInfo *ci) {  // 0x94A543
  uint16 v0 = cur_block_index;
  uint16 v1 = projectile_index;
  uint16 div = SnesDivide(cur_block_index, room_width_in_blocks);
  if ((projectile_y_pos[v1 >> 1] >> 4) == div)
    return BlockShotReact_Slope_NonSquare(ci, v1, v0);
  else
    return 0;
}

static uint8 BlockShotReactHoriz_Slope_NonSquare(CollInfo *ci) {  // 0x94A569
  uint16 v0 = cur_block_index;
  uint16 v1 = projectile_index;
  uint16 mod = SnesModulus(cur_block_index, room_width_in_blocks);
  if ((projectile_x_pos[v1 >> 1] >> 4) == mod)
    return BlockShotReact_Slope_NonSquare(ci, v1, v0);
  else
    return 0;
}

uint8 BlockCollSpreadBomb(uint16 k) {  // 0x94A621
  static Func_CollInfo_U8 *const kBlockCollSpreadBomb[16] = {
    ClearCarry_9,
    BlockSpreadBombReact_Slope,
    ClearCarry_9,
    ClearCarry_9,
    SetCarry_0,
    BlockReact_HorizExt,
    ClearCarry_9,
    ClearCarry_9,
    SetCarry_0,
    SetCarry_0,
    SetCarry_0,
    SetCarry_0,
    SetCarry_0,
    BlockReact_VertExt,
    SetCarry_0,
    SetCarry_0,
  };
  int v1 = k >> 1;
  uint16 r26 = projectile_x_pos[v1];
  uint16 r28 = projectile_y_pos[v1];
  cur_block_index = 0;
  CalculateBlockAt(r26, r28, 0, 0);
  CollInfo ci = { .ci_r24 = 0, .ci_r26 = r26, .ci_r28 = r28, .ci_r30 = 0, .ci_r32 = 0 };
  if (!projectile_variables[v1]) {
    BlockColl_BombExplosion(&ci, k);
    return 0;
  }
  if (cur_block_index == 0xFFFF)
    return 1;
  uint8 rv;
  do {
    rv = kBlockCollSpreadBomb[(level_data[cur_block_index] & 0xF000) >> 12](&ci);
  } while (rv & 0x80);
  return rv;
}

static uint8 BlockShotReactVert_Slope_Square(CollInfo *ci, uint16 a, uint16 k) {  // 0x94A66A
  uint16 temp_collision_DD4 = 4 * a;
  uint16 temp_collision_DD6 = BTS[k] >> 6;
  uint16 v2 = 4 * a + (temp_collision_DD6 ^ ((ci->ci_r28 & 8) >> 3));
  int v3 = projectile_index >> 1;
  if (!ci->ci_r30) {
    if (!ci->ci_r26) {
      if (((projectile_y_pos[v3] - projectile_y_radius[v3]) & 8) != 0 || !kTab948E54[v2]) {
        uint16 v4 = v2 ^ 2;
        if (((LOBYTE(projectile_y_radius[v3]) + LOBYTE(projectile_y_pos[v3]) - 1) & 8) == 0 || !kTab948E54[v4]) {
          return 0;
        }
      }
      return 1;
    }
    if (ci->ci_r38) {
      if (ci->ci_r38 == ci->ci_r26 && ((projectile_y_pos[v3] - projectile_y_radius[v3]) & 8) != 0) {
        return kTab948E54[v2 ^ 2] != 0;
      }
    } else if (((LOBYTE(projectile_y_radius[v3]) + LOBYTE(projectile_y_pos[v3]) - 1) & 8) == 0) {
      return kTab948E54[v2] != 0;
    }
    if (kTab948E54[v2] != 0)
      return 1;
    return kTab948E54[v2 ^ 2] != 0;
  }
  if ((projectile_y_pos[v3] & 8) != 0)
    v2 ^= 2;
  return kTab948E54[v2] != 0;
}

static uint8 BlockShotReactHoriz_Slope_Square(CollInfo *ci, uint16 a, uint16 k) {  // 0x94A71A
  uint16 temp_collision_DD4 = 4 * a;
  uint16 temp_collision_DD6 = BTS[k] >> 6;
  uint16 v2 = 4 * a + (temp_collision_DD6 ^ ((ci->ci_r28 & 8) >> 2));
  if (!ci->ci_r30) {
    if (!ci->ci_r26) {
      int v3 = projectile_index >> 1;
      if (((projectile_x_pos[v3] - projectile_x_radius[v3]) & 8) != 0 || kTab948E54[v2] == 0) {
        uint16 v4 = v2 ^ 1;
        if (((LOBYTE(projectile_x_radius[v3]) + LOBYTE(projectile_x_pos[v3]) - 1) & 8) == 0 || kTab948E54[v4] == 0) {
          return 0;
        }
      }
      return 1;
    }
    if (ci->ci_r38) {
      if (ci->ci_r38 == ci->ci_r26 && ((projectile_x_pos[projectile_index >> 1] - projectile_x_radius[projectile_index >> 1]) & 8) != 0) {
        return kTab948E54[v2 ^ 1] != 0;
      }
    } else if (((LOBYTE(projectile_x_radius[projectile_index >> 1]) + LOBYTE(projectile_x_pos[projectile_index >> 1]) - 1) & 8) == 0) {
      return kTab948E54[v2] != 0;
    }
    if (kTab948E54[v2] != 0)
      return 1;
    return kTab948E54[v2 ^ 1] != 0;
  }
  if ((projectile_x_pos[projectile_index >> 1] & 8) != 0)
    v2 ^= 1;
  return kTab948E54[v2] != 0;
}
