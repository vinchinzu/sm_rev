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

static const uint16 kSamus_HandleExtraRunspeedX_Tab0[3] = { 0, 0, 0 };
static const uint16 kSamus_HandleExtraRunspeedX_Tab1[3] = { 0x1000, 0x400, 0x400 };
static const uint16 kSamus_HandleExtraRunspeedX_Tab2[3] = { 7, 4, 4 };
static const uint16 kSamus_HandleExtraRunspeedX_Tab3[3] = { 0, 0, 0 };
static const uint16 kSamus_HandleExtraRunspeedX_Tab4[3] = { 2, 1, 0 };
static const uint16 kSamus_HandleExtraRunspeedX_Tab5[3] = { 0, 0, 0 };

void Samus_HandleExtraRunspeedX(void) {  // 0x90973E
  if ((equipped_items & 0x20) == 0) {
    uint16 r18 = Samus_GetBottom_R18();
    if ((fx_y_pos & 0x8000) != 0) {
      if ((lava_acid_y_pos & 0x8000) == 0 && sign16(lava_acid_y_pos - r18)) {
LABEL_24:
        if (!samus_has_momentum_flag) {
          samus_x_extra_run_speed = 0;
          samus_x_extra_run_subspeed = 0;
        }
        goto LABEL_26;
      }
    } else if (sign16(fx_y_pos - r18) && (fx_liquid_options & 4) == 0) {
      goto LABEL_24;
    }
  }
  if (samus_movement_type != 1 || (button_config_run_b & joypad1_lastkeys) == 0)
    goto LABEL_24;
  if ((equipped_items & 0x2000) != 0) {
    if (!samus_has_momentum_flag) {
      samus_has_momentum_flag = 1;
      special_samus_palette_timer = 1;
      special_samus_palette_frame = 0;
      speed_boost_counter = kSpeedBoostToCtr[0];
    }
    if ((int16)(samus_x_extra_run_speed - kSamus_HandleExtraRunspeedX_Tab2[0]) >= 0
        && (int16)(samus_x_extra_run_subspeed - kSamus_HandleExtraRunspeedX_Tab3[0]) >= 0) {
      samus_x_extra_run_speed = kSamus_HandleExtraRunspeedX_Tab2[0];
      samus_x_extra_run_subspeed = kSamus_HandleExtraRunspeedX_Tab3[0];
      goto LABEL_26;
    }
  } else {
    if (!samus_has_momentum_flag) {
      samus_has_momentum_flag = 1;
      speed_boost_counter = 0;
    }
    if ((int16)(samus_x_extra_run_speed - kSamus_HandleExtraRunspeedX_Tab4[0]) >= 0
        && (int16)(samus_x_extra_run_subspeed - kSamus_HandleExtraRunspeedX_Tab5[0]) >= 0) {
      samus_x_extra_run_speed = kSamus_HandleExtraRunspeedX_Tab4[0];
      samus_x_extra_run_subspeed = kSamus_HandleExtraRunspeedX_Tab5[0];
      goto LABEL_26;
    }
  }
  AddToHiLo(&samus_x_extra_run_speed, &samus_x_extra_run_subspeed,
      __PAIR32__(kSamus_HandleExtraRunspeedX_Tab0[0], kSamus_HandleExtraRunspeedX_Tab1[0]));
LABEL_26:
  if ((speed_boost_counter & 0xFF00) == 1024)
    samus_contact_damage_index = 1;
}

int32 Samus_CalcBaseSpeed_X(uint16 k) {  // 0x909A7E
  SamusSpeedTableEntry *sste = get_SamusSpeedTableEntry(k);
  uint16 accel = sste->accel, accel_sub = sste->accel_sub;
  uint16 decel = sste->decel, decel_sub = sste->decel_sub;
  uint16 max_speed = sste->max_speed, max_speed_sub = sste->max_speed_sub;

  if (k == addr_kSamusSpeedTable_Normal_X) {
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

  if (k == addr_kSamusSpeedTable_Normal_X) {
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
  if ((equipped_items & 0x20) == 0) {
    uint16 r18 = Samus_GetBottom_R18();
    if ((fx_y_pos & 0x8000) != 0) {
      if ((lava_acid_y_pos & 0x8000) == 0 && sign16(lava_acid_y_pos - r18))
        samus_x_speed_table_pointer = addr_kSamusSpeedTable_LavaAcid_X;
    } else if (sign16(fx_y_pos - r18) && (fx_liquid_options & 4) == 0) {
      samus_x_speed_table_pointer = addr_kSamusSpeedTable_Water_X;
    }
  }
  return samus_x_speed_table_pointer + 12 * samus_movement_type;
}

uint16 Samus_DetermineGrappleSwingSpeed_X(void) {  // 0x909C21
  if ((equipped_items & 0x20) != 0)
    return addr_stru_909F31;
  uint16 r18 = Samus_GetBottom_R18();
  if ((fx_y_pos & 0x8000) == 0) {
    if (sign16(fx_y_pos - r18) && (fx_liquid_options & 4) == 0)
      return addr_stru_909F3D;
    return addr_stru_909F31;
  }
  if ((lava_acid_y_pos & 0x8000) != 0 || !sign16(lava_acid_y_pos - r18))
    return addr_stru_909F31;
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
