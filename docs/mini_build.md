# Mini Build Shell

This repo now has a first-pass `mini` build target intended for subtractive refactoring work.

Current scope:
- `make mini` builds `sm_rev_mini`.
- `sm_rev_mini` is a shell binary, not a gameplay-complete Samus sandbox yet.
- The build is compiled with `CURRENT_BUILD=BUILD_MINI`, which activates negative feature flags in [src/features.h](/home/v/01_projects/11_games/speedrun/retro_rl/super_metroid_rl/sm_rev/src/features.h).
- The shell supports `--headless --frames N` for smoke testing and a small SDL window for manual inspection.

Linux:
- `make mini`
- `make mini-test`

macOS:
- `make mini-mac`
- `make mini NATIVE_MAC=1`

The existing native macOS path uses SDL2 frameworks and turns on bundled assets by default for the full build. The mini shell does not require a ROM and is the easiest target to validate first on macOS.

## Negative-First Build Shape

The initial setup uses exclusion-friendly build flags:
- `NO_ENEMIES`
- `NO_BOSSES`
- `NO_ROOMS`
- `NO_GAME_SYSTEMS`
- `NO_SOUND`

Right now those flags are only used by the mini shell, but they define the contract for the next steps: pull Samus/physics code into the mini target while keeping full-game systems excluded by default.

## Forward Plan

1. Split startup concerns out of [src/main.c](/home/v/01_projects/11_games/speedrun/retro_rl/super_metroid_rl/sm_rev/src/main.c) so the mini target can reuse input, render, and timing code without dragging full game boot.
2. Introduce `stubs_mini.c` for cross-system calls that Samus/physics code still reaches into.
3. Move the first runtime slice into mini: `physics.c`, `physics_config.c`, `samus_input.c`, `samus_motion.c`, `samus_jump.c`, and `samus_collision.c`.
4. Add the second Samus slice after link stability: `samus_pose.c`, `samus_runtime.c`, `samus_draw.c`, `samus_speed.c`, and `samus_transition.c`.
5. Keep `sm_*.c`, room logic, demo flow, enemies, bosses, and audio out of mini until each dependency is either stubbed or split cleanly.
6. Extend tests from shell smoke to deterministic headless mini-state checks once Samus runtime is actually linked in.
