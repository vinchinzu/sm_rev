# WRAM Peek Load - MiniSaveState Blobs

## Overview

The Mini physics kernel supports loading from **MiniSaveState blob** fixtures via the `--load-state` flag. This enables corresponding-start validation: Mini can hydrate from a serialized state snapshot with known WRAM values.

**Important**: Mini is a wind-tunnel for physics iteration, NOT a TAS-legal emulator. This load path is for testing and validation, not for claiming Mini = emu.

## MiniSaveState Blob Format

A MiniSaveState blob is a binary snapshot created by `MiniSaveState()` containing:

- Magic: `0x4D53534D` ('MSSM')
- Version: `4`
- Full `MiniGameState` structure
- Mini stub state (`MiniStubsSnapshot`)
- PPU state (`MiniPpuSnapshot`)
- **WRAM**: 128KB (`g_ram[0x20000]`)
- **SRAM**: 8KB (`g_sram[0x2000]`)
- Debug flags and counters

## Critical WRAM Addresses

Frame 0 validation checks these addresses in the blob's g_ram:

| Address | Size | Field | Description |
|---------|------|-------|-------------|
| `$0AF6` | uint16 | samus_x | Samus X position (pixels) |
| `$0AF8` | uint16 | samus_x_sub | Samus X subpixel (16-bit) |
| `$0AFA` | uint16 | samus_y | Samus Y position (pixels) |
| `$0AFC` | uint16 | samus_y_sub | Samus Y subpixel (16-bit) |
| `$0A1C` | uint16 | samus_pose | Samus pose |
| `$079B` | uint16 | room_id | Room ID |
| `$09C2` | uint16 | health | Health |

## CLI Usage

### Loading from Blob

```bash
sm_rev_predict --load-state path/to/snapshot.mss < input.json
```

The `--load-state` flag loads a MiniSaveState blob before prediction. Without this flag, prediction starts from Mini's fixed initial room state (legacy behavior).

### JSON stdin Format (unchanged)

```json
{
  "inputs": [
    {"buttons": 128},
    {"buttons": 128}
  ]
}
```

The `start` field in JSON stdin is echo-only. Mini does NOT hydrate from JSON `SimState`; it uses the binary MiniSaveState blob.

### Output Format

**Frame 0 is the pre-step state** from the loaded blob, captured BEFORE any `MiniStepButtons` call. Subsequent frames are post-step:

```json
{
  "frames": [
    {
      "frame": 0,
      "samus_x": 128,
      "samus_x_sub": 0,
      "samus_y": 176,
      "samus_y_sub": 0,
      ...
    },
    {
      "frame": 1,
      "samus_x": 128,  // after first input
      ...
    }
  ],
  "predictor": "sm_rev",
  "inputs": [...]
}
```

When `--load-state` is provided, the trajectory has `N+1` frames for `N` inputs:
- `frames[0]`: pre-step state (loaded blob)
- `frames[1..N]`: post-step states after inputs[0..N-1]

### Observation Tuple Fields

The observation tuple (Oσ†) includes:
- **Position**: `samus_x`, `samus_y`, `samus_x_sub`, `samus_y_sub` (from g_ram `$0AF6/$0AF8/$0AFA/$0AFC`)
- **Pose**: `pose` (from g_ram `$0A1C`)
- **Room**: `room_id` (from g_ram `$079B`)
- **Energy**: `energy` (from g_ram `$09C2`)
- **Frame counters**: `frame_counter_1`, `frame_counter_2` (from g_ram `$1842/$09DA`, both uint16)
- **Velocity/momentum**: `velocity_x`, `velocity_y`, `momentum_x`, etc.

**Not emitted** (hardcoded false, not measured):
- `is_dead`: Death state (field exists internally but not yet measured from game state)
- `is_game_over`: Game over state (field exists internally but not yet measured from game state)

These fields are documented in the struct but are NOT included in JSON output because they are not yet measured from WRAM or game state.

## Scope: Frame-0 Hydrate Only

**This PR implements frame-0 hydration only.** It verifies that:
1. `--load-state` correctly loads a MiniSaveState blob
2. Frame 0 observation tuple matches the blob's WRAM values (`$0AF6/$0AF8/$0AFA/$0AFC`)
3. Subsequent frames step from that loaded state

**This PR does NOT test**:
- Grounded-walk residuals R(τ) vs SuperMetroidEnv (that's later work)
- Movement distance validation (X may not change without proper room setup)
- Death/game-over state detection (fields exist but hardcoded false)

The C test (`tests/test_wram_peek_load.c`) is explicitly **frame-0 hydrate only**, not a grounded-walk residual test.

## Creating Fixtures

### Generate a MiniSaveState Blob

```c
// C code to create a fixture
MiniGameState *state = MiniCreate(320, 240);
MiniStepButtons(state, 0, false);  // Initialize

// Set WRAM values
g_ram[0x0AF6] = 128;  // samus_x
g_ram[0x0AF8] = 0;    // samus_x_sub
// ... set other addresses

// Save blob
size_t size = MiniSaveStateSize();
void *blob = malloc(size);
MiniSaveState(state, blob, size);

FILE *f = fopen("fixture.mss", "wb");
fwrite(blob, 1, size, f);
fclose(f);
```

### Using the Generator Tool

```bash
# Build and run the fixture generator
make generate_mss_fixture
./generate_mss_fixture tests/fixtures/my_fixture.mss
```

The generator creates a blob with:
- `samus_x = 128`, `samus_y = 176`
- `room_id = 0x91F8` (Landing Site)
- All subpixels = 0
- `health = 99`

## Testing

### Unit Test (C)

```c
// Load blob
FILE *f = fopen("tests/fixtures/landing_site_spawn.mss", "rb");
void *snapshot = malloc(MiniSaveStateSize());
fread(snapshot, 1, MiniSaveStateSize(), f);
fclose(f);

// Load into Mini
MiniGameState *state = MiniCreate(320, 240);
MiniLoadState(state, snapshot, MiniSaveStateSize());

// Verify frame 0 position
assert(g_ram[0x0AF6] == 128);  // samus_x
```

### Integration Test (CLI)

```bash
# Run Right for 10 frames from loaded blob
echo '{"inputs":[{"buttons":128},...]}' | \
  sm_rev_predict --load-state tests/fixtures/landing_site_spawn.mss > output.json

# Verify frame 0 matches blob
python3 -c "
import json
with open('output.json') as f:
    traj = json.load(f)
assert traj['frames'][0]['samus_x'] == 128
assert traj['frames'][0]['samus_x_sub'] == 0
"
```

### CI Test

```bash
make mini-test  # Includes mini-wram-peek-test
python3 tests/test_load_state_cli.py
```

The test asserts:
1. **Frame 0 is the pre-step state** from the blob's `$0AF6`/`$0AF8`/`$0AFA`/`$0AFC`
2. Position values include subpixels (16.16 fixed-point)
3. CLI works with both `--load-state <path>` and without (legacy)
4. When loading a state, the trajectory has `N+1` frames for `N` inputs

## Out of Scope

This load path does NOT:
- Fill residual-profile `fd_*` fields (future work)
- Fake reward `R(τ)` (Mini is physics-only)
- Support Snes9x `.state` files (use MiniSaveState blobs only)
- Guarantee movement without proper room setup (authored movement limitation)

## References

- Mini API: `src/mini/mini_game.h`
- WRAM addresses: `src/mini/mini_wram_peek.h`
- Fixture generator: `tests/generate_mss_fixture.c`
- Super Metroid WRAM map: [Bank $7E documentation](https://patrickjohnston.org/bank/7E)
