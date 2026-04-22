# sm_rev Function Coverage Audit

Function coverage is complete (5,156 fns, 0 missing). This file is now a **structural**
audit: which bank-shaped files still exist, how big they are, and what they contain.
The actionable porting plan lives in [port_triage.md](port_triage.md).

Last refreshed: 2026-04-22.

## Extracted topical modules

Already split out of the original bank soup. These are the long-term homes — refactor
into them, don't recreate bank-shaped files.

| Module | LOC | What it holds |
|--------|-----|---------------|
| `samus_anim_fx.c` | 581 | Per-pose animation/FX driver |
| `samus_camera_map.c` | 335 | Camera tracking + minimap updates |
| `samus_collision.c` | 1271 | Block/slope/enemy collision, pose-change collision |
| `samus_death.c` | 146 | Samus death animation + explosion palette choreography |
| `samus_demo.c` | 342 | Attract-mode demo playback |
| `samus_draw.c` | 761 | Samus OAM + sprite-map draw |
| `samus_enemy_collision.c` | 146 | Samus ↔ enemy hitbox dispatch |
| `samus_grapple.c` | 742 | Grapple beam (fire/connect/swing/cancel) |
| `samus_input.c` | 352 | Joypad → pose/movement-type dispatch |
| `samus_jump.c` | 224 | Jump/walljump/bombjump/knockback init |
| `samus_motion.c` | 406 | MoveX/Y primitives |
| `samus_palette.c` | 591 | Beam/suit/speed-boost palette cycles |
| `samus_pose.c` | 547 | Pose-change gate + movement-type dispatcher |
| `samus_projectile_core.c` / `samus_projectile_beam.c` / `samus_projectile_block.c` | split + shared impl | Samus projectile runtime, with shared implementation in `samus_projectile_impl.h` |
| `samus_resource.c` | 63 | Health/missile/super/PB add-sub helpers |
| `samus_runtime.c` | 1156 | Per-frame Samus handler entry + misc |
| `samus_special_move.c` | 913 | Shinespark, crystal flash, MB scripted states |
| `samus_speed.c` | 363 | Speed-booster acceleration & extra-run-speed |
| `samus_transition.c` | 947 | New-pose transition state machine |
| `samus_xray.c` | 749 | X-ray HDMA + activation |
| `cinematics.c` | 6395 | Title/intro/ending/credits cinematic runtime and Mode 7 scene flow |
| `hdma_core.c` / `hdma_power_bomb.c` / `room_fx_hdma.c` / `boss_hdma.c` / `cinematic_hdma.c` | ~2400 total | Bank `$88` HDMA split by runtime family |
| `enemy_main.c` | 1321 | Shared enemy lifecycle, draw path, and frame dispatch from Bank `$A0` |
| `enemy_collision.c` | 920 | Shared enemy/Samus/projectile/block collision layer from Bank `$A0` |
| `enemy_drops.c` | 467 | Enemy drops, grapple-death hooks, and respawn helpers from Bank `$A0` |
| `enemy_gunship.c` | 453 | Gunship-only enemy runtime peeled from Bank `$A2` |
| `enemy_elevator.c` | 126 | Samus-linked elevator enemy runtime peeled from Bank `$A3` |
| `enemy_fauna.c` | 2330 | Remaining Bank `$A3` fauna and hazard runtime |
| `enemy_metroid.c` | 346 | Metroid runtime peeled from Bank `$A3` |
| `enemy_mochtroid.c` | 149 | Mochtroid runtime peeled from Bank `$A3` |
| `enemy_torizo.c` | 1034 | Bomb/Golden Torizo runtime peeled from Bank `$AA` |
| `physics.c` | 492 | Movement-type dispatch table |
| `physics_config.c` | 181 | `PhysicsParams` + `sm_physics.json` hot-reload |
| `plm_core.c` | 1212 | PLM core + instruction byte-code handlers |
| `plm_blocks.c` | 646 | PLM block pre-instruction handlers |
| `plm_dispatch.c` | 293 | PLM dispatch |
| `plm_draw.c` | 322 | PLM OAM draw |
| `plm_preinstr.c` | 382 | PLM pre-instructions |
| `plm_rooms.c` | 525 | Room-triggered PLM handlers |
| `room_ceres.c` / `room_fx.c` / `room_main.c` / `room_scrolling.c` / `room_setup.c` / `room_state_select.c` / `room_transition.c` | ~2400 total | Room state machine, scrolling, FX, transitions |
| `hud.c` | 206 | HUD draw |
| `timer.c` | 137 | In-game timer |
| `irq.c` | 169 | IRQ handlers |
| `nmi_transfer.c` | 456 | NMI / VRAM upload |
| `game_init.c` | 488 | Boot / reset / logo |
| `game_state_extras.c` | 407 | Peripheral game-state handlers |
| `palette_fader.c` | 154 | Screen fade helpers |
| `util.c` | 183 | Shared util |
| `menu_assets.h` | header | Shared pause/map/options/equipment ROM tables and constants formerly grouped under Bank `$82` |

## Infrastructure (do not reshape)

These are not bank files — they are the C runtime shim. Keep.

| File | LOC | Purpose |
|------|-----|---------|
| `sm_rtl.c` / `sm_rtl.h` | 778 | ROM-access helpers (`RomPtr`, `RomFixedPtr`), async coroutines |
| `sm_cpu_infra.c` / `sm_cpu_infra.h` | 1030 | CPU/PPU register shim |
| `sm_dispatcher.c` / `sm_dispatcher.h` | 43 | Shim dispatch |
| `snes/*.c` | — | SNES hardware emulation (CPU/PPU/APU/DMA). Do not edit casually. |

## Bank-shaped files that still need porting

These `src/sm_XX.c` files are what the triage targets. Core priority rubric:
- **P0** — blocks core gameplay coverage or mini-build progress
- **P1** — important support / combat / shared infra
- **P2** — presentation, menus, non-core flow (safe to defer)

### System / engine banks

No system/engine bank files remain. The old `$80`-series engine work and the `$86`
enemy-projectile bank are now topical modules.

### Enemy banks

Core priority uses a separate rubric here:
- **P0** — shared enemy infra (unlocks everything else)
- **P1** — progression-critical bosses + broad combat coverage
- **P2** — isolated bosses / finale-specific / defer

| File | Bank | System | LOC | LOE | Priority | Split angle |
|------|------|--------|-----|-----|----------|-------------|
| `sm_a2.c` | $A2 | Enemy AI — shutters | 3693 | L | P2 | Gunship now lives in `enemy_gunship.c`; room-scripted shutter logic remains |
| `sm_a3.c` | $A3 | Enemy AI — shared A3 helpers, falling platforms | 237 | S | P1 | Fauna and Metroid now live in topical files; only the falling-platform slice plus shared wrappers remain |
| `sm_a4.c` | $A4 | Enemy AI — Crocomire | 1646 | M | P2 | Boss-only; self-contained |
| `sm_a5.c` | $A5 | Enemy AI — Draygon, Spore Spawn | 1603 | M | P2 | Two boss scripts |
| `sm_a6.c` | $A6 | Enemy AI — Ridley, zebetites | 4944 | XL | P1 | Late-game combat |
| `sm_a7.c` | $A7 | Enemy AI — Kraid, Phantoon | 3918 | L | P1 | Major boss pair |
| `sm_a8.c` | $A8 | Enemy AI — Ki-Hunter | 4068 | XL | P1 | Broad combat patterns, good midgame rep |
| `sm_a9.c` | $A9 | Enemy AI — Mother Brain, Shitroid | 6499 | XL | P2 | Largest + finale-specific. Defer. |
| `sm_aa.c` | $AA | Enemy AI — Tourian statue, Shaktool, Chozo statue | 516 | S | P1 | Torizo now lives in `enemy_torizo.c`; finish the Tourian statue/Shaktool cleanup to retire the bank |
| `sm_b2.c` | $B2 | Enemy AI — Space Pirates | 775 | S | P1 | Cheap common-enemy win |
| `sm_b3.c` | $B3 | Enemy AI — Botwoon | 1403 | M | P2 | Boss-only |

**Bank-shaped total remaining:** 29,302 raw lines across 11 files.

## What's driving priority

See [port_triage.md](port_triage.md) for the chunked plan. Short form:

1. Small enemies & progression-gate banks first (`sm_b2`, `sm_aa`, `sm_a3`).
2. Large boss banks next (`sm_a7`, `sm_a6`, `sm_a9`) once you want enemy-specific work.
3. Topical cleanup inside already-split modules stays secondary to retiring the remaining enemy banks.

## Non-coverage scope still open

1. **`cinematics.c` is still monolithic (6395 LOC)** — acceptable for now because it is
   presentation-only and not part of the mini path; only split it when someone wants to work
   on cinematics directly.
2. **`PhysicsParams`** — partially populated in `variables_extra.h`; not all Samus
   motion variables are exposed yet (beads `sm_rev-w93.6/7`).
3. **Headless test coverage** — tests use cold-boot RAM (title screen). No save-state
   loading, so physics/enemy regressions can't be caught yet (bead `sm_rev-w93.3`).
4. **Mini build link-up** — `samus_*.c` / `physics.c` / `physics_config.c` still not
   linked into `sm_rev_mini`. Needs `src/mini/stubs_mini.c` scaffolding first.
