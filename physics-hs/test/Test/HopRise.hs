-- | Test that Samus Y decreases after jump (rises, not falls).
module Test.HopRise (tests) where

import Physics.SM
import Test.Tasty (TestTree, testGroup)
import Test.Tasty.HUnit (testCase, assertBool, (@?=))

tests :: TestTree
tests = testGroup "Hop Rise"
  [ testCase "After jump squat, samus_y pixel < groundY" $ do
      let cfg = defaultConfig
          groundY = unPixel (cfgGroundY cfg)
          -- Press A to start jump squat
          inputPress = ControllerInput btnA (ButtonMask 0)
          inputHold = ControllerInput btnA btnA
          
          -- Start on ground
          state0 = initialState cfg
          
          -- Jump squat (4 frames)
          state1 = step cfg inputPress state0
          state2 = step cfg inputHold state1
          state3 = step cfg inputHold state2
          state4 = step cfg inputHold state3
          
          -- After jump squat, should be airborne with upward velocity
          stateAfterSquat = state4
          
          -- Run a few more frames to see rise
          state5 = step cfg inputHold stateAfterSquat
          state6 = step cfg inputHold state5
          state7 = step cfg inputHold state6
          
          peakState = state7
          peakY = unPixel (posPixel (stateYPos peakState))
      
      -- After jump squat, Samus should have launched
      stateOnGround stateAfterSquat @?= False
      
      -- Y velocity should be negative (upward)
      assertBool "Y velocity should be negative (upward)" $
        velPixel (stateYVel stateAfterSquat) < 0
      
      -- After a few frames, Y should be less than groundY (Samus rose)
      assertBool ("Peak Y (" ++ show peakY ++ ") should be < groundY (" ++ show groundY ++ ")") $
        peakY < groundY
  
  , testCase "Y velocity starts negative after jump" $ do
      let cfg = defaultConfig
          inputPress = ControllerInput btnA (ButtonMask 0)
          inputHold = ControllerInput btnA btnA
          state0 = initialState cfg
          
          -- Complete jump squat
          state1 = step cfg inputPress state0
          state2 = step cfg inputHold state1
          state3 = step cfg inputHold state2
          state4 = step cfg inputHold state3
          
          yVel = velPixel (stateYVel state4)
      
      assertBool ("Y velocity (" ++ show yVel ++ ") should be negative") $
        yVel < 0
  ]
