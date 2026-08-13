# physics-hs: Super Metroid Physics Kernel (Haskell)

A SpaceX-quality pure functional physics model for Super Metroid.

## Design

This is the **predictive model**, not a sketch:

- **Pure**: `(SamusState, ControllerInput) -> SamusState` per frame. No IO inside the step.
- **Fast iteration baseline**: C `MiniStep` for rapid golden testing (simplified model).
- **Acceptance layer**: Real emulator (snes9x/libretro) WRAM ($0AF6/$0AFA + $0AF8/$0AFC) via SMEDIT + retro_rl.
- **Type-safe**: Newtypes prevent mixing pixels, subpixels, and velocities.

## Architecture Layers (Do Not Invert)

```
Layer 1: Fast Iteration (MiniStep baseline - simplified model)
├─ Haskell pure (State, Input) -> State bisimulates Mini for SPEED
├─ Recorded Mini JSON goldens (CI tests without ROM/binary)
└─ Purpose: Rapid development iteration, NOT TAS-correct acceptance

Layer 2: Acceptance (Real Emulator - ground truth)
├─ Same tape → same Samus x/y/subX/subY/pose on REAL emu (snes9x/libretro)
├─ SMEDIT bridge + retro_rl stable-retro for WRAM telemetry
├─ If Mini and emu disagree, emu wins (file Mini delta)
└─ Ceres→Morph→Bomb usefulness is emu playback, not mini-only

Haskell Model (pure predictor)
├─ initialState :: Config -> SamusState
├─ step :: Input -> State -> State
├─ runTape :: State -> [Input] -> [State]
└─ Types enforce pixel/subpixel separation
```

**Critical**: MiniStep is NOT ground truth. It's a fast baseline for iteration. Emulator is acceptance.

## Building

### Prerequisites

- GHC >= 9.0 (Haskell compiler)
- Cabal >= 3.6 (Haskell build tool)

Install via [GHCup](https://www.haskell.org/ghcup/):

```bash
curl --proto '=https' --tlsv1.2 -sSf https://get-ghcup.haskell.org | sh
ghcup install ghc 9.4.8
ghcup install cabal 3.10.2.0
```

### Build

```bash
cd physics-hs
cabal build
cabal test
```

## Usage

```haskell
import Physics.SM

-- Create initial state
let state = initialState defaultConfig

-- Step one frame
let state' = step input state

-- Run a tape of inputs
let states = runTape state inputs
```

## Testing

Three levels of verification:

1. **Unit tests**: Accel, friction, jump squat verified against named constants
2. **Golden tapes**: Recorded MiniStep outputs for fast CI (baseline, not acceptance)
3. **Properties**: Determinism (same tape → same states)
4. **Acceptance** (future): Same tape → same WRAM on real emulator via SMEDIT/retro_rl

To regenerate golden tapes from MiniStep baseline:

```bash
# When MiniStep binary available:
sm_rev_mini_oracle --json < fixture.json > golden.json
# CI runs against checked-in goldens (no binary required)
cabal test
```

## What's Implemented

### ✅ Iteration Baseline (matches MiniStep for fast dev)
- Ground run (left/right with B-run)
- Jump (short hop + full jump)
- Gravity and fall

### 🚧 Still C-only
- Air control
- Morph ball
- Collision detection
- Enemies

## Constants

All physics constants are ported from:
- `src/physics_config.c` - Jump speeds, gravity, run accel
- `src/samus_motion.c` - Movement logic
- `src/samus_jump.c` - Jump initialization
- `src/samus_speed.c` - Horizontal speed

Named constants replace magic numbers. See `Physics.SM.Constants`.
