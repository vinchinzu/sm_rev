// Samus grapple helpers extracted from sm_94.c: post-grapple ejection,
// beam collision probing, swing constraints, and beam rendering.

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "block_reaction.h"

#define kAlignPos_Tab1 ((uint8*)RomFixedPtr(0x94892b))
#define kAlignYPos_Tab0 ((uint8*)RomFixedPtr(0x948b2b))
#define kTab948E54 ((uint8*)RomFixedPtr(0x948e54))

typedef struct PostGrappleCollInfo {
  uint16 pgci_r26;
  uint16 pgci_r28;
  uint16 pgci_r32;
} PostGrappleCollInfo;

typedef uint16 Func_PostGrappleCollInfo_U16(PostGrappleCollInfo *pgci);

static uint16 Samus_GetXposSpan(void);
static uint16 Samus_GetYposSpan(void);
static uint16 PostGrappleColl_Vert_Solid(PostGrappleCollInfo *pgci);
static uint16 PostGrappleColl_Horiz_Solid(PostGrappleCollInfo *pgci);
static uint8 BlockReactGrapple_GrappleBlock(CollInfo *ci);
static uint8 BlockReactGrapple_SpikeBlock(CollInfo *ci);
static void BlockFunc_A957(uint16 tmpD82, uint16 tmpD84);
static uint8 BlockReact_AA64(void);
static uint8 BlockReact_AA64_SpikeAir(CollInfo *ci);
static uint8 BlockReact_AA64_SpikeBlock(CollInfo *ci);
static uint8 BlockFunc_ABB0(void);
static uint8 BlockFunc_ABE6(uint16 tmpD82);
static uint16 GrappleInstr_Goto(uint16 j);
static uint16 CallGrappleInstr(uint32 ea, uint16 j);
static void DrawGrappleOams(uint16 x_r20, uint16 y_r24, uint16 chr_r38);
static void DrawGrappleOams3(void);

static uint16 PostGrappleColl_Horiz_Slope_NonSquare(PostGrappleCollInfo *pgci, uint16 k) {  // 0x948000
  if (!(samus_collision_direction & 1)) {
    if ((samus_x_pos >> 4) != SnesModulus(cur_block_index, room_width_in_blocks))
      return -1;
    if ((BTS[k] & 0x40) == 0)
      return PostGrappleColl_Horiz_Solid(pgci);
    uint16 v3 = samus_y_pos ^ (BTS[k] & 0x80 ? 0xF : 0);
    uint16 v4 = 16 * (BTS[k] & 0x1F) + (v3 & 0xF);
    int16 result = (kAlignPos_Tab1[v4] & 0x1F) - (pgci->pgci_r32 & 0xF) - 1;
    return (result >= 0) ? result : -1;
  }
  if ((samus_x_pos >> 4) != SnesModulus(cur_block_index, room_width_in_blocks))
    return -1;
  if (BTS[k] & 0x40)
    return PostGrappleColl_Horiz_Solid(pgci);
  uint16 v6 = samus_y_pos ^ (BTS[k] & 0x80 ? 0xF : 0);
  uint16 v7 = 16 * (BTS[k] & 0x1F) + (v6 & 0xF);
  int16 result = (kAlignPos_Tab1[v7] & 0x1F) - (pgci->pgci_r32 & 0xF) - 1;
  return (result <= 0) ? ~result : -1;
}

static uint16 PostGrappleColl_Vert_Slope_NonSquare(PostGrappleCollInfo *pgci, uint16 k) {  // 0x9480E0
  uint16 mod = SnesModulus(cur_block_index, room_width_in_blocks);
  if ((samus_x_pos >> 4) != mod)
    return -1;
  uint16 temp_collision_DD4 = pgci->pgci_r32 & 0xF;
  if (!(samus_collision_direction & 1)) {
    if (!(BTS[k] & 0x80))
      return PostGrappleColl_Vert_Solid(pgci);
    uint16 v4 = samus_x_pos ^ ((BTS[k] & 0x40) != 0 ? 0xF : 0);
    uint16 v5 = 16 * (BTS[k] & 0x1F) + (v4 & 0xF);
    uint16 result = (kAlignYPos_Tab0[v5] & 0x1F) - temp_collision_DD4 - 1;
    return ((int16)result >= 0) ? result : -1;
  }
  if (BTS[k] & 0x80)
    return PostGrappleColl_Vert_Solid(pgci);
  uint16 v8 = samus_x_pos ^ ((BTS[k] & 0x40) != 0 ? 0xF : 0);
  uint16 v9 = 16 * (BTS[k] & 0x1F) + (v8 & 0xF);
  uint16 v10 = (kAlignYPos_Tab0[v9] & 0x1F) - temp_collision_DD4 - 1;
  return ((int16)v10 <= 0) ? ~v10 : -1;
}

static uint16 PostGrappleColl_Horiz_Slope_Square(PostGrappleCollInfo *pgci, uint16 k) {  // 0x9481B8
  uint16 v1 = 4 * (BTS[k] & 0x1F) + ((BTS[k] >> 6) ^ ((pgci->pgci_r32 & 8) >> 3));
  if (!pgci->pgci_r26) {
    if (((samus_y_radius + samus_y_pos - 1) & 8) == 0) {
      if (!kTab948E54[v1])
        return -1;
      goto found_hit;
    }
    goto test_current_cell;
  }
  if (pgci->pgci_r26 != pgci->pgci_r28 || ((samus_y_pos - samus_y_radius) & 8) == 0) {
test_current_cell:
    if (kTab948E54[v1])
      goto found_hit;
  }
  if (!kTab948E54[v1 ^ 2])
    return -1;
found_hit:
  return (samus_collision_direction & 1) ? (pgci->pgci_r32 & 7) : ((pgci->pgci_r32 & 7) ^ 7);
}

static uint16 PostGrappleColl_Vertical_Slope_Square(PostGrappleCollInfo *pgci, uint16 k) {  // 0x948230
  uint16 v1 = 4 * (BTS[k] & 0x1F) + ((BTS[k] >> 6) ^ ((pgci->pgci_r32 & 8) >> 2));
  if (!pgci->pgci_r26) {
    if (((samus_x_radius + samus_x_pos - 1) & 8) == 0) {
      if (!kTab948E54[v1])
        return -1;
      goto found_hit;
    }
    goto test_current_cell;
  }
  if (pgci->pgci_r26 != pgci->pgci_r28 || ((samus_x_pos - samus_x_radius) & 8) == 0) {
test_current_cell:
    if (kTab948E54[v1])
      goto found_hit;
  }
  if (!kTab948E54[v1 ^ 1])
    return -1;
found_hit:
  return (samus_collision_direction & 1) ? (pgci->pgci_r32 & 7) : ((pgci->pgci_r32 & 7) ^ 7);
}

static uint16 ClearCarry_0(PostGrappleCollInfo *pgci) {  // 0x9482A7
  return -1;
}

static uint16 PostGrappleColl_Horiz_Slope(PostGrappleCollInfo *pgci) {  // 0x9482A9
  if ((BTS[cur_block_index] & 0x1F) < 5)
    return PostGrappleColl_Horiz_Slope_Square(pgci, cur_block_index);
  return PostGrappleColl_Horiz_Slope_NonSquare(pgci, cur_block_index);
}

static uint16 PostGrappleColl_Horiz_Solid(PostGrappleCollInfo *pgci) {  // 0x9482BE
  return pgci->pgci_r32 & 0xF;
}

static uint16 PostGrappleColl_Vert_Slope(PostGrappleCollInfo *pgci) {  // 0x9482C5
  if ((BTS[cur_block_index] & 0x1F) < 5)
    return PostGrappleColl_Vertical_Slope_Square(pgci, cur_block_index);
  return PostGrappleColl_Vert_Slope_NonSquare(pgci, cur_block_index);
}

static uint16 PostGrappleColl_Vert_Solid(PostGrappleCollInfo *pgci) {  // 0x9482DA
  return pgci->pgci_r32 & 0xF;
}

static Func_PostGrappleCollInfo_U16 *const kPostGrappleColl_Horiz[16] = {  // 0x948321
  ClearCarry_0,
  PostGrappleColl_Horiz_Slope,
  ClearCarry_0,
  ClearCarry_0,
  ClearCarry_0,
  ClearCarry_0,
  ClearCarry_0,
  ClearCarry_0,
  PostGrappleColl_Horiz_Solid,
  PostGrappleColl_Horiz_Solid,
  PostGrappleColl_Horiz_Solid,
  PostGrappleColl_Horiz_Solid,
  PostGrappleColl_Horiz_Solid,
  PostGrappleColl_Horiz_Solid,
  PostGrappleColl_Horiz_Solid,
  PostGrappleColl_Horiz_Solid,
};

static Func_PostGrappleCollInfo_U16 *const kPostGrappleColl_Vert[16] = {
  ClearCarry_0,
  PostGrappleColl_Vert_Slope,
  ClearCarry_0,
  ClearCarry_0,
  ClearCarry_0,
  ClearCarry_0,
  ClearCarry_0,
  ClearCarry_0,
  PostGrappleColl_Vert_Solid,
  PostGrappleColl_Vert_Solid,
  PostGrappleColl_Vert_Solid,
  PostGrappleColl_Vert_Solid,
  PostGrappleColl_Vert_Solid,
  PostGrappleColl_Vert_Solid,
  PostGrappleColl_Vert_Solid,
  PostGrappleColl_Vert_Solid,
};

static uint16 PostGrappleColl_Horiz(PostGrappleCollInfo *pgci, uint16 k) {
  cur_block_index = k >> 1;
  return kPostGrappleColl_Horiz[(level_data[cur_block_index] & 0xF000) >> 12](pgci);
}

static uint16 PostGrappleColl_Vert(PostGrappleCollInfo *pgci, uint16 k) {  // 0x948338
  cur_block_index = k >> 1;
  return kPostGrappleColl_Vert[(level_data[cur_block_index] & 0xF000) >> 12](pgci);
}

static void PostGrappleCollisionDetect_Right(void) {  // 0x94834F
  samus_collision_direction = 1;
  distance_to_eject_samus_left = 0;
  uint16 r28 = Samus_GetYposSpan();
  uint16 prod = Mult8x8((uint16)(samus_y_pos - samus_y_radius) >> 4, room_width_in_blocks);
  uint16 R32 = samus_x_radius + samus_x_pos - 1;
  uint16 v1 = 2 * (prod + (R32 >> 4));
  PostGrappleCollInfo pgci = { .pgci_r26 = r28, .pgci_r28 = r28, .pgci_r32 = R32 };
  do {
    int16 v2 = PostGrappleColl_Horiz(&pgci, v1);
    if (v2 >= 0) {
      uint16 v3 = v2 + 1;
      if (v3 >= distance_to_eject_samus_left)
        distance_to_eject_samus_left = v3;
    }
    v1 += room_width_in_blocks * 2;
  } while ((--pgci.pgci_r26 & 0x8000) == 0);
}

static void PostGrappleCollisionDetect_Left(void) {  // 0x9483B1
  samus_collision_direction = 0;
  distance_to_eject_samus_right = 0;
  uint16 r28 = Samus_GetYposSpan();
  uint16 prod = Mult8x8((uint16)(samus_y_pos - samus_y_radius) >> 4, room_width_in_blocks);
  uint16 R32 = samus_x_pos - samus_x_radius;
  uint16 v1 = 2 * (prod + (R32 >> 4));
  PostGrappleCollInfo pgci = { .pgci_r26 = r28, .pgci_r28 = r28, .pgci_r32 = R32 };
  do {
    int16 v2 = PostGrappleColl_Horiz(&pgci, v1);
    if (v2 >= 0) {
      uint16 v3 = v2 + 1;
      if (v3 >= distance_to_eject_samus_right)
        distance_to_eject_samus_right = v3;
    }
    v1 += room_width_in_blocks * 2;
  } while ((--pgci.pgci_r26 & 0x8000) == 0);
}

static void PostGrappleCollisionDetect_Down(void) {  // 0x94840F
  samus_collision_direction = 3;
  distance_to_eject_samus_up = 0;
  uint16 r28 = Samus_GetXposSpan();
  uint16 R32 = samus_y_radius + samus_y_pos - 1;
  uint16 v1 = 2 * (Mult8x8(R32 >> 4, room_width_in_blocks) + ((uint16)(samus_x_pos - samus_x_radius) >> 4));
  PostGrappleCollInfo pgci = { .pgci_r26 = r28, .pgci_r28 = r28, .pgci_r32 = R32 };
  do {
    int16 v2 = PostGrappleColl_Vert(&pgci, v1);
    if (v2 >= 0) {
      uint16 v3 = v2 + 1;
      if (v3 >= distance_to_eject_samus_up)
        distance_to_eject_samus_up = v3;
    }
    v1 += 2;
  } while ((--pgci.pgci_r26 & 0x8000) == 0);
}

static void PostGrappleCollisionDetect_Up(void) {  // 0x94846A
  samus_collision_direction = 2;
  distance_to_eject_samus_down = 0;
  uint16 r28 = Samus_GetXposSpan();
  uint16 R32 = samus_y_pos - samus_y_radius;
  uint16 v1 = 2 * (Mult8x8(R32 >> 4, room_width_in_blocks) + ((uint16)(samus_x_pos - samus_x_radius) >> 4));
  PostGrappleCollInfo pgci = { .pgci_r26 = r28, .pgci_r28 = r28, .pgci_r32 = R32 };
  do {
    int16 v2 = PostGrappleColl_Vert(&pgci, v1);
    if (v2 >= 0) {
      uint16 v3 = v2 + 1;
      if (v3 >= distance_to_eject_samus_down)
        distance_to_eject_samus_down = v3;
    }
    v1 += 2;
  } while ((--pgci.pgci_r26 & 0x8000) == 0);
}

void PostGrappleCollisionDetect_X(void) {  // 0x9484C4
  PostGrappleCollisionDetect_Right();
  PostGrappleCollisionDetect_Left();
}

void PostGrappleCollisionDetect_Y(void) {  // 0x9484CD
  PostGrappleCollisionDetect_Down();
  PostGrappleCollisionDetect_Up();
}

static uint8 ClearCarryZero(CollInfo *ci) {  // 0x94A7C9
  return 0;
}

static uint8 SetCarryClearOvf(CollInfo *ci) {  // 0x94A7CD
  return 1;
}

static uint8 BlockReactGrapple_GrappleBlock(CollInfo *ci) {  // 0x94A7D1
  static const uint16 kBlockReactGrapple_GrappleBlockPlm[4] = {
    0xd0d8, 0xd0dc, 0xd0e0, 0xd0d8,
  };
  grapple_beam_flags &= ~0x8000;
  if (BTS[cur_block_index] & 0x80)
    return 0;
  return SpawnPLM(kBlockReactGrapple_GrappleBlockPlm[BTS[cur_block_index]]);
}

static uint8 BlockReactGrapple_SpikeBlock(CollInfo *ci) {  // 0x94A7FD
  static const uint16 kBlockReactGrapple_SpikeBlockPlm[16] = {
    0xd0e4, 0xd0e4, 0xd0e4, 0xd0e8, 0xd0e4, 0xd0e4, 0xd0e4, 0xd0e4,
    0xd0e4, 0xd0e4, 0xd0e4, 0xd0e4, 0xd0e4, 0xd0e4, 0xd0e4, 0xd0e4,
  };
  if (BTS[cur_block_index] & 0x80)
    return 0;
  return SpawnPLM(kBlockReactGrapple_SpikeBlockPlm[BTS[cur_block_index]]);
}

static Func_CollInfo_U8 *const kBlockReactGrapple[16] = {
  ClearCarryZero,
  SetCarryClearOvf,
  ClearCarryZero,
  ClearCarryZero,
  BlockReact_ShootableAir,
  BlockReact_HorizExt,
  ClearCarryZero,
  BlockReact_BombableAir,
  SetCarryClearOvf,
  SetCarryClearOvf,
  BlockReactGrapple_SpikeBlock,
  SetCarryClearOvf,
  BlockReact_Shootable,
  BlockReact_VertExt,
  BlockReactGrapple_GrappleBlock,
  BlockReact_BombableBlock,
};

uint8 BlockCollGrappleBeam(void) {  // 0x94A85B
  int32 grapple_vel_x = (int16)grapple_beam_extension_x_velocity << 6;
  int32 grapple_vel_y = (int16)grapple_beam_extension_y_velocity << 6;
  uint8 result = 0;
  for (int i = 0; i < 4; i++) {
    AddToHiLo(&grapple_beam_end_x_offset, &grapple_beam_end_x_suboffset, grapple_vel_x);
    AddToHiLo(&grapple_beam_end_y_offset, &grapple_beam_end_y_suboffset, grapple_vel_y);
    SetHiLo(&grapple_beam_end_x_pos, &grapple_beam_end_x_subpos,
        __PAIR32__(grapple_beam_end_x_offset, grapple_beam_end_x_suboffset) + __PAIR32__(samus_x_pos, samus_x_subpos) + (grapple_beam_origin_x_offset << 16));
    SetHiLo(&grapple_beam_end_y_pos, &grapple_beam_end_y_subpos,
        __PAIR32__(grapple_beam_end_y_offset, grapple_beam_end_y_suboffset) + __PAIR32__(samus_y_pos, samus_y_subpos) + (grapple_beam_origin_y_offset << 16));
    result = BlockReactGrapple();
    if ((result & 0x40) != 0 && (result & 1) != 0) {
      grapple_beam_end_x_pos = grapple_beam_end_x_pos & 0xFFF0 | 8;
      grapple_beam_end_y_pos = grapple_beam_end_y_pos & 0xFFF0 | 8;
      return result;
    }
  }
  return result;
}

uint8 BlockReactGrapple(void) {  // 0x94A91F
  cur_block_index = Mult8x8(grapple_beam_end_y_pos >> 4, room_width_in_blocks) + (grapple_beam_end_x_pos >> 4);
  CollInfo ci = { 0 };
  uint8 rv;
  do {
    rv = kBlockReactGrapple[(level_data[cur_block_index] & 0xF000) >> 12](&ci);
  } while (rv & 0x80);
  return rv;
}

static void BlockFunc_A957(uint16 tmpD82, uint16 tmpD84) {
  uint16 v0 = tmpD82;
  if ((grapple_beam_flags & 0x8000) == 0) {
    int v1 = tmpD82 >> 1;
    grapple_beam_end_x_pos = (kSinCosTable8bit_Sext[v1 + 64] & 0x8000) != 0
        ? (grapple_beam_end_x_pos & 0xFFF0) | 7
        : (grapple_beam_end_x_pos & 0xFFF0) | 8;
    grapple_beam_end_y_pos = (kSinCosTable8bit_Sext[v1] & 0x8000) != 0
        ? (grapple_beam_end_y_pos & 0xFFF0) | 7
        : (grapple_beam_end_y_pos & 0xFFF0) | 8;
  }
  int v4 = v0 >> 1;
  int16 v5 = kSinCosTable8bit_Sext[v4 + 64];
  uint16 v6;
  if (v5 < 0) {
    v6 = (v5 == -256) ? (grapple_beam_end_x_pos - tmpD84)
                      : (grapple_beam_end_x_pos - (Mult8x8(tmpD84, -(int8)v5) >> 8));
  } else if (v5 == 256) {
    v6 = tmpD84 + grapple_beam_end_x_pos;
  } else {
    v6 = grapple_beam_end_x_pos + (Mult8x8(tmpD84, kSinCosTable8bit_Sext[v4 + 64]) >> 8);
  }
  grapple_beam_grapple_start_x = v6;
  grapple_beam_grapple_start_block_x = (uint8)(v6 >> 4);

  int16 v7 = kSinCosTable8bit_Sext[v4];
  uint16 v8;
  if (v7 < 0) {
    v8 = (v7 == -256) ? (grapple_beam_end_y_pos - tmpD84)
                      : (grapple_beam_end_y_pos - (Mult8x8(tmpD84, -v7) >> 8));
  } else if (v7 == 256) {
    v8 = tmpD84 + grapple_beam_end_y_pos;
  } else {
    v8 = grapple_beam_end_y_pos + (Mult8x8(tmpD84, kSinCosTable8bit_Sext[v4]) >> 8);
  }
  grapple_beam_grapple_start_y = v8;
  grapple_beam_grapple_start_block_y = (uint8)(v8 >> 4);
}

static uint8 ClearCarry_10(CollInfo *ci) {  // 0x94AA9A
  return 0;
}

static uint8 SetCarry_3(CollInfo *ci) {  // 0x94AA9C
  return 1;
}

static const uint16 g_word_94AAF7[16] = {
  0, 0, 0x10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static uint8 BlockReact_AA64_SpikeAir(CollInfo *ci) {
  if (samus_invincibility_timer)
    return 0;
  int16 v0 = *(uint16 *)&BTS[cur_block_index];
  if (v0 >= 0) {
    uint32 v = INT16_SHL16(g_word_94AAF7[v0]);
    if (v) {
      AddToHiLo(&samus_periodic_damage, &samus_periodic_subdamage, v);
      samus_invincibility_timer = 60;
      samus_knockback_timer = 10;
    }
  }
  return 0;
}

static const uint16 g_word_94AB70[16] = {
  60, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static uint8 BlockReact_AA64_SpikeBlock(CollInfo *ci) {
  if (samus_invincibility_timer)
    return 1;
  int16 v0 = *(uint16 *)&BTS[cur_block_index];
  if (v0 >= 0) {
    uint32 v = INT16_SHL16(g_word_94AB70[v0]);
    if (v) {
      AddToHiLo(&samus_periodic_damage, &samus_periodic_subdamage, v);
      samus_invincibility_timer = 60;
      samus_knockback_timer = 10;
    }
  }
  return 1;
}

static Func_CollInfo_U8 *const kBlockReactLut_AB90[16] = {
  ClearCarry_10,
  SetCarry_3,
  BlockReact_AA64_SpikeAir,
  ClearCarry_10,
  ClearCarry_10,
  BlockReact_HorizExt,
  ClearCarry_10,
  ClearCarry_10,
  SetCarry_3,
  SetCarry_3,
  BlockReact_AA64_SpikeBlock,
  SetCarry_3,
  SetCarry_3,
  BlockReact_VertExt,
  SetCarry_3,
  SetCarry_3,
};

static uint8 BlockReact_AA64(void) {  // 0x94AA64
  cur_block_index = Mult8x8(grapple_beam_grapple_start_block_y, room_width_in_blocks) + grapple_beam_grapple_start_block_x;
  CollInfo ci = { 0 };
  uint8 rv;
  do {
    rv = kBlockReactLut_AB90[(level_data[cur_block_index] & 0xF000) >> 12](&ci);
  } while (rv & 0x80);
  return rv;
}

static uint8 BlockFunc_ABB0(void) {
  cur_block_index = Mult8x8(grapple_beam_grapple_start_block_y, room_width_in_blocks) + grapple_beam_grapple_start_block_x;
  CollInfo ci = { 0 };
  uint8 rv;
  do {
    rv = kBlockReactLut_AB90[(level_data[cur_block_index] & 0xF000) >> 12](&ci);
  } while (rv & 0x80);
  return rv;
}

static uint8 BlockFunc_ABE6(uint16 tmpD82) {  // 0x94ABE6
  g_word_7E0D98 = 6;
  uint16 grapple_beam_tmpD84 = grapple_beam_length + 8;
  while (1) {
    BlockFunc_A957(tmpD82, grapple_beam_tmpD84);
    uint8 v0 = BlockFunc_ABB0() & 1;
    if (v0)
      return v0;
    grapple_beam_tmpD84 += 8;
    if (!--g_word_7E0D98)
      return 0;
  }
}

void BlockFunc_AC11(void) {  // 0x94AC11
  BlockFunc_A957(2 * grapple_beam_end_angle_hi, grapple_beam_length);
  x_pos_of_start_of_grapple_beam = grapple_beam_grapple_start_x;
  y_pos_of_start_of_grapple_beam = grapple_beam_grapple_start_y;
}

uint8 BlockFunc_AC31(void) {  // 0x94AC31
  if (!grapple_beam_length_delta)
    return 0;

  uint16 end = grapple_beam_length_delta + grapple_beam_length;
  if ((grapple_beam_length_delta & 0x8000) != 0) {
    if (end < 8) {
      grapple_beam_length_delta = 0;
      end = 8;
    }
    uint16 grapple_beam_tmpD82 = 2 * grapple_beam_end_angle_hi;
    for (uint16 v1 = grapple_beam_length; v1 != end; v1--) {
      BlockFunc_A957(grapple_beam_tmpD82, 8 + v1 - 1);
      if (BlockReact_AA64()) {
        grapple_beam_length = v1;
        return 1;
      }
    }
  } else {
    if (end >= 63) {
      grapple_beam_length_delta = 0;
      end = 63;
    }
    uint16 grapple_beam_tmpD82 = 2 * grapple_beam_end_angle_hi;
    for (uint16 v1 = grapple_beam_length; v1 != end; v1++) {
      BlockFunc_A957(grapple_beam_tmpD82, 56 + v1 + 1);
      if (BlockReact_AA64()) {
        grapple_beam_length = v1;
        return 1;
      }
    }
  }
  grapple_beam_length = end;
  return 0;
}

uint8 HandleMovementAndCollForSamusGrapple(void) {  // 0x94ACFE
  uint16 v0 = (grapple_beam_flags & 1) != 0 ? 160 : 256;
  uint16 v1 = grapple_beam_unkD2E + grapple_beam_unkD26;
  uint16 tmpD9C;
  if ((int16)v1 >= 0) {
    uint16 prod16 = Multiply16x16(v1, v0) >> 8;
    if (!prod16)
      return 0;
    tmpD9C = prod16;
    uint16 grapple_beam_tmpD88 = 2 * ((uint16)(grapple_beam_end_angle16 + prod16) >> 8);
    uint16 v2 = 2 * grapple_beam_end_angle_hi;
    if (v2 != grapple_beam_tmpD88) {
      while (1) {
        uint16 grapple_beam_tmpD86 = v2;
        if (BlockFunc_ABE6((v2 + 2) & 0x1FF) & 1) {
          int16 v4;
          LOBYTE(v4) = 0;
          HIBYTE(v4) = grapple_beam_tmpD86 >> 1;
          grapple_beam_end_angle16 = v4 | 0x80;
          grapple_beam_end_angles_mirror = v4 | 0x80;
          if ((g_word_7E0D98 == 6 || g_word_7E0D98 == 5) && grapple_beam_length == 8) {
            grapple_beam_unkD36 |= 0x8000;
            grapple_beam_unkD26 = 0;
            grapple_beam_unkD2E = 0;
            return 1;
          }
          grapple_beam_unkD30 = 16;
          grapple_beam_unkD26 = -((int16)grapple_beam_unkD26 >> 1);
          grapple_beam_unkD2E = -((int16)grapple_beam_unkD2E >> 1);
          return 1;
        }
        v2 = (grapple_beam_tmpD86 + 2) & 0x1FF;
        if (v2 == grapple_beam_tmpD88)
          break;
      }
    }
    grapple_beam_end_angle16 += tmpD9C;
    grapple_beam_end_angles_mirror = grapple_beam_end_angle16;
    grapple_beam_unkD36 &= ~0x8000;
    if ((--grapple_beam_unkD30 & 0x8000) != 0)
      grapple_beam_unkD30 = 0;
    if ((grapple_beam_unkD2E & 0x8000) == 0) {
      int16 v3 = (int16)grapple_beam_unkD2E - 6;
      grapple_beam_unkD2E = (v3 >= 0) ? v3 : 0;
    } else {
      int16 v3 = (int16)grapple_beam_unkD2E + 6;
      grapple_beam_unkD2E = (v3 < 0) ? v3 : 0;
    }
    return 0;
  }

  uint16 prod16 = Multiply16x16(-v1, v0) >> 8;
  if (!prod16)
    return 0;
  tmpD9C = -prod16;
  uint16 grapple_beam_tmpD88 = 2 * ((uint16)(grapple_beam_end_angle16 - prod16) >> 8);
  uint16 v5 = 2 * grapple_beam_end_angle_hi;
  if (v5 != grapple_beam_tmpD88) {
    while (1) {
      uint16 grapple_beam_tmpD86 = v5;
      if (BlockFunc_ABE6((v5 - 2) & 0x1FF) & 1) {
        int16 v7;
        LOBYTE(v7) = 0;
        HIBYTE(v7) = grapple_beam_tmpD86 >> 1;
        grapple_beam_end_angle16 = v7 | 0x80;
        grapple_beam_end_angles_mirror = v7 | 0x80;
        if ((g_word_7E0D98 == 6 || g_word_7E0D98 == 5) && grapple_beam_length == 8) {
          grapple_beam_unkD36 |= 0x8000;
          grapple_beam_unkD26 = 0;
          grapple_beam_unkD2E = 0;
          return 1;
        }
        grapple_beam_unkD30 = 16;
        grapple_beam_unkD26 = -((int16)grapple_beam_unkD26 >> 1);
        grapple_beam_unkD2E = -((int16)grapple_beam_unkD2E >> 1);
        return 1;
      }
      v5 = (grapple_beam_tmpD86 - 2) & 0x1FF;
      if (v5 == grapple_beam_tmpD88)
        break;
    }
  }
  grapple_beam_end_angle16 += tmpD9C;
  grapple_beam_end_angles_mirror = grapple_beam_end_angle16;
  grapple_beam_unkD36 &= ~0x8000;
  if ((--grapple_beam_unkD30 & 0x8000) != 0)
    grapple_beam_unkD30 = 0;
  if ((grapple_beam_unkD2E & 0x8000) == 0) {
    int16 v6 = (int16)grapple_beam_unkD2E - 6;
    grapple_beam_unkD2E = (v6 >= 0) ? v6 : 0;
  } else {
    int16 v6 = (int16)grapple_beam_unkD2E + 6;
    grapple_beam_unkD2E = (v6 < 0) ? v6 : 0;
  }
  return 0;
}

uint8 BlockFunc_AEE3(void) {  // 0x94AEE3
  if (((grapple_beam_unkD26 ^ grapple_beam_end_angle16) & 0x8000) != 0) {
    grapple_beam_unkD38 = 0;
    return 1;
  }
  if (++grapple_beam_unkD38 == 32)
    grapple_beam_function = FUNC16(GrappleBeam_Func2);
  grapple_beam_unkD26 = 0;
  grapple_beam_unkD2E = 0;
  return 1;
}

void GrappleFunc_AF87(void) {  // 0x94AF87
  for (int i = 30; i >= 0; i -= 8) {
    int v1 = i >> 1;
    grapple_segment_anim_instr_ptrs[v1] = addr_word_94B197;
    grapple_segment_anim_instr_timers[v1 + 15] = addr_word_94B193;
    grapple_segment_anim_instr_timers[v1 + 14] = addr_word_94B18F;
    grapple_segment_anim_instr_timers[v1 + 13] = addr_word_94B18B;
    grapple_segment_anim_instr_timers[v1] = 1;
    *(uint16 *)((uint8 *)&grapple_point_anim_ptr + i) = 1;
    *(uint16 *)((uint8 *)&grapple_point_anim_timer + i) = 1;
    *(uint16 *)((uint8 *)&grapple_beam_unkD3C + i) = 1;
  }
}

static uint16 GrappleInstr_Goto(uint16 j) {  // 0x94B0F4
  return *(uint16 *)RomPtr_94(j);
}

static uint16 CallGrappleInstr(uint32 ea, uint16 j) {
  switch (ea) {
  case fnGrappleInstr_Goto: return GrappleInstr_Goto(j);
  default: return Unreachable();
  }
}

void HandleGrappleBeamGfx(void) {  // 0x94AFBA
  int v1 = CalculateAngleFromXY(grapple_beam_end_x_pos - x_pos_of_start_of_grapple_beam_prevframe,
                                grapple_beam_end_y_pos - y_pos_of_start_of_grapple_beam_prevframe);
  uint32 r28_r26 = (int16)kSinCosTable8bit_Sext[v1 + 64] << 11;
  uint32 r32_r30 = (int16)kSinCosTable8bit_Sext[v1] << 11;

  uint16 chr = (grapple_beam_end_angle_hi & 0x80) >> 1;
  chr |= 2 * ((grapple_beam_end_angle_hi ^ chr) & 0x40 ^ 0x40);
  chr <<= 8;

  uint32 r20_r18 = (x_pos_of_start_of_grapple_beam_prevframe - layer1_x_pos - 4) << 16;
  uint32 r24_r22 = (y_pos_of_start_of_grapple_beam_prevframe - layer1_y_pos - 4) << 16;
  if ((grapple_beam_length & 0x8000) != 0)
    return;
  int n = ((grapple_beam_length / 8) & 0xF) - 1;
  int q = 15;
  do {
    if (grapple_segment_anim_instr_timers[q]-- == 1) {
      int i = grapple_segment_anim_instr_ptrs[q];
      const uint16 *v9;
      for (;;) {
        v9 = (const uint16 *)RomPtr_94(i);
        if ((*v9 & 0x8000) == 0)
          break;
        i = CallGrappleInstr(*v9 | 0x940000, i + 2);
      }
      grapple_segment_anim_instr_timers[q] = *v9;
      grapple_segment_anim_instr_ptrs[q] = i + 4;
    }
    if (((r24_r22 | r20_r18) & 0xFF000000) != 0)
      break;
    uint16 v12 = *(uint16 *)RomPtr_94(grapple_segment_anim_instr_ptrs[q] - 2);
    DrawGrappleOams(r20_r18 >> 16, r24_r22 >> 16, chr | v12);
    r20_r18 += r28_r26;
    r24_r22 += r32_r30;
  } while (q--, --n >= 0);
  if (samus_pose == kPose_B2_FaceR_Grapple_Air || samus_pose == kPose_B3_FaceL_Grapple_Air) {
    DrawGrappleOams3();
  } else if (((grapple_beam_end_y_pos - layer1_y_pos) & 0xFF00) == 0) {
    DrawGrappleOams3();
  }
}

static void DrawGrappleOams(uint16 x_r20, uint16 y_r24, uint16 chr_r38) {  // 0x94B0AA
  uint16 v1 = oam_next_ptr;
  OamEnt *v2 = gOamEnt(oam_next_ptr);
  v2->xcoord = x_r20;
  v2->ycoord = y_r24;
  *(uint16 *)&v2->charnum = chr_r38;
  oam_next_ptr = v1 + 4;
}

static void DrawGrappleOams3(void) {  // 0x94B14B
  uint16 idx = oam_next_ptr;
  OamEnt *v2 = gOamEnt(idx);
  uint16 x = grapple_beam_end_x_pos - layer1_x_pos - 4;
  v2->xcoord = x;
  v2->ycoord = grapple_beam_end_y_pos - layer1_y_pos - 4;
  *(uint16 *)&v2->charnum = 14880;
  oam_ext[idx >> 5] |= (((x & 0x100) >> 8)) << (2 * ((idx >> 2) & 7));
  oam_next_ptr = idx + 4;
}

static uint16 Samus_GetYposSpan(void) {  // 0x949495
  uint16 r26 = (samus_y_pos - samus_y_radius) & 0xFFF0;
  return (uint16)(samus_y_radius + samus_y_pos - 1 - r26) >> 4;
}

static uint16 Samus_GetXposSpan(void) {  // 0x9494B5
  uint16 r26 = (samus_x_pos - samus_x_radius) & 0xFFF0;
  return (uint16)(samus_x_radius + samus_x_pos - 1 - r26) >> 4;
}
