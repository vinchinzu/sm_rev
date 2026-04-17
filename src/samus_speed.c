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
  return addr_kSamusSpeedTable_Normal_X;
}
