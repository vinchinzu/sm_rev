// Samus speed-table and acceleration logic: the speed-table dispatch that
// chooses Normal / Water / LavaAcid entries, the base-speed accel/decel
// updater (and its "no decel" sibling used by jumping/falling), the
// grapple-swing speed selector, and the extra-run-speed handler that feeds
// speed boost. Extracted from sm_90.c.
//
// These functions operate on samus_x_base_speed / samus_x_base_subspeed plus
// the related extra-run-speed pair. Override tables live in ROM via the
// addr_kSamusSpeedTable_* enum values; run-mode is additionally overridden
// by g_physics_params so sm_physics.json can hot-reload feel values.

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "physics_config.h"
#include "samus_env.h"

// Extra-run-speed accel applied every frame while running on ground.
// Tables were originally keyed by env — but every call site read only
// the [0] slot. The [1]/[2] entries for water/lava-acid are preserved as
// dead data only to document the original table structure.
static const uint16 kSamus_ExtraRunAccel_Subspeed[3] = { 0x1000, 0x400, 0x400 };
static const uint16 kSamus_ExtraRunCap_SpeedBoostSpeed[3] = { 7, 4, 4 };
static const uint16 kSamus_ExtraRunCap_NormalSpeed[3] = { 2, 1, 0 };

// speed_boost_counter is a packed { high_byte = phase, low_byte = frame
// counter } pair. Phase 4 ("charged / shinespark ready") is what most
// callers test via `(counter & 0xFF00) == 0x400`. Expose the phase as a
// shifted enum so comparisons read as the state machine they really are.
enum {
  kSpeedBoostPhase_Charged = 4,
  kSamusSpeedTableEntrySize = 12,
  kSamusMovementTypeCount = 28,
};
#define SPEED_BOOST_PHASE(ctr) (((ctr) & 0xFF00) >> 8)

static void Samus_GetScaledRunPair(uint16 base_speed, uint16 base_subspeed,
                                   uint16 *scaled_speed, uint16 *scaled_subspeed) {
  uint64 full = ((uint64)base_speed << 16) | base_subspeed;
  full = (full * g_physics_mods.run_speed_scale_percent) / 100;
  *scaled_speed = (uint16)(full >> 16);
  *scaled_subspeed = (uint16)full;
}

static bool Samus_IsMovementSpeedTableEntry(uint16 table_entry, uint16 table_base) {
  int delta = (int)table_entry - (int)table_base;
  return delta >= 0
      && delta < kSamusSpeedTableEntrySize * kSamusMovementTypeCount
      && (delta % kSamusSpeedTableEntrySize) == 0;
}

static bool Samus_UsesPhysicsRunOverride(uint16 table_entry) {
  return Samus_IsMovementSpeedTableEntry(table_entry, addr_kSamusSpeedTable_Normal_X)
      || Samus_IsMovementSpeedTableEntry(table_entry, addr_kSamusSpeedTable_Water_X)
      || Samus_IsMovementSpeedTableEntry(table_entry, addr_kSamusSpeedTable_LavaAcid_X);
}

static void Samus_ClearExtraRunSpeedIfNoMomentum(void) {
  if (!samus_has_momentum_flag) {
    samus_x_extra_run_speed = 0;
    samus_x_extra_run_subspeed = 0;
  }
}

static bool Samus_IsRunningOnGround(void) {
  // movement_type 1 = Samus_Movement_01_Running (see physics.c handler table).
  return samus_movement_type == 1
      && (button_config_run_b & joypad1_lastkeys) != 0;
}

static void Samus_TickExtraRunSpeed_Boosted(void) {
  uint16 cap_speed, cap_subspeed;
  uint16 accel_speed, accel_subspeed;
  Samus_GetScaledRunPair(kSamus_ExtraRunCap_SpeedBoostSpeed[0], 0, &cap_speed, &cap_subspeed);
  Samus_GetScaledRunPair(0, kSamus_ExtraRunAccel_Subspeed[0], &accel_speed, &accel_subspeed);
  if (!samus_has_momentum_flag) {
    samus_has_momentum_flag = 1;
    special_samus_palette_timer = 1;
    special_samus_palette_frame = 0;
    speed_boost_counter = kSpeedBoostToCtr[0];
  }
  if (IsGreaterThanQuirked(samus_x_extra_run_speed, samus_x_extra_run_subspeed, cap_speed, cap_subspeed)
      || (samus_x_extra_run_speed == cap_speed && samus_x_extra_run_subspeed == cap_subspeed)) {
    samus_x_extra_run_speed = cap_speed;
    samus_x_extra_run_subspeed = cap_subspeed;
    return;
  }
  AddToHiLo(&samus_x_extra_run_speed, &samus_x_extra_run_subspeed,
      __PAIR32__(accel_speed, accel_subspeed));
}

static void Samus_TickExtraRunSpeed_Normal(void) {
  uint16 cap_speed, cap_subspeed;
  uint16 accel_speed, accel_subspeed;
  Samus_GetScaledRunPair(kSamus_ExtraRunCap_NormalSpeed[0], 0, &cap_speed, &cap_subspeed);
  Samus_GetScaledRunPair(0, kSamus_ExtraRunAccel_Subspeed[0], &accel_speed, &accel_subspeed);
  if (!samus_has_momentum_flag) {
    samus_has_momentum_flag = 1;
    speed_boost_counter = 0;
  }
  if (IsGreaterThanQuirked(samus_x_extra_run_speed, samus_x_extra_run_subspeed, cap_speed, cap_subspeed)
      || (samus_x_extra_run_speed == cap_speed && samus_x_extra_run_subspeed == cap_subspeed)) {
    samus_x_extra_run_speed = cap_speed;
    samus_x_extra_run_subspeed = cap_subspeed;
    return;
  }
  AddToHiLo(&samus_x_extra_run_speed, &samus_x_extra_run_subspeed,
      __PAIR32__(accel_speed, accel_subspeed));
}

void Samus_HandleExtraRunspeedX(void) {  // 0x90973E
  // If submerged (without Gravity Suit) or not running on the ground,
  // extra run speed cannot build up. Otherwise branch on Speed Booster.
  if (Samus_GetVerticalEnv() != kSamusVerticalEnv_Air
      || !Samus_IsRunningOnGround()) {
    Samus_ClearExtraRunSpeedIfNoMomentum();
  } else if (Samus_HasEquip(kSamusEquip_SpeedBooster)) {
    Samus_TickExtraRunSpeed_Boosted();
  } else {
    Samus_TickExtraRunSpeed_Normal();
  }
  if (SPEED_BOOST_PHASE(speed_boost_counter) == kSpeedBoostPhase_Charged)
    samus_contact_damage_index = 1;
}

int32 Samus_CalcBaseSpeed_X(uint16 k) {  // 0x909A7E
  SamusSpeedTableEntry *sste = get_SamusSpeedTableEntry(k);
  uint16 accel = sste->accel, accel_sub = sste->accel_sub;
  uint16 decel = sste->decel, decel_sub = sste->decel_sub;
  uint16 max_speed = sste->max_speed, max_speed_sub = sste->max_speed_sub;

  if (Samus_UsesPhysicsRunOverride(k)) {
    accel = g_physics_params.run_accel;
    accel_sub = g_physics_params.run_accel_sub;
    decel = g_physics_params.run_decel;
    decel_sub = g_physics_params.run_decel_sub;
    max_speed = g_physics_params.run_max_speed;
    max_speed_sub = g_physics_params.run_max_speed_sub;
  }

  if (samus_x_accel_mode) {
    int32 delta = samus_x_decel_mult ?
        __PAIR32__(Mult8x8(samus_x_decel_mult, decel) >> 8, Mult8x8(samus_x_decel_mult, HIBYTE(decel_sub))) :
        __PAIR32__(decel, decel_sub);
    AddToHiLo(&samus_x_base_speed, &samus_x_base_subspeed, -delta);
    if ((int16)samus_x_base_speed < 0) {
      samus_x_base_speed = 0;
      samus_x_base_subspeed = 0;
      samus_x_accel_mode = 0;
    }
  } else {
    AddToHiLo(&samus_x_base_speed, &samus_x_base_subspeed, __PAIR32__(accel, accel_sub));
    if (IsGreaterThanQuirked(samus_x_base_speed, samus_x_base_subspeed, max_speed, max_speed_sub)) {
      samus_x_base_speed = max_speed;
      samus_x_base_subspeed = max_speed_sub;
    }
  }
  return __PAIR32__(samus_x_base_speed, samus_x_base_subspeed);
}

Pair_Bool_Amt Samus_CalcBaseSpeed_NoDecel_X(uint16 k) {  // 0x909B1F
  SamusSpeedTableEntry *sste = get_SamusSpeedTableEntry(k);
  uint16 accel = sste->accel, accel_sub = sste->accel_sub;
  uint16 decel = sste->decel, decel_sub = sste->decel_sub;
  uint16 max_speed = sste->max_speed, max_speed_sub = sste->max_speed_sub;

  if (Samus_UsesPhysicsRunOverride(k)) {
    accel = g_physics_params.run_accel;
    accel_sub = g_physics_params.run_accel_sub;
    decel = g_physics_params.run_decel;
    decel_sub = g_physics_params.run_decel_sub;
    max_speed = g_physics_params.run_max_speed;
    max_speed_sub = g_physics_params.run_max_speed_sub;
  }

  bool rv = false;
  if ((samus_x_accel_mode & 1) != 0) {
    int32 delta = samus_x_decel_mult ?
      __PAIR32__(Mult8x8(samus_x_decel_mult, decel) >> 8, Mult8x8(samus_x_decel_mult, HIBYTE(decel_sub))) :
      __PAIR32__(decel, decel_sub);
    AddToHiLo(&samus_x_base_speed, &samus_x_base_subspeed, -delta);
    if ((int16)samus_x_base_speed < 0) {
      samus_x_base_speed = 0;
      samus_x_base_subspeed = 0;
      samus_x_accel_mode = 0;
    }
  } else {
    AddToHiLo(&samus_x_base_speed, &samus_x_base_subspeed, __PAIR32__(accel, accel_sub));
    if (IsGreaterThanQuirked(samus_x_base_speed, samus_x_base_subspeed, max_speed, max_speed_sub)) {
      samus_x_base_speed = max_speed;
      samus_x_base_subspeed = max_speed_sub;
      rv = true;
    }
  }
  return (Pair_Bool_Amt) { rv, __PAIR32__(samus_x_base_speed, samus_x_base_subspeed) };
}

uint16 Samus_DetermineSpeedTableEntryPtr_X(void) {  // 0x909BD1
  // Note: the "Air" case leaves samus_x_speed_table_pointer untouched
  // (matches Bank 90 — caller is expected to pre-set it to Normal).
  switch (Samus_GetVerticalEnv()) {
  case kSamusVerticalEnv_Water:
    samus_x_speed_table_pointer = addr_kSamusSpeedTable_Water_X;
    break;
  case kSamusVerticalEnv_LavaAcid:
    samus_x_speed_table_pointer = addr_kSamusSpeedTable_LavaAcid_X;
    break;
  default:
    break;
  }
  return samus_x_speed_table_pointer + 12 * samus_movement_type;
}

uint16 Samus_DetermineGrappleSwingSpeed_X(void) {  // 0x909C21
  // Bank 90 quirk preserved: the LavaAcid branch in the original had
  // no grapple-swing table and fell off the end without returning
  // (uninitialized return value). Grappling while submerged in
  // lava/acid is gameplay-impossible, so the quirk isn't observable.
  switch (Samus_GetVerticalEnv()) {
  case kSamusVerticalEnv_Water:
    return addr_stru_909F3D;
  case kSamusVerticalEnv_Air:
    return addr_stru_909F31;
  default:
    Unreachable();
    return addr_stru_909F31;
  }
}

void nullsub_17(void) {}

static Func_V *const off_91E6E1[28] = {  // 0x91E633
  SamusFunc_E633_0,
  nullsub_17,
  nullsub_17,
  SamusFunc_E633_3,
  SamusFunc_E633_4,
  nullsub_17,
  nullsub_17,
  nullsub_17,
  SamusFunc_E633_4,
  nullsub_17,
  nullsub_17,
  nullsub_17,
  nullsub_17,
  nullsub_17,
  nullsub_17,
  nullsub_17,
  nullsub_17,
  SamusFunc_E633_17,
  SamusFunc_E633_17,
  SamusFunc_E633_17,
  SamusFunc_E633_20,
  nullsub_17,
  nullsub_17,
  nullsub_17,
  nullsub_17,
  nullsub_17,
  nullsub_17,
  nullsub_17,
};

void SamusFunc_E633(void) {
  off_91E6E1[samus_movement_type]();
  if ((equipped_items & 0x2000) != 0) {
    if (samus_has_momentum_flag && !speed_boost_counter) {
      special_samus_palette_timer = speed_boost_counter;
      special_samus_palette_frame = 0;
      speed_boost_counter = kSpeedBoostToCtr[0];
    }
  } else {
    speed_echoes_index = 0;
    speed_echo_xspeed[0] = 0;
    speed_echo_xspeed[1] = 0;
    samus_has_momentum_flag = 0;
    speed_boost_counter = 0;
    special_samus_palette_frame = 0;
    special_samus_palette_timer = 0;
    speed_echo_xpos[0] = 0;
    speed_echo_xpos[1] = 0;
    speed_echo_ypos[0] = 0;
    speed_echo_ypos[1] = 0;
  }
  if (grapple_beam_function == FUNC16(GrappleBeamFunc_Inactive)) {
    if ((equipped_beams & 0x1000) != 0) {
      if (!sign16(flare_counter - 16))
        QueueSfx1_Max6(0x41);
    } else {
      flare_counter = 0;
      flare_animation_frame = 0;
      flare_slow_sparks_anim_frame = 0;
      flare_fast_sparks_anim_frame = 0;
      flare_animation_timer = 0;
      flare_slow_sparks_anim_timer = 0;
      flare_fast_sparks_anim_timer = 0;
    }
  } else {
    LoadProjectilePalette(2);
    QueueSfx1_Max6(6);
  }
  Samus_LoadSuitPalette();
  if (sign16(samus_health - 31))
    QueueSfx3_Max6(2);
}

void Samus_UpdatePreviousPose_0(void) {  // 0x91E719
  samus_last_different_pose = samus_prev_pose;
  *(uint16 *)&samus_last_different_pose_x_dir = *(uint16 *)&samus_prev_pose_x_dir;
  samus_prev_pose = samus_pose;
  *(uint16 *)&samus_prev_pose_x_dir = *(uint16 *)&samus_pose_x_dir;
}

void SamusFunc_E633_0(void) {  // 0x91E733
  if (samus_pose) {
    if (samus_pose == kPose_9B_FaceF_VariaGravitySuit && (equipped_items & 1) == 0 && (equipped_items & 0x20) == 0) {
      samus_pose = kPose_00_FaceF_Powersuit;
      goto LABEL_10;
    }
  } else if ((equipped_items & 1) != 0 || (equipped_items & 0x20) != 0) {
    samus_pose = kPose_9B_FaceF_VariaGravitySuit;
LABEL_10:
    SamusFunc_F433();
    Samus_SetAnimationFrameIfPoseChanged();
    Samus_UpdatePreviousPose_0();
  }
}

void SamusFunc_E633_3(void) {  // 0x91E776
  if (samus_pose_x_dir == 4)
    *(uint16 *)&samus_prev_pose_x_dir = 260;
  else
    *(uint16 *)&samus_prev_pose_x_dir = 264;
  if (samus_pose != kPose_81_FaceR_Screwattack && samus_pose != kPose_82_FaceL_Screwattack) {
    if (samus_pose != kPose_1B_FaceR_SpaceJump && samus_pose != kPose_1C_FaceL_SpaceJump)
      goto LABEL_18;
    if ((equipped_items & 8) != 0) {
      if (samus_pose_x_dir == 4)
        samus_pose = kPose_82_FaceL_Screwattack;
      else
        samus_pose = kPose_81_FaceR_Screwattack;
      goto LABEL_18;
    }
    goto LABEL_15;
  }
  if ((equipped_items & 8) == 0) {
LABEL_15:
    if (samus_pose_x_dir == 4)
      samus_pose = kPose_1A_FaceL_SpinJump;
    else
      samus_pose = kPose_19_FaceR_SpinJump;
  }
LABEL_18:
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
  if (samus_pose_x_dir == 4)
    *(uint16 *)&samus_prev_pose_x_dir = 772;
  else
    *(uint16 *)&samus_prev_pose_x_dir = 776;
  Samus_UpdatePreviousPose_0();
}

void SamusFunc_E633_4(void) {  // 0x91E83A
  if ((equipped_items & 2) != 0) {
    if (samus_pose_x_dir == 4)
      samus_pose = kPose_7A_FaceL_Springball_Ground;
    else
      samus_pose = kPose_79_FaceR_Springball_Ground;
    SamusFunc_F433();
    Samus_SetAnimationFrameIfPoseChanged();
    Samus_UpdatePreviousPose_0();
  }
}

void SamusFunc_E633_17(void) {  // 0x91E867
  if ((equipped_items & 2) == 0) {
    if (samus_pose_x_dir == 4)
      samus_pose = kPose_41_FaceL_Morphball_Ground;
    else
      samus_pose = kPose_1D_FaceR_Morphball_Ground;
    SamusFunc_F433();
    Samus_SetAnimationFrameIfPoseChanged();
    Samus_UpdatePreviousPose_0();
  }
}

void SamusFunc_E633_20(void) {  // 0x91E894
  if ((equipped_items & 8) != 0)
    samus_anim_frame = 23;
  else
    samus_anim_frame = 3;
}
