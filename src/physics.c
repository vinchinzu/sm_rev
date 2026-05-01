#include "physics.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "sm_rtl.h"
#include "torizo_config.h"

typedef void HandlerFunc(void);

static HandlerFunc *const kSamusMovementHandlers[28] = {
  &Samus_Movement_00_Standing,
  &Samus_Movement_01_Running,
  &Samus_Movement_02_NormalJumping,
  &Samus_Movement_03_SpinJumping,
  &Samus_Movement_04_MorphBallOnGround,
  &Samus_Movement_05_Crouching,
  &Samus_Movement_06_Falling,
  &Samus_Movement_07_Unused,
  &Samus_Movement_08_MorphBallFalling,
  &Samus_Movement_09_Unused,
  &Samus_Movement_0A_KnockbackOrCrystalFlashEnding,
  &Samus_Movement_0B_Unused,
  &Samus_Movement_0C_Unused,
  &Samus_Movement_0D_Unused,
  &Samus_Movement_0E_TurningAroundOnGround,
  &Samus_Movement_0F_CrouchingEtcTransition,
  &Samus_Movement_10_Moonwalking,
  &Samus_Movement_11_SpringBallOnGround,
  &Samus_Movement_12_SpringBallInAir,
  &Samus_Movement_13_SpringBallFalling,
  &Samus_Movement_14_WallJumping,
  &Samus_Movement_15_RanIntoWall,
  &Samus_Movement_16_Grappling,
  &Samus_Movement_17_TurningAroundJumping,
  &Samus_Movement_18_TurningAroundFalling,
  &Samus_Movement_19_DamageBoost,
  &Samus_Movement_1A_GrabbedByDraygon,
  &Samus_Movement_1B_ShinesparkEtc,
};

void Samus_MovementHandler_Normal(void) {
  if (!time_is_frozen_flag && !TorizoConfig_SamusFreezeActive()) {
    kSamusMovementHandlers[samus_movement_type]();
    Samus_UpdateSpeedEchoPos();
  }
}

void Samus_Movement_07_Unused(void) {
  input_to_pose_calc = 0;
}

void Samus_Movement_09_Unused(void) {
}

void Samus_Movement_0B_Unused(void) {
  input_to_pose_calc = 0;
}

void Samus_Movement_0C_Unused(void) {
  input_to_pose_calc = 0;
}

void Samus_Movement_0D_Unused(void) {
}
