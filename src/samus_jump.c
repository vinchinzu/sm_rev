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
#include "samus_env.h"

static void Samus_ResetVerticalMovementState(void) {
  grapple_walljump_timer = 0;
  reached_ceres_elevator_fade_timer = 0;
  samus_y_dir = 1;
}

static void Samus_AddSpeedBoosterJumpMomentum(void) {
  samus_y_subspeed += samus_x_extra_run_subspeed;
  samus_y_speed += samus_x_extra_run_speed >> 1;
}

// Apply an env-indexed { speed[env], subspeed[env] } pair from
// g_physics_params to Samus's Y speed registers.
static void Samus_ApplyJumpImpulse(const uint16 speed[3], const uint16 subspeed[3], uint16 env) {
  samus_y_speed = speed[env];
  samus_y_subspeed = subspeed[env];
}

void Samus_InitJump(void) {  // 0x9098BC
  uint16 env = Samus_GetVerticalEnv();
  if (Samus_HasEquip(kSamusEquip_HiJumpBoots)) {
    Samus_ApplyJumpImpulse(g_physics_params.jump_hi_initial_speed,
                           g_physics_params.jump_hi_initial_subspeed, env);
  } else {
    Samus_ApplyJumpImpulse(g_physics_params.jump_initial_speed,
                           g_physics_params.jump_initial_subspeed, env);
  }
  if (Samus_HasEquip(kSamusEquip_SpeedBooster))
    Samus_AddSpeedBoosterJumpMomentum();
  Samus_ResetVerticalMovementState();
}

void Samus_InitWallJump(void) {  // 0x909949
  uint16 env = Samus_GetVerticalEnv();
  if (Samus_HasEquip(kSamusEquip_HiJumpBoots)) {
    Samus_ApplyJumpImpulse(g_physics_params.wall_jump_hi_initial_speed,
                           g_physics_params.wall_jump_hi_initial_subspeed, env);
  } else {
    Samus_ApplyJumpImpulse(g_physics_params.wall_jump_initial_speed,
                           g_physics_params.wall_jump_initial_subspeed, env);
  }
  if (Samus_HasEquip(kSamusEquip_SpeedBooster))
    Samus_AddSpeedBoosterJumpMomentum();
  Samus_ResetVerticalMovementState();
}

void Samus_SetSpeedForKnockback_Y(void) {  // 0x9099D6
  Samus_ApplyJumpImpulse(g_physics_params.knockback_y_initial_speed,
                         g_physics_params.knockback_y_initial_subspeed,
                         Samus_GetVerticalEnv());
  Samus_ResetVerticalMovementState();
}

void Samus_InitBombJump(void) {  // 0x909A2C
  Samus_ApplyJumpImpulse(g_physics_params.bomb_jump_initial_speed,
                         g_physics_params.bomb_jump_initial_subspeed,
                         Samus_GetVerticalEnv());
  Samus_ResetVerticalMovementState();
}

void Samus_DetermineAccel_Y(void) {
  switch (Samus_GetVerticalEnv()) {
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
  // Fire a fresh jump when Samus leaves the ground as springball —
  // either facing direction.
  bool airborne_springball =
      (samus_pose == kPose_7F_FaceR_Springball_Air
       || samus_pose == kPose_80_FaceL_Springball_Air)
      && samus_prev_movement_type2 == kMovementType_11_SpringBallOnGround;
  if (airborne_springball)
    Samus_InitJump();
}

void UNUSED_sub_91FC42(void) {  // 0x91FC42
  // Dead code in the final ROM — the pose constants combined with the
  // prev-pose sentinels don't match any real state transition — but
  // preserved byte-for-byte so the function address table still links.
  uint16 target_r = kPose_44_FaceL_Turn_Crouch | kPose_01_FaceR_Normal | 0x20;
  uint16 target_l = kPose_44_FaceL_Turn_Crouch | kPose_02_FaceL_Normal | 0x20;
  bool match_r = (samus_pose == target_r) && (samus_prev_pose == 100);
  bool match_l = (samus_pose == target_l) && (samus_prev_pose == 99);
  if (match_r || match_l)
    Samus_InitJump();
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
