# Mini Multiplayer Melee Plan

This is the near-term plan for a local two-player Samus-vs-Samus prototype on
top of `make mini`, with a path toward a browser host later.

The goal is not a full game mode yet. The goal is a deterministic combat slice:
two players in the Landing Site mini scene, both controlled from one keyboard,
both able to move, fire the basic beam, and register projectile hits against the
other player. Damage can stay internal until the HUD and rules are ready.

## Current Implementation Status

Status after the current implementation pass:

- `sm_rev_mini --players 2` enables a local two-player mini state and routes
  room selection through the editor-authored/fallback mini gameplay lane instead
  of the original ROM gameplay loop.
- `sm_rev_mini --multiplayer-demo --frames 21 --headless` runs a built-in
  exchange-fire script: Player 1 and Player 2 both fire, both receive a hit,
  and the final JSON reports `player1_hit_count`, `player2_hit_count`, and
  per-player pending damage.
- Headless input scripts accept `P1:` and `P2:` prefixes while legacy
  unprefixed tokens still drive Player 1.
- Replay artifacts preserve `player_count` and per-player button arrays.
- The mini kernel exposes `MiniStepPlayers(...)` for future Rust/browser hosts.
- Editor/fallback mini multiplayer uses the authored mini movement path for both
  players so local keyboard tests can move both actors immediately.
- Player 2 starts 48 pixels to the right of Player 1.
- Non-ROM mini rendering draws Player 2 with a deterministic mini-only marker.
- Authored mini multiplayer can spawn a basic beam with owner metadata.
- Projectile-vs-player collision records hit reception in
  `MiniPlayerCombatState` without decrementing HUD health.
- ROM-backed original-runtime mini remains the single-player parity path. Local
  two-player mini currently avoids that path because full two-actor ROM
  gameplay/rendering is not implemented yet.

Covered checks:

- `python3 -m pytest tests/test_mini_multiplayer.py -q`
- `make mini-test`
- `python3 -m pytest tests/test_build.py -q`
- `make`
- `make mini-rust-host && ./sm_rev_mini_rs --frames 3`

## Goal

Build a minimal multiplayer melee MVP with these properties:

- `make` keeps full-game parity.
- `make mini` remains the implementation target.
- The first playable mode is local keyboard only.
- The first room is the existing mini Landing Site scene.
- Player 1 and Player 2 spawn at different visible positions.
- Both players step on the same deterministic frame clock.
- Both players can move with Samus physics.
- Both players can fire the basic beam.
- Projectile-vs-player collision is calculated.
- Hit reception is represented in state, even if health/HUD damage is skipped.
- The gameplay kernel stays host-agnostic so Rust and browser hosts can wrap it.

## Recommendation

Do the local two-player mini prototype before the browser work.

Rust and WebAssembly are a good direction for the eventual host, but the
gameplay contract needs to exist first. The repo already has a C mini gameplay
kernel and a Rust headless rollback host. The fastest safe path is:

1. add the two-player deterministic gameplay seam in C mini,
2. drive that seam from the existing C SDL/headless mini host,
3. extend the Rust host to understand two player inputs,
4. then build a browser host around the same C ABI or a wasm-compiled kernel.

Do not rewrite Samus physics in Rust as the first step. Keep the validated C
Samus/physics code as the gameplay authority until the two-player contract and
tests are stable.

## Existing Starting Points

Useful existing pieces:

- [`src/mini/mini_game.c`](../src/mini/mini_game.c) owns mini gameplay init,
  stepping, state hashing, save, and load.
- [`src/mini/mini_runtime.c`](../src/mini/mini_runtime.c) owns SDL/headless
  host orchestration and can be replaced by another host later.
- [`src/mini/mini_rust_host.rs`](../src/mini/mini_rust_host.rs) already drives
  the C mini kernel through FFI and exercises rollback.
- [`src/samus_projectile_view.c`](../src/samus_projectile_view.c) exposes typed
  read-only projectile state for mini telemetry and rendering.
- [`src/multi_samus.c`](../src/multi_samus.c) has an experimental full-build
  state-swap approach for multiple Samus actors. Treat it as a reference idea,
  not the final mini API.
- [`tests/mini_rollback_api.c`](../tests/mini_rollback_api.c) already validates
  save/load/resimulation behavior for the mini kernel.

The key constraint is that original Samus runtime state is still mostly global.
The multiplayer slice should make that explicit: keep per-player snapshots at
the mini boundary, swap one player into the original global Samus state for a
step, then sync the result back to a typed mini player state.

## Non-Goals For The MVP

Do not include these in the first implementation:

- online netcode
- WebRTC
- damage HUD
- win/loss rules
- enemies, bosses, PLMs as combat actors, or room expansion
- equipment selection
- full item/ammo economy
- pause/map/equipment screens
- audio work
- broad Rust gameplay rewrite
- broad full-build multiplayer integration

## MVP Runtime Contract

The first concrete runtime contract should be:

```c
enum { kMiniMaxPlayers = 2 };

typedef struct MiniPlayerInput {
  uint16 buttons;
  uint16 previous_buttons;
  uint16 new_buttons;
} MiniPlayerInput;

typedef struct MiniPlayerCombatState {
  uint16 hit_count;
  uint16 pending_damage;
  uint16 hitstun_frames;
  uint16 invulnerable_frames;
  uint8 last_hit_by_player;
} MiniPlayerCombatState;

typedef struct MiniPlayerState {
  MiniSamusCoreState samus;
  MiniPlayerCombatState combat;
} MiniPlayerState;

typedef struct MiniMeleeHitEvent {
  bool active;
  uint8 attacker_player;
  uint8 defender_player;
  uint16 projectile_slot;
  uint16 damage;
} MiniMeleeHitEvent;
```

The exact names can change during implementation, but the shape should stay:

- player input is indexed by player
- player Samus state is indexed by player
- combat reception is indexed by player
- projectile hit events name attacker, defender, projectile slot, and damage
- state hash/save/load include all player and combat fields

For compatibility, `MiniInputState.buttons` can remain as player 1 input during
the transition. New code should prefer `player_buttons[2]` or an equivalent
typed array.

## Keyboard Controls

Use a deterministic local keyboard split and document it in one place.

Proposed default:

- Player 1: current default controls from [`src/default_controls.h`](../src/default_controls.h)
- Player 2:
  - `WASD` for directions
  - `F` jump
  - `G` run
  - `H` shoot
  - `T` select
  - `Y` start
  - `Q` aim down
  - `E` aim up

Implementation detail:

- add a mini-specific helper such as `MiniLocalControls_PollKeyboard()`
- keep full-build controls untouched
- make headless input scripts able to address players explicitly

Script shape should be simple and stable:

```text
P1:RIGHT P2:LEFT
P1:RIGHT,SHOOT P2:JUMP
P1:. P2:SHOOT
```

The parser can also accept unprefixed legacy tokens as Player 1 input to avoid
breaking existing mini tests.

## Spawn Contract

For the Landing Site MVP:

- Player 1 starts at the existing room spawn.
- Player 2 starts at a fixed offset from Player 1, visible in the first
  viewport.
- Both players must start outside solid tiles.
- Both players must start far enough apart that their hurtboxes do not overlap.

The first implementation can use fixed Landing Site offsets:

- Player 1: `room.spawn_x`, `room.spawn_y`
- Player 2: `room.spawn_x + 48`, `room.spawn_y`

If that collides with authored/fallback rooms, clamp Player 2 to the nearest
non-solid point using existing mini collision-map helpers. Do not broaden the
mini content scope just to find a better arena.

## Simulation Plan

### Phase 0: Documentation Only

Status: this document.

Deliverables:

- document MVP scope
- document architecture
- document acceptance checks
- avoid gameplay implementation until the plan is accepted

### Phase 1: Two-Player State And Input Shape

Goal: mini can represent two players without changing gameplay behavior yet.

Tasks:

- add a player count to `MiniGameState`
- add per-player input fields while preserving legacy single-player fields
- add per-player public telemetry to the headless JSON result
- update `MiniGameState_ComputeHash` to include player count and player inputs
- update `MiniSaveState` / `MiniLoadState` snapshot version
- extend replay artifacts to store per-player inputs
- extend input scripts with `P1:` and `P2:` prefixes
- add `--players 1|2` or `--multiplayer local` to the mini CLI

Tests:

- headless `--players 2 --frames 1` reports `player_count: 2`
- old single-player input scripts still pass
- P2-only input changes the state hash once P2 state exists
- save/load preserves player count and inputs

### Phase 2: Two Samus Actors In Mini

Goal: both players can move in Landing Site using the existing Samus runtime.

Current playable status: `--players 2` uses the authored/fallback mini movement
lane so both players can move and fight immediately. The original shared Samus
runtime state-swap remains the longer-term parity direction, not the active
local multiplayer path.

Tasks:

- add `src/mini/mini_multiplayer.c` as the mini-owned multiplayer boundary
- store per-player Samus snapshots in typed mini state
- use the current global Samus state only as a temporary execution context
- for each frame:
  - load Player 1 Samus snapshot into globals
  - apply Player 1 input as joypad 1
  - step Samus movement/projectile handling for Player 1
  - save Player 1 state back into `MiniGameState`
  - repeat for Player 2
- make Player 2 use the same physics path, not a separate authored movement
  copy
- keep camera behavior minimal at first: either follow Player 1 or use a fixed
  camera that keeps both starting positions visible

Do not let `src/multi_samus.c` become the mini API. Its state-swap approach is
useful, but mini needs explicit deterministic player state, save/load, hashes,
and replay inputs.

Tests:

- players spawn at different coordinates
- P1 input moves only Player 1
- P2 input moves only Player 2
- both players remain deterministic across repeated headless runs
- mini rollback save/load preserves both players

### Phase 3: Render Both Players

Goal: the local player test is visually usable.

Tasks:

- render both player Samus snapshots in mini windowed mode
- add a small deterministic visual distinction for Player 2
- avoid changing full-build Samus drawing behavior
- keep ROM-backed original OAM rendering authoritative for single-player mini
- decide whether multiplayer mini temporarily uses a mini-specific overlay path
  or a scoped original-draw replay per player

Preferred first visual distinction:

- tint Player 2 through a mini-only palette/render overlay, not by changing
  shared Samus palette state globally

Tests:

- screenshot smoke can detect two non-overlapping Samus silhouettes
- single-player ROM-backed mini screenshot tests stay unchanged

### Phase 4: Basic Beam Ownership

Goal: projectiles know which player fired them.

Tasks:

- add mini-only projectile owner metadata indexed by original projectile slot
- before stepping a player, snapshot active projectile slots
- after the player's shooting path runs, assign newly active slots to that
  player
- preserve existing projectile state and rendering paths
- expose projectile owner in mini telemetry and state hash

Collision rules:

- a projectile cannot hit its owner
- a projectile can hit the other active player
- basic beam hurtbox uses existing projectile view radius fields
- defender hurtbox uses `MiniSamusCoreState` radius fields
- collision emits a `MiniMeleeHitEvent`

Tests:

- Player 1 firing toward Player 2 produces a projectile owned by Player 1
- Player 2 firing toward Player 1 produces a projectile owned by Player 2
- owner projectiles do not self-hit
- hit events are deterministic and included in the state hash

### Phase 5: Hit Reception Without HUD Damage

Goal: the defender can "take the hit" without requiring the full damage HUD.

Tasks:

- add a mini-owned `MiniPlayerCombatState`
- on projectile hit:
  - increment defender `hit_count`
  - store `last_hit_by_player`
  - store `pending_damage` from projectile damage
  - set a short `hitstun_frames` or `invulnerable_frames` value
  - optionally clear/kill the projectile through the shared projectile lifecycle
- do not decrement the full `samus_health` global in the first pass unless a
  test explicitly covers it
- do not update HUD yet

Acceptance rule:

- the gameplay state records that damage would have been received
- visual hit feedback is useful but secondary
- health/HUD can be implemented after collision and hit reception are stable

Tests:

- projectile contact increments defender `hit_count`
- `pending_damage` equals the projectile damage used for the hit event
- repeated collision during invulnerability does not spam hit counts
- save/load preserves combat state

### Phase 6: Deterministic Regression Suite

Goal: multiplayer behavior is locked down before browser work.

Required checks:

- `make`
- `make mini-test`
- `make mini-rust-host`
- `python3 -m pytest tests/test_build.py -q`
- focused multiplayer tests for:
  - spawn separation
  - P1/P2 independent movement
  - deterministic state hash
  - replay artifact roundtrip with P2 input
  - rollback save/load with two players
  - projectile owner assignment
  - projectile hit reception

Add the focused tests before making visual or browser changes depend on the new
behavior.

### Phase 7: Rust Host Expansion

Goal: make the non-C host drive the same two-player kernel.

Tasks:

- extend the C ABI with a typed two-player step function, for example:

```c
void MiniStepPlayers(MiniGameState *state,
                     const uint16 *player_buttons,
                     int player_count,
                     bool quit_requested);
```

- update [`src/mini/mini_rust_host.rs`](../src/mini/mini_rust_host.rs) to pass
  two player inputs
- extend rollback driver records from one `u16` input to `u16 inputs[2]`
- add trace output for per-player predicted/actual inputs
- keep Rust responsible for host logic only

Tests:

- Rust host can step two players headlessly
- Rust rollback mode catches P2 delayed input changes
- C and Rust hosts produce matching hashes for the same two-player script

### Phase 8: Browser And WebAssembly Host

Goal: run the same mini kernel in a browser without SDL.

Recommended browser shape:

- Rust owns browser host code through `wasm-bindgen`.
- Browser input is mapped to the same two-player button arrays.
- Browser rendering writes to a canvas.
- The gameplay kernel is still the existing mini kernel.
- Netcode, if added later, lives in the Rust host layer.

Two viable wasm kernel options:

1. Compile the C mini kernel to WebAssembly and call its C ABI from the Rust
   browser host.
2. Compile a Rust host and C kernel together through a Cargo build script that
   uses a wasm-capable C compiler for the C sources.

Choose the simplest option that preserves the existing `MiniCreate`,
`MiniStepPlayers`, `MiniSaveState`, `MiniLoadState`, and `MiniStateHash` ABI.
Do not require SDL in the browser build.

Browser milestones:

- local browser page loads the wasm module
- canvas renders the Landing Site frame
- keyboard controls both players locally
- replay input can be loaded in browser for deterministic debugging
- state hash can be displayed for desync diagnostics
- only after that, add WebRTC or another network transport

## File Ownership Plan

Expected new or changed files:

- `src/mini/mini_multiplayer.h`
- `src/mini/mini_multiplayer.c`
- `src/mini/mini_local_controls.h`
- `src/mini/mini_local_controls.c`
- `src/mini/mini_game.h`
- `src/mini/mini_game.c`
- `src/mini/mini_input_script.c`
- `src/mini/mini_replay.c`
- `src/mini/mini_runtime.c`
- `src/mini/mini_main.c`
- `src/mini/mini_rust_host.rs`
- `tests/test_mini_multiplayer.py`
- `tests/mini_rollback_api.c`

Keep full-build files untouched unless a shared Samus/physics extraction is
needed and can be tested independently.

## First Implementation Slice

The initial slice is now implemented. It includes:

1. `--players 2`
2. `--multiplayer-demo`
3. per-player input storage
4. per-player headless JSON telemetry
5. save/load/hash coverage for player state
6. scripted/replay per-player input
7. basic beam ownership and projectile-vs-player hit reception
8. tests for movement, exchange-fire, and replay roundtrip

This creates the seam that the Rust/browser work can wrap later. Full two-actor
ROM-runtime gameplay remains outside this first slice.

## Open Decisions

Resolve these before implementation reaches the relevant phase:

- Should the first camera follow Player 1, midpoint between both players, or a
  fixed Landing Site viewport?
- Should Player 2 use a palette tint, outline, or HUD-less name marker?
- Should the first hit response include knockback, hit flash, or only state
  telemetry?
- Should `--players 2` be the final CLI, or should the mode be named
  `--multiplayer local`?
- Should projectile hit clear the projectile immediately, or let the original
  projectile lifecycle continue?

These should be answered as small implementation choices, not blockers for the
state/input slice.
