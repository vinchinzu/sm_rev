/*
 * Host debug compositor: 8x8 collision tiles + Samus rect + beams.
 *
 *   gcc -O2 -Werror -I. -iquote src -iquote src/mini -Isrc/pico -DCURRENT_BUILD=BUILD_PICO \
 *       tests/test_pico_debug_tiles.c src/pico/pico_debug_tiles.c \
 *       -L. -lsm_rev_pico_kernel -lm -Wl,--gc-sections -o sm_rev_pico_debug_tiles_test
 *   ./sm_rev_pico_debug_tiles_test
 *
 * Do not pass -Isrc: it shadows glibc's <features.h> with src/features.h.
 * Dump buffer is allocated here; pico-kernel BSS has no 112 KB framebuffer.
 */

#include <stdio.h>
#include <stdlib.h>

#include "ida_types.h"
#include "mini/mini_game.h"
#include "pico_debug_tiles.h"
#include "variables.h"

enum {
  kPicoDebugTestFrames = 30,
  kPicoDebugTestWidth = kPicoDebugWidth,
  kPicoDebugTestHeight = kPicoDebugHeight
};

static int g_failures;

static void expect_true(const char *name, int cond) {
  if (!cond) {
    fprintf(stderr, "FAIL %s\n", name);
    g_failures++;
  }
}

static uint16_t pixel_at(const uint16_t *rgb565, int width, int height, int x, int y) {
  if (rgb565 == NULL || (unsigned)x >= (unsigned)width || (unsigned)y >= (unsigned)height)
    return 0;
  return rgb565[(size_t)y * (size_t)width + (size_t)x];
}

int main(void) {
  MiniGameState *state;
  uint16_t *dump;
  uint16 start_x;
  uint16 end_x;
  int i;
  int screen_x;
  int screen_y;
  uint16_t samus_pixel;
  uint16_t wall_pixel;
  BlockType under_samus;
  int block_size;

  state = MiniCreate(kPicoDebugTestWidth, kPicoDebugTestHeight);
  if (state == NULL) {
    fprintf(stderr, "test_pico_debug_tiles: MiniCreate failed\n");
    return 1;
  }

  dump = (uint16_t *)malloc((size_t)kPicoDebugTestWidth * kPicoDebugTestHeight * sizeof(*dump));
  if (dump == NULL) {
    fprintf(stderr, "test_pico_debug_tiles: dump alloc failed\n");
    MiniDestroy(state);
    return 1;
  }

  start_x = samus_x_pos;
  printf("samus_x_pos=%u samus_y_pos=%u pose=%u\n",
         (unsigned)start_x, (unsigned)samus_y_pos, (unsigned)samus_pose);

  for (i = 0; i < kPicoDebugTestFrames; i++)
    MiniStepButtons(state, kButton_Right, false);

  end_x = samus_x_pos;
  printf("samus_x_pos=%u samus_y_pos=%u pose=%u hash=0x%llx frames=%d\n",
         (unsigned)end_x, (unsigned)samus_y_pos, (unsigned)samus_pose,
         (unsigned long long)MiniStateHash(state), kPicoDebugTestFrames);

  expect_true("samus moved right", end_x > start_x);

  PicoDebugTiles_Composite(state, dump, kPicoDebugTestWidth, kPicoDebugTestHeight);

  expect_true("solid != air", PicoDebugTiles_SolidColor() != PicoDebugTiles_AirColor());
  expect_true("samus != solid", PicoDebugTiles_SamusColor() != PicoDebugTiles_SolidColor());

  wall_pixel = pixel_at(dump, kPicoDebugTestWidth, kPicoDebugTestHeight, 8, 8);
  expect_true("left/top padding is solid", wall_pixel == PicoDebugTiles_SolidColor());

  screen_x = state->samus.world_x - state->viewport.camera_x;
  screen_y = state->samus.world_y - state->viewport.camera_y;
  expect_true("samus on screen x", screen_x >= 0 && screen_x < kPicoDebugTestWidth);
  expect_true("samus on screen y", screen_y >= 0 && screen_y < kPicoDebugTestHeight);

  samus_pixel = pixel_at(dump, kPicoDebugTestWidth, kPicoDebugTestHeight, screen_x, screen_y);
  expect_true("pixel under samus is not solid-wall color",
              samus_pixel != PicoDebugTiles_SolidColor());
  expect_true("pixel under samus is samus rect",
              samus_pixel == PicoDebugTiles_SamusColor());

  block_size = state->collision_map.block_size > 0 ? state->collision_map.block_size
                                                   : kMiniBlockSize;
  under_samus = MiniStubs_GetCollisionMaterial(state->samus.world_x / block_size,
                                               state->samus.world_y / block_size);
  expect_true("samus walked into air", under_samus == kBlockType_Air);

  if (!PicoDebugTiles_WritePpm("out/pico_debug_tiles.ppm", dump,
                               kPicoDebugTestWidth, kPicoDebugTestHeight)) {
    fprintf(stderr, "test_pico_debug_tiles: optional PPM write skipped\n");
  } else {
    printf("wrote out/pico_debug_tiles.ppm %dx%d\n",
           kPicoDebugTestWidth, kPicoDebugTestHeight);
  }

  free(dump);
  MiniDestroy(state);
  return g_failures ? 2 : 0;
}
