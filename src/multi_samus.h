#pragma once
#include "types.h"

typedef void (*MultiSamusProjectileSpawnHook)(int player_index,
                                              uint16 projectile_slot,
                                              void *context);
typedef void (*MultiSamusPlayerInputHook)(int player_index, void *context);

void MultiSamus_Init(void);
void MultiSamus_SetNumSamus(int num_samus);
void MultiSamus_Switch(int index);
int MultiSamus_GetNumSamus(void);
void MultiSamus_SetProjectileSpawnHook(MultiSamusProjectileSpawnHook hook,
                                       void *context);
void MultiSamus_SetPlayerInputHook(MultiSamusPlayerInputHook hook,
                                   void *context);
void MultiSamus_SetPlayerInputEndHook(MultiSamusPlayerInputHook hook,
                                      void *context);
void MultiSamus_NotifyPlayerInput(int player_index);
void MultiSamus_NotifyPlayerInputEnd(int player_index);
void MultiSamus_SetProjectileSpawnPlayer(int player_index);
void MultiSamus_NotifyProjectileSpawn(uint16 projectile_slot);
