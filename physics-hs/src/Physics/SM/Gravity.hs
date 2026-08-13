-- | Gravity and falling logic with landing detection.
--
-- Ported from src/samus_motion.c (Samus_MoveY_WithSpeedCalc, Samus_DetermineAccel_Y).
module Physics.SM.Gravity
  ( applyGravity
  , updateVerticalMovement
  , checkLanding
  ) where

import Data.Bits ((.&.))
import Data.Int (Int16)
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
          state''' = clampTerminalVelocity cfg state''
          landed = checkLanding cfg state'''
      in landed

-- | Check for landing on flat floor at cfgGroundY.
--
-- If Samus Y >= cfgGroundY, set on_ground = True and Y = cfgGroundY.
checkLanding :: PhysicsConfig -> SamusState -> SamusState
checkLanding cfg state =
  let yPixel = unPixel (posPixel (stateYPos state))
      groundY = unPixel (cfgGroundY cfg)
  in if yPixel >= groundY
     then state
            { stateYPos = Position (Pixel groundY) (Subpixel 0)
            , stateOnGround = True
            , stateYVel = zeroVelocity
            }
     else state

-- | Release jump: if A not held, clip upward velocity to near-zero.
--
-- Ported from Samus_CheckJumpRelease ($90:926C).
checkJumpRelease :: ControllerInput -> SamusState -> SamusState
checkJumpRelease input state =
  let jumpHeld = (inputButtons input .&. btnA) /= ButtonMask 0
      vel = stateYVel state
      velPix = velPixel vel
  in if not jumpHeld && stateJumpHeld state && velPix < 0
     then state
            { stateYVel = Velocity 0 (Subpixel 0x0100)  -- Clip to ~1/256 pixels/frame
            , stateJumpHeld = False
            }
     else state { stateJumpHeld = jumpHeld }

-- | Apply gravity to Y velocity.
--
-- Ported from Samus_DetermineAccel_Y.
applyGravity :: PhysicsConfig -> Environment -> SamusState -> SamusState
applyGravity cfg env state =
  let gravity = selectGravity cfg env
      newVel = addVelocity (stateYVel state) gravity
  in state { stateYVel = newVel }

-- | Clamp falling velocity to terminal velocity (5 pixels/frame).
clampTerminalVelocity :: PhysicsConfig -> SamusState -> SamusState
clampTerminalVelocity _cfg state =
  let vel = stateYVel state
      velPix = velPixel vel
      termPix = 5 :: Int16  -- Terminal velocity: 5 pixels/frame downward
  in if velPix > termPix
     then state { stateYVel = Velocity termPix (Subpixel 0) }
     else state

-- | Select gravity based on environment.
selectGravity :: PhysicsConfig -> Environment -> Velocity
selectGravity cfg env =
  let envIdx = case env of
                 EnvAir -> 0
                 EnvWater -> 1
                 EnvLavaAcid -> 2
  in (cfgGravityAccel cfg) !! envIdx
