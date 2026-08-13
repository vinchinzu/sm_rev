-- | Properties: Determinism only (no oracle available).
module Test.Properties (tests) where

import Physics.SM
import Test.Tasty (TestTree, testGroup)
import Test.Tasty.HUnit ((@?=), testCase)

tests :: TestTree
tests = testGroup "Properties"
  [ testCase "Deterministic replay" $ do
      let cfg = defaultConfig
          state0 = initialState cfg
          input = ControllerInput (ButtonMask 0x081) (ButtonMask 0)
          inputs = replicate 10 input
          states1 = runTape cfg state0 inputs
          states2 = runTape cfg state0 inputs
      states1 @?= states2
  
  , testCase "B+Right produces rightward motion" $ do
      let cfg = defaultConfig
          state0 = initialState cfg
          input = ControllerInput (ButtonMask 0x081) (ButtonMask 0)  -- B=0x001 + Right=0x080
          -- Need ~410 frames for 1 pixel at accel 0x00a0: 0x00a0 * 410 = 66560 > 65536
          states = runTape cfg state0 (replicate 500 input)
          finalX = posPixel (stateXPos (last states))
      finalX > posPixel (stateXPos state0) @?= True
  ]
