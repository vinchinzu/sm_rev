#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ida_types.h"
#include "mini/mini_defs.h"
#include "mini/mini_game.h"
#include "mini/stubs_mini.h"
#include "types.h"

enum {
  kStressFrameCount = 360,
  kRepeatedCycleCount = 16,
  kRepeatedCycleWindow = 24,
  kProjectileSearchFrames = 60,
  kProjectileAdvanceFrames = 6,
  kRomRoomFrameCount = 120,
};

static void Require(bool condition, const char *message) {
  if (!condition) {
    fprintf(stderr, "%s\n", message);
    exit(1);
  }
}

static uint8 *AllocSnapshot(size_t *snapshot_size) {
  *snapshot_size = MiniSaveStateSize();
  uint8 *snapshot = (uint8 *)malloc(*snapshot_size);
  Require(snapshot != NULL, "snapshot allocation failed");
  return snapshot;
}

static void SaveSnapshot(const MiniGameState *state, uint8 *snapshot, size_t snapshot_size) {
  Require(MiniSaveState(state, snapshot, snapshot_size), "MiniSaveState failed");
}

static void LoadSnapshot(MiniGameState *state, const uint8 *snapshot, size_t snapshot_size) {
  Require(MiniLoadState(state, snapshot, snapshot_size), "MiniLoadState failed");
}

static uint64_t StepShortSequenceAndHash(MiniGameState *state) {
  static const uint16 kInputs[] = {
    kButton_Right,
    kButton_Right | kButton_A,
    kButton_Right,
  };
  for (size_t i = 0; i < sizeof(kInputs) / sizeof(kInputs[0]); i++)
    MiniStepButtons(state, kInputs[i], false);
  return MiniStateHash(state);
}

static uint16 StressInputForFrame(int frame) {
  uint16 buttons = 0;
  switch ((frame / 18) % 4) {
  case 0:
    buttons |= kButton_Right;
    break;
  case 1:
    buttons |= kButton_Right | kButton_A;
    break;
  case 2:
    buttons |= kButton_Left;
    break;
  default:
    break;
  }
  if (frame % 29 == 5)
    buttons |= kButton_X;
  if (frame % 47 >= 10 && frame % 47 < 20)
    buttons |= kButton_B;
  return buttons;
}

static uint16 RomRoomInputForFrame(int frame) {
  switch ((frame / 20) % 4) {
  case 0:
    return kButton_Right;
  case 1:
    return kButton_Right | kButton_A;
  case 2:
    return 0;
  default:
    return kButton_Left;
  }
}

static uint64_t StepStressFrames(MiniGameState *state, int frames) {
  for (int i = 0; i < frames; i++)
    MiniStepButtons(state, StressInputForFrame(state->frame), false);
  return MiniStateHash(state);
}

static uint64_t StepRomRoomFrames(MiniGameState *state, int frames) {
  for (int i = 0; i < frames; i++)
    MiniStepButtons(state, RomRoomInputForFrame(state->frame), false);
  return MiniStateHash(state);
}

static const SamusProjectileView *FirstActiveProjectile(const MiniGameState *state) {
  for (int i = 0; i < kMiniProjectileViewCapacity; i++) {
    if (state->projectiles[i].active)
      return &state->projectiles[i];
  }
  return NULL;
}

static uint64_t StepProjectileAdvanceFrames(MiniGameState *state) {
  for (int i = 0; i < kProjectileAdvanceFrames; i++)
    MiniStepButtons(state, kButton_Right, false);
  return MiniStateHash(state);
}

static void RequireFallbackRoom(const MiniGameState *state) {
  Require(state->has_room, "fallback mini state did not configure a room");
  Require(!state->uses_rom_room, "fallback mini test unexpectedly booted a ROM room");
  Require(state->room_source == kMiniRoomSource_Fallback,
          "fallback mini test did not use the fallback room");
}

static void TestBasicRollbackApi(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed");
  RequireFallbackRoom(state);

  MiniStepButtons(state, 0, false);
  MiniStepButtons(state, kButton_Right, false);
  uint64_t rewind_hash = MiniStateHash(state);

  size_t snapshot_size;
  uint8 *snapshot = AllocSnapshot(&snapshot_size);
  SaveSnapshot(state, snapshot, snapshot_size);
  Require(!MiniSaveState(state, snapshot, snapshot_size - 1),
          "MiniSaveState accepted a short buffer");

  uint64_t after_hash = StepShortSequenceAndHash(state);
  LoadSnapshot(state, snapshot, snapshot_size);
  Require(MiniStateHash(state) == rewind_hash, "loaded state hash did not match saved state");
  Require(StepShortSequenceAndHash(state) == after_hash, "re-simulated state hash did not match");

  snapshot[0] ^= 1;
  Require(!MiniLoadState(state, snapshot, snapshot_size),
          "MiniLoadState accepted a corrupted snapshot");

  free(snapshot);
  MiniDestroy(state);
}

static void TestLongScriptDeterminism(void) {
  MiniGameState *first = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(first != NULL, "MiniCreate failed for first long script test");
  RequireFallbackRoom(first);
  uint64_t first_hash = StepStressFrames(first, kStressFrameCount);
  MiniDestroy(first);

  MiniGameState *second = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(second != NULL, "MiniCreate failed for second long script test");
  RequireFallbackRoom(second);
  uint64_t second_hash = StepStressFrames(second, kStressFrameCount);
  Require(first_hash == second_hash, "long scripted mini run was not deterministic");
  Require(second->frame == kStressFrameCount, "long scripted mini run ended on the wrong frame");

  MiniDestroy(second);
}

static void TestRepeatedSaveLoadCycles(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for repeated save/load test");
  RequireFallbackRoom(state);

  size_t snapshot_size;
  uint8 *snapshot = AllocSnapshot(&snapshot_size);
  for (int cycle = 0; cycle < kRepeatedCycleCount; cycle++) {
    StepStressFrames(state, 7 + cycle);
    uint64_t saved_hash = MiniStateHash(state);
    SaveSnapshot(state, snapshot, snapshot_size);

    uint64_t branch_hash = StepStressFrames(state, kRepeatedCycleWindow);
    LoadSnapshot(state, snapshot, snapshot_size);
    Require(MiniStateHash(state) == saved_hash,
            "repeated MiniLoadState did not restore the saved hash");
    Require(StepStressFrames(state, kRepeatedCycleWindow) == branch_hash,
            "repeated rollback branch did not replay deterministically");
  }

  free(snapshot);
  MiniDestroy(state);
}

static void TestProjectileRollbackProgression(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for projectile rollback test");
  RequireFallbackRoom(state);

  for (int frame = 0; frame < kProjectileSearchFrames && state->projectile_count == 0; frame++) {
    MiniStepButtons(state, kButton_Right | kButton_X, false);
  }

  const SamusProjectileView *projectile = FirstActiveProjectile(state);
  Require(projectile != NULL, "shooting script did not spawn a projectile");
  Require(projectile->is_beam, "shooting script spawned a non-beam projectile");
  Require(projectile->is_basic_beam, "shooting script did not spawn a basic beam projectile");
  uint16 projectile_slot = projectile->slot_index;
  uint16 projectile_type = projectile->type;

  size_t snapshot_size;
  uint8 *snapshot = AllocSnapshot(&snapshot_size);
  uint64_t saved_hash = MiniStateHash(state);
  SaveSnapshot(state, snapshot, snapshot_size);

  uint64_t advanced_hash = StepProjectileAdvanceFrames(state);
  projectile = FirstActiveProjectile(state);
  Require(projectile != NULL, "projectile disappeared during short progression window");
  Require(projectile->slot_index == projectile_slot, "projectile changed slots unexpectedly");
  Require(projectile->type == projectile_type, "projectile type changed unexpectedly");
  Require(advanced_hash != saved_hash, "projectile progression window did not advance state");

  LoadSnapshot(state, snapshot, snapshot_size);
  Require(MiniStateHash(state) == saved_hash, "projectile snapshot did not restore saved hash");
  Require(StepProjectileAdvanceFrames(state) == advanced_hash,
          "projectile progression did not replay deterministically after load");

  free(snapshot);
  MiniDestroy(state);
}

static void RunFallbackTestsInIsolatedCwd(void) {
  char original_cwd[1024];
  char temp_dir[] = "/tmp/sm_rev_mini_rollback_api_XXXXXX";
  Require(getcwd(original_cwd, sizeof(original_cwd)) != NULL, "getcwd failed");
  Require(mkdtemp(temp_dir) != NULL, "mkdtemp failed");
  Require(chdir(temp_dir) == 0, "chdir to fallback temp dir failed");

  MiniStubs_SetRoomExportPath("missing_room.json");
  TestBasicRollbackApi();
  TestProjectileRollbackProgression();
  TestLongScriptDeterminism();
  TestRepeatedSaveLoadCycles();
  MiniStubs_SetRoomExportPath(NULL);

  Require(chdir(original_cwd) == 0, "failed to restore original cwd");
  (void)rmdir(temp_dir);
}

static void TestRomRoomDeterminismIfAvailable(void) {
  MiniStubs_SetRoomExportPath(NULL);
  MiniGameState *first = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(first != NULL, "MiniCreate failed for ROM room determinism test");
  if (!first->uses_rom_room) {
    MiniDestroy(first);
    return;
  }
  Require(first->uses_original_gameplay_runtime,
          "ROM room path did not enable original gameplay runtime");
  char first_room_handle[sizeof(first->room_handle)];
  memcpy(first_room_handle, first->room_handle, sizeof(first_room_handle));
  uint64_t first_hash = StepRomRoomFrames(first, kRomRoomFrameCount);
  MiniDestroy(first);

  MiniGameState *second = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(second != NULL, "MiniCreate failed for second ROM room determinism test");
  Require(second->uses_rom_room, "second ROM room boot did not use a ROM room");
  Require(memcmp(first_room_handle, second->room_handle, sizeof(first_room_handle)) == 0,
          "ROM room boot selected different room handles");

  uint64_t second_hash = StepRomRoomFrames(second, kRomRoomFrameCount);
  Require(first_hash == second_hash, "ROM-backed room path was not deterministic");

  size_t snapshot_size;
  uint8 *snapshot = AllocSnapshot(&snapshot_size);
  SaveSnapshot(second, snapshot, snapshot_size);
  uint64_t branch_hash = StepRomRoomFrames(second, kRepeatedCycleWindow);
  LoadSnapshot(second, snapshot, snapshot_size);
  Require(StepRomRoomFrames(second, kRepeatedCycleWindow) == branch_hash,
          "ROM-backed rollback branch did not replay deterministically");

  free(snapshot);
  MiniDestroy(second);
}

int main(void) {
  RunFallbackTestsInIsolatedCwd();
  TestRomRoomDeterminismIfAvailable();
  return 0;
}
