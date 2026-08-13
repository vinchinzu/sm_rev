-- | Gravity and falling logic with landing detection.
--
-- Ported from src/samus_motion.c (Samus_MoveY_WithSpeedCalc, Samus_DetermineAccel_Y).
module Physics.SM.Gravity
  ( applyGravity
  , updateVerticalMovement
  , checkLanding
  , subVelocity  -- Exported for tests
  ) where

import Data.Bits ((.&.))
import Data.Word (Word32)
import Physics.SM.Constants
import Physics.SM.Types

-- | Update vertical position and velocity (gravity, jump release, terminal velocity, landing).
--
-- Corresponds to Samus_MoveY_WithSpeedCalc and Samus_CheckStartFalling.
updateVerticalMovement :: PhysicsConfig -> ControllerInput -> SamusState -> SamusState
updateVerticalMovement cfg input state
  | stateOnGround state = state  -- No vertical movement on ground
  | otherwise =
      let state' = checkJumpRelease input state
          state'' = applyGravity cfg EnvAir state'
          -- Apply Y velocity with direction awareness (rising=subtract, falling=add)
          newPos = applyVelocityY (stateYPos state'') (stateYVel state'') (stateVerticalDir state'')
          state''' = state'' { stateYPos = newPos }
      in checkLanding cfg state'''

-- | Check if Samus has landed on ground (v1: flat floor at cfgGroundY).
checkLanding :: PhysicsConfig -> SamusState -> SamusState
checkLanding cfg state
  | stateOnGround state = state  -- Already on ground
  | posPixel (stateYPos state) >= cfgGroundY cfg =
      -- Landed: snap to ground, zero Y velocity, set on_ground
      state { stateYPos = Position (cfgGroundY cfg) (Subpixel 0)
            , stateYVel = zeroVelocity
            , stateOnGround = True
            , stateVerticalDir = VDirStationary
            }
  | otherwise = state

-- | Check if jump button released early (cut jump short).
--
-- Matches the `samus_y_dir == kSamusYDir_Rising && button not held` logic.
checkJumpRelease :: ControllerInput -> SamusState -> SamusState
checkJumpRelease input state
  | stateVerticalDir state == VDirRising && not jumpHeld =
      state { stateYVel = zeroVelocity
            , stateVerticalDir = VDirFalling
            , stateJumpHeld = False
            }
  | otherwise = state
  where
    jumpHeld = (inputButtons input .&. btnA) /= ButtonMask 0

-- | Apply gravity acceleration to vertical velocity.
--
-- Corresponds to Samus_DetermineAccel_Y and the terminal velocity check.
applyGravity :: PhysicsConfig -> Environment -> SamusState -> SamusState
applyGravity cfg env state
  | stateVerticalDir state == VDirRising =
      -- Rising: subtract gravity (decelerate upward velocity)
      let gravAccel = selectGravity cfg env
          newVel = subVelocity (stateYVel state) gravAccel
          -- Transition to falling when velocity reaches zero
      in if velPixel newVel == Pixel 0 && velSubpixel newVel <= Subpixel 0x1000
         then state { stateYVel = zeroVelocity, stateVerticalDir = VDirFalling }
         else state { stateYVel = newVel }
  | stateVerticalDir state == VDirFalling =
      -- Falling: add gravity (increase downward velocity)
      let gravAccel = selectGravity cfg env
          currentVel = stateYVel state
          terminalSpeed = cfgTerminalSpeed cfg
      in if velPixel currentVel >= terminalSpeed
         then state  -- Capped at terminal velocity
         else state { stateYVel = addVelocity currentVel gravAccel }
  | otherwise = state

-- | Select gravity based on environment.
selectGravity :: PhysicsConfig -> Environment -> Velocity
selectGravity cfg env =
  let envIdx = case env of
                 EnvAir -> 0
                 EnvWater -> 1
                 EnvLavaAcid -> 2
  in (cfgGravityAccel cfg) !! envIdx

-- | Subtract two velocities (v1 - v2) with borrow.
--
-- Returns zero (not negative wrap) if result would be negative.
subVelocity :: Velocity -> Velocity -> Velocity
subVelocity (Velocity p1 s1) (Velocity p2 s2) =
  let total1 = fromIntegral (unPixel p1) * 65536 + fromIntegral (unSubpixel s1) :: Word32
      total2 = fromIntegral (unPixel p2) * 65536 + fromIntegral (unSubpixel s2) :: Word32
      result = if total1 >= total2 then total1 - total2 else 0
      newPix = Pixel (fromIntegral (result `div` 65536))
      newSub = Subpixel (fromIntegral (result `mod` 65536))
  in Velocity newPix newSub
