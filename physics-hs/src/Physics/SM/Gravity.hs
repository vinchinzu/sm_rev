-- | Gravity and falling logic.
--
-- Ported from src/samus_motion.c (Samus_MoveY_WithSpeedCalc, Samus_DetermineAccel_Y).
module Physics.SM.Gravity
  ( applyGravity
  , updateVerticalMovement
  ) where

import Data.Bits ((.&.))
import Physics.SM.Constants
import Physics.SM.Types

-- | Update vertical position and velocity (gravity, jump release, terminal velocity).
--
-- Corresponds to Samus_MoveY_WithSpeedCalc and Samus_CheckStartFalling.
updateVerticalMovement :: PhysicsConfig -> ControllerInput -> SamusState -> SamusState
updateVerticalMovement cfg input state
  | stateOnGround state = state  -- No vertical movement on ground
  | otherwise =
      let state' = checkJumpRelease input state
          state'' = applyGravity cfg EnvAir state'  -- TODO: detect env
          newPos = applyVelocity (stateYPos state'') (stateYVel state'')
      in state'' { stateYPos = newPos }

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
      in if velIsNegative newVel
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
subVelocity :: Velocity -> Velocity -> Velocity
subVelocity (Velocity p1 s1) (Velocity p2 s2) =
  let s1' = fromIntegral (unSubpixel s1) :: Word32
      s2' = fromIntegral (unSubpixel s2) :: Word32
      (borrow, newSub) = if s1' < s2'
                         then (Pixel 1, Subpixel (fromIntegral (65536 + s1' - s2')))
                         else (Pixel 0, Subpixel (fromIntegral (s1' - s2')))
      newPix = if p1 < p2 + borrow
               then Pixel 0  -- Clamp to zero (negative)
               else p1 - p2 - borrow
  in Velocity newPix newSub

-- | Check if velocity is effectively negative (would be < 0 if signed).
--
-- In unsigned representation, "negative" means the subtraction wrapped.
velIsNegative :: Velocity -> Bool
velIsNegative (Velocity (Pixel p) _) = p > 32768  -- High bit set = wrapped

import Data.Word (Word32)
