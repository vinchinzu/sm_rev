# sm_rev Function Coverage Audit

Function coverage is complete (5,156 fns, 0 missing). This file is now a **structural**
audit: which bank-shaped files still exist, how big they are, and what they contain.
The actionable porting plan lives in [port_triage.md](port_triage.md).

Last refreshed: 2026-04-18.

## Extracted topical modules

Already split out of the original bank soup. These are the long-term homes — refactor
into them, don't recreate bank-shaped files.

| Module | LOC | What it holds |
|--------|-----|---------------|
| `samus_anim_fx.c` | 581 | Per-pose animation/FX driver |
| `samus_camera_map.c` | 335 | Camera tracking + minimap updates |
| `samus_collision.c` | 1271 | Block/slope/enemy collision, pose-change collision |
| `samus_demo.c` | 342 | Attract-mode demo playback |
| `samus_draw.c` | 761 | Samus OAM + sprite-map draw |
| `samus_enemy_collision.c` | 146 | Samus ↔ enemy hitbox dispatch |
| `samus_grapple.c` | 742 | Grapple beam (fire/connect/swing/cancel) |
| `samus_input.c` | 352 | Joypad → pose/movement-type dispatch |
| `samus_jump.c` | 224 | Jump/walljump/bombjump/knockback init |
| `samus_motion.c` | 406 | MoveX/Y primitives |
| `samus_palette.c` | 591 | Beam/suit/speed-boost palette cycles |
| `samus_pose.c` | 547 | Pose-change gate + movement-type dispatcher |
| `samus_projectile.c` | 2727 | Projectile spawn/move/collide — **oversized; split candidate** |
| `samus_resource.c` | 63 | Health/missile/super/PB add-sub helpers |
| `samus_runtime.c` | 1156 | Per-frame Samus handler entry + misc |
| `samus_special_move.c` | 913 | Shinespark, crystal flash, MB scripted states |
| `samus_speed.c` | 363 | Speed-booster acceleration & extra-run-speed |
| `samus_transition.c` | 947 | New-pose transition state machine |
| `samus_xray.c` | 749 | X-ray HDMA + activation |
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
| `sm_82_data.c` + `sm_82_data.h` | 30 | Tables formerly in Bank $82 |

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

| File | Bank | System | LOC | LOE | Core priority | Triage note |
|------|------|--------|-----|-----|---------------|-------------|
| `sm_81.c` | $81 | SRAM, spritemaps, save/file-select/map menus | 2527 | L | P2 | Menu/UI mass; low gameplay leverage. Clear split: save vs. menus vs. spritemap helpers. |
| `sm_85.c` | $85 | Message boxes | 427 | S | P2 | Isolated UI; fast standalone win. |
| `sm_86.c` | $86 | Enemy projectiles (eprojs) | 5294 | XL | P1 | Biggest remaining combat surface. Factors cleanly by owning boss/room. |
| `sm_87.c` | $87 | Animated tiles | 273 | XS | P2 | Tiny; lift to `anim_tiles.c` in one session. |
| `sm_88.c` | $88 | HDMA — layer blending, power bomb, liquid FX, suit pickup | 3342 | L | P2 | Large presentation bank; splits cleanly by HDMA effect. |
| `sm_8b.c` | $8B | Cinematics (intro, credits, Mode7, text) | 6395 | XL | P2 | Biggest file remaining. Safely last. |
| `sm_8d.c` | $8D | Palette-FX objects | 324 | S | P2 | Small enough to lift whole → `palette_fx.c`. |
| `sm_9b.c` | $9B | Samus death seq, grapple (partial), projectile trail | 1122 | M | P1 | Small but touches Samus death + grapple — split into existing `samus_*` files. |

### Enemy banks

Core priority uses a separate rubric here:
- **P0** — shared enemy infra (unlocks everything else)
- **P1** — progression-critical bosses + broad combat coverage
- **P2** — isolated bosses / finale-specific / defer

| File | Bank | System | LOC | LOE | Priority | Split angle |
|------|------|--------|-----|-----|----------|-------------|
| `sm_a0.c` | $A0 | Common enemy AI, collision, math, drops | 3656 | L | **P0** | Splits ≥4 ways: enemy_main / enemy_collision / enemy_math / enemy_drops. Unlocks everything below. |
| `sm_a2.c` | $A2 | Enemy AI — gunship, shutters | 4132 | XL | P2 | Room-scripted behaviors; defer |
| `sm_a3.c` | $A3 | Enemy AI — elevator, Metroid | 3111 | L | P1 | Isolate Metroid from elevator scripting |
| `sm_a4.c` | $A4 | Enemy AI — Crocomire | 1646 | M | P2 | Boss-only; self-contained |
| `sm_a5.c` | $A5 | Enemy AI — Draygon, Spore Spawn | 1603 | M | P2 | Two boss scripts |
| `sm_a6.c` | $A6 | Enemy AI — Ridley, zebetites | 4944 | XL | P1 | Late-game combat |
| `sm_a7.c` | $A7 | Enemy AI — Kraid, Phantoon | 3918 | L | P1 | Major boss pair |
| `sm_a8.c` | $A8 | Enemy AI — Ki-Hunter | 4068 | XL | P1 | Broad combat patterns, good midgame rep |
| `sm_a9.c` | $A9 | Enemy AI — Mother Brain, Shitroid | 6499 | XL | P2 | Largest + finale-specific. Defer. |
| `sm_aa.c` | $AA | Enemy AI — Torizo, Tourian statue, Shaktool | 1546 | M | P1 | Compact progression-gate coverage |
| `sm_ad.c` | $AD | Mother Brain HDMA support | 443 | S | P2 | Mostly Phase-3 HDMA |
| `sm_b2.c` | $B2 | Enemy AI — Space Pirates | 775 | S | P1 | Cheap common-enemy win |
| `sm_b3.c` | $B3 | Enemy AI — Botwoon | 1403 | M | P2 | Boss-only |

**Bank-shaped total remaining:** 60,402 raw lines across 22 files.

## What's driving priority

See [port_triage.md](port_triage.md) for the chunked plan. Short form:

1. Samus / SNES / mini-build unblockers first (`sm_9b`, leftover `sm_80` migration).
2. Cheap atomic modules next (`sm_87`, `sm_8d`, `sm_85`).
3. Shared enemy infrastructure (`sm_a0`) before any individual enemy bank.
4. Enemy projectiles (`sm_86`) after common enemy infra exists.
5. Small enemies & progression-gate bosses (`sm_b2`, `sm_aa`).
6. Large HDMA / menu banks (`sm_88`, `sm_81`) when blocked by them.
7. Cinematics (`sm_8b`) and full-boss banks (`sm_a9`, `sm_a6`, etc.) last.

## Non-coverage scope still open

1. **`samus_projectile.c` oversized (2727 LOC)** — candidate for projectile_core /
   projectile_special_move / projectile_block_collision split.
2. **`PhysicsParams`** — partially populated in `variables_extra.h`; not all Samus
   motion variables are exposed yet (beads `sm_rev-w93.6/7`).
3. **Headless test coverage** — tests use cold-boot RAM (title screen). No save-state
   loading, so physics/enemy regressions can't be caught yet (bead `sm_rev-w93.3`).
4. **Mini build link-up** — `samus_*.c` / `physics.c` / `physics_config.c` still not
   linked into `sm_rev_mini`. Needs `stubs_mini.c` scaffolding first.
