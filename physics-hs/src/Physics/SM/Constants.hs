-- | Named constants from Super Metroid C implementation.
--
-- All magic numbers replaced with descriptive names matching C enums.
module Physics.SM.Constants
  ( -- * Button masks (kButton_*)
    btnB
  , btnY
  , btnSelect
  , btnStart
  , btnUp
  , btnDown
  , btnLeft
  , btnRight
  , btnA
  , btnX
  , btnL
  , btnR
    -- * Movement types (kMovementType_*)
  , mvtStanding
  , mvtRunning
  , mvtNormalJumping
  , mvtSpinJumping
  , mvtMorphBallGround
  , mvtCrouching
  , mvtFalling
  , mvtMorphBallFalling
    -- * Common poses (kPose_*)
  , poseStandRight
  , poseStandLeft
  , poseRunRight
  , poseRunLeft
  , poseJumpRight
  , poseJumpLeft
  , poseSpinJumpRight
  , poseSpinJumpLeft
  , poseCrouchRight
  , poseCrouchLeft
  ) where

import Physics.SM.Types (ButtonMask (..), MovementType (..), SamusPose (..))

-- | Button masks from C kButton_* enum.
btnB, btnY, btnSelect, btnStart :: ButtonMask
btnUp, btnDown, btnLeft, btnRight :: ButtonMask
btnA, btnX, btnL, btnR :: ButtonMask

btnB      = ButtonMask 0x8000
btnY      = ButtonMask 0x4000
btnSelect = ButtonMask 0x2000
btnStart  = ButtonMask 0x1000
btnUp     = ButtonMask 0x0800
btnDown   = ButtonMask 0x0400
btnLeft   = ButtonMask 0x0200
btnRight  = ButtonMask 0x0100
btnA      = ButtonMask 0x0080
btnX      = ButtonMask 0x0040
btnL      = ButtonMask 0x0020
btnR      = ButtonMask 0x0010

-- | Movement types from C kMovementType_* enum.
mvtStanding, mvtRunning, mvtNormalJumping, mvtSpinJumping :: MovementType
mvtMorphBallGround, mvtCrouching, mvtFalling, mvtMorphBallFalling :: MovementType

mvtStanding        = MovementType 0x00
mvtRunning         = MovementType 0x01
mvtNormalJumping   = MovementType 0x02
mvtSpinJumping     = MovementType 0x03
mvtMorphBallGround = MovementType 0x04
mvtCrouching       = MovementType 0x05
mvtFalling         = MovementType 0x06
mvtMorphBallFalling = MovementType 0x08

-- | Common poses from C kPose_* enum.
poseStandRight, poseStandLeft :: SamusPose
poseRunRight, poseRunLeft :: SamusPose
poseJumpRight, poseJumpLeft :: SamusPose
poseSpinJumpRight, poseSpinJumpLeft :: SamusPose
poseCrouchRight, poseCrouchLeft :: SamusPose

poseStandRight     = SamusPose 0x01  -- kPose_01_FaceR_Normal
poseStandLeft      = SamusPose 0x02  -- kPose_02_FaceL_Normal
poseRunRight       = SamusPose 0x09  -- kPose_09_MoveR_NoAim
poseRunLeft        = SamusPose 0x0A  -- kPose_0A_MoveL_NoAim
poseJumpRight      = SamusPose 0x13  -- kPose_13_FaceR_Jump_NoAim_NoMove_Gun
poseJumpLeft       = SamusPose 0x14  -- kPose_14_FaceL_Jump_NoAim_NoMove_Gun
poseSpinJumpRight  = SamusPose 0x19  -- kPose_19_FaceR_SpinJump
poseSpinJumpLeft   = SamusPose 0x1A  -- kPose_1A_FaceL_SpinJump
poseCrouchRight    = SamusPose 0x27  -- kPose_27_FaceR_Crouch
poseCrouchLeft     = SamusPose 0x28  -- kPose_28_FaceL_Crouch
