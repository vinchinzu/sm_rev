{-# LANGUAGE DeriveAnyClass #-}

-- | Core types for Super Metroid physics simulation.
--
-- Newtypes prevent mixing incompatible units (pixels vs subpixels vs velocities).
-- All values match the C implementation's 16.16 fixed-point representation.
module Physics.SM.Types
  ( -- * Position and velocity components
    Pixel (..)
  , Subpixel (..)
  , Position (..)
  , Velocity (..)
    -- * Fixed-point combinators
  , addPosition
  , subPosition
  , addVelocity
  , applyVelocity
  , zeroPosition
  , zeroVelocity
    -- * Core state
  , SamusState (..)
  , ControllerInput (..)
  , ButtonMask (..)
  , SamusPose (..)
  , MovementType (..)
  , VerticalDirection (..)
  , AccelMode (..)
  , Environment (..)
    -- * Config
  , PhysicsConfig (..)
  , defaultConfig
  ) where

import Data.Aeson (FromJSON, ToJSON)
import Data.Bits (Bits, (.&.), shiftR)
import Data.Word (Word16, Word32)
import GHC.Generics (Generic)

-- | Whole pixel component (16.0 fixed-point).
--
-- Range: 0..65535 pixels. Screen space is typically 0..512.
newtype Pixel = Pixel { unPixel :: Word16 }
  deriving stock (Eq, Ord, Show, Generic)
  deriving newtype (Num, FromJSON, ToJSON)

-- | Subpixel component (0.16 fixed-point).
--
-- Range: 0x0000 (0/65536) to 0xFFFF (65535/65536).
-- 0x8000 = exactly 0.5 pixels.
newtype Subpixel = Subpixel { unSubpixel :: Word16 }
  deriving stock (Eq, Ord, Show, Generic)
  deriving newtype (Num, FromJSON, ToJSON)

-- | Full 16.16 fixed-point position.
data Position = Position
  { posPixel :: !Pixel
  , posSubpixel :: !Subpixel
  } deriving stock (Eq, Show, Generic)
    deriving anyclass (FromJSON, ToJSON)

-- | Full 16.16 fixed-point velocity (pixels per frame).
data Velocity = Velocity
  { velPixel :: !Pixel
  , velSubpixel :: !Subpixel
  } deriving stock (Eq, Show, Generic)
    deriving anyclass (FromJSON, ToJSON)

-- | Add two positions with carry from subpixels to pixels.
addPosition :: Position -> Position -> Position
addPosition (Position p1 s1) (Position p2 s2) =
  let subSum = fromIntegral (unSubpixel s1) + fromIntegral (unSubpixel s2) :: Word32
      carry = Pixel (fromIntegral (subSum `shiftR` 16))
      newSub = Subpixel (fromIntegral subSum)
      newPix = p1 + p2 + carry
  in Position newPix newSub

-- | Subtract positions (p1 - p2) with borrow.
subPosition :: Position -> Position -> Position
subPosition (Position p1 s1) (Position p2 s2) =
  let s1' = fromIntegral (unSubpixel s1) :: Word32
      s2' = fromIntegral (unSubpixel s2) :: Word32
      (borrow, newSub) = if s1' < s2'
                         then (Pixel 1, Subpixel (fromIntegral (65536 + s1' - s2')))
                         else (Pixel 0, Subpixel (fromIntegral (s1' - s2')))
      newPix = p1 - p2 - borrow
  in Position newPix newSub

-- | Add two velocities with carry.
addVelocity :: Velocity -> Velocity -> Velocity
addVelocity (Velocity p1 s1) (Velocity p2 s2) =
  let subSum = fromIntegral (unSubpixel s1) + fromIntegral (unSubpixel s2) :: Word32
      carry = Pixel (fromIntegral (subSum `div` 65536))
      newSub = Subpixel (fromIntegral subSum)
      newPix = p1 + p2 + carry
  in Velocity newPix newSub

-- | Apply velocity to position: pos + vel.
applyVelocity :: Position -> Velocity -> Position
applyVelocity (Position pp ps) (Velocity vp vs) =
  addPosition (Position pp ps) (Position vp vs)

zeroPosition :: Position
zeroPosition = Position (Pixel 0) (Subpixel 0)

zeroVelocity :: Velocity
zeroVelocity = Velocity (Pixel 0) (Subpixel 0)

-- | Button bitmask matching C enum kButton_*.
newtype ButtonMask = ButtonMask { unButtonMask :: Word16 }
  deriving stock (Eq, Show, Generic)
  deriving newtype (Num, Bits, FromJSON, ToJSON)

-- | Controller input for one frame.
data ControllerInput = ControllerInput
  { inputButtons :: !ButtonMask
  , inputPrevButtons :: !ButtonMask
  } deriving stock (Eq, Show, Generic)
    deriving anyclass (FromJSON, ToJSON)

-- | Samus pose index (kPose_* from C).
newtype SamusPose = SamusPose { unPose :: Word16 }
  deriving stock (Eq, Ord, Show, Generic)
  deriving newtype (Num, FromJSON, ToJSON)

-- | Movement type index (kMovementType_* from C).
newtype MovementType = MovementType { unMovementType :: Word16 }
  deriving stock (Eq, Ord, Show, Generic)
  deriving newtype (Num, FromJSON, ToJSON)

-- | Vertical movement direction (1 = rising, 2 = falling, 0 = stationary).
data VerticalDirection
  = VDirStationary
  | VDirRising
  | VDirFalling
  deriving stock (Eq, Show, Generic)
  deriving anyclass (FromJSON, ToJSON)

-- | Horizontal acceleration mode.
data AccelMode
  = AccelNone
  | AccelAccelerating
  | AccelDecelerating
  deriving stock (Eq, Show, Generic)
  deriving anyclass (FromJSON, ToJSON)

-- | Vertical environment for gravity/jump (air, water, lava/acid).
data Environment
  = EnvAir
  | EnvWater
  | EnvLavaAcid
  deriving stock (Eq, Show, Generic)
  deriving anyclass (FromJSON, ToJSON)

-- | Full Samus state for one frame.
data SamusState = SamusState
  { -- Position
    stateXPos :: !Position
  , stateYPos :: !Position
    -- Velocity
  , stateXVel :: !Velocity
  , stateYVel :: !Velocity
    -- Movement state
  , statePose :: !SamusPose
  , stateMovementType :: !MovementType
  , stateVerticalDir :: !VerticalDirection
  , stateAccelMode :: !AccelMode
  , stateOnGround :: !Bool
    -- Jump state
  , stateJumpHeld :: !Bool
  , stateJumpSquatFrames :: !Word16
  } deriving stock (Eq, Show, Generic)
    deriving anyclass (FromJSON, ToJSON)

-- | Physics configuration (constants from physics_config.c).
data PhysicsConfig = PhysicsConfig
  { -- Jump initial speeds [air, water, lava]
    cfgJumpInitialSpeed :: ![Velocity]
  , cfgJumpHiInitialSpeed :: ![Velocity]
    -- Gravity [air, water, lava]
  , cfgGravityAccel :: ![Velocity]
    -- Run acceleration
  , cfgRunAccel :: !Velocity
  , cfgRunDecel :: !Velocity
  , cfgRunMaxSpeed :: !Velocity
    -- Terminal velocity
  , cfgTerminalSpeed :: !Pixel
  } deriving stock (Eq, Show, Generic)
    deriving anyclass (FromJSON, ToJSON)

-- | Default config matching vanilla Super Metroid constants.
defaultConfig :: PhysicsConfig
defaultConfig = PhysicsConfig
  { cfgJumpInitialSpeed =
      [ Velocity (Pixel 4) (Subpixel 0xe000)  -- air
      , Velocity (Pixel 1) (Subpixel 0xc000)  -- water
      , Velocity (Pixel 2) (Subpixel 0xc000)  -- lava
      ]
  , cfgJumpHiInitialSpeed =
      [ Velocity (Pixel 6) (Subpixel 0x0000)  -- air
      , Velocity (Pixel 2) (Subpixel 0x8000)  -- water
      , Velocity (Pixel 3) (Subpixel 0x8000)  -- lava
      ]
  , cfgGravityAccel =
      [ Velocity (Pixel 0) (Subpixel 0x1c00)  -- air
      , Velocity (Pixel 0) (Subpixel 0x0800)  -- water
      , Velocity (Pixel 0) (Subpixel 0x0900)  -- lava
      ]
  , cfgRunAccel = Velocity (Pixel 0) (Subpixel 0x00a0)
  , cfgRunDecel = Velocity (Pixel 0) (Subpixel 0x0000)
  , cfgRunMaxSpeed = Velocity (Pixel 3) (Subpixel 0x0000)
  , cfgTerminalSpeed = Pixel 5
  }
