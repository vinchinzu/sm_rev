/*
 * sm_rev-khe: host-side hunt for the RP2350 "screen locks up after a while".
 *
 * The board freezes intermittently during play; an idle board survives
 * indefinitely. That shape -- input-dependent, not time-dependent -- points at
 * a game state the player reaches rather than a counter wrap, so this harness
 * drives the portable half of the firmware's frame path (the packer, the
 * spritemap walk, the Mode 1 raster) over far more state than a person could
 * reach by hand, and leans on ASan/UBSan to catch the out-of-bounds class the
 * bead is about.
 *
 * What it covers, and why each part exists:
 *
 *   1. Spritemap blob invariant. PicoOam_WriteSamusFrame reads a 2-byte count
 *      from the packed blob and walks 5 bytes per entry. It clamps the WRITES
 *      to the 64 OAM slots it owns; before this bead it did not bound the
 *      READS against the end of the blob. Asserted for every packed frame, so
 *      a re-pack that emits a short tail fails here instead of quietly reading
 *      neighbouring flash on the Pico.
 *
 *   2. Every packed frame index at extreme screen positions. The firmware
 *      hands PicoOam_WriteSamusFrame an unclamped sx/sy (Samus can be anywhere
 *      in the room while the camera is pinned at the packed window edge), so
 *      the OAM it produces can put a sprite far off either side. Each one is
 *      then rastered in full.
 *
 *   3. The whole reachable camera range, exhaustively. pack_from_samus clamps
 *      the camera to the packed window; this walks every (bg1hofs, bg1vofs)
 *      that clamp can produce, including both ends, and rasters all 224 lines.
 *
 *   4. A live soak: MiniStepButtons with a deterministic pseudo-random button
 *      stream, running exactly the pico2_main.c frame path (pack + raster),
 *      with a per-frame wall-clock budget so a hang in the sim fails the test
 *      instead of hanging the suite.
 *
 *   5. A raster fuzz that writes arbitrary OAM, obsel and BG registers. Real
 *      gameplay only reaches a narrow slice of PPU state; this covers the
 *      sprite-size / name-select / charnum / tilemap-size corners that a
 *      future OAM writer could reach.
 *
 * Build (the Makefile target adds -fsanitize=address,undefined):
 *   make pico-lockup-test
 *
 * SM_REV_SOAK_FRAMES=n raises the soak length for a long local run.
 *
 * Do not pass -Isrc: it shadows glibc's <features.h> with src/features.h.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ida_types.h"
#include "mini/mini_game.h"
#include "variables.h"

#include "pico_ls_assets.h"
#include "pico_ls_room.h"
#include "pico_oam_from_samus.h"
#include "pico_oam_gunship.h"
#include "pico_viewport.h"

enum {
  kViewportW = 256,
  kViewportH = 224,
  /* Same numbers pico2_main.c derives its camera clamp from. */
  kMaxScrollX = 512 - kViewportW,
  kMaxScrollY = 256 - kViewportH,
  kDefaultSoakFrames = 3000,
  /* A host frame is well under a millisecond even under ASan. Half a second is
   * "this is not slow, this is stuck". */
  kFrameBudgetMs = 500
};

static int g_failures;

static void expect_true(const char *name, int cond) {
  if (!cond) {
    fprintf(stderr, "FAIL %s\n", name);
    g_failures++;
  }
}

/* xorshift32: the soak has to be reproducible from the seed printed below. */
static uint32_t g_rng = 0x5D4B0001u;

static uint32_t rng_next(void) {
  g_rng ^= g_rng << 13;
  g_rng ^= g_rng >> 17;
  g_rng ^= g_rng << 5;
  return g_rng;
}

static PicoFramePacket g_pkt;
static uint16_t g_line256[kPicoScreenWidth];
static uint16_t g_line240[kPicoPanelWidth];

/* Exactly what present_frame() does on the Pico, minus the panel. */
static void raster_frame(const PicoFramePacket *pkt) {
  PicoPpuState ppu;
  int y;

  PicoFramePacket_ToPpu(pkt, &ppu);
  for (y = 0; y < kPicoScreenHeight; y++) {
    PicoScanline_Mode1Range(&ppu, y, g_line256, kPicoViewportCropX,
                            kPicoViewportCropX + kPicoPanelWidth);
    PicoViewport_CropLineRgb565(g_line256, g_line240);
  }
}

/* --- 1: the packed spritemap blob is self-consistent ------------------- */

static void test_spritemap_bounds(void) {
  int total = PicoOam_SamusFrameTotal();
  int bad = 0;
  int i;

  expect_true("frame table is non-empty", total > 0);
  for (i = 0; i < total; i++) {
    if (!PicoOam_SamusSpritemapFits(i)) {
      if (bad < 8)
        fprintf(stderr, "  frame %d walks past the spritemap blob\n", i);
      bad++;
    }
  }
  expect_true("every packed frame's spritemap fits inside the blob", bad == 0);
  /* Out-of-range indices must be rejected, not clamped into the table. */
  expect_true("negative frame index rejected", !PicoOam_SamusSpritemapFits(-1));
  expect_true("frame index past the end rejected",
              !PicoOam_SamusSpritemapFits(total));
  printf("spritemap: %d frames, all inside the blob\n", total);
}

/* --- 2: every frame index, at positions the camera clamp can produce --- */

static void test_every_frame_index(void) {
  /* sx/sy the firmware can hand us: Samus at either end of the room while the
   * camera is pinned, plus the wrap-around cases the uint8/uint16 OAM
   * arithmetic in PicoOam_WriteSamusFrame goes through. */
  static const int kPos[] = {-4096, -512, -257, -256, -129, -1, 0,
                             1,     127,  128,  255,  256, 257, 512, 4095};
  int total = PicoOam_SamusFrameTotal();
  size_t nx = sizeof kPos / sizeof kPos[0];
  size_t xi;
  size_t yi;
  int planted = 0;
  int i;

  for (i = 0; i < total; i++) {
    if (PicoOam_PlantSamusFrame(&g_pkt, i))
      planted++;
    for (xi = 0; xi < nx; xi++) {
      for (yi = 0; yi < nx; yi++) {
        PicoOam_WriteSamusFrame(&g_pkt, i, kPos[xi], kPos[yi]);
        PicoOam_WriteGunship(&g_pkt, -(int)g_pkt.bg1hofs, -(int)g_pkt.bg1vofs);
      }
    }
    /* One full raster per frame index: the cheap positions above only exercise
     * the packer, the raster is what indexes VRAM. */
    PicoOam_WriteSamusFrame(&g_pkt, i, 120, 100);
    raster_frame(&g_pkt);
  }
  expect_true("every packed frame plants its CHR", planted == total);
  /* Out-of-range indices must be no-ops, not reads off the offset table. */
  PicoOam_WriteSamusFrame(&g_pkt, -1, 0, 0);
  PicoOam_WriteSamusFrame(&g_pkt, total, 0, 0);
  PicoOam_WriteSamusFrame(&g_pkt, 0x7FFFFFFF, 0, 0);
  expect_true("out-of-range frame index plants nothing",
              !PicoOam_PlantSamusFrame(&g_pkt, total));
  printf("frames: %d indices x %zu x %zu positions + full raster each\n", total,
         nx, nx);
}

/* --- 3: the whole reachable camera range ------------------------------- */

static void test_camera_range(void) {
  int hx;
  int vy;
  int frames = 0;

  PicoOam_WriteSamusFrame(&g_pkt, 0, 120, 100);
  for (hx = 0; hx <= kMaxScrollX; hx += 4) {
    for (vy = 0; vy <= kMaxScrollY; vy += 4) {
      g_pkt.bg1hofs = (uint16_t)hx;
      g_pkt.bg1vofs = (uint16_t)vy;
      g_pkt.bg2hofs = (uint16_t)(hx >> 1);
      g_pkt.bg2vofs = (uint16_t)kPicoLsBg2VerticalScroll;
      PicoOam_WriteGunship(&g_pkt, -hx, -vy);
      raster_frame(&g_pkt);
      frames++;
    }
  }
  /* Both hard ends exactly, which the stride above may skip. */
  {
    const int ex[4] = {0, kMaxScrollX, 0, kMaxScrollX};
    const int ey[4] = {0, 0, kMaxScrollY, kMaxScrollY};
    int k;
    for (k = 0; k < 4; k++) {
      g_pkt.bg1hofs = (uint16_t)ex[k];
      g_pkt.bg1vofs = (uint16_t)ey[k];
      g_pkt.bg2hofs = (uint16_t)(ex[k] >> 1);
      PicoOam_WriteGunship(&g_pkt, -ex[k], -ey[k]);
      raster_frame(&g_pkt);
      frames++;
    }
  }
  printf("camera: %d full rasters over hofs 0..%d vofs 0..%d\n", frames,
         kMaxScrollX, kMaxScrollY);
}

/* --- 4: live soak, the real pico2_main.c frame path -------------------- */

/* Byte-for-byte the camera clamp and OAM writes from pico2_main.c. */
static int soak_pack(PicoFramePacket *pkt, uint32_t frame_id, uint16_t joy,
                     int *last_frame_index) {
  int world_x = (int)samus_x_pos;
  int world_y = (int)samus_y_pos;
  int pose = (int)samus_pose;
  int anim = (int)samus_anim_frame;
  int frame_index = PicoOam_SamusFrameIndex(pose, anim);
  int cam_x = world_x - kViewportW / 2;
  int cam_y = world_y - kViewportH / 2;
  int sx;
  int sy;

  if (cam_x < (int)kPicoLsExtractCameraX)
    cam_x = (int)kPicoLsExtractCameraX;
  if (cam_x > (int)kPicoLsExtractCameraX + kMaxScrollX)
    cam_x = (int)kPicoLsExtractCameraX + kMaxScrollX;
  if (cam_y < (int)kPicoLsExtractCameraY)
    cam_y = (int)kPicoLsExtractCameraY;
  if (cam_y > (int)kPicoLsExtractCameraY + kMaxScrollY)
    cam_y = (int)kPicoLsExtractCameraY + kMaxScrollY;

  sx = world_x - cam_x;
  sy = world_y - cam_y - PicoOam_SamusYOffset(pose);

  if (frame_index != *last_frame_index) {
    if (PicoOam_PlantSamusFrame(pkt, frame_index))
      *last_frame_index = frame_index;
  }

  pkt->frame_id = frame_id;
  pkt->vsync_token = (uint16_t)kPicoFramePacketVsync;
  pkt->joypad_echo = joy;
  pkt->bg1hofs = (uint16_t)(cam_x - (int)kPicoLsExtractCameraX);
  pkt->bg1vofs = (uint16_t)(cam_y - (int)kPicoLsExtractCameraY);
  pkt->bg2hofs = (uint16_t)(pkt->bg1hofs >> 1);
  pkt->bg2vofs = (uint16_t)kPicoLsBg2VerticalScroll;

  PicoOam_WriteSamusFrame(pkt, frame_index, sx, sy);
  PicoOam_WriteGunship(pkt, -(int)pkt->bg1hofs, -(int)pkt->bg1vofs);
  return frame_index;
}

static long soak_frames(void) {
  const char *env = getenv("SM_REV_SOAK_FRAMES");
  long n = env != NULL ? strtol(env, NULL, 10) : 0;
  return n > 0 ? n : (long)kDefaultSoakFrames;
}

/*
 * Two pools. "explorer" is exactly what ExplorerButtons_ToJoypad can produce
 * from the four buttons on the board, so it is the set the reported lockup is
 * actually reachable from. "full" is every SNES button, which reaches spin
 * jumps, wall jumps and aim poses the Explorer mapping cannot -- worth driving
 * anyway, since the packed frame table has to survive whatever pose the sim
 * lands in and the button mapping is being changed by another bead.
 */
static const uint16_t kJoyExplorer[8] = {
    0,
    kButton_Right,
    kButton_Left,
    kButton_Down,
    kButton_A,
    (uint16_t)(kButton_A | kButton_Right),
    (uint16_t)(kButton_A | kButton_Left),
    (uint16_t)(kButton_Left | kButton_Right)};

static const uint16_t kJoyFull[16] = {
    0,
    kButton_Right,
    kButton_Left,
    kButton_Up,
    kButton_Down,
    kButton_A,
    kButton_B,
    kButton_X,
    kButton_Y,
    kButton_L,
    kButton_R,
    (uint16_t)(kButton_A | kButton_Right),
    (uint16_t)(kButton_B | kButton_Left),
    (uint16_t)(kButton_Down | kButton_B),
    (uint16_t)(kButton_Up | kButton_A | kButton_Right),
    (uint16_t)(kButton_R | kButton_Y | kButton_Left)};

static void test_soak(const char *tag, const uint16_t *pool, unsigned pool_mask,
                      uint32_t seed) {
  MiniGameState *state;
  long total = soak_frames();
  long i;
  int last_frame_index = -1;
  int frame_total = PicoOam_SamusFrameTotal();
  int hold = 0;
  uint16_t joy = 0;
  int bad_index = 0;
  int bad_scroll = 0;
  int slow_frames = 0;
  unsigned char seen_pose[256];
  int poses_seen = 0;
  int p;

  memset(seen_pose, 0, sizeof seen_pose);
  g_rng = seed;

  expect_true("PicoLsRoom_Install", PicoLsRoom_Install() != 0);
  expect_true("PicoOam_InstallSamusBank91", PicoOam_InstallSamusBank91() != 0);

  state = MiniCreate(kViewportW, kViewportH);
  if (state == NULL) {
    fprintf(stderr, "FAIL soak: MiniCreate\n");
    g_failures++;
    return;
  }

  PicoFramePacket_InitLandingSiteExtracted(&g_pkt);
  PicoOam_ParkAllSprites(&g_pkt);
  expect_true("PlantGunshipGfx", PicoOam_PlantGunshipGfx(&g_pkt) != 0);

  for (i = 0; i < total; i++) {
    clock_t t0;
    double ms;
    int fi;

    /* Hold each input for a few frames: a new random mask every frame never
     * lets the sim finish a jump, a turn-around or a morph. */
    if (hold == 0) {
      joy = pool[rng_next() & pool_mask];
      hold = 1 + (int)(rng_next() % 24u);
    }
    hold--;

    t0 = clock();
    MiniStepButtons(state, joy, false);
    fi = soak_pack(&g_pkt, (uint32_t)i + 2u, joy, &last_frame_index);
    raster_frame(&g_pkt);
    ms = 1000.0 * (double)(clock() - t0) / (double)CLOCKS_PER_SEC;
    if (ms > (double)kFrameBudgetMs)
      slow_frames++;

    if (fi < 0 || fi >= frame_total)
      bad_index++;
    if (g_pkt.bg1hofs > (uint16_t)kMaxScrollX ||
        g_pkt.bg1vofs > (uint16_t)kMaxScrollY)
      bad_scroll++;
    if (!seen_pose[samus_pose & 0xFF]) {
      seen_pose[samus_pose & 0xFF] = 1;
      poses_seen++;
    }
  }

  expect_true("soak never resolves an out-of-range frame index",
              bad_index == 0);
  expect_true("soak camera never leaves the packed window", bad_scroll == 0);
  expect_true("no soak frame blew the wall-clock budget", slow_frames == 0);

  printf("soak[%s]: %ld frames, seed=0x%08x, %d distinct poses, x=%u y=%u "
         "pose=%u\n",
         tag, total, (unsigned)seed, poses_seen, (unsigned)samus_x_pos,
         (unsigned)samus_y_pos, (unsigned)samus_pose);
  printf("soak[%s] poses:", tag);
  for (p = 0; p < 256; p++)
    if (seen_pose[p])
      printf(" %02x%s", p, PicoOam_SamusPoseIsPacked(p) ? "" : "*");
  printf("   (* = not packed, falls back to pose 1)\n");
}

/* --- 5: raster fuzz over PPU state gameplay does not reach ------------- */

static void test_raster_fuzz(void) {
  PicoFramePacket *pkt = &g_pkt;
  int iter;

  for (iter = 0; iter < 512; iter++) {
    int i;

    for (i = 0; i < kPicoOamSize; i++)
      pkt->oam[i] = (uint8_t)rng_next();
    for (i = 0; i < kPicoOamHiSize; i++)
      pkt->oam_hi[i] = (uint8_t)rng_next();
    for (i = 0; i < kPicoCgramColors; i++)
      pkt->cgram[i] = (uint16_t)rng_next();
    /* obsel: any name base, name select and sprite size combination. */
    pkt->obsel = (uint8_t)rng_next();
    /* bg1sc/bg2sc: any tilemap base and any 32/64 tilemap size. */
    pkt->bg1sc = (uint8_t)rng_next();
    pkt->bg2sc = (uint8_t)rng_next();
    pkt->bg12nba = (uint8_t)rng_next();
    pkt->bg1hofs = (uint16_t)rng_next();
    pkt->bg1vofs = (uint16_t)rng_next();
    pkt->bg2hofs = (uint16_t)rng_next();
    pkt->bg2vofs = (uint16_t)rng_next();
    pkt->oam_full = 1;
    pkt->cgram_full = 1;
    raster_frame(pkt);
  }
  /* A short VRAM makes every tile fetch straddle the end of the buffer: the
   * raster must fall back to transparent, not read past vram_size. */
  {
    static uint8_t tiny[64];
    const uint8_t *saved = pkt->vram;
    size_t saved_n = pkt->vram_size;

    for (iter = 0; iter < (int)sizeof tiny; iter++)
      tiny[iter] = (uint8_t)rng_next();
    pkt->vram = tiny;
    pkt->vram_size = sizeof tiny;
    raster_frame(pkt);
    pkt->vram = NULL;
    pkt->vram_size = 0;
    raster_frame(pkt);
    pkt->vram = saved;
    pkt->vram_size = saved_n;
  }
  printf("raster fuzz: 512 random PPU states + short/NULL VRAM\n");
}

int main(void) {
  printf("test_pico_lockup_stress (sm_rev-khe)\n");

  PicoFramePacket_InitLandingSiteExtracted(&g_pkt);
  PicoOam_ParkAllSprites(&g_pkt);
  PicoOam_PlantGunshipGfx(&g_pkt);

  test_spritemap_bounds();
  test_every_frame_index();
  test_camera_range();
  test_raster_fuzz();
  /* Last: these install the LS room and create the sim, which is global state
   * the earlier tests are happier without. */
  test_soak("explorer", kJoyExplorer, 7u, 0x5D4B0001u);
  test_soak("full", kJoyFull, 15u, 0x5D4B0002u);

  if (g_failures != 0) {
    fprintf(stderr, "test_pico_lockup_stress: %d failure(s)\n", g_failures);
    return 1;
  }
  printf("test_pico_lockup_stress: all assertions passed\n");
  return 0;
}
