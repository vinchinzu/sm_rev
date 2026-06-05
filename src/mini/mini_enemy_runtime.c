#include "mini_enemy_runtime.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "funcs.h"
#include "ida_types.h"
#include "mini_asset_bootstrap.h"
#include "mini_climb_endless.h"
#include "mini_enemy_metadata.h"
#include "mini_run_mode.h"
#include "samus_projectile.h"
#include "samus_projectile_view.h"
#include "variables.h"

static int MiniClampInt(int value, int min_value, int max_value) {
  if (value < min_value)
    return min_value;
  if (value > max_value)
    return max_value;
  return value;
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

MiniEnemyPolicy MiniEnemyRuntime_PolicyForState(const MiniGameState *state) {
  if (state == NULL)
    return kMiniEnemyPolicy_None;
  if (state->room.uses_original_gameplay_runtime)
    return kMiniEnemyPolicy_RomOwned;
  return kMiniEnemyPolicy_MiniSim;
}

static MiniEnemyRuntimeState *MiniReserveEnemy(MiniGameState *state) {
  if (state->enemy_state.count >= kMiniEnemyCapacity)
    return NULL;
  return &state->enemy_state.enemies[state->enemy_state.count++];
}

static void MiniApplyPopulationWords(MiniEnemyRuntimeState *enemy,
                                     const MiniEditorEnemySpawnView *spawn) {
  if (spawn->has_population_words) {
    enemy->init_parameter = spawn->init_parameter;
    enemy->properties1 = spawn->properties1;
    enemy->properties2 = spawn->properties2;
    enemy->extra_parameter1 = spawn->extra_parameter1;
    enemy->extra_parameter2 = spawn->extra_parameter2;
    return;
  }

  MiniKnownEnemyPopulation population;
  if (MiniKnownClimbPopulationForSpawn(spawn, &population)) {
    enemy->init_parameter = population.init_parameter;
    enemy->properties1 = population.properties1;
    enemy->properties2 = population.properties2;
    enemy->extra_parameter1 = population.extra_parameter1;
    enemy->extra_parameter2 = population.extra_parameter2;
  }
}

static void MiniComputeSpriteViewOriginDelta(const MiniEditorRoomSpriteView *sprite,
                                             int16 *origin_dx, int16 *origin_dy) {
  int min_x = 0;
  int max_x = 0;
  int min_y = 0;
  int max_y = 0;
  if (sprite->entry_count > 0) {
    min_x = max_x = sprite->entries[0].x_offset;
    min_y = max_y = sprite->entries[0].y_offset;
    for (int entry_index = 0; entry_index < sprite->entry_count; entry_index++) {
      const MiniEditorRoomSpriteOamView *entry = &sprite->entries[entry_index];
      int size = entry->is_16x16 ? 16 : 8;
      if (entry->x_offset < min_x)
        min_x = entry->x_offset;
      if (entry->y_offset < min_y)
        min_y = entry->y_offset;
      if (entry->x_offset + size > max_x)
        max_x = entry->x_offset + size;
      if (entry->y_offset + size > max_y)
        max_y = entry->y_offset + size;
    }
  }
  *origin_dx = (int16)((min_x + max_x) / 2);
  *origin_dy = (int16)((min_y + max_y) / 2);
}

static void MiniEnemyBindSpriteView(MiniEnemyRuntimeState *enemy) {
  const MiniEditorRoomSpriteView *sprites = NULL;
  int sprite_view_count = MiniAssetBootstrap_GetEditorRoomSpriteViews(&sprites);
  enemy->sprite_view_index = kMiniEnemyNoSpriteView;
  enemy->sprite_origin_dx = 0;
  enemy->sprite_origin_dy = 0;
  if (sprite_view_count <= 0 || sprites == NULL)
    return;

  int best_index = kMiniEnemyNoSpriteView;
  int best_distance = 0x7FFFFFFF;
  for (int i = 0; i < sprite_view_count; i++) {
    const MiniEditorRoomSpriteView *sprite = &sprites[i];
    if (sprite->species_id != enemy->species_id || sprite->tile_data == NULL)
      continue;
    int dx = sprite->x_pos - enemy->home_x;
    int dy = sprite->y_pos - enemy->home_y;
    int distance = dx * dx + dy * dy;
    if (distance < best_distance) {
      best_distance = distance;
      best_index = i;
    }
  }
  if (best_index < 0)
    return;
  const MiniEditorRoomSpriteView *bound = &sprites[best_index];
  MiniComputeSpriteViewOriginDelta(bound, &enemy->sprite_origin_dx, &enemy->sprite_origin_dy);
  enemy->sprite_view_index = (int16)best_index;
  if (best_distance <= 64 * 64) {
    enemy->x = bound->x_pos + enemy->sprite_origin_dx;
    enemy->y = bound->y_pos + enemy->sprite_origin_dy;
  }
}

static MiniEnemyRuntimeState MiniEnemy_FromExportSpawn(
    const MiniEditorEnemySpawnView *spawn, int index) {
  const MiniEnemySpeciesMetadata *metadata =
      MiniEnemyMetadataForSpecies(spawn->species_id);
  int max_health = metadata != NULL ? metadata->max_health : 20;
  MiniEnemyRuntimeState enemy = {
    .active = true,
    .species_id = spawn->species_id,
    .x = spawn->x_pos,
    .y = spawn->y_pos,
    .home_x = spawn->x_pos,
    .home_y = spawn->y_pos,
    .x_radius = metadata != NULL ? metadata->x_radius : 12,
    .y_radius = metadata != NULL ? metadata->y_radius : 12,
    .health = max_health,
    .max_health = max_health,
    .damage = metadata != NULL ? metadata->damage : 0,
    .ai_bank = metadata != NULL ? metadata->ai_bank : 0,
    .init_ai = metadata != NULL ? metadata->init_ai : 0,
    .main_ai = metadata != NULL ? metadata->main_ai : 0,
    .sprite_view_index = kMiniEnemyNoSpriteView,
    .behavior = metadata != NULL ? metadata->behavior : kMiniEnemyBehavior_Passive,
  };
  MiniApplyPopulationWords(&enemy, spawn);
  snprintf(enemy.source_label, sizeof(enemy.source_label), "%s", spawn->name);
  snprintf(enemy.name, sizeof(enemy.name), "%s",
           metadata != NULL ? metadata->canonical_name : spawn->name);
  if (enemy.behavior == kMiniEnemyBehavior_SpacePirateShooter)
    enemy.shoot_cooldown = 24 + index * 9;
  return enemy;
}

static bool MiniRomEnemySlotVisible(const EnemyData *src) {
  return src->enemy_ptr != 0 &&
         src->enemy_ptr != addr_kEnemyDef_DAFF &&
         (src->properties & kEnemyProps_Deleted) == 0;
}

static MiniEnemyRuntimeState MiniEnemy_FromRomSlot(const EnemyData *src, int index) {
  const EnemyDef *def = get_EnemyDef_A2(src->enemy_ptr);
  const MiniEnemySpeciesMetadata *metadata = MiniEnemyMetadataForSpecies(src->enemy_ptr);
  MiniEnemyRuntimeState enemy = {
    .active = src->health != 0,
    .species_id = src->enemy_ptr,
    .init_parameter = src->current_instruction,
    .properties1 = src->properties,
    .properties2 = src->extra_properties,
    .extra_parameter1 = src->parameter_1,
    .extra_parameter2 = src->parameter_2,
    .ai_bank = def != NULL ? def->bank : src->bank,
    .init_ai = def != NULL ? def->ai_init : 0,
    .main_ai = def != NULL ? def->main_ai : 0,
    .x = src->x_pos,
    .y = src->y_pos,
    .home_x = src->x_pos,
    .home_y = src->y_pos,
    .x_radius = src->x_width,
    .y_radius = src->y_height,
    .health = src->health,
    .max_health = def != NULL ? def->health : src->health,
    .damage = def != NULL ? def->damage : 0,
    .invulnerable_frames = src->invincibility_timer,
    .ai_state = src->ai_handler_bits,
    .state_timer = src->timer,
    .sprite_view_index = kMiniEnemyNoSpriteView,
    .behavior = metadata != NULL ? metadata->behavior : kMiniEnemyBehavior_Passive,
  };
  snprintf(enemy.source_label, sizeof(enemy.source_label), "rom_enemy_%02d", index);
  snprintf(enemy.name, sizeof(enemy.name), "%s",
           metadata != NULL ? metadata->canonical_name : "Original Enemy");
  return enemy;
}

static void MiniInitializeOriginalExportEnemies(MiniGameState *state) {
  const MiniEditorEnemySpawnView *spawns = NULL;
  int spawn_count = MiniAssetBootstrap_GetEditorEnemySpawnViews(&spawns);
  if (spawn_count <= 0)
    return;
  for (int i = 0; i < spawn_count; i++) {
    MiniEnemyRuntimeState *enemy = MiniReserveEnemy(state);
    if (enemy == NULL)
      return;
    *enemy = MiniEnemy_FromExportSpawn(&spawns[i], i);
    MiniEnemyBindSpriteView(enemy);
  }
}

void MiniEnemyRuntime_RefreshCounts(MiniEnemyState *enemy_state) {
  int active_count = 0;
  int passive_count = 0;
  int renderable_count = 0;
  int shot_count = 0;
  for (int i = 0; i < enemy_state->count; i++) {
    const MiniEnemyRuntimeState *enemy = &enemy_state->enemies[i];
    if (enemy->active)
      active_count++;
    if (enemy->active && enemy->behavior == kMiniEnemyBehavior_Passive)
      passive_count++;
    if (MiniEnemy_IsRuntimeRenderable(enemy))
      renderable_count++;
  }
  for (int i = 0; i < kMiniEnemyShotCapacity; i++) {
    if (enemy_state->shots[i].active)
      shot_count++;
  }
  enemy_state->active_count = active_count;
  enemy_state->passive_count = passive_count;
  enemy_state->renderable_count = renderable_count;
  enemy_state->shot_count = shot_count;
}

void MiniEnemyRuntime_Initialize(MiniGameState *state) {
  memset(&state->enemy_state, 0, sizeof(state->enemy_state));
  switch (MiniEnemyRuntime_PolicyForState(state)) {
  case kMiniEnemyPolicy_RomOwned:
  case kMiniEnemyPolicy_None:
    return;
  case kMiniEnemyPolicy_MiniSim:
    MiniInitializeOriginalExportEnemies(state);
    MiniEnemyRuntime_RefreshCounts(&state->enemy_state);
    return;
  }
}

static bool MiniEditorClimbFallbackUsesRecycledEnemyBand(const MiniGameState *state) {
  return !state->room.uses_original_gameplay_runtime &&
         MiniRunMode_IsClimbEndless() &&
         state->room.room_id == kMiniClimbEndlessRoomId &&
         strcmp(state->room.room_handle, "climb") == 0;
}

static void MiniRecycleEditorClimbFallbackRoachIntoCameraBand(
    MiniGameState *state, int enemy_index, MiniEnemyRuntimeState *enemy) {
  enum {
    kClimbEnemyTopMargin = 24,
    kClimbEnemyBottomMargin = 24,
    kClimbEnemyPatternStride = 49,
  };

  MiniKnownEnemyPopulation population;
  MiniEditorEnemySpawnView spawn = {
    .species_id = enemy->species_id,
    .x_pos = enemy->home_x,
    .y_pos = enemy->home_y,
  };
  if (!MiniEditorClimbFallbackUsesRecycledEnemyBand(state) ||
      enemy->behavior != kMiniEnemyBehavior_Roach ||
      !MiniKnownClimbPopulationForSpawn(&spawn, &population)) {
    return;
  }

  int camera_y = state->viewport.camera_y;
  int min_y = camera_y + kClimbEnemyTopMargin;
  int max_y = camera_y + state->viewport.height - kClimbEnemyBottomMargin;
  if (enemy->y >= min_y && enemy->y <= max_y)
    return;

  int band_height = max_y - min_y + 1;
  if (band_height <= 0)
    band_height = state->viewport.height;
  if (band_height <= 0)
    band_height = 1;
  int pattern_y = (population.y + enemy_index * kClimbEnemyPatternStride) % band_height;
  enemy->x = population.x;
  enemy->y = min_y + pattern_y;
  enemy->home_x = enemy->x;
  enemy->home_y = enemy->y;
  enemy->x_velocity = 0;
  enemy->y_velocity = 0;
  enemy->ai_state = 0;
  enemy->state_timer = 0;
  enemy->facing_right = state->players[0].samus.world_x > enemy->x;
}

static void MiniRecycleEditorClimbFallbackEnemiesIntoCameraBand(MiniGameState *state) {
  if (!MiniEditorClimbFallbackUsesRecycledEnemyBand(state))
    return;
  for (int i = 0; i < state->enemy_state.count; i++) {
    MiniEnemyRuntimeState *enemy = &state->enemy_state.enemies[i];
    if (enemy->active)
      MiniRecycleEditorClimbFallbackRoachIntoCameraBand(state, i, enemy);
  }
}

static void MiniSpawnPirateShot(MiniGameState *state, const MiniEnemyRuntimeState *pirate) {
  for (int i = 0; i < kMiniEnemyShotCapacity; i++) {
    MiniEnemyShotState *shot = &state->enemy_state.shots[i];
    if (shot->active)
      continue;
    int dir = pirate->facing_right ? 1 : -1;
    *shot = (MiniEnemyShotState){
      .active = true,
      .x = pirate->x + dir * (pirate->x_radius + 6),
      .y = pirate->y - pirate->y_radius / 3,
      .x_velocity = dir * 4,
      .y_velocity = 0,
      .radius = 5,
      .damage = 20,
    };
    return;
  }
}

static void MiniDamagePlayer(MiniGameState *state, int player, int damage) {
  if (player < 0 || player >= state->player_count)
    return;
  MiniPlayerCombatState *combat = &state->players[player].combat;
  if (combat->invulnerable_frames != 0)
    return;
  combat->hit_count++;
  combat->pending_damage = (uint16)damage;
  combat->hitstun_frames = 8;
  combat->invulnerable_frames = 24;
  combat->last_hit_by_player = 0;
}

static void MiniUpdatePirateShots(MiniGameState *state) {
  for (int i = 0; i < kMiniEnemyShotCapacity; i++) {
    MiniEnemyShotState *shot = &state->enemy_state.shots[i];
    if (!shot->active)
      continue;
    shot->x += shot->x_velocity;
    shot->y += shot->y_velocity;
    if (shot->x < state->room.room_left - 32 || shot->x > state->room.room_right + 32 ||
        shot->y < state->room.room_top - 32 || shot->y > state->room.room_bottom + 32) {
      shot->active = false;
      continue;
    }
    for (int player = 0; player < state->player_count; player++) {
      const MiniSamusCoreState *samus = &state->players[player].samus;
      if (!MiniRectsOverlap(shot->x, shot->y, shot->radius, shot->radius,
                            samus->world_x, samus->world_y,
                            samus->x_radius, samus->y_radius)) {
        continue;
      }
      MiniDamagePlayer(state, player, shot->damage);
      shot->active = false;
      break;
    }
  }
}

static void MiniHandleSamusProjectilesVsEnemies(MiniGameState *state) {
  for (int projectile_view_index = 0; projectile_view_index < state->projectile_state.count; projectile_view_index++) {
    const SamusProjectileView *projectile = &state->projectile_state.views[projectile_view_index];
    if (projectile->slot_index >= kSamusProjectileSlotCount)
      continue;
    int projectile_rx = projectile->x_radius != 0 ? projectile->x_radius : 4;
    int projectile_ry = projectile->y_radius != 0 ? projectile->y_radius : 4;
    for (int enemy_index = 0; enemy_index < state->enemy_state.count; enemy_index++) {
      MiniEnemyRuntimeState *enemy = &state->enemy_state.enemies[enemy_index];
      if (!enemy->active || !MiniEnemyTakesProjectileDamage(enemy) ||
          enemy->invulnerable_frames != 0)
        continue;
      if (!MiniRectsOverlap(projectile->x_pos, projectile->y_pos, projectile_rx, projectile_ry,
                            enemy->x, enemy->y, enemy->x_radius, enemy->y_radius)) {
        continue;
      }
      int damage = projectile->damage != 0 ? projectile->damage : 20;
      enemy->health -= damage;
      enemy->hit_count++;
      enemy->invulnerable_frames = 4;
      MiniClearProjectileSlot(state, projectile->slot_index);
      if (enemy->health <= 0) {
        enemy->active = false;
        enemy->health = 0;
        state->enemy_state.defeated_count++;
      }
      break;
    }
  }
}

static void MiniHandleEnemyTouchDamage(MiniGameState *state) {
  for (int enemy_index = 0; enemy_index < state->enemy_state.count; enemy_index++) {
    const MiniEnemyRuntimeState *enemy = &state->enemy_state.enemies[enemy_index];
    if (!enemy->active || !MiniEnemyDoesTouchDamage(enemy))
      continue;
    for (int player = 0; player < state->player_count; player++) {
      const MiniSamusCoreState *samus = &state->players[player].samus;
      if (MiniRectsOverlap(enemy->x, enemy->y, enemy->x_radius, enemy->y_radius,
                           samus->world_x, samus->world_y, samus->x_radius, samus->y_radius)) {
        MiniDamagePlayer(state, player, enemy->damage);
      }
    }
  }
}

static void MiniRoachVelocityFromPopulation(const MiniEnemyRuntimeState *enemy,
                                            int *x_velocity, int *y_velocity) {
  static const int kOctantX[8] = {1, 1, 0, -1, -1, -1, 0, 1};
  static const int kOctantY[8] = {0, -1, -1, -1, 0, 1, 1, 1};
  uint8 angle = (uint8)(enemy->extra_parameter1 >> 8);
  int speed = enemy->extra_parameter1 & 0xFF;
  if (speed <= 0)
    speed = 2;
  speed = MiniClampInt(speed, 1, 4);
  int octant = ((int)angle + 16) >> 5;
  octant &= 7;
  *x_velocity = kOctantX[octant] * speed;
  *y_velocity = kOctantY[octant] * speed;
}

static void MiniUpdateRoach(MiniGameState *state, MiniEnemyRuntimeState *enemy) {
  int trigger_radius = enemy->extra_parameter2 & 0xFF;
  if (trigger_radius <= 0)
    trigger_radius = 80;
  const MiniSamusCoreState *samus = &state->players[0].samus;
  if (enemy->ai_state == 0) {
    if (abs(enemy->x - samus->world_x) > trigger_radius ||
        abs(enemy->y - samus->world_y) > trigger_radius) {
      return;
    }
    enemy->ai_state = 1;
    MiniRoachVelocityFromPopulation(enemy, &enemy->x_velocity, &enemy->y_velocity);
  }

  enemy->state_timer++;
  if ((enemy->state_timer & 31) == 0) {
    MiniRoachVelocityFromPopulation(enemy, &enemy->x_velocity, &enemy->y_velocity);
    if (((enemy->state_timer >> 5) & 1) != 0) {
      enemy->x_velocity = -enemy->x_velocity;
    }
  }

  enemy->x += enemy->x_velocity;
  enemy->y += enemy->y_velocity;
  if (enemy->x < state->room.room_left + enemy->x_radius ||
      enemy->x > state->room.room_right - enemy->x_radius) {
    enemy->x = MiniClampInt(enemy->x,
                            state->room.room_left + enemy->x_radius,
                            state->room.room_right - enemy->x_radius);
    enemy->x_velocity = -enemy->x_velocity;
  }
  if (enemy->y < state->room.room_top + enemy->y_radius ||
      enemy->y > state->room.room_bottom - enemy->y_radius) {
    enemy->y = MiniClampInt(enemy->y,
                            state->room.room_top + enemy->y_radius,
                            state->room.room_bottom - enemy->y_radius);
    enemy->y_velocity = -enemy->y_velocity;
  }
}

void MiniEnemyRuntime_Update(MiniGameState *state) {
  switch (MiniEnemyRuntime_PolicyForState(state)) {
  case kMiniEnemyPolicy_RomOwned:
  case kMiniEnemyPolicy_None:
    return;
  case kMiniEnemyPolicy_MiniSim:
    break;
  }

  if (state->enemy_state.count == 0)
    return;
  for (int i = 0; i < state->enemy_state.count; i++) {
    MiniEnemyRuntimeState *enemy = &state->enemy_state.enemies[i];
    if (!enemy->active)
      continue;
    if (enemy->invulnerable_frames != 0)
      enemy->invulnerable_frames--;
    if (enemy->behavior == kMiniEnemyBehavior_Roach) {
      MiniUpdateRoach(state, enemy);
      continue;
    }
    if (enemy->behavior != kMiniEnemyBehavior_SpacePirateShooter)
      continue;
    enemy->facing_right = state->players[0].samus.world_x > enemy->x;
    if (enemy->shoot_cooldown > 0) {
      enemy->shoot_cooldown--;
    } else {
      MiniSpawnPirateShot(state, enemy);
      enemy->shoot_cooldown = 64;
    }
  }
  MiniHandleSamusProjectilesVsEnemies(state);
  MiniHandleEnemyTouchDamage(state);
  MiniUpdatePirateShots(state);
  MiniRecycleEditorClimbFallbackEnemiesIntoCameraBand(state);
  MiniEnemyRuntime_RefreshCounts(&state->enemy_state);
}

static void MiniEnemyTelemetry_AddEnemy(MiniEnemyTelemetry *telemetry,
                                        const MiniEnemyRuntimeState *enemy) {
  telemetry->count++;
  if (enemy->active) {
    telemetry->active_count++;
    if (!telemetry->has_first_enemy) {
      telemetry->first_enemy = *enemy;
      telemetry->has_first_enemy = true;
    }
  }
  if (enemy->active && enemy->behavior == kMiniEnemyBehavior_Passive)
    telemetry->passive_count++;
  if (MiniEnemy_IsRuntimeRenderable(enemy))
    telemetry->renderable_count++;
  if (enemy->behavior == kMiniEnemyBehavior_SpacePirateShooter) {
    telemetry->pirate_count++;
    if (enemy->active) {
      telemetry->pirate_active_count++;
      if (!telemetry->has_first_pirate) {
        telemetry->first_pirate = *enemy;
        telemetry->has_first_pirate = true;
      }
    }
  }
}

static void MiniEnemyRuntime_BuildStateTelemetry(const MiniGameState *state,
                                                 MiniEnemyTelemetry *telemetry) {
  for (int i = 0; i < state->enemy_state.count; i++)
    MiniEnemyTelemetry_AddEnemy(telemetry, &state->enemy_state.enemies[i]);
  telemetry->shot_count = state->enemy_state.shot_count;
  telemetry->defeated_count = state->enemy_state.defeated_count;
}

static void MiniEnemyRuntime_BuildRomTelemetry(MiniEnemyTelemetry *telemetry) {
  int original_count = num_enemies_in_room;
  if (original_count > kMiniEnemyCapacity)
    original_count = kMiniEnemyCapacity;

  for (int i = 0; i < original_count; i++) {
    const EnemyData *src = &enemy_data[i];
    if (!MiniRomEnemySlotVisible(src))
      continue;
    MiniEnemyRuntimeState enemy = MiniEnemy_FromRomSlot(src, i);
    MiniEnemyTelemetry_AddEnemy(telemetry, &enemy);
  }
  telemetry->defeated_count = num_enemies_killed_in_room;
}

void MiniEnemyRuntime_BuildTelemetry(const MiniGameState *state,
                                     MiniEnemyTelemetry *telemetry) {
  memset(telemetry, 0, sizeof(*telemetry));
  if (state == NULL)
    return;
  switch (MiniEnemyRuntime_PolicyForState(state)) {
  case kMiniEnemyPolicy_RomOwned:
    MiniEnemyRuntime_BuildRomTelemetry(telemetry);
    return;
  case kMiniEnemyPolicy_MiniSim:
    MiniEnemyRuntime_BuildStateTelemetry(state, telemetry);
    return;
  case kMiniEnemyPolicy_None:
    return;
  }
}
