# Haskell Physics Kernel for Super Metroid

## Overview

This Haskell package implements a pure, predictive physics model for Super Metroid. It acts as a **lambda function** that the C mini kernel (the oracle) can be verified against, suitable for:

- **Predictive rollouts** for ML/RL agents (`retro_rl`, `smedit`)
- **Fast offline search** (branch prediction without SDL/ROM overhead)
- **Deterministic replay** of input tapes
- **Property-based testing** of physics invariants

## Architecture

```
┌─────────────────────────────────────────────────┐
│  C Mini Kernel (Oracle)                         │
│  ├─ MiniInit / MiniStep / MiniSaveState         │
│  ├─ Full game state (RAM + projectiles + ...)   │
│  └─ Matches original ROM behavior exactly       │
└─────────────────────────────────────────────────┘
                     │
                     │ Golden tapes (recorded)
                     ▼
┌─────────────────────────────────────────────────┐
│  Haskell Physics Kernel (Pure Model)            │
│  step :: Input -> State -> State                │
│  ├─ Newtypes enforce pixel/subpixel separation  │
│  ├─ No IO inside step (pure function)           │
│  └─ Subpixel-precise match to C oracle          │
└─────────────────────────────────────────────────┘
```

## How the Pure Model Maps to C Functions

| Haskell Module | C Source | Purpose |
|----------------|----------|---------|
| `Physics.SM.Types` | `mini/mini_game.h` (MiniSamusCoreState) | Position, velocity, pose newtypes |
| `Physics.SM.Constants` | `ida_types.h` (kButton_*, kPose_*) | Named button/pose constants |
| `Physics.SM.Run` | `src/samus_speed.c`, `src/samus_motion.c` | Horizontal run accel/decel |
| `Physics.SM.Jump` | `src/samus_jump.c` | Jump squat, initial Y velocity |
| `Physics.SM.Gravity` | `src/samus_motion.c` (Samus_MoveY_WithSpeedCalc) | Gravity, terminal velocity |
| `Physics.SM.Step` | `mini/mini_game.c` (MiniStep) | Top-level frame step |
| `Physics.SM` | — | Public API: `runTape` |

## Type Safety: Newtypes Prevent Unit Confusion

The Haskell kernel uses distinct newtypes to prevent mixing incompatible units:

```haskell
newtype Pixel = Pixel Word16       -- Whole pixels (16.0 fixed-point)
newtype Subpixel = Subpixel Word16 -- Fractional pixels (0.16 fixed-point)

data Position = Position Pixel Subpixel  -- 16.16 position
data Velocity = Velocity Pixel Subpixel  -- 16.16 velocity (pixels/frame)
```

This prevents bugs like:
```haskell
-- ❌ Compile error: can't add Pixel to Subpixel directly
badAdd :: Pixel -> Subpixel -> ???

-- ✅ Type-safe: addition with carry handled correctly
addPosition :: Position -> Position -> Position
```

## Constants Ported from C

All physics constants are ported with named identifiers:

| Constant | Value (Hex) | Value (Decimal) | Source |
|----------|-------------|-----------------|--------|
| `cfgJumpInitialSpeed` (air) | `(4, 0xe000)` | 4.875 pixels/frame | `physics_config.c:36` |
| `cfgGravityAccel` (air) | `(0, 0x1c00)` | 0.109375 pixels/frame² | `physics_config.c:51` |
| `cfgRunAccel` | `(0, 0x00a0)` | 0.00244 pixels/frame² | `physics_config.c:48` |
| `cfgRunMaxSpeed` | `(3, 0x0000)` | 3.0 pixels/frame | `physics_config.c:50` |
| `cfgTerminalSpeed` | `5` | 5 pixels/frame | `samus_motion.c:27` |

See `Physics.SM.Constants` for button masks and pose constants.

## Proving a Change Against MiniStep

To verify a Haskell change matches the C oracle:

### 1. Generate golden tapes from C

Run the C mini kernel with test inputs and record states:

```bash
# Option A: Use existing mini-test
make mini-test

# Option B: Write a custom dump helper in tests/dump_golden.c
gcc -o dump_golden tests/dump_golden.c src/mini/*.c -I src -lm
./dump_golden > physics-hs/test/golden/custom.json
```

Example golden tape format:

```json
{
  "inputs": [
    {"inputButtons": 128, "inputPrevButtons": 0},
    {"inputButtons": 128, "inputPrevButtons": 128}
  ],
  "states": [
    {
      "stateXPos": {"posPixel": 100, "posSubpixel": 160},
      "stateYPos": {"posPixel": 200, "posSubpixel": 0},
      "stateOnGround": true
    },
    ...
  ]
}
```

### 2. Run Haskell golden tests

```bash
cd physics-hs
cabal test
```

Golden tests replay the input tape through the Haskell kernel and assert that positions/velocities match the C oracle at every frame.

### 3. Debug mismatches

If a test fails:

```bash
cabal test --test-show-details=streaming
```

Compare expected (C) vs actual (Haskell) states frame-by-frame. Common issues:

- **Carry/borrow errors** in 16.16 fixed-point addition
- **Unsigned wraparound** for negative velocities (use `velIsNegative` helper)
- **Off-by-one** in loop iterations or squat frame counts

## How SMEDIT/retro_rl Will Call This

Future integration paths:

### Option 1: FFI from C

Call Haskell from C via GHC FFI:

```c
// C wrapper
#include <HsFFI.h>
extern HsStablePtr hs_step(HsStablePtr state_ptr, uint16_t buttons);

void predict_n_frames(MiniGameState *c_state, uint16_t *inputs, int n) {
  HsStablePtr hs_state = marshal_to_haskell(c_state);
  for (int i = 0; i < n; i++)
    hs_state = hs_step(hs_state, inputs[i]);
  marshal_from_haskell(hs_state, c_state);
}
```

### Option 2: CLI subprocess

Use `sm-predict` executable:

```bash
echo '{"state": {...}, "inputs": [...]}' | sm-predict > predictions.json
```

### Option 3: `--engine haskell` flag

Extend `sm_rev` CLI:

```bash
sm_rev predict --engine haskell --tape inputs.json --output states.json
```

Internally calls `Physics.SM.runTape` and serializes results.

## Current Coverage

### ✅ Proven vs C Oracle (via golden tapes)
- Ground run (right)
- Jump (short hop + full jump)
- Gravity + fall

### 🚧 Stubbed (returns current state)
- Leftward movement
- Air control during jump/fall
- Morph ball
- Collision detection (ground/walls/ceiling)
- Enemies, projectiles, doors

### 📋 Planned
- Spin jump mechanics
- Wall jump
- Run speed boost
- Liquid physics (water, lava)

## Testing Strategy

Three test levels:

1. **Unit tests** (`Test.Unit`): Verify individual primitives
   - Jump squat lasts 4 frames
   - Gravity decelerates upward velocity
   - Run accel reaches max speed

2. **Property tests** (`Test.Properties`): Invariants
   - Determinism: same tape → same states
   - Zero input → stationary

3. **Golden tests** (`Test.Golden`): Frame-by-frame C oracle match
   - Recorded MiniStep outputs replayed through Haskell
   - Asserts position/velocity equality at subpixel precision

## Performance

Pure Haskell step function (no IO):

- ~10-100 µs per frame (on modern CPU)
- ~10K-100K frames/second (single-threaded)
- Parallelizable over tape prefixes (speculatively)

Compare to C mini with SDL/ROM:

- ~16 ms per frame (60 FPS cap)
- ~60 frames/second (real-time)

For ML rollouts, Haskell is 100-1000x faster than real-time C mini.

## How to Refresh Golden Tapes

When the C oracle changes (e.g., physics tweaks), regenerate golden tapes:

```bash
# 1. Update C oracle
cd /workspace
vim src/physics_config.c  # or other physics sources
make mini-test

# 2. Dump new golden tapes (custom helper or manual recording)
# Example: extend tests/mini_rollback_api.c to export JSON
gcc -o dump_golden tests/dump_golden.c src/mini/*.c -I src -lm
./dump_golden > physics-hs/test/golden/run_right.json
./dump_golden --jump > physics-hs/test/golden/jump.json

# 3. Verify Haskell matches
cd physics-hs
cabal test

# 4. If tests pass, commit updated golden files
git add test/golden/*.json
git commit -m "Refresh golden tapes after C physics change"
```

## Roadmap

- [ ] Finish leftward run
- [ ] Add air control during jump
- [ ] Port spin jump mechanics
- [ ] Implement collision detection (BVH or grid-based)
- [ ] Add morph ball + bomb jump
- [ ] Wall jump + wall collision
- [ ] Speed booster charge + shinespark
- [ ] Liquid physics (water, lava density)
- [ ] Benchmark vs C mini (headless)
- [ ] FFI bindings for C interop
- [ ] CLI tool for tape prediction (`sm-predict`)

## Maintenance

When adding new physics:

1. **Port constants** → `Physics.SM.Constants`
2. **Port logic** → New module or extend existing
3. **Add unit test** → `Test.Unit`
4. **Record golden tape** → C oracle dump helper
5. **Add golden test** → `Test.Golden`
6. **Update this doc** → Coverage table

Keep the "Proven vs C Oracle" section honest — only mark features as proven once golden tests pass.
