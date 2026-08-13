#include "mini_multiplayer_weapons.h"

#include "ida_types.h"
#include "multi_samus.h"
#include "variables.h"

enum { kMiniMultiplayerStartingMissiles = 3 };

static bool MiniMultiplayerWeapons_ProjectileSlotActive(int slot) {
  if ((unsigned)slot >= kSamusProjectileSlotCount)
    return false;
  return projectile_type[slot] != 0 ||
         projectile_damage[slot] != 0 ||
         projectile_bomb_instruction_ptr[slot] != 0;
}

static uint16 MiniMultiplayerWeapons_OwnedProjectileCount(
    const MiniGameState *state, int player_index) {
  uint8 owner = (uint8)(player_index + 1);
  uint16 count = 0;
  for (int slot = 0; slot < kSamusProjectileSlotCount; slot++) {
    if (state->projectile_state.owner_by_slot[slot] == owner &&
        MiniMultiplayerWeapons_ProjectileSlotActive(slot)) {
      count++;
    }
  }
  return count;
}

static void MiniMultiplayerWeapons_ApplyPlayer(int player_index, void *context) {
  MiniGameState *state = (MiniGameState *)context;
  if (state == NULL || player_index < 0 || player_index >= state->player_count)
    return;

  const MiniPlayerWeaponState *weapon = &state->players[player_index].weapon;
  hud_item_index = weapon->selected == kMiniWeapon_Missile ? 1 : 0;
  samus_missiles = weapon->missiles;
  samus_max_missiles = weapon->missile_capacity;
  samus_super_missiles = 0;
  samus_max_super_missiles = 0;
  samus_auto_cancel_hud_item_index = 0;
  projectile_counter =
      MiniMultiplayerWeapons_OwnedProjectileCount(state, player_index);
}

static void MiniMultiplayerWeapons_CapturePlayer(int player_index,
                                                  void *context) {
  MiniGameState *state = (MiniGameState *)context;
  if (state == NULL || player_index < 0 || player_index >= state->player_count)
    return;

  MiniPlayerWeaponState *weapon = &state->players[player_index].weapon;
  weapon->selected = hud_item_index == 1
                         ? kMiniWeapon_Missile
                         : kMiniWeapon_PowerBeam;
  weapon->missiles = samus_missiles;
}

void MiniMultiplayerWeapons_Initialize(MiniGameState *state) {
  if (state == NULL)
    return;
  for (int player = 0; player < kMiniMaxPlayers; player++) {
    state->players[player].weapon = (MiniPlayerWeaponState){
      .selected = kMiniWeapon_PowerBeam,
      .missiles = kMiniMultiplayerStartingMissiles,
      .missile_capacity = kMiniMultiplayerStartingMissiles,
    };
  }
}

void MiniMultiplayerWeapons_BeginFrame(MiniGameState *state) {
  if (state == NULL || state->player_count < 2) {
    MultiSamus_SetPlayerInputHook(NULL, NULL);
    MultiSamus_SetPlayerInputEndHook(NULL, NULL);
    return;
  }
  MultiSamus_SetPlayerInputHook(MiniMultiplayerWeapons_ApplyPlayer, state);
  MultiSamus_SetPlayerInputEndHook(MiniMultiplayerWeapons_CapturePlayer, state);
}

void MiniMultiplayerWeapons_EndFrame(void) {
  MultiSamus_SetPlayerInputHook(NULL, NULL);
  MultiSamus_SetPlayerInputEndHook(NULL, NULL);
}
