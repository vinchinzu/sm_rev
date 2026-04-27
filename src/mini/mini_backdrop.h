#ifndef SM_MINI_BACKDROP_H_
#define SM_MINI_BACKDROP_H_

#include "types.h"

typedef enum MiniBackdropMode {
  kMiniBackdropMode_Game = 0,
  kMiniBackdropMode_Generated = 1,
} MiniBackdropMode;

bool MiniBackdropMode_Parse(const char *text, MiniBackdropMode *mode);
const char *MiniBackdropMode_Name(MiniBackdropMode mode);

#endif  // SM_MINI_BACKDROP_H_
