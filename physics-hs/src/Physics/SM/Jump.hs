-- | Jump initialization and jump-squat handling.
--
-- Ported from src/samus_jump.c (Samus_InitJump, HandleJumpTransition_*).
module Physics.SM.Jump
  ( handleJumpInput
  , initJump
  ) where

import Data.Bits ((.&.))
import Physics.SM.Constants
import Physics.SM.Types

-- | Handle jump input: squat detection and jump initialization.
--
-- Matches HandleJumpTransition_NormalJump logic: jump fires when squat REACHES duration.
handleJumpInput :: PhysicsConfig -> ControllerInput -> SamusState -> SamusState
handleJumpInput cfg input state
  | not (stateOnGround state) = state  -- Can't jump in air
  | justPressed btnA input && stateJumpSquatFrames state == 0 =
      -- Start jump squat
      state { stateJumpSquatFrames = 1, stateJumpHeld = True }
  | stateJumpSquatFrames state > 0 =
      -- Continue squat and check if duration reached
      let aHeld = (inputButtons input .&. btnA) /= ButtonMask 0
          newSquat = stateJumpSquatFrames state + 1
      in if not aHeld
         then state { stateJumpSquatFrames = 0, stateJumpHeld = False }  -- Cancel squat
         else if newSquat >= cfgJumpSquatDuration cfg
              then initJump cfg EnvAir False state  -- Fire jump when duration reached
              else state { stateJumpSquatFrames = newSquat }  -- Continue squat
  | otherwise = state

-- | Initialize jump: set upward velocity, change pose, leave ground.
--
-- Corresponds to Samus_InitJump from samus_jump.c.
initJump :: PhysicsConfig -> Environment -> Bool -> SamusState -> SamusState
initJump cfg env hasHiJump state =
  let jumpVel = selectJumpVel cfg env hasHiJump
  in state
       { stateYVel = jumpVel
       , stateVerticalDir = VDirRising
       , stateOnGround = False
       , stateJumpSquatFrames = 0
       , stateJumpHeld = True
       , statePose = if statePose state == poseStandRight then poseJumpRight else poseJumpLeft
       , stateMovementType = mvtNormalJumping
       }

-- | Select jump velocity based on environment and equipment.
selectJumpVel :: PhysicsConfig -> Environment -> Bool -> Velocity
selectJumpVel cfg env hasHiJump =
  let envIdx = envToIndex env
      table = if hasHiJump then cfgJumpHiInitialSpeed cfg else cfgJumpInitialSpeed cfg
  in table !! envIdx

envToIndex :: Environment -> Int
envToIndex EnvAir = 0
envToIndex EnvWater = 1
envToIndex EnvLavaAcid = 2

-- | Check if a button was just pressed this frame.
justPressed :: ButtonMask -> ControllerInput -> Bool
justPressed btn input =
  let curr = inputButtons input
      prev = inputPrevButtons input
  in (curr .&. btn) /= ButtonMask 0 && (prev .&. btn) == ButtonMask 0
