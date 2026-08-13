#include "mini_multiplayer_combat.h"

#include <string.h>

#include "funcs.h"
#include "multi_samus.h"
#include "samus_projectile.h"
#include "samus_projectile_view.h"
#include "variables.h"

static void MiniRefreshProjectileState(MiniGameState *state) {
  state->projectile_state.count = SamusProjectile_GetActiveViews(
      state->projectile_state.views, kMiniProjectileViewCapacity);
}

static bool MiniProjectileSlotActive(int slot) {
  if ((unsigned)slot >= kSamusProjectileSlotCount)
    return false;
  return projectile_type[slot] != 0 ||
         projectile_damage[slot] != 0 ||
         projectile_bomb_instruction_ptr[slot] != 0;
}

static void MiniPruneInactiveProjectileOwners(MiniGameState *state) {
  for (int slot = 0; slot < kSamusProjectileSlotCount; slot++) {
    if (!MiniProjectileSlotActive(slot))
      state->projectile_state.owner_by_slot[slot] = 0;
  }
}

static bool MiniRectsOverlap(int ax, int ay, int arx, int ary,
                             int bx, int by, int brx, int bry) {
  int dx = ax >= bx ? ax - bx : bx - ax;
  int dy = ay >= by ? ay - by : by - ay;
  return dx <= arx + brx && dy <= ary + bry;
}

static void MiniResolveProjectileHit(MiniGameState *state,
                                     const SamusProjectileView *projectile) {
  uint16 slot_index = projectile->slot_index;
  if (slot_index >= kSamusProjectileSlotCount)
    return;
  uint16 projectile_kind = projectile->type & kProjectileType_TypeMask;
  if (projectile_kind == kProjectileType_Missile ||
      projectile_kind == kProjectileType_SuperMissile) {
    KillProjectile((uint16)(slot_index * 2));
  } else {
    ClearProjectile((uint16)(slot_index * 2));
    state->projectile_state.owner_by_slot[slot_index] = 0;
  }
}

static void MiniTickCombatTimers(MiniGameState *state) {
  for (int player = 0; player < state->player_count; player++) {
    MiniPlayerCombatState *combat = &state->players[player].combat;
    if (combat->hitstun_frames != 0)
      combat->hitstun_frames--;
    if (combat->invulnerable_frames != 0)
      combat->invulnerable_frames--;
  }
}

static void MiniAppendHitEvent(MiniGameState *state, uint8 attacker_player,
                               uint8 defender_player, uint16 projectile_slot,
                               uint16 damage) {
  if (state->melee_hit_event_count >= kMiniMeleeHitEventCapacity) {
    state->melee_hit_event_dropped_count++;
    return;
  }
  state->melee_hit_events[state->melee_hit_event_count++] = (MiniMeleeHitEvent){
    .attacker_player = attacker_player,
    .defender_player = defender_player,
    .projectile_slot = projectile_slot,
    .damage = damage,
  };
}

static void MiniHandleProjectileHits(MiniGameState *state) {
  int projectile_count = state->projectile_state.count;
  if (projectile_count > kMiniProjectileViewCapacity)
    projectile_count = kMiniProjectileViewCapacity;
  for (int slot = 0; slot < kSamusProjectileSlotCount; slot++) {
    const SamusProjectileView *projectile = NULL;
    for (int i = 0; i < projectile_count; i++) {
      if (state->projectile_state.views[i].slot_index == slot) {
        projectile = &state->projectile_state.views[i];
        break;
      }
    }
    if (projectile == NULL)
      continue;
    uint16 projectile_kind = projectile->type & kProjectileType_TypeMask;
    if (projectile_kind != 0 &&
        projectile_kind != kProjectileType_Missile &&
        projectile_kind != kProjectileType_SuperMissile) {
      continue;
    }
    uint8 owner_plus_one = state->projectile_state.owner_by_slot[projectile->slot_index];
    if (owner_plus_one == 0)
      continue;
    int owner = owner_plus_one - 1;
    if (owner < 0 || owner >= state->player_count)
      continue;

    int projectile_rx = projectile->x_radius != 0 ? projectile->x_radius : 4;
    int projectile_ry = projectile->y_radius != 0 ? projectile->y_radius : 4;
    for (int player = 0; player < state->player_count; player++) {
      if (player == owner)
        continue;
      MiniPlayerCombatState *combat = &state->players[player].combat;
      if (combat->invulnerable_frames != 0)
        continue;
      const MiniSamusCoreState *samus = &state->players[player].samus;
      if (!MiniRectsOverlap(projectile->x_pos, projectile->y_pos,
                            projectile_rx, projectile_ry,
                            samus->world_x, samus->world_y,
                            samus->x_radius, samus->y_radius)) {
        continue;
      }

      combat->hit_count++;
      combat->pending_damage = projectile->damage != 0 ? projectile->damage : 20;
      combat->hitstun_frames = 6;
      combat->invulnerable_frames = 10;
      combat->last_hit_by_player = owner_plus_one;
      MiniAppendHitEvent(state, owner_plus_one, (uint8)(player + 1),
                         projectile->slot_index, combat->pending_damage);
      MiniResolveProjectileHit(state, projectile);
      break;
    }
  }
}

static void MiniCaptureProjectileSpawn(int player_index, uint16 projectile_slot,
                                       void *context) {
  MiniGameState *state = (MiniGameState *)context;
  int slot_index = projectile_slot >> 1;
  if (state == NULL || player_index < 0 || player_index >= state->player_count ||
      slot_index < 0 || slot_index >= kSamusProjectileSlotCount)
    return;
  uint8 *owner = &state->projectile_state.owner_by_slot[slot_index];
  if (*owner == 0)
    *owner = (uint8)(player_index + 1);
}

void MiniMultiplayerCombat_BeginFrame(MiniGameState *state) {
  if (state == NULL || state->player_count < 2) {
    MultiSamus_SetProjectileSpawnHook(NULL, NULL);
    return;
  }
  MiniPruneInactiveProjectileOwners(state);
  MultiSamus_SetProjectileSpawnHook(MiniCaptureProjectileSpawn, state);
}

void MiniMultiplayerCombat_Update(MiniGameState *state) {
  MultiSamus_SetProjectileSpawnHook(NULL, NULL);
  state->melee_hit_event_count = 0;
  memset(state->melee_hit_events, 0, sizeof(state->melee_hit_events));
  if (state->player_count < 2)
    return;
  MiniPruneInactiveProjectileOwners(state);
  MiniTickCombatTimers(state);
  MiniHandleProjectileHits(state);
  MiniRefreshProjectileState(state);
  MiniPruneInactiveProjectileOwners(state);
}
