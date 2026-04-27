# Mini Multiplayer Roadmap

This roadmap is for the long path from the current `mini` shell to a
deterministic, reusable, multiplayer-friendly Samus runtime.

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

### Phase 0: Freeze The Rules Of Engagement

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

### Phase 1: Make Mini The Authoritative Gameplay Boundary

Short-term goal:

- make `mini` the place where Samus movement and collision become clean first
- keep the full build using the same cleaned topic modules whenever an extraction
  can be shared without changing behavior

Required seams:

- `MiniGameState_Init(...)`
- `MiniUpdate(...)`
- next: explicit state snapshot/restore helpers for deterministic replay and rollback

Near-term work:

- keep `src/mini/mini_runtime.c` free of gameplay rules
- keep peeling parity shims out of `src/mini/stubs_mini.c`
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
- Landing Site editor-room drift and scanline overlay logic now lives in `src/mini/mini_room_fx.c`, keeping the renderer focused on drawing primitives
- this is intentionally only the first chunk, not full generalized HDMA emulation for every room FX family

Current weapon boundary for mini:

- basic shooting uses the existing Samus projectile runtime instead of a mini-only duplicate
- projectile slot reset/clear/kill lifecycle now lives in `src/samus_projectile_state.c`, so the original full build and mini both use the same cleaner module boundary
- beam fire/cooldown/palette setup now lives in `src/samus_projectile_weapon.c`, keeping mini shooting and the original `sm_rev` build on the same weapon path
- `src/samus_projectile_view.c` exposes a typed read-only projectile snapshot for mini telemetry and rendering
- mini renders active beam projectiles from projectile state directly, leaving ROM spritemap/OAM projectile drawing out until the weapon slice needs full presentation parity
- scripted `SHOOT` input now has a deterministic mini regression that asserts a basic power-beam projectile is spawned

Next visual extraction order:

1. split ROM loading/save-slot/demo-room selection out of `src/mini/stubs_mini.c`
2. keep the editor-room ROM visual fallback narrow and safe; do not run full room setup scripts just to fetch visuals
3. replace remaining mini-only projectile/room-sprite presentation in the ROM path with original OAM/VRAM state
4. expand the new mini room-fx module from one-off sky bands to reusable per-scanline layer scroll rendering
5. only then add the next families such as rain, haze, and liquid layers

Success condition:

- mini can run the ROM-backed Landing Site room with original shared gameplay systems active
- mini can still run authored/editor Landing Site data for fast iteration
- mini room visuals are built from shared original-engine chunks rather than a growing pile of placeholder special cases

### Phase 2: Turn `physics.c` Into Dispatch, Not A Bucket

Current problem:

- [`src/physics.c`](../src/physics.c) still mixes dispatch tables, movement-state
  handlers, sound triggers, liquid checks, pose checks, and initialization

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

### Phase 3: Replace Magic Values Only Where The Semantics Are Proven

Use existing names first:

- `kMovementType_*` from [`src/ida_types.h`](../src/ida_types.h)
- `SamusPose` constants from [`src/ida_types.h`](../src/ida_types.h)
- `LiquidPhysicsType` from [`src/ida_types.h`](../src/ida_types.h)
- environment/equipment helpers in [`src/samus_env.h`](../src/samus_env.h)

Safe immediate replacements in the air-movement slice:

- `equipped_items & 0x200` -> `Samus_HasEquip(kSamusEquip_HiJumpBoots)`
- `samus_suit_palette_index & 4` -> gravity-suit helper or named suit-palette enum
- `fx_liquid_options & 4` -> `kFxLiquidOptions_Passthrough`
- `liquid_physics_type` raw tests -> `kLiquidPhysicsType_*`

Do not rename by guesswork:

- generic SFX ids with no confirmed meaning
- contact-damage modes that are still only partially traced

Success condition:

- movement code reads as gameplay rules, not bitfield archaeology

### Phase 4: Introduce Narrow Typed State At The Air-Movement Boundary

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

### Phase 5: Add Rollback-Ready Seams Before Any Large Language Port

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

### Phase 6: Port The Host First, Not The Gameplay Core

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

The next extraction should come from `physics.c`, but not all handlers are equally useful.

| Candidate | Why it matters | Risk | Recommendation |
| --- | --- | --- | --- |
| `Samus_Movement_03_SpinJumping` | Core air-state behavior, mini-critical, still contains liquid/equip/contact-damage archaeology | Medium | **Pick first** |
| `Samus_Movement_06_Falling` | Airborne baseline, simpler than spin jump, pairs naturally with air module | Low | Move soon after spin jump |
| `Samus_Movement_14_WallJumping` | Important for advanced movement and multiplayer feel | Medium | Move with rest of air cluster |
| `Samus_Movement_04_MorphBallOnGround` | Good mini value, but better as part of ball cluster | Medium | Defer until `samus_ball.c` exists |
| `Samus_Movement_0F_CrouchingEtcTransition` | Still ASM-shaped, but tied to pose-transition cleanup more than core movement feel | Medium | Defer |
| `Samus_Initialize` | Valuable eventually, but too broad for the first cleanup pass | High | Do not pick first |

## Chosen First Extraction

Pick:

- `Samus_Movement_03_SpinJumping` from [`src/physics.c`](../src/physics.c)

Move it to:

- `src/samus_air.c`

Why this one first:

- it is part of the mini-critical movement slice
- it already relies on existing air/jump helpers such as `Samus_SpinJumpMovement()`
- it mixes several concerns that should be named explicitly:
  - liquid detection
  - hi-jump re-jump window logic
  - screw/pseudo-screw contact damage selection
  - underwater spin-jump SFX
- it is small enough to extract without forcing a large bank-wide split

## Concrete Refactor Target For `Samus_Movement_03_SpinJumping`

Current code problems:

- local scratch names like `r18`, `r20`
- duplicated liquid checks already abstracted elsewhere
- raw equip bit `0x200`
- raw contact-damage values `3` and `4`
- raw SFX id `0x2F`
- direct byte reinterpretation of `samus_y_subspeed`

Recommended replacement shape:

```c
static SamusSpinJumpContext Samus_BuildSpinJumpContext(void);
static SamusRejumpWindow Samus_GetSpinJumpRejumpWindow(uint16 vertical_env);
static void Samus_TrySpinJumpRejump(const SamusSpinJumpContext *ctx);
static void Samus_UpdateSpinJumpContactDamage(const SamusSpinJumpContext *ctx);
static void Samus_TickSpinJumpLiquidSfx(const SamusSpinJumpContext *ctx);

void Samus_Movement_03_SpinJumping(void);
```

Recommended implementation order:

1. extract the function unchanged except for file move
2. replace liquid checks with `samus_env.h` helpers
3. add `SamusContactDamageMode`
4. add `SamusSpinJumpContext`
5. replace the raw re-jump thresholds with a named `SamusRejumpWindow`
6. only then consider naming the SFX id once it is traced in the audio tables

## Immediate Safe Replacements For That Extraction

These changes are low-risk because the repo already has evidence for them:

- use `Samus_HasEquip(kSamusEquip_HiJumpBoots)` instead of `equipped_items & 0x200`
- use `Samus_GetVerticalEnv()` or `Samus_IsSubmergedInWater(...)` from `samus_env.h`
- use `kLiquidPhysicsType_Water` and `kLiquidPhysicsType_LavaAcid`
- replace `samus_contact_damage_index = 3` with `kSamusContactDamage_ScrewAttack`
- replace `samus_contact_damage_index = 4` with `kSamusContactDamage_PseudoScrew`

These should wait until better source tracing:

- naming `QueueSfx1_Max6(0x2F)`
- generalizing suit-palette checks beyond the spin-jump use case

## First Implementation Sequence

If this roadmap is followed immediately, do the next work in this order:

1. create `src/samus_air.c` and move `Samus_Movement_03_SpinJumping` there
2. add a small air-movement header if needed, or keep the exported prototype in `physics.h`
3. add `SamusContactDamageMode` in a shared header
4. rewrite the extracted function to use `samus_env.h` helpers and the new enum
5. add a focused headless test or replay-script test that exercises spin-jump behavior in air and water
6. only then peel `Samus_Movement_06_Falling` and `Samus_Movement_14_WallJumping` into the same module

## What "Elegant Reused Code" Means Here

The project is succeeding if:

- `physics.c` becomes a dispatcher instead of a monolith
- the Samus gameplay kernel can run in full build and mini without copy-paste forks
- new movement features are added in topical modules, not bank files
- rollback multiplayer wraps the same deterministic gameplay step used by single-player mini
- Rust, if adopted, replaces the host shell first and only later the validated gameplay modules

That is achievable. The constraint is sequencing, not possibility.
