-- | Gravity and falling logic with landing detection.
--
-- Ported from Samus_MoveY_WithSpeedCalc / Samus_CheckStartFalling.
-- C applies the *pre-gravity* Y speed to position, then updates speed
-- for the next frame. A-release and apex both snap to falling at 0
-- (not a 0x0100 clip). Landing keeps leftover Y subpixel.
module Physics.SM.Gravity
  ( applyGravity
  , updateVerticalMovement
  , checkLanding
  ) where

import Data.Int (Int16)
import Physics.SM.Constants
import Physics.SM.Pose (poseForLanding)
import Physics.SM.Types

-- | Update vertical position and velocity.
updateVerticalMovement :: PhysicsConfig -> ControllerInput -> SamusState -> SamusState
updateVerticalMovement cfg input state
  | stateOnGround state = state
  | otherwise =
      let released = applyJumpRelease input state
          moved = released { stateYPos = applyVelocity (stateYPos released) (stateYVel released) }
          accelerated = updateYSpeed cfg moved
      in checkLanding cfg accelerated

-- | A-release while rising: snap to 0 and start falling (C JumpingMovement).
applyJumpRelease :: ControllerInput -> SamusState -> SamusState
applyJumpRelease input state
  | stateVerticalDir state == VDirRising
      && not (buttonDown btnA input) =
      state
        { stateYVel = zeroVelocity
        , stateVerticalDir = VDirFalling
        , stateJumpHeld = False
        }
  | otherwise = state { stateJumpHeld = buttonDown btnA input }

-- | Gravity after the position step. Rising subtracts until underflow,
-- then snaps to falling at 0. Falling adds until terminal 5.0.
updateYSpeed :: PhysicsConfig -> SamusState -> SamusState
updateYSpeed cfg state =
  let gravity = selectGravity cfg (stateEnvironment state)
      term = fromIntegral (unPixel (cfgTerminalSpeed cfg)) :: Int16
  in case stateVerticalDir state of
       VDirRising ->
         let next = addVelocity (stateYVel state) gravity
         in if toSigned1616 next >= 0
               then state { stateYVel = zeroVelocity, stateVerticalDir = VDirFalling }
               else state { stateYVel = next }
       VDirFalling ->
         let vel = stateYVel state
             atCap = velPixel vel > term
                  || (velPixel vel == term && unSubpixel (velSubpixel vel) > 0)
             next = addVelocity vel gravity
         in if atCap || velPixel vel == term
               then state { stateYVel = Velocity term (Subpixel 0) }
               else if velPixel next > term
                       || (velPixel next == term && unSubpixel (velSubpixel next) > 0)
                       then state { stateYVel = Velocity term (Subpixel 0) }
                       else state { stateYVel = next }
       VDirStationary ->
         state { stateVerticalDir = VDirFalling, stateYVel = gravity }

-- | Land on the flat floor at cfgGroundY.
--
-- Pixel Y snaps to the floor. Y subpixel leftover is kept (SMB land
-- residual). X speed / extra run stay so a run-jump can land running.
checkLanding :: PhysicsConfig -> SamusState -> SamusState
checkLanding cfg state =
  let yPixel = unPixel (posPixel (stateYPos state))
      groundY = unPixel (cfgGroundY cfg)
  in if yPixel >= groundY
        then poseForLanding state
               { stateYPos = Position (Pixel groundY) (posSubpixel (stateYPos state))
               , stateYVel = zeroVelocity
               }
        else state

-- | Apply gravity to Y velocity (kept for unit tests that call it directly).
applyGravity :: PhysicsConfig -> Environment -> SamusState -> SamusState
applyGravity cfg env state =
  let gravity = selectGravity cfg env
  in state { stateYVel = addVelocity (stateYVel state) gravity }

selectGravity :: PhysicsConfig -> Environment -> Velocity
selectGravity cfg env =
  let envIdx = case env of
                 EnvAir -> 0
                 EnvWater -> 1
                 EnvLavaAcid -> 2
  in cfgGravityAccel cfg !! envIdx
