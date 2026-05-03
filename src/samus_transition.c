// Samus pose-transition state machine: the three-tier pipeline that commits
// samus_new_pose_transitional → samus_new_pose_interrupted → samus_new_pose,
// dispatching through the A/B/C tables for momentum handoff, hurt-switch
// behaviour, and post-commit fix-ups. Extracted from sm_91.c.
//
// Also includes the block-collision transition dispatcher, the
// "walked-into-something" solid-enemy recheck, morph-ball bounce and
// spring-ball bounce logic, and the per-area landing sound/gfx handlers.

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "samus_env.h"

#define word_91E921 ((uint16*)RomFixedPtr(0x91e921))
#define word_91E9F3 ((uint16*)RomFixedPtr(0x91e9f3))
#define word_91EB74 ((uint16*)RomFixedPtr(0x91eb74))

enum {
  kCrouchingEtcTransitionPoseBase = kPose_35_FaceR_CrouchTrans,
  kPoseCalcInput_ResetBallBounceLanding = 0x0401,
  kBlockCollisionSubtype_Ignore = 4,
  kBlockCollisionPosePairStride = 2,
  kBlockCollisionFacingRightPoseOffset = 1,
  kSamusMomentumRoutine_None = 0,
  kSamusMomentumRoutine_BlockCollision = 5,
  kSamusNoPendingPose = 0xFFFF,
  kSamusPendingPoseInvalidMask = 0x8000,
  kSamusHurtSwitch_SkipTransitionalPoseCommit = 1,
  kSamusHurtSwitch_Grapple = 3,
  kSamusSpecialTransGfx_Grapple = 9,
  kPoseDirection_None = 0xff,
  kPoseDirection_LandingFireA = 2,
  kPoseDirection_LandingFireB = 7,
  kBallLandingSpeedThreshold = 3,
  kBallLandingBounce_KeepCurrentPose = 2,
  kBallLandingBounce_SelectGroundPose = 4,
  kBallBounceState_NoSpring_First = 1,
  kSpringBallBounceState_First = 0x0601,
  kSpringBallBounceState_Second = 0x0602,
  kBallBounceYDir_Rising = 1,
  kBallBounceFirstYSpeed = 1,
  kBallBounceSecondYSpeed = 0,
  kBallBounceYSubspeed = 0,
  kPose_20_BlockCollisionMorphLanding = 0x20,
  kPose_42_BlockCollisionMorphLanding = 0x42,
  kKnockbackDir_NoHeldInput = 1,
  kKnockbackDir_XMomentum = 2,
  kKnockbackDir_HeldOpposite = 4,
  kKnockbackDir_XMomentumHeldOpposite = 5,
  kXrayAngle_FacingLeft = 64,
  kXrayAngle_FacingRight = 192,
  kXrayTransitionAnimFrame = 2,
  kXrayTransitionAnimTimer = 63,
  kXrayTransitionShineTimerDelay = 8,
  kSfx1_XrayOpen = 9,
  kGrappleTransitionMaxPrevDelta = 12,
  kGrappleTransitionPositivePrevDeltaThreshold = 13,
  kTransitionA7PoseYOffsetBase = kPose_35_FaceR_CrouchTrans,
  kTransitionA7AimUpYOffset = 5,
  kCrateriaLandingGfx_FxTypeGate = 0x01,
  kCrateriaLandingGfx_YThresholdGate = 0x02,
  kCrateriaLandingGfx_Always = 0x04,
  kCrateriaLandingGfxRuleCount = 16,
  kCrateriaLandingGfxNorfairRoomIndex = 28,
  kCrateriaLandingGfxRequiredFxType = 10,
  kCrateriaLandingGfxYThreshold = 944,
};

void nullsub_18(void) {
}

static Func_V *const kSamus_HandleTransFromBlockColl[6] = {  // 0x91E8B6
  nullsub_18,
  Samus_HandleTransFromBlockColl_1,
  Samus_HandleTransFromBlockColl_2,
  Samus_HandleTransFromBlockColl_3,
  Samus_HandleTransFromBlockColl_4,
  Samus_HandleTransFromBlockColl_5,
};

void Samus_HandleTransFromBlockColl(void) {
  if (input_to_pose_calc)
    kSamus_HandleTransFromBlockColl[(uint8)input_to_pose_calc]();
}
void Samus_HandleTransFromBlockColl_3(void) {  // 0x91E8D8
  samus_new_pose = samus_pose;
  samus_momentum_routine_index = kSamusMomentumRoutine_BlockCollision;
}

void Samus_HandleTransFromBlockColl_4(void) {  // 0x91E8E5
  samus_new_pose = samus_pose;
  samus_momentum_routine_index = kSamusMomentumRoutine_BlockCollision;
}

uint8 nullsub_18_U8(void) {
  return 0;
}

static Func_U8 *const off_91E951[6] = {  // 0x91E8F2
  Samus_HandleTransFromBlockColl_1_0,
  Samus_HandleTransFromBlockColl_1_1,
  Samus_HandleTransFromBlockColl_1_2,
  Samus_HandleTransFromBlockColl_1_3,
  nullsub_18_U8,
  Samus_HandleTransFromBlockColl_1_5,
};

static void SamusCrouchingEtcFunc_Noop(void) {
}

static Func_V *const kSamusCrouchingEtcFuncs[12] = {
  SamusCrouchingEtcFunc_Noop,
  SamusCrouchingEtcFunc_Noop,
  SamusCrouchingEtcFunc_Noop,
  SamusCrouchingEtcFunc_Noop,
  SamusCrouchingEtcFunc,
  SamusCrouchingEtcFunc,
  SamusCrouchingEtcFunc_Noop,
  SamusCrouchingEtcFunc_Noop,
  SamusCrouchingEtcFunc_Noop,
  SamusCrouchingEtcFunc_Noop,
  SamusCrouchingEtcFunc,
  SamusCrouchingEtcFunc,
};

static bool Samus_CrouchingEtcTransitionSkipsMovement(void) {
  return !sign16(samus_pose - kPose_DB) &&
         sign16(samus_pose - kPose_F1_FaceR_CrouchTrans_AimU);
}

static bool Samus_CrouchingEtcTransitionUsesPoseTable(void) {
  return sign16(samus_pose - kPose_DB);
}

static void Samus_ApplyCrouchingEtcPoseSetup(void) {
  if (Samus_CrouchingEtcTransitionUsesPoseTable())
    kSamusCrouchingEtcFuncs[samus_pose - kCrouchingEtcTransitionPoseBase]();
}

static void Samus_ClearBallBounceLanding(void) {
  samus_y_subspeed = 0;
  samus_y_speed = 0;
  samus_y_dir = 0;
  used_for_ball_bounce_on_landing = 0;
}

static uint8 Samus_BlockCollisionSubtype(void) {
  return HIBYTE(input_to_pose_calc);
}

static bool Samus_BlockCollisionShouldIgnoreSubtype(void) {
  return Samus_BlockCollisionSubtype() == kBlockCollisionSubtype_Ignore;
}

static bool Samus_IsFacingRight(void) {
  return samus_pose_x_dir == kSamusPoseXDir_FaceRight;
}

static bool Samus_IsHoldingOppositeFacingInput(void) {
  if (Samus_IsFacingRight())
    return (joypad1_lastkeys & kButton_Left) != 0;
  return (joypad1_lastkeys & kButton_Right) != 0;
}

static uint16 Samus_KnockbackDirForDamageBoost(void) {
  bool holding_opposite_input = Samus_IsHoldingOppositeFacingInput();
  if (knockback_x_dir)
    return holding_opposite_input ? kKnockbackDir_XMomentumHeldOpposite
                                  : kKnockbackDir_XMomentum;
  return holding_opposite_input ? kKnockbackDir_HeldOpposite
                                : kKnockbackDir_NoHeldInput;
}

static uint16 Samus_DefaultKnockbackDirForFacing(void) {
  return Samus_IsFacingRight() ? kKnockbackDir_NoHeldInput
                               : kKnockbackDir_XMomentum;
}

static uint16 Samus_XrayAngleForFacing(void) {
  return Samus_IsFacingRight() ? kXrayAngle_FacingRight
                               : kXrayAngle_FacingLeft;
}

static bool Samus_CanStartGroundXrayTransition(void) {
  switch (samus_movement_type) {
  case kMovementType_00_Standing:
  case kMovementType_01_Running:
  case kMovementType_05_Crouching:
  case kMovementType_15_RanIntoWall:
    return true;
  default:
    return false;
  }
}

static void Samus_StartGroundXrayTransition(void) {
  xray_angle = Samus_XrayAngleForFacing();
  samus_anim_frame = kXrayTransitionAnimFrame;
  samus_anim_frame_timer = kXrayTransitionAnimTimer;
  samus_movement_handler = FUNC16(SamusMovementType_Xray);
  samus_input_handler = FUNC16(Samus_Func20_);
  timer_for_shine_timer = kXrayTransitionShineTimerDelay;
  special_samus_palette_timer = 1;
  special_samus_palette_frame = 0;
  samus_shine_timer = 0;
  flare_counter = 0;
  flare_animation_frame = 0;
  flare_slow_sparks_anim_frame = 0;
  flare_fast_sparks_anim_frame = 0;
  flare_animation_timer = 0;
  flare_slow_sparks_anim_timer = 0;
  flare_fast_sparks_anim_timer = 0;
  QueueSfx1_Max6(kSfx1_XrayOpen);
}

static void Samus_ClampGrappleTransitionPreviousCoord(uint16 position,
                                                      uint16 *previous_position) {
  int16 delta = position - *previous_position;
  if (delta < 0) {
    if (delta < -kGrappleTransitionMaxPrevDelta)
      *previous_position = position + kGrappleTransitionMaxPrevDelta;
  } else if (delta >= kGrappleTransitionPositivePrevDeltaThreshold) {
    *previous_position = position - kGrappleTransitionMaxPrevDelta;
  }
}

static void Samus_ClampGrappleTransitionPreviousPosition(void) {
  Samus_ClampGrappleTransitionPreviousCoord(samus_x_pos, &samus_prev_x_pos);
  Samus_ClampGrappleTransitionPreviousCoord(samus_y_pos, &samus_prev_y_pos);
}

static void Samus_ResetGrappleTransitionMotion(void) {
  Samus_CancelSpeedBoost();
  samus_x_base_speed = 0;
  samus_x_base_subspeed = 0;
  samus_y_subspeed = 0;
  samus_y_speed = 0;
  SetHiLo(&samus_x_extra_run_speed, &samus_x_extra_run_subspeed, 0);
}

static SamusPose Samus_SelectCurrentFacingPose(SamusPose pose_when_facing_right,
                                               SamusPose pose_when_facing_left) {
  return Samus_IsFacingRight() ? pose_when_facing_right : pose_when_facing_left;
}

static void Samus_SetBlockCollisionMomentum(void) {
  samus_momentum_routine_index = kSamusMomentumRoutine_BlockCollision;
}

static bool Samus_DirectionUsesLandingFirePose(uint16 direction) {
  return (direction == kPoseDirection_LandingFireA ||
          direction == kPoseDirection_LandingFireB) &&
         (button_config_shoot_x & joypad1_lastkeys) != 0;
}

static bool Samus_BallLandingKeepsCurrentPose(int16 doubled_bounce_state,
                                              bool has_bounce_state) {
  if (has_bounce_state) {
    if (doubled_bounce_state == kBallLandingBounce_KeepCurrentPose)
      return true;
    if (doubled_bounce_state != kBallLandingBounce_SelectGroundPose)
      Unreachable();
  } else if (!sign16(samus_y_speed - kBallLandingSpeedThreshold)) {
    return true;
  }
  return false;
}

static bool Samus_BallLandingCanBounce(void) {
  return !sign16(samus_y_speed - kBallLandingSpeedThreshold);
}

static void Samus_StartBallLandingBounce(uint16 bounce_state, uint16 y_speed) {
  used_for_ball_bounce_on_landing = bounce_state;
  samus_y_dir = kBallBounceYDir_Rising;
  samus_y_subspeed = kBallBounceYSubspeed;
  samus_y_speed = y_speed;
}

void Samus_HandleTransFromBlockColl_2(void) {

  if (!Samus_BlockCollisionShouldIgnoreSubtype()) {
    uint16 pose_index = kBlockCollisionPosePairStride * Samus_BlockCollisionSubtype();
    if (Samus_IsFacingRight())
      pose_index += kBlockCollisionFacingRightPoseOffset;
    samus_new_pose = word_91E921[pose_index];
    Samus_SetBlockCollisionMomentum();
  }
}

void Samus_HandleTransFromBlockColl_1(void) {  // 0x91E931
  if (!Samus_BlockCollisionShouldIgnoreSubtype()) {
    if (off_91E951[Samus_BlockCollisionSubtype()]() & 1)
      samus_momentum_routine_index = kSamusMomentumRoutine_None;
    else
      Samus_SetBlockCollisionMomentum();
  }
}

uint8 Samus_HandleTransFromBlockColl_1_0(void) {  // 0x91E95D
  if (samus_prev_movement_type2 == kMovementType_03_SpinJumping
      || samus_prev_movement_type2 == kMovementType_14_WallJumping) {
    samus_new_pose = Samus_SelectCurrentFacingPose(kPose_A7_FaceL_LandSpinJump,
                                                   kPose_A6_FaceR_LandSpinJump);
    return 0;
  }

  uint16 direction = kPoseParams[samus_pose].direction_shots_fired;
  if (direction == kPoseDirection_None) {
    samus_new_pose = Samus_SelectCurrentFacingPose(kPose_A5_FaceL_LandJump,
                                                   kPose_A4_FaceR_LandJump);
    return 0;
  }

  if (Samus_DirectionUsesLandingFirePose(direction)) {
    samus_new_pose = Samus_SelectCurrentFacingPose(kPose_E7_FaceL_LandJump_Fire,
                                                   kPose_E6_FaceR_LandJump_Fire);
    return 0;
  }

  samus_new_pose = word_91E9F3[direction];
  return 0;
}

uint8 Samus_HandleTransFromBlockColl_1_1(void) {  // 0x91EA07
  int16 doubled_bounce_state = 2 * used_for_ball_bounce_on_landing;
  if (Samus_BallLandingKeepsCurrentPose(doubled_bounce_state,
                                        2 * used_for_ball_bounce_on_landing != 0)) {
    samus_new_pose = samus_pose;
    return 0;
  }
  samus_new_pose = Samus_SelectCurrentFacingPose(kPose_41_FaceL_Morphball_Ground,
                                                 kPose_1D_FaceR_Morphball_Ground);
  return 0;
}

uint8 Samus_HandleTransFromBlockColl_1_2(void) {  // 0x91EA48
  samus_new_pose = Samus_SelectCurrentFacingPose(kPose_42_BlockCollisionMorphLanding,
                                                 kPose_20_BlockCollisionMorphLanding);
  return 0;
}

uint8 Samus_HandleTransFromBlockColl_1_3(void) {  // 0x91EA63
  if ((button_config_jump_a & joypad1_lastkeys) != 0) {
    samus_new_pose = samus_pose;
    return 0;
  }
  int16 doubled_bounce_state = 2 * (uint8)used_for_ball_bounce_on_landing;
  if (Samus_BallLandingKeepsCurrentPose(doubled_bounce_state,
                                        doubled_bounce_state != 0)) {
    samus_new_pose = samus_pose;
    return 0;
  }
  samus_new_pose = Samus_SelectCurrentFacingPose(kPose_7A_FaceL_Springball_Ground,
                                                 kPose_79_FaceR_Springball_Ground);
  return 0;
}

uint8 Samus_HandleTransFromBlockColl_1_5(void) {  // 0x91EAB6
  samus_new_pose = samus_pose;
  return 0;
}

void Samus_HandleTransFromBlockColl_5(void) {  // 0x91EABE
  samus_new_pose = Samus_SelectCurrentFacingPose(kPose_84_FaceL_Walljump,
                                                 kPose_83_FaceR_Walljump);
  Samus_SetBlockCollisionMomentum();
}

void Samus_Movement_0F_CrouchingEtcTransition(void) {
  if (!Samus_CrouchingEtcTransitionSkipsMovement()) {
    Samus_ApplyCrouchingEtcPoseSetup();
    Samus_Move_NoBaseSpeed_X();
    if (!(Samus_MoveY_Simple_() & 1))
      Samus_Move_NoSpeedCalc_Y();
  }
  if (input_to_pose_calc == kPoseCalcInput_ResetBallBounceLanding)
    Samus_ClearBallBounceLanding();
  input_to_pose_calc = 0;
}

void SamusCrouchingEtcFunc(void) {
  enable_horiz_slope_coll = 3;
  UNUSEDword_7E0AA4 = 0;
}

static void Samus_ClearWalkedIntoSomethingCheck(void) {
  samus_collides_with_solid_enemy = 0;
}

static void Samus_SelectWalkedIntoSomethingPose(SamusPose pose) {
  samus_new_pose = word_91EB74[kPoseParams[pose].direction_shots_fired];
}

static bool Samus_WalkedIntoSomethingHitsSolidEnemy(uint16 collision_direction) {
  samus_collision_direction = collision_direction;
  CheckEnemyColl_Result cres = Samus_CheckSolidEnemyColl(INT16_SHL16(1));
  return cres.collision;
}

static bool Samus_WalkedIntoSomethingMoveHitsSolid(uint16 collision_direction,
                                                   int32 move_amt) {
  samus_collision_direction = collision_direction;
  Samus_MoveRight_NoSolidColl(move_amt);
  return samus_collision_flag;
}

static bool Samus_WalkedIntoSomethingCollisionDetected(void) {
  uint16 collision_direction = Samus_IsFacingRight() ? 0 : 1;
  if (Samus_WalkedIntoSomethingHitsSolidEnemy(collision_direction))
    return true;

  int32 move_amt = Samus_IsFacingRight() ? INT16_SHL16(-1) : INT16_SHL16(1);
  return Samus_WalkedIntoSomethingMoveHitsSolid(collision_direction, move_amt);
}

void Samus_CheckWalkedIntoSomething(void) {  // 0x91EADE
  if (samus_collides_with_solid_enemy &&
      samus_movement_type == kMovementType_01_Running) {
    Samus_SelectWalkedIntoSomethingPose(samus_pose);
    Samus_ClearWalkedIntoSomethingCheck();
    return;
  }
  if (samus_new_pose == kSamusNoPendingPose ||
      kPoseParams[samus_new_pose].movement_type != kMovementType_01_Running) {
    Samus_ClearWalkedIntoSomethingCheck();
    return;
  }

  if (Samus_WalkedIntoSomethingCollisionDetected())
    Samus_SelectWalkedIntoSomethingPose(samus_new_pose);
  Samus_ClearWalkedIntoSomethingCheck();
}

void nullsub_19(void) {}
void nullsub_20(void) {}
void nullsub_21(void) {}
void nullsub_22(void) {}
static Func_V *const kSamus_HandleTransitionsA[9] = {  // 0x91EB88
  nullsub_18,
  Samus_HandleTransitionsA_1,
  Samus_HandleTransitionsA_2,
  nullsub_21,
  nullsub_22,
  Samus_HandleTransitionsA_5,
  Samus_HandleTransitionsA_6,
  Samus_HandleTransitionsA_7,
  Samus_HandleTransitionsA_8,
};
static Func_V *const kSamus_HandleTransitionsB[11] = {
  nullsub_18,
  Samus_HandleTransitionsB_1,
  Samus_HandleTransitionsB_2,
  Samus_HandleTransitionsB_3,
  Samus_HandleTransitionsB_4,
  Samus_HandleTransitionsB_5,
  nullsub_19,
  nullsub_20,
  Samus_HandleTransitionsB_8,
  Samus_HandleTransitionsB_9,
  Samus_HandleTransitionsB_10,
};
static Func_V *const kSamus_HandleTransitionsC[9] = {
  nullsub_18,
  Samus_HandleTransitionsC_1,
  Samus_HandleTransitionsC_2,
  Samus_HandleTransitionsC_3,
  Samus_HandleTransitionsC_4,
  Samus_HandleTransitionsC_5,
  Samus_HandleTransitionsC_6,
  Samus_HandleTransitionsC_7,
  Samus_HandleTransitionsC_8,
};

static bool Samus_HasPendingPose(uint16 pose) {
  return (pose & kSamusPendingPoseInvalidMask) == 0;
}

static void Samus_RecordCommittedPoseHistory(void) {
  samus_last_different_pose = samus_prev_pose;
  *(uint16 *)&samus_last_different_pose_x_dir = *(uint16 *)&samus_prev_pose_x_dir;
  samus_prev_pose = samus_pose;
  *(uint16 *)&samus_prev_pose_x_dir = *(uint16 *)&samus_pose_x_dir;
}

static bool Samus_TransitionalPoseDefersToLaterPendingPose(void) {
  return samus_hurt_switch_index == kSamusHurtSwitch_Grapple &&
         samus_special_transgfx_index == kSamusSpecialTransGfx_Grapple;
}

static void Samus_CommitTransitionalPose(void) {
  samus_pose = samus_new_pose_transitional;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
}

static bool Samus_TryHandleTransitionalPose(void) {
  if (!Samus_HasPendingPose(samus_new_pose_transitional) ||
      Samus_TransitionalPoseDefersToLaterPendingPose()) {
    return false;
  }

  if (samus_hurt_switch_index != kSamusHurtSwitch_SkipTransitionalPoseCommit)
    Samus_CommitTransitionalPose();
  kSamus_HandleTransitionsC[samus_hurt_switch_index]();
  Samus_RecordCommittedPoseHistory();
  return true;
}

static bool Samus_TryHandleInterruptedPose(void) {
  if (!Samus_HasPendingPose(samus_new_pose_interrupted))
    return false;

  samus_pose = samus_new_pose_interrupted;
  if (!(SamusFunc_F404() & 1))
    kSamus_HandleTransitionsB[samus_special_transgfx_index]();
  Samus_RecordCommittedPoseHistory();
  return true;
}

static bool Samus_TryHandleNormalPendingPose(void) {
  Samus_CheckWalkedIntoSomething();
  if (!Samus_HasPendingPose(samus_new_pose))
    return false;

  samus_pose = samus_new_pose;
  if (!(SamusFunc_F404() & 1))
    kSamus_HandleTransitionsA[samus_momentum_routine_index]();
  Samus_RecordCommittedPoseHistory();
  return true;
}

void Samus_HandleTransitions(void) {
  if (!Samus_TryHandleTransitionalPose() && !Samus_TryHandleInterruptedPose())
    Samus_TryHandleNormalPendingPose();
  input_to_pose_calc = 0;
}

void Samus_HandleTransitionsA_1(void) {  // 0x91EC50
  if (samus_x_base_speed || samus_x_base_subspeed) {
    AddToHiLo(&samus_x_base_speed, &samus_x_base_subspeed, __PAIR32__(samus_x_extra_run_speed, samus_x_extra_run_subspeed));
    samus_x_accel_mode = kSamusXAccelMode_Accelerating;
    Samus_CancelSpeedBoost();
    SetHiLo(&samus_x_extra_run_speed, &samus_x_extra_run_subspeed, 0);
    SamusFunc_EC80();
  } else {
    Samus_HandleTransitionsA_2();
  }
}

void Samus_HandleTransitionsA_6(void) {  // 0x91EC85
  samus_x_accel_mode = kSamusXAccelMode_None;
  samus_x_base_speed = 0;
  samus_x_base_subspeed = 0;
  Samus_HandleTransitionsA_8();
}

void Samus_HandleTransitionsA_8(void) {  // 0x91EC8E
  Samus_CancelSpeedBoost();
  SetHiLo(&samus_x_extra_run_speed, &samus_x_extra_run_subspeed, 0);
  SamusFunc_EC80();
}

void Samus_HandleTransitionsA_2(void) {  // 0x91ECD0
  samus_x_accel_mode = kSamusXAccelMode_None;
  Samus_CancelSpeedBoost();
}

static const uint16 kTransitionA7PoseYOffsets[12] = {  // 0x91ECDA
  5, 5, 9, 9, 0, 0, 0, 0,
  0, 0, 0, 0,
};

static bool Samus_GetTransitionA7PoseYOffset(int16 *y_offset) {
  if (sign16(samus_pose - kPose_DB)) {
    *y_offset = kTransitionA7PoseYOffsets[samus_pose - kTransitionA7PoseYOffsetBase];
    return true;
  }
  if (!sign16(samus_pose - kPose_F1_FaceR_CrouchTrans_AimU) &&
      sign16(samus_pose - kPose_F7_FaceR_StandTrans_AimU)) {
    *y_offset = kTransitionA7AimUpYOffset;
    return true;
  }
  return false;
}

static void Samus_ApplyTransitionA7PoseYOffset(int16 y_offset) {
  int32 amt = INT16_SHL16(y_offset);
  if (amt) {
    samus_y_radius = kPoseParams[samus_pose].y_radius;
    amt = Samus_CollDetectChangedPose(amt);
  }
  samus_y_pos += amt >> 16;
  samus_prev_y_pos = samus_y_pos;
  if (used_for_ball_bounce_on_landing)
    Samus_ClearBallBounceLanding();
}

void Samus_HandleTransitionsA_7(void) {
  int16 y_offset;
  if (Samus_GetTransitionA7PoseYOffset(&y_offset))
    Samus_ApplyTransitionA7PoseYOffset(y_offset);
}

static Func_U8 *const kSamus_HandleTransitionsB_1[28] = {  // 0x91ED4E
  Samus_HandleTransitionsB_1_0,
  Samus_HandleTransitionsB_1_0,
  Samus_HandleTransitionsB_1_0,
  Samus_HandleTransitionsB_1_0,
  Samus_HandleTransitionsB_1_4,
  Samus_HandleTransitionsB_1_0,
  Samus_HandleTransitionsB_1_6,
  Samus_HandleTransitionsB_1_7,
  Samus_HandleTransitionsB_1_4,
  Samus_HandleTransitionsB_1_4,
  Samus_HandleTransitionsB_1_10,
  Samus_HandleTransitionsB_1_11,
  Samus_HandleTransitionsB_1_11,
  Samus_HandleTransitionsB_1_0,
  Samus_HandleTransitionsB_1_11,
  Samus_HandleTransitionsB_1_11,
  Samus_HandleTransitionsB_1_0,
  Samus_HandleTransitionsB_1_4,
  Samus_HandleTransitionsB_1_4,
  Samus_HandleTransitionsB_1_4,
  Samus_HandleTransitionsB_1_0,
  Samus_HandleTransitionsB_1_0,
  Samus_HandleTransitionsB_1_10,
  Samus_HandleTransitionsB_1_11,
  Samus_HandleTransitionsB_1_11,
  Samus_HandleTransitionsB_1_11,
  Samus_HandleTransitionsB_1_11,
  Samus_HandleTransitionsB_1_11,
};

void Samus_HandleTransitionsB_1(void) {
  kSamus_HandleTransitionsB_1[samus_prev_movement_type2]();
  Samus_SetSpeedForKnockback_Y();
  bomb_jump_dir = 0;
  samus_contact_damage_index = 0;
  samus_hurt_flash_counter = 1;
}

uint8 Samus_HandleTransitionsB_1_10(void) {  // 0x91EDA2
  return 0;
}

uint8 Samus_HandleTransitionsB_1_11(void) {  // 0x91EDA4
  return 0;
}

uint8 Samus_HandleTransitionsB_1_6(void) {  // 0x91EDA6
  if (frame_handler_gamma == FUNC16(Samus_Func9))
    return 0;
  else
    return Samus_HandleTransitionsB_1_0();
}

uint8 Samus_HandleTransitionsB_1_0(void) {  // 0x91EDB0
  knockback_dir = Samus_KnockbackDirForDamageBoost();
  samus_movement_handler = FUNC16(Samus_MoveHandler_Knockback);
  return 1;
}

uint8 Samus_HandleTransitionsB_1_4(void) {  // 0x91EE27
  knockback_dir = Samus_DefaultKnockbackDirForFacing();
  samus_movement_handler = FUNC16(Samus_MoveHandler_Knockback);
  return 0;
}

uint8 Samus_HandleTransitionsB_1_7(void) {  // 0x91EE48
  knockback_dir = Samus_DefaultKnockbackDirForFacing();
  samus_movement_handler = FUNC16(Samus_MoveHandler_Knockback);
  return 1;
}

void Samus_HandleTransitionsB_2(void) {  // 0x91EE69
  knockback_dir = 0;
  samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
  samus_y_speed = 0;
  samus_y_subspeed = 0;
  samus_y_dir = 0;
  Samus_AlignBottomWithPrevPose();
}

void Samus_HandleTransitionsB_3(void) {  // 0x91EE80
  bomb_jump_dir = (uint8)bomb_jump_dir | 0x800;
  samus_movement_handler = FUNC16(Samus_MoveHandler_BombJumpStart);
  if (samus_input_handler != FUNC16(Samus_InputHandler_E91D))
    samus_input_handler = FUNC16(nullsub_152);
}

void Samus_HandleTransitionsB_4(void) {  // 0x91EEA1
  Samus_InitJump();
}

void Samus_HandleTransitionsB_5(void) {  // 0x91EEA6
  if (Samus_CanStartGroundXrayTransition())
    Samus_StartGroundXrayTransition();
}

void Samus_HandleTransitionsB_8(void) {  // 0x91EF3B
  samus_y_pos -= 5;
  samus_prev_y_pos = samus_y_pos;
  frame_handler_alfa = FUNC16(EmptyFunction);
}

void Samus_HandleTransitionsB_9(void) {  // 0x91EF4F
  GrappleBeamFunc_BD95();
  Samus_HandleTransitionsB_9B();
}

void Samus_HandleTransitionsB_10(void) {  // 0x91EFBC
  GrappleBeamFunc_BEEB();
  Samus_HandleTransitionsB_9B();
}

void Samus_HandleTransitionsB_9B(void) {  // 0x91EF53
  Samus_ClampGrappleTransitionPreviousPosition();
  Samus_ResetGrappleTransitionMotion();
}


void nullsub_23(void) {  // 0x91EFDE
}


static Func_V *const kSamus_HandleTransitionsA_5[7] = {  // 0x91EFC4
  sub_91EFC3,
  Samus_HandleTransitionsA_5_1,
  Samus_HandleTransitionsA_5_2,
  nullsub_23,
  Samus_HandleTransitionsA_5_4,
  Samus_HandleTransitionsA_5_5,
  Samus_HandleTransitionsA_5_6,
};
void Samus_HandleTransitionsA_5(void) {
  kSamus_HandleTransitionsA_5[(uint8)input_to_pose_calc]();
}

void Samus_HandleTransitionsA_5_4(void) {  // 0x91EFDF
  samus_y_subspeed = 0;
  samus_y_speed = 0;
  UNUSED_word_7E0B1A = 0;
  samus_y_dir = 2;
}

void Samus_HandleTransitionsA_5_2(void) {  // 0x91EFEF
  if (samus_y_dir != 1) {
    used_for_ball_bounce_on_landing = 0;
    samus_y_subspeed = 0;
    samus_y_speed = 0;
    samus_is_falling_flag = 1;
    samus_y_dir = 2;
  }
  UNUSED_word_7E0A18 = 0;
}

void sub_91EFC3(void) {}
uint8 sub_91EFC3_rv(void) { return 0; }

static Func_U8 *const kSamus_HandleTransitionsA_5_1[6] = {  // 0x91F010
  Samus_HandleTransitionsA_5_1_0,
  Samus_MorphBallBounceNoSpringballTrans,
  Samus_HandleTransitionsA_5_1_2,
  Samus_MorphBallBounceSpringballTrans,
  sub_91EFC3_rv,
  Samus_HandleTransitionsA_5_1_5,
};

void Samus_HandleTransitionsA_5_1(void) {
  HandleLandingSoundEffectsAndGfx();
  if (HIBYTE(input_to_pose_calc) == 4) {
    SamusFunc_F1D3();
  } else if (!(kSamus_HandleTransitionsA_5_1[HIBYTE(input_to_pose_calc)]() & 1)) {
    UNUSED_word_7E0A18 = 0;
    samus_x_accel_mode = kSamusXAccelMode_None;
    samus_x_base_speed = 0;
    samus_x_base_subspeed = 0;
    SamusFunc_F1D3();
  }
}

void HandleLandingSoundEffectsAndGfx(void) {  // 0x91F046
  if ((samus_prev_movement_type2 == kMovementType_03_SpinJumping
       || samus_prev_movement_type2 == kMovementType_14_WallJumping)
      && !cinematic_function) {
    if (samus_prev_pose == kPose_81_FaceR_Screwattack || samus_prev_pose == kPose_82_FaceL_Screwattack)
      QueueSfx1_Max6(0x34);
    else
      QueueSfx1_Max6(0x32);
  }
  if (samus_y_speed && !sign16(samus_y_speed - 5)) {
    if (!cinematic_function)
      QueueSfx3_Max6(4);
  } else {
    if (!samus_y_subspeed)
      return;
    if (!cinematic_function)
      QueueSfx3_Max6(5);
  }
  HandleLandingGraphics();
}

static Func_V *const off_91F0AE[8] = {  // 0x91F0A5
  HandleLandingGraphics_Crateria,
  HandleLandingGraphics_Brinstar,
  HandleLandingGraphics_Norfair,
  HandleLandingGraphics_Norfair,
  HandleLandingGraphics_Maridia,
  HandleLandingGraphics_Tourian,
  HandleLandingGraphics_Ceres,
  HandleLandingGraphics_Ceres,
};
void HandleLandingGraphics(void) {
  off_91F0AE[(area_index)]();
}

void HandleLandingGraphics_Ceres(void) {  // 0x91F0BE
  atmospheric_gfx_frame_and_type[2] = 0;
  atmospheric_gfx_frame_and_type[3] = 0;
}
static const uint8 kCrateriaLandingGfxRules[17] = {  // 0x91F0C5
  1, 0, 0, 0, 0, 2, 0, 4,
  0, 4, 4, 4, 4, 0, 4, 0,
  0,
};

static bool Samus_CrateriaRoomUsesMaridiaLandingGfx(uint16 crateria_room_index) {
  uint8 rule = kCrateriaLandingGfxRules[crateria_room_index];
  if ((rule & kCrateriaLandingGfx_FxTypeGate) != 0)
    return fx_type == kCrateriaLandingGfxRequiredFxType;
  if ((rule & kCrateriaLandingGfx_YThresholdGate) != 0)
    return !sign16(samus_y_pos - kCrateriaLandingGfxYThreshold);
  return (rule & kCrateriaLandingGfx_Always) != 0;
}

void HandleLandingGraphics_Crateria(void) {
  if (cinematic_function) {
    HandleLandingGraphics_Ceres();
    return;
  }
  if (room_index == kCrateriaLandingGfxNorfairRoomIndex) {
    HandleLandingGraphics_Norfair();
    return;
  }
  if (!sign16(room_index - kCrateriaLandingGfxRuleCount) ||
      !Samus_CrateriaRoomUsesMaridiaLandingGfx(room_index)) {
    HandleLandingGraphics_Ceres();
    return;
  }
  HandleLandingGraphics_Maridia();
}

void HandleLandingGraphics_Maridia(void) {  // 0x91F116
  uint16 bottom = Samus_GetBottom_R18();
  if (Samus_IsSubmergedInRoomLiquid(bottom))
    return;

  atmospheric_gfx_frame_and_type[2] = 256;
  atmospheric_gfx_frame_and_type[3] = 256;
  atmospheric_gfx_anim_timer[2] = 3;
  atmospheric_gfx_anim_timer[3] = 3;
  atmospheric_gfx_x_pos[2] = samus_x_pos + 4;
  atmospheric_gfx_x_pos[3] = samus_x_pos - 3;
  atmospheric_gfx_y_pos[2] = bottom - 4;
  atmospheric_gfx_y_pos[3] = bottom - 4;
}

void HandleLandingGraphics_Norfair(void) {  // 0x91F166
  uint16 bottom = Samus_GetBottom_R18();
  if (Samus_IsSubmergedInRoomLiquid(bottom))
    return;

  atmospheric_gfx_frame_and_type[2] = 1536;
  atmospheric_gfx_frame_and_type[3] = 1536;
  atmospheric_gfx_anim_timer[2] = 3;
  atmospheric_gfx_anim_timer[3] = 3;
  atmospheric_gfx_x_pos[2] = samus_x_pos + 8;
  atmospheric_gfx_x_pos[3] = samus_x_pos - 8;
  atmospheric_gfx_y_pos[2] = bottom;
  atmospheric_gfx_y_pos[3] = bottom;
}

void HandleLandingGraphics_Brinstar(void) {  // 0x91F1B2
  if (room_index == 8)
    HandleLandingGraphics_Norfair();
  else
    HandleLandingGraphics_Tourian();
}

void HandleLandingGraphics_Tourian(void) {  // 0x91F1BA
  if (!sign16(room_index - 5) && (sign16(room_index - 9) || room_index == 11)) {
    HandleLandingGraphics_Norfair();
  } else {
    atmospheric_gfx_frame_and_type[2] = 0;
    atmospheric_gfx_frame_and_type[3] = 0;
  }
}

void SamusFunc_F1D3(void) {  // 0x91F1D3
  samus_is_falling_flag = 0;
  UNUSED_word_7E0B1A = 0;
  UNUSED_word_7E0B2A = 0;
  samus_y_subspeed = 0;
  samus_y_speed = 0;
  samus_y_dir = 0;
  UNUSED_word_7E0B38 = 0;
  used_for_ball_bounce_on_landing = 0;
}

uint8 Samus_HandleTransitionsA_5_1_0(void) {  // 0x91F1EC
  if (samus_input_handler != FUNC16(Samus_InputHandler_E91D))
    samus_input_handler = FUNC16(HandleAutoJumpHack);
  return 0;
}
uint8 Samus_MorphBallBounceNoSpringballTrans(void) {  // 0x91F1FC
  int16 doubled_bounce_state = 2 * used_for_ball_bounce_on_landing;
  if (2 * used_for_ball_bounce_on_landing) {
    if (doubled_bounce_state == kBallLandingBounce_KeepCurrentPose) {
      Samus_StartBallLandingBounce(used_for_ball_bounce_on_landing + 1,
                                   kBallBounceSecondYSpeed);
      return 1;
    }
    if (doubled_bounce_state != kBallLandingBounce_SelectGroundPose)
      Unreachable();
  } else if (Samus_BallLandingCanBounce()) {
    Samus_StartBallLandingBounce(kBallBounceState_NoSpring_First,
                                 kBallBounceFirstYSpeed);
    return 1;
  }
  Samus_ClearBallBounceLanding();
  return 0;
}

uint8 Samus_HandleTransitionsA_5_1_2(void) {  // 0x91F253
  used_for_ball_bounce_on_landing = 0;
  enable_horiz_slope_coll = 3;
  return 0;
}

uint8 Samus_MorphBallBounceSpringballTrans(void) {  // 0x91F25E
  if ((button_config_jump_a & joypad1_lastkeys) != 0) {
    used_for_ball_bounce_on_landing = 0;
    Samus_InitJump();
    return 1;
  }
  int16 doubled_bounce_state = 2 * (uint8)used_for_ball_bounce_on_landing;
  if (doubled_bounce_state) {
    if (doubled_bounce_state == kBallLandingBounce_KeepCurrentPose) {
      Samus_StartBallLandingBounce(kSpringBallBounceState_Second,
                                   kBallBounceSecondYSpeed);
      return 1;
    }
    if (doubled_bounce_state != kBallLandingBounce_SelectGroundPose)
      Unreachable();
  } else if (Samus_BallLandingCanBounce()) {
    Samus_StartBallLandingBounce(kSpringBallBounceState_First,
                                 kBallBounceFirstYSpeed);
    return 1;
  }
  Samus_ClearBallBounceLanding();
  return 0;
}

uint8 Samus_HandleTransitionsA_5_1_5(void) {  // 0x91F2CE
  used_for_ball_bounce_on_landing = 0;
  return 0;
}

void Samus_HandleTransitionsA_5_5(void) {  // 0x91F2D3
  samus_x_accel_mode = kSamusXAccelMode_None;
  samus_collides_with_solid_enemy = 0;
  samus_is_falling_flag = 0;
  UNUSED_word_7E0B1A = 0;
  samus_x_base_speed = 0;
  samus_x_base_subspeed = 0;
  UNUSED_word_7E0A18 = 0;
  QueueSfx3_Max6(5);
}

void Samus_HandleTransitionsA_5_6(void) {  // 0x91F2F0
  if (samus_collides_with_solid_enemy) {
    if (samus_prev_movement_type2 == 9) {
      enable_horiz_slope_coll = samus_pose_x_dir != kSamusPoseXDir_FaceRight;
      UNUSED_word_7E0A18 = 0;
    }
  }
}

void Samus_HandleTransitionsC_1(void) {  // 0x91F31D
  knockback_dir = 0;
  samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
  used_for_ball_bounce_on_landing = 0;
  samus_y_subspeed = 0;
  samus_y_speed = 0;
  samus_is_falling_flag = 1;
  samus_y_dir = 2;
  Samus_AlignBottomWithPrevPose();
  if (samus_input_handler != FUNC16(Samus_InputHandler_E91D))
    samus_input_handler = FUNC16(Samus_InputHandler_E913);
}

void Samus_HandleTransitionsC_2(void) {  // 0x91F34E
  Samus_AlignBottomWithPrevPose();
  samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
  if (game_state == kGameState_42_PlayingDemo)
    samus_input_handler = FUNC16(Samus_InputHandler_E91D);
  else
    samus_input_handler = FUNC16(Samus_InputHandler_E913);
}

void Samus_HandleTransitionsC_3(void) {  // 0x91F36E
  used_for_ball_bounce_on_landing = 0;
  samus_anim_frame_timer += samus_anim_frame_buffer;
}

void Samus_HandleTransitionsC_4(void) {  // 0x91F37C
  samus_x_pos = layer1_x_pos + 128;
  samus_prev_x_pos = layer1_x_pos + 128;
  samus_y_pos = layer1_y_pos + 128;
  samus_prev_y_pos = layer1_y_pos + 128;
}

void Samus_HandleTransitionsC_5(void) {  // 0x91F397
  samus_y_pos += 5;
  samus_prev_y_pos = samus_y_pos;
}

void Samus_HandleTransitionsC_6(void) {  // 0x91F3A5
  HandleJumpTransition();
}

void Samus_HandleTransitionsC_7(void) {  // 0x91F3AA
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

void Samus_HandleTransitionsC_8(void) {  // 0x91F3FD
  Samus_HandleTransitionsC_1();
  Samus_HandleTransitionsC_3();
}
