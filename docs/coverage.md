# sm_rev Function Coverage Audit

Generated: 2026-04-16

## Summary

| Metric | Value |
|--------|-------|
| Total source banks | 32 |
| Total functions | 5,156 |
| Intentional no-ops (nullsub/Nothing/Null callbacks) | 25 |
| Real missing implementations | **0** |

**The codebase is fully ported.** All empty functions are either:
- `nullsub_*` — ROM null subroutines (intentional no-ops from the original asm)
- `*_Nothing` / `*_Null*` — explicitly named null callbacks in function dispatch tables
- `PointlessFunctionStupidToo` — named by the decompiler as intentionally empty

## Bank-by-Bank Status

| File | Bank | System | Functions | Nullsubs | Status |
|------|------|--------|-----------|----------|--------|
| sm_80.c | $80 | System routines | 157 | 0 | ✅ |
| sm_81.c | $81 | SRAM, spritemap, menus | 129 | 0 | ✅ |
| sm_82.c | $82 | Top-level game routines | 267 | 1 | ✅ |
| sm_84.c | $84 | PLMs (Pre-Loaded Modules) | 186 | 0 | ✅ |
| sm_85.c | $85 | Message boxes | — | 0 | ✅ |
| sm_86.c | $86 | Enemy projectiles | 338 | 0 | ✅ |
| sm_87.c | $87 | Animated tiles | 33 | 0 | ✅ |
| sm_88.c | $88 | HDMA | 188 | 0 | ✅ |
| sm_89.c | $89 | Item PLM graphics, FX loader | 4 | 1 | ✅ |
| sm_8b.c | $8B | Non-gameplay (cinematics, intro) | 433 | 2 | ✅ |
| sm_8d.c | $8D | Projectile spritemaps, palette FX | 19 | 1 | ✅ |
| sm_8f.c | $8F | Room definitions | 135 | 0 | ✅ |
| sm_90.c | $90 | Samus (collision, physics) | 432 | 2 | ✅ |
| sm_91.c | $91 | Aran (extended Samus state) | 272 | 10 | ✅ |
| sm_92.c | $92 | Samus animations | — | 0 | ✅ |
| sm_93.c | $93 | Projectiles | — | 0 | ✅ |
| sm_94.c | $94 | Block properties, cutscene gfx | 134 | 3 | ✅ |
| sm_9b.c | $9B | CPU infrastructure (misc) | 55 | 0 | ✅ |
| sm_a0.c | $A0 | Enemies (common AI, collision) | 153 | 0 | ✅ |
| sm_a2.c | $A2 | Enemy AI — gunship, shutters | 306 | 1 | ✅ |
| sm_a3.c | $A3 | Enemy AI — elevator, metroid | 236 | 1 | ✅ |
| sm_a4.c | $A4 | Enemy AI — Crocomire | 66 | 1 | ✅ |
| sm_a5.c | $A5 | Enemy AI — Draygon, Spore Spawn | 82 | 0 | ✅ |
| sm_a6.c | $A6 | Enemy AI — Ridley, zebetites | 293 | 1 | ✅ |
| sm_a7.c | $A7 | Enemy AI — Kraid **(complete)**, Phantoon | 230 | 0 | ✅ |
| sm_a8.c | $A8 | Enemy AI — Ki-Hunter | 256 | 0 | ✅ |
| sm_a9.c | $A9 | Enemy AI — Mother Brain **(complete)**, Shitroid | 480 | 0 | ✅ |
| sm_aa.c | $AA | Enemy AI — Torizo, Tourian statue | 62 | 0 | ✅ |
| sm_ad.c | $AD | Enemy AI (misc) | 25 | 1 | ✅ |
| sm_b2.c | $B2 | Enemy AI — Space Pirates | 43 | 0 | ✅ |
| sm_b3.c | $B3 | Enemy AI — Botwoon | 95 | 0 | ✅ |
| sm_b4.c | $B4 | Enemy instructions/drops/resistances **(complete)** | — | 0 | ✅ |

## 80s/90s Porting Tracker

This is a cleanup/refactor tracker for the still bank-shaped `src/sm_8*.c` / `src/sm_9*.c`
files. It is separate from function coverage: coverage is complete, but these files still vary
widely in size and importance if the goal is to port them into smaller, purpose-built modules.

- LOC is raw `wc -l` output measured on 2026-04-17.
- LOE buckets are rough refactor size only: `XS < 300`, `S < 1000`, `M < 2500`, `L < 4000`, `XL >= 4000`.
- Core priority uses a simple triage rubric:
  - `P0` = directly blocks core gameplay coverage
  - `P1` = important support/combat/infrastructure
  - `P2` = mostly presentation, menus, or non-core flow

| File | Bank | System | LOC | Share | LOE | Core priority | Triage note |
|------|------|--------|-----|-------|-----|---------------|-------------|
| `sm_80.c` | $80 | System routines | 2668 | 7.0% | L | P1 | Shared engine routines; broad dependency surface |
| `sm_81.c` | $81 | SRAM, spritemap, menus | 2527 | 6.6% | L | P2 | Save/menu/UI work; low gameplay leverage |
| `sm_82.c` | $82 | Top-level game routines | 5130 | 13.4% | XL | P0 | Main game-state flow; high-leverage core bank |
| `sm_84.c` | $84 | PLMs (Pre-Loaded Modules) | 3199 | 8.4% | L | P0 | Doors, items, triggers, room scripting |
| `sm_85.c` | $85 | Message boxes | 427 | 1.1% | S | P2 | Isolated UI cleanup; fast win |
| `sm_86.c` | $86 | Enemy projectiles | 5294 | 13.9% | XL | P1 | Large combat surface; do after Samus/PLM core |
| `sm_87.c` | $87 | Animated tiles | 273 | 0.7% | XS | P2 | Tiny visual subsystem |
| `sm_88.c` | $88 | HDMA | 3342 | 8.8% | L | P2 | Large visual/HDMA bank; usually deferrable |
| `sm_89.c` | $89 | Item PLM graphics, FX loader | 150 | 0.4% | XS | P1 | Tiny FX/loader cleanup; easy opportunistic win |
| `sm_8b.c` | $8B | Non-gameplay (cinematics, intro) | 6395 | 16.8% | XL | P2 | Biggest file, but safely defer for core coverage |
| `sm_8d.c` | $8D | Projectile spritemaps, palette FX | 324 | 0.8% | S | P2 | Mostly presentation-side work |
| `sm_8f.c` | $8F | Room definitions | 969 | 2.5% | S | P0 | Room/state routing; useful content cleanup |
| `sm_90.c` | $90 | Samus (collision, physics) | 847 | 2.2% | S | P0 | Already partly split; high-value quick win |
| `sm_91.c` | $91 | Aran (extended Samus state) | 2654 | 7.0% | L | P0 | Samus input/pose/transition hub; highest core risk |
| `sm_92.c` | $92 | Samus animations | 52 | 0.1% | XS | P2 | Very small animation bank |
| `sm_93.c` | $93 | Projectiles | 262 | 0.7% | XS | P0 | Cheap combat-coverage win |
| `sm_94.c` | $94 | Block properties, cutscene gfx | 2510 | 6.6% | L | P0 | Traversal- and collision-critical |
| `sm_9b.c` | $9B | CPU infrastructure (misc) | 1119 | 2.9% | M | P1 | Support bank; touch when blocked by shared infra |

**80s/90s total:** 38,142 raw lines

### Suggested Triage Order

- **Start here for core coverage:** `sm_90.c`, `sm_91.c`, `sm_94.c`, `sm_82.c`, `sm_84.c`
- **Cheap core wins to slot in early:** `sm_93.c`, `sm_8f.c`, `sm_89.c`
- **Secondary but important after core path:** `sm_86.c`, `sm_80.c`, `sm_9b.c`
- **Usually safe to defer:** `sm_81.c`, `sm_85.c`, `sm_87.c`, `sm_88.c`, `sm_8b.c`, `sm_8d.c`, `sm_92.c`

## Enemy Bank Porting Tracker

This covers the `src/sm_a*.c` and `src/sm_b*.c` banks, which are mostly enemy AI plus shared
enemy support code. For these files, raw LOC matters, but coverage leverage matters more:
small shared enemy support banks can be higher-value than a very large boss-only file.

- LOC is raw `wc -l` output measured on 2026-04-17.
- LOE buckets are rough refactor size only: `XS < 300`, `S < 1000`, `M < 2500`, `L < 4000`, `XL >= 4000`.
- Enemy priority uses a separate rubric:
  - `P0` = shared enemy infra or the smallest highest-leverage progression coverage
  - `P1` = broad combat coverage or major progression-critical bosses
  - `P2` = isolated boss/finale/presentation work that can wait

| File | Bank | System | LOC | Share | LOE | Enemy priority | Porting angle |
|------|------|--------|-----|-------|-----|----------------|---------------|
| `sm_a0.c` | $A0 | Enemies (common AI, collision) | 3797 | 10.0% | L | P0 | Shared enemy runtime/collision helpers; highest enemy-side leverage |
| `sm_a2.c` | $A2 | Enemy AI — gunship, shutters | 4132 | 10.9% | XL | P2 | Scripted room-object behaviors; useful, but not core enemy breadth |
| `sm_a3.c` | $A3 | Enemy AI — elevator, metroid | 3111 | 8.2% | L | P1 | Mixed bank; isolate Metroid logic from room/elevator scripting |
| `sm_a4.c` | $A4 | Enemy AI — Crocomire | 1646 | 4.3% | M | P2 | Boss-only bank; self-contained refactor target |
| `sm_a5.c` | $A5 | Enemy AI — Draygon, Spore Spawn | 1603 | 4.2% | M | P2 | Two boss scripts; sizable, but low shared leverage |
| `sm_a6.c` | $A6 | Enemy AI — Ridley, zebetites | 4944 | 13.0% | XL | P1 | Late-game progression-critical combat; large but important |
| `sm_a7.c` | $A7 | Enemy AI — Kraid, Phantoon | 3918 | 10.3% | L | P1 | Major boss bank; good progression coverage once infra is stable |
| `sm_a8.c` | $A8 | Enemy AI — Ki-Hunter | 4068 | 10.7% | XL | P1 | Broad combat-pattern bank; good midgame enemy representative |
| `sm_a9.c` | $A9 | Enemy AI — Mother Brain, Shitroid | 6499 | 17.1% | XL | P2 | Biggest and most finale-specific enemy bank; defer until late |
| `sm_aa.c` | $AA | Enemy AI — Torizo, Tourian statue | 1546 | 4.1% | M | P1 | Compact progression-gate coverage; good value after shared infra |
| `sm_ad.c` | $AD | Enemy AI (misc) | 443 | 1.2% | S | P2 | Mostly Mother Brain HDMA/support; not first-pass gameplay coverage |
| `sm_b2.c` | $B2 | Enemy AI — Space Pirates | 775 | 2.0% | S | P1 | Cheap common-enemy win; high combat relevance for low size |
| `sm_b3.c` | $B3 | Enemy AI — Botwoon | 1403 | 3.7% | M | P2 | Boss-only bank; isolated cleanup target |
| `sm_b4.c` | $B4 | Enemy instructions/drops/resistances | 118 | 0.3% | XS | P0 | Tiny shared support bank; immediate high-leverage cleanup |

**Enemy-bank total:** 38,003 raw lines

### Suggested Enemy Triage Order

- **Immediate high-leverage enemy work:** `sm_b4.c`, `sm_a0.c`, `sm_b2.c`
- **Progression-critical next:** `sm_aa.c`, `sm_a3.c`, `sm_a6.c`, `sm_a7.c`
- **Broad combat depth after that:** `sm_a8.c`
- **Room-scripted or specialized combat later:** `sm_a2.c`, `sm_a4.c`, `sm_a5.c`, `sm_b3.c`
- **Finale-specific cleanup last:** `sm_ad.c`, `sm_a9.c`

### Practical Porting Notes

- `sm_b4.c` is the best “cheap infrastructure” target: tiny file, shared enemy-facing behavior, and low merge risk.
- `sm_a0.c` is the best “real enemy architecture” target: common reactions/collision code gives payoff across many banks.
- `sm_b2.c` and `sm_aa.c` are good early wins because they are compact and materially improve gameplay-path coverage.
- `sm_a9.c` should not be your first enemy refactor even though it is important; it is the largest enemy bank and heavily finale-specific.

## Notable Findings

### Intentional No-Ops Worth Understanding

- **`SetCarry_Spikeblk` / `ClearCarry_8`** (`sm_94.c`) — spike block collision table entries.
  These are legitimately empty: the carry flag manipulation was a SNES CPU artifact.
  In C, the carry-setting is handled by the dispatch table return value, so these are genuine no-ops.

- **`sub_91EFC3`** (`sm_91.c:2985`) — entry 0 in `kSamus_HandleTransitionsA_5[]` state machine.
  The ROM function at `$91EFC3` is also empty. Not a missing implementation.

- **`TimedShutter_Func_Null2`** (`sm_a2.c`) and **`Bang_Func_7`** (`sm_a3.c`) — null callbacks
  in enemy AI dispatch tables. ROM originals are also empty (null action slots).

### `cinematic_var10` (Fixed)
Was missing from `variables_extra.h`. Added in commit `d462f25`.
Address `$7E1997` is reused: `eproj_id` (projectile array pointer) during gameplay,
`cinematic_var10` (uint16 scratch) during cinematics. Both definitions coexist safely.

## What Needs Work (Non-Coverage)

Coverage is complete, but these areas have scope for improvement:

1. **PhysicsParams struct** — partially populated in `variables_extra.h`; not all Samus
   motion variables are exposed yet (see beads sm_rev-w93.6/7).
2. **Headless test coverage** — tests use cold-boot RAM (title screen). No save-state
   loading means physics/enemy regressions can't be caught yet (bead sm_rev-w93.3).
3. **Screenshot regression** — no reference PNGs captured yet (bead sm_rev-w93.5).
