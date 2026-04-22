#ifndef SM_MINI_RUNTIME_H_
#define SM_MINI_RUNTIME_H_

#include "mini_defs.h"
#include "types.h"

typedef struct MiniOptions {
  bool headless;
  bool record;
  int frames;
  bool frames_explicit;
  const char *screenshot_path;
  const char *input_script_path;
  const char *room_export_path;
} MiniOptions;

int MiniRun(const MiniOptions *options);

#endif  // SM_MINI_RUNTIME_H_
