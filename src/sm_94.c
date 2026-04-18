// Block properties, some cutscene graphics
#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "block_reaction.h"

#define fnkPlmHeaderDefPtrs 0x949139
#define off_9492D9 ((uint16*)RomFixedPtr(0x9492d9))
#define off_9492E9 ((uint16*)RomFixedPtr(0x9492e9))
#define kPlmHeaderDefPtrs ((uint16*)RomFixedPtr(0x949139))
#define off_94936B ((uint16*)RomFixedPtr(0x94936b))
#define kBlockShotBombedReactionShootablePlm ((uint16 *)RomPtr_94(0x9ea6))
#define kAlignYPos_Tab0 ((uint8*)RomFixedPtr(0x948b2b))
#define kTab948E54 ((uint8*)RomFixedPtr(0x948e54))

int32 *cur_coll_amt32;

static uint16 Samus_GetXposSpan(void);
static uint16 Samus_GetYposSpan(void);
static uint16 BlockFunc_9C73(uint16 k);
static void BlockFunc_A06A(uint16 k);
static void BlockFunc_A0F4(uint16 a, uint16 r22, uint16 r24, uint16 r26, uint16 r28);
static void BlockFunc_A11A(uint16 a, uint16 r24, uint16 r26, uint16 r28);
void CalculateBlockAt(uint16 r26, uint16 r28, uint16 r30, uint16 r32);
static void BlockBombedReact(CollInfo *ci, uint16 v0);
static void BlockColl_BombExplosion(CollInfo *ci, uint16 k);
static uint8 BlockShotReactHoriz_Slope_Square(CollInfo *ci, uint16 a, uint16 k);
static uint8 BlockShotReactVert_Slope_Square(CollInfo *ci, uint16 a, uint16 k);
static uint8 BlockShotReactHoriz_Slope_NonSquare(CollInfo *ci);
static uint8 BlockShotReactVert_Slope_NonSquare(CollInfo *ci);
//////////////////////////////////////////////////////////////
// BANK 94
//////////////////////////////////////////////////////////////

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

const int16 kSinCosTable8bit_Sext[320] = {  // 0x94A957
  -256, -255, -255, -255, -254, -254, -253, -252, -251, -249, -248, -246, -244, -243, -241, -238,
  -236, -234, -231, -228, -225, -222, -219, -216, -212, -209, -205, -201, -197, -193, -189, -185,
  -181, -176, -171, -167, -162, -157, -152, -147, -142, -136, -131, -126, -120, -115, -109, -103,
   -97,  -92,  -86,  -80,  -74,  -68,  -62,  -56,  -49,  -43,  -37,  -31,  -25,  -18,  -12,   -6,
     0,    6,   12,   18,   25,   31,   37,   43,   49,   56,   62,   68,   74,   80,   86,   92,
    97,  103,  109,  115,  120,  126,  131,  136,  142,  147,  152,  157,  162,  167,  171,  176,
   181,  185,  189,  193,  197,  201,  205,  209,  212,  216,  219,  222,  225,  228,  231,  234,
   236,  238,  241,  243,  244,  246,  248,  249,  251,  252,  253,  254,  254,  255,  255,  255,
   256,  255,  255,  255,  254,  254,  253,  252,  251,  249,  248,  246,  244,  243,  241,  238,
   236,  234,  231,  228,  225,  222,  219,  216,  212,  209,  205,  201,  197,  193,  189,  185,
   181,  176,  171,  167,  162,  157,  152,  147,  142,  136,  131,  126,  120,  115,  109,  103,
    97,   92,   86,   80,   74,   68,   62,   56,   49,   43,   37,   31,   25,   18,   12,    6,
     0,   -6,  -12,  -18,  -25,  -31,  -37,  -43,  -49,  -56,  -62,  -68,  -74,  -80,  -86,  -92,
   -97, -103, -109, -115, -120, -126, -131, -136, -142, -147, -152, -157, -162, -167, -171, -176,
  -181, -185, -189, -193, -197, -201, -205, -209, -212, -216, -219, -222, -225, -228, -231, -234,
  -236, -238, -241, -243, -244, -246, -248, -249, -251, -252, -253, -254, -254, -255, -255, -255,
  -256, -255, -255, -255, -254, -254, -253, -252, -251, -249, -248, -246, -244, -243, -241, -238,
  -236, -234, -231, -228, -225, -222, -219, -216, -212, -209, -205, -201, -197, -193, -189, -185,
  -181, -176, -171, -167, -162, -157, -152, -147, -142, -136, -131, -126, -120, -115, -109, -103,
   -97,  -92,  -86,  -80,  -74,  -68,  -62,  -56,  -49,  -43,  -37,  -31,  -25,  -18,  -12,   -6,
};

static uint8 BlockColl_Horiz_Slope_NonSquare(CollInfo *ci) {  // 0x9484D6
  if ((current_slope_bts & 0x80) != 0 || __PAIR32__(samus_y_speed, samus_y_subspeed))
    return 0;
  uint16 v1 = 4 * (current_slope_bts & 0x1F);
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
    if ((samus_x_pos >> 4) != SnesModulus(cur_block_index, room_width_in_blocks))
      return 0;
    if (BTS[k] & 0x80)
      return 0;
    uint16 v10 = (BTS[k] & 0x40) != 0 ? samus_x_pos ^ 0xF : samus_x_pos;
    uint16 v11 = 16 * (BTS[k] & 0x1F) + (v10 & 0xF);
    int16 v12 = (kAlignYPos_Tab0[v11] & 0x1F) - ((samus_y_radius + ci->ci_r24 - 1) & 0xF) - 1;
    if (v12 <= 0) {
      int16 v13 = (ci->ci_r18_r20 >> 16) + v12;
      if (v13 < 0)
        v13 = 0;
      ci->ci_r18_r20 = v13 << 16;
      return 1;
    } else {
      return 0;
    }
  } else {
    if ((samus_x_pos >> 4) != SnesModulus(cur_block_index, room_width_in_blocks))
      return 0;
    if (!(BTS[k] & 0x80))
      return 0;
    uint16 v4 = (BTS[k] & 0x40) != 0 ? (samus_x_pos ^ 0xF) : (samus_x_pos);
    uint16 v5 = 16 * (BTS[k] & 0x1F) + (v4 & 0xF);
    int16 v6 = (kAlignYPos_Tab0[v5] & 0x1F) - ((ci->ci_r24 - samus_y_radius) & 0xF ^ 0xF) - 1;
    if (v6 <= 0) {
      int16 v7 = (ci->ci_r18_r20 >> 16) + v6;
      if (v7 < 0)
        v7 = 0;
      ci->ci_r18_r20 = v7 << 16;
      return 1;
    } else {
      return 0;
    }
  }
}

static uint8 BlockColl_Horiz_Slope_Square(CollInfo *ci, uint16 a, uint16 k) {  // 0x948D2B
  uint16 temp_collision_DD4 = 4 * a;
  uint16 temp_collision_DD6 = BTS[k] >> 6;
  uint16 v2 = 4 * a + (temp_collision_DD6 ^ ((ci->ci_r32 & 8) >> 3));
  if (!ci->ci_r26) {
    if (((samus_y_radius + samus_y_pos - 1) & 8) == 0) {
      if (!kTab948E54[v2])
        return 0;
      goto LABEL_10;
    }
    goto LABEL_7;
  }
  if (ci->ci_r26 != ci->ci_r28 || ((samus_y_pos - samus_y_radius) & 8) == 0) {
LABEL_7:
    if (kTab948E54[v2])
      goto LABEL_10;
  }
  if (!kTab948E54[v2 ^ 2])
    return 0;
LABEL_10:
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
      if (!kTab948E54[v2])
        return 0;
      goto LABEL_10;
    }
    goto LABEL_7;
  }
  if (ci->ci_r26 != ci->ci_r28 || ((samus_x_pos - samus_x_radius) & 8) == 0) {
LABEL_7:
    if (kTab948E54[v2])
      goto LABEL_10;
  }
  if (!kTab948E54[v2 ^ 1])
    return 0;
LABEL_10:
  if (ci->ci_r18_r20 < 0) {
    int16 v5 = samus_y_radius + (ci->ci_r32 | 7) + 1 - samus_y_pos;
    if (v5 >= 0)
      v5 = 0;
    ci->ci_r18_r20 = v5 << 16;
    samus_y_subpos = 0;
    return 1;
  } else {
    int16 v4 = (ci->ci_r32 & 0xFFF8) - samus_y_radius - samus_y_pos;
    if (v4 < 0)
      v4 = 0;
    ci->ci_r18_r20 = v4 << 16;
    samus_y_subpos = -1;
    samus_pos_adjusted_by_slope_flag = 1;
    return 1;
  }
}

static uint8 ClearCarry_1(CollInfo *ci) {  // 0x948E7D
  return 0;
}

static uint8 ClearCarry_2(CollInfo *ci) {  // 0x948E7F
  return 0;
}

static uint8 ClearCarry_3(CollInfo *ci) {  // 0x948E81
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

static uint8 SetCarry(CollInfo *ci) {
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
  } else {
    int16 v0 = (ci->ci_r32 & 0xFFF0) - samus_x_radius - samus_x_pos;
    if (v0 < 0)
      v0 = 0;
    ci->ci_r18_r20 = v0 << 16;
    samus_x_subpos = -1;
    return 1;
  }
}

static uint8 BlockColl_Vert_SolidShootGrappleBlock(CollInfo *ci) {  // 0x948F82
  if (ci->ci_r18_r20 < 0) {
    int16 v2 = samus_y_radius + (ci->ci_r32 | 0xF) + 1 - samus_y_pos;
    if (v2 >= 0)
      v2 = 0;
    ci->ci_r18_r20 = v2 << 16;
    samus_y_subpos = 0;
    return 1;
  } else {
    int16 v0 = (ci->ci_r32 & 0xFFF0) - samus_y_radius - samus_y_pos;
    if (v0 < 0)
      v0 = 0;
    ci->ci_r18_r20 = v0 << 16;
    samus_y_subpos = -1;
    return 1;
  }
}

static uint8 BlockColl_Horiz_Slope(CollInfo *ci) {  // 0x948FBB
  uint16 v0 = BTS[cur_block_index] & 0x1F;
  if (v0 < 5)
    return BlockColl_Horiz_Slope_Square(ci, v0, cur_block_index);
  current_slope_bts = BTS[cur_block_index];
  return BlockColl_Horiz_Slope_NonSquare(ci);
}

static uint8 BlockColl_Vert_Slope(CollInfo *ci) {  // 0x948FDA
  uint16 v0 = BTS[cur_block_index] & 0x1F;
  if (v0 < 5)
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
    const uint16 *v2 = (const uint16 *)RomPtr_94(off_9492D9[area_index]);
    return SpawnPLM(v2[v0 & 0x7f]) & 1;
  } else {
    SpawnPLM(kPlmHeaderDefPtrs[v0]);
    return 0;
  }
}

static uint8 BlockColl_Vert_SpecialAir(CollInfo *ci) {  // 0x94909D
  uint8 v0 = BTS[cur_block_index];
  if ((v0 & 0x80) != 0) {
    const uint16 *v2 = (const uint16 *)RomPtr_94(off_9492D9[area_index]);
    return SpawnPLM(v2[v0 & 0x7f]) & 1;
  } else {
    SpawnPLM(kPlmHeaderDefPtrs[v0]);
    return 0;
  }
}

static uint8 BlockColl_Horiz_SpecialBlock(CollInfo *ci) {  // 0x9490CB
  uint8 v0 = BTS[cur_block_index];
  if ((v0 & 0x80) != 0) {
    const uint16 *v2 = (const uint16 *)RomPtr_94(off_9492E9[area_index]);
    uint8 v4 = SpawnPLM(v2[v0 & 0x7f]) & 1;
    if (v4)
      return BlockColl_Horiz_SolidShootGrappleBlock(ci);
    return v4;
  } else {
    uint8 v1 = SpawnPLM(kPlmHeaderDefPtrs[v0]) & 1;
    if (v1)
      return BlockColl_Horiz_SolidShootGrappleBlock(ci);
    return v1;
  }
}

static uint8 BlockColl_Vert_SpecialBlock(CollInfo *ci) {  // 0x949102
  uint8 v0 = BTS[cur_block_index];
  if ((v0 & 0x80) != 0) {
    const uint16 *v3 = (const uint16 *)RomPtr_94(off_9492E9[area_index]);
    uint8 v4 = SpawnPLM(v3[v0 & 0x7F]) & 1;
    if (v4)
      return BlockColl_Vert_SolidShootGrappleBlock(ci);
    return v4;
  } else {
    uint8 v1 = SpawnPLM(kPlmHeaderDefPtrs[v0]) & 1;
    if (v1)
      return BlockColl_Vert_SolidShootGrappleBlock(ci);
    return v1;
  }
}


static uint8 BlockColl_Horiz_BombableAir(CollInfo *ci) {  // 0x9492F9
  uint8 v0 = BTS[cur_block_index];
  if ((v0 & 0x80) == 0)
    SpawnPLM(off_94936B[v0]);
  return 0;
}

static uint8 BlockColl_Vert_BombableAir(CollInfo *ci) {  // 0x949313
  uint8 v0 = BTS[cur_block_index];
  if ((v0 & 0x80) == 0)
    SpawnPLM(off_94936B[v0]);
  return 0;
}

static uint8 BlockColl_Horiz_BombBlock(CollInfo *ci) {  // 0x94932D
  uint8 v0 = BTS[cur_block_index];
  if ((v0 & 0x80) != 0)
    return BlockColl_Horiz_SolidShootGrappleBlock(ci);
  uint8 v1 = SpawnPLM(off_94936B[v0]) & 1;
  if (v1)
    return BlockColl_Horiz_SolidShootGrappleBlock(ci);
  else
    return v1;
}

static uint8 BlockColl_Vert_BombBlock(CollInfo *ci) {  // 0x94934C
  uint8 v0 = BTS[cur_block_index];
  if ((v0 & 0x80) != 0)
    return BlockColl_Vert_SolidShootGrappleBlock(ci);
  uint8 v1 = SpawnPLM(off_94936B[v0]) & 1;
  if (v1)
    return BlockColl_Vert_SolidShootGrappleBlock(ci);
  else
    return v1;
}

uint8 BlockColl_Horiz_Door(CollInfo *ci) {  // 0x94938B
  door_transition_function = FUNC16(DoorTransitionFunction_HandleElevator);
  uint8 door_bts = BTS[cur_block_index];
  uint16 v0 = *(uint16 *)RomPtr_8F(door_list_pointer + 2 * (door_bts & 0x7F));
  if ((get_DoorDef(v0)->room_definition_ptr & 0x8000) == 0) {
    if (samus_pose < kGameState_9_HitDoorBlock)
      elevator_flags = 1;
    return BlockColl_Horiz_SolidShootGrappleBlock(ci);
  } else {
    door_def_ptr = v0;
    game_state = kGameState_9_HitDoorBlock;
    return 0;
  }
}

uint8 BlockColl_Vert_Door(CollInfo *ci) {  // 0x9493CE
  door_transition_function = FUNC16(DoorTransitionFunction_HandleElevator);
  uint8 door_bts = BTS[cur_block_index];
  uint16 v0 = *(uint16 *)RomPtr_8F(door_list_pointer + 2 * (door_bts & 0x7F));
  if ((get_DoorDef(v0)->room_definition_ptr & 0x8000) == 0) {
    if (samus_pose < kPose_09_MoveR_NoAim)
      elevator_flags = 1;
    return BlockColl_Vert_SolidShootGrappleBlock(ci);
  } else {
    door_def_ptr = v0;
    game_state = kPose_09_MoveR_NoAim;
    return 0;
  }
}

uint8 BlockReact_HorizExt(CollInfo *ci) {  // 0x949411
  if (BTS[cur_block_index]) {
    cur_block_index += (int8)BTS[cur_block_index];
    return 0xff; // special
  }
  return 0;
}

uint8 BlockReact_VertExt(CollInfo *ci) {  // 0x949447
  if (BTS[cur_block_index]) {
    cur_block_index += (int8)BTS[cur_block_index] * room_width_in_blocks;
    return 0xff; // special
  }
  return 0;
}

static uint16 Samus_GetYposSpan(void) {  // 0x949495
  uint16 r26 = (samus_y_pos - samus_y_radius) & 0xFFF0;
  r26 = (uint16)(samus_y_radius + samus_y_pos - 1 - r26) >> 4;
  return r26;
}

static uint16 Samus_GetXposSpan(void) {  // 0x9494B5
  uint16 r26 = (samus_x_pos - samus_x_radius) & 0xFFF0;
  r26 = (uint16)(samus_x_radius + samus_x_pos - 1 - r26) >> 4;
  return r26;
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
  cur_coll_amt32 = &ci->ci_r18_r20;  // kludge needed for SpawnPLM
  do {
    rv = kBlockColl_Horiz_CheckColl[(level_data[cur_block_index] & 0xF000) >> 12](ci);
  } while (rv & 0x80);
  cur_coll_amt32 = NULL;
  return rv;
}

static uint8 BlockColl_Vert_CheckColl(CollInfo *ci, uint16 k) {  // 0x94952C
  uint8 rv;
  cur_block_index = k >> 1;
  cur_coll_amt32 = &ci->ci_r18_r20;
  do {
    rv = kBlockColl_Vert_CheckColl[(level_data[cur_block_index] & 0xF000) >> 12](ci);
  } while (rv & 0x80);
  cur_coll_amt32 = NULL;
  return rv;
}

Pair_Bool_Amt BlockColl_Handle_Horiz(int32 amt) {  // 0x949543
  uint16 r28 = Samus_GetYposSpan();
  uint16 prod = Mult8x8((uint16)(samus_y_pos - samus_y_radius) >> 4, room_width_in_blocks);
  uint16 v1;
  //r22 = r20 + samus_x_subpos;
  uint16 v0 = (amt + __PAIR32__(samus_x_pos, samus_x_subpos)) >> 16;
  uint16 r24 = v0;
  if (amt >= 0)
    v1 = samus_x_radius + v0 - 1;
  else
    v1 = v0 - samus_x_radius;
  uint16 v2 = 2 * (prod + (v1 >> 4));
  CollInfo ci = { .ci_r18_r20 = amt, .ci_r24 = r24, .ci_r26 = r28, .ci_r28 = r28, .ci_r32 = v1 };
  while (!(BlockColl_Horiz_CheckColl(&ci, v2) & 1)) {
    v2 += room_width_in_blocks * 2;
    if ((--ci.ci_r26 & 0x8000) != 0)
      return (Pair_Bool_Amt) {false, ci.ci_r18_r20};
  }
  return (Pair_Bool_Amt) { true, ci.ci_r18_r20 };
}

Pair_Bool_Amt BlockColl_Handle_Vert_LeftToRight(int32 amt) {  // 0x94959E
  uint16 r28 = Samus_GetXposSpan();
  //r22 = r20 + samus_y_subpos;
  uint16 v0 = (amt + __PAIR32__(samus_y_pos, samus_y_subpos)) >> 16, v1;
  uint16 r24 = v0;
  if (amt >= 0)
    v1 = samus_y_radius + v0 - 1;
  else
    v1 = v0 - samus_y_radius;
  uint16 prod = Mult8x8(v1 >> 4, room_width_in_blocks);
  uint16 v2 = (uint16)(samus_x_pos - samus_x_radius) >> 4;
  cur_block_index = prod + v2;
  CollInfo ci = { .ci_r18_r20 = amt, .ci_r24 = r24, .ci_r26 = r28, .ci_r28 = r28, .ci_r32 = v1 };
  for (int i = 2 * cur_block_index; !(BlockColl_Vert_CheckColl(&ci, i) & 1); i += 2) {
    if ((--ci.ci_r26 & 0x8000) != 0)
      return (Pair_Bool_Amt) { false, ci.ci_r18_r20 };
  }
  return (Pair_Bool_Amt) { true, ci.ci_r18_r20 };
}

Pair_Bool_Amt BlockColl_Handle_Vert_RightToLeft(int32 amt) {  // 0x9495F5
  uint16 r28 = Samus_GetXposSpan();
  uint16 r26 = 0;
  //r22 = r20 + samus_y_subpos;
  uint16 v0 = (amt + __PAIR32__(samus_y_pos, samus_y_subpos)) >> 16, v1;
  uint16 r24 = v0;
  if (amt >= 0)
    v1 = samus_y_radius + v0 - 1;
  else
    v1 = v0 - samus_y_radius;
  uint16 prod = Mult8x8(v1 >> 4, room_width_in_blocks);
  uint16 v2 = (uint16)(samus_x_radius + samus_x_pos - 1) >> 4;
  cur_block_index = prod + v2;
  CollInfo ci = { .ci_r18_r20 = amt, .ci_r24 = r24, .ci_r26 = r26, .ci_r28 = r28, .ci_r32 = v1 };
  for (int i = 2 * cur_block_index; !(BlockColl_Vert_CheckColl(&ci, i) & 1); i -= 2) {
    if (ci.ci_r28 < ++ci.ci_r26)
      return (Pair_Bool_Amt) { false, ci.ci_r18_r20 };
  }
  return (Pair_Bool_Amt) { true, ci.ci_r18_r20 };
}

// Returns the new value of amt and not the carry flag,
// it can be inferred from samus_collision_flag
int32 WallJumpBlockCollDetect(int32 amt) {  // 0x94967F
  samus_collision_direction |= 0xF;
  flag_samus_in_quicksand = 0;
  Pair_Bool_Amt pair = BlockColl_Handle_Horiz(amt);
  samus_collision_flag = pair.flag;
  return pair.amt >= 0 ? pair.amt : -pair.amt;
}

// Returns the new value of amt and not the carry flag,
// it can be inferred from samus_collision_flag
static int32 CollDetectDueToPoseChange_SingleBlock(int32 amt) {  // 0x9496E3
  samus_collision_direction |= 0xF;
  flag_samus_in_quicksand = 0;
  Pair_Bool_Amt pair = !(nmi_frame_counter_word & 1) ?
      BlockColl_Handle_Vert_LeftToRight(amt) : BlockColl_Handle_Vert_RightToLeft(amt);
  samus_collision_flag = pair.flag;
  return pair.amt >= 0 ? pair.amt : -pair.amt;
}

// Returns the new value of amt and not the carry flag,
// it can be inferred from samus_collision_flag
int32 Samus_CollDetectChangedPose(int32 amt) {  // 0x9496AB
  if ((abs16(amt >> 16) & 0xFFF8) == 0)
    return CollDetectDueToPoseChange_SingleBlock(amt);
  int32 amt_backup = amt;
  amt = CollDetectDueToPoseChange_SingleBlock(INT16_SHL16((amt >> 16) & 0xFFF0 | 8));
  if (samus_collision_flag)
    return amt;
  return CollDetectDueToPoseChange_SingleBlock(amt_backup);
}

static uint8 BlockInsideReact_Slope(CollInfo *ci) {  // 0x9497BF
  return 0;
}

static uint8 BlockInsideReact_ShootableAir(CollInfo *ci) {  // 0x9497D0
  samus_x_speed_table_pointer = addr_kSamusSpeedTable_Normal_X + 12;
  return 0;
}

static void ClearCarry_8(void) {  // 0x9497D8
}

static void nullsub_165(void) {  // 0x9497D7
}

static void BlockInsideReact_SpikeAir_BTS2(void) {  // 0x949866
  if (!samus_contact_damage_index && !samus_invincibility_timer) {
    samus_invincibility_timer = 60;
    samus_knockback_timer = 10;
    samus_periodic_damage += 16;
    knockback_x_dir = ((*(uint16 *)&samus_pose_x_dir ^ 0xC) & 8) != 0;
  }
  samus_x_speed_table_pointer = addr_kSamusSpeedTable_Normal_X + 12;
}

static Func_V *const kBlockInsideReact_SpikeAir[16] = {  // 0x9498CC
  ClearCarry_8,
  ClearCarry_8,
  BlockInsideReact_SpikeAir_BTS2,
  nullsub_165,
  nullsub_165,
  nullsub_165,
  nullsub_165,
  nullsub_165,
  nullsub_165,
  nullsub_165,
  nullsub_165,
  nullsub_165,
  nullsub_165,
  nullsub_165,
  nullsub_165,
  nullsub_165,
};

static uint8 BlockInsideReact_SpikeAir(CollInfo *ci) {
  kBlockInsideReact_SpikeAir[BTS[cur_block_index]]();
  return 0;
}

static uint8 BlockInsideReact_Special_(CollInfo *ci) {  // 0x9498DC
  samus_x_speed_table_pointer = addr_kSamusSpeedTable_Normal_X + 12;
  return 0;
}

static void BlockInsideReact_SpecialAir_Default(void) {  // 0x9498E3
  samus_x_speed_table_pointer = addr_kSamusSpeedTable_Normal_X + 12;
}

static void BlockInsideReact_SpecialAir_8(void) {  // 0x9498EA
  if ((area_index != 3 || CheckBossBitForCurArea(1) & 1) && !samus_y_speed) {
    extra_samus_x_subdisplacement = 0;
    extra_samus_x_displacement = 2;
  }
  samus_x_speed_table_pointer = addr_kSamusSpeedTable_Normal_X + 12;
}

static void BlockInsideReact_SpecialAir_9(void) {  // 0x949910
  if ((area_index != 3 || CheckBossBitForCurArea(1) & 1) && !samus_y_speed) {
    extra_samus_x_subdisplacement = 0;
    extra_samus_x_displacement = -2;
  }
  samus_x_speed_table_pointer = addr_kSamusSpeedTable_Normal_X + 12;
}

static void BlockInsideReact_SpecialAir_10(void) {  // 0x949936
  extra_samus_x_subdisplacement = 0;
  extra_samus_x_displacement = 2;
  samus_x_speed_table_pointer = addr_kSamusSpeedTable_Normal_X + 12;
}

static void BlockInsideReact_SpecialAir_11(void) {  // 0x949946
  extra_samus_x_subdisplacement = 0;
  extra_samus_x_displacement = -2;
  samus_x_speed_table_pointer = addr_kSamusSpeedTable_Normal_X + 12;
}

static void BlockInsideReact_SpecialAir_70(void) {  // 0x949956
  if (inside_block_reaction_samus_point == 1)
    SpawnPLM(addr_kPlmHeader_B6FF);
}

static const uint16 g_off_949B06[8] = {  // 0x949B16
  0x9a06,
  0x9a26,
  0x9a46,
  0x9a66,
  0x9a86,
  0x9aa6,
  0x9ac6,
  0x9ae6,
};

static Func_V *const off_949966[80] = {
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_8,
  BlockInsideReact_SpecialAir_9,
  BlockInsideReact_SpecialAir_10,
  BlockInsideReact_SpecialAir_11,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_70,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
  BlockInsideReact_SpecialAir_Default,
};

static uint8 BlockInsideReact_SpecialAir(CollInfo *ci) {
  uint8 v0 = BTS[cur_block_index];
  if (v0 & 0x80) {
    const uint8 *v3 = RomPtr_94(g_off_949B06[area_index]);
    SpawnPLM(*(uint16 *)&v3[2 * (v0 & 0x7F)]);
  } else {
    off_949966[v0]();
  }
  return 0;
}

void BlockInsideDetection(void) {  // 0x949B60
  static Func_CollInfo_U8 *const kSamus_BlockInsideDetection[16] = {
    BlockInsideReact_ShootableAir,
    BlockInsideReact_Slope,
    BlockInsideReact_SpikeAir,
    BlockInsideReact_SpecialAir,
    BlockInsideReact_ShootableAir,
    BlockReact_HorizExt,
    BlockInsideReact_ShootableAir,
    BlockInsideReact_ShootableAir,
    BlockInsideReact_Special_,
    BlockInsideReact_Special_,
    BlockInsideReact_Special_,
    BlockInsideReact_Special_,
    BlockInsideReact_Special_,
    BlockReact_VertExt,
    BlockInsideReact_Special_,
    BlockInsideReact_Special_,
  };

  *(uint16 *)&samus_x_decel_mult = 0;
  extra_samus_x_subdisplacement = 0;
  extra_samus_x_displacement = 0;
  extra_samus_y_subdisplacement = 0;
  extra_samus_y_displacement = 0;
  inside_block_reaction_samus_point = 0;
  uint16 r26 = samus_x_pos;
  uint16 samus_bottom_boundary_position = samus_y_radius + samus_y_pos - 1;
  uint16 r28 = samus_y_radius + samus_y_pos - 1;
  CalculateBlockAt(r26, r28, 0, 0);
  CollInfo ci = { .ci_r24 = 0, .ci_r26 = r26, .ci_r28 = r28 };
  uint8 rv;
  do {
    rv = kSamus_BlockInsideDetection[(HIBYTE(level_data[cur_block_index]) & 0xF0) >> 4](&ci);
  } while (rv & 0x80);
  inside_block_reaction_samus_point = 1;
  if (((samus_bottom_boundary_position ^ samus_y_pos) & 0xFFF0) != 0) {
    r26 = samus_x_pos;
    r28 = samus_y_pos;
    CalculateBlockAt(r26, r28, 0, 0);
    CollInfo ci = { .ci_r24 = 0, .ci_r26 = r26, .ci_r28 = r28 };
    do {
      rv = kSamus_BlockInsideDetection[(HIBYTE(level_data[cur_block_index]) & 0xF0) >> 4](&ci);
    } while (rv & 0x80);
  }
  inside_block_reaction_samus_point = 2;
  if (((samus_bottom_boundary_position ^ (uint16)(samus_y_pos - samus_y_radius)) & 0xFFF0) != 0
      && ((samus_y_pos ^ (samus_bottom_boundary_position ^ (uint16)(samus_y_pos - samus_y_radius)) & 0xFFF0) & 0xFFF0) != 0) {
    r26 = samus_x_pos;
    r28 = samus_y_pos - samus_y_radius;
    CalculateBlockAt(r26, r28, 0, 0);
    CollInfo ci = { .ci_r24 = 0, .ci_r26 = r26, .ci_r28 = r28 };
    do {
      rv = kSamus_BlockInsideDetection[(HIBYTE(level_data[cur_block_index]) & 0xF0) >> 4](&ci);
    } while (rv & 0x80);
  }
}

void CalculateBlockAt(uint16 r26, uint16 r28, uint16 r30, uint16 r32) {  // 0x949C1D
  int16 v0;
  int16 v1;
  uint16 temp_collision_DD4;

  v0 = r30 + r26;
  if ((int16)(r30 + r26) >= 0
      && sign16(v0 - 4096)
      && (temp_collision_DD4 = (uint16)(v0 & 0xFFF0) >> 4,
          v1 = r32 + r28,
          (int16)(r32 + r28) >= 0)
      && sign16(v1 - 4096)) {
    uint16 RegWord = (uint16)(v1 & 0xFFF0) >> 4;
    RegWord = Mult8x8(RegWord, room_width_in_blocks);
    cur_block_index = temp_collision_DD4 + RegWord;
  } else {
    cur_block_index = -1;
  }
}

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

uint16 BlockFunc_9C73(uint16 v0) {
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

static uint8 sub_949D5D(CollInfo *ci) {  // 0x949D5D
  if ((BTS[cur_block_index] & 0x1F) < 5)
    return 1;
  else
    return BlockShotReactVert_Slope_NonSquare(ci);
}

static uint8 BlockBombedReact_Special(CollInfo *ci) {  // 0x949D71
  uint8 v0 = BTS[cur_block_index];
  if ((v0 & 0x80) != 0) {
    uint16 *kBlockBombedReact_Region_Plm = (uint16 *)RomPtr_94(0x9e44);
    const uint16 *v2 = (uint16*)RomPtr_94(kBlockBombedReact_Region_Plm[area_index]);
    SpawnPLM(v2[v0 & 0x7F]);
  } else {
    uint16 *kBlockBombedReact_Special_Plm = (uint16 *)RomPtr_94(0x9dA4);
    SpawnPLM(kBlockBombedReact_Special_Plm[v0]);
  }
  return 1;
}

uint8 BlockReact_ShootableAir(CollInfo *ci) {  // 0x949E55
  uint8 v0 = BTS[cur_block_index];
  if ((v0 & 0x80) == 0) {
    uint16 t = kBlockShotBombedReactionShootablePlm[v0];
    if (t == 0xb974)
      ci->ci_r38 = 0, ci->ci_r40 = 0xffff;
    SpawnPLM(t);
  }
  return 0;
}

uint8 BlockReact_Shootable(CollInfo *ci) {  // 0x949E73
  uint8 v0 = BTS[cur_block_index];
  if ((v0 & 0x80) != 0) {
    uint16 *kBlockShotBombGrappleReaction_ShootableBlock_RegionPlm = (uint16 *)RomPtr_94(0x9FC6);
    uint16 *v2 = (uint16*)RomPtr_94(kBlockShotBombGrappleReaction_ShootableBlock_RegionPlm[area_index]);
    SpawnPLM(v2[v0 & 0x7f]);
  } else {
    uint16 t = kBlockShotBombedReactionShootablePlm[v0];
    if (t == 0xb974)
      ci->ci_r38 = 0, ci->ci_r40 = 0xffff;
    SpawnPLM(t);
  }
  return 1;
}

uint8 BlockReact_BombableAir(CollInfo *ci) {  // 0x949FD6
  uint16 *kBlockShotBombGrappleReact_BombableBlockPlm = (uint16 *)RomPtr_94(0xA012);
  uint8 v0 = BTS[cur_block_index];
  if ((v0 & 0x80) == 0)
    SpawnPLM(kBlockShotBombGrappleReact_BombableBlockPlm[v0]);
  return 0;
}

uint8 BlockReact_BombableBlock(CollInfo *ci) {  // 0x949FF4
  uint16 *kBlockShotBombGrappleReact_BombableBlockPlm = (uint16 *)RomPtr_94(0xA012);
  uint8 v0 = BTS[cur_block_index];
  if ((v0 & 0x80) == 0)
    SpawnPLM(kBlockShotBombGrappleReact_BombableBlockPlm[v0]);
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
  while (kBlockBombedReact[(level_data[cur_block_index] & 0xF000) >> 12](ci) & 0x80) {}
}

void BlockFunc_A06A(uint16 k) {  // 0x94A06A
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

static uint8 SetCarry_1(void) {  // 0x94A15C
  return 1;
}

static uint8 BlockShotReactVert_Slope(CollInfo *ci) {  // 0x94A15E
  if ((BTS[cur_block_index] & 0x1F) < 5)
    return BlockShotReactHoriz_Slope_Square(ci, BTS[cur_block_index] & 0x1F, cur_block_index);
  else
    return BlockShotReactHoriz_Slope_NonSquare(ci);
}

static uint8 SetCarry_2(void) {  // 0x94A173
  return 1;
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

static uint16 BlockGetSomePos1(uint16 k) {  // 0x94A1F7
  int v1 = k >> 1;
  uint16 r38 = (projectile_y_pos[v1] - projectile_y_radius[v1]) & 0xFFF0;
  return (uint16)(projectile_y_radius[v1] + projectile_y_pos[v1] - 1 - r38) >> 4;
}

static uint16 BlockGetSomePos2(uint16 k) {  // 0x94A219
  int v1 = k >> 1;
  uint16 r38 = (projectile_x_pos[v1] - projectile_x_radius[v1]) & 0xFFF0;
  return (uint16)(projectile_x_radius[v1] + projectile_x_pos[v1] - 1 - r38) >> 4;
}

uint8 BlockCollNoWaveBeamHoriz(uint16 k) {  // 0x94A23B
  uint16 v7;

  int v1 = k >> 1;
  int32 amt = INT16_SHL8(projectile_bomb_x_speed[v1]);
  uint16 some_pos = BlockGetSomePos1(k);
  uint16 prod = Mult8x8((uint16)(projectile_y_pos[v1] - projectile_y_radius[v1]) >> 4, room_width_in_blocks);

  AddToHiLo(&projectile_x_pos[v1], &projectile_bomb_x_subpos[v1], amt);
  //r22 = projectile_bomb_x_subpos[v1];
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
  uint16 some_pos = BlockGetSomePos2(k);
  AddToHiLo(&projectile_y_pos[v1], &projectile_bomb_y_subpos[v1], amt);
  //r22 = projectile_bomb_y_subpos[v1];
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
  uint16 some_pos = BlockGetSomePos1(k);
  uint16 prod = Mult8x8((uint16)(projectile_y_pos[v1] - projectile_y_radius[v1]) >> 4, room_width_in_blocks);
  AddToHiLo(&projectile_x_pos[v1], &projectile_bomb_x_subpos[v1], amt);
  //r22 = projectile_bomb_x_subpos[v1];
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
  uint16 some_pos = BlockGetSomePos2(k);
  AddToHiLo(&projectile_y_pos[v1], &projectile_bomb_y_subpos[v1], amt);
  //r22 = projectile_bomb_y_subpos[v1];
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
  uint16 v3 = projectile_bomb_x_subpos[v1];
  //r22 = projectile_bomb_x_subpos[v1];
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
  //2r22 = projectile_bomb_y_subpos[v1];
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
  uint16 v2 = (projectile_x_pos[j >> 1] & 0xf) ^ ((BTS[k] & 0x40) != 0 ? 0xf : 0);
  uint16 v4 = (projectile_y_pos[j >> 1] & 0xf) ^ ((BTS[k] & 0x80) != 0 ? 0xf : 0);
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
    sub_949D5D,
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

static uint8 ClearCarryZero(CollInfo *ci) {  // 0x94A7C9
  return 0;
}

static uint8 SetCarryClearOvf(CollInfo *ci) {  // 0x94A7CD
  return 1;
}
