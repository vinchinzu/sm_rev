#include <stdio.h>
#include <stdlib.h>

#include "ida_types.h"
#include "mini/mini_defs.h"
#include "mini/mini_game.h"
#include "mini/stubs_mini.h"
#include "types.h"

static void Require(bool condition, const char *message) {
  if (!condition) {
    fprintf(stderr, "%s\n", message);
    exit(1);
  }
}

static uint64_t StepSequenceAndHash(MiniGameState *state) {
  static const uint16 kInputs[] = {
    kButton_Right,
    kButton_Right | kButton_A,
    kButton_Right,
  };
  for (size_t i = 0; i < sizeof(kInputs) / sizeof(kInputs[0]); i++)
    MiniStepButtons(state, kInputs[i], false);
  return MiniStateHash(state);
}

int main(void) {
  MiniStubs_SetRoomExportPath(NULL);

  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed");

  MiniStepButtons(state, 0, false);
  MiniStepButtons(state, kButton_Right, false);
  uint64_t rewind_hash = MiniStateHash(state);

  size_t snapshot_size = MiniSaveStateSize();
  uint8 *snapshot = (uint8 *)malloc(snapshot_size);
  Require(snapshot != NULL, "snapshot allocation failed");
  Require(MiniSaveState(state, snapshot, snapshot_size), "MiniSaveState failed");
  Require(!MiniSaveState(state, snapshot, snapshot_size - 1),
          "MiniSaveState accepted a short buffer");

  uint64_t after_hash = StepSequenceAndHash(state);
  Require(MiniLoadState(state, snapshot, snapshot_size), "MiniLoadState failed");
  Require(MiniStateHash(state) == rewind_hash, "loaded state hash did not match saved state");
  Require(StepSequenceAndHash(state) == after_hash, "re-simulated state hash did not match");

  snapshot[0] ^= 1;
  Require(!MiniLoadState(state, snapshot, snapshot_size),
          "MiniLoadState accepted a corrupted snapshot");

  free(snapshot);
  MiniDestroy(state);
  return 0;
}
