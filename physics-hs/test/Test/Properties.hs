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
  
  , testCase "Right produces rightward motion" $ do
      let cfg = defaultConfig
          state0 = initialState cfg
          input = ControllerInput (ButtonMask 0x080) (ButtonMask 0)
          states = runTape cfg state0 (replicate 20 input)
          finalX = posPixel (stateXPos (tapeEnd state0 states))
      finalX > posPixel (stateXPos state0) @?= True
  , testCase "Same tape is deterministic with extra run" $ do
      let cfg = defaultConfig
          state0 = initialState cfg
          inputs = replicate 30 (ControllerInput (ButtonMask 0x081) (ButtonMask 0x081))
      runTape cfg state0 inputs @?= runTape cfg state0 inputs
  ]

tapeEnd :: SamusState -> [SamusState] -> SamusState
tapeEnd fallback [] = fallback
tapeEnd _ (x:xs) = foldl (\_ y -> y) x xs
