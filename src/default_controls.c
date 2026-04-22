#include "default_controls.h"

static const uint16 kDefaultControlButtonMasks[kSmDefaultControl_Count] = {
#define SM_DECLARE_MASK(name, button_mask, scancode, gamepad_button) button_mask,
  SM_DEFAULT_CONTROLS(SM_DECLARE_MASK)
#undef SM_DECLARE_MASK
};

static const SDL_Scancode kDefaultControlKeyboardScancodes[kSmDefaultControl_Count] = {
#define SM_DECLARE_SCANCODE(name, button_mask, scancode, gamepad_button) scancode,
  SM_DEFAULT_CONTROLS(SM_DECLARE_SCANCODE)
#undef SM_DECLARE_SCANCODE
};

static const uint8 kDefaultControlGamepadButtons[kSmDefaultControl_Count] = {
#define SM_DECLARE_GAMEPAD(name, button_mask, scancode, gamepad_button) gamepad_button,
  SM_DEFAULT_CONTROLS(SM_DECLARE_GAMEPAD)
#undef SM_DECLARE_GAMEPAD
};

uint16 SmDefaultControlButtonMask(SmDefaultControl control) {
  return kDefaultControlButtonMasks[control];
}

SDL_Scancode SmDefaultControlKeyboardScancode(SmDefaultControl control) {
  return kDefaultControlKeyboardScancodes[control];
}

int SmDefaultControlGamepadButton(SmDefaultControl control) {
  return kDefaultControlGamepadButtons[control];
}

uint16 SmDefaultButtonsForKeyboardState(const Uint8 *keys) {
  uint16 buttons = 0;
  for (int i = 0; i < kSmDefaultControl_Count; i++) {
    if (keys[kDefaultControlKeyboardScancodes[i]])
      buttons |= kDefaultControlButtonMasks[i];
  }
  return buttons;
}

uint16 SmDefaultButtonsForGamepadButton(int gamepad_button) {
  for (int i = 0; i < kSmDefaultControl_Count; i++) {
    if (kDefaultControlGamepadButtons[i] == gamepad_button)
      return kDefaultControlButtonMasks[i];
  }
  return 0;
}

int SmGamepadButtonFromSdlButton(int sdl_button) {
  switch (sdl_button) {
  case SDL_CONTROLLER_BUTTON_A: return kGamepadBtn_A;
  case SDL_CONTROLLER_BUTTON_B: return kGamepadBtn_B;
  case SDL_CONTROLLER_BUTTON_X: return kGamepadBtn_X;
  case SDL_CONTROLLER_BUTTON_Y: return kGamepadBtn_Y;
  case SDL_CONTROLLER_BUTTON_BACK: return kGamepadBtn_Back;
  case SDL_CONTROLLER_BUTTON_GUIDE: return kGamepadBtn_Guide;
  case SDL_CONTROLLER_BUTTON_START: return kGamepadBtn_Start;
  case SDL_CONTROLLER_BUTTON_LEFTSTICK: return kGamepadBtn_L3;
  case SDL_CONTROLLER_BUTTON_RIGHTSTICK: return kGamepadBtn_R3;
  case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return kGamepadBtn_L1;
  case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return kGamepadBtn_R1;
  case SDL_CONTROLLER_BUTTON_DPAD_UP: return kGamepadBtn_DpadUp;
  case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return kGamepadBtn_DpadDown;
  case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return kGamepadBtn_DpadLeft;
  case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return kGamepadBtn_DpadRight;
  default: return kGamepadBtn_Invalid;
  }
}

uint16 SmDefaultButtonsForControllerState(SDL_GameController *controller) {
  uint16 buttons = 0;
  for (int sdl_button = SDL_CONTROLLER_BUTTON_A; sdl_button < SDL_CONTROLLER_BUTTON_MAX; sdl_button++) {
    if (!SDL_GameControllerGetButton(controller, (SDL_GameControllerButton)sdl_button))
      continue;
    int gamepad_button = SmGamepadButtonFromSdlButton(sdl_button);
    if (gamepad_button != kGamepadBtn_Invalid)
      buttons |= SmDefaultButtonsForGamepadButton(gamepad_button);
  }
  return buttons;
}
