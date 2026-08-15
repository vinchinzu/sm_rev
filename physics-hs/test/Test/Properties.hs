-- | Determinism: HUnit fixtures plus one QuickCheck tape property.
module Test.Properties (tests) where

import Data.Word (Word16)
import Physics.SM
import Test.QuickCheck (Gen, Property, choose, forAll, vectorOf)
import Test.Tasty (TestTree, testGroup)
import Test.Tasty.HUnit ((@?=), testCase)
import Test.Tasty.QuickCheck (testProperty)

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

  , testProperty "random button tape is deterministic" propRandomTapeDeterministic
  ]

propRandomTapeDeterministic :: Property
propRandomTapeDeterministic =
  forAll genButtonTape $ \inputs ->
    let s0 = initialState defaultConfig
    in runTape defaultConfig s0 inputs == runTape defaultConfig s0 inputs

genButtonTape :: Gen [ControllerInput]
genButtonTape = do
  n <- choose (1, 40)
  masks <- vectorOf n (choose (0, 0xFFF) :: Gen Word16)
  pure (threadMasks masks)

threadMasks :: [Word16] -> [ControllerInput]
threadMasks = go (ButtonMask 0)
  where
    go _ [] = []
    go prev (m:ms) =
      let cur = ControllerInput (ButtonMask m) prev
      in cur : go (ButtonMask m) ms

tapeEnd :: SamusState -> [SamusState] -> SamusState
tapeEnd fallback [] = fallback
tapeEnd _ xs = last xs
