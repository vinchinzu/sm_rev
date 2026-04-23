#include "physics.h"

#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "samus_env.h"
#include "sm_rtl.h"

typedef struct SamusSpinJumpRejumpWindow {
  uint16 min_y_vel;
  uint16 max_y_vel;
} SamusSpinJumpRejumpWindow;

enum {
  kSamusSpinJumpRejumpYVel_AirMin = 0x280,
  kSamusSpinJumpRejumpYVel_SubmergedMin = 0x80,
  kSamusSpinJumpRejumpYVel_Max = 0x500,
  kSamusPseudoScrewChargeFrames = 60,
  kSamusYSpeedTerminal = 5,
  kSamusFastFallAnimFrame = 5,
  kSamusFastFallAnimFrameTimer = 8,
  kSamusWallJumpPseudoScrewStartFrame = 3,
  kSamusWallJumpScrewAttackStartFrame = 23,
  kUnusedWordHighByteMask = 0xFF00,
  kSfx1_UnderwaterSpinJump = 0x2F,
};

enum SamusContactDamageMode {
  kSamusContactDamageMode_None = 0,
  kSamusContactDamageMode_SpeedBoost = 1,
  kSamusContactDamageMode_Shinespark = 2,
  kSamusContactDamageMode_ScrewAttack = 3,
  kSamusContactDamageMode_PseudoScrew = 4,
};

enum SamusSuitPaletteVariant {
  kSamusSuitPaletteVariant_Power = 0,
  kSamusSuitPaletteVariant_Varia = 2,
  kSamusSuitPaletteVariant_Gravity = 4,
};

static const uint8 *Samus_GetFramesForUnderwaterSfx(void) {
  return RomFixedPtr(0x90a514);
}

static uint16 Samus_GetSpinJumpVerticalVelocity(void) {
  return *(uint16 *)((uint8 *)&samus_y_subspeed + 1);
}

static bool Samus_IsSpinJumpSubmerged(void) {
  if ((samus_suit_palette_index & kSamusSuitPaletteVariant_Gravity) != 0)
    return false;

  uint16 samus_top = Samus_GetTop_R20();
  if ((fx_y_pos & kLiquidYPos_Disabled) != 0)
    return Samus_IsSubmergedInLavaAcid(samus_top);
  return Samus_IsSubmergedInWater(samus_top);
}

static bool Samus_CanHiJumpRejumpDuringSpin(const SamusSpinJumpRejumpWindow *window) {
  if (!Samus_HasEquip(kSamusEquip_HiJumpBoots) || samus_y_dir != 2)
    return false;

  uint16 vertical_velocity = Samus_GetSpinJumpVerticalVelocity();
  return (int16)(vertical_velocity - window->min_y_vel) >= 0
      && sign16(vertical_velocity - window->max_y_vel);
}

static void Samus_SetUnusedDfaLowByte(uint8 value) {
  UNUSED_word_7E0DFA = (UNUSED_word_7E0DFA & kUnusedWordHighByteMask) | value;
}

static bool Samus_HasPseudoScrewCharge(void) {
  return !sign16(flare_counter - kSamusPseudoScrewChargeFrames);
}

static void Samus_TurningAroundInAirMovement(void) {
  Samus_HandleMovement_X();
  if (!(Samus_CheckAndMoveY() & 1))
    Samus_Move_NoSpeedCalc_Y();
  Samus_CancelSpeedBoost();
  samus_x_extra_run_speed = 0;
  samus_x_extra_run_subspeed = 0;
  input_to_pose_calc = 0;
}

void Samus_Movement_02_NormalJumping(void) {
  Samus_JumpingMovement();
}

void Samus_Movement_03_SpinJumping(void) {  // 0x90A436
  static const SamusSpinJumpRejumpWindow kAirRejumpWindow = {
    .min_y_vel = kSamusSpinJumpRejumpYVel_AirMin,
    .max_y_vel = kSamusSpinJumpRejumpYVel_Max,
  };
  static const SamusSpinJumpRejumpWindow kWaterRejumpWindow = {
    .min_y_vel = kSamusSpinJumpRejumpYVel_SubmergedMin,
    .max_y_vel = kSamusSpinJumpRejumpYVel_Max,
  };

  if (!Samus_IsSpinJumpSubmerged()) {
    const SamusSpinJumpRejumpWindow *rejump_window =
        (liquid_physics_type != kLiquidPhysicsType_Air) ? &kWaterRejumpWindow : &kAirRejumpWindow;
    if (Samus_CanHiJumpRejumpDuringSpin(rejump_window)) {
      Samus_SetUnusedDfaLowByte(1);
      if ((button_config_jump_a & joypad1_newkeys) != 0)
        Samus_InitJump();
    }

    if (samus_pose == kPose_81_FaceR_Screwattack || samus_pose == kPose_82_FaceL_Screwattack) {
      samus_contact_damage_index = kSamusContactDamageMode_ScrewAttack;
    } else if (Samus_HasPseudoScrewCharge()) {
      samus_contact_damage_index = kSamusContactDamageMode_PseudoScrew;
    }
  } else if (samus_anim_frame_timer == 1 && Samus_GetFramesForUnderwaterSfx()[samus_anim_frame]) {
    QueueSfx1_Max6(kSfx1_UnderwaterSpinJump);
  }

  Samus_SpinJumpMovement();
}

void Samus_Movement_06_Falling(void) {
  Samus_FallingMovement();
  if ((samus_pose == kPose_29_FaceR_Fall
       || samus_pose == kPose_2A_FaceL_Fall
       || samus_pose == kPose_67_FaceR_Fall_Gun
       || samus_pose == kPose_68_FaceL_Fall_Gun)
      && !sign16(samus_y_speed - kSamusYSpeedTerminal)) {
    if (sign16(samus_anim_frame - kSamusFastFallAnimFrame)) {
      samus_anim_frame_timer = kSamusFastFallAnimFrameTimer;
      samus_anim_frame = kSamusFastFallAnimFrame;
    }
  }
}

void Samus_Movement_14_WallJumping(void) {
  if (sign16(samus_anim_frame - kSamusWallJumpScrewAttackStartFrame)) {
    if (!sign16(samus_anim_frame - kSamusWallJumpPseudoScrewStartFrame) && Samus_HasPseudoScrewCharge())
      samus_contact_damage_index = kSamusContactDamageMode_PseudoScrew;
  } else {
    samus_contact_damage_index = kSamusContactDamageMode_ScrewAttack;
  }
  Samus_JumpingMovement();
}

void Samus_Movement_19_DamageBoost(void) {
  Samus_JumpingMovement();
}

void Samus_Movement_17_TurningAroundJumping(void) {
  Samus_TurningAroundInAirMovement();
}

void Samus_Movement_18_TurningAroundFalling(void) {
  Samus_TurningAroundInAirMovement();
}
