# Mini Core Modability Plan

This plan treats the `mini` build as the future **authoritative gameplay kernel**
for Samus movement, collision, and simple map/navigation rules.

It is **not** a plan to clean the largest `*_core.c` files first. In this repo,
"core" for modability means the smallest runtime that can answer:

1. How does Samus move?
2. What geometry can she collide with?
3. What minimal map/nav data is needed to simulate traversal?

Current baseline on 2026-05-02:
- `sm_rev_mini` has moved past the shell milestone.
- [`src/mini/mini_main.c`](../src/mini/mini_main.c) is CLI parsing only.
- [`src/mini/mini_runtime.c`](../src/mini/mini_runtime.c) owns the SDL/headless host loop.
- [`src/mini/mini_game.c`](../src/mini/mini_game.c) exposes deterministic create,
  step, save/load, and hash seams for replay/rollback.
- The default mini path can boot a ROM-backed Landing Site slice when compatible
  assets are present; explicit editor exports remain the fast authoring path.
- Physics tuning already has a data seam in [src/physics_config.c](../src/physics_config.c).

Implemented checkpoint on 2026-05-02:
- editor exports can provide named `materials` grids backed by
  [`src/block_reaction.h`](../src/block_reaction.h) block types instead of raw
  collision/BTS grids
- material-only authored Landing Site rooms can run deterministic Samus
  traversal without ROM-backed pose tables through
  [`src/mini/mini_authored_movement.c`](../src/mini/mini_authored_movement.c)
- authored traversal now includes a room-authored camera-follow target that is
  clamped to the authored room bounds
- the authored mini movement path reads the existing `PhysicsParams` /
  `sm_mods.json` tuning seam
- [`src/mini/mini_game.h`](../src/mini/mini_game.h) exposes typed viewport,
  room, collision-map, Samus, control, and projectile views while keeping
  compatibility fields synchronized for existing callers
- stable low-level mini platform/RTL shims have been split from `stubs_mini.c` into
  [`src/mini/mini_platform_stubs.c`](../src/mini/mini_platform_stubs.c)
- stable editor/ROM/fallback room selection and collision-map adapter code has
  been split from `stubs_mini.c` into
  [`src/mini/mini_room_adapter.c`](../src/mini/mini_room_adapter.c) and
  [`src/mini/mini_room_adapter.h`](../src/mini/mini_room_adapter.h)
- the remaining reset facade has been retired into
  [`src/mini/mini_system.c`](../src/mini/mini_system.c), and editor asset views
  are exposed directly from
  [`src/mini/mini_asset_bootstrap.h`](../src/mini/mini_asset_bootstrap.h)
- deterministic authored traversal coverage now includes a multi-tile authored
  slope segment, a passable door marker at the camera boundary, an airborne
  wall-jump contact, a morph-ball tunnel, a Down+Shoot authored bomb-jump
  impulse, and configured doorway transitions
- authored room exports can tune `cameraFollow.targetXPercent` /
  `targetYPercent`, and the resulting camera target is covered by deterministic
  headless state-hash assertions
- non-Landing authored room exports have deterministic content-scope coverage so
  mini keeps rejecting or falling back until those dependencies have parity
  boundaries
- ROM-backed Landing Site runs now have deterministic frame-progression
  state-hash assertions when a local ROM is available
- `BUILD_MODDABLE` now builds as `sm_rev_moddable`, reports
  `"build":"moddable"` in headless JSON, and defaults to authored/editor or
  fallback room data instead of selecting the ROM save/demo parity runtime
- focused coverage lives in
  [`tests/test_mini_authored_room.py`](../tests/test_mini_authored_room.py) and
  typed-boundary rollback assertions live in
  [`tests/mini_rollback_api.c`](../tests/mini_rollback_api.c)

## Target Shape

The long-term target is a split architecture:

- **Parity adapters**
  - Full-build code that still reads ROM tables, bank data, and legacy globals.
  - Responsible for translating original game data into the new core-facing forms.
- **Mini core runtime**
  - Small, deterministic, testable movement and collision engine.
  - Owns Samus kinematics, environment flags, tile/material collision, and simple camera/nav rules.
- **Authorable content layer**
  - Fresh mini maps and scenarios can be created without ROM extraction knowledge.
  - Uses named materials, movement presets, spawn definitions, and rule tables instead of raw hex.

The full game should continue to build around this. The mini path should become the
clean place where new mechanics, experiments, and mods are introduced first.

Important project rule:

- keep the full build moving toward cleaner topic-based modules too
- do that in small parity-preserving steps, not broad rewrites
- run the smallest relevant test around each extraction when practical
- check regressions against the sibling `../sm/` baseline before assuming the new split is correct

## Non-Goals

Do not start with:
- `plm_core.c`
- generic bank-wide hex scrubbing
- pause/map/menu systems
- enemies, bosses, demos, audio, or room-state scripting

Those may need cleanup later, but they are not the leverage point for a moddable
movement sandbox.

## Phase 1: Define The Mini-Core Boundary

First milestone: explicitly define which files are part of the mini gameplay kernel.
That boundary now exists, but it still needs tightening as more globals become
typed mini-facing state.

Adapter layer, not core:
- `samus_input.c`
- `samus_runtime.c`
- `samus_draw.c`

Excluded from early mini core:
- `samus_demo.c`
- `samus_palette.c`
- `samus_xray.c`
- `samus_special_move.c`
- room setup / room transitions
- PLM / enemy / eproj systems

Practical rule:
- If a file mixes core movement with heavy full-game dependencies, split the movement
  cluster out before trying to link it into mini.
- When making that split, prefer the smallest behavior-preserving extraction that
  improves both mini reuse and the readability of the full build.

## Phase 2: Replace Magic Hex With Domain Names In The Mini Slice

This is the right place to do readability work, but only inside the kernel slice.

Priority order for constant cleanup:
1. Samus movement-state values
2. Pose groups used by transitions
3. Collision/material flags
4. Environment variants
5. Camera/nav mode flags

Preferred replacements:
- raw pose ids -> existing named pose constants from `ida_types.h`
- bit tests -> named flags/enums
- special tile values -> named block/material constants
- repeated numeric thresholds -> named gameplay constants

Avoid this anti-pattern:
- replacing every hex literal in a file with a `#define` that still has no meaning

The goal is not "no hex exists." The goal is "a reader can tell what system rule is being applied."

## Phase 3: Introduce Core-Facing Types

Modability needs typed state boundaries. The next step is to stop letting the mini
runtime depend directly on scattered globals for every rule.

Add narrow core-facing types such as:
- `SamusCoreState`
- `SamusControlState`
- `MovementEnvironment`
- `CollisionMap`
- `CollisionMaterial`
- `NavRoom` or `MiniRoom`

Important constraint:
- do not rewrite the whole game around these immediately
- first add adapter functions that fill these from legacy globals in the full build
- let mini own the clean versions first

This gives the project a place where "fresh mini version" content can be authored
without inheriting the full ROM/global layout.

## Phase 4: Build A Minimal Map/Nav Substrate

The mini runtime does not need full room loading first. It needs a small geometry
contract that is enough for movement and traversal testing.

Start with:
- solid / air / slope / hazard / door marker materials
- room dimensions
- spawn point
- camera bounds
- optional nav markers such as ladders, ledges, tunnels, one-tile gaps

Recommended approach:
- define a tiny hand-authored map format for mini
- keep a separate adapter path that can derive the same collision/material view
  from existing room data later

This prevents the mini path from being blocked on the entire room system.

## Phase 5: Expand Data-Driven Rules Beyond Raw Physics Numbers

[src/physics_config.c]
already exposes a strong seam for numeric tuning. Use that as the template for
broader modability.

As mini movement and room paths stabilize, extend config/data control to cover:
- movement presets
- friction / acceleration families
- jump families
- environment modifiers
- collision-material responses
- camera tuning

Keep the config layered:
- vanilla defaults in code
- optional JSON overrides for mini
- full-build parity adapters can still map ROM-era values into the same runtime fields

## Phase 6: Deterministic Traversal Tests

Modability without deterministic tests becomes guesswork.

Add headless mini tests for:
- flat-ground acceleration and deceleration
- jump arc invariants
- wall-jump and bomb-jump cases
- slope traversal
- doorway/camera-boundary transitions
- simple nav reachability on hand-authored rooms

Tests should assert world-state outcomes, not just "program exits 0".

## Phase 7: Promote `BUILD_MODDABLE` From Placeholder To Product Shape

Status: Initial product shape promoted.

[`src/features.h`](../src/features.h) reserves `BUILD_MODDABLE`, and the
Makefile now exposes it through `make moddable` / `sm_rev_moddable`.

Current meaning:
- `BUILD_MINI`: scoped parity/bring-up harness
- `BUILD_MODDABLE`: actual movement sandbox with authored maps and stable gameplay contracts
- `BUILD_FULL`: full ROM-parity game

`BUILD_MODDABLE` intentionally shares the mini host/kernel modules, but its
default room selection skips the ROM save/demo runtime and lands on editor
exports or fallback authored geometry. Explicit `--room-export` remains the
primary authored-map path. The build still keeps the Landing Site content scope
until non-Landing Site dependencies have deliberate parity boundaries.

That keeps experimentation separate from the current parity harness while
preserving the full build.

## Recommended Execution Order

1. Keep `src/mini/mini_runtime.c` and `src/mini/mini_main.c` host-only.
2. Keep stable room, rendering, reset, and editor bridge responsibilities in
   named mini modules rather than catch-all facade files.
3. Clean magic values only in the active movement/collision slice:
   - movement-state and pose constants
   - collision material and slope flags
   - camera/nav mode flags
4. Add deterministic traversal assertions beyond smoke tests.
5. Promote editor-export data from "fallback room source" toward the authored
   mini content surface.
6. Add or widen mod config only after the target behavior has deterministic
   save/load coverage.

## Completed Current Tasks

1. **Name more collision semantics**
   - material-grid editor exports now use
     [`src/block_reaction.h`](../src/block_reaction.h) constants and helpers
   - the mini collision view exposes block material names through typed accessors

2. **Type the mini-facing state boundary**
   - `MiniGameState` now carries narrow snapshots for viewport, room,
     collision-map metadata, Samus state, controls, and projectiles
   - compatibility fields remain populated beside the typed state until callers
     are migrated

3. **Make authored rooms less ROM-shaped**
   - material-only room exports can define spawn/camera/material metadata without
     numeric ROM-shaped collision grids
   - keep the ROM-backed Landing Site path as the parity reference

4. **Broaden authored traversal assertions**
   - authored movement now has deterministic coverage for multi-tile slope
     traversal, door-marker pass-through at the camera boundary, morph-ball
     tunnel traversal, wall-jump contact, bomb-jump launch, and configured
     doorway transitions
   - headless mini JSON now exposes world-position and grounded state telemetry
     for traversal assertions

5. **Split stable mini adapters**
   - low-level platform/RTL shims now live in
     [`src/mini/mini_platform_stubs.c`](../src/mini/mini_platform_stubs.c)
   - room selection, collision-map setup, room metadata, and camera clamping now
     live in [`src/mini/mini_room_adapter.c`](../src/mini/mini_room_adapter.c)
   - reset orchestration now lives in
     [`src/mini/mini_system.c`](../src/mini/mini_system.c), and editor asset
     view access is direct through
     [`src/mini/mini_asset_bootstrap.h`](../src/mini/mini_asset_bootstrap.h)

## Remaining Strategic Follow-Ups

- Grow `BUILD_MODDABLE` beyond the current Landing Site content scope only after
  room transitions, rendering substitutions, and collision dependencies have
  explicit parity boundaries.
- Continue adding broader authored gameplay config in small room-data contracts
  once each behavior has deterministic save/load or state-hash coverage.

## Success Criteria

This plan is working when:
- mini can run Samus movement on a small authored test room
- movement rules are configurable without ROM archaeology
- collision/material semantics are named and testable
- new maps can be authored for mini without touching bank files
- the full build still compiles and can continue using parity adapters
