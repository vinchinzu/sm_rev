# physics-hs

Pure Haskell model of the residual-relevant Samus fragment. Mini is the
fast baseline; the emulator is ground truth. If Mini ≠ emu, emu wins.

Use it for side-effect-free rollouts, property tests, and a typed spec of
the implemented motion fragment. Do not grow it into a second Super Metroid.

## What Works

- ROM walk / run / jump / spin / fall tables
- Extra-run (including C overshoot-then-snap)
- Air X on leave-ground, vanilla `4.E000` jump impulse, A-release
- Flat-floor land keeps Y subpixel leftover
- `cabal test`: unit, segment, determinism property
- `Test.MiniCompare` calls `sm_rev_predict` when present (`make hm-test`
  requires the CLI via `HM_REQUIRED=1`)

## Build

```bash
cd physics-hs
cabal build
cabal test
```

See [docs/physics_haskell.md](../docs/physics_haskell.md).
