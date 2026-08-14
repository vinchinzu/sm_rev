-- | Narrow residual-relevant pose / movement-type machine.
--
-- Stand, walk/run, crouch, morph, and facing. Jump squat / takeoff live
-- in Physics.SM.Jump so the leave-ground frame can switch tables first.
module Physics.SM.Pose
  ( updateGroundPose
  , poseForLanding
  , facingFromDir
  ) where

import Data.Word (Word16)
import Physics.SM.Constants
import Physics.SM.Types

facingFromDir :: Int -> Word16
facingFromDir dir
  | dir < 0   = faceLeft
  | otherwise = faceRight

-- | Grounded pose from held buttons. Airborne poses are owned by Jump.
updateGroundPose :: ControllerInput -> SamusState -> SamusState
updateGroundPose input state
  | not (stateOnGround state) = state
  | stateJumpSquatFrames state > 0 = state
  | otherwise =
      let dir = xDirection input
          down = buttonDown btnDown input
          facing' = if dir == 0 then stateFacing state else facingFromDir dir
          right = isFacingRight facing'
          morph = isMorph (stateMovementType state)
      in if morph
            then updateMorph input state facing' right down
            else updateUnmorphed state facing' right down dir

updateUnmorphed
  :: SamusState
  -> Word16
  -> Bool
  -> Bool
  -> Int
  -> SamusState
updateUnmorphed state facing' right down dir
  | down && isCrouch (stateMovementType state) && equipMorph (stateEquipment state) =
      state
        { stateFacing = facing'
        , stateMovementType = mvtMorphBallGround
        , statePose = if right then poseMorphRight else poseMorphLeft
        , stateXVel = zeroVelocity
        }
  | down =
      state
        { stateFacing = facing'
        , stateMovementType = mvtCrouching
        , statePose = if right then poseCrouchRight else poseCrouchLeft
        }
  | dir /= 0 =
      state
        { stateFacing = facing'
        , stateMovementType = mvtRunning
        , statePose = if right then poseRunRight else poseRunLeft
        }
  | otherwise =
      state
        { stateFacing = facing'
        , stateMovementType = mvtStanding
        , statePose = if right then poseStandRight else poseStandLeft
        }

updateMorph
  :: ControllerInput
  -> SamusState
  -> Word16
  -> Bool
  -> Bool
  -> SamusState
updateMorph input state facing' right down
  | justPressed btnUp input || (not down && justPressed btnA input) =
      state
        { stateFacing = facing'
        , stateMovementType = mvtStanding
        , statePose = if right then poseStandRight else poseStandLeft
        }
  | xDirection input /= 0 =
      state
        { stateFacing = facing'
        , stateMovementType = mvtMorphBallGround
        , statePose = if right then poseMorphRight else poseMorphLeft
        }
  | otherwise =
      state
        { stateFacing = facing'
        , stateMovementType = mvtMorphBallGround
        , statePose = if right then poseMorphRight else poseMorphLeft
        }

-- | After a flat-floor land: keep run pose if leftover X speed or extra run.
poseForLanding :: SamusState -> SamusState
poseForLanding state =
  let right = isFacingRight (stateFacing state)
      moving = stateXVel state /= zeroVelocity
            || stateXExtra state /= zeroVelocity
      morph = isMorph (stateMovementType state)
                || isMorphFalling (stateMovementType state)
  in if morph
        then state
               { stateOnGround = True
               , stateMovementType = mvtMorphBallGround
               , statePose = if right then poseMorphRight else poseMorphLeft
               , stateVerticalDir = VDirStationary
               }
        else if moving
        then state
               { stateOnGround = True
               , stateMovementType = mvtRunning
               , statePose = if right then poseRunRight else poseRunLeft
               , stateVerticalDir = VDirStationary
               }
        else state
               { stateOnGround = True
               , stateMovementType = mvtStanding
               , statePose = if right then poseStandRight else poseStandLeft
               , stateVerticalDir = VDirStationary
               }

isCrouch :: MovementType -> Bool
isCrouch mt = mt == mvtCrouching

isMorph :: MovementType -> Bool
isMorph mt = mt == mvtMorphBallGround

isMorphFalling :: MovementType -> Bool
isMorphFalling mt = mt == mvtMorphBallFalling
