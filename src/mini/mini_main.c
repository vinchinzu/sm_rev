#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "features.h"
#include "mini_editor_bridge.h"
#include "mini_runtime.h"

#if !BUILD_IS_MINI && !BUILD_IS_MODDABLE
#error "mini_main.c must be compiled with CURRENT_BUILD=BUILD_MINI or BUILD_MODDABLE"
#endif

static void PrintUsage(const char *argv0) {
  fprintf(stderr,
          "Usage: %s [--headless] [--record] [--frames N] [--players N] [--multiplayer-demo] [--screenshot PATH] [--input-script PATH] [--replay-in PATH] [--replay-out PATH] [--room-export PATH] [--start HANDLE] [--trace-wram PATH] [--background MODE] [--ai-background]\n"
          "  --headless   Run the mini runtime without SDL video.\n"
          "  --record   Save a low-resolution quick clip under out/mini_recording_YYYYMMDD_HHMMSS.mp4.\n"
          "  --frames N   Limit the run to N frames. Windowed mode runs until quit by default.\n"
          "  --players N   Run the mini kernel with 1 or 2 local players.\n"
          "  --multiplayer-demo  Run two local players with built-in scripted exchange-fire input.\n"
          "  --screenshot PATH  Save the last rendered frame to a BMP file.\n"
          "  --input-script PATH  Replay one line of input tokens per frame in headless or windowed mode.\n"
          "  --replay-in PATH  Replay a mini artifact and verify its final state hash.\n"
          "  --replay-out PATH  Write a mini replay artifact for the completed run.\n"
          "  --room-export PATH  Load room collision data from a Super Metroid Editor export JSON file.\n"
          "  --start HANDLE  Boot a scoped ROM start: ceres (Ceres Elevator) or landing-site.\n"
          "  --trace-wram PATH  Write per-frame WRAM $0AF6/$0AFA/$0AF8/$0AFC JSONL.\n"
          "  --background MODE  Select mini backdrop mode: game or generated.\n"
          "  --ai-background  Alias for --background generated.\n",
          argv0);
}

static bool ParseInt(const char *text, int *value) {
  char *end = NULL;
  long parsed = strtol(text, &end, 10);
  if (text[0] == '\0' || end == NULL || *end != '\0' || parsed <= 0 || parsed > 1000000)
    return false;
  *value = (int)parsed;
  return true;
}

static bool ParseArgs(int argc, char **argv, MiniOptions *options) {
  options->headless = false;
  options->record = false;
  options->frames = kMiniDefaultFrames;
  options->frames_explicit = false;
  options->player_count = 1;
  options->multiplayer_demo = false;
  options->screenshot_path = NULL;
  options->input_script_path = NULL;
  options->replay_in_path = NULL;
  options->replay_out_path = NULL;
  options->room_export_path = NULL;
  options->start_handle = NULL;
  options->trace_wram_path = NULL;
  options->backdrop_mode = kMiniBackdropMode_Game;
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "--headless")) {
      options->headless = true;
    } else if (!strcmp(argv[i], "--record")) {
      options->record = true;
    } else if (!strcmp(argv[i], "--frames")) {
      if (i + 1 >= argc || !ParseInt(argv[i + 1], &options->frames))
        return false;
      options->frames_explicit = true;
      i++;
    } else if (!strcmp(argv[i], "--players")) {
      if (i + 1 >= argc || !ParseInt(argv[i + 1], &options->player_count) ||
          options->player_count < 1 || options->player_count > kMiniMaxPlayers)
        return false;
      i++;
    } else if (!strcmp(argv[i], "--multiplayer-demo")) {
      options->multiplayer_demo = true;
      options->player_count = kMiniMaxPlayers;
    } else if (!strcmp(argv[i], "--screenshot")) {
      if (i + 1 >= argc || argv[i + 1][0] == '\0')
        return false;
      options->screenshot_path = argv[i + 1];
      i++;
    } else if (!strcmp(argv[i], "--input-script")) {
      if (i + 1 >= argc || argv[i + 1][0] == '\0')
        return false;
      options->input_script_path = argv[i + 1];
      i++;
    } else if (!strcmp(argv[i], "--replay-in")) {
      if (i + 1 >= argc || argv[i + 1][0] == '\0')
        return false;
      options->replay_in_path = argv[i + 1];
      i++;
    } else if (!strcmp(argv[i], "--replay-out")) {
      if (i + 1 >= argc || argv[i + 1][0] == '\0')
        return false;
      options->replay_out_path = argv[i + 1];
      i++;
    } else if (!strcmp(argv[i], "--room-export")) {
      if (i + 1 >= argc || argv[i + 1][0] == '\0')
        return false;
      options->room_export_path = argv[i + 1];
      i++;
    } else if (!strcmp(argv[i], "--start")) {
      if (i + 1 >= argc || argv[i + 1][0] == '\0')
        return false;
      if (strcmp(argv[i + 1], "ceres") != 0 && strcmp(argv[i + 1], "landing-site") != 0)
        return false;
      options->start_handle = argv[i + 1];
      i++;
    } else if (!strcmp(argv[i], "--trace-wram")) {
      if (i + 1 >= argc || argv[i + 1][0] == '\0')
        return false;
      options->trace_wram_path = argv[i + 1];
      i++;
    } else if (!strcmp(argv[i], "--background")) {
      if (i + 1 >= argc || !MiniBackdropMode_Parse(argv[i + 1], &options->backdrop_mode))
        return false;
      i++;
    } else if (!strcmp(argv[i], "--ai-background")) {
      options->backdrop_mode = kMiniBackdropMode_Generated;
    } else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
      PrintUsage(argv[0]);
      exit(0);
    } else {
      return false;
    }
  }
  if (options->replay_in_path != NULL && options->input_script_path != NULL)
    return false;
  if (options->replay_in_path != NULL && options->frames_explicit)
    return false;
  if (options->multiplayer_demo &&
      (options->input_script_path != NULL || options->replay_in_path != NULL))
    return false;
  return true;
}

int main(int argc, char **argv) {
  MiniOptions options;
  MiniEditorBridge_SetBasePath(argv[0]);
  if (!ParseArgs(argc, argv, &options)) {
    PrintUsage(argv[0]);
    return 2;
  }

  return MiniRun(&options);
}
