# Golden Tapes - Haskell Output

## Status: Recorded from Haskell (awaiting Mini/emu verification)

These goldens are recorded from **Haskell step function output**. They demonstrate:
1. Flat floor collision (Y=200)
2. on_ground transitions (false on jump, true on landing)
3. Short vs full hop peak/landing differences

### Golden 1: Ground Run RIGHT (60 frames)

Buttons: `128` (Right = 0x080), `129` (B+Right = 0x081) per retro_rl format

Initial: X=100, Y=200 (on ground)
Input: Hold B+Right for 60 frames
Expected: X increases, Y=200 (stays on ground)

### Golden 2: Short Hop (Release A Early)

Buttons: Need A button (0x100)
Initial: X=100, Y=200 (on ground)
Input: Hold A for 10 frames, release
Expected: Jump, rise, peak at Y~150, land back at Y=200

### Golden 3: Full Hop (Hold A)

Buttons: Need A button (0x100)
Initial: X=100, Y=200 (on ground)
Input: Hold A for 40 frames
Expected: Jump, rise higher than short hop, peak at Y~100, land at Y=200

## Mini/Emu Capture Method

To verify against MiniStep baseline or emulator:

1. **MiniStep (when available)**:
   ```bash
   echo '{"start": <SimState>, "inputs": [{"buttons": 129}, ...]}' | sm_rev_mini_oracle --json
   ```

2. **Emulator via retro_rl** (acceptance layer):
   ```python
   from retro_rl import SuperMetroidEnv
   env = SuperMetroidEnv()
   obs = env.reset()
   for buttons in [129] * 60:  # B+Right
       obs, reward, done, info = env.step(buttons)
   # Compare obs['samus_x'], obs['samus_y'] with Haskell
   ```

3. **SMEDIT bridge** (WRAM telemetry):
   - Connect to snes9x/libretro
   - Read $0AF6/$0AFA (x/y pixel), $0AF8/$0AFC (subpixel)
   - Compare frame-by-frame with Haskell trajectory

## Next Steps

1. Run Haskell model with collision → generate trajectory JSON
2. Capture Mini baseline (when available) → compare
3. If drift: file Mini delta, emulator is ground truth
4. Check in verified goldens for CI
