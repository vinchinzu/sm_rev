-- | Property-based tests calling C MiniStep oracle.
--
-- Verifies Haskell step matches C oracle for three critical scenarios:
--   1. Ground run holding RIGHT
--   2. Short hop vs full hop (peak y/subY + landing frame)
--   3. Run-up onto 1-tile platform
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
tests = testGroup "Properties (vs MiniStep Oracle)"
  [ testGroundRunRight
  , testShortVsFullHop
  , testPlatformClimb
  ]

-- | Test 1: Ground run holding RIGHT for N frames.
--
-- Verifies position and velocity match C oracle at subpixel precision.
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
  
  -- Run C oracle
  oracleResult <- runMiniStepOracle state0 packed
  case oracleResult of
    Left err -> assertFailure $ "Oracle failed: " ++ err
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
  assertFailure "Short hop test requires A button in packed format (byte 1), documenting ROM block"

-- | Test 3: Run up onto a 1-tile platform.
--
-- ROM BLOCK: MiniStep requires collision map (room layout) to express platform geometry.
-- Without a ROM, MiniStep cannot load room collision data. The C oracle needs either:
--   1. A .sfc ROM file to load room data, OR
--   2. Programmatic collision map injection (not currently exposed by MiniStep API)
--
-- This test documents the ROM dependency. When MiniStep gains ROM-free collision
-- map injection (e.g., via JSON fixture), this test can be implemented as:
--   - Initial state: Samus on ground, 1-tile platform ahead at Y-16
--   - Input: B+Right for run-up, then A to jump
--   - Assert: Samus lands on platform (Y position stable, on_ground=true)
testPlatformClimb :: TestTree
testPlatformClimb = testCase "Run-up onto 1-tile platform (ROM BLOCK)" $ do
  -- This test requires:
  --   1. Room collision map (1-tile platform at specific X,Y)
  --   2. Ground detection logic (check block below Samus)
  --   3. Platform edge handling (land on top, not fall through)
  --
  -- Current ROM BLOCK: MiniStep boots from ROM/saves which contain room data.
  -- Without ROM, cannot express the fixture. Options:
  --   A. MiniStep --json-collision flag to inject map (not yet available)
  --   B. Recorded golden from ROM run (defeats pure test intent)
  --   C. Document as blocked until ROM-free collision API
  --
  -- Choosing C: document the block explicitly
  assertFailure $ unlines
    [ "ROM BLOCK: 1-tile platform test requires collision map"
    , ""
    , "MiniStep needs room geometry to express a 1-tile platform."
    , "Current MiniStep API loads rooms from ROM/saves only."
    , ""
    , "To unblock:"
    , "  1. Add MiniStep --collision-json fixture injection, OR"
    , "  2. Extend MiniStep API to programmatically set collision map"
    , ""
    , "Expected test:"
    , "  - Initial: Samus at (100, 200), platform tile at (200, 184)"
    , "  - Input: B+Right (run-up) then A (jump)"
    , "  - Assert: Land on platform (Y=184, on_ground=true, x/y/subX/subY match oracle)"
    ]

-- | Call C MiniStep oracle via subprocess.
--
-- Expects a CLI tool: sm_rev_mini_oracle --json < input.json
-- Or falls back to testing Haskell determinism if oracle unavailable.
runMiniStepOracle :: SamusState -> [Word8] -> IO (Either String [SamusState])
runMiniStepOracle initialSt packedInputs = do
  -- Prepare input JSON
  let request = OracleRequest
        { reqInitialState = toTrajectory initialSt
        , reqInputs = map FrameInput packedInputs
        }
      inputJson = encode request
  
  -- Call oracle (or check if it exists)
  (exitCode, stdout, stderr) <- readProcessWithExitCode "sm_rev_mini_oracle" ["--json"] (BL.unpack inputJson)
  case exitCode of
    ExitSuccess -> do
      -- Parse oracle response
      case eitherDecode (BL.pack stdout) of
        Left parseErr -> return $ Left $ "Oracle parse error: " ++ parseErr
        Right (OracleResponse states) -> return $ Right $ map fromTrajectory states
    ExitFailure code ->
      return $ Left $ "Oracle failed (exit " ++ show code ++ "): " ++ stderr

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
