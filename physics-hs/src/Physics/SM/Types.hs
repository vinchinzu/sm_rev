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
  , subVelocity
  , applyVelocity
  , zeroPosition
  , zeroVelocity
  , toSigned1616
  , fromSigned1616
  , toUnsigned1616
  , fromUnsigned1616
  , negateVelocity
  , velocityMagnitude
  , cmpMagnitude
  , capMagnitude
  , scaleVelocity
    -- * Core state
  , SamusState (..)
  , ControllerInput (..)
  , ButtonMask (..)
  , SamusPose (..)
  , MovementType (..)
  , VerticalDirection (..)
  , AccelMode (..)
  , Environment (..)
  , Equipment (..)
  , defaultEquipment
    -- * Config
  , EnvTable (..)
  , selectEnv
  , PhysicsConfig (..)
  , defaultConfig
  ) where

import Data.Aeson (FromJSON, ToJSON)
import Data.Bits (Bits, shiftR)
import Data.Int (Int16, Int32)
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
-- Encoding: Velocity (-5, 0x2000) = -5 + 0x2000/65536 = -4.875.
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

-- | Signed 16.16 as a single Int32. Velocity (-5, 0x2000) = -4.875.
toSigned1616 :: Velocity -> Int32
toSigned1616 (Velocity p s) =
  fromIntegral p * 65536 + fromIntegral (unSubpixel s)

-- | Inverse of 'toSigned1616'. Uses toward-(-inf) division so negative
-- values keep the Haskell (pixel, subpixel) encoding.
fromSigned1616 :: Int32 -> Velocity
fromSigned1616 n =
  Velocity (fromIntegral (n `div` 65536)) (Subpixel (fromIntegral (n `mod` 65536)))

negateVelocity :: Velocity -> Velocity
negateVelocity = fromSigned1616 . negate . toSigned1616

-- | Unsigned 16.16 magnitude of a signed velocity.
velocityMagnitude :: Velocity -> Word32
velocityMagnitude v = fromIntegral (abs (toSigned1616 v))

cmpMagnitude :: Velocity -> Velocity -> Ordering
cmpMagnitude a b = compare (velocityMagnitude a) (velocityMagnitude b)

-- | Cap |v| to |limit|. Preserves sign of v. Limit should be non-negative.
capMagnitude :: Velocity -> Velocity -> Velocity
capMagnitude v limit =
  if velocityMagnitude v > velocityMagnitude limit
     then if toSigned1616 v < 0 then negateVelocity limit else limit
     else v

-- | Multiply a non-negative velocity by an integer.
scaleVelocity :: Velocity -> Int32 -> Velocity
scaleVelocity v n = fromSigned1616 (toSigned1616 v * n)

-- | Add two velocities with signed pixel carry.
addVelocity :: Velocity -> Velocity -> Velocity
addVelocity a b = fromSigned1616 (toSigned1616 a + toSigned1616 b)

-- | Subtract velocity (v1 - v2) with signed arithmetic.
subVelocity :: Velocity -> Velocity -> Velocity
subVelocity a b = fromSigned1616 (toSigned1616 a - toSigned1616 b)

-- | Unsigned 16.16 position as a single Word32.
toUnsigned1616 :: Position -> Word32
toUnsigned1616 (Position (Pixel p) (Subpixel s)) =
  fromIntegral p * 65536 + fromIntegral s

-- | Inverse of 'toUnsigned1616'. Low 16 bits are the subpixel.
fromUnsigned1616 :: Word32 -> Position
fromUnsigned1616 n =
  Position (Pixel (fromIntegral (n `shiftR` 16)))
           (Subpixel (fromIntegral n))

-- | Apply signed 16.16 velocity to unsigned position via 2^32 wrap.
-- Negative Int32 -> Word32 is the intended wrap.
applyVelocity :: Position -> Velocity -> Position
applyVelocity pos vel =
  fromUnsigned1616 (toUnsigned1616 pos + fromIntegral (toSigned1616 vel))

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

-- | Equipment flags that change residual-relevant tables.
data Equipment = Equipment
  { equipHiJump :: !Bool
  , equipSpeedBooster :: !Bool
  , equipMorph :: !Bool
  } deriving stock (Eq, Show, Generic)
    deriving anyclass (FromJSON, ToJSON)

defaultEquipment :: Equipment
defaultEquipment = Equipment
  { equipHiJump = False
  , equipSpeedBooster = False
  , equipMorph = False
  }

-- | Per-environment table (air / water / lava-acid).
data EnvTable a = EnvTable
  { envAir :: !a
  , envWater :: !a
  , envLava :: !a
  } deriving stock (Eq, Show, Generic)
    deriving anyclass (FromJSON, ToJSON)

selectEnv :: Environment -> EnvTable a -> a
selectEnv EnvAir      = envAir
selectEnv EnvWater    = envWater
selectEnv EnvLavaAcid = envLava

-- | Full Samus state for one frame.
data SamusState = SamusState
  { -- Position
    stateXPos :: !Position
  , stateYPos :: !Position
    -- Base X velocity (speed table). Extra run is stateXExtra.
  , stateXVel :: !Velocity
  , stateYVel :: !Velocity
    -- Extra run / speed-booster residual (unsigned magnitude).
  , stateXExtra :: !Velocity
  , stateHasMomentum :: !Bool
  , stateSpeedBoostCounter :: !Word16
    -- Movement state
  , statePose :: !SamusPose
  , stateMovementType :: !MovementType
  , stateVerticalDir :: !VerticalDirection
  , stateAccelMode :: !AccelMode
  , stateOnGround :: !Bool
  , stateFacing :: !Word16
  , stateFrame :: !Word16
  , statePrevButtons :: !ButtonMask
    -- Jump state
  , stateJumpHeld :: !Bool
  , stateJumpSquatFrames :: !Word16
    -- Environment / equipment (selected tables)
  , stateEnvironment :: !Environment
  , stateEquipment :: !Equipment
  } deriving stock (Eq, Show, Generic)
    deriving anyclass (FromJSON, ToJSON)

-- | Physics configuration. Jump / gravity / extra-run numbers match
-- physics_config.c and the ROM extra-run table at $90:9F07.
data PhysicsConfig = PhysicsConfig
  { cfgJumpInitialSpeed :: !(EnvTable Velocity)
  , cfgJumpHiInitialSpeed :: !(EnvTable Velocity)
  , cfgGravityAccel :: !(EnvTable Velocity)
  , cfgTerminalSpeed :: !Pixel
  , cfgJumpSquatDuration :: !Word16
  , cfgGroundY :: !Pixel
  , cfgExtraRunAccel :: !Velocity
  , cfgExtraRunCapNormal :: !Velocity
  , cfgExtraRunCapBoost :: !Velocity
  , cfgCrouchJumpYOffset :: !Pixel
  } deriving stock (Eq, Show, Generic)
    deriving anyclass (FromJSON, ToJSON)

-- | Default config matching vanilla Super Metroid constants.
--
-- Jump impulses are the C unsigned {speed, sub} pairs stored as signed
-- 16.16. Air 4.E000 = -4.875 = Velocity (-5, 0x2000).
defaultConfig :: PhysicsConfig
defaultConfig = PhysicsConfig
  { cfgJumpInitialSpeed = EnvTable
      { envAir   = Velocity (-5) (Subpixel 0x2000)  -- air: 4.E000 upward
      , envWater = Velocity (-2) (Subpixel 0x4000)  -- water: 1.C000
      , envLava  = Velocity (-3) (Subpixel 0x4000)  -- lava: 2.C000
      }
  , cfgJumpHiInitialSpeed = EnvTable
      { envAir   = Velocity (-6) (Subpixel 0x0000)  -- air: 6.0000
      , envWater = Velocity (-3) (Subpixel 0x8000)  -- water: 2.8000
      , envLava  = Velocity (-4) (Subpixel 0x8000)  -- lava: 3.8000
      }
  , cfgGravityAccel = EnvTable
      { envAir   = Velocity 0 (Subpixel 0x1c00)
      , envWater = Velocity 0 (Subpixel 0x0800)
      , envLava  = Velocity 0 (Subpixel 0x0900)
      }
  , cfgTerminalSpeed = Pixel 5
  , cfgJumpSquatDuration = 4
  , cfgGroundY = Pixel 200
  , cfgExtraRunAccel = Velocity 0 (Subpixel 0x1000)
  , cfgExtraRunCapNormal = Velocity 2 (Subpixel 0)
  , cfgExtraRunCapBoost = Velocity 7 (Subpixel 0)
  , cfgCrouchJumpYOffset = Pixel 10
  }
