#include "mini_runtime.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

#include <SDL.h>

#include "default_controls.h"
#include "features.h"
#include "ida_types.h"
#include "mini_content_scope.h"
#include "mini_game.h"
#include "mini_input_script.h"
#include "mini_record.h"
#include "mini_replay.h"
#include "mini_renderer.h"
#include "stubs_mini.h"

#if CURRENT_BUILD != BUILD_MINI
#error "mini_runtime.c must be compiled with CURRENT_BUILD=BUILD_MINI"
#endif

static void PrintResult(const MiniOptions *options, const MiniGameState *state,
                        const char *record_path, bool replay_verified) {
  uint64_t state_hash = MiniStateHash(state);
  const SamusProjectileView *first_projectile =
      state->projectile_count > 0 ? &state->projectiles[0] : NULL;
  printf("{\"build\":\"mini\",\"headless\":%s,\"frames\":%d,"
         "\"content_scope\":\"%s\","
         "\"no_enemies\":%s,\"no_bosses\":true,\"no_rooms\":%s,"
         "\"room_ptr\":%u,\"room_width\":%d,\"room_height\":%d,"
         "\"room_source\":\"%s\",\"room_visuals\":\"%s\",\"room_handle\":\"%s\","
         "\"background\":\"%s\","
         "\"original_runtime\":%s,\"original_enemies\":%s,\"original_plms\":%s,"
         "\"samus_suit\":\"%s\",\"recording\":%s,\"record_path\":\"%s\","
         "\"replay_in\":\"%s\",\"replay_out\":\"%s\",\"replay_verified\":%s,"
         "\"rom_room\":%s,"
         "\"samus_x\":%d,\"samus_y\":%d,\"samus_pose\":%u,\"samus_movement_type\":%u,"
         "\"projectile_count\":%d,\"first_projectile_type\":%u,"
         "\"first_projectile_x\":%u,\"first_projectile_y\":%u,"
         "\"first_projectile_dir\":%u,"
         "\"state_hash\":\"0x%016llx\"}\n",
         options->headless ? "true" : "false", state->frame,
         MiniContentScope_Name(),
         state->has_original_enemies ? "false" : "true",
         state->has_room ? "false" : "true",
         state->room_id,
         state->room_width_blocks * kMiniBlockSize,
         state->room_height_blocks * kMiniBlockSize,
         MiniStubs_RoomSourceName(state->room_source),
         state->uses_rom_room ? "rom" : (state->has_editor_room_visuals ? "editor_tileset" : "placeholder"),
         state->room_handle,
         MiniBackdropMode_Name(options->backdrop_mode),
         state->uses_original_gameplay_runtime ? "true" : "false",
         state->has_original_enemies ? "true" : "false",
         state->has_original_plms ? "true" : "false",
         MiniStubs_SamusSuitName(state->samus_suit),
         options->record ? "true" : "false",
         record_path != NULL ? record_path : "",
         options->replay_in_path != NULL ? options->replay_in_path : "",
         options->replay_out_path != NULL ? options->replay_out_path : "",
         replay_verified ? "true" : "false",
         state->uses_rom_room ? "true" : "false",
         state->samus_x, state->samus_y, state->samus_pose_value, state->samus_movement_type_value,
         state->projectile_count,
         first_projectile != NULL ? first_projectile->type : 0,
         first_projectile != NULL ? first_projectile->x_pos : 0,
         first_projectile != NULL ? first_projectile->y_pos : 0,
         first_projectile != NULL ? first_projectile->direction : 0,
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

static bool LoadReplayInput(const MiniOptions *options, MiniReplayArtifact *replay) {
  if (options->replay_in_path == NULL)
    return true;
  return MiniReplay_Load(replay, options->replay_in_path);
}

static const MiniReplayArtifact *ReplayInputOrNull(const MiniOptions *options,
                                                   const MiniReplayArtifact *replay) {
  return options->replay_in_path != NULL ? replay : NULL;
}

static const char *RoomExportPathForRun(const MiniOptions *options,
                                        const MiniReplayArtifact *replay) {
  if (options->room_export_path != NULL)
    return options->room_export_path;
  if (replay != NULL && replay->room_export_path[0] != '\0')
    return replay->room_export_path;
  return NULL;
}

static bool ValidateReplayInitialState(const MiniReplayArtifact *replay,
                                       const MiniGameState *state,
                                       uint64_t initial_hash) {
  if (replay == NULL)
    return true;
  if (replay->viewport_width != state->viewport_width ||
      replay->viewport_height != state->viewport_height) {
    fprintf(stderr,
            "mini: replay viewport mismatch (artifact=%dx%d runtime=%dx%d)\n",
            replay->viewport_width, replay->viewport_height,
            state->viewport_width, state->viewport_height);
    return false;
  }
  if (replay->initial_hash != initial_hash) {
    fprintf(stderr,
            "mini: replay initial hash mismatch (expected 0x%016llx, got 0x%016llx)\n",
            (unsigned long long)replay->initial_hash,
            (unsigned long long)initial_hash);
    return false;
  }
  return true;
}

static bool ValidateReplayFinalState(const MiniReplayArtifact *replay, uint64_t final_hash) {
  if (replay == NULL)
    return true;
  if (replay->final_hash != final_hash) {
    fprintf(stderr,
            "mini: replay final hash mismatch (expected 0x%016llx, got 0x%016llx)\n",
            (unsigned long long)replay->final_hash,
            (unsigned long long)final_hash);
    return false;
  }
  return true;
}

static bool WriteReplayOutput(const MiniOptions *options, const MiniGameState *state,
                              const MiniReplayFrames *frames,
                              uint64_t initial_hash, uint64_t final_hash,
                              const char *room_export_path) {
  if (options->replay_out_path == NULL)
    return true;
  MiniReplayWriteInfo info = {
    .frames = frames->count,
    .viewport_width = state->viewport_width,
    .viewport_height = state->viewport_height,
    .initial_hash = initial_hash,
    .final_hash = final_hash,
    .content_scope = MiniContentScope_Name(),
    .background = MiniBackdropMode_Name(options->backdrop_mode),
    .room_export_path = room_export_path,
    .final_state = state,
  };
  return MiniReplay_Write(options->replay_out_path, &info, frames);
}

static bool RunFrames(MiniGameState *state, const MiniOptions *options, SDL_Renderer *renderer,
                      SDL_Texture *frame_texture, SDL_GameController *controller,
                      MiniRecorder *recorder, const MiniReplayArtifact *replay,
                      MiniReplayFrames *replay_out_frames) {
  MiniInputScript script = {0};
  uint32_t frame_pixels[kMiniGameWidth * kMiniGameHeight];
  if (replay == NULL && options->input_script_path != NULL &&
      !MiniInputScript_Load(&script, options->input_script_path))
    return false;

  int frame_limit = replay != NULL
                        ? replay->frames
                        : ((options->headless || options->frames_explicit) ? options->frames : INT_MAX);
  bool ok = true;
  for (int i = 0; i < frame_limit && !state->quit_requested; i++) {
    MiniInputState input = {0};
    MiniScriptFrame scripted = {0};
    if (replay != NULL)
      scripted = replay->inputs.frames[i];
    else
      MiniInputScript_ApplyFrame(&script, i, &scripted);
    input.buttons = scripted.buttons;
    input.quit_requested = scripted.quit_requested;

    if (renderer != NULL) {
      MiniPollWindowEvents(&input);
      if (options->input_script_path == NULL && replay == NULL)
        MiniPollLiveButtons(&input, controller);
    }

    MiniScriptFrame applied = {
      .buttons = input.buttons,
      .quit_requested = input.quit_requested,
    };
    if (replay_out_frames != NULL && !MiniReplayFrames_Append(replay_out_frames, applied)) {
      ok = false;
      break;
    }

    MiniStep(state, &input);

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
  return ok;
}

static int RunHeadless(const MiniOptions *options) {
  MiniGameState state;
  MiniRecorder recorder = {0};
  MiniReplayArtifact replay = {0};
  MiniReplayFrames replay_out_frames = {0};
  const MiniReplayArtifact *replay_in = NULL;
  int result = 1;
  if (!LoadReplayInput(options, &replay))
    goto done;
  replay_in = ReplayInputOrNull(options, &replay);

  MiniStubs_SetRoomExportPath(RoomExportPathForRun(options, replay_in));
  MiniInit(&state, kMiniGameWidth, kMiniGameHeight);
  uint64_t initial_hash = MiniStateHash(&state);
  if (!ValidateReplayInitialState(replay_in, &state, initial_hash))
    goto done;
  if (options->record && !MiniRecord_Start(&recorder))
    goto done;
  if (!RunFrames(&state, options, NULL, NULL, NULL, &recorder, replay_in,
                 options->replay_out_path != NULL ? &replay_out_frames : NULL))
    goto done;
  if (!MiniRecord_Finish(&recorder))
    goto done;
  if (options->screenshot_path != NULL && !MiniSaveScreenshot(options->screenshot_path, &state))
    goto done;
  uint64_t final_hash = MiniStateHash(&state);
  if (!ValidateReplayFinalState(replay_in, final_hash))
    goto done;
  if (!WriteReplayOutput(options, &state, &replay_out_frames, initial_hash, final_hash,
                         RoomExportPathForRun(options, replay_in)))
    goto done;
  PrintResult(options, &state, options->record ? recorder.output_path : NULL, replay_in != NULL);
  result = 0;

done:
  MiniRecord_Finish(&recorder);
  MiniReplay_Clear(&replay);
  MiniReplayFrames_Clear(&replay_out_frames);
  return result;
}

static int RunWindowed(const MiniOptions *options) {
  MiniReplayArtifact replay = {0};
  MiniReplayFrames replay_out_frames = {0};
  const MiniReplayArtifact *replay_in = NULL;
  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;
  SDL_Texture *frame_texture = NULL;
  SDL_GameController *controller = NULL;
  MiniGameState state;
  MiniRecorder recorder = {0};
  uint64_t initial_hash = 0;
  uint64_t final_hash = 0;
  bool sdl_initialized = false;
  int result = 1;

  if (!LoadReplayInput(options, &replay))
    goto done;
  replay_in = ReplayInputOrNull(options, &replay);

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
    fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
    goto done;
  }
  sdl_initialized = true;

  window = SDL_CreateWindow("sm_rev mini",
                            SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED,
                            kMiniWindowWidth,
                            kMiniWindowHeight,
                            SDL_WINDOW_SHOWN);
  if (window == NULL) {
    fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
    goto done;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == NULL) {
    fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
    goto done;
  }
  SDL_RenderSetLogicalSize(renderer, kMiniGameWidth, kMiniGameHeight);

  frame_texture = SDL_CreateTexture(renderer,
                                    SDL_PIXELFORMAT_ARGB8888,
                                    SDL_TEXTUREACCESS_STREAMING,
                                    kMiniGameWidth,
                                    kMiniGameHeight);
  if (frame_texture == NULL) {
    fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
    goto done;
  }

  controller = MiniOpenFirstController();

  MiniStubs_SetRoomExportPath(RoomExportPathForRun(options, replay_in));
  MiniInit(&state, kMiniGameWidth, kMiniGameHeight);
  initial_hash = MiniStateHash(&state);
  if (!ValidateReplayInitialState(replay_in, &state, initial_hash))
    goto done;
  if (options->record && !MiniRecord_Start(&recorder))
    goto done;
  if (!RunFrames(&state, options, renderer, frame_texture, controller, &recorder, replay_in,
                 options->replay_out_path != NULL ? &replay_out_frames : NULL))
    goto done;
  if (options->screenshot_path != NULL && !MiniSaveScreenshot(options->screenshot_path, &state))
    goto done;
  if (!MiniRecord_Finish(&recorder))
    goto done;
  final_hash = MiniStateHash(&state);
  if (!ValidateReplayFinalState(replay_in, final_hash))
    goto done;
  if (!WriteReplayOutput(options, &state, &replay_out_frames, initial_hash, final_hash,
                         RoomExportPathForRun(options, replay_in)))
    goto done;

  PrintResult(options, &state, options->record ? recorder.output_path : NULL, replay_in != NULL);
  result = 0;

done:
  MiniRecord_Finish(&recorder);
  if (controller != NULL)
    SDL_GameControllerClose(controller);
  if (frame_texture != NULL)
    SDL_DestroyTexture(frame_texture);
  if (renderer != NULL)
    SDL_DestroyRenderer(renderer);
  if (window != NULL)
    SDL_DestroyWindow(window);
  if (sdl_initialized)
    SDL_Quit();
  MiniReplay_Clear(&replay);
  MiniReplayFrames_Clear(&replay_out_frames);
  return result;
}

int MiniRun(const MiniOptions *options) {
  MiniRenderer_SetBackdropMode(options->backdrop_mode);
  return options->headless ? RunHeadless(options) : RunWindowed(options);
}
