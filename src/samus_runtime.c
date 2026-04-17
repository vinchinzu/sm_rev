// Samus runtime/state helpers extracted from sm_90.c: frame-handler
// dispatch, input/movement shims, damage/pause checks, and SamusCode state
// routines that coordinate the main per-frame control flow.

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"

static const uint8 kSamus_MoveDown_SetPoseCalcInput_Tab0[28] = {  // 0x90E61B
  0, 0, 4, 4, 1, 0, 4, 2,
  4, 4, 0, 0, 0, 0, 4, 4,
  0, 3, 4, 4, 4, 0, 4, 4,
  4, 4, 4, 4,
};

static const uint8 kSamus_MoveDown_SetPoseCalcInput_Tab1[28] = {
  4, 4, 0, 0, 4, 4, 0, 4,
  1, 2, 0, 4, 4, 0, 4, 4,
  4, 4, 3, 3, 0, 4, 4, 4,
  4, 0, 4, 4,
};

static void CallFrameHandlerGamma(uint32 ea) {
  switch (ea) {
  case fnSamus_Func1: Samus_Func1(); return;
  case fnSamus_Func2: Samus_Func2(); return;
  case fnSamus_Func3: Samus_Func3(); return;
  case fnDrawTimer_: DrawTimer_(); return;
  case fnSamus_PushOutOfRidleysWay: Samus_PushOutOfRidleysWay(); return;
  case fnSamus_Func4: Samus_Func4(); return;
  case fnSamus_GrabbedByDraygonFrameHandler: Samus_GrabbedByDraygonFrameHandler(); return;
  case fnnullsub_151: return;
  case fnSamus_Func7: Samus_Func7(); return;
  case fnSamus_Func9: Samus_Func9(); return;
  case fnnullsub_152: return;
  default: Unreachable();
  }
}

static void CallFrameHandlerAlfa(uint32 ea) {
  switch (ea) {
  case fnSamus_FrameHandlerAlfa_Func11: Samus_FrameHandlerAlfa_Func11(); return;
  case fnSamus_FrameHandlerAlfa_Func12: Samus_FrameHandlerAlfa_Func12(); return;
  case fnSamus_FrameHandlerAlfa_Func13: Samus_FrameHandlerAlfa_Func13(); return;
  case fnEmptyFunction: return;
  default: Unreachable();
  }
}

static void CallFrameHandlerBeta(uint32 ea) {
  switch (ea) {
  case fnSamus_FrameHandlerBeta_Func17: Samus_FrameHandlerBeta_Func17(); return;
  case fnHandleDemoRecorder_3: HandleDemoRecorder_3(); return;
  case fnSamus_FrameHandlerBeta_Func14: Samus_FrameHandlerBeta_Func14(); return;
  case fnSamus_Func15: Samus_Func15(); return;
  case fnSamus_Func16: Samus_Func16(); return;
  case fnSamus_Func18: Samus_Func18(); return;
  case fnEmptyFunction: return;
  case fnj_HandleDemoRecorder_2: return;
  case fnj_HandleDemoRecorder_2_0: return;
  case fnSetContactDamageIndexAndUpdateMinimap: SetContactDamageIndexAndUpdateMinimap(); return;
  case fnSamus_Func19: Samus_Func19(); return;
  case fnSamus_LowHealthCheck: Samus_LowHealthCheck(); return;
  default: Unreachable();
  }
}

static void CallSamusInputHandler(uint32 ea) {
  switch (ea) {
  case fnnullsub_152: return;
  case fnSamus_InputHandler_E913: Samus_InputHandler_E913(); return;
  case fnSamus_Func20_: Samus_Func20_(); return;
  case fnSamus_InputHandler_E91D: Samus_InputHandler_E91D(); return;
  case fnHandleAutoJumpHack: HandleAutoJumpHack(); return;
  default: Unreachable();
  }
}

static void CallSamusMovementHandler(uint32 ea) {
  switch (ea) {
  case fnSamus_MoveHandler_ReleaseFromGrapple: Samus_MoveHandler_ReleaseFromGrapple(); return;
  case fnSamus_HandleMovement_DrainedCrouching: Samus_HandleMovement_DrainedCrouching(); return;
  case fnSamus_MovementHandler_Normal: Samus_MovementHandler_Normal(); return;
  case fnSamus_MoveHandlerShinesparkWindup: Samus_MoveHandlerShinesparkWindup(); return;
  case fnSamus_MoveHandlerVerticalShinespark: Samus_MoveHandlerVerticalShinespark(); return;
  case fnSamus_MoveHandler_Shinespark_Diag: Samus_MoveHandler_Shinespark_Diag(); return;
  case fnSamus_MoveHandler_Shinespark_Horiz: Samus_MoveHandler_Shinespark_Horiz(); return;
  case fnSamus_MoveHandler_ShinesparkCrash: Samus_MoveHandler_ShinesparkCrash(); return;
  case fnSamus_MoveHandler_ShinesparkCrash_0: Samus_MoveHandler_ShinesparkCrash_0(); return;
  case fnSamus_MoveHandler_ShinesparkCrash_1: Samus_MoveHandler_ShinesparkCrash_1(); return;
  case fnSamus_MoveHandler_ShinesparkCrash_2: Samus_MoveHandler_ShinesparkCrash_2(); return;
  case fnSamus_MoveHandler_ShinesparkCrashEchoCircle: Samus_MoveHandler_ShinesparkCrashEchoCircle(); return;
  case fnSamus_MoveHandler_ShinesparkCrashFinish: Samus_MoveHandler_ShinesparkCrashFinish(); return;
  case fnSamusMoveHandler_CrystalFlashStart: SamusMoveHandler_CrystalFlashStart(); return;
  case fnSamusMoveHandler_CrystalFlashMain: SamusMoveHandler_CrystalFlashMain(); return;
  case fnkSamusMoveHandler_CrystalFlashFinish: kSamusMoveHandler_CrystalFlashFinish(); return;
  case fnSamus_MoveHandler_Knockback: Samus_MoveHandler_Knockback(); return;
  case fnSamus_MoveHandler_Knockback_0: Samus_MoveHandler_Knockback_0(); return;
  case fnSamus_MoveHandler_Knockback_Up: Samus_MoveHandler_Knockback_Up(); return;
  case fnSamus_MoveHandler_Knockback_3: Samus_MoveHandler_Knockback_3(); return;
  case fnSamus_MoveHandler_Knockback_Down: Samus_MoveHandler_Knockback_Down(); return;
  case fnSamus_MoveHandler_BombJumpStart: Samus_MoveHandler_BombJumpStart(); return;
  case fnSamus_MoveHandler_BombJumpMain: Samus_MoveHandler_BombJumpMain(); return;
  case fnSamus_MoveHandler_BombJumpFunc1: Samus_MoveHandler_BombJumpFunc1(); return;
  case fnnullsub_152: return;
  case fnSamusMovementType_Xray: SamusMovementType_Xray(); return;
  case fnSamus_Func25_ShineSpark: Samus_Func25_ShineSpark(); return;
  case fnSamus_MoveHandler_F072: Samus_MoveHandler_F072(); return;
  default: Unreachable();
  }
}

static Func_U8 *const kSamusCodeHandlers[32] = {  // 0x90F084
  SamusCode_00_LockSamus,
  SamusCode_01_UnlockSamus,
  SamusCode_02_ReachCeresElevator,
  SamusCode_03,
  SamusCode_04,
  SamusCode_05_SetupDrained,
  SamusCode_06_LockToStation,
  SamusCode_07_SetupForElevator,
  SamusCode_08_SetupForCeresStart,
  SamusCode_08_SetupForZebesStart,
  SamusCode_0A_ClearDrawHandler,
  SamusCode_0B_DrawHandlerDefault,
  SamusCode_0C_UnlockFromMapStation,
  SamusCode_0D_IsGrappleActive,
  SamusCode_0E,
  SamusCode_0F_EnableTimerHandling,
  SamusCode_10,
  SamusCode_11_SetupForDeath,
  SamusCode_12_SetSuperPaletteFlag1,
  SamusCode_12_SetSuperPaletteFlag0,
  SamusCode_14,
  SamusCode_15_CalledBySuitAcquision,
  SamusCode_16,
  SamusCode_17_DisableRainbowSamusAndStandUp,
  SamusCode_18,
  SamusCode_17_FreezeDrainedSamus,
  SamusCode_1A,
  SamusCode_1B_CheckedLockSamus,
  SamusCode_1C,
  SamusCode_1D_ClearSoundInDoor,
  SamusCode_1E,
  SamusCode_1F,
};

void RunFrameHandlerGamma(void) {  // 0x90E097
  CallFrameHandlerGamma(frame_handler_gamma | 0x900000);
}

void Samus_Func1(void) {  // 0x90E09B
  if (samus_pose == kPose_E9_FaceL_Drained_CrouchFalling
      && !sign16(samus_anim_frame - 8)
      && (joypad1_newkeys & kButton_Up) != 0) {
    samus_anim_frame_timer = 1;
    samus_anim_frame = 13;
    frame_handler_gamma = FUNC16(nullsub_152);
  }
}

void Samus_Func2(void) {  // 0x90E0C5
  if (!sign16(samus_anim_frame - 8) && sign16(samus_anim_frame - 12) && (joypad1_newkeys & kButton_Up) != 0) {
    samus_anim_frame_timer = 1;
    samus_anim_frame = 18;
  }
}

void Samus_Func3(void) {  // 0x90E0E6
  if (ProcessTimer() & 1) {
    game_state = kGameState_35_TimeUp;
    for (int i = 510; i >= 0; i -= 2)
      target_palettes[i >> 1] = 0x7FFF;
    frame_handler_gamma = FUNC16(DrawTimer_);
    DisablePaletteFx();
  }
  if (timer_status)
    DrawTimer();
}

void DrawTimer_(void) {  // 0x90E114
  DrawTimer();
}

void Samus_SetPushedOutOfCeresRidley(void) {  // 0x90E119
  samus_movement_handler = FUNC16(nullsub_152);
  frame_handler_gamma = FUNC16(Samus_PushOutOfRidleysWay);
}

void Samus_PushOutOfRidleysWay(void) {  // 0x90E12E
  if (samus_pose_x_dir == 4)
    samus_pose = kPose_54_FaceL_Knockback;
  else
    samus_pose = kPose_53_FaceR_Knockback;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
  samus_last_different_pose = samus_prev_pose;
  *(uint16 *)&samus_last_different_pose_x_dir = *(uint16 *)&samus_prev_pose_x_dir;
  samus_prev_pose = samus_pose;
  *(uint16 *)&samus_prev_pose_x_dir = *(uint16 *)&samus_pose_x_dir;
  samus_y_pos -= 21 - samus_y_radius;
  if (sign16(samus_x_pos - layer1_x_pos - 128))
    samus_var62 = 1;
  else
    samus_var62 = 2;
  samus_y_speed = 5;
  samus_y_subspeed = 0;
  bomb_jump_dir = 0;
  frame_handler_gamma = FUNC16(Samus_Func4);
  samus_new_pose = -1;
  samus_new_pose_interrupted = -1;
  samus_new_pose_transitional = -1;
  samus_momentum_routine_index = 0;
  samus_special_transgfx_index = 0;
  samus_hurt_switch_index = 0;
  ProcessTimer();
  if (timer_status)
    DrawTimer();
}

void Samus_Func4(void) {  // 0x90E1C8
  static Func_V *const off_90E1F7[3] = {
    0,
    Samus_Func5,
    Samus_Func6,
  };

  if (samus_new_pose == kPose_4F_FaceL_Dmgboost || samus_new_pose == kPose_50_FaceR_Dmgboost) {
    samus_new_pose = -1;
    samus_momentum_routine_index = 0;
  }
  off_90E1F7[samus_var62]();
  input_to_pose_calc = 0;
  ProcessTimer();
  if (timer_status)
    DrawTimer();
}

void Samus_Func5(void) {  // 0x90E1FD
  Samus_BombJumpFallingXMovement_();
  if (samus_collision_flag) {
    samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
    frame_handler_gamma = FUNC16(Samus_Func3);
    samus_var62 = 0;
    Samus_ClearMoveVars();
  } else {
    Samus_BombJumpFallingYMovement_();
  }
}

void Samus_Func6(void) {  // 0x90E21C
  Samus_BombJumpFallingXMovement_();
  if (samus_collision_flag) {
    samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
    frame_handler_gamma = FUNC16(Samus_Func3);
    samus_var62 = 0;
    Samus_ClearMoveVars();
  } else {
    Samus_BombJumpFallingYMovement_();
  }
}

void Samus_GrabbedByDraygonFrameHandler(void) {  // 0x90E2A1
  if (grapple_beam_function == FUNC16(GrappleBeamFunc_ConnectedLockedInPlace)) {
    samus_new_pose = -1;
    samus_momentum_routine_index = 0;
  }
  if ((joypad1_newkeys & (kButton_Up | kButton_Down | kButton_Left | kButton_Right)) != 0
      && (joypad1_newkeys & (kButton_Up | kButton_Down | kButton_Left | kButton_Right)) != suit_pickup_light_beam_pos) {
    suit_pickup_light_beam_pos = joypad1_newkeys & (kButton_Up | kButton_Down | kButton_Left | kButton_Right);
    if ((int16)(++substate - 60) >= 0)
      Samus_ReleaseFromDraygon();
  }
}

void Samus_SetGrabbedByDraygonPose(uint16 a) {  // 0x90E23B
  if ((a & 1) != 0)
    samus_pose = kPose_EC_FaceR_Draygon_NoMove_NoAim;
  else
    samus_pose = kPose_BA_FaceL_Draygon_NoMove_NoAim;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
  frame_handler_gamma = FUNC16(Samus_GrabbedByDraygonFrameHandler);
  samus_movement_handler = FUNC16(nullsub_152);
  substate = 0;
  suit_pickup_light_beam_pos = 0;
  *(uint16 *)&suit_pickup_color_math_R = 0;
  samus_last_different_pose = samus_prev_pose;
  *(uint16 *)&samus_last_different_pose_x_dir = *(uint16 *)&samus_prev_pose_x_dir;
  samus_prev_pose = samus_pose;
  *(uint16 *)&samus_prev_pose_x_dir = *(uint16 *)&samus_pose_x_dir;
  samus_new_pose = -1;
  samus_new_pose_interrupted = -1;
  samus_new_pose_transitional = -1;
  samus_momentum_routine_index = 0;
  samus_special_transgfx_index = 0;
  samus_hurt_switch_index = 0;
}

void Samus_ReleaseFromDraygon_(void) {  // 0x90E2D4
  Samus_ReleaseFromDraygon();
}

void Samus_ReleaseFromDraygon(void) {  // 0x90E2DE
  if (samus_pose_x_dir == 4)
    samus_pose = kPose_02_FaceL_Normal;
  else
    samus_pose = kPose_01_FaceR_Normal;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
  samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
  frame_handler_gamma = FUNC16(nullsub_152);
  samus_last_different_pose = samus_prev_pose;
  *(uint16 *)&samus_last_different_pose_x_dir = *(uint16 *)&samus_prev_pose_x_dir;
  samus_prev_pose = samus_pose;
  *(uint16 *)&samus_prev_pose_x_dir = *(uint16 *)&samus_pose_x_dir;
  samus_x_base_speed = 0;
  samus_x_base_subspeed = 0;
  samus_new_pose = -1;
  samus_new_pose_interrupted = -1;
  samus_new_pose_transitional = -1;
  samus_momentum_routine_index = 0;
  samus_special_transgfx_index = 0;
  samus_hurt_switch_index = 0;
  samus_y_speed = 0;
  samus_y_subspeed = 0;
  samus_y_dir = 0;
  used_for_ball_bounce_on_landing = 0;
  samus_x_accel_mode = 0;
  samus_grapple_flags = samus_grapple_flags & 0xFFFD | 2;
}

void Samus_Func7(void) {  // 0x90E3A3
  Samus_BombJumpFallingXMovement_();
  if (samus_collision_flag || (Samus_BombJumpFallingYMovement_(), samus_collision_flag)) {
    samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
    frame_handler_gamma = FUNC16(nullsub_152);
    samus_var62 = 0;
    Samus_ClearMoveVars();
    samus_new_pose_interrupted = 65;
    samus_special_transgfx_index = 0;
  }
}

void Samus_Func8(void) {  // 0x90E400
  samus_y_subspeed = 0;
  samus_y_speed = 0;
  samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
  frame_handler_gamma = FUNC16(nullsub_152);
}

void Samus_Func9(void) {  // 0x90E41B
  if (sign16(samus_y_speed - 5))
    AddToHiLo(&samus_y_speed, &samus_y_subspeed, __PAIR32__(samus_y_accel, samus_y_subaccel));
  if ((samus_pose == kPose_29_FaceR_Fall || samus_pose == kPose_2A_FaceL_Fall
       || samus_pose == kPose_67_FaceR_Fall_Gun || samus_pose == kPose_68_FaceL_Fall_Gun)
      && !sign16(samus_y_speed - 5)) {
    samus_anim_frame_timer = 16;
    samus_anim_frame = 4;
  }
}

uint32 Samus_CalcSpeed_X(uint32 amt) {  // 0x90E4E6
  amt += __PAIR32__(samus_x_extra_run_speed, samus_x_extra_run_subspeed);
  amt >>= (samus_x_speed_divisor <= 4) ? samus_x_speed_divisor : 4;
  SetHiLo(&samus_total_x_speed, &samus_total_x_subspeed, amt);
  return amt;
}

void Samus_MoveUp_SetPoseCalcInput(void) {  // 0x90E606
  if (samus_collision_flag)
    input_to_pose_calc = 4;
  else
    input_to_pose_calc = 0;
}

void Samus_MoveDown_SetPoseCalcInput(void) {
  if (samus_collision_flag) {
    input_to_pose_calc = 1;
    HIBYTE(input_to_pose_calc) = kSamus_MoveDown_SetPoseCalcInput_Tab1[samus_movement_type];
  } else if ((uint8)input_to_pose_calc != 5) {
    input_to_pose_calc = 2;
    HIBYTE(input_to_pose_calc) = kSamus_MoveDown_SetPoseCalcInput_Tab0[samus_movement_type];
  }
}

void HandleControllerInputForGamePhysics(void) {  // 0x90E692
  CallFrameHandlerAlfa(frame_handler_alfa | 0x900000);
}

void Samus_FrameHandlerAlfa_Func11(void) {  // 0x90E695
  samus_new_pose = -1;
  samus_new_pose_interrupted = -1;
  samus_new_pose_transitional = -1;
  samus_momentum_routine_index = 0;
  samus_special_transgfx_index = 0;
  samus_hurt_switch_index = 0;
  Samus_SetRadius();
  Samus_CallInputHandler();
  Samus_UpdateSuitPaletteIndex();
  Samus_DetermineAccel_Y();
  BlockInsideDetection();
  Samus_HandleHudSpecificBehaviorAndProjs();
  Samus_Func10();
}

void Samus_FrameHandlerAlfa_Func12(void) {  // 0x90E6C9
  samus_new_pose = -1;
  samus_new_pose_interrupted = -1;
  samus_new_pose_transitional = -1;
  samus_momentum_routine_index = 0;
  samus_special_transgfx_index = 0;
  samus_hurt_switch_index = 0;
  controller1_input_for_demo = joypad1_lastkeys;
  controller1_new_input_for_demo = joypad1_newkeys;
  demo_backup_prev_controller_input = joypad1_input_samusfilter;
  demo_backup_prev_controller_input_new = joypad1_newinput_samusfilter;
  Samus_SetRadius();
  Samus_UpdateSuitPaletteIndex();
  Samus_CallInputHandler();
  Samus_DetermineAccel_Y();
  BlockInsideDetection();
  Samus_HandleHudSpecificBehaviorAndProjs();
  Samus_Func10();
}

void Samus_FrameHandlerAlfa_Func13(void) {  // 0x90E713
  HandleProjectile();
  Samus_Func10();
}

void HandleSamusMovementAndPause(void) {  // 0x90E722
  CallFrameHandlerBeta(frame_handler_beta | 0x900000);
}

void Samus_FrameHandlerBeta_Func17(void) {  // 0x90E725
  samus_contact_damage_index = 0;
  RunSamusMovementHandler();
  UpdateMinimap();
  RunFrameHandlerGamma();
  Samus_Animate();
  Samus_HitInterruption();
  Samus_HandleTransFromBlockColl();
  Samus_HandleTransitions();
  Samus_HandlePalette();
  Samus_HandlePeriodicDamage();
  Samus_PauseCheck();
  Samus_LowHealthCheck_();
}

void HandleDemoRecorder_1(void) {  // 0x90E786
  if ((joypad2_new_keys & 0x8000) == 0) {
    if (!debug_flag && (joypad2_new_keys & 0x80) != 0) {
      DisableEprojs();
      time_is_frozen_flag = 1;
      frame_handler_alfa = FUNC16(EmptyFunction);
      frame_handler_beta = FUNC16(HandleDemoRecorder_3);
    }
  } else if (debug_flag) {
    samus_draw_handler = FUNC16(nullsub_152);
    debug_flag = 0;
  } else {
    debug_flag = 1;
    samus_draw_handler = FUNC16(SamusDrawHandler_Default);
  }
}

void HandleDemoRecorder_3(void) {  // 0x90E7D2
  if ((joypad2_new_keys & 0x80) != 0) {
    frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func11);
    frame_handler_beta = FUNC16(Samus_FrameHandlerBeta_Func17);
    EnableEprojs();
    time_is_frozen_flag = 0;
  }
}

void Samus_FrameHandlerBeta_Func14(void) {  // 0x90E7F5
  samus_contact_damage_index = 0;
  RunSamusMovementHandler();
  UpdateMinimap();
  Samus_Animate();
  Samus_HitInterruption();
  Samus_HandleTransFromBlockColl();
  Samus_HandleTransitions();
  Samus_HandlePalette();
  joypad1_lastkeys = controller1_input_for_demo;
  joypad1_newkeys = controller1_new_input_for_demo;
  joypad1_input_samusfilter = demo_backup_prev_controller_input;
  joypad1_newinput_samusfilter = demo_backup_prev_controller_input_new;
}

void Samus_Func15(void) {  // 0x90E833
  samus_contact_damage_index = 0;
  RunSamusMovementHandler();
  Samus_Animate();
  Samus_HitInterruption();
  Samus_HandleTransFromBlockColl();
  Samus_HandleTransitions();
  Samus_HandlePalette();
  joypad1_lastkeys = controller1_input_for_demo;
  joypad1_newkeys = controller1_new_input_for_demo;
  joypad1_input_samusfilter = demo_backup_prev_controller_input;
  joypad1_newinput_samusfilter = demo_backup_prev_controller_input_new;
}

void Samus_Func16(void) {  // 0x90E86A
  Samus_SetRadius();
  UpdateMinimap();
  Samus_Animate();
  elevator_status = 0;
  samus_prev_y_pos = samus_y_pos;
  if (PlaySamusFanfare() & 1) {
    if (sign16(debug_invincibility - 7) || (joypad2_last & 0x8000) == 0)
      debug_invincibility = 0;
    frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func11);
    frame_handler_beta = FUNC16(Samus_FrameHandlerBeta_Func17);
  }
}

void Samus_Func18(void) {  // 0x90E8AA
  Samus_FrameHandlerBeta_Func17();
  if (frame_handler_gamma == FUNC16(DrawTimer_) && game_state != kGameState_35_TimeUp)
    game_state = kGameState_35_TimeUp;
}

void Samus_Func19(void) {  // 0x90E8EC
  samus_contact_damage_index = 0;
  RunSamusMovementHandler();
  UpdateMinimap();
  Samus_Animate();
}

void Samus_LowHealthCheck(void) {  // 0x90E902
  Samus_LowHealthCheck_();
}

void Samus_SetRadius(void) {  // 0x90EC22
  samus_y_radius = kPoseParams[samus_pose].y_radius;
  samus_x_radius = 5;
}

uint16 Samus_GetBottom_R18(void) {
  return samus_y_pos + kPoseParams[samus_pose].y_radius - 1;
}

uint16 Samus_GetTop_R20(void) {
  return samus_y_pos - kPoseParams[samus_pose].y_radius;
}

void Samus_AlignBottomWithPrevPose(void) {  // 0x90EC7E
  uint16 r18 = kPoseParams[samus_pose].y_radius;
  r18 = kPoseParams[samus_prev_pose].y_radius - r18;
  samus_y_pos += r18;
  samus_prev_y_pos += r18;
}

void SwappedAmmoRoutine(void) {
  static const uint8 byte_90ED50[28] = {  // 0x90ED26
    0, 0, 0, 0, 1, 0, 0, 1,
    1, 1, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 0,
  };

  if (byte_90ED50[samus_movement_type]) {
    samus_missiles = samus_x_pos >> 4;
    samus_max_missiles = samus_x_pos >> 4;
    samus_super_missiles = samus_y_pos >> 4;
    samus_max_super_missiles = samus_y_pos >> 4;
  }
}

void UNUSEDsub_90ED6C(void) {  // 0x90ED6C
  samus_missiles = game_time_hours;
  samus_max_missiles = game_time_hours;
  samus_super_missiles = game_time_minutes;
  samus_max_super_missiles = game_time_minutes;
  samus_power_bombs = game_time_seconds;
  samus_max_power_bombs = game_time_seconds;
}

void Samus_CallInputHandler(void) {  // 0x90E90F
  CallSamusInputHandler(samus_input_handler | 0x900000);
}

void Samus_InputHandler_E913(void) {  // 0x90E913
  Samus_InputHandler();
}

void Samus_Func20_(void) {  // 0x90E918
  Samus_Func20();
}

void Samus_InputHandler_E91D(void) {  // 0x90E91D
  DemoObjectInputHandler();
  Samus_InputHandler();
}

void HandleAutoJumpHack(void) {  // 0x90E926
  uint16 v0 = joypad1_newkeys;
  if (autojump_timer && sign16(autojump_timer - 9)) {
    joypad1_newkeys |= button_config_jump_a;
    autojump_timer = 0;
  }
  Samus_InputHandler();
  joypad1_newkeys = v0;
  samus_input_handler = FUNC16(Samus_InputHandler_E913);
}

void RunSamusMovementHandler(void) {  // 0x90E94B
  CallSamusMovementHandler(samus_movement_handler | 0x900000);
}

void SamusMovementType_Xray(void) {  // 0x90E94F
  uint16 v0;
  if (samus_movement_type != kMovementType_0E_TurningAroundOnGround) {
    samus_anim_frame_timer = 15;
    if (samus_pose_x_dir == 4) {
      if (sign16(xray_angle - 153)) {
        v0 = 4;
      } else if (sign16(xray_angle - 178)) {
        v0 = 3;
      } else if (sign16(xray_angle - 203)) {
        v0 = 2;
      } else {
        v0 = sign16(xray_angle - 228) != 0;
      }
    } else if (sign16(xray_angle - 25)) {
      v0 = 0;
    } else if (sign16(xray_angle - 50)) {
      v0 = 1;
    } else if (sign16(xray_angle - 75)) {
      v0 = 2;
    } else if (sign16(xray_angle - 100)) {
      v0 = 3;
    } else {
      v0 = 4;
    }
    samus_anim_frame = v0;
  }
}

void Samus_HandlePeriodicDamage(void) {  // 0x90E9CE
  if (time_is_frozen_flag) {
    samus_periodic_damage = samus_periodic_subdamage = 0;
    return;
  }
  int32 t = __PAIR32__(samus_periodic_damage, samus_periodic_subdamage);
  if (t < 0) {
    InvalidInterrupt_Crash();
    return;
  }
  if ((equipped_items & 0x20) != 0)
    t = (t >> 2) & 0xffff00;
  else if ((equipped_items & 1) != 0)
    t = (t >> 1) & 0xffff00;
  AddToHiLo(&samus_health, &samus_subunit_health, -t);
  if ((int16)samus_health < 0)
    samus_health = samus_subunit_health = 0;
  samus_periodic_damage = samus_periodic_subdamage = 0;
}

void Samus_PauseCheck(void) {  // 0x90EA45
  if (!power_bomb_flag
      && !time_is_frozen_flag
      && !door_transition_flag_enemies
      && area_index != 6
      && game_state == kGameState_8_MainGameplay
      && (joypad1_newkeys & kButton_Start) != 0) {
    screen_fade_delay = 1;
    screen_fade_counter = 1;
    game_state = kGameState_12_Pausing;
  }
}

void Samus_LowHealthCheck_(void) {  // 0x90EA7F
  if (sign16(samus_health - 31)) {
    if (!samus_health_warning) {
      QueueSfx3_Max6(2);
      samus_health_warning = 1;
    }
  } else if (samus_health_warning) {
    samus_health_warning = 0;
    QueueSfx3_Max6(1);
  }
}

void Samus_LowHealthCheck_0(void) {  // 0x90EAAB
  Samus_LowHealthCheck_();
}

void Samus_JumpCheck(void) {  // 0x90EAB3
  if ((button_config_jump_a & joypad1_lastkeys) != 0 && (button_config_jump_a & joypad1_input_samusfilter) != 0)
    ++autojump_timer;
  else
    autojump_timer = 0;
  joypad1_input_samusfilter = joypad1_lastkeys;
  joypad1_newinput_samusfilter = joypad1_newkeys;
  if ((int16)(samus_health - samus_prev_health_for_flash) >= 0)
    goto LABEL_10;
  if (!samus_hurt_flash_counter)
    samus_hurt_flash_counter = 1;
  if (sign16(debug_invincibility - 7))
LABEL_10:
    samus_prev_health_for_flash = samus_health;
  else
    samus_health = samus_prev_health_for_flash;
}

void Samus_Func10(void) {  // 0x90EB02
  projectile_init_speed_samus_moved_left = 0;
  projectile_init_speed_samus_moved_left_fract = 0;
  projectile_init_speed_samus_moved_right = 0;
  projectile_init_speed_samus_moved_right_fract = 0;
  projectile_init_speed_samus_moved_up = 0;
  projectile_init_speed_samus_moved_up_fract = 0;
  projectile_init_speed_samus_moved_down = 0;
  projectile_init_speed_samus_moved_down_fract = 0;
  samus_anim_frame_skip = 0;
  new_projectile_direction_changed_pose = 0;
  UNUSED_word_7E0DFA <<= 8;
  g_word_7E0A10 = WORD(samus_pose_x_dir);
}

uint16 CallSomeSamusCode(uint16 a) {
  uint16 code = kSamusCodeHandlers[a & 0x1F]();
  if (!(code & 1))
    return code >> 1;
  samus_new_pose = -1;
  samus_new_pose_interrupted = -1;
  samus_new_pose_transitional = -1;
  samus_momentum_routine_index = 0;
  samus_special_transgfx_index = 0;
  samus_hurt_switch_index = 0;
  return -1;
}

void Samus_UpdatePreviousPose(void) {  // 0x90F0EE
  samus_last_different_pose = samus_prev_pose;
  *(uint16 *)&samus_last_different_pose_x_dir = *(uint16 *)&samus_prev_pose_x_dir;
  samus_prev_pose = samus_pose;
  *(uint16 *)&samus_prev_pose_x_dir = *(uint16 *)&samus_pose_x_dir;
}

uint8 ClearCarry(void) {  // 0x90F107
  return 0;
}

uint8 SamusCode_00_LockSamus(void) {  // 0x90F109
  frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func13);
  frame_handler_beta = FUNC16(SetContactDamageIndexAndUpdateMinimap);
  return 1;
}

uint8 SamusCode_01_UnlockSamus(void) {  // 0x90F117
  frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func11);
  frame_handler_beta = FUNC16(Samus_FrameHandlerBeta_Func17);
  return 1;
}

uint8 SamusCode_02_ReachCeresElevator(void) {  // 0x90F125
  if (samus_pose_x_dir == 4)
    samus_pose = kPose_02_FaceL_Normal;
  else
    samus_pose = kPose_01_FaceR_Normal;
  samus_anim_frame_skip = 0;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
  reached_ceres_elevator_fade_timer = 60;
  return SamusCode_00_LockSamus();
}

uint8 SamusCode_03(void) {  // 0x90F152
  if (grapple_beam_function != FUNC16(GrappleBeamFunc_Inactive)) {
    grapple_beam_function = FUNC16(GrappleBeam_Func2);
    return 0;
  }
  if (samus_movement_type != kMovementType_03_SpinJumping && samus_movement_type != kMovementType_14_WallJumping)
    return 0;
  if (samus_pose_x_dir == kMovementType_04_MorphBallOnGround)
    samus_pose = kPose_02_FaceL_Normal;
  else
    samus_pose = kPose_01_FaceR_Normal;
  samus_anim_frame_skip = 0;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
  Samus_UpdatePreviousPose();
  return 1;
}

uint8 SamusCode_04_06_Common(void) {  // 0x90F19E
  flare_counter = 0;
  ClearFlareAnimationState();
  Samus_LoadSuitPalette();
  return 0;
}

uint8 SamusCode_04(void) {  // 0x90F19B
  samus_charge_palette_index = 0;
  return SamusCode_04_06_Common();
}

uint8 SamusCode_06_LockToStation(void) {  // 0x90F1AA
  frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func13);
  frame_handler_beta = FUNC16(j_HandleDemoRecorder_2);
  if (!sign16(flare_counter - 15))
    QueueSfx1_Max15(2);
  return SamusCode_04_06_Common();
}

uint8 SamusCode_07_SetupForElevator(void) {  // 0x90F1C8
  MakeSamusFaceForward();
  frame_handler_beta = FUNC16(Samus_Func19);
  samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
  samus_draw_handler = FUNC16(SamusDisplayHandler_UsingElevator);
  samus_input_handler = FUNC16(Samus_InputHandler_E913);
  bomb_jump_dir = 0;
  return 1;
}

uint8 SamusCode_08_SetupForCeresStart(void) {  // 0x90F1E9
  frame_handler_alfa = FUNC16(EmptyFunction);
  frame_handler_beta = FUNC16(SetContactDamageIndexAndUpdateMinimap);
  samus_pose = kPose_00_FaceF_Powersuit;
  samus_anim_frame_skip = 0;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
  Samus_LoadSuitPalette();
  samus_draw_handler = FUNC16(SamusDrawHandler_Default);
  samus_prev_pose = samus_pose;
  samus_last_different_pose = samus_pose;
  *(uint16 *)&samus_prev_pose_x_dir = *(uint16 *)&samus_pose_x_dir;
  *(uint16 *)&samus_last_different_pose_x_dir = *(uint16 *)&samus_pose_x_dir;
  samus_y_pos = 0;
  SpawnEprojWithGfx(0, 0, addr_kEproj_CeresElevatorPad);
  SpawnEprojWithGfx(0, 0, addr_kEproj_CeresElevatorPlatform);
  debug_disable_minimap = 0;
  PlayRoomMusicTrackAfterAFrames(0x20);
  return 1;
}

uint8 SamusCode_08_SetupForZebesStart(void) {  // 0x90F23C
  if ((equipped_items & 0x20) != 0) {
    SpawnPalfxObject(addr_stru_8DE1FC);
    samus_pose = kPose_9B_FaceF_VariaGravitySuit;
  } else if ((equipped_items & 1) != 0) {
    SpawnPalfxObject(addr_stru_8DE1F8);
    samus_pose = kPose_9B_FaceF_VariaGravitySuit;
  } else {
    SpawnPalfxObject(addr_stru_8DE1F4);
    samus_pose = kPose_00_FaceF_Powersuit;
  }
  Samus_LoadSuitPalette();
  SamusFunc_F433();
  samus_anim_frame_timer = 3;
  samus_anim_frame = 2;
  substate = 0;
  return 1;
}

uint8 SamusCode_0A_ClearDrawHandler(void) {  // 0x90F28D
  samus_draw_handler = FUNC16(nullsub_152);
  return 0;
}

uint8 SamusCode_0B_DrawHandlerDefault(void) {  // 0x90F295
  samus_draw_handler = FUNC16(SamusDrawHandler_Default);
  return SamusCode_01_UnlockSamus();
}

uint8 SamusCode_0C_UnlockFromMapStation(void) {  // 0x90F29E
  SamusFunc_E633();
  if (frame_handler_beta == FUNC16(j_HandleDemoRecorder_2)) {
    frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func11);
    frame_handler_beta = FUNC16(Samus_FrameHandlerBeta_Func17);
  }
  return 1;
}

uint8 SamusCode_0D_IsGrappleActive(void) {  // 0x90F2B8
  return (grapple_beam_function != FUNC16(GrappleBeamFunc_Inactive)) ? 2 : 0;
}

uint8 SamusCode_0D_IsGrappleActive_A(void) {
  return grapple_beam_function != FUNC16(GrappleBeamFunc_Inactive);
}

uint8 SamusCode_0E(void) {  // 0x90F2CA
  frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func11);
  frame_handler_beta = FUNC16(Samus_Func18);
  return 1;
}

uint8 SamusCode_0F_EnableTimerHandling(void) {  // 0x90F2D8
  frame_handler_gamma = FUNC16(Samus_Func3);
  return 0;
}

uint8 SamusCode_10(void) {  // 0x90F2E0
  if (frame_handler_beta != FUNC16(j_HandleDemoRecorder_2_0)) {
    frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func11);
    frame_handler_beta = FUNC16(Samus_FrameHandlerBeta_Func17);
  }
  return 1;
}

uint8 SamusCode_11_15_Common(void) {  // 0x90F2FC
  frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func13);
  frame_handler_beta = FUNC16(EmptyFunction);
  samus_draw_handler = FUNC16(SamusDisplayHandler_SamusReceivedFatal);
  return 1;
}

uint8 SamusCode_11_SetupForDeath(void) {  // 0x90F2F8
  DisablePaletteFx();
  return SamusCode_11_15_Common();
}

uint8 SamusCode_15_CalledBySuitAcquision(void) {  // 0x90F310
  Samus_UpdatePreviousPose();
  flare_counter = 0;
  ClearFlareAnimationState();
  Samus_LoadSuitPalette();
  return SamusCode_11_15_Common();
}

uint8 SamusCode_12_SetSuperPaletteFlag1(void) {  // 0x90F320
  samus_special_super_palette_flags = 1;
  return 0;
}

uint8 SamusCode_12_SetSuperPaletteFlag0(void) {  // 0x90F328
  samus_special_super_palette_flags = 0;
  Samus_LoadSuitPalette();
  return 0;
}

uint8 SamusCode_14(void) {  // 0x90F331
  if (sign16(samus_health - 31))
    QueueSfx3_Max6(2);
  if (!SamusCode_0D_IsGrappleActive_A()) {
    if (samus_pose_x_dir == 3) {
      if (samus_pose == kPose_81_FaceR_Screwattack || samus_pose == kPose_82_FaceL_Screwattack) {
        QueueSfx1_Max6(0x33);
      } else if (samus_pose == kPose_1B_FaceR_SpaceJump || samus_pose == kPose_1C_FaceL_SpaceJump) {
        QueueSfx1_Max6(0x3E);
      } else {
        QueueSfx1_Max6(0x31);
      }
    }
  } else {
    QueueSfx1_Max6(6);
  }
  return 0;
}

uint8 SamusCode_05_SetupDrained(void) {  // 0x90F38E
  frame_handler_gamma = FUNC16(Samus_Func1);
  return Samus_SetupForBeingDrained();
}

uint8 Samus_SetupForBeingDrained(void) {  // 0x90F394
  samus_pose = kPose_54_FaceL_Knockback;
  samus_anim_frame_skip = 0;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
  Samus_UpdatePreviousPose();
  flare_counter = 0;
  ClearFlareAnimationState();
  Samus_LoadSuitPalette();
  frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func13);
  frame_handler_beta = FUNC16(j_HandleDemoRecorder_2_0);
  return 1;
}

uint8 SamusCode_18(void) {  // 0x90F3C0
  frame_handler_gamma = FUNC16(Samus_Func2);
  return Samus_SetupForBeingDrained();
}

uint8 SamusCode_16(void) {  // 0x90F3C9
  samus_special_super_palette_flags = 0x8000;
  special_samus_palette_frame = 1;
  special_samus_palette_timer = 1;
  samus_charge_palette_index = 0;
  return 0;
}

uint8 SamusCode_17_DisableRainbowSamusAndStandUp(void) {  // 0x90F3DD
  samus_special_super_palette_flags = 0;
  special_samus_palette_frame = 0;
  special_samus_palette_timer = 0;
  samus_charge_palette_index = 0;
  Samus_LoadSuitPalette();
  samus_anim_frame_timer = 1;
  samus_anim_frame = 13;
  return 0;
}

uint8 SamusCode_17_FreezeDrainedSamus(void) {  // 0x90F3FB
  samus_anim_frame_timer = 1;
  samus_anim_frame = 28;
  return 1;
}

uint8 SamusCode_1A(void) {  // 0x90F409
  frame_handler_beta = FUNC16(Samus_LowHealthCheck);
  return 0;
}

uint8 SamusCode_1B_CheckedLockSamus(void) {  // 0x90F411
  if (frame_handler_beta == FUNC16(j_HandleDemoRecorder_2_0))
    return 1;
  else
    return SamusCode_00_LockSamus();
}

uint8 SamusCode_1C(void) {  // 0x90F41E
  if (samus_movement_type == kMovementType_14_WallJumping) {
    if (sign16(samus_anim_frame - 23)) {
      if (sign16(samus_anim_frame - 13)) {
LABEL_11:
        QueueSfx1_Max9(0x31);
        return 0;
      }
      goto LABEL_12;
    }
  } else {
    if (samus_movement_type != kMovementType_03_SpinJumping)
      return 0;
    if (samus_pose != kPose_81_FaceR_Screwattack && samus_pose != kPose_82_FaceL_Screwattack) {
      if (samus_pose != kPose_1B_FaceR_SpaceJump && samus_pose != kPose_1C_FaceL_SpaceJump)
        goto LABEL_11;
LABEL_12:
      QueueSfx1_Max9(0x3E);
      return 0;
    }
  }
  QueueSfx1_Max9(0x33);
  return 0;
}

uint8 SamusCode_1D_ClearSoundInDoor(void) {  // 0x90F471
  if (samus_movement_type == 3 || samus_movement_type == 20) {
    QueueSfx1_Max15(0x32);
    return 0;
  } else {
    if ((button_config_shoot_x & joypad1_lastkeys) == 0) {
      if (sign16(flare_counter - 16))
        QueueSfx1_Max15(2);
    }
    return 0;
  }
}

uint8 SamusCode_1E(void) {  // 0x90F4A2
  if (game_state == 8) {
    if (samus_movement_type == 3 || samus_movement_type == 20) {
      SamusCode_1C();
      return 0;
    }
    if (!sign16(flare_counter - 16))
      QueueSfx1_Max9(0x41);
  }
  return 0;
}

uint8 SamusCode_1F(void) {  // 0x90F4D0
  if (grapple_beam_function != (uint16)addr_loc_90C4F0) {
    grapple_beam_unkD1E = 0;
    grapple_beam_unkD20 = 0;
    grapple_beam_direction = 0;
    grapple_beam_unkD36 = 0;
    grapple_walljump_timer = 0;
    slow_grabble_scrolling_flag = 0;
    grapple_varCF6 = 0;
    grapple_beam_flags = 0;
    LoadProjectilePalette(equipped_beams);
    grapple_beam_function = -15120;
    samus_draw_handler = FUNC16(SamusDrawHandler_Default);
  }
  return 0;
}

uint8 Samus_Func26(void) {  // 0x90F507
  if (samus_movement_type != kMovementType_03_SpinJumping
      && samus_movement_type != kMovementType_14_WallJumping
      && (button_config_shoot_x & joypad1_lastkeys) != 0
      && !sign16(flare_counter - 16)) {
    QueueSfx1_Max9(0x41);
  }
  return 0;
}

void Samus_ShootCheck(void) {  // 0x90F576
  if ((play_resume_charging_beam_sfx & 0x8000) != 0)
    goto LABEL_15;
  if (play_resume_charging_beam_sfx) {
    if ((button_config_shoot_x & joypad1_lastkeys) != 0)
      QueueSfx1_Max9(0x41);
    play_resume_charging_beam_sfx = 0;
  }
  if (samus_echoes_sound_flag && (speed_boost_counter & 0x400) == 0) {
    samus_echoes_sound_flag = 0;
    QueueSfx3_Max15(0x25);
  }
  if ((samus_prev_movement_type == 3 || samus_prev_movement_type == 20)
      && samus_movement_type != kMovementType_03_SpinJumping
      && samus_movement_type != kMovementType_14_WallJumping) {
    QueueSfx1_Max15(0x32);
    if (!sign16(flare_counter - 16) && (button_config_shoot_x & joypad1_lastkeys) != 0)
LABEL_15:
      play_resume_charging_beam_sfx = 1;
  }
  if (enable_debug) {
    if (samus_pose == kPose_00_FaceF_Powersuit || samus_pose == kPose_9B_FaceF_VariaGravitySuit) {
      if ((joypad2_last & 0x30) == 48 && (joypad2_new_keys & 0x80) != 0)
        debug_invincibility = 7;
    } else {
      if (!sign16(debug_invincibility - 7))
        return;
      debug_invincibility = 0;
    }
  }
  if (CheckEventHappened(0xE) & 1
      && frame_handler_gamma == FUNC16(DrawTimer_)
      && game_state != kGameState_35_TimeUp) {
    game_state = kGameState_35_TimeUp;
  }
}
