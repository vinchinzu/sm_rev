# Signed Velocity - Implementation Status

## Core Design Complete ✅

**Velocity type changed** to signed Int16 pixel:
```haskell
data Velocity = Velocity
  { velPixel :: !Int16      -- SIGNED (-32768 to +32767)
  , velSubpixel :: !Subpixel -- Unsigned (0x0000 to 0xFFFF)
  }
```

**Jump velocities negative** (upward in SM coords where Y grows down):
- `cfgJumpInitialSpeed`: `Velocity (-5) (Subpixel 0x8000)` = -4.5 pixels/frame
- Negative Y velocity makes Y decrease (move up)
- Positive Y velocity makes Y increase (move down)

**applyVelocity direction-aware**:
- Negative velocity: subtracts from position
- Positive velocity: adds to position

## Build Incomplete 🚧

Remaining type errors in:
1. FFI.hs: toJSON instance conflict
2. Gravity.hs: old Pixel/unPixel code
3. Run.hs: velocity helpers

## Engineering Bar

**DO NOT RECORD HOP JSON** until:
1. Build compiles (`cabal build`)
2. Test passes proving Y decreases: from groundY=200, after jump squat, Y pixel < 200
3. Super Metroid coords: Y grows downward, so hop makes samus_y SMALLER

**Ground-run JSON only** until rise proven.

## Next: Complete Build

Fix remaining type errors, then prove rise with unit test.
