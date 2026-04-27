#include "mini_backdrop.h"

#include <string.h>

bool MiniBackdropMode_Parse(const char *text, MiniBackdropMode *mode) {
  if (text == NULL || mode == NULL)
    return false;
  if (!strcmp(text, "game") || !strcmp(text, "default")) {
    *mode = kMiniBackdropMode_Game;
    return true;
  }
  if (!strcmp(text, "generated") || !strcmp(text, "ai") ||
      !strcmp(text, "ai-landing-site")) {
    *mode = kMiniBackdropMode_Generated;
    return true;
  }
  return false;
}

const char *MiniBackdropMode_Name(MiniBackdropMode mode) {
  switch (mode) {
  case kMiniBackdropMode_Generated:
    return "generated";
  case kMiniBackdropMode_Game:
  default:
    return "game";
  }
}
