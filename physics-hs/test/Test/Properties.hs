-- | Property-based tests (determinism, etc).
module Test.Properties (tests) where

import Physics.SM
import Test.Tasty (TestTree, testGroup)
import Test.Tasty.QuickCheck (testProperty)

tests :: TestTree
tests = testGroup "Properties"
  [ testProperty "Determinism: same tape produces same states" prop_determinism
  , testProperty "Zero input is stationary" prop_zeroInputStationary
  ]

-- | Same tape replayed twice yields identical states.
prop_determinism :: [Word16] -> Bool
prop_determinism buttonSeq =
  let cfg = defaultConfig
      inputs = map (\b -> ControllerInput (ButtonMask b) (ButtonMask 0)) buttonSeq
      state0 = initialState cfg
      run1 = runTape cfg state0 inputs
      run2 = runTape cfg state0 inputs
  in run1 == run2

-- | Zero input (no buttons) on standing state should not move.
prop_zeroInputStationary :: Bool
prop_zeroInputStationary =
  let cfg = defaultConfig
      input = ControllerInput (ButtonMask 0) (ButtonMask 0)
      state0 = initialState cfg
      state1 = step cfg input state0
  in stateXPos state1 == stateXPos state0 && stateYPos state1 == stateYPos state0

import Data.Word (Word16)
