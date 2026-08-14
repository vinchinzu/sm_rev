# Mini–emulator residual delta

Measured 2026-08-13. **If Mini ≠ emu, emu wins.**

- E: `sm_rev --runmode theirs` (in-repo snes9x). Not RetroRL SuperMetroidEnv.
- C-port: `sm_rev --runmode mine --load-state saves/save0.sav`
- Mini: `sm_rev_mini` ROM-backed Landing Site (demo boot)
- Horizon: 60 frames
- Residual words: `$0AF6`/`$0AFA` (pixels) + `$0AF8`/`$0AFC` (subpixels)

| Tape | Pair | Start match | fd pixels | fd sub+pixel | First field | Notes |
| --- | --- | --- | --- | --- | --- | --- |
| idle | mine vs theirs | yes | ∞ | ∞ | — | both stay at (1153, 1088) sub (0, 0x8000), room `$91F8` |
| idle | mini vs theirs | pixels only | ∞ | 0 | samus_y_sub | Mini `$0AFC=0`, emu `$0AFC=0x8000` at frame 0 |
| walk-right | mine vs theirs | yes | ∞ | ∞ | — | `--inputs 100` did not change pose or position on either machine |
| walk-right | mini vs theirs | pixels only | 2 | 0 | samus_y_sub | Mini consumed RIGHT (pose 0→38, x 1153→1262). Emu did not. Starts already disagree on `$0AFC` |

## Mini deltas

1. **Mini boot zeros subpixels.** Same Landing Site pixels (1153, 1088) as `save0`, but Mini writes `$0AF8/$0AFC = 0`. Emu/`save0` has `$0AFC = 0x8000`. This is a Mini initialization delta, not a step-function residual. Planning that cares about subpixels cannot treat Mini CLI boot as a corresponding start.
2. **Mini CLI has no `--load-state`.** Corresponding-start walk residual against a `.sav` is not available from `sm_rev_mini`. `MiniLoadState` exists in-process.
3. **`sm_rev --inputs` did not drive Samus from `save0`.** Pose stays 0 (face-forward). Mini's joypad path did drive a turn/run. Walk M–E is unmeasured until the full-port input tape actually moves Samus.

## What this tells planning

The C port matches the emulator on `$0AF6/$0AFA/$0AF8/$0AFC` for 60 idle frames from the same `.sav`. The residual-relevant kernel Mini original-runtime uses is not immediately drifting while standing.

Mini CLI is not yet a planning oracle: it does not share `$0AFC` at boot, and it cannot hydrate from the emu start. File those as Mini work. Do not grow Haskell to paper over them.

Haskell is not part of this measurement.
