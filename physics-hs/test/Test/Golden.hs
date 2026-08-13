-- | Golden tape tests: verify against recorded C oracle outputs.
--
-- To regenerate golden tapes:
--   1. Run C oracle with test inputs: make mini-test
--   2. Dump state snapshots to JSON: (custom C helper or manual recording)
--   3. Place golden files in test/golden/*.json
--   4. Run: cabal test
module Test.Golden (tests) where

import Data.Aeson (eitherDecodeFileStrict')
import Data.ByteString.Lazy qualified as BL
import Physics.SM
import System.Directory (doesFileExist)
import Test.Tasty (TestTree, testGroup)
import Test.Tasty.HUnit ((@?=), assertBool, assertFailure, testCase)

tests :: TestTree
tests = testGroup "Golden Tapes"
  [ testCase "Golden tape: ground run right" $ do
      runGoldenTest "test/golden/run_right.json"

  , testCase "Golden tape: jump" $ do
      runGoldenTest "test/golden/jump.json"

  , testCase "Golden tape: run + jump" $ do
      runGoldenTest "test/golden/run_jump.json"
  ]

-- | Load a golden tape and verify Haskell states match C oracle.
runGoldenTest :: FilePath -> IO ()
runGoldenTest path = do
  exists <- doesFileExist path
  if not exists
    then assertFailure $ "Golden file missing: " ++ path ++
                         "\nRun C oracle to generate: make mini-test (see README.md)"
    else do
      result <- eitherDecodeFileStrict' path
      case result of
        Left err -> assertFailure $ "Failed to parse golden file: " ++ err
        Right golden -> verifyGolden golden

-- | Verify Haskell replay matches golden states.
verifyGolden :: GoldenTape -> IO ()
verifyGolden (GoldenTape inputs expected) = do
  let cfg = defaultConfig
      state0 = initialState cfg
      actual = runTape cfg state0 inputs
  if length actual /= length expected
    then assertFailure $ "State count mismatch: got " ++ show (length actual) ++
                         ", expected " ++ show (length expected)
    else mapM_ checkState (zip actual expected)
  where
    checkState (a, e) = do
      stateXPos a @?= stateXPos e
      stateYPos a @?= stateYPos e
      stateXVel a @?= stateXVel e
      stateYVel a @?= stateYVel e
      stateOnGround a @?= stateOnGround e

-- | Golden tape format (inputs + expected C oracle states).
data GoldenTape = GoldenTape
  { gtInputs :: [ControllerInput]
  , gtExpected :: [SamusState]
  } deriving (Show)

instance FromJSON GoldenTape where
  parseJSON = withObject "GoldenTape" $ \o -> do
    inputs <- o .: "inputs"
    expected <- o .: "states"
    return (GoldenTape inputs expected)

import Data.Aeson (FromJSON, withObject, (.:))
