-- | Main physics step function.
--
-- Pose → jump → extra-run → X (table + extra) → Y (pre-gravity move).
-- Order matches the residual-relevant C path and the SMB takeoff lesson:
-- leave-ground happens before the same-frame air X step.
module Physics.SM.Step
  ( step
  , initialState
  ) where

import Physics.SM.Constants
import Physics.SM.Gravity
import Physics.SM.Jump
import Physics.SM.Momentum
import Physics.SM.Pose
import Physics.SM.Run
import Physics.SM.Types

-- | Step one frame: (State, Input) -> State.
step :: PhysicsConfig -> ControllerInput -> SamusState -> SamusState
step cfg input state =
  let posed     = updateGroundPose input state
      jumped    = handleJumpInput cfg input posed
      momentumed = tickExtraRun cfg input jumped
      movedX    = updateHorizontalMovement cfg input momentumed
      leftGround = stateOnGround posed && not (stateOnGround jumped)
      movedY    = updateVerticalMovement cfg input leftGround movedX
  in movedY
       { stateFrame = stateFrame state + 1
       , statePrevButtons = inputButtons input
       }

-- | Create initial standing state.
initialState :: PhysicsConfig -> SamusState
initialState cfg = SamusState
  { stateXPos = Position (Pixel 100) (Subpixel 0)
  , stateYPos = Position (cfgGroundY cfg) (Subpixel 0)
  , stateXVel = zeroVelocity
  , stateYVel = zeroVelocity
  , stateXExtra = zeroVelocity
  , stateHasMomentum = False
  , stateSpeedBoostCounter = 0
  , statePose = poseStandRight
  , stateMovementType = mvtStanding
  , stateVerticalDir = VDirStationary
  , stateAccelMode = AccelNone
  , stateOnGround = True
  , stateFacing = faceRight
  , stateFrame = 0
  , statePrevButtons = ButtonMask 0
  , stateJumpHeld = False
  , stateJumpSquatFrames = 0
  , stateEnvironment = EnvAir
  , stateEquipment = defaultEquipment
  }
