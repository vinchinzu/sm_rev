# Physics Prediction API

## Overview

The physics prediction API provides headless trajectory simulation for Super Metroid route planning and RL training. It exposes the mini Samus/physics kernel through a JSON-based CLI interface matching the [retro_rl 66836f5](https://github.com/vinchinzu/retro_rl/tree/66836f5) wire format.

**Architecture Note**: Mini is a **simplified speed baseline** for fast iteration, NOT TAS ground truth. Acceptance testing uses the real emulator (snes9x/libretro via SMEDIT + retro_rl). When Mini ≠ emulator, **emulator wins**. Golden tests verify internal consistency (prediction API = MiniStep oracle), not correctness vs. full Super Metroid physics.

## CLI Interface

### Command

```bash
sm_rev_predict [predict]
```

The CLI binary is discovered via the `SM_REV_PATH` environment variable. The optional `predict` argument is accepted for `SmRevClient` compatibility.

### Version Check

```bash
sm_rev_predict --version
```

Returns version string (no stdin required).

### Input Format (stdin)

JSON object matching `retro_rl` 66836f5 `physics_sim.py`:

```json
{
  "start": {
    "frame": 0,
    "room_id": 37368,
    "samus_x": 128,
    "samus_y": 160,
    "samus_x_sub": 0,
    "samus_y_sub": 0,
    "velocity_x": 0,
    "velocity_y": 0,
    "velocity_x_sub": 0,
    "velocity_y_sub": 0,
    "momentum_x": 0,
    "momentum_x_sub": 0,
    "pose": 0,
    "facing": 8,
    "movement_type": 0,
    "speed_counter": 0,
    "speed_flag": 0,
    "shinespark_timer": 0
  },
  "inputs": [
    {"buttons": 0},
    {"buttons": 128},
    {"buttons": 384}
  ]
}
```

#### SimState Fields (start)

| Field | Type | Description |
|-------|------|-------------|
| `frame` | int | Frame number |
| `room_id` | int | Room ID (e.g., 37368 = 0x91F8) |
| `samus_x` | int | Samus X position (pixels) |
| `samus_y` | int | Samus Y position (pixels) |
| `samus_x_sub` | int | X subpixel (16-bit) |
| `samus_y_sub` | int | Y subpixel (16-bit) |
| `velocity_x` | int16 | X velocity |
| `velocity_y` | int16 | Y velocity |
| `velocity_x_sub` | int16 | X velocity subpixel |
| `velocity_y_sub` | int16 | Y velocity subpixel |
| `momentum_x` | int16 | Speed booster momentum X |
| `momentum_x_sub` | int16 | Momentum X subpixel |
| `pose` | uint16 | Samus pose (`ida_types.h`) |
| `facing` | uint8 | Facing (0x04=left, 0x08=right) |
| `movement_type` | uint16 | Movement state |
| `speed_counter` | uint16 | Speed booster counter |
| `speed_flag` | uint16 | Speed booster flag |
| `shinespark_timer` | uint16 | Shinespark timer |

**Mini Limitation**: Mini cannot hydrate from arbitrary `SimState` JSON. The `start` field is echoed back for wire compatibility, but prediction begins from a fixed Mini room state. To predict from arbitrary states, use binary snapshots (`MiniStateSnapshot`).

#### FrameInput (inputs array)

Each input is a JSON object with a 12-bit packed SNES button mask:

```json
{"buttons": 128}
```

**Button Bit Order (Standard SNES)**

| Button | Bit | Decimal | Example |
|--------|-----|---------|---------|
| B      | 0x001 | 1     | `{"buttons": 1}` |
| Y      | 0x002 | 2     | `{"buttons": 2}` |
| Select | 0x004 | 4     | — |
| Start  | 0x008 | 8     | — |
| Up     | 0x010 | 16    | — |
| Down   | 0x020 | 32    | — |
| Left   | 0x040 | 64    | — |
| Right  | 0x080 | 128   | `{"buttons": 128}` |
| A      | 0x100 | 256   | `{"buttons": 256}` |
| X      | 0x200 | 512   | — |
| L      | 0x400 | 1024  | — |
| R      | 0x800 | 2048  | — |

**Examples:**
- Right only: `{"buttons": 128}`
- B + Right: `{"buttons": 129}` (0x001 | 0x080)
- A + Right: `{"buttons": 384}` (0x100 | 0x080)

### Output Format (stdout)

JSON object matching `Trajectory.to_dict()`:

```json
{
  "start": {
    "frame": 0,
    "room_id": 37368,
    "samus_x": 128,
    "samus_y": 160,
    "samus_x_sub": 0,
    "samus_y_sub": 0,
    "velocity_x": 0,
    "velocity_y": 0,
    "velocity_x_sub": 0,
    "velocity_y_sub": 0,
    "momentum_x": 0,
    "momentum_x_sub": 0,
    "pose": 0,
    "facing": 8,
    "movement_type": 0,
    "speed_counter": 0,
    "speed_flag": 0,
    "shinespark_timer": 0
  },
  "frames": [
    {
      "frame": 0,
      "room_id": 0,
      "samus_x": 80,
      "samus_y": 176,
      "samus_x_sub": 0,
      "samus_y_sub": 65535,
      "velocity_x": 0,
      "velocity_y": 0,
      "velocity_x_sub": 0,
      "velocity_y_sub": 0,
      "momentum_x": 0,
      "momentum_x_sub": 0,
      "pose": 0,
      "facing": 8,
      "movement_type": 0,
      "speed_counter": 0,
      "speed_flag": 0,
      "shinespark_timer": 0
    }
  ],
  "predictor": "sm_rev",
  "inputs": [
    {"buttons": 0},
    {"buttons": 128}
  ]
}
```

#### Trajectory Schema

- **`start`**: `SimState` — Echoed from input (or first frame if not provided)
- **`frames`**: `TrajectoryFrame[]` — Per-frame Samus state snapshots
- **`predictor`**: `string` — Predictor identifier (`"sm_rev"`)
- **`inputs`**: `FrameInput[]` — Input sequence (echoed from request)

#### TrajectoryFrame Fields

Same as `SimState`, plus optional `enemies[]` array (currently always empty for Mini).

## Sub-Pixel Accuracy

Mini tracks sub-pixel position via global variables `samus_x_subpos` and `samus_y_subpos` (16-bit fixed-point fractions). Position is stored as:

```
full_position = samus_x + (samus_x_sub / 65536.0)
```

This matches SNES RAM layout:
- `$0AF6` (samus_x) + `$0AF8` (samus_x_sub)
- `$0AFA` (samus_y) + `$0AFC` (samus_y_sub)

## Determinism

Same inputs + same start → same trajectory. Tested via:
- **C golden tests** (`tests/mini_predict_golden.c`): MiniStep oracle for ground run, short/full hop, 1-tile platform
- **CLI golden tests** (`tests/test_cli_golden.sh`): Wire format compatibility (128=Right, 256=A)

## Integration with retro_rl and SMEDIT

### Route Planning Workflow

1. **Initialize State**: Load or construct `SimState` JSON
2. **Predict Trajectory**: Pipe input JSON to `sm_rev_predict` via stdin
3. **Analyze Path**: Parse `frames` to evaluate route viability
4. **Prune Search Space**: Use prediction to skip dead-end branches

### Python Client Example

```python
import json
import subprocess
import os

def predict_trajectory(start_state, inputs):
    """
    Predict trajectory using sm_rev_predict CLI.
    
    Args:
        start_state: SimState dict (or None)
        inputs: list of button masks (int)
    
    Returns:
        Trajectory dict
    """
    request = {
        "start": start_state,
        "inputs": [{"buttons": btn} for btn in inputs]
    }
    
    cli_path = os.environ.get("SM_REV_PATH", "./sm_rev_predict")
    result = subprocess.run(
        [cli_path],
        input=json.dumps(request),
        capture_output=True,
        text=True,
        check=True
    )
    
    return json.loads(result.stdout)

# Example: Run right for 10 frames
inputs = [128] * 10  # Right button
trajectory = predict_trajectory(None, inputs)

print(f"Frames: {len(trajectory['frames'])}")
print(f"Final X: {trajectory['frames'][-1]['samus_x']}")
```

## Authored Movement Physics Limitations

Mini uses a simplified "authored movement" physics system for ROM-free testing:

- **Velocity subpixels**: Always zero (authored movement doesn't track)
- **Momentum**: Always zero (no speed booster in mini)
- **Speed booster state**: `speed_counter`, `speed_flag`, `shinespark_timer` always zero
- **Jump height**: Variable-height jumps implemented (hold A for full height)
- **Enemies**: Stubbed (no AI, collision, or damage boost)
- **Room-specific physics**: Simplified (water, lava, acid effects not modeled)

These simplifications make Mini a **speed baseline** for fast iteration, not a substitute for full emulator acceptance testing.

## Testing

### C Golden Tests (Mini Baseline)

```bash
make mini-predict-golden && ./sm_rev_mini_predict_golden
```

Tests ground run, short/full hop, and run+jump combined trajectory against `MiniStep` oracle. **Note**: Mini does not model platform collision; 1-tile landing unverified.

### CLI Golden Tests (Wire Format)

```bash
SM_REV_PATH=./sm_rev_predict bash tests/test_cli_golden.sh
```

Verifies JSON parsing, wire button format (128=Right, 256=A), and Trajectory schema.

### Full Build

```bash
make && make mini-test
```

Ensures full game behavioral parity (no regressions).

## Future Work

- **State hydration**: Load arbitrary `SimState` JSON (requires mapping to `MiniGameState`)
- **Mini–emulator residual profiles**: first profile is [mini_emu_delta.md](mini_emu_delta.md) (`make mini-emu-residual`). C-port matches emu on residual words while idle from `save0`. Mini CLI boot zeros `$0AFC`. If Mini ≠ emu, emu wins.
- **Damage boost**: Contact + knockback + i-frames for route optimization, in C Mini first
- **Haskell model**: keep `physics-hs` as a subordinate spec and pure-rollout layer of the residual-relevant fragment (ROM tables, air X, extra run, land leftovers). Wire `MiniPredict` / `MiniStep` as an H↔M CI check. Do not treat Haskell as a separate agent or a second physics kernel.

## References

- Wire format: [retro_rl 66836f5 physics_sim.py](https://github.com/vinchinzu/retro_rl/blob/66836f5/snes/super_metroid/physics_sim.py)
- SNES button order: Standard SNES controller bit layout
- Samus poses: `src/ida_types.h`
- Mini API: `src/mini/mini_game.h`, `src/mini/mini_predict.h`
