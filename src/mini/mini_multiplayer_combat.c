#include "mini_multiplayer_combat.h"

#include <stdlib.h>

#include "ida_types.h"
#include "mini_game.h"
#include "samus_projectile.h"
#include "variables.h"

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

static int MiniProjectileOwnerScore(const SamusProjectileView *projectile,
                                    const MiniSamusCoreState *samus) {
  int projectile_x = projectile->x_pos;
  int projectile_y = projectile->y_pos;
  int score = abs(projectile_x - samus->world_x) + 2 * abs(projectile_y - samus->world_y);
  int direction = projectile->direction & 0xF;
  bool moving_right = direction == 1 || direction == 2 || direction == 3;
  bool moving_left = direction == 6 || direction == 7 || direction == 8;
  if (moving_right && samus->world_x > projectile_x)
    score += 10000;
  if (moving_left && samus->world_x < projectile_x)
    score += 10000;
  return score;
}

static void MiniAssignUnownedProjectiles(MiniGameState *state) {
  for (int i = 0; i < state->projectile_state.count; i++) {
    const SamusProjectileView *projectile = &state->projectile_state.views[i];
    if (projectile->slot_index >= kSamusProjectileSlotCount)
      continue;
    if (state->projectile_state.owner_by_slot[projectile->slot_index] != 0)
      continue;

    int best_player = -1;
    int best_score = 0;
    for (int player = 0; player < state->player_count; player++) {
      int score = MiniProjectileOwnerScore(projectile, &state->players[player].samus);
      if (best_player < 0 || score < best_score) {
        best_player = player;
        best_score = score;
      }
    }
    if (best_player >= 0)
      state->projectile_state.owner_by_slot[projectile->slot_index] = (uint8)(best_player + 1);
  }
}

static bool MiniRectsOverlap(int ax, int ay, int arx, int ary,
                             int bx, int by, int brx, int bry) {
  return abs(ax - bx) <= arx + brx && abs(ay - by) <= ary + bry;
}

static void MiniClearProjectileSlot(MiniGameState *state, uint16 slot_index) {
  if (slot_index >= kSamusProjectileSlotCount)
    return;
  ClearProjectile((uint16)(slot_index * 2));
  state->projectile_state.owner_by_slot[slot_index] = 0;
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

static void MiniHandleProjectileHits(MiniGameState *state) {
  for (int i = 0; i < state->projectile_state.count; i++) {
    const SamusProjectileView *projectile = &state->projectile_state.views[i];
    if (projectile->slot_index >= kSamusProjectileSlotCount)
      continue;
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
      MiniClearProjectileSlot(state, projectile->slot_index);
      break;
    }
  }
}

void MiniUpdateMultiplayerCombat(MiniGameState *state) {
  if (state->player_count < 2)
    return;
  MiniPruneInactiveProjectileOwners(state);
  MiniAssignUnownedProjectiles(state);
  MiniTickCombatTimers(state);
  MiniHandleProjectileHits(state);
  MiniRefreshProjectileState(state);
  MiniPruneInactiveProjectileOwners(state);
  MiniSyncPublicViews(state);
}
