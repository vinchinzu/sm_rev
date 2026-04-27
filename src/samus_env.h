// Shared helpers for Samus environment + equipment checks that were
// previously duplicated inline as magic-hex bitfield tests across
// samus_jump.c, samus_speed.c, samus_collision.c, and similar files.
//
// Semantics mirror the original Bank 90 branching exactly:
//   - Gravity Suit forces the "Air" index for the vertical-environment
//     classifier. Otherwise the current liquid state is determined from
//     Samus's bottom edge vs the FX water level or lava/acid level.
//   - Speed-booster "extra run" and related palette logic branch on the
//     same pair of equip bits, so they live here alongside the env query.

#pragma once
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"

// Equipment bitfield masks (subset used by Samus motion/physics).
// Full set lives in ida_types.h's kEquipment_* enum, but the speed /
// jump / runspeed code historically used raw 0x20 / 0x100 / 0x2000
// literals. These named aliases keep each call site readable without
// forcing a dependency on the whole kEquipment_ enum ordering.
enum {
  kSamusEquip_VariaSuit    = 0x01,
  kSamusEquip_SpringBall   = 0x02,
  kSamusEquip_MorphBall    = 0x04,
  kSamusEquip_ScrewAttack  = 0x08,
  kSamusEquip_GravitySuit  = 0x20,
  kSamusEquip_HiJumpBoots  = 0x100,
  kSamusEquip_SpeedBooster = 0x2000,
};

typedef enum SamusContactDamageMode {
  kSamusContactDamage_None = 0,
  kSamusContactDamage_SpeedBoost = 1,
  kSamusContactDamage_Shinespark = 2,
  kSamusContactDamage_ScrewAttack = 3,
  kSamusContactDamage_PseudoScrew = 4,
} SamusContactDamageMode;

typedef enum SamusPoseXDirection {
  kSamusPoseXDir_FaceRight = 4,
  kSamusPoseXDir_FaceLeft = 8,
} SamusPoseXDirection;

enum {
  kSpeedBoostCounter_Charged = 0x0400,
};

// fx_liquid_options bit 2 = "liquid is pass-through" (treated as air).
enum {
  kFxLiquidOptions_Passthrough = 0x04,
};

// Sign bit on fx_y_pos / lava_acid_y_pos means "no water / no lava",
// i.e. those layers are disabled for the current room.
enum {
  kLiquidYPos_Disabled = 0x8000,
};

// Vertical-environment classification used by jump init, gravity
// selection, speed-table lookup, and grapple-swing speed.
enum {
  kSamusVerticalEnv_Air      = 0,
  kSamusVerticalEnv_Water    = 1,
  kSamusVerticalEnv_LavaAcid = 2,
};

static inline bool Samus_HasEquip(uint16 mask) {
  return (equipped_items & mask) != 0;
}

static inline bool Samus_IsSubmergedInWater(uint16 samus_bottom) {
  if ((fx_y_pos & kLiquidYPos_Disabled) != 0)
    return false;
  if ((fx_liquid_options & kFxLiquidOptions_Passthrough) != 0)
    return false;
  return sign16(fx_y_pos - samus_bottom);
}

static inline bool Samus_IsSubmergedInLavaAcid(uint16 samus_bottom) {
  if ((lava_acid_y_pos & kLiquidYPos_Disabled) != 0)
    return false;
  return sign16(lava_acid_y_pos - samus_bottom);
}

// Returns one of kSamusVerticalEnv_*.
//
// Gravity Suit short-circuits to Air. Otherwise: if the water layer is
// active, a submerged bottom edge classifies as Water; else if lava/acid
// is active and submerged, LavaAcid; else Air. Matches Bank 90's
// original ordering — water wins over lava/acid when both are active.
static inline uint16 Samus_GetVerticalEnv(void) {
  if (Samus_HasEquip(kSamusEquip_GravitySuit))
    return kSamusVerticalEnv_Air;

  uint16 samus_bottom = Samus_GetBottom_R18();
  if ((fx_y_pos & kLiquidYPos_Disabled) == 0) {
    if (Samus_IsSubmergedInWater(samus_bottom))
      return kSamusVerticalEnv_Water;
    return kSamusVerticalEnv_Air;
  }
  if (Samus_IsSubmergedInLavaAcid(samus_bottom))
    return kSamusVerticalEnv_LavaAcid;
  return kSamusVerticalEnv_Air;
}
