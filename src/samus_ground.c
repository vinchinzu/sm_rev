#include "physics.h"

#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "sm_rtl.h"

enum {
  kSamusFootstepSfx = 6,
  kSpeedBoostCounter_ChargedBit = 0x400,
  kSamusCollisionDirection_Up = 2,
};

static void Samus_ClearGroundHorizontalState(void) {
  Samus_CancelSpeedBoost();
  samus_x_extra_run_speed = 0;
  samus_x_extra_run_subspeed = 0;
  samus_x_base_speed = 0;
  samus_x_base_subspeed = 0;
  samus_x_accel_mode = 0;
}

void Samus_Movement_00_Standing(void) {
  if (samus_pose && samus_pose != kPose_9B_FaceF_VariaGravitySuit) {
    if ((samus_pose == kPose_01_FaceR_Normal || samus_pose == kPose_02_FaceL_Normal)
        && (button_config_shoot_x & joypad1_lastkeys) != 0) {
      samus_anim_frame_timer = 16;
      samus_anim_frame = 0;
    }
    Samus_Move_NoBaseSpeed_X();
    Samus_Move_NoSpeedCalc_Y();
    Samus_ClearGroundHorizontalState();
  } else {
    if (elevator_status) {
      samus_collision_direction = kSamusCollisionDirection_Up;
      Samus_MoveDown_NoSolidColl(INT16_SHL16(1));
    }
    input_to_pose_calc = 0;
  }
}

void Samus_Movement_01_Running(void) {
  static const uint8 kSamusFootstepFrame[10] = { 0, 0, 1, 0, 0, 0, 0, 1, 0, 0 };
  Samus_HandleMovement_X();
  Samus_Move_NoSpeedCalc_Y();
  if (samus_anim_frame_timer == 1 && kSamusFootstepFrame[samus_anim_frame]) {
    Samus_FootstepGraphics();
    if (!cinematic_function && !boss_id && !samus_shine_timer
        && (speed_boost_counter & kSpeedBoostCounter_ChargedBit) == 0) {
      QueueSfx3_Max6(kSamusFootstepSfx);
    }
  }
}

void Samus_Movement_05_Crouching(void) {
  Samus_Move_NoBaseSpeed_X();
  Samus_Move_NoSpeedCalc_Y();
  Samus_ClearGroundHorizontalState();
}

void Samus_Movement_0E_TurningAroundOnGround(void) {
  Samus_HandleMovement_X();
  Samus_Move_NoSpeedCalc_Y();
  Samus_CancelSpeedBoost();
  samus_x_extra_run_speed = 0;
  samus_x_extra_run_subspeed = 0;
  input_to_pose_calc = 0;
}

void Samus_Movement_10_Moonwalking(void) {
  Samus_HandleMovement_X();
  Samus_Move_NoSpeedCalc_Y();
}

void Samus_Movement_15_RanIntoWall(void) {
  Samus_Move_NoBaseSpeed_X();
  Samus_Move_NoSpeedCalc_Y();
  Samus_ClearGroundHorizontalState();
}
