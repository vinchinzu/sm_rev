#include <stdio.h>
#include <stdlib.h>

#include "ida_types.h"
#include "mini/mini_game.h"
#include "variables.h"

enum {
  kPicoKernelDefaultFrames = 30,
  kPicoKernelViewportWidth = 256,
  kPicoKernelViewportHeight = 224,
};

int main(int argc, char **argv) {
  int frames = kPicoKernelDefaultFrames;
  MiniGameState *state;
  uint16 start_x;
  uint16 end_x;
  int i;

  if (argc > 1) {
    frames = atoi(argv[1]);
    if (frames < 1)
      frames = kPicoKernelDefaultFrames;
  }

  state = MiniCreate(kPicoKernelViewportWidth, kPicoKernelViewportHeight);
  if (state == NULL) {
    fprintf(stderr, "pico-kernel: MiniCreate failed\n");
    return 1;
  }

  start_x = samus_x_pos;
  printf("pico-kernel start samus_x_pos=%u samus_y_pos=%u pose=%u hash=0x%llx\n",
         (unsigned)samus_x_pos, (unsigned)samus_y_pos, (unsigned)samus_pose,
         (unsigned long long)MiniStateHash(state));

  for (i = 0; i < frames; i++)
    MiniStepButtons(state, kButton_Right, false);

  end_x = samus_x_pos;
  printf("pico-kernel end frames=%d samus_x_pos=%u samus_y_pos=%u pose=%u hash=0x%llx\n",
         frames, (unsigned)samus_x_pos, (unsigned)samus_y_pos, (unsigned)samus_pose,
         (unsigned long long)MiniStateHash(state));

  MiniDestroy(state);
  return end_x != start_x ? 0 : 2;
}
