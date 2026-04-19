# `save0` First-Room Diagnostics

Target repro:

```bash
SDL_VIDEODRIVER=offscreen SDL_AUDIODRIVER=dummy DISPLAY= \
./sm_rev --headless 120 --runmode mine --load-state saves/save0.sav --dump-frame /tmp/save0.raw --dump -
```

Current observed room state from `save0.sav`:

- `room_ptr = 0x92FD`
- `roomdefroomstate_ptr = 0x932E`
- `room_enemy_population_ptr = 0x8261`
- `room_enemy_tilesets_ptr = 0x8067`
- `num_enemies_in_room = 16`

What was verified:

- The rendered frame from `save0.sav` is byte-identical in `mine` and `theirs`.
- That means the visible corruption is in shared systems, not in the native-C gameplay divergence path.
- Room-state selection for `0x92FD` is stable and resolves to state `0x932E` because event `0` is set.
- The selected enemy population and enemy tileset table are internally consistent:
  - population enemy defs: `0xDCFF`, `0xDB7F`, `0xD47F`
  - tileset entries: `0xDCFF`, `0xDB7F`, `0xD47F`
- So this specific repro is not explained by a mismatched enemy population / enemy-tileset pairing.

Repairs applied:

1. Fixed an inherited direct-to-VRAM decompression bug in [`src/decompress.c`](/home/v/01_projects/11_games/speedrun/retro_rl/super_metroid_rl/sm_rev/src/decompress.c:130).
   The literal-copy path in `DecompressToVRAM` was incrementing each byte before writing it.
   The same bug exists in the older `../sm` baseline, so this is not new to `sm_rev`.
2. Added a post-load room asset rebuild in [`src/sm_rtl.c`](/home/v/01_projects/11_games/speedrun/retro_rl/super_metroid_rl/sm_rev/src/sm_rtl.c:89) for gameplay savestates.
   After state load, the code now rebuilds room headers/state-derived graphics and then copies the refreshed PPU state into both render paths.

Status after those repairs:

- `save0.sav` still renders the same frame after load.
- So the remaining issue is not just stale savestate VRAM/room caches.

Most likely remaining fault domains:

- shared room graphics generation after the correct room/state has already been selected
- shared PPU-side rendering behavior
- a baseline content/asset-loading bug that predates this refactor branch

Recommended next check:

- Compare this same room against the older `../sm` build with an equivalent `MINE`-only path or a screenshot reference.
- Restore the old screenshot-reference test harness for `save0` once a trusted `crateria_start.png` reference is available.
