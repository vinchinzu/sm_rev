# physics-hs — Residual-Complete Pure Model, Still Subordinate To Mini/Emu

Haskell stays. It does not compete with Mini or the emulator. It is now
allowed to grow the **residual-relevant Samus fragment** so pure rollouts
are actually useful, the same way `retro_rl/nes/smb/approx.py` grew until
idle / walk / jump / run-jump / land tapes held.

Use it for:
- side-effect-free rollouts (easy to parallelize from ML / RL / planning)
- property tests that are painful in mutating C
- type-safe residual-relevant state (`Pixel` / `Subpixel` / `Position` / `Velocity`)
- a readable spec of the implemented motion fragment
- cheap H↔M CI once `MiniPredict` / `MiniStep` is wired

Do not use it as a second Super Metroid.

## Architecture (Do Not Invert)

```
H  (pure fragment)  --observe-->  M  (MiniStep / MiniPredict)  --residual-->  E  (snes9x)
```

- Mini is the fast baseline, not TAS-correct.
- Emulator is ground truth. If Mini ≠ emu, emu wins.
- Haskell agrees with the *implemented* residual-relevant fragment, or it is wrong.
- Mini–emulator residual profiles stay the acceptance check for TAS planning.

## What Works

- Compiles with `-Wall -Werror`
- Wire format matches retro_rl@66836f5 (`FrameInput`, `SimState`, `Trajectory`)
- ROM X speed tables (`$90:9F55` / water / lava): walk, run, jump, spin, fall, morph, crouch
- Walk without B; run with B builds extra-run / momentum
- Air X on the leave-ground frame (not grounded leftovers)
- Vanilla jump impulse `4.E000` (−4.875), A-release snaps to falling
- Flat-floor landing keeps Y subpixel leftover
- Segment tests: idle, walk, run, jump, run-jump, land
- Determinism property: same tape → same states

## What This Package Must Not Become

- A dual implementation of doors, enemies, full lag, or slopes
- A kernel that blocks C collision / knockback / M–E work
- A planning oracle that is never called from `cabal test` or retro_rl

## Gaps That Still Matter

- No slopes, walls, ceilings, or knockback
- Jump squat is a 4-frame timer, not the full pose-anim table
- `run_right.json` is Haskell self-output, not a Mini tape
- H↔M compare lives in `Test.MiniCompare` (`make hm-test`) and is optional
- First Mini–emulator residual: [docs/mini_emu_delta.md](../docs/mini_emu_delta.md)

## Build

```bash
cd physics-hs
cabal build
cabal test
```

See [docs/physics_haskell.md](../docs/physics_haskell.md) and
[docs/roadmap.md](../docs/roadmap.md) for the project-level path.
