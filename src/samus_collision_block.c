// Samus block collision layer: block-type dispatch, slope/solid reactions,
// block traversal helpers, and the movement-facing block handlers used by
// Samus movement, wall-jumps, and pose-change collision.

#include "block_reaction.h"
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"

#define kAlignYPos_Tab0 ((uint8*)RomFixedPtr(0x948b2b))
#define kBlockCollSquareSlopeTab ((uint8*)RomFixedPtr(0x948e54))
#define kPlmHeaderDefPtrs ((uint16*)RomFixedPtr(0x949139))
#define kSpecialAirPlmLists ((uint16*)RomFixedPtr(0x9492d9))
#define kSpecialBlockPlmLists ((uint16*)RomFixedPtr(0x9492e9))
#define kBombBlockPlmList ((uint16*)RomFixedPtr(0x94936b))

int32 *cur_coll_amt32;

static const uint16 kBlockColl_Horiz_Slope_NonSquare_Tab[64] = {
       0, 0x100,
       0, 0x100,
       0, 0x100,
       0, 0x100,
       0, 0x100,
       0, 0x100,
       0, 0x100,
       0, 0x100,
       0, 0x100,
       0, 0x100,
       0, 0x100,
       0, 0x100,
       0, 0x100,
       0, 0x100,
  0x1000,  0xb0,
  0x1000,  0xb0,
       0, 0x100,
       0, 0x100,
  0x1000,  0xc0,
       0, 0x100,
  0x1000,  0xc0,
  0x1000,  0xc0,
   0x800,  0xd8,
   0x800,  0xd8,
   0x600,  0xf0,
   0x600,  0xf0,
   0x600,  0xf0,
  0x4000,  0x80,
  0x4000,  0x80,
  0x6000,  0x50,
  0x6000,  0x50,
  0x6000,  0x50,
};

static uint8 BlockColl_Horiz_Slope_NonSquare(CollInfo *ci) {  // 0x9484D6
  if ((current_slope_bts & kSlopeBts_Ceiling) != 0 || __PAIR32__(samus_y_speed, samus_y_subspeed))
    return 0;
  uint16 v1 = 4 * (current_slope_bts & kSlopeBts_ShapeMask);
  uint16 v2 = ci->ci_r18_r20 >> 8;
  if (ci->ci_r18_r20 >= 0) {
    ci->ci_r18_r20 = Multiply16x16(v2, kBlockColl_Horiz_Slope_NonSquare_Tab[(v1 >> 1) + 1]);
  } else {
    ci->ci_r18_r20 = -(int32)Multiply16x16(-v2, kBlockColl_Horiz_Slope_NonSquare_Tab[(v1 >> 1) + 1]);
  }
  return 0;
}

static uint8 BlockColl_Vert_Slope_NonSquare(CollInfo *ci, uint16 k) {  // 0x9486FE
  if (samus_collision_direction & 1) {
    if ((samus_x_pos >> kBlockPixelShift) != SnesModulus(cur_block_index, room_width_in_blocks))
      return 0;
    if (BTS[k] & kSlopeBts_Ceiling)
      return 0;
    uint16 v10 = (BTS[k] & kSlopeBts_MirrorX) != 0 ? samus_x_pos ^ kBlockPixelMask : samus_x_pos;
    uint16 v11 = kBlockPixelSize * (BTS[k] & kSlopeBts_ShapeMask) + (v10 & kBlockPixelMask);
    int16 v12 = (kAlignYPos_Tab0[v11] & kSlopeBts_ShapeMask)
              - ((samus_y_radius + ci->ci_r24 - 1) & kBlockPixelMask) - 1;
    if (v12 <= 0) {
      int16 v13 = (ci->ci_r18_r20 >> 16) + v12;
      if (v13 < 0)
        v13 = 0;
      ci->ci_r18_r20 = v13 << 16;
      return 1;
    }
    return 0;
  }

  if ((samus_x_pos >> kBlockPixelShift) != SnesModulus(cur_block_index, room_width_in_blocks))
    return 0;
  if (!(BTS[k] & kSlopeBts_Ceiling))
    return 0;
  uint16 v4 = (BTS[k] & kSlopeBts_MirrorX) != 0 ? (samus_x_pos ^ kBlockPixelMask) : samus_x_pos;
  uint16 v5 = kBlockPixelSize * (BTS[k] & kSlopeBts_ShapeMask) + (v4 & kBlockPixelMask);
  int16 v6 = (kAlignYPos_Tab0[v5] & kSlopeBts_ShapeMask)
            - (((ci->ci_r24 - samus_y_radius) & kBlockPixelMask) ^ kBlockPixelMask) - 1;
  if (v6 <= 0) {
    int16 v7 = (ci->ci_r18_r20 >> 16) + v6;
    if (v7 < 0)
      v7 = 0;
    ci->ci_r18_r20 = v7 << 16;
    return 1;
  }
  return 0;
}

static uint8 BlockColl_Horiz_Slope_Square(CollInfo *ci, uint16 a, uint16 k) {  // 0x948D2B
  uint16 temp_collision_DD4 = 4 * a;
  uint16 temp_collision_DD6 = BTS[k] >> 6;
  uint16 v2 = 4 * a + (temp_collision_DD6 ^ ((ci->ci_r32 & 8) >> 3));
  if (!ci->ci_r26) {
    if (((samus_y_radius + samus_y_pos - 1) & 8) == 0) {
      if (!kBlockCollSquareSlopeTab[v2])
        return 0;
      goto block_collision;
    }
    goto check_next_quadrant;
  }
  if (ci->ci_r26 != ci->ci_r28 || ((samus_y_pos - samus_y_radius) & 8) == 0) {
check_next_quadrant:
    if (kBlockCollSquareSlopeTab[v2])
      goto block_collision;
  }
  if (!kBlockCollSquareSlopeTab[v2 ^ 2])
    return 0;

block_collision:
  if (ci->ci_r18_r20 < 0) {
    int16 v5 = samus_x_radius + (ci->ci_r32 | 7) + 1 - samus_x_pos;
    if (v5 >= 0)
      v5 = 0;
    ci->ci_r18_r20 = v5 << 16;
    samus_x_subpos = 0;
  } else {
    int16 v4 = (ci->ci_r32 & 0xFFF8) - samus_x_radius - samus_x_pos;
    if (v4 < 0)
      v4 = 0;
    ci->ci_r18_r20 = v4 << 16;
    samus_x_subpos = -1;
  }
  return 1;
}

static uint8 BlockColl_Vert_Slope_Square(CollInfo *ci, uint16 a, uint16 k) {  // 0x948DBD
  uint16 temp_collision_DD4 = 4 * a;
  uint16 temp_collision_DD6 = BTS[k] >> 6;
  uint16 v2 = 4 * a + (temp_collision_DD6 ^ ((ci->ci_r32 & 8) >> 2));
  if (!ci->ci_r26) {
    if (((samus_x_radius + samus_x_pos - 1) & 8) == 0) {
      if (!kBlockCollSquareSlopeTab[v2])
        return 0;
      goto block_collision;
    }
    goto check_next_quadrant;
  }
  if (ci->ci_r26 != ci->ci_r28 || ((samus_x_pos - samus_x_radius) & 8) == 0) {
check_next_quadrant:
    if (kBlockCollSquareSlopeTab[v2])
      goto block_collision;
  }
  if (!kBlockCollSquareSlopeTab[v2 ^ 1])
    return 0;

block_collision:
  if (ci->ci_r18_r20 < 0) {
    int16 v5 = samus_y_radius + (ci->ci_r32 | 7) + 1 - samus_y_pos;
    if (v5 >= 0)
      v5 = 0;
    ci->ci_r18_r20 = v5 << 16;
    samus_y_subpos = 0;
    return 1;
  }

  int16 v4 = (ci->ci_r32 & 0xFFF8) - samus_y_radius - samus_y_pos;
  if (v4 < 0)
    v4 = 0;
  ci->ci_r18_r20 = v4 << 16;
  samus_y_subpos = -1;
  samus_pos_adjusted_by_slope_flag = 1;
  return 1;
}

static uint8 ClearCarry_4(CollInfo *ci) {  // 0x948F47
  return 0;
}

static uint8 BlockColl_Horiz_SolidShootGrappleBlock(CollInfo *ci) {  // 0x948F49
  if (ci->ci_r18_r20 < 0) {
    int16 v2 = samus_x_radius + (ci->ci_r32 | 0xF) + 1 - samus_x_pos;
    if (v2 >= 0)
      v2 = 0;
    ci->ci_r18_r20 = v2 << 16;
    samus_x_subpos = 0;
    return 1;
  }

  int16 v0 = (ci->ci_r32 & 0xFFF0) - samus_x_radius - samus_x_pos;
  if (v0 < 0)
    v0 = 0;
  ci->ci_r18_r20 = v0 << 16;
  samus_x_subpos = -1;
  return 1;
}

static uint8 BlockColl_Vert_SolidShootGrappleBlock(CollInfo *ci) {  // 0x948F82
  if (ci->ci_r18_r20 < 0) {
    int16 v2 = samus_y_radius + (ci->ci_r32 | 0xF) + 1 - samus_y_pos;
    if (v2 >= 0)
      v2 = 0;
    ci->ci_r18_r20 = v2 << 16;
    samus_y_subpos = 0;
    return 1;
  }

  int16 v0 = (ci->ci_r32 & 0xFFF0) - samus_y_radius - samus_y_pos;
  if (v0 < 0)
    v0 = 0;
  ci->ci_r18_r20 = v0 << 16;
  samus_y_subpos = -1;
  return 1;
}

static uint8 BlockColl_Horiz_Slope(CollInfo *ci) {  // 0x948FBB
  uint16 v0 = BTS[cur_block_index] & kSlopeBts_ShapeMask;
  if (v0 < kSlopeBts_FirstAlignedShape)
    return BlockColl_Horiz_Slope_Square(ci, v0, cur_block_index);
  current_slope_bts = BTS[cur_block_index];
  return BlockColl_Horiz_Slope_NonSquare(ci);
}

static uint8 BlockColl_Vert_Slope(CollInfo *ci) {  // 0x948FDA
  uint16 v0 = BTS[cur_block_index] & kSlopeBts_ShapeMask;
  if (v0 < kSlopeBts_FirstAlignedShape)
    return BlockColl_Vert_Slope_Square(ci, v0, cur_block_index);
  current_slope_bts = BTS[cur_block_index];
  return BlockColl_Vert_Slope_NonSquare(ci, cur_block_index);
}

static uint8 ClearCarry_5(CollInfo *ci) {  // 0x949018
  return 0;
}

static uint8 BlockColl_Vert_SpikeAir(CollInfo *ci) {  // 0x94901A
  return 0;
}

static void BlockColl_SpikeBlock_BTS0(void) {  // 0x948E83
  if ((area_index != 3 || CheckBossBitForCurArea(1) & 1) && !samus_invincibility_timer) {
    samus_invincibility_timer = 60;
    samus_knockback_timer = 10;
    samus_periodic_damage += 60;
    knockback_x_dir = ((samus_pose_x_dir ^ 0xC) & 8) != 0;
  }
}

static void BlockColl_SpikeBlock_BTS1(void) {  // 0x948ECF
  if (!samus_invincibility_timer) {
    samus_invincibility_timer = 60;
    samus_knockback_timer = 10;
    samus_periodic_damage += 16;
    knockback_x_dir = ((samus_pose_x_dir ^ 0xC) & 8) != 0;
  }
}

static void BlockColl_SpikeBlock_BTS3(void) {  // 0x948F0A
  if (!samus_invincibility_timer) {
    samus_invincibility_timer = 60;
    samus_knockback_timer = 10;
    samus_periodic_damage += 16;
    knockback_x_dir = ((samus_pose_x_dir ^ 0xC) & 8) != 0;
  }
}

static void SetCarry_Spikeblk(void) {
}

static Func_V *const kBlockColl_SpikeBlock[16] = {  // 0x94904B
  BlockColl_SpikeBlock_BTS0,
  BlockColl_SpikeBlock_BTS1,
  SetCarry_Spikeblk,
  BlockColl_SpikeBlock_BTS3,
  SetCarry_Spikeblk,
  SetCarry_Spikeblk,
  SetCarry_Spikeblk,
  SetCarry_Spikeblk,
  SetCarry_Spikeblk,
  SetCarry_Spikeblk,
  SetCarry_Spikeblk,
  SetCarry_Spikeblk,
  SetCarry_Spikeblk,
  SetCarry_Spikeblk,
  SetCarry_Spikeblk,
  SetCarry_Spikeblk,
};

static uint8 BlockColl_Horiz_SpikeBlock(CollInfo *ci) {
  kBlockColl_SpikeBlock[BTS[cur_block_index]]();
  return BlockColl_Horiz_SolidShootGrappleBlock(ci);
}

static uint8 BlockColl_Vert_SpikeBlock(CollInfo *ci) {  // 0x94905D
  kBlockColl_SpikeBlock[BTS[cur_block_index]]();
  return BlockColl_Vert_SolidShootGrappleBlock(ci);
}

static uint8 BlockColl_Horiz_SpecialAir(CollInfo *ci) {  // 0x94906F
  uint8 v0 = BTS[cur_block_index];
  if ((v0 & 0x80) != 0) {
    const uint16 *v2 = (const uint16 *)RomPtr_94(kSpecialAirPlmLists[area_index]);
    return SpawnPLM(v2[v0 & 0x7f]) & 1;
  }
  return SpawnPLM(kPlmHeaderDefPtrs[v0]) & 1;
}

static uint8 BlockColl_Vert_SpecialAir(CollInfo *ci) {  // 0x94909D
  uint8 v0 = BTS[cur_block_index];
  if ((v0 & 0x80) != 0) {
    const uint16 *v2 = (const uint16 *)RomPtr_94(kSpecialAirPlmLists[area_index]);
    return SpawnPLM(v2[v0 & 0x7f]) & 1;
  }
  return SpawnPLM(kPlmHeaderDefPtrs[v0]) & 1;
}

static uint8 BlockColl_Horiz_SpecialBlock(CollInfo *ci) {  // 0x9490CB
  uint8 v0 = BTS[cur_block_index];
  uint8 v1;
  if ((v0 & 0x80) != 0) {
    const uint16 *v2 = (const uint16 *)RomPtr_94(kSpecialBlockPlmLists[area_index]);
    v1 = SpawnPLM(v2[v0 & 0x7f]) & 1;
  } else {
    v1 = SpawnPLM(kPlmHeaderDefPtrs[v0]) & 1;
  }
  return v1 ? BlockColl_Horiz_SolidShootGrappleBlock(ci) : v1;
}

static uint8 BlockColl_Vert_SpecialBlock(CollInfo *ci) {  // 0x949102
  uint8 v0 = BTS[cur_block_index];
  uint8 v1;
  if ((v0 & 0x80) != 0) {
    const uint16 *v2 = (const uint16 *)RomPtr_94(kSpecialBlockPlmLists[area_index]);
    v1 = SpawnPLM(v2[v0 & 0x7f]) & 1;
  } else {
    v1 = SpawnPLM(kPlmHeaderDefPtrs[v0]) & 1;
  }
  return v1 ? BlockColl_Vert_SolidShootGrappleBlock(ci) : v1;
}

static uint8 BlockColl_Horiz_BombableAir(CollInfo *ci) {  // 0x9492F9
  uint8 v0 = BTS[cur_block_index];
  if ((v0 & 0x80) == 0)
    SpawnPLM(kBombBlockPlmList[v0]);
  return 0;
}

static uint8 BlockColl_Vert_BombableAir(CollInfo *ci) {  // 0x949313
  uint8 v0 = BTS[cur_block_index];
  if ((v0 & 0x80) == 0)
    SpawnPLM(kBombBlockPlmList[v0]);
  return 0;
}

static uint8 BlockColl_Horiz_BombBlock(CollInfo *ci) {  // 0x94932D
  uint8 v0 = BTS[cur_block_index];
  if ((v0 & 0x80) != 0)
    return BlockColl_Horiz_SolidShootGrappleBlock(ci);
  uint8 v1 = SpawnPLM(kBombBlockPlmList[v0]) & 1;
  return v1 ? BlockColl_Horiz_SolidShootGrappleBlock(ci) : v1;
}

static uint8 BlockColl_Vert_BombBlock(CollInfo *ci) {  // 0x94934C
  uint8 v0 = BTS[cur_block_index];
  if ((v0 & 0x80) != 0)
    return BlockColl_Vert_SolidShootGrappleBlock(ci);
  uint8 v1 = SpawnPLM(kBombBlockPlmList[v0]) & 1;
  return v1 ? BlockColl_Vert_SolidShootGrappleBlock(ci) : v1;
}

uint8 BlockColl_Horiz_Door(CollInfo *ci) {  // 0x94938B
  door_transition_function = FUNC16(DoorTransitionFunction_HandleElevator);
  uint8 door_bts = BTS[cur_block_index];
  uint16 v0 = *(uint16 *)RomPtr_8F(door_list_pointer + 2 * (door_bts & 0x7F));
  if ((get_DoorDef(v0)->room_definition_ptr & 0x8000) == 0) {
    if (samus_pose < kGameState_9_HitDoorBlock)
      elevator_flags = 1;
    return BlockColl_Horiz_SolidShootGrappleBlock(ci);
  }
  door_def_ptr = v0;
  game_state = kGameState_9_HitDoorBlock;
  return 0;
}

uint8 BlockColl_Vert_Door(CollInfo *ci) {  // 0x9493CE
  door_transition_function = FUNC16(DoorTransitionFunction_HandleElevator);
  uint8 door_bts = BTS[cur_block_index];
  uint16 v0 = *(uint16 *)RomPtr_8F(door_list_pointer + 2 * (door_bts & 0x7F));
  if ((get_DoorDef(v0)->room_definition_ptr & 0x8000) == 0) {
    if (samus_pose < kPose_09_MoveR_NoAim)
      elevator_flags = 1;
    return BlockColl_Vert_SolidShootGrappleBlock(ci);
  }
  door_def_ptr = v0;
  game_state = kPose_09_MoveR_NoAim;
  return 0;
}

uint8 BlockReact_HorizExt(CollInfo *ci) {  // 0x949411
  if (BTS[cur_block_index]) {
    cur_block_index += (int8)BTS[cur_block_index];
    return 0xff;
  }
  return 0;
}

uint8 BlockReact_VertExt(CollInfo *ci) {  // 0x949447
  if (BTS[cur_block_index]) {
    cur_block_index += (int8)BTS[cur_block_index] * room_width_in_blocks;
    return 0xff;
  }
  return 0;
}

static uint16 Samus_GetYposSpan(void) {  // 0x949495
  uint16 r26 = (samus_y_pos - samus_y_radius) & 0xFFF0;
  return (uint16)(samus_y_radius + samus_y_pos - 1 - r26) >> 4;
}

static uint16 Samus_GetXposSpan(void) {  // 0x9494B5
  uint16 r26 = (samus_x_pos - samus_x_radius) & 0xFFF0;
  return (uint16)(samus_x_radius + samus_x_pos - 1 - r26) >> 4;
}

static Func_CollInfo_U8 *const kBlockColl_Horiz_CheckColl[16] = {  // 0x949515
  ClearCarry_4,
  BlockColl_Horiz_Slope,
  ClearCarry_5,
  BlockColl_Horiz_SpecialAir,
  ClearCarry_4,
  BlockReact_HorizExt,
  ClearCarry_4,
  BlockColl_Horiz_BombableAir,
  BlockColl_Horiz_SolidShootGrappleBlock,
  BlockColl_Horiz_Door,
  BlockColl_Horiz_SpikeBlock,
  BlockColl_Horiz_SpecialBlock,
  BlockColl_Horiz_SolidShootGrappleBlock,
  BlockReact_VertExt,
  BlockColl_Horiz_SolidShootGrappleBlock,
  BlockColl_Horiz_BombBlock,
};

static Func_CollInfo_U8 *const kBlockColl_Vert_CheckColl[16] = {
  ClearCarry_4,
  BlockColl_Vert_Slope,
  BlockColl_Vert_SpikeAir,
  BlockColl_Vert_SpecialAir,
  ClearCarry_4,
  BlockReact_HorizExt,
  ClearCarry_4,
  BlockColl_Vert_BombableAir,
  BlockColl_Vert_SolidShootGrappleBlock,
  BlockColl_Vert_Door,
  BlockColl_Vert_SpikeBlock,
  BlockColl_Vert_SpecialBlock,
  BlockColl_Vert_SolidShootGrappleBlock,
  BlockReact_VertExt,
  BlockColl_Vert_SolidShootGrappleBlock,
  BlockColl_Vert_BombBlock,
};

static uint8 BlockColl_Horiz_CheckColl(CollInfo *ci, uint16 k) {
  uint8 rv;
  cur_block_index = k >> 1;
  cur_coll_amt32 = &ci->ci_r18_r20;
  do {
    rv = kBlockColl_Horiz_CheckColl[BlockTypeIndexFromTile(level_data[cur_block_index])](ci);
  } while (rv & 0x80);
  cur_coll_amt32 = NULL;
  return rv;
}

static uint8 BlockColl_Vert_CheckColl(CollInfo *ci, uint16 k) {  // 0x94952C
  uint8 rv;
  cur_block_index = k >> 1;
  cur_coll_amt32 = &ci->ci_r18_r20;
  do {
    rv = kBlockColl_Vert_CheckColl[BlockTypeIndexFromTile(level_data[cur_block_index])](ci);
  } while (rv & 0x80);
  cur_coll_amt32 = NULL;
  return rv;
}

Pair_Bool_Amt BlockColl_Handle_Horiz(int32 amt) {  // 0x949543
  uint16 r28 = Samus_GetYposSpan();
  uint16 prod = Mult8x8((uint16)(samus_y_pos - samus_y_radius) >> 4, room_width_in_blocks);
  uint16 v0 = (amt + __PAIR32__(samus_x_pos, samus_x_subpos)) >> 16;
  uint16 r24 = v0;
  uint16 v1 = (amt >= 0) ? samus_x_radius + v0 - 1 : v0 - samus_x_radius;
  uint16 v2 = 2 * (prod + (v1 >> 4));
  CollInfo ci = { .ci_r18_r20 = amt, .ci_r24 = r24, .ci_r26 = r28, .ci_r28 = r28, .ci_r32 = v1 };
  while (!(BlockColl_Horiz_CheckColl(&ci, v2) & 1)) {
    v2 += room_width_in_blocks * 2;
    if ((--ci.ci_r26 & 0x8000) != 0)
      return (Pair_Bool_Amt){false, ci.ci_r18_r20};
  }
  return (Pair_Bool_Amt){true, ci.ci_r18_r20};
}

Pair_Bool_Amt BlockColl_Handle_Vert_LeftToRight(int32 amt) {  // 0x94959E
  uint16 r28 = Samus_GetXposSpan();
  uint16 v0 = (amt + __PAIR32__(samus_y_pos, samus_y_subpos)) >> 16;
  uint16 r24 = v0;
  uint16 v1 = (amt >= 0) ? samus_y_radius + v0 - 1 : v0 - samus_y_radius;
  uint16 prod = Mult8x8(v1 >> 4, room_width_in_blocks);
  uint16 v2 = (uint16)(samus_x_pos - samus_x_radius) >> 4;
  cur_block_index = prod + v2;
  CollInfo ci = { .ci_r18_r20 = amt, .ci_r24 = r24, .ci_r26 = r28, .ci_r28 = r28, .ci_r32 = v1 };
  for (int i = 2 * cur_block_index; !(BlockColl_Vert_CheckColl(&ci, i) & 1); i += 2) {
    if ((--ci.ci_r26 & 0x8000) != 0)
      return (Pair_Bool_Amt){false, ci.ci_r18_r20};
  }
  return (Pair_Bool_Amt){true, ci.ci_r18_r20};
}

Pair_Bool_Amt BlockColl_Handle_Vert_RightToLeft(int32 amt) {  // 0x9495F5
  uint16 r28 = Samus_GetXposSpan();
  uint16 r26 = 0;
  uint16 v0 = (amt + __PAIR32__(samus_y_pos, samus_y_subpos)) >> 16;
  uint16 r24 = v0;
  uint16 v1 = (amt >= 0) ? samus_y_radius + v0 - 1 : v0 - samus_y_radius;
  uint16 prod = Mult8x8(v1 >> 4, room_width_in_blocks);
  uint16 v2 = (uint16)(samus_x_radius + samus_x_pos - 1) >> 4;
  cur_block_index = prod + v2;
  CollInfo ci = { .ci_r18_r20 = amt, .ci_r24 = r24, .ci_r26 = r26, .ci_r28 = r28, .ci_r32 = v1 };
  for (int i = 2 * cur_block_index; !(BlockColl_Vert_CheckColl(&ci, i) & 1); i -= 2) {
    if (ci.ci_r28 < ++ci.ci_r26)
      return (Pair_Bool_Amt){false, ci.ci_r18_r20};
  }
  return (Pair_Bool_Amt){true, ci.ci_r18_r20};
}

// Returns the new value of amt and not the carry flag,
// it can be inferred from samus_collision_flag.
int32 WallJumpBlockCollDetect(int32 amt) {  // 0x94967F
  samus_collision_direction |= 0xF;
  flag_samus_in_quicksand = 0;
  Pair_Bool_Amt pair = BlockColl_Handle_Horiz(amt);
  samus_collision_flag = pair.flag;
  return pair.amt >= 0 ? pair.amt : -pair.amt;
}

// Returns the new value of amt and not the carry flag,
// it can be inferred from samus_collision_flag.
static int32 CollDetectDueToPoseChange_SingleBlock(int32 amt) {  // 0x9496E3
  samus_collision_direction |= 0xF;
  flag_samus_in_quicksand = 0;
  Pair_Bool_Amt pair = !(nmi_frame_counter_word & 1)
      ? BlockColl_Handle_Vert_LeftToRight(amt)
      : BlockColl_Handle_Vert_RightToLeft(amt);
  samus_collision_flag = pair.flag;
  return pair.amt >= 0 ? pair.amt : -pair.amt;
}

// Returns the new value of amt and not the carry flag,
// it can be inferred from samus_collision_flag.
int32 Samus_CollDetectChangedPose(int32 amt) {  // 0x9496AB
  if ((abs16(amt >> 16) & 0xFFF8) == 0)
    return CollDetectDueToPoseChange_SingleBlock(amt);
  int32 amt_backup = amt;
  amt = CollDetectDueToPoseChange_SingleBlock(INT16_SHL16((amt >> 16) & 0xFFF0 | 8));
  if (samus_collision_flag)
    return amt;
  return CollDetectDueToPoseChange_SingleBlock(amt_backup);
}
