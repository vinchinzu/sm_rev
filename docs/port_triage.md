# sm_rev Port Triage — Session-Sized Chunks

Target: eliminate the bank-shaped `src/sm_*.c` files in an order that maximizes
mini-build value and keeps each session within a 700–1000 LOC refactor budget
(the comfortable scope for one AI pass with behavior parity + build verify).

## How to use this doc

- Phases are in **dev-importance order**. Do Phase 0 first, bosses last.
- Each **session** lists: source file(s), target file(s), function cluster, LOC budget.
- After each session:
  1. `python3 -m pytest tests/test_build.py -q` (9 tests, ~80s)
  2. `make mini-test` for shell smoke (~1s)
  3. `python3 tests/run_tests.py -v` if runtime behavior may have shifted
     (`test_deterministic_across_runs` is flaky on SPC state — re-run before investigating)
- Keep moves **mechanical and behavior-preserving**. Refactor second.
- When cross-file calls need to be visible, add decls to `funcs.h` or a local
  topic header (e.g. `samus_env.h`). Keep helpers `static` when possible.

## Status snapshot (2026-04-20)

- ✅ GONE: `sm_80.c` (→ `game_init`/`game_state_extras`/`nmi_transfer`/`palette_fader`/
  `util`/`hud`/`timer`/`irq`), `sm_82.c` (→ split), `sm_84.c` (→ `plm_*`), `sm_89.c`,
  `sm_8f.c`, `sm_90.c` / `sm_91.c` (→ `samus_*`), `sm_92.c`, `sm_93.c`, `sm_b4.c`,
  `sm_85.c` (→ `message_box.c`), `sm_87.c` (→ `anim_tiles.c`), `sm_8d.c` (→ `palette_fx.c`),
  `sm_ad.c` (→ `mother_brain_hdma.c`), `sm_86.c` (→ `eproj_core.c` / `eproj_environment.c` /
  `eproj_tourian.c` / `eproj_combat.c`), `sm_81.c` (→ menu/save topical files), `sm_88.c`
  (→ HDMA topical files), `sm_9b.c` (→ `samus_death.c` / `samus_grapple.c` / projectile
  helpers), `sm_8b.c` (→ `cinematics.c` topical rename), `sm_a0.c` (→ `enemy_main.c` /
  `enemy_collision.c` / `enemy_math.c` / `enemy_drops.c`), Bank `$82` shared tables
  (→ `menu_assets.h` plus local owner-owned constants).
- 📁 Remaining bank-shaped gameplay files: 11, totalling ~33.6k LOC. All remaining bank-shaped
  gameplay files are enemy banks.

---

# Phase 1 — Samus-adjacent bank cleanup (finish the Samus story)

These unblock the mini-build path. The old Bank 9B work is already done, so the
remaining Samus-adjacent cleanup is topical cleanup inside the existing
`samus_*` modules rather than another bank extraction.

### Session 1.1 — Bank 9B split ✅
- Projectile trail now lives with the Samus projectile runtime.
- Samus death sequence lives in `samus_death.c`.
- Grapple beam FSM lives in `samus_grapple.c`.
- `sm_9b.c` is retired.

### Session 1.2 — Samus projectile split ✅
- Runtime entry points now route through `samus_projectile_core.c`,
  `samus_projectile_beam.c`, and `samus_projectile_block.c`.
- Future work here is internal cleanup of the shared `samus_projectile_impl.h`,
  not another bank retirement step.

---

# Phase 2 — small standalone modules (complete)

Each of these is <500 LOC and can be lifted whole into a topical file. Keep for
days when you want low-risk forward progress.

### Session 2.1 — `sm_87.c` (273 LOC) → `anim_tiles.c` ✅
- Straight rename + internal cleanup (instr table, handler).
- Make instr handlers `static`; expose only `EnableAnimtiles`/`DisableAnimtiles`/
  `ClearAnimtiles`/`SpawnAnimtiles`/`AnimtilesHandler` via `funcs.h`.

### Session 2.2 — `sm_8d.c` (324 LOC) → `palette_fx.c` ✅
- Straight rename. Small cluster of `PalFx*` handlers + palette-object dispatcher.
- Expose `EnablePaletteFx`/`DisablePaletteFx`/`ClearPaletteFXObjects`/
  `SpawnPalfxObject`/`PaletteFxHandler`.

### Session 2.3 — `sm_85.c` (427 LOC) → `message_box.c` ✅
- Already fully `static`-gated internally; lift as-is.
- Keep the existing `DisplayMessageBox*` entry points stable; internal helpers stay local.

### Session 2.4 — `sm_ad.c` (443 LOC) → `mother_brain_hdma.c` ✅
- Self-contained Mother Brain Phase-3 HDMA helpers lifted as a topical module.

---

# Phase 3 — shared enemy infrastructure (complete)

Bank `$A0` is retired.

- `enemy_main.c` now owns shared enemy lifecycle, spawn/load, and frame dispatch.
- `enemy_collision.c` now owns shared enemy collision work.
- `enemy_math.c` and `enemy_drops.c` hold the common math/drop helpers that used to
  block every enemy-bank split.

---

# Phase 4 — enemy projectiles (complete)

Bank `$86` is retired.

- `eproj_core.c`, `eproj_environment.c`, `eproj_tourian.c`, and `eproj_combat.c`
  now own the enemy-projectile runtime.
- Eproj trig helpers that were worth sharing moved into `enemy_math.c`.

---

# Phase 5 — small enemies & progression bosses

### Session 5.1 — `sm_b2.c` (775 LOC) → `enemy_space_pirates.c`
- Walking + ninja + wall pirates. Already fits one session.
- Depends on Phase 3 (`enemy_main.c`/`enemy_collision.c`).

### Session 5.2 — `sm_aa.c` (1546 LOC) → `enemy_torizo.c` + `enemy_tourian_statue.c` + `enemy_shaktool.c`
- Three independent subsystems in one bank; split ≈500 LOC each.

### Session 5.3 — `sm_a3.c` part 1 (≈1500 LOC) → `enemy_metroid.c`
- Pull Metroid AI out of the mixed elevator/Metroid bank.

### Session 5.4 — `sm_a3.c` part 2 (≈1600 LOC) → `enemy_elevator.c`
- Elevator scripting remainder. Retires `sm_a3.c`.

---

# Phase 6 — HDMA / presentation (complete)

Bank `$88` is retired into `hdma_core.c`, `hdma_power_bomb.c`, `room_fx_hdma.c`,
`boss_hdma.c`, and `cinematic_hdma.c`, with the suit/xray-specific leftovers
living in the Samus topical files.

---

# Phase 7 — menus / save (complete)

Bank `$81` is retired into `save_sram.c`, `spritemap_draw.c`, `menu_common.c`,
`file_select_menu.c`, `file_select_map.c`, and `game_over_menu.c`.

---

# Phase 8 — cinematics (topical rename complete; optional cleanup only)

`cinematics.c` is the former Bank 8B file under a topical name. That is enough
for the current mission because cinematics do not help the mini path.

### Session 8.1 — `cinematics.c` part 1 (≈900 LOC) → `cinematic_ppu.c`
- PPU setup for title/intro/mode7/mode1 scenes.
- Mode7 transformation + matrix helpers.

### Session 8.2 — `cinematics.c` part 2 (≈900 LOC) → `cinematic_bg.c`
- BG tilemap drivers, text-tile drawing, Japanese text transfer.

### Session 8.3 — `cinematics.c` part 3 (≈900 LOC) → `cinematic_sprite.c`
- Cinematic sprite objects + instr list, Mode7 objects.

### Session 8.4 — `cinematics.c` part 4 (≈900 LOC) → `cinematic_palette.c`
- Fade in/out, palette compose/decompose.

### Session 8.5 — `cinematics.c` part 5 (≈900 LOC) → `cinematic_intro.c`
- Intro-specific handlers, Samus during intro, text glow.

### Session 8.6 — `cinematics.c` part 6 (≈900 LOC) → `cinematic_ending.c` + `credits.c`
- Credits object, ending sprites.
- Optional only; `cinematics.c` already replaced the bank-shaped filename.

---

# Phase 9 — bosses (DEFERRED — do only after everything above)

Each boss bank is a self-contained chunk. Size them to one session each:

| Bank | LOC | Split target | Sessions |
|------|-----|--------------|----------|
| `sm_a4.c` Crocomire | 1646 | `enemy_crocomire.c` | 2× |
| `sm_a5.c` Draygon + SporeSpawn | 1603 | `enemy_draygon.c` + `enemy_spore_spawn.c` | 2× |
| `sm_a7.c` Kraid + Phantoon | 3918 | `enemy_kraid.c` + `enemy_phantoon.c` | 4× |
| `sm_a8.c` Ki-Hunter | 4068 | `enemy_ki_hunter.c` (split by behavior) | 4× |
| `sm_a6.c` Ridley + zebetites | 4944 | `enemy_ridley.c` + `enemy_zebetite.c` | 5× |
| `sm_a2.c` Gunship + shutters | 4132 | `gunship.c` + `shutters.c` | 4× |
| `sm_b3.c` Botwoon | 1403 | `enemy_botwoon.c` | 2× |
| `sm_a9.c` Mother Brain + Shitroid | 6499 | `enemy_mother_brain.c` + `enemy_shitroid.c` | 7× |

---

# Quick picker — "what should I do next?"

1. Phase 0 housekeeping not done? → do it.
2. Samus-adjacent cleanup needed? → review the completed notes in Phase 1, then stay in topical files.
3. Cheapest active bank target? → `sm_b2.c` or `sm_aa.c`.
4. Need progression/pathing coverage? → `sm_a3.c`.
5. Need a major enemy bank? → `sm_a7.c`, then `sm_a6.c` or `sm_a9.c`.
6. Blocked on a specific subsystem? → stay in the topical file that now owns it; don’t recreate bank-shaped plans.
