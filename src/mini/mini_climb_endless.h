#ifndef SM_MINI_CLIMB_ENDLESS_H_
#define SM_MINI_CLIMB_ENDLESS_H_

#include "mini_game.h"
#include "types.h"

enum {
  kMiniClimbEndlessRoomId = 0x96BA,
};

int MiniClimbEndless_PirateShotCooldownFrames(const MiniGameState *state);

#endif  // SM_MINI_CLIMB_ENDLESS_H_
