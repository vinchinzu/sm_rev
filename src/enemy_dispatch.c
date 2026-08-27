// Enemy ROM-address dispatch — routes ea to the matching vanilla handler.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"

void CallEnemyAi(uint32 ea) {
  switch (ea) {
  case fnEnemy_GrappleReact_SamusLatchesOn_A2: Enemy_GrappleReact_SamusLatchesOn_A2(); return;
  case fnEnemy_GrappleReact_KillEnemy_A2: Enemy_GrappleReact_KillEnemy_A2(); return;
  case fnEnemy_GrappleReact_CancelBeam_A2: Enemy_GrappleReact_CancelBeam_A2(); return;
  case fnEnemy_NormalTouchAI_A2: Enemy_NormalTouchAI_A2(); return;
  case fnEnemy_NormalShotAI_A2: Enemy_NormalShotAI_A2(); return;
  case fnEnemy_NormalFrozenAI_A2: Enemy_NormalFrozenAI_A2(); return;
  case fnnullsub_170_A2: return;
  case fnBouncingGoofball_Init: BouncingGoofball_Init(); return;
  case fnBouncingGoofball_Main: BouncingGoofball_Main(); return;
  case fnMiniCrocomire_Init: MiniCrocomire_Init(); return;
  case fnMiniCrocomire_Main: MiniCrocomire_Main(); return;
  case fnMaridiaBeybladeTurtle_Init: MaridiaBeybladeTurtle_Init(); return;
  case fnMiniMaridiaBeybladeTurtle_Init: MiniMaridiaBeybladeTurtle_Init(); return;
  case fnMaridiaBeybladeTurtle_Main: MaridiaBeybladeTurtle_Main(); return;
  case fnMiniMaridiaBeybladeTurtle_Main: MiniMaridiaBeybladeTurtle_Main(); return;
  case fnMaridiaBeybladeTurtle_Touch: MaridiaBeybladeTurtle_Touch(); return;
  case fnMiniMaridiaBeybladeTurtle_Touch: MiniMaridiaBeybladeTurtle_Touch(); return;
  case fnMiniMaridiaBeybladeTurtle_Shot: MiniMaridiaBeybladeTurtle_Shot(); return;
  case fnThinHoppingBlobs_Init: ThinHoppingBlobs_Init(); return;
  case fnThinHoppingBlobs_Main: ThinHoppingBlobs_Main(); return;
  case fnSpikeShootingPlant_Init: SpikeShootingPlant_Init(); return;
  case fnSpikeShootingPlant_Main: SpikeShootingPlant_Main(); return;
  case fnMaridiaSpikeyShell_Init: MaridiaSpikeyShell_Init(); return;
  case fnMaridiaSpikeyShell_Main: MaridiaSpikeyShell_Main(); return;
  case fnMaridiaSpikeyShell_Shot: MaridiaSpikeyShell_Shot(); return;
  case fnGunshipTop_Init: GunshipTop_Init(); return;
  case fnGunshipBottom_Init: GunshipBottom_Init(); return;
  case fnGunshipTop_Main: GunshipTop_Main(); return;
  case fnFlies_Init: Flies_Init(); return;
  case fnFlies_Main: Flies_Main(); return;
  case fnNorfairErraticFireball_Init: NorfairErraticFireball_Init(); return;
  case fnNorfairErraticFireball_Main: NorfairErraticFireball_Main(); return;
  case fnLavaquakeRocks_Init: LavaquakeRocks_Init(); return;
  case fnLavaquakeRocks_Main: LavaquakeRocks_Main(); return;
  case fnRinka_Init: Rinka_Init(); return;
  case fnRinka_Main: Rinka_Main(); return;
  case fnRinka_Frozen: Rinka_Frozen(cur_enemy_index); return;
  case fnRinka_Touch: Rinka_Touch(); return;
  case fnRinka_Shot: Rinka_Shot(); return;
  case fnRinka_Powerbomb: Rinka_Powerbomb(cur_enemy_index); return;
  case fnRio_Init: Rio_Init(); return;
  case fnRio_Main: Rio_Main(); return;
  case fnNorfairLavajumpingEnemy_Init: NorfairLavajumpingEnemy_Init(); return;
  case fnNorfairLavajumpingEnemy_Main: NorfairLavajumpingEnemy_Main(); return;
  case fnNorfairRio_Init: NorfairRio_Init(); return;
  case fnNorfairRio_Main: NorfairRio_Main(); return;
  case fnLowerNorfairRio_Init: LowerNorfairRio_Init(); return;
  case fnLowerNorfairRio_Main: LowerNorfairRio_Main(); return;
  case fnMaridiaLargeSnail_Init: MaridiaLargeSnail_Init(); return;
  case fnMaridiaLargeSnail_Main: MaridiaLargeSnail_Main(); return;
  case fnMaridiaLargeSnail_Touch: MaridiaLargeSnail_Touch(); return;
  case fnMaridiaLargeSnail_Shot: MaridiaLargeSnail_Shot(); return;
  case fnHirisingSlowfalling_Init: HirisingSlowfalling_Init(); return;
  case fnHirisingSlowfalling_Main: HirisingSlowfalling_Main(); return;
  case fnGripper_Init: Gripper_Init(); return;
  case fnGripper_Main: Gripper_Main(); return;
  case fnJetPowerRipper_Init: JetPowerRipper_Init(); return;
  case fnJetPowerRipper_Main: JetPowerRipper_Main(); return;
  case fnJetPowerRipper_Shot: JetPowerRipper_Shot(); return;
  case fnRipper_Init: Ripper_Init(); return;
  case fnRipper_Main: Ripper_Main(); return;
  case fnLavaSeahorse_Init: LavaSeahorse_Init(); return;
  case fnLavaSeahorse_Main: LavaSeahorse_Main(); return;
  case fnLavaSeahorse_Touch: LavaSeahorse_Touch(); return;
  case fnLavaSeahorse_Shot: LavaSeahorse_Shot(); return;
  case fnLavaSeahorse_Powerbomb: LavaSeahorse_Powerbomb(); return;
  case fnTimedShutter_Init: TimedShutter_Init(); return;
  case fnTimedShutter_Main: TimedShutter_Main(); return;
  case fnRisingFallingPlatform_Init: RisingFallingPlatform_Init(); return;
  case fnShootableShutter_Init: ShootableShutter_Init(); return;
  case fnRisingFallingPlatform_Main: RisingFallingPlatform_Main(); return;
  case fnRisingFallingPlatform_Touch: RisingFallingPlatform_Touch(); return;
  case fnRisingFallingPlatform_Shot: RisingFallingPlatform_Shot(); return;
  case fnShootableShutter_Shot: ShootableShutter_Shot(); return;
  case fnRisingFallingPlatform_Powerbomb: RisingFallingPlatform_Powerbomb(); return;
  case fnHorizontalShootableShutter_Init: HorizontalShootableShutter_Init(); return;
  case fnHorizontalShootableShutter_Main: HorizontalShootableShutter_Main(); return;
  case fnHorizontalShootableShutter_Touch: HorizontalShootableShutter_Touch(); return;
  case fnHorizontalShootableShutter_Shot: HorizontalShootableShutter_Shot(); return;
  case fnHorizontalShootableShutter_Powerbomb: HorizontalShootableShutter_Powerbomb(); return;
  case fnEnemy_GrappleReact_NoInteract_A3: Enemy_GrappleReact_NoInteract_A3(); return;
  case fnEnemy_GrappleReact_KillEnemy_A3: Enemy_GrappleReact_KillEnemy_A3(); return;
  case fnEnemy_GrappleReact_CancelBeam_A3: Enemy_GrappleReact_CancelBeam_A3(); return;
  case fnEnemy_NormalTouchAI_A3: Enemy_NormalTouchAI_A3(); return;
  case fnEnemy_NormalShotAI_A3: Enemy_NormalShotAI_A3(); return;
  case fnEnemy_NormalFrozenAI_A3: Enemy_NormalFrozenAI_A3(); return;
  case fnnullsub_170_A3: return;
  case fnWaver_Init: Waver_Init(); return;
  case fnWaver_Main: Waver_Main(); return;
  case fnMetalee_Init: Metalee_Init(); return;
  case fnMetalee_Main: Metalee_Main(); return;
  case fnMetalee_Shot: Metalee_Shot(); return;
  case fnFireflea_Init: Fireflea_Init(); return;
  case fnFireflea_Main: Fireflea_Main(); return;
  case fnFireflea_Touch: Fireflea_Touch(cur_enemy_index); return;
  case fnFireflea_Powerbomb: Fireflea_Powerbomb(); return;
  case fnFireflea_Shot: Fireflea_Shot(); return;
  case fnMaridiaFish_Init: MaridiaFish_Init(); return;
  case fnMaridiaFish_Main: MaridiaFish_Main(); return;
  case fnElevator_Init: Elevator_Init(); return;
  case fnElevator_Frozen: Elevator_Frozen(); return;
  case fnCrab_Init: Crab_Init(); return;
  case fnSlug_Init: Slug_Init(); return;
  case fnPlatformThatFallsWithSamus_Init: PlatformThatFallsWithSamus_Init(); return;
  case fnFastMovingSlowSinkingPlatform_Init: FastMovingSlowSinkingPlatform_Init(); return;
  case fnPlatformThatFallsWithSamus_Main: PlatformThatFallsWithSamus_Main(); return;
  case fnnullsub_32: return;
  case fnFastMovingSlowSinkingPlatform_Shot: FastMovingSlowSinkingPlatform_Shot(); return;
  case fnRoach_Init: Roach_Init(); return;
  case fnRoach_Main: Roach_Main(); return;
  case fnMochtroid_Init: Mochtroid_Init(); return;
  case fnMochtroid_Main: Mochtroid_Main(); return;
  case fnMochtroid_Touch: Mochtroid_Touch(); return;
  case fnMochtroid_Shot: Mochtroid_Shot(); return;
  case fnSidehopper_Init: Sidehopper_Init(); return;
  case fnSidehopper_Main: Sidehopper_Main(); return;
  case fnMaridiaRefillCandy_Init: MaridiaRefillCandy_Init(); return;
  case fnMaridiaRefillCandy_Main: MaridiaRefillCandy_Main(); return;
  case fnNorfairSlowFireball_Init: NorfairSlowFireball_Init(); return;
  case fnBang_Init: Bang_Init(); return;
  case fnBang_Main: Bang_Main(); return;
  case fnBang_Shot: Bang_Shot(); return;
  case fnSkree_Init: Skree_Init(); return;
  case fnSkree_Main: Skree_Main(); return;
  case fnSkree_Shot: Skree_Shot(); return;
  case fnMaridiaSnail_Init: MaridiaSnail_Init(); return;
  case fnMaridiaSnail_Main: MaridiaSnail_Main(); return;
  case fnMaridiaSnail_Touch: MaridiaSnail_Touch(); return;
  case fnMaridiaSnail_Shot: MaridiaSnail_Shot(); return;
  case fnReflec_Init: Reflec_Init(); return;
  case fnnullsub_33: return;
  case fnReflec_Shot: Reflec_Shot(); return;
  case fnWreckedShipOrangeZoomer_Init: WreckedShipOrangeZoomer_Init(); return;
  case fnWreckedShipOrangeZoomer_Main: WreckedShipOrangeZoomer_Main(); return;
  case fnBigEyeBugs_Init: BigEyeBugs_Init(); return;
  case fnFireZoomer_Init: FireZoomer_Init(); return;
  case fnStoneZoomer_Init: StoneZoomer_Init(); return;
  case fnStoneZoomer_Main: StoneZoomer_Main(); return;
  case fnMetroid_Init: Metroid_Init(); return;
  case fnMetroid_Frozen: Metroid_Frozen(); return;
  case fnMetroid_Hurt: Metroid_Hurt(); return;
  case fnMetroid_Main: Metroid_Main(); return;
  case fnMetroid_Touch: Metroid_Touch(); return;
  case fnMetroid_Shot: Metroid_Shot(); return;
  case fnMetroid_Powerbomb: Metroid_Powerbomb(cur_enemy_index); return;
  case fnEnemy_GrappleReact_SamusLatchesOn_A4: Enemy_GrappleReact_SamusLatchesOn_A4(); return;
  case fnEnemy_NormalTouchAI_A4: Enemy_NormalTouchAI_A4(); return;
  case fnEnemy_NormalShotAI_A4: Enemy_NormalShotAI_A4(); return;
  case fnEnemy_NormalFrozenAI_A4: Enemy_NormalFrozenAI_A4(); return;
  case fnnullsub_170_A4: return;
  case fnCrocomire_Hurt: Crocomire_Hurt(); return;
  case fnCrocomire_Init: Crocomire_Init(); return;
  case fnCrocomire_Main: Crocomire_Main(); return;
  case fnnullsub_34: return;
  case fnCrocomire_Powerbomb: Crocomire_Powerbomb(); return;
  case fnCrocomireTongue_Init: CrocomireTongue_Init(); return;
  case fnCrocomireTongue_Main: CrocomireTongue_Main(); return;
  case fnEnemy_GrappleReact_NoInteract_A5: Enemy_GrappleReact_NoInteract_A5(); return;
  case fnEnemy_GrappleReact_CancelBeam_A5: Enemy_GrappleReact_CancelBeam_A5(); return;
  case fnEnemy_NormalTouchAI_A5: Enemy_NormalTouchAI_A5(); return;
  case fnEnemy_NormalShotAI_A5: Enemy_NormalShotAI_A5(); return;
  case fnEnemy_NormalFrozenAI_A5: Enemy_NormalFrozenAI_A5(); return;
  case fnnullsub_170_A5: return;
  case fnDraygon_Init: Draygon_Init(); return;
  case fnDraygon_Main: Draygon_Main(); return;
  case fnDraygon_Hurt: Draygon_Hurt(); return;
  case fnDraygon_Touch: Draygon_Touch(); return;
  case fnDraygon_Shot: Draygon_Shot(); return;
  case fnDraygon_Powerbomb: Draygon_Powerbomb(); return;
  case fnDraygonsEye_Init: DraygonsEye_Init(); return;
  case fnDraygonsEye_Main: DraygonsEye_Main(); return;
  case fnDraygonsTail_Init: DraygonsTail_Init(); return;
  case fnnullsub_37: return;
  case fnDraygonsArms_Init: DraygonsArms_Init(); return;
  case fnnullsub_38: return;
  case fnSporeSpawn_Init: SporeSpawn_Init(); return;
  case fnSporeSpawn_Main: SporeSpawn_Main(); return;
  case fnSporeSpawn_Shot: SporeSpawn_Shot(); return;
  case fnSporeSpawn_Touch: SporeSpawn_Touch(); return;
  case fnnullsub_39: return;
  case fnEnemy_GrappleReact_NoInteract_A6: Enemy_GrappleReact_NoInteract_A6(); return;
  case fnEnemy_GrappleReact_CancelBeam_A6: Enemy_GrappleReact_CancelBeam_A6(); return;
  case fnEnemy_NormalTouchAI_A6: Enemy_NormalTouchAI_A6(); return;
  case fnEnemy_NormalShotAI_A6: Enemy_NormalShotAI_A6(); return;
  case fnEnemy_NormalFrozenAI_A6: Enemy_NormalFrozenAI_A6(); return;
  case fnnullsub_170_A6: return;
  case fnBoulder_Init: Boulder_Init(); return;
  case fnBoulder_Main: Boulder_Main(); return;
  case fnSpikeyPlatform_Init: SpikeyPlatform_Init(); return;
  case fnSpikeyPlatform2ndEnemy_Init: SpikeyPlatform2ndEnemy_Init(); return;
  case fnSpikeyPlatform2ndEnemy_Main: SpikeyPlatform2ndEnemy_Main(); return;
  case fnSpikeyPlatform_Main: SpikeyPlatform_Main(); return;
  case fnFireGeyser_Init: FireGeyser_Init(); return;
  case fnFireGeyser_Main: FireGeyser_Main(); return;
  case fnNuclearWaffle_Init: NuclearWaffle_Init(); return;
  case fnNuclearWaffle_Main: NuclearWaffle_Main(); return;
  case fnFakeKraid_Init: FakeKraid_Init(); return;
  case fnFakeKraid_Main: FakeKraid_Main(); return;
  case fnFakeKraid_Touch: FakeKraid_Touch(); return;
  case fnFakeKraid_Shot: FakeKraid_Shot(); return;
  case fnCeresRidley_Init: CeresRidley_Init(); return;
  case fnCeresRidley_Main: CeresRidley_Main(); return;
  case fnCeresRidley_Hurt: CeresRidley_Hurt(); return;
  case fnRidley_Main: Ridley_Main(); return;
  case fnRidley_Func_2: Ridley_Func_2(); return;
  case fnRidley_Hurt: Ridley_Hurt(); return;
  case fnRidleysExplosion_Init: RidleysExplosion_Init(); return;
  case fnRidleysExplosion_Main: RidleysExplosion_Main(); return;
  case fnRidley_Shot: Ridley_Shot(); return;
  case fnRidley_Powerbomb: Ridley_Powerbomb(); return;
  case fnCeresSteam_Init: CeresSteam_Init(); return;
  case fnCeresSteam_Main: CeresSteam_Main(); return;
  case fnCeresSteam_Touch: CeresSteam_Touch(); return;
  case fnCeresDoor_Init: CeresDoor_Init(); return;
  case fnCeresDoor_Main: CeresDoor_Main(); return;
  case fnnullsub_41: return;
  case fnZebetites_Init: Zebetites_Init(); return;
  case fnZebetites_Main: Zebetites_Main(); return;
  case fnZebetites_Touch: Zebetites_Touch(); return;
  case fnZebetites_Shot: Zebetites_Shot(); return;
  case fnEnemy_GrappleReact_NoInteract_A7: Enemy_GrappleReact_NoInteract_A7(); return;
  case fnEnemy_GrappleReact_CancelBeam_A7: Enemy_GrappleReact_CancelBeam_A7(); return;
  case fnEnemy_NormalShotAI_A7: Enemy_NormalShotAI_A7(); return;
  case fnEnemy_NormalPowerBombAI_SkipDeathAnim_A7: Enemy_NormalPowerBombAI_SkipDeathAnim_A7(); return;
  case fnEnemy_NormalFrozenAI_A7: Enemy_NormalFrozenAI_A7(); return;
  case fnnullsub_170_A7: return;
  case fnnullsub_44: return;
  case fnKraidsArm_Touch: KraidsArm_Touch(); return;
  case fnKraid_Touch: Kraid_Touch(); return;
  case fnnullsub_43: return;
  case fnKraid_Init: Kraid_Init(); return;
  case fnKraidsArm_Init: KraidsArm_Init(); return;
  case fnKraidsTopLint_Init: KraidsTopLint_Init(); return;
  case fnKraidsMiddleLint_Init: KraidsMiddleLint_Init(); return;
  case fnKraidsBottomLint_Init: KraidsBottomLint_Init(); return;
  case fnKraidsFoot_Init: KraidsFoot_Init(); return;
  case fnKraid_Main: Kraid_Main(); return;
  case fnKraidsArm_Main: KraidsArm_Main(); return;
  case fnKraidsTopLint_Main: KraidsTopLint_Main(); return;
  case fnKraidsMiddleLint_Main: KraidsMiddleLint_Main(); return;
  case fnKraidsBottomLint_Main: KraidsBottomLint_Main(); return;
  case fnKraidsFoot_Main: KraidsFoot_Main(); return;
  case fnKraidsGoodFingernail_Touch: KraidsGoodFingernail_Touch(); return;
  case fnKraidsBadFingernail_Touch: KraidsBadFingernail_Touch(); return;
  case fnKraidsGoodFingernail_Init: KraidsGoodFingernail_Init(); return;
  case fnKraidsBadFingernail_Init: KraidsBadFingernail_Init(); return;
  case fnKraidsGoodFingernail_Main: KraidsGoodFingernail_Main(); return;
  case fnKraidsBadFingernail_Main: KraidsBadFingernail_Main(); return;
  case fnPhantoon_Init: Phantoon_Init(); return;
  case fnPhantoon2_Init: Phantoon2_Init(); return;
  case fnPhantoon_Main: Phantoon_Main(); return;
  case fnPhantoon_Hurt: Phantoon_Hurt(); return;
  case fnPhantoon_Touch: Phantoon_Touch(); return;
  case fnnullsub_358: return;
  case fnPhantoon_Shot: Phantoon_Shot(); return;
  case fnnullsub_45: return;
  case fnEtecoon_Init: Etecoon_Init(); return;
  case fnEtecoon_Main: Etecoon_Main(); return;
  case fnDachora_Init: Dachora_Init(); return;
  case fnDachora_Main: Dachora_Main(); return;
  case fnEnemy_GrappleReact_NoInteract_A8: Enemy_GrappleReact_NoInteract_A8(); return;
  case fnEnemy_GrappleReact_KillEnemy_A8: Enemy_GrappleReact_KillEnemy_A8(); return;
  case fnEnemy_GrappleReact_CancelBeam_A8: Enemy_GrappleReact_CancelBeam_A8(); return;
  case fnEnemy_GrappleReact_SamusLatchesNoInvinc_A8: Enemy_GrappleReact_SamusLatchesNoInvinc_A8(); return;
  case fnEnemy_GrappleReact_HurtSamus_A8: Enemy_GrappleReact_HurtSamus_A8(); return;
  case fnEnemy_NormalTouchAI_A8: Enemy_NormalTouchAI_A8(); return;
  case fnEnemy_NormalShotAI_A8: Enemy_NormalShotAI_A8(); return;
  case fnEnemy_NormalFrozenAI_A8: Enemy_NormalFrozenAI_A8(); return;
  case fnnullsub_170_A8: return;
  case fnMiniDraygon_Init: MiniDraygon_Init(); return;
  case fnEvirProjectile_Init: EvirProjectile_Init(); return;
  case fnMiniDraygon_Main: MiniDraygon_Main(); return;
  case fnEvirProjectile_Main: EvirProjectile_Main(); return;
  case fnMiniDraygon_Touch: MiniDraygon_Touch(); return;
  case fnMiniDraygon_Powerbomb: MiniDraygon_Powerbomb(); return;
  case fnMiniDraygon_Shot: MiniDraygon_Shot(); return;
  case fnMorphBallEye_Init: MorphBallEye_Init(); return;
  case fnMorphBallEye_Main: MorphBallEye_Main(); return;
  case fnFune_Init: Fune_Init(); return;
  case fnFune_Main: Fune_Main(); return;
  case fnWreckedShipGhost_Init: WreckedShipGhost_Init(); return;
  case fnWreckedShipGhost_Main: WreckedShipGhost_Main(); return;
  case fnYappingMaw_Init: YappingMaw_Init(); return;
  case fnYappingMaw_Main: YappingMaw_Main(); return;
  case fnYappingMaw_Touch: YappingMaw_Touch(); return;
  case fnYappingMaw_Shot: YappingMaw_Shot(); return;
  case fnYappingMaw_Frozen: YappingMaw_Frozen(); return;
  case fnKago_Init: Kago_Init(); return;
  case fnKago_Main: Kago_Main(); return;
  case fnKago_Shot: Kago_Shot(); return;
  case fnNorfairLavaMan_Init: NorfairLavaMan_Init(); return;
  case fnNorfairLavaMan_Main: NorfairLavaMan_Main(); return;
  case fnNorfairLavaMan_Powerbomb: NorfairLavaMan_Powerbomb(); return;
  case fnNorfairLavaMan_Touch: NorfairLavaMan_Touch(); return;
  case fnNorfairLavaMan_Shot: NorfairLavaMan_Shot(); return;
  case fnBeetom_Init: Beetom_Init(); return;
  case fnBeetom_Main: Beetom_Main(); return;
  case fnBeetom_Touch: Beetom_Touch(); return;
  case fnBeetom_Shot: Beetom_Shot(); return;
  case fnMaridiaFloater_Init: MaridiaFloater_Init(); return;
  case fnMaridiaFloater_Main: MaridiaFloater_Main(); return;
  case fnMaridiaFloater_Touch: MaridiaFloater_Touch(); return;
  case fnMaridiaFloater_Shot: MaridiaFloater_Shot(); return;
  case fnMaridiaFloater_Powerbomb: MaridiaFloater_Powerbomb(); return;
  case fnWreckedShipRobot_Init: WreckedShipRobot_Init(); return;
  case fnWreckedShipRobotDeactivated_Init: WreckedShipRobotDeactivated_Init(); return;
  case fnWreckedShipRobot_Main: WreckedShipRobot_Main(); return;
  case fnnullsub_342: return;
  case fnWreckedShipRobotDeactivated_Touch: WreckedShipRobotDeactivated_Touch(); return;
  case fnWreckedShipRobotDeactivated_Shot: WreckedShipRobotDeactivated_Shot(); return;
  case fnWreckedShipRobot_Shot: WreckedShipRobot_Shot(); return;
  case fnMaridiaPuffer_Init: MaridiaPuffer_Init(); return;
  case fnMaridiaPuffer_Main: MaridiaPuffer_Main(); return;
  case fnMaridiaPuffer_Shot: MaridiaPuffer_Shot(); return;
  case fnWalkingLavaSeahorse_Init: WalkingLavaSeahorse_Init(); return;
  case fnWalkingLavaSeahorse_Main: WalkingLavaSeahorse_Main(); return;
  case fnWreckedShipOrbs_Init: WreckedShipOrbs_Init(); return;
  case fnWreckedShipOrbs_Main: WreckedShipOrbs_Main(); return;
  case fnWreckedShipSpark_Init: WreckedShipSpark_Init(); return;
  case fnWreckedShipSpark_Main: WreckedShipSpark_Main(); return;
  case fnWreckedShipSpark_Shot: WreckedShipSpark_Shot(); return;
  case fnBlueBrinstarFaceBlock_Init: BlueBrinstarFaceBlock_Init(); return;
  case fnBlueBrinstarFaceBlock_Main: BlueBrinstarFaceBlock_Main(); return;
  case fnBlueBrinstarFaceBlock_Shot: BlueBrinstarFaceBlock_Shot(); return;
  case fnKiHunter_Init: KiHunter_Init(); return;
  case fnKiHunterWings_Init: KiHunterWings_Init(); return;
  case fnKiHunter_Main: KiHunter_Main(); return;
  case fnKiHunterWings_Main: KiHunterWings_Main(); return;
  case fnKiHunter_Shot: KiHunter_Shot(); return;
  case fnEnemy_GrappleReact_CancelBeam_A9: Enemy_GrappleReact_CancelBeam_A9(); return;
  case fnEnemy_NormalFrozenAI_A9: Enemy_NormalFrozenAI_A9(); return;
  case fnnullsub_170_A9: return;
  case fnMotherBrainsBody_Init: MotherBrainsBody_Init(); return;
  case fnMotherBrainsBrain_Init: MotherBrainsBrain_Init(); return;
  case fnMotherBrainsBody_Hurt: MotherBrainsBody_Hurt(); return;
  case fnMotherBrainsBody_Powerbomb: MotherBrainsBody_Powerbomb(); return;
  case fnMotherBrainsBrain_Hurt: MotherBrainsBrain_Hurt(); return;
  case fnMotherBrainsTubesFalling_Init: MotherBrainsTubesFalling_Init(); return;
  case fnMotherBrainsTubesFalling_Main: MotherBrainsTubesFalling_Main(cur_enemy_index); return;
  case fnMotherBrainsBody_Shot: MotherBrainsBody_Shot(); return;
  case fnMotherBrainsBrain_Shot: MotherBrainsBrain_Shot(); return;
  case fnnullsub_47: return;
  case fnMotherBrainsBrain_Touch: MotherBrainsBrain_Touch(); return;
  case fnShitroidInCutscene_Init: ShitroidInCutscene_Init(); return;
  case fnShitroidInCutscene_Main: ShitroidInCutscene_Main(); return;
  case fnShitroidInCutscene_Touch: ShitroidInCutscene_Touch(); return;
  case fnDeadTorizo_Init: DeadTorizo_Init(); return;
  case fnDeadTorizo_Main: DeadTorizo_Main(); return;
  case fnDeadTorizo_Powerbomb: DeadTorizo_Powerbomb(); return;
  case fnDeadTorizo_Shot: DeadTorizo_Shot(); return;
  case fnDeadSidehopper_Init: DeadSidehopper_Init(); return;
  case fnDeadZoomer_Init: DeadZoomer_Init(); return;
  case fnDeadRipper_Init: DeadRipper_Init(); return;
  case fnDeadSkree_Init: DeadSkree_Init(); return;
  case fnDeadSidehopper_Powerbomb: DeadSidehopper_Powerbomb(); return;
  case fnDeadSidehopper_Main: DeadSidehopper_Main(); return;
  case fnDeadZoomer_Powerbomb: DeadZoomer_Powerbomb(); return;
  case fnDeadZoomer_Shot: DeadZoomer_Shot(); return;
  case fnDeadRipper_Powerbomb: DeadRipper_Powerbomb(); return;
  case fnDeadRipper_Shot: DeadRipper_Shot(); return;
  case fnDeadSkree_Powerbomb: DeadSkree_Powerbomb(); return;
  case fnDeadSkree_Shot: DeadSkree_Shot(); return;
  case fnDeadSidehopper_Shot: DeadSidehopper_Shot(); return;
  case fnDeadSidehopper_Touch: DeadSidehopper_Touch(); return;
  case fnShitroid_Init: Shitroid_Init(); return;
  case fnShitroid_Powerbomb: Shitroid_Powerbomb(); return;
  case fnShitroid_Main: Shitroid_Main(); return;
  case fnShitroid_Touch: Shitroid_Touch(); return;
  case fnShitroid_Shot: Shitroid_Shot(); return;
  case fnEnemy_GrappleReact_CancelBeam_AA: Enemy_GrappleReact_CancelBeam_AA(); return;
  case fnEnemy_NormalFrozenAI_AA: Enemy_NormalFrozenAI_AA(); return;
  case fnnullsub_170_AA: return;
  case fnTorizo_Hurt: Torizo_Hurt(); return;
  case fnTorizo_Main: Torizo_Main(); return;
  case fnTorizo_Init: Torizo_Init(); return;
  case fnGoldTorizo_Touch: GoldTorizo_Touch(); return;
  case fnTorizo_Shot: Torizo_Shot(); return;
  case fnGoldTorizo_Main: GoldTorizo_Main(); return;
  case fnGoldTorizo_Hurt: GoldTorizo_Hurt(); return;
  case fnGoldTorizo_Shot: GoldTorizo_Shot(); return;
  case fnnullsub_49: return;
  case fnTourianEntranceStatue_Init: TourianEntranceStatue_Init(); return;
  case fnShaktool_Hurt: Shaktool_Hurt(); return;
  case fnShaktool_Init: Shaktool_Init(); return;
  case fnShaktool_Touch: Shaktool_Touch(); return;
  case fnShaktool_Shot: Shaktool_Shot(); return;
  case fnN00bTubeCracks_Init: N00bTubeCracks_Init(); return;
  case fnChozoStatue_Init: ChozoStatue_Init(); return;
  case fnChozoStatue_Main: ChozoStatue_Main(); return;
  case fnnullsub_51: return;
  case fnnullsub_52: return;
  case fnEnemy_GrappleReact_CancelBeam_B2: Enemy_GrappleReact_CancelBeam_B2(); return;
  case fnEnemy_NormalFrozenAI_B2: Enemy_NormalFrozenAI_B2(); return;
  case fnnullsub_170_B2: return;
  case fnWalkingSpacePirates_Powerbomb: WalkingSpacePirates_Powerbomb(); return;
  case fnWalkingSpacePirates_Touch: WalkingSpacePirates_Touch(); return;
  case fnWalkingSpacePirates_Shot: WalkingSpacePirates_Shot(); return;
  case fnWallSpacePirates_Init: WallSpacePirates_Init(); return;
  case fnWallSpacePirates_Main: WallSpacePirates_Main(); return;
  case fnNinjaSpacePirates_Init: NinjaSpacePirates_Init(); return;
  case fnNinjaSpacePirates_Main: NinjaSpacePirates_Main(); return;
  case fnWalkingSpacePirates_Init: WalkingSpacePirates_Init(); return;
  case fnWalkingSpacePirates_Main: WalkingSpacePirates_Main(); return;
  case fnEnemy_GrappleReact_NoInteract_B3: Enemy_GrappleReact_NoInteract_B3(); return;
  case fnEnemy_GrappleReact_KillEnemy_B3: Enemy_GrappleReact_KillEnemy_B3(); return;
  case fnEnemy_GrappleReact_CancelBeam_B3: Enemy_GrappleReact_CancelBeam_B3(); return;
  case fnEnemy_NormalTouchAI_B3: Enemy_NormalTouchAI_B3(); return;
  case fnEnemy_NormalShotAI_B3: Enemy_NormalShotAI_B3(); return;
  case fnEnemy_NormalFrozenAI_B3: Enemy_NormalFrozenAI_B3(); return;
  case fnnullsub_170_B3: return;
  case fnUnusedSpinningTurtleEye_Init: UnusedSpinningTurtleEye_Init(); return;
  case fnUnusedSpinningTurtleEye_Main: UnusedSpinningTurtleEye_Main(); return;
  case fnBrinstarPipeBug_Init: BrinstarPipeBug_Init(); return;
  case fnBrinstarPipeBug_Main: BrinstarPipeBug_Main(); return;
  case fnNorfairPipeBug_Init: NorfairPipeBug_Init(); return;
  case fnNorfairPipeBug_Main: NorfairPipeBug_Main(); return;
  case fnBrinstarYellowPipeBug_Init: BrinstarYellowPipeBug_Init(); return;
  case fnBrinstarYellowPipeBug_Main: BrinstarYellowPipeBug_Main(); return;
  case fnBotwoon_Init: Botwoon_Init(); return;
  case fnBotwoon_Main: Botwoon_Main(); return;
  case fnBotwoon_Touch: Botwoon_Touch(); return;
  case fnBotwoon_Shot: Botwoon_Shot(); return;
  case fnBotwoon_Powerbomb: Botwoon_Powerbomb(); return;
  case fnEscapeEtecoon_Main: EscapeEtecoon_Main(); return;
  case fnEscapeEtecoon_Init: EscapeEtecoon_Init(); return;
  case fnEscapeDachora_Init: EscapeDachora_Init(); return;
  case fnnullsub_54: return;
  case fnEnemy_NormalPowerBombAI_A0:
  case fnEnemy_NormalPowerBombAI_A2:
  case fnEnemy_NormalPowerBombAI_A3:
  case fnEnemy_NormalPowerBombAI_A4:
  case fnEnemy_NormalPowerBombAI_A5:
  case fnEnemy_NormalPowerBombAI_A6:
  case fnEnemy_NormalPowerBombAI_A7:
  case fnEnemy_NormalPowerBombAI_A8:
  case fnEnemy_NormalPowerBombAI_A9:
  case fnEnemy_NormalPowerBombAI_AA:
  case fnEnemy_NormalPowerBombAI_B2:
  case fnEnemy_NormalPowerBombAI_B3: NormalEnemyPowerBombAi(); return;
  default: Unreachable();
  }
}
void CallEnemyPreInstr(uint32 ea) {
  uint16 k = cur_enemy_index;
  switch (ea) {
  case fnnullsub_171: return;  // 0xa0807b
  case fnnullsub_171_A2: return;  // 0xa2807b
  case fnMiniCrocomire_PreInstr5: MiniCrocomire_PreInstr5(k); return;  // 0xa28a43
  case fnMiniCrocomire_PreInstr6: MiniCrocomire_PreInstr6(k); return;  // 0xa28a5c
  case fnnullsub_175: return;  // 0xa28a75
  case fnSpikeShootingPlant_2: SpikeShootingPlant_2(k); return;  // 0xa29fba
  case fnSpikeShootingPlant_3: SpikeShootingPlant_3(k); return;  // 0xa29fec
  case fnnullsub_182: return;  // 0xa2a01b
  case fnnullsub_187: return;  // 0xa2a7d7
  case fnGunshipTop_3: GunshipTop_3(k); return;  // 0xa2a80c
  case fnGunshipTop_4: GunshipTop_4(k); return;  // 0xa2a8d0
  case fnGunshipTop_5: GunshipTop_5(k); return;  // 0xa2a942
  case fnGunshipTop_6: GunshipTop_6(k); return;  // 0xa2a950
  case fnGunshipTop_7: GunshipTop_7(k); return;  // 0xa2a987
  case fnGunshipTop_8: GunshipTop_8(k); return;  // 0xa2a9bd
  case fnGunshipTop_9: GunshipTop_9(k); return;  // 0xa2aa4f
  case fnGunshipTop_10: GunshipTop_10(k); return;  // 0xa2aa5d
  case fnGunshipTop_11: GunshipTop_11(k); return;  // 0xa2aa94
  case fnGunshipTop_12: GunshipTop_12(k); return;  // 0xa2aaa2
  case fnGunshipTop_13: GunshipTop_13(k); return;  // 0xa2ab1f
  case fnGunshipTop_14: GunshipTop_14(k); return;  // 0xa2ab60
  case fnGunshipTop_15: GunshipTop_15(k); return;  // 0xa2ab6e
  case fnGunshipTop_16: GunshipTop_16(k); return;  // 0xa2aba5
  case fnGunshipTop_17: GunshipTop_17(k); return;  // 0xa2abc7
  case fnGunshipTop_18: GunshipTop_18(k); return;  // 0xa2ac1b
  case fnGunshipTop_19: GunshipTop_19(k); return;  // 0xa2acd7
  case fnGunshipTop_20: GunshipTop_20(k); return;  // 0xa2ad0e
  case fnGunshipTop_21: GunshipTop_21(k); return;  // 0xa2ad2d
  case fnFlies_4: Flies_4(k); return;  // 0xa2b14e
  case fnFlies_5: Flies_5(k); return;  // 0xa2b17c
  case fnFlies_6: Flies_6(k); return;  // 0xa2b1aa
  case fnFlies_7: Flies_7(k); return;  // 0xa2b1d2
  case fnNorfairLavajumpingEnemy_Func_1: NorfairLavajumpingEnemy_Func_1(k); return;  // 0xa2bedc
  case fnNorfairLavajumpingEnemy_Func_2: NorfairLavajumpingEnemy_Func_2(k); return;  // 0xa2bf1a
  case fnNorfairLavajumpingEnemy_Func_3: NorfairLavajumpingEnemy_Func_3(k); return;  // 0xa2bf3e
  case fnNorfairLavajumpingEnemy_Func_4: NorfairLavajumpingEnemy_Func_4(k); return;  // 0xa2bf7c
  case fnNorfairLavajumpingEnemy_Func_5: NorfairLavajumpingEnemy_Func_5(k); return;  // 0xa2bfbc
  case fnNorfairRio_Func_1: NorfairRio_Func_1(k); return;  // 0xa2c281
  case fnNorfairRio_Func_2: NorfairRio_Func_2(k); return;  // 0xa2c2e7
  case fnNorfairRio_Func_3: NorfairRio_Func_3(k); return;  // 0xa2c33f
  case fnNorfairRio_Func_4: NorfairRio_Func_4(k); return;  // 0xa2c361
  case fnNorfairRio_Func_5: NorfairRio_Func_5(k); return;  // 0xa2c3b1
  case fnNorfairRio_Func_6: NorfairRio_Func_6(k); return;  // 0xa2c406
  case fnLowerNorfairRio_Func_1: LowerNorfairRio_Func_1(k); return;  // 0xa2c72e
  case fnLowerNorfairRio_Func_2: LowerNorfairRio_Func_2(k); return;  // 0xa2c771
  case fnLowerNorfairRio_Func_3: LowerNorfairRio_Func_3(k); return;  // 0xa2c7bb
  case fnLowerNorfairRio_Func_4: LowerNorfairRio_Func_4(k); return;  // 0xa2c7d6
  case fnLowerNorfairRio_Func_5: LowerNorfairRio_Func_5(k); return;  // 0xa2c82d
  case fnLowerNorfairRio_Func_6: LowerNorfairRio_Func_6(k); return;  // 0xa2c888
  case fnMaridiaLargeSnail_Func_7: MaridiaLargeSnail_Func_7(k); return;  // 0xa2cf66
  case fnMaridiaLargeSnail_Func_8: MaridiaLargeSnail_Func_8(k); return;  // 0xa2cfa9
  case fnLavaSeahorse_Func_1: LavaSeahorse_Func_1(k); return;  // 0xa2e654
  case fnLavaSeahorse_Func_2: LavaSeahorse_Func_2(k); return;  // 0xa2e6ad
  case fnLavaSeahorse_Func_3: LavaSeahorse_Func_3(k); return;  // 0xa2e6f1
  case fnLavaSeahorse_Func_4: LavaSeahorse_Func_4(k); return;  // 0xa2e734
  case fnLavaSeahorse_Func_5: LavaSeahorse_Func_5(k); return;  // 0xa2e749
  case fnnullsub_196: return;  // 0xa2e781
  case fnBang_Func_6: Bang_Func_6(k); return;  // 0xa3bca5
  case fnBang_Func_7: Bang_Func_7(k); return;  // 0xa3bcc1
  case fnBang_Func_8: Bang_Func_8(k); return;  // 0xa3bcc5
  case fnBang_Func_10: Bang_Func_10(k); return;  // 0xa3bd1c
  case fnBang_Func_11: Bang_Func_11(k); return;  // 0xa3bd2c
  case fnnullsub_215: return;  // 0xa3cf5f
  case fnMaridiaSnail_Func_15: MaridiaSnail_Func_15(k); return;  // 0xa3d1b3
  case fnnullsub_343: return;  // 0xa3e08a
  case fnWreckedShipOrangeZoomer_Func_2: WreckedShipOrangeZoomer_Func_2(k); return;
  case fnnullsub_304: return;  // 0xa3e6c1
  case fnFireZoomer_Func_1: FireZoomer_Func_1(k); return;  // 0xa3e6c8
  case fnFireZoomer_Func_2: FireZoomer_Func_2(k); return;  // 0xa3e785
  case fnFireZoomer_Func_3: FireZoomer_Func_3(k); return;  // 0xa3e7f2
  case fnnullsub_237: return;  // 0xa7d4a8
  case fnPhantoon_Spawn8FireballsInCircleAtStart: Phantoon_Spawn8FireballsInCircleAtStart(k); return;  // 0xa7d4a9
  case fnPhantoon_WaitBetweenSpawningAndSpinningFireballs: Phantoon_WaitBetweenSpawningAndSpinningFireballs(k); return;  // 0xa7d4ee
  case fnPhantoon_SpawnFireballsBeforeFight: Phantoon_SpawnFireballsBeforeFight(k); return;  // 0xa7d508
  case fnPhantoon_WavyFadeIn: Phantoon_WavyFadeIn(k); return;  // 0xa7d54a
  case fnPhantoon_PickPatternForRound1: Phantoon_PickPatternForRound1(k); return;  // 0xa7d596
  case fnPhantoon_MovePhantoonInFigure8ThenOpenEye: Phantoon_MovePhantoonInFigure8ThenOpenEye(k); return;  // 0xa7d5e7
  case fnPhantoon_EyeFollowsSamusUntilTimerRunsOut: Phantoon_EyeFollowsSamusUntilTimerRunsOut(k); return;  // 0xa7d60d
  case fnPhantoon_BecomesSolidAndBodyVuln: Phantoon_BecomesSolidAndBodyVuln(k); return;  // 0xa7d65c
  case fnPhantoon_IsSwooping: Phantoon_IsSwooping(k); return;  // 0xa7d678
  case fnPhantoon_FadeoutWithSwoop: Phantoon_FadeoutWithSwoop(k); return;  // 0xa7d6b9
  case fnPhantoon_WaitAfterFadeOut: Phantoon_WaitAfterFadeOut(k); return;  // 0xa7d6d4
  case fnPhantoon_MoveLeftOrRightAndPickEyeOpenPatt: Phantoon_MoveLeftOrRightAndPickEyeOpenPatt(k); return;  // 0xa7d6e2
  case fnPhantoon_FadeInBeforeFigure8: Phantoon_FadeInBeforeFigure8(k); return;  // 0xa7d72d
  case fnPhantoon_BecomeSolidAfterRainingFireballs: Phantoon_BecomeSolidAfterRainingFireballs(k); return;  // 0xa7d73f
  case fnPhantoon_FadeInDuringFireballRain: Phantoon_FadeInDuringFireballRain(k); return;  // 0xa7d767
  case fnPhantoon_FollowSamusWithEyeDuringFireballRain: Phantoon_FollowSamusWithEyeDuringFireballRain(k); return;  // 0xa7d788
  case fnPhantoon_FadeOutDuringFireballRain: Phantoon_FadeOutDuringFireballRain(k); return;  // 0xa7d7d5
  case fnPhantoon_SpawnRainingFireballs: Phantoon_SpawnRainingFireballs(k); return;  // 0xa7d7f7
  case fnPhantoon_FadeOutBeforeFirstFireballRain: Phantoon_FadeOutBeforeFirstFireballRain(k); return;  // 0xa7d82a
  case fnPhantoon_FadeOutBeforeEnrage: Phantoon_FadeOutBeforeEnrage(k); return;  // 0xa7d85c
  case fnPhantoon_MoveEnragedPhantoonToTopCenter: Phantoon_MoveEnragedPhantoonToTopCenter(k); return;  // 0xa7d874
  case fnPhantoon_FadeInEnragedPhantoon: Phantoon_FadeInEnragedPhantoon(k); return;  // 0xa7d891
  case fnPhantoon_Enraged: Phantoon_Enraged(k); return;  // 0xa7d8ac
  case fnPhantoon_FadeoutAfterEnrage: Phantoon_FadeoutAfterEnrage(k); return;  // 0xa7d916
  case fnPhantoon_CompleteSwoopAfterFatalShot: Phantoon_CompleteSwoopAfterFatalShot(k); return;  // 0xa7d92e
  case fnPhantoon_DyingPhantoonFadeInOut: Phantoon_DyingPhantoonFadeInOut(k); return;  // 0xa7d948
  case fnPhantoon_DyingPhantoonExplosions: Phantoon_DyingPhantoonExplosions(k); return;  // 0xa7d98b
  case fnPhantoon_WavyDyingPhantoonAndCry: Phantoon_WavyDyingPhantoonAndCry(k); return;  // 0xa7da51
  case fnPhantoon_DyingFadeOut: Phantoon_DyingFadeOut(k); return;  // 0xa7da86
  case fnPhantoon_AlmostDead: Phantoon_AlmostDead(k); return;  // 0xa7dad7
  case fnPhantoon_Dead: Phantoon_Dead(k); return;  // 0xa7db3d
  case fnEtecoon_Func_4: Etecoon_Func_4(k); return;  // 0xa7e9af
  case fnEtecoon_Func_5: Etecoon_Func_5(k); return;  // 0xa7ea00
  case fnEtecoon_Func_6: Etecoon_Func_6(k); return;  // 0xa7ea37
  case fnEtecoon_Func_7: Etecoon_Func_7(k); return;  // 0xa7eab5
  case fnEtecoon_Func_8: Etecoon_Func_8(k); return;  // 0xa7eb02
  case fnEtecoon_Func_9: Etecoon_Func_9(k); return;  // 0xa7eb2c
  case fnEtecoon_Func_10: Etecoon_Func_10(k); return;  // 0xa7eb50
  case fnEtecoon_Func_11: Etecoon_Func_11(k); return;  // 0xa7ebcd
  case fnEtecoon_Func_12: Etecoon_Func_12(k); return;  // 0xa7ec1b
  case fnEtecoon_Func_16: Etecoon_Func_16(k); return;  // 0xa7ec97
  case fnEtecoon_Func_17: Etecoon_Func_17(k); return;  // 0xa7ecbb
  case fnEtecoon_Func_18: Etecoon_Func_18(k); return;  // 0xa7ecdf
  case fnEtecoon_Func_19: Etecoon_Func_19(k); return;  // 0xa7ed09
  case fnEtecoon_Func_20: Etecoon_Func_20(k); return;  // 0xa7ed2a
  case fnEtecoon_Func_21: Etecoon_Func_21(k); return;  // 0xa7ed54
  case fnEtecoon_Func_22: Etecoon_Func_22(k); return;  // 0xa7ed75
  case fnEtecoon_Func_23: Etecoon_Func_23(k); return;  // 0xa7edc7
  case fnEtecoon_Func_24: Etecoon_Func_24(k); return;  // 0xa7ee3e
  case fnEtecoon_Func_25: Etecoon_Func_25(k); return;  // 0xa7ee9a
  case fnEtecoon_Func_26: Etecoon_Func_26(k); return;  // 0xa7eeb8
  case fnDachora_Func_2: Dachora_Func_2(k); return;  // 0xa7f570
  case fnDachora_Func_3: Dachora_Func_3(k); return;  // 0xa7f5bc
  case fnDachora_Func_4: Dachora_Func_4(k); return;  // 0xa7f5ed
  case fnDachora_Func_5: Dachora_Func_5(k); return;  // 0xa7f65e
  case fnDachora_Func_7: Dachora_Func_7(k); return;  // 0xa7f78f
  case fnDachora_Func_8: Dachora_Func_8(k); return;  // 0xa7f806
  case fnDachora_Func_11: Dachora_Func_11(k); return;  // 0xa7f935
  case fnDachora_Func_12: Dachora_Func_12(k); return;  // 0xa7f98c
  case fnMorphBallEye_Func_1: MorphBallEye_Func_1(k); return;  // 0xa890f1
  case fnMorphBallEye_Func_2: MorphBallEye_Func_2(k); return;  // 0xa8912e
  case fnMorphBallEye_Func_3: MorphBallEye_Func_3(k); return;  // 0xa89160
  case fnMorphBallEye_Func_4: MorphBallEye_Func_4(k); return;  // 0xa891ce
  case fnnullsub_244: return;  // 0xa891dc
  case fnNorfairLavaMan_Func_7: NorfairLavaMan_Func_7(k); return;  // 0xa8b11a
  case fnNorfairLavaMan_Func_8: NorfairLavaMan_Func_8(k); return;  // 0xa8b175
  case fnsub_A8B193: sub_A8B193(k); return;  // 0xa8b193
  case fnNorfairLavaMan_Func_9: NorfairLavaMan_Func_9(k); return;  // 0xa8b1b8
  case fnNorfairLavaMan_Func_10: NorfairLavaMan_Func_10(k); return;  // 0xa8b1dd
  case fnNorfairLavaMan_Func_11: NorfairLavaMan_Func_11(k); return;  // 0xa8b204
  case fnsub_A8B291: sub_A8B291(k); return;  // 0xa8b291
  case fnNorfairLavaMan_Func_13: NorfairLavaMan_Func_13(); return;
  case fnNorfairLavaMan_Func_15: NorfairLavaMan_Func_15(k); return;  // 0xa8b30d
  case fnNorfairLavaMan_Func_16: NorfairLavaMan_Func_16(k); return;  // 0xa8b31f
  case fnNorfairLavaMan_Func_17: NorfairLavaMan_Func_17(k); return;  // 0xa8b356
  case fnNorfairLavaMan_Func_18: NorfairLavaMan_Func_18(k); return;  // 0xa8b3a7
  case fnMaridiaFloater_Func_3: MaridiaFloater_Func_3(k); return;  // 0xa8c283
  case fnMaridiaFloater_Func_4: MaridiaFloater_Func_4(k); return;  // 0xa8c2a6
  case fnMaridiaFloater_Func_5: MaridiaFloater_Func_5(k); return;  // 0xa8c2cf
  case fnMaridiaFloater_Func_6: MaridiaFloater_Func_6(k); return;  // 0xa8c36b
  case fnMaridiaFloater_Func_7: MaridiaFloater_Func_7(k); return;  // 0xa8c3e1
  case fnMaridiaFloater_Func_8: MaridiaFloater_Func_8(k); return;  // 0xa8c469
  case fnMaridiaFloater_Func_9: MaridiaFloater_Func_9(k); return;  // 0xa8c4dc
  case fnMaridiaFloater_Func_10: MaridiaFloater_Func_10(k); return;  // 0xa8c500
  case fnMaridiaFloater_Func_11: MaridiaFloater_Func_11(k); return;  // 0xa8c51d
  case fnnullsub_256: return;  // 0xa8c568
  case fnMaridiaFloater_Func_12: MaridiaFloater_Func_12(k); return;  // 0xa8c569
  case fnMaridiaFloater_Func_13: MaridiaFloater_Func_13(k); return;  // 0xa8c59f
  case fnnullsub_344: return;  // 0xaac95e
  case fnnullsub_274: return;  // 0xaadcaa
  case fnnullsub_276: return;  // 0xaae7a6
  case fnnullsub_171_AA: return;  // 0xaa807b
  case fnnullsub_171_B3: return;  // 0xb3807b
  case fnBrinstarPipeBug_PreInstr_1: BrinstarPipeBug_PreInstr_1(k); return;  // 0xb38880
  case fnBrinstarPipeBug_PreInstr_2: BrinstarPipeBug_PreInstr_2(k); return;  // 0xb38890
  case fnBrinstarPipeBug_PreInstr_3: BrinstarPipeBug_PreInstr_3(k); return;  // 0xb388e3
  case fnBrinstarPipeBug_PreInstr_4: BrinstarPipeBug_PreInstr_4(k); return;  // 0xb3891c
  case fnBrinstarPipeBug_PreInstr_5: BrinstarPipeBug_PreInstr_5(k); return;  // 0xb3897e
  case fnNorfairPipeBug_Func_5: NorfairPipeBug_Func_5(k); return;  // 0xb38cff
  case fnBotwoon_Func_26: Botwoon_Func_26(k); return;  // 0xb39dc0
  case fnBotwoon_Func_27: Botwoon_Func_27(k); return;  // 0xb39e7d
  case fnBotwoon_Func_28: Botwoon_Func_28(k); return;  // 0xb39ee0
  case fnBotwoon_Func_29: Botwoon_Func_29(k); return;  // 0xb39f34
  case fnBotwoon_Func_30: Botwoon_Func_30(k); return;  // 0xb39f7a
  case fnTorizo_Func_5: Torizo_Func_5(k); return;
  case fnTorizo_Func_6: Torizo_Func_6(k); return;
  case fnTorizo_Func_7: Torizo_Func_7(k); return;
  case fnTorizo_D5ED: Torizo_D5ED(k); return;
  case fnTorizo_D5F1: Torizo_D5F1(k); return;
  case fnsub_AAE445: sub_AAE445(k); return;
  case fnShaktool_PreInstr_0: Shaktool_PreInstr_0(k); return;
  case fnnullsub_277: return;
  case fnEscapeEtecoon_E65C: EscapeEtecoon_E65C(k); return;
  case fnEscapeEtecoon_E670: EscapeEtecoon_E670(k); return;
  case fnEscapeEtecoon_E680: EscapeEtecoon_E680(k); return;
  case fnsub_A3E168: sub_A3E168(k); return;
  case fnMaridiaSnail_Func_7: MaridiaSnail_Func_7(k); return;
  case fnMaridiaSnail_Func_9: MaridiaSnail_Func_9(k); return;
  case fnMaridiaSnail_CFB7: MaridiaSnail_CFB7(k); return;
  case fnMaridiaSnail_CFBD: MaridiaSnail_CFBD(k); return;
  case fnMaridiaSnail_CFCE: MaridiaSnail_CFCE(k); return;
  case fnMaridiaSnail_CFD4: MaridiaSnail_CFD4(k); return;
  case fnMaridiaSnail_CFE5: MaridiaSnail_CFE5(k); return;
  case fnMaridiaSnail_CFEB: MaridiaSnail_CFEB(k); return;
  case fnMaridiaSnail_CFFC: MaridiaSnail_CFFC(k); return;
  case fnnullsub_275: return;
  case fnShaktool_DCAC: Shaktool_DCAC(k); return;
  case fnShaktool_DCD7: Shaktool_DCD7(k); return;
  case fnShaktool_DD25: Shaktool_DD25(k); return;
  default: Unreachable();
  }
}

const uint16 *CallEnemyInstr(uint32 ea, uint16 k, const uint16 *j) {
  switch (ea) {
  case fnEnemyInstr_Goto_A2: return EnemyInstr_Goto(k, j);
  case fnEnemyInstr_DecTimerAndGoto2_A2: return EnemyInstr_DecTimerAndGoto(k, j);
  case fnEnemyInstr_SetTimer_A2: return EnemyInstr_SetTimer(k, j);
  case fnEnemyInstr_Sleep_A2: return EnemyInstr_Sleep(k, j);
  case fnEnemyInstr_EnableOffScreenProcessing_A2: return EnemyInstr_EnableOffScreenProcessing(k, j);
  case fnEnemyInstr_DisableOffScreenProcessing_A2: return EnemyInstr_DisableOffScreenProcessing(k, j);
  case fnBouncingGoofball_Instr_88C5: return BouncingGoofball_Instr_88C5(k, j);
  case fnBouncingGoofball_Instr_88C6: return BouncingGoofball_Instr_88C6(k, j);
  case fnMiniCrocomire_Instr_897E: return MiniCrocomire_Instr_897E(k, j);
  case fnMiniCrocomire_Instr_8990: return MiniCrocomire_Instr_8990(k, j);
  case fnMiniCrocomire_Instr_899D: return MiniCrocomire_Instr_899D(k, j);
  case fnMaridiaBeybladeTurtle_Instr_9381: return MaridiaBeybladeTurtle_Instr_9381(k, j);
  case fnMaridiaBeybladeTurtle_Instr_9412: return MaridiaBeybladeTurtle_Instr_9412(k, j);
  case fnMaridiaBeybladeTurtle_Instr_9447: return MaridiaBeybladeTurtle_Instr_9447(k, j);
  case fnMaridiaBeybladeTurtle_Instr_9451: return MaridiaBeybladeTurtle_Instr_9451(k, j);
  case fnMaridiaBeybladeTurtle_Instr_946B: return MaridiaBeybladeTurtle_Instr_946B(k, j);
  case fnMaridiaBeybladeTurtle_Instr_9485: return MaridiaBeybladeTurtle_Instr_9485(k, j);
  case fnMaridiaBeybladeTurtle_Instr_94A1: return MaridiaBeybladeTurtle_Instr_94A1(k, j);
  case fnMaridiaBeybladeTurtle_Instr_94C7: return MaridiaBeybladeTurtle_Instr_94C7(k, j);
  case fnMaridiaBeybladeTurtle_Instr_94D1: return MaridiaBeybladeTurtle_Instr_94D1(k, j);
  case fnSpikeShootingPlant_Instr_9F2A: return SpikeShootingPlant_Instr_9F2A(k, j);
  case fnSpikeShootingPlant_Instr_A095: return SpikeShootingPlant_Instr_A095(k, j);
  case fnSpikeShootingPlant_Instr_A0A7: return SpikeShootingPlant_Instr_A0A7(k, j);
  case fnMaridiaSpikeyShell_Instr_A56D: return MaridiaSpikeyShell_Instr_A56D(k, j);
  case fnMaridiaSpikeyShell_Instr_A571: return MaridiaSpikeyShell_Instr_A571(k, j);
  case fnRinka_Instr_B9B3: return Rinka_Instr_B9B3(k, j);
  case fnRinka_Instr_B9BD: return Rinka_Instr_B9BD(k, j);
  case fnRinka_Instr_B9C7: return Rinka_Instr_B9C7(k, j);
  case fnEnemyInstr_Rio_Instr_1: return EnemyInstr_Rio_Instr_1(k, j);
  case fnNorfairLavajumpingEnemy_Instr_BE8E: return NorfairLavajumpingEnemy_Instr_BE8E(k, j);
  case fnNorfairRio_Instr_C1C9: return NorfairRio_Instr_C1C9(k, j);
  case fnNorfairRio_Instr_C1D4: return NorfairRio_Instr_C1D4(k, j);
  case fnNorfairRio_Instr_C1DF: return NorfairRio_Instr_C1DF(k, j);
  case fnNorfairRio_Instr_C1EA: return NorfairRio_Instr_C1EA(k, j);
  case fnNorfairRio_Instr_C1F5: return NorfairRio_Instr_C1F5(k, j);
  case fnNorfairRio_Instr_C200: return NorfairRio_Instr_C200(k, j);
  case fnNorfairRio_Instr_C20B: return NorfairRio_Instr_C20B(k, j);
  case fnNorfairRio_Instr_C216: return NorfairRio_Instr_C216(k, j);
  case fnNorfairRio_Instr_C221: return NorfairRio_Instr_C221(k, j);
  case fnNorfairRio_Instr_C22C: return NorfairRio_Instr_C22C(k, j);
  case fnNorfairRio_Instr_C237: return NorfairRio_Instr_C237(k, j);
  case fnLowerNorfairRio_Instr_C6D2: return LowerNorfairRio_Instr_C6D2(k, j);
  case fnLowerNorfairRio_Instr_C6DD: return LowerNorfairRio_Instr_C6DD(k, j);
  case fnLowerNorfairRio_Instr_C6E8: return LowerNorfairRio_Instr_C6E8(k, j);
  case fnMaridiaLargeSnail_Instr_CB6B: return MaridiaLargeSnail_Instr_CB6B(k, j);
  case fnMaridiaLargeSnail_Instr_CCB3: return MaridiaLargeSnail_Instr_CCB3(k, j);
  case fnMaridiaLargeSnail_Instr_CCBE: return MaridiaLargeSnail_Instr_CCBE(k, j);
  case fnMaridiaLargeSnail_Instr_CCC9: return MaridiaLargeSnail_Instr_CCC9(k, j);
  case fnLavaSeahorse_Instr_E5FB: return LavaSeahorse_Instr_E5FB(k, j);
  case fnEnemyInstr_Goto_A3: return EnemyInstr_Goto(k, j);
  case fnEnemyInstr_Sleep_A3: return EnemyInstr_Sleep(k, j);
  case fnEnemyInstr_EnableOffScreenProcessing_A3: return EnemyInstr_EnableOffScreenProcessing(k, j);
  case fnEnemyInstr_DisableOffScreenProcessing_A3: return EnemyInstr_DisableOffScreenProcessing(k, j);
  case fnWaver_Instr_1: return Waver_Instr_1(k, j);
  case fnMetalee_Instr_1: return Metalee_Instr_1(k, j);
  case fnMaridiaFish_Instr_3: return MaridiaFish_Instr_3(k, j);
  case fnMaridiaFish_Instr_1: return MaridiaFish_Instr_1(k, j);
  case fnMaridiaFish_Instr_2: return MaridiaFish_Instr_2(k, j);
  case fnPlatformThatFallsWithSamus_Instr_3: return PlatformThatFallsWithSamus_Instr_3(k, j);
  case fnPlatformThatFallsWithSamus_Instr_4: return PlatformThatFallsWithSamus_Instr_4(k, j);
  case fnPlatformThatFallsWithSamus_Instr_1: return PlatformThatFallsWithSamus_Instr_1(k, j);
  case fnPlatformThatFallsWithSamus_Instr_2: return PlatformThatFallsWithSamus_Instr_2(k, j);
  case fnSidehopper_Func_1: return Sidehopper_Func_1(k, j);
  case fnSidehopper_Instr_1: return Sidehopper_Instr_1(k, j);
  case fnMaridiaRefillCandy_Instr_1: return MaridiaRefillCandy_Instr_1(k, j);
  case fnMaridiaRefillCandy_Instr_2: return MaridiaRefillCandy_Instr_2(k, j);
  case fnMaridiaRefillCandy_Instr_3: return MaridiaRefillCandy_Instr_3(k, j);
  case fnBang_Instr_1: return Bang_Instr_1(k, j);
  case fnBang_Instr_2: return Bang_Instr_2(k, j);
  case fnSkree_Instr_1: return Skree_Instr_1(k, j);
  case fnMaridiaSnail_Instr_1: return MaridiaSnail_Instr_1(k, j);
  case fnMaridiaSnail_Instr_2: return MaridiaSnail_Instr_2(k, j);
  case fnMaridiaSnail_Instr_4: return MaridiaSnail_Instr_4(k, j);
  case fnMaridiaSnail_Instr_3: return MaridiaSnail_Instr_3(k, j);
  case fnMaridiaSnail_Instr_5: return MaridiaSnail_Instr_5(k, j);
  case fnReflec_Instr_1: return Reflec_Instr_1(k, j);
  case fnWreckedShipOrangeZoomer_Func_1: return WreckedShipOrangeZoomer_Func_1(k, j);
  case fnZoomer_Instr_SetPreinstr: return Zoomer_Instr_SetPreinstr(k, j);
  case fnMetroid_Instr_2: return Metroid_Instr_2(k, j);
  case fnMetroid_Instr_1: return Metroid_Instr_1(k, j);
  case fnEnemyInstr_Goto_A4: return EnemyInstr_Goto(k, j);
  case fnEnemyInstr_Sleep_A4: return EnemyInstr_Sleep(k, j);
  case fnCrocomire_Instr_1: return Crocomire_Instr_1(k, j);
  case fnCrocomire_Instr_14: return Crocomire_Instr_14(k, j);
  case fnCrocomire_Instr_11: return Crocomire_Instr_11(k, j);
  case fnCrocomire_Instr_7: return Crocomire_Instr_7(k, j);
  case fnCrocomire_Instr_19: return Crocomire_Instr_19(k, j);
  case fnCrocomire_Instr_2: return Crocomire_Instr_2(k, j);
  case fnCrocomire_Instr_4: return Crocomire_Instr_4(k, j);
  case fnCrocomire_Instr_3: return Crocomire_Instr_3(k, j);
  case fnCrocomire_Instr_15: return Crocomire_Instr_15(k, j);
  case fnCrocomire_Instr_16: return Crocomire_Instr_16(k, j);
  case fnCrocomire_Instr_13: return Crocomire_Instr_13(k, j);
  case fnCrocomire_Instr_18: return Crocomire_Instr_18(k, j);
  case fnCrocomire_Instr_12: return Crocomire_Instr_12(k, j);
  case fnCrocomire_Instr_17: return Crocomire_Instr_17(k, j);
  case fnCrocomire_Instr_8: return Crocomire_Instr_8(k, j);
  case fnCrocomire_Instr_6: return Crocomire_Instr_6(k, j);
  case fnCrocomire_Instr_9: return Crocomire_Instr_9(k, j);
  case fnCrocomire_Instr_5: return Crocomire_Instr_5(k, j);
  case fnCrocomire_Instr_20: return Crocomire_Instr_20(k, j);
  case fnCrocomire_Instr_21: return Crocomire_Instr_21(k, j);
  case fnCrocomire_Instr_22: return Crocomire_Instr_22(k, j);
  case fnCrocomire_Instr_23: return Crocomire_Instr_23(k, j);
  case fnCrocomire_Instr_24: return Crocomire_Instr_24(k, j);
  case fnCrocomire_Instr_10: return Crocomire_Instr_10(k, j);
  case fnCrocomire_Instr_25: return Crocomire_Instr_25(k, j);
  case fnCrocomire_Instr_26: return Crocomire_Instr_26(k, j);
  case fnCrocomire_Instr_27: return Crocomire_Instr_27(k, j);
  case fnEnemyInstr_StopScript_A5: return EnemyInstr_StopScript(k, j);
  case fnEnemyInstr_Goto_A5: return EnemyInstr_Goto(k, j);
  case fnEnemyInstr_DecTimerAndGoto2_A5: return EnemyInstr_DecTimerAndGoto(k, j);
  case fnEnemyInstr_SetTimer_A5: return EnemyInstr_SetTimer(k, j);
  case fnEnemyInstr_Sleep_A5: return EnemyInstr_Sleep(k, j);
  case fnEnemyInstr_WaitNframes_A5: return EnemyInstr_WaitNframes(k, j);
  case fnDraygon_Instr_1: return Draygon_Instr_1(k, j);
  case fnDraygon_Instr_13: return Draygon_Instr_13(k, j);
  case fnDraygon_Instr_8: return Draygon_Instr_8(k, j);
  case fnDraygon_Instr_7: return Draygon_Instr_7(k, j);
  case fnDraygon_Instr_6: return Draygon_Instr_6(k, j);
  case fnDraygon_Instr_9: return Draygon_Instr_9(k, j);
  case fnDraygon_Instr_2: return Draygon_Instr_2(k, j);
  case fnDraygon_Instr_11: return Draygon_Instr_11(k, j);
  case fnDraygon_Instr_5: return Draygon_Instr_5(k, j);
  case fnDraygon_Instr_15: return Draygon_Instr_15(k, j);
  case fnDraygon_Instr_17: return Draygon_Instr_17(k, j);
  case fnDraygon_Instr_14: return Draygon_Instr_14(k, j);
  case fnDraygon_Instr_16: return Draygon_Instr_16(k, j);
  case fnDraygon_Instr_10: return Draygon_Instr_10(k, j);
  case fnDraygon_Instr_4: return Draygon_Instr_4(k, j);
  case fnDraygon_Instr_12: return Draygon_Instr_12(k, j);
  case fnDraygon_Instr_18: return Draygon_Instr_18(k, j);
  case fnDraygon_Instr_3: return Draygon_Instr_3(k, j);
  case fnDraygon_Instr_25: return Draygon_Instr_25(k, j);
  case fnDraygon_Instr_24: return Draygon_Instr_24(k, j);
  case fnDraygon_Instr_21: return Draygon_Instr_21(k, j);
  case fnDraygon_Instr_22: return Draygon_Instr_22(k, j);
  case fnDraygon_Instr_27: return Draygon_Instr_27(k, j);
  case fnDraygon_Instr_23: return Draygon_Instr_23(k, j);
  case fnDraygon_Instr_30: return Draygon_Instr_30(k, j);
  case fnDraygon_Instr_20: return Draygon_Instr_20(k, j);
  case fnDraygon_Instr_29: return Draygon_Instr_29(k, j);
  case fnDraygon_Instr_19: return Draygon_Instr_19(k, j);
  case fnDraygon_Instr_28: return Draygon_Instr_28(k, j);
  case fnDraygon_Instr_26: return Draygon_Instr_26(k, j);
  case fnEnemyInstr_Goto_A6: return EnemyInstr_Goto(k, j);
  case fnEnemyInstr_Sleep_A6: return EnemyInstr_Sleep(k, j);
  case fnFireGeyser_Instr_1: return FireGeyser_Instr_1(k, j);
  case fnFireGeyser_Instr_2: return FireGeyser_Instr_2(k, j);
  case fnFireGeyser_Instr_3: return FireGeyser_Instr_3(k, j);
  case fnFireGeyser_Instr_4: return FireGeyser_Instr_4(k, j);
  case fnFireGeyser_Instr_5: return FireGeyser_Instr_5(k, j);
  case fnFireGeyser_Instr_6: return FireGeyser_Instr_6(k, j);
  case fnFireGeyser_Instr_7: return FireGeyser_Instr_7(k, j);
  case fnFireGeyser_Instr_8: return FireGeyser_Instr_8(k, j);
  case fnFireGeyser_Instr_9: return FireGeyser_Instr_9(k, j);
  case fnFireGeyser_Instr_10: return FireGeyser_Instr_10(k, j);
  case fnFireGeyser_Instr_11: return FireGeyser_Instr_11(k, j);
  case fnFireGeyser_Instr_12: return FireGeyser_Instr_12(k, j);
  case fnFireGeyser_Instr_13: return FireGeyser_Instr_13(k, j);
  case fnFireGeyser_Instr_14: return FireGeyser_Instr_14(k, j);
  case fnFireGeyser_Instr_15: return FireGeyser_Instr_15(k, j);
  case fnFireGeyser_Instr_16: return FireGeyser_Instr_16(k, j);
  case fnFireGeyser_Instr_17: return FireGeyser_Instr_17(k, j);
  case fnFireGeyser_Instr_18: return FireGeyser_Instr_18(k, j);
  case fnFireGeyser_Instr_19: return FireGeyser_Instr_19(k, j);
  case fnFireGeyser_Instr_20: return FireGeyser_Instr_20(k, j);
  case fnFireGeyser_Instr_21: return FireGeyser_Instr_21(k, j);
  case fnFireGeyser_Instr_22: return FireGeyser_Instr_22(k, j);
  case fnFireGeyser_Instr_23: return FireGeyser_Instr_23(k, j);
  case fnFireGeyser_Instr_24: return FireGeyser_Instr_24(k, j);
  case fnFakeKraid_Instr_2: return FakeKraid_Instr_2(k, j);
  case fnFakeKraid_Instr_1: return FakeKraid_Instr_1(k, j);
  case fnFakeKraid_Instr_3: return FakeKraid_Instr_3(k, j);
  case fnFakeKraid_Instr_4: return FakeKraid_Instr_4(k, j);
  case fnFakeKraid_Instr_5: return FakeKraid_Instr_5(k, j);
  case fnRidley_Instr_5: return Ridley_Instr_5(k, j);
  case fnRidley_Instr_6: return Ridley_Instr_6(k, j);
  case fnRidley_Instr_10: return Ridley_Instr_10(k, j);
  case fnRidley_Instr_4: return Ridley_Instr_4(k, j);
  case fnRidley_Instr_3: return Ridley_Instr_3(k, j);
  case fnRidley_Instr_2: return Ridley_Instr_2(k, j);
  case fnRidley_Instr_1: return Ridley_Instr_1(k, j);
  case fnRidley_Instr_14: return Ridley_Instr_14(k, j);
  case fnRidley_Instr_9: return Ridley_Instr_9(k, j);
  case fnRidley_Instr_7: return Ridley_Instr_7(k, j);
  case fnRidley_Instr_8: return Ridley_Instr_8(k, j);
  case fnRidley_Instr_11: return Ridley_Instr_11(k, j);
  case fnRidley_Instr_12: return Ridley_Instr_12(k, j);
  case fnRidley_Instr_13: return Ridley_Instr_13(k, j);
  case fnRidley_Instr_15: return Ridley_Instr_15(k, j);
  case fnRidley_Instr_16: return Ridley_Instr_16(k, j);
  case fnCeresSteam_Instr_1: return CeresSteam_Instr_1(k, j);
  case fnCeresSteam_Instr_2: return CeresSteam_Instr_2(k, j);
  case fnCeresSteam_Instr_3: return CeresSteam_Instr_3(k, j);
  case fnCeresDoor_Instr_6: return CeresDoor_Instr_6(k, j);
  case fnCeresDoor_Instr_4: return CeresDoor_Instr_4(k, j);
  case fnCeresDoor_Instr_8: return CeresDoor_Instr_8(k, j);
  case fnCeresSteam_Instr_4: return CeresSteam_Instr_4(k, j);
  case fnCeresDoor_Instr_1: return CeresDoor_Instr_1(k, j);
  case fnCeresDoor_Instr_3: return CeresDoor_Instr_3(k, j);
  case fnCeresSteam_Instr_5: return CeresSteam_Instr_5(k, j);
  case fnCeresDoor_Instr_5: return CeresDoor_Instr_5(k, j);
  case fnCeresDoor_Instr_2: return CeresDoor_Instr_2(k, j);
  case fnCeresDoor_Instr_7: return CeresDoor_Instr_7(k, j);
  case fnEnemyInstr_Call_A7: return EnemyInstr_Call_A7(k, j);
  case fnEnemyInstr_Goto_A7: return EnemyInstr_Goto(k, j);
  case fnEnemyInstr_DecTimerAndGoto2_A7: return EnemyInstr_DecTimerAndGoto(k, j);
  case fnEnemyInstr_SetTimer_A7: return EnemyInstr_SetTimer(k, j);
  case fnEnemyInstr_Sleep_A7: return EnemyInstr_Sleep(k, j);
  case fnKraid_Instr_9: return Kraid_Instr_9(k, j);
  case fnKraid_Instr_1: return Kraid_Instr_1(k, j);
  case fnKraid_Instr_DecYpos: return Kraid_Instr_DecYpos(k, j);
  case fnKraid_Instr_IncrYpos_Shake: return Kraid_Instr_IncrYpos_Shake(k, j);
  case fnKraid_Instr_PlaySound_0x76: return Kraid_Instr_PlaySound_0x76(k, j);
  case fnKraid_Instr_XposMinus3: return Kraid_Instr_XposMinus3(k, j);
  case fnKraid_Instr_XposMinus3b: return Kraid_Instr_XposMinus3b(k, j);
  case fnKraid_Instr_XposPlus3: return Kraid_Instr_XposPlus3(k, j);
  case fnKraid_Instr_MoveHimRight: return Kraid_Instr_MoveHimRight(k, j);
  case fnEnemyInstr_Goto_A8: return EnemyInstr_Goto(k, j);
  case fnEnemyInstr_DecTimerAndGoto2_A8: return EnemyInstr_DecTimerAndGoto(k, j);
  case fnEnemyInstr_SetTimer_A8: return EnemyInstr_SetTimer(k, j);
  case fnEnemyInstr_Sleep_A8: return EnemyInstr_Sleep(k, j);
  case fnEnemyInstr_EnableOffScreenProcessing_A8: return EnemyInstr_EnableOffScreenProcessing(k, j);
  case fnEnemyInstr_DisableOffScreenProcessing_A8: return EnemyInstr_DisableOffScreenProcessing(k, j);
  case fnMiniDraygon_Instr_2: return MiniDraygon_Instr_2(k, j);
  case fnMiniDraygon_Instr_1: return MiniDraygon_Instr_1(k, j);
  case fnMiniDraygon_Instr_3: return MiniDraygon_Instr_3(k, j);
  case fnMiniDraygon_Instr_4: return MiniDraygon_Instr_4(k, j);
  case fnFune_Instr_2: return Fune_Instr_2(k, j);
  case fnFune_Instr_6: return Fune_Instr_6(k, j);
  case fnFune_Instr_7: return Fune_Instr_7(k, j);
  case fnFune_Instr_1: return Fune_Instr_1(k, j);
  case fnFune_Instr_4: return Fune_Instr_4(k, j);
  case fnFune_Instr_3: return Fune_Instr_3(k, j);
  case fnFune_Instr_5: return Fune_Instr_5(k, j);
  case fnYappingMaw_Instr_2: return YappingMaw_Instr_2(k, j);
  case fnYappingMaw_Instr_4: return YappingMaw_Instr_4(k, j);
  case fnYappingMaw_Instr_5: return YappingMaw_Instr_5(k, j);
  case fnYappingMaw_Instr_7: return YappingMaw_Instr_7(k, j);
  case fnYappingMaw_Instr_3: return YappingMaw_Instr_3(k, j);
  case fnYappingMaw_Instr_6: return YappingMaw_Instr_6(k, j);
  case fnYappingMaw_Instr_1: return YappingMaw_Instr_1(k, j);
  case fnNorfairLavaMan_Instr_1: return NorfairLavaMan_Instr_1(k, j);
  case fnNorfairLavaMan_Instr_8: return NorfairLavaMan_Instr_8(k, j);
  case fnNorfairLavaMan_Instr_14: return NorfairLavaMan_Instr_14(k, j);
  case fnNorfairLavaMan_Instr_2: return NorfairLavaMan_Instr_2(k, j);
  case fnNorfairLavaMan_Instr_7: return NorfairLavaMan_Instr_7(k, j);
  case fnNorfairLavaMan_Instr_10: return NorfairLavaMan_Instr_10(k, j);
  case fnNorfairLavaMan_Instr_12: return NorfairLavaMan_Instr_12(k, j);
  case fnNorfairLavaMan_Instr_9: return NorfairLavaMan_Instr_9(k, j);
  case fnNorfairLavaMan_Instr_11: return NorfairLavaMan_Instr_11(k, j);
  case fnNorfairLavaMan_Instr_13: return NorfairLavaMan_Instr_13(k, j);
  case fnNorfairLavaMan_Instr_5: return NorfairLavaMan_Instr_5(k, j);
  case fnNorfairLavaMan_Instr_15: return NorfairLavaMan_Instr_15(k, j);
  case fnNorfairLavaMan_Instr_4: return NorfairLavaMan_Instr_4(k, j);
  case fnNorfairLavaMan_Instr_16: return NorfairLavaMan_Instr_16(k, j);
  case fnNorfairLavaMan_Instr_6: return NorfairLavaMan_Instr_6(k, j);
  case fnNorfairLavaMan_Instr_3: return NorfairLavaMan_Instr_3(k, j);
  case fnBeetom_Instr_1: return Beetom_Instr_1(k, j);
  case fnWreckedShipRobot_Instr_4: return WreckedShipRobot_Instr_4(k, j);
  case fnWreckedShipRobot_Instr_9: return WreckedShipRobot_Instr_9(k, j);
  case fnWreckedShipRobot_Instr_6: return WreckedShipRobot_Instr_6(k, j);
  case fnWreckedShipRobot_Instr_8: return WreckedShipRobot_Instr_8(k, j);
  case fnWreckedShipRobot_Instr_7: return WreckedShipRobot_Instr_7(k, j);
  case fnWreckedShipRobot_Instr_15: return WreckedShipRobot_Instr_15(k, j);
  case fnWreckedShipRobot_Instr_18: return WreckedShipRobot_Instr_18(k, j);
  case fnWreckedShipRobot_Instr_16: return WreckedShipRobot_Instr_16(k, j);
  case fnWreckedShipRobot_Instr_17: return WreckedShipRobot_Instr_17(k, j);
  case fnWreckedShipRobot_Instr_3: return WreckedShipRobot_Instr_3(k, j);
  case fnWreckedShipRobot_Instr_10: return WreckedShipRobot_Instr_10(k, j);
  case fnWreckedShipRobot_Instr_14: return WreckedShipRobot_Instr_14(k, j);
  case fnWreckedShipRobot_Instr_2: return WreckedShipRobot_Instr_2(k, j);
  case fnWreckedShipRobot_Instr_13: return WreckedShipRobot_Instr_13(k, j);
  case fnWreckedShipRobot_Instr_1: return WreckedShipRobot_Instr_1(k, j);
  case fnWreckedShipRobot_Instr_12: return WreckedShipRobot_Instr_12(k, j);
  case fnWreckedShipRobot_Instr_5: return WreckedShipRobot_Instr_5(k, j);
  case fnWreckedShipRobot_Instr_11: return WreckedShipRobot_Instr_11(k, j);
  case fnWalkingLavaSeahorse_Instr_4: return WalkingLavaSeahorse_Instr_4(k, j);
  case fnWalkingLavaSeahorse_Instr_3: return WalkingLavaSeahorse_Instr_3(k, j);
  case fnWalkingLavaSeahorse_Instr_5: return WalkingLavaSeahorse_Instr_5(k, j);
  case fnWalkingLavaSeahorse_Instr_6: return WalkingLavaSeahorse_Instr_6(k, j);
  case fnWalkingLavaSeahorse_Instr_2: return WalkingLavaSeahorse_Instr_2(k, j);
  case fnWalkingLavaSeahorse_Instr_1: return WalkingLavaSeahorse_Instr_1(k, j);
  case fnWreckedShipSpark_Instr_2: return WreckedShipSpark_Instr_2(k, j);
  case fnWreckedShipSpark_Instr_1: return WreckedShipSpark_Instr_1(k, j);
  case fnKiHunter_Instr_1: return KiHunter_Instr_1(k, j);
  case fnKiHunter_Instr_2: return KiHunter_Instr_2(k, j);
  case fnKiHunter_Instr_3: return KiHunter_Instr_3(k, j);
  case fnKiHunter_Instr_4: return KiHunter_Instr_4(k, j);
  case fnKiHunter_Instr_5: return KiHunter_Instr_5(k, j);
  case fnEnemyInstr_Sleep_A9: return EnemyInstr_Sleep(k, j);
  case fnShitroid_Instr_1: return Shitroid_Instr_1(k, j);
  case fnShitroid_Instr_2: return Shitroid_Instr_2(k, j);
  case fnsub_A9ECD0: return sub_A9ECD0(k, j);
  case fnShitroid_Instr_3: return Shitroid_Instr_3(k, j);
  case fnShitroid_Instr_4: return Shitroid_Instr_4(k, j);
  case fnShitroid_Instr_6: return Shitroid_Instr_6(k, j);
  case fnShitroid_Instr_5: return Shitroid_Instr_5(k, j);
  case fnEnemy_SetAiPreInstr_AA: return Enemy_SetAiPreInstr_AA(k, j);
  case fnEnemy_ClearAiPreInstr_AA: return Enemy_ClearAiPreInstr_AA(k, j);
  case fnEnemyInstr_StopScript_AA: return EnemyInstr_StopScript(k, j);
  case fnEnemyInstr_Goto_AA: return EnemyInstr_Goto(k, j);
  case fnEnemyInstr_DecTimerAndGoto2_AA: return EnemyInstr_DecTimerAndGoto(k, j);
  case fnEnemyInstr_SetTimer_AA: return EnemyInstr_SetTimer(k, j);
  case fnEnemyInstr_Sleep_AA: return EnemyInstr_Sleep(k, j);
  case fnEnemyInstr_WaitNframes_AA: return EnemyInstr_WaitNframes(k, j);
  case fnEnemyInstr_CopyToVram_AA: return EnemyInstr_CopyToVram(k, j);
  case fnTorizo_Instr_3: return Torizo_Instr_3(k, j);
  case fnTorizo_Instr_31: return Torizo_Instr_31(k, j);
  case fnTorizo_Instr_33: return Torizo_Instr_33(k, j);
  case fnTorizo_Instr_36: return Torizo_Instr_36(k, j);
  case fnTorizo_Instr_37: return Torizo_Instr_37(k, j);
  case fnTorizo_Instr_35: return Torizo_Instr_35(k, j);
  case fnTorizo_Instr_38: return Torizo_Instr_38(k, j);
  case fnTorizo_Instr_6: return Torizo_Instr_6(k, j);
  case fnTorizo_Instr_5: return Torizo_Instr_5(k, j);
  case fnTorizo_Instr_9: return Torizo_Instr_9(k, j);
  case fnTorizo_Instr_7: return Torizo_Instr_7(k, j);
  case fnTorizo_Instr_2: return Torizo_Instr_2(k, j);
  case fnTorizo_Instr_8: return Torizo_Instr_8(k, j);
  case fnTorizo_Instr_25: return Torizo_Instr_25(k, j);
  case fnTorizo_Instr_22: return Torizo_Instr_22(k, j);
  case fnTorizo_Instr_19: return Torizo_Instr_19(k, j);
  case fnTorizo_Instr_32: return Torizo_Instr_32(k, j);
  case fnTorizo_Instr_30: return Torizo_Instr_30(k, j);
  case fnTorizo_Instr_34: return Torizo_Instr_34(k, j);
  case fnTorizo_Instr_24: return Torizo_Instr_24(k, j);
  case fnTorizo_Instr_12: return Torizo_Instr_12(k, j);
  case fnTorizo_Instr_10: return Torizo_Instr_10(k, j);
  case fnTorizo_Instr_11: return Torizo_Instr_11(k, j);
  case fnTorizo_Instr_29: return Torizo_Instr_29(k, j);
  case fnTorizo_Instr_1: return Torizo_Instr_1(k, j);
  case fnTorizo_Instr_28: return Torizo_Instr_28(k, j);
  case fnTorizo_Instr_4: return Torizo_Instr_4(k, j);
  case fnTorizo_Instr_40: return Torizo_Instr_40(k, j);
  case fnTorizo_Instr_16: return Torizo_Instr_16(k, j);
  case fnTorizo_Instr_27: return Torizo_Instr_27(k, j);
  case fnTorizo_Instr_23: return Torizo_Instr_23(k, j);
  case fnTorizo_Instr_14: return Torizo_Instr_14(k, j);
  case fnTorizo_Instr_15: return Torizo_Instr_15(k, j);
  case fnTorizo_Instr_26: return Torizo_Instr_26(k, j);
  case fnTorizo_Instr_18: return Torizo_Instr_18(k, j);
  case fnTorizo_Instr_20: return Torizo_Instr_20(k, j);
  case fnTorizo_Instr_44: return Torizo_Instr_44(k, j);
  case fnTorizo_Instr_21: return Torizo_Instr_21(k, j);
  case fnTorizo_Instr_17: return Torizo_Instr_17(k, j);
  case fnTorizo_Instr_13: return Torizo_Instr_13(k, j);
  case fnTorizo_Instr_39: return Torizo_Instr_39(k, j);
  case fnTorizo_Instr_41: return Torizo_Instr_41(k, j);
  case fnTorizo_Instr_42: return Torizo_Instr_42(k, j);
  case fnTorizo_Instr_48: return Torizo_Instr_48(k, j);
  case fnTorizo_Instr_57: return Torizo_Instr_57(k, j);
  case fnTorizo_Instr_58: return Torizo_Instr_58(k, j);
  case fnTorizo_Instr_59: return Torizo_Instr_59(k, j);
  case fnTorizo_Instr_62: return Torizo_Instr_62(k, j);
  case fnTorizo_Instr_63: return Torizo_Instr_63(k, j);
  case fnTorizo_Instr_56: return Torizo_Instr_56(k, j);
  case fnTorizo_Instr_60: return Torizo_Instr_60(k, j);
  case fnTorizo_Instr_46: return Torizo_Instr_46(k, j);
  case fnTorizo_Instr_47: return Torizo_Instr_47(k, j);
  case fnTorizo_Instr_49: return Torizo_Instr_49(k, j);
  case fnTorizo_Instr_61: return Torizo_Instr_61(k, j);
  case fnTorizo_Instr_53: return Torizo_Instr_53(k, j);
  case fnTorizo_Instr_55: return Torizo_Instr_55(k, j);
  case fnTorizo_Instr_52: return Torizo_Instr_52(k, j);
  case fnTorizo_Instr_50: return Torizo_Instr_50(k, j);
  case fnTorizo_Instr_43: return Torizo_Instr_43(k, j);
  case fnTorizo_Instr_51: return Torizo_Instr_51(k, j);
  case fnTorizo_Instr_45: return Torizo_Instr_45(k, j);
  case fnTorizo_Instr_54: return Torizo_Instr_54(k, j);
  case fnShaktool_Instr_2: return Shaktool_Instr_2(k, j);
  case fnShaktool_Instr_3: return Shaktool_Instr_3(k, j);
  case fnShaktool_Instr_4: return Shaktool_Instr_4(k, j);
  case fnShaktool_Instr_5: return Shaktool_Instr_5(k, j);
  case fnShaktool_Instr_6: return Shaktool_Instr_6(k, j);
  case fnShaktool_Instr_1: return Shaktool_Instr_1(k, j);
  case fnShaktool_Instr_9: return Shaktool_Instr_9(k, j);
  case fnShaktool_Instr_11: return Shaktool_Instr_11(k, j);
  case fnShaktool_Instr_10: return Shaktool_Instr_10(k, j);
  case fnShaktool_Instr_8: return Shaktool_Instr_8(k, j);
  case fnShaktool_Instr_13: return Shaktool_Instr_13(k, j);
  case fnShaktool_Instr_12: return Shaktool_Instr_12(k, j);
  case fnShaktool_Instr_7: return Shaktool_Instr_7(k, j);
  case fnShaktool_Instr_14: return Shaktool_Instr_14(k, j);
  case fnEnemyInstr_Goto_B2: return EnemyInstr_Goto(k, j);
  case fnEnemyInstr_DecTimerAndGoto2_B2: return EnemyInstr_DecTimerAndGoto(k, j);
  case fnEnemyInstr_SetTimer_B2: return EnemyInstr_SetTimer(k, j);
  case fnEnemyInstr_Sleep_B2: return EnemyInstr_Sleep(k, j);
  case fnEnemyInstr_WaitNframes_B2: return EnemyInstr_WaitNframes(k, j);
  case fnSpacePirates_Instr_MovePixelsDownAndChangeDirFaceRight: return SpacePirates_Instr_MovePixelsDownAndChangeDirFaceRight(k, j);
  case fnSpacePirates_Instr_MovePixelsDownAndChangeDirFaceLeft: return SpacePirates_Instr_MovePixelsDownAndChangeDirFaceLeft(k, j);
  case fnSpacePirates_Instr_RandomNewDirFaceR: return SpacePirates_Instr_RandomNewDirFaceR(k, j);
  case fnSpacePirates_Instr_RandomNewDirFaceL: return SpacePirates_Instr_RandomNewDirFaceL(k, j);
  case fnSpacePirates_Instr_PrepareWallJumpR: return SpacePirates_Instr_PrepareWallJumpR(k, j);
  case fnSpacePirates_Instr_PrepareWallJumpL: return SpacePirates_Instr_PrepareWallJumpL(k, j);
  case fnSpacePirates_Instr_FireLaserL: return SpacePirates_Instr_FireLaserL(k, j);
  case fnSpacePirates_Instr_FireLaserR: return SpacePirates_Instr_FireLaserR(k, j);
  case fnSpacePirates_Instr_SetEnemyFunc: return SpacePirates_Instr_SetEnemyFunc(k, j);
  case fnSpacePirates_Instr_PlaySfx: return SpacePirates_Instr_PlaySfx(k, j);
  case fnSpacePirates_Instr_20: return SpacePirates_Instr_20(k, j);
  case fnSpacePirates_Instr_16: return SpacePirates_Instr_16(k, j);
  case fnSpacePirates_Instr_15: return SpacePirates_Instr_15(k, j);
  case fnSpacePirates_Instr_18: return SpacePirates_Instr_18(k, j);
  case fnSpacePirates_Instr_17: return SpacePirates_Instr_17(k, j);
  case fnSpacePirates_Instr_19: return SpacePirates_Instr_19(k, j);
  case fnSpacePirates_Instr_21: return SpacePirates_Instr_21(k, j);
  case fnSpacePirates_Instr_12: return SpacePirates_Instr_12(k, j);
  case fnSpacePirates_Instr_14: return SpacePirates_Instr_14(k, j);
  case fnSpacePirates_Instr_11: return SpacePirates_Instr_11(k, j);
  case fnSpacePirates_Instr_13: return SpacePirates_Instr_13(k, j);
  case fnEnemy_SetAiPreInstr_B3: return Enemy_SetAiPreInstr_B3(k, j);
  case fnEnemy_ClearAiPreInstr_B3: return Enemy_ClearAiPreInstr_B3(k, j);
  case fnEnemyInstr_Goto_B3: return EnemyInstr_Goto(k, j);
  case fnEnemyInstr_DecTimerAndGoto2_B3: return EnemyInstr_DecTimerAndGoto(k, j);
  case fnEnemyInstr_SetTimer_B3: return EnemyInstr_SetTimer(k, j);
  case fnEnemyInstr_Sleep_B3: return EnemyInstr_Sleep(k, j);
  case fnBotwoon_Instr_1: return Botwoon_Instr_1(k, j);
  case fnBotwoon_Instr_2: return Botwoon_Instr_2(k, j);
  case fnBotwoon_Instr_3: return Botwoon_Instr_3(k, j);
  case fnBotwoon_Instr_4: return Botwoon_Instr_4(k, j);
  case fnBotwoon_Instr_5: return Botwoon_Instr_5(k, j);
  case fnBotwoon_Instr_6: return Botwoon_Instr_6(k, j);
  case fnBotwoon_Instr_7: return Botwoon_Instr_7(k, j);
  case fnBotwoon_Instr_8: return Botwoon_Instr_8(k, j);
  case fnBotwoon_Instr_9: return Botwoon_Instr_9(k, j);
  case fnBotwoon_Instr_10: return Botwoon_Instr_10(k, j);
  case fnBotwoon_Instr_SetSpitting: return Botwoon_Instr_SetSpitting(k, j);
  case fnBotwoon_Instr_QueueSpitSfx: return Botwoon_Instr_QueueSpitSfx(k, j);
  case fnEscapeEtecoon_Instr_1: return EscapeEtecoon_Instr_1(k, j);
  case fnEscapeEtecoon_Instr_2: return EscapeEtecoon_Instr_2(k, j);
  case fnEscapeDachora_Instr_2: return EscapeDachora_Instr_2(k, j);
  case fnEscapeDachora_Instr_3: return EscapeDachora_Instr_3(k, j);
  case fnEscapeDachora_Instr_1: return EscapeDachora_Instr_1(k, j);
  case fnEscapeDachora_Instr_4: return EscapeDachora_Instr_4(k, j);
  case fnMotherBrain_Instr_MoveBodyUp10Left4: return MotherBrain_Instr_MoveBodyUp10Left4(k, j);
  case fnMotherBrain_Instr_MoveBodyUp16Left4: return MotherBrain_Instr_MoveBodyUp16Left4(k, j);
  case fnMotherBrain_Instr_MoveBodyUp12Right2: return MotherBrain_Instr_MoveBodyUp12Right2(k, j);
  case fnMotherBrain_Instr_MoveDown12Left4: return MotherBrain_Instr_MoveDown12Left4(k, j);
  case fnMotherBrain_Instr_MoveDown16Right2: return MotherBrain_Instr_MoveDown16Right2(k, j);
  case fnMotherBrain_Instr_MoveDown10Right2: return MotherBrain_Instr_MoveDown10Right2(k, j);
  case fnMotherBrain_Instr_MoveUp2Right1: return MotherBrain_Instr_MoveUp2Right1(k, j);
  case fnMotherBrain_Instr_MoveRight2: return MotherBrain_Instr_MoveRight2(k, j);
  case fnMotherBrain_Instr_MoveUp1: return MotherBrain_Instr_MoveUp1(k, j);
  case fnMotherBrain_Instr_MoveUp1Right3_Sfx: return MotherBrain_Instr_MoveUp1Right3_Sfx(k, j);
  case fnMotherBrain_Instr_Down2Right15: return MotherBrain_Instr_Down2Right15(k, j);
  case fnMotherBrain_Instr_Down4Right6: return MotherBrain_Instr_Down4Right6(k, j);
  case fnMotherBrain_Instr_Up4Left2: return MotherBrain_Instr_Up4Left2(k, j);
  case fnMotherBrain_Instr_Up2Left1_Sfx: return MotherBrain_Instr_Up2Left1_Sfx(k, j);
  case fnMotherBrain_Instr_Up2Left1_Sfx2: return MotherBrain_Instr_Up2Left1_Sfx2(k, j);
  case fnMotherBrain_Instr_MoveLeft2: return MotherBrain_Instr_MoveLeft2(k, j);
  case fnMotherBrain_Instr_MoveDown1: return MotherBrain_Instr_MoveDown1(k, j);
  case fnMotherBrain_Instr_MoveDown1Left3: return MotherBrain_Instr_MoveDown1Left3(k, j);
  case fnMotherBrain_Instr_MoveUp2Left15_Sfx: return MotherBrain_Instr_MoveUp2Left15_Sfx(k, j);
  case fnMotherBrain_Instr_MoveUp4Left6: return MotherBrain_Instr_MoveUp4Left6(k, j);
  case fnMotherBrain_Instr_MoveDown4Right2: return MotherBrain_Instr_MoveDown4Right2(k, j);
  case fnMotherBrain_Instr_MoveDown2Right1: return MotherBrain_Instr_MoveDown2Right1(k, j);
  case fnMotherBrain_Instr_SetPose_Standing: return MotherBrain_Instr_SetPose_Standing(k, j);
  case fnMotherBrain_Instr_SetPose_Walking: return MotherBrain_Instr_SetPose_Walking(k, j);
  case fnMotherBrain_Instr_SetPose_Crouched: return MotherBrain_Instr_SetPose_Crouched(k, j);
  case fnMotherBrain_Instr_SetPose_CrouchedTrans: return MotherBrain_Instr_SetPose_CrouchedTrans(k, j);
  case fnMotherBrain_Instr_SetPose_DeathBeamMode: return MotherBrain_Instr_SetPose_DeathBeamMode(k, j);
  case fnMotherBrain_Instr_SetPose_LeaningDown: return MotherBrain_Instr_SetPose_LeaningDown(k, j);
  case fnMotherBrain_Instr_SpawnEprojToOffset: return MotherBrain_Instr_SpawnEprojToOffset(k, j);
  case fnMotherBrain_Instr_SpawnDeathBeamEproj: return MotherBrain_Instr_SpawnDeathBeamEproj(k, j);
  case fnMotherBrain_Instr_IncrBeamAttackPhase: return MotherBrain_Instr_IncrBeamAttackPhase(k, j);
  default: Unreachable(); return NULL;
  }
}
