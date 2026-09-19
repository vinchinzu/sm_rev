#include "pico/stdlib.h"

#include <stdio.h>

#include "ida_types.h"
#include "mini/mini_game.h"
#include "variables.h"

enum {
  kPico2KernelFrames = 30,
  kPico2ViewportWidth = 256,
  kPico2ViewportHeight = 224,
};

int main(void) {
  MiniGameState *state;
  unsigned start_x;
  unsigned end_x;
  int i;

  stdio_init_all();
  /* UART0 is live immediately; USB CDC may enumerate a bit later. */
  sleep_ms(1500);

  state = MiniCreate(kPico2ViewportWidth, kPico2ViewportHeight);
  if (state == NULL) {
    printf("pico2: MiniCreate failed\n");
    while (true)
      tight_loop_contents();
  }

  start_x = samus_x_pos;
  printf("pico2 start samus_x_pos=%u samus_y_pos=%u pose=%u\n",
         start_x, (unsigned)samus_y_pos, (unsigned)samus_pose);

  for (i = 0; i < kPico2KernelFrames; i++)
    MiniStepButtons(state, kButton_Right, false);

  end_x = samus_x_pos;
  printf("pico2 samus_x_pos=%u samus_y_pos=%u pose=%u frames=%d moved=%u\n",
         end_x, (unsigned)samus_y_pos, (unsigned)samus_pose, kPico2KernelFrames,
         end_x != start_x);

  MiniDestroy(state);
  while (true)
    tight_loop_contents();
}
