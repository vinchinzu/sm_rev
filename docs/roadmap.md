# Active Refactor Roadmap

This is the short current plan. Older topic plans remain useful as deep notes,
but this file is the top-level map for modability and portability work.

## Current Shape

- The bank-shaped gameplay files have been split into topical modules.
- `make` remains the full-game parity target.
- `make mini` now links the shared gameplay engine under a Ceres content
  scope (Ceres Station plus Landing Site) instead of being only a shell.
- `make moddable` builds the authored movement sandbox variant on the same
  mini-family kernel while avoiding the ROM save/demo runtime by default.
- `src/mini/` owns the deterministic mini API, replay/rollback helpers, editor
  bridge, mini renderer, and remaining mini shims.
- `src/host/` owns desktop OpenGL/GLSL renderer code that should stay outside
  the mini gameplay kernel.
- `physics-hs/` is a subordinate pure model of the residual-relevant Samus
  fragment. It may grow that fragment (tables, air X, extra run, landing
  leftovers) for useful rollouts. It is not a second gameplay kernel.

## Working Order

1. Keep full-build parity intact.
2. Keep mini deterministic and Ceres-scoped.
3. Measure Mini–emulator residual on the residual-relevant state before
   widening the approximate model.
4. Keep `physics-hs` subordinate: pure rollouts, property tests, type-safe
   residual-relevant state, and cheap H↔M CI. Grow the fragment when SMB-
   style residual work shows the extra state is useful (tables, air X,
   extra run, land leftovers).
5. Do not expand Haskell into emulator-only behavior (full lag, complex
   enemy interactions, exact door transitions, slopes). Those are residual
   killers that force a re-sync, not a dual implementation.
6. Expose narrow typed mod surfaces before adding new behavior.
7. Replace hex only when the domain meaning is known.
8. Move files only at real dependency boundaries.
9. Port host/runtime orchestration before porting gameplay logic.

## Near-Term Code Work

- Continue semantic cleanup in the Samus movement/collision slice:
  movement states, pose groups, slope/material flags, and camera/nav rules.
- Treat `src/block_reaction.h` as the shared block material contract for full,
  mini, and editor-exported collision data.
- Keep stable mini seams in named modules such as `mini_room_adapter.c`,
  `mini_system.c`, and `mini_platform_stubs.c`.
- Prefer typed snapshots and config structs for mini-facing state instead of
  adding more direct global reads.
- Keep boss/mod work behind explicit config and deterministic save-state tests.
- Wire existing `MiniPredict` / `MiniStep` as the Haskell golden oracle for
  the implemented fragment only. H↔M observational agreement is a CI signal,
  not a research result.
- First Mini–emu residual is [mini_emu_delta.md](mini_emu_delta.md). Mini
  boot zeros `$0AFC`; corresponding-start walk is still unmeasured.
- Do not port slopes, walls, knockback, doors, or enemies into Haskell.
  Those belong in C stubs and M–E diagnostics first. The current Haskell
  fragment already includes ROM speed tables, air X, extra run, and
  landing leftovers.

## Mod Surface Goals

Authoring and tuning should move toward:
- `sm_physics.json` / `PhysicsParams` for movement tuning
- typed projectile snapshots for mini telemetry
- named collision materials and room geometry contracts built on the shared
  block type helpers
- room-authored camera/nav contracts such as mini `cameraFollow` targets
- boss-specific config modules, starting with Torizo
- editor-exported Landing Site data for mini rooms and assets
- `BUILD_MODDABLE` as the authored room/movement sandbox while `BUILD_MINI`
  remains the scoped parity harness

Avoid expanding mods by adding unrelated globals or patching one-off hex values
in the middle of behavior code.

## Portability Goals

The next portable layers are:
- mini headless host and rollback orchestration
- replay artifact reader/writer
- room/editor asset bridge
- renderer front ends

Gameplay modules should move only after mini save/load/step behavior is stable
enough to catch deterministic regressions.

## Canonical Docs

- [source_layout.md](source_layout.md): folder and source ownership policy.
- [mini_build.md](mini_build.md): current mini target behavior and commands.
- [mini_multiplayer_roadmap.md](mini_multiplayer_roadmap.md): detailed rollback
  and multiplayer path.
- [mini_modability_plan.md](mini_modability_plan.md): deeper Samus/physics mod
  plan.
- [bank_origin_map.md](bank_origin_map.md): original-bank lookup for regressions.
- [port_triage.md](port_triage.md): closed-out bank retirement status.
- [physics_haskell.md](physics_haskell.md): subordinate Haskell kernel posture.
- [physics_predict.md](physics_predict.md): Mini prediction API and Mini–emu
  residual stance.
