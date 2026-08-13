# Haskell Physics Kernel for Super Metroid

## Overview

This Haskell package implements a pure, predictive physics model for Super Metroid suitable for:

- **Fast iteration**: Deterministic physics exploration (no C oracle yet)
- **Predictive rollouts** for ML/RL agents (`retro_rl`, `smedit`)
- **Deterministic replay** of input tapes
- **Property-based testing** of physics invariants
- **Future acceptance**: Verify against real emulator (snes9x/libretro) via SMEDIT/retro_rl

## Architecture (Do Not Invert)

```
┌─────────────────────────────────────────────────┐
│  Layer 1: MiniStep Baseline (Future)            │
│  ├─ MiniInit / MiniStep / MiniSaveState         │
│  ├─ Simplified model (NOT TAS-correct)          │
│  ├─ Golden tapes for fast iteration             │
│  └─ NOT INTEGRATED YET                          │
└─────────────────────────────────────────────────┘
                     │
                     │ Bisimulation for speed (future)
                     ▼
┌─────────────────────────────────────────────────┐
│  Haskell Physics Kernel (Pure Model)            │
│  step :: Input -> State -> State                │
│  ├─ Newtypes enforce pixel/subpixel separation  │
│  ├─ No IO inside step (pure function)           │
│  ├─ Unit tests prove rise/fall/accel            │
│  └─ Signed 16.16 velocity (Int16 + Word16)      │
└─────────────────────────────────────────────────┘
                     │
                     │ Acceptance layer (future)
                     ▼
┌─────────────────────────────────────────────────┐
│  Layer 2: Real Emulator (Ground Truth)          │
│  ├─ snes9x / libretro core                      │
│  ├─ SMEDIT bridge + retro_rl telemetry          │
│  ├─ WRAM $0AF6/$0AFA + $0AF8/$0AFC              │
│  ├─ If Mini ≠ emu, emu wins (file Mini delta)   │
│  └─ NOT IMPLEMENTED YET                         │
└─────────────────────────────────────────────────┘
```

**Critical**: MiniStep is NOT ground truth. It's a fast baseline. Emulator is acceptance. **Neither layer integrated yet.**

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
newtype Pixel = Pixel Word16       -- Whole pixels (unsigned position)
newtype Subpixel = Subpixel Word16 -- Fractional pixels (0.16 fixed-point)

data Position = Position Pixel Subpixel    -- 16.16 unsigned position
data Velocity = Velocity Int16 Subpixel    -- Signed 16.16 velocity (pixels/frame)
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
| `cfgJumpInitialSpeed` (air) | `(-5, 0x8000)` | -4.5 pixels/frame | `physics_config.c:36` |
| `cfgGravityAccel` (air) | `(0, 0x1c00)` | 0.109375 pixels/frame² | `physics_config.c:51` |
| `cfgRunAccel` | `(0, 0x00a0)` | 0.00244 pixels/frame² | `physics_config.c:48` |
| `cfgRunMaxSpeed` | `(3, 0x0000)` | 3.0 pixels/frame | `physics_config.c:50` |
| `cfgTerminalSpeed` | `5` | 5 pixels/frame | `samus_motion.c:27` |

See `Physics.SM.Constants` for button masks and pose constants.

## Current Coverage

### ✅ Proven (Unit Tests)

- **Ground run**: B+Right accelerates to 3.0 pixels/frame max
- **Leftward run**: B+Left produces negative X velocity
- **Jump squat**: 4-frame timing
- **Jump initiation**: Y velocity starts negative (-4.5 pixels/frame)
- **Rise**: Y decreases after jump (HopRise test)
- **Gravity**: Decelerates upward, accelerates downward
- **Terminal velocity**: Caps at 5.0 pixels/frame
- **Landing**: Flat floor detection at Y=200
- **Signed 16.16**: `Velocity (-5, 0x8000)` = -4.5 moves 4.5 pixels correctly

### ⚠️  No C Oracle Integration Yet

**Current golden**: Only `run_right.json` exists (Haskell self-output, predictor: "haskell-v1")

**No hop JSON files** - HopRise proven via unit tests only, not C oracle goldens.

**No MiniStep integration** - no `sm_rev_mini_oracle` binary, no recorded C baseline tapes.

### 🚧 Stubbed (Minimal Implementation)

- **Collision**: Only flat floor at Y=200, no slopes/platforms/walls
- **Air control**: Velocity frozen during jump/fall
- **Morph ball**: Not implemented
- **Equipment**: Hi-Jump/Spin/Speed Booster stubbed
- **Enemies, projectiles, doors**: Not implemented

### 📋 Planned (When Mini/Emu Layers Ready)

- Integrate C `sm_rev_mini_oracle` for fast baseline comparison
- Record Mini golden tapes (iteration speed)
- Build emulator acceptance layer (TAS ground truth)
- Add collision detection (BVH or grid-based)
- Spin jump mechanics
- Wall jump
- Run speed boost + shinespark
- Liquid physics (water, lava)

## Testing Strategy

Three test levels:

1. **Unit tests** (`Test.Unit`): Verify individual primitives
   - Jump squat lasts 4 frames
   - Gravity decelerates upward velocity
   - Run accel reaches max speed
   - **B+Left produces negative velocity**

2. **Property tests** (`Test.Properties`): Invariants
   - Determinism: same tape → same states
   - Rightward motion accumulates over time

3. **Golden tests** (`Test.Golden`): File existence checks
   - `run_right.json` exists (Haskell self-output)
   - **NOT C oracle tapes** - no MiniStep integration yet

**Current: 16/16 tests pass locally (no CI yet)**

## Future: Proving Against MiniStep (Not Implemented)

When C `sm_rev_mini_oracle` is merged and integrated, golden testing workflow:

### 1. Generate golden tapes from C (future)

```bash
# Once sm_rev_mini_oracle exists:
echo '{"start": {...}, "inputs": [...]}' | sm_rev_mini_oracle --json > golden.json
```

### 2. Compare Haskell vs Mini

```bash
cd physics-hs
cabal test  # Compares Haskell step vs recorded Mini baseline
```

### 3. Acceptance Against Emulator (future)

Final validation against real SNES emulator (snes9x/libretro):
- Read WRAM $0AF6/$0AFA (X position) + $0AF8/$0AFC (Y position)
- Compare Haskell, Mini, and emulator outputs
- **If Mini ≠ emu, emu wins** (file Mini as known delta)

## Performance

Pure Haskell step function (no IO):

- ~10-100 µs per frame (on modern CPU)
- ~10K-100K frames/second (single-threaded)
- Parallelizable over tape prefixes (speculatively)

For ML rollouts, Haskell is fast enough for speculative prediction without C FFI overhead.

## Roadmap

- [ ] Integrate C `sm_rev_mini_oracle` (when merged)
- [ ] Record Mini baseline golden tapes
- [ ] Build emulator acceptance layer (snes9x/libretro)
- [ ] Add collision detection (slopes, platforms, walls)
- [ ] Implement air control during jump
- [ ] Port spin jump mechanics
- [ ] Add morph ball + bomb jump
- [ ] Wall jump + wall collision
- [ ] Speed booster charge + shinespark
- [ ] Liquid physics (water, lava density)
- [ ] Benchmark vs C mini (headless)
- [ ] FFI bindings for C interop
- [ ] CI running `cabal test`

## Maintenance

When adding new physics:

1. **Port constants** → `Physics.SM.Constants`
2. **Port logic** → New module or extend existing
3. **Add unit test** → `Test.Unit` (prove behavior)
4. **Future: Record golden tape** → When Mini oracle integrated
5. **Future: Add golden test** → When C baseline exists
6. **Update this doc** → Coverage table

**Keep the "Proven" section honest** — only mark features as proven once unit tests or oracle goldens exist.

## Current Status (HEAD)

**Branch**: `cursor/haskell-physics-kernel-76e5`  
**Tests**: 16/16 pass locally (no CI)  
**Golden**: Only `run_right.json` (Haskell self-output)  
**Oracle**: No MiniStep integration  
**Emulator**: No acceptance layer  

**This is an iteration kernel**, not a Mini replacement or TAS-correct physics implementation.
