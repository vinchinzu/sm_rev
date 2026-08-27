# sm_rev Port Triage — Closed Out

The bank-shaped `src/sm_*.c` era is over. All gameplay banks have been split into
topical modules. The only `sm_*.c` files left are runtime shims (`sm_rtl.c`,
`sm_cpu_infra.c`, `sm_dispatcher.c`), not bank files.

Final retirement landed 2026-04-26 (`1652b61`). Use [coverage.md](coverage.md)
to find the current home of any function.

## Where each retired bank lives now

| Retired bank | New home(s) |
|---|---|
| `sm_80.c` | `game_init.c`, `game_state_extras.c`, `nmi_transfer.c`, `palette_fader.c`, `util.c`, `hud.c`, `timer.c`, `irq.c` |
| `sm_81.c` | `save_sram.c`, `spritemap_draw.c`, `menu_common.c`, `file_select_menu.c`, `file_select_map.c`, `game_over_menu.c` |
| `sm_82.c` | split + `menu_assets.h` |
| `sm_84.c` | `plm_core.c`, `plm_blocks.c`, `plm_rooms.c`, `plm_preinstr.c`, `plm_draw.c`, `plm_dispatch.c` |
| `sm_85.c` | `message_box.c` |
| `sm_86.c` | `eproj_core.c`, `eproj_environment.c`, `eproj_tourian.c`, family files (`eproj_skree.c`, `eproj_draygon.c`, `eproj_crocomire.c`, `eproj_kraid.c`, `eproj_pirate.c`, `eproj_ki_hunter.c`, `eproj_botwoon.c`, `eproj_pickup.c`), leftover `eproj_combat.c` (dispatch + drain list) |
| `sm_87.c` | `anim_tiles.c` |
| `sm_88.c` | `hdma_core.c`, `hdma_power_bomb.c`, `room_fx_hdma.c`, `boss_hdma.c`, `cinematic_hdma.c` |
| `sm_89.c` / `sm_8f.c` | split |
| `sm_8b.c` | `cinematics.c` (topical rename) |
| `sm_8d.c` | `palette_fx.c` |
| `sm_90.c` / `sm_91.c` / `sm_92.c` / `sm_93.c` | `samus_*.c` cluster |
| `sm_9b.c` | `samus_death.c`, `samus_grapple.c`, projectile helpers |
| `sm_a0.c` | `enemy_main.c`, `enemy_ai_canon.c`, `enemy_ai_table.c`, `enemy_touch.c`, `enemy_shot.c`, `enemy_block_collision.c`, `enemy_math.c`, `enemy_drops.c`, `samus_status.c` |
| `sm_a2.c` | `enemy_gunship.c` plus family files (`enemy_shutter.c`, `enemy_goofball_rio.c`, `enemy_rinka.c`, `enemy_beyblade_turtle.c`, `enemy_hopping_blobs.c`, …) |
| `sm_a3.c` | `enemy_mochtroid.c`, `enemy_elevator.c`, `enemy_metroid.c`, `enemy_falling_platform.c` plus family files (`enemy_roach.c`, `enemy_maridia_snail.c`, `enemy_sidehopper.c`, `enemy_bang.c`, `enemy_zoomer.c`, …) |
| `sm_a4.c` | `enemy_crocomire.c` |
| `sm_a5.c` | `enemy_draygon.c`, `enemy_spore_spawn.c` |
| `sm_a6.c` | `enemy_ridley.c`, `enemy_ceres_ridley.c`, `enemy_zebetite.c`, plus Ceres door/steam, baby metroid, typewriter, waffle, fake Kraid, boulder, fire geyser |
| `sm_a7.c` | `enemy_kraid.c`, `enemy_phantoon.c` |
| `sm_a8.c` | `enemy_ki_hunter.c` (Ki-Hunter only) plus family files (`enemy_beetom.c`, `enemy_wrecked_ship.c`, `enemy_yapping_maw.c`, …) |
| `sm_a9.c` | `enemy_mother_brain.c`, `enemy_shitroid.c`, `enemy_dead_monsters.c` |
| `sm_aa.c` | `enemy_torizo.c`, `enemy_chozo_shaktool.c` |
| `sm_ad.c` | `mother_brain_hdma.c` |
| `sm_b2.c` | `enemy_space_pirates.c` |
| `sm_b3.c` | `enemy_botwoon.c` |
| `sm_b4.c` | split |

## Optional follow-up splits (do only when a topic blocks you)

Leftover bank-remainder dumps are gone. The remaining large files are topical,
not mixed-bank leftovers. Split only if you're working in one and the size hurts.

- `cinematics.c` (~6400 LOC, ex `sm_8b.c`) — could split into `cinematic_ppu.c`,
  `cinematic_bg.c`, `cinematic_sprite.c`, `cinematic_palette.c`, `cinematic_intro.c`,
  `cinematic_ending.c` + `credits.c`. Presentation-only; not the vanilla-gameplay
  cleanliness path.
- `eproj_combat.c` (~999 LOC) — leftover `$86` dispatch plus a drain list in
  `docs/bank_origin_map.md` (Phantoon fireballs, walking lava seahorse, remaining
  Torizo/Chozo handlers).
- `enemy_mother_brain.c` (~3643 LOC) and `enemy_ridley.c` (~2765 LOC) are now
  single-boss files; C-ify in place rather than splitting for line count.

The mini-build / Samus-physics path doesn't depend on these.

## Refactor protocol (still applies for any future work)

1. `python3 -m pytest tests/test_build.py -q` (9 tests, ~80s)
2. `make mini-test` (~1s)
3. `python3 tests/run_tests.py -v` if runtime behavior may have shifted
   (`test_deterministic_across_runs` is flaky on SPC state — re-run before investigating)

Keep moves mechanical and behavior-preserving. Refactor second.
