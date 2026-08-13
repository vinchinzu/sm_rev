#ifndef SM_MINI_PREDICT_H_
#define SM_MINI_PREDICT_H_

#include <stddef.h>
#include <stdint.h>
#include "types.h"

// Forward declaration
typedef struct MiniGameState MiniGameState;

// Enemy snapshot for trajectory frame (future use)
typedef struct MiniEnemySnapshot {
  uint16 enemy_id;        // species_id
  uint16 enemy_type;      // behavior enum
  int world_x;
  int world_y;
  int16 x_velocity;
  int16 y_velocity;
} MiniEnemySnapshot;

// Trajectory frame: per-frame Samus state snapshot for prediction
// Wire format compatible with retro_rl physics_sim.TrajectoryFrame
// Exposes full observable state (Oσ†) from g_ram after MiniLoadState
typedef struct MiniTrajectoryFrame {
  int frame;
  int room_id;
  // Position (samus_x/y + sub-pixel from g_ram $0AF6/$0AF8/$0AFA/$0AFC)
  int samus_x;
  int samus_y;
  int samus_x_sub;
  int samus_y_sub;
  // Velocity (signed 16-bit + sub-pixel)
  int16 velocity_x;
  int16 velocity_y;
  int16 velocity_x_sub;
  int16 velocity_y_sub;
  // Momentum (speed booster state - mini always zero)
  int16 momentum_x;
  int16 momentum_x_sub;
  // Pose and movement
  uint16 pose;
  uint8 facing;           // 0x04=left, 0x08=right
  uint16 movement_type;
  // Speed booster state (mini always zero)
  uint16 speed_counter;
  uint16 speed_flag;
  uint16 shinespark_timer;
  // Energy and death state (from g_ram $09C2, game state)
  uint16 energy;          // $09C2: current health
  bool is_dead;           // Samus death state
  bool is_game_over;      // Game over state
  // Frame counters (from g_ram $1842/$09DA)
  uint32 frame_counter_1;    // $1842: first frame counter (uint32)
  uint16 frame_counter_2;    // $09DA: second frame counter (uint16)
  
  // Enemy tracking (wire format compatibility - currently always empty)
  MiniEnemySnapshot *enemies;
  size_t enemy_count;
} MiniTrajectoryFrame;

// Prediction result: array of trajectory frames
typedef struct MiniPrediction {
  MiniTrajectoryFrame *frames;
  size_t frame_count;
  size_t capacity;
} MiniPrediction;

// Create a prediction with given capacity
MiniPrediction *MiniPrediction_Create(size_t capacity);

// Destroy a prediction
void MiniPrediction_Destroy(MiniPrediction *prediction);

// Predict trajectory from initial state + input sequence
// Returns true on success, false on error
// If state_snapshot is NULL, creates a fresh mini state
// Otherwise loads from the snapshot before prediction
bool MiniPredict(MiniPrediction *prediction,
                 const void *state_snapshot,
                 size_t snapshot_size,
                 const uint16 *input_buttons,
                 size_t input_count,
                 int viewport_width,
                 int viewport_height);

// Capture current frame state for trajectory (exposed for testing)
MiniTrajectoryFrame MiniCaptureTrajectoryFrame(const MiniGameState *state, int frame);

#endif  // SM_MINI_PREDICT_H_
