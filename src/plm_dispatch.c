// PLM dispatch tables — routes ea (effective address) to the correct handler.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"

#define fnPlmPreInstr_Empty4 0x848AA6

uint8 CallPlmHeaderFunc(uint32 ea, uint16 j) {
  switch (ea) {
  case fnnullsub_67:
  case fnnullsub_290:
  case fnnullsub_68:
  case fnnullsub_84BAFA:
  case fnnullsub_71:
  case fnnullsub_72:
  case fnnullsub_69: return 0;
  case fnPlmSetup_CrumbleBotwoonWall: return PlmSetup_CrumbleBotwoonWall(j);
  case fnPlmSetup_SetrupWreckedShipEntrance: return PlmSetup_SetrupWreckedShipEntrance(j);
  case fnPlmSetup_BTS_Brinstar_0x80_Floorplant: return PlmSetup_BTS_Brinstar_0x80_Floorplant(j);
  case fnPlmSetup_BTS_Brinstar_0x81_Ceilingplant: return PlmSetup_BTS_Brinstar_0x81_Ceilingplant(j);
  case fnPlmSetup_B6D3_MapStation: return PlmSetup_B6D3_MapStation(j);
  case fnPlmSetup_Bts47_MapStationRightAccess: return PlmSetup_Bts47_MapStationRightAccess(j);
  case fnPlmSetup_Bts4_MapStationLeftAccess: return PlmSetup_Bts4_MapStationLeftAccess(j);
  case fnPlmSetup_PlmB6DF_EnergyStation: return PlmSetup_PlmB6DF_EnergyStation(j);
  case fnPlmSetup_PlmB6EB_EnergyStation: return PlmSetup_PlmB6EB_EnergyStation(j);
  case fnPlmSetup_B6E3_EnergyStationRightAccess: return PlmSetup_B6E3_EnergyStationRightAccess(j);
  case fnPlmSetup_B6E7_EnergyStationLeftAccess: return PlmSetup_B6E7_EnergyStationLeftAccess(j);
  case fnPlmSetup_B6EF_MissileStationRightAccess: return PlmSetup_B6EF_MissileStationRightAccess(j);
  case fnPlmSetup_B6F3_MissileStationLeftAccess: return PlmSetup_B6F3_MissileStationLeftAccess(j);
  case fnPlmSetup_B638_Rightwards_Extension: return PlmSetup_B638_Rightwards_Extension(j);
  case fnPlmSetup_B63F_Leftwards_Extension: return PlmSetup_B63F_Leftwards_Extension(j);
  case fnPlmSetup_B643_Downwards_Extension: return PlmSetup_B643_Downwards_Extension(j);
  case fnPlmSetup_B647_Upwards_Extension: return PlmSetup_B647_Upwards_Extension(j);
  case fnPlmSetup_B703_ScrollPLM: return PlmSetup_B703_ScrollPLM(j);
  case fnPlmSetup_B707_SolidScrollPLM: return PlmSetup_B707_SolidScrollPLM(j);
  case fnPlmSetup_B6FF_ScrollBlockTouch: return PlmSetup_B6FF_ScrollBlockTouch(j);
  case fnPlmSetup_DeactivatePlm: return PlmSetup_DeactivatePlm(j);
  case fnPlmSetup_ReturnCarryClear: return PlmSetup_ReturnCarryClear(j);
  case fnPlmSetup_ReturnCarrySet: return PlmSetup_ReturnCarrySet(j);
  case fnPlmSetup_D094_EnemyBreakableBlock: return PlmSetup_D094_EnemyBreakableBlock(j);
  case fnUNUSED_sub_84B3E3: return UNUSED_sub_84B3E3(j);
  case fnPlmSetup_B70F_IcePhysics: return PlmSetup_B70F_IcePhysics(j);
  case fnPlmSetup_QuicksandSurface: return PlmSetup_QuicksandSurface(j);
  case fnPlmSetup_B71F_SubmergingQuicksand: return PlmSetup_B71F_SubmergingQuicksand(j);
  case fnPlmSetup_B723_SandfallsSlow: return PlmSetup_B723_SandfallsSlow(j);
  case fnPlmSetup_B727_SandFallsFast: return PlmSetup_B727_SandFallsFast(j);
  case fnPlmSetup_QuicksandSurfaceB: return PlmSetup_QuicksandSurfaceB(j);
  case fnPlmSetup_B737_SubmergingQuicksand: return PlmSetup_B737_SubmergingQuicksand(j);
  case fnPlmSetup_B73B_B73F_SandFalls: return PlmSetup_B73B_B73F_SandFalls(j);
  case fnPlmSetup_ClearShitroidInvisibleWall: return PlmSetup_ClearShitroidInvisibleWall(j);
  case fnPlmSetup_B767_ClearShitroidInvisibleWall: return PlmSetup_B767_ClearShitroidInvisibleWall(j);
  case fnPlmSetup_B76B_SaveStationTrigger: return PlmSetup_B76B_SaveStationTrigger(j);
  case fnPlmSetup_B76F_SaveStation: return PlmSetup_B76F_SaveStation(j);
  case fnPlmSetup_MotherBrainRoomEscapeDoor: return PlmSetup_MotherBrainRoomEscapeDoor(j);
  case fnPlmSetup_B7EB_EnableSoundsIn32Frames: return PlmSetup_B7EB_EnableSoundsIn32Frames(j);
  case fnPlmSetup_SpeedBoosterEscape: return PlmSetup_SpeedBoosterEscape(j);
  case fnPlmSetup_ShaktoolsRoom: return PlmSetup_ShaktoolsRoom(j);
  case fnPlmSetup_B974: return PlmSetup_B974(j);
  case fnPlmSetup_B9C1_CrittersEscapeBlock: return PlmSetup_B9C1_CrittersEscapeBlock(j);
  case fnPlmSetup_B9ED_CrittersEscapeBlock: return PlmSetup_B9ED_CrittersEscapeBlock(j);
  case fnsub_84B9F1: return sub_84B9F1(j);
  case fnPlmSetup_BB30_CrateriaMainstreetEscape: return PlmSetup_BB30_CrateriaMainstreetEscape(j);
  case fnPlmSetup_C806_LeftGreenGateTrigger: return PlmSetup_C806_LeftGreenGateTrigger(j);
  case fnPlmSetup_C80A_RightGreenGateTrigger: return PlmSetup_C80A_RightGreenGateTrigger(j);
  case fnPlmSetup_C80E_LeftRedGateTrigger: return PlmSetup_C80E_LeftRedGateTrigger(j);
  case fnPlmSetup_C812_RightRedGateTrigger: return PlmSetup_C812_RightRedGateTrigger(j);
  case fnPlmSetup_C81E_LeftYellowGateTrigger: return PlmSetup_C81E_LeftYellowGateTrigger(j);
  case fnPlmSetup_C822_RightYellowGateTrigger: return PlmSetup_C822_RightYellowGateTrigger(j);
  case fnPlmSetup_C816_LeftBlueGateTrigger: return PlmSetup_C816_LeftBlueGateTrigger(j);
  case fnPlmSetup_C81A_RightBlueGateTrigger: return PlmSetup_C81A_RightBlueGateTrigger(j);
  case fnPlmSetup_C82A_DownwardsClosedGate: return PlmSetup_C82A_DownwardsClosedGate(j);
  case fnPlmSetup_C832_UpwardsClosedGate: return PlmSetup_C832_UpwardsClosedGate(j);
  case fnPlmSetup_C826_DownwardsOpenGate: return PlmSetup_C826_DownwardsOpenGate(j);
  case fnPlmSetup_C82E_UpwardsOpenGate: return PlmSetup_C82E_UpwardsOpenGate(j);
  case fnPlmSetup_C836_DownwardsGateShootblock: return PlmSetup_C836_DownwardsGateShootblock(j);
  case fnPlmSetup_C73A_UpwardsGateShootblock: return PlmSetup_C73A_UpwardsGateShootblock(j);
  case fnPlmSetup_C794_GreyDoor: return PlmSetup_C794_GreyDoor(j);
  case fnPlmSetup_Door_Colored: return PlmSetup_Door_Colored(j);
  case fnPlmSetup_Door_Blue: return PlmSetup_Door_Blue(j);
  case fnPlmSetup_Door_Strange: return PlmSetup_Door_Strange(j);
  case fnPlmSetup_D028_D02C_Unused: return PlmSetup_D028_D02C_Unused(j);
  case fnPlmSetup_RespawningSpeedBoostBlock: return PlmSetup_RespawningSpeedBoostBlock(j);
  case fnPlmSetup_RespawningCrumbleBlock: return PlmSetup_RespawningCrumbleBlock(j);
  case fnPlmSetup_RespawningShotBlock: return PlmSetup_RespawningShotBlock(j);
  case fnPlmSetup_RespawningBombBlock: return PlmSetup_RespawningBombBlock(j);
  case fnPlmSetup_RespawningBombBlock2: return PlmSetup_RespawningBombBlock2(j);
  case fnPlmSetup_RespawningPowerBombBlock: return PlmSetup_RespawningPowerBombBlock(j);
  case fnPlmSetup_D08C_SuperMissileBlockRespawning: return PlmSetup_D08C_SuperMissileBlockRespawning(j);
  case fnPlmSetup_D08C_CrumbleBlock: return PlmSetup_D08C_CrumbleBlock(j);
  case fnPlmSetup_D0DC_BreakableGrappleBlock: return PlmSetup_D0DC_BreakableGrappleBlock(j);
  case fnPlmSetup_D0D8_SetVFlag: return PlmSetup_D0D8_SetVFlag(j);
  case fnPlmSetup_D0D8_ClearVflag: return PlmSetup_D0D8_ClearVflag(j);
  case fnPlmSetup_D0E8_GiveSamusDamage: return PlmSetup_D0E8_GiveSamusDamage(j);
  case fnPlmSetup_D113_LowerNorfairChozoRoomPlug: return PlmSetup_D113_LowerNorfairChozoRoomPlug(j);
  case fnPlmSetup_D127: return PlmSetup_D127(j);
  case fnPlmSetup_D138: return PlmSetup_D138(j);
  case fnPlmSetup_D6DA_LowerNorfairChozoHandTrigger: return PlmSetup_D6DA_LowerNorfairChozoHandTrigger(j);
  case fnPlmSetup_MotherBrainGlass: return PlmSetup_MotherBrainGlass(j);
  case fnPlmSetup_DeletePlmIfAreaTorizoDead: return PlmSetup_DeletePlmIfAreaTorizoDead(j);
  case fnPlmSetup_MakeBllockChozoHandTrigger: return PlmSetup_MakeBllockChozoHandTrigger(j);
  case fnPlmSetup_D6F2_WreckedShipChozoHandTrigger: return PlmSetup_D6F2_WreckedShipChozoHandTrigger(j);
  case fnPlmSetup_D700_MakePlmAirBlock_Unused: return PlmSetup_D700_MakePlmAirBlock_Unused(j);
  case fnPlmSetup_D704_AlteranateLowerNorfairChozoHand_Unused: return PlmSetup_D704_AlteranateLowerNorfairChozoHand_Unused(j);
  case fnPlmSetup_D708_LowerNorfairChozoBlockUnused: return PlmSetup_D708_LowerNorfairChozoBlockUnused(j);
  case fnPlmSetup_D70C_NoobTube: return PlmSetup_D70C_NoobTube(j);
  case fnPlmSetup_EyeDoorEye: return PlmSetup_EyeDoorEye(j);
  case fnPlmSetup_EyeDoor: return PlmSetup_EyeDoor(j);
  case fnPlmSetup_SetMetroidRequiredClearState: return PlmSetup_SetMetroidRequiredClearState(j);
  case fnPlmSetup_DraygonCannonFacingRight: return PlmSetup_DraygonCannonFacingRight(j);
  case fnPlmSetup_DraygonCannonFacingDiagonalRight: return PlmSetup_DraygonCannonFacingDiagonalRight(j);
  case fnPlmSetup_DraygonCannonFacingLeft: return PlmSetup_DraygonCannonFacingLeft(j);
  case fnPlmSetup_DraygonCannonFacingDiagonalLeft: return PlmSetup_DraygonCannonFacingDiagonalLeft(j);
  case fnPlmSetup_DraygonCannon: return PlmSetup_DraygonCannon(j);
  case fnsub_84EE4D: return sub_84EE4D(j);
  case fnsub_84EE52: return sub_84EE52(j);
  case fnsub_84EE57: return sub_84EE57(j);
  case fnsub_84EE5C: return sub_84EE5C(j);
  case fnsub_84EE64: return sub_84EE64(j);
  case fnsub_84EE77: return sub_84EE77(j);
  case fnsub_84EE7C: return sub_84EE7C(j);
  case fnsub_84EE81: return sub_84EE81(j);
  case fnsub_84EE86: return sub_84EE86(j);
  case fnsub_84EE8E: return sub_84EE8E(j);
  case fnsub_84EEAB: return sub_84EEAB(j);
  default: return Unreachable();
  }
}

void CallPlmPreInstr(uint32 ea, uint16 k) {
  switch (ea) {
  case fnPlmPreInstr_nullsub_60: return;
  case fnPlmPreInstr_nullsub_301: return;
  case fnPlmPreInstr_Empty: return;
  case fnPlmPreInstr_Empty2: return;
  case fnPlmPreInstr_Empty3: return;
  case fnPlmPreInstr_Empty4: return;
  case fnPlmPreInstr_Empty5: return;
  case fnnullsub_351: return;
  case fnnullsub_84BAFA: return;
  case fnPlmSetup_BTS_Brinstar_0x80_Floorplant: PlmSetup_BTS_Brinstar_0x80_Floorplant(k); return;
  case fnPlmSetup_BTS_Brinstar_0x81_Ceilingplant: PlmSetup_BTS_Brinstar_0x81_Ceilingplant(k); return;
  case fnPlmPreInstr_B7EB_DecTimerEnableSoundsDeletePlm: PlmPreInstr_B7EB_DecTimerEnableSoundsDeletePlm(k); return;
  case fnPlmPreInstr_WakeAndLavaIfBoosterCollected: PlmPreInstr_WakeAndLavaIfBoosterCollected(k); return;
  case fnPlmPreInstr_WakePLMAndStartFxMotionSamusFarLeft: PlmPreInstr_WakePLMAndStartFxMotionSamusFarLeft(k); return;
  case fnPlmPreInstr_AdvanceLavaSamusMovesLeft: PlmPreInstr_AdvanceLavaSamusMovesLeft(k); return;
  case fnPlmPreInstr_ShaktoolsRoom: PlmPreInstr_ShaktoolsRoom(k); return;
  case fnPlmPreInstr_OldTourianEscapeShaftEscape: PlmPreInstr_OldTourianEscapeShaftEscape(k); return;
  case fnPlmPreInstr_EscapeRoomBeforeOldTourianEscapeShaft: PlmPreInstr_EscapeRoomBeforeOldTourianEscapeShaft(k); return;
  case fnPlmPreInstr_WakePlmIfTriggered: PlmPreInstr_WakePlmIfTriggered(k); return;
  case fnPlmPreInstr_WakePlmIfTriggeredOrSamusBelowPlm: PlmPreInstr_WakePlmIfTriggeredOrSamusBelowPlm(k); return;
  case fnPlmPreInstr_WakePlmIfTriggeredOrSamusAbovePlm: PlmPreInstr_WakePlmIfTriggeredOrSamusAbovePlm(k); return;
  case fnPlmPreInstr_DeletePlmAndSpawnTriggerIfBlockDestroyed: PlmPreInstr_DeletePlmAndSpawnTriggerIfBlockDestroyed(k); return;
  case fnPlmPreInstr_IncrementRoomArgIfShotBySuperMissile: PlmPreInstr_IncrementRoomArgIfShotBySuperMissile(k); return;
  case fnPlmPreInstr_WakePlmIfSamusHasBombs: PlmPreInstr_WakePlmIfSamusHasBombs(k); return;
  case fnPlmPreInstr_WakePlmIfRoomArgumentDoorIsSet: PlmPreInstr_WakePlmIfRoomArgumentDoorIsSet(k); return;
  case fnPlmPreInstr_GotoLinkIfShotWithSuperMissile: PlmPreInstr_GotoLinkIfShotWithSuperMissile(k); return;
  case fnPlmPreInstr_GotoLinkIfTriggered: PlmPreInstr_GotoLinkIfTriggered(k); return;
  case fnPlmPreInstr_WakeIfTriggered: PlmPreInstr_WakeIfTriggered(k); return;
  case fnPlmPreInstr_GoToLinkInstrIfShot: PlmPreInstr_GoToLinkInstrIfShot(k); return;
  case fnPlmPreInstr_GoToLinkInstrIfShotWithPowerBomb: PlmPreInstr_GoToLinkInstrIfShotWithPowerBomb(k); return;
  case fnPlmPreInstr_GoToLinkInstrIfShotWithAnyMissile: PlmPreInstr_GoToLinkInstrIfShotWithAnyMissile(k); return;
  case fnPlmPreInstr_GoToLinkInstrIfShotWithSuperMissile: PlmPreInstr_GoToLinkInstrIfShotWithSuperMissile(k); return;
  case fnPlmPreInstr_GoToLinkInstruction: PlmPreInstr_GoToLinkInstruction(k); return;
  case fnPlmPreInstr_PlayDudSound: PlmPreInstr_PlayDudSound(k); return;
  case fnPlmPreInstr_GotoLinkIfBoss1Dead: PlmPreInstr_GotoLinkIfBoss1Dead(k); return;
  case fnPlmPreInstr_GotoLinkIfMiniBossDead: PlmPreInstr_GotoLinkIfMiniBossDead(k); return;
  case fnPlmPreInstr_GotoLinkIfTorizoDead: PlmPreInstr_GotoLinkIfTorizoDead(k); return;
  case fnPlmPreInstr_GotoLinkIfEnemyDeathQuotaOk: PlmPreInstr_GotoLinkIfEnemyDeathQuotaOk(k); return;
  case fnPlmPreInstr_GotoLinkIfTourianStatueFinishedProcessing: PlmPreInstr_GotoLinkIfTourianStatueFinishedProcessing(k); return;
  case fnPlmPreInstr_GotoLinkIfCrittersEscaped: PlmPreInstr_GotoLinkIfCrittersEscaped(k); return;
  case fnPlmPreInstr_PositionSamusAndInvincible: PlmPreInstr_PositionSamusAndInvincible(k); return;
  case fnPlmPreInstr_WakeOnKeyPress: PlmPreInstr_WakeOnKeyPress(k); return;
  case fnnullsub_359: return;
  case fnnullsub_73:
  case fnnullsub_74:
  case fnnullsub_75:
  case fnnullsub_76:
  case fnnullsub_77:
  case fnnullsub_78:
  case fnnullsub_79:
  case fnnullsub_80:
  case fnnullsub_81:
  case fnlocret_848AE0: return;
  case fnPlmPreInstr_SetMetroidsClearState_Ev0x10: PlmPreInstr_SetMetroidsClearState_Ev0x10(k); return;
  case fnPlmPreInstr_SetMetroidsClearState_Ev0x11: PlmPreInstr_SetMetroidsClearState_Ev0x11(k); return;
  case fnPlmPreInstr_SetMetroidsClearState_Ev0x12: PlmPreInstr_SetMetroidsClearState_Ev0x12(k); return;
  case fnPlmPreInstr_SetMetroidsClearState_Ev0x13: PlmPreInstr_SetMetroidsClearState_Ev0x13(k); return;
  default: Unreachable();
  }
}

const uint8 *CallPlmInstr(uint32 ea, const uint8 *j, uint16 k) {
  switch (ea) {
  case fnPlmInstr_Sleep: return PlmInstr_Sleep(j, k);
  case fnPlmInstr_Delete: return PlmInstr_Delete(j, k);
  case fnPlmInstr_PreInstr: return PlmInstr_PreInstr(j, k);
  case fnPlmInstr_ClearPreInstr: return PlmInstr_ClearPreInstr(j, k);
  case fnPlmInstr_CallFunction: return PlmInstr_CallFunction(j, k);
  case fnPlmInstr_Goto: return PlmInstr_Goto(j, k);
  case fnPlmInstr_DecrementAndBranchNonzero: return PlmInstr_DecrementAndBranchNonzero(j, k);
  case fnPlmInstr_SetTimer: return PlmInstr_SetTimer(j, k);
  case fnPlmInstr_LoadItemPlmGfx: return PlmInstr_LoadItemPlmGfx(j, k);
  case fnPlmInstr_CopyFromRamToVram: return PlmInstr_CopyFromRamToVram(j, k);
  case fnPlmInstr_GotoIfBossBitSet: return PlmInstr_GotoIfBossBitSet(j, k);
  case fnPlmInstr_GotoIfEventSet: return PlmInstr_GotoIfEventSet(j, k);
  case fnPlmInstr_SetEvent: return PlmInstr_SetEvent(j, k);
  case fnPlmInstr_GotoIfChozoSet: return PlmInstr_GotoIfChozoSet(j, k);
  case fnPlmInstr_SetRoomChozoBit: return PlmInstr_SetRoomChozoBit(j, k);
  case fnPlmInstr_GotoIfItemBitSet: return PlmInstr_GotoIfItemBitSet(j, k);
  case fnPlmInstr_SetItemBit: return PlmInstr_SetItemBit(j, k);
  case fnPlmInstr_PickupBeamAndShowMessage: return PlmInstr_PickupBeamAndShowMessage(j, k);
  case fnPlmInstr_PickupEquipmentAndShowMessage: return PlmInstr_PickupEquipmentAndShowMessage(j, k);
  case fnPlmInstr_PickupEquipmentAddGrappleShowMessage: return PlmInstr_PickupEquipmentAddGrappleShowMessage(j, k);
  case fnPlmInstr_PickupEquipmentAddXrayShowMessage: return PlmInstr_PickupEquipmentAddXrayShowMessage(j, k);
  case fnPlmInstr_CollectHealthEnergyTank: return PlmInstr_CollectHealthEnergyTank(j, k);
  case fnPlmInstr_CollectHealthReserveTank: return PlmInstr_CollectHealthReserveTank(j, k);
  case fnPlmInstr_CollectAmmoMissileTank: return PlmInstr_CollectAmmoMissileTank(j, k);
  case fnPlmInstr_CollectAmmoSuperMissileTank: return PlmInstr_CollectAmmoSuperMissileTank(j, k);
  case fnPlmInstr_CollectAmmoPowerBombTank: return PlmInstr_CollectAmmoPowerBombTank(j, k);
  case fnPlmInstr_SetLinkReg: return PlmInstr_SetLinkReg(j, k);
  case fnPlmInstr_Call: return PlmInstr_Call(j, k);
  case fnPlmInstr_Return: return PlmInstr_Return(j, k);
  case fnPlmInstr_GotoIfDoorBitSet: return PlmInstr_GotoIfDoorBitSet(j, k);
  case fnPlmInstr_IncrementDoorHitCounterAndJGE: return PlmInstr_IncrementDoorHitCounterAndJGE(j, k);
  case fnPlmInstr_IncrementArgumentAndJGE: return PlmInstr_IncrementArgumentAndJGE(j, k);
  case fnPlmInstr_SetBTS: return PlmInstr_SetBTS(j, k);
  case fnPlmInstr_DrawPlmBlock: return PlmInstr_DrawPlmBlock(j, k);
  case fnPlmInstr_DrawPlmBlock_: return PlmInstr_DrawPlmBlock(j, k);
  case fnPlmInstr_ProcessAirScrollUpdate: return PlmInstr_ProcessAirScrollUpdate(j, k);
  case fnPlmInstr_ProcessSolidScrollUpdate: return PlmInstr_ProcessSolidScrollUpdate(j, k);
  case fnPlmInstr_QueueMusic: return PlmInstr_QueueMusic(j, k);
  case fnPlmInstr_ClearMusicQueueAndQueueTrack: return PlmInstr_ClearMusicQueueAndQueueTrack(j, k);
  case fnPlmInstr_QueueSfx1_Max6: return PlmInstr_QueueSfx1_Max6(j, k);
  case fnPlmInstr_QueueSfx2_Max6: return PlmInstr_QueueSfx2_Max6(j, k);
  case fnPlmInstr_QueueSfx3_Max6: return PlmInstr_QueueSfx3_Max6(j, k);
  case fnPlmInstr_QueueSfx1_Max15: return PlmInstr_QueueSfx1_Max15(j, k);
  case fnPlmInstr_QueueSfx2_Max15: return PlmInstr_QueueSfx2_Max15(j, k);
  case fnPlmInstr_QueueSfx3_Max15: return PlmInstr_QueueSfx3_Max15(j, k);
  case fnPlmInstr_QueueSfx1_Max3: return PlmInstr_QueueSfx1_Max3(j, k);
  case fnPlmInstr_QueueSfx2_Max3: return PlmInstr_QueueSfx2_Max3(j, k);
  case fnPlmInstr_QueueSfx_Max3: return PlmInstr_QueueSfx_Max3(j, k);
  case fnPlmInstr_QueueSfx1_Max9: return PlmInstr_QueueSfx1_Max9(j, k);
  case fnPlmInstr_QueueSfx2_Max9: return PlmInstr_QueueSfx2_Max9(j, k);
  case fnPlmInstr_QueueSfx3_Max9: return PlmInstr_QueueSfx3_Max9(j, k);
  case fnPlmInstr_QueueSfx1_Max1: return PlmInstr_QueueSfx1_Max1(j, k);
  case fnPlmInstr_QueueSfx2_Max1: return PlmInstr_QueueSfx2_Max1(j, k);
  case fnPlmInstr_QueueSfx3_Max1: return PlmInstr_QueueSfx3_Max1(j, k);
  case fnPlmInstr_ActivateMapStation: return PlmInstr_ActivateMapStation(j, k);
  case fnPlmInstr_ActivateEnergyStation: return PlmInstr_ActivateEnergyStation(j, k);
  case fnPlmInstr_ActivateMissileStation: return PlmInstr_ActivateMissileStation(j, k);
  case fnPlmInstr_ActivateSaveStationAndGotoIfNo: return PlmInstr_ActivateSaveStationAndGotoIfNo(j, k);
  case fnPlmInstr_GotoIfSamusNear: return PlmInstr_GotoIfSamusNear(j, k);
  case fnPlmInstr_Scroll_0_1_Blue: return PlmInstr_Scroll_0_1_Blue(j, k);
  case fnPlmInstr_MovePlmDownOneBlock: return PlmInstr_MovePlmDownOneBlock(j, k);
  case fnPlmInstr_MovePlmDownOneBlock_0: return PlmInstr_MovePlmDownOneBlock_0(j, k);
  case fnPlmInstr_DealDamage_2: return PlmInstr_DealDamage_2(j, k);
  case fnPlmInstr_GiveInvincibility: return PlmInstr_GiveInvincibility(j, k);
  case fnPlmInstr_Draw0x38FramesOfRightTreadmill: return PlmInstr_Draw0x38FramesOfRightTreadmill(j, k);
  case fnPlmInstr_Draw0x38FramesOfLeftTreadmill: return PlmInstr_Draw0x38FramesOfLeftTreadmill(j, k);
  case fnPlmInstr_GotoIfSamusHealthFull: return PlmInstr_GotoIfSamusHealthFull(j, k);
  case fnPlmInstr_GotoIfMissilesFull: return PlmInstr_GotoIfMissilesFull(j, k);
  case fnPlmInstr_PlaceSamusOnSaveStation: return PlmInstr_PlaceSamusOnSaveStation(j, k);
  case fnPlmInstr_DisplayGameSavedMessageBox: return PlmInstr_DisplayGameSavedMessageBox(j, k);
  case fnPlmInstr_EnableMovementAndSetSaveStationUsed: return PlmInstr_EnableMovementAndSetSaveStationUsed(j, k);
  case fnPlmInstr_SetCrittersEscapedEvent: return PlmInstr_SetCrittersEscapedEvent(j, k);
  case fnPlmInstr_JumpIfSamusHasNoBombs: return PlmInstr_JumpIfSamusHasNoBombs(j, k);
  case fnPlmInstr_MovePlmRight4Blocks: return PlmInstr_MovePlmRight4Blocks(j, k);
  case fnPlmInstr_ClearTrigger: return PlmInstr_ClearTrigger(j, k);
  case fnPlmInstr_SpawnEproj: return PlmInstr_SpawnEproj(j, k);
  case fnPlmInstr_WakeEprojAtPlmPos: return PlmInstr_WakeEprojAtPlmPos(j, k);
  case fnPlmInstr_SetGreyDoorPreInstr: return PlmInstr_SetGreyDoorPreInstr(j, k);
  case fnPlmInstr_FxBaseYPos_0x2D2: return PlmInstr_FxBaseYPos_0x2D2(j, k);
  case fnPlmInstr_GotoIfRoomArgLess: return PlmInstr_GotoIfRoomArgLess(j, k);
  case fnPlmInstr_SpawnFourMotherBrainGlass: return PlmInstr_SpawnFourMotherBrainGlass(j, k);
  case fnPlmInstr_SpawnTorizoStatueBreaking: return PlmInstr_SpawnTorizoStatueBreaking(j, k);
  case fnPlmInstr_QueueSong1MusicTrack: return PlmInstr_QueueSong1MusicTrack(j, k);
  case fnPlmInstr_TransferWreckedShipChozoSpikesToSlopes: return PlmInstr_TransferWreckedShipChozoSpikesToSlopes(j, k);
  case fnPlmInstr_TransferWreckedShipSlopesToChozoSpikes: return PlmInstr_TransferWreckedShipSlopesToChozoSpikes(j, k);
  case fnPlmInstr_EnableWaterPhysics: return PlmInstr_EnableWaterPhysics(j, k);
  case fnPlmInstr_SpawnN00bTubeCrackEproj: return PlmInstr_SpawnN00bTubeCrackEproj(j, k);
  case fnPlmInstr_DiagonalEarthquake: return PlmInstr_DiagonalEarthquake(j, k);
  case fnPlmInstr_Spawn10shardsAnd6n00bs: return PlmInstr_Spawn10shardsAnd6n00bs(j, k);
  case fnPlmInstr_ShootEyeDoorProjectileWithProjectileArg: return PlmInstr_ShootEyeDoorProjectileWithProjectileArg(j, k);
  case fnPlmInstr_SpawnEyeDoorSweatEproj: return PlmInstr_SpawnEyeDoorSweatEproj(j, k);
  case fnPlmInstr_SpawnTwoEyeDoorSmoke: return PlmInstr_SpawnTwoEyeDoorSmoke(j, k);
  case fnPlmInstr_SpawnEyeDoorSmokeProjectile: return PlmInstr_SpawnEyeDoorSmokeProjectile(j, k);
  case fnPlmInstr_MoveUpAndMakeBlueDoorFacingRight: return PlmInstr_MoveUpAndMakeBlueDoorFacingRight(j, k);
  case fnPlmInstr_MoveUpAndMakeBlueDoorFacingLeft: return PlmInstr_MoveUpAndMakeBlueDoorFacingLeft(j, k);
  case fnPlmInstr_DamageDraygonTurret: return PlmInstr_DamageDraygonTurret(j, k);
  case fnPlmInstr_DamageDraygonTurretFacingDownRight: return PlmInstr_DamageDraygonTurretFacingDownRight(j, k);
  case fnPlmInstr_DamageDraygonTurretFacingUpRight: return PlmInstr_DamageDraygonTurretFacingUpRight(j, k);
  case fnPlmInstr_DamageDraygonTurret2: return PlmInstr_DamageDraygonTurret2(j, k);
  case fnPlmInstr_DamageDraygonTurretFacingDownLeft: return PlmInstr_DamageDraygonTurretFacingDownLeft(j, k);
  case fnPlmInstr_DamageDraygonTurretFacingUpLeft: return PlmInstr_DamageDraygonTurretFacingUpLeft(j, k);
  case fnPlmInstr_DrawItemFrame0: return PlmInstr_DrawItemFrame0(j, k);
  case fnPlmInstr_DrawItemFrame1: return PlmInstr_DrawItemFrame1(j, k);
  case fnPlmInstr_DrawItemFrame_Common: return PlmInstr_DrawItemFrame_Common(j, k);
  case fnPlmInstr_ClearChargeBeamCounter: return PlmInstr_ClearChargeBeamCounter(j, k);
  case fnPlmInstr_ABD6: return PlmInstr_ABD6(j, k);
  case fnPlmInstr_E63B: return PlmInstr_E63B(j, k);
  case fnPlmInstr_SetBtsTo1: return PlmInstr_SetBtsTo1(j, k);
  case fnPlmInstr_DisableSamusControls: return PlmInstr_DisableSamusControls(j, k);
  case fnPlmInstr_EnableSamusControls: return PlmInstr_EnableSamusControls(j, k);
  default: Unreachable(); return NULL;
  }
}
