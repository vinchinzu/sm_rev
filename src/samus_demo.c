#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "sm_rtl.h"

void EnableDemoInput(void) {  // 0x91834E
  samus_input_handler = FUNC16(Samus_InputHandler_E91D);
  demo_enable |= 0x8000;
}

void DisableDemoInput(void) {  // 0x91835F
  samus_input_handler = FUNC16(Samus_InputHandler_E913);
  demo_enable &= ~0x8000;
}

void ResetDemoData(void) {  // 0x918370
  demo_input_pre_instr = 0;
  demo_input_instr_timer = 0;
  demo_input_instr_ptr = 0;
  demo_timer_counter = 0;
  xray_angle = 0;
  demo_input = 0;
  demo_input_new = 0;
  demo_input_prev = 0;
  demo_input_prev_new = 0;
  demo_enable = 0;
}

void LoadDemoInputObject(uint16 a, uint16 j) {  // 0x918395
  DemoInputObject *dio;
  xray_angle = a;
  dio = get_DemoInputObject(j);
  demo_input_pre_instr = dio->pre_instr;
  demo_input_instr_ptr = dio->instr_ptr;
  demo_input_instr_timer = 1;
  demo_timer_counter = 0;
  //  Call(dio->ptr | 0x910000);
}

void DemoObjectInputHandler(void) {  // 0x9183C0
  if ((demo_enable & 0x8000) != 0) {
    if (demo_input_instr_ptr) {
      ProcessDemoInputObject();
      joypad1_input_samusfilter = demo_input_prev;
      joypad1_newinput_samusfilter = demo_input_prev_new;
      joypad1_lastkeys = demo_input;
      demo_input_prev = demo_input;
      joypad1_newkeys = demo_input_new;
      demo_input_prev_new = demo_input_new;
    }
  }
}

void CallDemoPreInstr(uint32 ea) {
  switch (ea) {
  case fnDemoPreInstr_nullsub_162: return;
  case fnDemoPreInstr_864F: DemoPreInstr_864F(); return;
  case fnDemoPreInstr_866A: DemoPreInstr_866A(); return;
  case fnDemoPreInstr_CheckLeaveDemo: DemoPreInstr_CheckLeaveDemo(); return;
  case fnDemoPreInstr_8AB0: DemoPreInstr_8AB0(); return;
  default: Unreachable();
  }
}

uint16 CallDemoInstr(uint32 ea, uint16 k, uint16 j) {
  switch (ea) {
  case fnDemoInstr_Finish: return DemoInstr_Finish(k, j);
  case fnDemoInstr_SetPreInstr: return DemoInstr_SetPreInstr(k, j);
  case fnDemoInstr_ClearPreInstr: return DemoInstr_ClearPreInstr(k, j);
  case fnDemoInstr_Goto: return DemoInstr_Goto(k, j);
  case fnDemoInstr_DecTimerAndGoto: return DemoInstr_DecTimerAndGoto(k, j);
  case fnDemoInstr_SetTimer: return DemoInstr_SetTimer(k, j);
  case fnDemoInstr_Func2: return DemoInstr_Func2(k, j);
  case fnDemoInstr_Disable: return DemoInstr_Disable(k, j);
  case fnDemoInstr_Func3: return DemoInstr_Func3(k, j);
  default: return Unreachable();
  }
}

void ProcessDemoInputObject(void) {  // 0x9183F2
  CallDemoPreInstr(demo_input_pre_instr | 0x910000);
  if (!--demo_input_instr_timer) {
    uint16 v0 = demo_input_instr_ptr;
    uint16 *v1;
    while (1) {
      v1 = (uint16 *)RomPtr_91(v0);
      uint16 v2 = *v1;
      if ((*v1 & 0x8000) == 0)
        break;
      v0 = CallDemoInstr(v2 | 0x910000, 0, v0 + 2);
      if (!v0)
        return;
    }
    demo_input_instr_timer = *v1;
    const uint8 *v3 = RomPtr_91(v0);
    demo_input = GET_WORD(v3 + 2);
    demo_input_new = GET_WORD(v3 + 4);
    demo_input_instr_ptr = v0 + 6;
  }
}

uint16 DemoInstr_Finish(uint16 k, uint16 j) {  // 0x918427
  demo_input_instr_ptr = 0;
  demo_input = 0;
  demo_input_new = 0;
  return 0;
}

uint16 DemoInstr_SetPreInstr(uint16 k, uint16 j) {  // 0x918434
  demo_input_pre_instr = *(uint16 *)RomPtr_91(j);
  return j + 2;
}

uint16 DemoInstr_ClearPreInstr(uint16 k, uint16 j) {  // 0x91843F
  demo_input_pre_instr = 0x8447;
  return j;
}

uint16 DemoInstr_Goto(uint16 k, uint16 j) {  // 0x918448
  return *(uint16 *)RomPtr_91(j);
}

uint16 DemoInstr_DecTimerAndGoto(uint16 k, uint16 j) {  // 0x91844F
  if (--demo_timer_counter)
    return DemoInstr_Goto(k, j);
  else
    return j + 2;
}

uint16 DemoInstr_SetTimer(uint16 k, uint16 j) {  // 0x918459
  demo_timer_counter = *(uint16 *)RomPtr_91(j);
  return j + 2;
}

uint16 DemoInstr_Func2(uint16 k, uint16 j) {  // 0x9185FC
  frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func11);
  frame_handler_beta = FUNC16(Samus_FrameHandlerBeta_Func17);
  DisableDemoInput();
  return j;
}

void DemoPreInstr_864F(void) {  // 0x91864F
  if (sign16(samus_x_pos - 178)) {
    demo_input_pre_instr = FUNC16(DemoPreInstr_866A);
    demo_input_instr_ptr = 0x8623;
    demo_input_instr_timer = 1;
  }
}

void DemoPreInstr_866A(void) {  // 0x91866A
  if (!eproj_x_pos[0]) {
    demo_input_pre_instr = FUNC16(DemoPreInstr_nullsub_162);
    demo_input_instr_ptr = addr_off_91864B;
    demo_input_instr_timer = 1;
  }
}

uint16 DemoInstr_Disable(uint16 k, uint16 j) {  // 0x918682
  frame_handler_alfa = FUNC16(EmptyFunction);
  frame_handler_beta = FUNC16(EmptyFunction);
  DisableDemoInput();
  return j;
}

void UNUSED_DemoInstr_Func4(void) {  // 0x9186FE
  frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func13);
  samus_pose = kPose_02_FaceL_Normal;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
  samus_last_different_pose = samus_prev_pose;
  *(uint16 *)&samus_last_different_pose_x_dir = *(uint16 *)&samus_prev_pose_x_dir;
  samus_prev_pose = samus_pose;
  *(uint16 *)&samus_prev_pose_x_dir = *(uint16 *)&samus_pose_x_dir;
  DisableDemoInput();
  samus_input_handler = FUNC16(nullsub_152);
}

uint16 DemoInstr_Func3(uint16 k, uint16 j) {  // 0x918739
  frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func13);
  samus_pose = kPose_02_FaceL_Normal;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
  samus_last_different_pose = samus_prev_pose;
  *(uint16 *)&samus_last_different_pose_x_dir = *(uint16 *)&samus_prev_pose_x_dir;
  samus_prev_pose = samus_pose;
  *(uint16 *)&samus_prev_pose_x_dir = *(uint16 *)&samus_pose_x_dir;
  DisableDemoInput();
  samus_input_handler = FUNC16(nullsub_152);
  return j;
}

static Func_V *const kDemoSetFuncPtrs_0[6] = {  // 0x918790
  DemoSetFunc_0,
  DemoSetFunc_1,
  DemoSetFunc_1,
  DemoSetFunc_2,
  DemoSetFunc_1,
  DemoSetFunc_3,
};
static Func_V *const kDemoSetFuncPtrs_1[6] = {
  DemoSetFunc_2,
  DemoSetFunc_2,
  DemoSetFunc_1,
  DemoSetFunc_2,
  DemoSetFunc_1,
  DemoSetFunc_4,
};
static Func_V *const kDemoSetFuncPtrs_2[6] = {
  DemoSetFunc_2,
  DemoSetFunc_5,
  DemoSetFunc_1,
  DemoSetFunc_1,
  DemoSetFunc_1,
  DemoSetFunc_1,
};
static Func_V *const kDemoSetFuncPtrs_3[5] = {
  DemoSetFunc_6,
  DemoSetFunc_2,
  DemoSetFunc_2,
  DemoSetFunc_1,
  DemoSetFunc_7,
};

static Func_V *const *const kDemoSetFuncPtrs[4] = {
  kDemoSetFuncPtrs_0,
  kDemoSetFuncPtrs_1,
  kDemoSetFuncPtrs_2,
  kDemoSetFuncPtrs_3,
};

void LoadDemoData(void) {
  DemoSetDef *DemoSetDef;

  uint16 v0 = 16 * demo_scene + kDemoSetDefPtrs[demo_set];
  DemoSetDef = get_DemoSetDef(v0);
  collected_items = DemoSetDef->items;
  equipped_items = collected_items;
  samus_max_missiles = DemoSetDef->missiles;
  samus_missiles = samus_max_missiles;
  samus_max_super_missiles = DemoSetDef->super_missiles;
  samus_super_missiles = samus_max_super_missiles;
  samus_max_power_bombs = DemoSetDef->power_bombs;
  samus_power_bombs = samus_max_power_bombs;
  samus_max_health = DemoSetDef->health;
  samus_health = samus_max_health;
  collected_beams = DemoSetDef->beams;
  equipped_beams = DemoSetDef->equipped_beams_;
  samus_reserve_health = 0;
  ResetDemoData();
  EnableDemoInput();
  uint16 demo_obj = get_DemoSetDef(v0)->demo_obj;
  LoadDemoInputObject(demo_obj, demo_obj);
  kDemoSetFuncPtrs[demo_set][demo_scene]();
  frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func12);
  frame_handler_beta = FUNC16(Samus_FrameHandlerBeta_Func14);
  samus_momentum_routine_index = 0;
  samus_special_transgfx_index = 0;
  samus_hurt_switch_index = 0;
  Samus_LoadSuitPalette();
  UpdateBeamTilesAndPalette();
  button_config_up = kButton_Up;
  button_config_down = kButton_Down;
  button_config_left = kButton_Left;
  button_config_right = kButton_Right;
  button_config_shoot_x = kButton_X;
  button_config_jump_a = kButton_A;
  button_config_run_b = kButton_B;
  button_config_itemcancel_y = kButton_Y;
  button_config_itemswitch = kButton_Select;
  button_config_aim_up_R = kButton_R;
  button_config_aim_down_L = kButton_L;
  UNUSED_word_7E09E8 = 1;
  debug_flag = 1;
  moonwalk_flag = 0;
  UNUSED_word_7E0DF8 = 0;
  UNUSED_word_7E0DFA = 0;
  UNUSED_word_7E0DFC = 0;
}

void DemoSetFunc_0(void) {  // 0x918A33
  MakeSamusFaceForward();
  samus_draw_handler = FUNC16(SamusDrawHandler_Default);
}

void DemoSetFunc_3(void) {  // 0x918A3E
  DemoSetFunc_Common(0x1F);
}

void DemoSetFunc_7(void) {  // 0x918A43
  samus_health = 20;
  DemoSetFunc_2();
}

void DemoSetFunc_2(void) {  // 0x918A49
  DemoSetFunc_Common(2);
}

void DemoSetFunc_4(void) {  // 0x918A4E
  DemoSetFunc_Common(0x2A);
}

void DemoSetFunc_1(void) {  // 0x918A53
  DemoSetFunc_Common(1);
}

void DemoSetFunc_Common(uint16 a) {  // 0x918A56
  samus_pose = a;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
  samus_draw_handler = FUNC16(SamusDrawHandler_Default);
}

void DemoSetFunc_5(void) {  // 0x918A68
  samus_draw_handler = FUNC16(SamusDrawHandler_Default);
  Projectile_Func7_Shinespark();
  samus_pose = kPose_CD_FaceR_Shinespark_Diag;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
}

void DemoSetFunc_6(void) {  // 0x918A81
  samus_draw_handler = FUNC16(SamusDrawHandler_Default);
  Projectile_Func7_Shinespark();
  samus_pose = kPose_CA_FaceL_Shinespark_Horiz;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
}

void DemoPreInstr_CheckLeaveDemo(void) {  // 0x918A9B
  if (game_state == kGameState_44_TransitionFromDemo) {
    demo_input_instr_ptr = addr_kDemoInstrs_LeaveDemo;
    demo_input_instr_timer = 1;
  }
}

void DemoPreInstr_8AB0(void) {  // 0x918AB0
  if (samus_movement_type != kMovementType_1A_GrabbedByDraygon) {
    demo_input_pre_instr = FUNC16(DemoPreInstr_CheckLeaveDemo);
    demo_input_instr_ptr = addr_stru_919346;
    demo_input_instr_timer = 1;
  }
}
