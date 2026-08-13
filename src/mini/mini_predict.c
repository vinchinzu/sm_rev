#include "mini_predict.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mini_game.h"
#include "mini_room_adapter.h"
#include "variables.h"

static MiniTrajectoryFrame MiniCaptureTrajectoryFrame(const MiniGameState *state, int frame) {
  MiniTrajectoryFrame result = {0};
  result.frame = frame;
  result.world_x = state->samus.world_x;
  result.world_y = state->samus.world_y;
  result.x_subpos = samus_x_subpos;
  result.y_subpos = samus_y_subpos;
  result.x_velocity = state->samus.x_velocity;
  result.y_velocity = state->samus.y_velocity;
  result.x_extra_run_speed = samus_x_extra_run_speed;
  result.y_speed = samus_y_speed;
  result.pose = state->samus.pose;
  result.movement_type = state->samus.movement_type;
  result.buttons = state->controls.buttons;
  result.on_ground = state->samus.on_ground;
  result.state_hash = MiniStateHash(state);
  return result;
}

MiniPrediction *MiniPrediction_Create(size_t capacity) {
  MiniPrediction *prediction = (MiniPrediction *)malloc(sizeof(MiniPrediction));
  if (!prediction)
    return NULL;

  prediction->frames = (MiniTrajectoryFrame *)malloc(capacity * sizeof(MiniTrajectoryFrame));
  if (!prediction->frames) {
    free(prediction);
    return NULL;
  }

  prediction->frame_count = 0;
  prediction->capacity = capacity;
  return prediction;
}

void MiniPrediction_Destroy(MiniPrediction *prediction) {
  if (!prediction)
    return;
  free(prediction->frames);
  free(prediction);
}

bool MiniPredict(MiniPrediction *prediction,
                 const void *state_snapshot,
                 size_t snapshot_size,
                 const uint16 *input_buttons,
                 size_t input_count,
                 int viewport_width,
                 int viewport_height) {
  if (!prediction || !input_buttons || input_count == 0)
    return false;

  if (input_count > prediction->capacity)
    return false;

  MiniGameState *state = NULL;
  bool created_state = false;

  if (state_snapshot != NULL && snapshot_size > 0) {
    state = MiniCreate(viewport_width, viewport_height);
    if (!state)
      return false;
    created_state = true;
    if (!MiniLoadState(state, state_snapshot, snapshot_size)) {
      MiniDestroy(state);
      return false;
    }
  } else {
    state = MiniCreate(viewport_width, viewport_height);
    if (!state)
      return false;
    created_state = true;
  }

  prediction->frame_count = 0;
  for (size_t i = 0; i < input_count; i++) {
    MiniStepButtons(state, input_buttons[i], false);
    prediction->frames[prediction->frame_count++] = MiniCaptureTrajectoryFrame(state, (int)i);
  }

  if (created_state)
    MiniDestroy(state);

  return true;
}
