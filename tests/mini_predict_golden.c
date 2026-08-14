#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "ida_types.h"
#include "mini/mini_defs.h"
#include "mini/mini_game.h"
#include "mini/mini_predict.h"
#include "mini_test_room.h"
#include "types.h"
#include "variables.h"  // For samus_x_subpos, samus_y_subpos

#define ASSERT_EQ(actual, expected, field) \
  do { \
    if ((actual) != (expected)) { \
      fprintf(stderr, "FAIL at frame %zu: %s mismatch: expected %d, got %d\n", \
              i, (field), (int)(expected), (int)(actual)); \
      return false; \
    } \
  } while (0)

static bool FramesMatch(const MiniTrajectoryFrame *pred, const MiniGameState *oracle, size_t i) {
  MiniTrajectoryFrame captured = MiniCaptureTrajectoryFrame(oracle, (int)i);
  ASSERT_EQ(pred->samus_x, captured.samus_x, "samus_x");
  ASSERT_EQ(pred->samus_x_sub, captured.samus_x_sub, "samus_x_sub");
  ASSERT_EQ(pred->samus_y, captured.samus_y, "samus_y");
  ASSERT_EQ(pred->samus_y_sub, captured.samus_y_sub, "samus_y_sub");
  ASSERT_EQ(pred->velocity_x, captured.velocity_x, "velocity_x");
  ASSERT_EQ(pred->velocity_y, captured.velocity_y, "velocity_y");
  ASSERT_EQ(pred->pose, captured.pose, "pose");
  ASSERT_EQ(pred->movement_type, captured.movement_type, "movement_type");
  if (captured.enemies)
    free(captured.enemies);
  return true;
}

static bool test_ground_run_golden(void) {
  printf("Running golden test: Ground run (hold RIGHT)\n");
  
  const size_t kInitFrames = 10;
  const size_t kRunFrames = 60;
  const size_t kFrameCount = kInitFrames + kRunFrames;
  uint16 inputs[70];
  // Let state initialize
  for (size_t i = 0; i < kInitFrames; i++) {
    inputs[i] = 0;
  }
  // Hold RIGHT
  for (size_t i = kInitFrames; i < kFrameCount; i++) {
    inputs[i] = kButton_Right;
  }

  // Create test room and save initial state ONCE
  MiniGameState *test_state = MiniTestRoom_CreateWithFloor(kMiniGameWidth, kMiniGameHeight);
  if (!test_state) {
    fprintf(stderr, "FAIL: MiniTestRoom_CreateWithFloor failed\n");
    return false;
  }
  
  size_t snapshot_size = MiniSaveStateSize();
  void *snapshot = malloc(snapshot_size);
  if (!snapshot || !MiniSaveState(test_state, snapshot, snapshot_size)) {
    MiniDestroy(test_state);
    fprintf(stderr, "FAIL: MiniSaveState failed\n");
    return false;
  }
  MiniDestroy(test_state);

  // Run prediction from snapshot
  // When loading a snapshot, prediction captures frame 0 (pre-step) plus all input frames
  MiniPrediction *prediction = MiniPrediction_Create(kFrameCount + 1);
  if (!prediction) {
    free(snapshot);
    fprintf(stderr, "FAIL: MiniPrediction_Create failed\n");
    return false;
  }

  if (!MiniPredict(prediction, snapshot, snapshot_size, inputs, kFrameCount, kMiniGameWidth, kMiniGameHeight)) {
    MiniPrediction_Destroy(prediction);
    free(snapshot);
    fprintf(stderr, "FAIL: MiniPredict failed\n");
    return false;
  }

  // Run oracle (MiniStep) from the SAME snapshot
  MiniGameState *oracle_state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  if (!oracle_state) {
    MiniPrediction_Destroy(prediction);
    free(snapshot);
    fprintf(stderr, "FAIL: MiniCreate failed for oracle\n");
    return false;
  }
  
  if (!MiniLoadState(oracle_state, snapshot, snapshot_size)) {
    MiniDestroy(oracle_state);
    MiniPrediction_Destroy(prediction);
    free(snapshot);
    fprintf(stderr, "FAIL: MiniLoadState failed for oracle\n");
    return false;
  }
  free(snapshot);

  // Debug: Print initial position
  printf("  Initial: x=%d, y=%d, on_ground=%d, movement_type=%d\n",
         oracle_state->samus.world_x, oracle_state->samus.world_y,
         oracle_state->samus.on_ground, oracle_state->samus.movement_type);

  // Verify frame-by-frame against oracle at SUB-PIXEL accuracy
  // Note: frames[0] is the pre-step state, frames[1..N] are post-step for inputs[0..N-1]
  for (size_t i = 0; i < kFrameCount; i++) {
    MiniStepButtons(oracle_state, inputs[i], false);
    
    const MiniTrajectoryFrame *pred_frame = &prediction->frames[i + 1];  // Skip frame 0 (pre-step)
    
    // Debug: Print position every 10 frames
    if (i % 10 == 0 || i == kFrameCount - 1) {
      printf("  Frame %zu: oracle x=%d, pred x=%d, vx=%d, on_ground=%d\n",
             i, oracle_state->samus.world_x, pred_frame->samus_x,
             oracle_state->samus.x_velocity, oracle_state->samus.on_ground);
    }
    
    if (!FramesMatch(pred_frame, oracle_state, i))
      return false;
    
    // Note: state_hash no longer in wire format frame
  }

  // Print final position for verification
  const MiniTrajectoryFrame *final_frame = &prediction->frames[kFrameCount];  // Last post-step frame
  printf("  Final position after %zu frames: x=%d.%04x, y=%d.%04x\n",
         kFrameCount,
         final_frame->samus_x, final_frame->samus_x_sub & 0xFFFF,
         final_frame->samus_y, final_frame->samus_y_sub & 0xFFFF);
  printf("  Final velocity: vx=%d, vy=%d\n",
         final_frame->velocity_x, final_frame->velocity_y);

  MiniDestroy(oracle_state);
  MiniPrediction_Destroy(prediction);
  printf("  ✓ Ground run golden test PASSED (sub-pixel accuracy verified)\n");
  return true;
}

static bool test_jump_height_golden(void) {
  printf("Running golden test: Jump height (short hop vs full hop)\n");
  
  // Short hop: press jump for 1 frame
  const size_t kShortHopFrames = 30;
  uint16 short_hop_inputs[30];
  short_hop_inputs[0] = 0;  // standing still
  short_hop_inputs[1] = kButton_A;  // press jump
  for (size_t i = 2; i < kShortHopFrames; i++) {
    short_hop_inputs[i] = 0;  // release immediately
  }

  // Full hop: hold jump for longer
  const size_t kFullHopFrames = 50;
  uint16 full_hop_inputs[50];
  full_hop_inputs[0] = 0;
  for (size_t i = 1; i <= 10; i++) {
    full_hop_inputs[i] = kButton_A;  // hold jump
  }
  for (size_t i = 11; i < kFullHopFrames; i++) {
    full_hop_inputs[i] = 0;
  }

  // Create test room snapshot
  MiniGameState *test_state = MiniTestRoom_CreateWithFloor(kMiniGameWidth, kMiniGameHeight);
  if (!test_state) {
    fprintf(stderr, "FAIL: MiniTestRoom_CreateWithFloor failed\n");
    return false;
  }
  size_t snapshot_size = MiniSaveStateSize();
  void *snapshot = malloc(snapshot_size);
  if (!snapshot || !MiniSaveState(test_state, snapshot, snapshot_size)) {
    MiniDestroy(test_state);
    fprintf(stderr, "FAIL: MiniSaveState failed\n");
    return false;
  }
  MiniDestroy(test_state);

  // Test short hop from test room
  // When loading a snapshot, allocate space for pre-step frame + all inputs
  MiniPrediction *short_prediction = MiniPrediction_Create(kShortHopFrames + 1);
  if (!short_prediction || !MiniPredict(short_prediction, snapshot, snapshot_size, short_hop_inputs, kShortHopFrames, kMiniGameWidth, kMiniGameHeight)) {
    free(snapshot);
    fprintf(stderr, "FAIL: Short hop prediction failed\n");
    return false;
  }

  // Test full hop from test room
  MiniPrediction *full_prediction = MiniPrediction_Create(kFullHopFrames + 1);
  if (!full_prediction || !MiniPredict(full_prediction, snapshot, snapshot_size, full_hop_inputs, kFullHopFrames, kMiniGameWidth, kMiniGameHeight)) {
    free(snapshot);
    MiniPrediction_Destroy(short_prediction);
    fprintf(stderr, "FAIL: Full hop prediction failed\n");
    return false;
  }
  free(snapshot);

  // Verify short hop against oracle with test room
  MiniGameState *short_oracle = MiniTestRoom_CreateWithFloor(kMiniGameWidth, kMiniGameHeight);
  if (!short_oracle) {
    MiniPrediction_Destroy(short_prediction);
    MiniPrediction_Destroy(full_prediction);
    return false;
  }

  int short_peak_y = short_prediction->frames[1].samus_y;  // Start from frame 1 (first post-step)
  size_t short_peak_frame = 0;
  for (size_t i = 0; i < kShortHopFrames; i++) {
    MiniStepButtons(short_oracle, short_hop_inputs[i], false);
    const MiniTrajectoryFrame *frame = &short_prediction->frames[i + 1];  // Skip frame 0 (pre-step)
    
    ASSERT_EQ(frame->samus_y, short_oracle->samus.world_y, "short_hop_y");
    ASSERT_EQ(frame->samus_y_sub, samus_y_subpos, "short_hop_y_sub");
    
    if (frame->samus_y < short_peak_y) {
      short_peak_y = frame->samus_y;
      short_peak_frame = i;
    }
  }

  // Verify full hop against oracle with test room
  MiniGameState *full_oracle = MiniTestRoom_CreateWithFloor(kMiniGameWidth, kMiniGameHeight);
  if (!full_oracle) {
    MiniDestroy(short_oracle);
    MiniPrediction_Destroy(short_prediction);
    MiniPrediction_Destroy(full_prediction);
    return false;
  }

  int full_peak_y = full_prediction->frames[1].samus_y;  // Start from frame 1 (first post-step)
  size_t full_peak_frame = 0;
  for (size_t i = 0; i < kFullHopFrames; i++) {
    MiniStepButtons(full_oracle, full_hop_inputs[i], false);
    const MiniTrajectoryFrame *frame = &full_prediction->frames[i + 1];  // Skip frame 0 (pre-step)
    
    ASSERT_EQ(frame->samus_y, full_oracle->samus.world_y, "full_hop_y");
    ASSERT_EQ(frame->samus_y_sub, samus_y_subpos, "full_hop_y_sub");
    
    if (frame->samus_y < full_peak_y) {
      full_peak_y = frame->samus_y;
      full_peak_frame = i;
    }
  }

  printf("  Short hop: peak y=%d at frame %zu\n", short_peak_y, short_peak_frame);
  printf("  Full hop: peak y=%d at frame %zu\n", full_peak_y, full_peak_frame);
  printf("  Jump height difference: %d pixels\n", short_peak_y - full_peak_y);

  // Full hop should reach higher (lower y value)
  if (full_peak_y >= short_peak_y) {
    fprintf(stderr, "FAIL: Full hop (%d) did not reach higher than short hop (%d)\n",
            full_peak_y, short_peak_y);
    MiniDestroy(short_oracle);
    MiniDestroy(full_oracle);
    MiniPrediction_Destroy(short_prediction);
    MiniPrediction_Destroy(full_prediction);
    return false;
  }

  MiniDestroy(short_oracle);
  MiniDestroy(full_oracle);
  MiniPrediction_Destroy(short_prediction);
  MiniPrediction_Destroy(full_prediction);
  printf("  ✓ Jump height golden test PASSED (sub-pixel accuracy verified)\n");
  return true;
}

static bool test_run_jump_platform_golden(void) {
  printf("Running golden test: Run + jump (combined platforming)\n");
  
  // Run right for a few frames, then jump
  const size_t kFrameCount = 40;
  uint16 inputs[40];
  
  // Frames 0-5: stand still
  for (size_t i = 0; i < 6; i++) {
    inputs[i] = 0;
  }
  
  // Frames 6-15: run right
  for (size_t i = 6; i < 16; i++) {
    inputs[i] = kButton_Right;
  }
  
  // Frames 16-25: run right + jump
  for (size_t i = 16; i < 26; i++) {
    inputs[i] = kButton_Right | kButton_A;
  }
  
  // Frames 26-39: continue right (in air and landing)
  for (size_t i = 26; i < kFrameCount; i++) {
    inputs[i] = kButton_Right;
  }

  // Create test room snapshot (single source of truth for both prediction and oracle)
  MiniGameState *test_state = MiniTestRoom_CreateWithFloor(kMiniGameWidth, kMiniGameHeight);
  if (!test_state) {
    fprintf(stderr, "FAIL: MiniTestRoom_CreateWithFloor failed\n");
    return false;
  }
  size_t snapshot_size = MiniSaveStateSize();
  void *snapshot = malloc(snapshot_size);
  if (!snapshot || !MiniSaveState(test_state, snapshot, snapshot_size)) {
    MiniDestroy(test_state);
    fprintf(stderr, "FAIL: MiniSaveState failed\n");
    return false;
  }
  MiniDestroy(test_state);

  // Run prediction from snapshot
  // When loading a snapshot, allocate space for pre-step frame + all inputs
  MiniPrediction *prediction = MiniPrediction_Create(kFrameCount + 1);
  if (!prediction) {
    free(snapshot);
    fprintf(stderr, "FAIL: MiniPrediction_Create failed\n");
    return false;
  }

  if (!MiniPredict(prediction, snapshot, snapshot_size, inputs, kFrameCount, kMiniGameWidth, kMiniGameHeight)) {
    free(snapshot);
    MiniPrediction_Destroy(prediction);
    fprintf(stderr, "FAIL: MiniPredict failed\n");
    return false;
  }

  // Create oracle from SAME snapshot (ensures identical starting state)
  MiniGameState *oracle = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  if (!oracle || !MiniLoadState(oracle, snapshot, snapshot_size)) {
    free(snapshot);
    MiniPrediction_Destroy(prediction);
    fprintf(stderr, "FAIL: oracle MiniLoadState failed\n");
    return false;
  }
  free(snapshot);

  int peak_y = prediction->frames[1].samus_y;  // Start from frame 1 (first post-step)
  size_t peak_frame = 0;
  size_t takeoff_frame = 0;
  size_t landing_frame = 0;
  bool in_air = false;

  for (size_t i = 0; i < kFrameCount; i++) {
    MiniStepButtons(oracle, inputs[i], false);
    const MiniTrajectoryFrame *frame = &prediction->frames[i + 1];  // Skip frame 0 (pre-step)
    
    if (!FramesMatch(frame, oracle, i))
      return false;
    
    // Track jump phases (infer from velocity_y: negative = rising, positive = falling, zero on ground)
    bool is_grounded = (frame->velocity_y == 0 && i > 0 && prediction->frames[i].velocity_y >= 0);
    if (!in_air && !is_grounded) {
      takeoff_frame = i;
      in_air = true;
    }
    if (in_air && is_grounded) {
      landing_frame = i;
      in_air = false;
    }
    if (frame->samus_y < peak_y) {
      peak_y = frame->samus_y;
      peak_frame = i;
    }
  }

  const MiniTrajectoryFrame *final_frame = &prediction->frames[kFrameCount];  // Last post-step frame
  printf("  Takeoff frame: %zu\n", takeoff_frame);
  printf("  Peak y=%d at frame %zu\n", peak_y, peak_frame);
  if (landing_frame > 0) {
    printf("  Landing frame: %zu\n", landing_frame);
    printf("  Air time: %zu frames\n", landing_frame - takeoff_frame);
  }
  printf("  Final position: x=%d.%04x, y=%d.%04x\n",
         final_frame->samus_x, final_frame->samus_x_sub & 0xFFFF,
         final_frame->samus_y, final_frame->samus_y_sub & 0xFFFF);
  printf("  Horizontal distance: %d pixels\n",
         final_frame->samus_x - prediction->frames[1].samus_x);  // Compare to first post-step frame

  MiniDestroy(oracle);
  MiniPrediction_Destroy(prediction);
  printf("  NOTE: Mini baseline does not yet model platform collision (1-tile landing unverified).\n");
  printf("  ✓ Run + jump golden test PASSED (sub-pixel accuracy verified)\n");
  return true;
}

int main(void) {
  printf("=== Mini Prediction Golden Tests (Sub-Pixel Accuracy) ===\n\n");
  
  bool all_passed = true;
  
  if (!test_ground_run_golden()) {
    all_passed = false;
  }
  printf("\n");
  
  if (!test_jump_height_golden()) {
    all_passed = false;
  }
  printf("\n");
  
  if (!test_run_jump_platform_golden()) {
    all_passed = false;
  }
  printf("\n");
  
  if (all_passed) {
    printf("\n=== ALL MINI BASELINE GOLDEN TESTS PASSED ===\n");
    printf("Prediction matches MiniStep baseline (sub-pixel consistent, fast iteration).\n");
    printf("NOT TAS ground truth - emulator acceptance via retro_rl determines correctness.\n");
    return 0;
  } else {
    printf("=== SOME GOLDEN TESTS FAILED ===\n");
    return 1;
  }
}
