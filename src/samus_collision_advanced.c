// Samus collision advanced layer: pose-change collision reconciliation and
// inside-block reactions. Block traversal / dispatch lives in
// samus_collision_block.c.

#include "block_reaction.h"
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"

static Func_U8 *const off_91FE8A[4] = {  // 0x91FDAE
  HandleCollDueToChangedPose_Solid_NoColl,
  HandleCollDueToChangedPose_Solid_CollAbove,
  HandleCollDueToChangedPose_Solid_CollBelow,
  HandleCollDueToChangedPose_Solid_CollBoth,
};
static Func_U8 *const off_91FE92[4] = {
  HandleCollDueToChangedPose_Block_NoColl,
  HandleCollDueToChangedPose_Block_CollAbove,
  HandleCollDueToChangedPose_Block_CollBelow,
  HandleCollDueToChangedPose_Block_CollBoth,
};

void HandleCollDueToChangedPose(void) {
  CheckEnemyColl_Result cres;
  int32 amt;
  int16 v0;

  if (!samus_pose || samus_pose == kPose_9B_FaceF_VariaGravitySuit)
    return;
  solid_enemy_collision_flags = 0;
  block_collision_flags = 0;
  v0 = kPoseParams[samus_prev_pose].y_radius;
  if (!sign16(v0 - kPoseParams[samus_pose].y_radius))
    return;
  samus_y_radius = kPoseParams[samus_prev_pose].y_radius;
  samus_y_radius_diff = kPoseParams[samus_pose].y_radius - v0;
  samus_collision_direction = 2;
  cres = Samus_CheckSolidEnemyColl(INT16_SHL16(samus_y_radius_diff));
  samus_collision_flag = cres.collision, amt = cres.amt;
  if (samus_collision_flag)
    solid_enemy_collision_flags = 1;
  samus_space_to_move_up_enemy = (amt >> 16);
  samus_collision_direction = 3;
  cres = Samus_CheckSolidEnemyColl(INT16_SHL16(samus_y_radius_diff));
  samus_collision_flag = cres.collision, amt = cres.amt;
  if (samus_collision_flag)
    solid_enemy_collision_flags |= 2;
  samus_space_to_move_down_enemy = (amt >> 16);
  if (off_91FE8A[solid_enemy_collision_flags]()) {
    samus_pose = samus_prev_pose;
    return;
  }
  amt = Samus_CollDetectChangedPose(INT16_SHL16(-samus_y_radius_diff));
  if (samus_collision_flag)
    block_collision_flags = 1;
  samus_space_to_move_up_blocks = (amt >> 16);

  amt = Samus_CollDetectChangedPose(INT16_SHL16(samus_y_radius_diff));
  if (samus_collision_flag)
    block_collision_flags |= 2;
  samus_space_to_move_down_blocks = (amt >> 16);
  if (off_91FE92[block_collision_flags]()) {
    samus_pose = samus_prev_pose;
  }
}

uint8 HandleCollDueToChangedPose_Solid_NoColl(void) {  // 0x91FE9A
  return 0;
}

uint8 HandleCollDueToChangedPose_Solid_CollBoth(void) {  // 0x91FE9C
  return 0;
}

uint8 HandleCollDueToChangedPose_Solid_CollAbove(void) {  // 0x91FE9E
  uint16 v1 = samus_y_radius;
  samus_y_radius = kPoseParams[samus_pose].y_radius;
  samus_collision_direction = 3;
  CheckEnemyColl_Result cres = Samus_CheckSolidEnemyColl(INT16_SHL16(samus_y_radius_diff - samus_space_to_move_up_enemy));
  samus_y_radius = v1;
  samus_collision_flag = cres.collision;
  if (samus_collision_flag)
    return 1;
  samus_space_to_move_up_enemy = cres.amt >> 16;
  return 0;
}

uint8 HandleCollDueToChangedPose_Solid_CollBelow(void) {  // 0x91FEDF
  uint16 v1 = samus_y_radius;
  samus_y_radius = kPoseParams[samus_pose].y_radius;
  samus_collision_direction = 2;
  CheckEnemyColl_Result cres = Samus_CheckSolidEnemyColl(INT16_SHL16(samus_y_radius_diff - samus_space_to_move_down_enemy));
  samus_y_radius = v1;
  samus_collision_flag = cres.collision;
  if (samus_collision_flag)
    return 1;
  samus_space_to_move_down_enemy = cres.amt >> 16;
  return 0;
}

uint8 HandleCollDueToChangedPose_Block_CollAbove(void) {  // 0x91FF20
  int32 amt = Samus_CollDetectChangedPose(INT16_SHL16(samus_y_radius_diff - samus_space_to_move_up_blocks));
  if (samus_collision_flag)
    return 1;
  if ((solid_enemy_collision_flags & 2) != 0)
    return HandleCollDueToChangedPose_Block_CollBoth();
  samus_y_pos += amt >> 16;
  samus_prev_y_pos = samus_y_pos;
  return 0;
}

uint8 HandleCollDueToChangedPose_Block_CollBelow(void) {  // 0x91FF49
  int32 amt = Samus_CollDetectChangedPose(INT16_SHL16(samus_space_to_move_down_blocks - samus_y_radius_diff));
  if (samus_collision_flag)
    return 1;
  if ((solid_enemy_collision_flags & 1) != 0)
    return HandleCollDueToChangedPose_Block_CollBoth();
  samus_y_pos -= amt >> 16;
  samus_prev_y_pos = samus_y_pos;
  return 0;
}

uint8 HandleCollDueToChangedPose_Block_NoColl(void) {  // 0x91FF76
  switch (solid_enemy_collision_flags) {
  case 0:
    return 0;
  case 1:
    samus_y_pos += samus_space_to_move_up_enemy;
    samus_prev_y_pos = samus_y_pos;
    return 0;
  case 2:
    samus_y_pos -= samus_space_to_move_down_enemy;
    samus_prev_y_pos = samus_y_pos;
    return 0;
  case 3:
    return HandleCollDueToChangedPose_Block_CollBoth();
  default:
    Unreachable();
    return 0;
  }
}

uint8 HandleCollDueToChangedPose_Block_CollBoth(void) {  // 0x91FFA7
  if (sign16(samus_y_radius - 8))
    return 1;
  samus_pose = (samus_pose_x_dir == 4) ? kPose_28_FaceL_Crouch : kPose_27_FaceR_Crouch;
  uint16 r18 = kPoseParams[samus_pose].y_radius;
  if (sign16(samus_y_radius - r18)) {
    samus_y_pos += samus_y_radius - r18;
    samus_prev_y_pos = samus_y_pos;
  }
  return 0;
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
  0x9a06, 0x9a26, 0x9a46, 0x9a66,
  0x9a86, 0x9aa6, 0x9ac6, 0x9ae6,
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
    ci = (CollInfo){ .ci_r24 = 0, .ci_r26 = r26, .ci_r28 = r28 };
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
    ci = (CollInfo){ .ci_r24 = 0, .ci_r26 = r26, .ci_r28 = r28 };
    do {
      rv = kSamus_BlockInsideDetection[(HIBYTE(level_data[cur_block_index]) & 0xF0) >> 4](&ci);
    } while (rv & 0x80);
  }
}
