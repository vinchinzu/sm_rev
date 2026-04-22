#ifndef SM_MINI_RECORD_H_
#define SM_MINI_RECORD_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

typedef struct MiniRecorder {
  FILE *pipe;
  char output_path[PATH_MAX];
  int frames_written;
  bool closed_early;
} MiniRecorder;

bool MiniRecord_Start(MiniRecorder *recorder);
bool MiniRecord_WriteFrame(MiniRecorder *recorder, const uint32_t *pixels);
bool MiniRecord_Finish(MiniRecorder *recorder);

#endif  // SM_MINI_RECORD_H_
