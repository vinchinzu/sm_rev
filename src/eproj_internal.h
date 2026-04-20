#ifndef EPROJ_INTERNAL_H_
#define EPROJ_INTERNAL_H_

#include "types.h"

void CallEprojPreInstr(uint32 ea, uint16 k);
const uint8 *CallEprojInstr(uint32 ea, uint16 k, const uint8 *j);
void CallEprojInit(uint32 ea, uint16 j);

void EprojInit_NorfairLavaquakeRocks(uint16 j);
void EprojPreInstr_NorfairLavaquakeRocks(uint16 k);
void EprojPreInstr_NorfairLavaquakeRocks_Inner(uint16 k);

const uint8 *EprojInstr_Delete(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_Sleep(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_SetPreInstr_(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_ClearPreInstr(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_CallFunc(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_Goto(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_GotoRel(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_DecTimerAndGotoIfNonZero(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_DecTimerAndGotoRelIfNonZero(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_SetTimer(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_MoveRandomlyWithinRadius(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_SetProjectileProperties(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_ClearProjectileProperties(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_EnableCollisionWithSamusProj(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_DisableCollisionWithSamusProj(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_DisableCollisionWithSamus(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_EnableCollisionWithSamus(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_SetToNotDieOnContact(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_SetToDieOnContact(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_SetLowPriority(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_SetHighPriority(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_SetXyRadius(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_SetXyRadiusZero(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_CalculateDirectionTowardsSamus(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_WriteColorsToPalette(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_QueueMusic(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_QueueSfx1_Max6(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_QueueSfx2_Max6(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_QueueSfx3_Max6(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_QueueSfx1_Max15(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_QueueSfx2_Max15(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_QueueSfx3_Max15(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_QueueSfx1_Max3(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_QueueSfx2_Max3(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_QueueSfx3_Max3(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_QueueSfx1_Max9(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_QueueSfx2_Max9(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_QueueSfx3_Max9(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_QueueSfx1_Max1(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_QueueSfx2_Max1(uint16 k, const uint8 *epjp);
const uint8 *EprojInstr_QueueSfx3_Max1(uint16 k, const uint8 *epjp);

#endif
