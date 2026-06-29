#ifndef SM_MINI_GAME_H_
#define SM_MINI_GAME_H_

#include "mini_room_adapter.h"
#include "mini_defs.h"
#include "mini_enemy.h"
#include "samus_projectile_view.h"
#include "types.h"
#include <stddef.h>
#include <stdint.h>

enum {
  kMiniSamusWidth = 24,
  kMiniSamusHeight = 40,
  kMiniGroundSpeed = 3,
  kMiniProjectileViewCapacity = kSamusBeamProjectileSlotCount,
  kMiniPlayerRuntimePreProjectileSize = 0xB64 - 0xA94,
  kMiniPlayerRuntimePostProjectileSize = 0xDC4 - 0xCCC,
};

typedef struct MiniInputState {
  uint16 buttons;
  uint16 player_buttons[kMiniMaxPlayers];
  int player_count;
  bool quit_requested;
} MiniInputState;

typedef struct MiniViewportState {
  int width;
  int height;
  int camera_x;
  int camera_y;
} MiniViewportState;

typedef struct MiniControlState {
  uint16 buttons;
  uint16 previous_buttons;
  uint16 new_buttons;
  bool quit_requested;
} MiniControlState;

typedef struct MiniPlayerInputState {
  uint16 buttons;
  uint16 previous_buttons;
  uint16 new_buttons;
} MiniPlayerInputState;

typedef struct MiniSamusCoreState {
  int world_x;
  int world_y;
  int x_velocity;
  int y_velocity;
  int screen_x;
  int screen_y;
  uint16 x_radius;
  uint16 y_radius;
  uint16 pose;
  uint16 movement_type;
  MiniSamusSuit suit;
  bool on_ground;
} MiniSamusCoreState;

typedef struct MiniPlayerCombatState {
  uint16 hit_count;
  uint16 pending_damage;
  uint16 hitstun_frames;
  uint16 invulnerable_frames;
  uint8 last_hit_by_player;
} MiniPlayerCombatState;

typedef struct MiniPlayerState {
  MiniSamusCoreState samus;
  MiniPlayerCombatState combat;
} MiniPlayerState;

typedef struct MiniRoomState {
  bool has_room;
  bool uses_rom_room;
  bool booted_from_save_slot;
  bool has_editor_room_visuals;
  bool uses_original_gameplay_runtime;
  bool has_original_enemies;
  bool has_original_plms;
  MiniSamusSuit samus_suit;
  uint16 room_id;
  char room_handle[kMiniRoomHandleCapacity];
  char room_name[kMiniRoomNameCapacity];
  MiniRoomSource room_source;
  int room_left;
  int room_top;
  int room_right;
  int room_bottom;
  int room_width_blocks;
  int room_height_blocks;
  int camera_x;
  int camera_y;
  int spawn_x;
  int spawn_y;
  int camera_target_x_percent;
  int camera_target_y_percent;
  int doorway_count;
  MiniDoorwayTransition doorways[kMiniDoorwayTransitionCapacity];
} MiniRoomState;

typedef struct MiniProjectileState {
  int count;
  SamusProjectileView views[kMiniProjectileViewCapacity];
  uint8 owner_by_slot[kSamusProjectileSlotCount];
} MiniProjectileState;

// Endless-climb progress. Lives in MiniGameState so save states, rollback,
// and the state hash cover it without module side channels.
typedef struct MiniClimbState {
  int virtual_floors;
  bool lava_enabled;
  int lava_floor_y;
  int lava_rise_carry_q8;
  int lava_enable_frame;
  int lava_damage_cooldown;
  int run_start_frame;
  int deaths;
  int best_ascent_pixels;
  int ascent_pixels;
  int last_samus_y;
  bool has_score_anchor;
} MiniClimbState;

typedef struct MiniGameState {
  MiniViewportState viewport;
  MiniRoomState room;
  MiniCollisionMapView collision_map;
  MiniSamusCoreState samus;
  MiniControlState controls;
  MiniProjectileState projectile_state;
  MiniEnemyState enemy_state;
  MiniClimbState climb;
  int player_count;
  MiniPlayerInputState player_inputs[kMiniMaxPlayers];
  MiniPlayerState players[kMiniMaxPlayers];
  uint8 player_runtime_pre_projectile[kMiniMaxPlayers][kMiniPlayerRuntimePreProjectileSize];
  uint8 player_runtime_post_projectile[kMiniMaxPlayers][kMiniPlayerRuntimePostProjectileSize];

  // Compatibility fields for existing mini callers. Prefer the typed views above
  // for new mini-facing code.
  int frame;
  int viewport_width;
  int viewport_height;
  uint16 room_id;
  int room_width_blocks;
  int room_height_blocks;
  int room_left;
  int room_top;
  int room_right;
  int room_bottom;
  int camera_x;
  int camera_y;
  int samus_x;
  int samus_y;
  uint16 samus_pose_value;
  uint16 samus_movement_type_value;
  int original_oam_next_ptr;
  int projectile_count;
  SamusProjectileView projectiles[kMiniProjectileViewCapacity];
  int ground_y;
  uint16 last_buttons;
  bool has_room;
  bool uses_rom_room;
  bool has_editor_room_visuals;
  bool uses_original_gameplay_runtime;
  bool has_original_enemies;
  bool has_original_plms;
  bool quit_requested;
  MiniSamusSuit samus_suit;
  MiniRoomSource room_source;
  char room_handle[32];
  char room_name[64];
} MiniGameState;

void MiniGameState_Init(MiniGameState *state, int viewport_width, int viewport_height);
void MiniUpdate(MiniGameState *state, const MiniInputState *input);
uint64_t MiniGameState_ComputeHash(const MiniGameState *state);

void MiniInit(MiniGameState *state, int viewport_width, int viewport_height);
void MiniSetPlayerCount(MiniGameState *state, int player_count);
void MiniStep(MiniGameState *state, const MiniInputState *input);
void MiniStepButtons(MiniGameState *state, uint16 buttons, bool quit_requested);
void MiniStepPlayers(MiniGameState *state, const uint16 *player_buttons,
                     int player_count, bool quit_requested);
uint64_t MiniStateHash(const MiniGameState *state);
size_t MiniSaveStateSize(void);
bool MiniSaveState(const MiniGameState *state, void *buffer, size_t buffer_size);
bool MiniLoadState(MiniGameState *state, const void *buffer, size_t buffer_size);

MiniGameState *MiniCreate(int viewport_width, int viewport_height);
void MiniDestroy(MiniGameState *state);

#endif  // SM_MINI_GAME_H_
