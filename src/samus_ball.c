#include "physics.h"

#include "ida_types.h"
#include "variables.h"
#include "funcs.h"

static void Samus_ClearBallHorizontalState(void) {
  Samus_CancelSpeedBoost();
  samus_x_extra_run_speed = 0;
  samus_x_extra_run_subspeed = 0;
  samus_x_base_speed = 0;
  samus_x_base_subspeed = 0;
  samus_x_accel_mode = 0;
}

static bool Samus_IsNoAccelGroundBallPose(uint16 right_pose, uint16 left_pose) {
  if (samus_pose_x_dir == 4)
    return samus_pose == left_pose;
  return samus_pose == right_pose;
}

static void Samus_GroundBallMovement(uint16 right_pose, uint16 left_pose) {
  if (!samus_x_accel_mode && Samus_IsNoAccelGroundBallPose(right_pose, left_pose)) {
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
  if ((joypad1_lastkeys & (kButton_Left | kButton_Right)) == 0 && !samus_x_accel_mode)
    Samus_ClearBallHorizontalState();

  if (used_for_ball_bounce_on_landing)
    Samus_MorphedBouncingMovement();
  else
    Samus_MorphedFallingMovement();
}

void Samus_Movement_04_MorphBallOnGround(void) {
  Samus_GroundBallMovement(kPose_1D_FaceR_Morphball_Ground, kPose_41_FaceL_Morphball_Ground);
}

void Samus_Movement_08_MorphBallFalling(void) {
  Samus_FallingBallMovement();
}

void Samus_Movement_11_SpringBallOnGround(void) {
  Samus_GroundBallMovement(kPose_79_FaceR_Springball_Ground, kPose_7A_FaceL_Springball_Ground);
}

void Samus_Movement_12_SpringBallInAir(void) {
  if (used_for_ball_bounce_on_landing)
    Samus_MorphedBouncingMovement();
  else
    Samus_JumpingMovement();
}

void Samus_Movement_13_SpringBallFalling(void) {
  Samus_FallingBallMovement();
}
