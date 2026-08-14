# Known Issues and Scope

Haskell is a supporting layer that is now allowed to own the residual-
relevant Samus fragment. Issues below are either fragment bugs or
reminders of what this package must not absorb.

## Residual-Relevant Boundary

`toSimState` / `fromSimState` round-trip residual-relevant fields
(`samus_x/y`, subs, velocities, extra-run as `momentum_x`, pose, facing,
movement type, frame, speed flag/counter). `on_ground` is derived from
movement type. Previous buttons are still not on the wire; `sm-predict`
threads them across a tape.

Shinespark sparking itself stays out until Mini has an M–E budget.

## Collision

**Status**: Flat floor at `cfgGroundY` only. Landing keeps leftover Y
subpixel (SMB land residual) and leftover X / extra run.

Slopes, platforms, walls, and ceilings stay Mini / emulator work.

## Environment / Equipment

`Environment` and hi-jump / speed-booster / morph flags are live. Default
config is air + no items (Ceres). Water and lava tables exist and are
selected when `stateEnvironment` is set.

## Oracle / Baseline

Mini C already has `MiniPredict` and `tests/mini_predict_golden.c`.
`Test.MiniCompare` calls `sm_rev_predict` when it is on `PATH` or
`SM_REV_PATH`, and skips if the binary is missing. `run_right.json` is
still Haskell self-output.

## Emulator Acceptance

Not a Haskell job. First profile: [docs/mini_emu_delta.md](../docs/mini_emu_delta.md).
If Mini ≠ emu, emu wins.

## Testing

`cabal test` covers unit + segment + property tests. Segment list matches
the SMB residual harness shape: idle, walk, run, jump, run-jump, land.

## Out Of Scope

Do not treat these as Haskell backlog:

- slopes / walls / doors / enemies / projectiles
- TAS-correct lag or exact door transitions
- full pose-animation tables
- shinespark crash / crystal flash
