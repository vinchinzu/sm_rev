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
    .min_y_vel = 0x280,
    .max_y_vel = 0x500,
  };
  static const SamusSpinJumpRejumpWindow kWaterRejumpWindow = {
    .min_y_vel = 0x80,
    .max_y_vel = 0x500,
  };

  if (!Samus_IsSpinJumpSubmerged()) {
    const SamusSpinJumpRejumpWindow *rejump_window =
        (liquid_physics_type != kLiquidPhysicsType_Air) ? &kWaterRejumpWindow : &kAirRejumpWindow;
    if (Samus_CanHiJumpRejumpDuringSpin(rejump_window)) {
      UNUSED_word_7E0DFA = UNUSED_word_7E0DFA & 0xFF00 | 1;
      if ((button_config_jump_a & joypad1_newkeys) != 0)
        Samus_InitJump();
    }

    if (samus_pose == kPose_81_FaceR_Screwattack || samus_pose == kPose_82_FaceL_Screwattack) {
      samus_contact_damage_index = kSamusContactDamageMode_ScrewAttack;
    } else if (!sign16(flare_counter - 60)) {
      samus_contact_damage_index = kSamusContactDamageMode_PseudoScrew;
    }
  } else if (samus_anim_frame_timer == 1 && Samus_GetFramesForUnderwaterSfx()[samus_anim_frame]) {
    QueueSfx1_Max6(0x2F);
  }

  Samus_SpinJumpMovement();
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

void Samus_Movement_14_WallJumping(void) {
  if (sign16(samus_anim_frame - 23)) {
    if (!sign16(samus_anim_frame - 3) && !sign16(flare_counter - 60))
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
