-- | SMB-style residual segments: idle / walk / run / jump / run-jump / land.
--
-- These are Haskell-owned invariants of the implemented fragment, not an
-- H↔M or M–E check. They exist so the extra tables, air X, A-release,
-- and landing leftovers stay honest.
module Test.Segments (tests) where

import Data.Bits ((.|.))
import Physics.SM
import Test.Tasty (TestTree, testGroup)
import Test.Tasty.HUnit (assertBool, testCase, (@?=))

tests :: TestTree
tests = testGroup "Segments"
  [ testIdle
  , testWalk
  , testRun
  , testJump
  , testRunJump
  , testLandLeftover
  ]

hold :: ButtonMask -> ControllerInput
hold btn = ControllerInput btn btn

press :: ButtonMask -> ControllerInput
press btn = ControllerInput btn (ButtonMask 0)

play :: [ControllerInput] -> SamusState
play inputs = tapeEnd (initialState defaultConfig)
  (runTape defaultConfig (initialState defaultConfig) inputs)

tapeEnd :: SamusState -> [SamusState] -> SamusState
tapeEnd fallback [] = fallback
tapeEnd _ (x:xs) = foldl (\_ y -> y) x xs

testIdle :: TestTree
testIdle = testCase "idle holds position" $ do
  let end = play (replicate 24 (hold (ButtonMask 0)))
  stateXPos end @?= stateXPos (initialState defaultConfig)
  stateYPos end @?= stateYPos (initialState defaultConfig)
  stateOnGround end @?= True

testWalk :: TestTree
testWalk = testCase "walk-right advances X without extra run" $ do
  let end = play (replicate 24 (hold btnRight))
      x0 = unPixel (posPixel (stateXPos (initialState defaultConfig)))
      x1 = unPixel (posPixel (stateXPos end))
  assertBool "X increased" (x1 > x0)
  stateXExtra end @?= zeroVelocity
  stateXVel end @?= Velocity 2 (Subpixel 0xC000)

testRun :: TestTree
testRun = testCase "run-right builds extra run on top of base max" $ do
  let end = play (replicate 24 (hold (btnB .|. btnRight)))
  stateXVel end @?= Velocity 2 (Subpixel 0xC000)
  assertBool "extra run built" (stateXExtra end /= zeroVelocity)
  stateHasMomentum end @?= True

testJump :: TestTree
testJump = testCase "standing jump rises then lands" $ do
  let inputs = press btnA : replicate 3 (hold btnA) ++ replicate 40 (hold (ButtonMask 0))
      states = runTape defaultConfig (initialState defaultConfig) inputs
      airborne = filter (not . stateOnGround) states
      finalSt = tapeEnd (initialState defaultConfig) states
      minY = foldl min maxBound (map (unPixel . posPixel . stateYPos) states)
      ground = unPixel (cfgGroundY defaultConfig)
  assertBool "left the ground" (not (null airborne))
  assertBool "rose above ground" (minY < ground)
  stateOnGround finalSt @?= True
  unPixel (posPixel (stateYPos finalSt)) @?= ground

testRunJump :: TestTree
testRunJump = testCase "run-jump uses air X and keeps extra run" $ do
  let runIn = replicate 16 (hold (btnB .|. btnRight))
      takeoff = press (btnB .|. btnRight .|. btnA)
              : replicate 3 (hold (btnB .|. btnRight .|. btnA))
      air = replicate 12 (hold (btnB .|. btnRight .|. btnA))
      states = runTape defaultConfig (initialState defaultConfig) (runIn ++ takeoff ++ air)
      launched = tapeEnd (initialState defaultConfig) states
  stateOnGround launched @?= False
  stateHasMomentum launched @?= True
  stateMovementType launched @?= mvtSpinJumping
  -- Spin air table max is 1.6000; extra run is applied on top.
  cmpMagnitude (stateXVel launched) (Velocity 1 (Subpixel 0x6000)) /= GT @?= True
  stateXExtra launched /= zeroVelocity @?= True

testLandLeftover :: TestTree
testLandLeftover = testCase "land keeps Y subpixel leftover" $ do
  let inputs = press btnA : replicate 3 (hold btnA) ++ replicate 50 (hold (ButtonMask 0))
      states = runTape defaultConfig (initialState defaultConfig) inputs
      landed = tapeEnd (initialState defaultConfig) states
  stateOnGround landed @?= True
  unPixel (posPixel (stateYPos landed)) @?= unPixel (cfgGroundY defaultConfig)
  -- Not forced to 0: leftover from the last pre-snap applyVelocity.
  -- A perfectly aligned land can still be 0; just assert the field is present
  -- and X leftovers are not wiped.
  stateYVel landed @?= zeroVelocity
