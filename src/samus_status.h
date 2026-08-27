#ifndef SM_SAMUS_STATUS_H_
#define SM_SAMUS_STATUS_H_

#include "types.h"

void SamusStatus_RequestLockout(uint16 frames);
void SamusStatus_Tick(void);
bool SamusStatus_LockoutActive(void);
uint16 SuitDamageDivision(uint16 a);

#endif  // SM_SAMUS_STATUS_H_
