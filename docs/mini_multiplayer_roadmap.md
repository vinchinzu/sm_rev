# Mini Multiplayer Roadmap

This roadmap is for the long path from the current `mini` runtime to a
deterministic, reusable, multiplayer-friendly Samus runtime.

For the concrete local two-player melee MVP, including keyboard controls,
projectile ownership, hit reception, Rust host expansion, and browser/WASM
staging, see [mini_multiplayer_melee_plan.md](mini_multiplayer_melee_plan.md).

It is intentionally staged:

1. keep the current full build working
2. keep `make mini` growing as the clean gameplay kernel
3. extract and refactor the Samus/physics slice into topic-based modules
4. add deterministic save/load/step seams for rollback
5. only then move host/runtime pieces to Rust

This is not a rewrite-everything plan. It is a plan to make the existing
reverse-engineered code reusable instead of bank-shaped.

It also does not treat `mini` as the only code that deserves cleanup. The full
build should keep moving toward cleaner topic-based modules too, but through
small extractions and behavior-preserving refactors that are tested against the
original baseline before the next step lands.

## Why This Is Feasible

The repo already has the right high-level seam:

- [`src/mini/mini_runtime.c`](../src/mini/mini_runtime.c) owns the host loop
- [`src/mini/mini_game.c`](../src/mini/mini_game.c) owns gameplay setup and
  per-frame stepping
- [`docs/mini_build.md`](mini_build.md) already treats the mini host as
  replaceable before the gameplay kernel is rewritten

That means the right target is:

- thin host/runtime shell
- deterministic gameplay step
- topic-based Samus/physics modules
- full-build parity adapters around that core

## End State

The target architecture is:

- **Full build parity path**
  - keeps original behavior
  - continues to use ROM-backed adapters where needed
- **Mini gameplay kernel**
  - owns deterministic movement, collision, authored room geometry, and camera
  - exposes `init`, `save`, `load`, and `step` seams
- **Multiplayer host**
  - polls input, exchanges remote input, drives rollback, renders frames
  - can be a Rust shell around a C gameplay kernel first
- **Selective rewrite path**
  - only migrate Samus/physics modules after their behavior is isolated and tested

## Phase Plan

### Phase 0: Freeze The Rules Of Engagement [complete]

Status: Complete.

Completed work:

- the roadmap now states the ordering constraints explicitly: full-build parity
  first, deterministic seams second, and language migration last
- the mini scope is documented as a subtractive Samus/physics slice instead of
  an accidental second full-game build
- the validation expectations are captured in the project instructions and this
  roadmap, with `make`, `make mini-test`, and focused deterministic checks used
  before widening the slice

Before broad cleanup, hold these rules:

- do not break `make`
- do not let `mini` expand past its declared content scope by accident
- do not rewrite large bank files for aesthetics alone
- do not port to Rust before the gameplay seam is stable enough for rollback
- keep the full build moving toward topical modules in parallel with mini work
- prefer small extractions, small semantic cleanups, and immediate tests over large reorganizations
- compare refactored behavior against the original bank-shaped baseline before stacking more cleanup on top

Success condition:

- the team has one accepted rule: parity first, seams second, language migration last

### Phase 1: Make Mini The Authoritative Gameplay Boundary [complete]

Status: Complete for the current mini gameplay boundary.

Completed work:

- `src/mini/mini_runtime.c` remains the host/runtime shell and delegates gameplay
  work to `src/mini/mini_game.c`
- mini boots a named `landing_site_only` content scope and can use the
  ROM-backed Landing Site path when no explicit editor export is requested
- Samus projectile reset/fire/view behavior is shared with the full build
  through topical `samus_projectile_*` modules
- deterministic replay/rollback seams are now exposed by the mini gameplay API,
  including explicit save/load helpers

Short-term goal:

- make `mini` the place where Samus movement and collision become clean first
- keep the full build using the same cleaned topic modules whenever an extraction
  can be shared without changing behavior

Required seams:

- `MiniGameState_Init(...)`
- `MiniUpdate(...)`
- `MiniSaveState(...)` / `MiniLoadState(...)` for deterministic replay and rollback

Near-term work:

- keep `src/mini/mini_runtime.c` free of gameplay rules
- keep parity shims in named mini modules instead of catch-all facades
- keep the first Samus slice linked under mini:
  - `physics.c`
  - `physics_config.c`
  - `samus_input.c`
  - `samus_motion.c`
  - `samus_jump.c`
  - `samus_collision.c`
- keep moving room visuals and authored-room rendering toward reusable chunks from the original engine instead of placeholder-only logic

Current visual boundary for mini:

- `mini` now has a named `landing_site_only` content scope
- when no explicit editor export is requested and a ROM-backed Landing Site path is available, mini boots that original room path first
- the ROM-backed path steps the shared original gameplay loop and renders the generated original OAM buffer instead of mini-only Samus/enemy sprites
- `mini` now has a larger windowed shell while keeping the same internal logical gameplay resolution
- plain editor-export JSON without embedded asset paths can fall back to ROM-backed room visuals instead of dropping straight to placeholder rendering
- the first Landing Site background pass now uses original runtime-driven state to render visible scanline-varied sky/cloud bands
- editor BG2 rendering now asks `src/mini/mini_room_fx.c` for per-scanline layer scrolls instead of baking Landing Site-only drift into the renderer
- `src/mini/mini_room_fx.c` now owns renderer-side FX families for the current sky/cloud pass plus first liquid, rain, and haze overlays
- this is still a scoped renderer bridge for mini, not permission to widen the `landing_site_only` content boundary without a parity plan

Current weapon boundary for mini:

- basic shooting uses the existing Samus projectile runtime instead of a mini-only duplicate
- projectile slot reset/clear/kill lifecycle now lives in `src/samus_projectile_state.c`, so the original full build and mini both use the same cleaner module boundary
- beam fire/cooldown/palette setup now lives in `src/samus_projectile_weapon.c`, keeping mini shooting and the original `sm_rev` build on the same weapon path
- `src/samus_projectile_view.c` exposes a typed read-only projectile snapshot for mini telemetry and rendering
- editor/fallback mini still renders active beam projectiles from projectile state directly for the movement sandbox
- ROM-backed mini frames render original OAM/VRAM state instead of layering mini projectile or room-sprite substitutes on top
- scripted `SHOOT` input now has a deterministic mini regression that asserts a basic power-beam projectile is spawned

Completed visual extraction order:

1. [x] split ROM loading/save-slot/demo-room selection out of the old mini stubs facade
2. [x] keep the editor-room ROM visual fallback narrow and safe; do not run full room setup scripts just to fetch visuals
3. [x] replace remaining mini-only projectile/room-sprite presentation in the ROM path with original OAM/VRAM state
4. [x] expand the new mini room-fx module from one-off sky bands to reusable per-scanline layer scroll rendering
5. [x] only then add the next families such as rain, haze, and liquid layers

Completed follow-on visual work:

- [`src/mini/mini_rom_bootstrap.c`](../src/mini/mini_rom_bootstrap.c) now owns
  mini ROM/SRAM loading plus save-slot and demo-room boot selection
- [`src/mini/mini_room_adapter.c`](../src/mini/mini_room_adapter.c) keeps
  editor/fallback world setup and applies the ROM bootstrap's `MiniRoomInfo`
  result instead of owning save/demo startup details
- the editor-room ROM visual fallback still primes only the narrow visual state
  it needs for missing room FX, tileset, background, and Samus assets
- ROM-backed rendering no longer has a mini-only room-sprite path; original-runtime
  rooms render the OAM produced by the shared gameplay loop
- editor BG2 rendering now uses per-scanline scrolls from
  [`src/mini/mini_room_fx.c`](../src/mini/mini_room_fx.c), and the FX module has
  first-pass liquid, rain, and haze overlay families ready for future scoped
  room expansion

Success condition:

- mini can run the ROM-backed Landing Site room with original shared gameplay systems active
- mini can still run authored/editor Landing Site data for fast iteration
- mini room visuals are built from shared original-engine chunks rather than a growing pile of placeholder special cases

### Phase 2: Turn `physics.c` Into Dispatch, Not A Bucket [complete]

Status: Complete.

Completed shape:

- [`src/physics.c`](../src/physics.c) owns the movement dispatch table, the normal
  movement entry point, and unused handler shells only
- [`src/samus_ground.c`](../src/samus_ground.c) owns standing, running,
  crouching, moonwalking, turning-on-ground, and wall-bump ground behavior
- [`src/samus_air.c`](../src/samus_air.c) owns normal jump, spin jump, falling,
  wall jump, turning-in-air, and damage-boost behavior
- [`src/samus_ball.c`](../src/samus_ball.c) owns morph-ball and spring-ball
  ground/air/falling behavior
- [`src/samus_special_move.c`](../src/samus_special_move.c) keeps shinespark and
  adjacent special-move behavior clustered

Target shape:

- `physics.c` keeps dispatch tables and small top-level movement entry points
- topic modules own behavior clusters

Recommended eventual split:

- `physics.c`
  - movement dispatch table
  - `Samus_MovementHandler_Normal`
- `samus_ground.c`
  - standing, running, crouching, moonwalking, turn-on-ground
- `samus_air.c`
  - normal jump, spin jump, falling, wall jump, turning-in-air, damage boost
- `samus_ball.c`
  - morph-ball ground/air/fall and spring-ball variants
- `samus_special_move.c`
  - shinespark and other special-move handlers already clustered there

Success condition:

- a reader can tell where to look for "ground", "air", and "ball" behavior

### Phase 3: Replace Magic Values Only Where The Semantics Are Proven [complete]

Status: Complete for the proven mini movement slice.

Completed work:

- [`src/samus_env.h`](../src/samus_env.h) names the Samus equipment bits,
  contact-damage modes, suit-palette variants, liquid pass-through bit, and
  disabled-liquid sentinel used by the movement/palette slice
- the shared liquid classifier now names the original water-vs-lava ordering,
  so pose, spin-jump, landing-gfx, and animation-delay paths no longer repeat
  raw `fx_y_pos` / `lava_acid_y_pos` bit tests
- [`src/samus_air.c`](../src/samus_air.c) uses named hi-jump, liquid, screw,
  pseudo-screw, and pose constants in the spin-jump/wall-jump paths
- [`src/samus_palette.c`](../src/samus_palette.c) now uses the shared Samus
  equipment/liquid/suit-palette names in screw/speed-boost palette handling
- raw SFX ids that still lack table tracing remain intentionally unnamed except
  for the local underwater spin-jump constant

Use existing names first:

- `kMovementType_*` from [`src/ida_types.h`](../src/ida_types.h)
- `SamusPose` constants from [`src/ida_types.h`](../src/ida_types.h)
- `LiquidPhysicsType` from [`src/ida_types.h`](../src/ida_types.h)
- environment/equipment helpers in [`src/samus_env.h`](../src/samus_env.h)

Safe immediate replacements in the air-movement slice:

- `equipped_items & 0x100` -> `Samus_HasEquip(kSamusEquip_HiJumpBoots)`
- `equipped_items & 0x200` -> `Samus_HasEquip(kSamusEquip_SpaceJump)`
- `samus_suit_palette_index & 4` -> gravity-suit helper or named suit-palette enum
- `fx_liquid_options & 4` -> `kFxLiquidOptions_Passthrough`
- `liquid_physics_type` raw tests -> `kLiquidPhysicsType_*`

Do not rename by guesswork:

- generic SFX ids with no confirmed meaning
- contact-damage modes that are still only partially traced

Success condition:

- movement code reads as gameplay rules, not bitfield archaeology

### Phase 4: Introduce Narrow Typed State At The Air-Movement Boundary [complete]

Status: Complete.

Completed work:

- [`src/samus_env.h`](../src/samus_env.h) now exposes `SamusContactDamageMode`,
  `SamusSuitPaletteVariant`, `SamusSpinJumpContext`, and `SamusRejumpWindow`
- [`src/samus_air.c`](../src/samus_air.c) builds a `SamusSpinJumpContext` from
  globals once per spin-jump frame, then passes that typed context into the
  re-jump, contact-damage, and liquid-SFX helpers
- the re-jump window thresholds are named through `SamusRejumpWindow` instead
  of local scratch words

Do not try to replace all globals at once.

Instead, add narrow typed views around one behavior cluster at a time.

First useful types:

```c
typedef enum SamusContactDamageMode {
  kSamusContactDamage_None = 0,
  kSamusContactDamage_SpeedBoost = 1,
  kSamusContactDamage_Shinespark = 2,
  kSamusContactDamage_ScrewAttack = 3,
  kSamusContactDamage_PseudoScrew = 4,
} SamusContactDamageMode;

typedef enum SamusSuitPaletteVariant {
  kSamusSuitPalette_Power = 0,
  kSamusSuitPalette_Varia = 2,
  kSamusSuitPalette_Gravity = 4,
} SamusSuitPaletteVariant;

typedef struct SamusSpinJumpContext {
  uint16 vertical_env;
  bool submerged;
  bool can_rejump;
  bool screw_attack_active;
  bool pseudo_screw_active;
} SamusSpinJumpContext;

typedef struct SamusRejumpWindow {
  uint16 min_y_subspeed_hi;
  uint16 max_y_subspeed_hi;
} SamusRejumpWindow;
```

Notes:

- `SamusContactDamageMode` is already traceable from:
  - `samus_speed.c` for speed boost
  - `samus_special_move.c` for shinespark
  - `physics.c` and `samus_palette.c` for screw/pseudo-screw
- `SamusSuitPaletteVariant` is safe because current code already uses `0`, `2`, and `4`
  as meaningful palette-state values
- `SamusSpinJumpContext` should be filled from globals at first; it is not a full
  replacement for Samus state

Success condition:

- the first extracted movement function consumes named state instead of manually
  re-decoding globals every frame

### Phase 5: Add Rollback-Ready Seams Before Any Large Language Port [complete]

Status: Complete.

Implemented API:

- `MiniInit(...)`
- `MiniStep(...)`
- `MiniStepButtons(...)`
- `MiniSaveStateSize()`
- `MiniSaveState(...)`
- `MiniLoadState(...)`
- `MiniStateHash(...)`
- `MiniCreate(...)` / `MiniDestroy(...)` for FFI hosts

Snapshot coverage:

- `MiniGameState`
- mini room-boundary state from [`src/mini/mini_room_adapter.c`](../src/mini/mini_room_adapter.c)
- WRAM and SRAM
- mini PPU registers and VRAM from [`src/mini/mini_ppu_stub.c`](../src/mini/mini_ppu_stub.c)
- host-global flags needed by the mini runtime

Validation:

- [`tests/mini_rollback_api.c`](../tests/mini_rollback_api.c) saves a state,
  advances deterministic input, reloads, and verifies the re-simulated hash
- `make mini-rollback-test` builds and runs the focused rollback seam check
- `make mini-test` now runs the smoke test and rollback seam check

Multiplayer is not mainly a rendering problem. It is a deterministic state problem.

Required API shape:

- `MiniInit(...)`
- `MiniStep(input)`
- `MiniSaveState(buffer)`
- `MiniLoadState(buffer)`
- deterministic checksum/hash for state validation

Only after this exists should the project add:

- rollback netcode
- spectator/replay support
- remote desync detection
- a Rust host shell

Success condition:

- the gameplay kernel can be replayed, rewound, and re-simulated without SDL or host state leaks

### Phase 6: Port The Host First, Not The Gameplay Core [complete]

Status: Complete for the first headless host and rollback driver.

Completed work:

- [`src/mini/mini_rust_host.rs`](../src/mini/mini_rust_host.rs) is a Rust
  headless host that calls the C gameplay kernel through the phase-5 API
- `make mini-rust-host` builds `sm_rev_mini_rs` against
  `libsm_rev_mini_kernel.a`
- the Rust host initializes the C kernel, steps deterministic input, saves and
  reloads state, and reports hashes without moving gameplay code out of C
- `./sm_rev_mini_rs --rollback` keeps a ring of opaque C snapshots, injects
  actual input after a configured delay, rewinds to the changed frame,
  re-simulates back to the current frame, and reports hash desyncs against a
  clean reference run
- the SDL windowed host remains in C for now; the important boundary is that a
  non-C host can drive the validated gameplay kernel without owning gameplay

When the above seam exists:

- move the host/runtime/netcode shell to Rust first
- keep the gameplay kernel in C initially
- call the deterministic `step/save/load` C API from Rust

Only then consider migrating topical gameplay modules from C to Rust:

- `samus_air.c`
- `samus_ground.c`
- `samus_ball.c`
- collision/map helpers used by mini

Success condition:

- the project gets Rust ergonomics for host/network code without throwing away validated gameplay logic

## Physics Extraction Triage

Status: First air-movement extraction complete.

Completed extraction:

- [`src/samus_air.c`](../src/samus_air.c) now owns
  `Samus_Movement_02_NormalJumping`, `Samus_Movement_03_SpinJumping`,
  `Samus_Movement_06_Falling`, `Samus_Movement_14_WallJumping`,
  `Samus_Movement_17_TurningAroundJumping`,
  `Samus_Movement_18_TurningAroundFalling`, and
  `Samus_Movement_19_DamageBoost`.
- [`src/physics.c`](../src/physics.c) is now closer to a movement dispatcher for
  the air slice instead of carrying the spin/fall/wall-jump implementations.
- `Samus_Movement_03_SpinJumping` builds a `SamusSpinJumpContext`, uses named
  hi-jump/liquid/contact-damage helpers, and keeps the underwater spin-jump SFX
  id scoped locally.
- [`src/samus_ball.c`](../src/samus_ball.c) also already owns the morph-ball and
  spring-ball movement handlers that were previously listed as extraction
  candidates.
- [`tests/test_mini_spin_jump.py`](../tests/test_mini_spin_jump.py) provides a
  deterministic headless spin-jump replay check for the editor-export mini path.
- [`tests/test_mini_authored_room.py`](../tests/test_mini_authored_room.py)
  now includes deterministic authored morph-ball tunnel coverage, so
  `samus_ball.c` cleanup can start from a tested traversal contract.
- [`src/samus_ball.c`](../src/samus_ball.c) has a first small semantic cleanup
  pass for ball movement, naming the no-accel/no-input checks and preserving
  the Bank 90 x-dir branch shape with the shared pose-direction enum.
- The ball movement slice now uses a tiny `SamusGroundBallPosePair` for the
  Bank 90 ground-pose mapping and a named bounce-active predicate, keeping
  morph-ball and spring-ball movement on one shared path without adding new
  behavior.
- [`tests/mini_rollback_api.c`](../tests/mini_rollback_api.c) now covers the
  crouching/pose-transition movement handler's slope-collision dispatch and
  ball-bounce reset signal; [`src/samus_transition.c`](../src/samus_transition.c)
  uses that contract for a first no-`goto` semantic cleanup of
  `Samus_Movement_0F_CrouchingEtcTransition`.
- The same rollback test now covers the adjacent block-collision transition
  dispatcher for spin-jump, morph-ball, spring-ball, wall-jump, and ignored
  high-byte cases; `samus_transition.c` uses that coverage to name its
  block-collision subtypes, momentum handoff, current-facing pose selection,
  and ball-landing bounce tests while preserving the original branch contract.
- The landing/bounce transition helpers now have direct no-springball and
  springball assertions for first bounce, second bounce, terminal reset, and
  slope-collision setup. That let `samus_transition.c` share the vertical
  bounce setup/reset helpers without changing the original low-byte bounce
  state checks.
- The same transition contract test now pins the knockback and ground x-ray
  transition-table helpers, letting `samus_transition.c` name their button
  masks, knockback indices, x-ray angles, and setup timers without widening
  the transition-table cleanup beyond covered behavior.
- Grapple transition-table cleanup now shares the previous-position clamp and
  motion reset paths between the grapple-release handlers, with direct tests
  for both clamp directions and the no-change window.
- The adjacent A7 pose-adjustment transition now has direct tests for table
  offsets, zero-offset bounce reset, aim-up transition offsets, and unsupported
  stand-transition poses; `samus_transition.c` uses that coverage to remove the
  shared-label control flow and name the Y-offset table.
- Crateria landing graphics now have direct contract coverage for cinematic
  clearing, out-of-table rooms, FX-type-gated splashes, Y-threshold-gated
  splashes, always-splash rooms, and the Norfair room delegation; the handler
  now uses named rule flags instead of label jumps over the ROM rule table.
- The top-level Samus transition dispatcher now has direct contracts for the
  transitional, interrupted, normal-pending, no-pending, and grapple fallthrough
  paths. `samus_transition.c` now routes those through named helper functions
  instead of label jumps, while keeping the same A/B/C dispatch table ordering.
- `Samus_CheckWalkedIntoSomething` now has focused contracts for immediate
  solid-enemy running conversion and non-running no-op clearing; the handler no
  longer uses label jumps for the solid-enemy/move retry path.
- `samus_pose.c` now has focused contracts for the morph/springball facing-flip
  speed-carry helper, and the implementation names the flip test plus the
  extra-run-speed carry instead of jumping through a shared label.
- The crouch/stand/morph transition dispatch in `samus_pose.c` now has direct
  low-table, high-table, aim-up, and stand-transition range contracts, and the
  implementation names its table bases plus the transition momentum routine.
- The unsupported screw/space-jump pose downgrade helper now has direct
  equipment-gated contracts and uses named advanced-spin support checks instead
  of a shared downgrade label.
- The normal-jump pose-transition helper now has direct contracts for
  right/left shinespark windup selection, normal horizontal-accel setup,
  aim-up animation-frame carry, and shot-direction bookkeeping. The
  implementation now names the shinespark-eligible poses instead of jumping
  through shared labels.
- The spin-jump pose-transition helper now has direct contracts for
  opposite-facing extra-run-speed carry, screw attack remapping, space-jump
  remapping, and the liquid gate before advanced spin poses. With that
  coverage, the remaining label jumps in `samus_pose.c` were replaced by named
  carry/remap helpers while preserving the original equipment priority.
- The currently covered transition slice in `samus_pose.c` and
  `samus_transition.c` is now free of `goto`/`LABEL_` control flow.
- The adjacent runtime jump/sound/shoot checks now have direct contracts for
  health-flash input filtering, spin/wall-jump SFX selection, charging-beam
  resume state, echo sound cleanup, and debug invincibility preservation.
  `samus_runtime.c` no longer has `goto`/`LABEL_` control flow in those paths.
- The first-slice input helper cleanup now has direct contracts for
  pose-momentum selection and timed release filtering, and
  `samus_input.c` no longer uses label jumps for those helper paths.
- The speed/pose remap helper now has direct contracts for face-forward
  Varia/Gravity suit pose correction and screw/space-jump equipment remapping.
  `samus_speed.c` now names those pose decisions without shared label jumps.
- The horizontal shinespark movement helper now has direct contracts for speed
  capping and previous-X history clamping. `samus_special_move.c` now names the
  shared clamp path instead of jumping around the no-collision move/align block.
- The slow grapple camera-scrolling helper now has direct contracts for the
  left/right/up/down thresholds, the negative-X skip behavior, and Samus
  previous-position history copying. `samus_camera_map.c` no longer uses label
  jumps in the main scrolling routine.
- The transition bottom-half drawing helper now has direct contracts for the
  aim-up transition range, DB/DD frame gates, classic crouch/stand transition
  poses, and hidden fallback. `samus_draw.c` now names those visibility rules
  without a shared hide label.
- The x-ray HDMA/setup helpers now have direct contracts for behind-Samus beam
  hiding and fireflea-room color/blend setup. `samus_xray.c` is now free of
  `goto`/`LABEL_` control flow in those x-ray setup paths.
- The Crateria footstep splash selector now has direct contracts for cinematic,
  out-of-table, FX-gated, Y-threshold, and always-splash rooms. The corresponding
  `samus_anim_fx.c` room-rule path now uses named rule flags instead of label
  jumps.
- The wall-jump animation-delay selector in `samus_anim_fx.c` now reuses the
  shared liquid classifier and named equipment/SFX/frame-delta constants instead
  of jumping to the basic-spin delay label.
- The misc Samus palette hurt-flash and screw/speed-boost palette helpers now
  have direct contracts for damage-SFX gating, counter-40 sound refresh,
  screw-attack frame windows, wall-jump frame gates, and speed-boost palette
  advance. `samus_palette.c` is now free of `goto`/`LABEL_` control flow.
- Solid enemy collision now has direct contracts for no-interactive-enemy,
  inactive-enemy skip, frozen touching hits, property-solid gap hits, and both
  horizontal/vertical directions. `samus_enemy_collision.c` now names the
  solid-enemy eligibility and contact/gap decision instead of branching through
  shared hit labels.
- Projectile firing now has a direct super-missile gate contract for the
  selected four-projectile limit, the normal five-projectile limit, and cooldown
  blocking. `samus_projectile_weapon.c` now shares named beam collision/finish
  helpers for charged and uncharged beam setup instead of jumping through the
  collision labels.
- Square-slope block collision in `samus_collision_block.c` now names the
  primary/alternate quadrant hit decision for horizontal and vertical checks,
  leaving the original displacement and subpixel adjustment paths intact without
  label jumps.
- The matching post-grapple square-slope probe in `samus_grapple.c` now uses
  the same named primary/alternate quadrant decision before returning the
  original grapple collision distance.
- The remaining grapple cancel/swing/release helpers now name active-cancel
  checks, angle-gated swing input, swing animation frame selection,
  still-connected probing, and release-pose selection. With that pass,
  `samus_grapple.c` is also free of `goto`/`LABEL_` control flow.
- The remaining turn-transition pose tables now have direct contracts for
  standing, crouching, moonwalk jump-held, jumping, and falling turn dispatches.
  `samus_pose.c` names the previous-pose shot direction, moonwalk projectile
  flag, ground-turn table choice, and shared speed carry helper.

Remaining cleanup candidates:

No explicit cleanup candidates remain in this roadmap. Future work should be
added as a new scoped candidate with a parity plan and a focused test before
implementation starts.

## What "Elegant Reused Code" Means Here

The project is succeeding if:

- `physics.c` becomes a dispatcher instead of a monolith
- the Samus gameplay kernel can run in full build and mini without copy-paste forks
- new movement features are added in topical modules, not bank files
- rollback multiplayer wraps the same deterministic gameplay step used by single-player mini
- Rust, if adopted, replaces the host shell first and only later the validated gameplay modules

That is achievable. The constraint is sequencing, not possibility.
