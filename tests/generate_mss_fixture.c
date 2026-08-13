#include <stdio.h>
#include <stdlib.h>

#include "mini/mini_game.h"
#include "mini/mini_wram_peek.h"
#include "variables.h"

// Tool to generate a MiniSaveState blob fixture
// Usage: ./generate_mss_fixture output.mss

static void SetWramU16(uint16 addr, uint16 value) {
  g_ram[addr] = value & 0xFF;
  g_ram[addr + 1] = (value >> 8) & 0xFF;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s output.mss\n", argv[0]);
    return 1;
  }

  const char *output_path = argv[1];

  // Create a Mini state
  MiniGameState *state = MiniCreate(320, 240);
  if (!state) {
    fprintf(stderr, "Failed to create Mini state\n");
    return 1;
  }

  // Step one frame to initialize state properly
  MiniStepButtons(state, 0, false);

  // Set known WRAM values for testing
  SetWramU16(kWramAddr_SamusX, 128);
  SetWramU16(kWramAddr_SamusXSub, 0);
  SetWramU16(kWramAddr_SamusY, 176);
  SetWramU16(kWramAddr_SamusYSub, 0);
  SetWramU16(kWramAddr_SamusPose, 0);
  SetWramU16(kWramAddr_RoomId, 0x91F8);  // Landing Site
  SetWramU16(kWramAddr_Health, 99);

  // Update state to match WRAM
  state->samus.world_x = 128;
  state->samus.world_y = 176;
  state->samus.pose = 0;
  state->room.room_id = 0x91F8;

  // Save to file
  size_t snapshot_size = MiniSaveStateSize();
  void *snapshot = malloc(snapshot_size);
  if (!snapshot) {
    fprintf(stderr, "Failed to allocate snapshot buffer\n");
    MiniDestroy(state);
    return 1;
  }

  if (!MiniSaveState(state, snapshot, snapshot_size)) {
    fprintf(stderr, "Failed to save state\n");
    free(snapshot);
    MiniDestroy(state);
    return 1;
  }

  FILE *f = fopen(output_path, "wb");
  if (!f) {
    fprintf(stderr, "Failed to open output file: %s\n", output_path);
    free(snapshot);
    MiniDestroy(state);
    return 1;
  }

  size_t written = fwrite(snapshot, 1, snapshot_size, f);
  fclose(f);

  if (written != snapshot_size) {
    fprintf(stderr, "Failed to write complete snapshot\n");
    free(snapshot);
    MiniDestroy(state);
    return 1;
  }

  printf("Generated MiniSaveState fixture: %s (%zu bytes)\n", output_path, snapshot_size);
  printf("  samus_x = 128, samus_y = 176\n");
  printf("  room_id = 0x91F8 (Landing Site)\n");

  free(snapshot);
  MiniDestroy(state);
  return 0;
}
