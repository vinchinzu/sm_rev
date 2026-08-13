# Mini Build Shell

This repo now has a first-pass `mini` build target intended for subtractive refactoring work.

For the gameplay-kernel roadmap that reframes mini around moddable Samus movement,
collision, and authored map/nav rules, see [mini_modability_plan.md](mini_modability_plan.md).
For the staged path from the current mini runtime to a deterministic multiplayer-ready
kernel, including the next `physics.c` extraction target, see
[mini_multiplayer_roadmap.md](mini_multiplayer_roadmap.md).
For the concrete local two-player melee MVP and browser/WASM staging plan, see
[mini_multiplayer_melee_plan.md](mini_multiplayer_melee_plan.md).
For the current top-level plan and source ownership rules, see
[roadmap.md](roadmap.md) and [source_layout.md](source_layout.md).

Current scope:
- `make mini` builds `sm_rev_mini`.
- `make moddable` builds `sm_rev_moddable`, the user-facing authored movement
  sandbox variant compiled with `CURRENT_BUILD=BUILD_MODDABLE`.
- `sm_rev_mini` now defaults to the ROM-backed Landing Site slice when a ROM
  and compatible save/demo entry are available.
- `sm_rev_mini --players 2` keeps the ROM-backed Landing Site runtime when it is
  available and enables the shared MultiSamus two-actor path.
- `sm_rev_mini --multiplayer-demo` enables two players and drives a built-in
  local input script for quick headless or windowed verification.
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
- [mini_audio.c](../src/mini/mini_audio.c): kernel-side APU bridge (SPC player, RTL APU shims, music/sfx queue stepping); no SDL
- [mini_audio_host.c](../src/mini/mini_audio_host.c): SDL audio device and mutex; installs lock hooks into the kernel bridge and stays out of `libsm_rev_mini_kernel.a`
- [mini_ppu_stub.c](../src/mini/mini_ppu_stub.c): mini-owned VRAM/CGRAM/DMA register emulation for rendering and asset uploads
- [mini_game.c](../src/mini/mini_game.c): gameplay-state setup and per-frame update
- [mini_frame_step.c](../src/mini/mini_frame_step.c): original-gameplay frame wrapper and shared multiplayer Samus stepping
- [mini_multiplayer_players.c](../src/mini/mini_multiplayer_players.c): multiplayer player runtime save/load, spawn, facing, and post-movement checks
- [mini_multiplayer_combat.c](../src/mini/mini_multiplayer_combat.c): two-player projectile ownership and hit reception
- [mini_net_bridge.c](../src/mini/mini_net_bridge.c): narrow C ABI for local browser multiplayer host snapshots, inputs, and per-player cameras
- [mini_content_scope.c](../src/mini/mini_content_scope.c): allowed mini content boundary for Landing Site and The Climb modes
- [mini_room_adapter.c](../src/mini/mini_room_adapter.c): editor/ROM/fallback room selection, collision-map setup, and room-boundary metadata
- [mini_system.c](../src/mini/mini_system.c): mini reset orchestration across WRAM, PPU, assets, and ROM bootstrap state
- [mini_platform_stubs.c](../src/mini/mini_platform_stubs.c): mini low-level platform, RTL, SRAM/audio no-op, and error shims

That split is intentional for future portability work. A Rust or other-language port
can replace the host loop and renderer independently before touching the gameplay
update path.

## The Climb (endless ascent)

- `./sm_rev_mini --climb-endless` loads original ROM room `0x96BA` when a ROM is available,
  with `assets/local_mini/room_96BA.json` as the editor-export fallback.
- `./sm_rev_mini --climb-original` loads the same original room state without virtual-floor
  wrapping, which is useful for comparing the original Space Pirate room behavior.
- Add `--no-rom` to force the editor-export / mini-sim path even when `sm.smc` is present;
  no-ROM climb uses the exported awake Space Pirate population and room sprite assets.
- Regenerate climb assets from ROM after collision export changes:
  `python3 tools/bundle_mini_room_assets.py --room 0x96BA`
- Samus starts centered on the bottom platform `(384, 2192)` with Morph
  and the default power beam (`equipped_beams = 0`, same loadout path as the ROM demo slice).
- The climb ROM bootstrap sets the original "Zebes awake" event so The Climb selects its
  original Space Pirate enemy population instead of the pre-event Roach population.
- The climb asset bundler applies that same event (`0`) at export time and records the
  selected room state / enemy population in `roomState`.
- Per-frame Samus input, movement, and projectile simulation use the shared
  `GameplayFrame_*` slices extracted from `GameState_8_MainGameplay` (`0x828B44`).
- Projectile drawing on the editor-render path uses `Samus_DrawActiveProjectiles()`
  (`DrawPlayerExplosions2` @ `0x938254`), the same decomposed draw half of
  `DrawSamusAndProjectiles` (`0x90EB35`).
- `mini_climb_endless` owns down-scroller camera follow and vertical world wrap
  (projectiles shift with each wrap via `MiniWorldShift_ApplyY`).
- Climb progress (`virtual_floors`, ascent score, lava) lives in
  `MiniGameState.climb` (`MiniClimbState`), so `MiniSaveState`/`MiniLoadState`,
  rollback, and `MiniStateHash` cover it with no module side channels. Run mode
  itself is process-level boot configuration (`mini_run_mode`), chosen by the
  host before kernel init and restored by `MiniLoadState`.
- Rising lava drives the run loop: after a 10s grace window each run, lava
  enters from below the bottom platform and rises faster with every virtual
  floor (Q8 speed, capped), never trailing Samus by more than ~1.5 screens.
  Contact drains energy on a fixed cadence; reaching zero energy (or any
  original-runtime death transition) restarts the run from the bottom platform
  with the score and lava reset while the deaths counter and session-best
  ascent persist.
- Wrap band selection is a deterministic per-floor shuffle whose window slides
  toward the harder high-row bands as the lava speed tier rises
  (`MiniClimbEndless_NextWrapTargetRow`), so platform layouts vary more and get
  sparser as the lava speeds up.
- Climb difficulty tier also tightens mini-sim Space Pirate fire cadence and
  gives their shots vertical aim toward Samus; on the ROM path the original
  enemy AI is authoritative.
- The HUD shows run clock, ascent score, floors, and energy, flashes red while
  submerged or low on energy, and adds a session line (best ascent | deaths)
  after the first death. The renderer overlays animated lava plus a
  bottom-of-screen proximity warning and a heat tint that grows with the tier.
- Known limitation: `MiniWorldShift_ApplyY` only shifts the live Samus globals,
  not per-player saved runtimes, so endless wrap is single-player only for now.

Linux:
- `make mini`
- `make mini-test`
- `make moddable`
- `make moddable-test`
- `make mini-browser-lib`
- `make mini-browser-server`

Replay artifact smoke:
- `./sm_rev_mini --headless --frames 4 --input-script path/to/script.txt --replay-out out/mini_replay.json`
- `./sm_rev_mini --headless --replay-in out/mini_replay.json`

Browser multiplayer MVP:
- `make mini-browser-lib` builds `libsm_rev_mini_net.so`, a narrow `MiniNet*`
  shared-library bridge over the C mini gameplay kernel and renderer.
- `make mini-browser-server` starts a local Python stdlib HTTP server on
  `127.0.0.1:8765` by default.
- Open `http://127.0.0.1:8765/p1` and `http://127.0.0.1:8765/p2` in two browser
  windows. Both windows drive the same running two-player kernel; each route
  gets a separate token and a camera centered on its assigned player.
- The browser client receives raw RGBA frames from `mini_renderer.c`; it does
  not redraw rooms, Samus, or projectiles with approximate JavaScript shapes.
- Use the same default mini keyboard layout in either browser window: arrows
  move, `X` jumps, `Z` runs, `S` shoots, `A` item-cancels, `C`/`V` aim.
- Input requests are sequence-ordered and held buttons expire without the
  browser heartbeat, preventing delayed HTTP requests or disconnected tabs
  from leaving controls stuck.
- The compact browser strip reports the latest deterministic melee hit event;
  the full fixed-capacity per-frame queue and cumulative dropped-event counter
  are available through browser JSON and the `MiniNetSnapshot` ABI.
- Override the bind address with `python3 tools/mini_browser_server.py --host 0.0.0.0 --port 8765`
  when another device on the LAN should connect.

Browser multiplayer tests:
- `python3 -m pytest tests/test_mini_browser_server.py -q`
- See [mini_multiplayer_architecture.md](mini_multiplayer_architecture.md) for
  the current review findings and staged match/rollback/release plan.

Deterministic multiplayer demo recording:
- `python3 tools/record_mini_multiplayer_demo.py` drives both player seats
  through the shared C kernel and writes `out/mini_multiplayer_demo.mp4`.
- The 60 fps recording includes beam volleys, independently selected vanilla
  missiles, missile trails/impact animations, deterministic collision sparks,
  player markers, ammo, and hit/damage counters.
- It exits nonzero unless the bout records hits in both directions, sees both
  missile flight and explosion states, and drops no hit events; gameplay state,
  ammo, ownership, and collision remain authoritative in the kernel.

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
