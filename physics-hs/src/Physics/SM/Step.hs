-- | Main physics step function.
--
-- Combines run, jump, and gravity into one frame update.
module Physics.SM.Step
  ( step
  , initialState
  ) where

import Physics.SM.Constants
import Physics.SM.Gravity
import Physics.SM.Jump
import Physics.SM.Run
import Physics.SM.Types

-- | Step one frame: (State, Input) -> State.
--
-- Pure function matching MiniStep semantics.
step :: PhysicsConfig -> ControllerInput -> SamusState -> SamusState
step cfg input state =
  let state' = handleJumpInput cfg input state
      state'' = updateHorizontalMovement cfg input state'
      state''' = updateVerticalMovement cfg input state''
  in state'''

-- | Create initial standing state.
--
-- Matches MiniGameState_Init defaults for a grounded Samus.
initialState :: PhysicsConfig -> SamusState
initialState cfg = SamusState
  { stateXPos = Position (Pixel 100) (Subpixel 0)
  , stateYPos = Position (cfgGroundY cfg) (Subpixel 0)  -- On ground at cfgGroundY
  , stateXVel = zeroVelocity
  , stateYVel = zeroVelocity
  , statePose = poseStandRight
  , stateMovementType = mvtStanding
  , stateVerticalDir = VDirStationary
  , stateAccelMode = AccelNone
  , stateOnGround = True  -- Start on ground
  , stateJumpHeld = False
  , stateJumpSquatFrames = 0
  }
