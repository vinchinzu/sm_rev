# Known Issues and Gaps

## Collision

**Status**: Minimal flat floor only.

**Current**: `checkLanding` detects Y >= cfgGroundY (200), sets on_ground=true.

**Gaps**:
- No slopes
- No platforms (1-tile or otherwise)
- No walls
- No ceiling collision

**Impact**: Hops work (rise + fall + land on flat floor), but no level geometry.

## Horizontal Movement

**Status**: Run acceleration works for rightward motion.

**Gap**: `accelerateLeft` currently adds positive X velocity (same as `accelerateLeft`).

**Root cause**: C implementation uses unsigned velocity magnitude with separate `pose_x_dir` flag. Full leftward movement requires:
1. Pose/facing field to track direction
2. Position update to apply signed velocity
3. Collision to prevent moving through walls

**Workaround**: Both `accelerateLeft` and `accelerateRight` use identical magnitude logic. This matches C's magnitude-based acceleration but doesn't produce screen-left motion yet.

**Impact**: B+Left input accelerates but doesn't move Samus leftward on screen. Rightward motion (B+Right) works correctly.

## Air Control

**Status**: In-air horizontal velocity is maintained but not modified.

**Current**: When airborne, `updateHorizontalMovement` applies current X velocity to X position but skips acceleration logic.

**Gap**: No in-air acceleration or deceleration (requires collision detection to determine "on ground" vs "in air" reliably).

**Impact**: After jumping, X velocity freezes at jump-time value. Full air control needs collision layer.

## Equipment / Modes

**Status**: Stub fields exist but are not wired.

**Gaps**:
- Hi-Jump / Space Jump (jump velocity selection works for EnvAir, but equipment flags aren't checked)
- Morph Ball (no alternate hitbox or morph-specific movement)
- Spin Jump (no spin state tracking)
- Speed Booster / Shinespark (no run timer or stored momentum)
- Gravity Suit (environment detection exists, underwater drag not tested)

**Impact**: Physics model assumes standing Samus in standard suit.

## Environment

**Status**: `Environment` enum exists (Air, Water, LavaAcid) with separate gravity/jump tables.

**Gap**: Only `EnvAir` tested. Underwater and lava/acid drag not verified.

## Oracle / Baseline

**Status**: No C MiniStep integration.

**Current**: Haskell tests verify determinism (same inputs → same states) and physics behavior (HopRise: Y decreases after jump).

**Gap**: No recorded Mini baseline goldens. `run_right.json` is Haskell self-output ("predictor": "haskell-v1"), not MiniStep.

**Impact**: Iteration is Haskell-only. No fast comparison to C baseline. Acceptance must go directly to emulator (snes9x/libretro) when that layer is built.

## Emulator Acceptance

**Status**: Not implemented.

**Future work**: Validate Haskell step function against real SNES emulator (snes9x/libretro via SMEDIT bridge + retro_rl stable-retro, reading WRAM $0AF6/$0AFA + $0AF8/$0AFC).

**Emulator is TAS ground truth**. Mini is fast iteration baseline. If Mini ≠ emu, emu wins.

## Testing

**Status**: `cabal test` passes (15/15 tests) in local dev environment.

**Gap**: No CI running `cabal test` yet. Do not claim "CI green" until CI actually runs Haskell tests.

**Current tests**:
- Unit: acceleration, friction, jump squat (4-frame timing), jump velocity, gravity, terminal velocity
- Properties: deterministic replay, rightward motion accumulates
- Golden: `run_right.json` exists
- HopRise: Y decreases after jump, Y velocity starts negative

## What's Proven

✅ Jump squat 4-frame timing  
✅ Jump fires with negative Y velocity (-4.5 pixels/frame)  
✅ Y decreases after jump (rise mechanics)  
✅ Gravity decelerates upward velocity  
✅ Gravity accelerates downward velocity  
✅ Terminal velocity clamps at 5.0 pixels/frame  
✅ B+Right accelerates to max speed (3.0 pixels/frame)  
✅ Signed 16.16 velocity arithmetic (Int16 pixel + Word16 subpixel)  
✅ Flat floor landing detection  

## What's Stubbed

❌ Collision beyond flat floor  
❌ Leftward screen motion (accelerateLeft adds +X)  
❌ Air control  
❌ Equipment / alternate modes  
❌ Underwater / lava drag (not tested)  
❌ Mini baseline parity  
❌ Emulator acceptance  
❌ CI test execution  
