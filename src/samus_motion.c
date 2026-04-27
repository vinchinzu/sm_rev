// Samus motion primitives: horizontal/vertical movement, jump/fall/spin,
// and the entry points that wrap solid-collision checks around raw position
// updates. Dispatched to from the movement handlers in physics.c.

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "physics_config.h"
#include "samus_env.h"

// File-local named constants for magic numbers that used to be bare literals.
// Values match the original ROM semantics exactly — see comments for origin.

enum SamusVerticalDirection {
  kSamusYDir_Rising = 1,
  kSamusYDir_Falling = 2,
};

enum SamusHorizontalAccelMode {
  kSamusXAccelMode_None = 0,
  kSamusXAccelMode_Decelerating = 1,
  kSamusXAccelMode_Accelerating = 2,
};

enum SamusCollisionDirection {
  kSamusCollisionDirection_Up = 2,
  kSamusCollisionDirection_Down = 3,
};

// Sentinel used as the gravity cutoff: when samus_y_speed hits this value
// during a fall, gravity stops adding. See Samus_MoveY_WithSpeedCalc.
static const uint16 kSamusYSpeedTerminal = 5;

// Threshold (in 16.16 fixed-point) passed to Samus_WallJumpCheck from the
// spin-jump handler. Originally a globally-scoped `g_word_909E9F = 8`.
static const uint16 kWallJumpCheckThreshold = 8;

void Samus_HandleMovement_X(void) {  // 0x908E64
  Samus_HandleExtraRunspeedX();
  Samus_MoveX(Samus_CalcBaseSpeed_X(Samus_DetermineSpeedTableEntryPtr_X()));
}

void Samus_MoveX(int32 amt) {  // 0x908EA9
  // Original ASM: branch picks between the "move-left-calc" and "move-right-calc"
  // displacement paths based on accel mode and facing direction. The displacement
  // calc returns a signed amount; the final MoveLeft/MoveRight call is decided
  // purely by the sign of that result.
  bool use_left_calc;
  if (samus_x_accel_mode == kSamusXAccelMode_None
      || samus_x_accel_mode == kSamusXAccelMode_Accelerating) {
    use_left_calc = (samus_pose_x_dir == kSamusPoseXDir_FaceRight);
  } else {
    use_left_calc = (samus_pose_x_dir == kSamusPoseXDir_FaceLeft);
  }
  amt = use_left_calc ? Samus_CalcDisplacementMoveLeft(amt)
                      : Samus_CalcDisplacementMoveRight(amt);
  if (amt < 0)
    Samus_MoveLeft(amt);
  else
    Samus_MoveRight(amt);
}

void Samus_BombJumpRisingXMovement_(void) {
  int32 amt;
  bool move_right;
  if (knockback_dir) {
    amt = Samus_CalcBaseSpeed_X(Samus_DetermineSpeedTableEntryPtr_X());
    move_right = (knockback_x_dir != 0);
  } else {
    amt = Samus_CalcBaseSpeed_X(addr_stru_909F25);
    move_right = ((uint8)bomb_jump_dir != 1);
  }
  amt = move_right ? Samus_CalcDisplacementMoveRight(amt)
                   : Samus_CalcDisplacementMoveLeft(amt);
  if (amt < 0)
    Samus_MoveLeft(amt);
  else
    Samus_MoveRight(amt);
}

void Samus_BombJumpRisingYMovement_(void) {
  if (samus_y_dir != kSamusYDir_Rising)
    return;
  if ((int16)samus_y_speed < 0) {
    samus_y_subspeed = 0;
    samus_y_speed = 0;
    samus_y_dir = kSamusYDir_Falling;
    if ((uint8)bomb_jump_dir != 2)
      samus_x_accel_mode = kSamusXAccelMode_Accelerating;
  } else if ((int16)samus_y_speed < 1) {
    if (samus_input_handler != FUNC16(Samus_InputHandler_E91D))
      samus_input_handler = FUNC16(Samus_InputHandler_E913);
  }
}

void Samus_BombJumpFallingXMovement_(void) {
  int32 amt = Samus_CalcBaseSpeed_X(Samus_DetermineSpeedTableEntryPtr_X());
  if (samus_var62 == 1)
    amt = Samus_CalcDisplacementMoveLeft(amt);
  else
    amt = Samus_CalcDisplacementMoveRight(amt);
  if (amt < 0)
    Samus_MoveLeft(amt);
  else
    Samus_MoveRight(amt);
}

void Samus_BombJumpFallingYMovement_(void) {
  if ((int16)samus_y_speed < (int16)kSamusYSpeedTerminal)
    AddToHiLo(&samus_y_speed, &samus_y_subspeed, __PAIR32__(samus_y_accel, samus_y_subaccel));
  Samus_MoveDown(__PAIR32__(samus_y_speed, samus_y_subspeed));
}

void Samus_JumpingMovement(void) {  // 0x908FB3
  Samus_HandleExtraRunspeedX();
  if (samus_pose == kPose_4B_FaceR_Jumptrans
      || samus_pose == kPose_4C_FaceL_Jumptrans
      || ((int16)samus_pose >= (int16)kPose_55_FaceR_Jumptrans_AimU
          && (int16)samus_pose < (int16)kPose_5B)) {
    samus_x_accel_mode = kSamusXAccelMode_None;
    Samus_Move_NoBaseSpeed_X();
    Samus_MoveExtraY();
    return;
  }
  if (samus_y_dir == kSamusYDir_Rising
      && ((button_config_jump_a & joypad1_lastkeys) == 0
          || (int16)samus_y_speed < 0)) {
    samus_y_subspeed = 0;
    samus_y_speed = 0;
    samus_y_dir = kSamusYDir_Falling;
  }
  Pair_Bool_Amt pair = Samus_CalcBaseSpeed_NoDecel_X(Samus_DetermineSpeedTableEntryPtr_X());

  bool do_move_x;
  if (samus_movement_type == kMovementType_14_WallJumping) {
    if (!samus_x_accel_mode)
      samus_x_accel_mode = kSamusXAccelMode_Accelerating;
    do_move_x = true;
  } else {
    do_move_x = samus_x_accel_mode
             || (joypad1_lastkeys & kButton_Right) != 0
             || (joypad1_lastkeys & kButton_Left) != 0;
  }
  if (do_move_x) {
    Samus_MoveX(pair.amt);
  } else {
    samus_x_base_speed = 0;
    samus_x_base_subspeed = 0;
    samus_collision_flag = 0;
  }
  Samus_MoveY_WithSpeedCalc();
}

void Samus_SpinJumpMovement(void) {  // 0x909040
  Samus_HandleExtraRunspeedX();
  if (samus_y_dir == kSamusYDir_Rising
      && ((button_config_jump_a & joypad1_lastkeys) == 0
          || (int16)samus_y_speed < 0)) {
    samus_y_subspeed = 0;
    samus_y_speed = 0;
    samus_y_dir = kSamusYDir_Falling;
  }
  Pair_Bool_Amt pair = Samus_CalcBaseSpeed_NoDecel_X(Samus_DetermineSpeedTableEntryPtr_X());

  // "Continue moving" means: speed was capped (pair.flag), we're already
  // decelerating, or player input opposes facing direction (turn-around).
  // Otherwise freeze horizontal motion.
  bool continue_moving;
  if (pair.flag || samus_x_accel_mode == kSamusXAccelMode_Decelerating) {
    continue_moving = true;
  } else if (samus_pose_x_dir == kSamusPoseXDir_FaceRight) {
    continue_moving = ((joypad1_lastkeys & kButton_Left) != 0);
  } else {
    continue_moving = ((joypad1_lastkeys & kButton_Right) != 0);
  }
  if (continue_moving) {
    if (!samus_x_accel_mode)
      samus_x_accel_mode = kSamusXAccelMode_Accelerating;
  } else {
    pair.amt = 0;
    samus_x_base_speed = 0;
    samus_x_base_subspeed = 0;
    samus_collision_flag = 0;
    samus_x_accel_mode = kSamusXAccelMode_None;
  }
  Samus_MoveX(pair.amt);
  if (!Samus_WallJumpCheck(INT16_SHL16(kWallJumpCheckThreshold)))
    Samus_MoveY_WithSpeedCalc();
}

void Samus_CheckStartFalling(void) {  // 0x9090C4
  if (samus_y_dir == kSamusYDir_Rising && (int16)samus_y_speed < 0) {
    samus_y_subspeed = 0;
    samus_y_speed = 0;
    samus_y_dir = kSamusYDir_Falling;
  }
}

void Samus_MoveY_WithSpeedCalc(void) {  // 0x9090E2
  int32 amt = __PAIR32__(samus_y_speed, samus_y_subspeed);
  if (samus_y_dir == kSamusYDir_Falling) {
    if (samus_y_speed != kSamusYSpeedTerminal)
      AddToHiLo(&samus_y_speed, &samus_y_subspeed, __PAIR32__(samus_y_accel, samus_y_subaccel));
  } else {
    AddToHiLo(&samus_y_speed, &samus_y_subspeed, -IPAIR32(samus_y_accel, samus_y_subaccel));
  }
  if (samus_y_dir != kSamusYDir_Falling)
    amt = -amt;
  amt += __PAIR32__(extra_samus_y_displacement, extra_samus_y_subdisplacement);
  if (amt < 0)
    Samus_MoveUp(amt);
  else
    Samus_MoveDown(amt);
}

void Samus_FallingMovement(void) {  // 0x909168
  Samus_HandleExtraRunspeedX();
  Pair_Bool_Amt pair = Samus_CalcBaseSpeed_NoDecel_X(Samus_DetermineSpeedTableEntryPtr_X());
  if (samus_x_accel_mode
      || (joypad1_lastkeys & kButton_Right) != 0
      || (joypad1_lastkeys & kButton_Left) != 0) {
    Samus_MoveX(pair.amt);
  } else {
    samus_x_base_speed = 0;
    samus_x_base_subspeed = 0;
    samus_collision_flag = 0;
  }
  Samus_CheckStartFalling();
  Samus_MoveY_WithSpeedCalc();
}

void Samus_MorphedFallingMovement(void) {  // 0x90919F
  Pair_Bool_Amt pair = Samus_CalcBaseSpeed_NoDecel_X(Samus_DetermineSpeedTableEntryPtr_X());
  if (!samus_x_accel_mode
      && (joypad1_lastkeys & kButton_Right) == 0
      && (joypad1_lastkeys & kButton_Left) == 0) {
    pair.amt = 0;
    samus_x_base_speed = 0;
    samus_x_base_subspeed = 0;
    samus_collision_flag = 0;
  }
  Samus_MoveX(pair.amt);
  Samus_CheckStartFalling();
  Samus_MoveY_WithSpeedCalc();
}

void Samus_MorphedBouncingMovement(void) {  // 0x9091D1
  Pair_Bool_Amt pair = Samus_CalcBaseSpeed_NoDecel_X(Samus_DetermineSpeedTableEntryPtr_X());
  if (!samus_x_accel_mode
      && (joypad1_lastkeys & kButton_Right) == 0
      && (joypad1_lastkeys & kButton_Left) == 0) {
    pair.amt = 0;
    samus_x_base_speed = 0;
    samus_x_base_subspeed = 0;
    samus_collision_flag = 0;
  }
  Samus_MoveX(pair.amt);
  if (knockback_dir)
    return;
  if (extra_samus_y_displacement || extra_samus_y_subdisplacement) {
    samus_y_dir = kSamusYDir_Falling;
    samus_y_subspeed = 0;
    samus_y_speed = 0;
    int32 amt = __PAIR32__(extra_samus_y_displacement, extra_samus_y_subdisplacement);
    if (amt < 0)
      Samus_MoveUp(amt);
    else
      Samus_MoveDown(amt + INT16_SHL16(1));
  } else {
    Samus_CheckStartFalling();
    Samus_MoveY_WithSpeedCalc();
  }
}

void Samus_Move_NoSpeedCalc_Y(void) {  // 0x90923F
  int32 amt = __PAIR32__(extra_samus_y_displacement, extra_samus_y_subdisplacement);
  if (amt == 0) {
    if (!samus_pos_adjusted_by_slope_flag)
      amt = __PAIR32__(samus_total_x_speed, samus_total_x_subspeed);
    Samus_MoveDown(amt + INT16_SHL16(1));
  } else if (amt < 0) {
    Samus_MoveUp(amt);
  } else {
    Samus_MoveDown(amt + INT16_SHL16(1));
  }
}

void Samus_MoveExtraY(void) {  // 0x909288
  int32 amt = __PAIR32__(extra_samus_y_displacement, extra_samus_y_subdisplacement);
  if (amt == 0)
    return;
  if (amt < 0)
    Samus_MoveUp(amt);
  else
    Samus_MoveDown(amt + INT16_SHL16(1));
}

uint8 Samus_CheckAndMoveY(void) {  // 0x9092B8
  if (!samus_y_dir)
    return 0;
  Samus_CheckStartFalling();
  Samus_MoveY_WithSpeedCalc();
  return 1;
}

uint8 Samus_MoveY_Simple_(void) {  // 0x9092C7
  if (!samus_y_dir)
    return 0;
  Samus_CheckStartFalling();
  Samus_MoveY_WithSpeedCalc();
  return 1;
}

void Samus_Move_NoBaseSpeed_X(void) {  // 0x909348
  Samus_MoveX(0);
}

void Samus_MoveLeft(int32 amt) {  // 0x909350
  amt = -amt;
  CheckEnemyColl_Result cres = Samus_CheckSolidEnemyColl(amt);
  samus_collision_flag = cres.collision, amt = cres.amt;
  if (samus_collision_flag) {
    Samus_ClearXSpeedIfColl();
    Samus_MoveLeft_NoColl(amt);
    Samus_AlignYPosSlope();
  } else {
    amt = Samus_MoveRight_NoSolidColl(-amt);
    SetHiLo(&projectile_init_speed_samus_moved_left, &projectile_init_speed_samus_moved_left_fract, amt);
    if ((samus_collision_direction & 1) != 0)
      samus_collision_flag = 0;
    Samus_ClearXSpeedIfColl();
    Samus_AlignYPosSlope();
  }
}

void Samus_MoveRight(int32 amt) {  // 0x9093B1
  CheckEnemyColl_Result cres = Samus_CheckSolidEnemyColl(amt);
  samus_collision_flag = cres.collision, amt = cres.amt;
  if (samus_collision_flag) {
    Samus_ClearXSpeedIfColl();
    Samus_MoveRight_NoColl(amt);
    Samus_AlignYPosSlope();
  } else {
    amt = Samus_MoveRight_NoSolidColl(amt);
    SetHiLo(&projectile_init_speed_samus_moved_right, &projectile_init_speed_samus_moved_right_fract, amt);
    if ((samus_collision_direction & 1) == 0)
      samus_collision_flag = 0;
    Samus_ClearXSpeedIfColl();
    Samus_AlignYPosSlope();
  }
}

void Samus_MoveUp(int32 amt) {  // 0x9093EC
  samus_collision_direction = kSamusCollisionDirection_Up;
  CheckEnemyColl_Result cres = Samus_CheckSolidEnemyColl(-amt);
  samus_collision_flag = cres.collision, amt = cres.amt;
  if (samus_collision_flag) {
    Samus_MoveUp_SetPoseCalcInput();
    Samus_MoveUp_NoColl(amt);
  } else {
    amt = Samus_MoveDown_NoSolidColl(-amt);
    SetHiLo(&projectile_init_speed_samus_moved_up, &projectile_init_speed_samus_moved_up_fract, amt);
    Samus_MoveUp_SetPoseCalcInput();
  }
}

void Samus_MoveDown(int32 amt) {  // 0x909440
  samus_collision_direction = kSamusCollisionDirection_Down;
  CheckEnemyColl_Result cres = Samus_CheckSolidEnemyColl(amt);
  samus_collision_flag = cres.collision, amt = cres.amt;
  if (samus_collision_flag) {
    Samus_MoveDown_SetPoseCalcInput();
    Samus_MoveDown_NoColl(amt);
  } else {
    amt = Samus_MoveDown_NoSolidColl(amt);
    projectile_init_speed_samus_moved_down_fract = amt;
    projectile_init_speed_samus_moved_down = amt >> 16;
    Samus_MoveDown_SetPoseCalcInput();
  }
}

void Samus_MoveHandler_ReleaseFromGrapple(void) {  // 0x90946E
  if (samus_y_dir == kSamusYDir_Rising && (int16)samus_y_speed < 0) {
    samus_y_subspeed = 0;
    samus_y_speed = 0;
    samus_y_dir = kSamusYDir_Falling;
    samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
  }
  samus_x_accel_mode = kSamusXAccelMode_Accelerating;
  int32 amt = Samus_CalcBaseSpeed_X(Samus_DetermineGrappleSwingSpeed_X());
  if (samus_x_accel_mode
      || (joypad1_lastkeys & kButton_Right) != 0
      || (joypad1_lastkeys & kButton_Left) != 0) {
    Samus_MoveX(amt);
  } else {
    samus_x_base_speed = 0;
    samus_x_base_subspeed = 0;
    samus_collision_flag = 0;
  }
  Samus_MoveY_WithSpeedCalc();
  if (input_to_pose_calc)
    samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
}

void Samus_HandleMovement_DrainedCrouching(void) {  // 0x9094CB
  Samus_MoveY_WithSpeedCalc();
  if (samus_collision_flag) {
    samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
    samus_anim_frame_timer = 8;
    samus_anim_frame = 7;
    samus_y_subspeed = 0;
    samus_y_speed = 0;
  }
}
