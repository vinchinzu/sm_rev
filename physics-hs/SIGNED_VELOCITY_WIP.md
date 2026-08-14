# Signed Velocity Implementation - WIP

## Status: Done in the fragment. Do not reopen as a Haskell expansion track.

Signed 16.16 is implemented and unit-tested. Remaining work is H↔M wiring
and Mini–emulator residual, not more velocity architecture. See
[docs/physics_haskell.md](../docs/physics_haskell.md).

## Historical notes (build errors below were already fixed)

## Status when this note was written: Core architecture changed, build has remaining errors

### Completed ✅

1. **Velocity type**: Changed to signed Int16 pixel component
   ```haskell
   data Velocity = Velocity
     { velPixel :: !Int16      -- SIGNED (was Word16)
     , velSubpixel :: !Subpixel
     }
   ```

2. **Jump velocities**: Negative values for upward motion
   - `cfgJumpInitialSpeed`: Velocity (-5) (Subpixel 0x8000) = -4.5 pixels/frame (up)
   - Gravity adds positive (pulls down), reducing negative velocity

3. **applyVelocity**: Direction-aware application
   - Negative velocity: subtracts from position (moves up/left)
   - Positive velocity: adds to position (moves down/right)

4. **Test added**: Test.HopRise verifies peak Y < groundY

5. **GHC/cabal installed**: 9.4.8 / 3.10.2.1

### Remaining Build Errors 🚧

1. **FFI.hs**: `toJSON` instance conflicts with `deriving anyclass`
   - Fix: Remove manual `toJSON` instance or remove `deriving anyclass (ToJSON)`

2. **Gravity.hs line 96-101**: `subVelocity` uses old `unPixel`
   - Fix: Update to work with Int16 pixel component

3. **Run.hs**: Similar issues in velocity helpers

### Next Steps

1. Fix FFI ToJSON conflict
2. Update Gravity.hs subVelocity for Int16
3. Fix Run.hs helpers
4. Run `cabal test` to verify hop rise test passes

### Architecture Complete

The fundamental change from unsigned+direction to signed velocity is done.
Jumps will now properly move upward (Y decreases) without needing VDirRising enum.
