-- | Horizontal movement (run/walk) implementation.
--
-- Ported from src/samus_speed.c and src/samus_motion.c.
module Physics.SM.Run
  ( updateHorizontalMovement
  , applyRunAcceleration
  ) where

import Data.Bits ((.&.))
import Data.Int (Int16)
import Data.Word (Word32)
import Physics.SM.Constants
import Physics.SM.Types

-- | Update horizontal position and velocity based on input.
--
-- Corresponds to Samus_HandleMovement_X and Samus_CalcBaseSpeed_X.
updateHorizontalMovement :: PhysicsConfig -> ControllerInput -> SamusState -> SamusState
updateHorizontalMovement cfg input state
  | stateOnGround state =
      let (newVel, newAccelMode) = applyRunAcceleration cfg input (stateXVel state)
          newPos = applyVelocity (stateXPos state) newVel
      in state { stateXPos = newPos
               , stateXVel = newVel
               , stateAccelMode = newAccelMode
               }
  | otherwise =
      -- In air: maintain current velocity (no air accel for now, full air control needs collision)
      let newPos = applyVelocity (stateXPos state) (stateXVel state)
      in state { stateXPos = newPos }

-- | Apply run acceleration/deceleration based on held buttons.
--
-- Matches the physics_params run_accel/run_max_speed logic.
applyRunAcceleration :: PhysicsConfig -> ControllerInput -> Velocity -> (Velocity, AccelMode)
applyRunAcceleration cfg input currentVel =
  let buttons = inputButtons input
      runHeld = (buttons .&. btnB) /= ButtonMask 0
      leftHeld = (buttons .&. btnLeft) /= ButtonMask 0
      rightHeld = (buttons .&. btnRight) /= ButtonMask 0

      -- Determine acceleration direction
      (newVel, newMode)
        | not runHeld = applyDeceleration cfg currentVel  -- Use decel, not zero
        | rightHeld = accelerateRight cfg currentVel
        | leftHeld = accelerateLeft cfg currentVel
        | otherwise = (currentVel, AccelNone)  -- No directional input

  in (newVel, newMode)

-- | Apply deceleration when B button released.
applyDeceleration :: PhysicsConfig -> Velocity -> (Velocity, AccelMode)
applyDeceleration cfg currentVel =
  let decel = cfgRunDecel cfg
      currentMag = velToWord32 currentVel
      decelMag = velToWord32 decel
  in if currentMag <= decelMag
     then (zeroVelocity, AccelNone)  -- Stop if decel would overshoot
     else (subVelocitySafe currentVel decel, AccelDecelerating)

-- | Subtract velocity safely for deceleration (magnitude-based).
subVelocitySafe :: Velocity -> Velocity -> Velocity
subVelocitySafe (Velocity p1 s1) (Velocity p2 s2) =
  let total1 = abs (fromIntegral p1) * 65536 + fromIntegral (unSubpixel s1) :: Word32
      total2 = abs (fromIntegral p2) * 65536 + fromIntegral (unSubpixel s2) :: Word32
      result = if total1 >= total2 then total1 - total2 else 0
      newPix = fromIntegral (result `div` 65536) :: Int16
      newSub = Subpixel (fromIntegral (result `mod` 65536))
  in Velocity newPix newSub

velToWord32 :: Velocity -> Word32
velToWord32 (Velocity p s) =
  abs (fromIntegral p) * 65536 + fromIntegral (unSubpixel s)

-- | Accelerate rightward (positive X).
accelerateRight :: PhysicsConfig -> Velocity -> (Velocity, AccelMode)
accelerateRight cfg currentVel =
  let maxSpeed = cfgRunMaxSpeed cfg
      accel = cfgRunAccel cfg
      newVel = addVelocity currentVel accel
  in if velExceeds newVel maxSpeed
     then (maxSpeed, AccelNone)  -- Capped at max
     else (newVel, AccelAccelerating)

-- | Accelerate leftward (negative X).
--
-- Subtracts acceleration to produce negative X velocity.
accelerateLeft :: PhysicsConfig -> Velocity -> (Velocity, AccelMode)
accelerateLeft cfg currentVel =
  let maxSpeed = cfgRunMaxSpeed cfg  -- Velocity 3 (Subpixel 0)
      accel = cfgRunAccel cfg          -- Velocity 0 (Subpixel 0x00a0)
      -- Subtract acceleration (makes velocity more negative)
      newVel = subVelocity currentVel accel
      -- Max leftward speed is -3.0 (negative max)
      maxNegPixel = negate (velPixel maxSpeed)
  in if velPixel newVel < maxNegPixel
     then (Velocity maxNegPixel (Subpixel 0), AccelNone)
     else (newVel, AccelAccelerating)

-- | Check if velocity exceeds maximum (unsigned comparison).
velExceeds :: Velocity -> Velocity -> Bool
velExceeds (Velocity vp vs) (Velocity mp ms) =
  vp > mp || (vp == mp && vs > ms)
