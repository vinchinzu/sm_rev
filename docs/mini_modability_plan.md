# Mini Core Modability Plan

This plan treats the `mini` build as the future **authoritative gameplay kernel**
for Samus movement, collision, and simple map/navigation rules.

It is **not** a plan to clean the largest `*_core.c` files first. In this repo,
"core" for modability means the smallest runtime that can answer:

1. How does Samus move?
2. What geometry can she collide with?
3. What minimal map/nav data is needed to simulate traversal?

Current baseline on 2026-04-20:
- `make mini-test` passes.
- `sm_rev_mini` is still a shell binary in [src/mini_main.c]
- Physics tuning already has a data seam in [src/physics_config.c]

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

After basic movement is linked into mini, extend config/data control to cover:
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

[src/features.h](/
already reserves `BUILD_MODDABLE`.

Use it only after the mini kernel is real. Suggested meaning:
- `BUILD_MINI`: shell / bring-up / narrow smoke harness
- `BUILD_MODDABLE`: actual movement sandbox with authored maps and stable gameplay contracts
- `BUILD_FULL`: full ROM-parity game

That keeps experimentation separate from the current shell while preserving the full build.

## Recommended Execution Order

1. Extract startup/shared app code out of `mini_main.c` so mini can host a real runtime loop.
2. Add `stubs_mini.c` for non-core calls that still leak into the Samus slice.
3. Link first movement slice:
   - `physics.c`
   - `physics_config.c`
   - `samus_motion.c`
   - `samus_jump.c`
   - `samus_collision.c`
   - if `samus_collision.c` still mixes movement-facing wrappers with block-map / pose-change logic, peel that heavy substrate into `samus_collision_advanced.c` before pulling more collision into mini
4. Clean magic values only in that slice.
5. Add minimal authored collision-map format for mini.
   - before pulling more of `samus_collision_advanced.c`, peel shared map helpers such as `CalculateBlockAt` into reusable files and let mini seed `level_data` / `BTS` with a hand-authored block map
   - next peel the block-dispatch / block-handle cluster into its own file so mini can link real movement-facing block collision while stubbing only exotic block reactions when necessary
6. Link second behavior slice:
   - `samus_pose.c`
   - `samus_transition.c`
   - `samus_speed.c`
   - `samus_camera_map.c`
7. Add deterministic traversal tests.
8. Only then decide whether `samus_runtime.c` should be adapted or partially replaced for mini.

## First Three Concrete Tasks

If starting immediately, do these in order:

1. **Create the mini runtime seam**
   - split reusable loop/input/timing code out of `mini_main.c`
   - add a tiny `MiniGameState` and `MiniUpdate()` entry point

2. **Link movement without room systems**
   - introduce `stubs_mini.c`
   - get `physics.c`, `physics_config.c`, `samus_motion.c`, `samus_jump.c`, and the smallest safe part of `samus_collision.c` building under mini
   - split `samus_collision.c` when needed so the movement-facing layer stays separable from room/block collision substrate

3. **Begin semantic cleanup where it matters**
   - replace the first wave of movement/collision magic values with named enums and constants
   - do not do file-wide cosmetic sweeps outside the linked mini slice

## Success Criteria

This plan is working when:
- mini can run Samus movement on a small authored test room
- movement rules are configurable without ROM archaeology
- collision/material semantics are named and testable
- new maps can be authored for mini without touching bank files
- the full build still compiles and can continue using parity adapters
