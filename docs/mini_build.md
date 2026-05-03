# Mini Build Shell

This repo now has a first-pass `mini` build target intended for subtractive refactoring work.

For the gameplay-kernel roadmap that reframes mini around moddable Samus movement,
collision, and authored map/nav rules, see [mini_modability_plan.md](mini_modability_plan.md).
For the staged path from the current mini runtime to a deterministic multiplayer-ready
kernel, including the next `physics.c` extraction target, see
[mini_multiplayer_roadmap.md](mini_multiplayer_roadmap.md).
For the current top-level plan and source ownership rules, see
[roadmap.md](roadmap.md) and [source_layout.md](source_layout.md).

Current scope:
- `make mini` builds `sm_rev_mini`.
- `make moddable` builds `sm_rev_moddable`, the user-facing authored movement
  sandbox variant compiled with `CURRENT_BUILD=BUILD_MODDABLE`.
- `sm_rev_mini` now defaults to the ROM-backed Landing Site slice when a ROM
  and compatible save/demo entry are available.
- `sm_rev_moddable` shares the mini host/kernel modules but defaults to
  editor-authored or fallback room data instead of selecting the ROM save/demo
  runtime.
- The default ROM path steps the shared original gameplay loop for that room,
  including original room runtime, PLMs, enemy setup, OAM drawing, Samus, physics,
  projectiles, palette FX, HDMA objects, and room main code.
- Explicit `--room-export PATH` still selects the editor-authored Landing Site
  sandbox path for fast movement/editor work.
- The editor/authored path can opt into the generated background with
  `--background generated` or `--ai-background`; the default remains `game`.
- The build is compiled with `CURRENT_BUILD=BUILD_MINI`, which now means
  "Landing Site content scope" rather than broad compile-time subtraction.
- Desktop OpenGL/GLSL host code lives under [`src/host/`](../src/host) and is
  intentionally outside the mini source set.
- The runtime supports `--headless --frames N` for smoke testing and a small SDL window for manual inspection.

## Current Mini Layers

The mini target is now split into clearer responsibilities under [`src/mini/`](../src/mini):
- [mini_main.c](../src/mini/mini_main.c): CLI parsing only
- [mini_runtime.c](../src/mini/mini_runtime.c): SDL/headless host loop and process orchestration
- [mini_input_script.c](../src/mini/mini_input_script.c): deterministic replay-script parsing
- [mini_replay.c](../src/mini/mini_replay.c): versioned replay artifact read/write and state-hash verification
- [mini_renderer.c](../src/mini/mini_renderer.c): software frame rendering and screenshot output
- [mini_asset_bootstrap.c](../src/mini/mini_asset_bootstrap.c): editor/ROM asset import, Samus visual bootstrap, and mini room sprite setup
- [mini_ppu_stub.c](../src/mini/mini_ppu_stub.c): mini-owned VRAM/CGRAM/DMA register emulation for rendering and asset uploads
- [mini_game.c](../src/mini/mini_game.c): gameplay-state setup and per-frame update
- [mini_content_scope.c](../src/mini/mini_content_scope.c): allowed mini content boundary, currently Landing Site only
- [mini_room_adapter.c](../src/mini/mini_room_adapter.c): editor/ROM/fallback room selection, collision-map setup, and room-boundary metadata
- [mini_system.c](../src/mini/mini_system.c): mini reset orchestration across WRAM, PPU, assets, and ROM bootstrap state
- [mini_platform_stubs.c](../src/mini/mini_platform_stubs.c): mini low-level platform, RTL, SRAM/audio no-op, and error shims

That split is intentional for future portability work. A Rust or other-language port
can replace the host loop and renderer independently before touching the gameplay
update path.

Linux:
- `make mini`
- `make mini-test`
- `make moddable`
- `make moddable-test`

Replay artifact smoke:
- `./sm_rev_mini --headless --frames 4 --input-script path/to/script.txt --replay-out out/mini_replay.json`
- `./sm_rev_mini --headless --replay-in out/mini_replay.json`

macOS:
- `make mini-mac`
- `make mini NATIVE_MAC=1`

The existing native macOS path uses SDL2 frameworks and turns on bundled assets by default for the full build. The mini runtime does not require a ROM and is the easiest target to validate first on macOS.

## Rust Rollback Host

`make mini-rust-host` builds `sm_rev_mini_rs`, a headless Rust host that drives
the C mini gameplay kernel through `MiniCreate`, `MiniStepButtons`,
`MiniSaveState`, `MiniLoadState`, and `MiniStateHash`.

Useful commands:

- `./sm_rev_mini_rs --frames 6`
- `./sm_rev_mini_rs --rollback --frames 18 --input-delay 3 --rollback-window 8 --trace`

Rollback mode keeps a fixed-size ring of pre-step snapshots, predicts input,
reveals actual delayed input later, rewinds to the changed frame, re-simulates
to the current frame, and compares the final per-frame hashes with a clean
reference run. A hash mismatch is reported as a desync and exits non-zero.

## Landing Site Parity Shape

Mini has moved past the first negative-only shell. The target is now:

- link the shared C gameplay engine
- default to the original ROM-backed Landing Site runtime when available
- reject or fall back from non-Landing Site room data
- keep SDL/headless/editor host code separate from gameplay code
- keep audio disabled for now with `NO_SOUND`

The editor-export path remains valuable, but it is not the parity authority. It is
the fast authoring/movement lane. The ROM-backed Landing Site path is the parity
lane. `sm_rev_moddable` makes that authoring lane explicit by reporting
`"build":"moddable"` and avoiding the ROM save/demo runtime by default.

Current deterministic coverage includes editor/authored-room state hashes,
authored slope/door/morph-tunnel/wall-jump/bomb-jump/doorway-transition
traversal checks, authored camera-follow target checks, rollback save/load
checks, and non-Landing editor-export rejection checks. ROM-backed Landing Site
frame-progression hash contracts run when a local ROM is available.

## Forward Plan

1. Keep the host layer thin: `src/mini/mini_main.c` and `src/mini/mini_runtime.c` should stay free of gameplay rules.
2. Keep new mini seams in named modules instead of reintroducing catch-all facade files.
3. Move remaining mini-only rendering substitutions toward shared original OAM/VRAM paths before expanding beyond Landing Site.
4. Keep full-build behavior authoritative: shared modules must continue to build and run in `sm_rev`.
5. Keep non-Landing Site content blocked until each dependency has an intentional parity boundary.
6. Broaden ROM-backed Landing Site assertions toward transition and room-state semantics once those contracts are stable enough.
