-- | Debug: minimal jump squat test
module Test.JumpDebug where

import Physics.SM
import Data.Bits ((.|.))

main :: IO ()
main = do
  let cfg = defaultConfig
      inputPress = ControllerInput btnA (ButtonMask 0)
      inputHold = ControllerInput btnA btnA
      
      state0 = initialState cfg
      state1 = step cfg inputPress state0
      state2 = step cfg inputHold state1
      state3 = step cfg inputHold state2
      state4 = step cfg inputHold state3
      
  putStrLn $ "Initial: squat=" ++ show (stateJumpSquatFrames state0) ++ " onGround=" ++ show (stateOnGround state0)
  putStrLn $ "Frame 1: squat=" ++ show (stateJumpSquatFrames state1) ++ " onGround=" ++ show (stateOnGround state1)
  putStrLn $ "Frame 2: squat=" ++ show (stateJumpSquatFrames state2) ++ " onGround=" ++ show (stateOnGround state2)
  putStrLn $ "Frame 3: squat=" ++ show (stateJumpSquatFrames state3) ++ " onGround=" ++ show (stateOnGround state3)
  putStrLn $ "Frame 4: squat=" ++ show (stateJumpSquatFrames state4) ++ " onGround=" ++ show (stateOnGround state4)
  putStrLn $ "         yVel=" ++ show (velPixel (stateYVel state4))
  putStrLn $ "         yPos=" ++ show (unPixel (posPixel (stateYPos state4)))
