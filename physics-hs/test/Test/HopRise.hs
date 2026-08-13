-- | Test that hops actually rise (peak Y < groundY).
module Test.HopRise (tests) where

import Physics.SM
import Test.Tasty (TestTree, testGroup)
import Test.Tasty.HUnit ((@?=), assertBool, testCase)

tests :: TestTree
tests = testGroup "Hop Rise Verification"
  [ testCase "Jump moves upward (Y decreases)" $ do
      let cfg = defaultConfig
          state0 = initialState cfg
          groundY = unPixel (cfgGroundY cfg)
          
          -- Press A to start jump
          inputA = ControllerInput (ButtonMask 0x100) (ButtonMask 0)
          
          -- Run for 10 frames (squat 4 + jump 6)
          states = runTape cfg state0 (replicate 10 inputA)
          
          -- Find peak Y (minimum Y value, since lower Y = higher on screen)
          yValues = map (unPixel . posPixel . stateYPos) states
          peakY = minimum yValues
      
      -- Assert: peak Y < groundY (jumped up from floor)
      assertBool
        ("Peak Y (" ++ show peakY ++ ") should be < groundY (" ++ show groundY ++ ")")
        (peakY < groundY)
  
  , testCase "Y velocity starts negative on jump" $ do
      let cfg = defaultConfig
          state0 = initialState cfg
          inputA = ControllerInput (ButtonMask 0x100) (ButtonMask 0)
          
          -- Run until jump fires (5 frames: 4 squat + 1 jump)
          states = runTape cfg state0 (replicate 5 inputA)
          finalState = last states
          yVel = velPixel (stateYVel finalState)
      
      -- Assert: Y velocity is negative (upward)
      assertBool
        ("Y velocity (" ++ show yVel ++ ") should be negative (upward)")
        (yVel < 0)
  ]
