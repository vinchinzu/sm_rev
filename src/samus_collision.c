// Samus collision helpers: block/slope collision wrappers, no-collision
// position updates, wall-jump contact check, and the misc "quirked" 32-bit
// compare used by speed clamping. Extracted from sm_90.c and sm_94.c.

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"

// Slope-alignment lookup table (256 slope patterns * 16 x-positions each).
// Lives in ROM at bank 94; mirroring the RomFixedPtr pattern used by
// sm_a0.c / sm_86.c keeps us from copying the 512-byte table.
#define kAlignYPos_Tab0 ((uint8*)RomFixedPtr(0x948b2b))

bool IsGreaterThanQuirked(uint16 vhi, uint16 vlo, uint16 cmphi, uint16 cmplo) {
  if ((int16)(vhi - cmphi) >= 0) {
    if (vhi != cmphi || ((int16)(vlo - cmplo) >= 0) && vlo != cmplo)
      return true;
  }
  return false;
}

static int32 Samus_ClampSpeed(int32 amt) {
  uint16 r18 = amt >> 16, r20 = amt;
  int16 v0 = r18;
  if (v0 < 0) {
    if (sign16(v0 + 15))
      r18 = -15;
  } else if (!sign16(v0 - 16)) {
    r18 = 15;
  }
  return r18 << 16 | r20;
}

int32 Samus_CalcDisplacementMoveLeft(int32 amt) {  // 0x90E464
  amt = Samus_CalcSpeed_X(amt);
  samus_collision_direction = 0;
  amt = __PAIR32__(extra_samus_x_displacement, extra_samus_x_subdisplacement) - amt;
  return Samus_ClampSpeed(amt);
}

int32 Samus_CalcDisplacementMoveRight(int32 amt) {  // 0x90E4AD
  amt = Samus_CalcSpeed_X(amt);
  samus_collision_direction = 1;
  amt += __PAIR32__(extra_samus_x_displacement, extra_samus_x_subdisplacement);
  return Samus_ClampSpeed(amt);
}

void Samus_MoveRight_NoColl(int32 amt) {  // 0x909826
  AddToHiLo(&samus_x_pos, &samus_x_subpos, amt);
  SetHiLo(&projectile_init_speed_samus_moved_right, &projectile_init_speed_samus_moved_right_fract, amt);
}

void Samus_MoveLeft_NoColl(int32 amt) {  // 0x909842
  AddToHiLo(&samus_x_pos, &samus_x_subpos, -amt);
  SetHiLo(&projectile_init_speed_samus_moved_left, &projectile_init_speed_samus_moved_left_fract, -amt);
}

void Samus_MoveDown_NoColl(int32 amt) {  // 0x909871
  AddToHiLo(&samus_y_pos, &samus_y_subpos, amt);
  SetHiLo(&projectile_init_speed_samus_moved_down, &projectile_init_speed_samus_moved_down_fract, amt);
}

void Samus_MoveUp_NoColl(int32 amt) {  // 0x90988D
  AddToHiLo(&samus_y_pos, &samus_y_subpos, -amt);
  SetHiLo(&projectile_init_speed_samus_moved_up, &projectile_init_speed_samus_moved_up_fract, -amt);
}

int32 Samus_MoveRight_NoSolidColl(int32 amt) {  // 0x94971E
  Pair_Bool_Amt pair;
  samus_collision_flag = (amt != 0 && (flag_samus_in_quicksand = 0, pair = BlockColl_Handle_Horiz(amt), amt = pair.amt, pair.flag));
  AddToHiLo(&samus_x_pos, &samus_x_subpos, amt);
  return amt;
}

int32 Samus_MoveDown_NoSolidColl(int32 amt) {  // 0x949763
  if (amt) {
    samus_pos_adjusted_by_slope_flag = 0;
    flag_samus_in_quicksand = 0;
    Pair_Bool_Amt pair = (nmi_frame_counter_word & 1) == 0 ? BlockColl_Handle_Vert_LeftToRight(amt) : BlockColl_Handle_Vert_RightToLeft(amt);
    amt = pair.amt;
    if (pair.flag) {
      samus_collision_flag = 1;
      AddToHiLo(&samus_y_pos, &samus_y_subpos, amt);
      return amt;
    }
  }
  samus_collision_flag = (flag_samus_in_quicksand != 0);
  AddToHiLo(&samus_y_pos, &samus_y_subpos, amt);
  return amt;
}

void Samus_AlignYPosSlope(void) {  // 0x9487F4
  if ((enable_horiz_slope_coll & 2) == 0)
    return;
  uint16 r26 = samus_x_pos;
  uint16 R28 = samus_y_radius + samus_y_pos - 1;
  CalculateBlockAt(r26, R28, 0, 0);
  if ((level_data[cur_block_index] & 0xF000) == 4096 && (BTS[cur_block_index] & 0x1F) >= 5) {
    uint16 temp_collision_DD4 = (samus_y_radius + samus_y_pos - 1) & 0xF;
    uint16 temp_collision_DD6 = 16 * (BTS[cur_block_index] & 0x1F);
    uint8 v0 = BTS[cur_block_index];
    if (!(v0 & 0x80)) {
      uint16 v1 = (v0 & 0x40) != 0 ? samus_x_pos ^ 0xF : samus_x_pos;
      uint16 v2 = (kAlignYPos_Tab0[temp_collision_DD6 + (v1 & 0xF)] & 0x1F) - temp_collision_DD4 - 1;
      if ((int16)v2 < 0) {
        samus_y_pos += v2;
        samus_pos_adjusted_by_slope_flag = 1;
      }
    }
  }
  r26 = samus_x_pos;
  R28 = samus_y_pos - samus_y_radius;
  CalculateBlockAt(r26, R28, 0, 0);
  if ((level_data[cur_block_index] & 0xF000) == 4096 && (BTS[cur_block_index] & 0x1F) >= 5) {
    uint16 temp_collision_DD4 = (samus_y_pos - samus_y_radius) & 0xF ^ 0xF;
    uint16 temp_collision_DD6 = 16 * (BTS[cur_block_index] & 0x1F);
    uint8 v3 = BTS[cur_block_index];
    if (v3 & 0x80) {
      uint16 v4 = (v3 & 0x40) != 0 ? samus_x_pos ^ 0xF : samus_x_pos;
      uint16 v6 = (kAlignYPos_Tab0[temp_collision_DD6 + (v4 & 0xF)] & 0x1F) - temp_collision_DD4 - 1;
      if ((int16)v6 <= 0) {
        samus_y_pos -= v6;
        samus_pos_adjusted_by_slope_flag = 1;
      }
    }
  }
}

void Samus_ClearXSpeedIfColl(void) {  // 0x90E5CE
  if (samus_collision_flag) {
    if (samus_collision_direction)
      samus_collides_with_solid_enemy = 8;
    else
      samus_collides_with_solid_enemy = 4;
    Samus_CancelSpeedBoost();
    samus_x_extra_run_speed = 0;
    samus_x_extra_run_subspeed = 0;
    samus_x_base_speed = 0;
    samus_x_base_subspeed = 0;
    samus_x_accel_mode = 0;
  } else {
    input_to_pose_calc = 0;
    samus_collides_with_solid_enemy = 0;
  }
}

// Wall-jump anim-frame gates: Samus only becomes wall-jump-eligible once
// the spin/screw-attack animation has advanced far enough. The "bump"
// frame is set while she's still bouncing off the wall pre-jump; the
// "ready" threshold is when the jump-commit check starts accepting the
// jump button.
enum {
  kWallJumpBumpFrame_Spin       = 10,
  kWallJumpBumpFrame_Screw      = 26,
  kWallJumpReadyFrame_Spin      = 11,
  kWallJumpReadyFrame_Screw     = 27,
  // Max wall penetration (in whole pixels) that still counts as
  // "in contact with the wall" for the purposes of wall-jumping.
  kWallJumpContactMaxPenetration = 8,
  // input_to_pose_calc value that tells the caller "wall-jump triggered".
  kWallJumpInputSignal           = 5,
};

static bool Samus_IsInScrewAttackPose(void) {
  return samus_pose == kPose_81_FaceR_Screwattack
      || samus_pose == kPose_82_FaceL_Screwattack;
}

static bool Samus_IsWallJumpReadyFrame(void) {
  int16 ready = Samus_IsInScrewAttackPose()
                  ? kWallJumpReadyFrame_Screw
                  : kWallJumpReadyFrame_Spin;
  return (int16)samus_anim_frame >= ready;
}

// "Bump" phase: Samus has just touched a wall and is still in the bump
// animation. If she contacts a wall (block or solid enemy) in the
// direction the player is holding, lock her into the bump anim frame so
// the ready frame fires next tick.
static uint8 Samus_WallJumpCheck_Bump(int32 amt) {
  CheckEnemyColl_Result cres;
  bool holding_left = (joypad1_lastkeys & kButton_Left) != 0;
  bool holding_right = (joypad1_lastkeys & kButton_Right) != 0;
  if (!holding_left && !holding_right)
    return 0;

  if (holding_left) {
    samus_collision_direction = 1;
    cres = Samus_CheckSolidEnemyColl(amt);
    amt = cres.amt;
    if (!cres.collision) {
      WallJumpBlockCollDetect(amt);
      if (!samus_collision_flag)
        return 0;
    }
  } else {
    samus_collision_direction = 0;
    cres = Samus_CheckSolidEnemyColl(amt);
    amt = cres.amt;
    if (!cres.collision) {
      WallJumpBlockCollDetect(-amt);
      if (!samus_collision_flag)
        return 0;
    }
  }
  samus_anim_frame_timer = 1;
  samus_anim_frame = Samus_IsInScrewAttackPose()
                       ? kWallJumpBumpFrame_Screw
                       : kWallJumpBumpFrame_Spin;
  return 0;
}

// "Commit" phase: Samus is past the bump frame and can trigger a
// wall-jump if the player presses Jump while in contact with the wall.
// Behaves slightly differently for block vs solid-enemy contact — the
// solid-enemy case also notes the enemy in enemy_index_to_shake.
static uint8 Samus_WallJumpCheck_Commit(int32 amt) {
  CheckEnemyColl_Result cres;
  bool holding_left = (joypad1_lastkeys & kButton_Left) != 0;
  bool holding_right = (joypad1_lastkeys & kButton_Right) != 0;

  enemy_index_to_shake = -1;
  if (!holding_left && !holding_right)
    return 0;

  bool hit_enemy;
  bool hit_block_with_jump;
  if (holding_left) {
    samus_collision_direction = 1;
    cres = Samus_CheckSolidEnemyColl(amt);
    amt = cres.amt;
    hit_enemy = cres.collision;
    if (!hit_enemy) {
      amt = WallJumpBlockCollDetect(amt);
      hit_block_with_jump = samus_collision_flag
                         && (button_config_jump_a & joypad1_newkeys) != 0;
    } else {
      hit_block_with_jump = false;
    }
  } else {
    samus_collision_direction = 0;
    cres = Samus_CheckSolidEnemyColl(amt);
    amt = cres.amt;
    hit_enemy = cres.collision;
    if (!hit_enemy) {
      amt = -amt;
      WallJumpBlockCollDetect(amt);
      hit_block_with_jump = samus_collision_flag
                         && (button_config_jump_a & joypad1_newkeys) != 0;
    } else {
      hit_block_with_jump = false;
    }
  }

  bool jump_pressed = (button_config_jump_a & joypad1_newkeys) != 0;
  if (hit_enemy && !jump_pressed)
    return 0;
  if (!hit_enemy && !hit_block_with_jump)
    return 0;
  if (!sign16((amt >> 16) - kWallJumpContactMaxPenetration))
    return 0;

  input_to_pose_calc = kWallJumpInputSignal;
  if (hit_enemy)
    enemy_index_to_shake = collision_detection_index;
  return 1;
}

uint8 Samus_WallJumpCheck(int32 amt) {  // 0x909D35
  // Gate: wall-jump is only reachable from the prior spin-jump or the
  // previous wall-jump movement state. Anything else (grounded, falling,
  // morphball, ...) bails immediately.
  if (samus_last_different_pose_movement_type != kMovementType_03_SpinJumping
      && samus_last_different_pose_movement_type != kMovementType_14_WallJumping)
    return 0;

  if (Samus_IsWallJumpReadyFrame())
    return Samus_WallJumpCheck_Commit(amt);
  return Samus_WallJumpCheck_Bump(amt);
}

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
