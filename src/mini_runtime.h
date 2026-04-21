#ifndef SM_MINI_RUNTIME_H_
#define SM_MINI_RUNTIME_H_

#include "types.h"

enum {
  kMiniDefaultFrames = 180,
  kMiniGameWidth = 256,
  kMiniGameHeight = 224,
  kMiniWindowWidth = 960,
  kMiniWindowHeight = 540,
  kMiniFrameDelayMs = 16,
};

typedef struct MiniOptions {
  bool headless;
  int frames;
  bool frames_explicit;
  const char *screenshot_path;
  const char *input_script_path;
  const char *room_export_path;
} MiniOptions;

int MiniRun(const MiniOptions *options);

#endif  // SM_MINI_RUNTIME_H_
