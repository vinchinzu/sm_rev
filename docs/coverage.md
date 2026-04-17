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
