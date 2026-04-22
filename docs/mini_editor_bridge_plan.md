# Mini Editor Bridge Plan

This document defines the bridge between `../super_metroid_editor/` and
`sm_rev_mini` so the mini build can stop depending on broad ROM room imports
 while staying backward compatible.

## Goal

Short term:
- keep `make mini` working
- prefer structured Landing Site data exported from `../super_metroid_editor/`
- fall back to the current ROM-backed room bootstrap when the export is missing

Long term:
- keep only the Landing Site content and Samus art assets as imported data
- move gameplay, collision, and runtime behavior into the C port here
- let the editor remain the authoring surface for the room instead of making
  mini depend on ROM-level room decompression

## Current Bridge Shape

First implemented slice:
- `src/mini/mini_editor_bridge.c` reads a room export JSON file
- default search path includes
  `../super_metroid_editor/export/sm_nav/rooms/room_91F8.json`
- `src/mini/stubs_mini.c` now tries editor JSON before ROM save/demo boot
- exported collision/BTS grids are translated into `level_data` and `BTS`
- non-ROM rooms render through a local software block renderer in
  `src/mini/mini_runtime.c`
- headless output reports `room_source` so tests can verify which path booted

This reduces room-system ROM imports first, without breaking the old path.

## Data Ownership

Bridge-owned data from editor export:
- room dimensions
- collision grid
- BTS grid
- room identity metadata

Still ROM-backed today:
- Samus animation/sprite data
- room decorative art/tile graphics
- enemy sprite/bootstrap data such as the Landing Site ship

## Next Steps

1. Add an export focused on mini needs, not the full editor room model.
2. Move Landing Site spawn/camera metadata into export data instead of hardcoded defaults.
3. Export a compact editable visual layer for Landing Site so mini no longer needs ROM room art.
4. Narrow Samus asset imports to the minimum sprite/animation subset needed by mini.
5. Remove ROM-based Landing Site bootstrap once the editor-export path fully covers room visuals.

## Proper Asset Plan

The correct bridge is source-driven:
- `../super_metroid_editor` exports structured room and asset data
- `sm_rev_mini` loads that exported data directly
- a local generated cache exists only as a convenience artifact, not as the
  source of truth

What we should not do:
- do not snapshot mini runtime VRAM/CGRAM after loading a ROM room and call
  that the bridge
- do not depend on a baked BMP/PNG room render for gameplay visuals
- do not keep hardcoded Landing Site-only state if the editor can author it

Why:
- a VRAM snapshot is not editable at the room/tile level
- it hides whether a visual change came from room data, tileset data, or
  runtime state
- it would make later support for edited tiles, palettes, and room metadata
  harder, not easier

## Required Export Contract

The current room JSON is enough for collision/BTS, but not for visuals.
The mini bridge needs these additional editor-owned fields.

Room data:
- `blockWords`: full 16-bit layer-1 words per block, not only collision type
  and BTS
- `camera`: spawn/camera defaults for mini boot
- `scroll`: room scroll table and scroller metadata
- `tileset`: room tileset id and CRE flag

Tileset asset data:
- decompressed 4bpp tile graphics
- metatile word table
- tileset palettes
- any project-side tile or palette overrides already applied by the editor

Samus asset data:
- default object VRAM seed tiles
- weapon/common object tiles used by Samus rendering
- suit palettes
- the minimum DMA/tilemap data needed for the mini animation path, or a
  documented reduced export if we intentionally narrow supported poses

Optional later:
- Landing Site decorative sprites such as the gunship
- layer-2 or special-room data if mini grows beyond the current Landing Site
  target

## Implementation Order

Phase 1: Room Visual Export
- extend the editor export model so a room export includes `blockWords`,
  camera/spawn, and scroll metadata
- load those words in mini and render Landing Site through the metatile path
  instead of the placeholder collision renderer
- keep ROM fallback in place for backward compatibility

Phase 2: Tileset Asset Bridge
- add a mini-focused editor export for decompressed tileset graphics,
  metatiles, and palettes
- store generated files under `assets/local_mini/`
- make mini prefer the exported tileset bundle before any ROM room-art path

Phase 3: Samus Asset Bridge
- export the minimum Samus object tiles, palettes, and animation indirection
  needed by `samus_draw.c`
- replace the broad `RomFixedPtr` sprite bootstrap in mini with the exported
  Samus asset bundle
- keep a ROM fallback until rendered pose parity is good enough

Phase 4: Regression Harness
- add headless tests that compare editor-export and ROM-fallback room setup for
  Landing Site camera/scroll invariants
- add a smoke check that mini can boot from exported room + tileset assets
  without loading ROM room art
- only remove the placeholder renderer after the export-backed visual path is
  stable

## Generated Asset Folder

Generated bridge assets should live under:
- `assets/local_mini/`

Rules:
- ignore this directory in git
- treat it as a reproducible local cache
- keep schemas and loaders in source control, but not generated binary payloads

## Validation

- `make mini`
- `./sm_rev_mini --headless --frames 1`
- `./sm_rev_mini --headless --frames 1 --room-export ../super_metroid_editor/export/sm_nav/rooms/room_91F8.json`
