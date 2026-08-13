# Honest README: Mini Baseline Gaps

## Status: Cannot Record Goldens Yet

**Mini baseline gaps** prevent recording valid tapes.

### What's Missing

1. **Signed velocity**: Left movement needs -X (currently magnitude only)
2. **Momentum tracking**: momentum_x/momentum_x_sub not in SamusState
3. **Speed tracking**: speed_counter/speed_flag not tracked
4. **Shinespark**: shinespark_timer not tracked
5. **Frame counter**: frame not in SamusState
6. **Room ID**: room_id not tracked

### Wire Format Ready ✅

**FrameInput**: `{"buttons": int}` 12-bit (B=0x001 ... R=0x800)
**SimState**: 18 required keys (all fields defined, signed i16 for velocity/momentum)
**Trajectory**: Keys start, frames, predictor, inputs

### When Goldens Can Be Recorded

After implementing:
1. Full SimState fields (momentum, speed, shinespark, frame, room)
2. Signed velocity (for leftward movement)
3. MiniStep integration (subprocess or FFI)

Then record 3 tapes: run, short hop, full hop (vs Mini baseline).

**Acceptance**: Emulator is ground truth. If Mini ≠ emu, emu wins.

**Draft until Mini baseline gaps filled.**
