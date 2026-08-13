# physics-hs: Super Metroid Physics Kernel (Haskell)

A SpaceX-quality pure functional physics model for Super Metroid, verified against the C mini oracle.

## Design

This is the **predictive model**, not a sketch:

- **Pure**: `(SamusState, ControllerInput) -> SamusState` per frame. No IO inside the step.
- **Oracle**: The C `MiniStep` function is the ground truth. This Haskell kernel must match at subpixel precision.
- **Type-safe**: Newtypes prevent mixing pixels, subpixels, and velocities.

## Architecture

```
C Oracle (ground truth)       Haskell Model (pure predictor)
├─ MiniInit                   ├─ initialState :: Config -> SamusState
├─ MiniStep                   ├─ step :: Input -> State -> State
├─ MiniSaveState              ├─ runTape :: State -> [Input] -> [State]
├─ MiniLoadState              └─ Types enforce pixel/subpixel separation
└─ MiniStateHash
```

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

1. **Unit tests**: Accel, friction, jump squat verified against known constants
2. **Golden tapes**: Recorded C oracle outputs replayed through Haskell
3. **Properties**: Determinism (same tape → same states)

To regenerate golden tapes from C oracle:

```bash
make mini-test  # or custom dump helper
# generates test/golden/*.json
cabal test
```

## What's Implemented

### ✅ Proven against MiniStep
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
