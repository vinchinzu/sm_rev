/*
 * Scripted MiniStepButtons feel on an authored room (no ROM).
 *
 *   make pico-feel-test
 *
 * MiniAuthoredMovement has bombs (X+Down), not beams. Slope tiles are
 * JSON materials "slope" plus a bts grid. BTS shape >= 5 uses the packed
 * kAlignYPos_Tab0 copy; this toy ramp is BTS 0 and still interpolates
 * same-MirrorX segments.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ida_types.h"
#include "mini/mini_editor_path.h"
#include "mini/mini_game.h"
#include "variables.h"

enum {
  kFeelViewportW = 256,
  kFeelViewportH = 224,
  kFeelRoomWidth = 48,
  kFeelRoomHeight = 16,
  kFeelFloorY = 13,
  kFeelSlopeY = 12,
  kFeelSlopeX0 = 6,
  kFeelSlopeX1 = 14,
  kFeelTunnelY = 11,
  kFeelTunnelX0 = 24,
  kFeelTunnelX1 = 38,
  kFeelSpawnX = 64,
  kFeelSpawnY = 192,
  kFeelRunFrames = 30,
  kFeelSpinRunup = 3,
  kFeelSlopeFrames = 28,
  kFeelTunnelFrames = 200,
  kFeelRoomId = 0x91F8
};

static int g_failures;
static char g_feel_room_path[256];

static void expect_true(const char *name, int cond) {
  if (!cond) {
    fprintf(stderr, "FAIL %s\n", name);
    g_failures++;
    return;
  }
  printf("PASS %s\n", name);
}

static const char *feel_cell_material(int x, int y) {
  if (x == 0 || x == kFeelRoomWidth - 1 || y >= kFeelFloorY)
    return "solid";
  if (y == kFeelTunnelY && x >= kFeelTunnelX0 && x < kFeelTunnelX1)
    return "solid";
  if (y == kFeelSlopeY && x >= kFeelSlopeX0 && x < kFeelSlopeX1)
    return "slope";
  return "air";
}

static bool write_feel_room(void) {
  FILE *f;
  int x;
  int y;

  snprintf(g_feel_room_path, sizeof(g_feel_room_path), "out/pico_feel_room.json");
  f = fopen(g_feel_room_path, "w");
  if (f == NULL) {
    snprintf(g_feel_room_path, sizeof(g_feel_room_path), "/tmp/pico_feel_room.json");
    f = fopen(g_feel_room_path, "w");
  }
  if (f == NULL)
    return false;

  fprintf(f,
          "{\n"
          "  \"roomId\": %d,\n"
          "  \"handle\": \"picoFeel\",\n"
          "  \"name\": \"Pico Feel Authored Room\",\n"
          "  \"widthScreens\": %d,\n"
          "  \"heightScreens\": %d,\n"
          "  \"widthBlocks\": %d,\n"
          "  \"heightBlocks\": %d,\n"
          "  \"materials\": [\n",
          kFeelRoomId, kFeelRoomWidth / 16, kFeelRoomHeight / 16, kFeelRoomWidth,
          kFeelRoomHeight);
  for (y = 0; y < kFeelRoomHeight; y++) {
    fputs("    [", f);
    for (x = 0; x < kFeelRoomWidth; x++) {
      fprintf(f, "\"%s\"%s", feel_cell_material(x, y),
              x + 1 < kFeelRoomWidth ? ", " : "");
    }
    fprintf(f, "]%s\n", y + 1 < kFeelRoomHeight ? "," : "");
  }
  fputs("  ],\n  \"bts\": [\n", f);
  for (y = 0; y < kFeelRoomHeight; y++) {
    fputs("    [", f);
    for (x = 0; x < kFeelRoomWidth; x++)
      fprintf(f, "0%s", x + 1 < kFeelRoomWidth ? ", " : "");
    fprintf(f, "]%s\n", y + 1 < kFeelRoomHeight ? "," : "");
  }
  fprintf(f,
          "  ],\n"
          "  \"camera\": {\n"
          "    \"spawnX\": %d,\n"
          "    \"spawnY\": %d,\n"
          "    \"cameraX\": 0,\n"
          "    \"cameraY\": 32\n"
          "  }\n"
          "}\n",
          kFeelSpawnX, kFeelSpawnY);
  fclose(f);
  return true;
}

static void hold_buttons(MiniGameState *state, uint16 buttons, int frames) {
  int i;
  for (i = 0; i < frames; i++)
    MiniStepButtons(state, buttons, false);
}

static MiniGameState *create_feel_state(void) {
  MiniGameState *state;

  MiniEditorPath_SetRoomExportPath(g_feel_room_path);
  MiniStubs_SetRoomExportPath(g_feel_room_path);
  state = MiniCreate(kFeelViewportW, kFeelViewportH);
  if (state == NULL)
    return NULL;
  if (state->room.room_source != kMiniRoomSource_EditorExport) {
    fprintf(stderr, "test_pico_feel: expected editor_export, got source=%d handle=%s\n",
            (int)state->room.room_source, state->room.room_handle);
    MiniDestroy(state);
    return NULL;
  }
  return state;
}

static void test_run(void) {
  MiniGameState *state = create_feel_state();
  uint16 start_x;
  uint16 end_x;

  if (state == NULL) {
    expect_true("run: MiniCreate authored room", 0);
    return;
  }
  start_x = samus_x_pos;
  hold_buttons(state, kButton_Right, kFeelRunFrames);
  end_x = samus_x_pos;
  printf("run start_x=%u end_x=%u pose=%u movement=%u\n",
         (unsigned)start_x, (unsigned)end_x, (unsigned)samus_pose,
         (unsigned)samus_movement_type);
  expect_true("run: x increases", end_x > start_x);
  MiniDestroy(state);
}

static void test_spin(void) {
  MiniGameState *state = create_feel_state();
  int i;
  uint16 pose;
  uint16 movement;

  if (state == NULL) {
    expect_true("spin: MiniCreate authored room", 0);
    return;
  }
  hold_buttons(state, kButton_Right, kFeelSpinRunup);
  MiniStepButtons(state, kButton_Right | kButton_A, false);
  pose = samus_pose;
  movement = samus_movement_type;
  printf("spin pose=%u movement=%u on_ground=%d y=%u\n",
         (unsigned)pose, (unsigned)movement, (int)state->samus.on_ground,
         (unsigned)samus_y_pos);
  expect_true("spin: pose or movement is spin/jump after A while running",
              movement == kMovementType_03_SpinJumping ||
                  pose == kPose_19_FaceR_SpinJump ||
                  pose == kPose_1A_FaceL_SpinJump);
  for (i = 0; i < 4; i++)
    MiniStepButtons(state, kButton_Right, false);
  expect_true("spin: still airborne spin shortly after jump",
              samus_movement_type == kMovementType_03_SpinJumping ||
                  !state->samus.on_ground);
  MiniDestroy(state);
}

static void test_morph(void) {
  MiniGameState *state = create_feel_state();
  uint16 stand_x;
  uint16 morph_x;
  uint16 pose;
  uint16 movement;

  if (state == NULL) {
    expect_true("morph: MiniCreate authored room", 0);
    return;
  }
  MiniStepButtons(state, kButton_Down, false);
  pose = samus_pose;
  movement = samus_movement_type;
  printf("morph pose=%u movement=%u y_radius=%u y=%u\n",
         (unsigned)pose, (unsigned)movement, (unsigned)samus_y_radius,
         (unsigned)samus_y_pos);
  expect_true("morph: Down on ground sets morph pose/movement_type",
              movement == kMovementType_04_MorphBallOnGround ||
                  pose == kPose_1D_FaceR_Morphball_Ground ||
                  pose == kPose_41_FaceL_Morphball_Ground);
  hold_buttons(state, kButton_Right, kFeelTunnelFrames - 1);
  morph_x = samus_x_pos;
  MiniDestroy(state);

  state = create_feel_state();
  if (state == NULL) {
    expect_true("morph tunnel: standing MiniCreate", 0);
    return;
  }
  hold_buttons(state, kButton_Right, kFeelTunnelFrames);
  stand_x = samus_x_pos;
  printf("morph tunnel stand_x=%u morph_x=%u ceiling=[%d,%d)\n",
         (unsigned)stand_x, (unsigned)morph_x, kFeelTunnelX0 * 16,
         kFeelTunnelX1 * 16);
  expect_true("morph tunnel: standing blocked by ceiling",
              stand_x < (uint16)(kFeelTunnelX0 * 16));
  expect_true("morph tunnel: ball passes through",
              morph_x > (uint16)(kFeelTunnelX1 * 16));
  MiniDestroy(state);
}

static void test_slope(void) {
  MiniGameState *state = create_feel_state();
  uint16 start_y;
  uint16 end_y;
  uint16 end_x;
  BlockType slope_tile;

  if (state == NULL) {
    expect_true("slope: MiniCreate authored room", 0);
    return;
  }
  slope_tile = MiniStubs_GetCollisionMaterial((kFeelSlopeX0 + kFeelSlopeX1) / 2,
                                              kFeelSlopeY);
  expect_true("slope: authored slope block present",
              slope_tile == kBlockType_Slope);
  start_y = samus_y_pos;
  hold_buttons(state, kButton_Right, kFeelSlopeFrames);
  end_x = samus_x_pos;
  end_y = samus_y_pos;
  printf("slope start_y=%u end_x=%u end_y=%u on_ground=%d\n",
         (unsigned)start_y, (unsigned)end_x, (unsigned)end_y,
         (int)state->samus.on_ground);
  expect_true("slope: walked onto ramp (x past slope start)",
              end_x > (uint16)(kFeelSlopeX0 * 16));
  expect_true("slope: y decreases while walking up", end_y < start_y);
  expect_true("slope: still on ground", state->samus.on_ground);
  MiniDestroy(state);
}

static void test_shoot(void) {
  MiniGameState *state = create_feel_state();

  if (state == NULL) {
    expect_true("shoot: MiniCreate authored room", 0);
    return;
  }
  MiniStepButtons(state, kButton_X, false);
  printf("shoot X-only projectile_type[0]=0x%x count=%d\n",
         (unsigned)projectile_type[0], state->projectile_state.count);
  expect_true("shoot: X alone does not spawn a beam (MiniAuthoredMovement has none)",
              projectile_type[0] == 0);

  MiniDestroy(state);
  state = create_feel_state();
  if (state == NULL) {
    expect_true("shoot bomb: MiniCreate", 0);
    return;
  }
  MiniStepButtons(state, kButton_Down, false);
  MiniStepButtons(state, kButton_Down | kButton_X, false);
  printf("shoot bomb projectile_type[0]=0x%x count=%d\n",
         (unsigned)projectile_type[0], state->projectile_state.count);
  expect_true("shoot: X+Down makes bomb slot live",
              projectile_type[0] == kProjectileType_Bomb);
  expect_true("shoot: projectile view count >= 1",
              state->projectile_state.count >= 1);
  MiniDestroy(state);
}

int main(void) {
  if (!write_feel_room()) {
    fprintf(stderr, "test_pico_feel: failed to write %s\n", g_feel_room_path);
    return 1;
  }
  printf("wrote %s\n", g_feel_room_path);

  test_run();
  test_spin();
  test_morph();
  test_slope();
  test_shoot();

  MiniEditorPath_SetRoomExportPath(NULL);
  MiniStubs_SetRoomExportPath(NULL);

  if (g_failures) {
    fprintf(stderr, "test_pico_feel: %d failure(s)\n", g_failures);
    return 2;
  }
  printf("test_pico_feel: all assertions passed\n");
  return 0;
}
