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
  
  // CRITICAL: Verify g_ram values immediately after MiniLoadState, BEFORE any step
  MiniGameState *state = MiniCreate(320, 240);
  assert(state != NULL);
  assert(MiniLoadState(state, snapshot, MiniSaveStateSize()));
  
  // Capture pre-step position from g_ram (this is the TRUE loaded state)
  uint16 loaded_x = ReadU16LE(g_ram, kWramAddr_SamusX);
  uint16 loaded_x_sub = ReadU16LE(g_ram, kWramAddr_SamusXSub);
  uint16 loaded_y = ReadU16LE(g_ram, kWramAddr_SamusY);
  uint16 loaded_y_sub = ReadU16LE(g_ram, kWramAddr_SamusYSub);
  
  printf("\n  Loaded from g_ram (pre-step): x=%d.%d, y=%d.%d\n", 
         loaded_x, loaded_x_sub, loaded_y, loaded_y_sub);
  
  MiniDestroy(state);
  
  // Run prediction: 60 frames of Right button (0x100 in Mini format)
  // REQUIRED A: When loading a snapshot, frames[0] is captured BEFORE any MiniStepButtons
  const int input_count = 60;
  uint16 buttons[60];
  for (int i = 0; i < input_count; i++) {
    buttons[i] = 0x100;  // Mini button format: Right
  }
  
  // Allocate space for pre-step frame (frame 0) plus all input frames
  MiniPrediction *prediction = MiniPrediction_Create(input_count + 1);
  assert(prediction != NULL);
  
  bool success = MiniPredict(
    prediction,
    snapshot,
    MiniSaveStateSize(),
    buttons,
    input_count,
    320,
    240
  );
  assert(success);
  
  free(snapshot);
  
  // TEST SCOPE: Frame-0 hydrate only
  // This test verifies that --load-state correctly captures frame 0 BEFORE stepping.
  // frames[0] is the pre-step state from loaded g_ram.
  // frames[1..N] are post-step states after each input.
  // This does NOT test grounded-walk residuals vs SuperMetroidEnv (that's later work).
  
  assert(prediction->frame_count == (size_t)(input_count + 1));  // pre-step + all inputs
  const MiniTrajectoryFrame *frame0 = &prediction->frames[0];
  
  printf("  frames[0] (pre-step): x=%d.%d, y=%d.%d\n", 
         frame0->samus_x, frame0->samus_x_sub,
         frame0->samus_y, frame0->samus_y_sub);
  
  // Frame 0 should exactly match loaded values (pre-step capture)
  assert(frame0->samus_x == loaded_x);
  assert(frame0->samus_x_sub == loaded_x_sub);
  assert(frame0->samus_y == loaded_y);
  assert(frame0->samus_y_sub == loaded_y_sub);
  
  printf("PASSED (frames[0] is pre-step state from loaded g_ram)\n");
  printf("Note: grounded-walk R(τ) vs SuperMetroidEnv is later work\n");
  
  MiniPrediction_Destroy(prediction);
}

int main(void) {
  printf("=== Mini MiniSaveState Load Tests ===\n");
  
  TestLoadMiniSaveState();
  TestPredictFromLoadedState();
  
  printf("=== All tests PASSED ===\n");
  return 0;
}
