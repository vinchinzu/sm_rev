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

#define word_91E921 ((uint16*)RomFixedPtr(0x91e921))
#define word_91E9F3 ((uint16*)RomFixedPtr(0x91e9f3))
#define word_91EB74 ((uint16*)RomFixedPtr(0x91eb74))

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
  samus_momentum_routine_index = 5;
}

void Samus_HandleTransFromBlockColl_4(void) {  // 0x91E8E5
  samus_new_pose = samus_pose;
  samus_momentum_routine_index = 5;
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

void Samus_HandleTransFromBlockColl_2(void) {

  if (HIBYTE(input_to_pose_calc) != 4) {
    uint16 v0 = 4 * HIBYTE(input_to_pose_calc);
    if (samus_pose_x_dir == 4)
      samus_new_pose = word_91E921[(v0 >> 1) + 1];
    else
      samus_new_pose = word_91E921[v0 >> 1];
    samus_momentum_routine_index = 5;
  }
}

void Samus_HandleTransFromBlockColl_1(void) {  // 0x91E931
  if (HIBYTE(input_to_pose_calc) != 4) {
    if (off_91E951[HIBYTE(input_to_pose_calc)]() & 1)
      samus_momentum_routine_index = 0;
    else
      samus_momentum_routine_index = 5;
  }
}

uint8 Samus_HandleTransFromBlockColl_1_0(void) {  // 0x91E95D
  int16 v0;

  if (samus_prev_movement_type2 == kMovementType_03_SpinJumping
      || samus_prev_movement_type2 == kMovementType_14_WallJumping) {
    if (samus_pose_x_dir == 4)
      samus_new_pose = kPose_A7_FaceL_LandSpinJump;
    else
      samus_new_pose = kPose_A6_FaceR_LandSpinJump;
    return 0;
  } else {
    v0 = kPoseParams[samus_pose].direction_shots_fired;
    if (v0 == 255) {
      if (samus_pose_x_dir == 4)
        samus_new_pose = kPose_A5_FaceL_LandJump;
      else
        samus_new_pose = kPose_A4_FaceR_LandJump;
      return 0;
    } else {
      if (v0 != 2 && v0 != 7)
        goto LABEL_6;
      if ((button_config_shoot_x & joypad1_lastkeys) == 0) {
        v0 = kPoseParams[samus_pose].direction_shots_fired;
LABEL_6:
        samus_new_pose = word_91E9F3[v0];
        return 0;
      }
      if (samus_pose_x_dir == 4)
        samus_new_pose = kPose_E7_FaceL_LandJump_Fire;
      else
        samus_new_pose = kPose_E6_FaceR_LandJump_Fire;
      return 0;
    }
  }
}

uint8 Samus_HandleTransFromBlockColl_1_1(void) {  // 0x91EA07
  int16 v0;

  v0 = 2 * used_for_ball_bounce_on_landing;
  if (2 * used_for_ball_bounce_on_landing) {
    if (v0 == 2) {
      samus_new_pose = samus_pose;
      return 0;
    }
    if (v0 != 4)
      Unreachable();
  } else if (!sign16(samus_y_speed - 3)) {
    samus_new_pose = samus_pose;
    return 0;
  }
  if (samus_pose_x_dir == 4)
    samus_new_pose = kPose_41_FaceL_Morphball_Ground;
  else
    samus_new_pose = kPose_1D_FaceR_Morphball_Ground;
  return 0;
}

uint8 Samus_HandleTransFromBlockColl_1_2(void) {  // 0x91EA48
  if (samus_pose_x_dir == 4)
    samus_new_pose = 66;
  else
    samus_new_pose = 32;
  return 0;
}

uint8 Samus_HandleTransFromBlockColl_1_3(void) {  // 0x91EA63
  int16 v1;

  if ((button_config_jump_a & joypad1_lastkeys) != 0) {
    samus_new_pose = samus_pose;
    return 0;
  }
  v1 = 2 * (uint8)used_for_ball_bounce_on_landing;
  if (v1) {
    if (v1 == 2) {
      samus_new_pose = samus_pose;
      return 0;
    }
    if (v1 != 4)
      Unreachable();
  } else if (!sign16(samus_y_speed - 3)) {
    samus_new_pose = samus_pose;
    return 0;
  }
  if (samus_pose_x_dir == 4)
    samus_new_pose = kPose_7A_FaceL_Springball_Ground;
  else
    samus_new_pose = 121;
  return 0;
}

uint8 Samus_HandleTransFromBlockColl_1_5(void) {  // 0x91EAB6
  samus_new_pose = samus_pose;
  return 0;
}

void Samus_HandleTransFromBlockColl_5(void) {  // 0x91EABE
  if (samus_pose_x_dir == 4)
    samus_new_pose = kPose_84_FaceL_Walljump;
  else
    samus_new_pose = kPose_83_FaceR_Walljump;
  samus_momentum_routine_index = 5;
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


void Samus_CheckWalkedIntoSomething(void) {  // 0x91EADE
  int32 amt;

  if (samus_collides_with_solid_enemy && samus_movement_type == 1) {
    samus_new_pose = word_91EB74[*(&kPoseParams[0].direction_shots_fired + (8 * samus_pose))];
    samus_collides_with_solid_enemy = 0;
    return;
  }
  if (samus_new_pose == 0xFFFF || kPoseParams[samus_new_pose].movement_type != 1) {
    samus_collides_with_solid_enemy = 0;
    return;
  }
  if (samus_pose_x_dir != 4) {
    samus_collision_direction = 1;
    amt = INT16_SHL16(1);
    CheckEnemyColl_Result cres = Samus_CheckSolidEnemyColl(amt);
    if (!cres.collision) {
      samus_collision_direction = 1;
      amt = INT16_SHL16(1);
      goto LABEL_10;
    }
LABEL_11:
    samus_new_pose = word_91EB74[*(&kPoseParams[0].direction_shots_fired + (8 * samus_new_pose))];
    samus_collides_with_solid_enemy = 0;
    return;
  }
  samus_collision_direction = 0;
  amt = INT16_SHL16(1);
  CheckEnemyColl_Result cres = Samus_CheckSolidEnemyColl(amt);
  if (cres.collision)
    goto LABEL_11;
  amt = INT16_SHL16(-1);
  samus_collision_direction = 0;
LABEL_10:
  Samus_MoveRight_NoSolidColl(amt);
  if (samus_collision_flag)
    goto LABEL_11;
  samus_collides_with_solid_enemy = 0;
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

void Samus_HandleTransitions(void) {
  if ((samus_new_pose_transitional & 0x8000) == 0) {
    if (samus_hurt_switch_index != 3) {
      if (samus_hurt_switch_index == 1)
        goto LABEL_7;
      goto LABEL_6;
    }
    if (samus_special_transgfx_index != 9) {
LABEL_6:
      samus_pose = samus_new_pose_transitional;
      SamusFunc_F433();
      Samus_SetAnimationFrameIfPoseChanged();
LABEL_7:
      kSamus_HandleTransitionsC[samus_hurt_switch_index]();
LABEL_15:
      samus_last_different_pose = samus_prev_pose;
      *(uint16 *)&samus_last_different_pose_x_dir = *(uint16 *)&samus_prev_pose_x_dir;
      samus_prev_pose = samus_pose;
      *(uint16 *)&samus_prev_pose_x_dir = *(uint16 *)&samus_pose_x_dir;
      goto LABEL_16;
    }
  }
  if ((samus_new_pose_interrupted & 0x8000) == 0) {
    samus_pose = samus_new_pose_interrupted;
    if (!(SamusFunc_F404() & 1))
      kSamus_HandleTransitionsB[samus_special_transgfx_index]();
    goto LABEL_15;
  }
  Samus_CheckWalkedIntoSomething();
  if ((samus_new_pose & 0x8000) == 0) {
    samus_pose = samus_new_pose;
    if (!(SamusFunc_F404() & 1))
      kSamus_HandleTransitionsA[samus_momentum_routine_index]();
    goto LABEL_15;
  }
LABEL_16:
  input_to_pose_calc = 0;
}

void Samus_HandleTransitionsA_1(void) {  // 0x91EC50
  if (samus_x_base_speed || samus_x_base_subspeed) {
    AddToHiLo(&samus_x_base_speed, &samus_x_base_subspeed, __PAIR32__(samus_x_extra_run_speed, samus_x_extra_run_subspeed));
    samus_x_accel_mode = 2;
    Samus_CancelSpeedBoost();
    SetHiLo(&samus_x_extra_run_speed, &samus_x_extra_run_subspeed, 0);
    SamusFunc_EC80();
  } else {
    Samus_HandleTransitionsA_2();
  }
}

void Samus_HandleTransitionsA_6(void) {  // 0x91EC85
  samus_x_accel_mode = 0;
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
  samus_x_accel_mode = 0;
  Samus_CancelSpeedBoost();
}

static const uint16 word_91ED36[12] = {  // 0x91ECDA
  5, 5, 9, 9, 0, 0, 0, 0,
  0, 0, 0, 0,
};

void Samus_HandleTransitionsA_7(void) {
  int32 amt;

  if (sign16(samus_pose - kPose_DB)) {
    amt = INT16_SHL16(word_91ED36[samus_pose - 53]);
    if (amt) {
LABEL_4:
      samus_y_radius = kPoseParams[samus_pose].y_radius;
      amt = Samus_CollDetectChangedPose(amt);
    }
    samus_y_pos += amt >> 16;
    samus_prev_y_pos = samus_y_pos;
    if (used_for_ball_bounce_on_landing) {
      used_for_ball_bounce_on_landing = 0;
      samus_y_subspeed = 0;
      samus_y_speed = 0;
      samus_y_dir = 0;
    }
  } else if (!sign16(samus_pose - kPose_F1_FaceR_CrouchTrans_AimU) && sign16(samus_pose - kPose_F7_FaceR_StandTrans_AimU)) {
    amt = INT16_SHL16(5);
    goto LABEL_4;
  }
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
  if (samus_pose_x_dir == 4) {
    if (knockback_x_dir) {
      if ((joypad1_lastkeys & 0x200) != 0)
        knockback_dir = 5;
      else
        knockback_dir = 2;
    } else if ((joypad1_lastkeys & 0x200) != 0) {
      knockback_dir = 4;
    } else {
      knockback_dir = 1;
    }
  } else if (knockback_x_dir) {
    if ((joypad1_lastkeys & 0x100) != 0)
      knockback_dir = 5;
    else
      knockback_dir = 2;
  } else if ((joypad1_lastkeys & 0x100) != 0) {
    knockback_dir = 4;
  } else {
    knockback_dir = 1;
  }
  samus_movement_handler = FUNC16(Samus_MoveHandler_Knockback);
  return 1;
}

uint8 Samus_HandleTransitionsB_1_4(void) {  // 0x91EE27
  if (samus_pose_x_dir == 4)
    knockback_dir = 1;
  else
    knockback_dir = 2;
  samus_movement_handler = FUNC16(Samus_MoveHandler_Knockback);
  return 0;
}

uint8 Samus_HandleTransitionsB_1_7(void) {  // 0x91EE48
  if (samus_pose_x_dir == 4)
    knockback_dir = 1;
  else
    knockback_dir = 2;
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
  switch (samus_movement_type) {
  case kMovementType_00_Standing:
    goto LABEL_5;
  case kMovementType_05_Crouching:
    if (samus_pose_x_dir == 4)
      xray_angle = 192;
    else
      xray_angle = 64;
    goto LABEL_11;
  case kMovementType_15_RanIntoWall:
  case kMovementType_01_Running:
LABEL_5:
    if (samus_pose_x_dir == 4)
      xray_angle = 192;
    else
      xray_angle = 64;
LABEL_11:
    samus_anim_frame = 2;
    samus_anim_frame_timer = 63;
    samus_movement_handler = FUNC16(SamusMovementType_Xray);
    samus_input_handler = FUNC16(Samus_Func20_);
    timer_for_shine_timer = 8;
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
    QueueSfx1_Max6(9);
    break;
  }
}

void Samus_HandleTransitionsB_8(void) {  // 0x91EF3B
  samus_y_pos -= 5;
  samus_prev_y_pos = samus_y_pos;
  frame_handler_alfa = FUNC16(EmptyFunction);
}

void Samus_HandleTransitionsB_9(void) {  // 0x91EF4F
  int16 v0;
  int16 v1;

  GrappleBeamFunc_BD95();
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
  Samus_CancelSpeedBoost();
  samus_x_base_speed = 0;
  samus_x_base_subspeed = 0;
  samus_y_subspeed = 0;
  samus_y_speed = 0;
  SetHiLo(&samus_x_extra_run_speed, &samus_x_extra_run_subspeed, 0);
}

void Samus_HandleTransitionsB_10(void) {  // 0x91EFBC
  GrappleBeamFunc_BEEB();
  Samus_HandleTransitionsB_9B();
}

void Samus_HandleTransitionsB_9B(void) {  // 0x91EF53
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
  Samus_CancelSpeedBoost();
  samus_x_base_speed = 0;
  samus_x_base_subspeed = 0;
  samus_y_subspeed = 0;
  samus_y_speed = 0;
  SetHiLo(&samus_x_extra_run_speed, &samus_x_extra_run_subspeed, 0);
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
    samus_x_accel_mode = 0;
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
static const uint8 g_byte_91F0F3[17] = {  // 0x91F0C5
  1, 0, 0, 0, 0, 2, 0, 4,
  0, 4, 4, 4, 4, 0, 4, 0,
  0,
};
void HandleLandingGraphics_Crateria(void) {
  if (cinematic_function)
    goto LABEL_13;
  if (room_index == 28) {
    HandleLandingGraphics_Norfair();
    return;
  }
  if ((int16)(room_index - 16) >= 0)
    goto LABEL_13;
  if ((g_byte_91F0F3[room_index] & 1) == 0) {
    if ((g_byte_91F0F3[room_index] & 2) != 0) {
      if (!sign16(samus_y_pos - 944))
        goto LABEL_14;
    } else if ((g_byte_91F0F3[room_index] & 4) != 0) {
      goto LABEL_14;
    }
LABEL_13:
    HandleLandingGraphics_Ceres();
    return;
  }
  if (fx_type != 10)
    goto LABEL_13;
LABEL_14:
  HandleLandingGraphics_Maridia();
}

void HandleLandingGraphics_Maridia(void) {  // 0x91F116
  uint16 bottom = Samus_GetBottom_R18();
  if ((fx_y_pos & 0x8000) == 0) {
    if (sign16(fx_y_pos - bottom) && (fx_liquid_options & 4) == 0)
      return;
LABEL_7:
    atmospheric_gfx_frame_and_type[2] = 256;
    atmospheric_gfx_frame_and_type[3] = 256;
    atmospheric_gfx_anim_timer[2] = 3;
    atmospheric_gfx_anim_timer[3] = 3;
    atmospheric_gfx_x_pos[2] = samus_x_pos + 4;
    atmospheric_gfx_x_pos[3] = samus_x_pos - 3;
    atmospheric_gfx_y_pos[2] = bottom - 4;
    atmospheric_gfx_y_pos[3] = bottom - 4;
    return;
  }
  if ((lava_acid_y_pos & 0x8000) != 0 || !sign16(lava_acid_y_pos - bottom))
    goto LABEL_7;
}

void HandleLandingGraphics_Norfair(void) {  // 0x91F166
  uint16 bottom = Samus_GetBottom_R18();
  if ((fx_y_pos & 0x8000) == 0) {
    if (sign16(fx_y_pos - bottom) && (fx_liquid_options & 4) == 0)
      return;
LABEL_7:
    atmospheric_gfx_frame_and_type[2] = 1536;
    atmospheric_gfx_frame_and_type[3] = 1536;
    atmospheric_gfx_anim_timer[2] = 3;
    atmospheric_gfx_anim_timer[3] = 3;
    atmospheric_gfx_x_pos[2] = samus_x_pos + 8;
    atmospheric_gfx_x_pos[3] = samus_x_pos - 8;
    atmospheric_gfx_y_pos[2] = bottom;
    atmospheric_gfx_y_pos[3] = bottom;
    return;
  }
  if ((lava_acid_y_pos & 0x8000) != 0 || !sign16(lava_acid_y_pos - bottom))
    goto LABEL_7;
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
static const uint16 g_word_909EB5 = 1;
static const uint16 g_word_909EB7 = 0;
uint8 Samus_MorphBallBounceNoSpringballTrans(void) {  // 0x91F1FC
  int16 v0;

  v0 = 2 * used_for_ball_bounce_on_landing;
  if (2 * used_for_ball_bounce_on_landing) {
    if (v0 == 2) {
      ++used_for_ball_bounce_on_landing;
      samus_y_dir = 1;
      samus_y_subspeed = g_word_909EB7;
      samus_y_speed = g_word_909EB5 - 1;
      return 1;
    }
    if (v0 != 4)
      Unreachable();
  } else if (!sign16(samus_y_speed - 3)) {
    ++used_for_ball_bounce_on_landing;
    samus_y_dir = 1;
    samus_y_subspeed = g_word_909EB7;
    samus_y_speed = g_word_909EB5;
    return 1;
  }
  used_for_ball_bounce_on_landing = 0;
  samus_y_dir = 0;
  samus_y_subspeed = 0;
  samus_y_speed = 0;
  return 0;
}

uint8 Samus_HandleTransitionsA_5_1_2(void) {  // 0x91F253
  used_for_ball_bounce_on_landing = 0;
  enable_horiz_slope_coll = 3;
  return 0;
}

uint8 Samus_MorphBallBounceSpringballTrans(void) {  // 0x91F25E
  int16 v1;

  if ((button_config_jump_a & joypad1_lastkeys) != 0) {
    used_for_ball_bounce_on_landing = 0;
    Samus_InitJump();
    return 1;
  }
  v1 = 2 * (uint8)used_for_ball_bounce_on_landing;
  if (v1) {
    if (v1 == 2) {
      used_for_ball_bounce_on_landing = 1538;
      samus_y_dir = 1;
      samus_y_subspeed = g_word_909EB7;
      samus_y_speed = g_word_909EB5 - 1;
      return 1;
    }
    if (v1 != 4)
      Unreachable();
  } else if (!sign16(samus_y_speed - 3)) {
    used_for_ball_bounce_on_landing = 1537;
    samus_y_dir = 1;
    samus_y_subspeed = g_word_909EB7;
    samus_y_speed = g_word_909EB5;
    return 1;
  }
  used_for_ball_bounce_on_landing = 0;
  samus_y_dir = 0;
  samus_y_subspeed = 0;
  samus_y_speed = 0;
  return 0;
}

uint8 Samus_HandleTransitionsA_5_1_5(void) {  // 0x91F2CE
  used_for_ball_bounce_on_landing = 0;
  return 0;
}

void Samus_HandleTransitionsA_5_5(void) {  // 0x91F2D3
  samus_x_accel_mode = 0;
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
      enable_horiz_slope_coll = samus_pose_x_dir != 4;
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
