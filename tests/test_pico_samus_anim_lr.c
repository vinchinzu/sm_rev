/*
 * sm_rev-17t: left/right Samus animation parity on the Pico 2 path.
 *
 * The on-glass bug this pins down: holding Right animated Samus, holding Left
 * moved her without animating. The cause was NOT the bank 0x91 clock and NOT
 * missing left CHR. It was that mini rolls LEFT on the ground in pose 0x41
 * (kPose_41_FaceL_Morphball_Ground), which tools/pico_pack_ls_assets.py did
 * not pack. PicoOam_SamusFrameIndex() then silently substituted
 * kPicoOamSamusFallbackPose (0x01, a near-static stand), so every left-facing
 * morphball frame resolved to the same handful of standing frames: "moves but
 * does not animate".
 *
 * Four things are proved here, all on the host:
 *   1. every packed pose's bank 0x91 delay stream resolves inside the packed
 *      $91B000..$91BFFF window, for every frame the pose has;
 *   2. each left/right pose pair packs the same number of frames;
 *   3. driving mini with a held Left produces as many distinct packed frame
 *      indices as a held Right -- standing AND morphball;
 *   4. no pose mini can reach under the Explorer's four buttons falls back to
 *      kPicoOamSamusFallbackPose.
 *
 *   make pico-samus-anim-lr-test
 */

#include <stdio.h>
#include <string.h>

#include "ida_types.h"
#include "mini/mini_game.h"
#include "sm_rtl.h"
#include "variables.h"

#include "pico_ls_room.h"
#include "pico_oam_from_samus.h"

enum {
  kViewportW = 256,
  kViewportH = 224,
  kSettleFrames = 8,
  /* Long enough for the slowest packed pose (10 frames at ~4 ticks each) to
   * come all the way round more than once. */
  kHoldFrames = 240,
  kSweepTrials = 120,
  kSweepFrames = 400,
  kMaxFrameIndex = 256,
  kDelayTableAddr = 0xB010
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

/* --- 1. bank 0x91 delay data, per packed pose ------------------------- */

static void check_delay_window(void) {
  int pose;
  int checked = 0;

  expect_true("bank 0x91 window installs", PicoOam_InstallSamusBank91());
  for (pose = 0; pose < 256; pose++) {
    int frames = PicoOam_SamusPoseFrames(pose);
    const uint8 *entry;
    const uint8 *stream;
    uint16 ptr;
    int any = 0;
    int i;

    if (frames <= 0)
      continue;
    checked++;

    entry = RomPtr_91((uint16)(kDelayTableAddr + pose * 2));
    if (entry == NULL) {
      fprintf(stderr,
              "FAIL pose %#04x: delay-table entry is outside the packed "
              "bank 0x91 window\n",
              pose);
      g_failures++;
      continue;
    }
    ptr = (uint16)(entry[0] | ((uint16)entry[1] << 8));
    /* The last byte the vanilla clock can index is stream[frames - 1]:
     * Samus_HandleAnimDelay() reads it by samus_anim_frame. */
    stream = RomPtr_91(ptr);
    if (stream == NULL || RomPtr_91((uint16)(ptr + frames - 1)) == NULL) {
      fprintf(stderr,
              "FAIL pose %#04x: delay stream %#06x+%d leaves the packed "
              "bank 0x91 window; the animation clock would stall\n",
              pose, (unsigned)ptr, frames);
      g_failures++;
      continue;
    }
    for (i = 0; i < frames; i++)
      if (stream[i] != 0)
        any = 1;
    if (!any) {
      fprintf(stderr, "FAIL pose %#04x: delay stream is all zero\n", pose);
      g_failures++;
    }
  }
  printf("checked bank 0x91 delay data for %d packed poses\n", checked);
  expect_true("more than a handful of poses are packed", checked >= 20);
}

/* --- 2. left/right pose pairs -------------------------------------------- */

static void check_pose_pairs(void) {
  /* {right, left, label}. These are the partner poses mini flips between. */
  static const struct {
    int r;
    int l;
    const char *what;
  } kPairs[] = {
      {0x01, 0x02, "stand"},        {0x09, 0x0A, "run"},
      {0x0B, 0x0C, "run with gun"}, {0x13, 0x14, "jump"},
      {0x19, 0x1A, "spin jump"},    {0x25, 0x26, "turn"},
      {0x27, 0x28, "crouch"},       {0x2F, 0x30, "turn jump"},
      /* 0x41, not 0x1F, is the pose mini uses to roll left on the ground. */
      {0x1D, 0x41, "morphball ground"},
      {0x31, 0x32, "morphball air"},
      {0x37, 0x38, "morph transition"},
      {0x3D, 0x3E, "unmorph transition"},
      {0x35, 0x36, "crouch transition"},
      {0x3B, 0x3C, "stand transition"},
      {0x43, 0x44, "turn while crouched"},
  };
  size_t i;

  for (i = 0; i < sizeof(kPairs) / sizeof(kPairs[0]); i++) {
    int nr = PicoOam_SamusPoseFrames(kPairs[i].r);
    int nl = PicoOam_SamusPoseFrames(kPairs[i].l);
    char name[96];

    snprintf(name, sizeof(name), "%s packs both facings (%#04x=%d, %#04x=%d)",
             kPairs[i].what, kPairs[i].r, nr, kPairs[i].l, nl);
    expect_true(name, nr > 0 && nr == nl);
  }
}

/* --- 3. held Left animates as much as held Right ------------------------- */

typedef struct HoldResult {
  int distinct_frames;
  int fell_back;
  int end_pose;
} HoldResult;

static int is_fallback_frame(int pose) {
  return !PicoOam_SamusPoseIsPacked(pose);
}

static HoldResult hold(MiniGameState *state, uint16 button, int frames) {
  HoldResult out;
  int seen[kMaxFrameIndex];
  int i;

  memset(seen, 0, sizeof(seen));
  out.distinct_frames = 0;
  out.fell_back = 0;
  for (i = 0; i < frames; i++) {
    int fi;

    MiniStepButtons(state, button, false);
    if (is_fallback_frame((int)samus_pose))
      out.fell_back = 1;
    fi = PicoOam_SamusFrameIndex((int)samus_pose, (int)samus_anim_frame);
    if (fi >= 0 && fi < kMaxFrameIndex && !seen[fi]) {
      seen[fi] = 1;
      out.distinct_frames++;
    }
  }
  out.end_pose = (int)samus_pose;
  return out;
}

/* Down-Down morphs Samus on the Explorer pad (B is mapped to Down). */
static void morph(MiniGameState *state) {
  int i;
  for (i = 0; i < 20; i++)
    MiniStepButtons(state, kButton_Down, false);
  for (i = 0; i < 5; i++)
    MiniStepButtons(state, 0, false);
  for (i = 0; i < 20; i++)
    MiniStepButtons(state, kButton_Down, false);
  for (i = 0; i < 5; i++)
    MiniStepButtons(state, 0, false);
}

static void check_hold_parity(const char *tag, int do_morph) {
  MiniGameState *state;
  HoldResult r;
  HoldResult l;
  char name[128];
  int i;

  state = MiniCreate(kViewportW, kViewportH);
  if (state == NULL) {
    fprintf(stderr, "FAIL %s: MiniCreate\n", tag);
    g_failures++;
    return;
  }
  for (i = 0; i < kSettleFrames; i++)
    MiniStepButtons(state, 0, false);
  if (do_morph)
    morph(state);
  r = hold(state, kButton_Right, kHoldFrames);
  l = hold(state, kButton_Left, kHoldFrames);
  printf("%s: right pose=%#04x %d distinct frames | left pose=%#04x %d "
         "distinct frames\n",
         tag, r.end_pose, r.distinct_frames, l.end_pose, l.distinct_frames);

  snprintf(name, sizeof(name), "%s: holding Right animates", tag);
  expect_true(name, r.distinct_frames >= 2);
  snprintf(name, sizeof(name), "%s: holding Left animates", tag);
  expect_true(name, l.distinct_frames >= 2);
  /* The real regression: left used to collapse onto the static fallback. */
  snprintf(name, sizeof(name),
           "%s: left animates as much as right (%d vs %d)", tag,
           l.distinct_frames, r.distinct_frames);
  expect_true(name, l.distinct_frames == r.distinct_frames);
  snprintf(name, sizeof(name), "%s: right never hits the fallback pose", tag);
  expect_true(name, !r.fell_back);
  snprintf(name, sizeof(name), "%s: left never hits the fallback pose", tag);
  expect_true(name, !l.fell_back);
  MiniDestroy(state);
}

/* --- 4. every pose the Explorer's buttons can reach is packed ------------ */

static void check_reachable_poses(void) {
  static const uint16 kButtons[4] = {kButton_Right, kButton_Left, kButton_Down,
                                     kButton_A};
  int seen[256];
  /* Fixed seed: this sweep must be deterministic. */
  unsigned seed = 12345u;
  int trial;
  int pose;
  int unpacked = 0;

  memset(seen, 0, sizeof(seen));
  for (trial = 0; trial < kSweepTrials; trial++) {
    MiniGameState *state = MiniCreate(kViewportW, kViewportH);
    uint16 held = 0;
    int i;

    if (state == NULL) {
      fprintf(stderr, "FAIL pose sweep: MiniCreate\n");
      g_failures++;
      return;
    }
    for (i = 0; i < kSweepFrames; i++) {
      if ((i % 7) == 0) {
        seed = seed * 1103515245u + 12345u;
        held = 0;
        if ((seed >> 16) & 1u)
          held |= kButtons[(seed >> 17) & 3u];
        if ((seed >> 21) & 1u)
          held |= kButtons[(seed >> 22) & 3u];
      }
      MiniStepButtons(state, held, false);
      seen[samus_pose & 0xff] = 1;
    }
    MiniDestroy(state);
  }
  printf("pose sweep visited:");
  for (pose = 0; pose < 256; pose++) {
    if (!seen[pose])
      continue;
    printf(" %#04x%s", pose,
           PicoOam_SamusPoseIsPacked(pose) ? "" : "(UNPACKED)");
    if (!PicoOam_SamusPoseIsPacked(pose))
      unpacked++;
  }
  printf("\n");
  expect_true("every pose mini reaches from the Explorer pad is packed",
              unpacked == 0);
}

int main(void) {
  expect_true("packed Landing Site room installs", PicoLsRoom_Install());
  check_delay_window();
  check_pose_pairs();
  check_hold_parity("standing", 0);
  check_hold_parity("morphball", 1);
  check_reachable_poses();

  if (g_failures != 0) {
    fprintf(stderr, "test_pico_samus_anim_lr: %d failure(s)\n", g_failures);
    return 1;
  }
  printf("test_pico_samus_anim_lr: OK\n");
  return 0;
}
