# Physics Prediction API

## Overview

The physics prediction API provides headless trajectory simulation for Super Metroid TAS (tool-assisted speedrun) route planning and RL (reinforcement learning) training. It exposes the mini Samus/physics kernel through a JSON-based CLI interface compatible with the [retro_rl](https://github.com/vinchinzu/retro_rl/pull/1) wire format.

**Architecture Note**: Mini is a **simplified speed baseline** for fast iteration, NOT TAS ground truth. Acceptance testing is the real emulator (snes9x/libretro via SMEDIT + retro_rl). When Mini ≠ emulator, **emulator wins**. Golden tests verify internal consistency (prediction = MiniStep), not correctness vs. real SM physics.

## CLI Interface

### Command

```bash
SM_REV_PATH=/path/to/sm_rev_predict sm_rev predict
```

The `sm_rev predict` binary location is discovered via the `SM_REV_PATH` environment variable.

### Input Format (stdin)

JSON object with an array of packed SNES button masks:

```json
{
  "inputs": [0, 128, 128, 384, 384, 128, 128, 0]
}
```

#### Button Bit Order (Standard SNES)

| Button | Bit Value | Hex  |
|--------|-----------|------|
| B      | 0x01      | 1    |
| Y      | 0x02      | 2    |
| Select | 0x04      | 4    |
| Start  | 0x08      | 8    |
| Up     | 0x10      | 16   |
| Down   | 0x20      | 32   |
| **Left**   | **0x40**      | **64**   |
| **Right**  | **0x80**      | **128**  |
| A      | 0x100     | 256  |
| X      | 0x200     | 512  |
| L      | 0x400     | 1024 |
| R      | 0x800     | 2048 |

**Note**: Standard SNES bit order has Left=0x40 and Right=0x80. The retro_rl client initially had these swapped; the bug is being fixed. This API uses the **correct standard order**.

### Output Format (stdout)

JSON object with a trajectory array in snake_case:

```json
{
  "trajectory": [
    {
      "frame": 0,
      "samus_x": 80,
      "samus_x_sub": 0,
      "samus_y": 176,
      "samus_y_sub": 65535,
      "velocity_x": 0,
      "velocity_y": 0,
      "velocity_x_extra": 0,
      "velocity_y_speed": 0,
      "pose": 0,
      "movement_type": 0,
      "on_ground": false,
      "room_id": 0,
      "state_hash": 17750130111486533260
    }
  ]
}
```

#### Trajectory Frame Fields

| Field | Type | Description |
|-------|------|-------------|
| `frame` | int | Frame number (0-indexed) |
| `samus_x` | int | Samus X position in pixels (SNES RAM $0AF6) |
| `samus_x_sub` | int | Samus X subpixel (SNES RAM $0AF8, 16-bit fixed-point fraction) |
| `samus_y` | int | Samus Y position in pixels (SNES RAM $0AFA) |
| `samus_y_sub` | int | Samus Y subpixel (SNES RAM $0AFC, 16-bit fixed-point fraction) |
| `velocity_x` | int16 | Horizontal velocity |
| `velocity_y` | int16 | Vertical velocity |
| `velocity_x_extra` | int16 | Extra horizontal run speed |
| `velocity_y_speed` | int16 | Y-axis speed component |
| `pose` | uint16 | Samus animation pose ID (see `ida_types.h`) |
| `movement_type` | uint16 | Movement state (standing, running, jumping, etc.) |
| `on_ground` | bool | Ground contact flag |
| `room_id` | int | Current room ID (0 for fallback rooms) |
| `state_hash` | uint64 | Deterministic state hash (for rollback verification) |

## Integration with retro_rl and SMEDIT

### Route Planning Workflow

1. **Initialize State**: Load initial Samus position from save state or room setup
2. **Predict Trajectory**: Send input sequence to `sm_rev predict` via stdin
3. **Analyze Path**: Parse trajectory JSON to evaluate:
   - Jump heights and landing positions
   - Damage boosts and enemy interactions
   - Speed-run optimizations (e.g., optimal jump timing)
4. **Iterate**: Refine input sequence based on predicted trajectory

### RL Training Loop

The prediction API serves as the physics oracle for reinforcement learning:

```python
# Pseudocode
state = load_initial_state()
for episode in training_episodes:
    inputs = agent.select_actions(state)
    trajectory = sm_rev_predict(inputs)
    reward = evaluate_trajectory(trajectory, goal)
    agent.update_policy(reward)
```

### Sub-pixel Accuracy

Position fields include both pixel (`samus_x`, `samus_y`) and subpixel (`samus_x_sub`, `samus_y_sub`) components. Subpixels are 16-bit fixed-point fractions where:
- `0x0000` = 0.0 (start of pixel)
- `0x8000` = 0.5 (middle of pixel)
- `0xFFFF` ≈ 0.9999 (end of pixel)

Sub-pixel precision is critical for:
- Frame-perfect jumps
- Speed-run optimizations
- Damage boost calculations

## Determinism and Oracle Verification

The `state_hash` field provides a deterministic hash of the full mini kernel state (including RAM, VRAM, and SRAM). This hash is computed by `MiniStateHash()` and ensures:

1. **Reproducibility**: Same inputs + same start → same hashes/trajectory
2. **Rollback Verification**: Network/async systems can verify predictions match reality
3. **Regression Testing**: Golden tests assert sub-pixel accuracy vs. the `MiniStep` oracle

See `tests/mini_predict_golden.c` for sub-pixel golden test examples.

## C API (Library Interface)

For advanced users, the C prediction API can be linked directly:

```c
#include "mini/mini_predict.h"

MiniPrediction *prediction = MiniPrediction_Create(frame_count);
bool success = MiniPredict(
    prediction,
    state_snapshot,      // NULL for fresh state
    snapshot_size,
    input_buttons,       // uint16 array
    input_count,
    viewport_width,
    viewport_height
);

// Access trajectory
for (size_t i = 0; i < prediction->frame_count; i++) {
    MiniTrajectoryFrame *frame = &prediction->frames[i];
    printf("Frame %d: x=%d.%04x y=%d.%04x\n",
           frame->frame,
           frame->world_x, frame->x_subpos & 0xFFFF,
           frame->world_y, frame->y_subpos & 0xFFFF);
}

MiniPrediction_Destroy(prediction);
```

## Current Limitations and Workarounds

### ROM-Free Test Infrastructure ✅

The mini kernel originally required a Super Metroid ROM for room collision geometry. This has been **resolved** via the **authored movement system** (`kMiniRoomSource_EditorExport`):

- **ROM-free test rooms** with programmatic collision geometry (see `tests/mini_test_room.c`)
- **Deterministic Samus physics** suitable for golden test validation
- **Sub-pixel accurate** ground movement ✅ (verified in `test_ground_run_golden`)

### Authored Movement Physics Limitations

The ROM-free authored movement system provides **simplified physics** compared to full Super Metroid:

**What Works:**
- ✅ Ground movement (walk, run) with accurate speed
- ✅ Ground detection and block collision
- ✅ Basic jumping with gravity and falling
- ✅ Morph ball mode
- ✅ Wall jumps
- ✅ Sub-pixel position tracking

**Known Limitations:**
- ⚠️ **Enemies stubbed**: No enemy spawn/movement in authored movement rooms (see Enemy Data below)
- ⚠️ Some advanced movement tech may differ from ROM physics (no spin jump, speed booster, grapple)

**Mini Baseline Golden Status** (fast iteration, not TAS correctness):
1. ✅ **Ground run** (hold RIGHT): prediction = MiniStep (sub-pixel consistent)
2. ✅ **Jump height** (short vs full hop): prediction = MiniStep (63px difference)
3. ✅ **Combined platforming** (run + jump): prediction = MiniStep (102px horizontal, y=80 peak)

Note: These verify **internal consistency** (no prediction drift from MiniStep). For TAS acceptance, compare against real emulator via retro_rl.

For **full physics validation**, use ROM-based tests with `uses_original_gameplay_runtime=true`.

### Enemy Data

Per project requirements, enemy position tracking is planned but **currently stubbed**:

**Current Status:**
- Enemy source files (`src/enemy_*.c`) included in mini build
- No enemy spawning in `kMiniRoomSource_EditorExport` rooms
- Enemy tracking requires linking enemy system into authored movement

**Planned Approach:**
1. Extend `MiniTrajectoryFrame` with optional `enemies[]` field
2. Link `enemy_main.c`, `enemy_collision.c` into mini
3. Implement enemy spawn for test rooms
4. Add golden test for enemy x/y movement vs oracle
5. **Damage boost fixture** (contact + knockback + i-frames) for later validation

**Relevant enemy files** for integration:
- `src/enemy_main.c` - Enemy update loop
- `src/enemy_collision.c` - Samus-enemy collision
- `src/enemy_drops.c` - Enemy death/drops
- `src/enemy_config.c` - Configuration tables

### Other Missing Features

- Room transitions not yet supported
- Save state serialization format not yet finalized for retro_rl

## Future Work

### Pure Functional Model

A pure functional physics model is planned for lambda-calculus / formal extraction. See `docs/pure_model_plan.md` (TODO) for the extraction roadmap.

### Mini Frame Step

A lightweight `mini_frame_step` API (planned on branch `feature/mini-climb-endless`) will provide a simpler oracle for unit testing without full mini kernel overhead.

## References

- Wire format schema: https://github.com/vinchinzu/retro_rl/pull/1
- Mini kernel: `src/mini/mini_game.h`, `src/mini/mini_game.c`
- Prediction API: `src/mini/mini_predict.h`, `src/mini/mini_predict.c`
- CLI tool: `src/predict_cli.c`
- Golden tests: `tests/mini_predict_golden.c`
