#include "mini_net_bridge.h"

#include <string.h>

#include "mini_renderer.h"

static int32_t MiniNetNormalizePlayerCount(int32_t player_count) {
  if (player_count < 1)
    return 1;
  if (player_count > kMiniMaxPlayers)
    return kMiniMaxPlayers;
  return player_count;
}

static int32_t MiniNetNormalizePlayerIndex(const MiniGameState *state, int32_t focus_player) {
  int32_t player_count = state != NULL ? MiniNetNormalizePlayerCount(state->player_count) : 1;
  if (focus_player < 1)
    return 0;
  if (focus_player > player_count)
    return player_count - 1;
  return focus_player - 1;
}

static int32_t MiniNetClampInt32(int32_t value, int32_t min_value, int32_t max_value) {
  if (value < min_value)
    return min_value;
  if (value > max_value)
    return max_value;
  return value;
}

static int32_t MiniNetRoomWidth(const MiniGameState *state) {
  int32_t width = state->room.room_right - state->room.room_left;
  if (width <= 0)
    width = state->room.room_width_blocks * kMiniBlockSize;
  return width;
}

static int32_t MiniNetRoomHeight(const MiniGameState *state) {
  int32_t height = state->room.room_bottom - state->room.room_top;
  if (height <= 0)
    height = state->room.room_height_blocks * kMiniBlockSize;
  return height;
}

static void MiniNetComputePlayerCamera(const MiniGameState *state, int32_t player_index,
                                       int32_t *camera_x, int32_t *camera_y) {
  if (player_index == 0) {
    *camera_x = state->viewport.camera_x;
    *camera_y = state->viewport.camera_y;
    return;
  }

  const MiniSamusCoreState *samus = &state->players[player_index].samus;
  int32_t target_x_percent = state->room.camera_target_x_percent != 0
                                 ? state->room.camera_target_x_percent
                                 : 50;
  int32_t target_y_percent = state->room.camera_target_y_percent != 0
                                 ? state->room.camera_target_y_percent
                                 : 50;
  int32_t room_width = MiniNetRoomWidth(state);
  int32_t room_height = MiniNetRoomHeight(state);
  int32_t room_right = state->room.room_left + room_width;
  int32_t room_bottom = state->room.room_top + room_height;
  int32_t min_x = state->room.room_left;
  int32_t min_y = state->room.room_top;
  int32_t max_x = room_right - state->viewport.width;
  int32_t max_y = room_bottom - state->viewport.height;

  if (max_x < min_x)
    max_x = min_x;
  if (max_y < min_y)
    max_y = min_y;

  int32_t desired_x = samus->world_x - state->viewport.width * target_x_percent / 100;
  int32_t desired_y = samus->world_y - state->viewport.height * target_y_percent / 100;
  *camera_x = MiniNetClampInt32(desired_x, min_x, max_x);
  *camera_y = MiniNetClampInt32(desired_y, min_y, max_y);
}

MiniGameState *MiniNetCreate(int32_t player_count, int32_t viewport_width,
                             int32_t viewport_height) {
  if (viewport_width <= 0)
    viewport_width = kMiniGameWidth;
  if (viewport_height <= 0)
    viewport_height = kMiniGameHeight;

  MiniGameState *state = MiniCreate(viewport_width, viewport_height);
  if (state == NULL)
    return NULL;
  MiniSetPlayerCount(state, MiniNetNormalizePlayerCount(player_count));
  return state;
}

void MiniNetDestroy(MiniGameState *state) {
  MiniDestroy(state);
}

void MiniNetSetPlayerCount(MiniGameState *state, int32_t player_count) {
  if (state == NULL)
    return;
  MiniSetPlayerCount(state, MiniNetNormalizePlayerCount(player_count));
}

void MiniNetStep2(MiniGameState *state, uint16_t player1_buttons,
                  uint16_t player2_buttons, uint8_t quit_requested) {
  if (state == NULL)
    return;
  uint16_t buttons[kMiniMaxPlayers] = { player1_buttons, player2_buttons };
  MiniStepPlayers(state, buttons, kMiniMaxPlayers, quit_requested != 0);
}

uint8_t MiniNetReadSnapshot(const MiniGameState *state, int32_t focus_player,
                            MiniNetSnapshot *snapshot) {
  if (state == NULL || snapshot == NULL)
    return 0;

  memset(snapshot, 0, sizeof(*snapshot));
  int32_t focus_index = MiniNetNormalizePlayerIndex(state, focus_player);
  int32_t camera_x = 0;
  int32_t camera_y = 0;
  MiniNetComputePlayerCamera(state, focus_index, &camera_x, &camera_y);

  snapshot->frame = (uint32_t)state->frame;
  snapshot->player_count = MiniNetNormalizePlayerCount(state->player_count);
  snapshot->focus_player = focus_index + 1;
  snapshot->viewport_width = state->viewport.width;
  snapshot->viewport_height = state->viewport.height;
  snapshot->camera_x = camera_x;
  snapshot->camera_y = camera_y;
  snapshot->room_left = state->room.room_left;
  snapshot->room_top = state->room.room_top;
  snapshot->room_right = state->room.room_left + MiniNetRoomWidth(state);
  snapshot->room_bottom = state->room.room_top + MiniNetRoomHeight(state);
  snapshot->room_width = MiniNetRoomWidth(state);
  snapshot->room_height = MiniNetRoomHeight(state);
  snapshot->projectile_count = state->projectile_state.count;
  snapshot->state_hash = MiniStateHash(state);
  snapshot->hit_event_count = state->melee_hit_event_count;
  snapshot->hit_event_dropped_count = state->melee_hit_event_dropped_count;
  for (int i = 0; i < state->melee_hit_event_count; i++) {
    const MiniMeleeHitEvent *event = &state->melee_hit_events[i];
    snapshot->hit_events[i] = (MiniNetMeleeHitEventSnapshot){
      .attacker = event->attacker_player,
      .defender = event->defender_player,
      .projectile_slot = event->projectile_slot,
      .damage = event->damage,
    };
  }

  for (int i = 0; i < kMiniMaxPlayers; i++) {
    const MiniPlayerState *player = &state->players[i];
    MiniNetPlayerSnapshot *dst = &snapshot->players[i];
    dst->world_x = player->samus.world_x;
    dst->world_y = player->samus.world_y;
    dst->screen_x = player->samus.world_x - camera_x - player->samus.x_radius;
    dst->screen_y = player->samus.world_y - camera_y - player->samus.y_radius;
    dst->x_velocity = player->samus.x_velocity;
    dst->y_velocity = player->samus.y_velocity;
    dst->x_radius = player->samus.x_radius;
    dst->y_radius = player->samus.y_radius;
    dst->pose = player->samus.pose;
    dst->movement_type = player->samus.movement_type;
    dst->buttons = state->player_inputs[i].buttons;
    dst->hit_count = player->combat.hit_count;
    dst->pending_damage = player->combat.pending_damage;
    dst->hitstun_frames = player->combat.hitstun_frames;
    dst->invulnerable_frames = player->combat.invulnerable_frames;
    dst->missiles = player->weapon.missiles;
    dst->missile_capacity = player->weapon.missile_capacity;
    dst->selected_weapon = (uint8_t)player->weapon.selected;
    dst->suit = (uint8_t)player->samus.suit;
    dst->on_ground = player->samus.on_ground ? 1 : 0;
    dst->last_hit_by_player = player->combat.last_hit_by_player;
    dst->active = i < snapshot->player_count ? 1 : 0;
  }

  int projectile_count = state->projectile_state.count;
  if (projectile_count > kMiniNetMaxProjectiles)
    projectile_count = kMiniNetMaxProjectiles;
  snapshot->projectile_count = projectile_count;
  for (int i = 0; i < projectile_count; i++) {
    const SamusProjectileView *projectile = &state->projectile_state.views[i];
    MiniNetProjectileSnapshot *dst = &snapshot->projectiles[i];
    dst->screen_x = projectile->x_pos - camera_x - projectile->x_radius;
    dst->screen_y = projectile->y_pos - camera_y - projectile->y_radius;
    dst->slot_index = projectile->slot_index;
    dst->type = projectile->type;
    dst->direction = projectile->direction;
    dst->x_pos = projectile->x_pos;
    dst->y_pos = projectile->y_pos;
    dst->x_radius = projectile->x_radius;
    dst->y_radius = projectile->y_radius;
    dst->damage = projectile->damage;
    dst->active = projectile->active ? 1 : 0;
    if (projectile->slot_index < kSamusProjectileSlotCount)
      dst->owner = state->projectile_state.owner_by_slot[projectile->slot_index];
  }

  return 1;
}

uint8_t MiniNetRenderFrame(const MiniGameState *state, int32_t focus_player,
                           uint32_t *pixels, int32_t pitch_pixels) {
  if (state == NULL || pixels == NULL || pitch_pixels < state->viewport.width)
    return 0;

  int32_t focus_index = MiniNetNormalizePlayerIndex(state, focus_player);
  int32_t camera_x = 0;
  int32_t camera_y = 0;
  MiniNetComputePlayerCamera(state, focus_index, &camera_x, &camera_y);

  if (camera_x == state->viewport.camera_x && camera_y == state->viewport.camera_y) {
    MiniRenderFrameToPixels(pixels, pitch_pixels, state);
  } else {
    MiniRenderFrameToPixelsWithCamera(pixels, pitch_pixels, state, camera_x, camera_y);
  }
  return 1;
}

uint8_t MiniNetRenderFrameRgba(const MiniGameState *state, int32_t focus_player,
                               uint8_t *rgba, int32_t pitch_bytes) {
  if (state == NULL || rgba == NULL || pitch_bytes < kMiniGameWidth * 4)
    return 0;

  uint32_t pixels[kMiniGameWidth * kMiniGameHeight];
  if (!MiniNetRenderFrame(state, focus_player, pixels, kMiniGameWidth))
    return 0;

  for (int y = 0; y < kMiniGameHeight; y++) {
    uint8_t *dst = rgba + y * pitch_bytes;
    const uint32_t *src = pixels + y * kMiniGameWidth;
    for (int x = 0; x < kMiniGameWidth; x++) {
      uint32_t argb = src[x];
      dst[x * 4] = (uint8_t)(argb >> 16);
      dst[x * 4 + 1] = (uint8_t)(argb >> 8);
      dst[x * 4 + 2] = (uint8_t)argb;
      dst[x * 4 + 3] = (uint8_t)(argb >> 24);
    }
  }
  return 1;
}
