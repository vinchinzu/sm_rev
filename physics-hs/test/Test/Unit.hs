-- | Unit tests for physics primitives (accel, friction, jump squat).
module Test.Unit (tests) where

import Data.Bits ((.&.), (.|.))
import Physics.SM
import Physics.SM.Gravity (subVelocity)
import Test.Tasty (TestTree, testGroup)
import Test.Tasty.HUnit ((@?=), testCase)

tests :: TestTree
tests = testGroup "Unit"
  [ testAcceleration
  , testFriction
  , testJumpSquat
  , testGravity
  ]

testAcceleration :: TestTree
testAcceleration = testGroup "Horizontal Acceleration"
  [ testCase "B+Right accelerates from zero" $ do
      let cfg = defaultConfig
          input = ControllerInput (btnB .|. btnRight) (ButtonMask 0)
          state0 = initialState cfg
          state1 = step cfg input state0
      velPixel (stateXVel state1) @?= Pixel 0
      unSubpixel (velSubpixel (stateXVel state1)) @?= 0x00a0  -- cfgRunAccel
      stateAccelMode state1 @?= AccelAccelerating

  , testCase "B+Right reaches max speed after N frames" $ do
      let cfg = defaultConfig
          input = ControllerInput (btnB .|. btnRight) (btnB .|. btnRight)
          state0 = initialState cfg
          -- Accel is 0.00a0 per frame, max is 3.0000
          -- Need ceiling(3.0000 / 0.00a0) = ceiling(49152) = 49152 frames
          -- For simplicity, run 200 frames
          states = take 201 $ iterate (step cfg input) state0
          finalState = last states
      velPixel (stateXVel finalState) @?= Pixel 3  -- cfgRunMaxSpeed
      velSubpixel (stateXVel finalState) @?= Subpixel 0x0000

  , testCase "Releasing B stops acceleration" $ do
      let cfg = defaultConfig
          inputAccel = ControllerInput (btnB .|. btnRight) (ButtonMask 0)
          inputCoast = ControllerInput btnRight btnRight
          state0 = initialState cfg
          state1 = step cfg inputAccel state0
          state2 = step cfg inputCoast state1
      stateAccelMode state2 @?= AccelNone
      stateXVel state2 @?= zeroVelocity  -- Immediate stop (no decel for now)
  ]

testFriction :: TestTree
testFriction = testGroup "Friction/Decel"
  [ testCase "Vanilla config has zero decel" $ do
      unSubpixel (velSubpixel (cfgRunDecel defaultConfig)) @?= 0x0000
  ]

testJumpSquat :: TestTree
testJumpSquat = testGroup "Jump Squat"
  [ testCase "Pressing A starts jump squat" $ do
      let cfg = defaultConfig
          input = ControllerInput btnA (ButtonMask 0)
          state0 = (initialState cfg) { stateOnGround = True }
          state1 = step cfg input state0
      stateJumpSquatFrames state1 @?= 1

  , testCase "Jump squat lasts 4 frames" $ do
      let cfg = defaultConfig
          inputPress = ControllerInput btnA (ButtonMask 0)
          inputHold = ControllerInput btnA btnA
          state0 = (initialState cfg) { stateOnGround = True }
          state1 = step cfg inputPress state0
          state2 = step cfg inputHold state1
          state3 = step cfg inputHold state2
          state4 = step cfg inputHold state3
      stateJumpSquatFrames state1 @?= 1
      stateJumpSquatFrames state2 @?= 2
      stateJumpSquatFrames state3 @?= 3
      stateOnGround state3 @?= True  -- Still on ground
      stateOnGround state4 @?= False  -- Jump fires on frame 4

  , testCase "Jump fires with upward velocity" $ do
      let cfg = defaultConfig
          inputPress = ControllerInput btnA (ButtonMask 0)
          inputHold = ControllerInput btnA btnA
          state0 = (initialState cfg) { stateOnGround = True }
          states = take 5 $ iterate (step cfg inputHold) (step cfg inputPress state0)
          finalState = last states
      stateVerticalDir finalState @?= VDirRising
      velPixel (stateYVel finalState) @?= Pixel 4  -- Air jump velocity
      unSubpixel (velSubpixel (stateYVel finalState)) @?= 0xe000
  ]

testGravity :: TestTree
testGravity = testGroup "Gravity"
  [ testCase "Gravity decelerates upward velocity" $ do
      let cfg = defaultConfig
          input = ControllerInput btnA btnA  -- Hold A
          state0 = (initialState cfg)
               { stateOnGround = False
               , stateVerticalDir = VDirRising
               , stateYVel = Velocity (Pixel 2) (Subpixel 0)
               }
          state1 = step cfg input state0
          expectedVel = subVelocity (Velocity (Pixel 2) (Subpixel 0))
                                    (Velocity (Pixel 0) (Subpixel 0x1c00))  -- Gravity
      stateYVel state1 @?= expectedVel

  , testCase "Gravity accelerates downward velocity" $ do
      let cfg = defaultConfig
          input = ControllerInput (ButtonMask 0) (ButtonMask 0)
          state0 = (initialState cfg)
               { stateOnGround = False
               , stateVerticalDir = VDirFalling
               , stateYVel = Velocity (Pixel 1) (Subpixel 0)
               }
          state1 = step cfg input state0
          expectedVel = addVelocity (Velocity (Pixel 1) (Subpixel 0))
                                    (Velocity (Pixel 0) (Subpixel 0x1c00))
      stateYVel state1 @?= expectedVel

  , testCase "Terminal velocity caps falling speed" $ do
      let cfg = defaultConfig
          input = ControllerInput (ButtonMask 0) (ButtonMask 0)
          state0 = (initialState cfg)
               { stateOnGround = False
               , stateVerticalDir = VDirFalling
               , stateYVel = Velocity (Pixel 5) (Subpixel 0)  -- At terminal
               }
          state1 = step cfg input state0
      stateYVel state1 @?= Velocity (Pixel 5) (Subpixel 0)  -- Unchanged
  ]
