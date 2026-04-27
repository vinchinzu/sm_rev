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

## Status snapshot (2026-04-26)

- ✅ GONE: `sm_80.c` (→ `game_init`/`game_state_extras`/`nmi_transfer`/`palette_fader`/
  `util`/`hud`/`timer`/`irq`), `sm_82.c` (→ split), `sm_84.c` (→ `plm_*`), `sm_89.c`,
  `sm_8f.c`, `sm_90.c` / `sm_91.c` (→ `samus_*`), `sm_92.c`, `sm_93.c`, `sm_b4.c`,
  `sm_85.c` (→ `message_box.c`), `sm_87.c` (→ `anim_tiles.c`), `sm_8d.c` (→ `palette_fx.c`),
  `sm_ad.c` (→ `mother_brain_hdma.c`), `sm_86.c` (→ `eproj_core.c` / `eproj_environment.c` /
  `eproj_tourian.c` / `eproj_combat.c`), `sm_81.c` (→ menu/save topical files), `sm_88.c`
  (→ HDMA topical files), `sm_9b.c` (→ `samus_death.c` / `samus_grapple.c` / projectile
  helpers), `sm_8b.c` (→ `cinematics.c` topical rename), `sm_a0.c` (→ `enemy_main.c` /
  `enemy_collision.c` / `enemy_math.c` / `enemy_drops.c`), Bank `$82` shared tables
  (→ `menu_assets.h` plus local owner-owned constants), `sm_a3.c` (→ `enemy_falling_platform.c`),
  `sm_aa.c` (→ `enemy_chozo_shaktool.c`), `sm_a4.c` (→ `enemy_crocomire.c`), `sm_a5.c` (→
  `enemy_draygon_spore.c`), `sm_b2.c` (→ `enemy_space_pirates.c`), `sm_b3.c` (→
  `enemy_botwoon.c`), `sm_a2.c` (→ `enemy_a2_misc.c`), `sm_a6.c` (→ `enemy_ridley_zebetite.c`),
  `sm_a7.c` (→ `enemy_kraid_phantoon.c`), `sm_a8.c` (→ `enemy_ki_hunter.c`), `sm_a9.c` (→
  `enemy_mother_brain.c`).
- 🎉 **All bank-shaped gameplay files retired.** Only `sm_rtl.c`, `sm_cpu_infra.c`, and
  `sm_dispatcher.c` remain — those are the C runtime shim, not bank files.

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

### Session 5.1 — `sm_b2.c` (775 LOC) → `enemy_space_pirates.c` ✅
- Walking + ninja + wall pirate runtime peeled into `enemy_space_pirates.c` on 2026-04-26.
- `sm_b2.c` is retired.

### Session 5.2 — `sm_aa.c` part 1 (1034 LOC) → `enemy_torizo.c` ✅
- Torizo runtime peeled into `enemy_torizo.c` on 2026-04-22.

### Session 5.2b — `sm_aa.c` part 2 (516 LOC) → `enemy_chozo_shaktool.c` ✅
- Tourian Entrance Statue, Shaktool, Chozo Statue runtimes peeled into `enemy_chozo_shaktool.c` on 2026-04-26.
- `sm_aa.c` is retired.

### Session 5.3 — `sm_a3.c` part 1 (149 LOC) → `enemy_mochtroid.c`
- Mochtroid runtime peeled into `enemy_mochtroid.c` on 2026-04-22.
- This was the clean cluster between Roach and Sidehopper, with no remaining bank-local callers.

### Session 5.4 — `sm_a3.c` part 2 (126 LOC) → `enemy_elevator.c`
- Elevator runtime peeled first into `enemy_elevator.c` on 2026-04-22.
- Remaining Bank `$A3` work is still mixed fauna/platform code; `sm_a3.c` is not retired yet.

### Session 5.5 — `sm_a3.c` part 3 (346 LOC) → `enemy_metroid.c`
- Metroid runtime peeled into `enemy_metroid.c` on 2026-04-22.
- This owns the sprite-linked Metroid chase, latch, freeze, hurt, and drop logic.

### Session 5.6 — `sm_a3.c` part 4 (2330 LOC) → `enemy_fauna.c` ✅
- Remaining Bank `$A3` fauna/hazard runtime peeled into `enemy_fauna.c` on 2026-04-22.

### Session 5.7 — `sm_a3.c` part 5 (237 LOC) → `enemy_falling_platform.c` ✅
- Falling/sinking platform runtime + shared A3 enemy-bank wrappers peeled into `enemy_falling_platform.c` on 2026-04-26.
- `sm_a3.c` is retired.

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

All boss banks have been lifted whole into combined topical files on 2026-04-26.
A future topical-cleanup pass can split the multi-boss files further:

| Bank | LOC | Current home | Future split (optional) |
|------|-----|--------------|-------------------------|
| ~~`sm_a4.c` Crocomire~~ ✅ | 1646 | `enemy_crocomire.c` | already topical |
| ~~`sm_a5.c` Draygon + SporeSpawn~~ ✅ | 1603 | `enemy_draygon_spore.c` | → `enemy_draygon.c` + `enemy_spore_spawn.c` |
| ~~`sm_a7.c` Kraid + Phantoon~~ ✅ | 3918 | `enemy_kraid_phantoon.c` | → `enemy_kraid.c` + `enemy_phantoon.c` |
| ~~`sm_a8.c` Ki-Hunter~~ ✅ | 4068 | `enemy_ki_hunter.c` | split by behavior |
| ~~`sm_a6.c` Ridley + zebetites~~ ✅ | 4944 | `enemy_ridley_zebetite.c` | → `enemy_ridley.c` + `enemy_zebetite.c` |
| ~~`sm_a2.c` shutters/fauna~~ ✅ | 3693 | `enemy_a2_misc.c` | → `enemy_shutters.c` + Norfair/Maridia files |
| ~~`sm_b3.c` Botwoon~~ ✅ | 1403 | `enemy_botwoon.c` | already topical |
| ~~`sm_a9.c` Mother Brain + Shitroid~~ ✅ | 6499 | `enemy_mother_brain.c` | → `enemy_mother_brain.c` + `enemy_shitroid.c` |

---

# Quick picker — "what should I do next?"

1. Phase 0 housekeeping not done? → do it.
2. All bank files are retired. Future work is **topical cleanup** inside the new
   combined-boss files (see Phase 9 table above for the splits worth considering).
3. Blocked on a specific subsystem? → stay in the topical file that now owns it;
   don't recreate bank-shaped plans.
