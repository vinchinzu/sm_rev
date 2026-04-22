#pragma once

#include <SDL.h>

#include "config.h"
#include "ida_types.h"
#include "types.h"

// Canonical default control layout shared by the full build config defaults
// and the mini runtime input shim.
#define SM_DEFAULT_CONTROLS(X) \
  X(Up, kButton_Up, SDL_SCANCODE_UP, kGamepadBtn_DpadUp) \
  X(Down, kButton_Down, SDL_SCANCODE_DOWN, kGamepadBtn_DpadDown) \
  X(Left, kButton_Left, SDL_SCANCODE_LEFT, kGamepadBtn_DpadLeft) \
  X(Right, kButton_Right, SDL_SCANCODE_RIGHT, kGamepadBtn_DpadRight) \
  X(Select, kButton_Select, SDL_SCANCODE_RSHIFT, kGamepadBtn_Back) \
  X(Start, kButton_Start, SDL_SCANCODE_RETURN, kGamepadBtn_Start) \
  X(A, kButton_A, SDL_SCANCODE_X, kGamepadBtn_B) \
  X(B, kButton_B, SDL_SCANCODE_Z, kGamepadBtn_A) \
  X(X, kButton_X, SDL_SCANCODE_S, kGamepadBtn_Y) \
  X(Y, kButton_Y, SDL_SCANCODE_A, kGamepadBtn_X) \
  X(L, kButton_L, SDL_SCANCODE_C, kGamepadBtn_L1) \
  X(R, kButton_R, SDL_SCANCODE_V, kGamepadBtn_R1)

typedef enum SmDefaultControl {
#define SM_DECLARE_CONTROL(name, button_mask, scancode, gamepad_button) kSmDefaultControl_##name,
  SM_DEFAULT_CONTROLS(SM_DECLARE_CONTROL)
#undef SM_DECLARE_CONTROL
  kSmDefaultControl_Count,
} SmDefaultControl;

uint16 SmDefaultControlButtonMask(SmDefaultControl control);
SDL_Scancode SmDefaultControlKeyboardScancode(SmDefaultControl control);
int SmDefaultControlGamepadButton(SmDefaultControl control);

uint16 SmDefaultButtonsForKeyboardState(const Uint8 *keys);
uint16 SmDefaultButtonsForGamepadButton(int gamepad_button);
uint16 SmDefaultButtonsForControllerState(SDL_GameController *controller);
int SmGamepadButtonFromSdlButton(int sdl_button);
