-- Quick script to generate golden test data
{-# LANGUAGE OverloadedStrings #-}
import Data.Aeson
import qualified Data.ByteString.Lazy.Char8 as BL
import Physics.SM
import Physics.SM.Types
import Physics.SM.FFI

main :: IO ()
main = do
  let cfg = defaultConfig
      state0 = initialState cfg
  
  -- Golden 1: Ground run right (B+Right for 60 frames)
  let rightInput = 0x81 :: Word8  -- Right (0x80) + B (0x01)
      runInputs = replicate 60 (fromFrameInput (FrameInput rightInput) (ControllerInput (ButtonMask 0) (ButtonMask 0)))
      runStates = scanl (flip (step cfg)) state0 runInputs
  
  BL.writeFile "test/golden/run_right.json" $ encode $ object
    [ "name" .= ("ground_run_right_60frames" :: String)
    , "description" .= ("Ground run holding B+Right for 60 frames" :: String)
    , "inputs" .= replicate 60 (object ["frameInputPacked" .= rightInput])
    , "states" .= runStates
    ]
  
  -- Golden 2: Jump squat and rise (standing, no horizontal input)
  let jumpInput = 0x01 :: Word8  -- Just B (jump button in this simplified model)
      jumpInputs = replicate 10 (fromFrameInput (FrameInput jumpInput) (ControllerInput (ButtonMask 0) (ButtonMask 0)))
      jumpStates = scanl (flip (step cfg)) state0 jumpInputs
  
  BL.writeFile "test/golden/jump.json" $ encode $ object
    [ "name" .= ("jump_squat_and_rise" :: String)
    , "description" .= ("Press B for jump squat (4 frames) then rise" :: String)
    , "inputs" .= replicate 10 (object ["frameInputPacked" .= jumpInput])
    , "states" .= jumpStates
    ]
  
  -- Golden 3: Run + jump combined
  let runJumpInputs = replicate 20 (fromFrameInput (FrameInput 0x80) (ControllerInput (ButtonMask 0) (ButtonMask 0)))  -- Right only
                   ++ replicate 20 (fromFrameInput (FrameInput 0x81) (ControllerInput (ButtonMask 0) (ButtonMask 0)))  -- Right + B (jump)
      runJumpStates = scanl (flip (step cfg)) state0 runJumpInputs
  
  BL.writeFile "test/golden/run_jump.json" $ encode $ object
    [ "name" .= ("run_jump_combined" :: String)
    , "description" .= ("Run right then jump (combined horizontal + vertical)" :: String)
    , "inputs" .= (replicate 20 (object ["frameInputPacked" .= (0x80 :: Word8)])
                ++ replicate 20 (object ["frameInputPacked" .= (0x81 :: Word8)]))
    , "states" .= runJumpStates
    ]
  
  putStrLn "Golden files generated in test/golden/"
