# Haskell Physics Kernel for Super Metroid

`physics-hs` is a supporting pure model of the residual-relevant Samus
fragment. Mini is the fast baseline; the emulator is ground truth. If
Mini ≠ emu, emu wins. Do not grow this package into a second Super Metroid.

```
Haskell (pure fragment / rollouts)
        │  optional CLI compare via sm_rev_predict
        ▼
MiniStep / MiniPredict (fast baseline)
        │  residual profiles
        ▼
Emulator (snes9x / libretro)
```

## Mapping

| Haskell Module | C Source | Purpose |
|----------------|----------|---------|
| `Physics.SM.Types` | `mini/mini_game.h` | Position, velocity, extra run |
| `Physics.SM.Constants` | `ida_types.h` | Buttons / poses |
| `Physics.SM.SpeedTable` | ROM `$90:9F55` | Walk / run / jump / spin / fall |
| `Physics.SM.Momentum` | `src/samus_speed.c` | Extra run / speed-booster residual |
| `Physics.SM.Pose` | `src/samus_pose.c` | Stand / run / crouch / morph / land |
| `Physics.SM.Run` | `src/samus_speed.c` | Ground + air X |
| `Physics.SM.Jump` | `src/samus_jump.c` | Jump squat, `4.E000` impulse, spin |
| `Physics.SM.Gravity` | `src/samus_motion.c` | Pre-gravity move, A-release, land leftover |
| `Physics.SM.Step` | `mini/mini_game.c` | Frame step |
| `Physics.SM` | — | `runTape`, FFI |

Jump / gravity config is `EnvTable Velocity` (`envAir` / `envWater` /
`envLava`). Extra-run matches C: quirked-greater check, then add
(overshoot allowed). Speed-booster jump uses the C two-register add.

## Tests

```bash
cd physics-hs && cabal test
make hm-test   # builds sm_rev_predict, sets HM_REQUIRED=1
```

- `Test.Unit`: walk/run, extra-run overshoot, squat A-release, booster jump
- `Test.Segments`: idle / walk / run / jump / run-jump / land leftover
- `Test.Properties`: determinism (HUnit + one random-tape QuickCheck)
- `Test.MiniCompare`: `HM_REQUIRED=1` requires `sm_rev_predict`. Field
  compare runs only when Mini's first frame matches the requested start.
  Mini does not hydrate SimState JSON yet, so `make hm-test` is a CLI
  smoke plus a ready-made compare.

`test/golden/run_right.json` is a sample Haskell tape, not an H↔M check.

## Out Of Scope

Slopes, walls, knockback, doors, enemies, TAS lag. Those stay Mini /
emulator work.
