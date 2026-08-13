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
import Data.Bits (Bits, shiftR)
import Data.Int (Int16)
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

-- | Signed 16.16 fixed-point velocity (pixels per frame).
--
-- Matches C kernel: negative = upward/leftward, positive = downward/rightward.
-- Y: negative velocity moves up (Y decreases), positive moves down (Y increases).
data Velocity = Velocity
  { velPixel :: !Int16      -- SIGNED pixel component
  , velSubpixel :: !Subpixel -- Unsigned subpixel (0x0000-0xFFFF)
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

-- | Add two velocities with signed pixel carry.
addVelocity :: Velocity -> Velocity -> Velocity
addVelocity (Velocity p1 s1) (Velocity p2 s2) =
  let subSum = fromIntegral (unSubpixel s1) + fromIntegral (unSubpixel s2) :: Word32
      carry = fromIntegral (subSum `div` 65536) :: Int16
      newSub = Subpixel (fromIntegral subSum)
      newPix = p1 + p2 + carry
  in Velocity newPix newSub

-- | Apply velocity to position (signed 16.16 velocity, unsigned position).
--
-- Signed 16.16: pixel (Int16) + subpixel (Word16 fraction).
-- Velocity (-5, 0x8000) = -5 + 0.5 = -4.5, NOT -(5 + 0.5).
--
-- Y: negative velocity = upward (Y decreases), positive = downward (Y increases).
-- X: negative velocity = leftward (X decreases), positive = rightward (X increases).
applyVelocity :: Position -> Velocity -> Position
applyVelocity (Position pp ps) (Velocity vp vs)
  | vp < 0 && unSubpixel vs /= 0 =
      -- Negative velocity with fractional part: -5.5 means pixel=-5, sub=0x8000
      -- Magnitude is (abs(vp) - 1) pixels + (0x10000 - vs) subpixels
      -- Example: Velocity (-5, 0x8000) = -4.5 → subtract (4, 0x8000)
      let mag_pixel = fromIntegral (abs vp - 1) :: Word16
          mag_sub_val = (0x10000 :: Word32) - fromIntegral (unSubpixel vs)
          mag_sub = Subpixel (fromIntegral mag_sub_val :: Word16)
          absPos = Position (Pixel mag_pixel) mag_sub
      in subPosition (Position pp ps) absPos
  | vp < 0 =
      -- Negative velocity, zero fractional part: -5.0
      let absP = fromIntegral (abs vp) :: Word16
          absPos = Position (Pixel absP) (Subpixel 0)
      in subPosition (Position pp ps) absPos
  | otherwise =
      -- Positive velocity: add
      let posP = fromIntegral vp :: Word16
          posPos = Position (Pixel posP) vs
      in addPosition (Position pp ps) posPos

zeroPosition :: Position
zeroPosition = Position (Pixel 0) (Subpixel 0)

zeroVelocity :: Velocity
zeroVelocity = Velocity 0 (Subpixel 0)

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
    -- Jump squat duration (frames)
  , cfgJumpSquatDuration :: !Word16
    -- Collision (v1: flat infinite floor)
  , cfgGroundY :: !Pixel  -- Y coordinate of ground (flat floor for v1)
  } deriving stock (Eq, Show, Generic)
    deriving anyclass (FromJSON, ToJSON)

-- | Default config matching vanilla Super Metroid constants.
defaultConfig :: PhysicsConfig
defaultConfig = PhysicsConfig
  { cfgJumpInitialSpeed =
      [ Velocity (-5) (Subpixel 0x8000)  -- air: -4.5 pixels/frame (upward)
      , Velocity (-2) (Subpixel 0x4000)  -- water
      , Velocity (-3) (Subpixel 0x0000)  -- lava
      ]
  , cfgJumpHiInitialSpeed =
      [ Velocity (-6) (Subpixel 0x0000)  -- air: -6.0 pixels/frame (upward)
      , Velocity (-3) (Subpixel 0x0000)  -- water
      , Velocity (-4) (Subpixel 0x0000)  -- lava
      ]
  , cfgGravityAccel =
      [ Velocity 0 (Subpixel 0x1c00)  -- air: +0.109 pixels/frame (downward)
      , Velocity 0 (Subpixel 0x0800)  -- water
      , Velocity 0 (Subpixel 0x0900)  -- lava
      ]
  , cfgRunAccel = Velocity 0 (Subpixel 0x00a0)  -- +0.0098 pixels/frame
  , cfgRunDecel = Velocity 0 (Subpixel 0x0000)
  , cfgRunMaxSpeed = Velocity 3 (Subpixel 0x0000)  -- +3.0 pixels/frame
  , cfgTerminalSpeed = Pixel 5  -- Terminal Y velocity magnitude
  , cfgJumpSquatDuration = 4
  , cfgGroundY = Pixel 200
  }
