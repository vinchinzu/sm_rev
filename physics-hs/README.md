# Haskell Physics Kernel - WORK IN PROGRESS

## Status: NOT READY FOR MINISTEP PARITY

This package is an **early skeleton** with major gaps. It does NOT bisimulate MiniStep yet.

### What Compiles

- Type definitions (Position, Velocity, SamusState)
- Named constants (button masks, poses)
- Basic arithmetic (16.16 fixed-point add/sub)
- Module structure

### Critical Model Gaps (Prevent MiniStep Parity)

1. **No collision detection**
   - `stateOnGround` never becomes `false` after jump
   - Cannot detect landing → no landing frame golden
   - Cannot detect platform edges → no 1-tile platform golden

2. **accelerateLeft is wrong**
   - Currently: unsigned +X (copy of accelerateRight)
   - Should be: -X (leftward movement)

3. **Apex hang bug**
   - `subVelocity` clamps to 0 when negative
   - Full hop with A held never enters `VDirFalling`
   - Rising velocity doesn't transition smoothly to falling

4. **jumpSquatDuration magic number**
   - Hardcoded `4` in `Jump.hs`
   - Should be in `PhysicsConfig`
   - Unit test "Jump squat lasts 4 frames" disagrees with `handleJumpInput`

5. **B-release zeros X velocity**
   - `applyRunAcceleration`: `not runHeld = (zeroVelocity, AccelNone)`
   - Should use `cfgRunDecel` (currently unused)
   - Pose almost never updated during run

6. **Wire format**
   - `btnLeft=0x0200 btnRight=0x0100` (internal 16-bit)
   - Locked wire is `Left=0x40 Right=0x80` (packed 8-bit)
   - Need adapter or type alignment

### What's Missing

- Collision detection (ground, walls, ceiling, platforms)
- Leftward movement (accelerateLeft is broken)
- Smooth apex transition (falling after peak)
- Deceleration (B-release handling)
- Pose updates (run animation)
- Short hop vs full hop (peak Y differs)
- Landing detection (on_ground transition)

### Testing

- **Unit tests**: Pass (but test magic numbers, not real behavior)
- **Property tests**: FAKE (Haskell-vs-Haskell, not vs MiniStep)
- **Golden tests**: NO GOLDENS (removed fake JSON)

### Cannot Claim Until

1. Collision detection implemented (stateOnGround transitions)
2. accelerateLeft fixed (-X movement)
3. Apex transition fixed (VDirFalling after peak)
4. jumpSquatDuration moved to PhysicsConfig
5. Deceleration implemented (cfgRunDecel)
6. Pose updates during run
7. Real MiniStep goldens recorded (3 tapes: run, short/full hop, 1-tile platform)
8. Wire format aligned (0x40/0x80 packed)

### Architecture (Correct)

**Layer 1: MiniStep Baseline** (fast iteration, simplified model, NOT TAS-correct)  
**Layer 2: Emulator Acceptance** (snes9x/libretro ground truth)

If Mini ≠ emu, emulator wins.

### Build

```bash
cd physics-hs
cabal build   # Compiles with -Wall -Werror (imports/unused fixed)
cabal test    # Unit tests pass (but don't prove MiniStep parity)
```

**DO NOT CLAIM MINISTEP PARITY**. This is an early skeleton with major bugs.
