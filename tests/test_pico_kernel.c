#include <stdio.h>
#include <stdlib.h>

#include "ida_types.h"
#include "mini/mini_game.h"
#include "variables.h"

enum {
  kPicoKernelTestFrames = 30,
  kPicoKernelViewportWidth = 256,
  kPicoKernelViewportHeight = 224,
};

int main(void) {
  MiniGameState *state = MiniCreate(kPicoKernelViewportWidth, kPicoKernelViewportHeight);
  uint16 start_x;
  uint16 end_x;
  int i;

  if (state == NULL) {
    fprintf(stderr, "test_pico_kernel: MiniCreate failed\n");
    return 1;
  }

  start_x = samus_x_pos;
  printf("samus_x_pos=%u samus_y_pos=%u pose=%u\n",
         (unsigned)start_x, (unsigned)samus_y_pos, (unsigned)samus_pose);

  for (i = 0; i < kPicoKernelTestFrames; i++)
    MiniStepButtons(state, kButton_Right, false);

  end_x = samus_x_pos;
  printf("samus_x_pos=%u samus_y_pos=%u pose=%u hash=0x%llx frames=%d\n",
         (unsigned)end_x, (unsigned)samus_y_pos, (unsigned)samus_pose,
         (unsigned long long)MiniStateHash(state), kPicoKernelTestFrames);

  MiniDestroy(state);
  if (end_x == start_x) {
    fprintf(stderr, "test_pico_kernel: samus_x_pos did not increase (%u)\n",
            (unsigned)start_x);
    return 2;
  }
  return 0;
}
