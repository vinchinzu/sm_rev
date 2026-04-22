# Mini Build Shell

This repo now has a first-pass `mini` build target intended for subtractive refactoring work.

For the gameplay-kernel roadmap that reframes mini around moddable Samus movement,
collision, and authored map/nav rules, see [mini_modability_plan.md](mini_modability_plan.md).
For the staged path from the current mini shell to a deterministic multiplayer-ready
kernel, including the next `physics.c` extraction target, see
[mini_multiplayer_roadmap.md](mini_multiplayer_roadmap.md).

Current scope:
- `make mini` builds `sm_rev_mini`.
- `sm_rev_mini` now runs Samus inside a small authored room backed by the same
  mini collision tilemap used for movement bring-up.
- It is still not a gameplay-complete Samus sandbox yet.
- The build is compiled with `CURRENT_BUILD=BUILD_MINI`, which activates negative feature flags in [src/features.h](../src/features.h).
- The shell supports `--headless --frames N` for smoke testing and a small SDL window for manual inspection.

## Current Mini Layers

The mini target is now split into clearer responsibilities under [`src/mini/`](../src/mini):
- [mini_main.c](../src/mini/mini_main.c): CLI parsing only
- [mini_runtime.c](../src/mini/mini_runtime.c): SDL/headless host loop and process orchestration
- [mini_input_script.c](../src/mini/mini_input_script.c): deterministic replay-script parsing
- [mini_renderer.c](../src/mini/mini_renderer.c): software frame rendering and screenshot output
- [mini_ppu_stub.c](../src/mini/mini_ppu_stub.c): mini-owned VRAM/CGRAM/DMA register emulation for rendering and asset uploads
- [mini_game.c](../src/mini/mini_game.c): gameplay-state setup and per-frame update
- [stubs_mini.c](../src/mini/stubs_mini.c): parity adapter layer for room/bootstrap, ROM-backed assets, and remaining legacy globals

That split is intentional for future portability work. A Rust or other-language port
can replace the host loop and renderer independently before touching the gameplay
update path.

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

In mini, `NO_ROOMS` now means "no full room-loading / room-transition systems", not
"no room geometry at all". The mini runtime owns a small authored room so Samus can
move inside visible bounds without booting the full game room pipeline.

## Forward Plan

1. Keep the host layer thin: `src/mini/mini_main.c` and `src/mini/mini_runtime.c` should stay free of gameplay rules.
2. Shrink `src/mini/stubs_mini.c` by peeling room bootstrap and asset loading into narrower mini-owned modules.
3. Continue moving the gameplay kernel into mini-first files: `physics.c`, `physics_config.c`, `samus_input.c`, `samus_motion.c`, `samus_jump.c`, and `samus_collision.c`.
4. Add the second Samus slice after link stability: `samus_pose.c`, `samus_runtime.c`, `samus_draw.c`, `samus_speed.c`, and `samus_transition.c`.
5. Keep `sm_*.c`, room logic, demo flow, enemies, bosses, and audio out of mini until each dependency is either stubbed or split cleanly.
6. Extend tests from shell smoke to deterministic headless mini-state checks once Samus runtime is actually linked in.
