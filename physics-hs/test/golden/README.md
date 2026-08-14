# Golden Tapes

## Status: Haskell Self-Output Only

`run_right.json` is Haskell `step` output for determinism. It is **not** a
Mini baseline and not TAS-correct.

### File

**run_right.json**: hold B+Right (`0x081`) for 6 frames

- predictor: `"haskell-v1"`
- flat floor at Y=200
- X increases, Y stays 200

### Wire Format: retro_rl@66836f5

```
B=0x001 Y=0x002 Select=0x004 Start=0x008
Up=0x010 Down=0x020 Left=0x040 Right=0x080
A=0x100 X=0x200 L=0x400 R=0x800
```

### Next Tape Work

Replace file-existence tests with residual-relevant compares against
existing C Mini:

1. Record a Mini tape from `MiniPredict` / `MiniStep`
   (`make mini-predict-golden` already exercises this oracle).
2. Compare H vs M on `samus_x/y`, subpixels, velocities, pose, movement type.
3. Keep Mini–emulator residual as the acceptance check
   ([docs/mini_emu_delta.md](../../../docs/mini_emu_delta.md)). If Mini ≠ emu,
   emu wins.

Segment coverage (idle / walk / run / jump / run-jump / land) lives in
`Test.Segments`, not in this golden file. Do not add slope / door goldens
in Haskell until Mini has an M–E residual budget for that geometry.

### Layers

1. **Haskell**: pure fragment, supporting layer
2. **Mini**: fast baseline (`MiniStep` / `MiniPredict`)
3. **Emulator**: ground truth (snes9x / libretro)
