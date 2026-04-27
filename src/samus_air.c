#include "physics.h"

#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "samus_env.h"
#include "sm_rtl.h"

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

static const uint8 *Samus_GetFramesForUnderwaterSfx(void) {
  return RomFixedPtr(0x90a514);
}

static uint16 Samus_GetSpinJumpVerticalVelocity(void) {
  return *(uint16 *)((uint8 *)&samus_y_subspeed + 1);
}

static bool Samus_IsSpinJumpSubmerged(void) {
  if ((samus_suit_palette_index & kSamusSuitPalette_Gravity) != 0)
    return false;

  uint16 samus_top = Samus_GetTop_R20();
  if ((fx_y_pos & kLiquidYPos_Disabled) != 0)
    return Samus_IsSubmergedInLavaAcid(samus_top);
  return Samus_IsSubmergedInWater(samus_top);
}

static SamusRejumpWindow Samus_GetSpinJumpRejumpWindow(uint16 vertical_env) {
  if (vertical_env != kLiquidPhysicsType_Air) {
    return (SamusRejumpWindow){
      .min_y_subspeed_hi = kSamusSpinJumpRejumpYVel_SubmergedMin,
      .max_y_subspeed_hi = kSamusSpinJumpRejumpYVel_Max,
    };
  }
  return (SamusRejumpWindow){
    .min_y_subspeed_hi = kSamusSpinJumpRejumpYVel_AirMin,
    .max_y_subspeed_hi = kSamusSpinJumpRejumpYVel_Max,
  };
}

static bool Samus_CanHiJumpRejumpDuringSpin(const SamusRejumpWindow *window) {
  if (!Samus_HasEquip(kSamusEquip_HiJumpBoots) || samus_y_dir != 2)
    return false;

  uint16 vertical_velocity = Samus_GetSpinJumpVerticalVelocity();
  return (int16)(vertical_velocity - window->min_y_subspeed_hi) >= 0
      && sign16(vertical_velocity - window->max_y_subspeed_hi);
}

static void Samus_SetUnusedDfaLowByte(uint8 value) {
  UNUSED_word_7E0DFA = (UNUSED_word_7E0DFA & kUnusedWordHighByteMask) | value;
}

static bool Samus_HasPseudoScrewCharge(void) {
  return !sign16(flare_counter - kSamusPseudoScrewChargeFrames);
}

static bool Samus_IsScrewAttackSpinPose(void) {
  return samus_pose == kPose_81_FaceR_Screwattack
      || samus_pose == kPose_82_FaceL_Screwattack;
}

static SamusSpinJumpContext Samus_BuildSpinJumpContext(void) {
  uint16 vertical_env = liquid_physics_type;
  SamusRejumpWindow rejump_window = Samus_GetSpinJumpRejumpWindow(vertical_env);
  bool submerged = Samus_IsSpinJumpSubmerged();
  return (SamusSpinJumpContext){
    .vertical_env = vertical_env,
    .submerged = submerged,
    .can_rejump = !submerged && Samus_CanHiJumpRejumpDuringSpin(&rejump_window),
    .screw_attack_active = Samus_IsScrewAttackSpinPose(),
    .pseudo_screw_active = Samus_HasPseudoScrewCharge(),
  };
}

static void Samus_TrySpinJumpRejump(const SamusSpinJumpContext *ctx) {
  if (!ctx->can_rejump)
    return;
  Samus_SetUnusedDfaLowByte(1);
  if ((button_config_jump_a & joypad1_newkeys) != 0)
    Samus_InitJump();
}

static void Samus_UpdateSpinJumpContactDamage(const SamusSpinJumpContext *ctx) {
  if (ctx->screw_attack_active) {
    samus_contact_damage_index = kSamusContactDamage_ScrewAttack;
  } else if (ctx->pseudo_screw_active) {
    samus_contact_damage_index = kSamusContactDamage_PseudoScrew;
  }
}

static void Samus_TickSpinJumpLiquidSfx(const SamusSpinJumpContext *ctx) {
  (void)ctx;
  if (samus_anim_frame_timer == 1 && Samus_GetFramesForUnderwaterSfx()[samus_anim_frame])
    QueueSfx1_Max6(kSfx1_UnderwaterSpinJump);
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
  SamusSpinJumpContext ctx = Samus_BuildSpinJumpContext();
  if (!ctx.submerged) {
    Samus_TrySpinJumpRejump(&ctx);
    Samus_UpdateSpinJumpContactDamage(&ctx);
  } else {
    Samus_TickSpinJumpLiquidSfx(&ctx);
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
      samus_contact_damage_index = kSamusContactDamage_PseudoScrew;
  } else {
    samus_contact_damage_index = kSamusContactDamage_ScrewAttack;
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
