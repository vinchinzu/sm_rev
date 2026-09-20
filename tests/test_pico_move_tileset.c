/*
 * Host-paired proof: MiniStepButtons Right over extracted Landing Site tiles.
 * Game side packs OAM+scrolls every frame (VRAM Samus CHR on frame 1 only).
 * Display side decodes and rasters Mode 1. Packed pose-1 spritemap tracks Samus.
 *
 * Second phase: the packed Landing Site GAMEPLAY room (PicoLsRoom_Install).
 * Walks Samus off each side of the gunship deck and proves she falls to the
 * terrain instead of being held up by mini's flat fallback-room floor.
 *
 *   make pico-kernel
 *   gcc -O2 -fno-strict-aliasing -Werror -DSYSTEM_VOLUME_MIXER_AVAILABLE=0 \
 *       -I. -Isrc/pico -iquote src -iquote src/mini -DCURRENT_BUILD=BUILD_PICO \
 *       tests/test_pico_move_tileset.c src/pico/pico_oam_from_samus.c \
 *       src/pico/pico_frame_wire.c src/pico/pico_frame_packet.c \
 *       src/pico/pico_ls_assets.c src/pico/pico_ls_room.c \
 *       src/pico/scanline_mode1.c \
 *       src/pico/pico_viewport.c \
 *       -o sm_rev_pico_move_tileset_test -L. -lsm_rev_pico_kernel -lm
 *   ./sm_rev_pico_move_tileset_test
 *
 * Do not pass -Isrc: it shadows glibc's <features.h> with src/features.h.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ida_types.h"
#include "mini/mini_game.h"
#include "variables.h"

#include "pico_frame_wire.h"
#include "pico_ls_assets.h"
#include "pico_ls_room.h"
#include "pico_oam_from_samus.h"
#include "pico_viewport.h"

enum {
  kMoveFrames = 30,
  kViewportW = 256,
  kViewportH = 224,
  kFramebufferBytes = kPicoScreenWidth * kPicoScreenHeight * 2,
  /* Long enough to clear the deck (its solid span is 8 blocks) and finish the
   * fall at the authored terminal fall speed. Measured landing is frame ~45. */
  kDeckWalkFrames = 70,
  kSettleFrames = 4,
  /* Deck top to terrain is 7 block rows. Anything under a block of movement is
   * slope jitter on the deck itself, so demand well over half the real drop. */
  kMinDeckDrop = 96
};

static int g_failures;

static void expect_true(const char *name, int cond) {
  if (!cond) {
    fprintf(stderr, "FAIL %s\n", name);
    g_failures++;
  }
}

static void pack_from_samus(PicoFramePacket *pkt, uint32_t frame_id) {
  int sx = (int)samus_x_pos - (int)layer1_x_pos;
  int sy = (int)samus_y_pos - (int)layer1_y_pos - PicoOam_SamusYOffset((int)samus_pose);
  uint16_t hofs = PicoViewport_ExtractScroll(layer1_x_pos,
                                             (uint16_t)kPicoLsExtractCameraX);
  uint16_t vofs = PicoViewport_ExtractScroll(layer1_y_pos,
                                             (uint16_t)kPicoLsExtractCameraY);

  pkt->frame_id = frame_id;
  pkt->vsync_token = (uint16_t)kPicoFramePacketVsync;
  pkt->joypad_echo = kButton_Right;
  pkt->bg1hofs = hofs;
  pkt->bg1vofs = vofs;
  pkt->bg2hofs = hofs;
  pkt->bg2vofs = vofs;
  {
    int frame_index =
        PicoOam_SamusFrameIndex((int)samus_pose, (int)samus_anim_frame);
    PicoOam_PlantSamusFrame(pkt, frame_index);
    PicoOam_WriteSamusFrame(pkt, frame_index, sx, sy);
  }
}

static void raster_packet(const PicoFramePacket *pkt, uint16_t *out) {
  PicoPpuState ppu;
  int y;

  PicoFramePacket_ToPpu(pkt, &ppu);
  for (y = 0; y < kPicoScreenHeight; y++)
    PicoScanline_Mode1(&ppu, y, out + (size_t)y * kPicoScreenWidth);
}

static void raster_bg_only(const PicoFramePacket *pkt, uint16_t *out) {
  PicoFramePacket bg = *pkt;
  bg.oam_full = 0;
  raster_packet(&bg, out);
}

typedef struct SpriteStats {
  int count;
  int min_x;
  int max_x;
  int min_y;
  int over_bg;
} SpriteStats;

static SpriteStats measure_sprite(const uint16_t *with_obj, const uint16_t *bg_only) {
  SpriteStats s;
  int y;
  int x;

  s.count = 0;
  s.min_x = kPicoScreenWidth;
  s.max_x = -1;
  s.min_y = kPicoScreenHeight;
  s.over_bg = 0;
  for (y = 0; y < kPicoScreenHeight; y++) {
    for (x = 0; x < kPicoScreenWidth; x++) {
      int i = y * kPicoScreenWidth + x;
      if (with_obj[i] == bg_only[i])
        continue;
      s.count++;
      if (x < s.min_x)
        s.min_x = x;
      if (x > s.max_x)
        s.max_x = x;
      if (y < s.min_y)
        s.min_y = y;
      if (bg_only[i] != 0)
        s.over_bg++;
    }
  }
  return s;
}

static int write_ppm(const char *path, const uint16_t *frame) {
  FILE *f;
  int i;
  int n = kPicoScreenWidth * kPicoScreenHeight;

  f = fopen(path, "wb");
  if (f == NULL)
    return 0;
  fprintf(f, "P6\n%d %d\n255\n", kPicoScreenWidth, kPicoScreenHeight);
  for (i = 0; i < n; i++) {
    uint16_t c = frame[i];
    uint8_t rgb[3];
    rgb[0] = (uint8_t)(((c >> 11) & 0x1F) * 255 / 31);
    rgb[1] = (uint8_t)(((c >> 5) & 0x3F) * 255 / 63);
    rgb[2] = (uint8_t)((c & 0x1F) * 255 / 31);
    if (fwrite(rgb, 1, 3, f) != 3) {
      fclose(f);
      return 0;
    }
  }
  fclose(f);
  return 1;
}

/* --- packed Landing Site gameplay room -------------------------------- */

static int feet_block_row(void) {
  return ((int)samus_y_pos + (int)samus_y_radius - 1) / kPicoLsRoomBlockPx;
}

/* Walks one direction off the gunship deck and reports the descent. */
static int walk_off_deck(const char *tag, uint16 button) {
  MiniGameState *state = MiniCreate(kViewportW, kViewportH);
  int start_y;
  int start_feet;
  int end_feet;
  int drop;
  int i;

  if (state == NULL) {
    fprintf(stderr, "FAIL %s: MiniCreate\n", tag);
    g_failures++;
    return 0;
  }
  for (i = 0; i < kSettleFrames; i++)
    MiniStepButtons(state, 0, false);

  start_y = (int)samus_y_pos;
  start_feet = feet_block_row();
  expect_true("settles standing on the ship deck",
              start_feet >= kPicoLsRoomDeckTopRow &&
                  start_feet <= kPicoLsRoomDeckBottomRow);
  expect_true("settles on the ground", state->samus.on_ground);

  for (i = 0; i < kDeckWalkFrames; i++)
    MiniStepButtons(state, button, false);

  drop = (int)samus_y_pos - start_y;
  end_feet = feet_block_row();
  printf("%s: start y=%d feet_by=%d -> end x=%u y=%u feet_by=%d drop=%d og=%d\n",
         tag, start_y, start_feet, (unsigned)samus_x_pos,
         (unsigned)samus_y_pos, end_feet, drop, state->samus.on_ground);
  expect_true("walking off the deck descends", drop >= kMinDeckDrop);
  expect_true("lands on the terrain below the deck",
              end_feet >= kPicoLsRoomFloorTopRow - 1);
  expect_true("ends standing on something", state->samus.on_ground);
  MiniDestroy(state);
  return drop;
}

static void check_packed_room(void) {
  MiniGameState *state;
  MiniRoomInfo info;
  MiniCollisionMapView view;
  int by;

  expect_true("packed Landing Site room installs", PicoLsRoom_Install());
  state = MiniCreate(kViewportW, kViewportH);
  if (state == NULL) {
    fprintf(stderr, "FAIL packed room: MiniCreate\n");
    g_failures++;
    return;
  }
  MiniStubs_GetRoomInfo(&info);
  MiniStubs_GetCollisionMapView(&view);
  printf("packed room %s src=%s %dx%d blocks spawn=%d,%d (%zu B of level+bts)\n",
         info.room_handle, MiniStubs_RoomSourceName(info.room_source),
         info.room_width_blocks, info.room_height_blocks, info.spawn_x,
         info.spawn_y, PicoLsRoom_PackedSize());

  expect_true("mini no longer runs the fallback room",
              info.room_source != kMiniRoomSource_Fallback);
  expect_true("room is Landing Site 0x91F8", info.room_id == 0x91F8);
  expect_true("room is 144x80 blocks",
              view.width_blocks == 144 && view.height_blocks == 80);
  expect_true("spawn is the room's, not the fallback's",
              info.spawn_x == (int)kPicoLsRoomSpawnX &&
                  info.spawn_y == (int)kPicoLsRoomSpawnY);
  expect_true("samus boots at the room spawn",
              samus_x_pos == (uint16)kPicoLsRoomSpawnX &&
                  samus_y_pos == (uint16)kPicoLsRoomSpawnY);

  /* The three measured facts the fall depends on. */
  expect_true("spawn block is air",
              MiniStubs_GetCollisionMaterial(kPicoLsRoomSpawnBlockX,
                                             kPicoLsRoomSpawnBlockY) ==
                  kBlockType_Air);
  expect_true("ship deck under the spawn is solid 0x80FF",
              MiniStubs_GetLevelBlock(kPicoLsRoomSpawnBlockX, 71) == 0x80FF);
  for (by = kPicoLsRoomFloorTopRow; by < 80; by++) {
    expect_true("terrain floor rows are solid",
                MiniStubs_GetCollisionMaterial(kPicoLsRoomSpawnBlockX, by) ==
                    kBlockType_Solid);
  }
  expect_true("there is open air between the deck and the terrain",
              MiniStubs_GetCollisionMaterial(kPicoLsRoomSpawnBlockX, 74) ==
                  kBlockType_Air &&
                  MiniStubs_GetCollisionMaterial(kPicoLsRoomSpawnBlockX, 75) ==
                      kBlockType_Air);
  MiniDestroy(state);

  (void)walk_off_deck("walk right off the deck", kButton_Right);
  (void)walk_off_deck("walk left off the deck", kButton_Left);

  /* Control: take the packed room away and mini is back on its flat fallback
   * floor, which is what held Samus up on the panel before this landed. */
  PicoLsRoom_Uninstall();
  state = MiniCreate(kViewportW, kViewportH);
  if (state != NULL) {
    MiniStubs_GetRoomInfo(&info);
    expect_true("without the packed room mini falls back",
                info.room_source == kMiniRoomSource_Fallback);
    expect_true("the fallback room is not Landing Site sized",
                info.room_width_blocks != 144);
    MiniDestroy(state);
  }
  expect_true("packed room re-installs", PicoLsRoom_Install());
}

int main(void) {
  int pose_log[64];
  int seen_poses = 0;
  static PicoFramePacket src;
  static PicoFramePacket dst;
  static uint8_t game_vram[kPicoVramSize];
  static uint8_t display_vram[kPicoVramSize];
  static uint8_t wire[kPicoFrameWireMaxSize];
  static uint16_t frame_start[kPicoScreenHeight * kPicoScreenWidth];
  static uint16_t frame_end[kPicoScreenHeight * kPicoScreenWidth];
  static uint16_t frame_bg[kPicoScreenHeight * kPicoScreenWidth];
  MiniGameState *state;
  uint16 start_x;
  uint16 end_x;
  size_t n_first;
  size_t n_live;
  SpriteStats start_spr;
  SpriteStats end_spr;
  int i;

  state = MiniCreate(kViewportW, kViewportH);
  if (state == NULL) {
    fprintf(stderr, "test_pico_move_tileset: MiniCreate failed\n");
    return 1;
  }

  start_x = samus_x_pos;
  PicoFramePacket_InitLandingSiteExtracted(&src);
  expect_true("ls vram", src.vram != NULL && src.vram_size == (size_t)kPicoVramSize);
  memcpy(game_vram, src.vram, src.vram_size);
  src.vram = game_vram;
  expect_true("plant samus gfx",
              PicoOam_PlantSamusFrame(&src, PicoOam_SamusFrameIndex(1, 0)));

  pack_from_samus(&src, 1);
  n_first = PicoFrameWire_Encode(&src, wire, sizeof(wire), 1);
  expect_true("encode first with vram", n_first == PicoFrameWire_EncodedSize(&src, 1));
  expect_true("decode first",
              PicoFrameWire_Decode(wire, n_first, &dst, display_vram, sizeof(display_vram)));
  raster_packet(&dst, frame_start);
  raster_bg_only(&dst, frame_bg);
  start_spr = measure_sprite(frame_start, frame_bg);

  for (i = 0; i < kMoveFrames; i++) {
    MiniStepButtons(state, kButton_Right, false);
    pack_from_samus(&src, (uint32_t)(i + 2));
    if (seen_poses < (int)(sizeof(pose_log) / sizeof(pose_log[0]))) {
      if (seen_poses == 0 || pose_log[seen_poses - 1] != (int)samus_pose)
        pose_log[seen_poses++] = (int)samus_pose;
    }
    n_live = PicoFrameWire_Encode(&src, wire, sizeof(wire), 0);
    expect_true("live packet is header+oam+cgram",
                n_live == (size_t)kPicoFrameWireBaseSize);
    expect_true("live packet smaller than framebuffer",
                n_live > 0 && n_live < (size_t)kFramebufferBytes);
    expect_true("decode live retained vram",
                PicoFrameWire_Decode(wire, n_live, &dst, display_vram, sizeof(display_vram)));
    expect_true("display kept vram pointer", dst.vram == display_vram);
  }

  end_x = samus_x_pos;
  raster_packet(&dst, frame_end);
  raster_bg_only(&dst, frame_bg);
  end_spr = measure_sprite(frame_end, frame_bg);

  expect_true("samus_x increased", end_x > start_x);
  expect_true("start sprite pixels", start_spr.count >= 80);
  expect_true("end sprite pixels", end_spr.count >= 80);
  expect_true("sprite moved right", end_spr.min_x > start_spr.min_x);
  expect_true("start sprite over non-black BG", start_spr.over_bg > 0);
  expect_true("end sprite over non-black BG", end_spr.over_bg > 0);
  expect_true("first packet carries vram blob",
              n_first > (size_t)kPicoFrameWireBaseSize);

  (void)write_ppm("out/pico_move_start.ppm", frame_start);
  (void)write_ppm("out/pico_move_end.ppm", frame_end);

  printf("samus_x before=%u after=%u dy=%d\n",
         (unsigned)start_x, (unsigned)end_x, (int)end_x - (int)start_x);
  printf("samus_y=%u camera=%u,%u radius=%u,%u\n",
         (unsigned)samus_y_pos, (unsigned)layer1_x_pos, (unsigned)layer1_y_pos,
         (unsigned)samus_x_radius, (unsigned)samus_y_radius);
  printf("packet first=%zu live=%zu framebuffer=%d\n",
         n_first, n_live, kFramebufferBytes);
  /* Turn around: the sim must reach a left-facing pose, and every pose it
   * visits must be one the packer covers, or Samus falls back to standing. */
  for (i = 0; i < kMoveFrames; i++) {
    MiniStepButtons(state, kButton_Left, false);
    if (seen_poses < (int)(sizeof(pose_log) / sizeof(pose_log[0]))) {
      if (pose_log[seen_poses - 1] != (int)samus_pose)
        pose_log[seen_poses++] = (int)samus_pose;
    }
  }
  printf("right-then-left visited %d poses:", seen_poses);
  for (i = 0; i < seen_poses; i++)
    printf(" %#04x%s", pose_log[i],
           PicoOam_SamusPoseIsPacked(pose_log[i]) ? "" : "(UNPACKED)");
  printf("\n");
  {
    int all_packed = 1;
    int saw_left = 0;
    for (i = 0; i < seen_poses; i++) {
      if (!PicoOam_SamusPoseIsPacked(pose_log[i]))
        all_packed = 0;
      /* Left-facing / left-moving poses are the odd-numbered partners. */
      if (pose_log[i] == 2 || pose_log[i] == 0x0A || pose_log[i] == 0x0C ||
          pose_log[i] == 0x26)
        saw_left = 1;
    }
    expect_true("turning reaches a left-facing pose", saw_left);
    expect_true("every visited pose is packed", all_packed);
    expect_true("direction change is visible", seen_poses >= 2);
  }
  /* The display-side clock must actually cycle a run pose. */
  {
    int a = PicoOam_SamusFrameIndex(9, 0);
    int b = PicoOam_SamusFrameIndex(9, 5);
    expect_true("run cycle spans distinct frames", a != b);
  }
  printf("sprite min_x before=%d after=%d count=%d over_bg=%d\n",
         start_spr.min_x, end_spr.min_x, end_spr.count, end_spr.over_bg);

  MiniDestroy(state);

  /* Phase 2: the packed gameplay room. Installed last so everything above
   * still exercises the path mini takes with no room registered. */
  check_packed_room();

  if (g_failures) {
    fprintf(stderr, "%d failure(s)\n", g_failures);
    return 1;
  }
  puts("ok");
  return 0;
}
