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
  ]
