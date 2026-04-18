// Samus collision helpers: block/slope collision wrappers, no-collision
// position updates, wall-jump contact check, pose-change collision, and
// inside-block reactions. Extracted from sm_90.c, sm_91.c, and bank 94.

#include "block_reaction.h"
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"

// Slope-alignment lookup table (256 slope patterns * 16 x-positions each).
// Lives in ROM at bank 94; mirroring the RomFixedPtr pattern used by
// sm_a0.c / sm_86.c keeps us from copying the 512-byte table.
#define kAlignYPos_Tab0 ((uint8*)RomFixedPtr(0x948b2b))
#define kBlockCollSquareSlopeTab ((uint8*)RomFixedPtr(0x948e54))
#define kPlmHeaderDefPtrs ((uint16*)RomFixedPtr(0x949139))
#define kSpecialAirPlmLists ((uint16*)RomFixedPtr(0x9492d9))
#define kSpecialBlockPlmLists ((uint16*)RomFixedPtr(0x9492e9))
#define kBombBlockPlmList ((uint16*)RomFixedPtr(0x94936b))

int32 *cur_coll_amt32;

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
    }
    return 0;
  }

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

void CalculateBlockAt(uint16 r26, uint16 r28, uint16 r30, uint16 r32) {  // 0x949C1D
  int16 v0 = r30 + r26;
  int16 v1;
  uint16 temp_collision_DD4;

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
