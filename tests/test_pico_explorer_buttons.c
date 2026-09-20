/*
 * Explorer A/B/X/Y pressed-mask → MiniStepButtons walk map.
 * Crop 256×224 → 240×240 (8px L/R, 8px letterbox). No GPIO, no SDK.
 *
 *   make pico-explorer-buttons-test
 */

#include <stdio.h>
#include <string.h>

#include "ida_types.h"
#include "mini/mini_game.h"
#include "variables.h"

#include "explorer_buttons.h"
#include "pico_viewport.h"

enum {
  kWalkFrames = 30,
  kViewportW = 256,
  kViewportH = 224
};

static int g_failures;

static void expect_true(const char *name, int cond) {
  if (!cond) {
    fprintf(stderr, "FAIL %s\n", name);
    g_failures++;
    return;
  }
  printf("PASS %s\n", name);
}

int main(void) {
  MiniGameState *state;
  uint16 start_x;
  uint16 end_x;
  uint16_t src[kPicoScreenWidth];
  uint16_t dst[kPicoPanelWidth];
  int i;

  expect_true("idle is no buttons", ExplorerButtons_ToJoypad(0) == 0);
  expect_true("Y is Right",
              ExplorerButtons_ToJoypad(kExplorerBtnY) == kButton_Right);
  expect_true("X is Left",
              ExplorerButtons_ToJoypad(kExplorerBtnX) == kButton_Left);
  expect_true("B is Down",
              ExplorerButtons_ToJoypad(kExplorerBtnB) == kButton_Down);
  expect_true("A is jump",
              ExplorerButtons_ToJoypad(kExplorerBtnA) == kButton_A);
  expect_true("X+Y is Left|Right",
              ExplorerButtons_ToJoypad(kExplorerBtnX | kExplorerBtnY) ==
                  (kButton_Left | kButton_Right));
  expect_true("A+Y is jump+Right",
              ExplorerButtons_ToJoypad(kExplorerBtnA | kExplorerBtnY) ==
                  (kButton_A | kButton_Right));

  expect_true("crop 8+240+8 = 256",
              kPicoViewportCropX + kPicoPanelWidth + kPicoViewportCropX ==
                  kPicoScreenWidth);
  expect_true("letterbox 8+224+8 = 240",
              kPicoViewportLetterboxY + kPicoScreenHeight +
                      kPicoViewportLetterboxY ==
                  kPicoPanelHeight);

  memset(src, 0, sizeof(src));
  src[7] = 0x1111;
  src[8] = 0xF800;
  src[247] = 0x07E0;
  src[248] = 0x4444;
  PicoViewport_CropLineRgb565(src, dst);
  expect_true("crop drops x=0..7", dst[0] == 0xF800);
  expect_true("crop keeps x=247 as last panel pixel", dst[239] == 0x07E0);

  expect_true("scroll at extract origin is 0",
              PicoViewport_ExtractScroll(1024, 1024) == 0);
  expect_true("scroll past origin",
              PicoViewport_ExtractScroll(1100, 1024) == 76);
  expect_true("scroll below origin is 0 (camera 0)",
              PicoViewport_ExtractScroll(0, 1024) == 0);
  expect_true("scroll below origin is 0 (camera 80)",
              PicoViewport_ExtractScroll(80, 1024) == 0);

  state = MiniCreate(kViewportW, kViewportH);
  if (state == NULL) {
    fprintf(stderr, "test_pico_explorer_buttons: MiniCreate failed\n");
    return 1;
  }
  start_x = samus_x_pos;
  for (i = 0; i < kWalkFrames; i++)
    MiniStepButtons(state, ExplorerButtons_ToJoypad(kExplorerBtnY), false);
  end_x = samus_x_pos;
  MiniDestroy(state);
  expect_true("Y walk increases samus_x_pos", end_x > start_x);

  if (g_failures) {
    fprintf(stderr, "test_pico_explorer_buttons: %d failure(s)\n", g_failures);
    return 2;
  }
  printf("test_pico_explorer_buttons: all assertions passed (x %u -> %u)\n",
         (unsigned)start_x, (unsigned)end_x);
  return 0;
}
