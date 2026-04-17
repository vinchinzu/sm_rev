#ifndef SM_PHYSICS_H
#define SM_PHYSICS_H

#include "types.h"

// Core movement functions extracted from Bank 90
void Samus_MovementHandler_Normal(void);
void Samus_Movement_00_Standing(void);
void Samus_Movement_01_Running(void);
void Samus_Movement_02_NormalJumping(void);
void Samus_Movement_03_SpinJumping(void);
void Samus_Movement_04_MorphBallOnGround(void);
void Samus_Movement_05_Crouching(void);
void Samus_Movement_06_Falling(void);
void Samus_Movement_07_Unused(void);
void Samus_Movement_08_MorphBallFalling(void);
void Samus_Movement_09_Unused(void);
void Samus_Movement_0A_KnockbackOrCrystalFlashEnding(void);
void Samus_Movement_0B_Unused(void);
void Samus_Movement_0C_Unused(void);
void Samus_Movement_0D_Unused(void);
void Samus_Movement_0E_TurningAroundOnGround(void);
void Samus_Movement_0F_CrouchingEtcTransition(void);
void Samus_Movement_10_Moonwalking(void);
void Samus_Movement_11_SpringBallOnGround(void);
void Samus_Movement_12_SpringBallInAir(void);
void Samus_Movement_13_SpringBallFalling(void);
void Samus_Movement_14_WallJumping(void);
void Samus_Movement_15_RanIntoWall(void);
void Samus_Movement_16_Grappling(void);
void Samus_Movement_17_TurningAroundJumping(void);
void Samus_Movement_18_TurningAroundFalling(void);
void Samus_Movement_19_DamageBoost(void);
void Samus_Movement_1A_GrabbedByDraygon(void);
void Samus_Movement_1B_ShinesparkEtc(void);
void SamusCrouchingEtcFunc(void);

#endif // SM_PHYSICS_H
