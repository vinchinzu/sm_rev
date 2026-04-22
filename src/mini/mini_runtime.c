#include "mini_runtime.h"

#include <limits.h>
#include <stdio.h>

#include <SDL.h>

#include "features.h"
#include "ida_types.h"
#include "mini_game.h"
#include "mini_input_script.h"
#include "mini_renderer.h"
#include "stubs_mini.h"

#if CURRENT_BUILD != BUILD_MINI
#error "mini_runtime.c must be compiled with CURRENT_BUILD=BUILD_MINI"
#endif

static void PrintResult(const MiniOptions *options, const MiniGameState *state) {
  printf("{\"build\":\"mini\",\"headless\":%s,\"frames\":%d,"
         "\"no_enemies\":true,\"no_bosses\":true,\"no_rooms\":%s,"
         "\"room_ptr\":%u,\"room_width\":%d,\"room_height\":%d,"
         "\"room_source\":\"%s\",\"room_visuals\":\"%s\",\"room_handle\":\"%s\","
         "\"samus_suit\":\"%s\","
         "\"rom_room\":%s,"
         "\"samus_x\":%d,\"samus_y\":%d}\n",
         options->headless ? "true" : "false", state->frame,
         state->has_room ? "false" : "true",
         state->room_id,
         state->room_width_blocks * kMiniBlockSize,
         state->room_height_blocks * kMiniBlockSize,
         MiniStubs_RoomSourceName(state->room_source),
         state->uses_rom_room ? "rom" : (state->has_editor_room_visuals ? "editor_tileset" : "placeholder"),
         state->room_handle,
         MiniStubs_SamusSuitName(state->samus_suit),
         state->uses_rom_room ? "true" : "false",
         state->samus_x, state->samus_y);
}

static void MiniRenderPresent(SDL_Renderer *renderer, SDL_Texture *frame_texture,
                              const MiniGameState *state) {
  void *pixels_void = NULL;
  int pitch_bytes = 0;
  if (SDL_LockTexture(frame_texture, NULL, &pixels_void, &pitch_bytes) != 0)
    return;

  MiniRenderFrameToPixels((uint32_t *)pixels_void, pitch_bytes / (int)sizeof(uint32_t), state);
  SDL_UnlockTexture(frame_texture);

  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, frame_texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

static void MiniPollWindowEvents(MiniInputState *input) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT)
      input->quit_requested = true;
  }
}

static void MiniPollLiveButtons(MiniInputState *input, SDL_GameController *controller) {
  SDL_PumpEvents();
  const Uint8 *keys = SDL_GetKeyboardState(NULL);
  if (keys[SDL_SCANCODE_UP])
    input->buttons |= kButton_Up;
  if (keys[SDL_SCANCODE_DOWN])
    input->buttons |= kButton_Down;
  if (keys[SDL_SCANCODE_LEFT])
    input->buttons |= kButton_Left;
  if (keys[SDL_SCANCODE_RIGHT])
    input->buttons |= kButton_Right;
  if (keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_Z])
    input->buttons |= kButton_A;
  if (keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT] || keys[SDL_SCANCODE_C])
    input->buttons |= kButton_B;
  if (keys[SDL_SCANCODE_X])
    input->buttons |= kButton_X;
  if (keys[SDL_SCANCODE_A])
    input->buttons |= kButton_Y;
  if (keys[SDL_SCANCODE_S])
    input->buttons |= kButton_L;
  if (keys[SDL_SCANCODE_D])
    input->buttons |= kButton_R;

  if (controller == NULL || !SDL_GameControllerGetAttached(controller))
    return;

  Sint16 axis_x = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
  Sint16 axis_y = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
  if (axis_y <= -16000 || SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP))
    input->buttons |= kButton_Up;
  if (axis_y >= 16000 || SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN))
    input->buttons |= kButton_Down;
  if (axis_x <= -16000 || SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT))
    input->buttons |= kButton_Left;
  if (axis_x >= 16000 || SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
    input->buttons |= kButton_Right;
  if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A))
    input->buttons |= kButton_A;
  if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B))
    input->buttons |= kButton_B;
  if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X))
    input->buttons |= kButton_X;
  if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_Y))
    input->buttons |= kButton_Y;
  if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER))
    input->buttons |= kButton_L;
  if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER))
    input->buttons |= kButton_R;
}

static SDL_GameController *MiniOpenFirstController(void) {
  for (int i = 0; i < SDL_NumJoysticks(); i++) {
    if (SDL_IsGameController(i)) {
      SDL_GameController *controller = SDL_GameControllerOpen(i);
      if (controller != NULL)
        return controller;
    }
  }
  return NULL;
}

static void RunFrames(MiniGameState *state, const MiniOptions *options, SDL_Renderer *renderer,
                      SDL_Texture *frame_texture, SDL_GameController *controller) {
  MiniInputScript script = {0};
  if (options->input_script_path != NULL && !MiniInputScript_Load(&script, options->input_script_path))
    return;

  int frame_limit = (options->headless || options->frames_explicit) ? options->frames : INT_MAX;
  for (int i = 0; i < frame_limit && !state->quit_requested; i++) {
    MiniInputState input = {0};
    MiniScriptFrame scripted = {0};
    MiniInputScript_ApplyFrame(&script, i, &scripted);
    input.buttons = scripted.buttons;
    input.quit_requested = scripted.quit_requested;

    if (renderer != NULL) {
      MiniPollWindowEvents(&input);
      if (options->input_script_path == NULL)
        MiniPollLiveButtons(&input, controller);
    }

    MiniUpdate(state, &input);

    if (renderer != NULL && frame_texture != NULL) {
      MiniRenderPresent(renderer, frame_texture, state);
      SDL_Delay(kMiniFrameDelayMs);
    }
  }

  MiniInputScript_Clear(&script);
}

static int RunHeadless(const MiniOptions *options) {
  MiniGameState state;
  MiniStubs_SetRoomExportPath(options->room_export_path);
  MiniGameState_Init(&state, kMiniGameWidth, kMiniGameHeight);
  RunFrames(&state, options, NULL, NULL, NULL);
  if (options->screenshot_path != NULL && !MiniSaveScreenshot(options->screenshot_path, &state))
    return 1;
  PrintResult(options, &state);
  return 0;
}

static int RunWindowed(const MiniOptions *options) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
    fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Window *window = SDL_CreateWindow("sm_rev mini shell",
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        kMiniWindowWidth,
                                        kMiniWindowHeight,
                                        SDL_WINDOW_SHOWN);
  if (window == NULL) {
    fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == NULL) {
    fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }
  SDL_RenderSetLogicalSize(renderer, kMiniGameWidth, kMiniGameHeight);

  SDL_Texture *frame_texture = SDL_CreateTexture(renderer,
                                                 SDL_PIXELFORMAT_ARGB8888,
                                                 SDL_TEXTUREACCESS_STREAMING,
                                                 kMiniGameWidth,
                                                 kMiniGameHeight);
  if (frame_texture == NULL) {
    fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  SDL_GameController *controller = MiniOpenFirstController();

  MiniGameState state;
  MiniStubs_SetRoomExportPath(options->room_export_path);
  MiniGameState_Init(&state, kMiniGameWidth, kMiniGameHeight);
  RunFrames(&state, options, renderer, frame_texture, controller);
  bool screenshot_ok = options->screenshot_path == NULL ||
                       MiniSaveScreenshot(options->screenshot_path, &state);

  if (controller != NULL)
    SDL_GameControllerClose(controller);
  SDL_DestroyTexture(frame_texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  if (!screenshot_ok)
    return 1;

  PrintResult(options, &state);
  return 0;
}

int MiniRun(const MiniOptions *options) {
  return options->headless ? RunHeadless(options) : RunWindowed(options);
}
