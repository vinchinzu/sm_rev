# Known Issues - Y Velocity Representation

## CRITICAL: Y Velocity Must Be Signed

**Current workaround**: `applyVelocityY` uses direction flag.  
**Proper fix required**: Velocity type needs signed representation.

### Problem

C code uses **signed** velocity:
- Negative Y velocity = upward movement (Y decreases)
- Positive Y velocity = downward movement (Y increases)

Haskell currently uses **unsigned** `Velocity`:
```haskell
data Velocity = Velocity
  { velPixel :: !Pixel       -- Word16 (unsigned)
  , velSubpixel :: !Subpixel -- Word16 (unsigned)
  }
```

### Current Workaround (Commit 2ee9f8a)

`applyVelocityY` uses `VerticalDirection` flag:
```haskell
applyVelocityY :: Position -> Velocity -> VerticalDirection -> Position
applyVelocityY pos vel dir = case dir of
  VDirRising -> subPosition pos (Position (velPixel vel) (velSubpixel vel))
  VDirFalling -> addPosition pos (Position (velPixel vel) (velSubpixel vel))
```

**Limitation**: Magnitude-only. Cannot represent leftward X velocity or complex Y motion.

### Proper Fix Required

Option 1: Signed Velocity type
```haskell
data SignedVelocity = SignedVelocity
  { velPixel :: !Int16      -- Signed
  , velSubpixel :: !Word16  -- Unsigned subpixel
  }
```

Option 2: Direction + magnitude (current approach, but formalize it)

Option 3: Match C's 16.16 signed fixed-point exactly

### Impact

**Works now**:
- Ground run (unsigned X magnitude OK)
- Simple hops (rise/fall with direction flag)

**Needs signed velocity**:
- Left movement (negative X)
- Momentum tracking (signed)
- Full Mini baseline parity

### Status

- **Hop goldens**: DELETED (won't record until rise verified)
- **Run golden**: EXISTS (run_right.json, X magnitude only)
- **1-tile platform**: Documented gap
- **Mini parity**: Blocked on signed velocity

DO NOT claim hop parity until signed velocity implemented and verified.
