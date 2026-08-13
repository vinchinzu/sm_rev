# Honest README: Early Skeleton, Major Gaps

## Status: Compiles but NOT MiniStep-Ready

This package is an **early implementation** with critical gaps that prevent MiniStep bisimulation.

### What Works ✅

- **Compilation**: -Wall -Werror clean (imports fixed, no unused bindings)
- **Type safety**: Newtypes prevent unit mixing (Pixel, Subpixel, Position, Velocity)
- **Named constants**: PhysicsConfig holds accel/decel/gravity/squat values
- **Fixed model bugs**:
  - B-release uses cfgRunDecel (not zero)
  - jumpSquatDuration in PhysicsConfig (not magic 4)
  - Apex transitions to VDirFalling (gravity after peak)

### Critical Gaps Remaining 🚫

**1. No collision detection**
   - `stateOnGround` never becomes `false` after `initJump`
   - Cannot detect landing → no landing frame golden
   - Cannot detect platforms → no 1-tile platform golden

**2. Direction tracking incomplete**
   - accelerateLeft/Right both increase magnitude (C design)
   - Actual direction requires pose_x_dir tracking
   - Position updates don't apply signed velocity

**3. Wire format misaligned**
   - Internal: `btnLeft=0x0200 btnRight=0x0100` (16-bit)
   - Required: `Left=0x40 Right=0x80` (8-bit packed for retro_rl)

### Cannot Record MiniStep Goldens Until

1. **Collision system**: Ground detection, platform edges, wall collision
2. **Landing detection**: stateOnGround transitions false→true
3. **Pose/direction**: Track facing for leftward movement
4. **Wire alignment**: 0x40/0x80 packed format

### Test Status

- ❌ **Golden tapes**: Cannot record (no collision/landing)
- ❌ **Property tests**: Would need MiniStep binary (not in env)
- ✅ **Unit tests**: Would pass if cabal available
- ✅ **Compilation**: All imports at headers, no unused bindings

### Architecture (Correct)

**Layer 1: MiniStep Baseline** (fast iteration, simplified model, NOT TAS-correct)  
**Layer 2: Emulator Acceptance** (snes9x/libretro via SMEDIT/retro_rl, ground truth)

If Mini ≠ emu, emulator wins.

### Build

```bash
cd physics-hs
cabal build   # Would compile with -Wall -Werror
cabal test    # Would run unit tests
```

**Cannot claim MiniStep parity**. Collision system required for goldens.
