# Haskell Physics Kernel for Super Metroid

## Posture (Do Not Invert)

`physics-hs` is a useful supporting layer. It is not the main event.

Keep it for things C `MiniStep` cannot give cheaply:

- side-effect-free deterministic rollouts that are easy to parallelize from
  pure / ML / RL code
- property tests and invariants that are painful in mutating C
- type-safe modelling of residual-relevant state (`Pixel` / `Subpixel` /
  `Position` / `Velocity`)
- a readable executable specification of the implemented motion fragment
- a cheap continuous check once Mini is wired: H and M still agree on that
  fragment

Grow the fragment when the extra state is useful for those jobs. The SMB
stepper (`retro_rl/nes/smb/approx.py`) taught the useful set: real walk/run
tables, air X on takeoff, A-release gravity, landing leftovers, and
internal residual fields (`momentum_x` / extra run). That is now in
Haskell. Do not grow past it into emu-only behavior.

It becomes overhead the moment any of these happen:

- every physics tweak must be mirrored in Haskell before Mini or emulator
  work can proceed
- the kernel is never called from planning, rollouts, property tests, or CI
- the pure model expands into areas only the emulator can get right (full
  lag, complex enemy interactions, exact door transitions). Ceres rooms
  and original door-transition game states belong in Mini and the emulator.

```
Haskell (residual-complete spec + pure rollouts)
        │  observational agreement on residual-relevant state
        ▼
MiniStep / MiniPredict (fast baseline, not TAS-correct)
        │  residual profiles
        ▼
Emulator (snes9x / libretro)  ← ground truth
```

If Mini ≠ emu, **emu wins**. File the Mini delta. Do not grow Haskell to
paper over it.

The higher-leverage tool is still the Mini–emulator residual profile.
Haskell owns the parts of the workflow that benefit from purity.

## Overview

This Haskell package implements a pure model of the residual-relevant Samus
fragment:

- **Pure rollouts** for `retro_rl` / SMEDIT planning
- **Deterministic replay** of input tapes
- **Property-based testing** of the implemented fragment
- **H↔M CI** against existing `MiniPredict` / `MiniStep` (not wired yet)

It does not replace Mini, and it is not TAS-correct.

## How the Pure Model Maps to C Functions

This is the intended fragment mapping, not a claim that H and M currently
agree. `MiniStep` is a mode dispatcher (original runtime, authored
movement, multiplayer). Haskell models one residual-relevant motion
fragment, not all of `MiniUpdate`.

| Haskell Module | C Source | Purpose |
|----------------|----------|---------|
| `Physics.SM.Types` | `mini/mini_game.h` (MiniSamusCoreState) | Position, velocity, pose, extra run |
| `Physics.SM.Constants` | `ida_types.h` (kButton_*, kPose_*) | Named button/pose constants |
| `Physics.SM.SpeedTable` | ROM `$90:9F55` / `$A08D` / `$A1DD` | Walk / run / jump / spin / fall tables |
| `Physics.SM.Momentum` | `src/samus_speed.c` (HandleExtraRunspeedX) | Extra run / speed-booster residual |
| `Physics.SM.Pose` | `src/samus_pose.c` (narrow) | Stand / run / crouch / morph / land |
| `Physics.SM.Run` | `src/samus_speed.c`, `src/samus_motion.c` | Ground + air X |
| `Physics.SM.Jump` | `src/samus_jump.c` | Jump squat, 4.E000 impulse, spin |
| `Physics.SM.Gravity` | `src/samus_motion.c` (Samus_MoveY_WithSpeedCalc) | Pre-gravity move, A-release, land leftover |
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
| `cfgJumpInitialSpeed` (air) | `(-5, 0x2000)` | -4.875 pixels/frame (`4.E000`) | `physics_config.c` |
| `cfgGravityAccel` (air) | `(0, 0x1c00)` | 0.109375 pixels/frame² | `physics_config.c` |
| Air run table accel / max | `(0, 0x3000)` / `(2, 0xC000)` | 0.1875 / 2.75 px/frame | ROM `$90:9F61` |
| Extra run accel / cap | `(0, 0x1000)` / `(2, 0)` | 0.0625 / 2.0 px/frame | ROM `$90:9F07` / `$9F19` |
| `cfgTerminalSpeed` | `5` | 5 pixels/frame | `samus_motion.c` |

See `Physics.SM.Constants` for button masks and pose constants.

## Current Coverage

### ✅ Proven (Unit Tests)

- **Ground walk**: Right without B uses the ROM run row (`0.3000` / `2.C000`)
- **Ground run**: B+Right also builds extra run (`0.1000`/frame, cap `2.0`)
- **Leftward run**: B+Left produces negative X velocity
- **Air X**: leave-ground frame uses jump/spin/fall tables, not ground leftovers
- **Jump squat**: 4-frame timing, pose `4B`/`4C`
- **Jump initiation**: vanilla `4.E000` (−4.875 px/frame)
- **A-release**: snaps to falling at 0, then applies gravity
- **Rise / land**: Y decreases; land snaps pixel Y and keeps Y sub leftover
- **Terminal velocity**: Caps at 5.0 pixels/frame
- **Signed 16.16**: `Velocity (-5, 0x2000)` = -4.875 moves 4.875 pixels correctly

### ⚠️  Mini Oracle Exists In C, Not Wired Here

Mini already has `MiniPredict`, `MiniStep`, and
`tests/mini_predict_golden.c`. There is no separate `sm_rev_mini_oracle`
binary to wait for.

**Current golden**: Only `run_right.json` exists (Haskell self-output,
predictor: `"haskell-v1"`). That is a determinism tape, not an H↔M check.

**No hop JSON files** — HopRise is proven via unit tests only.

Wiring H to Mini is still the next *CI* task. The model itself now includes
the SMB-useful fragment (tables, air X, extra run, land leftovers).

### Out Of Scope For Haskell

These belong in C Mini stubs and Mini–emulator residual profiles first:

- slopes, platforms, walls, ceilings
- knockback / damage boost
- wall jump, shinespark crash
- enemies, projectiles, doors
- TAS-correct liquid FX / lag

Haskell already observes a *narrow* useful subset: spin vs normal jump
tables, morph/crouch pose, water/lava *tables* when `stateEnvironment` is
set. Do not port the rest to "keep the spec complete."

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

3. **Segment tests** (`Test.Segments`): SMB residual shape
   - idle, walk, run, jump, run-jump, land leftovers

4. **Golden tests** (`Test.Golden`): currently file-existence only
   - `run_right.json` exists (Haskell self-output)
   - this is **not** an H↔M check

**Current: unit + segment + property tests pass locally. `Test.MiniCompare` is the H↔M hook (`make hm-test`).**

## Next Haskell Work: Wire Mini; Grow Only Residual-Useful State

`make mini-predict-golden` already compares `MiniPredict` to `MiniStep`.
Haskell should consume that same residual-relevant fragment.

### 1. Record Mini tapes from the existing C API

```bash
make mini-predict-golden
# or the predict CLI once it can hydrate a known start state
```

### 2. Compare Haskell vs Mini in `cabal test`

Same tape, same residual-relevant fields (`samus_x/y`, subpixels,
`velocity_x/y` and subs, pose, movement type). Treat disagreement as a CI
failure of the *implemented fragment*, not as permission to fork the model.

### 3. Mini–emulator residual stays the higher-leverage check

Final validation against snes9x/libretro:
- Read WRAM `$0AF6`/`$0AFA` (X) + `$0AF8`/`$0AFC` (Y)
- Compare Mini and emulator first
- **If Mini ≠ emu, emu wins** (file Mini as a known delta)
- Only then ask whether Haskell still agrees with Mini

## Performance

Pure Haskell step function (no IO):

- ~10-100 µs per frame (on modern CPU)
- ~10K-100K frames/second (single-threaded)
- Parallelizable over tape prefixes (speculatively)

For ML rollouts, Haskell is fast enough for speculative prediction without C FFI overhead.

## Roadmap

Keep this list short. Do not treat it as a second Super Metroid port.

- [ ] Wire existing `MiniPredict` / `MiniStep` as the H↔M oracle
- [ ] Replace file-existence goldens with residual-relevant field compares
- [ ] Run `cabal test` in CI as a cheap H↔M signal
- [ ] Keep property tests on the implemented fragment (determinism,
      signed 16.16, jump rise, accel)
- [x] Measure Mini–emulator residual profiles (see [mini_emu_delta.md](mini_emu_delta.md))
- [x] ROM speed tables, air X, extra run, A-release, land leftovers
- [ ] Only then consider slopes/walls in Haskell, after Mini already has
      an M–E residual budget for that geometry

## Maintenance

When the C fragment changes:

1. Update Mini first. Measure M–E residual if the change can affect TAS
   planning.
2. If the change is inside the Haskell fragment, port constants then
   logic, then add a unit or property test.
3. Record or refresh a Mini tape and keep the H↔M compare green.
4. Update the coverage table. Only mark a feature proven when H and M
   agree on it, or when a unit test owns a Haskell-only invariant.

Add a Haskell mechanic because pure rollouts or residual CI need that
fragment (SMB lesson), not because the C side grew.

## Current Status (HEAD)

**Tests**: unit + segment + property tests pass locally; `Test.MiniCompare` is the H↔M hook  
**Golden**: only `run_right.json` (Haskell self-output)  
**Oracle**: Mini C API exists; `cabal test` calls `sm_rev_predict` when present  
**Emulator**: first M–E profile is in [mini_emu_delta.md](mini_emu_delta.md)

This is a supporting specification and rollout kernel for the residual-
relevant fragment. It is not a Mini replacement and not TAS-correct.
