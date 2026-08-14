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
import Physics.SM.Constants (faceRight, isFacingRight)
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
-- Manual FromJSON/ToJSON to omit enemies field when Nothing.
data TrajectoryFrame = TrajectoryFrame
  { frameState :: !SimState
  , enemies :: !(Maybe [Enemy])
  } deriving stock (Eq, Show, Generic)

instance FromJSON TrajectoryFrame where
  parseJSON = withObject "TrajectoryFrame" $ \o -> do
    st <- parseJSON (Object o)
    maybeEnemies <- o .:? "enemies"
    return (TrajectoryFrame st maybeEnemies)

instance ToJSON TrajectoryFrame where
  toJSON (TrajectoryFrame st Nothing) = object
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
  toJSON (TrajectoryFrame st (Just es)) = object
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

-- Residual-relevant fields must round-trip. Extra run is momentum_x.
-- Speed-booster counter is exposed; shinespark stays zero until Mini
-- has an M–E budget for it.
toSimState :: SamusState -> SimState
toSimState state = SimState
  { frame = stateFrame state
  , room_id = 0x91F8
  , samus_x = unPixel (posPixel (stateXPos state))
  , samus_y = unPixel (posPixel (stateYPos state))
  , samus_x_sub = unSubpixel (posSubpixel (stateXPos state))
  , samus_y_sub = unSubpixel (posSubpixel (stateYPos state))
  , velocity_x = velPixel (stateXVel state)
  , velocity_y = velPixel (stateYVel state)
  , velocity_x_sub = unSubpixel (velSubpixel (stateXVel state))
  , velocity_y_sub = unSubpixel (velSubpixel (stateYVel state))
  , momentum_x = velPixel extraSigned
  , momentum_x_sub = unSubpixel (velSubpixel extraSigned)
  , pose = unPose (statePose state)
  , facing = stateFacing state
  , movement_type = unMovementType (stateMovementType state)
  , speed_counter = stateSpeedBoostCounter state
  , speed_flag = if stateHasMomentum state then 1 else 0
  , shinespark_timer = 0
  }
  where
    extraSigned
      | isFacingRight (stateFacing state) = stateXExtra state
      | otherwise = negateVelocity (stateXExtra state)

-- Residual-relevant fields come from SimState. Previous buttons are not on
-- the wire; callers that have a tape must thread them via fromFrameInput.
fromSimState :: SimState -> SamusState
fromSimState st = SamusState
  { stateXPos = Position (Pixel (samus_x st)) (Subpixel (samus_x_sub st))
  , stateYPos = Position (Pixel (samus_y st)) (Subpixel (samus_y_sub st))
  , stateXVel = Velocity (velocity_x st) (Subpixel (velocity_x_sub st))
  , stateYVel = Velocity (velocity_y st) (Subpixel (velocity_y_sub st))
  , stateXExtra = extra
  , stateHasMomentum = speed_flag st /= 0 || extra /= zeroVelocity
  , stateSpeedBoostCounter = speed_counter st
  , statePose = SamusPose (pose st)
  , stateMovementType = MovementType (movement_type st)
  , stateVerticalDir = dir
  , stateAccelMode = AccelNone
  , stateOnGround = onGround
  , stateFacing = if facing st == 0 then faceRight else facing st
  , stateFrame = frame st
  , statePrevButtons = ButtonMask 0
  , stateJumpHeld = False
  , stateJumpSquatFrames = 0
  , stateEnvironment = EnvAir
  , stateEquipment = defaultEquipment
  }
  where
    mt = movement_type st
    onGround = mt == 0x00 || mt == 0x01 || mt == 0x04 || mt == 0x05 || mt == 0x10
    extraMag = fromSigned1616 (abs (toSigned1616 (Velocity (momentum_x st) (Subpixel (momentum_x_sub st)))))
    extra = extraMag
    dir
      | velocity_y st < 0 = VDirRising
      | velocity_y st > 0 = VDirFalling
      | otherwise = VDirStationary
