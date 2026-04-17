# TODO

## Torizo Room Crash / Missing Enemies

### Current state

- Repro save: `saves/save2.sav`
- Crash room from `door.log`: room `0x9804`, state `0x981B`
- Room state debug showed:
  - music set / track: `36 / 3`
  - enemy population: `0x84ED`
  - enemy tilesets: `0x80B3`
- Crash is not movement code. It dies in the SPC player:
  - `SPC unknown effect=0xFF`
  - assert in `src/spc_player.c`

### What was cleaned up

- Removed temporary movement spam from:
  - `src/sm_90.c`
  - `src/physics.c`
- Reduced repeated room-enemy spam in:
  - `src/sm_a0.c`

### Debug instrumentation currently present

- `src/sm_8f.c`
  - logs room-state selection to stderr as `ROOM state-select ...`
- `src/sm_a0.c`
  - logs enemy load summary to stderr as `ROOM enemies ...`
  - throttled so it only prints when the room/enemy state actually changes
- `src/sm_80.c`
  - logs music queue/application as:
    - `MUSIC queue8 ...`
    - `MUSIC queueY ...`
    - `MUSIC apply upload ...`
    - `MUSIC apply track ...`
- `src/spc_player.c`
  - logs unknown SPC effect with surrounding bytes and current song state as `SPC unknown effect=...`
- `src/main.c`
  - wider `--dump` JSON
  - temporary debug env hook:
    - `SM_FORCE_MUSIC=<set>,<track>`
    - example: `SM_FORCE_MUSIC=36,3`

### Findings so far

- Forcing music set `0x24` / track `3` directly from `save2` does **not** crash headless by itself.
- That means the bad path is likely an additional music command during the real door / bomb / event sequence, not just the initial room music load.
- The forced-music log sequence was:
  - `MUSIC queue8 raw=0x0000`
  - `MUSIC queue8 raw=0xFF24`
  - `MUSIC queueY raw=0x0003 delay=6`
  - `MUSIC apply track=0`
  - `MUSIC apply upload set=0x24`
  - `MUSIC apply track=3`
- `save2` loaded headless before the door with suspicious SPC state:
  - `spc_port0=5`
  - `spc_track3_ptr=0`
  - `spc_chan8_ptr=0`
  - but this alone did not reproduce the crash when music was forced manually

### Enemy note

- `ROOM enemies ... first=EEFF,0000,0000,0000` is expected from room enemy population `0x84ED`.
- ROM inspection shows the first entry at `0xA184ED` is enemy id `0xEEFF`, followed immediately by terminator `0xFFFF`.
- So that specific line does not by itself prove the missing-enemies bug.

### Most useful next step

Reproduce the actual door transition with:

```bash
./sm_rev --debug --load-state saves/save2.sav 2> door.log
```

Then inspect:

```bash
grep -E 'ROOM |MUSIC |SPC ' door.log | tail -n 80
```

The goal is to capture the exact sequence right before the assert:

- `ROOM state-select`
- `MUSIC queue8`
- `MUSIC queueY`
- `MUSIC apply upload`
- `MUSIC apply track`
- `SPC unknown effect`

That should identify which actual play index / queue sequence causes channel 8 to land on invalid SPC data.

### Likely follow-up after that

- If the bad sequence differs from plain `36/3`, inspect the caller that queued the extra music command.
- If the queue sequence is correct, compare live SPC RAM before and after the door transition to see where the bad pointer enters.
- Once resolved, remove the temporary music/SPC debug logging and `SM_FORCE_MUSIC` hook from:
  - `src/main.c`
  - `src/sm_80.c`
  - `src/sm_8f.c`
  - `src/sm_a0.c`
  - `src/spc_player.c`
