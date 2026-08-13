# Golden Tapes - Haskell Determinism Only

## Status: Haskell Self-Output (NOT Mini Baseline)

This JSON file is **Haskell step function output** for determinism testing.

**NOT Mini baseline** - no C MiniStep oracle. No TAS-correct claims.

### File

**run_right.json**: Ground run, hold B+Right (0x081) for 6 frames
- Haskell `step` function output (predictor: "haskell-v1")
- Flat floor collision at Y=200
- X position increases, Y stays 200 (on_ground=true)
- Input: buttons=0x081 (B=0x001 + Right=0x080)

### Wire Format: retro_rl@66836f5 12-bit SNES Mask

**FrameInput buttons**:
```
B=0x001 Y=0x002 Select=0x004 Start=0x008
Up=0x010 Down=0x020 Left=0x040 Right=0x080
A=0x100 X=0x200 L=0x400 R=0x800
```

### No Hop JSON

**HopRise tests prove Y decreases** via unit tests, not golden JSON.

Do not record hop JSON until:
1. Mini C oracle (`sm_rev_mini_oracle`) exists and is merged
2. Haskell vs Mini short/full hop compared on x/y/subX/subY/pose
3. Both validated against real emulator (snes9x/libretro)

### Layers

1. **Haskell pure model**: `(State, Input) -> State` (this repo)
2. **Mini baseline**: Fast iteration oracle (C `MiniStep`, not yet integrated)
3. **Emulator acceptance**: TAS ground truth (snes9x/libretro, future)

**Emulator is ground truth**. If Mini ≠ emu, emu wins. Mini is iteration speed, not correctness stamp.

### Current Use

Tests verify:
- `run_right.json` file exists
- Haskell step function is deterministic (same input → same output)

Not verifying:
- Mini baseline parity (no C oracle integrated)
- TAS correctness (emulator acceptance layer is future work)
