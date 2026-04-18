# sm_rev

`sm_rev` is a reverse-engineered C port of Super Metroid with two active goals:
- preserve full-game behavior in the main build
- carve out a subtractive `mini` build focused on Samus and physics as a small refactor sandbox

The mini build is intentionally exclusion-based. We keep the full build intact, then remove enemies, bosses, rooms, audio, and other heavy systems from the mini target until a compact Samus-runtime shell remains.

## Build Targets

Full build:
- `make`

Mini shell:
- `make mini`
- `make mini-test`

Native macOS:
- `make NATIVE_MAC=1`
- `make mini-mac`
- `make mini NATIVE_MAC=1`

The current `sm_rev_mini` binary is only a verified shell. It supports `--headless --frames N` for smoke tests and a small SDL window for manual inspection. It does not yet link the full Samus/physics runtime.

## Mission

The main refactor mission has two tracks that must stay compatible:
- refactor ASM-like C into readable topic-based modules
- grow the subtractive mini build into a Samus-and-physics sandbox without regressing the full game

That means:
- keep full-build behavioral parity as the default constraint
- prefer negative-first build cuts over scattering wrappers through unrelated code
- move mixed logic out of monolithic `sm_*.c` files into topical modules before pulling it into mini

## Current Plan

1. Split startup and shell concerns out of `src/main.c` so mini can reuse input, render, and timing code without full-game boot flow.
2. Add `stubs_mini.c` for cross-system dependencies that still leak into Samus and physics code.
3. Move the first mini runtime slice into the target: `physics.c`, `physics_config.c`, `samus_input.c`, `samus_motion.c`, `samus_jump.c`, and `samus_collision.c`.
4. Add the next Samus slice once links are stable: `samus_pose.c`, `samus_runtime.c`, `samus_draw.c`, `samus_speed.c`, and `samus_transition.c`.
5. Keep `sm_*.c`, room systems, enemy logic, bosses, demos, and audio excluded from mini until each dependency is split or stubbed cleanly.
6. Expand tests from build smoke to deterministic headless mini-state checks once the runtime slice is active.

## Validation

Use the smallest test that matches the change:
- full build integrity: `python3 -m pytest tests/test_build.py -q`
- full runtime smoke: `python3 tests/run_tests.py -v`
- mini shell smoke: `make mini-test`

## Docs

- Mini build notes: [docs/mini_build.md](/home/v/01_projects/11_games/speedrun/retro_rl/super_metroid_rl/sm_rev/docs/mini_build.md)
- Refactor guide and standing instructions: [AGENTS.md](/home/v/01_projects/11_games/speedrun/retro_rl/super_metroid_rl/sm_rev/AGENTS.md)
