#ifndef SM_MINI_NET_BRIDGE_H_
#define SM_MINI_NET_BRIDGE_H_

#include <stdint.h>

#include "mini_defs.h"
#include "mini_game.h"

#if defined(__GNUC__) || defined(__clang__)
#define MINI_NET_EXPORT __attribute__((visibility("default")))
#else
#define MINI_NET_EXPORT
#endif

enum {
  kMiniNetMaxPlayers = kMiniMaxPlayers,
  kMiniNetMaxProjectiles = kMiniProjectileViewCapacity,
  kMiniNetMeleeHitEventCapacity = kMiniMeleeHitEventCapacity,
};

typedef struct MiniNetPlayerSnapshot {
  int32_t world_x;
  int32_t world_y;
  int32_t screen_x;
  int32_t screen_y;
  int32_t x_velocity;
  int32_t y_velocity;
  uint16_t x_radius;
  uint16_t y_radius;
  uint16_t pose;
  uint16_t movement_type;
  uint16_t buttons;
  uint16_t hit_count;
  uint16_t pending_damage;
  uint16_t hitstun_frames;
  uint16_t invulnerable_frames;
  uint16_t missiles;
  uint16_t missile_capacity;
  uint8_t selected_weapon;
  uint8_t suit;
  uint8_t on_ground;
  uint8_t last_hit_by_player;
  uint8_t active;
} MiniNetPlayerSnapshot;

typedef struct MiniNetProjectileSnapshot {
  int32_t screen_x;
  int32_t screen_y;
  uint16_t slot_index;
  uint16_t type;
  uint16_t direction;
  uint16_t x_pos;
  uint16_t y_pos;
  uint16_t x_radius;
  uint16_t y_radius;
  uint16_t damage;
  uint8_t active;
  uint8_t owner;
} MiniNetProjectileSnapshot;

typedef struct MiniNetMeleeHitEventSnapshot {
  uint8_t attacker;
  uint8_t defender;
  uint16_t projectile_slot;
  uint16_t damage;
} MiniNetMeleeHitEventSnapshot;

typedef struct MiniNetSnapshot {
  uint32_t frame;
  int32_t player_count;
  int32_t focus_player;
  int32_t viewport_width;
  int32_t viewport_height;
  int32_t camera_x;
  int32_t camera_y;
  int32_t room_left;
  int32_t room_top;
  int32_t room_right;
  int32_t room_bottom;
  int32_t room_width;
  int32_t room_height;
  int32_t projectile_count;
  uint64_t state_hash;
  uint8_t hit_event_count;
  uint8_t hit_event_reserved[3];
  uint32_t hit_event_dropped_count;
  MiniNetMeleeHitEventSnapshot hit_events[kMiniNetMeleeHitEventCapacity];
  MiniNetPlayerSnapshot players[kMiniNetMaxPlayers];
  MiniNetProjectileSnapshot projectiles[kMiniNetMaxProjectiles];
} MiniNetSnapshot;

MINI_NET_EXPORT MiniGameState *MiniNetCreate(int32_t player_count, int32_t viewport_width,
                                             int32_t viewport_height);
MINI_NET_EXPORT void MiniNetDestroy(MiniGameState *state);
MINI_NET_EXPORT void MiniNetSetPlayerCount(MiniGameState *state, int32_t player_count);
MINI_NET_EXPORT void MiniNetStep2(MiniGameState *state, uint16_t player1_buttons,
                                  uint16_t player2_buttons, uint8_t quit_requested);
MINI_NET_EXPORT uint8_t MiniNetReadSnapshot(const MiniGameState *state, int32_t focus_player,
                                            MiniNetSnapshot *snapshot);
MINI_NET_EXPORT uint8_t MiniNetRenderFrame(const MiniGameState *state, int32_t focus_player,
                                           uint32_t *pixels, int32_t pitch_pixels);
MINI_NET_EXPORT uint8_t MiniNetRenderFrameRgba(const MiniGameState *state, int32_t focus_player,
                                               uint8_t *rgba, int32_t pitch_bytes);

#endif  // SM_MINI_NET_BRIDGE_H_
