# Known Issues and Scope

Haskell owns the residual-relevant Samus fragment. It is not a Mini
replacement and not TAS-correct.

## Residual-Relevant Boundary

`toSimState` / `fromSimState` round-trip position, velocity, extra-run
(`momentum_x`), pose, facing, movement type, frame, and speed flag/counter.
`on_ground` is derived from movement type. Previous buttons are not on the
wire; `threadInputs` / `sm-predict` thread them across a tape.

## Collision

Flat floor at `cfgGroundY` only. Landing keeps leftover Y subpixel.
Slopes, walls, and ceilings stay Mini / emulator work.

## H↔M

`Test.MiniCompare` requires `sm_rev_predict` when `HM_REQUIRED=1`
(`make hm-test`). Field compare runs only if Mini's first frame matches
the requested start. Mini does not hydrate SimState JSON yet, so the
hook is a CLI smoke plus a ready-made compare.

## Out Of Scope

- slopes / walls / doors / enemies / projectiles
- TAS-correct lag or exact door transitions
- full pose-animation tables
- shinespark crash / crystal flash
