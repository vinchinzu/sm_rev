# MiniSim Enemy CLI Seeding

## Overview

The `sm_rev_predict` CLI emits `enemies[]` in trajectory frames when MiniSim has active enemies. To demonstrate this capability, `MiniAssetBootstrap_GetEditorEnemySpawnViews()` returns a default spawn list when no ROM or editor room is loaded.

## Default Enemy Spawn

When the CLI starts with a default MiniSim room (no ROM loaded), it seeds one enemy:

- **Species**: Roach (0xD87F)
- **Position**: x=150, y=176 (block 9, 11)
- **Behavior**: Active (pre-triggered, walking right)
- **Velocity**: 3 pixels/frame (encoded in `extra_parameter1 = 0x0003`)
- **Trigger Radius**: 80 pixels (encoded in `extra_parameter2 = 0x0050`)

This allows the CLI to demonstrate enemy prediction without requiring ROM data or external snapshot files.

## Wire Format

Each enemy in `enemies[]` contains:

- `id`: Enemy slot/identifier (e.g., `55423` = `0xD87F`)
- `type`: Species ID (e.g., `1` for Roach, `2` for Space Pirate)
- `x`: World X position (pixel precision)
- `y`: World Y position (pixel precision)

The `enemies[]` array is omitted from trajectory frames when no enemies are active (retro_rl compatibility).

## Limitations

- **Pixel Precision Only**: MiniSim tracks enemy positions at pixel precision. Subpixel positions (ROM `$0F78`) are not simulated.
- **Mini ≠ Emulator**: MiniSim is an iteration baseline, not a TAS-accurate emulator. Enemy AI may differ from ROM behavior in edge cases.
- **No Knockback**: Enemy-Samus collision and damage-boost are not yet implemented in MiniSim.
- **1-tile Platforms**: Enemies do not interact with 1-tile wide platforms.

## Testing

The golden test `tests/test_cli_enemy_prediction.py` verifies:

1. CLI emits `enemies[]` when MiniSim has enemies
2. Enemy structure includes `id`, `type`, `x`, `y`
3. At least one enemy's position changes across frames
4. Roach moves as expected (3 pixels/frame rightward)

Run: `make mini-cli-enemy-test`
