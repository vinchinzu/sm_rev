// Bomb Torizo attack-entry helpers and opening Chozo orb scheduling.
#include "enemy_torizo_attacks.h"

#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "enemy_types.h"
#include "torizo_config.h"

enum {
  kTorizoSignBit = 0x8000,
  kTorizoParam1_ClearHighBitsMask = 0x1fff,
  kTorizoParam1_FacingLeft = 0,
  kTorizoMod_OpeningAttackDone = 0x1,
  kTorizoMod_OpeningOrbAttackActive = 0x2,

  addr_kTorizo_Ilist_BombOpeningOrbsSignSet = 0xba04,
  addr_kTorizo_Ilist_BombOpeningOrbsSignClear = 0xbe7e,
  addr_kTorizo_Ilist_BombOpeningOrbsContinueSignSet = 0xbd0a,
  addr_kTorizo_Ilist_BombOpeningOrbsContinueSignClear = 0xc184,
};

static bool TorizoAttacks_Param1SignSet(const Enemy_Torizo *E) {
  return (E->toriz_parameter_1 & kTorizoSignBit) != 0;
}

static uint16 TorizoAttacks_SelectByParam1Sign(const Enemy_Torizo *E,
                                               uint16 sign_set_value,
                                               uint16 sign_clear_value) {
  return TorizoAttacks_Param1SignSet(E) ? sign_set_value : sign_clear_value;
}

static void TorizoAttacks_SetParam1State(Enemy_Torizo *E, uint16 state) {
  E->toriz_parameter_1 = (E->toriz_parameter_1 & kTorizoParam1_ClearHighBitsMask) | state;
}

static void TorizoAttacks_SetInstruction(Enemy_Torizo *E, uint16 instruction) {
  E->base.current_instruction = instruction;
  E->base.instruction_timer = 1;
}

static void TorizoAttacks_SpawnEprojBurst(uint16 k, uint16 projectile_id, uint16 count) {
  for (uint16 i = 0; i < count; i++)
    SpawnEprojWithGfx(0, k, projectile_id);
}

static bool TorizoAttacks_OpeningChozoOrbAttackActive(const Enemy_Torizo *E) {
  return (E->toriz_var_08 & kTorizoMod_OpeningOrbAttackActive) != 0;
}

void TorizoAttacks_RunScheduledOpeningChozoOrbWaves(uint16 k, Enemy_Torizo *E) {
  if (!TorizoAttacks_OpeningChozoOrbAttackActive(E) ||
      !TorizoConfig_BombOpeningUsesScheduledChozoOrbWaves() ||
      E->toriz_var_0A == 0) {
    return;
  }
  if (E->toriz_var_0B != 0) {
    --E->toriz_var_0B;
    return;
  }

  TorizoAttacks_SpawnEprojBurst(k, addr_kEproj_BombTorizosChozoOrbs,
                                TorizoConfig_BombOpeningChozoOrbBurstCount());
  --E->toriz_var_0A;
  E->toriz_var_0B = TorizoConfig_BombOpeningChozoOrbWaveIntervalFrames();
}

bool TorizoAttacks_TryStartBombOpeningChozoOrbAttack(uint16 k, Enemy_Torizo *E) {
  if (area_index ||
      !TorizoConfig_BombOpeningAttackIsChozoOrbs() ||
      (E->toriz_var_08 & kTorizoMod_OpeningAttackDone) != 0) {
    return false;
  }

  TorizoAttacks_SetParam1State(E, kTorizoParam1_FacingLeft);
  E->toriz_var_08 |= kTorizoMod_OpeningAttackDone | kTorizoMod_OpeningOrbAttackActive;
  E->toriz_var_0A = TorizoConfig_BombOpeningChozoOrbWaveCount();
  E->toriz_var_0B = 0;
  E->toriz_var_00 = TorizoAttacks_SelectByParam1Sign(E,
      addr_kTorizo_Ilist_BombOpeningOrbsContinueSignSet,
      addr_kTorizo_Ilist_BombOpeningOrbsContinueSignClear);
  TorizoAttacks_SetInstruction(E, TorizoAttacks_SelectByParam1Sign(E,
      addr_kTorizo_Ilist_BombOpeningOrbsSignSet,
      addr_kTorizo_Ilist_BombOpeningOrbsSignClear));
  TorizoAttacks_RunScheduledOpeningChozoOrbWaves(k, E);
  return true;
}

const uint16 *Torizo_Instr_18(uint16 k, const uint16 *jp) {  // 0xAAC5CB
  Enemy_Torizo *E = Get_Torizo(k);
  if (TorizoAttacks_OpeningChozoOrbAttackActive(E) &&
      TorizoConfig_BombOpeningUsesScheduledChozoOrbWaves()) {
    return jp;
  }
  uint16 count = TorizoAttacks_OpeningChozoOrbAttackActive(E)
      ? TorizoConfig_BombOpeningChozoOrbBurstCount()
      : TorizoConfig_BombChozoOrbBurstCount();
  TorizoAttacks_SpawnEprojBurst(k, addr_kEproj_BombTorizosChozoOrbs, count);
  return jp;
}

const uint16 *Torizo_Instr_19(uint16 k, const uint16 *jp) {  // 0xAAC2F7
  Enemy_Torizo *E = Get_Torizo(k);
  E->toriz_var_08 &= ~kTorizoMod_OpeningOrbAttackActive;
  E->toriz_var_0A = 0;
  E->toriz_var_0B = 0;
  return INSTR_RETURN_ADDR(E->toriz_var_00);
}
