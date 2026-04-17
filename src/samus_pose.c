// Samus pose parameter update and animation-frame initialisation.
// Extracted from sm_91.c (Bank $91 Cluster L, L3314-L3796).
//
// When a new pose is committed, SamusFunc_F404 detects the change, calls
// collision adjustment / jump transition, re-reads pose_x_dir, and dispatches
// through SamusFunc_F468 to handle momentum carry-over, screw-attack /
// space-jump sound, shinespark movement-handler assignment, and
// animation-frame initialisation.

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"

#define kSamusTurnPose_Standing ((uint8*)RomFixedPtr(0x91f9c2))
#define kSamusTurnPose_Crouching ((uint8*)RomFixedPtr(0x91f9cc))
#define kSamusTurnPose_Jumping ((uint8*)RomFixedPtr(0x91f9d6))
#define kSamusTurnPose_Falling ((uint8*)RomFixedPtr(0x91f9e0))
#define kSamusTurnPose_Moonwalk ((uint8*)RomFixedPtr(0x91f9ea))

uint8 SamusFunc_F404(void) {  // 0x91F404
  SamusPose v1;

  v1 = samus_pose;
  if (samus_pose != samus_prev_pose) {
    HandleCollDueToChangedPose();
    SamusFunc_F433();
    HandleJumpTransition();
    Samus_SetAnimationFrameIfPoseChanged();
    samus_anim_frame_skip = 0;
  }
  return v1 != samus_pose;
}

void SamusFunc_F433(void) {  // 0x91F433
  *(uint16 *)&samus_pose_x_dir = *(uint16 *)(&kPoseParams[0].pose_x_dir + (8 * samus_pose));
  SamusFunc_F468();
  if ((samus_prev_movement_type2 == kMovementType_03_SpinJumping
       || samus_prev_movement_type2 == kMovementType_14_WallJumping)
      && (equipped_items & 8) != 0) {
    Samus_LoadSuitPalette();
  }
}

static Func_U8 *const off_91F4A2[28] = {  // 0x91F468
  SamusFunc_F468_Standing,
  SamusFunc_F468_Running,
  SamusFunc_F468_NormalJump,
  SamusFunc_F468_SpinJump,
  SamusFunc_F468_MorphBall,
  SamusFunc_F468_Crouching,
  SamusFunc_F468_Falling,
  SamusFunc_F468_Unused,
  SamusFunc_F468_MorphBall,
  SamusFunc_F468_Unused,
  SamusFunc_F468_Unused,
  SamusFunc_F468_Unused,
  SamusFunc_F468_Unused,
  SamusFunc_F468_Unused,
  SamusFunc_F468_TurningAroundOnGround,
  SamusFunc_F468_CrouchTransEtc,
  SamusFunc_F468_Moonwalking,
  SamusFunc_F468_Springball,
  SamusFunc_F468_Springball,
  SamusFunc_F468_Springball,
  SamusFunc_F468_WallJumping,
  SamusFunc_F468_Unused,
  SamusFunc_F468_Unused,
  SamusFunc_F468_TurnAroundJumping,
  SamusFunc_F468_TurnAroundFalling,
  SamusFunc_F468_DamageBoost,
  SamusFunc_F468_Unused,
  SamusFunc_F468_Shinespark,
};

void SamusFunc_F468(void) {
  if (off_91F4A2[samus_movement_type]() & 1) {
    *(uint16 *)&samus_pose_x_dir = *(uint16 *)(&kPoseParams[0].pose_x_dir + (8 * samus_pose));
    if ((*(uint16 *)&samus_pose_x_dir & 0xFF00) == 3584) {
      off_91F4A2[14]();
      *(uint16 *)&samus_pose_x_dir = *(uint16 *)(&kPoseParams[0].pose_x_dir + (8 * samus_pose));
    }
  }
}

uint8 SamusFunc_F468_Unused(void) {  // 0x91F4DA
  return 0;
}

uint8 SamusFunc_F468_Standing(void) {  // 0x91F4DC
  if ((!kPoseParams[samus_pose].direction_shots_fired
       || kPoseParams[samus_pose].direction_shots_fired == 9)
      && (!kPoseParams[samus_prev_pose].direction_shots_fired
          || kPoseParams[samus_prev_pose].direction_shots_fired == 9)) {
    samus_anim_frame_skip = 1;
  }
  return 0;
}

uint8 SamusFunc_F468_Running(void) {  // 0x91F50C
  if (samus_prev_movement_type2 == 1)
    samus_anim_frame_skip = FUNC16(Samus_InputHandler);
  if (!UNUSED_word_7E0DF8)
    return 0;
  if (samus_pose == (kPose_44_FaceL_Turn_Crouch | kPose_01_FaceR_Normal)) {
    samus_pose = kPose_25_FaceR_Turn_Stand;
    return 1;
  }
  if (samus_pose == (kPose_44_FaceL_Turn_Crouch | kPose_02_FaceL_Normal)) {
    samus_pose = kPose_26_FaceL_Turn_Stand;
    return 1;
  }
  return 0;
}

uint8 SamusFunc_F468_NormalJump(void) {  // 0x91F543
  if (samus_pose != kPose_4E_FaceL_Jump_NoAim_NoMove_NoGun) {
    if (samus_pose == kPose_4D_FaceR_Jump_NoAim_NoMove_NoGun || samus_pose == kPose_15_FaceR_Jump_AimU) {
LABEL_7:
      if (samus_shine_timer) {
        samus_pose = kPose_C7_FaceR_ShinesparkWindup_Vert;
LABEL_11:
        Projectile_Func7_Shinespark();
        if (samus_prev_movement_type2 == kMovementType_02_NormalJumping)
          samus_prev_y_pos = --samus_y_pos;
        return 1;
      }
      goto LABEL_14;
    }
    if (samus_pose != kPose_16_FaceL_Jump_AimU && samus_pose != kPose_6A_FaceL_Jump_AimUL) {
      if (samus_pose != kPose_69_FaceR_Jump_AimUR)
        goto LABEL_14;
      goto LABEL_7;
    }
  }
  if (samus_shine_timer) {
    samus_pose = kPose_C8_FaceL_ShinesparkWindup_Vert;
    goto LABEL_11;
  }
LABEL_14:
  if (samus_x_extra_run_speed || samus_x_extra_run_subspeed)
    samus_x_accel_mode = 2;
  else
    samus_x_accel_mode = 0;
  if ((samus_pose == kPose_15_FaceR_Jump_AimU || samus_pose == kPose_16_FaceL_Jump_AimU)
      && (samus_prev_pose == kPose_55_FaceR_Jumptrans_AimU || samus_prev_pose == kPose_56_FaceL_Jumptrans_AimU)) {
    samus_anim_frame_skip = 1;
  }
  if ((button_config_shoot_x & joypad1_newkeys) != 0)
    new_projectile_direction_changed_pose = kPoseParams[samus_pose].direction_shots_fired | 0x8000;
  return 0;
}

uint8 SamusFunc_F468_Crouching(void) {  // 0x91F5EB
  if ((samus_pose == kPose_85_FaceR_Crouch_AimU || samus_pose == kPose_86_FaceL_Crouch_AimU)
      && (samus_prev_pose == kPose_F1_FaceR_CrouchTrans_AimU || samus_prev_pose == kPose_F2_FaceL_CrouchTrans_AimU)) {
    samus_anim_frame_skip = 1;
  }
  return 0;
}

uint8 SamusFunc_F468_Falling(void) {  // 0x91F60D
  if (samus_x_extra_run_speed || samus_x_extra_run_subspeed)
    samus_x_accel_mode = 2;
  else
    samus_x_accel_mode = 0;
  return 0;
}

uint8 SamusFunc_F468_SpinJump(void) {  // 0x91F624
  if (samus_prev_movement_type2 == kMovementType_03_SpinJumping
      || samus_prev_movement_type2 == kMovementType_14_WallJumping) {
    samus_anim_frame_skip = 1;
    if ((samus_prev_pose_x_dir & 0xF) == 8) {
      if (*(uint16 *)&samus_pose_x_dir != 772)
        goto LABEL_9;
    } else if ((samus_prev_pose_x_dir & 0xF) != 4 || *(uint16 *)&samus_pose_x_dir != 776) {
      goto LABEL_9;
    }
    AddToHiLo(&samus_x_base_speed, &samus_x_base_subspeed, __PAIR32__(samus_x_extra_run_speed, samus_x_extra_run_subspeed));
    Samus_CancelSpeedBoost();
    SetHiLo(&samus_x_extra_run_speed, &samus_x_extra_run_subspeed, 0);
    samus_x_accel_mode = 1;
  }
LABEL_9:
  if (samus_pose_x_dir == 4) {
    if ((equipped_items & 0x20) == 0) {
      uint16 r20 = Samus_GetTop_R20();
      if ((fx_y_pos & 0x8000) != 0) {
        if ((lava_acid_y_pos & 0x8000) == 0 && sign16(lava_acid_y_pos - r20))
          return 0;
      } else if (sign16(fx_y_pos - r20) && (fx_liquid_options & 4) == 0) {
        return 0;
      }
    }
    if ((equipped_items & 8) != 0) {
      samus_pose = kPose_82_FaceL_Screwattack;
      goto LABEL_40;
    }
    if ((equipped_items & 0x200) != 0) {
      QueueSfx1_Max6(0x3E);
      samus_pose = kPose_1C_FaceL_SpaceJump;
      return 0;
    }
    if (!samus_anim_frame_skip && !cinematic_function) {
LABEL_22:
      QueueSfx1_Max6(0x31);
      return 0;
    }
    return 0;
  }
  if ((equipped_items & 0x20) == 0) {
    uint16 r20 = Samus_GetTop_R20();
    if ((fx_y_pos & 0x8000) != 0) {
      if ((lava_acid_y_pos & 0x8000) == 0 && sign16(lava_acid_y_pos - r20))
        return 0;
    } else if (sign16(fx_y_pos - r20) && (fx_liquid_options & 4) == 0) {
      return 0;
    }
  }
  if ((equipped_items & 8) != 0) {
    samus_pose = kPose_81_FaceR_Screwattack;
LABEL_40:
    if (!samus_anim_frame_skip)
      QueueSfx1_Max6(0x33);
    return 0;
  }
  if ((equipped_items & 0x200) == 0) {
    if (samus_anim_frame_skip || cinematic_function)
      return 0;
    goto LABEL_22;
  }
  QueueSfx1_Max6(0x3E);
  samus_pose = kPose_1B_FaceR_SpaceJump;
  return 0;
}

static Func_U8 *const off_91F790[12] = {  // 0x91F758
  Samus_CrouchTrans,
  Samus_CrouchTrans,
  Samus_MorphTrans,
  Samus_MorphTrans,
  MaybeUnused_sub_91F7F4,
  MaybeUnused_sub_91F840,
  Samus_StandOrUnmorphTrans,
  Samus_StandOrUnmorphTrans,
  Samus_StandOrUnmorphTrans,
  Samus_StandOrUnmorphTrans,
  Samus_StandOrUnmorphTrans,
  Samus_StandOrUnmorphTrans,
};

static Func_U8 *const off_91F7A8[4] = {
  Samus_MorphTrans,
  Samus_MorphTrans,
  Samus_StandOrUnmorphTrans,
  Samus_StandOrUnmorphTrans,
};

uint8 SamusFunc_F468_CrouchTransEtc(void) {
  uint16 v0;
  if (sign16(samus_pose - kPose_F1_FaceR_CrouchTrans_AimU)) {
    if (sign16(samus_pose - kPose_DB)) {
      v0 = 2 * (samus_pose - 53);
LABEL_4:
      samus_momentum_routine_index = 7;
      return off_91F790[v0 >> 1]();
    }
    return off_91F7A8[samus_pose - 219]();
  } else {
    if (sign16(samus_pose - kPose_F7_FaceR_StandTrans_AimU)) {
      v0 = 0;
      goto LABEL_4;
    }
    samus_momentum_routine_index = 7;
  }
  return 0;
}

uint8 Samus_CrouchTrans(void) {  // 0x91F7B0
  if (!sign16((speed_boost_counter & 0xFF00) - 1024)) {
    samus_shine_timer = 180;
    timer_for_shine_timer = 1;
    special_samus_palette_frame = 0;
  }
  return 0;
}

uint8 Samus_StandOrUnmorphTrans(void) {  // 0x91F7CC
  return 0;
}

uint8 Samus_MorphTrans(void) {  // 0x91F7CE
  if ((equipped_items & 4) != 0) {
    if (samus_prev_movement_type2 == kMovementType_03_SpinJumping)
      samus_x_accel_mode = 2;
    bomb_spread_charge_timeout_counter = 0;
    return 0;
  } else {
    samus_pose = samus_prev_pose;
    return 1;
  }
}

uint8 MaybeUnused_sub_91F7F4(void) {  // 0x91F7F4
  if (samus_prev_movement_type2 == kMovementType_08_MorphBallFalling
      || samus_prev_movement_type2 == kMovementType_13_SpringBallFalling) {
    if ((equipped_items & 2) != 0)
      samus_pose = kPose_7D_FaceR_Springball_Fall;
    else
      samus_pose = kPose_31_FaceR_Morphball_Air;
  } else if ((equipped_items & 2) != 0) {
    samus_pose = kPose_79_FaceR_Springball_Ground;
  } else {
    samus_pose = kPose_1D_FaceR_Morphball_Ground;
  }
  return 1;
}

uint8 MaybeUnused_sub_91F840(void) {  // 0x91F840
  if (samus_prev_movement_type2 == kMovementType_08_MorphBallFalling
      || samus_prev_movement_type2 == kMovementType_13_SpringBallFalling) {
    if ((equipped_items & 2) != 0)
      samus_pose = kPose_7E_FaceL_Springball_Fall;
    else
      samus_pose = kPose_32_FaceL_Morphball_Air;
  } else if ((equipped_items & 2) != 0) {
    samus_pose = kPose_7A_FaceL_Springball_Ground;
  } else {
    samus_pose = kPose_41_FaceL_Morphball_Ground;
  }
  return 1;
}

uint8 SamusFunc_F468_Moonwalking(void) {  // 0x91F88C
  if (moonwalk_flag)
    return 0;
  if (samus_pose_x_dir == 4)
    samus_pose = kPose_25_FaceR_Turn_Stand;
  else
    samus_pose = kPose_26_FaceL_Turn_Stand;
  return 1;
}

uint8 SamusFunc_F468_DamageBoost(void) {  // 0x91F8AE
  return SamusFunc_F468_DamageBoost_();
}

uint8 MaybeUnused_sub_91F8B0(void) {  // 0x91F8B0
  if (samus_pose_x_dir == 4)
    samus_pose = kPose_54_FaceL_Knockback;
  else
    samus_pose = kPose_53_FaceR_Knockback;
  return 1;
}

uint8 SamusFunc_F468_DamageBoost_(void) {  // 0x91F8CB
  samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
  return 0;
}

uint8 SamusFunc_F468_TurningAroundOnGround(void) {  // 0x91F8D3
  if (samus_prev_pose && samus_prev_pose != kPose_9B_FaceF_VariaGravitySuit) {
    uint16 v0 = kPoseParams[samus_prev_pose].direction_shots_fired;
    if (samus_prev_movement_type2 == kMovementType_10_Moonwalking) {
      new_projectile_direction_changed_pose = v0 | 0x100;
      if ((button_config_jump_a & joypad1_lastkeys) != 0) {
        samus_pose = kSamusTurnPose_Moonwalk[v0];
      } else {
        samus_pose = kSamusTurnPose_Standing[v0];
      }
    } else if (samus_prev_movement_type2 == 5) {
      samus_pose = kSamusTurnPose_Crouching[v0];
    } else {
      samus_pose = kSamusTurnPose_Standing[v0];
    }
  }
  AddToHiLo(&samus_x_base_speed, &samus_x_base_subspeed, __PAIR32__(samus_x_extra_run_speed, samus_x_extra_run_subspeed));
  SetHiLo(&samus_x_extra_run_speed, &samus_x_extra_run_subspeed, 0);
  samus_x_accel_mode = 1;
  return 1;
}

uint8 SamusFunc_F468_TurnAroundJumping(void) {  // 0x91F952
  samus_pose = kSamusTurnPose_Jumping[kPoseParams[samus_prev_pose].direction_shots_fired];
  AddToHiLo(&samus_x_base_speed, &samus_x_base_subspeed, __PAIR32__(samus_x_extra_run_speed, samus_x_extra_run_subspeed));
  SetHiLo(&samus_x_extra_run_speed, &samus_x_extra_run_subspeed, 0);
  samus_x_accel_mode = 1;
  return 1;
}

uint8 SamusFunc_F468_TurnAroundFalling(void) {  // 0x91F98A
  samus_pose = kSamusTurnPose_Falling[kPoseParams[samus_prev_pose].direction_shots_fired];
  AddToHiLo(&samus_x_base_speed, &samus_x_base_subspeed, __PAIR32__(samus_x_extra_run_speed, samus_x_extra_run_subspeed));
  SetHiLo(&samus_x_extra_run_speed, &samus_x_extra_run_subspeed, 0);
  samus_x_accel_mode = 1;
  return 1;
}

uint8 SamusFunc_F468_MorphBall(void) {  // 0x91F9F4
  if (samus_prev_movement_type2 == kMovementType_04_MorphBallOnGround
      || samus_prev_movement_type2 == kMovementType_08_MorphBallFalling) {
    samus_anim_frame_skip = FUNC16(Samus_InputHandler);
  }
  SamusFunc_FA0A();
  return 0;
}

void SamusFunc_FA0A(void) {  // 0x91FA0F
  if (samus_prev_pose_x_dir == 8) {
    if (samus_pose_x_dir != 4)
      return;
    goto LABEL_5;
  }
  if (samus_pose_x_dir == 8) {
LABEL_5:;
    AddToHiLo(&samus_x_base_speed, &samus_x_base_subspeed, __PAIR32__(samus_x_extra_run_speed, samus_x_extra_run_subspeed));
    Samus_CancelSpeedBoost();
    SetHiLo(&samus_x_extra_run_speed, &samus_x_extra_run_subspeed, 0);
    samus_x_accel_mode = 1;
  }
}

uint8 SamusFunc_F468_Springball(void) {  // 0x91FA56
  if (samus_prev_movement_type2 == kMovementType_11_SpringBallOnGround
      || samus_prev_movement_type2 == kMovementType_12_SpringBallInAir
      || samus_prev_movement_type2 == kMovementType_13_SpringBallFalling) {
    samus_anim_frame_skip = FUNC16(Samus_InputHandler);
  }
  SamusFunc_FA0A();
  return 0;
}

uint8 SamusFunc_F468_WallJumping(void) {  // 0x91FA76
  uint16 bottom = Samus_GetBottom_R18();
  if ((fx_y_pos & 0x8000) != 0) {
    if ((lava_acid_y_pos & 0x8000) == 0 && sign16(lava_acid_y_pos - bottom))
      return 0;
  } else if (sign16(fx_y_pos - bottom) && (fx_liquid_options & 4) == 0) {
    return 0;
  }
  atmospheric_gfx_frame_and_type[3] = 1536;
  atmospheric_gfx_anim_timer[3] = 3;
  atmospheric_gfx_y_pos[3] = bottom;
  if (samus_pose_x_dir != 8) {
    atmospheric_gfx_x_pos[3] = samus_x_pos + 6;
    return 0;
  }
  atmospheric_gfx_x_pos[3] = samus_x_pos - 6;
  return 0;
}

static uint16 kSamusFunc_F468_Shinespark[6] = {  // 0x91FACA
  (uint16)fnSamus_MoveHandler_Shinespark_Horiz,
  (uint16)fnSamus_MoveHandler_Shinespark_Horiz,
  (uint16)fnSamus_MoveHandlerVerticalShinespark,
  (uint16)fnSamus_MoveHandlerVerticalShinespark,
  (uint16)fnSamus_MoveHandler_Shinespark_Diag,
  (uint16)fnSamus_MoveHandler_Shinespark_Diag,
};
uint8 SamusFunc_F468_Shinespark(void) {
  if (sign16(samus_pose - kPose_CF_FaceR_Ranintowall_AimUR)) {
    samus_movement_handler = kSamusFunc_F468_Shinespark[samus_pose - 201];
    samus_input_handler = FUNC16(nullsub_152);
    speed_echoes_index = 0;
    speed_echo_xspeed[0] = 0;
    speed_echo_xspeed[1] = 0;
    speed_echo_xpos[0] = 0;
    speed_echo_xpos[1] = 0;
    QueueSfx3_Max9(0xF);
  }
  return 0;
}
static const uint16 kSamusPhys_AnimDelayInWater = 3;
static const uint16 kSamusPhys_AnimDelayInAcid = 2;

void Samus_SetAnimationFrameIfPoseChanged(void) {  // 0x91FB08
  uint16 t;
  if ((equipped_items & 0x20) != 0) {
    t = samus_x_speed_divisor;
  } else {
    uint16 r = samus_y_pos + kPoseParams[samus_pose].y_radius - 1;
    if ((fx_y_pos & 0x8000) == 0) {
      t = (sign16(fx_y_pos - r) && (fx_liquid_options & 4) == 0) ? kSamusPhys_AnimDelayInWater : samus_x_speed_divisor;
    } else if ((lava_acid_y_pos & 0x8000) != 0 || !sign16(lava_acid_y_pos - r)) {
      t = samus_x_speed_divisor;
    } else {
      t = kSamusPhys_AnimDelayInAcid;
    }
  }
  if ((samus_anim_frame_skip & 0x8000) == 0 && samus_pose != samus_prev_pose) {
    samus_anim_frame = samus_anim_frame_skip;
    samus_anim_frame_timer = t + *RomPtr_91(kSamusAnimationDelayData[samus_pose] + samus_anim_frame_skip);
  }
}

void SamusFunc_EC80(void) {  // 0x91FB8E
  if (samus_prev_movement_type2 != kMovementType_06_Falling && samus_movement_type == kMovementType_06_Falling) {
    samus_y_subspeed = 0;
    samus_y_speed = 0;
    samus_y_dir = 2;
  }
}
