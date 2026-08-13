#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "mini/mini_game.h"
#include "mini/mini_predict.h"
#include "mini/mini_wram_peek.h"
#include "variables.h"

static uint16 ReadU16LE(const uint8 *data, size_t offset) {
  return (uint16)(data[offset] | (data[offset + 1] << 8));
}

static void TestLoadMiniSaveState(void) {
  printf("TestLoadMiniSaveState: ");
  
  // Load MiniSaveState blob
  FILE *f = fopen("tests/fixtures/landing_site_spawn.mss", "rb");
  assert(f != NULL);
  
  fseek(f, 0, SEEK_END);
  long file_size = ftell(f);
  fseek(f, 0, SEEK_SET);
  
  assert(file_size == (long)MiniSaveStateSize());
  
  void *snapshot = malloc(MiniSaveStateSize());
  assert(snapshot != NULL);
  
  size_t read_size = fread(snapshot, 1, MiniSaveStateSize(), f);
  fclose(f);
  
  assert(read_size == MiniSaveStateSize());
  
  // Load into a Mini state
  MiniGameState *state = MiniCreate(320, 240);
  assert(state != NULL);
  
  bool success = MiniLoadState(state, snapshot, MiniSaveStateSize());
  assert(success);
  
  // Verify the WRAM was loaded correctly
  // The fixture should have: x=128, y=176, room=0x91F8
  uint16 samus_x = ReadU16LE(g_ram, kWramAddr_SamusX);
  uint16 samus_y = ReadU16LE(g_ram, kWramAddr_SamusY);
  uint16 room_id = ReadU16LE(g_ram, kWramAddr_RoomId);
  
  assert(samus_x == 128);
  assert(samus_y == 176);
  assert(room_id == 0x91F8);
  
  free(snapshot);
  MiniDestroy(state);
  
  printf("PASSED\n");
}

static void TestPredictFromLoadedState(void) {
  printf("TestPredictFromLoadedState (frame-0 hydrate only): ");
  
  // Load MiniSaveState blob
  FILE *f = fopen("tests/fixtures/landing_site_spawn.mss", "rb");
  assert(f != NULL);
  
  void *snapshot = malloc(MiniSaveStateSize());
  assert(snapshot != NULL);
  
  size_t read_size = fread(snapshot, 1, MiniSaveStateSize(), f);
  fclose(f);
  assert(read_size == MiniSaveStateSize());
  
  // Get initial position from snapshot's g_ram
  MiniGameState *temp_state = MiniCreate(320, 240);
  assert(temp_state != NULL);
  assert(MiniLoadState(temp_state, snapshot, MiniSaveStateSize()));
  
  uint16 initial_x = ReadU16LE(g_ram, kWramAddr_SamusX);
  uint16 initial_x_sub = ReadU16LE(g_ram, kWramAddr_SamusXSub);
  uint16 initial_y = ReadU16LE(g_ram, kWramAddr_SamusY);
  uint16 initial_y_sub = ReadU16LE(g_ram, kWramAddr_SamusYSub);
  
  MiniDestroy(temp_state);
  
  // Run prediction: 60 frames of Right button (0x100 in Mini format)
  const int frame_count = 60;
  uint16 buttons[60];
  for (int i = 0; i < frame_count; i++) {
    buttons[i] = 0x100;  // Mini button format: Right
  }
  
  MiniPrediction *prediction = MiniPrediction_Create(frame_count);
  assert(prediction != NULL);
  
  bool success = MiniPredict(
    prediction,
    snapshot,
    MiniSaveStateSize(),
    buttons,
    frame_count,
    320,
    240
  );
  assert(success);
  
  free(snapshot);
  
  // TEST SCOPE: Frame-0 hydrate only
  // This test verifies that --load-state correctly hydrates g_ram at frame 0.
  // It does NOT test grounded-walk residuals vs SuperMetroidEnv (that's later work).
  // Mini authored movement may not produce position changes without proper room setup.
  
  assert(prediction->frame_count > 0);
  const MiniTrajectoryFrame *frame0 = &prediction->frames[0];
  
  printf("  Frame 0: x=%d.%d, y=%d.%d\n", 
         frame0->samus_x, frame0->samus_x_sub,
         frame0->samus_y, frame0->samus_y_sub);
  printf("  Expected: x=%d.%d, y=%d.%d\n",
         initial_x, initial_x_sub, initial_y, initial_y_sub);
  
  // Assert frame 0 matches the blob's g_ram values
  assert(frame0->samus_x == initial_x);
  assert(frame0->samus_x_sub == initial_x_sub);
  assert(frame0->samus_y == initial_y);
  assert(frame0->samus_y_sub == initial_y_sub);
  
  printf("PASSED (frame-0 hydrate verified, grounded-walk R(τ) vs SuperMetroidEnv is later work)\n");
  
  MiniPrediction_Destroy(prediction);
}

int main(void) {
  printf("=== Mini MiniSaveState Load Tests ===\n");
  
  TestLoadMiniSaveState();
  TestPredictFromLoadedState();
  
  printf("=== All tests PASSED ===\n");
  return 0;
}
