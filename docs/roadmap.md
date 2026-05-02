# Active Refactor Roadmap

This is the short current plan. Older topic plans remain useful as deep notes,
but this file is the top-level map for modability and portability work.

## Current Shape

- The bank-shaped gameplay files have been split into topical modules.
- `make` remains the full-game parity target.
- `make mini` now links the shared gameplay engine under a Landing Site content
  scope instead of being only a shell.
- `src/mini/` owns the deterministic mini API, replay/rollback helpers, editor
  bridge, mini renderer, and remaining mini shims.
- `src/host/` owns desktop OpenGL/GLSL renderer code that should stay outside
  the mini gameplay kernel.

## Working Order

1. Keep full-build parity intact.
2. Keep mini deterministic and Landing Site-scoped.
3. Expose narrow typed mod surfaces before adding new behavior.
4. Replace hex only when the domain meaning is known.
5. Move files only at real dependency boundaries.
6. Port host/runtime orchestration before porting gameplay logic.

## Near-Term Code Work

- Continue semantic cleanup in the Samus movement/collision slice:
  movement states, pose groups, slope/material flags, and camera/nav rules.
- Treat `src/block_reaction.h` as the shared block material contract for full,
  mini, and editor-exported collision data.
- Keep shrinking `src/mini/stubs_mini.c` into named mini modules when a shim
  becomes a stable boundary.
- Prefer typed snapshots and config structs for mini-facing state instead of
  adding more direct global reads.
- Keep boss/mod work behind explicit config and deterministic save-state tests.

## Mod Surface Goals

Authoring and tuning should move toward:
- `sm_physics.json` / `PhysicsParams` for movement tuning
- typed projectile snapshots for mini telemetry
- named collision materials and room geometry contracts built on the shared
  block type helpers
- boss-specific config modules, starting with Torizo
- editor-exported Landing Site data for mini rooms and assets

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
