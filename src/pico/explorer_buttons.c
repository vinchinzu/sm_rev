#include "explorer_buttons.h"

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
