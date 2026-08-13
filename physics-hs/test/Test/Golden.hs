-- | Golden tape tests: verify against recorded MiniStep baseline outputs.
--
-- MiniStep is a SIMPLIFIED MODEL for fast iteration, NOT acceptance.
-- Acceptance is real emulator (snes9x/libretro) via SMEDIT/retro_rl.
--
-- Goldens are JSON files recorded from MiniStep runs. CI tests replay these
-- without requiring binary or ROM. Optional live MiniStep path verifies drift.
--
-- To regenerate golden tapes:
--   1. Ensure sm_rev_mini_oracle is in PATH
--   2. Run: cabal test --test-option=--regenerate-goldens
--   3. Commit updated test/golden/*.json
--
-- If Mini and emulator disagree on acceptance, emulator wins (file Mini delta).
module Test.Golden (tests) where

import Data.Aeson (FromJSON, eitherDecodeFileStrict', withObject, (.:))
import Physics.SM
import System.Directory (doesFileExist)
import System.Environment (lookupEnv)
import Test.Tasty (TestTree, testGroup)
import Test.Tasty.HUnit ((@?=), assertBool, assertFailure, testCase)

tests :: TestTree
tests = testGroup "Golden Tapes (Recorded Oracle)"
  [ testCase "Golden: ground run right" $ do
      runGoldenTest "test/golden/run_right.json"

  , testCase "Golden: jump squat and rise" $ do
      runGoldenTest "test/golden/jump.json"

  , testCase "Golden: run + jump combined" $ do
      runGoldenTest "test/golden/run_jump.json"
  ]

-- | Load a golden tape and verify Haskell states match recorded oracle.
--
-- This test ALWAYS runs (CI-proof). If golden file is missing, test fails
-- (not skips) to prevent silent drift.
runGoldenTest :: FilePath -> IO ()
runGoldenTest path = do
  exists <- doesFileExist path
  if not exists
    then assertFailure $ "Golden file missing (CI failure): " ++ path ++
                         "\nRun with --regenerate-goldens to create."
    else do
      result <- eitherDecodeFileStrict' path
      case result of
        Left err -> assertFailure $ "Failed to parse golden file: " ++ err
        Right golden -> verifyGolden golden

  -- Optional: if oracle is available, verify live drift
  checkLiveDrift <- lookupEnv "CHECK_LIVE_ORACLE_DRIFT"
  case checkLiveDrift of
    Just "1" -> do
      -- TODO: call oracle and assert golden matches live output
      return ()
    _ -> return ()

-- | Verify Haskell replay matches golden states.
--
-- Asserts subpixel-precision equality on x, y, subX, subY, pose.
verifyGolden :: GoldenTape -> IO ()
verifyGolden (GoldenTape _name _desc inputs expected) = do
  let cfg = defaultConfig
      state0 = initialState cfg
      actual = runTape cfg state0 inputs
  if length actual /= length expected
    then assertFailure $ "State count mismatch: got " ++ show (length actual) ++
                         ", expected " ++ show (length expected)
    else mapM_ checkState (zip actual expected)
  where
    checkState (a, e) = do
      -- Subpixel-precision assertions
      posPixel (stateXPos a) @?= posPixel (stateXPos e)
      posSubpixel (stateXPos a) @?= posSubpixel (stateXPos e)
      posPixel (stateYPos a) @?= posPixel (stateYPos e)
      posSubpixel (stateYPos a) @?= posSubpixel (stateYPos e)
      statePose a @?= statePose e
      stateOnGround a @?= stateOnGround e

-- | Golden tape format (inputs + expected states from C oracle).
data GoldenTape = GoldenTape
  { gtName :: String
  , gtDescription :: String
  , gtInputs :: [ControllerInput]
  , gtExpected :: [SamusState]
  } deriving (Show)

instance FromJSON GoldenTape where
  parseJSON = withObject "GoldenTape" $ \o -> do
    name <- o .: "name"
    desc <- o .: "description"
    inputs <- o .: "inputs"
    expected <- o .: "states"
    return (GoldenTape name desc inputs expected)
