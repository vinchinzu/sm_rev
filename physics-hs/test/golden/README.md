# Golden Tapes - Haskell Determinism

## Status: Haskell Self-Output (NOT Mini Baseline)

These JSON files are **Haskell step function output** for determinism testing.

**NOT Mini baseline** - just verify Haskell is consistent with itself.

### Files

1. **run_right_60f.json**: Ground run, hold B+Right (0x081) for 60 frames
   - Flat floor Y=200
   - X position increases, Y stays 200 (on ground)
   - Input: buttons=129 (B=0x001 + Right=0x080)

2. **short_hop.json**: Hold A (0x100) for 10 frames, release
   - Jump squat (4 frames) → jump → rise → fall → land Y=200
   - Peak Y < full hop

3. **full_hop.json**: Hold A (0x100) for 40 frames
   - Jump squat (4 frames) → jump → rise higher → fall → land Y=200
   - Peak Y lower (higher on screen) than short hop

### Wire Format: retro_rl@66836f5 12-bit SNES Mask

**FrameInput buttons** (LOCKED encoding):
```
B=0x001 Y=0x002 Select=0x004 Start=0x008
Up=0x010 Down=0x020 Left=0x040 Right=0x080
A=0x100 X=0x200 L=0x400 R=0x800
```

**Golden examples**:
- 128 = Right only (0x080)
- 129 = B+Right (0x081)
- 256 = A only (0x100)

### Source Labeling

All files have:
```json
{
  "source": "haskell-step-function",
  "note": "NOT Mini baseline - Haskell self-output for determinism testing only"
}
```

### Mini Baseline Goldens (Future)

To record **Mini baseline** goldens (fast iteration, not TAS-correct):
1. Implement signed velocity, momentum, speed tracking
2. Run: `echo '{"start": <SimState>, "inputs": [...]}' | sm_rev_mini_oracle --json`
3. Compare Haskell vs Mini, document deltas
4. **Acceptance**: Verify both against real emulator (snes9x/libretro)

**Emulator is ground truth**. If Mini ≠ emu, emu wins.

### Current Use

Tests verify:
- JSON files exist
- Haskell step function is deterministic (same input → same output)

Not verifying:
- Mini baseline parity (gaps: signed velocity, momentum, speed)
- TAS correctness (emulator acceptance layer)
