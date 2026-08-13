#ifndef SM_MINI_PREDICT_H_
#define SM_MINI_PREDICT_H_

#include <stddef.h>
#include <stdint.h>
#include "types.h"

// Trajectory frame: per-frame Samus state snapshot for prediction
typedef struct MiniTrajectoryFrame {
  int frame;
  int world_x;
  int world_y;
  int x_subpos;
  int y_subpos;
  int16 x_velocity;
  int16 y_velocity;
  int16 x_extra_run_speed;
  int16 y_speed;
  uint16 pose;
  uint16 movement_type;
  uint16 buttons;
  bool on_ground;
  uint64_t state_hash;
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

#endif  // SM_MINI_PREDICT_H_
