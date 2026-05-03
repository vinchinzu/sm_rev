#include "physics.h"

#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "samus_env.h"

typedef struct SamusGroundBallPosePair {
  uint16 bank90_xdir8_pose;
  uint16 bank90_xdir4_pose;
} SamusGroundBallPosePair;

static const SamusGroundBallPosePair kMorphBallGroundPoses = {
  kPose_1D_FaceR_Morphball_Ground,
  kPose_41_FaceL_Morphball_Ground,
};

static const SamusGroundBallPosePair kSpringBallGroundPoses = {
  kPose_79_FaceR_Springball_Ground,
  kPose_7A_FaceL_Springball_Ground,
};

static void Samus_ClearBallHorizontalState(void) {
  Samus_CancelSpeedBoost();
  samus_x_extra_run_speed = 0;
  samus_x_extra_run_subspeed = 0;
  samus_x_base_speed = 0;
  samus_x_base_subspeed = 0;
  samus_x_accel_mode = kSamusXAccelMode_None;
}

static bool Samus_GroundBallPoseMatchesXDir(const SamusGroundBallPosePair *poses) {
  if (samus_pose_x_dir == kSamusPoseXDir_FaceRight)
    return samus_pose == poses->bank90_xdir4_pose;
  return samus_pose == poses->bank90_xdir8_pose;
}

static bool Samus_HasNoHorizontalBallInput(void) {
  return (joypad1_lastkeys & (kButton_Left | kButton_Right)) == 0;
}

static bool Samus_BallBounceIsActive(void) {
  return used_for_ball_bounce_on_landing != 0;
}

static void Samus_GroundBallMovement(const SamusGroundBallPosePair *poses) {
  if (samus_x_accel_mode == kSamusXAccelMode_None &&
      Samus_GroundBallPoseMatchesXDir(poses)) {
    Samus_Move_NoBaseSpeed_X();
    if (!(Samus_CheckAndMoveY() & 1)) {
      Samus_Move_NoSpeedCalc_Y();
      Samus_ClearBallHorizontalState();
    }
    return;
  }

  Samus_HandleMovement_X();
  if (!(Samus_CheckAndMoveY() & 1))
    Samus_Move_NoSpeedCalc_Y();
}

static void Samus_FallingBallMovement(void) {
  if (Samus_HasNoHorizontalBallInput() &&
      samus_x_accel_mode == kSamusXAccelMode_None)
    Samus_ClearBallHorizontalState();

  if (Samus_BallBounceIsActive())
    Samus_MorphedBouncingMovement();
  else
    Samus_MorphedFallingMovement();
}

void Samus_Movement_04_MorphBallOnGround(void) {
  Samus_GroundBallMovement(&kMorphBallGroundPoses);
}

void Samus_Movement_08_MorphBallFalling(void) {
  Samus_FallingBallMovement();
}

void Samus_Movement_11_SpringBallOnGround(void) {
  Samus_GroundBallMovement(&kSpringBallGroundPoses);
}

void Samus_Movement_12_SpringBallInAir(void) {
  if (Samus_BallBounceIsActive())
    Samus_MorphedBouncingMovement();
  else
    Samus_JumpingMovement();
}

void Samus_Movement_13_SpringBallFalling(void) {
  Samus_FallingBallMovement();
}
