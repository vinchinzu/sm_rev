#include "physics.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "sm_rtl.h"

typedef void HandlerFunc(void);

#define kSamusFramesForUnderwaterSfx ((uint8*)RomFixedPtr(0x90a514))



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

static void nullsub_13(void) {}

static HandlerFunc *const kSamusCrouchingEtcFuncs[12] = {
  nullsub_13,
  nullsub_13,
  nullsub_13,
  nullsub_13,
  SamusCrouchingEtcFunc,
  SamusCrouchingEtcFunc,
  nullsub_13,
  nullsub_13,
  nullsub_13,
  nullsub_13,
  SamusCrouchingEtcFunc,
  SamusCrouchingEtcFunc,
};

void Samus_UpdateSpeedEchoPos(void) {  // 0x90EEE7
  if ((speed_boost_counter & 0xFF00) == 1024 && (speed_echoes_index & 0x8000) == 0 && (game_time_frames & 3) == 0) {
    uint16 v0 = speed_echoes_index;
    int v1 = speed_echoes_index >> 1;
    speed_echo_xpos[v1] = samus_x_pos;
    speed_echo_ypos[v1] = samus_y_pos;
    uint16 v2 = v0 + 2;
    if ((int16)(v2 - 4) >= 0)
      v2 = 0;
    speed_echoes_index = v2;
  }
}

void DEPRECATED_Samus_UpdateSpeedEchoPos(void) {  // 0x90EEE7
  Samus_UpdateSpeedEchoPos();
}

void MoveSamusWithControlPad(void) {  // 0x90ECD5
  if ((joypad1_lastkeys & 0x800) != 0)
    Samus_MoveUp(INT16_SHL16(-4));
  if ((joypad1_lastkeys & 0x400) != 0)
    Samus_MoveDown(INT16_SHL16(4));
  if ((joypad1_lastkeys & 0x200) != 0)
    Samus_MoveLeft(INT16_SHL16(-4));
  if ((joypad1_lastkeys & 0x100) != 0)
    Samus_MoveRight(INT16_SHL16(4));
}

void Samus_MovementHandler_Normal(void) {
  if (!time_is_frozen_flag) {
    kSamusMovementHandlers[samus_movement_type]();
    Samus_UpdateSpeedEchoPos();
  }
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
    Samus_CancelSpeedBoost();
    samus_x_extra_run_speed = 0;
    samus_x_extra_run_subspeed = 0;
    samus_x_base_speed = 0;
    samus_x_base_subspeed = 0;
    samus_x_accel_mode = 0;
  } else {
    if (elevator_status) {
      samus_collision_direction = 2;
      Samus_MoveDown_NoSolidColl(INT16_SHL16(1));
    }
    input_to_pose_calc = 0;
  }
}

void Samus_Movement_01_Running(void) {
  static const uint8 kSamusFootstepFrame[10] = { 0, 0, 1, 0, 0, 0, 0, 1, 0, 0 };
  Samus_HandleMovement_X();
  Samus_Move_NoSpeedCalc_Y();
  if (samus_anim_frame_timer == 1) {
    if (kSamusFootstepFrame[samus_anim_frame]) {
      Samus_FootstepGraphics();
      if (!cinematic_function && !boss_id && !samus_shine_timer && (speed_boost_counter & 0x400) == 0)
        QueueSfx3_Max6(6);
    }
  }
}

void Samus_Movement_02_NormalJumping(void) {
  Samus_JumpingMovement();
}

void Samus_Movement_03_SpinJumping(void) {
  static const uint16 kSamusPhys_JumpMinYVelAir = 0x280;
  static const uint16 kSamusPhys_JumpMaxYVelAir = 0x500;
  static const uint16 kSamusPhys_JumpMinYVelWater = 0x80;
  static const uint16 kSamusPhys_JumpMaxYVelWater = 0x500;
  uint16 r18 = 0;
  if ((samus_suit_palette_index & 4) == 0) {
    uint16 r20 = Samus_GetTop_R20();
    if ((fx_y_pos & 0x8000) != 0) {
      if ((lava_acid_y_pos & 0x8000) == 0 && sign16(lava_acid_y_pos - r20))
        r18 = 1;
    } else if (sign16(fx_y_pos - r20) && (fx_liquid_options & 4) == 0) {
      r18 = 1;
    }
  }
  if (!r18) {
    if ((equipped_items & 0x200) != 0) {
      if (samus_y_dir != 2)
        goto done;
      if (liquid_physics_type) {
        if ((int16)(*(uint16 *)((uint8 *)&samus_y_subspeed + 1) - kSamusPhys_JumpMinYVelWater) < 0
            || (int16)(*(uint16 *)((uint8 *)&samus_y_subspeed + 1) - kSamusPhys_JumpMaxYVelWater) >= 0) {
          goto done;
        }
      } else if ((int16)(*(uint16 *)((uint8 *)&samus_y_subspeed + 1) - kSamusPhys_JumpMinYVelAir) < 0
          || (int16)(*(uint16 *)((uint8 *)&samus_y_subspeed + 1) - kSamusPhys_JumpMaxYVelAir) >= 0) {
        goto done;
      }
      UNUSED_word_7E0DFA = UNUSED_word_7E0DFA & 0xFF00 | 1;
      if ((button_config_jump_a & joypad1_newkeys) != 0)
        Samus_InitJump();
    }
done:
    if (samus_pose == kPose_81_FaceR_Screwattack || samus_pose == kPose_82_FaceL_Screwattack) {
      samus_contact_damage_index = 3;
    } else if (!sign16(flare_counter - 60)) {
      samus_contact_damage_index = 4;
    }
  } else {
    if (samus_anim_frame_timer == 1 && kSamusFramesForUnderwaterSfx[samus_anim_frame])
      QueueSfx1_Max6(0x2F);
  }
  Samus_SpinJumpMovement();
}

void Samus_Movement_04_MorphBallOnGround(void) {
  if (!samus_x_accel_mode) {
    if (samus_pose_x_dir == 4) {
      if (samus_pose == kPose_41_FaceL_Morphball_Ground)
        goto no_accel_ground;
    } else if (samus_pose == kPose_1D_FaceR_Morphball_Ground) {
no_accel_ground:
      Samus_Move_NoBaseSpeed_X();
      if (!(Samus_CheckAndMoveY() & 1)) {
        Samus_Move_NoSpeedCalc_Y();
        Samus_CancelSpeedBoost();
        samus_x_extra_run_speed = 0;
        samus_x_extra_run_subspeed = 0;
        samus_x_base_speed = 0;
        samus_x_base_subspeed = 0;
        samus_x_accel_mode = 0;
      }
      return;
    }
  }
  Samus_HandleMovement_X();
  if (!(Samus_CheckAndMoveY() & 1))
    Samus_Move_NoSpeedCalc_Y();
}

void Samus_Movement_05_Crouching(void) {
  Samus_Move_NoBaseSpeed_X();
  Samus_Move_NoSpeedCalc_Y();
  samus_x_extra_run_speed = 0;
  samus_x_extra_run_subspeed = 0;
  samus_x_base_speed = 0;
  samus_x_base_subspeed = 0;
  samus_x_accel_mode = 0;
}

void Samus_Movement_06_Falling(void) {
  Samus_FallingMovement();
  if ((samus_pose == kPose_29_FaceR_Fall
       || samus_pose == kPose_2A_FaceL_Fall
       || samus_pose == kPose_67_FaceR_Fall_Gun
       || samus_pose == kPose_68_FaceL_Fall_Gun)
      && !sign16(samus_y_speed - 5)) {
    if (sign16(samus_anim_frame - 5)) {
      samus_anim_frame_timer = 8;
      samus_anim_frame = 5;
    }
  }
}

void Samus_Movement_07_Unused(void) {
  input_to_pose_calc = 0;
}

void Samus_Movement_08_MorphBallFalling(void) {
  if ((joypad1_lastkeys & (kButton_Left | kButton_Right)) == 0 && !samus_x_accel_mode) {
    Samus_CancelSpeedBoost();
    samus_x_extra_run_speed = 0;
    samus_x_extra_run_subspeed = 0;
    samus_x_base_speed = 0;
    samus_x_base_subspeed = 0;
    samus_x_accel_mode = 0;
  }
  if (used_for_ball_bounce_on_landing)
    Samus_MorphedBouncingMovement();
  else
    Samus_MorphedFallingMovement();
}

void Samus_Movement_14_WallJumping(void) {
  if (sign16(samus_anim_frame - 23)) {
    if (!sign16(samus_anim_frame - 3) && !sign16(flare_counter - 60))
      samus_contact_damage_index = 4;
  } else {
    samus_contact_damage_index = 3;
  }
  Samus_JumpingMovement();
}

void Samus_Movement_09_Unused(void) {
}

void Samus_Movement_0A_KnockbackOrCrystalFlashEnding(void) {
  input_to_pose_calc = 0;
  Samus_Move_NoSpeedCalc_Y();
}

void Samus_Movement_0B_Unused(void) {
  input_to_pose_calc = 0;
}

void Samus_Movement_0C_Unused(void) {
  input_to_pose_calc = 0;
}

void Samus_Movement_0D_Unused(void) {
}

void Samus_Movement_0F_CrouchingEtcTransition(void) {
  if (sign16(samus_pose - kPose_F1_FaceR_CrouchTrans_AimU)) {
    if (!sign16(samus_pose - kPose_DB))
      goto done;
    kSamusCrouchingEtcFuncs[samus_pose - 53]();
  }
  Samus_Move_NoBaseSpeed_X();
  if (!(Samus_MoveY_Simple_() & 1))
    Samus_Move_NoSpeedCalc_Y();
done:
  if (input_to_pose_calc == 1025) {
    samus_y_subspeed = 0;
    samus_y_speed = 0;
    samus_y_dir = 0;
    used_for_ball_bounce_on_landing = 0;
  }
  input_to_pose_calc = 0;
}

void SamusCrouchingEtcFunc(void) {
  enable_horiz_slope_coll = 3;
  UNUSEDword_7E0AA4 = 0;
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

void Samus_Movement_11_SpringBallOnGround(void) {
  if (!samus_x_accel_mode) {
    if (samus_pose_x_dir == 4) {
      if (samus_pose == kPose_7A_FaceL_Springball_Ground)
        goto no_accel_ground;
    } else if (samus_pose == kPose_79_FaceR_Springball_Ground) {
no_accel_ground:
      Samus_Move_NoBaseSpeed_X();
      if (!(Samus_CheckAndMoveY() & 1)) {
        Samus_Move_NoSpeedCalc_Y();
        Samus_CancelSpeedBoost();
        samus_x_extra_run_speed = 0;
        samus_x_extra_run_subspeed = 0;
        samus_x_base_speed = 0;
        samus_x_base_subspeed = 0;
        samus_x_accel_mode = 0;
      }
      return;
    }
  }
  Samus_HandleMovement_X();
  if (!(Samus_CheckAndMoveY() & 1))
    Samus_Move_NoSpeedCalc_Y();
}

void Samus_Movement_12_SpringBallInAir(void) {
  if (used_for_ball_bounce_on_landing)
    Samus_MorphedBouncingMovement();
  else
    Samus_JumpingMovement();
}

void Samus_Movement_13_SpringBallFalling(void) {
  if ((joypad1_lastkeys & (kButton_Left | kButton_Right)) == 0 && !samus_x_accel_mode) {
    Samus_CancelSpeedBoost();
    samus_x_extra_run_speed = 0;
    samus_x_extra_run_subspeed = 0;
    samus_x_base_speed = 0;
    samus_x_base_subspeed = 0;
    samus_x_accel_mode = 0;
  }
  if (used_for_ball_bounce_on_landing)
    Samus_MorphedBouncingMovement();
  else
    Samus_MorphedFallingMovement();
}

void Samus_Movement_15_RanIntoWall(void) {
  Samus_Move_NoBaseSpeed_X();
  Samus_Move_NoSpeedCalc_Y();
  Samus_CancelSpeedBoost();
  samus_x_extra_run_speed = 0;
  samus_x_extra_run_subspeed = 0;
  samus_x_base_speed = 0;
  samus_x_base_subspeed = 0;
  samus_x_accel_mode = 0;
}

void Samus_Movement_16_Grappling(void) {
  if (input_to_pose_calc != 5)
    input_to_pose_calc = 0;
}

void Samus_Movement_17_TurningAroundJumping(void) {
  Samus_HandleMovement_X();
  if (!(Samus_CheckAndMoveY() & 1))
    Samus_Move_NoSpeedCalc_Y();
  Samus_CancelSpeedBoost();
  samus_x_extra_run_speed = 0;
  samus_x_extra_run_subspeed = 0;
  input_to_pose_calc = 0;
}

void Samus_Movement_18_TurningAroundFalling(void) {
  Samus_HandleMovement_X();
  if (!(Samus_CheckAndMoveY() & 1))
    Samus_Move_NoSpeedCalc_Y();
  Samus_CancelSpeedBoost();
  samus_x_extra_run_speed = 0;
  samus_x_extra_run_subspeed = 0;
  input_to_pose_calc = 0;
}

void Samus_Movement_19_DamageBoost(void) {
  Samus_JumpingMovement();
}

void Samus_Movement_1A_GrabbedByDraygon(void) {
  input_to_pose_calc = 0;
}

void Samus_Movement_1B_ShinesparkEtc(void) {
  input_to_pose_calc = 0;
}
