# Mini Multiplayer Architecture Review

This is the current architecture assessment and staged plan for turning the
two-player Landing Site demo into a production-quality deterministic mode
without widening the mini content scope or changing full-game behavior.

## Review Outcome

The demo now has a sound kernel boundary:

- `mini_game.c` owns public state, update ordering, save/load, and hashing.
- `mini_frame_step.c` owns the original-gameplay frame wrapper and shared
  per-player Samus stepping.
- `mini_multiplayer_players.c` owns typed player snapshots and temporary WRAM
  state swaps.
- `mini_multiplayer_combat.c` owns projectile ownership, collision, timers, and
  explicit melee hit events.
- `mini_net_bridge.c` exposes read-only host snapshots and per-player cameras.
- `mini_browser_server.py` owns browser sessions, ordered input delivery,
  stale-input release, and presentation only.

Projectile ownership is assigned at `InitializeProjectile`, while the active
Samus execution context is still known. This removes the former proximity and
direction heuristic, including its ambiguous simultaneous-fire behavior.

The deterministic state contract now includes a four-entry per-frame
`MiniMeleeHitEvent` queue, its cumulative dropped-event counter, projectile
owners, both player runtimes, combat timers, inputs, and all existing mini
state. Save-state version 12 records the same shape.

## Closed Review Findings

- Frame stepping no longer leaves the original-gameplay wrapper and the full
  per-player movement loop embedded in `mini_game.c`.
- `multi_samus.c` no longer exports its WRAM snapshots and bookkeeping globals;
  its known WRAM ranges and spawn offset have descriptive local names.
- Make object builds now emit and include compiler dependency files, so changes
  to shared state headers cannot silently mix incompatible old and new objects.
- New deterministic state is appended to `MiniGameState`, reducing incidental
  offset churn for existing typed views and host code.
- Projectile ownership is event-driven at allocation instead of inferred from
  actor distance after the frame.
- Combat produces a fixed-capacity queue of typed attacker/defender/slot/damage
  events available to C, headless JSON, and browser hosts. Same-frame trades
  retain both events in projectile-slot order.
- Browser inputs carry monotonically increasing sequence numbers, so a delayed
  key-down request cannot overwrite a newer key-up request.
- Browser-held controls automatically release after an input heartbeat timeout,
  preventing a disconnected tab from moving forever.

## Remaining Constraints

- The gameplay engine still uses one global WRAM execution context and swaps
  per-player Samus ranges. This is deterministic, but it is not reentrant and
  one process should host only one active match.
- The browser server is a local/LAN demo host, not internet-grade netcode. It
  has no authentication, encryption, prediction, rollback input exchange, or
  authoritative lobby service.
- Combat records damage and invulnerability state but does not yet apply stock,
  health, knockback, respawn, or win rules.
- Independent cameras reuse the authoritative simulation and renderer state;
  broad arbitrary-room BG2/OAM regeneration remains outside the Landing Site
  scope.

## Excellence Plan

### P0: Current demo quality [complete]

- deterministic spawn, movement, replay, rollback hash, beam ownership, and hit
  reception
- server-rendered frames and per-player follow cameras
- explicit hit telemetry and visible browser control guidance
- ordered browser input and stuck-key protection
- focused local multiplayer and browser regression tests

### P1: Match rules and diagnostics

- consume the fixed-capacity per-frame combat event queue in match rules
- add stocks or health, knockback, respawn points, round reset, and a match timer
- expose a compact scoreboard and connection/input-age indicators
- add deterministic tests for simultaneous hits, trades, invulnerability,
  respawn, and match completion
- add a replay-inspection command that prints the first divergent frame and
  state section instead of only the final hash

Exit gate: a two-player best-of match can run headlessly, replay exactly, and
report a winner without host-side gameplay rules.

### P2: Real rollback host

- move the browser host to the existing Rust-host direction
- exchange frame-numbered input records, predict only missing remote input, and
  keep a bounded snapshot ring
- resimulate from the earliest changed frame and compare periodic state hashes
- define input delay, rollback window, pause/disconnect, reconnect, and spectator
  contracts
- keep rendering and transport outside the C gameplay kernel

Exit gate: an artificial latency/jitter/loss test produces the same final hash
as a zero-latency reference match.

### P3: Internet transport and release hardening

- add WebRTC or a relay-backed transport only after P2 is deterministic
- add exclusive seat/lobby ownership, expiring reconnect tokens, rate limits,
  origin policy, TLS termination guidance, and bounded request bodies
- version the C ABI and wire protocol independently from save-state and replay
  formats
- add CI matrices for native Linux/macOS, sanitizer builds, and the selected
  browser/WASM toolchain

Exit gate: two remote browsers can complete a match through a supported secure
deployment while replay and desync artifacts remain reproducible locally.

## Required Validation

Every multiplayer architecture change should run, at minimum:

- `python3 -m pytest tests/test_mini_multiplayer.py tests/test_mini_browser_server.py -q`
- `make mini-test`
- `python3 -m pytest tests/test_build.py -q`
- `make`

Changes to snapshots, player switching, projectile allocation, or shared Samus
code should also run `make mini-rollback-test` and the full headless regression
suite when practical.
