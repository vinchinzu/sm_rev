// Samus jump and vertical-setup helpers: environment-sensitive initial Y
// speeds for jump / wall jump / bomb jump / knockback, plus per-medium
// gravity selection. Extracted from sm_90.c.
//
// Gravity Suit forces "air" handling for these routines. Otherwise the
// current liquid state is determined from Samus's bottom edge against the FX
// water level or lava/acid level, matching the original Bank 90 branching.

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "physics_config.h"

static const uint16 kEquippedItem_GravitySuit = 0x20;
static const uint16 kEquippedItem_HiJumpBoots = 0x100;
static const uint16 kEquippedItem_SpeedBooster = 0x2000;

enum {
  kSamusVerticalEnv_Air,
  kSamusVerticalEnv_Water,
  kSamusVerticalEnv_LavaAcid,
};

static const uint16 kSamus_InitWallJump_Speed[3] = { 4, 0, 2 };
static const uint16 kSamus_InitWallJump_Subspeed[3] = { 0xa000, 0x4000, 0xa000 };
static const uint16 kSamus_InitWallJump_HiJumpSpeed[3] = { 5, 0, 3 };
static const uint16 kSamus_InitWallJump_HiJumpSubspeed[3] = { 0x8000, 0x8000, 0x8000 };
static const uint16 kSamus_SetSpeedForKnockback_Y_Speed[3] = { 5, 2, 2 };
static const uint16 kSamus_SetSpeedForKnockback_Y_Subspeed[3] = { 0, 0, 0 };
static const uint16 kSamus_InitBombJump_Speed[3] = { 2, 0, 0 };
static const uint16 kSamus_InitBombJump_Subspeed[3] = { 0xc000, 0x1000, 0x1000 };

static uint16 Samus_GetVerticalEnvironmentIndex(void) {
  if ((equipped_items & kEquippedItem_GravitySuit) != 0)
    return kSamusVerticalEnv_Air;

  uint16 samus_bottom = Samus_GetBottom_R18();
  if ((fx_y_pos & 0x8000) == 0) {
    if (sign16(fx_y_pos - samus_bottom) && (fx_liquid_options & 4) == 0)
      return kSamusVerticalEnv_Water;
    return kSamusVerticalEnv_Air;
  }
  if ((lava_acid_y_pos & 0x8000) == 0 && sign16(lava_acid_y_pos - samus_bottom))
    return kSamusVerticalEnv_LavaAcid;
  return kSamusVerticalEnv_Air;
}

static void Samus_ResetVerticalMovementState(void) {
  grapple_walljump_timer = 0;
  reached_ceres_elevator_fade_timer = 0;
  samus_y_dir = 1;
}

static void Samus_AddSpeedBoosterJumpMomentum(void) {
  samus_y_subspeed += samus_x_extra_run_subspeed;
  samus_y_speed += samus_x_extra_run_speed >> 1;
}

void Samus_InitJump(void) {  // 0x9098BC
  uint16 env = Samus_GetVerticalEnvironmentIndex();

  if ((equipped_items & kEquippedItem_HiJumpBoots) != 0) {
    if (env == kSamusVerticalEnv_Air) {
      samus_y_subspeed = g_physics_params.jump_hi_initial_subspeed;
      samus_y_speed = g_physics_params.jump_hi_initial_speed;
    } else if (env == kSamusVerticalEnv_Water) {
      samus_y_subspeed = g_physics_params.jump_hi_underwater_initial_subspeed;
      samus_y_speed = g_physics_params.jump_hi_underwater_initial_speed;
    } else {
      samus_y_subspeed = g_physics_params.jump_hi_lava_acid_initial_subspeed;
      samus_y_speed = g_physics_params.jump_hi_lava_acid_initial_speed;
    }
  } else {
    if (env == kSamusVerticalEnv_Air) {
      samus_y_subspeed = g_physics_params.jump_initial_subspeed;
      samus_y_speed = g_physics_params.jump_initial_speed;
    } else if (env == kSamusVerticalEnv_Water) {
      samus_y_subspeed = g_physics_params.jump_underwater_initial_subspeed;
      samus_y_speed = g_physics_params.jump_underwater_initial_speed;
    } else {
      samus_y_subspeed = g_physics_params.jump_lava_acid_initial_subspeed;
      samus_y_speed = g_physics_params.jump_lava_acid_initial_speed;
    }
  }
  if ((equipped_items & kEquippedItem_SpeedBooster) != 0)
    Samus_AddSpeedBoosterJumpMomentum();
  Samus_ResetVerticalMovementState();
}

void Samus_InitWallJump(void) {  // 0x909949
  uint16 env = Samus_GetVerticalEnvironmentIndex();

  if ((equipped_items & kEquippedItem_HiJumpBoots) != 0) {
    samus_y_subspeed = kSamus_InitWallJump_HiJumpSubspeed[env];
    samus_y_speed = kSamus_InitWallJump_HiJumpSpeed[env];
  } else {
    samus_y_subspeed = kSamus_InitWallJump_Subspeed[env];
    samus_y_speed = kSamus_InitWallJump_Speed[env];
  }
  if ((equipped_items & kEquippedItem_SpeedBooster) != 0)
    Samus_AddSpeedBoosterJumpMomentum();
  Samus_ResetVerticalMovementState();
}

void Samus_SetSpeedForKnockback_Y(void) {  // 0x9099D6
  uint16 env = Samus_GetVerticalEnvironmentIndex();
  samus_y_subspeed = kSamus_SetSpeedForKnockback_Y_Subspeed[env];
  samus_y_speed = kSamus_SetSpeedForKnockback_Y_Speed[env];
  Samus_ResetVerticalMovementState();
}

void Samus_InitBombJump(void) {  // 0x909A2C
  uint16 env = Samus_GetVerticalEnvironmentIndex();
  samus_y_subspeed = kSamus_InitBombJump_Subspeed[env];
  samus_y_speed = kSamus_InitBombJump_Speed[env];
  Samus_ResetVerticalMovementState();
}

void Samus_DetermineAccel_Y(void) {
  switch (Samus_GetVerticalEnvironmentIndex()) {
  case kSamusVerticalEnv_Water:
    samus_y_subaccel = g_physics_params.gravity_underwater_subaccel;
    samus_y_accel = g_physics_params.gravity_accel;
    return;
  case kSamusVerticalEnv_LavaAcid:
    samus_y_subaccel = g_physics_params.gravity_lava_acid_subaccel;
    samus_y_accel = g_physics_params.gravity_accel;
    return;
  default:
    samus_y_subaccel = g_physics_params.gravity_subaccel;
    samus_y_accel = g_physics_params.gravity_accel;
    return;
  }
}

void UNUSED_sub_91FC42(void);

void nullsub_24(void) {}
void nullsub_25(void) {}

static Func_V *const kHandleJumpTrans[28] = {  // 0x91FBBB
  nullsub_24,
  nullsub_24,
  HandleJumpTransition_NormalJump,
  HandleJumpTransition_SpinJump,
  nullsub_24,
  nullsub_24,
  nullsub_24,
  nullsub_24,
  nullsub_24,
  nullsub_24,
  nullsub_24,
  nullsub_24,
  nullsub_24,
  UNUSED_sub_91FC42,
  nullsub_24,
  nullsub_24,
  nullsub_24,
  nullsub_24,
  HandleJumpTransition_SpringBallInAir,
  nullsub_24,
  HandleJumpTransition_WallJump,
  nullsub_24,
  nullsub_24,
  nullsub_24,
  nullsub_24,
  nullsub_25,
  nullsub_24,
  nullsub_24,
};

void HandleJumpTransition(void) {
  kHandleJumpTrans[(samus_movement_type)]();
}


void HandleJumpTransition_WallJump(void) {  // 0x91FC08
  if (samus_prev_movement_type2 != kMovementType_14_WallJumping)
    Samus_InitWallJump();
}

void HandleJumpTransition_SpringBallInAir(void) {  // 0x91FC18
  if (samus_pose == kPose_7F_FaceR_Springball_Air) {
    if (samus_prev_movement_type2 != kMovementType_11_SpringBallOnGround)
      return;
LABEL_6:
    Samus_InitJump();
    return;
  }
  if (samus_pose == kPose_80_FaceL_Springball_Air && samus_prev_movement_type2 == kMovementType_11_SpringBallOnGround)
    goto LABEL_6;
}

void UNUSED_sub_91FC42(void) {  // 0x91FC42
  if (samus_pose == (kPose_44_FaceL_Turn_Crouch | kPose_01_FaceR_Normal | 0x20)) {
    if (samus_prev_pose != 100)
      return;
LABEL_6:
    Samus_InitJump();
    return;
  }
  if (samus_pose == (kPose_44_FaceL_Turn_Crouch | kPose_02_FaceL_Normal | 0x20) && samus_prev_pose == 99)
    goto LABEL_6;
}

void HandleJumpTransition_NormalJump(void) {  // 0x91FC66
  if (samus_pose == kPose_4B_FaceR_Jumptrans
      || samus_pose == kPose_4C_FaceL_Jumptrans
      || !sign16(samus_pose - kPose_55_FaceR_Jumptrans_AimU) && sign16(samus_pose - kPose_5B)) {
    if (samus_prev_pose == kPose_27_FaceR_Crouch || samus_prev_pose == kPose_28_FaceL_Crouch)
      samus_y_pos -= 10;
    Samus_InitJump();
  }
}

void HandleJumpTransition_SpinJump(void) {  // 0x91FC99
  if (samus_prev_movement_type2 != kMovementType_03_SpinJumping
      && samus_prev_movement_type2 != kMovementType_14_WallJumping) {
    Samus_InitJump();
  }
}

void Samus_Func20(void) {  // 0x91FCAF
  if (samus_movement_type == 14) {
    if (samus_anim_frame == 2 && samus_anim_frame_timer == 1) {
      if (samus_pose_x_dir == 4) {
        if (samus_pose == kPose_25_FaceR_Turn_Stand)
          samus_pose = kPose_D6_FaceL_Xray_Stand;
        else
          samus_pose = kPose_DA_FaceL_Xray_Crouch;
      } else if (samus_pose == kPose_26_FaceL_Turn_Stand) {
        samus_pose = kPose_D5_FaceR_Xray_Stand;
      } else {
        samus_pose = kPose_D9_FaceR_Xray_Crouch;
      }
      SamusFunc_F433();
      Samus_SetAnimationFrameIfPoseChanged();
      samus_last_different_pose = samus_prev_pose;
      *(uint16 *)&samus_last_different_pose_x_dir = *(uint16 *)&samus_prev_pose_x_dir;
      samus_prev_pose = samus_pose;
      *(uint16 *)&samus_prev_pose_x_dir = *(uint16 *)&samus_pose_x_dir;
    }
  } else {
    if (samus_pose_x_dir == 4) {
      if ((button_config_right & joypad1_lastkeys) == 0)
        return;
      xray_angle = 256 - xray_angle;
      if (samus_movement_type == 5)
        samus_pose = kPose_44_FaceL_Turn_Crouch;
      else
        samus_pose = kPose_26_FaceL_Turn_Stand;
    } else {
      if ((button_config_left & joypad1_lastkeys) == 0)
        return;
      xray_angle = 256 - xray_angle;
      if (samus_movement_type == kMovementType_05_Crouching)
        samus_pose = kPose_43_FaceR_Turn_Crouch;
      else
        samus_pose = kPose_25_FaceR_Turn_Stand;
    }
    SamusFunc_F433();
    Samus_SetAnimationFrameIfPoseChanged();
    samus_last_different_pose = samus_prev_pose;
    *(uint16 *)&samus_last_different_pose_x_dir = *(uint16 *)&samus_prev_pose_x_dir;
    samus_prev_pose = samus_pose;
    *(uint16 *)&samus_prev_pose_x_dir = *(uint16 *)&samus_pose_x_dir;
  }
}
