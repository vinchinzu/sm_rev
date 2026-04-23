#include "mini_runtime.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

#include <SDL.h>

#include "default_controls.h"
#include "features.h"
#include "ida_types.h"
#include "mini_game.h"
#include "mini_input_script.h"
#include "mini_record.h"
#include "mini_renderer.h"
#include "stubs_mini.h"

#if CURRENT_BUILD != BUILD_MINI
#error "mini_runtime.c must be compiled with CURRENT_BUILD=BUILD_MINI"
#endif

static void PrintResult(const MiniOptions *options, const MiniGameState *state,
                        const char *record_path) {
  uint64_t state_hash = MiniGameState_ComputeHash(state);
  printf("{\"build\":\"mini\",\"headless\":%s,\"frames\":%d,"
         "\"no_enemies\":true,\"no_bosses\":true,\"no_rooms\":%s,"
         "\"room_ptr\":%u,\"room_width\":%d,\"room_height\":%d,"
         "\"room_source\":\"%s\",\"room_visuals\":\"%s\",\"room_handle\":\"%s\","
         "\"samus_suit\":\"%s\",\"recording\":%s,\"record_path\":\"%s\","
         "\"rom_room\":%s,"
         "\"samus_x\":%d,\"samus_y\":%d,\"samus_pose\":%u,\"samus_movement_type\":%u,"
         "\"state_hash\":\"0x%016llx\"}\n",
         options->headless ? "true" : "false", state->frame,
         state->has_room ? "false" : "true",
         state->room_id,
         state->room_width_blocks * kMiniBlockSize,
         state->room_height_blocks * kMiniBlockSize,
         MiniStubs_RoomSourceName(state->room_source),
         state->uses_rom_room ? "rom" : (state->has_editor_room_visuals ? "editor_tileset" : "placeholder"),
         state->room_handle,
         MiniStubs_SamusSuitName(state->samus_suit),
         options->record ? "true" : "false",
         record_path != NULL ? record_path : "",
         state->uses_rom_room ? "true" : "false",
         state->samus_x, state->samus_y, state->samus_pose_value, state->samus_movement_type_value,
         (unsigned long long)state_hash);
}

static void MiniRenderPresent(SDL_Renderer *renderer, SDL_Texture *frame_texture,
                              const uint32_t *pixels) {
  void *pixels_void = NULL;
  int pitch_bytes = 0;
  if (SDL_LockTexture(frame_texture, NULL, &pixels_void, &pitch_bytes) != 0)
    return;

  uint32_t *dst = (uint32_t *)pixels_void;
  int pitch_pixels = pitch_bytes / (int)sizeof(uint32_t);
  for (int y = 0; y < kMiniGameHeight; y++)
    memcpy(dst + y * pitch_pixels, pixels + y * kMiniGameWidth, kMiniGameWidth * sizeof(uint32_t));
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
  input->buttons |= SmDefaultButtonsForKeyboardState(keys);

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
  input->buttons |= SmDefaultButtonsForControllerState(controller);
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
                      SDL_Texture *frame_texture, SDL_GameController *controller,
                      MiniRecorder *recorder) {
  MiniInputScript script = {0};
  uint32_t frame_pixels[kMiniGameWidth * kMiniGameHeight];
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

    if (renderer != NULL || (recorder != NULL && recorder->pipe != NULL && !recorder->closed_early)) {
      MiniRenderFrameToPixels(frame_pixels, kMiniGameWidth, state);
      if (recorder != NULL && recorder->pipe != NULL && !recorder->closed_early)
        MiniRecord_WriteFrame(recorder, frame_pixels);
    }

    if (renderer != NULL && frame_texture != NULL) {
      MiniRenderPresent(renderer, frame_texture, frame_pixels);
      SDL_Delay(kMiniFrameDelayMs);
    }
  }

  MiniInputScript_Clear(&script);
}

static int RunHeadless(const MiniOptions *options) {
  MiniGameState state;
  MiniRecorder recorder = {0};
  MiniStubs_SetRoomExportPath(options->room_export_path);
  MiniGameState_Init(&state, kMiniGameWidth, kMiniGameHeight);
  if (options->record && !MiniRecord_Start(&recorder))
    return 1;
  RunFrames(&state, options, NULL, NULL, NULL, &recorder);
  if (!MiniRecord_Finish(&recorder))
    return 1;
  if (options->screenshot_path != NULL && !MiniSaveScreenshot(options->screenshot_path, &state))
    return 1;
  PrintResult(options, &state, options->record ? recorder.output_path : NULL);
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
  MiniRecorder recorder = {0};
  MiniStubs_SetRoomExportPath(options->room_export_path);
  MiniGameState_Init(&state, kMiniGameWidth, kMiniGameHeight);
  if (options->record && !MiniRecord_Start(&recorder)) {
    if (controller != NULL)
      SDL_GameControllerClose(controller);
    SDL_DestroyTexture(frame_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }
  RunFrames(&state, options, renderer, frame_texture, controller, &recorder);
  bool screenshot_ok = options->screenshot_path == NULL ||
                       MiniSaveScreenshot(options->screenshot_path, &state);
  bool recording_ok = MiniRecord_Finish(&recorder);

  if (controller != NULL)
    SDL_GameControllerClose(controller);
  SDL_DestroyTexture(frame_texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  if (!screenshot_ok || !recording_ok)
    return 1;

  PrintResult(options, &state, options->record ? recorder.output_path : NULL);
  return 0;
}

int MiniRun(const MiniOptions *options) {
  return options->headless ? RunHeadless(options) : RunWindowed(options);
}
