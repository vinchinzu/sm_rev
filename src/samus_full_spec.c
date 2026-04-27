#include "samus_full_spec.h"

#include "config.h"
#include "funcs.h"
#include "ida_types.h"
#include "samus_env.h"
#include "variables.h"

enum {
  kFullSpec_MaxHealth = 1499,
  kFullSpec_MaxReserveHealth = 400,
  kFullSpec_MaxMissiles = 230,
  kFullSpec_MaxSuperMissiles = 50,
  kFullSpec_MaxPowerBombs = 50,

  kFullSpec_AllItems = 0xf337,
  kFullSpec_AllBeamsCollected = 0x100f,
  kFullSpec_EquippedBeamSet = 0x100b,
  kFullSpec_GravitySuitPalette = 4,
  kFullSpec_ReserveAuto = 1,
};

static void SamusFullSpec_MarkAllItemPickupsCollected(void) {
  for (uint16 offset = 0; offset < 64; offset += 2) {
    WORD(room_chozo_bits[offset]) = 0xffff;
    *(uint16 *)&item_bit_array[offset] = 0xffff;
  }
}

static void SamusFullSpec_NormalizeScrewAttackPose(void) {
  uint16 screw_pose = 0;
  switch (samus_pose) {
  case kPose_19_FaceR_SpinJump:
  case kPose_1B_FaceR_SpaceJump:
  case kPose_81_FaceR_Screwattack:
    screw_pose = kPose_81_FaceR_Screwattack;
    break;
  case kPose_1A_FaceL_SpinJump:
  case kPose_1C_FaceL_SpaceJump:
  case kPose_82_FaceL_Screwattack:
    screw_pose = kPose_82_FaceL_Screwattack;
    break;
  default:
    break;
  }

  if (!screw_pose)
    return;

  if (samus_pose != screw_pose) {
    samus_pose = screw_pose;
    SamusFunc_F433();
    Samus_SetAnimationFrameIfPoseChanged();
  }

  if (samus_movement_type == kMovementType_03_SpinJumping)
    samus_contact_damage_index = kSamusContactDamage_ScrewAttack;
}

void SamusFullSpec_Apply(void) {
  samus_max_health = kFullSpec_MaxHealth;
  samus_health = kFullSpec_MaxHealth;
  samus_max_reserve_health = kFullSpec_MaxReserveHealth;
  samus_reserve_health = kFullSpec_MaxReserveHealth;
  reserve_health_mode = kFullSpec_ReserveAuto;

  samus_max_missiles = kFullSpec_MaxMissiles;
  samus_missiles = kFullSpec_MaxMissiles;
  samus_max_super_missiles = kFullSpec_MaxSuperMissiles;
  samus_super_missiles = kFullSpec_MaxSuperMissiles;
  samus_max_power_bombs = kFullSpec_MaxPowerBombs;
  samus_power_bombs = kFullSpec_MaxPowerBombs;

  collected_items = kFullSpec_AllItems;
  equipped_items = kFullSpec_AllItems;
  collected_beams = kFullSpec_AllBeamsCollected;
  equipped_beams = kFullSpec_EquippedBeamSet;
  samus_suit_palette_index = kFullSpec_GravitySuitPalette;

  SamusFullSpec_MarkAllItemPickupsCollected();
  SamusFullSpec_NormalizeScrewAttackPose();
}

void SamusFullSpec_ApplyIfEnabled(void) {
  if (g_config.full_spec)
    SamusFullSpec_Apply();
}
