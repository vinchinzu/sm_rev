#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "features.h"
#include "mini_runtime.h"

#if CURRENT_BUILD != BUILD_MINI
#error "mini_main.c must be compiled with CURRENT_BUILD=BUILD_MINI"
#endif

static void PrintUsage(const char *argv0) {
  fprintf(stderr,
          "Usage: %s [--headless] [--frames N] [--screenshot PATH] [--input-script PATH] [--room-export PATH]\n"
          "  --headless   Run the mini shell without SDL video.\n"
          "  --frames N   Limit the run to N frames. Windowed mode runs until quit by default.\n"
          "  --screenshot PATH  Save the last rendered frame to a BMP file.\n"
          "  --input-script PATH  Replay one line of input tokens per frame in headless or windowed mode.\n"
          "  --room-export PATH  Load room collision data from a Super Metroid Editor export JSON file.\n",
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
  options->frames = kMiniDefaultFrames;
  options->frames_explicit = false;
  options->screenshot_path = NULL;
  options->input_script_path = NULL;
  options->room_export_path = NULL;
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "--headless")) {
      options->headless = true;
    } else if (!strcmp(argv[i], "--frames")) {
      if (i + 1 >= argc || !ParseInt(argv[i + 1], &options->frames))
        return false;
      options->frames_explicit = true;
      i++;
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
    } else if (!strcmp(argv[i], "--room-export")) {
      if (i + 1 >= argc || argv[i + 1][0] == '\0')
        return false;
      options->room_export_path = argv[i + 1];
      i++;
    } else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
      PrintUsage(argv[0]);
      exit(0);
    } else {
      return false;
    }
  }
  return true;
}

int main(int argc, char **argv) {
  MiniOptions options;
  if (!ParseArgs(argc, argv, &options)) {
    PrintUsage(argv[0]);
    return 2;
  }

  return MiniRun(&options);
}
