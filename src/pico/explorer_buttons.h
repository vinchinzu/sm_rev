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
 * Map (four buttons, no combos):
 *   Y → Right
 *   X → Left
 *   A → Jump (kButton_A)
 *   B → Morph / Unmorph  (kButton_Down when standing, kButton_Up when balled)
 *
 * WHY B IS CONTEXT-SENSITIVE (sm_rev-28r)
 * ---------------------------------------
 * With g_rom == NULL the Pico does not run the vanilla Samus state machine; it
 * runs MiniAuthoredMovement_Step() in src/mini/mini_authored_movement.c. There,
 *   - a single NEW kButton_Down while grounded morphs instantly (no crouch
 *     step, no second press), and
 *   - the ONLY way out is a NEW kButton_Up.
 * A four-button board has no Up, so the old "B → Down" map made morphball a
 * one-way door: one stray B press and Samus was a ball for the rest of the
 * session. B is therefore a toggle: it emits Down while she is on her feet and
 * Up while she is a ball, so entering morphball stays a deliberate single press
 * and leaving it is the same press again.
 *
 * `pressed` bits are already 1 = down. Host tests this; GPIO poll stays in the
 * RP2350 main.
 */
enum {
  kExplorerBtnA = 1u << 0,
  kExplorerBtnB = 1u << 1,
  kExplorerBtnX = 1u << 2,
  kExplorerBtnY = 1u << 3
};

/*
 * Frame-to-frame mapper state. Zero-initialised means "not armed yet".
 */
typedef struct ExplorerButtonsState {
  unsigned prev_pressed;  /* raw mask handed to the previous _Step */
  uint16_t morph_bit;     /* Down or Up, latched for as long as B is held */
  unsigned char armed;    /* 0 until one frame with nothing pressed is seen */
} ExplorerButtonsState;

void ExplorerButtons_Init(ExplorerButtonsState *state);

/*
 * Non-zero when this samus_movement_type is a morphball state. Mirrors
 * MiniAuthoredIsBallMovement() in src/mini/mini_authored_movement.c, which is
 * static there; keep the two in step.
 */
int ExplorerButtons_IsMorphMovement(unsigned movement_type);

/*
 * One frame of the map. `pressed` is the raw active-high button mask,
 * `movement_type` is the live samus_movement_type.
 *
 * Two behaviours the stateless map below cannot provide:
 *
 *  1. BOOT / CHORD GATE. Nothing is reported until one frame has been seen
 *     with no button down. A button still held when the sim starts -- the tail
 *     of the A+B+X+Y BOOTSEL chord, or a finger resting on the board through a
 *     reset -- would otherwise arrive as a fresh press on frame 0 and be
 *     consumed as a real Down/Left. That is how Samus ended up morphed and
 *     154px to the left of spawn with nobody touching the board.
 *
 *  2. B LATCH. The emitted direction is chosen on B's rising edge and held
 *     until B is released. Re-deciding it every frame would flip Down->Up the
 *     instant the morph landed, and MiniUpdateButtons() would see that flip as
 *     a new keypress, so a held B would morph and unmorph forever.
 */
uint16_t ExplorerButtons_Step(ExplorerButtonsState *state, unsigned pressed,
                              unsigned movement_type);

/*
 * Stateless legacy map: Y=Right X=Left B=Down A=Jump. No boot gate, no morph
 * toggle -- B is always Down. Kept for tests and for callers that only need the
 * pin-to-bit table; new code should use ExplorerButtons_Step().
 */
uint16_t ExplorerButtons_ToJoypad(unsigned pressed);

#ifdef __cplusplus
}
#endif

#endif /* SM_PICO_EXPLORER_BUTTONS_H_ */
