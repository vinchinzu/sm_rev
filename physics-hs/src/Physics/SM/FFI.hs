-- | FFI wire format types for C interop.
--
-- These types match the snake_case layout expected by C MiniStep oracle
-- and future ML/RL agent integration.
module Physics.SM.FFI
  ( -- * Frame-level wire types
    FrameInput (..)
  , Trajectory (..)
  , EnemyState (..)
    -- * Conversion to/from internal types
  , toFrameInput
  , fromFrameInput
  , toTrajectory
  , fromTrajectory
  ) where

import Data.Aeson (FromJSON, ToJSON)
import Data.Word (Word16)
import GHC.Generics (Generic)
import Physics.SM.Types

-- | Packed SNES controller input (one frame).
--
-- Button layout matches SNES hardware:
--   Bit 15-8: B Y Select Start Up Down Left Right
--   Bit  7-0: A X L R (unused) (unused) (unused) (unused)
--
-- Standard bindings:
--   Left  = 0x0200 (0x40 in high byte)
--   Right = 0x0100 (0x80 in high byte)
--   B     = 0x8000
--   A     = 0x0080
newtype FrameInput = FrameInput { frameInputButtons :: Word16 }
  deriving stock (Eq, Show, Generic)
  deriving newtype (Num, FromJSON, ToJSON)

-- | Convert internal ControllerInput to FFI FrameInput.
toFrameInput :: ControllerInput -> FrameInput
toFrameInput input = FrameInput (unButtonMask (inputButtons input))

-- | Convert FFI FrameInput to internal ControllerInput (with prev = current for first frame).
fromFrameInput :: FrameInput -> ControllerInput -> ControllerInput
fromFrameInput (FrameInput buttons) prev =
  ControllerInput
    { inputButtons = ButtonMask buttons
    , inputPrevButtons = inputButtons prev
    }

-- | Trajectory state for one frame (snake_case for C FFI).
--
-- Matches C MiniSamusCoreState layout: $0AF6/$0AF8 (x), $0AFA/$0AFC (y).
data Trajectory = Trajectory
  { samus_x :: !Word16        -- Pixel component of X position
  , samus_x_sub :: !Word16    -- Subpixel component of X
  , samus_y :: !Word16        -- Pixel component of Y position
  , samus_y_sub :: !Word16    -- Subpixel component of Y
  , velocity_x :: !Word16     -- X velocity (pixels/frame)
  , velocity_x_sub :: !Word16 -- X velocity subpixel
  , velocity_y :: !Word16     -- Y velocity (pixels/frame)
  , velocity_y_sub :: !Word16 -- Y velocity subpixel
  , pose :: !Word16           -- Samus pose index (kPose_*)
  , facing :: !Word16         -- 0 = right, 1 = left (from pose_x_dir)
  , movement_type :: !Word16  -- Movement type index (kMovementType_*)
  , on_ground :: !Bool        -- Ground contact flag
  , enemies :: ![EnemyState]  -- Optional enemy list (empty for pure Samus physics)
  } deriving stock (Eq, Show, Generic)
    deriving anyclass (FromJSON, ToJSON)

-- | Enemy state placeholder (for future collision integration).
data EnemyState = EnemyState
  { enemy_x :: !Word16
  , enemy_y :: !Word16
  , enemy_type :: !Word16
  , enemy_health :: !Word16
  } deriving stock (Eq, Show, Generic)
    deriving anyclass (FromJSON, ToJSON)

-- | Convert internal SamusState to FFI Trajectory.
toTrajectory :: SamusState -> Trajectory
toTrajectory state = Trajectory
  { samus_x = unPixel (posPixel (stateXPos state))
  , samus_x_sub = unSubpixel (posSubpixel (stateXPos state))
  , samus_y = unPixel (posPixel (stateYPos state))
  , samus_y_sub = unSubpixel (posSubpixel (stateYPos state))
  , velocity_x = unPixel (velPixel (stateXVel state))
  , velocity_x_sub = unSubpixel (velSubpixel (stateXVel state))
  , velocity_y = unPixel (velPixel (stateYVel state))
  , velocity_y_sub = unSubpixel (velSubpixel (stateYVel state))
  , pose = unPose (statePose state)
  , facing = if stateFacingRight state then 0 else 1
  , movement_type = unMovementType (stateMovementType state)
  , on_ground = stateOnGround state
  , enemies = []  -- Pure Samus physics (no enemies yet)
  }
  where
    -- Extract facing from pose (even = right, odd = left for most poses)
    stateFacingRight st = even (unPose (statePose st))

-- | Convert FFI Trajectory to internal SamusState.
fromTrajectory :: Trajectory -> SamusState
fromTrajectory traj = SamusState
  { stateXPos = Position (Pixel (samus_x traj)) (Subpixel (samus_x_sub traj))
  , stateYPos = Position (Pixel (samus_y traj)) (Subpixel (samus_y_sub traj))
  , stateXVel = Velocity (Pixel (velocity_x traj)) (Subpixel (velocity_x_sub traj))
  , stateYVel = Velocity (Pixel (velocity_y traj)) (Subpixel (velocity_y_sub traj))
  , statePose = SamusPose (pose traj)
  , stateMovementType = MovementType (movement_type traj)
  , stateVerticalDir = VDirStationary  -- Derived from velocity_y in step
  , stateAccelMode = AccelNone         -- Derived from input in step
  , stateOnGround = on_ground traj
  , stateJumpHeld = False              -- Derived from input in step
  , stateJumpSquatFrames = 0           -- Internal state not in trajectory
  }
