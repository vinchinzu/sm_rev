-- | Jump initialization and jump-squat handling.
--
-- Ported from src/samus_jump.c (Samus_InitJump, HandleJumpTransition_*).
-- Takeoff switches movement type before the same-frame X step so air
-- tables apply on the leave-ground frame (SMB takeoff residual).
module Physics.SM.Jump
  ( handleJumpInput
  , initJump
  ) where

import Data.Bits (shiftR)
import Data.Int (Int32)
import Data.Word (Word16)
import Physics.SM.Constants
import Physics.SM.Types

-- | Handle jump input: squat detection and jump initialization.
handleJumpInput :: PhysicsConfig -> ControllerInput -> SamusState -> SamusState
handleJumpInput cfg input state
  | not (stateOnGround state) = state
  | justPressed btnA input && stateJumpSquatFrames state == 0 =
      state
        { stateJumpSquatFrames = 1
        , stateJumpHeld = True
        , statePose = if isFacingRight (stateFacing state)
                         then poseJumpTransRight
                         else poseJumpTransLeft
        }
  | stateJumpSquatFrames state > 0 =
      let newSquat = stateJumpSquatFrames state + 1
      in if newSquat >= cfgJumpSquatDuration cfg
            then initJump cfg state
            else state { stateJumpSquatFrames = newSquat
                       , stateJumpHeld = buttonDown btnA input }
  | otherwise = state

-- | Initialize jump: set upward velocity, change pose, leave ground.
initJump :: PhysicsConfig -> SamusState -> SamusState
initJump cfg state =
  let env = stateEnvironment state
      hasHi = equipHiJump (stateEquipment state)
      jumpVel = selectJumpVel cfg env hasHi
      boosted = if equipSpeedBooster (stateEquipment state)
                   then addSpeedBoosterJumpMomentum state jumpVel
                   else jumpVel
      dir = if stateXVel state /= zeroVelocity
               then signOf (stateXVel state)
               else if isFacingRight (stateFacing state) then 1 else -1
      spinning = stateMovementType state == mvtRunning
                   || stateXVel state /= zeroVelocity
                   || stateXExtra state /= zeroVelocity
      crouched = stateMovementType state == mvtCrouching
      yPos = if crouched
                then subPosition (stateYPos state)
                       (Position (cfgCrouchJumpYOffset cfg) (Subpixel 0))
                else stateYPos state
      right = isFacingRight (stateFacing state)
  in state
       { stateYPos = yPos
       , stateYVel = boosted
       , stateVerticalDir = VDirRising
       , stateOnGround = False
       , stateJumpSquatFrames = 0
       , stateJumpHeld = True
       , statePose = jumpPose spinning right
       , stateMovementType = if spinning then mvtSpinJumping else mvtNormalJumping
       , stateFacing = if dir < 0 then faceLeft else faceRight
       }

jumpPose :: Bool -> Bool -> SamusPose
jumpPose spinning right
  | spinning && right = poseSpinJumpRight
  | spinning          = poseSpinJumpLeft
  | right             = poseJumpRight
  | otherwise         = poseJumpLeft

selectJumpVel :: PhysicsConfig -> Environment -> Bool -> Velocity
selectJumpVel cfg env hasHiJump =
  let table = if hasHiJump then cfgJumpHiInitialSpeed cfg else cfgJumpInitialSpeed cfg
  in selectEnv env table

-- C: y_sub += extra_sub (uint16 wrap, no carry); y_speed += extra_speed >> 1.
addSpeedBoosterJumpMomentum :: SamusState -> Velocity -> Velocity
addSpeedBoosterJumpMomentum state jumpVel =
  let mag = abs (toSigned1616 jumpVel)
      ySpeed = fromIntegral (mag `shiftR` 16) :: Word16
      ySub   = fromIntegral mag :: Word16
      extraS = fromIntegral (velPixel (stateXExtra state)) :: Word16
      extraSub = unSubpixel (velSubpixel (stateXExtra state))
      newSub = ySub + extraSub
      newS   = ySpeed + (extraS `shiftR` 1)
      unsigned = fromIntegral newS * 65536 + fromIntegral newSub :: Int32
  in negateVelocity (fromSigned1616 unsigned)

signOf :: Velocity -> Int
signOf v = if toSigned1616 v < 0 then -1 else 1
