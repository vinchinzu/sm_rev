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
