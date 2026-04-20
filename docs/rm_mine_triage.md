# RM_MINE Triage Log

Started 2026-04-19. The C port (RM_MINE) is the production target. Mods rely on it. RM_BOTH reconciled divergences to THEIRS every frame, masking ~24 wip-commits' worth of regressions since `d9c45c6 Remove sm_82.c`. Forcing RM_MINE surfaced them all at once.

THEIRS and RM_BOTH are now used **only** as diagnostic oracles, never as the correctness goal.

Baseline note:
- The sibling `../sm/` tree is the pre-refactor, bank-shaped baseline to consult when a split topical file regresses.
- A recurring cause of regressions in this branch has been refactoring without running the smallest relevant build/runtime checks before and after the change.
- See [bank_origin_map.md](/home/v/01_projects/11_games/speedrun/retro_rl/super_metroid_rl/sm_rev/docs/bank_origin_map.md) for the current file-to-bank lookup.

## Known regressions (user-reported 2026-04-19)

| # | Where | Symptom | Status |
|---|-------|---------|--------|
| 1 | save0 (Parlor) | wrong tiles, camera scroll wrong | open |
| 2 | First room (Alcatraz on cold boot) | mangled enemy sprites + wrong tiles | open |
| 3 | save1 (ship landing) | rain FX missing | open |
| 4 | save3 (post-spore-spawn room) | crashes at end of room with `Unreachable!` at sm_rtl.c:576 | open |
| 5 | Ceres intro | metroid jar sprite missing | open |
| 6 | Ridley fight | sprites mangled | open |
| 7 | (general) | "no fire" — Norfair ambient fire FX | open |

## Repro recipes

Cold boot (RM_MINE forced via `sm_mods.json` knobs):
```
./sm_rev
```

Headless slot load:
```
SDL_VIDEODRIVER=offscreen SDL_AUDIODRIVER=dummy DISPLAY= \
  ./sm_rev --headless 600 --runmode mine --load-state saves/save3.sav
```

RM_BOTH divergence dump (THEIRS as oracle, NOT as goal):
```
./sm_rev --runmode both --verbose --load-state saves/save0.sav
```

## Strategy

1. Establish per-slot RM_BOTH divergence baseline — first frame & subsystem where MINE diverges (oracle-only; production stays RM_MINE).
2. Fix slot 3 `Unreachable` first (deterministic, easy backtrace).
3. Tiles + scroll in parlor + alcatraz (user-flagged priority).
4. FX cluster: jar / fire / rain — likely same root cause.
5. Enemy gfx cluster: alcatraz enemies + Ridley.
6. Lock in: per-slot RM_BOTH smoke test in tests/ so this class can't silently regress.

## Progress

- [ ] Per-slot divergence baseline captured
- [ ] Slot 3 `Unreachable!` root-caused (cold-load of save3.sav alone does NOT crash for 5000+ frames; needs slot-cycling or post-load gameplay to trigger — Unreachable() now prints a backtrace via execinfo to make next repro self-localizing)
- [ ] Slot 3 crash fixed
- [ ] Parlor (save0) tiles correct
- [ ] Parlor (save0) camera scroll correct
- [ ] Alcatraz (cold boot) tiles correct
- [ ] Alcatraz (cold boot) enemy gfx correct
- [ ] Save1 ship-landing rain restored
- [ ] Ceres metroid jar sprite restored
- [ ] Norfair fire FX restored
- [ ] Ridley sprite gfx fixed
- [ ] RM_BOTH divergence smoke test added

## Notes

(Findings get appended below as we go.)

### Prior diagnostic notes

- `docs/save0_first_room_diagnostics.md` — earlier investigation of save0. Confirmed the rendered frame is byte-identical between MINE and THEIRS for that single load (i.e. visible corruption isn't divergence on that frame — it's a baseline asset/render issue affecting both paths). Two repairs already applied:
  - `src/decompress.c:142` — fixed inherited `b++` bug in `DecompressToVRAM` literal-copy path.
  - `src/sm_rtl.c` — added `RtlRefreshRoomAssetsAfterLoad()` to rebuild room/CRE/scroll/enemy-gfx after gameplay savestate load.
