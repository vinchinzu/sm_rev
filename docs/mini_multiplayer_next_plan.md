# Mini Multiplayer: Next Execution Plan

This is the immediate implementation plan after the deterministic multiplayer
and browser-demo architecture pass. The detailed architecture rationale remains
in [mini_multiplayer_architecture.md](mini_multiplayer_architecture.md). The
follow-on weapon, death-presentation, missile, explosion, and combat-showcase
work is specified in
[mini_multiplayer_combat_expansion_plan.md](mini_multiplayer_combat_expansion_plan.md).

## Objective

Turn the current two-player combat demo into a complete deterministic match:

- both players can trade hits without losing events
- damage has visible gameplay consequences
- players can be defeated and respawn
- a round can start, end, and reset
- replay and rollback reproduce the exact winner and final state hash

Keep the work inside the Landing Site mini scope. Do not add internet transport
until the match rules are deterministic under delayed-input rollback tests.

## Progress

- Step 1, the fixed-capacity hit-event queue, is complete. Same-frame trades
  retain both hits in projectile-slot order; save-state version 13, state
  hashes, headless JSON, the C network snapshot, and browser JSON include the
  queue and its cumulative dropped-event counter.
- A requested combat-feedback slice landed ahead of match rules: one shared
  projectile tick removes seat asymmetry, per-player normal missiles reuse the
  vanilla flight/impact path, and queued hits render deterministic sparks.
- The next implementation slice is step 2: deterministic match rules.

## Working Order

### 1. Replace the single hit event with a fixed event queue [complete]

The former `MiniGameState` stored only the latest hit event, so two hits in one
frame could overwrite each other.

Implement:

- add `kMiniMeleeHitEventCapacity`, initially 4
- replace `MiniMeleeHitEvent melee_hit_event` with a count plus fixed array
- clear the queue at the beginning of each combat update
- append every accepted hit in deterministic projectile-slot order
- define deterministic overflow behavior: retain the first events and increment
  a dropped-event counter
- expose the queue through headless JSON and `MiniNetSnapshot`
- increment the save-state version

Primary files:

- `src/mini/mini_game.h`
- `src/mini/mini_game.c`
- `src/mini/mini_multiplayer_combat.c`
- `src/mini/mini_net_bridge.h`
- `src/mini/mini_net_bridge.c`
- `src/mini/mini_runtime.c`
- `tools/mini_browser_server.py`

Tests:

- simultaneous exchange-fire records both attacker/defender pairs
- event order is stable across repeated runs
- save/load preserves the event queue and dropped-event counter
- state hashes differ when event contents differ

Exit gate: a same-frame trade reports two events without relying on accumulated
hit counters.

### 2. Add a deterministic match-rules module

Create `mini_multiplayer_rules.c/.h`. Combat detection should emit events; the
rules module should consume them and own match consequences.

Initial state shape:

```c
typedef enum MiniMatchPhase {
  kMiniMatchPhase_Countdown,
  kMiniMatchPhase_Playing,
  kMiniMatchPhase_RoundOver,
} MiniMatchPhase;

typedef struct MiniPlayerMatchState {
  uint16 health;
  uint8 stocks;
  uint16 respawn_timer;
  bool eliminated;
} MiniPlayerMatchState;

typedef struct MiniMatchState {
  MiniMatchPhase phase;
  uint16 phase_timer;
  uint32 match_timer_frames;
  uint8 winner_player;
  MiniPlayerMatchState players[kMiniMaxPlayers];
} MiniMatchState;
```

First rules:

- 99 health and 3 stocks per player
- subtract event damage exactly once
- zero health consumes one stock
- simultaneous final-stock defeats produce a draw
- defeated players become non-interactive during a fixed respawn delay
- round-over state freezes combat input but keeps rendering active
- Start after round-over resets the match deterministically

Do not add host-side rules. The browser, C host, and Rust host must only present
the kernel's match state.

Tests:

- damage, stock loss, respawn, elimination, winner, draw, and reset
- inactive players cannot fire or receive repeated damage
- replay roundtrip preserves the winner and ending hash
- rollback across a stock loss reproduces the reference result

Exit gate: a headless scripted match ends with the expected winner or draw and
replays to the same hash.

### 3. Add minimal combat feedback

After the rules state is stable:

- apply deterministic horizontal knockback based on attacker position
- add a short hurt flash using mini-owned render state
- show health, stocks, timer, phase, and winner in the browser strip
- add countdown and round-over presentation without putting timing rules in
  JavaScript
- distinguish Player 2 using a mini-only palette/tint path if the original OAM
  palette can be adjusted without changing full-build rendering

Then follow the staged weapon and presentation work in the combat expansion
plan: held-beam cadence and charge, per-player missiles/ammo, deterministic
explosion events, trails/sparks/shake/audio, and an upgraded showcase video.

Tests:

- knockback direction and magnitude are deterministic
- invulnerability prevents hit spam
- screenshot checks detect both player distinction and round-over presentation
- full-build rendering tests remain unchanged

Exit gate: two people can understand damage, stocks, respawns, and the result
without reading debug JSON.

### 4. Strengthen replay and desync diagnostics

Add tools before real networking:

- record per-frame player inputs, match phase, and state hash
- report the first divergent frame during replay verification
- report the first divergent state section: players, projectiles, combat events,
  match rules, WRAM, or PPU
- add a deterministic artificial input-delay/jitter driver to the Rust host
- run a clean reference and delayed rollback simulation from the same input log

Exit gate: every injected delay/jitter test ends on the reference hash, and a
deliberate corruption reports its first divergent frame and state section.

### 5. Add session and transport hardening

Only begin this after steps 1–4 pass:

- exclusive P1/P2 seats with expiring reconnect tokens
- spectator sessions that cannot submit player input
- bounded request bodies and input rate limits
- versioned network snapshot/wire contract
- disconnect, pause, forfeit, and reconnect rules
- Rust rollback host as the transport owner
- WebRTC or relay transport after local artificial-network tests pass

Exit gate: two remote clients can finish a secure match while the server retains
a deterministic replay and useful desync artifact.

## Completed First Implementation Slice

This slice implemented only step 1: the fixed hit-event queue. It closes the
known correctness gap and creates the input contract needed by match rules.

Acceptance checklist:

- [x] fixed-capacity queue lives in `MiniGameState`
- [x] both sides of a simultaneous trade are recorded
- [x] queue is hashed and snapshotted
- [x] C ABI, headless JSON, and browser JSON expose all events
- [x] focused multiplayer/browser tests pass
- [x] `make mini-test` passes from a header-consistent build
- [x] `python3 -m pytest tests/test_build.py -q` passes after `make`
- [x] full `make` remains unchanged in behavior

## Validation Commands

```sh
make
python3 -m pytest tests/test_build.py -q
make mini-test
python3 -m pytest tests/test_mini_multiplayer.py tests/test_mini_browser_server.py -q
make mini-rust-host
./sm_rev_mini_rs --rollback --frames 60 --input-delay 3 --rollback-window 8
```

## Explicitly Deferred

- broader rooms, enemies, bosses, PLMs, pause, map, equipment, save flow, and
  audio expansion
- rewriting Samus physics in Rust
- internet transport before deterministic match and rollback acceptance gates
- full-build multiplayer integration
