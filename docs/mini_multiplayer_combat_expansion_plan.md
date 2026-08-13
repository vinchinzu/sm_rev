# Mini Multiplayer Combat Expansion Plan

This plan turns the current reciprocal power-beam test into a readable,
high-energy battle with deaths, missiles, explosions, and enough weapon variety
for human play and later self-play training. It extends the deterministic match
work in [mini_multiplayer_next_plan.md](mini_multiplayer_next_plan.md).

## Outcome

The first expanded showcase should demonstrate, in one deterministic bout:

- beam volleys rather than isolated test shots
- a charged shot and a limited-ammo missile
- direct-hit and world-impact explosions
- damage, knockback, death, stocks, respawn, and a round result
- clear Player 1 and Player 2 silhouettes, hit confirmation, and weapon state
- replay/rollback ending on the same winner and state hash

The C mini kernel remains authoritative. Rendering, browser code, and recording
tools may present events, but they must not decide damage, death, ammo, or match
results.

## Progress

The first weapon/feedback slice is complete:

- multiplayer now separates per-player weapon input from one shared projectile
  simulation tick, removing the P1/P2 shot-slot asymmetry
- each player has hashed/snapshotted beam-or-missile selection and three
  independent missiles, driven through vanilla Select and Shoot behavior
- normal missiles reuse vanilla type `0x8100`, 100 damage, acceleration, trail,
  cooldown, collision, audio, and type `0x8800` impact animation
- player impacts retain the native missile explosion instead of immediately
  clearing the projectile slot
- deterministic camera-correct collision sparks render every queued PvP hit,
  including both sides of a simultaneous trade
- the 60 fps demo now requires reciprocal hits, missile flight, missile
  explosion, zero dropped events, and reports symmetric 220 damage / 7 hits

Still pending: match health/death/stocks, respawn, charge shots, deterministic
splash events, knockback/shake, and audio/presentation polish beyond the reused
vanilla projectile cues.

## Required Order

### 1. Finish deterministic death and match rules

Implement the match-rules module described in the next execution plan before
adding new damage sources.

Tasks:

- add health, stocks, respawn timer, eliminated state, match phase, and winner
- consume each queued hit exactly once
- define death as health reaching zero and emit a deterministic defeat result
- disable movement, firing, and hurtbox interaction while defeated
- respawn at named Landing Site spawn points with a fixed invulnerability timer
- resolve simultaneous final-stock deaths as a draw
- reset players, projectiles, combat queues, ammo, and timers from one kernel API

Acceptance gate:

- scripted beam-only matches cover death, respawn, winner, draw, and reset
- save/load and delayed-input rollback reproduce the stock loss and final hash

### 2. Add a per-player weapon and ammo contract [normal missile slice complete]

Do not drive missiles through host-side button tricks. Add typed, snapshotted
state at the mini boundary:

```c
typedef enum MiniWeaponKind {
  kMiniWeapon_PowerBeam,
  kMiniWeapon_ChargeBeam,
  kMiniWeapon_Missile,
} MiniWeaponKind;

typedef struct MiniPlayerWeaponState {
  MiniWeaponKind selected;
  uint8 missiles;
  uint8 missile_capacity;
  uint16 charge_frames;
  uint16 cooldown_frames;
} MiniPlayerWeaponState;
```

Tasks:

- audit the current per-player WRAM snapshot around beam charge, HUD selection,
  missile ammo, projectile cooldown, and explosion slots
- preserve the original shared projectile implementation for beam and missile
  motion, collision, instruction lists, and OAM wherever mini can safely reach it
- make Select cycle weapons and make Shoot obey the selected weapon
- make held Shoot produce normal beam cadence and charge/release behavior rather
  than relying on widely spaced scripted taps
- give each stock a small, fixed missile allotment; reset it on respawn
- include weapon selection, ammo, charge, and cooldown in save/load, hashes,
  replay telemetry, the C network snapshot, and browser JSON

Initial tuning values belong in one versioned `MiniBattleTuning` structure,
not scattered constants. Preserve the current beam behavior as the baseline;
start missile ammo low enough that beam movement and aim still matter.

Acceptance gate:

- both players can independently select, fire, exhaust, and replenish missiles
- simultaneous mixed beam/missile fire retains correct projectile ownership
- charge and ammo state survive rollback without duplicated shots or ammo loss

### 3. Make missile impacts and explosions deterministic gameplay events [visual impact imported]

Add a fixed-capacity `MiniCombatFxEvent` queue, separate from the hit queue.
Suggested event kinds are beam impact, missile detonation, player defeat, and
respawn. Each event should include owner, world position, source projectile
slot, radius, start frame, and a monotonic sequence number.

Rules for the first pass:

- missiles detonate on a player or solid world collision
- direct-hit damage and optional splash damage are applied once in deterministic
  player-index order
- the shooter is immune to their own splash for the first balance pass
- an explosion may hit each eligible player at most once
- gameplay uses integer distance/box tests; rendering never performs collision
- overflow retains earliest events and increments a visible dropped-event count
- visual particle placement derives from event sequence and frame, with no
  untracked random-number state

Reuse the original missile and projectile-explosion instruction paths where
possible. If a mini-owned effect is still required, keep it in a focused
`mini_combat_fx.c/.h` module instead of adding special cases to the renderer or
the full-build projectile code.

Acceptance gate:

- player impact, wall impact, simultaneous detonations, splash edge, and queue
  overflow have deterministic tests
- explosions do not apply damage twice after save/load or rollback

### 4. Add combat feedback in layers

Add presentation only after event and rules state is testable:

1. health, stocks, missile ammo, charge, match timer, and phase in the HUD
2. distinct P1/P2 palette treatment and persistent overhead markers
3. beam trails, charge glow, missile exhaust, impact sparks, and expanding
   explosion rings
4. directional knockback, short hurt/armor flash, and a readable death burst
5. event-derived camera shake and one- or two-frame hit stop for strong impacts
6. weapon, impact, low-health, death, respawn, and round-over audio cues

Gameplay-affecting hit stop and knockback must be hashed kernel state. Pure
camera shake and particles can remain derived presentation as long as a replay
at a given frame produces the same image. Any renderer work must retain the
Landing Site jump test that rejects placeholder-purple background bands.

Acceptance gate:

- an observer can identify who fired, what hit, how much danger remains, who
  died, and who won without reading debug JSON
- screenshot/video checks cover P1/P2 distinction, missile trail, explosion,
  death, respawn, and round-over presentation

### 5. Replace the test-fire clip with a combat showcase

Extend `tools/record_mini_multiplayer_demo.py` to choreograph a 15–20 second
kernel-driven match:

- opening movement and a short reciprocal beam volley
- aerial exchange without horizontal background banding
- charge/release shot with a stronger hit cue
- missile launch, exhaust trail, impact explosion, and stock loss
- respawn with visible invulnerability
- final exchange and deterministic winner or draw slate

The recorder should fail unless its report sees both attacker directions, at
least two weapon kinds, a missile detonation, a stock loss, a respawn, a round
result, zero dropped gameplay/FX events, and the expected final state hash.

Acceptance gate:

- the video is visually inspected at start, both jump arcs, every explosion,
  respawn, and the ending slate
- the same input artifact reproduces the report and final hash in C and Rust

### 6. Make abilities tunable and training-safe

The expansion should leave one deliberate balance seam for humans and agents:

- versioned `MiniBattleTuning` values for health, stocks, movement multipliers,
  weapon damage, fire cadence, charge time, missile ammo, splash radius,
  knockback, invulnerability, and respawn delay
- tuning configuration included in the initial state hash and replay artifact
- observations expose rules and weapon state without renderer pixel scraping
- episode reset accepts a seed and tuning profile and returns a clean state
- evaluation reports damage, accuracy, weapon use, stocks, winner, timeout,
  dropped events, rollback corrections, and final hash
- human-v-agent difficulty profiles change only named tuning or policy settings,
  never hidden host-side assistance

This becomes the self-correction loop: run deterministic scripted baselines and
self-play evaluations, reject hash/event/render regressions, retain replay and
video artifacts for failures, then adjust a versioned tuning profile.

## First Implementation Slices

Keep each slice independently reviewable and playable:

1. health, stocks, death, respawn, and winner with beam-only combat
2. weapon state plus held-beam cadence and charge/release
3. missile firing, ammo, ownership, and direct hits
4. deterministic explosion events and splash damage
5. HUD, trails, sparks, shake, death burst, and audio
6. upgraded demo recorder and tuning/evaluation contract

Do not widen the room scope, add internet transport, or start RL optimization
until the beam-and-missile match passes replay and delayed-input rollback gates.
