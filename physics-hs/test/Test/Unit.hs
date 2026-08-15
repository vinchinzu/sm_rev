-- | Unit tests for physics primitives (accel, extra-run, jump squat, gravity).
module Test.Unit (tests) where

import Data.Bits ((.|.))
import Physics.SM
import Test.Tasty (TestTree, testGroup)
import Test.Tasty.HUnit ((@?=), testCase)

tests :: TestTree
tests = testGroup "Unit"
  [ testAcceleration
  , testFriction
  , testJumpSquat
  , testGravity
  , testMomentum
  ]

testAcceleration :: TestTree
testAcceleration = testGroup "Horizontal Acceleration"
  [ testCase "Right walks from zero without B" $ do
      let cfg = defaultConfig
          input = ControllerInput btnRight (ButtonMask 0)
          state1 = step cfg input (initialState cfg)
      velPixel (stateXVel state1) @?= 0
      unSubpixel (velSubpixel (stateXVel state1)) @?= 0x3000
      stateAccelMode state1 @?= AccelAccelerating
      stateXExtra state1 @?= zeroVelocity
      stateMovementType state1 @?= mvtRunning

  , testCase "B+Right also builds extra run" $ do
      let cfg = defaultConfig
          input = ControllerInput (btnB .|. btnRight) (ButtonMask 0)
          state1 = step cfg input (initialState cfg)
      velPixel (stateXVel state1) @?= 0
      unSubpixel (velSubpixel (stateXVel state1)) @?= 0x3000
      stateXExtra state1 @?= Velocity 0 (Subpixel 0x1000)
      stateHasMomentum state1 @?= True

  , testCase "B+Left accelerates to negative velocity" $ do
      let cfg = defaultConfig
          input = ControllerInput (btnB .|. btnLeft) (ButtonMask 0)
          state0 = initialState cfg
          state1 = step cfg input state0
          state10 = iterate (step cfg input) state0 !! 10
      velPixel (stateXVel state1) @?= (-1)
      unSubpixel (velSubpixel (stateXVel state1)) @?= 0xD000
      velPixel (stateXVel state10) < 0 @?= True
      stateAccelMode state10 @?= AccelAccelerating

  , testCase "Right reaches ROM run max 2.C000" $ do
      let cfg = defaultConfig
          input = ControllerInput btnRight btnRight
          state0 = initialState cfg
          -- 0x3000 * 15 = 0x2D000, cap 0x2C000
          finalState = iterate (step cfg input) state0 !! 16
      stateXVel finalState @?= Velocity 2 (Subpixel 0xC000)

  , testCase "Releasing direction applies table decel" $ do
      let cfg = defaultConfig
          inputAccel = ControllerInput btnRight btnRight
          inputCoast = ControllerInput (ButtonMask 0) btnRight
          -- 4 frames → 0xC000, then decel 0x8000 → 0x4000
          state4 = iterate (step cfg inputAccel) (initialState cfg) !! 4
          state5 = step cfg inputCoast state4
      stateAccelMode state5 @?= AccelDecelerating
      stateXVel state5 @?= Velocity 0 (Subpixel 0x4000)
  ]

testFriction :: TestTree
testFriction = testGroup "Friction/Decel"
  [ testCase "Air run table decel is 0.8000" $ do
      let entry = speedEntry EnvAir mvtRunning
      seDecel entry @?= Velocity 0 (Subpixel 0x8000)
  , testCase "Air run table max is 2.C000" $ do
      let entry = speedEntry EnvAir mvtRunning
      seMax entry @?= Velocity 2 (Subpixel 0xC000)
  ]

testJumpSquat :: TestTree
testJumpSquat = testGroup "Jump Squat"
  [ testCase "Pressing A starts jump squat" $ do
      let cfg = defaultConfig
          input = ControllerInput btnA (ButtonMask 0)
          state1 = step cfg input (initialState cfg)
      stateJumpSquatFrames state1 @?= 1
      statePose state1 @?= poseJumpTransRight

  , testCase "Jump squat lasts 4 frames" $ do
      let cfg = defaultConfig
          inputPress = ControllerInput btnA (ButtonMask 0)
          inputHold = ControllerInput btnA btnA
          state0 = initialState cfg
          state1 = step cfg inputPress state0
          state2 = step cfg inputHold state1
          state3 = step cfg inputHold state2
          state4 = step cfg inputHold state3
      stateJumpSquatFrames state1 @?= 1
      stateJumpSquatFrames state2 @?= 2
      stateJumpSquatFrames state3 @?= 3
      stateOnGround state3 @?= True
      stateOnGround state4 @?= False

  , testCase "Jump fires with vanilla 4.E000 impulse" $ do
      let cfg = defaultConfig
          inputPress = ControllerInput btnA (ButtonMask 0)
          inputHold = ControllerInput btnA btnA
          state0 = initialState cfg
          state4 = iterate (step cfg inputHold) (step cfg inputPress state0) !! 3
      stateVerticalDir state4 @?= VDirRising
      -- Impulse -4.E000 then one same-frame gravity 0x1C00 → (-5, 0x3C00)
      velPixel (stateYVel state4) @?= (-5)
      unSubpixel (velSubpixel (stateYVel state4)) @?= 0x3C00

  , testCase "Releasing A during squat still launches" $ do
      let cfg = defaultConfig
          pressA = ControllerInput btnA (ButtonMask 0)
          aUp = ControllerInput (ButtonMask 0) btnA
          aStillUp = ControllerInput (ButtonMask 0) (ButtonMask 0)
          s1 = step cfg pressA (initialState cfg)
          s2 = step cfg aUp s1
          s3 = step cfg aStillUp s2
          takeoff = step cfg aStillUp s3
      stateJumpSquatFrames s2 @?= 2
      stateJumpSquatFrames s3 @?= 3
      stateOnGround s3 @?= True
      stateOnGround takeoff @?= False
      velPixel (stateYVel takeoff) < 0 @?= True


  , testCase "Speed-booster jump uses C two-register add" $ do
      let cfg = defaultConfig
          pressA = ControllerInput btnA (ButtonMask 0)
          holdA = ControllerInput btnA btnA
          state0 = (initialState cfg)
            { stateEquipment = defaultEquipment { equipSpeedBooster = True }
            , stateXExtra = Velocity 2 (Subpixel 0x8000)
            , stateHasMomentum = True
            }
          launched = iterate (step cfg holdA) (step cfg pressA state0) !! 3
          -- C: unsigned 4.E000 + extra 2.8000 → y_sub = E000+8000 wrap,
          -- y_speed = 4+1 = 5 → unsigned 5.6000 up = Velocity (-6, 0xA000).
          -- Same-frame gravity then adds 0x1c00.
          expected = addVelocity (Velocity (-6) (Subpixel 0xA000))
                                 (Velocity 0 (Subpixel 0x1c00))
      stateOnGround launched @?= False
      stateYVel launched @?= expected
      stateYVel launched @?= Velocity (-6) (Subpixel 0xBC00)
  ]

testGravity :: TestTree
testGravity = testGroup "Gravity"
  [ testCase "Gravity decelerates upward velocity" $ do
      let cfg = defaultConfig
          input = ControllerInput btnA btnA
          state0 = (initialState cfg)
               { stateOnGround = False
               , stateVerticalDir = VDirRising
               , stateYVel = Velocity (-2) (Subpixel 0)
               }
          state1 = step cfg input state0
          expectedVel = addVelocity (Velocity (-2) (Subpixel 0))
                                    (Velocity 0 (Subpixel 0x1c00))
      stateYVel state1 @?= expectedVel

  , testCase "Gravity accelerates downward velocity" $ do
      let cfg = defaultConfig
          input = ControllerInput (ButtonMask 0) (ButtonMask 0)
          airY = Pixel (unPixel (cfgGroundY cfg) - 50)
          state0 = (initialState cfg)
               { stateYPos = Position airY (Subpixel 0)
               , stateOnGround = False
               , stateVerticalDir = VDirFalling
               , stateYVel = Velocity 1 (Subpixel 0)
               }
          state1 = step cfg input state0
          expectedVel = addVelocity (Velocity 1 (Subpixel 0))
                                    (Velocity 0 (Subpixel 0x1c00))
      stateYVel state1 @?= expectedVel

  , testCase "Terminal velocity caps falling speed" $ do
      let cfg = defaultConfig
          input = ControllerInput (ButtonMask 0) (ButtonMask 0)
          airY = Pixel (unPixel (cfgGroundY cfg) - 50)
          state0 = (initialState cfg)
               { stateYPos = Position airY (Subpixel 0)
               , stateOnGround = False
               , stateVerticalDir = VDirFalling
               , stateYVel = Velocity 5 (Subpixel 0)
               }
          state1 = step cfg input state0
      stateYVel state1 @?= Velocity 5 (Subpixel 0)

  , testCase "A-release snaps rising speed to falling 0" $ do
      let cfg = defaultConfig
          input = ControllerInput (ButtonMask 0) btnA
          airY = Pixel (unPixel (cfgGroundY cfg) - 50)
          state0 = (initialState cfg)
               { stateYPos = Position airY (Subpixel 0)
               , stateOnGround = False
               , stateVerticalDir = VDirRising
               , stateYVel = Velocity (-3) (Subpixel 0)
               , stateJumpHeld = True
               }
          state1 = step cfg input state0
      -- Release zeros Y, then the same falling frame applies gravity.
      stateYVel state1 @?= Velocity 0 (Subpixel 0x1c00)
      stateVerticalDir state1 @?= VDirFalling
  ]

testMomentum :: TestTree
testMomentum = testGroup "Extra run"
  [ testCase "Walk-jump does not keep extra run" $ do
      let cfg = defaultConfig
          right = ControllerInput btnRight btnRight
          jumpPress = ControllerInput (btnRight .|. btnA) btnRight
          jumpHold = ControllerInput (btnRight .|. btnA) (btnRight .|. btnA)
          walked = iterate (step cfg right) (initialState cfg) !! 4
          crouched = step cfg jumpPress walked
          launched = iterate (step cfg jumpHold) crouched !! 3
      stateOnGround launched @?= False
      stateXExtra launched @?= zeroVelocity

  , testCase "Run-jump keeps extra run in air" $ do
      let cfg = defaultConfig
          run = ControllerInput (btnB .|. btnRight) (btnB .|. btnRight)
          jumpPress = ControllerInput (btnB .|. btnRight .|. btnA) (btnB .|. btnRight)
          jumpHold = ControllerInput (btnB .|. btnRight .|. btnA)
                                     (btnB .|. btnRight .|. btnA)
          running = iterate (step cfg run) (initialState cfg) !! 4
          launched = iterate (step cfg jumpHold) (step cfg jumpPress running) !! 3
      stateOnGround launched @?= False
      stateHasMomentum launched @?= True
      stateXExtra launched /= zeroVelocity @?= True

  , testCase "Extra-run overshoots cap then snaps back" $ do
      let cfg = defaultConfig
          input = ControllerInput (btnB .|. btnRight) (btnB .|. btnRight)
          after n = iterate (step cfg input) (initialState cfg) !! n
      -- C: check-quirked-greater then add. 32 * 0x1000 = 2.0000;
      -- frame 33 is allowed to overshoot; frame 34 snaps to the cap.
      stateXExtra (after 32) @?= Velocity 2 (Subpixel 0)
      stateXExtra (after 33) @?= Velocity 2 (Subpixel 0x1000)
      stateXExtra (after 34) @?= Velocity 2 (Subpixel 0)
  ]
