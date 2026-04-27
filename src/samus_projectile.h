#ifndef SAMUS_PROJECTILE_H_
#define SAMUS_PROJECTILE_H_

#include "ida_types.h"

void Samus_HandleCooldown(void);
uint8 Samus_CanFireBeam(void);
uint8 Samus_CanFireSuperMissile(void);
void UpdateBeamTilesAndPalette(void);
void WriteBeamPalette_A(uint16 a);
void WriteBeamPalette_Y(uint16 j);
void LoadProjectilePalette(uint16 a);
uint8 SamusProjectile_GetUnchargedBeamCooldown(uint16 type);
void ResetProjectileData(void);
void ClearProjectile(uint16 k);
void KillProjectile(uint16 k);
void HandleProjectile(void);
void HandleProjectileTrails(void);
void FireUnchargedBeam(void);
void FireChargedBeam(void);
uint8 InitProjectilePositionDirection(uint16 r20);
void HandleChargingBeamGfxAudio(void);
void ClearFlareAnimationState(void);
void FireHyperBeam(void);
void ProjectileReflection(uint16 r20);
void Samus_HandleHudSpecificBehaviorAndProjs(void);
void InitializeProjectile(uint16 k);
void InitializeInstrForSuperMissile(uint16 k);
void InitializeInstrForMissile(uint16 k);
void KillProjectileInner(uint16 k);
void InitializeBombExplosion(uint16 k);
void InitializeShinesparkEchoOrSpazerSba(uint16 k);
void InitializeSbaProjectile(uint16 k);
uint16 ProjectileInsts_GetValue(uint16 k);
void RunProjectileInstructions(void);
void DrawPlayerExplosions2(void);
void DrawBombAndProjectileExplosions(void);
Point16U Projectile_SinLookup(uint16 j, uint16 a);

void BombOrPowerBomb_Func1(uint16 k);
uint8 BlockCollNoWaveBeamHoriz(uint16 k);
uint8 BlockCollNoWaveBeamVert(uint16 k);
uint8 BlockCollWaveBeamHoriz(uint16 k);
uint8 BlockCollWaveBeamVert(uint16 k);
uint8 BlockCollMissileHoriz(uint16 k);
uint8 BlockCollMissileVert(uint16 k);
uint8 BlockCollSpreadBomb(uint16 k);

#endif  // SAMUS_PROJECTILE_H_
