#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "features.h"

#if CURRENT_BUILD != BUILD_MINI
#error "mini_main.c must be compiled with CURRENT_BUILD=BUILD_MINI"
#endif

enum {
  kMiniDefaultFrames = 180,
  kMiniWindowWidth = 960,
  kMiniWindowHeight = 540,
  kMiniFrameDelayMs = 16,
};

typedef struct MiniOptions {
  bool headless;
  int frames;
} MiniOptions;

static void PrintUsage(const char *argv0) {
  fprintf(stderr,
          "Usage: %s [--headless] [--frames N]\n"
          "  --headless   Run the mini shell without SDL video.\n"
          "  --frames N   Limit the shell to N frames (default: %d).\n",
          argv0, kMiniDefaultFrames);
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
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "--headless")) {
      options->headless = true;
    } else if (!strcmp(argv[i], "--frames")) {
      if (i + 1 >= argc || !ParseInt(argv[i + 1], &options->frames))
        return false;
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

static void PrintResult(const MiniOptions *options) {
  printf("{\"build\":\"mini\",\"headless\":%s,\"frames\":%d,"
         "\"no_enemies\":true,\"no_bosses\":true,\"no_rooms\":true}\n",
         options->headless ? "true" : "false", options->frames);
}

static int RunHeadless(const MiniOptions *options) {
  for (int frame = 0; frame < options->frames; frame++) {
    (void)frame;
  }
  PrintResult(options);
  return 0;
}

static int RunWindowed(const MiniOptions *options) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
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

  bool quit = false;
  for (int frame = 0; frame < options->frames && !quit; frame++) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT)
        quit = true;
    }

    SDL_SetRenderDrawColor(renderer, 12, 16, 24, 255);
    SDL_RenderClear(renderer);

    SDL_Rect marker = {
      .x = 48 + ((frame * 5) % (kMiniWindowWidth - 144)),
      .y = kMiniWindowHeight / 2 - 24,
      .w = 96,
      .h = 48,
    };
    SDL_SetRenderDrawColor(renderer, 107, 196, 255, 255);
    SDL_RenderFillRect(renderer, &marker);
    SDL_RenderPresent(renderer);
    SDL_Delay(kMiniFrameDelayMs);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  PrintResult(options);
  return 0;
}

int main(int argc, char **argv) {
  MiniOptions options;
  if (!ParseArgs(argc, argv, &options)) {
    PrintUsage(argv[0]);
    return 2;
  }

  return options.headless ? RunHeadless(&options) : RunWindowed(&options);
}
