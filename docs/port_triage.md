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

## Status snapshot (2026-04-18)

- ✅ GONE: `sm_80.c` (→ `game_init`/`game_state_extras`/`nmi_transfer`/`palette_fader`/
  `util`/`hud`/`timer`/`irq`), `sm_82.c` (→ split), `sm_84.c` (→ `plm_*`), `sm_89.c`,
  `sm_8f.c`, `sm_90.c` / `sm_91.c` (→ `samus_*`), `sm_92.c`, `sm_93.c`, `sm_b4.c`,
  `sm_85.c` (→ `message_box.c`), `sm_87.c` (→ `anim_tiles.c`), `sm_8d.c` (→ `palette_fx.c`),
  `sm_ad.c` (→ `mother_brain_hdma.c`), `sm_86.c` (→ `eproj_core.c` / `eproj_environment.c` /
  `eproj_tourian.c` / `eproj_combat.c`).
- 📁 Remaining bank-shaped files: 17, totalling ~53.6k LOC.

---

# Phase 1 — Samus-adjacent bank cleanup (finish the Samus story)

These unblock the mini-build path. `sm_9b.c` is the last bank file still holding
Samus-core state. Retire it, then trim the oversized `samus_projectile.c`.

### Session 1.1 — `sm_9b.c` (1122 LOC) → existing `samus_*` files
Split cleanly by topic. No new bank-shaped file is created.
- **Cluster A (≈200 LOC):** Projectile trail (`ProjectileTrail_Func5` etc., L79–L108)
  → **merge into** `samus_projectile.c` section.
- **Cluster B (≈200 LOC):** Samus death sequence (`StartSamusDeathAnimation`,
  `HandleSamusDeathSequence*`, `CopyPalettesForSamusDeath`,
  `QueueTransferOfSamusDeathSequence`, `GameState_24_SamusNoHealth_Explosion_*`,
  L109–L224) → **new** `samus_death.c`.
- **Cluster C (≈700 LOC):** Grapple beam FSM + helpers (`CancelGrappleBeamIfIncompatiblePose`,
  `GrappleNext_*`, `GrappleBeamFunc_*`, `GrappleBeamHandler`, `PropelSamusFromGrappleSwing`,
  `HandleConnectingGrapple*`, L225–end) → **extend** `samus_grapple.c` (currently 742 LOC;
  ends up ~1400 LOC — still cohesive).
- **Retires:** `sm_9b.c`.

### Session 1.2 — `samus_projectile.c` split (2727 LOC → 3 files)
Not a bank file, but oversized and blocks future projectile work.
- **Target:**
  - `samus_projectile_core.c` — spawn/move/delete + `CallProjFunc`
  - `samus_projectile_beam.c` — beam/charge/missile/super types
  - `samus_projectile_block.c` — projectile-vs-block collision helpers (may merge with existing `projectile_block_collision.c`)
- Size per file: aim 800–1000 LOC.
- Header: `samus_projectile.h` with public API so callers in `physics.c` / enemy banks aren't touched beyond include swap.

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

# Phase 3 — shared enemy infrastructure (unblocks every enemy bank)

`sm_a0.c` is the highest-leverage enemy bank. Split it **before** touching any
boss bank — the split seams below become the shared API everything else calls into.

### Session 3.1 — `sm_a0.c` part 1 (≈900 LOC) → `enemy_main.c`
- **Contents:** Enemy lifecycle + per-frame dispatch.
  - `LoadEnemies`, `ClearEnemyDataAndProcessEnemySet`, `InitializeEnemies`,
    `LoadEnemyGfxIndexes`, `LoadEnemyTileData`, `TransferEnemyTilesToVramAndInit`,
    `ProcessEnemyTilesets`, `DetermineWhichEnemiesToProcess`, `CallEnemyAi`,
    `CallEnemyPreInstr`, `EnemyMain`, `DecrementSamusTimers`, `SpawnEnemy`,
    `SpawnEnemyDrops`, `DeleteEnemyAndConnectedEnemies`, `RecordEnemySpawnData`,
    `DrawOneEnemy`, `WriteEnemyOams`, `NormalEnemyFrozenAI`,
    `ProcessExtendedTilemap`, `QueueEnemyBG2TilemapTransfers`.
- Covers L75–L2050 (roughly).

### Session 3.2 — `sm_a0.c` part 2 (≈900 LOC) → `enemy_collision.c`
- **Contents:** All enemy ↔ samus / enemy ↔ projectile / enemy ↔ block collision.
  - `EnemyCollisionHandler`, `SamusProjectileInteractionHandler`,
    `EprojSamusCollDetect`/`HandleEprojCollWithSamus`, `EprojProjCollDet`/
    `HandleEprojCollWithProj`, `CallHitboxTouch`/`CallHitboxShot`,
    `EnemySamusCollHandler*`, `EprojCollHandler*`, `EnemyBombCollHandler*`,
    `ProcessEnemyPowerBombInteraction`, `NormalEnemy{Touch,Shot,PowerBomb}Ai*`,
    `EnemyDeathAnimation`, `RinkasDeathAnimation`, `SuitDamageDivision`,
    `CreateDudShot`, `EnemyBlockCollReact_*`, `Enemy_MoveRight*`,
    `Enemy_MoveDown`, `EnemyBlockCollHoriz/VertReact_*`,
    `CalculateBlockContainingPixelPos`.
- Covers L2050–L3600.

### Session 3.3 — `sm_a0.c` part 3 (≈900 LOC) → `enemy_math.c` + `enemy_drops.c`
- **`enemy_math.c` (≈500 LOC):** trig / angle / sign-extend / abs helpers.
  - `SignExtend8`, `Mult32`, `Abs16`, `SubtractThenAbs16`, `SineMult8bit*`,
    `CosineMult8bit*`, angle-from-xy tables, `CalculateAngleOfSamusFrom*`,
    `CalculateAngleFromXY`, `Math_*` helpers, `CompareDistToSamus_*`.
- **`enemy_drops.c` (≈400 LOC):** Per-boss item-drop routines.
  - `Enemy_ItemDrop_*` (Kraid, Phantoon, Ridley, Crocomire, Draygon, Botwoon,
    SporeSpawn, BombTorizo, GoldenTorizo, Metroid, MiniKraid, LowerNorfairSpacePirate),
    `SwitchEnemyAiToMainAi`, `EnemyGrappleDeath`, `SamusLatchesOnWith*`,
    `SamusHurtFromGrapple`, `RandomDropRoutine`, `RespawnEnemy`.
- Retires `sm_a0.c`.

### Session 3.4 — shared header `enemy.h`
- Collect the public API surface from Sessions 3.1–3.3 into one header.
- Do this in a separate small session after 3.1–3.3 land, to catch missed callers.

---

# Phase 4 — enemy projectiles (the big combat surface)

`sm_86.c` (5294 LOC) — after Phase 3 lands, split by owning boss/room. Function
name is already the best seam: `EprojInit_*`/`EprojPreInstr_*` cluster by topic.

### Session 4.1 — eproj core (≈900 LOC) → `eproj_core.c`
- Lifecycle + movement + block collision:
  - `Enable/Disable/ClearEprojs`, `SpawnEprojWithGfx/RoomGfx`, `EprojRunAll`,
    `CallEprojFunc`, `DrawLow/HighPriorityEprojs`, `DrawEprojs`,
    `GetValuesForScreenShaking`, `EprojColl_873D`,
    `EprojBlockCollisition_*` family, `Eproj_PreInit_0x8aaf`,
    `MoveEprojWithVelocity[X|Y]`, `SetAreaDependentEprojProperties*`,
    `Eproj_DeleteIfYposOutside`, `CallEprojInit`/`CallEprojPreInstr`/`CallEprojInstr`.
- Covers sm_86 L1–~L1400.

### Session 4.2 — eproj generic / environment (≈900 LOC) → `eproj_environment.c`
- Spores, Namifune, Lavaman, spike-shooting plant, nuclear waffle, fake walls,
  lavaquake rocks, eye door, save-station electricity, item pickups,
  small skree / power-bomb collision, N00b tube, maridia floaters, WS robot laser.
- Covers most of sm_86 L2200–L4400 (the non-boss region).

### Session 4.3 — eproj bosses/MB (≈1000 LOC) → `eproj_bosses.c`
- Mother Brain turrets, MB bomb, MB rainbow beam, MB drool, MB glass shards,
  MB death beam, Tourian statue set, Torizo sonic boom/orbs/egg/super missile/eye beam,
  Botwoon body/spit, yapping maws, Shaktool, Ki-Hunter acid spit, Kago bugs,
  Ceres elevator pad/platform, pre-Phantoon, Draygon turret/gunk, Crocomire spikewall/bridge.
- If >1000 LOC, split Session 4.3 into `eproj_bosses_a.c` (MB + Tourian) vs
  `eproj_bosses_b.c` (everything else).

### Session 4.4 — eproj cleanup
- Move eproj math (`Math_MultBySin`/`Cos`/`SinCos`, `Eproj_AngleToSamus`) into
  `enemy_math.c` (Phase 3) if not already there.
- Collect public API into `eproj.h`.
- Retires `sm_86.c`.

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

# Phase 6 — HDMA / presentation (do when blocked on rooms or suit pickup)

### Session 6.1 — `sm_88.c` part 1 (≈900 LOC) → `hdma_core.c`
- Layer-blending dispatch + HDMA object lifecycle.
  - `LayerBlendingHandler`, `LayerBlendFunc_*`, `HdmaObjectHandler`,
    `HdmaobjInstructionHandler`, `Enable/Disable/ClearHdmaObjects`,
    `SpawnHdmaObject*`, `WaitUntilEndOfVblankAndClearHdma`,
    `InitializeSpecialEffectsForNewRoom`, `RaiseOrLowerFx`.

### Session 6.2 — `sm_88.c` part 2 (≈900 LOC) → `hdma_power_bomb.c` + `hdma_crystal_flash.c`
- Power-bomb explosion HDMA tables, Crystal Flash HDMA setup.

### Session 6.3 — `sm_88.c` part 3 (≈900 LOC) → `hdma_liquid.c`
- Lava/acid/water rising, tide, BG2/BG3 scrolling, spore/rain/fog/haze FX.

### Session 6.4 — `sm_88.c` part 4 (≈600 LOC) → `hdma_xray.c` (extend existing) + suit pickup
- Merge remaining xray HDMA handlers into existing `samus_xray.c` or a new
  `hdma_xray.c`.
- Suit pickup HDMA (`VariaSuitPickup_*`, `GravitySuitPickup_*`) → extend
  `samus_palette.c` or new `samus_suit_pickup.c`.
- Retires `sm_88.c`.

---

# Phase 7 — menus / save

`sm_81.c` (2527 LOC) — mostly orthogonal to gameplay; split by screen.

### Session 7.1 — `sm_81.c` part 1 (≈700 LOC) → `save_sram.c`
- `SaveToSram`, `LoadFromSram`, `PackMapToSave`, `UnpackMapFromSave`,
  `SoftReset`.

### Session 7.2 — `sm_81.c` part 2 (≈800 LOC) → `spritemap_draw.c`
- All `DrawSpritemap*` variants (base tile, offscreen, menu, samus, beam, eproj).

### Session 7.3 — `sm_81.c` part 3 (≈1000 LOC) → `file_select_menu.c` + `game_over_menu.c`
- Game-over screen, file-select (copy/clear/confirm), helmet drawing.
- Retires `sm_81.c`.

---

# Phase 8 — cinematics (lowest core-priority, do last before bosses)

`sm_8b.c` (6395 LOC) — the biggest single file. Safely defer.

### Session 8.1 — `sm_8b.c` part 1 (≈900 LOC) → `cinematic_ppu.c`
- PPU setup for title/intro/mode7/mode1 scenes.
- Mode7 transformation + matrix helpers.

### Session 8.2 — `sm_8b.c` part 2 (≈900 LOC) → `cinematic_bg.c`
- BG tilemap drivers, text-tile drawing, Japanese text transfer.

### Session 8.3 — `sm_8b.c` part 3 (≈900 LOC) → `cinematic_sprite.c`
- Cinematic sprite objects + instr list, Mode7 objects.

### Session 8.4 — `sm_8b.c` part 4 (≈900 LOC) → `cinematic_palette.c`
- Fade in/out, palette compose/decompose.

### Session 8.5 — `sm_8b.c` part 5 (≈900 LOC) → `cinematic_intro.c`
- Intro-specific handlers, Samus during intro, text glow.

### Session 8.6 — `sm_8b.c` part 6 (≈900 LOC) → `cinematic_ending.c` + `credits.c`
- Credits object, ending sprites.
- Retires `sm_8b.c`.

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
2. `sm_9b.c` still present? → Session 1.1.
3. Any Phase 2 small module? → easy single-session target.
4. `sm_a0.c` still present? → Session 3.1 (then 3.2, 3.3).
5. `sm_86.c` still present **and** `sm_a0.c` gone? → Session 4.x.
6. Blocked on a specific feature? → jump to the matching session in Phase 5–8.
7. Otherwise → pick a small module from Phase 2 or a progression boss from Phase 5.

Bosses (Phase 9) are **never** the next thing.
