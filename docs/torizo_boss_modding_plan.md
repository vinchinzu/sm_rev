# Torizo / Boss Modding Plan

Date: 2026-05-01

This plan is for continuing the current Bomb Torizo / Golden Torizo modding work
without turning the codebase into a spaghetti hydra wearing somebody else's hat.

## Current Snapshot

What exists right now:

- `src/enemy_torizo.c` is still the main behavior file, but some useful seams now exist.
- New extraction/config files exist:
  - `src/torizo_config.c`
  - `src/torizo_config.h`
  - `src/enemy_torizo_projectiles.c`
  - `src/enemy_torizo_finale.c`
- Runtime config and docs exist:
  - `sm_torizo.json`
  - `docs/torizo_mods.md`
- Supporting integration changes exist in:
  - `src/main.c`
  - `src/physics.c`
  - `src/enemy_collision.c`
  - `src/eproj_combat.c`
  - `src/funcs.h`
  - `docs/bank_origin_map.md`

Observed mod surface already in place:

- Chozo orb count multiplier
- Orb contact-damage multiplier
- Orb palette override
- Orb spawn jitter
- Optional Bomb Torizo opening attack replacement with orb waves
- Finish-explosion burst multipliers/counts
- Optional Samus freeze-on-hit behavior
- Hot-reloadable JSON config in the full build
- Headless support improvements in `main.c` including `--save-state`

## What Feels Good

- The work is already more than a hacky color swap; it has the beginnings of a real gameplay seam.
- Splitting projectile/finale behavior out of `enemy_torizo.c` is the right instinct.
- JSON config for rapid iteration is exactly the sort of evil we should keep.
- The current changes still pass the project's baseline checks:
  - `python3 -m pytest tests/test_build.py -q`
  - `make mini-test`

## What Is Still Messy

The code is not doomed, but it is definitely wearing three coats at once:

1. **Behavior logic and tuning data are still mixed together**
   - `enemy_torizo.c` still owns too much raw state-machine behavior.
   - `torizo_config.c` exposes knobs, but the mapping from knob -> gameplay intent is still ad hoc.

2. **The config surface is feature-shaped, not system-shaped**
   - The JSON is good for the exact mods already made.
   - It is not yet a general surface for attacks, phases, telegraphs, cooldowns, or variant packs.

3. **Global side effects leaked outside the boss module**
   - `main.c`, `physics.c`, and `enemy_collision.c` now know about Torizo-specific freeze behavior.
   - That is acceptable as a first mod seam, but it should become a generic boss-effect or scripted status seam if more bosses get this treatment.

4. **There are still local/dev artifacts mixed with source changes**
   - `saves/save1.sav`
   - `saves/flyway_pre_torizo.sav`
   - `sm.ini`
   - `sm_rev_mini_rollback_test`
   These are useful while iterating, but they should not define the long-term code shape.

5. **No boss-specific deterministic test harness yet**
   - Build/test smoke passes are good.
   - They do not prove the modded Torizo fight is stable or fun.

## Non-Goals

Do not do these yet:

- generic "clean every enemy file in the repo" passes
- broad rewrites of the whole Torizo state machine in one shot
- a universal boss scripting engine before two bosses justify it
- randomization that destroys telegraph readability
- modding by stacking more globals and hoping the cathedral stands up

## Target Shape

The next stable architecture should look like this:

- **Boss parity module**
  - original behavior still lives in readable C
  - vanilla defaults stay intact
- **Boss tuning/config module**
  - owns authorable knobs, presets, and validation
- **Boss attack surface module**
  - explicit attack profiles, projectile profiles, phase rules, and cooldown rules
- **Boss test harness**
  - deterministic save-state-driven checks for known scenarios
- **Boss content presets**
  - "vanilla+", "chaos", "bullet hell lite", "training", etc.

For Torizo specifically, the ideal split is:

- `enemy_torizo.c`
  - core state machine and parity logic
- `enemy_torizo_projectiles.c`
  - orb and projectile spawn behavior
- `enemy_torizo_finale.c`
  - death/finale scripting
- future `enemy_torizo_attacks.c`
  - attack-entry helpers and attack selection logic
- `torizo_config.c`
  - validated authorable config and presets

## Recommended Execution Order

## Phase 0: Housekeeping Before More Features

Goal: stop mixing experiment debris with gameplay architecture.

Tasks:

1. Separate source changes from local iteration artifacts.
   - Keep reusable fixtures under a clear path like `testdata/` or `saves/boss/`.
   - Treat `sm.ini` as local unless there is a deliberate repo-level reason to track it.
   - Remove build outputs like `sm_rev_mini_rollback_test` from working diffs.

2. Freeze the current Torizo mod surface in docs.
   - Keep `docs/torizo_mods.md` current.
   - Add one short section for known caveats and safe ranges.

3. Decide the config contract.
   - Are missing keys allowed? yes.
   - Are out-of-range values clamped? should be yes.
   - Are presets supported? should be soon.

Exit criteria:

- source diff is clearly separated from local saves/build artifacts
- docs explain every live JSON field and its expected range/behavior

## Phase 1: Turn The Current Knobs Into Real Gameplay Types

Goal: replace one-off numeric fields with named gameplay concepts.

Add or formalize types like:

- `TorizoProjectileProfile`
- `TorizoAttackProfile`
- `TorizoPhaseProfile`
- `TorizoReactionProfile`
- `TorizoFightProfile`

Examples of what should move behind named structs:

- orb burst count / clamp / jitter / palette
- orb contact damage multiplier
- opening attack choice and wave cadence
- finish explosion burst cadence
- on-hit freeze or stun effect
- future jump, slam, sonic-boom, or statue-wake parameters

Practical rule:

- if a number affects fight feel, it should belong to a named profile
- if a number only preserves vanilla parity in one code path, file-local constant is enough

Exit criteria:

- JSON maps to profile structs, not scattered getters alone
- attack and projectile tuning can be discussed in English instead of archaeological hex

## Phase 2: Cleanly Extract Attack Clusters From `enemy_torizo.c`

Goal: make behavior modification possible without playing whack-a-mole in one giant file.

Recommended extraction order:

1. Opening / wake / activation flow
2. Chozo orb attack helpers
3. Jump / reposition helpers
4. Ground attack / melee attack helpers
5. Low-health / desperation behavior
6. Bomb Torizo vs Golden Torizo variant selection

Extraction rules:

- preserve behavior first
- prefer `static` helpers when possible
- use file-local enums/constants for state names
- do not create `helpers.c` landfill
- update `docs/bank_origin_map.md` when functions move

Exit criteria:

- the next behavior mod can be made by editing a small topical file rather than spelunking the whole boss file

## Phase 3: Replace Torizo-Specific Engine Leaks With Generic Boss Effects

Goal: avoid each boss teaching `main.c` a new curse.

Current cross-cutting seam:

- Samus freeze-on-orb-hit uses Torizo-specific logic in global runtime paths.

Refactor target:

- convert this to a generic effect/status mechanism such as:
  - `BossEffect_RequestSamusLockout(frames)`
  - `BossEffect_RequestKnockback(...)`
  - `BossEffect_RequestScreenShake(...)`
  - `BossEffect_RequestPaletteFlash(...)`

Why:

- if Phantoon, Kraid, Crocomire, or Draygon gets touched later, the same hook can be reused
- it keeps `main.c` from becoming the shrine of one boss's weird ideas

Exit criteria:

- no Torizo-specific naming remains in generic movement/runtime code unless absolutely necessary

## Phase 4: Build A Boss Test Harness That Uses Save States

Goal: make boss modding repeatable instead of vibes-only.

Use the new headless save/load support to create deterministic scenarios:

1. Pre-fight snapshot
2. Fight-start snapshot
3. Low-health snapshot
4. Death/finale snapshot

Then add focused checks for:

- build still succeeds
- mod config loads and hot reloads cleanly
- opening attack path does not crash or softlock
- orb-heavy variants respect projectile-slot limits
- low-health transitions still complete
- death/finale sequence still exits correctly
- repeated runs produce expected state hashes for a fixed seed/config

Suggested future artifact layout:

- `saves/boss/torizo_pre_fight.sav`
- `saves/boss/torizo_low_health.sav`
- `tests/test_torizo_mods.py`
- optional helper under `tools/` for scenario replay / frame capture

Exit criteria:

- every new Torizo gameplay change has at least one scenario check besides "I played it and it seemed metal"

## Phase 5: Build A Small Preset System Instead Of Endless Raw JSON Tweaks

Goal: make fun variants easy to try and easy to reason about.

Recommended preset categories:

- `vanilla_plus`
  - slightly bigger bursts, clearer telegraphs, mild aggression
- `arena_control`
  - area denial via orb waves and landing pressure
- `duelist`
  - fewer projectiles, sharper melee and jump punish windows
- `desperation`
  - stable early fight, spicy low-health phase
- `chaos_but_fair`
  - higher randomness, but telegraphs and cooldown budgets preserved

Implementation direction:

- keep raw JSON overrides
- add named preset loading or preset blocks in JSON
- let one preset inherit from vanilla defaults and only override the interesting knobs

Exit criteria:

- fun variants are shareable, documented, and not hand-edited field soup every time

## Phase 6: Generalize The Pattern To A Second Boss

Goal: prove this is a boss-mod framework, not a one-boss science fair volcano.

Best candidates:

- **Kraid**: clear attack families, easy spectacle, good phase hooks
- **Phantoon**: teleport + flame pattern variety, good for fairness-vs-chaos tuning
- **Crocomire**: more about arena pressure and knockback scripting than pure projectile spam

Do not jump to all bosses at once.
Choose one second boss only after Torizo has:

- extracted attack clusters
- deterministic scenario coverage
- reusable status/effect seam
- a preset system that does not make future-you cry in the shower

## First Three Concrete Tasks

If continuing immediately, do these next:

1. **Stabilize the current patch surface**
   - clean working tree noise
   - document live JSON fields and safe ranges
   - decide what is source vs fixture vs local-only

2. **Extract the next behavior slice from `enemy_torizo.c`**
   - prioritize opening attack flow and orb scheduling
   - move the behavior behind named attack/profile helpers

3. **Add one save-state-driven Torizo regression check**
   - start with opening attack stability under the current blue-orb preset
   - assert no crash, no softlock, and stable completion behavior for fixed input/config

## Interesting Gameplay Ideas Worth Trying

These are the ones most likely to be fun rather than merely louder:

### 1. Fair Desperation Phase

- Keep phase 1 close to readable vanilla.
- At low health, switch to a more aggressive jump + orb cadence.
- Preserve telegraph windows so the player can learn it instead of just being mugged by geometry.

### 2. Arena Control Torizo

- Use orbiting or staged orb waves to restrict space rather than only deal damage.
- Encourage movement choices: high route, low route, bait-and-punish.
- Better than simply multiplying every projectile until the screen resembles a tax audit.

### 3. Counterplay-Sensitive Attack Picks

- Bias attack selection based on Samus distance, height, or recent player behavior.
- Example: more jumps if Samus turtles far away; more close pressure if she camps under him.
- Important: use weighted selection, not omniscient cheating.

### 4. Telegraph Remix, Not Just Damage Remix

- Change palette, startup pose timing, sound, or flash behavior to create readable new attacks.
- Players forgive hard bosses sooner than unreadable ones.

### 5. Variant Profiles By Fantasy

Give each preset a personality, not just stat inflation:

- *Temple Sentinel*: slower, heavier, bigger punish windows
- *Orb Weaver*: projectile zoning specialist
- *Berserker Statue*: shorter telegraphs, more jump pressure
- *Royal Guard*: Golden Torizo leaning, counterattack-heavy

### 6. Recovery Windows As A Tunable Resource

- Explicitly budget downtime after big attacks.
- Makes the fight feel designed rather than randomly caffeinated.
- This should become part of attack profiles: startup, active, recovery, cooldown weight.

## Extension Path For Other Bosses

When the Torizo path is solid, extend the same design vocabulary to other bosses:

- **Kraid**
  - belly spike cadence
  - hand swipe timing
  - room-control patterns
- **Phantoon**
  - flame arc presets
  - teleport pattern pools
  - eye-open punish windows
- **Draygon**
  - swoop routing presets
  - grab escape windows
  - turret interaction emphasis
- **Ridley**
  - pogo cadence
  - fireball spread families
  - retreat / dive pressure balance

The shared framework should revolve around:

- attack profiles
- phase transitions
- telegraph definitions
- arena effects
- deterministic validation

## Validation Checklist For Every Boss-Mod PR

Run at minimum:

- `python3 -m pytest tests/test_build.py -q`
- `make mini-test`

When behavior changes materially, also run the smallest relevant save-state or headless boss scenario.

Every PR should answer:

- What new gameplay surface was added?
- What code was extracted or clarified?
- What parity behavior was intentionally preserved?
- What scenario proves this did not quietly break?

## Bottom Line

The current work is a good first mutation.
The next win is **not** adding five more cool hacks.
The next win is making the current hacks live inside a structure that can survive a second boss.
