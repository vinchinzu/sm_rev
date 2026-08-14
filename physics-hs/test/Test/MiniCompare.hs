-- | H↔M observational compare on residual-relevant fields.
--
-- Uses MiniPredict / MiniStep via sm_rev_predict when present. fromSimState
-- hydrates the Haskell start from Mini's first post-step frame so the
-- remaining tape is a real compare, not a fiction.
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

testRoundTrip :: IO ()
testRoundTrip = do
  let back = toSimState (fromSimState emptyStart)
  assertEqual "samus_x" (samus_x emptyStart) (samus_x back)
  assertEqual "samus_y" (samus_y emptyStart) (samus_y back)
  assertEqual "samus_x_sub" (samus_x_sub emptyStart) (samus_x_sub back)
  assertEqual "samus_y_sub" (samus_y_sub emptyStart) (samus_y_sub back)
  assertEqual "velocity_x" (velocity_x emptyStart) (velocity_x back)
  assertEqual "velocity_y" (velocity_y emptyStart) (velocity_y back)
  assertEqual "pose" (pose emptyStart) (pose back)
  assertEqual "facing" (facing emptyStart) (facing back)
  assertEqual "movement_type" (movement_type emptyStart) (movement_type back)
  assertBool "on_ground from standing movement type"
    (stateOnGround (fromSimState emptyStart))

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

testMiniPredict :: IO ()
testMiniPredict = do
  mbin <- findPredict
  case mbin of
    Nothing ->
      -- Predict CLI is an optional H↔M hook, not a Haskell-fragment gate.
      return ()
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
compareTrajectories miniTraj = do
  let miniFrames = map frameState (frames miniTraj)
  case miniFrames of
    [] -> assertFailure "Mini returned no frames"
    (firstFrame:restMini) -> do
      let startSt = fromSimState firstFrame
          restInputs = drop 1 (inputs miniTraj)
          threaded = threadInputs restInputs
          haskellFrames = drop 1 (runTape defaultConfig startSt threaded)
          pairs = zip3 [1 :: Int ..] restMini haskellFrames
      mapM_ checkPair pairs

threadInputs :: [FrameInput] -> [ControllerInput]
threadInputs = go (ControllerInput (ButtonMask 0) (ButtonMask 0))
  where
    go _ [] = []
    go prev (fi:rest) =
      let cur = fromFrameInput fi prev
      in cur : go cur rest

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
  assertEqual (label "pose") (pose mini) (pose hsSim)
  assertEqual (label "movement_type") (movement_type mini) (movement_type hsSim)
