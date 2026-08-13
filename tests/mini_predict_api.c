#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ida_types.h"
#include "mini/mini_defs.h"
#include "mini/mini_game.h"
#include "mini/mini_predict.h"
#include "types.h"

static void Require(bool condition, const char *message) {
  if (!condition) {
    fprintf(stderr, "FAIL: %s\n", message);
    exit(1);
  }
}

static void TestPredictionDeterminism(void) {
  const size_t kFrameCount = 12;
  uint16 inputs[12] = {
    0,
    kButton_Right,
    kButton_Right,
    kButton_Right | kButton_A,
    kButton_Right | kButton_A,
    kButton_Right,
    kButton_Right,
    0,
    kButton_Left,
    kButton_Left,
    0,
    0,
  };

  MiniPrediction *pred1 = MiniPrediction_Create(kFrameCount);
  Require(pred1 != NULL, "MiniPrediction_Create failed for first prediction");

  MiniPrediction *pred2 = MiniPrediction_Create(kFrameCount);
  Require(pred2 != NULL, "MiniPrediction_Create failed for second prediction");

  bool success1 = MiniPredict(pred1, NULL, 0, inputs, kFrameCount, kMiniGameWidth, kMiniGameHeight);
  Require(success1, "MiniPredict failed for first run");
  Require(pred1->frame_count == kFrameCount, "First prediction frame count mismatch");

  bool success2 = MiniPredict(pred2, NULL, 0, inputs, kFrameCount, kMiniGameWidth, kMiniGameHeight);
  Require(success2, "MiniPredict failed for second run");
  Require(pred2->frame_count == kFrameCount, "Second prediction frame count mismatch");

  for (size_t i = 0; i < kFrameCount; i++) {
    Require(pred1->frames[i].frame == pred2->frames[i].frame,
            "Frame number mismatch for determinism test");
    Require(pred1->frames[i].state_hash == pred2->frames[i].state_hash,
            "State hash mismatch for determinism test");
    Require(pred1->frames[i].world_x == pred2->frames[i].world_x,
            "World X mismatch for determinism test");
    Require(pred1->frames[i].world_y == pred2->frames[i].world_y,
            "World Y mismatch for determinism test");
    Require(pred1->frames[i].pose == pred2->frames[i].pose,
            "Pose mismatch for determinism test");
  }

  MiniPrediction_Destroy(pred1);
  MiniPrediction_Destroy(pred2);
}

static void TestPredictionInputSensitivity(void) {
  const size_t kFrameCount = 8;
  uint16 idle_inputs[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  uint16 move_inputs[8] = {
    kButton_Right,
    kButton_Right,
    kButton_Right | kButton_A,
    kButton_Right,
    kButton_Right,
    kButton_Right,
    kButton_Right,
    kButton_Right,
  };

  MiniPrediction *idle_pred = MiniPrediction_Create(kFrameCount);
  Require(idle_pred != NULL, "MiniPrediction_Create failed for idle prediction");

  MiniPrediction *move_pred = MiniPrediction_Create(kFrameCount);
  Require(move_pred != NULL, "MiniPrediction_Create failed for move prediction");

  bool idle_success = MiniPredict(idle_pred, NULL, 0, idle_inputs, kFrameCount, kMiniGameWidth, kMiniGameHeight);
  Require(idle_success, "MiniPredict failed for idle run");
  Require(idle_pred->frame_count == kFrameCount, "Idle prediction frame count mismatch");

  bool move_success = MiniPredict(move_pred, NULL, 0, move_inputs, kFrameCount, kMiniGameWidth, kMiniGameHeight);
  Require(move_success, "MiniPredict failed for move run");
  Require(move_pred->frame_count == kFrameCount, "Move prediction frame count mismatch");

  bool found_difference = false;
  for (size_t i = 0; i < kFrameCount; i++) {
    if (idle_pred->frames[i].state_hash != move_pred->frames[i].state_hash ||
        idle_pred->frames[i].world_x != move_pred->frames[i].world_x) {
      found_difference = true;
      break;
    }
  }
  Require(found_difference, "Different inputs produced identical trajectories");

  MiniPrediction_Destroy(idle_pred);
  MiniPrediction_Destroy(move_pred);
}

static void TestPredictionFromSnapshot(void) {
  const size_t kSetupFrames = 6;
  const size_t kPredictFrames = 6;
  uint16 setup_inputs[6] = {
    kButton_Right,
    kButton_Right,
    kButton_Right | kButton_A,
    kButton_Right,
    kButton_Right,
    kButton_Right,
  };
  uint16 predict_inputs[6] = {
    kButton_Left,
    kButton_Left,
    kButton_Left | kButton_A,
    kButton_Left,
    0,
    0,
  };

  MiniGameState *setup_state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(setup_state != NULL, "MiniCreate failed for snapshot test");

  for (size_t i = 0; i < kSetupFrames; i++)
    MiniStepButtons(setup_state, setup_inputs[i], false);

  size_t snapshot_size = MiniSaveStateSize();
  void *snapshot = malloc(snapshot_size);
  Require(snapshot != NULL, "snapshot allocation failed");
  Require(MiniSaveState(setup_state, snapshot, snapshot_size), "MiniSaveState failed");

  MiniPrediction *pred = MiniPrediction_Create(kPredictFrames);
  Require(pred != NULL, "MiniPrediction_Create failed for snapshot prediction");

  bool success = MiniPredict(pred, snapshot, snapshot_size, predict_inputs, kPredictFrames, kMiniGameWidth, kMiniGameHeight);
  Require(success, "MiniPredict failed from snapshot");
  Require(pred->frame_count == kPredictFrames, "Snapshot prediction frame count mismatch");

  MiniGameState *verify_state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(verify_state != NULL, "MiniCreate failed for verification");
  Require(MiniLoadState(verify_state, snapshot, snapshot_size), "MiniLoadState failed");

  for (size_t i = 0; i < kPredictFrames; i++) {
    MiniStepButtons(verify_state, predict_inputs[i], false);
    uint64_t verify_hash = MiniStateHash(verify_state);
    Require(pred->frames[i].state_hash == verify_hash,
            "Prediction from snapshot did not match manual stepping");
  }

  MiniPrediction_Destroy(pred);
  free(snapshot);
  MiniDestroy(setup_state);
  MiniDestroy(verify_state);
}

int main(void) {
  printf("Running mini prediction API tests...\n");
  TestPredictionDeterminism();
  printf("  ✓ Prediction determinism\n");
  TestPredictionInputSensitivity();
  printf("  ✓ Prediction input sensitivity\n");
  TestPredictionFromSnapshot();
  printf("  ✓ Prediction from snapshot\n");
  printf("All prediction API tests passed.\n");
  return 0;
}
