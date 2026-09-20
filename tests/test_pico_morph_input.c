/*
 * sm_rev-28r regression: Samus must not morph on her own, and must be able to
 * get back out with the four buttons the Explorer has.
 *
 * The Pico runs MiniAuthoredMovement_Step (g_rom == NULL, so the vanilla Samus
 * state machine is not in play). There, one NEW kButton_Down while grounded
 * morphs instantly and only a NEW kButton_Up unmorphs. The board has no Up, so
 * ExplorerButtons_Step() makes B a Down/Up toggle and gates the first frames.
 *
 *   make pico-morph-test
 */

#include <stdio.h>
#include <string.h>

#include "ida_types.h"
#include "mini/mini_game.h"
#include "variables.h"

#include "explorer_buttons.h"
#include "pico_ls_room.h"
#include "pico_oam_from_samus.h"

enum {
  kViewportW = 256,
  kViewportH = 224,
  kNeutralFrames = 600
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

static int pose_is_morphball(unsigned pose) {
  return pose == (unsigned)kPose_1D_FaceR_Morphball_Ground ||
         pose == (unsigned)kPose_1E_MoveR_Morphball_Ground ||
         pose == (unsigned)kPose_1F_MoveL_Morphball_Ground ||
         pose == (unsigned)kPose_41_FaceL_Morphball_Ground ||
         pose == (unsigned)kPose_31_FaceR_Morphball_Air ||
         pose == (unsigned)kPose_32_FaceL_Morphball_Air;
}

static int pose_is_standing(unsigned pose) {
  return pose == (unsigned)kPose_01_FaceR_Normal ||
         pose == (unsigned)kPose_02_FaceL_Normal ||
         pose == (unsigned)kPose_00_FaceF_Powersuit;
}

/* One frame through the real mapper, exactly as pico2_main.c does it. */
static void step(MiniGameState *state, ExplorerButtonsState *btn,
                 unsigned pressed) {
  uint16_t joy =
      ExplorerButtons_Step(btn, pressed, (unsigned)samus_movement_type);
  MiniStepButtons(state, joy, false);
}

static void step_n(MiniGameState *state, ExplorerButtonsState *btn,
                   unsigned pressed, int n) {
  int i;
  for (i = 0; i < n; i++)
    step(state, btn, pressed);
}

/* Press-and-release: the mapper is edge driven, so a tap needs both halves. */
static void tap(MiniGameState *state, ExplorerButtonsState *btn,
                unsigned pressed, int hold, int release) {
  step_n(state, btn, pressed, hold);
  step_n(state, btn, 0, release);
}

static MiniGameState *fresh_landing_site(ExplorerButtonsState *btn) {
  MiniGameState *state;
  if (!PicoLsRoom_Install()) {
    fprintf(stderr, "test_pico_morph_input: PicoLsRoom_Install failed\n");
    return NULL;
  }
  if (!PicoOam_InstallSamusBank91()) {
    fprintf(stderr, "test_pico_morph_input: InstallSamusBank91 failed\n");
    return NULL;
  }
  state = MiniCreate(kViewportW, kViewportH);
  ExplorerButtons_Init(btn);
  return state;
}

static void test_mapper_units(void) {
  ExplorerButtonsState btn;

  expect_true("legacy map: idle is no buttons",
              ExplorerButtons_ToJoypad(0) == 0);
  expect_true("legacy map: Y is Right",
              ExplorerButtons_ToJoypad(kExplorerBtnY) == kButton_Right);

  expect_true("movement 04 is morphball",
              ExplorerButtons_IsMorphMovement(kMovementType_04_MorphBallOnGround));
  expect_true("movement 08 is morphball",
              ExplorerButtons_IsMorphMovement(kMovementType_08_MorphBallFalling));
  expect_true("movement 00 is not morphball",
              !ExplorerButtons_IsMorphMovement(kMovementType_00_Standing));

  /* Boot gate: a button already held when the sim starts reports nothing. */
  ExplorerButtons_Init(&btn);
  expect_true("held-at-boot B reports nothing (frame 0)",
              ExplorerButtons_Step(&btn, kExplorerBtnB,
                                   kMovementType_00_Standing) == 0);
  expect_true("held-at-boot B still reports nothing (frame 1)",
              ExplorerButtons_Step(&btn, kExplorerBtnB,
                                   kMovementType_00_Standing) == 0);
  expect_true("all-released frame reports nothing and arms",
              ExplorerButtons_Step(&btn, 0, kMovementType_00_Standing) == 0);
  expect_true("B after arming is Down",
              ExplorerButtons_Step(&btn, kExplorerBtnB,
                                   kMovementType_00_Standing) == kButton_Down);
  /* Latch: the direction is chosen on the rising edge and does NOT flip to Up
   * the moment the morph lands, or a held B would toggle every other frame. */
  expect_true("held B stays Down after the morph lands",
              ExplorerButtons_Step(&btn, kExplorerBtnB,
                                   kMovementType_04_MorphBallOnGround) ==
                  kButton_Down);
  expect_true("release reports nothing",
              ExplorerButtons_Step(&btn, 0, kMovementType_04_MorphBallOnGround) ==
                  0);
  expect_true("next B press while balled is Up",
              ExplorerButtons_Step(&btn, kExplorerBtnB,
                                   kMovementType_04_MorphBallOnGround) ==
                  kButton_Up);
  expect_true("held B stays Up after the unmorph lands",
              ExplorerButtons_Step(&btn, kExplorerBtnB,
                                   kMovementType_00_Standing) == kButton_Up);

  /* The other three are unchanged by the gate. */
  expect_true("X is Left after arming",
              ExplorerButtons_Step(&btn, kExplorerBtnX,
                                   kMovementType_00_Standing) == kButton_Left);
  expect_true("A is jump after arming",
              ExplorerButtons_Step(&btn, kExplorerBtnA,
                                   kMovementType_00_Standing) == kButton_A);
}

static void test_neutral_never_morphs(void) {
  ExplorerButtonsState btn;
  MiniGameState *state = fresh_landing_site(&btn);
  int i;
  int morphed_on_frame = -1;

  if (state == NULL) {
    g_failures++;
    return;
  }
  for (i = 0; i < kNeutralFrames; i++) {
    step(state, &btn, 0);
    if (morphed_on_frame < 0 && pose_is_morphball(samus_pose))
      morphed_on_frame = i;
  }
  if (morphed_on_frame >= 0)
    fprintf(stderr, "  neutral morphed on frame %d (pose=%02X)\n",
            morphed_on_frame, (unsigned)samus_pose);
  expect_true("neutral 600 frames never reach a morphball pose",
              morphed_on_frame < 0);
  expect_true("neutral 600 frames end standing", pose_is_standing(samus_pose));
  expect_true("neutral 600 frames leave movement_type non-ball",
              !ExplorerButtons_IsMorphMovement(samus_movement_type));
  MiniDestroy(state);
}

/* The exact failure from the board: a button still down when the sim starts. */
static void test_button_held_through_boot_never_morphs(void) {
  ExplorerButtonsState btn;
  MiniGameState *state = fresh_landing_site(&btn);
  unsigned chord = kExplorerBtnA | kExplorerBtnB | kExplorerBtnX | kExplorerBtnY;

  if (state == NULL) {
    g_failures++;
    return;
  }
  /* BOOTSEL chord still held for 60 frames, then Y and A let go first so a
   * plain Left+Down survives the release -- that is what rolled her 154px
   * left of spawn and left her a ball. */
  step_n(state, &btn, chord, 60);
  step_n(state, &btn, kExplorerBtnB | kExplorerBtnX, 20);
  step_n(state, &btn, 0, 60);
  expect_true("chord held through boot does not morph",
              !pose_is_morphball(samus_pose));
  expect_true("chord held through boot does not move Samus",
              samus_x_pos == (uint16)kPicoLsRoomSpawnBlockX * kPicoLsRoomBlockPx + 1);
  MiniDestroy(state);
}

static void test_morph_then_unmorph(void) {
  ExplorerButtonsState btn;
  MiniGameState *state = fresh_landing_site(&btn);
  unsigned morphed_pose;

  if (state == NULL) {
    g_failures++;
    return;
  }
  /* Arm the gate and let her settle on the deck. */
  step_n(state, &btn, 0, 10);
  expect_true("settled standing before the morph",
              pose_is_standing(samus_pose));

  /* One press, held for 60 frames. The direction latch must keep emitting Down
   * for the whole hold: if it re-decided every frame it would flip to Up the
   * moment the morph landed, MiniUpdateButtons() would see that as a new press,
   * and a held B would morph/unmorph forever. */
  step_n(state, &btn, kExplorerBtnB, 60);
  morphed_pose = samus_pose;
  expect_true("one deliberate B press morphs", pose_is_morphball(morphed_pose));
  expect_true("morph sets a ball movement_type",
              ExplorerButtons_IsMorphMovement(samus_movement_type));
  expect_true("holding B does not toggle back and forth",
              pose_is_morphball(samus_pose));
  step_n(state, &btn, 0, 5);

  tap(state, &btn, kExplorerBtnB, 3, 20);
  expect_true("the next B tap unmorphs", !pose_is_morphball(samus_pose));
  expect_true("unmorph returns a standing pose", pose_is_standing(samus_pose));
  expect_true("unmorph clears the ball movement_type",
              !ExplorerButtons_IsMorphMovement(samus_movement_type));

  /* And it is repeatable, both ways, forever. */
  tap(state, &btn, kExplorerBtnB, 3, 10);
  expect_true("morph again", pose_is_morphball(samus_pose));
  tap(state, &btn, kExplorerBtnB, 3, 20);
  expect_true("unmorph again", pose_is_standing(samus_pose));
  MiniDestroy(state);
}

/* Walking still works while balled and after coming out of the ball. */
static void test_ball_and_walk_still_move(void) {
  ExplorerButtonsState btn;
  MiniGameState *state = fresh_landing_site(&btn);
  uint16 x_before;
  uint16 x_rolled;

  if (state == NULL) {
    g_failures++;
    return;
  }
  step_n(state, &btn, 0, 10);
  x_before = samus_x_pos;
  step_n(state, &btn, kExplorerBtnY, 30);
  expect_true("Y still walks right", samus_x_pos > x_before);

  step_n(state, &btn, 0, 5);
  tap(state, &btn, kExplorerBtnB, 3, 5);
  expect_true("morph from a walk", pose_is_morphball(samus_pose));
  x_rolled = samus_x_pos;
  step_n(state, &btn, kExplorerBtnY, 30);
  expect_true("Y still rolls right while balled", samus_x_pos > x_rolled);

  step_n(state, &btn, 0, 5);
  tap(state, &btn, kExplorerBtnB, 3, 20);
  expect_true("unmorph after rolling", pose_is_standing(samus_pose));
  MiniDestroy(state);
}

int main(void) {
  test_mapper_units();
  test_neutral_never_morphs();
  test_button_held_through_boot_never_morphs();
  test_morph_then_unmorph();
  test_ball_and_walk_still_move();

  if (g_failures) {
    fprintf(stderr, "test_pico_morph_input: %d failure(s)\n", g_failures);
    return 1;
  }
  printf("test_pico_morph_input: all assertions passed\n");
  return 0;
}
