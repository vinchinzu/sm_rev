#ifndef SM_MINI_GAME_H_
#define SM_MINI_GAME_H_

#include "stubs_mini.h"
#include "types.h"
#include <stdint.h>

enum {
  kMiniSamusWidth = 24,
  kMiniSamusHeight = 40,
  kMiniGroundSpeed = 3,
};

typedef struct MiniInputState {
  uint16 buttons;
  bool quit_requested;
} MiniInputState;

typedef struct MiniGameState {
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
  int ground_y;
  uint16 last_buttons;
  bool has_room;
  bool uses_rom_room;
  bool has_editor_room_visuals;
  bool quit_requested;
  MiniSamusSuit samus_suit;
  MiniRoomSource room_source;
  char room_handle[32];
  char room_name[64];
} MiniGameState;

void MiniGameState_Init(MiniGameState *state, int viewport_width, int viewport_height);
void MiniUpdate(MiniGameState *state, const MiniInputState *input);
uint64_t MiniGameState_ComputeHash(const MiniGameState *state);

#endif  // SM_MINI_GAME_H_
