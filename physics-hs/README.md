# Honest README - Mini Baseline Iteration

## Status: Compiles, NO Mini Parity

Wire format matches retro_rl@66836f5. **Cannot record Mini goldens yet.**

### What Works ✅

- **Compilation**: -Wall -Werror (DeriveAnyClass, directory dep, exports)
- **Wire format**: FrameInput 12-bit Word, SimState 18 keys, Trajectory.to_dict() order
- **Type safety**: Newtypes (Pixel, Subpixel, Position, Velocity)
- **Flat floor collision**: on_ground transitions (Y >= cfgGroundY)
- **Model fixes**: B-release decel, apex→falling, jumpSquatDuration in config

### Mini Baseline Gaps 🚫

**Cannot record goldens until**:
1. **Signed velocity**: Left movement needs -X (currently magnitude only)
2. **Momentum tracking**: momentum_x/momentum_x_sub not in SamusState
3. **Speed tracking**: speed_counter/speed_flag not tracked
4. **Shinespark**: shinespark_timer not tracked
5. **Frame counter**: frame not in SamusState
6. **Room ID**: room_id not tracked

### Architecture (Correct)

**Layer 1**: MiniStep baseline (fast iteration, simplified, NOT TAS-correct)  
**Layer 2**: Emulator acceptance (snes9x/libretro via SMEDIT/retro_rl, ground truth)

If Mini ≠ emu, **emulator wins**.

### Test Status

- ✅ Unit tests: Pass (accel, decel, squat, gravity)
- ✅ Properties: Determinism verified
- ❌ Goldens: NONE (Mini gaps block recording)

### Build

```bash
cd physics-hs
cabal build   # -Wall -Werror clean
cabal test    # Unit + determinism pass, no goldens
```

**DO NOT CLAIM MINI PARITY**. Signed velocity + momentum required.

**Draft until Mini gaps filled**.
