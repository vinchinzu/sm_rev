-- | Property-based tests calling MiniStep baseline (fast iteration).
--
-- MiniStep is a SIMPLIFIED MODEL, not TAS-correct acceptance.
-- Acceptance is real emulator (snes9x/libretro) via SMEDIT/retro_rl.
--
-- Verifies Haskell step bisimulates Mini baseline for three scenarios:
--   1. Ground run holding RIGHT
--   2. Short hop vs full hop (peak y/subY + landing frame)
--   3. Run-up onto 1-tile platform
--
-- If Mini and emulator disagree, emulator wins (file Mini delta).
module Test.Properties (tests) where

import Data.Aeson (eitherDecode, encode)
import Data.ByteString.Lazy qualified as BL
import Data.Word (Word16)
import Physics.SM
import System.Exit (ExitCode (..))
import System.Process (readProcessWithExitCode)
import Test.Tasty (TestTree, testGroup)
import Test.Tasty.HUnit ((@?=), assertBool, assertFailure, testCase)

tests :: TestTree
tests = testGroup "Properties (vs MiniStep Baseline)"
  [ testGroundRunRight
  , testShortVsFullHop
  , testPlatformClimb
  ]

-- | Test 1: Ground run holding RIGHT for N frames.
--
-- Verifies position and velocity bisimulate MiniStep baseline at subpixel precision.
testGroundRunRight :: TestTree
testGroundRunRight = testCase "Ground run RIGHT (B+Right held)" $ do
  let frames = 60  -- 1 second at 60fps
      -- Packed SNES: Right=0x80, B=0x01
      packed = replicate frames (0x81 :: Word8)  -- B + Right
  
  -- Run Haskell model
  let cfg = defaultConfig
      state0 = initialState cfg
      inputs = map (\p -> fromFrameInput (FrameInput p) (ControllerInput (ButtonMask 0) (ButtonMask 0))) packed
      hsStates = runTape cfg state0 inputs
      hsLast = last hsStates
  
  -- Run MiniStep baseline
  baselineResult <- runMiniStepBaseline state0 packed
  case baselineResult of
    Left err -> assertFailure $ "Baseline failed: " ++ err
    Right cStates -> do
      let cLast = last cStates
      -- Assert subpixel-precision equality
      stateXPos hsLast @?= stateXPos cLast
      stateYPos hsLast @?= stateYPos cLast
      stateXVel hsLast @?= stateXVel cLast
      statePose hsLast @?= statePose cLast

-- | Test 2: Short hop (release A early) vs full hop (hold A).
--
-- Verifies peak Y position and landing frame differ correctly.
testShortVsFullHop :: TestTree
testShortVsFullHop = testCase "Short hop vs full hop peak/landing" $ do
  -- Packed SNES: A=0 (in byte 1, not in packed byte 0), use empty for now
  -- Short hop: press jump (A would be 0x01 in byte 1, packed doesn't have A)
  -- For now, use internal format until A button mapping is clear
  -- User wants packed format but A is in second byte. Document this limitation.
  assertFailure "Short hop test requires A button in packed format (byte 1), Mini baseline gap"

-- | Test 3: Run up onto a 1-tile platform.
--
-- MINI BASELINE GAP: MiniStep requires collision map (room layout).
-- Without ROM, MiniStep cannot load room collision data. Baseline needs:
--   1. Programmatic collision map injection (not currently exposed), OR
--   2. Emulator acceptance test via SMEDIT/retro_rl
--
-- This test documents the baseline gap. Emulator acceptance will validate
-- platform climb via real room data.
testPlatformClimb :: TestTree
testPlatformClimb = testCase "Run-up onto 1-tile platform (Mini baseline gap)" $ do
  assertFailure $ unlines
    [ "MINI BASELINE GAP: 1-tile platform requires collision map"
    , ""
    , "MiniStep needs room geometry to express a 1-tile platform."
    , "Current MiniStep API loads rooms from ROM/saves only."
    , ""
    , "Baseline options:"
    , "  1. Add MiniStep --collision-json fixture injection"
    , "  2. Skip baseline, test via emulator acceptance (SMEDIT/retro_rl)"
    , ""
    , "Expected emulator acceptance:"
    , "  - Initial: Samus at (100, 200), platform tile at (200, 184)"
    , "  - Input: B+Right (run-up) then A (jump)"
    , "  - Assert: Land on platform (Y=184, on_ground=true, WRAM match)"
    , ""
    , "If Mini and emu disagree, emu wins (file Mini delta)."
    ]

-- | Call MiniStep baseline via subprocess (optional, for golden regeneration).
--
-- Expects: sm_rev_mini_oracle --json < input.json
-- Returns: Baseline states or error (CI uses recorded goldens instead)
runMiniStepBaseline :: SamusState -> [Word8] -> IO (Either String [SamusState])
runMiniStepBaseline initialSt packedInputs = do
  -- Prepare input JSON
  let request = OracleRequest
        { reqInitialState = toTrajectory initialSt
        , reqInputs = map FrameInput packedInputs
        }
      inputJson = encode request
  
  -- Call baseline (or check if it exists)
  (exitCode, stdout, stderr) <- readProcessWithExitCode "sm_rev_mini_oracle" ["--json"] (BL.unpack inputJson)
  case exitCode of
    ExitSuccess -> do
      -- Parse baseline response
      case eitherDecode (BL.pack stdout) of
        Left parseErr -> return $ Left $ "Baseline parse error: " ++ parseErr
        Right (OracleResponse states) -> return $ Right $ map fromTrajectory states
    ExitFailure code ->
      return $ Left $ "Baseline failed (exit " ++ show code ++ "): " ++ stderr

import Data.Word (Word8)

-- | Oracle request format (JSON).
data OracleRequest = OracleRequest
  { reqInitialState :: Trajectory
  , reqInputs :: [FrameInput]
  } deriving (Show, Generic)

instance ToJSON OracleRequest where
  toJSON req = object
    [ "initial_state" .= reqInitialState req
    , "inputs" .= reqInputs req
    ]

-- | Oracle response format (JSON).
newtype OracleResponse = OracleResponse [Trajectory]
  deriving (Show, Generic)

instance FromJSON OracleResponse where
  parseJSON = withObject "OracleResponse" $ \o -> do
    states <- o .: "states"
    return (OracleResponse states)

import Data.Aeson (FromJSON, ToJSON, object, withObject, (.:), (.=))
import GHC.Generics (Generic)
