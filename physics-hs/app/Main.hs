-- | CLI for running Haskell physics predictions.
--
-- INPUT: {"start": <SimState>, "inputs": [<FrameInput>, ...]}
-- OUTPUT: {"start": <SimState>, "frames": [<TrajectoryFrame>, ...], "predictor": "haskell-v1", "inputs": [...]}
--
-- Matches retro_rl@66836f5 Trajectory.to_dict() wire format.
module Main (main) where

import Data.Aeson (eitherDecode, encode)
import Data.ByteString.Lazy qualified as BL
import Physics.SM
import Physics.SM.FFI
import System.Environment (getArgs)
import System.Exit (die)
import System.IO (hPutStrLn, stderr)

main :: IO ()
main = do
  args <- getArgs
  case args of
    [] -> runStdin
    ["--tape", inputPath, "--output", outputPath] -> runFile inputPath outputPath
    _ -> usage

usage :: IO ()
usage = die $ unlines
  [ "Usage:"
  , "  sm-predict                             # Read JSON from stdin"
  , "  sm-predict --tape IN --output OUT      # Read/write files"
  , ""
  , "Input JSON format (retro_rl@66836f5):"
  , "  {\"start\": <SimState>, \"inputs\": [<FrameInput>, ...]}"
  , ""
  , "Output JSON format (Trajectory.to_dict):"
  , "  {\"start\": <SimState>, \"frames\": [<TrajectoryFrame>, ...], \"predictor\": \"haskell-v1\", \"inputs\": [...]}"
  ]

runStdin :: IO ()
runStdin = do
  input <- BL.getContents
  case eitherDecode input of
    Left err -> die $ "Failed to parse input: " ++ err
    Right req -> do
      let result = processRequest req
      BL.putStr (encode result)

runFile :: FilePath -> FilePath -> IO ()
runFile inputPath outputPath = do
  input <- BL.readFile inputPath
  case eitherDecode input of
    Left err -> die $ "Failed to parse input: " ++ err
    Right req -> do
      let result = processRequest req
      BL.writeFile outputPath (encode result)
      hPutStrLn stderr $ "Wrote trajectory with " ++ show (length (frames result)) ++ " frames to " ++ outputPath

-- | Run the Haskell physics kernel.
--
-- Convert retro_rl SimState/FrameInput to internal types, run steps, convert back.
processRequest :: Trajectory -> Trajectory
processRequest (Trajectory startSim _ _ inputFrames) =
  let cfg = defaultConfig
      startState = fromSimState startSim
      inputs = map (\fi -> fromFrameInput fi (ControllerInput (ButtonMask 0) (ButtonMask 0))) inputFrames
      states = runTape cfg startState inputs
      trajFrames = map (\s -> TrajectoryFrame (toSimState s) Nothing) states
  in Trajectory
       { start = startSim
       , frames = trajFrames
       , predictor = "haskell-v1-collision-flat-floor-y200"
       , inputs = inputFrames
       }
