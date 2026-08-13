-- | Horizontal movement (run/walk) implementation.
--
-- Ported from src/samus_speed.c and src/samus_motion.c.
module Physics.SM.Run
  ( updateHorizontalMovement
  , applyRunAcceleration
  ) where

import Data.Bits ((.&.))
import Physics.SM.Constants
import Physics.SM.Types

-- | Update horizontal position and velocity based on input.
--
-- Corresponds to Samus_HandleMovement_X and Samus_CalcBaseSpeed_X.
updateHorizontalMovement :: PhysicsConfig -> ControllerInput -> SamusState -> SamusState
updateHorizontalMovement cfg input state
  | stateOnGround state =
      let (newVel, newAccelMode) = applyRunAcceleration cfg input (stateXVel state) (stateAccelMode state)
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
applyRunAcceleration :: PhysicsConfig -> ControllerInput -> Velocity -> AccelMode -> (Velocity, AccelMode)
applyRunAcceleration cfg input currentVel accelMode =
  let buttons = inputButtons input
      runHeld = (buttons .&. btnB) /= ButtonMask 0
      leftHeld = (buttons .&. btnLeft) /= ButtonMask 0
      rightHeld = (buttons .&. btnRight) /= ButtonMask 0

      -- Determine acceleration direction
      (newVel, newMode)
        | not runHeld = (zeroVelocity, AccelNone)  -- Decel when B not held
        | rightHeld = accelerateRight cfg currentVel
        | leftHeld = accelerateLeft cfg currentVel
        | otherwise = (currentVel, AccelNone)  -- No directional input

  in (newVel, newMode)

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
-- C uses separate left/right tables but same accel values. We mirror the right logic.
-- Note: In C, leftward velocity is stored as magnitude in the same unsigned fields,
-- with direction tracked by pose_x_dir. For this pure model, we use the same accel
-- but would need pose tracking to determine actual screen direction.
accelerateLeft :: PhysicsConfig -> Velocity -> (Velocity, AccelMode)
accelerateLeft cfg currentVel =
  let maxSpeed = cfgRunMaxSpeed cfg
      accel = cfgRunAccel cfg
      newVel = addVelocity currentVel accel
  in if velExceeds newVel maxSpeed
     then (maxSpeed, AccelNone)
     else (newVel, AccelAccelerating)

-- | Check if velocity exceeds maximum (unsigned comparison).
velExceeds :: Velocity -> Velocity -> Bool
velExceeds (Velocity vp vs) (Velocity mp ms) =
  vp > mp || (vp == mp && vs > ms)
