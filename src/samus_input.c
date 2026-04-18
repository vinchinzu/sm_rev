// Samus input dispatch: per-movement-type joypad handlers, the transition
// table walker that produces samus_new_pose from the current pose plus
// pressed buttons, and the custom-binding remapper that normalises
// configurable controller layouts back to default button indices.
// Extracted from sm_91.c.

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"

typedef struct Pair_R18_R20 {
  uint16 r18, r20;
} Pair_R18_R20;

Pair_R18_R20 TranslateCustomControllerBindingsToDefault(void);

static Func_V *const kSamusInputHandlers[28] = {
  Samus_Input_00_Standing,
  Samus_Input_01_Running,
  Samus_Input_02_NormalJumping,
  Samus_Input_03_SpinJumping,
  Samus_Input_04_MorphBallOnGround,
  Samus_Input_05_Crouching,
  Samus_Input_06_Falling,
  Samus_Input_07_Unused,
  Samus_Input_08_MorphBallFalling,
  Samus_Input_09_Unused,
  Samus_Input_0A_KnockbackOrCrystalFlashEnding,
  Samus_Input_0B_Unused,
  Samus_Input_0C_Unused,
  Samus_Input_0D_Unused,
  Samus_Input_0E_TurningAroundOnGround,
  Samus_Input_0F_CrouchingEtcTransition,
  Samus_Input_10_Moonwalking,
  Samus_Input_11_SpringBallOnGround,
  Samus_Input_12_SpringBallInAir,
  Samus_Input_13_SpringBallFalling,
  Samus_Input_14_WallJumping,
  Samus_Input_15_RanIntoWall,
  Samus_Input_16_Grappling,
  Samus_Input_17_TurningAroundJumping,
  Samus_Input_18_TurningAroundFalling,
  Samus_Input_19_DamageBoost,
  Samus_Input_1A_GrabbedByDraygon,
  Samus_Input_1B_ShinesparkEtc,
};
void Samus_InputHandler(void) {  // 0x918000
  kSamusInputHandlers[samus_movement_type]();
}

void Samus_Input_00_Standing(void) {  // 0x91804D
  if (samus_pose && samus_pose != kPose_9B_FaceF_VariaGravitySuit || !elevator_status)
    Samus_LookupTransitionTable();
}

void Samus_Input_01_Running(void) {  // 0x918066
  Samus_LookupTransitionTable();
}

void Samus_Input_02_NormalJumping(void) {  // 0x91806E
  Samus_LookupTransitionTable();
}

void Samus_Input_03_SpinJumping(void) {  // 0x918076
  Samus_LookupTransitionTable();
}

void Samus_Input_04_MorphBallOnGround(void) {  // 0x91807E
  Samus_LookupTransitionTable();
}

void Samus_Input_07_Unused(void) {  // 0x918086
  ;
}

void Samus_Input_05_Crouching(void) {  // 0x918087
  if (time_is_frozen_flag) {
    Samus_Func20();
  } else {
    Samus_LookupTransitionTable();
    if (!samus_movement_type) {
      samus_y_pos -= 5;
      samus_prev_y_pos -= 5;
    }
  }
}

void Samus_Input_06_Falling(void) {  // 0x9180B6
  Samus_LookupTransitionTable();
}

void Samus_Input_08_MorphBallFalling(void) {  // 0x91810A
  Samus_LookupTransitionTable();
}

void Samus_Input_09_Unused(void) {  // 0x918112
  ;
}

void Samus_Input_0A_KnockbackOrCrystalFlashEnding(void) {  // 0x918113
  Samus_LookupTransitionTable();
  if (samus_movement_type != kMovementType_0A_KnockbackOrCrystalFlashEnding) {
    Samus_InitJump();
    samus_knockback_timer = 0;
  }
}

void Samus_Input_0B_Unused(void) {  // 0x91812D
  ;
}

void Samus_Input_0C_Unused(void) {  // 0x918132
  Samus_LookupTransitionTable();
}

void Samus_Input_0D_Unused(void) {  // 0x91813A
  Samus_LookupTransitionTable();
}

void Samus_Input_0E_TurningAroundOnGround(void) {  // 0x918142
  Samus_LookupTransitionTable();
}

void Samus_Input_0F_CrouchingEtcTransition(void) {  // 0x918146
  ;
}

void Samus_Input_10_Moonwalking(void) {  // 0x918147
  Samus_LookupTransitionTable();
}

void Samus_Input_11_SpringBallOnGround(void) {  // 0x91814F
  Samus_LookupTransitionTable();
}

void Samus_Input_12_SpringBallInAir(void) {  // 0x918157
  Samus_LookupTransitionTable();
}

void Samus_Input_13_SpringBallFalling(void) {  // 0x91815F
  Samus_LookupTransitionTable();
}

void Samus_Input_14_WallJumping(void) {  // 0x918167
  Samus_LookupTransitionTable();
}

void Samus_Input_15_RanIntoWall(void) {  // 0x91816F
  if (time_is_frozen_flag)
    Samus_Func20();
  else
    Samus_LookupTransitionTable();
}

void Samus_Input_16_Grappling(void) {  // 0x918181
  Samus_LookupTransitionTable();
}

void Samus_Input_17_TurningAroundJumping(void) {  // 0x918189
  Samus_LookupTransitionTable();
}

void Samus_Input_18_TurningAroundFalling(void) {  // 0x91818D
  Samus_LookupTransitionTable();
}

void Samus_Input_19_DamageBoost(void) {  // 0x918191
  Samus_LookupTransitionTable();
}

void Samus_Input_1A_GrabbedByDraygon(void) {  // 0x918199
  Samus_LookupTransitionTable();
}

void Samus_Input_1B_ShinesparkEtc(void) {  // 0x9181A1
  Samus_LookupTransitionTable();
}

void Samus_LookupTransitionTable(void) {  // 0x9181A9
  if (joypad1_lastkeys) {
    Pair_R18_R20 pair = TranslateCustomControllerBindingsToDefault();
    PoseEntry *pe = get_PoseEntry(kPoseTransitionTable[samus_pose]);
    if (pe->new_input == 0xFFFF)
      return;
    do {
      if ((pair.r18 & pe->new_input) == 0 && (pair.r20 & pe->cur_input) == 0) {
        if (pe->new_pose != samus_pose) {
          samus_new_pose = pe->new_pose;
          bomb_jump_dir = 0;
        }
        return;
      }
      pe++;
    } while (pe->new_input != 0xFFFF);
  }
  UNUSED_word_7E0A18 = 0;
  Samus_Pose_CancelGrapple();
}

Pair_R18_R20 TranslateCustomControllerBindingsToDefault(void) {  // 0x9181F4
  uint16 r18 = joypad1_newkeys & (kButton_Up | kButton_Down | kButton_Left | kButton_Right);
  uint16 r20 = joypad1_lastkeys & (kButton_Up | kButton_Down | kButton_Left | kButton_Right);
  uint16 v0 = joypad1_newkeys;
  if ((button_config_shoot_x & joypad1_newkeys) != 0)
    r18 |= kButton_X;
  if ((button_config_jump_a & v0) != 0)
    r18 |= kButton_A;
  if ((button_config_run_b & v0) != 0)
    r18 |= kButton_B;
  if ((button_config_itemcancel_y & v0) != 0)
    r18 |= kButton_Y;
  if ((button_config_aim_up_R & v0) != 0) {
    if ((button_config_aim_up_R & (kButton_L | kButton_R)) != 0)
      r18 |= kButton_R;
  }
  if ((button_config_aim_down_L & v0) != 0 && (button_config_aim_down_L & (kButton_L | kButton_R)) != 0)
    r18 |= kButton_L;
  uint16 v1 = joypad1_lastkeys;
  if ((button_config_shoot_x & joypad1_lastkeys) != 0)
    r20 |= kButton_X;
  if ((button_config_jump_a & v1) != 0)
    r20 |= kButton_A;
  if ((button_config_run_b & v1) != 0)
    r20 |= kButton_B;
  if ((button_config_itemcancel_y & v1) != 0)
    r20 |= kButton_Y;
  if ((button_config_aim_up_R & v1) != 0) {
    if ((button_config_aim_up_R & (kButton_L | kButton_R)) != 0)
      r20 |= kButton_R;
  }
  if ((button_config_aim_down_L & v1) != 0 && (button_config_aim_down_L & (kButton_L | kButton_R)) != 0)
    r20 |= kButton_L;
  return (Pair_R18_R20) { ~r18, ~r20 };
}

void Samus_Pose_CancelGrapple(void) {  // 0x9182D9
  if (Samus_Pose_Func2() & 1 || kPoseParams[samus_pose].new_pose_unless_buttons == 255)
    samus_new_pose = samus_pose;
  else
    samus_new_pose = kPoseParams[samus_pose].new_pose_unless_buttons;
}

static const uint8 kSamus_Pose_Func2_Tab[28] = {  // 0x918304
  2, 1, 1, 0, 6, 2, 8, 2, 1, 6, 2, 2, 2, 6, 2, 2,
  2, 6, 6, 6, 6, 2, 6, 2, 2, 2, 2, 2,
};

uint8 Samus_Pose_Func2(void) {

  uint16 v0 = kSamus_Pose_Func2_Tab[samus_movement_type];
  if (v0 != 1)
    goto LABEL_2;
  if (!samus_x_base_speed && !samus_x_base_subspeed) {
    v0 = 2;
LABEL_2:
    samus_momentum_routine_index = v0;
    return 0;
  }
  samus_momentum_routine_index = 1;
  return 1;
}

void ReleaseButtonsFilter(uint16 v0) {  // 0x808146
  timed_held_input_timer_reset = v0;
  bool v1 = ((uint16)~joypad1_newkeys & joypad1_lastkeys) == joypad_released_keys;
  joypad_released_keys = ~joypad1_newkeys & joypad1_lastkeys;
  if (!v1) {
    timed_held_input_timer = timed_held_input_timer_reset;
    timed_held_input = 0;
    goto LABEL_6;
  }
  if ((--timed_held_input_timer & 0x8000) == 0) {
    timed_held_input = 0;
    goto LABEL_6;
  }
  timed_held_input_timer = 0;
  previous_timed_held_input = timed_held_input;
  timed_held_input = ~joypad1_newkeys & joypad1_lastkeys;
LABEL_6:
  newly_held_down_timed_held_input = timed_held_input & (previous_timed_held_input ^ timed_held_input);
}

void ReadJoypadInputs(void) {  // 0x809459
  uint16 RegWord = ReadRegWord(JOY1L);
  uint16 v1 = ReadRegWord(JOY2L);
  joypad1_lastkeys = RegWord;
  joypad1_newkeys = RegWord & (joypad1_prev ^ RegWord);
  joypad1_newkeys2_UNUSED = RegWord & (joypad1_prev ^ RegWord);
  if (RegWord && RegWord == joypad1_prev) {
    if (!--joypad1_keyrepeat_ctr) {
      joypad1_newkeys2_UNUSED = joypad1_lastkeys;
      joypad1_keyrepeat_ctr = joypad_ctr_repeat_next;
    }
  } else {
    joypad1_keyrepeat_ctr = joypad_ctr_repeat_first;
  }
  joypad1_prev = joypad1_lastkeys;
  joypad2_last = v1;
  joypad2_new_keys = v1 & (joypad2_prev ^ v1);
  joypad2_newkeys2 = v1 & (joypad2_prev ^ v1);
  if (v1 && v1 == joypad2_prev) {
    if (!--joypad2_keyrepeat_ctr) {
      joypad2_newkeys2 = joypad2_last;
      joypad2_keyrepeat_ctr = joypad_ctr_repeat_next;
    }
  } else {
    joypad2_keyrepeat_ctr = joypad_ctr_repeat_first;
  }
  joypad2_prev = joypad2_last;
  if (enable_debug) {
    if (is_uploading_apu || joypad1_lastkeys != (kButton_Select | kButton_Start | kButton_L | kButton_R)) {
      joypad_dbg_1 = 0;
      joypad_dbg_2 = 0;
      if ((joypad_dbg_flags & 0x4000) == 0) {
        if ((joypad1_lastkeys & (kButton_Select | kButton_L)) == (kButton_Select | kButton_L)) {
          joypad_dbg_1 = joypad1_newkeys;
          joypad1_lastkeys = 0;
          joypad1_newkeys = 0;
        }
        if ((joypad1_lastkeys & (kButton_Select | kButton_R)) == (kButton_Select | kButton_R)) {
          joypad_dbg_2 = joypad1_newkeys;
          joypad1_lastkeys = 0;
          joypad1_newkeys = 0;
        }
        if ((joypad_dbg_2 & 0x80) != 0)
          *(uint16 *)&reg_NMITIMEN ^= 0x30;
        if ((joypad_dbg_2 & 0x8000) != 0) {
          bool v2 = (~joypad_dbg_flags & 0x8000) != 0;
          joypad_dbg_flags ^= 0x8000;
          if (v2) {
            joypad_dbg_missiles_swap = samus_missiles;
            joypad_dbg_super_missiles_swap = samus_super_missiles;
            joypad_dbg_power_bombs_swap = samus_power_bombs;
            samus_missiles = 0;
            samus_super_missiles = 0;
            samus_power_bombs = 0;
          } else {
            samus_missiles = joypad_dbg_missiles_swap;
            samus_super_missiles = joypad_dbg_super_missiles_swap;
            samus_power_bombs = joypad_dbg_power_bombs_swap;
          }
        }
        if ((joypad_dbg_2 & 0x40) != 0)
          joypad_dbg_flags ^= 0x2000;
      }
    } else {
      debug_disable_sounds = 0;
      SoftReset();
    }
  }
}
