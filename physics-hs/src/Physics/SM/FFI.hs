-- | FFI wire format matching vinchinzu/retro_rl@66836f5 exactly.
--
-- SOURCE: snes/super_metroid/physics_sim.py Trajectory.to_dict()
-- CLI: stdin {"start": <SimState>, "inputs": [{"buttons": int}, ...]}
-- CLI: stdout Trajectory.to_dict() keys: start, frames, predictor, inputs
module Physics.SM.FFI
  ( -- * Frame-level wire types (retro_rl format)
    FrameInput (..)
  , SimState (..)
  , TrajectoryFrame (..)
  , Trajectory (..)
    -- * Conversion to/from internal types
  , toFrameInput
  , fromFrameInput
  , toSimState
  , fromSimState
  ) where

import Data.Aeson
  ( FromJSON, ToJSON, Value(Object), object, parseJSON, toJSON, withObject
  , (.:?), (.=)
  )
import Data.Int (Int16)
import Data.Word (Word16)
import GHC.Generics (Generic)
import Physics.SM.Types

-- | FrameInput from retro_rl: {"buttons": int} with 12-bit mask.
--
-- LOCKED: Do not fork this encoding.
-- B=0x001 Y=0x002 Select=0x004 Start=0x008
-- Up=0x010 Down=0x020 Left=0x040 Right=0x080
-- A=0x100 X=0x200 L=0x400 R=0x800
--
-- Golden: 128=Right (0x080), 129=B+Right (0x081)
newtype FrameInput = FrameInput { buttons :: Word16 }
  deriving stock (Eq, Show, Generic)
  deriving anyclass (FromJSON, ToJSON)

-- | SimState from retro_rl physics_sim.py (18 required keys).
--
-- LOCKED: All fields required. Match retro_rl exactly.
-- velocity_x/y and momentum_x are SIGNED i16 JSON.
-- facing: LEFT=0x04 RIGHT=0x08 (not 0/1)
-- room_id: $079B room header ptr (Landing Site = 0x91F8)
data SimState = SimState
  { frame :: !Word16
  , room_id :: !Word16            -- $079B room header ptr
  , samus_x :: !Word16
  , samus_y :: !Word16
  , samus_x_sub :: !Word16
  , samus_y_sub :: !Word16
  , velocity_x :: !Int16          -- SIGNED
  , velocity_y :: !Int16          -- SIGNED
  , velocity_x_sub :: !Word16
  , velocity_y_sub :: !Word16
  , momentum_x :: !Int16          -- SIGNED
  , momentum_x_sub :: !Word16
  , pose :: !Word16
  , facing :: !Word16             -- LEFT=0x04 RIGHT=0x08
  , movement_type :: !Word16
  , speed_counter :: !Word16
  , speed_flag :: !Word16
  , shinespark_timer :: !Word16
  } deriving stock (Eq, Show, Generic)
    deriving anyclass (FromJSON, ToJSON)

-- | TrajectoryFrame: SimState + optional enemies.
--
-- Note: enemies field omitted from JSON if Nothing (manual ToJSON below).
data TrajectoryFrame = TrajectoryFrame
  { frameState :: !SimState
  , enemies :: !(Maybe [Enemy])
  } deriving stock (Eq, Show, Generic)

-- Manual ToJSON to omit enemies field when Nothing
instance ToJSON TrajectoryFrame where
  toJSON (TrajectoryFrame st Nothing) = Data.Aeson.object
    [ "frame" .= frame st
    , "room_id" .= room_id st
    , "samus_x" .= samus_x st
    , "samus_y" .= samus_y st
    , "samus_x_sub" .= samus_x_sub st
    , "samus_y_sub" .= samus_y_sub st
    , "velocity_x" .= velocity_x st
    , "velocity_y" .= velocity_y st
    , "velocity_x_sub" .= velocity_x_sub st
    , "velocity_y_sub" .= velocity_y_sub st
    , "momentum_x" .= momentum_x st
    , "momentum_x_sub" .= momentum_x_sub st
    , "pose" .= pose st
    , "facing" .= facing st
    , "movement_type" .= movement_type st
    , "speed_counter" .= speed_counter st
    , "speed_flag" .= speed_flag st
    , "shinespark_timer" .= shinespark_timer st
    ]
  toJSON (TrajectoryFrame st (Just es)) = Data.Aeson.object
    [ "frame" .= frame st
    , "room_id" .= room_id st
    , "samus_x" .= samus_x st
    , "samus_y" .= samus_y st
    , "samus_x_sub" .= samus_x_sub st
    , "samus_y_sub" .= samus_y_sub st
    , "velocity_x" .= velocity_x st
    , "velocity_y" .= velocity_y st
    , "velocity_x_sub" .= velocity_x_sub st
    , "velocity_y_sub" .= velocity_y_sub st
    , "momentum_x" .= momentum_x st
    , "momentum_x_sub" .= momentum_x_sub st
    , "pose" .= pose st
    , "facing" .= facing st
    , "movement_type" .= movement_type st
    , "speed_counter" .= speed_counter st
    , "speed_flag" .= speed_flag st
    , "shinespark_timer" .= shinespark_timer st
    , "enemies" .= es
    ]

data Enemy = Enemy
  { enemy_x :: !Word16
  , enemy_y :: !Word16
  , enemy_type :: !Word16
  , enemy_health :: !Word16
  } deriving stock (Eq, Show, Generic)
    deriving anyclass (FromJSON, ToJSON)

-- | Trajectory.to_dict() from retro_rl physics_sim.py.
--
-- Key order: start, frames, predictor, inputs
data Trajectory = Trajectory
  { start :: !SimState
  , frames :: ![TrajectoryFrame]
  , predictor :: !String          -- "haskell" or "mini"
  , inputs :: ![FrameInput]
  } deriving stock (Eq, Show, Generic)
    deriving anyclass (FromJSON, ToJSON)

-- | Convert internal ControllerInput to retro_rl FrameInput.
toFrameInput :: ControllerInput -> FrameInput
toFrameInput input = FrameInput (unButtonMask (inputButtons input))

-- | Convert retro_rl FrameInput to internal ControllerInput.
fromFrameInput :: FrameInput -> ControllerInput -> ControllerInput
fromFrameInput (FrameInput btns) prev =
  ControllerInput
    { inputButtons = ButtonMask btns
    , inputPrevButtons = inputButtons prev
    }

-- | Convert internal SamusState to retro_rl SimState.
--
-- NOTE: Many fields not yet in SamusState (momentum, speed_counter, etc.)
-- This is a PARTIAL conversion until full state is implemented.
toSimState :: SamusState -> SimState
toSimState state = SimState
  { frame = 0  -- TODO: add frame counter to SamusState
  , room_id = 0x91F8  -- TODO: Landing Site default
  , samus_x = unPixel (posPixel (stateXPos state))
  , samus_y = unPixel (posPixel (stateYPos state))
  , samus_x_sub = unSubpixel (posSubpixel (stateXPos state))
  , samus_y_sub = unSubpixel (posSubpixel (stateYPos state))
  , velocity_x = fromIntegral (unPixel (velPixel (stateXVel state)))  -- TODO: signed
  , velocity_y = fromIntegral (unPixel (velPixel (stateYVel state)))  -- TODO: signed
  , velocity_x_sub = unSubpixel (velSubpixel (stateXVel state))
  , velocity_y_sub = unSubpixel (velSubpixel (stateYVel state))
  , momentum_x = 0  -- TODO: momentum not yet tracked
  , momentum_x_sub = 0
  , pose = unPose (statePose state)
  , facing = 0x08  -- TODO: derive from pose (RIGHT default)
  , movement_type = unMovementType (stateMovementType state)
  , speed_counter = 0  -- TODO: not yet tracked
  , speed_flag = 0     -- TODO: not yet tracked
  , shinespark_timer = 0  -- TODO: not yet tracked
  }

-- | Convert retro_rl SimState to internal SamusState.
--
-- PARTIAL: Only uses subset of SimState fields.
fromSimState :: SimState -> SamusState
fromSimState st = SamusState
  { stateXPos = Position (Pixel (samus_x st)) (Subpixel (samus_x_sub st))
  , stateYPos = Position (Pixel (samus_y st)) (Subpixel (samus_y_sub st))
  , stateXVel = Velocity (Pixel (fromIntegral (velocity_x st))) (Subpixel (velocity_x_sub st))
  , stateYVel = Velocity (Pixel (fromIntegral (velocity_y st))) (Subpixel (velocity_y_sub st))
  , statePose = SamusPose (pose st)
  , stateMovementType = MovementType (movement_type st)
  , stateVerticalDir = VDirStationary  -- TODO: derive from velocity_y
  , stateAccelMode = AccelNone
  , stateOnGround = False  -- TODO: derive from movement_type
  , stateJumpHeld = False
  , stateJumpSquatFrames = 0
  }
