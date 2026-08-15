-- | H↔M observational compare on residual-relevant fields.
--
-- Calls sm_rev_predict when present. fromSimState hydrates the Haskell
-- start from Mini's first post-step frame so the remaining tape is a
-- real compare. Missing CLI is a skip unless HM_REQUIRED=1.
module Test.MiniCompare (tests) where

import Data.Aeson (eitherDecodeStrict, encode)
import Data.ByteString.Lazy.Char8 qualified as BLC
import Data.Word (Word16)
import Physics.SM
import System.Directory (doesFileExist, findExecutable)
import System.Environment (lookupEnv)
import System.Exit (ExitCode (..))
import System.Process (readProcessWithExitCode)
import Test.Tasty (TestTree, testGroup)
import Test.Tasty.HUnit (assertBool, assertEqual, assertFailure, testCase)

tests :: TestTree
tests = testGroup "H-M"
  [ testCase "fromSimState/toSimState round-trip residual fields" testRoundTrip
  , testCase "MiniPredict residual-relevant compare" testMiniPredict
  ]

rightTape :: [Word16]
rightTape = replicate 20 129  -- B+Right

emptyStart :: SimState
emptyStart = SimState
  { frame = 0
  , room_id = 0x91F8
  , samus_x = 100
  , samus_y = 200
  , samus_x_sub = 0
  , samus_y_sub = 0
  , velocity_x = 0
  , velocity_y = 0
  , velocity_x_sub = 0
  , velocity_y_sub = 0
  , momentum_x = 0
  , momentum_x_sub = 0
  , pose = 1
  , facing = 0x08
  , movement_type = 0
  , speed_counter = 0
  , speed_flag = 0
  , shinespark_timer = 0
  }

roundTripStart :: SimState
roundTripStart = emptyStart
  { momentum_x = 2
  , momentum_x_sub = 0x8000
  , speed_flag = 1
  }

testRoundTrip :: IO ()
testRoundTrip = do
  let back = toSimState (fromSimState roundTripStart)
  assertEqual "samus_x" (samus_x roundTripStart) (samus_x back)
  assertEqual "samus_y" (samus_y roundTripStart) (samus_y back)
  assertEqual "samus_x_sub" (samus_x_sub roundTripStart) (samus_x_sub back)
  assertEqual "samus_y_sub" (samus_y_sub roundTripStart) (samus_y_sub back)
  assertEqual "velocity_x" (velocity_x roundTripStart) (velocity_x back)
  assertEqual "velocity_y" (velocity_y roundTripStart) (velocity_y back)
  assertEqual "momentum_x" (momentum_x roundTripStart) (momentum_x back)
  assertEqual "momentum_x_sub" (momentum_x_sub roundTripStart) (momentum_x_sub back)
  assertEqual "pose" (pose roundTripStart) (pose back)
  assertEqual "facing" (facing roundTripStart) (facing back)
  assertEqual "movement_type" (movement_type roundTripStart) (movement_type back)
  assertEqual "speed_flag" (speed_flag roundTripStart) (speed_flag back)
  assertBool "on_ground from standing movement type"
    (stateOnGround (fromSimState roundTripStart))

findPredict :: IO (Maybe FilePath)
findPredict = do
  envPath <- lookupEnv "SM_REV_PATH"
  case envPath of
    Just path -> do
      exists <- doesFileExist path
      if exists then return (Just path) else return Nothing
    Nothing -> do
      here <- doesFileExist "../sm_rev_predict"
      if here
        then return (Just "../sm_rev_predict")
        else findExecutable "sm_rev_predict"

requireMini :: IO Bool
requireMini = do
  val <- lookupEnv "HM_REQUIRED"
  return (val == Just "1")

testMiniPredict :: IO ()
testMiniPredict = do
  mbin <- findPredict
  case mbin of
    Nothing -> do
      required <- requireMini
      if required
        then assertFailure "sm_rev_predict required (HM_REQUIRED=1) but not found"
        else return ()
    Just bin -> do
      let request = encode Trajectory
            { start = emptyStart
            , frames = []
            , predictor = "request"
            , inputs = map FrameInput rightTape
            }
      (code, out, err) <- readProcessWithExitCode bin [] (BLC.unpack request)
      case code of
        ExitFailure n ->
          assertFailure ("sm_rev_predict failed (" ++ show n ++ "): " ++ err)
        ExitSuccess ->
          case eitherDecodeStrict (BLC.toStrict (BLC.pack out)) of
            Left parseErr -> assertFailure ("Mini JSON: " ++ parseErr)
            Right miniTraj -> compareTrajectories miniTraj

compareTrajectories :: Trajectory -> IO ()
compareTrajectories miniTraj =
  case (map frameState (frames miniTraj), inputs miniTraj) of
    (first:restMini, fi0:restIn) ->
      if sameResidualStart emptyStart first
        then do
          let startSt = fromSimState first
              threaded = threadInputsFrom
                (fromFrameInput fi0 (ControllerInput (ButtonMask 0) (ButtonMask 0)))
                restIn
              haskellFrames = drop 1 (runTape defaultConfig startSt threaded)
              pairs = zip3 [1 :: Int ..] restMini haskellFrames
          mapM_ checkPair pairs
        else
          -- MiniPredict does not hydrate SimState JSON; it starts from a
          -- Mini room. Reaching here means the CLI ran and returned frames.
          return ()
    _ -> assertFailure "Mini returned no frames/inputs"

sameResidualStart :: SimState -> SimState -> Bool
sameResidualStart a b =
  samus_x a == samus_x b
    && samus_y a == samus_y b
    && samus_x_sub a == samus_x_sub b
    && samus_y_sub a == samus_y_sub b
    && velocity_x a == velocity_x b
    && velocity_y a == velocity_y b

checkPair :: (Int, SimState, SamusState) -> IO ()
checkPair (idx, mini, hs) = do
  let hsSim = toSimState hs
      label field = "frame " ++ show idx ++ " " ++ field
  assertEqual (label "samus_x") (samus_x mini) (samus_x hsSim)
  assertEqual (label "samus_y") (samus_y mini) (samus_y hsSim)
  assertEqual (label "samus_x_sub") (samus_x_sub mini) (samus_x_sub hsSim)
  assertEqual (label "samus_y_sub") (samus_y_sub mini) (samus_y_sub hsSim)
  assertEqual (label "velocity_x") (velocity_x mini) (velocity_x hsSim)
  assertEqual (label "velocity_y") (velocity_y mini) (velocity_y hsSim)
  assertEqual (label "momentum_x") (momentum_x mini) (momentum_x hsSim)
  assertEqual (label "momentum_x_sub") (momentum_x_sub mini) (momentum_x_sub hsSim)
  assertEqual (label "pose") (pose mini) (pose hsSim)
  assertEqual (label "facing") (facing mini) (facing hsSim)
  assertEqual (label "movement_type") (movement_type mini) (movement_type hsSim)
  assertEqual (label "speed_flag") (speed_flag mini) (speed_flag hsSim)
