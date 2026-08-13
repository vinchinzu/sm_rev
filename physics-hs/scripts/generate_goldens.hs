#!/usr/bin/env runhaskell
{-# LANGUAGE OverloadedStrings #-}

-- | Generate golden trajectory JSON files from Haskell step function.
--
-- Output format matches retro_rl@66836f5 Trajectory.to_dict()

import Data.Aeson (encode)
import Data.ByteString.Lazy.Char8 qualified as BL
import Physics.SM
import Physics.SM.FFI

main :: IO ()
main = do
  let cfg = defaultConfig
  
  -- 1. Ground run: B+Right for 60 frames
  putStrLn "Generating run_right.json..."
  let runInput = ControllerInput (ButtonMask 0x081) (ButtonMask 0)
      runInputs = replicate 60 runInput
      runStates = runTape cfg (initialState cfg) runInputs
      runTraj = Trajectory
        { start = toSimState (initialState cfg)
        , frames = map (\s -> TrajectoryFrame (toSimState s) Nothing) runStates
        , predictor = "haskell-v1"
        , inputs = map toFrameInput runInputs
        }
  BL.writeFile "test/golden/run_right.json" (encode runTraj)
  
  -- 2. Short hop: A held for 10 frames, then 30 frames falling/landing
  putStrLn "Generating short_hop.json..."
  let shortInputs = replicate 10 (ControllerInput (ButtonMask 0x100) (ButtonMask 0))
                 ++ replicate 30 (ControllerInput (ButtonMask 0) (ButtonMask 0))
      shortStates = runTape cfg (initialState cfg) shortInputs
      shortTraj = Trajectory
        { start = toSimState (initialState cfg)
        , frames = map (\s -> TrajectoryFrame (toSimState s) Nothing) shortStates
        , predictor = "haskell-v1"
        , inputs = map toFrameInput shortInputs
        }
  BL.writeFile "test/golden/short_hop.json" (encode shortTraj)
  
  -- 3. Full hop: A held for 40 frames
  putStrLn "Generating full_hop.json..."
  let fullInputs = replicate 40 (ControllerInput (ButtonMask 0x100) (ButtonMask 0))
                ++ replicate 20 (ControllerInput (ButtonMask 0) (ButtonMask 0))
      fullStates = runTape cfg (initialState cfg) fullInputs
      fullTraj = Trajectory
        { start = toSimState (initialState cfg)
        , frames = map (\s -> TrajectoryFrame (toSimState s) Nothing) fullStates
        , predictor = "haskell-v1"
        , inputs = map toFrameInput fullInputs
        }
  BL.writeFile "test/golden/full_hop.json" (encode fullTraj)
  
  putStrLn "Done. Generated 3 golden files."
