#ifndef SM_PICO_EXPLORER_BUTTONS_H_
#define SM_PICO_EXPLORER_BUTTONS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Pimoroni Pico Explorer A/B/X/Y → SNES joypad bits for MiniStepButtons.
 *
 * Physical pins (active-low, pull-up) are not part of this mapper:
 *   A=GP12  B=GP13  X=GP14  Y=GP15
 *
 * Smallest walk map (four buttons, no combos):
 *   Y → Right
 *   X → Left
 *   B → Down
 *   A → Jump (kButton_A)
 *
 * `pressed` bits are already 1 = down. Host tests this; GPIO poll stays
 * in the RP2350 main.
 */
enum {
  kExplorerBtnA = 1u << 0,
  kExplorerBtnB = 1u << 1,
  kExplorerBtnX = 1u << 2,
  kExplorerBtnY = 1u << 3
};

uint16_t ExplorerButtons_ToJoypad(unsigned pressed);

#ifdef __cplusplus
}
#endif

#endif /* SM_PICO_EXPLORER_BUTTONS_H_ */
