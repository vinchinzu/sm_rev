#include "explorer_buttons.h"

#include <stddef.h>

#include "ida_types.h"

uint16_t ExplorerButtons_ToJoypad(unsigned pressed) {
  uint16_t joy = 0;
  if (pressed & kExplorerBtnA)
    joy |= kButton_A;
  if (pressed & kExplorerBtnB)
    joy |= kButton_Down;
  if (pressed & kExplorerBtnX)
    joy |= kButton_Left;
  if (pressed & kExplorerBtnY)
    joy |= kButton_Right;
  return joy;
}

void ExplorerButtons_Init(ExplorerButtonsState *state) {
  if (state == NULL)
    return;
  state->prev_pressed = 0;
  state->morph_bit = 0;
  state->armed = 0;
}

int ExplorerButtons_IsMorphMovement(unsigned movement_type) {
  return movement_type == (unsigned)kMovementType_04_MorphBallOnGround ||
         movement_type == (unsigned)kMovementType_08_MorphBallFalling;
}

uint16_t ExplorerButtons_Step(ExplorerButtonsState *state, unsigned pressed,
                              unsigned movement_type) {
  uint16_t joy = 0;

  if (state == NULL)
    return ExplorerButtons_ToJoypad(pressed);

  /* Boot / chord gate: swallow whatever is already held when we start. */
  if (!state->armed) {
    if (pressed == 0)
      state->armed = 1;
    state->prev_pressed = pressed;
    state->morph_bit = 0;
    return 0;
  }

  if (pressed & kExplorerBtnA)
    joy |= kButton_A;
  if (pressed & kExplorerBtnX)
    joy |= kButton_Left;
  if (pressed & kExplorerBtnY)
    joy |= kButton_Right;

  if (pressed & kExplorerBtnB) {
    if ((state->prev_pressed & kExplorerBtnB) == 0) {
      /* Rising edge: pick the direction once, from the pose she is in now. */
      state->morph_bit = ExplorerButtons_IsMorphMovement(movement_type)
                             ? (uint16_t)kButton_Up
                             : (uint16_t)kButton_Down;
    }
    joy |= state->morph_bit;
  } else {
    state->morph_bit = 0;
  }

  state->prev_pressed = pressed;
  return joy;
}
