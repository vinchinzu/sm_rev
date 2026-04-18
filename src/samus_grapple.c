// Samus grapple helpers extracted from bank 94: post-grapple ejection,
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
static void UpdateGrappleBeamTiles(void);
static void CallGrappleBeamFunc(uint32 ea);

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

// ============================================================================
// Grapple Beam FSM - Extracted from sm_9b.c (Cluster C)
// ============================================================================

#define g_off_9BC3C6 ((uint16*)RomFixedPtr(0x9bc3c6))
#define g_off_9BC3EE ((uint16*)RomFixedPtr(0x9bc3ee))
#define g_off_9BC416 ((uint16*)RomFixedPtr(0x9bc416))
#define grapple_beam_special_angles ((GrappleBeamSpecialAngles*)RomFixedPtr(0x9bc43e))
#define kGrappleBeam_SwingingData ((uint8*)RomFixedPtr(0x9bc1c2))
#define kGrappleBeam_SwingingData2 ((uint8*)RomFixedPtr(0x9bc2c2))
#define kGrappleBeam_SwingingData3 ((uint8*)RomFixedPtr(0x9bc302))
#define kGrappleBeam_OriginX_NoRun ((uint16*)RomFixedPtr(0x9bc122))
#define kGrappleBeam_OriginY_NoRun ((uint16*)RomFixedPtr(0x9bc136))
#define kGrappleBeam_0x0d1a_offs_NoRun ((uint16*)RomFixedPtr(0x9bc14a))
#define kGrappleBeam_0x0d1c_offs_NoRun ((uint16*)RomFixedPtr(0x9bc15e))
#define kGrappleBeam_OriginX_Run ((uint16*)RomFixedPtr(0x9bc172))
#define kGrappleBeam_OriginY_Run ((uint16*)RomFixedPtr(0x9bc186))
#define kGrappleBeam_0x0d1a_offs_Run ((uint16*)RomFixedPtr(0x9bc19a))
#define kGrappleBeam_0x0d1c_offs_Run ((uint16*)RomFixedPtr(0x9bc1ae))
#define g_off_9BC344 (*(uint16*)RomFixedPtr(0x9bc344))
#define g_off_9BC342 (*(uint16*)RomFixedPtr(0x9bc342))
#define g_off_9BC346 ((uint16*)RomFixedPtr(0x9bc346))
#define kFlareAnimDelays ((uint16*)RomFixedPtr(0x90c481))
#define kFlareAnimDelays_Main ((uint8*)RomFixedPtr(0x90c487))
#define kFlareAnimDelays_SlowSparks ((uint8*)RomFixedPtr(0x90c4a7))
#define kFlareAnimDelays_FastSparks ((uint8*)RomFixedPtr(0x90c4ae))
#define g_word_93A22B ((uint16*)RomFixedPtr(0x93a22b))
#define g_word_93A225 ((uint16*)RomFixedPtr(0x93a225))
#define g_byte_9BC9BA ((uint8*)RomFixedPtr(0x9bc9ba))
#define g_byte_9BC9C4 ((uint8*)RomFixedPtr(0x9bc9c4))
#define kGrappleBeam_Ext_Xvel ((uint16*)RomFixedPtr(0x9bc0db))
#define kGrappleBeam_Ext_Yvel ((uint16*)RomFixedPtr(0x9bc0ef))
#define kGrappleBeam_Init_EndAngle ((uint16*)RomFixedPtr(0x9bc104))

static const uint16 g_word_9BC118 = 24;
static const uint16 g_word_9BC11A = 0xc;
static const uint16 g_word_9BC11C = 5;
static const uint16 g_word_9BC11E = 0x480;
static const uint16 g_word_9BC120 = 0x300;

static const uint8 kIsGrappleBannedForMovementType[28] = {
  0, 0, 0, 1, 1, 0, 0, 1, 1, 1,
  1, 0, 0, 1, 1, 1, 0, 1, 1, 1,
  1, 0, 0, 1, 1, 1, 0, 1,
};

// Forward declarations for internal functions
static void CallGrappleBeamFunc(uint32 ea);
static void GrappleBeamFunc_BE98(void);
static void GrappleBeamFunc_FireGoToCancel(void);
static void UpdateGrappleBeamTiles(void);

void CancelGrappleBeamIfIncompatiblePose(void) {  // 0x9BB861
  int16 v0;

  if (kIsGrappleBannedForMovementType[samus_movement_type]) {
LABEL_2:
    if (grapple_beam_function != FUNC16(GrappleBeamFunc_Inactive))
      grapple_beam_function = FUNC16(GrappleBeamFunc_Cancel);
    return;
  }
  if (grapple_beam_function != FUNC16(GrappleBeamFunc_Inactive)
      && sign16(grapple_beam_function + 0x3882)) {
    v0 = kPoseParams[samus_pose].direction_shots_fired;
    if ((v0 & 0xF0) == 0) {
      if (v0 == grapple_beam_direction)
        return;
      if (grapple_varCF6) {
        QueueSfx1_Max6(7);
        grapple_beam_function = FUNC16(GrappleBeamFunc_FireGoToCancel);
        return;
      }
    }
    goto LABEL_2;
  }
}

uint8 CheckIfGrappleIsConnectedToBlock(void) {  // 0x9BB8F1
  grapple_beam_extension_x_velocity = 0;
  grapple_beam_extension_y_velocity = 0;
  if ((BlockReactGrapple() & 1) == 0)
    return 0;
  samus_grapple_flags = 1;
  return 1;
}

static int ProcessEnemyGrappleBeamColl(uint16 a, uint16 r18) {  // 0x9BB907
  switch (a) {
  case 0:
  case 2:
    return -1;  // clc
  case 3:
    return 1;
  case 1:
  case 4:
  case 5:
    return 0;
  case 6:
    r18 = *((uint16 *)RomPtr_A0(r18) + 3);
    if ((equipped_items & 0x20) != 0) {
      r18 >>= 2;
    } else if (equipped_items & 1) {
      r18 >>= 1;
    }
    Samus_DealDamage(r18);
    samus_invincibility_timer = 96;
    samus_knockback_timer = 5;
    knockback_x_dir = samus_pose_x_dir == 4;
    return 1;
  default:
    Unreachable();
    return 0;
  }
}

void CallGrappleNextFunc(uint32 ea) {
  switch (ea) {
  case fnGrappleNext_SwingClockwise: GrappleNext_SwingClockwise(); return;
  case fnGrappleNext_SwingAntiClockwise: GrappleNext_SwingAntiClockwise(); return;
  case fnGrappleNext_StandAimRight: GrappleNext_StandAimRight(); return;
  case fnGrappleNext_StandAimDownRight: GrappleNext_StandAimDownRight(); return;
  case fnGrappleNext_StandAimDownLeft: GrappleNext_StandAimDownLeft(); return;
  case fnGrappleNext_StandAimLeft: GrappleNext_StandAimLeft(); return;
  case fnGrappleNext_CrouchAimRight: GrappleNext_CrouchAimRight(); return;
  case fnGrappleNext_CrouchAimDownRight: GrappleNext_CrouchAimDownRight(); return;
  case fnGrappleNext_CrouchAimDownLeft: GrappleNext_CrouchAimDownLeft(); return;
  case fnGrappleNext_CrouchAimLeft: GrappleNext_CrouchAimLeft(); return;
  default: Unreachable();
  }
}

void HandleConnectingGrapple(void) {  // 0x9BB97C
  if (samus_movement_type == kMovementType_1A_GrabbedByDraygon) {
    grapple_beam_function = FUNC16(GrappleBeamFunc_ConnectedLockedInPlace);
    grapple_beam_length_delta = 0;
  } else {
    int v1 = 2 * grapple_beam_direction;
    if (samus_y_speed || samus_y_subspeed) {
      grapple_beam_function = g_off_9BC3EE[v1];
      CallGrappleNextFunc(g_off_9BC3EE[v1 + 1] | 0x9B0000);
    } else if (samus_movement_type == 5) {
      grapple_beam_function = g_off_9BC416[v1];
      CallGrappleNextFunc(g_off_9BC416[v1 + 1] | 0x9B0000);
    } else {
      grapple_beam_function = g_off_9BC3C6[v1];
      CallGrappleNextFunc(g_off_9BC3C6[v1 + 1] | 0x9B0000);
    }
  }
}

void GrappleNext_SwingClockwise(void) {  // 0x9BB9D9
  samus_new_pose_interrupted = kPose_B2_FaceR_Grapple_Air;
  HandleConnectingGrapple_Swinging();
}

void GrappleNext_SwingAntiClockwise(void) {  // 0x9BB9E2
  samus_new_pose_interrupted = kPose_B3_FaceL_Grapple_Air;
  HandleConnectingGrapple_Swinging();
}

void GrappleNext_StandAimRight(void) {  // 0x9BB9EA
  samus_new_pose_interrupted = kPose_A8_FaceR_Grappling;
  HandleConnectingGrapple_StuckInPlace();
}

void GrappleNext_StandAimDownRight(void) {  // 0x9BB9F3
  samus_new_pose_interrupted = kPose_AA_FaceR_Grappling_AimDR;
  HandleConnectingGrapple_StuckInPlace();
}

void GrappleNext_StandAimDownLeft(void) {  // 0x9BB9FC
  samus_new_pose_interrupted = kPose_AB_FaceL_Grappling_AimDL;
  HandleConnectingGrapple_StuckInPlace();
}

void GrappleNext_StandAimLeft(void) {  // 0x9BBA05
  samus_new_pose_interrupted = kPose_A9_FaceL_Grappling;
  HandleConnectingGrapple_StuckInPlace();
}

void GrappleNext_CrouchAimRight(void) {  // 0x9BBA0E
  samus_new_pose_interrupted = kPose_B4_FaceR_Grappling_Crouch;
  HandleConnectingGrapple_StuckInPlace();
}

void GrappleNext_CrouchAimDownRight(void) {  // 0x9BBA17
  samus_new_pose_interrupted = kPose_B6_FaceR_Grappling_Crouch_AimDR;
  HandleConnectingGrapple_StuckInPlace();
}

void GrappleNext_CrouchAimDownLeft(void) {  // 0x9BBA20
  samus_new_pose_interrupted = kPose_B7_FaceL_Grappling_Crouch_AimDL;
  HandleConnectingGrapple_StuckInPlace();
}

void GrappleNext_CrouchAimLeft(void) {  // 0x9BBA29
  samus_new_pose_interrupted = kPose_B5_FaceL_Grappling_Crouch;
  HandleConnectingGrapple_StuckInPlace();
}

void HandleConnectingGrapple_Swinging(void) {  // 0x9BBA61
  samus_special_transgfx_index = 9;
  uint16 v0 = swap16(CalculateAngleFromXY(samus_x_pos - grapple_beam_end_x_pos, samus_y_pos - grapple_beam_end_y_pos));
  grapple_beam_end_angle16 = v0;
  grapple_beam_end_angles_mirror = v0;
  grapple_beam_length_delta = 0;
  if (!sign16(grapple_beam_length - 64))
    grapple_beam_length -= 24;
  BlockFunc_AC11();
}

void HandleConnectingGrapple_StuckInPlace(void) {  // 0x9BBA9B
  samus_special_transgfx_index = 10;
  uint16 v0 = swap16(CalculateAngleFromXY(samus_x_pos - grapple_beam_end_x_pos, samus_y_pos - grapple_beam_end_y_pos));
  grapple_beam_end_angle16 = v0;
  grapple_beam_end_angles_mirror = v0;
  grapple_beam_length_delta = 0;
  if (!sign16(grapple_beam_length - 64))
    grapple_beam_length -= 24;
  BlockFunc_AC11();
}

uint8 HandleSpecialGrappleBeamAngles(void) {  // 0x9BBAD5
  uint16 v0 = 7;
  while (grapple_beam_end_angle16 != grapple_beam_special_angles[v0].field_0) {
    if ((--v0 & 0x8000) != 0)
      return 0;
  }
  samus_new_pose_interrupted = grapple_beam_special_angles[v0].field_2;
  samus_x_pos = grapple_beam_end_x_pos + grapple_beam_special_angles[v0].field_4;
  samus_y_pos = grapple_beam_end_y_pos + grapple_beam_special_angles[v0].field_6;
  grapple_beam_function = grapple_beam_special_angles[v0].field_8;
  samus_special_transgfx_index = 0;
  slow_grabble_scrolling_flag = 0;
  int16 v2 = samus_x_pos - samus_prev_x_pos;
  if ((int16)(samus_x_pos - samus_prev_x_pos) < 0) {
    if (sign16(v2 + 12))
      samus_prev_x_pos = samus_x_pos + 12;
  } else if (!sign16(v2 - 13)) {
    samus_prev_x_pos = samus_x_pos - 12;
  }
  int16 v3 = samus_y_pos - samus_prev_y_pos;
  if ((int16)(samus_y_pos - samus_prev_y_pos) < 0) {
    if (sign16(v3 + 12))
      samus_prev_y_pos = samus_y_pos + 12;
  } else if (!sign16(v3 - 13)) {
    samus_prev_y_pos = samus_y_pos - 12;
  }
  return 1;
}

static void GrappleBeamFunc_BB64(void) {  // 0x9BBB64
  if ((joypad1_newkeys & kButton_Up) != 0) {
    if (grapple_beam_length)
      grapple_beam_length_delta = -2;
  } else if ((joypad1_newkeys & kButton_Down) != 0) {
    if (sign16(grapple_beam_length - 64))
      grapple_beam_length_delta = 2;
    else
      grapple_beam_length = 64;
  }
  if (sign16((grapple_beam_end_angle_hi << 8) - 0x4000) || !sign16((grapple_beam_end_angle_hi << 8) + 0x4000))
    goto LABEL_13;
  if ((joypad1_lastkeys & 0x200) != 0) {
    if (grapple_beam_end_angle_hi << 8 == 0x8000 && !grapple_beam_unkD26)
      grapple_beam_unkD26 = 256;
    if (grapple_beam_flags && (grapple_beam_flags & 1) != 0)
      grapple_beam_unkD2A = g_word_9BC11A >> 1;
    else
      grapple_beam_unkD2A = g_word_9BC11A;
  } else {
    if ((joypad1_lastkeys & 0x100) == 0) {
LABEL_13:
      grapple_beam_unkD2A = 0;
      return;
    }
    if (grapple_beam_end_angle_hi << 8 == 0x8000 && !grapple_beam_unkD26)
      grapple_beam_unkD26 = -256;
    if (grapple_beam_flags && (grapple_beam_flags & 1) != 0)
      grapple_beam_unkD2A = -(g_word_9BC11A >> 1);
    else
      grapple_beam_unkD2A = -g_word_9BC11A;
  }
}

static void GrappleBeamFunc_BC1F(void) {  // 0x9BBC1F
  if ((grapple_beam_end_angle16 & 0xC000) == 0xC000) {
    grapple_beam_unkD2C = -(g_word_9BC11C >> 2);
    if (grapple_beam_flags && (grapple_beam_flags & 1) != 0)
      grapple_beam_unkD28 = -(g_word_9BC118 >> 3);
    else
      grapple_beam_unkD28 = -(g_word_9BC118 >> 2);
  } else if (sign16(grapple_beam_end_angle16)) {
    if (grapple_beam_end_angle_hi << 8 == 0x8000) {
      grapple_beam_unkD28 = 0;
      grapple_beam_unkD2C = 0;
      uint16 v0 = grapple_beam_unkD26;
      if ((grapple_beam_unkD26 & 0x8000) != 0)
        v0 = ~(grapple_beam_unkD26 - 1);
      if (sign16(HIBYTE(v0) - 1))
        grapple_beam_unkD26 = 0;
    } else {
      grapple_beam_unkD2C = -g_word_9BC11C;
      if (grapple_beam_flags && (grapple_beam_flags & 1) != 0)
        grapple_beam_unkD28 = -(g_word_9BC118 >> 1);
      else
        grapple_beam_unkD28 = -g_word_9BC118;
    }
  } else if ((grapple_beam_end_angle16 & 0x4000) != 0) {
    grapple_beam_unkD2C = g_word_9BC11C;
    if (grapple_beam_flags && (grapple_beam_flags & 1) != 0)
      grapple_beam_unkD28 = g_word_9BC118 >> 1;
    else
      grapple_beam_unkD28 = g_word_9BC118;
  } else {
    grapple_beam_unkD2C = g_word_9BC11C >> 2;
    if (grapple_beam_flags && (grapple_beam_flags & 1) != 0)
      grapple_beam_unkD28 = g_word_9BC118 >> 3;
    else
      grapple_beam_unkD28 = g_word_9BC118 >> 2;
  }
}

static void GrappleBeamFunc_BCFF(void) {  // 0x9BBCFF
  grapple_beam_unkD26 += grapple_beam_unkD2A + grapple_beam_unkD28;
  if (((grapple_beam_unkD26 ^ grapple_beam_end_angle16) & 0x8000) != 0)
    grapple_beam_unkD26 += grapple_beam_unkD2C;
  if ((grapple_beam_unkD26 & 0x8000) == 0) {
    if (grapple_beam_unkD26 >= g_word_9BC11E)
      grapple_beam_unkD26 = g_word_9BC11E;
  } else if ((uint16)-grapple_beam_unkD26 >= g_word_9BC11E) {
    grapple_beam_unkD26 = -g_word_9BC11E;
  }
}

static void GrappleBeamFunc_BD44(void) {  // 0x9BBD44
  if (grapple_beam_unkD30 && (button_config_jump_a & joypad1_newkeys) != 0) {
    if (grapple_beam_unkD26) {
      if ((grapple_beam_unkD26 & 0x8000) != 0) {
        if (grapple_beam_flags && (grapple_beam_flags & 1) != 0)
          grapple_beam_unkD2E = -(g_word_9BC120 >> 1);
        else
          grapple_beam_unkD2E = -g_word_9BC120;
      } else if (grapple_beam_flags && (grapple_beam_flags & 1) != 0) {
        grapple_beam_unkD2E = g_word_9BC120 >> 1;
      } else {
        grapple_beam_unkD2E = g_word_9BC120;
      }
    } else {
      grapple_beam_unkD2E = 0;
    }
  }
}

void GrappleBeamFunc_BD95(void) {  // 0x9BBD95
  uint16 v0 = abs16(grapple_beam_unkD26);
  uint16 v1;

  if (!sign16(v0 - 64)) {
    slow_grabble_scrolling_flag = 1;
LABEL_7:
    samus_anim_frame_timer = 15;
    v1 = kGrappleBeam_SwingingData[HIBYTE(grapple_beam_end_angles_mirror)];
    samus_anim_frame = v1;
    goto LABEL_8;
  }
  slow_grabble_scrolling_flag = 0;
  if (grapple_beam_end_angle_hi << 8 != 0x8000)
    goto LABEL_7;
  if (sign16(samus_anim_frame - 64)) {
    samus_anim_frame_timer = 8;
    samus_anim_frame = 64;
  }
  v1 = kGrappleBeam_SwingingData[HIBYTE(grapple_beam_end_angles_mirror)];
LABEL_8:;
  uint16 v2 = 2 * v1;
  if ((abs16(grapple_beam_unkD2E) & 0xFF00) == 256) {
    uint16 v3 = samus_anim_frame;
    if (!sign16(samus_anim_frame - 64))
      v3 = 16;
    samus_anim_frame = v3 + 32;
  }
  if (samus_pose_x_dir == 4) {
    samus_x_pos = x_pos_of_start_of_grapple_beam + (int8)kGrappleBeam_SwingingData2[v2];
    samus_y_pos = y_pos_of_start_of_grapple_beam + (int8)kGrappleBeam_SwingingData2[v2 + 1];
  } else {
    samus_x_pos = x_pos_of_start_of_grapple_beam + (int8)kGrappleBeam_SwingingData3[v2];
    samus_y_pos = y_pos_of_start_of_grapple_beam + (int8)kGrappleBeam_SwingingData3[v2 + 1];
  }
  x_pos_of_start_of_grapple_beam_prevframe = x_pos_of_start_of_grapple_beam;
  y_pos_of_start_of_grapple_beam_prevframe = y_pos_of_start_of_grapple_beam;
  GrappleBeamFunc_BE98();
}

static void GrappleBeamFunc_BE98(void) {  // 0x9BBE98
  int16 v0;
  int16 v1;

  v0 = samus_x_pos - samus_prev_x_pos;
  if ((int16)(samus_x_pos - samus_prev_x_pos) < 0) {
    if (sign16(v0 + 12))
      samus_prev_x_pos = samus_x_pos + 12;
  } else if (!sign16(v0 - 13)) {
    samus_prev_x_pos = samus_x_pos - 12;
  }
  v1 = samus_y_pos - samus_prev_y_pos;
  if ((int16)(samus_y_pos - samus_prev_y_pos) < 0) {
    if (sign16(v1 + 12))
      samus_prev_y_pos = samus_y_pos + 12;
  } else if (!sign16(v1 - 13)) {
    samus_prev_y_pos = samus_y_pos - 12;
  }
}

void GrappleBeamFunc_BEEB(void) {  // 0x9BBEEB
  int v0 = grapple_beam_direction;
  samus_x_pos = x_pos_of_start_of_grapple_beam - kGrappleBeam_OriginX_NoRun[v0];
  x_pos_of_start_of_grapple_beam_prevframe = kGrappleBeam_0x0d1a_offs_NoRun[v0] + samus_x_pos;
  samus_y_pos = y_pos_of_start_of_grapple_beam - kGrappleBeam_OriginY_NoRun[v0];
  y_pos_of_start_of_grapple_beam_prevframe = kGrappleBeam_0x0d1c_offs_NoRun[v0] + samus_y_pos;
}

void GrappleBeamFunc_BF1B(void) {  // 0x9BBF1B
  uint16 r22 = kPoseParams[samus_pose].y_offset_to_gfx;
  uint16 v0 = 2 * grapple_beam_direction;
  if (samus_pose == kPose_49_FaceL_Moonwalk || samus_pose == kPose_4A_FaceR_Moonwalk || samus_movement_type != 1) {
    int v1 = v0 >> 1;
    x_pos_of_start_of_grapple_beam = kGrappleBeam_OriginX_NoRun[v1] + samus_x_pos;
    x_pos_of_start_of_grapple_beam_prevframe = kGrappleBeam_0x0d1a_offs_NoRun[v1] + samus_x_pos;
    y_pos_of_start_of_grapple_beam = kGrappleBeam_OriginY_NoRun[v1] + samus_y_pos - r22;
    y_pos_of_start_of_grapple_beam_prevframe = kGrappleBeam_0x0d1c_offs_NoRun[v1] + samus_y_pos - r22;
  } else {
    int v2 = v0 >> 1;
    x_pos_of_start_of_grapple_beam = kGrappleBeam_OriginX_Run[v2] + samus_x_pos;
    x_pos_of_start_of_grapple_beam_prevframe = kGrappleBeam_0x0d1a_offs_Run[v2] + samus_x_pos;
    y_pos_of_start_of_grapple_beam = kGrappleBeam_OriginY_Run[v2] + samus_y_pos - r22;
    y_pos_of_start_of_grapple_beam_prevframe = kGrappleBeam_0x0d1c_offs_Run[v2] + samus_y_pos - r22;
  }
}

void UpdateGrappleBeamTilesAndIncrFlameCtr(void) {  // 0x9BBFA5
  UpdateGrappleBeamTiles();
  if (sign16(flare_counter - 120))
    ++flare_counter;
}

static void UpdateGrappleBeamTiles(void) {  // 0x9BBFBD
  if ((--grapple_point_anim_timer & 0x8000) != 0) {
    grapple_point_anim_timer = 5;
    grapple_point_anim_ptr += 512;
    if ((int16)(grapple_point_anim_ptr - g_off_9BC344) >= 0)
      grapple_point_anim_ptr = g_off_9BC342;
  }
  uint16 v0 = vram_write_queue_tail;
  gVramWriteEntry(vram_write_queue_tail)->size = 32;
  v0 += 2;
  gVramWriteEntry(v0)->size = grapple_point_anim_ptr;
  v0 += 2;
  LOBYTE(gVramWriteEntry(v0++)->size) = -102;
  gVramWriteEntry(v0)->size = 25088;
  vram_write_queue_tail = v0 + 2;
  uint16 v1 = (grapple_beam_end_angle_hi >> 1) & 0xFE;
  v0 += 2;
  gVramWriteEntry(v0)->size = 128;
  v0 += 2;
  gVramWriteEntry(v0)->size = g_off_9BC346[v1 >> 1];
  v0 += 2;
  LOBYTE(gVramWriteEntry(v0++)->size) = -102;
  gVramWriteEntry(v0)->size = 25104;
  vram_write_queue_tail = v0 + 2;
}

void HandleGrappleBeamFlare(void) {  // 0x9BC036
  if (flare_counter) {
    if (flare_counter == 1) {
      flare_animation_frame = 16;
      flare_animation_timer = 3;
    }
    if ((--flare_animation_timer & 0x8000) != 0) {
      uint16 v0 = ++flare_animation_frame;
      if (kFlareAnimDelays_Main[flare_animation_frame] == 254) {
        flare_animation_frame -= kFlareAnimDelays_Main[(uint16)(flare_animation_frame + 1)];
        v0 = flare_animation_frame;
      }
      flare_animation_timer = kFlareAnimDelays_Main[v0];
    }
    uint16 r22;
    if (samus_pose_x_dir == 4)
      r22 = flare_animation_frame + g_word_93A22B[0];
    else
      r22 = flare_animation_frame + g_word_93A225[0];
    uint16 r20 = x_pos_of_start_of_grapple_beam_prevframe - layer1_x_pos;
    uint16 r18 = y_pos_of_start_of_grapple_beam_prevframe - layer1_y_pos;
    if (((y_pos_of_start_of_grapple_beam_prevframe - layer1_y_pos) & 0xFF00) != 0)
      ;
    else
      DrawBeamGrappleSpritemap(r22, r20, r18);
  }
}

void GrappleBeamHandler(void) {  // 0x9BC490
  uint16 r18;
  if (grapple_varCF6)
    --grapple_varCF6;
  samus_grapple_flags &= ~1;
  CancelGrappleBeamIfIncompatiblePose();
  CallGrappleBeamFunc(grapple_beam_function | 0x9B0000);
  if (grapple_beam_function != FUNC16(GrappleBeamFunc_Inactive)
      && sign16(grapple_beam_function - FUNC16(GrappleBeamFunc_Cancel))
      && (samus_suit_palette_index & 4) == 0
      && fx_type
      && (r18 = Samus_GetBottom_R18(), (fx_y_pos & 0x8000) == 0)
      && sign16(fx_y_pos - r18)) {
    grapple_beam_flags |= 1;
  } else {
    grapple_beam_flags &= ~1;
  }
}

static void GrappleBeamFunc_Inactive(void) {  // 0x9BC4F0
  if ((button_config_shoot_x & joypad1_newkeys) != 0 || (button_config_shoot_x & joypad1_newinput_samusfilter) != 0) {
    GrappleBeamFunc_FireGoToCancel();
  } else if (flare_counter) {
    flare_counter = 0;
    flare_animation_frame = 0;
    flare_slow_sparks_anim_frame = 0;
    flare_fast_sparks_anim_frame = 0;
    flare_animation_timer = 0;
    flare_slow_sparks_anim_timer = 0;
    flare_fast_sparks_anim_timer = 0;
    Samus_LoadSuitPalette();
  }
}

static void GrappleBeamFunc_FireGoToCancel(void) {  // 0x9BC51E
  uint16 r22;

  int v0;
  if (samus_pose == kPose_F0_FaceR_Draygon_Move || samus_pose == kPose_BE_FaceL_Draygon_Move) {
    v0 = CheckBannedDraygonGrappleDirs(samus_pose);
    r22 = 6;
  } else {
    r22 = kPoseParams[samus_pose].y_offset_to_gfx;
    v0 = kPoseParams[samus_pose].direction_shots_fired;
    if ((v0 & 0xF0) != 0) {
      grapple_beam_function = FUNC16(GrappleBeamFunc_Cancel);
      return;
    }
  }
  grapple_beam_direction = v0;
  int v1 = v0;
  grapple_beam_extension_x_velocity = kGrappleBeam_Ext_Xvel[v1];
  grapple_beam_extension_y_velocity = kGrappleBeam_Ext_Yvel[v1];
  grapple_beam_end_angle16 = kGrappleBeam_Init_EndAngle[v1];
  grapple_beam_end_angles_mirror = grapple_beam_end_angle16;
  grapple_varCF6 = 10;
  if (samus_pose == kPose_49_FaceL_Moonwalk || samus_pose == kPose_4A_FaceR_Moonwalk || samus_movement_type != 1) {
    grapple_beam_origin_x_offset = kGrappleBeam_OriginX_NoRun[v1];
    grapple_beam_origin_y_offset = kGrappleBeam_OriginY_NoRun[v1] - r22;
    grapple_beam_end_x_pos = kGrappleBeam_OriginX_NoRun[v1] + samus_x_pos;
    x_pos_of_start_of_grapple_beam = grapple_beam_end_x_pos;
    x_pos_of_start_of_grapple_beam_prevframe = kGrappleBeam_0x0d1a_offs_NoRun[v1] + samus_x_pos;
    grapple_beam_end_y_pos = kGrappleBeam_OriginY_NoRun[v1] + samus_y_pos - r22;
    y_pos_of_start_of_grapple_beam = grapple_beam_end_y_pos;
    y_pos_of_start_of_grapple_beam_prevframe = kGrappleBeam_0x0d1c_offs_NoRun[v1] + samus_y_pos - r22;
  } else {
    grapple_beam_origin_x_offset = kGrappleBeam_OriginX_Run[v1];
    grapple_beam_origin_y_offset = kGrappleBeam_OriginY_Run[v1] - r22;
    grapple_beam_end_x_pos = kGrappleBeam_OriginX_Run[v1] + samus_x_pos;
    x_pos_of_start_of_grapple_beam = grapple_beam_end_x_pos;
    x_pos_of_start_of_grapple_beam_prevframe = kGrappleBeam_0x0d1a_offs_Run[v1] + samus_x_pos;
    grapple_beam_end_y_pos = kGrappleBeam_OriginY_Run[v1] + samus_y_pos - r22;
    y_pos_of_start_of_grapple_beam = grapple_beam_end_y_pos;
    y_pos_of_start_of_grapple_beam_prevframe = kGrappleBeam_0x0d1c_offs_Run[v1] + samus_y_pos - r22;
  }
  grapple_beam_end_x_subpos = 0;
  grapple_beam_end_y_subpos = 0;
  grapple_beam_end_x_suboffset = 0;
  grapple_beam_end_x_offset = 0;
  grapple_beam_end_y_suboffset = 0;
  grapple_beam_end_y_offset = 0;
  grapple_beam_flags = 0;
  grapple_beam_length_delta = 12;
  grapple_beam_length = 0;
  grapple_beam_unkD26 = 0;
  grapple_beam_unkD28 = 0;
  grapple_beam_unkD2A = 0;
  grapple_beam_unkD2C = 0;
  grapple_beam_unkD2E = 0;
  grapple_beam_unkD30 = 0;
  grapple_beam_unkD1E = 0;
  grapple_beam_unkD20 = 0;
  grapple_beam_unkD3A = 2;
  grapple_beam_unkD3C = 0;
  grapple_point_anim_timer = 5;
  grapple_point_anim_ptr = g_off_9BC342;
  grapple_beam_grapple_start_x = 0;
  grapple_beam_unkD38 = 0;
  grapple_beam_unkD36 = 0;
  slow_grabble_scrolling_flag = 0;
  GrappleFunc_AF87();
  samus_draw_handler = FUNC16(sub_90EB86);
  grapple_walljump_timer = 0;
  LoadProjectilePalette(2);
  palette_buffer[223] = 32657;
  grapple_beam_function = FUNC16(GrappleBeamFunc_Firing);
  QueueSfx1_Max1(5);
  flare_counter = 1;
  play_resume_charging_beam_sfx = 0;
  if (samus_movement_handler == FUNC16(Samus_MoveHandler_ReleaseFromGrapple))
    samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
}

uint16 CheckBannedDraygonGrappleDirs(uint16 a) {  // 0x9BC6B2
  if (a == kPose_BE_FaceL_Draygon_Move) {
    if ((joypad1_lastkeys & kButton_Left) != 0) {
      if ((joypad1_lastkeys & kButton_Down) != 0)
        return 6;
      if ((joypad1_lastkeys & kButton_Up) != 0)
        return 8;
    }
    return 7;
  }
  if ((joypad1_lastkeys & kButton_Right) == 0)
    return 2;
  if ((joypad1_lastkeys & kButton_Down) != 0)
    return 3;
  if ((joypad1_lastkeys & kButton_Up) == 0)
    return 2;
  return 1;
}

static uint8 ClearCarry_12(void) {  // 0x9BC701
  return 0;
}

static void GrappleBeamFunc_Firing(void) {  // 0x9BC703
  if ((button_config_shoot_x & joypad1_lastkeys) == 0) {
    grapple_beam_function = FUNC16(GrappleBeamFunc_Cancel);
    return;
  }
  grapple_beam_length += grapple_beam_length_delta;
  if (!sign16(grapple_beam_length - 128)) {
    grapple_beam_function = FUNC16(GrappleBeamFunc_Cancel);
    return;
  }
  PairU16 pair = GrappleBeam_CollDetect_Enemy();
  int v1 = ProcessEnemyGrappleBeamColl(pair.k, pair.j);
  if (v1 >= 0) {
    if (v1) {
      grapple_beam_function = FUNC16(GrappleBeamFunc_Cancel);
      return;
    }
  } else {
    uint8 v2 = BlockCollGrappleBeam();
    if ((v2 & 1) == 0)
      return;
    if ((v2 & 0x40) == 0) {
      grapple_beam_function = FUNC16(GrappleBeamFunc_Cancel);
      return;
    }
  }
  QueueSfx1_Max6(6);
  HandleConnectingGrapple();
  grapple_beam_length_delta = -8;
  samus_grapple_flags |= 1;
}

static void GrappleBeamFunc_ConnectedLockedInPlace(void) {  // 0x9BC77E
  if ((button_config_shoot_x & joypad1_lastkeys) != 0 && (GrappleBeam_CollDetect_Enemy().k || CheckIfGrappleIsConnectedToBlock())) {
  } else {
    grapple_beam_function = FUNC16(GrappleBeamFunc_Cancel);
  }
}

static void GrappleBeamFunc_Connected_Swinging(void) {  // 0x9BC79D
  if ((button_config_shoot_x & joypad1_lastkeys) != 0) {
    GrappleBeamFunc_BB64();
    if (grapple_beam_length_delta)
      BlockFunc_AC31();
    GrappleBeamFunc_BC1F();
    GrappleBeamFunc_BCFF();
    GrappleBeamFunc_BD44();
    HandleMovementAndCollForSamusGrapple();
    if ((grapple_beam_unkD36 & 0x8000) != 0 && HandleSpecialGrappleBeamAngles() & 1) {
      return;
    }
    if (GrappleBeam_CollDetect_Enemy().k) {
      grapple_beam_flags |= 0x8000;
    } else if (!(CheckIfGrappleIsConnectedToBlock() & 1)) {
      goto LABEL_2;
    }
    BlockFunc_AC11();
    GrappleBeamFunc_BD95();
    return;
  }
LABEL_2:
  if (grapple_beam_unkD26 || grapple_beam_end_angle16 != 0x8000) {
    PropelSamusFromGrappleSwing();
    grapple_beam_function = FUNC16(GrappleBeamFunc_ReleaseFromSwing);
    samus_movement_handler = FUNC16(Samus_MoveHandler_ReleaseFromGrapple);
  } else {
    grapple_beam_function = FUNC16(GrappleBeam_Func2);
  }
}

static void GrappleBeamFunc_Wallgrab(void) {  // 0x9BC814
  if ((button_config_shoot_x & joypad1_lastkeys) != 0 && CheckIfGrappleIsConnectedToBlock() & 1) {

  } else {
    grapple_walljump_timer = 30;
    grapple_beam_function = FUNC16(GrappleBeamFunc_C832);
  }
}

static void GrappleBeamFunc_C832(void) {  // 0x9BC832
  if ((--grapple_walljump_timer & 0x8000) == 0) {
    if (Samus_GrappleWallJumpCheck(INT16_SHL16(16)))
      grapple_beam_function = FUNC16(GrappleBeamFunc_C9CE);
  } else {
    grapple_beam_function = FUNC16(GrappleBeam_Func2);
  }
}

static void GrappleBeamFunc_Cancel(void) {  // 0x9BC856
  QueueSfx1_Max15(7);
  if (samus_movement_type == kMovementType_16_Grappling)
    Samus_Pose_CancelGrapple();
  else
    CallSomeSamusCode(0x1C);
  grapple_beam_unkD1E = 0;
  grapple_beam_unkD20 = 0;
  grapple_beam_direction = 0;
  grapple_beam_unkD36 = 0;
  grapple_walljump_timer = 0;
  slow_grabble_scrolling_flag = 0;
  grapple_varCF6 = 0;
  grapple_beam_flags = 0;
  flare_counter = 0;
  flare_animation_frame = 0;
  flare_slow_sparks_anim_frame = 0;
  flare_fast_sparks_anim_frame = 0;
  flare_animation_timer = 0;
  flare_slow_sparks_anim_timer = 0;
  flare_fast_sparks_anim_timer = 0;
  LoadProjectilePalette(equipped_beams);
  grapple_beam_function = FUNC16(GrappleBeamFunc_Inactive);
  samus_draw_handler = FUNC16(SamusDrawHandler_Default);
  Samus_PostGrappleCollisionDetect();
  if (samus_auto_cancel_hud_item_index) {
    hud_item_index = 0;
    samus_auto_cancel_hud_item_index = 0;
  }
}

static void GrappleBeam_Func2(void) {  // 0x9BC8C5
  QueueSfx1_Max15(7);
  if (samus_pose == kPose_B2_FaceR_Grapple_Air)
    goto LABEL_5;
  if (samus_pose == kPose_B3_FaceL_Grapple_Air) {
LABEL_6:
    samus_new_pose_transitional = kPose_02_FaceL_Normal;
    goto LABEL_15;
  }
  if (sign16(samus_y_radius - 17)) {
    if ((kPoseParams[samus_pose].direction_shots_fired & 0xF0) != 0) {
      if (samus_pose_x_dir == 4)
        samus_new_pose_transitional = kPose_28_FaceL_Crouch;
      else
        samus_new_pose_transitional = kPose_27_FaceR_Crouch;
    } else {
      samus_new_pose_transitional = g_byte_9BC9C4[*(&kPoseParams[0].direction_shots_fired
                                                    + (8 * samus_pose))];
    }
    goto LABEL_15;
  }
  if ((kPoseParams[samus_pose].direction_shots_fired & 0xF0) != 0) {
    if (samus_pose_x_dir != 4) {
LABEL_5:
      samus_new_pose_transitional = kPose_01_FaceR_Normal;
      goto LABEL_15;
    }
    goto LABEL_6;
  }
  samus_new_pose_transitional = g_byte_9BC9BA[*(&kPoseParams[0].direction_shots_fired
                                                + (8 * samus_pose))];
LABEL_15:
  samus_hurt_switch_index = 0;
  input_to_pose_calc = 1;
  samus_x_base_speed = 0;
  samus_x_base_subspeed = 0;
  samus_y_subspeed = 0;
  samus_y_speed = 0;
  grapple_beam_unkD1E = 0;
  grapple_beam_unkD20 = 0;
  grapple_beam_direction = 0;
  grapple_beam_unkD36 = 0;
  grapple_walljump_timer = 0;
  slow_grabble_scrolling_flag = 0;
  grapple_varCF6 = 0;
  grapple_beam_flags = 0;
  flare_counter = 0;
  flare_animation_frame = 0;
  flare_slow_sparks_anim_frame = 0;
  flare_fast_sparks_anim_frame = 0;
  flare_animation_timer = 0;
  flare_slow_sparks_anim_timer = 0;
  flare_fast_sparks_anim_timer = 0;
  LoadProjectilePalette(equipped_beams);
  grapple_beam_function = FUNC16(GrappleBeamFunc_Inactive);
  samus_draw_handler = FUNC16(SamusDrawHandler_Default);
  Samus_PostGrappleCollisionDetect();
  if (samus_auto_cancel_hud_item_index) {
    hud_item_index = 0;
    samus_auto_cancel_hud_item_index = 0;
  }
}

static void GrappleBeamFunc_C9CE(void) {  // 0x9BC9CE
  QueueSfx1_Max15(7);
  if (samus_pose_x_dir == 8)
    samus_new_pose_transitional = kPose_84_FaceL_Walljump;
  else
    samus_new_pose_transitional = kPose_83_FaceR_Walljump;
  samus_hurt_switch_index = 6;
  samus_x_accel_mode = 0;
  samus_collides_with_solid_enemy = 0;
  samus_is_falling_flag = 0;
  UNUSED_word_7E0B1A = 0;
  UNUSED_word_7E0B2A = 0;
  samus_y_subspeed = 0;
  samus_y_speed = 0;
  samus_y_dir = 0;
  UNUSED_word_7E0B38 = 0;
  samus_x_base_speed = 0;
  samus_x_base_subspeed = 0;
  grapple_beam_unkD1E = 0;
  grapple_beam_unkD20 = 0;
  grapple_beam_direction = 0;
  grapple_beam_unkD36 = 0;
  grapple_walljump_timer = 0;
  slow_grabble_scrolling_flag = 0;
  grapple_varCF6 = 0;
  grapple_beam_flags = 0;
  flare_counter = 0;
  flare_animation_frame = 0;
  flare_slow_sparks_anim_frame = 0;
  flare_fast_sparks_anim_frame = 0;
  flare_animation_timer = 0;
  flare_slow_sparks_anim_timer = 0;
  flare_fast_sparks_anim_timer = 0;
  LoadProjectilePalette(equipped_beams);
  grapple_beam_function = FUNC16(GrappleBeamFunc_Inactive);
  samus_draw_handler = FUNC16(SamusDrawHandler_Default);
  Samus_PostGrappleCollisionDetect();
  if (samus_auto_cancel_hud_item_index) {
    hud_item_index = 0;
    samus_auto_cancel_hud_item_index = 0;
  }
}

void PropelSamusFromGrappleSwing(void) {  // 0x9BCA65
  int16 v1;
  int16 v2;
  uint16 v0;

  if ((grapple_beam_unkD26 & 0x8000) == 0) {
    v0 = 2 * grapple_beam_unkD26;
    v2 = kSinCosTable8bit_Sext[HIBYTE(grapple_beam_end_angle16) + 64];
    if (v2 >= 0) {
      SetHiLo(&samus_y_speed, &samus_y_subspeed, Multiply16x16(v2, v0));
      samus_y_dir = 2;
    } else {
      SetHiLo(&samus_y_speed, &samus_y_subspeed, Multiply16x16(-v2, v0));
      samus_y_dir = 1;
    }
  } else {
    v0 = -2 * grapple_beam_unkD26;
    v1 = kSinCosTable8bit_Sext[HIBYTE(grapple_beam_end_angle16) + 64];
    if (v1 < 0) {
      SetHiLo(&samus_y_speed, &samus_y_subspeed, Multiply16x16(-v1, v0));
      samus_y_dir = 2;
    } else {
      SetHiLo(&samus_y_speed, &samus_y_subspeed, Multiply16x16(v1, v0));
      samus_y_dir = 1;
    }
  }
  samus_x_accel_mode = 2;
  uint16 r18 = 3 * (v0 >> 9);
  r18 = 64 - r18;
  uint16 v3 = abs16(kSinCosTable8bit_Sext[(uint8)(grapple_beam_end_angle_hi - r18) + 64]);
  SetHiLo(&samus_x_base_speed, &samus_x_base_subspeed, Multiply16x16(v3, v0));
}

static void GrappleBeamFunc_ReleaseFromSwing(void) {  // 0x9BCB8B
  QueueSfx1_Max15(7);
  if ((grapple_beam_unkD26 & 0x8000) == 0)
    samus_new_pose_transitional = kPose_52_FaceL_Jump_NoAim_MoveF;
  else
    samus_new_pose_transitional = kPose_51_FaceR_Jump_NoAim_MoveF;
  samus_hurt_switch_index = 7;
  grapple_beam_unkD1E = 0;
  grapple_beam_unkD20 = 0;
  grapple_beam_direction = 0;
  grapple_beam_unkD36 = 0;
  grapple_walljump_timer = 0;
  slow_grabble_scrolling_flag = 0;
  grapple_varCF6 = 0;
  grapple_beam_flags = 0;
  flare_counter = 0;
  flare_animation_frame = 0;
  flare_slow_sparks_anim_frame = 0;
  flare_fast_sparks_anim_frame = 0;
  flare_animation_timer = 0;
  flare_slow_sparks_anim_timer = 0;
  flare_fast_sparks_anim_timer = 0;
  LoadProjectilePalette(equipped_beams);
  grapple_beam_function = FUNC16(GrappleBeamFunc_Inactive);
  samus_draw_handler = FUNC16(SamusDrawHandler_Default);
  Samus_PostGrappleCollisionDetect();
  if (samus_auto_cancel_hud_item_index) {
    hud_item_index = 0;
    samus_auto_cancel_hud_item_index = 0;
  }
}

static void CallGrappleBeamFunc(uint32 ea) {
  switch (ea) {
  case fnGrappleBeamFunc_BB64: GrappleBeamFunc_BB64(); return;
  case fnGrappleBeamFunc_BC1F: GrappleBeamFunc_BC1F(); return;
  case fnGrappleBeamFunc_BCFF: GrappleBeamFunc_BCFF(); return;
  case fnGrappleBeamFunc_BD44: GrappleBeamFunc_BD44(); return;
  case fnGrappleBeamFunc_BD95: GrappleBeamFunc_BD95(); return;
  case fnGrappleBeamFunc_BE98: GrappleBeamFunc_BE98(); return;
  case fnGrappleBeamFunc_BEEB: GrappleBeamFunc_BEEB(); return;
  case fnGrappleBeamFunc_BF1B: GrappleBeamFunc_BF1B(); return;
  case fnGrappleBeamFunc_Inactive: GrappleBeamFunc_Inactive(); return;
  case fnGrappleBeamFunc_FireGoToCancel: GrappleBeamFunc_FireGoToCancel(); return;
  case fnGrappleBeamFunc_Firing: GrappleBeamFunc_Firing(); return;
  case fnGrappleBeamFunc_ConnectedLockedInPlace: GrappleBeamFunc_ConnectedLockedInPlace(); return;
  case fnGrappleBeamFunc_Connected_Swinging: GrappleBeamFunc_Connected_Swinging(); return;
  case fnGrappleBeamFunc_Wallgrab: GrappleBeamFunc_Wallgrab(); return;
  case fnGrappleBeamFunc_C832: GrappleBeamFunc_C832(); return;
  case fnGrappleBeamFunc_Cancel: GrappleBeamFunc_Cancel(); return;
  case fnGrappleBeamFunc_C9CE: GrappleBeamFunc_C9CE(); return;
  case fnGrappleBeamFunc_ReleaseFromSwing: GrappleBeamFunc_ReleaseFromSwing(); return;
  case fnGrappleBeam_Func2: GrappleBeam_Func2(); return;
  default: Unreachable();
  }
}
