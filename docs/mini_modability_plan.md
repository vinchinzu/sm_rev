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

[src/features.h](/
already reserves `BUILD_MODDABLE`.

Use it only after the mini kernel is stable enough to be a user-facing sandbox.
Suggested meaning:
- `BUILD_MINI`: scoped parity/bring-up harness
- `BUILD_MODDABLE`: actual movement sandbox with authored maps and stable gameplay contracts
- `BUILD_FULL`: full ROM-parity game

That keeps experimentation separate from the current parity harness while preserving the full build.

## Recommended Execution Order

1. Keep `src/mini/mini_runtime.c` and `src/mini/mini_main.c` host-only.
2. Shrink `src/mini/stubs_mini.c` by moving stable ROM bootstrap, room scope,
   rendering, and editor bridge responsibilities into named mini modules.
3. Clean magic values only in the active movement/collision slice:
   - movement-state and pose constants
   - collision material and slope flags
   - camera/nav mode flags
4. Add deterministic traversal assertions beyond smoke tests.
5. Promote editor-export data from "fallback room source" toward the authored
   mini content surface.
6. Add or widen mod config only after the target behavior has deterministic
   save/load coverage.

## Next Three Concrete Tasks

1. **Name more collision semantics**
   - continue replacing proven block/slope/material masks with
     [`src/block_reaction.h`](../src/block_reaction.h) constants and helpers
   - keep unsigned SNES-era arithmetic intact while naming the rule being applied

2. **Type the mini-facing state boundary**
   - add narrow snapshots for Samus state, controls, projectiles, and room
     collision where mini currently reads broad globals
   - keep full-build adapters beside existing globals until parity is proven

3. **Make authored rooms less ROM-shaped**
   - load spawn/camera/material metadata from editor export data
   - keep the ROM-backed Landing Site path as the parity reference

## Success Criteria

This plan is working when:
- mini can run Samus movement on a small authored test room
- movement rules are configurable without ROM archaeology
- collision/material semantics are named and testable
- new maps can be authored for mini without touching bank files
- the full build still compiles and can continue using parity adapters
