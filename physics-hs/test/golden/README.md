# Golden Tapes Status

## Cannot Record Yet - Missing Collision

**Blocker**: No collision detection system.

### Why Goldens Don't Exist

MiniStep golden tapes require:

1. **Ground run RIGHT N frames**
   - ✅ Horizontal movement works
   - ✅ Acceleration/deceleration implemented
   - ⚠️ Can record partial (position drift, no landing check)

2. **Short hop vs full hop**
   - ❌ Requires A button (not in packed 8-bit yet)
   - ❌ Landing frame needs collision
   - ❌ Peak Y detection needs apex transition (NOW FIXED)

3. **1-tile platform**
   - ❌ Requires collision map (platform edges)
   - ❌ Landing detection (stateOnGround false→true)
   - ❌ Ground check per frame

### What's Needed

**Collision system**:
- Ground height detection (Y coordinate checks)
- Platform edge detection (solid block queries)
- `stateOnGround` transitions (leave ground on jump, land after fall)

**Wire format**:
- Align to 0x40/0x80 packed (currently 0x0200/0x0100)

**MiniStep integration**:
- Subprocess call to `sm_rev_mini_oracle --json`
- JSON serialization of states/inputs
- Recorded tapes in this directory

### Plan

1. Add basic collision (ground Y check)
2. Implement landing detection (stateOnGround transitions)
3. Record 3 MiniStep tapes (when collision works)
4. Check in JSON goldens for CI

**Draft until collision system exists.**
