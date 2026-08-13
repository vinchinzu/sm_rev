-- | FFI wire format types for C interop.
--
-- These types match the snake_case layout for:
-- - MiniStep baseline (fast iteration, not acceptance)
-- - retro_rl stable-retro (emulator integration, acceptance layer)
-- - SMEDIT bridge (emulator WRAM telemetry)
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
    -- * Packed SNES adapter
  , packedToInternal
  , internalToPacked
  ) where

import Data.Aeson (FromJSON, ToJSON)
import Data.Bits ((.|.), (.&.), shiftL, shiftR)
import Data.Word (Word8, Word16)
import GHC.Generics (Generic)
import Physics.SM.Types

-- | Packed 8-bit SNES controller input (LOCKED wire format).
--
-- This is the standard SNES D-pad byte used by retro_rl:
--   Bit 7: Right = 0x80
--   Bit 6: Left  = 0x40
--   Bit 5: Down  = 0x20
--   Bit 4: Up    = 0x10
--   Bit 3: Start = 0x08
--   Bit 2: Select= 0x04
--   Bit 1: Y     = 0x02
--   Bit 0: B     = 0x01
--
-- Wire protocol: Left=0x40, Right=0x80 (packed byte 0)
-- Internal: converted to 16-bit ButtonMask via packedToInternal
--
-- Extended buttons (A/X/L/R) in byte 1 not yet implemented.
newtype FrameInput = FrameInput { frameInputPacked :: Word8 }
  deriving stock (Eq, Show, Generic)
  deriving newtype (Num, FromJSON, ToJSON)

-- | Convert packed 8-bit SNES input to internal 16-bit ButtonMask.
--
-- Maps packed byte to full 16-bit mask matching C kButton_* constants.
packedToInternal :: Word8 -> ButtonMask
packedToInternal packed =
  let byte0 = fromIntegral packed :: Word16
      -- Map packed bits to 16-bit positions:
      -- Packed bit 0 (B) -> 0x8000, bit 1 (Y) -> 0x4000, etc.
      b     = if packed .&. 0x01 /= 0 then 0x8000 else 0
      y     = if packed .&. 0x02 /= 0 then 0x4000 else 0
      sel   = if packed .&. 0x04 /= 0 then 0x2000 else 0
      start = if packed .&. 0x08 /= 0 then 0x1000 else 0
      up    = if packed .&. 0x10 /= 0 then 0x0800 else 0
      down  = if packed .&. 0x20 /= 0 then 0x0400 else 0
      left  = if packed .&. 0x40 /= 0 then 0x0200 else 0
      right = if packed .&. 0x80 /= 0 then 0x0100 else 0
  in ButtonMask (b .|. y .|. sel .|. start .|. up .|. down .|. left .|. right)

-- | Convert internal 16-bit ButtonMask to packed 8-bit SNES format.
internalToPacked :: ButtonMask -> Word8
internalToPacked (ButtonMask mask) =
  let b     = if mask .&. 0x8000 /= 0 then 0x01 else 0
      y     = if mask .&. 0x4000 /= 0 then 0x02 else 0
      sel   = if mask .&. 0x2000 /= 0 then 0x04 else 0
      start = if mask .&. 0x1000 /= 0 then 0x08 else 0
      up    = if mask .&. 0x0800 /= 0 then 0x10 else 0
      down  = if mask .&. 0x0400 /= 0 then 0x20 else 0
      left  = if mask .&. 0x0200 /= 0 then 0x40 else 0
      right = if mask .&. 0x0100 /= 0 then 0x80 else 0
  in b .|. y .|. sel .|. start .|. up .|. down .|. left .|. right

-- | Convert internal ControllerInput to FFI FrameInput (packed).
toFrameInput :: ControllerInput -> FrameInput
toFrameInput input = FrameInput (internalToPacked (inputButtons input))

-- | Convert FFI FrameInput (packed) to internal ControllerInput.
fromFrameInput :: FrameInput -> ControllerInput -> ControllerInput
fromFrameInput (FrameInput packed) prev =
  ControllerInput
    { inputButtons = packedToInternal packed
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
