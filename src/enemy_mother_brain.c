// Enemy AI - Mother Brain runtime — peeled from Bank $A9

#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "enemy_types.h"
#include "sm_rtl.h"

#define g_word_A98929 ((uint16*)RomFixedPtr(0xa98929))
#define g_off_A98B7B ((uint16*)RomFixedPtr(0xa98b7b))
#define g_word_A9B099 ((uint16*)RomFixedPtr(0xa9b099))
#define g_word_A9B109 ((uint16*)RomFixedPtr(0xa9b109))
#define g_word_A9B10F ((uint16*)RomFixedPtr(0xa9b10f))
#define g_off_A9B6D4 ((uint16*)RomFixedPtr(0xa9b6d4))
#define MotherBrain_RainbowBeamPalettes ((uint16*)RomFixedPtr(0xade434))
#define g_off_A9C61E ((uint16*)RomFixedPtr(0xa9c61e))
#define g_off_A9C664 ((uint16*)RomFixedPtr(0xa9c664))
#define g_off_A9D260 ((uint16*)RomFixedPtr(0xa9d260))

static const uint16 g_word_A98B5D[5] = { 0x10, 0x10, 8, 8, 0x10 };
static const uint16 g_word_A98B67[5] = { 0x20, 0x20, 0x18, 0x18, 0x20 };
static const uint16 g_word_A98B71[5] = { 0xf8, 0xf8, 0xf0, 0xf0, 0xf6 };
static const uint16 g_word_A98C61[4] = { 0xfff8, 2, 0xfffc, 6 };
static const uint8 g_byte_A98F7D[2] = { 9, 0x12 };
static const uint16 g_word_A98F7F[8] = { 0x3d, 0x54, 0x20, 0x35, 0x5a, 0x43, 0x67, 0x29 };
static const int16 g_word_A993BB[4] = { 0, -1, 0, 1 };
static const int16 g_word_A993C3[4] = { 0, 1, -1, 1 };
static const uint16 g_word_A99E0F[13] = { 0x6f, 0x6f, 0x6f, 0x7e, 0x6f, 0x6f, 0x7e, 0x6f, 0x6f, 0x7e, 0x7e, 0x6f, 0x6f };
static const uint16 g_word_A9B393[8] = { 8, 0x6c, 0x18, 0x80, 9, 0x90, 0x18, 0x74 };
static const uint8 g_byte_A9B546[8] = { 0, 1, 1, 0, 0, 0, 0, 0 };
static const uint8 g_byte_A9B5A1[8] = { 2, 1, 1, 0, 0, 0, 0, 0 };
static const uint8 g_byte_A9B6DC[3] = { 0x40, 0x80, 0xc0 };
static const uint8 g_byte_A9B6DF[3] = { 0x10, 0x20, 0xd0 };
static const int16 g_word_A9B72C[28] = {
  -1, -17, 0, 0, -1,  -1,  0, -1,  0,  0, -1,  0, 0, 0, -17, -1,
  -1,  -1, 0, 0,  0, -17, -1, -1, -1, -1, -1, -1,
};
static const int16 g_word_A9BCA6[8] = { -8, 6, -4, 2, 3, -6, 8, 0 };
static const int16 g_word_A9BCB6[8] = { -7, 2, 5, -4, 6, -2, -6, 7 };
static const uint8 g_byte_A9BEEE[16] = { 2, 0, 2, 0, 6, 0, 6, 0, 8, 0, 8, 0, 10, 0, 10, 0 };
static const uint16 g_word_A9BEFE[8] = { 0x500, 0x500, 0x200, 0x200, 0xc0, 0xc0, 0x40, 0x40 };
static const int16 g_word_A9C049[8] = { 0x10, 0x10, 0x20, 0x20, 0x30, 0x30, 0x40, 0x40 };
static const uint16 g_word_A9C544 = 1;


void CallMotherBrainFunc(uint32 ea) {
  switch (ea) {
  case fnMotherBrainsBrain_SetupBrainToDraw: MotherBrainsBrain_SetupBrainToDraw(); return;
  case fnMotherBrainsBody_FirstPhase_DoubleRet: Unreachable(); return;
  case fnMotherBrainBody_0_Wait: MotherBrainBody_0_Wait(); return;
  case fnMotherBrainBody_1_ClearBottomLeftTube: MotherBrainBody_1_ClearBottomLeftTube(); return;
  case fnMotherBrainBody_2_SpawnTopRightTubeFalling: MotherBrainBody_2_SpawnTopRightTubeFalling(); return;
  case fnMotherBrainBody_3_ClearCeilingBlock9: MotherBrainBody_3_ClearCeilingBlock9(); return;
  case fnMotherBrainBody_4_SpawnTopLeftTubeFalling: MotherBrainBody_4_SpawnTopLeftTubeFalling(); return;
  case fnMotherBrainBody_4_ClearCeilingBlock6: MotherBrainBody_4_ClearCeilingBlock6(); return;
  case fnMotherBrainBody_5_SpawnTubeFallingEnemy1: MotherBrainBody_5_SpawnTubeFallingEnemy1(); return;
  case fnMotherBrainBody_6_ClearBottomRightTube: MotherBrainBody_6_ClearBottomRightTube(); return;
  case fnMotherBrainBody_7_SpawnTubeFallingEnemy2: MotherBrainBody_7_SpawnTubeFallingEnemy2(); return;
  case fnMotherBrainBody_8_ClearBottomMiddleLeftTube: MotherBrainBody_8_ClearBottomMiddleLeftTube(); return;
  case fnMotherBrainBody_9_SpawnTopMiddleLeftFalling: MotherBrainBody_9_SpawnTopMiddleLeftFalling(); return;
  case fnMotherBrainBody_10_ClearCeilingTubeColumn7: MotherBrainBody_10_ClearCeilingTubeColumn7(); return;
  case fnMotherBrainBody_11_SpawnTopMiddleRightFalling: MotherBrainBody_11_SpawnTopMiddleRightFalling(); return;
  case fnMotherBrainBody_12_ClearCeilingTubeColumn8: MotherBrainBody_12_ClearCeilingTubeColumn8(); return;
  case fnMotherBrainBody_13_SpawnTubeFallingEnemy3: MotherBrainBody_13_SpawnTubeFallingEnemy3(); return;
  case fnMotherBrainBody_14_ClearBottomMiddleRightTube: MotherBrainBody_14_ClearBottomMiddleRightTube(); return;
  case fnMotherBrainBody_15_SpawnTubeFallingEnemy4: MotherBrainBody_15_SpawnTubeFallingEnemy4(); return;
  case fnMotherBrainBody_16_ClearBottomMiddleTubes: MotherBrainBody_16_ClearBottomMiddleTubes(); return;
  case fnMotherBrainBody_FakeDeath_Descent_0_Pause: MotherBrainBody_FakeDeath_Descent_0_Pause(); return;
  case fnMotherBrainBody_FakeDeath_Descent_1: MotherBrainBody_FakeDeath_Descent_1(); return;
  case fnMotherBrainBody_FakeDeath_Descent_2: MotherBrainBody_FakeDeath_Descent_2(); return;
  case fnMotherBrainBody_FakeDeath_Descent_3: MotherBrainBody_FakeDeath_Descent_3(); return;
  case fnMotherBrainBody_FakeDeath_Descent_4: MotherBrainBody_FakeDeath_Descent_4(); return;
  case fnMotherBrainBody_FakeDeath_Descent_5: MotherBrainBody_FakeDeath_Descent_5(); return;
  case fnMotherBrainBody_FakeDeath_Descent_6: MotherBrainBody_FakeDeath_Descent_6(); return;
  case fnMotherBrainBody_FakeDeath_Ascent_0_DrawBG1Row23: MotherBrainBody_FakeDeath_Ascent_0_DrawBG1Row23(); return;
  case fnMotherBrainBody_FakeDeath_Ascent_1_DrawBG1Row45: MotherBrainBody_FakeDeath_Ascent_1_DrawBG1Row45(); return;
  case fnMotherBrainBody_FakeDeath_Ascent_2_DrawBG1Row67: MotherBrainBody_FakeDeath_Ascent_2_DrawBG1Row67(); return;
  case fnMotherBrainBody_FakeDeath_Ascent_3_DrawBG1Row89: MotherBrainBody_FakeDeath_Ascent_3_DrawBG1Row89(); return;
  case fnMotherBrainBody_FakeDeath_Ascent_4_DrawBG1RowAB: MotherBrainBody_FakeDeath_Ascent_4_DrawBG1RowAB(); return;
  case fnMotherBrainBody_FakeDeath_Ascent_5_DrawBG1RowCD: MotherBrainBody_FakeDeath_Ascent_5_DrawBG1RowCD(); return;
  case fnMotherBrainBody_FakeDeath_Ascent_6_SetupPhase2Gfx: MotherBrainBody_FakeDeath_Ascent_6_SetupPhase2Gfx(); return;
  case fnMotherBrainBody_FakeDeath_Ascent_7_SetupPhase2Brain: MotherBrainBody_FakeDeath_Ascent_7_SetupPhase2Brain(); return;
  case fnMotherBrainBody_FakeDeath_Ascent_8_Pause: MotherBrainBody_FakeDeath_Ascent_8_Pause(); return;
  case fnMotherBrainBody_FakeDeath_Ascent_9_PrepareRise: MotherBrainBody_FakeDeath_Ascent_9_PrepareRise(); return;
  case fnMotherBrainBody_FakeDeath_Ascent_10_LoadLegTiles: MotherBrainBody_FakeDeath_Ascent_10_LoadLegTiles(); return;
  case fnMotherBrainBody_FakeDeath_Ascent_11_ContinuePause: MotherBrainBody_FakeDeath_Ascent_11_ContinuePause(); return;
  case fnMotherBrainBody_FakeDeath_Ascent_12_StartMusic: MotherBrainBody_FakeDeath_Ascent_12_StartMusic(); return;
  case fnMotherBrainBody_FakeDeath_Ascent_13_Raise: MotherBrainBody_FakeDeath_Ascent_13_Raise(); return;
  case fnMotherBrainBody_FakeDeath_Ascent_14_WaitForUncouching: MotherBrainBody_FakeDeath_Ascent_14_WaitForUncouching(); return;
  case fnMotherBrainBody_FakeDeath_Ascent_15_TransitionFromGrey: MotherBrainBody_FakeDeath_Ascent_15_TransitionFromGrey(); return;
  case fnMotherBrainBody_2ndphase_16_ShakeHeadMenacingly: MotherBrainBody_2ndphase_16_ShakeHeadMenacingly(); return;
  case fnMotherBrainBody_2ndphase_17_BringHeadBackUp: MotherBrainBody_2ndphase_17_BringHeadBackUp(); return;
  case fnMotherBrainBody_2ndphase_18_FinishStretching: MotherBrainBody_2ndphase_18_FinishStretching(); return;
  case fnMotherBrain_Phase3_Death_0: MotherBrain_Phase3_Death_0(); return;
  case fnMotherBrain_Phase3_Death_1: MotherBrain_Phase3_Death_1(); return;
  case fnMotherBrain_Phase3_Death_2: MotherBrain_Phase3_Death_2(); return;
  case fnMotherBrain_Phase3_Death_3: MotherBrain_Phase3_Death_3(); return;
  case fnMotherBrain_Phase3_Death_4: MotherBrain_Phase3_Death_4(); return;
  case fnMotherBrain_Phase3_Death_5: MotherBrain_Phase3_Death_5(); return;
  case fnMotherBrain_Phase3_Death_6: MotherBrain_Phase3_Death_6(); return;
  case fnMotherBrain_Phase3_Death_7: MotherBrain_Phase3_Death_7(); return;
  case fnMotherBrain_Phase3_Death_8: MotherBrain_Phase3_Death_8(); return;
  case fnMotherBrain_Phase3_Death_9: MotherBrain_Phase3_Death_9(); return;
  case fnMotherBrain_Phase3_Death_10: MotherBrain_Phase3_Death_10(); return;
  case fnMotherBrain_Phase3_Death_11: MotherBrain_Phase3_Death_11(); return;
  case fnMotherBrain_Phase3_Death_12: MotherBrain_Phase3_Death_12(); return;
  case fnMotherBrain_Phase3_Death_13: MotherBrain_Phase3_Death_13(); return;
  case fnMotherBrain_Phase3_Death_14_20framedelay: MotherBrain_Phase3_Death_14_20framedelay(); return;
  case fnMotherBrain_Phase3_Death_15_LoadEscapeTimerTiles: MotherBrain_Phase3_Death_15_LoadEscapeTimerTiles(); return;
  case fnMotherBrain_Phase3_Death_16_StartEscape: MotherBrain_Phase3_Death_16_StartEscape(); return;
  case fnMotherBrain_Phase3_Death_17_SpawnTimeBomb: MotherBrain_Phase3_Death_17_SpawnTimeBomb(); return;
  case fnMotherBrain_Phase3_Death_18_TypesZebesText: MotherBrain_Phase3_Death_18_TypesZebesText(); return;
  case fnMotherBrain_Phase3_Death_19_EscapeDoorExploding: MotherBrain_Phase3_Death_19_EscapeDoorExploding(); return;
  case fnMotherBrain_Phase3_Death_20_BlowUpEscapeDoor: MotherBrain_Phase3_Death_20_BlowUpEscapeDoor(); return;
  case fnMotherBrain_Phase3_Death_21_KeepEarthquakeGoing: MotherBrain_Phase3_Death_21_KeepEarthquakeGoing(); return;
  case fnMotherBrain_Body_Phase2_Thinking: MotherBrain_Body_Phase2_Thinking(); return;
  case fnMotherBrain_Body_Phase2_TryAttack: MotherBrain_Body_Phase2_TryAttack(); return;
  case fnMotherBrain_FiringBomb_DecideOnWalking: MotherBrain_FiringBomb_DecideOnWalking(); return;
  case fnMotherBrain_FiringBomb_WalkingBackwards: MotherBrain_FiringBomb_WalkingBackwards(); return;
  case fnMotherBrain_FiringBomb_Crouch: MotherBrain_FiringBomb_Crouch(); return;
  case fnMotherBrain_FiringBomb_Fired: MotherBrain_FiringBomb_Fired(); return;
  case fnMotherBrain_FiringBomb_Standup: MotherBrain_FiringBomb_Standup(); return;
  case fnMotherBomb_FiringLaser_PositionHead: MotherBomb_FiringLaser_PositionHead(); return;
  case fnMotherBomb_FiringLaser_PositionHeadSlowlyFire: MotherBomb_FiringLaser_PositionHeadSlowlyFire(); return;
  case fnMotherBomb_FiringLaser_FinishAttack: MotherBomb_FiringLaser_FinishAttack(); return;
  case fnMotherBomb_FiringDeathBeam: MotherBomb_FiringDeathBeam(); return;
  case fnMotherBomb_FiringRainbowBeam_0: MotherBomb_FiringRainbowBeam_0(); return;
  case fnMotherBomb_FiringRainbowBeam_1_StartCharge: MotherBomb_FiringRainbowBeam_1_StartCharge(); return;
  case fnMotherBomb_FiringRainbowBeam_2_RetractNeck: MotherBomb_FiringRainbowBeam_2_RetractNeck(); return;
  case fnMotherBomb_FiringRainbowBeam_3_Wait: MotherBomb_FiringRainbowBeam_3_Wait(); return;
  case fnMotherBomb_FiringRainbowBeam_4_ExtendNeckDown: MotherBomb_FiringRainbowBeam_4_ExtendNeckDown(); return;
  case fnMotherBomb_FiringRainbowBeam_5_StartFiring: MotherBomb_FiringRainbowBeam_5_StartFiring(); return;
  case fnMotherBomb_FiringRainbowBeam_6_MoveSamusToWall: MotherBomb_FiringRainbowBeam_6_MoveSamusToWall(); return;
  case fnMotherBomb_FiringRainbowBeam_7_DelayFrame: MotherBomb_FiringRainbowBeam_7_DelayFrame(); return;
  case fnMotherBomb_FiringRainbowBeam_8_StartDrainSamus: MotherBomb_FiringRainbowBeam_8_StartDrainSamus(); return;
  case fnMotherBomb_FiringRainbowBeam_9_DrainingSamus: MotherBomb_FiringRainbowBeam_9_DrainingSamus(); return;
  case fnMotherBomb_FiringRainbowBeam_10_FinishFiringRainbow: MotherBomb_FiringRainbowBeam_10_FinishFiringRainbow(); return;
  case fnMotherBomb_FiringRainbowBeam_11_LetSamusFall: MotherBomb_FiringRainbowBeam_11_LetSamusFall(); return;
  case fnMotherBomb_FiringRainbowBeam_12_WaitForSamusHitGround: MotherBomb_FiringRainbowBeam_12_WaitForSamusHitGround(); return;
  case fnMotherBomb_FiringRainbowBeam_13_LowerHead: MotherBomb_FiringRainbowBeam_13_LowerHead(); return;
  case fnMotherBomb_FiringRainbowBeam_14_DecideNextAction: MotherBomb_FiringRainbowBeam_14_DecideNextAction(); return;
  case fnMotherBrain_Phase2Cut_0: MotherBrain_Phase2Cut_0(); return;
  case fnMotherBrain_Phase2Cut_1: MotherBrain_Phase2Cut_1(); return;
  case fnMotherBrain_Phase2Cut_2: MotherBrain_Phase2Cut_2(); return;
  case fnMotherBrain_Phase2Cut_3: MotherBrain_Phase2Cut_3(); return;
  case fnMotherBrain_Phase2Cut_4: MotherBrain_Phase2Cut_4(); return;
  case fnMotherBrain_Phase2Cut_5: MotherBrain_Phase2Cut_5(); return;
  case fnnullsub_364: return;
  case fnMotherBrain_DrainedByShitroid_0: MotherBrain_DrainedByShitroid_0(); return;
  case fnMotherBrain_DrainedByShitroid_1: MotherBrain_DrainedByShitroid_1(); return;
  case fnMotherBrain_DrainedByShitroid_2: MotherBrain_DrainedByShitroid_2(); return;
  case fnMotherBrain_DrainedByShitroid_3: MotherBrain_DrainedByShitroid_3(); return;
  case fnMotherBrain_DrainedByShitroid_4: MotherBrain_DrainedByShitroid_4(); return;
  case fnMotherBrain_DrainedByShitroid_5: MotherBrain_DrainedByShitroid_5(); return;
  case fnMotherBrain_DrainedByShitroid_6: MotherBrain_DrainedByShitroid_6(); return;
  case fnMotherBrain_DrainedByShitroid_7: MotherBrain_DrainedByShitroid_7(); return;
  case fnMotherBrain_Phase2_Revive_0: MotherBrain_Phase2_Revive_0(); return;
  case fnMotherBrain_Phase2_Revive_1: MotherBrain_Phase2_Revive_1(); return;
  case fnMotherBrain_Phase2_Revive_2: MotherBrain_Phase2_Revive_2(); return;
  case fnMotherBrain_Phase2_Revive_3: MotherBrain_Phase2_Revive_3(); return;
  case fnMotherBrain_Phase2_Revive_4: MotherBrain_Phase2_Revive_4(); return;
  case fnMotherBrain_Phase2_Revive_5: MotherBrain_Phase2_Revive_5(); return;
  case fnMotherBrain_Phase2_Revive_6: MotherBrain_Phase2_Revive_6(); return;
  case fnMotherBrain_Phase2_Revive_7: MotherBrain_Phase2_Revive_7(); return;
  case fnMotherBrain_Phase2_Revive_8: MotherBrain_Phase2_Revive_8(); return;
  case fnMotherBrain_Phase2_MurderShitroid_1: MotherBrain_Phase2_MurderShitroid_1(); return;
  case fnMotherBrain_Phase2_MurderShitroid_2: MotherBrain_Phase2_MurderShitroid_2(); return;
  case fnMotherBrain_Phase2_PrepareForFinalShitroid: MotherBrain_Phase2_PrepareForFinalShitroid(); return;
  case fnMotherBrain_Phase2_ExecuteFinalkShitroid: MotherBrain_Phase2_ExecuteFinalkShitroid(); return;
  case fnnullsub_363: return;
  case fnMotherBrain_Phase3_Recover_MakeDistance: MotherBrain_Phase3_Recover_MakeDistance(); return;
  case fnMotherBrain_Phase3_Recover_SetupForFight: MotherBrain_Phase3_Recover_SetupForFight(); return;
  case fnMotherBrain_Phase3_Fighting_Main: MotherBrain_Phase3_Fighting_Main(); return;
  case fnMotherBrain_Phase3_Fighting_Cooldown: MotherBrain_Phase3_Fighting_Cooldown(); return;
  case fnnullsub_365: return;
  case fnnullsub_367: return;
  case fnMotherBrainsBrain_SetupBrainAndNeckToDraw: MotherBrainsBrain_SetupBrainAndNeckToDraw(); return;
  default: Unreachable();
  }
}


void MotherBrainsBody_Init(void) {  // 0xA98687
  for (int i = 4094; i >= 0; i -= 2)
    tilemap_stuff[i >> 1] = 824;
  MotherBrain_SetBodyInstrs(addr_kMotherBrain_Ilist_9C13);
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->base.vram_tiles_index = 0;
  E->base.properties |= kEnemyProps_BlockPlasmaBeam | kEnemyProps_Intangible | kEnemyProps_Invisible;
  E->base.palette_index = 0;
  WriteColorsToTargetPalette(0xa9, 0x162, addr_kMotherBrainPalette_4 + 2, 0xF);
  WriteColorsToTargetPalette(0xa9, 0x1E2, addr_kMotherBrainPalette_3 + 2, 0xF);
  E->mbn_var_00 = 0;

  Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
  E1->mbn_var_02 = 0;
  E->mbn_var_04 = 2;
  E1->mbn_var_A = FUNC16(MotherBrainsBrain_SetupBrainToDraw);
  E->mbn_var_A = FUNC16(MotherBrainsBody_FirstPhase_DoubleRet);
  LoadFxEntry(1);
  uint16 v3 = 0, v4;
  do {
    v4 = v3;
    SpawnEprojWithRoomGfx(addr_kEproj_MotherBrainRoomTurrets, v3);
    v3 = v4 + 1;
  } while ((uint16)(v4 + 1) < 0xC);
}

void MotherBrainsBrain_Init(void) {  // 0xA98705
  InitializeEnemyCorpseRotting(0x40, addr_stru_A9DE08);
  Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
  E1->base.health = 3000;
  MotherBrain_SetBrainUnusedInstrs(addr_kMotherBrain_Ilist_9C13);
  E1->base.vram_tiles_index = 0;
  E1->base.properties |= 0x1100;
  E1->base.palette_index = 512;
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->mbn_var_0C = 512;
  E->mbn_var_0D = 512;
  MotherBrain_SetBrainInstrs(addr_stru_A99C21);
  MotherBrain_SetupBrainNormalPal();
}

void MotherBrainsBody_Hurt(void) {  // 0xA9873E
  MotherBrain_Pal_HandleRoomPal();
  Enemy_MotherBrain *E = Get_MotherBrain(0);

  if (E->mbn_var_A == (uint16)fnMotherBrainsBody_FirstPhase_DoubleRet) {
    MotherBrainsBody_FirstPhase_DoubleRet();
    return;
  }

  CallMotherBrainFunc(E->mbn_var_A | 0xA90000);
  MotherBrain_HandlePalette();
  MotherBrain_SamusCollDetect();
  MotherBrain_Pal_ProcessInvincibility();
  if (Get_MotherBrain(0x40)->mbn_var_02)
    mov24(&unpause_hook, fnMotherBrainsBody_UnpauseHook);
}

CoroutineRet MotherBrainsBody_UnpauseHook(void) {  // 0xA98763
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if (E->mbn_var_16)
    QueueSfx1_Max6(0x40);
  if ((E->base.extra_properties & 4) != 0) {
    enemy_bg2_tilemap_size = 2048;
    nmi_flag_bg2_enemy_vram_transfer = 1;
  }
  return kCoroutineNone;
}

void MotherBrainsBody_Powerbomb(void) {  // 0xA98787
  NormalEnemyPowerBombAiSkipDeathAnim_CurEnemy();
  MotherBrainsBrain_Hurt();
}

void MotherBrainsBrain_Hurt(void) {  // 0xA9878B
  mov24(&enemy_gfx_drawn_hook, 0xA98786);
  EnemyData *v0 = gEnemyData(0);
  if ((v0[1].properties & 0x100) != 0)
    CallMotherBrainFunc(v0[1].ai_var_A | 0xA90000);
}

void MotherBrainsBrain_SetupBrainAndNeckToDraw(void) {  // 0xA987A2
  if (!time_is_frozen_flag)
    MotherBrain_HandleNeck();
  Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
  E1->base.x_pos = E1->mbn_var_2E;
  E1->base.y_pos = E1->mbn_var_2F - 21;
  mov24(&enemy_gfx_drawn_hook, 0xA987C9);
}

void MotherBrain_DrawBrainNeck_EnemyGfxDrawHook(void) {  // 0xA987C9
  MotherBrain_DrawBrain();
  MotherBrain_DrawNeck();
}

void MotherBrainsBrain_SetupBrainToDraw(void) {  // 0xA987D0
  mov24(&enemy_gfx_drawn_hook, 0xA987DD);
}

void MotherBrainsBrain_GfxDrawHook(void) {  // 0xA987DD
  MotherBrain_DrawBrain();
}

void MotherBrainsBody_FirstPhase_DoubleRet(void) {  // 0xA987E1
  if (CheckEventHappened(2)) {
    Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
    E1->mbn_var_00 = earthquake_timer;
    if (sign16(samus_x_pos - 236)) {
      if (!E1->base.health) {
        Enemy_MotherBrain *E = Get_MotherBrain(0);
        E->mbn_var_1D = 1;
        E->mbn_var_00 = 1;
        DisableMinimapAndMarkBossRoomAsExplored();
        QueueMusic_Delayed8(6);
        MotherBrain_SealWall();
      }
    }
  }
  MotherBrain_SamusCollDetect();
}

void MotherBrainBody_FakeDeath_Descent_0_Pause(void) {  // 0xA9881D
  Enemy_MotherBrainBody *E = Get_MotherBrainBody(0);
  E->mbby_var_A = FUNC16(MotherBrainBody_FakeDeath_Descent_1);
  E->mbby_var_F = 64;
  MotherBrainBody_FakeDeath_Descent_1();
}

void MotherBrainBody_FakeDeath_Descent_1(void) {  // 0xA98829
  Enemy_MotherBrainBody *E = Get_MotherBrainBody(0);
  if ((--E->mbby_var_F & 0x8000) != 0) {
    CallSomeSamusCode(0);
    *(uint16 *)scrolls = scrolls[0];
    E->mbby_var_A = FUNC16(MotherBrainBody_FakeDeath_Descent_2);
    E->mbby_var_F = 32;
    MotherBrainBody_FakeDeath_Descent_2();
  }
}

void MotherBrainBody_FakeDeath_Descent_2(void) {  // 0xA9884D
  Enemy_MotherBrainBody *E = Get_MotherBrainBody(0);
  if ((--E->mbby_var_F & 0x8000) != 0) {
    QueueMusic_Delayed8(0);
    QueueMusic_Delayed8(0xFF21);
    E->mbby_var_A = FUNC16(MotherBrainBody_FakeDeath_Descent_3);
    E->mbby_var_F = 12;
    MotherBrainBody_FakeDeath_Descent_3();
  }
}

void MotherBrainBody_FakeDeath_Descent_3(void) {  // 0xA9886C
  Enemy_MotherBrainBody *E = Get_MotherBrainBody(0);
  if ((--E->mbby_var_F & 0x8000) != 0) {
    CallSomeSamusCode(1);
    E->mbby_var_A = FUNC16(MotherBrainBody_FakeDeath_Descent_4);
    E->mbby_var_F = 8;
    MotherBrainBody_FakeDeath_Descent_4();
  }
}

void MotherBrainBody_FakeDeath_Descent_4(void) {  // 0xA98884
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0) {
    MotherBrain_Pal_BeginScreenFlash();
    LoadFxEntry(2);
    Get_MotherBrain(0x40)->mbn_var_E = FUNC16(MotherBrainBody_0_Wait);
    E->mbn_var_A = FUNC16(MotherBrainBody_FakeDeath_Descent_5);
    E->mbn_var_F = 0;
    E->mbn_var_37 = 0;
    SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x0e, 0x02, 0xb6b3 });
  }
}

void MotherBrainBody_FakeDeath_Descent_5(void) {  // 0xA988B2
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0) {
    E->mbn_var_F = 8;
    uint16 v2 = E->mbn_var_37 + 1;
    E->mbn_var_37 = v2;
    if (MotherBrain_FadeToGray_FakeDeath(v2 - 1) & 1)
      E->mbn_var_A = FUNC16(MotherBrainBody_FakeDeath_Descent_6);
  }
  MotherBrainBody_FakeDeath_Descent_6();
}

void MotherBrainBody_FakeDeath_Descent_6(void) {  // 0xA988D3
  Enemy_MotherBrainBody *E = Get_MotherBrainBody(0x40);
  CallMotherBrainFunc(E->mbby_var_E | 0xA90000);
  MotherBrain_HandleFakeDeathExplosions();
}

void MotherBrain_HandleFakeDeathExplosions(void) {  // 0xA988DD
  int16 v1;
  int16 v2;

  Enemy_MotherBrain *E = Get_MotherBrain(0);
  v1 = E->mbn_var_38 - 1;
  if (v1 < 0) {
    E->mbn_var_38 = 8;
    v2 = E->mbn_var_39 - 1;
    if (v2 < 0)
      v2 = 7;
    E->mbn_var_39 = v2;
    int v3 = (uint16)(4 * v2) >> 1;
    eproj_spawn_pt = (Point16U){ g_word_A98929[v3], g_word_A98929[v3 + 1] };
    uint16 v4 = (random_number < 0x4000) ? 12 : 3;
    SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, v4);
    QueueSfx2_Max3(0x24);
  } else {
    E->mbn_var_38 = v1;
  }
}

void MotherBrainBody_0_Wait(void) {  // 0xA98949
  int16 v1;

  uint16 v0 = 0;
  v1 = 0;
  do {
    if (!*(uint16 *)((uint8 *)eproj_id + v0))
      ++v1;
    v0 += 2;
  } while ((int16)(v0 - 36) < 0);
  if ((int16)(v1 - 4) >= 0) {
    SpawnEnemy(0xA9, addr_stru_A98AE5);
    Get_MotherBrainBody(0x40)->mbby_var_E = FUNC16(MotherBrainBody_1_ClearBottomLeftTube);
  }
}

void MotherBrainBody_1_ClearBottomLeftTube(void) {  // 0xA9896E
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x05, 0x09, 0xb6c3 });
  Enemy_MotherBrainBody *E = Get_MotherBrainBody(0x40);
  E->mbby_var_E = FUNC16(MotherBrainBody_2_SpawnTopRightTubeFalling);
  E->mbby_var_F = 32;
}

void MotherBrainBody_2_SpawnTopRightTubeFalling(void) {  // 0xA98983
  Enemy_MotherBrainBody *E = Get_MotherBrainBody(0x40);
  if ((--E->mbby_var_F & 0x8000) != 0) {
    eproj_spawn_pt = (Point16U){ 152, 47 };
    SpawnEprojWithRoomGfx(0xCC5B, 0x2F);
    E->mbby_var_E = -30304;
  }
}

void MotherBrainBody_3_ClearCeilingBlock9(void) {  // 0xA989A0
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x09, 0x02, 0xb6b3 });
  Enemy_MotherBrainBody *E = Get_MotherBrainBody(0x40);
  E->mbby_var_E = FUNC16(MotherBrainBody_4_SpawnTopLeftTubeFalling);
  E->mbby_var_F = 32;
}

void MotherBrainBody_4_SpawnTopLeftTubeFalling(void) {  // 0xA989B5
  Enemy_MotherBrainBody *E = Get_MotherBrainBody(0x40);
  if ((--E->mbby_var_F & 0x8000) != 0) {
    eproj_spawn_pt = (Point16U){ 104, 47 };
    SpawnEprojWithRoomGfx(addr_kEproj_MotherBrainTubeFalling_TopLeft, 0x2F);
    E->mbby_var_E = FUNC16(MotherBrainBody_4_ClearCeilingBlock6);
  }
}

void MotherBrainBody_4_ClearCeilingBlock6(void) {  // 0xA989D2
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x06, 0x02, 0xb6b3 });
  Enemy_MotherBrainBody *E = Get_MotherBrainBody(0x40);
  E->mbby_var_E = FUNC16(MotherBrainBody_5_SpawnTubeFallingEnemy1);
  E->mbby_var_F = 32;
}

void MotherBrainBody_5_SpawnTubeFallingEnemy1(void) {  // 0xA989E7
  Enemy_MotherBrainBody *E = Get_MotherBrainBody(0x40);
  if ((--E->mbby_var_F & 0x8000) != 0) {
    SpawnEnemy(0xA9, addr_stru_A98AF5);
    E->mbby_var_E = FUNC16(MotherBrainBody_6_ClearBottomRightTube);
  }
}

void MotherBrainBody_6_ClearBottomRightTube(void) {  // 0xA989FA
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x0a, 0x09, 0xb6c7 });
  Enemy_MotherBrainBody *E = Get_MotherBrainBody(0x40);
  E->mbby_var_E = FUNC16(MotherBrainBody_7_SpawnTubeFallingEnemy2);
  E->mbby_var_F = 32;
}

void MotherBrainBody_7_SpawnTubeFallingEnemy2(void) {  // 0xA98A0F
  Enemy_MotherBrainBody *E = Get_MotherBrainBody(0x40);
  if ((--E->mbby_var_F & 0x8000) != 0) {
    SpawnEnemy(0xA9, addr_stru_A98B05);
    E->mbby_var_E = FUNC16(MotherBrainBody_8_ClearBottomMiddleLeftTube);
  }
}

void MotherBrainBody_8_ClearBottomMiddleLeftTube(void) {  // 0xA98A22
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x06, 0x0a, 0xb6bb });
  Enemy_MotherBrainBody *E = Get_MotherBrainBody(0x40);
  E->mbby_var_E = FUNC16(MotherBrainBody_9_SpawnTopMiddleLeftFalling);
  E->mbby_var_F = 32;
}

void MotherBrainBody_9_SpawnTopMiddleLeftFalling(void) {  // 0xA98A37
  Enemy_MotherBrainBody *E = Get_MotherBrainBody(0x40);
  if ((--E->mbby_var_F & 0x8000) != 0) {
    eproj_spawn_pt = (Point16U){ 120, 59 };
    SpawnEprojWithRoomGfx(addr_kEproj_MotherBrainTubeFalling_TopMiddleLeft, 0x3B);
    E->mbby_var_E = FUNC16(MotherBrainBody_10_ClearCeilingTubeColumn7);
  }
}

void MotherBrainBody_10_ClearCeilingTubeColumn7(void) {  // 0xA98A54
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x07, 0x02, 0xb6b7 });
  Enemy_MotherBrainBody *E = Get_MotherBrainBody(0x40);
  E->mbby_var_E = FUNC16(MotherBrainBody_11_SpawnTopMiddleRightFalling);
  E->mbby_var_F = 32;
}

void MotherBrainBody_11_SpawnTopMiddleRightFalling(void) {  // 0xA98A69
  Enemy_MotherBrainBody *E = Get_MotherBrainBody(0x40);
  if ((--E->mbby_var_F & 0x8000) != 0) {
    eproj_spawn_pt = (Point16U){ 136, 59 };
    SpawnEprojWithRoomGfx(addr_kEproj_MotherBrainTubeFalling_TopMiddleRight, 0x3B);
    E->mbby_var_E = FUNC16(MotherBrainBody_12_ClearCeilingTubeColumn8);
  }
}

void MotherBrainBody_12_ClearCeilingTubeColumn8(void) {  // 0xA98A86
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x08, 0x02, 0xb6b7 });
  Enemy_MotherBrainBody *E = Get_MotherBrainBody(0x40);
  E->mbby_var_E = FUNC16(MotherBrainBody_13_SpawnTubeFallingEnemy3);
  E->mbby_var_F = 32;
}

void MotherBrainBody_13_SpawnTubeFallingEnemy3(void) {  // 0xA98A9B
  Enemy_MotherBrainBody *E = Get_MotherBrainBody(0x40);
  if ((--E->mbby_var_F & 0x8000) != 0) {
    SpawnEnemy(0xA9, FUNC16(Eproj_Init_0x8bde_SkreeDownLeft));
    E->mbby_var_E = FUNC16(MotherBrainBody_14_ClearBottomMiddleRightTube);
  }
}

void MotherBrainBody_14_ClearBottomMiddleRightTube(void) {  // 0xA98AAE
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x09, 0x0a, 0xb6bb });
  Enemy_MotherBrainBody *E = Get_MotherBrainBody(0x40);
  E->mbby_var_E = FUNC16(MotherBrainBody_15_SpawnTubeFallingEnemy4);
  E->mbby_var_F = 2;
}

void MotherBrainBody_15_SpawnTubeFallingEnemy4(void) {  // 0xA98AC3
  Enemy_MotherBrainBody *E = Get_MotherBrainBody(0x40);
  if ((--E->mbby_var_F & 0x8000) != 0) {
    SpawnEnemy(0xA9, addr_stru_A98B25);
    E->mbby_var_E = FUNC16(MotherBrainBody_16_ClearBottomMiddleTubes);
  }
}

void MotherBrainBody_16_ClearBottomMiddleTubes(void) {  // 0xA98AD6
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x07, 0x07, 0xb6bf });
  Get_MotherBrainBody(0x40)->mbby_var_E = addr_locret_A98AE4;
}

void MotherBrainsTubesFalling_Init(void) {  // 0xA98B35
  Enemy_MotherBrainsTubesFalling *E = Get_MotherBrainsTubesFalling(cur_enemy_index);
  int v1 = E->mbtfg_parameter_1 >> 1;
  E->base.x_width = g_word_A98B5D[v1];
  E->base.y_height = g_word_A98B67[v1];
  E->mbtfg_var_B = g_word_A98B71[v1];
  E->mbtfg_var_D = 0;
  E->mbtfg_var_E = 0;
  E->mbtfg_var_C = 0;
  E->mbtfg_var_A = g_off_A98B7B[v1];
}

void CallMotherBrainsTubesFalling(uint32 ea, uint16 k) {
  switch (ea) {
  case fnMotherBrainsTubesFalling_Main_NonMain: MotherBrainsTubesFalling_Main_NonMain(k); return;
  case fnMotherBrainsTubesFalling_WaitToFall: MotherBrainsTubesFalling_WaitToFall(k); return;
  case fnMotherBrainsTubesFalling_Falling: MotherBrainsTubesFalling_Falling(k); return;
  default: Unreachable();
  }
}

void MotherBrainsTubesFalling_Main(uint16 k) {  // 0xA98B85
  Enemy_MotherBrainsTubesFalling *E = Get_MotherBrainsTubesFalling(k);
  CallMotherBrainsTubesFalling(E->mbtfg_var_A | 0xA90000, k);
}

void MotherBrainsTubesFalling_Main_NonMain(uint16 k) {  // 0xA98B88
  Enemy_MotherBrainsTubesFalling *E = Get_MotherBrainsTubesFalling(k);
  uint16 v2 = E->mbtfg_var_C + 6;
  E->mbtfg_var_C = v2;
  Enemy_IncreaseYpos(k, v2);
  if ((int16)(E->base.y_pos - E->mbtfg_var_B) < 0)
    MotherBrainsTubesFalling_HandleSmoke(k);
  else
    MotherBrainsTubesFalling_Explode(k);
}

void MotherBrainsTubesFalling_HandleSmoke(uint16 k) {  // 0xA98B9D
  Enemy_MotherBrainsTubesFalling *E = Get_MotherBrainsTubesFalling(k);
  if ((--E->mbtfg_var_D & 0x8000) != 0)
    MotherBrainsTubesFalling_SpawnSmoke(k);
}

void MotherBrainsTubesFalling_Explode(uint16 k) {  // 0xA98BA6
  Enemy_MotherBrainsTubesFalling *E = Get_MotherBrainsTubesFalling(k);
  E->base.properties |= kEnemyProps_Deleted;
  eproj_spawn_pt = (Point16U){ E->base.x_pos, E->base.y_pos };
  SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 3);
  QueueSfx2_Max3(0x24);
}

void MotherBrainsTubesFalling_WaitToFall(uint16 k) {  // 0xA98BCB
  Enemy_MotherBrainsTubesFalling *E = Get_MotherBrainsTubesFalling(k);
  if ((--E->mbtfg_parameter_2 & 0x8000) != 0) {
    E->mbtfg_var_A = FUNC16(MotherBrainsTubesFalling_Falling);
    MotherBrainsTubesFalling_Falling(k);
  }
}

void MotherBrainsTubesFalling_Falling(uint16 k) {  // 0xA98BD6
  Enemy_MotherBrainsTubesFalling *E = Get_MotherBrainsTubesFalling(k);
  uint16 v2 = E->mbtfg_var_C + 6;
  E->mbtfg_var_C = v2;
  Enemy_IncreaseYpos(k, v2);
  uint16 ypos = E->base.y_pos;
  if (!sign16(ypos - 244))
    E->base.properties |= kEnemyProps_Invisible;
  Enemy_MotherBrainsTubesFalling *E1 = Get_MotherBrainsTubesFalling(0x40);
  E1->base.y_pos = ypos - 56;
  if (sign16(E1->base.y_pos - 0xc4)) {
    MotherBrainsTubesFalling_HandleSmoke(k);
  } else {
    MotherBrain_Pal_EndScreenFlash();
    EnableEarthquakeAframes(0x19);
    hdma_object_channels_bitmask[0] = 0;
    hdma_object_channels_bitmask[1] = 0;
    // BUG!
    //E->mbtfg_var_C = 0;
    E1->base.y_pos = 196;
    Enemy_MotherBrainsTubesFalling *E0 = Get_MotherBrainsTubesFalling(0);
    E0->base.x_pos = 59;
    E0->base.y_pos = 279;
    MotherBrain_SetupNeckForFakeAscent();
    E0->mbtfg_var_A = FUNC16(MotherBrainBody_FakeDeath_Ascent_0_DrawBG1Row23);
    MotherBrainsTubesFalling_Explode(cur_enemy_index);
  }
}

void MotherBrainsTubesFalling_SpawnSmoke(uint16 k) {  // 0xA98C36
  Enemy_MotherBrainsTubesFalling *E = Get_MotherBrainsTubesFalling(k);
  E->mbtfg_var_D = 8;
  uint16 v2 = (E->mbtfg_var_E + 1) & 3;
  E->mbtfg_var_E = v2;
  eproj_spawn_pt = (Point16U){ E->base.x_pos + g_word_A98C61[v2], 208 };
  SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 9);
}

void MotherBrainBody_FakeDeath_Ascent_0_DrawBG1Row23(void) {  // 0xA98C87
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x02, 0x02, 0xb67b });
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x02, 0x03, 0xb67f });
  Get_MotherBrainBody(0)->mbby_var_A = FUNC16(MotherBrainBody_FakeDeath_Ascent_1_DrawBG1Row45);
}

void MotherBrainBody_FakeDeath_Ascent_1_DrawBG1Row45(void) {  // 0xA98C9E
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x02, 0x04, 0xb683 });
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x02, 0x05, 0xb687 });
  Get_MotherBrainBody(0)->mbby_var_A = FUNC16(MotherBrainBody_FakeDeath_Ascent_2_DrawBG1Row67);
}

void MotherBrainBody_FakeDeath_Ascent_2_DrawBG1Row67(void) {  // 0xA98CB5
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x02, 0x06, 0xb68b });
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x02, 0x07, 0xb68f });
  Get_MotherBrainBody(0)->mbby_var_A = FUNC16(MotherBrainBody_FakeDeath_Ascent_3_DrawBG1Row89);
}

void MotherBrainBody_FakeDeath_Ascent_3_DrawBG1Row89(void) {  // 0xA98CCC
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x02, 0x08, 0xb693 });
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x02, 0x09, 0xb697 });
  Get_MotherBrainBody(0)->mbby_var_A = FUNC16(MotherBrainBody_FakeDeath_Ascent_4_DrawBG1RowAB);
}

void MotherBrainBody_FakeDeath_Ascent_4_DrawBG1RowAB(void) {  // 0xA98CE3
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x02, 0x0a, 0xb69b });
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x02, 0x0b, 0xb69f });
  Get_MotherBrainBody(0)->mbby_var_A = FUNC16(MotherBrainBody_FakeDeath_Ascent_5_DrawBG1RowCD);
}

void MotherBrainBody_FakeDeath_Ascent_5_DrawBG1RowCD(void) {  // 0xA98CFA
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x02, 0x0c, 0xb6a3 });
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x02, 0x0d, 0xb6a7 });
  Get_MotherBrainBody(0)->mbby_var_A = FUNC16(MotherBrainBody_FakeDeath_Ascent_6_SetupPhase2Gfx);
}

void MotherBrainBody_FakeDeath_Ascent_6_SetupPhase2Gfx(void) {  // 0xA98D11
  *(uint16 *)&layer2_scroll_x = 257;
  *(uint16 *)&reg_BG2SC &= 0xFFFC;
  WriteColorsToPalette(0x142, 0xa9, addr_kMotherBrainPalette_1 + 2, 0xF);
  WriteColorsToPalette(0x162, 0xa9, addr_kMotherBrainPalette_0 + 2, 0xF);
  Get_MotherBrain(0)->mbn_var_A = FUNC16(MotherBrainBody_FakeDeath_Ascent_7_SetupPhase2Brain);
  nmi_flag_bg2_enemy_vram_transfer = 1;
  Get_MotherBrain(0x40)->mbn_var_02 = 1;
}

void MotherBrainBody_FakeDeath_Ascent_7_SetupPhase2Brain(void) {  // 0xA98D49
  fx_layer_blending_config_a = 52;
  Enemy_MotherBrainBody *E = Get_MotherBrainBody(0x40);
  E->mbby_var_A = FUNC16(MotherBrainsBrain_SetupBrainAndNeckToDraw);
  Enemy_MotherBrainBody *E0 = Get_MotherBrainBody(0);
  E0->base.properties &= ~kEnemyProps_Intangible;
  E->base.properties &= ~0x400;
  E->base.health = 18000;
  E0->mbby_var_A = FUNC16(MotherBrainBody_FakeDeath_Ascent_8_Pause);
  E0->mbby_var_F = 128;
  MotherBrainBody_FakeDeath_Ascent_8_Pause();
}

void MotherBrainBody_FakeDeath_Ascent_8_Pause(void) {  // 0xA98D79
  Enemy_MotherBrainBody *E = Get_MotherBrainBody(0);
  if ((--E->mbby_var_F & 0x8000) != 0) {
    E->mbby_var_A = FUNC16(MotherBrainBody_FakeDeath_Ascent_9_PrepareRise);
    E->mbby_var_F = 32;
    MotherBrainBody_FakeDeath_Ascent_9_PrepareRise();
  }
}

void MotherBrainBody_FakeDeath_Ascent_9_PrepareRise(void) {  // 0xA98D8B
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0) {
    E->mbn_var_09 = MotherBrainRisingHdmaObject();
    Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
    E1->base.properties |= 0x100;
    MotherBrain_SetBrainInstrs(addr_stru_A99C21);
    E->mbn_var_A = FUNC16(MotherBrainBody_FakeDeath_Ascent_10_LoadLegTiles);
    E->mbn_var_F = 256;
    MotherBrainBody_FakeDeath_Ascent_10_LoadLegTiles();
  }
}

void MotherBrainBody_FakeDeath_Ascent_10_LoadLegTiles(void) {  // 0xA98DB4
  if (ProcessSpriteTilesTransfers(0xa9, addr_stru_A98F8F)) {
    Get_MotherBrainBody(0)->mbby_var_A = FUNC16(MotherBrainBody_FakeDeath_Ascent_11_ContinuePause);
    MotherBrainBody_FakeDeath_Ascent_11_ContinuePause();
  }
}

void MotherBrainBody_FakeDeath_Ascent_11_ContinuePause(void) {  // 0xA98DC3
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0) {
    E->base.x_pos = 59;
    E->base.y_pos = 279;
    reg_BG2HOFS = -27;
    reg_BG2VOFS = -217;
    E->mbn_var_04 = 7;
    E->mbn_var_A = FUNC16(MotherBrainBody_FakeDeath_Ascent_12_StartMusic);
  }
}

void MotherBrainBody_FakeDeath_Ascent_12_StartMusic(void) {  // 0xA98DEC
  MotherBrain_SetBodyInstrs(addr_kMotherBrain_Ilist_9A02);
  Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
  E1->base.instruction_timer = 1;
  Enemy_MotherBrain *E0 = Get_MotherBrain(0);
  E0->base.properties &= ~kEnemyProps_Invisible;
  E0->base.x_pos = 59;
  E0->base.y_pos = 279;
  reg_BG2HOFS = -27;
  reg_BG2VOFS = -217;
  QueueMusic_Delayed8(5);
  earthquake_type = 2;
  earthquake_timer = 256;
  E1->mbn_var_34 = 80;
  E1->mbn_var_31 = 1;
  E1->mbn_var_32 = 8;
  E1->mbn_var_33 = 6;
  E0->mbn_var_A = FUNC16(MotherBrainBody_FakeDeath_Ascent_13_Raise);
}

void MotherBrainBody_FakeDeath_Ascent_13_Raise(void) {  // 0xA98E4D
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  uint16 v1;
  if ((nmi_frame_counter_word & 3) != 0
      || (MotherBrain_SpawnDustCloudsForAscent(),
          reg_BG2VOFS += 2,
          v1 = E->base.y_pos - 2,
          E->base.y_pos = v1,
          v1 >= 0xBD)) {
  } else {
    enemy_bg2_tilemap_size = 320;
    E->base.y_pos = 188;
    earthquake_timer = 0;
    hdma_object_channels_bitmask[E->mbn_var_09 >> 1] = 0;
    MotherBrain_SetBodyInstrs(addr_kMotherBrain_Ilist_99AA);
    E->mbn_var_A = FUNC16(MotherBrainBody_FakeDeath_Ascent_14_WaitForUncouching);
    MotherBrainBody_FakeDeath_Ascent_14_WaitForUncouching();
  }
}

void MotherBrainBody_FakeDeath_Ascent_14_WaitForUncouching(void) {  // 0xA98E95
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if (!E->mbn_var_02) {
    E->mbn_var_37 = 0;
    E->mbn_var_A = FUNC16(MotherBrainBody_FakeDeath_Ascent_15_TransitionFromGrey);
    E->mbn_var_F = 0;
  }
}

void MotherBrainBody_FakeDeath_Ascent_15_TransitionFromGrey(void) {  // 0xA98EAA
  Enemy_MotherBrain *E0 = Get_MotherBrain(0);
  if ((--E0->mbn_var_F & 0x8000) != 0) {
    E0->mbn_var_F = 4;
    uint16 v3 = E0->mbn_var_37 + 1;
    E0->mbn_var_37 = v3;
    if (MotherBrain_FadeFromGray_FakeDeath(v3 - 1) & 1) {
      Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
      E1->mbn_var_10 = 1;
      E0->mbn_var_00 = 2;
      E1->mbn_var_12 = 1;
      E1->mbn_var_32 = 6;
      E1->mbn_var_33 = 6;
      E1->mbn_var_34 = 1280;
      E0->mbn_var_A = FUNC16(MotherBrainBody_2ndphase_16_ShakeHeadMenacingly);
      E0->mbn_var_F = 23;
    }
  }
}

void MotherBrainBody_2ndphase_16_ShakeHeadMenacingly(void) {  // 0xA98EF5
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0) {
    MotherBrain_SetBrainInstrs(addr_kMotherBrain_Ilist_9B7F);
    E->mbn_var_A = FUNC16(MotherBrainBody_2ndphase_17_BringHeadBackUp);
    Get_MotherBrain(0x40)->mbn_var_34 = 64;
    E->mbn_var_F = 256;
    MotherBrainBody_2ndphase_17_BringHeadBackUp();
  }
}

void MotherBrainBody_2ndphase_17_BringHeadBackUp(void) {  // 0xA98F14
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0) {
    Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
    E1->mbn_var_32 = 2;
    E1->mbn_var_33 = 4;
    E->mbn_var_A = FUNC16(MotherBrainBody_2ndphase_18_FinishStretching);
    E->mbn_var_F = 64;
    MotherBrainBody_2ndphase_18_FinishStretching();
  }
}

void MotherBrainBody_2ndphase_18_FinishStretching(void) {  // 0xA98F33
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0) {
    Get_MotherBrain(0x40)->mbn_var_14 = 1;
    E->mbn_var_A = FUNC16(MotherBrain_Body_Phase2_Thinking);
  }
}

void MotherBrain_SpawnDustCloudsForAscent(void) {  // 0xA98F46
  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  if ((--E->mbn_var_F & 0x8000) != 0)
    E->mbn_var_F = 7;
  eproj_spawn_pt = (Point16U){ g_word_A98F7F[E->mbn_var_F], 212 };
  SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, g_byte_A98F7D[(uint16)(random_number & 0x100) >> 8]);
  QueueSfx2_Max3(0x29);
}

void MotherBrain_SetupNeckForFakeAscent(void) {  // 0xA9903F
  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  E->mbn_var_24 = 2;
  E->mbn_var_27 = 10;
  E->mbn_var_2D = 10;
  E->mbn_var_2A = 20;
  E->mbn_var_30 = 20;
  E->mbn_var_20 = 18432;
  E->mbn_var_21 = 20480;
  E->mbn_var_34 = 256;
}

void MotherBrain_HandleNeckLower(void) {  // 0xA99072
  uint16 mbn_var_32 = Get_MotherBrain(0x40)->mbn_var_32;
  if (mbn_var_32) {
    switch (mbn_var_32) {
    case 2:
      MotherBrain_HandleNeckLower_2_BobDown();
      break;
    case 4:
      MotherBrain_HandleNeckLower_4_BobUp();
      break;
    case 6:
      MotherBrain_HandleNeckLower_6_Lower();
      break;
    case 8:
      MotherBrain_HandleNeckLower_8_Raise();
      break;
    default:
      Unreachable();
      while (1)
        ;
    }
  } else {
    MotherBrain_HandleNeckLower_0();
  }
}

void MotherBrain_HandleNeckLower_0(void) {  // 0xA99084
  ;
}

void MotherBrain_HandleNeckLower_2_BobDown(void) {  // 0xA99085
  int16 v1;

  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  v1 = E->mbn_var_20 - E->mbn_var_34;
  if ((uint16)v1 < 0x2800) {
    E->mbn_var_32 = 4;
    v1 = 10240;
  }
  E->mbn_var_20 = v1;
}

void MotherBrain_HandleNeckLower_4_BobUp(void) {  // 0xA990A2
  int16 v1;

  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  if (sign16(E->base.y_pos - 60)) {
    E->mbn_var_32 = 2;
  } else {
    v1 = E->mbn_var_34 + E->mbn_var_20;
    if ((uint16)v1 >= 0x9000) {
      E->mbn_var_32 = 2;
      v1 = -28672;
    }
    E->mbn_var_20 = v1;
  }
}

void MotherBrain_HandleNeckLower_6_Lower(void) {  // 0xA990CF
  int16 v1;

  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  v1 = E->mbn_var_20 - E->mbn_var_34;
  if ((uint16)v1 < 0x3000) {
    E->mbn_var_32 = 0;
    v1 = 12288;
  }
  E->mbn_var_20 = v1;
}

void MotherBrain_HandleNeckLower_8_Raise(void) {  // 0xA990EC
  int16 v1;

  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  v1 = E->mbn_var_34 + E->mbn_var_20;
  if ((uint16)v1 >= 0x9000) {
    E->mbn_var_32 = 0;
    v1 = -28672;
  }
  E->mbn_var_20 = v1;
}

void MotherBrain_HandleNeckUpper(void) {  // 0xA99109
  int16 v2;
  int16 v6;
  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  uint16 mbn_var_33 = E->mbn_var_33;
  if (mbn_var_33) {
    switch (mbn_var_33) {
    case 2:
      if ((int16)(E->base.y_pos + 4 - samus_y_pos) < 0) {
        v2 = E->mbn_var_21 - E->mbn_var_34;
        if ((uint16)v2 < 0x2000) {
          E->mbn_var_33 = 4;
          v2 = 0x2000;
        }
        E->mbn_var_21 = v2;
      } else {
        E->mbn_var_32 = 4;
        E->mbn_var_33 = 4;
      }
      break;
    case 4: {
      uint16 r18 = E->mbn_var_20 + 2048;
      uint16 v4 = E->mbn_var_34 + E->mbn_var_21;
      if (v4 >= r18) {
        E->mbn_var_33 = 2;
        v4 = r18;
      }
      E->mbn_var_21 = v4;
      break;
    }
    case 6: {
      v6 = E->mbn_var_21 - E->mbn_var_34;
      if ((uint16)v6 < 0x2000) {
        E->mbn_var_33 = 0;
        v6 = 0x2000;
      }
      E->mbn_var_21 = v6;
      break;
    }
    case 8: {
      uint16 r18 = E->mbn_var_20 + 2048;
      uint16 v8 = E->mbn_var_34 + E->mbn_var_21;
      if (v8 >= r18) {
        E->mbn_var_33 = 0;
        v8 = r18;
      }
      E->mbn_var_21 = v8;
      break;
    }
    default:
      Unreachable();
      while (1)
        ;
    }
  }
}

void MotherBrain_HandleNeck(void) {  // 0xA991B8
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->mbn_var_0A = E->base.x_pos - 80;
  E->mbn_var_0B = E->base.y_pos + 46;

  Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
  if (E1->mbn_var_31) {
    MotherBrain_HandleNeckLower();
    MotherBrain_HandleNeckUpper();
  }
  uint16 r18 = HIBYTE(E1->mbn_var_20);
  E1->mbn_var_22 = E->mbn_var_0A + ComputeSinMult(E1->mbn_var_24, r18) + 112;
  E1->mbn_var_23 = E->mbn_var_0B + ComputeCosMult(E1->mbn_var_24, r18) - 96;
  E1->mbn_var_25 = E->mbn_var_0A + ComputeSinMult(E1->mbn_var_27, r18) + 112;
  E1->mbn_var_26 = E->mbn_var_0B + ComputeCosMult(E1->mbn_var_27, r18) - 96;
  E1->mbn_var_28 = E->mbn_var_0A + ComputeSinMult(E1->mbn_var_2A, r18) + 112;
  E1->mbn_var_29 = E->mbn_var_0B + ComputeCosMult(E1->mbn_var_2A, r18) - 96;
  r18 = HIBYTE(E1->mbn_var_21);
  E1->mbn_var_2B = E1->mbn_var_28 + ComputeSinMult(E1->mbn_var_2D, r18);
  E1->mbn_var_2C = E1->mbn_var_29 + ComputeCosMult(E1->mbn_var_2D, r18);
  E1->mbn_var_2E = E1->mbn_var_28 + ComputeSinMult(E1->mbn_var_30, r18);
  E1->mbn_var_2F = E1->mbn_var_29 + ComputeCosMult(E1->mbn_var_30, r18);
}


uint16 CallMotherBrainInstr(uint32 ea, uint16 k) {
  switch (ea) {
  case fnMotherBrain_Instr_Goto: return MotherBrain_Instr_Goto(k);
  case fnMotherBrain_Instr_EnableNeckMovementGoto: return MotherBrain_Instr_EnableNeckMovementGoto(k);
  case fnMotherBrain_Instr_DisableNeckMovement: return MotherBrain_Instr_DisableNeckMovement(k);
  case fnMotherBrain_Instr_QueueSfx2: return MotherBrain_Instr_QueueSfx2(k);
  case fnMotherBrain_Instr_QueueSfx3: return MotherBrain_Instr_QueueSfx3(k);
  case fnMotherBrain_Instr_SpawnDroolEproj: return MotherBrain_Instr_SpawnDroolEproj(k);
  case fnMotherBrain_Instr_SpawnPurpleBreath: return MotherBrain_Instr_SpawnPurpleBreath(k);
  case fnMotherBrain_Instr_SetMainShakeTimer50: return MotherBrain_Instr_SetMainShakeTimer50(k);
  case fnMotherBrain_Instr_GotoEitherOr: return MotherBrain_Instr_GotoEitherOr(k);
  case fnMotherBrain_Instr_MaybeGoto: return MotherBrain_Instr_MaybeGoto(k);
  case fnMotherBrain_Instr_MaybeGoto2: return MotherBrain_Instr_MaybeGoto2(k);
  case fnMotherBrain_Instr_Goto2: return MotherBrain_Instr_Goto2(k);
  case fnMotherBrain_Instr_QueueShitroidAttackSfx: return MotherBrain_Instr_QueueShitroidAttackSfx(k);
  case fnMotherBrain_Instr_SpawnBlueRingEproj: return MotherBrain_Instr_SpawnBlueRingEproj(k);
  case fnMotherBrain_Instr_AimRingsAtShitroid: return MotherBrain_Instr_AimRingsAtShitroid(k);
  case fnMotherBrain_Instr_AimRingsAtSamus: return MotherBrain_Instr_AimRingsAtSamus(k);
  case fnMotherBrain_Instr_IncrShitroidAttackCtr: return MotherBrain_Instr_IncrShitroidAttackCtr(k);
  case fnMotherBrain_Instr_SetShitroidAttackCtr0: return MotherBrain_Instr_SetShitroidAttackCtr0(k);
  case fnMotherBrain_Instr_SpawnBombEproj: return MotherBrain_Instr_SpawnBombEproj(k);
  case fnMotherBrain_Instr_SpawnLaserEproj: return MotherBrain_Instr_SpawnLaserEproj(k);
  case fnMotherBrain_Instr_SpawnRainbowEproj: return MotherBrain_Instr_SpawnRainbowEproj(k);
  case fnMotherBrain_Instr_SetupFxForRainbowBeam: return MotherBrain_Instr_SetupFxForRainbowBeam(k);
  default: return Unreachable();
  }
}

int MotherBrain_Func_1_DoubleRet(void) {
  uint16 v5;

  Enemy_MotherBrain *E = Get_MotherBrain(0);
  uint16 pc = E->mbn_var_21;
  if (!sign16(pc))
    return -1;

  const uint8 *p = RomPtr_A9(pc);
  if (time_is_frozen_flag)
    return GET_WORD(p + 2);

  v5 = GET_WORD(p);
  if (sign16(v5))
    goto LABEL_10;
  if ((int16)(v5 - E->mbn_var_20) >= 0) {
    E->mbn_var_20++;
    return GET_WORD(p + 2);
  }
  for (pc += 4; ; ) {
    p = RomPtr_A9(pc);
    v5 = GET_WORD(p);
    if (!sign16(v5))
      break;
LABEL_10:
    pc = CallMotherBrainInstr(v5 | 0xA90000, pc + 2);
  }
  E->mbn_var_20 = 1;
  E->mbn_var_21 = pc;
  return GET_WORD(p + 2);
}

void MotherBrain_DrawNeck(void) {  // 0xA99303
  if ((Get_MotherBrain(0)->base.properties & kEnemyProps_Invisible) == 0) {
    Enemy_MotherBrain *E = Get_MotherBrain(0x40);
    MotherBrain_DrawNeckSegment(E->mbn_var_2E, E->mbn_var_2F);
    MotherBrain_DrawNeckSegment(E->mbn_var_2B, E->mbn_var_2C);
    MotherBrain_DrawNeckSegment(E->mbn_var_28, E->mbn_var_29);
    MotherBrain_DrawNeckSegment(E->mbn_var_25, E->mbn_var_26);
    MotherBrain_DrawNeckSegment(E->mbn_var_22, E->mbn_var_23);
  }
}

void MotherBrain_DrawBrain(void) {  // 0xA99357
  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  if (E->mbn_var_14 && !E->mbn_var_15 && (random_number & 0x8000) == 0)
    SpawnEprojWithRoomGfx(addr_kEproj_MotherBrainPurpleBreathSmall, random_number);
  int tt = MotherBrain_Func_1_DoubleRet();
  if (tt < 0)
    return;
  uint16 v2 = tt;
  uint16 mbn_var_0D = Get_MotherBrain(0)->mbn_var_0D;
  if (E->base.flash_timer & 1)
    mbn_var_0D = 0;
  uint16 r22 = mbn_var_0D;
  uint16 mbn_var_00 = E->mbn_var_00;
  uint16 flash_timer;
  if (mbn_var_00) {
    flash_timer = mbn_var_00 - 1;
    E->mbn_var_00 = flash_timer;
  } else {
    flash_timer = E->base.flash_timer;
    if (!flash_timer)
      flash_timer = E->base.shake_timer;
  }
  int v6 = (flash_timer & 6) >> 1;
  uint16 r18 = E->base.x_pos + g_word_A993BB[v6];
  if ((int16)(r18 + 32 - layer1_x_pos) >= 0) {
    uint16 r20 = E->base.y_pos + g_word_A993C3[v6];
    MotherBrain_AddSpritemapToOam(v2, r18, r20, r22);
  }
}

void MotherBrain_DrawNeckSegment(uint16 x, uint16 y) {  // 0xA993CB
  int v0 = (Get_MotherBrain(0x40)->base.flash_timer & 6) >> 1;
  MotherBrain_AddSpritemapToOam(addr_kMotherBrain_Sprmap_A694,
      x + g_word_A993BB[v0], y + g_word_A993C3[v0], Get_MotherBrain(0)->mbn_var_0C);
}

void MotherBrain_AddSpritemapToOam(uint16 j, uint16 r18, uint16 r20, uint16 r22) {  // 0xA993EE
  const uint8 *p = RomPtr_A9(j);
  int n = GET_WORD(p);
  p += 2;
  int idx = oam_next_ptr;
  do {
    int16 y = r20 + (int8)p[2] - layer1_y_pos;
    if (y >= 0) {
      OamEnt *oam = gOamEnt(idx);
      uint16 x = r18 + GET_WORD(p) - layer1_x_pos;
      oam->xcoord = x;
      oam->ycoord = y;
      *(uint16 *)&oam->charnum = r22 | GET_WORD(p + 3);
      oam_ext[idx >> 5] |= (((x & 0x100) >> 8) | (*(int16 *)p < 0) * 2) << (2 * ((idx >> 2) & 7));
      idx = (idx + 4) & 0x1FF;
    }
    p += 5;
  } while (--n);
  oam_next_ptr = idx;
}

void MotherBrain_CalculateRainbowBeamHdma(void) {  // 0xA99466
  MotherBrain_CalcHdma();
}

void MotherBrain_MoveBodyDownScrollLeft(uint16 k, uint16 a) {  // 0xA99552
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->base.y_pos += a;
  reg_BG2VOFS -= a;
  reg_BG2HOFS = k + 34 - E->base.x_pos;
}

void MotherBrain_MoveBodyDown(uint16 a) {  // 0xA99579
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->base.y_pos += a;
  reg_BG2VOFS -= a;
  reg_BG2HOFS = 34 - E->base.x_pos;
}

void MotherBrain_FootstepEffect(void) {  // 0xA99599
  earthquake_type = 1;
  earthquake_timer = 4;
  if (Get_MotherBrain(0)->mbn_var_00 == 3)
    printf("Write to rom!\n");
  //    word_80914D = 22;
}

const uint16 *MotherBrain_Instr_MoveBodyUp10Left4(uint16 k, const uint16 *jp) {  // 0xA995B6
  MotherBrain_MoveBodyDownScrollLeft(4, 0xFFF6);
  return jp;
}

const uint16 *MotherBrain_Instr_MoveBodyUp16Left4(uint16 k, const uint16 *jp) {  // 0xA995C0
  MotherBrain_MoveBodyDownScrollLeft(4, 0xFFF0);
  return jp;
}

const uint16 *MotherBrain_Instr_MoveBodyUp12Right2(uint16 k, const uint16 *jp) {  // 0xA995CA
  MotherBrain_MoveBodyDownScrollLeft(0xFFFE, 0xFFF4);
  return jp;
}

const uint16 *MotherBrain_Instr_MoveDown12Left4(uint16 k, const uint16 *jp) {  // 0xA995DE
  MotherBrain_MoveBodyDownScrollLeft(4, 0xC);
  return jp;
}

const uint16 *MotherBrain_Instr_MoveDown16Right2(uint16 k, const uint16 *jp) {  // 0xA995E8
  MotherBrain_MoveBodyDownScrollLeft(0xFFFE, 0x10);
  return jp;
}

const uint16 *MotherBrain_Instr_MoveDown10Right2(uint16 k, const uint16 *jp) {  // 0xA995F2
  MotherBrain_MoveBodyDownScrollLeft(0xFFFE, 0xA);
  return jp;
}

const uint16 *MotherBrain_Instr_MoveUp2Right1(uint16 k, const uint16 *jp) {  // 0xA995FC
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  ++E->base.x_pos;
  MotherBrain_MoveBodyDown(0xFFFE);
  return jp;
}

const uint16 *MotherBrain_Instr_MoveRight2(uint16 k, const uint16 *jp) {  // 0xA9960C
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->base.x_pos += 2;
  MotherBrain_MoveBodyDown(0);
  return jp;
}

const uint16 *MotherBrain_Instr_MoveUp1(uint16 k, const uint16 *jp) {  // 0xA9961C
  MotherBrain_MoveBodyDown(1);
  return jp;
}

const uint16 *MotherBrain_Instr_MoveUp1Right3_Sfx(uint16 k, const uint16 *jp) {  // 0xA99622
  MotherBrain_FootstepEffect();
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->base.x_pos += 3;
  MotherBrain_MoveBodyDown(1);
  return jp;
}

const uint16 *MotherBrain_Instr_Down2Right15(uint16 k, const uint16 *jp) {  // 0xA99638
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->base.x_pos += 15;
  MotherBrain_MoveBodyDown(0xFFFE);
  return jp;
}

const uint16 *MotherBrain_Instr_Down4Right6(uint16 k, const uint16 *jp) {  // 0xA99648
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->base.x_pos += 6;
  MotherBrain_MoveBodyDown(0xFFFC);
  return jp;
}

const uint16 *MotherBrain_Instr_Up4Left2(uint16 k, const uint16 *jp) {  // 0xA99658
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->base.x_pos -= 2;
  MotherBrain_MoveBodyDown(4);
  return jp;
}

const uint16 *MotherBrain_Instr_Up2Left1_Sfx(uint16 k, const uint16 *jp) {  // 0xA99668
  MotherBrain_FootstepEffect();
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  --E->base.x_pos;
  MotherBrain_MoveBodyDown(2);
  return jp;
}

const uint16 *MotherBrain_Instr_Up2Left1_Sfx2(uint16 k, const uint16 *jp) {  // 0xA9967E
  MotherBrain_FootstepEffect();
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  --E->base.x_pos;
  MotherBrain_MoveBodyDown(2);
  return jp;
}

const uint16 *MotherBrain_Instr_MoveLeft2(uint16 k, const uint16 *jp) {  // 0xA99694
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->base.x_pos -= 2;
  MotherBrain_MoveBodyDown(0);
  return jp;
}

const uint16 *MotherBrain_Instr_MoveDown1(uint16 k, const uint16 *jp) {  // 0xA996A4
  MotherBrain_MoveBodyDown(0xFFFF);
  return jp;
}

const uint16 *MotherBrain_Instr_MoveDown1Left3(uint16 k, const uint16 *jp) {  // 0xA996AA
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->base.x_pos -= 3;
  MotherBrain_MoveBodyDown(0xFFFF);
  return jp;
}

const uint16 *MotherBrain_Instr_MoveUp2Left15_Sfx(uint16 k, const uint16 *jp) {  // 0xA996BA
  MotherBrain_FootstepEffect();
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->base.x_pos -= 15;
  MotherBrain_MoveBodyDown(2);
  return jp;
}

const uint16 *MotherBrain_Instr_MoveUp4Left6(uint16 k, const uint16 *jp) {  // 0xA996D0
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->base.x_pos -= 6;
  MotherBrain_MoveBodyDown(4);
  return jp;
}

const uint16 *MotherBrain_Instr_MoveDown4Right2(uint16 k, const uint16 *jp) {  // 0xA996E0
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->base.x_pos += 2;
  MotherBrain_MoveBodyDown(0xFFFC);
  return jp;
}

const uint16 *MotherBrain_Instr_MoveDown2Right1(uint16 k, const uint16 *jp) {  // 0xA996F0
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  ++E->base.x_pos;
  MotherBrain_MoveBodyDown(0xFFFE);
  return jp;
}

const uint16 *MotherBrain_Instr_SetPose_Standing(uint16 k, const uint16 *jp) {  // 0xA99700
  Get_MotherBrain(0)->mbn_var_02 = 0;
  return jp;
}

const uint16 *MotherBrain_Instr_SetPose_Walking(uint16 k, const uint16 *jp) {  // 0xA99708
  Get_MotherBrain(0)->mbn_var_02 = 1;
  return jp;
}

const uint16 *MotherBrain_Instr_SetPose_Crouched(uint16 k, const uint16 *jp) {  // 0xA99710
  Get_MotherBrain(0)->mbn_var_02 = 3;
  return jp;
}

const uint16 *MotherBrain_Instr_SetPose_CrouchedTrans(uint16 k, const uint16 *jp) {  // 0xA99718
  Get_MotherBrain(0)->mbn_var_02 = 2;
  return jp;
}

const uint16 *MotherBrain_Instr_SetPose_DeathBeamMode(uint16 k, const uint16 *jp) {  // 0xA99720
  Get_MotherBrain(0)->mbn_var_02 = 4;
  return jp;
}

const uint16 *MotherBrain_Instr_SetPose_LeaningDown(uint16 k, const uint16 *jp) {  // 0xA99728
  Get_MotherBrain(0)->mbn_var_02 = 6;
  return jp;
}

const uint16 *MotherBrain_Instr_SpawnEprojToOffset(uint16 k, const uint16 *jp) {  // 0xA99AC8
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  eproj_spawn_pt = (Point16U){ E->base.x_pos + jp[0], E->base.y_pos + jp[1] };
  SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, jp[2]);
  return jp + 3;
}

const uint16 *MotherBrain_Instr_SpawnDeathBeamEproj(uint16 k, const uint16 *jp) {  // 0xA99AEF
  QueueSfx2_Max6(0x63);
  SpawnEprojWithGfx(0, 0x40, addr_kEproj_MotherBrainDeathBeamCharging);
  return jp;
}

const uint16 *MotherBrain_Instr_IncrBeamAttackPhase(uint16 k, const uint16 *jp) {  // 0xA99B05
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  ++E->mbn_var_17;
  return jp;
}

uint16 MotherBrain_Instr_Goto(uint16 k) {  // 0xA99B0F
  return *(uint16 *)RomPtr_A9(k);
}

uint16 MotherBrain_Instr_EnableNeckMovementGoto(uint16 k) {  // 0xA99B14
  Get_MotherBrain(0x40)->mbn_var_31 = 1;
  return *(uint16 *)RomPtr_A9(k);
}

uint16 MotherBrain_Instr_DisableNeckMovement(uint16 k) {  // 0xA99B20
  Get_MotherBrain(0x40)->mbn_var_31 = 0;
  return k;
}

uint16 MotherBrain_Instr_QueueSfx2(uint16 k) {  // 0xA99B28
  const uint16 *v2 = (const uint16 *)RomPtr_A9(k);
  QueueSfx2_Max6(*v2);
  return k + 2;
}

uint16 MotherBrain_Instr_QueueSfx3(uint16 k) {  // 0xA99B32
  const uint16 *v2 = (const uint16 *)RomPtr_A9(k);
  QueueSfx3_Max6(*v2);
  return k + 2;
}

uint16 MotherBrain_Instr_SpawnDroolEproj(uint16 k) {  // 0xA99B3C
  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  if (E->mbn_var_12) {
    uint16 v2 = E->mbn_var_13 + 1;
    if (!sign16(E->mbn_var_13 - 5))
      v2 = 0;
    E->mbn_var_13 = v2;
    uint16 v3 = sign16(E->mbn_var_34 - 128) ? addr_kEproj_MotherBrainDrool : addr_kEproj_MotherBrainDyingDrool;
    SpawnEprojWithRoomGfx(v3, E->mbn_var_13);
  }
  return k;
}

uint16 MotherBrain_Instr_SpawnPurpleBreath(uint16 k) {  // 0xA99B6D
  SpawnEprojWithRoomGfx(addr_kEproj_MotherBrainPurpleBreathBig, 0);
  return k;
}

uint16 MotherBrain_Instr_SetMainShakeTimer50(uint16 k) {  // 0xA99B77
  Get_MotherBrain(0x40)->mbn_var_00 = 50;
  return k;
}

uint16 MotherBrain_Instr_GotoEitherOr(uint16 k) {  // 0xA99C65
  uint16 result = addr_stru_A99C5F;
  if ((random_number & 0xFFF) >= 0xFE0)
    return addr_stru_A99C47;
  return result;
}

uint16 MotherBrain_Instr_MaybeGoto(uint16 k) {  // 0xA99CAD
  if (random_number < 0xF000)
    return addr_stru_A99C9F;
  return k;
}

uint16 MotherBrain_Instr_MaybeGoto2(uint16 k) {  // 0xA99D0D
  if ((random_number & 0xFFF) < 0xEC0)
    return MotherBrain_Instr_Goto2(k);
  return k;
}

uint16 MotherBrain_Instr_QueueShitroidAttackSfx(uint16 k) {  // 0xA99DF7
  if (Get_MotherBrain(0)->mbn_var_13 != 11)
    QueueSfx2_Max6(g_word_A99E0F[0]);
  return k;
}

uint16 MotherBrain_Instr_Goto2(uint16 k) {  // 0xA99D21
  return addr_stru_A99CD1;
}

uint16 MotherBrain_Instr_SpawnBlueRingEproj(uint16 k) {  // 0xA99E29
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  SpawnEprojWithRoomGfx(addr_kEproj_MotherBrainBlueRingLasers, E->mbn_var_1A);
  return k;
}

uint16 MotherBrain_Instr_AimRingsAtShitroid(uint16 k) {  // 0xA99E37
  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  Enemy_MotherBrain *E1 = Get_MotherBrain(E->mbn_var_0A);
  MotherBrain_Instr_AimRings(E1->base.x_pos - E->base.x_pos - 10, E1->base.y_pos - E->base.y_pos - 16);
  return k;
}

uint16 MotherBrain_Instr_AimRingsAtSamus(uint16 k) {  // 0xA99E5B
  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  MotherBrain_Instr_AimRings(samus_x_pos - E->base.x_pos - 10, samus_y_pos - E->base.y_pos - 16);
  return k;
}

void MotherBrain_Instr_AimRings(uint16 x, uint16 y) {  // 0xA99E77
  uint16 v0 = (uint8)-(CalculateAngleFromXY(x, y) + 0x80);
  if (!sign8(v0 - 16)) {
    if ((uint8)v0 < 0x48)
      goto LABEL_6;
LABEL_5:
    LOBYTE(v0) = 72;
    goto LABEL_6;
  }
  if (sign8(v0 + 64))
    goto LABEL_5;
  LOBYTE(v0) = 16;
LABEL_6:
  Get_MotherBrain(0)->mbn_var_1A = v0;
}

uint16 MotherBrain_Instr_IncrShitroidAttackCtr(uint16 k) {  // 0xA99EA3
  int16 v3;

  Enemy_MotherBrain *E = Get_MotherBrain(0);
  v3 = E->mbn_var_13 + 1;
  if ((uint16)v3 >= 0xC)
    v3 = 12;
  E->mbn_var_13 = v3;
  return k;
}

uint16 MotherBrain_Instr_SetShitroidAttackCtr0(uint16 k) {  // 0xA99EB5
  Get_MotherBrain(0)->mbn_var_13 = 0;
  return k;
}

uint16 MotherBrain_Instr_SpawnBombEproj(uint16 k) {  // 0xA99EBD
  const uint16 *v2 = (const uint16 *)RomPtr_A9(k);
  SpawnEprojWithRoomGfx(addr_kEproj_MotherBrainBomb, *v2);
  return k + 2;
}

uint16 MotherBrain_Instr_SpawnLaserEproj(uint16 k) {  // 0xA99F46
  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  E->mbn_var_31 = 0;
  eproj_spawn_r22 = 1;
  eproj_spawn_pt = (Point16U){ E->base.x_pos + 16, E->base.y_pos + 4 };
  SpawnEprojWithRoomGfx(addr_stru_86A17B, 1);
  return k;
}

uint16 MotherBrain_Instr_SpawnRainbowEproj(uint16 k) {  // 0xA99F84
  SpawnEprojWithRoomGfx(addr_kEproj_MotherBrainRainbowBeamCharging, 0);
  return k;
}

uint16 MotherBrain_Instr_SetupFxForRainbowBeam(uint16 k) {  // 0xA99F8E
  Get_MotherBrain(0x40)->mbn_var_14 = 0;
  MotherBrain_SetupBrainPalForLaser();
  QueueSfx2_Max6(0x7F);
  return k;
}

void MotherBrain_Phase3_Death_0(void) {  // 0xA9AEE1
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->base.properties |= kEnemyProps_Intangible;
  Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
  E1->base.properties |= 0x400;
  E->mbn_var_04 = 0;
  if (MotherBrain_MakeWalkBackwards(0x28, 6) & 1) {
    E->mbn_var_A = FUNC16(MotherBrain_Phase3_Death_1);
    E->mbn_var_F = 128;
    MotherBrain_Phase3_Death_1();
  }
}

void MotherBrain_Phase3_Death_1(void) {  // 0xA9AF12
  MotherBrain_GenerateSmokyExplosions();
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0)
    E->mbn_var_A = FUNC16(MotherBrain_Phase3_Death_2);
}

void MotherBrain_Phase3_Death_2(void) {  // 0xA9AF21
  MotherBrain_GenerateSmokyExplosions();
  if (MotherBrain_MakeWalkForwards(2, 0x60) & 1) {
    MotherBrain_SetBrainInstrs(addr_kMotherBrain_Ilist_9C39);
    Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
    E1->mbn_var_32 = 6;
    E1->mbn_var_33 = 6;
    E1->mbn_var_34 = 1280;
    Enemy_MotherBrain *E = Get_MotherBrain(0);
    E->mbn_var_A = FUNC16(MotherBrain_Phase3_Death_3);
    E->mbn_var_F = 32;
  }
}

void MotherBrain_Phase3_Death_3(void) {  // 0xA9AF54
  MotherBrain_GenerateSmokyExplosions();
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0) {
    Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
    E1->mbn_var_32 = 0;
    E1->mbn_var_33 = 0;
    E1->mbn_var_12 = 0;
    E1->mbn_var_14 = 0;
    E1->mbn_var_10 = 0;
    E1->mbn_var_11 = 0;
    for (int i = 28; i >= 0; i -= 2)
      palette_buffer[(i >> 1) + 241] = palette_buffer[(i >> 1) + 145];
    MotherBrain_HealthBasedPaletteHandling();
    E->mbn_var_0D = 3584;
    E1->mbn_var_E = 0;
    E1->mbn_var_F = 0;
    E->mbn_var_A = FUNC16(MotherBrain_Phase3_Death_4);
    MotherBrain_Phase3_Death_4();
  }
}

void MotherBrain_Phase3_Death_4(void) {  // 0xA9AF9D
  MotherBrain_GenerateMixedExplosions();
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0) {
    E->mbn_var_37 = 0;
    E->mbn_var_A = FUNC16(MotherBrain_Phase3_Death_5);
    E->mbn_var_F = 0;
    MotherBrain_Phase3_Death_5();
  }
}

void MotherBrain_Phase3_Death_5(void) {  // 0xA9AFB6
  HandleMotherBrainBodyFlickering();
  MotherBrain_GenerateMixedExplosions();
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0) {
    E->mbn_var_F = 16;
    uint16 v2 = E->mbn_var_37 + 1;
    E->mbn_var_37 = v2;
    if (MotherBrain_FadePalToBlack(v2 - 1) & 1) {
      enemy_bg2_tilemap_size = 710;
      for (int i = 710; i >= 0; i -= 2)
        tilemap_stuff[i >> 1] = 824;
      nmi_flag_bg2_enemy_vram_transfer = 1;
      E->base.properties = E->base.properties & 0xDEFF | 0x100;
      E->base.extra_properties = 0;
      E->mbn_var_A = FUNC16(MotherBrain_Phase3_Death_6);
      E->mbn_var_F = 16;
    }
  }
}

void MotherBrain_Phase3_Death_6(void) {  // 0xA9B013
  MotherBrain_GenerateMixedExplosions();
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0)
    E->mbn_var_A = FUNC16(MotherBrain_Phase3_Death_7);
}

void MotherBrain_GenerateSmokyExplosions(void) {  // 0xA9B022
  MotherBrain_GenerateExplosions(0x10, addr_word_A9B10F, 2);
}

void MotherBrain_GenerateMixedExplosions(void) {  // 0xA9B031
  MotherBrain_GenerateExplosions(8, addr_word_A9B109, 4);
}

void MotherBrain_GenerateExplosions(uint16 a, uint16 r22, uint16 r24) {  // 0xA9B03E
  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  bool v2 = (--E->mbn_var_E & 0x8000) != 0;
  if (v2) {
    E->mbn_var_E = a;
    v2 = (--E->mbn_var_F & 0x8000) != 0;
    if (v2)
      E->mbn_var_F = 6;
    uint16 v3 = 16 * E->mbn_var_F;
    uint16 v4 = r24;
    uint16 v9;
    do {
      v9 = v4;
      int v5 = v3 >> 1;
      eproj_spawn_pt = (Point16U){ g_word_A9B099[v5], g_word_A9B099[v5 + 1] };
      const uint16 *v6 = (const uint16 *)RomPtr_A9(r22);
      uint16 v7 = *v6;
      uint16 Random = NextRandom();
      if (Random >= 0x4000) {
        v7 = v6[1];
        if (Random >= 0xE000)
          v7 = v6[2];
      }
      SpawnEprojWithRoomGfx(addr_kEproj_MotherBrainDeathExplosion, v7);
      v3 += 4;
      v4 = v9 - 1;
    } while (v9 != 1);
    QueueSfx3_Max3(0x13);
  }
}

void MotherBrain_Phase3_Death_7(void) {  // 0xA9B115
  MotherBrain_SetBrainInstrs(addr_kMotherBrain_Ilist_9C29);
  Get_MotherBrain(0x40)->mbn_var_A = FUNC16(MotherBrainsBrain_SetupBrainToDraw);
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->mbn_var_F = 0;
  E->mbn_var_A = FUNC16(MotherBrain_Phase3_Death_8);
  MotherBrain_Phase3_Death_8();
}

void MotherBrain_Phase3_Death_8(void) {  // 0xA9B12D
  int16 v3;

  Enemy_MotherBrain *E = Get_MotherBrain(0);
  uint16 v1 = E->mbn_var_F + 32;
  E->mbn_var_F = v1;
  Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
  v3 = E1->base.y_pos + HIBYTE(v1);
  if ((uint16)v3 >= 0xC4) {
    EnableEarthquakeAframes(2);
    E->mbn_var_A = FUNC16(MotherBrain_Phase3_Death_9);
    E->mbn_var_F = 256;
    v3 = 196;
  }
  E1->base.y_pos = v3;
}

void MotherBrain_Phase3_Death_9(void) {  // 0xA9B15E
  if (ProcessSpriteTilesTransfers(0xa9, addr_stru_A99003)) {
    Enemy_MotherBrain *E = Get_MotherBrain(0);
    E->mbn_var_A = FUNC16(MotherBrain_Phase3_Death_10);
    E->mbn_var_F = 32;
  }
}

void MotherBrain_Phase3_Death_10(void) {  // 0xA9B173
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0) {
    E->mbn_var_37 = 0;
    E->mbn_var_A = FUNC16(MotherBrain_Phase3_Death_11);
    E->mbn_var_F = 0;
  }
}

void MotherBrain_Phase3_Death_11(void) {  // 0xA9B189
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0) {
    uint16 v2 = E->mbn_var_37 + 1;
    E->mbn_var_37 = v2;
    if (MotherBrain_FadeToGray_RealDeath(v2 - 1) & 1) {
      MotherBrain_SetBrainInstrs(addr_kMotherBrain_Ilist_9D25);
      E->mbn_var_A = FUNC16(MotherBrain_Phase3_Death_12);
      E->mbn_var_F = 256;
    } else {
      E->mbn_var_F = 16;
    }
  }
}

void MotherBrain_Phase3_Death_12(void) {  // 0xA9B1B8
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0) {
    E->mbn_var_A = FUNC16(MotherBrain_Phase3_Death_13);
    Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
    E1->base.properties |= 0x400;
    E->mbn_var_04 = 0;
  }
}

void MotherBrain_Phase3_Death_13(void) {  // 0xA9B1D5
  if (ProcessCorpseRotting(0x40) & 1) {
    uint16 dms_var_53 = Get_DeadMonsters(0x40)->dms_var_53;
    ProcessCorpseRottingVramTransfers(dms_var_53);
  } else {
    Enemy_DeadMonsters *E1 = Get_DeadMonsters(0x40);
    E1->base.properties = E1->base.properties & 0xDEFF | 0x100;
    E1->base.extra_properties = 0;
    QueueMusic_Delayed8(0);
    QueueMusic_Delayed8(0xFF24);
    E1 = Get_DeadMonsters(0);
    E1->dms_var_A = FUNC16(MotherBrain_Phase3_Death_14_20framedelay);
    E1->dms_var_F = 20;
    MotherBrain_Phase3_Death_14_20framedelay();
  }
}

void MotherBrain_Phase3_Death_14_20framedelay(void) {  // 0xA9B211
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0) {
    Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
    E1->base.x_pos = 0;
    E1->base.y_pos = 0;
    E->mbn_var_A = FUNC16(MotherBrain_Phase3_Death_15_LoadEscapeTimerTiles);
  }
}

void MotherBrain_CorpseRottingFinished(void) {  // 0xA9B223
  Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
  eproj_spawn_pt = (Point16U){ E1->base.x_pos + (random_number & 0x1F) - 16, E1->base.y_pos + 16 };
  SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 0xA);
  if ((random_enemy_counter & 7) == 0)
    QueueSfx2_Max3(0x10);
}

void MotherBrain_Phase3_Death_15_LoadEscapeTimerTiles(void) {  // 0xA9B258
  if (ProcessSpriteTilesTransfers(0xa6, addr_stru_A6C4CB)) {
    Get_MotherBrain(0)->mbn_var_A = FUNC16(MotherBrain_Phase3_Death_16_StartEscape);
    MotherBrain_Phase3_Death_16_StartEscape();
  }
}

void MotherBrain_Phase3_Death_16_StartEscape(void) {  // 0xA9B26D
  if (ProcessSpriteTilesTransfers(0xa9, addr_stru_A9902F)) {
    WriteColorsToPalette(0x122, 0xa9, addr_kMotherBrainPalette_5 + 2, 0xE);
    QueueMusic_Delayed8(7);
    earthquake_type = 5;
    earthquake_timer = -1;
    SpawnPalfxObject(addr_kPalfx_FFC9);
    SpawnPalfxObject(addr_kPalfx_FFCD);
    SpawnPalfxObject(addr_kPalfx_FFD1);
    SpawnPalfxObject(addr_kPalfx_FFD5);
    Get_MotherBrain(0x40)->mbn_var_02 = 0;
    SetupZebesEscapeTypewriter();
    Enemy_MotherBrain *E = Get_MotherBrain(0);
    E->mbn_var_F = 32;
    uint16 v1 = FUNC16(MotherBrain_Phase3_Death_17_SpawnTimeBomb);
    if (!japanese_text_flag)
      v1 = FUNC16(MotherBrain_Phase3_Death_18_TypesZebesText);
    E->mbn_var_A = v1;
  }
}

void MotherBrain_Phase3_Death_17_SpawnTimeBomb(void) {  // 0xA9B2D1
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0) {
    E->mbn_var_A = FUNC16(MotherBrain_Phase3_Death_18_TypesZebesText);
    SpawnEprojWithRoomGfx(addr_kEproj_TimeBombSetJapaneseText, FUNC16(MotherBrain_Phase3_Death_18_TypesZebesText));
  }
  MotherBrain_Phase3_Death_18_TypesZebesText();
}

void MotherBrain_Phase3_Death_18_TypesZebesText(void) {  // 0xA9B2E3
  if (HandleTypewriterText_Ext(0x2610) & 1) {
    Enemy_MotherBrain *E = Get_MotherBrain(0);
    E->mbn_var_A = FUNC16(MotherBrain_Phase3_Death_19_EscapeDoorExploding);
    E->mbn_var_F = 32;
  }
}

void MotherBrain_Phase3_Death_19_EscapeDoorExploding(void) {  // 0xA9B2F9
  MotherBrain_GenerateEscapeDoorExploding();
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0) {
    CallSomeSamusCode(0xF);
    timer_status = 2;
    SetBossBitForCurArea(2);
    SetEventHappened(0xE);
    E->mbn_var_A = FUNC16(MotherBrain_Phase3_Death_20_BlowUpEscapeDoor);
    Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
    E1->mbn_var_E = 0;
    E1->mbn_var_F = 0;
  }
}

void MotherBrain_Phase3_Death_20_BlowUpEscapeDoor(void) {  // 0xA9B32A
  MotherBrain_ExplodeEscapeDoor();
  Get_MotherBrain(0)->mbn_var_A = FUNC16(MotherBrain_Phase3_Death_21_KeepEarthquakeGoing);
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x00, 0x06, 0xb677 });
}

void MotherBrain_Phase3_Death_21_KeepEarthquakeGoing(void) {  // 0xA9B33C
  if (!earthquake_timer)
    --earthquake_timer;
}

void MotherBrain_GenerateEscapeDoorExploding(void) {  // 0xA9B346
  int16 v1;
  int16 v2;

  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  v1 = E->mbn_var_E - 1;
  E->mbn_var_E = v1;
  if (v1 < 0) {
    E->mbn_var_E = 4;
    v2 = E->mbn_var_F - 1;
    E->mbn_var_F = v2;
    if (v2 < 0)
      E->mbn_var_F = 3;
    int v3 = (uint16)(4 * E->mbn_var_F) >> 1;
    eproj_spawn_pt = (Point16U){ g_word_A9B393[v3], g_word_A9B393[v3 + 1] };
    uint16 v4 = 3;
    if (NextRandom() < 0x4000)
      v4 = 12;
    SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, v4);
    QueueSfx2_Max3(0x24);
  }
}

void MotherBrain_ExplodeEscapeDoor(void) {  // 0xA9B3A3
  uint16 v0 = 0, v1;
  do {
    v1 = v0;
    SpawnEprojWithRoomGfx(addr_kEproj_MotherBrainExplodedEscapeDoor, v0);
    v0 = v1 + 1;
  } while ((uint16)(v1 + 1) < 8);
}

void MotherBrain_SamusCollDetect(void) {  // 0xA9B3B6
  int8 v1; // cf

  Enemy_MotherBrain *E0 = Get_MotherBrain(0);
  uint16 R26 = E0->mbn_var_04;
  v1 = R26 & 1;
  R26 >>= 1;
  if (!v1
      || (!(MotherBrain_SamusCollDetectPart(addr_word_A9B427, E0->base.x_pos, E0->base.y_pos) & 1))) {

    Enemy_MotherBrain *E = Get_MotherBrain(0x40);
    v1 = R26 & 1;
    R26 >>= 1;
    if (!v1
        || (!(MotherBrain_SamusCollDetectPart(0xB439, E->base.x_pos, E->base.y_pos) & 1))) {
      v1 = R26 & 1;
      R26 >>= 1;
      if (v1) {
        if (!(MotherBrain_SamusCollDetectPart(0xB44B, E->mbn_var_25, E->mbn_var_26) & 1)) {
          if (!(MotherBrain_SamusCollDetectPart(0xB44B, E->mbn_var_28, E->mbn_var_29) & 1)) {
            MotherBrain_SamusCollDetectPart(0xB44B, E->mbn_var_2B, E->mbn_var_2C);
          }
        }
      }
    }
  }
}

uint8 MotherBrain_SamusCollDetectPart(uint16 k, uint16 r18, uint16 r20) {  // 0xA9B455
  int16 v5;
  const uint8 *p = RomPtr_A9(k);
  int n = GET_WORD(p);
  for (p += 2; n; p += 8, n--) {
    uint16 v3, v4, R24;
    if ((int16)(samus_y_pos - r20) >= 0) {
      R24 = samus_y_pos - r20;
      v3 = GET_WORD(p + 6);
    } else {
      R24 = r20 - samus_y_pos;
      v3 = GET_WORD(p + 2);
    }
    if ((int16)(samus_y_radius + abs16(v3) - R24) >= 0) {
      if ((int16)(samus_x_pos - r18) >= 0) {
        R24 = samus_x_pos - r18;
        v4 = GET_WORD(p + 4);
      } else {
        R24 = r18 - samus_x_pos;
        v4 = GET_WORD(p);
      }
      v5 = samus_x_radius + abs16(v4) - R24;
      if (v5 >= 0) {
        if (sign16(v5 - 4))
          v5 = 4;
        extra_samus_x_displacement = v5;
        extra_samus_y_displacement = 4;
        extra_samus_x_subdisplacement = 0;
        extra_samus_y_subdisplacement = 0;
        samus_invincibility_timer = 96;
        samus_knockback_timer = 5;
        knockback_x_dir = 1;
        if (sign16(samus_y_pos - 192))
          samus_y_dir = 2;
        if ((int16)(Get_MotherBrain(0)->base.x_pos + 24 - samus_x_pos) < 0)
          MotherBrain_HurtSamus();
        return 1;
      }
    }
  }
  return 0;
}

void MotherBrainsBody_Shot(void) {  // 0xA9B503
  CreateDudShot();
}

void MotherBrainsBrain_Shot(void) {  // 0xA9B507
  int16 v2;

  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if (E->mbn_var_00) {
    MotherBrain_Phase23_ShotReaction();
    if (E->mbn_var_00 == 1)
      CreateDudShot();
    else
      NormalEnemyShotAiSkipDeathAnim_CurEnemy();
  } else {
    uint16 v1 = HIBYTE(projectile_type[collision_detection_index]) & 7;
    if (g_byte_A9B546[v1]) {
      plm_room_arguments[39] += g_byte_A9B546[v1];
      QueueSfx2_Max6(0x6E);
      v2 = 13;
      Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
      uint16 flash_timer = E1->base.flash_timer;
      if (flash_timer) {
        if (flash_timer & 1)
          v2 = 14;
      }
      E1->base.flash_timer = v2;
      NormalEnemyShotAiSkipDeathAnim_CurEnemy();
    }
  }
}

void MotherBrain_Phase23_ShotReaction(void) {  // 0xA9B562
  int16 v1;

  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if (E->mbn_var_00 == 4 && MotherBrain_DetermineShotReactionType() == 2) {
    MotherBrain_Phase3_BeamShotReaction();
  } else {
    if (MotherBrain_DetermineShotReactionType() == 1 || (v1 = E->mbn_var_07 - 256, v1 < 0))
      v1 = 0;
    E->mbn_var_07 = v1;
  }
}

uint16 MotherBrain_DetermineShotReactionType(void) {  // 0xA9B58E
  return g_byte_A9B5A1[HIBYTE(projectile_type[collision_detection_index]) & 7];
}

void MotherBrain_Phase3_BeamShotReaction(void) {  // 0xA9B5A9
  int16 v1;

  Enemy_MotherBrain *E = Get_MotherBrain(0);
  v1 = E->mbn_var_07 - 266;
  if (v1 < 0) {
    Get_MotherBrain(0x40)->mbn_var_18 = FUNC16(MotherBrain_Phase3_Neck_SetupHyperBeamRecoil);
    v1 = 0;
    E->mbn_var_F = 0;
  }
  E->mbn_var_07 = v1;
}

void MotherBrainsBrain_Touch(void) {  // 0xA9B5C6
  int16 v0;

  if (samus_movement_type == 3) {
    v0 = 13;
    EnemyData *v1 = gEnemyData(0);
    uint16 flash_timer = v1[1].flash_timer;
    if (flash_timer) {
      if (flash_timer & 1)
        v0 = 14;
    }
    v1[1].flash_timer = v0;
  }
}

void MotherBrain_HurtSamus(void) {  // 0xA9B5E1
  Ridley_Func_98();
  samus_invincibility_timer = 96;
  samus_knockback_timer = 5;
  knockback_x_dir = (int16)(samus_x_pos - Get_MotherBrain(cur_enemy_index)->base.x_pos) >= 0;
}

void MotherBrain_Body_Phase2_Thinking(void) {  // 0xA9B605
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
  if (!E1->base.health) {
    E->mbn_var_A = FUNC16(MotherBomb_FiringRainbowBeam_0);
    MotherBomb_FiringRainbowBeam_0();
    return;
  }
  if (!E->mbn_var_02) {
    if (!sign16(E1->base.health - 4500)) {
      if (random_number >= 0x1000) {
LABEL_6:
        MotherBrain_HandleWalking();
        return;
      }
LABEL_7:
      E->mbn_var_A = FUNC16(MotherBrain_Body_Phase2_TryAttack);
      return;
    }
    if (random_number < 0x2000)
      goto LABEL_6;
    if (random_number >= 0xA000)
      goto LABEL_7;
    E->mbn_var_A = FUNC16(MotherBomb_FiringDeathBeam);
  }
}

void MotherBrain_Body_Phase2_TryAttack(void) {  // 0xA9B64B
  int16 v0;
  Enemy_MotherBrain *E = Get_MotherBrain(0);

  v0 = 2 * E->mbn_var_18;
  if (v0) {
    if (v0 == 2) {
      MotherBrain_Phase2_Attack_Cooldown();
    } else {
      if (v0 != 4) {
        Unreachable();
        while (1)
          ;
      }
      MotherBrain_Phase2_Attack_End();
    }
  } else {
    E->mbn_parameter_1 = 64;
    ++E->mbn_var_18;
    if (MotherBrain_Phase2_DecideAttackStrategy_DoubleRet())
      return;
    uint16 v2 = addr_byte_A9B6DC;
    Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
    if (abs16(E1->base.y_pos + 4 - samus_y_pos) < 0x20)
      v2 = addr_byte_A9B6DF;
    uint16 v4 = 0;
    const uint8 *v5 = RomPtr_A9(v2);
    if ((uint8)random_number >= *v5) {
      v4 = 2;
      if ((uint8)random_number >= v5[1]) {
        v4 = 4;
        if ((uint8)random_number >= v5[2])
          v4 = 6;
      }
    }
    uint16 v6 = g_off_A9B6D4[v4 >> 1];
    if (v6 == addr_kMotherBrain_Ilist_9ECC) {
      if (sign16(E1->mbn_var_05 - 1)) {
        E->mbn_var_A = FUNC16(MotherBrain_FiringBomb_DecideOnWalking);
        MotherBrain_FiringBomb_DecideOnWalking();
      }
    } else if (v6 == addr_kMotherBrain_Ilist_9F34) {
      E->mbn_var_A = FUNC16(MotherBomb_FiringLaser_PositionHead);
      MotherBomb_FiringLaser_PositionHead();
    } else {
      MotherBrain_SetBrainInstrs(v6);
    }
  }
}

uint8 MotherBrain_Phase2_DecideAttackStrategy_DoubleRet(void) {  // 0xA9B6E2
  if (g_word_A9B72C[samus_movement_type]) {
    if (sign16((uint8)random_number - 128)) {
      return 0;
    } else {
      Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
      if (sign16(E1->mbn_var_05 - 1)) {
        Get_MotherBrain(0)->mbn_var_A = FUNC16(MotherBrain_FiringBomb_DecideOnWalking);
        MotherBrain_FiringBomb_DecideOnWalking();
        return 1;
      }
      return 0;
    }
  } else {
    if (sign16((uint8)random_number - 128)) {
      Get_MotherBrain(0)->mbn_var_A = FUNC16(MotherBomb_FiringLaser_PositionHead);
      MotherBomb_FiringLaser_PositionHead();
    } else {
      MotherBrain_SetBrainInstrs(addr_kMotherBrain_Ilist_9D3D);
    }
    return 1;
  }
}

void MotherBrain_Phase2_Attack_Cooldown(void) {  // 0xA9B764
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if (E->mbn_parameter_1-- == 1)
    ++E->mbn_var_18;
}

void MotherBrain_Phase2_Attack_End(void) {  // 0xA9B773
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->mbn_var_18 = 0;
  E->mbn_var_A = FUNC16(MotherBrain_Body_Phase2_Thinking);
}

void MotherBrain_FiringBomb_DecideOnWalking(void) {  // 0xA9B781
  if (random_number >= 0xFF80) {
    MotherBrain_FiringBomb_DecideOnCrouching();
    return;
  }
  uint16 v0 = 64;
  if (random_number < 0x6000)
    v0 = 96;
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((int16)(v0 - E->base.x_pos) >= 0
      || (E->mbn_var_F = v0, MotherBrain_MakeWalkBackwards(v0, 6) & 1)) {
    MotherBrain_FiringBomb_DecideOnCrouching();
  } else {
    E->mbn_var_A = FUNC16(MotherBrain_FiringBomb_WalkingBackwards);
  }
}

void MotherBrain_FiringBomb_WalkingBackwards(void) {  // 0xA9B7AC
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if (MotherBrain_MakeWalkBackwards(E->mbn_var_F, 6) & 1)
    MotherBrain_FiringBomb_DecideOnCrouching();
}

void MotherBrain_FiringBomb_DecideOnCrouching(void) {  // 0xA9B7B7
  if (NextRandom() >= 0x8000) {
    Get_MotherBrain(0)->mbn_var_A = FUNC16(MotherBrain_FiringBomb_Crouch);
    MotherBrain_FiringBomb_Crouch();
  } else {
    MotherBrain_B7CB();
  }
}

void MotherBrain_FiringBomb_Crouch(void) {  // 0xA9B7C6
  if (MotherBrain_MakeHerCrouch() & 1)
    MotherBrain_B7CB();
}

void MotherBrain_B7CB(void) {  // 0xA9B7CB
  uint16 v0 = addr_kMotherBrain_Ilist_9ECC;
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if (E->mbn_var_1F)
    v0 = addr_kMotherBrain_Ilist_9F00;
  MotherBrain_SetBrainInstrs(v0);
  E->mbn_var_A = FUNC16(MotherBrain_FiringBomb_Fired);
  E->mbn_var_F = 44;
}

void MotherBrain_FiringBomb_Fired(void) {  // 0xA9B7E8
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0) {
    if (MotherBrain_MakeHerStandUp() & 1) {
      MotherBrain_FiringBomb_Finish();
    } else {
      E->mbn_var_A = FUNC16(MotherBrain_FiringBomb_Standup);
      MotherBrain_FiringBomb_Standup();
    }
  }
}

void MotherBrain_FiringBomb_Standup(void) {  // 0xA9B7F8
  if (MotherBrain_MakeHerStandUp() & 1)
    MotherBrain_FiringBomb_Finish();
}

void MotherBrain_FiringBomb_Finish(void) {  // 0xA9B7FD
  uint16 v0 = FUNC16(MotherBrain_Body_Phase2_Thinking);
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if (E->mbn_var_1F)
    v0 = FUNC16(MotherBrain_Phase2_MurderShitroid_1);
  E->mbn_var_A = v0;
}

void MotherBomb_FiringLaser_PositionHead(void) {  // 0xA9B80E
  int16 v0;

  v0 = 8;
  Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
  if ((int16)(E1->base.y_pos - samus_y_pos) < 0)
    v0 = 6;
  E1->mbn_var_32 = v0;
  E1->mbn_var_33 = v0;
  E1->mbn_var_34 = 512;
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->mbn_var_A = FUNC16(MotherBomb_FiringLaser_PositionHeadSlowlyFire);
  E->mbn_var_F = 4;
}

void MotherBomb_FiringLaser_PositionHeadSlowlyFire(void) {  // 0xA9B839
  int16 v2;

  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0) {
    v2 = 256;
    Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
    if ((E1->mbn_var_34 & 0x8000) != 0)
      v2 = -256;
    E1->mbn_var_34 = v2;
    MotherBrain_SetBrainInstrs(addr_kMotherBrain_Ilist_9F34);
    E->mbn_var_A = FUNC16(MotherBomb_FiringLaser_FinishAttack);
    E->mbn_var_F = 16;
  }
}

void MotherBomb_FiringLaser_FinishAttack(void) {  // 0xA9B863
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0) {
    Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
    E1->mbn_var_32 = 4;
    E1->mbn_var_33 = 4;
    E->mbn_var_A = FUNC16(MotherBrain_Body_Phase2_Thinking);
    MotherBrain_Body_Phase2_Thinking();
  }
}

static Func_V *const off_A9B887[4] = {  // 0xA9B87D
  MotherBomb_FiringDeathBeam_0,
  MotherBomb_FiringDeathBeam_1,
  MotherBomb_FiringDeathBeam_2,
  MotherBomb_FiringDeathBeam_3,
};
void MotherBomb_FiringDeathBeam(void) {
  int v0 = Get_MotherBrain(0)->mbn_var_17;
  off_A9B887[v0]();
}

void MotherBomb_FiringDeathBeam_0(void) {  // 0xA9B88F
  if (MotherBrain_MakeWalkBackwards(0x28, 8) & 1) {
    Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
    E1->mbn_var_32 = 8;
    E1->mbn_var_33 = 6;
    Enemy_MotherBrain *E = Get_MotherBrain(0);
    ++E->mbn_var_17;
  }
}

void MotherBomb_FiringDeathBeam_1(void) {  // 0xA9B8B2
  if (!Get_MotherBrain(0x40)->mbn_var_05) {
    MotherBrain_SetBodyInstrs(addr_kMotherBrain_Ilist_9A42);
    Enemy_MotherBrain *E = Get_MotherBrain(0);
    ++E->mbn_var_17;
  }
}

void MotherBomb_FiringDeathBeam_2(void) {  // 0xA9B8C8
  ;
}

void MotherBomb_FiringDeathBeam_3(void) {  // 0xA9B8C9
  MotherBrain_SetBrainInstrs(addr_kMotherBrain_Ilist_9C87);
  Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
  E1->mbn_var_32 = 2;
  E1->mbn_var_33 = 4;
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->mbn_var_17 = 0;
  E->mbn_var_A = FUNC16(MotherBrain_Body_Phase2_Thinking);
}

void MotherBomb_FiringRainbowBeam_0(void) {  // 0xA9B8EB
  MotherBrain_SetBrainInstrs(addr_kMotherBrain_Ilist_9C87);
  Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
  E1->mbn_var_34 = 64;
  E1->mbn_var_31 = 1;
  E1->mbn_var_32 = 2;
  E1->mbn_var_33 = 4;
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->mbn_var_A = FUNC16(MotherBomb_FiringRainbowBeam_1_StartCharge);
  E->mbn_var_F = 256;

}

void MotherBomb_FiringRainbowBeam_1_StartCharge(void) {  // 0xA9B91A
  Enemy_MotherBomb *E = Get_MotherBomb(0);
  if ((--E->mbb_var_F & 0x8000) != 0) {
    MotherBrain_SetBrainInstrs(addr_kMotherBrain_Ilist_9F6C);
    E->mbb_var_A = FUNC16(MotherBomb_FiringRainbowBeam_2_RetractNeck);
    MotherBomb_FiringRainbowBeam_2_RetractNeck();
  }
}

void MotherBomb_FiringRainbowBeam_2_RetractNeck(void) {  // 0xA9B92B
  if (MotherBrain_WalkBackwardsSlowlyAndRetractHead(0x28) & 1) {
    Enemy_MotherBomb *E = Get_MotherBomb(0);
    E->mbb_var_A = FUNC16(MotherBomb_FiringRainbowBeam_3_Wait);
    E->mbb_var_F = 256;
    MotherBomb_FiringRainbowBeam_3_Wait();
  }
}

void MotherBomb_FiringRainbowBeam_3_Wait(void) {  // 0xA9B93F
  Enemy_MotherBomb *E = Get_MotherBomb(0);
  if ((--E->mbb_var_F & 0x8000) != 0) {
    QueueSfx2_Max6(0x71);
    E->mbb_var_A = FUNC16(MotherBomb_FiringRainbowBeam_4_ExtendNeckDown);
    MotherBomb_FiringRainbowBeam_4_ExtendNeckDown();
  }
}

void MotherBomb_FiringRainbowBeam_4_ExtendNeckDown(void) {  // 0xA9B951
  cooldown_timer = 8;
  Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
  E1->mbn_var_32 = 6;
  E1->mbn_var_33 = 6;
  E1->mbn_var_34 = 1280;
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->mbn_var_A = FUNC16(MotherBomb_FiringRainbowBeam_5_StartFiring);
  E->mbn_var_F = 16;
  MotherBomb_FiringRainbowBeam_5_StartFiring();
}

void MotherBomb_FiringRainbowBeam_5_StartFiring(void) {  // 0xA9B975
  MotherBrain_AimBeamAndIncrWidth();
  if (!power_bomb_flag) {
    Enemy_MotherBrain *E = Get_MotherBrain(0);
    if ((--E->mbn_var_F & 0x8000) != 0 && !power_bomb_flag) {
      cooldown_timer = 0;
      MotherBrain_BodyRainbowBeamPalAnimIndex0();
      MotherBrain_SetBrainInstrs(addr_kMotherBrain_Ilist_9C77);
      E->mbn_var_33 = 512;
      E->mbn_var_09 = SpawnMotherBrainRainbowBeamHdma();
      Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
      E1->mbn_var_34 = 64;
      E1->mbn_var_31 = 1;
      E1->mbn_var_32 = 2;
      E1->mbn_var_33 = 4;
      E->mbn_parameter_1 = 0;
      E->mbn_parameter_2 = 0;
      uint16 v4 = 5;
      if ((int16)(samus_health - 700) < 0)
        v4 = 24;
      CallSomeSamusCode(v4);
      E->mbn_var_15 = 6;
      E->mbn_var_A = FUNC16(MotherBomb_FiringRainbowBeam_6_MoveSamusToWall);
    }
  }
}

void MotherBomb_FiringRainbowBeam_6_MoveSamusToWall(void) {  // 0xA9B9E5
  MotherBrain_PlayRainbowBeamSfx();
  MotherBrain_HandleRainbowBeamPalette();
  MotherBrain_AimBeamAndIncrWidth();
  MotherBrain_HandleRainbowBeamExplosions();
  if (MotherBrain_MoveSamusTowardsWallDueToBeam() & 1) {
    Enemy_MotherBomb *E = Get_MotherBomb(0);
    E->mbb_var_A = FUNC16(MotherBomb_FiringRainbowBeam_7_DelayFrame);
    E->mbb_var_F = 0;
  }
}

void MotherBomb_FiringRainbowBeam_7_DelayFrame(void) {  // 0xA9BA00
  MotherBrain_PlayRainbowBeamSfx();
  MotherBrain_HandleRainbowBeamPalette();
  MotherBrain_AimBeamAndIncrWidth();
  MotherBrain_HandleRainbowBeamExplosions();
  MotherBrain_MoveSamusTowardsWallDueToBeam();
  Enemy_MotherBomb *E = Get_MotherBomb(0);
  if ((--E->mbb_var_F & 0x8000) != 0) {
    earthquake_type = 8;
    earthquake_timer = 8;
    E->mbb_var_A = FUNC16(MotherBomb_FiringRainbowBeam_8_StartDrainSamus);
  }
}

void MotherBomb_FiringRainbowBeam_8_StartDrainSamus(void) {  // 0xA9BA27
  Enemy_MotherBomb *E = Get_MotherBomb(0);
  E->mbb_var_A = FUNC16(MotherBomb_FiringRainbowBeam_9_DrainingSamus);
  E->mbb_var_F = 299;
  earthquake_timer = 299;
  earthquake_type = 8;
  MotherBomb_FiringRainbowBeam_9_DrainingSamus();
}

void MotherBomb_FiringRainbowBeam_9_DrainingSamus(void) {  // 0xA9BA3C
  MotherBrain_PlayRainbowBeamSfx();
  MotherBrain_HandleRainbowBeamPalette();
  MotherBrain_AimBeamAndIncrWidth();
  MotherBrain_HandleRainbowBeamExplosions();
  Samus_DamageDueToRainbowBeam();
  Samus_DecrementAmmoDueToRainbowBeam();
  MotherBrain_MoveSamusTowardsMiddleOfWall();
  Enemy_MotherBomb *E = Get_MotherBomb(0);
  if ((--E->mbb_var_F & 0x8000) != 0)
    E->mbb_var_A = FUNC16(MotherBomb_FiringRainbowBeam_10_FinishFiringRainbow);
}

void MotherBomb_FiringRainbowBeam_10_FinishFiringRainbow(void) {  // 0xA9BA5E
  MotherBrain_PlayRainbowBeamSfx();
  MotherBrain_HandleRainbowBeamPalette();
  MotherBrain_AimBeam();
  MotherBrain_HandleRainbowBeamExplosions();
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  uint16 v1 = E->mbn_var_33 - 384;
  E->mbn_var_33 = v1;
  if (sign16(v1 - 512)) {
    E->mbn_var_33 = 512;
    E->mbn_parameter_1 = -256;
    E->mbn_parameter_2 = 0;
    hdma_object_channels_bitmask[E->mbn_var_09 >> 1] = 0;
    earthquake_timer = 0;
    MotherBrain_SetBrainInstrs(addr_kMotherBrain_Ilist_9C87);
    MotherBrain_SetupBrainNormalPal();
    MotherBrain_WriteDefaultPalette();
    QueueSfx1_Max6(2);
    E->mbn_var_16 = 0;
    CallSomeSamusCode(1);
    cooldown_timer = 8;
    E->mbn_var_A = FUNC16(MotherBomb_FiringRainbowBeam_11_LetSamusFall);
  }
}

void MotherBomb_FiringRainbowBeam_11_LetSamusFall(void) {  // 0xA9BAC4
  SomeMotherBrainScripts(0);
  Get_MotherBomb(0)->mbb_var_A = FUNC16(MotherBomb_FiringRainbowBeam_12_WaitForSamusHitGround);
  MotherBomb_FiringRainbowBeam_12_WaitForSamusHitGround();
}

void MotherBomb_FiringRainbowBeam_12_WaitForSamusHitGround(void) {  // 0xA9BAD1
  if (MotherBrain_MoveSamusForFallingAfterBeam() & 1)
    Get_MotherBomb(0)->mbb_var_A = FUNC16(MotherBomb_FiringRainbowBeam_13_LowerHead);
}

void MotherBomb_FiringRainbowBeam_13_LowerHead(void) {  // 0xA9BADD
  Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
  E1->mbn_var_34 = 64;
  E1->mbn_var_31 = 1;
  E1->mbn_var_32 = 2;
  E1->mbn_var_33 = 4;
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->mbn_var_A = FUNC16(MotherBomb_FiringRainbowBeam_14_DecideNextAction);
  E->mbn_var_F = 128;
}

void MotherBomb_FiringRainbowBeam_14_DecideNextAction(void) {  // 0xA9BB06
  Enemy_MotherBomb *E = Get_MotherBomb(0);
  if ((--E->mbb_var_F & 0x8000) != 0) {
    if (sign16(samus_health - 400)) {
      MotherBrain_MakeWalkForwards(0xA, E->base.x_pos + 16);
      E->mbb_var_A = FUNC16(MotherBrain_Phase2Cut_0);
    } else {
      E->mbb_var_A = FUNC16(MotherBomb_FiringRainbowBeam_0);
    }
  }
}

void MotherBrain_PlayRainbowBeamSfx(void) {  // 0xA9BB2E
  int16 mbn_var_15;

  Enemy_MotherBrain *E = Get_MotherBrain(0);
  mbn_var_15 = E->mbn_var_15;
  if (mbn_var_15 >= 0) {
    E->mbn_var_15 = mbn_var_15 - 1;
    QueueSfx1_Max6(0x40);
    E->mbn_var_16 = 1;
  }
}

uint8 MotherBrain_WalkBackwardsSlowlyAndRetractHead(uint16 a) {  // 0xA9BB48
  uint8 rv = MotherBrain_MakeWalkBackwards(a, 0xA);
  if (rv)
    MotherBrain_RetractHead();
  return rv;
}

void MotherBrain_RetractHead(void) {  // 0xA9BB51
  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  E->mbn_var_34 = 80;
  E->mbn_var_31 = 1;
  E->mbn_var_32 = 8;
  E->mbn_var_33 = 6;
}

void MotherBrain_AimBeamAndIncrWidth(void) {  // 0xA9BB6E
  int16 v1;

  Enemy_MotherBrain *E = Get_MotherBrain(0);
  v1 = E->mbn_var_33 + 384;
  if (!sign16(E->mbn_var_33 - 2688))
    v1 = 3072;
  E->mbn_var_33 = v1;
  MotherBrain_AimBeam();
}

void MotherBrain_AimBeam(void) {  // 0xA9BB82
  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  Get_MotherBrain(0)->mbn_var_31 =
      (uint8)-(CalculateAngleFromXY(samus_x_pos - E->base.x_pos - 16, samus_y_pos - E->base.y_pos - 4) + 0x80);
}

void MotherBrain_CalculateRainbowBeamHdma_(void) {  // 0xA9BBB0
  MotherBrain_CalculateRainbowBeamHdma();
}

uint8 MotherBrain_MoveSamusTowardsWallDueToBeam(void) {  // 0xA9BBB5
  uint8 v0 = MotherBrain_MoveSamusHorizTowardsWall(0x1000);
  if (!v0) {
    uint16 v1 = Math_MultByCos(0x1000, Get_MotherBrain(0)->mbn_var_31);
    MotherBrain_MoveSamusVerticallyTowardsCeilingFloor(v1);
  }
  return v0;
}

void MotherBrain_MoveSamusTowardsMiddleOfWall(void) {  // 0xA9BBCF
  uint16 v0 = 64;
  if ((int16)(124 - samus_y_pos) < 0)
    v0 = -64;
  MotherBrain_MoveSamusVerticallyTowardsCeilingFloor(v0);
}

uint8 MotherBrain_MoveSamusForFallingAfterBeam(void) {  // 0xA9BBE1
  int16 v1;

  Enemy_MotherBrain *E = Get_MotherBrain(0);
  v1 = E->mbn_parameter_1 + 2;
  if (v1 >= 0)
    v1 = 0;
  E->mbn_parameter_1 = v1;
  MotherBrain_MoveSamusHorizTowardsWall(v1);
  uint16 v2 = E->mbn_parameter_2 + 24;
  E->mbn_parameter_2 = v2;
  return MotherBrain_MoveSamusVerticallyTowardsCeilingFloor(v2) & 1;
}

uint8 MotherBrain_MoveSamusVerticallyTowardsCeilingFloor(uint16 a) {  // 0xA9BBFD
  int carry = HIBYTE(samus_y_subpos) + LOBYTE(a);
  HIBYTE(samus_y_subpos) = carry;
  HIBYTE(samus_prev_y_subpos) = carry;
  uint16 v4 = samus_y_pos + (int8)(a >> 8) + (carry >> 8), v6;
  if (sign16(v4 - 48)) {
    v6 = 48;
  } else {
    if (sign16(v4 - 192)) {
      samus_prev_y_pos = samus_y_pos = v4;
      return 0;
    }
    v6 = 192;
  }
  samus_prev_y_pos = samus_y_pos = v6;
  samus_y_subpos = 0;
  samus_prev_y_subpos = 0;
  return 1;
}

uint8 MotherBrain_MoveSamusHorizTowardsWall(uint16 a) {  // 0xA9BC3F
  int carry = HIBYTE(samus_x_subpos) + LOBYTE(a);
  HIBYTE(samus_x_subpos) = carry;
  HIBYTE(samus_prev_x_subpos) = carry;
  uint16 v4 = samus_x_pos + (int8)(a >> 8) + (carry >> 8);
  if (sign16(v4 - 235)) {
    samus_x_pos = v4;
    samus_prev_x_pos = v4;
    return 0;
  } else {
    samus_x_pos = 235;
    samus_prev_x_pos = 235;
    samus_x_subpos = 0;
    samus_prev_x_subpos = 0;
    return 1;
  }
}

void MotherBrain_HandleRainbowBeamExplosions(void) {  // 0xA9BC76
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_parameter_2 & 0x8000) != 0) {
    E->mbn_parameter_2 = 8;
    ++E->mbn_parameter_1;
    int v2 = E->mbn_parameter_1 & 7;
    eproj_spawn_pt = (Point16U){ g_word_A9BCA6[v2], g_word_A9BCB6[v2] };
    SpawnEprojWithRoomGfx(addr_kEproj_MotherBrainRainbowBeamExplosion, 0);
    QueueSfx2_Max6(0x24);
  }
}

void MotherBrain_BodyRainbowBeamPalAnimIndex0(void) {  // 0xA9BCC6
  Get_MotherBrain(0x40)->mbn_var_01 = 0;
}

void MotherBrain_WriteDefaultPalette(void) {  // 0xA9BCCE
  WriteColorsToPalette(0x82, 0xa9, addr_kMotherBrainsBrain_Palette + 2, 0xF);
  WriteColorsToPalette(0x122, 0xa9, addr_kMotherBrainsBrain_Palette + 2, 0xF);
  WriteColorsToPalette(0x162, 0xa9, addr_kMotherBrainPalette_0 + 2, 0xF);
}

void MotherBrain_WritePhase2DeathPalette(void) {  // 0xA9BCF6
  MotherBrain_WritePalette(MotherBrain_RainbowBeamPalettes[6]);
}

void MotherBrain_HandleRainbowBeamPalette(void) {  // 0xA9BCFD
  if ((Get_MotherBrain(0)->base.frame_counter & 2) != 0) {
    uint16 mbn_var_01 = Get_MotherBrain(0x40)->mbn_var_01;
    uint16 v1;
    do {
      v1 = mbn_var_01;
      mbn_var_01 = MotherBrain_RainbowBeamPalettes[mbn_var_01 >> 1];
    } while (!mbn_var_01);
    uint16 v2 = v1 + 2;
    Get_MotherBrain(0x40)->mbn_var_01 = v2;
    MotherBrain_WritePalette(*(uint16 *)((uint8 *)MotherBrain_RainbowBeamPalettes + v2 - 2));
  }
}

void MotherBrain_WritePalette(uint16 j) {  // 0xA9BD1D
  WriteColorsToPalette(0x82, 0xad, j, 15);
  WriteColorsToPalette(0x122, 0xad, j, 15);
  WriteColorsToPalette(0x162, 0xad, j + 30, 15);
}

void MotherBrain_Phase2Cut_0(void) {  // 0xA9BD45
  uint16 v0 = SuitDamageDivision(0x50);
  if ((int16)(4 * v0 + 20 - samus_health) >= 0) {
    Get_MotherBrain(0)->mbn_var_A = FUNC16(MotherBrain_Phase2Cut_1);
  } else if ((random_number & 0xFFF) < 0xFA0) {
    if ((Get_MotherBrain(0)->base.frame_counter & 0x1F) == 0)
      MotherBrain_MaybeStandupOrLeanDown();
  } else {
    uint16 v1;
    if ((int16)(SuitDamageDivision(0xA0) + 20 - samus_health) >= 0
        || (v1 = addr_kMotherBrain_Ilist_9ECC, (random_number & 0xFFF) < 0xFF0)) {
      v1 = addr_kMotherBrain_Ilist_9D7F;
    }
    MotherBrain_SetBrainInstrs(v1);
  }
}

void MotherBrain_Phase2Cut_1(void) {  // 0xA9BD98
  if (MotherBrain_MakeHerStandUp() & 1) {
    Enemy_MotherBrain *E = Get_MotherBrain(0);
    E->mbn_var_A = FUNC16(MotherBrain_Phase2Cut_2);
    E->mbn_var_F = 16;
    MotherBrain_Phase2Cut_2();
  }
}

void MotherBrain_Phase2Cut_2(void) {  // 0xA9BDA9
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0) {
    MotherBrain_SetBrainInstrs(addr_kMotherBrain_Ilist_9B7F);
    E->mbn_var_A = FUNC16(MotherBrain_Phase2Cut_3);
    E->mbn_var_F = 256;
  }
}

void MotherBrain_Phase2Cut_3(void) {  // 0xA9BDC1
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0) {
    MotherBrain_SetBrainInstrs(addr_kMotherBrain_Ilist_9F6C);
    E->mbn_var_A = FUNC16(MotherBrain_Phase2Cut_4);
    MotherBrain_Phase2Cut_4();
  }
}

void MotherBrain_Phase2Cut_4(void) {  // 0xA9BDD2
  if (ProcessSpriteTilesTransfers(0xa9, addr_stru_A98FE5)) {
    MotherBrain_RetractHead();
    MotherBrain_SpawnShitroidInCutscene();
    Enemy_MotherBrain *E = Get_MotherBrain(0);
    E->mbn_var_A = FUNC16(MotherBrain_Phase2Cut_5);
    E->mbn_var_F = 256;
  }
}

void MotherBrain_Phase2Cut_5(void) {  // 0xA9BDED
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0) {
    MotherBrain_BodyRainbowBeamPalAnimIndex0();
    MotherBrain_SetBrainInstrs(addr_kMotherBrain_Ilist_9C77);
    Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
    E1->mbn_var_32 = 6;
    E1->mbn_var_33 = 6;
    E1->mbn_var_34 = 1280;
    QueueSfx2_Max6(0x71);
    E->mbn_var_A = FUNC16(nullsub_364);
  }
}

void MotherBrain_SpawnShitroidInCutscene(void) {  // 0xA9BE1B
  Get_MotherBrain(0x40)->mbn_var_0A = SpawnEnemy(0xA9, addr_stru_A9BE28);
}

void MotherBrain_DrainedByShitroid_0(void) {  // 0xA9BE38
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->mbn_var_00 = 3;
  Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
  E1->mbn_var_32 = 8;
  E1->mbn_var_33 = 8;
  E1->mbn_var_34 = 1792;
  E->mbn_var_A = FUNC16(MotherBrain_DrainedByShitroid_1);
  E->mbn_var_F = 48;
  MotherBrain_DrainedByShitroid_1();
}

void MotherBrain_DrainedByShitroid_1(void) {  // 0xA9BE5D
  MotherBrain_HandleRainbowBeamPalette();
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0) {
    E->mbn_var_A = FUNC16(MotherBrain_DrainedByShitroid_2);
    Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
    E1->mbn_var_08 = FUNC16(MotherBrain_PainfulWalkForwards);
    E1->mbn_var_06 = 0;
    E1->mbn_var_07 = 2;
    E1->mbn_var_32 = 2;
    E1->mbn_var_33 = 4;
  }
}

void CallMotherBrainPainfulWalk(uint32 ea) {
  switch (ea) {
  case fnMotherBrain_PainfulWalkForwards: MotherBrain_PainfulWalkForwards(); return;
  case fnMotherBrain_PainfulWalkingForwards: MotherBrain_PainfulWalkingForwards(); return;
  case fnMotherBrain_PainfulWalkBackwards: MotherBrain_PainfulWalkBackwards(); return;
  case fnMotherBrain_PainfulWalkingBackwards: MotherBrain_PainfulWalkingBackwards(); return;
  default: Unreachable();
  }
}

void MotherBrain_DrainedByShitroid_2(void) {  // 0xA9BE96
  Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
  if (!E1->mbn_var_00)
    E1->mbn_var_00 = 50;
  MotherBrain_HandleRainbowBeamPalette();
  CallMotherBrainPainfulWalk(E1->mbn_var_08 | 0xA90000);
  uint16 v2 = 2 * E1->mbn_var_06;
  E1->mbn_var_07 = g_byte_A9BEEE[v2];
  E1->mbn_var_34 = g_word_A9BEFE[v2 >> 1];
  if (E1->mbn_var_06 == 6) {
    Enemy_MotherBrain *E = Get_MotherBrain(0);
    E->mbn_var_16 = 0;
    E1->mbn_var_10 = 0;
    MotherBrain_WritePhase2DeathPalette();
    QueueSfx1_Max6(2);
    E->mbn_var_A = FUNC16(MotherBrain_DrainedByShitroid_3);
  }
}

void MotherBrain_DrainedByShitroid_3(void) {  // 0xA9BF0E
  Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
  CallMotherBrainPainfulWalk(E1->mbn_var_08 | 0xA90000);
  if (sign16(E1->mbn_var_06 - 8)) {
  } else {
    E1->mbn_var_34 = 64;
    E1->mbn_var_32 = 8;
    E1->mbn_var_33 = 8;
    MotherBrain_SetBrainInstrs(addr_kMotherBrain_Ilist_9C39);
    Get_MotherBrain(0)->mbn_var_A = FUNC16(MotherBrain_DrainedByShitroid_4);
    MotherBrain_DrainedByShitroid_4();
  }
}

void MotherBrain_DrainedByShitroid_4(void) {  // 0xA9BF41
  if (MotherBrain_MakeWalkBackwards(0x28, 0) & 1) {
    Get_MotherBrain(0)->mbn_var_A = FUNC16(MotherBrain_DrainedByShitroid_5);
    Get_MotherBrain(0x40)->mbn_var_33 = 0;
    MotherBrain_DrainedByShitroid_5();
  }
}

void MotherBrain_DrainedByShitroid_5(void) {  // 0xA9BF56
  Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
  if (!*(uint32 *)&E1->mbn_var_32) {
    E1->mbn_var_12 = E1->mbn_var_33 | E1->mbn_var_32;
    Enemy_MotherBrain *E = Get_MotherBrain(0);
    if (!E->mbn_var_02) {
      MotherBrain_SetBodyInstrs(addr_kMotherBrain_Ilist_9A26);
      E->mbn_var_A = FUNC16(MotherBrain_DrainedByShitroid_6);
      E->mbn_var_F = 64;
    }
  }
}

void MotherBrain_DrainedByShitroid_6(void) {  // 0xA9BF7D
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0) {
    E->mbn_var_37 = 0;
    E->mbn_var_A = FUNC16(MotherBrain_DrainedByShitroid_7);
    E->mbn_var_F = 16;
    MotherBrain_DrainedByShitroid_7();
  }
}

void MotherBrain_DrainedByShitroid_7(void) {  // 0xA9BF95
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  bool v2 = (--E->mbn_var_F & 0x8000) != 0;
  uint16 v3;
  if (v2
      && (E->mbn_var_F = 16,
          v3 = E->mbn_var_37 + 1,
          E->mbn_var_37 = v3,
          MotherBrain_FadeToGray_Drained(v3 - 1) & 1)) {
    Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
    E1->base.health = -29536;
    E->mbn_var_1F = 1;
    E1->mbn_var_14 = 0;
    E->mbn_var_00 = 2;
    E->mbn_var_A = FUNC16(MotherBrain_Phase2_Revive_0);
  }
}

void MotherBrain_PainfulWalkForwards(void) {  // 0xA9BFD0
  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  if (MotherBrain_MakeWalkForwards(E->mbn_var_07, 0x48) & 1) {
    E->mbn_var_08 = FUNC16(MotherBrain_PainfulWalkingForwards);
    MotherBrain_SetPainfulWalkingTimer();
  }
}

void MotherBrain_PainfulWalkingForwards(void) {  // 0xA9BFE8
  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  uint16 v1 = E->mbn_var_09 - 1;
  E->mbn_var_09 = v1;
  if (!v1) {
    ++E->mbn_var_06;
    E->mbn_var_08 = FUNC16(MotherBrain_PainfulWalkBackwards);
  }
}

void MotherBrain_PainfulWalkBackwards(void) {  // 0xA9C004
  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  if (MotherBrain_MakeWalkBackwards(0x28, E->mbn_var_07) & 1) {
    E->mbn_var_08 = FUNC16(MotherBrain_PainfulWalkingBackwards);
    MotherBrain_SetPainfulWalkingTimer();
  }
}

void MotherBrain_PainfulWalkingBackwards(void) {  // 0xA9C01C
  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  uint16 v1 = E->mbn_var_09 - 1;
  E->mbn_var_09 = v1;
  if (!v1) {
    ++E->mbn_var_06;
    E->mbn_var_08 = FUNC16(MotherBrain_PainfulWalkForwards);
  }
}

void MotherBrain_SetPainfulWalkingTimer(void) {  // 0xA9C038
  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  E->mbn_var_09 = LOBYTE(g_word_A9C049[E->mbn_var_06]);
}

void MotherBrain_Phase2_Revive_0(void) {  // 0xA9C059
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->mbn_var_A = FUNC16(MotherBrain_Phase2_Revive_1);
  E->mbn_var_F = 768;
}

void MotherBrain_Phase2_Revive_1(void) {  // 0xA9C066
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0) {
    Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
    E1->mbn_var_14 = 1;
    E1->mbn_var_12 = 1;
    E->mbn_var_A = FUNC16(MotherBrain_Phase2_Revive_2);
    E->mbn_var_F = 224;
    MotherBrain_Phase2_Revive_2();
  }
}

void MotherBrain_Phase2_Revive_2(void) {  // 0xA9C082
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->mbn_var_A = FUNC16(MotherBrain_Phase2_Revive_3);
  E->mbn_var_37 = 0;
  MotherBrain_Phase2_Revive_3();
}

void MotherBrain_Phase2_Revive_3(void) {  // 0xA9C08F
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  bool v1 = (--E->mbn_var_F & 0x8000) != 0;
  uint16 v2;
  if (v1
      && (E->mbn_var_F = 16,
          v2 = E->mbn_var_37 + 1,
          E->mbn_var_37 = v2,
          MotherBrain_FadeFromGray_Drained(v2 - 1) & 1)) {
    E->mbn_var_A = FUNC16(MotherBrain_Phase2_Revive_4);
    Get_MotherBrain(0x40)->mbn_var_10 = 1;
    MotherBrain_SetupBrainNormalPal();
    MotherBrain_Phase2_Revive_4();
  }
}

void MotherBrain_Phase2_Revive_4(void) {  // 0xA9C0BA
  if (MotherBrain_MakeHerStandUp() & 1) {
    Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
    E1->mbn_var_32 = 6;
    E1->mbn_var_33 = 6;
    E1->mbn_var_34 = 1280;
    E1->mbn_var_31 = 1;
    Enemy_MotherBrain *E = Get_MotherBrain(0);
    E->mbn_var_A = FUNC16(MotherBrain_Phase2_Revive_5);
    E->mbn_var_F = 16;
    MotherBrain_Phase2_Revive_5();
  }
}

void MotherBrain_Phase2_Revive_5(void) {  // 0xA9C0E4
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0) {
    MotherBrain_SetBrainInstrs(addr_kMotherBrain_Ilist_9BB3);
    E->mbn_var_A = FUNC16(MotherBrain_Phase2_Revive_6);
    E->mbn_var_F = 128;
    MotherBrain_Phase2_Revive_6();
  }
}

void MotherBrain_Phase2_Revive_6(void) {  // 0xA9C0FB
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  bool v1 = (--E->mbn_var_F & 0x8000) != 0;
  if (v1 && MotherBrain_MakeWalkForwards(4, 0x50) & 1) {
    E->mbn_var_A = FUNC16(MotherBrain_Phase2_Revive_7);
    E->mbn_var_1F = 2;
    Get_MotherBrain(0x40)->mbn_var_11 = 1;
  }
}

void MotherBrain_Phase2_Revive_7(void) {  // 0xA9C11E
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->mbn_var_13 = 0;
  Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
  E1->mbn_var_31 = 1;
  E1->mbn_var_32 = 2;
  E1->mbn_var_33 = 4;
  E1->mbn_var_34 = 64;
  E->mbn_var_A = FUNC16(MotherBrain_Phase2_Revive_8);
  MotherBrain_Phase2_Revive_8();
}

void MotherBrain_Phase2_Revive_8(void) {  // 0xA9C147
  if (MotherBrain_MakeHerStandUp() & 1) {
    Get_MotherBrain(0)->mbn_var_A = FUNC16(MotherBrain_Phase2_MurderShitroid_1);
    MotherBrain_MakeWalkForwards(0xA, 0x50);
    MotherBrain_Phase2_MurderShitroid_1();
  }
}

void MotherBrain_Phase2_MurderShitroid_1(void) {  // 0xA9C15C
  MotherBrain_MaybeStandupOrLeanDown();
  if ((random_number & 0x8000) != 0) {
    uint16 v0 = addr_kMotherBrain_Ilist_9DBB;
    if (Get_MotherBrain(0x40)->mbn_var_0A)
      v0 = addr_kMotherBrain_Ilist_9DB1;
    MotherBrain_SetBrainInstrs(v0);
    Enemy_MotherBrain *E = Get_MotherBrain(0);
    E->mbn_var_A = FUNC16(MotherBrain_Phase2_MurderShitroid_2);
    E->mbn_var_F = 64;
  }
}

void MotherBrain_Phase2_MurderShitroid_2(void) {  // 0xA9C182
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0)
    E->mbn_var_A = FUNC16(MotherBrain_Phase2_MurderShitroid_1);
}

void MotherBrain_Phase2_PrepareForFinalShitroid(void) {  // 0xA9C18E
  MotherBrain_MakeHerStandUp();
  MotherBrain_MakeWalkBackwards(0x40, 4);
}

void MotherBrain_Phase2_ExecuteFinalkShitroid(void) {  // 0xA9C19A
  MotherBrain_SetBrainInstrs(addr_kMotherBrain_Ilist_9DB1);
  Get_MotherBrain(0)->mbn_var_A = FUNC16(nullsub_363);
}

void MotherBrain_MaybeStandupOrLeanDown(void) {  // 0xA9C1A7
  uint16 mbn_var_02 = Get_MotherBrain(0)->mbn_var_02;
  if (mbn_var_02) {
    if (mbn_var_02 == 6 && (uint8)random_number >= 0xC0)
      MotherBrain_MakeHerStandUp();
  } else if ((uint8)random_number >= 0xC0) {
    MotherBrain_MakeHerLeanDown();
  }
}

void MotherBrain_Phase3_Recover_MakeDistance(void) {  // 0xA9C1CF
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->mbn_var_00 = 4;
  E->mbn_var_A = FUNC16(MotherBrain_Phase3_Recover_SetupForFight);
  E->mbn_var_F = 32;
  MotherBrain_MakeWalkBackwards(E->base.x_pos - 14, 2);
}

void MotherBrain_Phase3_Recover_SetupForFight(void) {  // 0xA9C1F0
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0) {
    E->mbn_var_A = FUNC16(MotherBrain_Phase3_Fighting_Main);
    Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
    E1->mbn_var_18 = FUNC16(MotherBrain_Phase3_Neck_Normal);
    E1->mbn_var_1A = FUNC16(MotherBrain_Phase3_Walk_TryToInchForward);
    MotherBrain_Phase3_Fighting_Main();
  }
}

void MotherBrain_Phase3_Fighting_Main(void) {  // 0xA9C209
  if (Get_MotherBrain(0x40)->base.health) {
    MotherBrain_Phase3_NeckHandler();
    MotherBrain_Phase3_WalkHandler();
    Enemy_MotherBrain *E = Get_MotherBrain(0);
    if (!E->mbn_var_02 && !E->mbn_var_06 && (random_number & 0x8000) != 0) {
      uint16 v1 = addr_kMotherBrain_Ilist_9F00;
      if ((uint8)random_number >= 0x80)
        v1 = addr_kMotherBrain_Ilist_9DBB;
      MotherBrain_SetBrainInstrs(v1);
      E->mbn_var_A = FUNC16(MotherBrain_Phase3_Fighting_Cooldown);
      E->mbn_var_F = 64;
    }
  } else {
    Get_MotherBrain(0)->mbn_var_A = FUNC16(MotherBrain_Phase3_Death_0);
  }
}

void MotherBrain_Phase3_Fighting_Cooldown(void) {  // 0xA9C24E
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((--E->mbn_var_F & 0x8000) != 0)
    E->mbn_var_A = FUNC16(MotherBrain_Phase3_Fighting_Main);
}

void CallMotherBrainWallkFunc1A(uint32 ea) {
  switch (ea) {
  case fnMotherBrain_Phase3_Walk_TryToInchForward: MotherBrain_Phase3_Walk_TryToInchForward(); return;
  case fnMotherBrain_Phase3_Walk_RetreatQuickly: MotherBrain_Phase3_Walk_RetreatQuickly(); return;
  case fnMotherBrain_Phase3_Walk_RetreatSlowly: MotherBrain_Phase3_Walk_RetreatSlowly(); return;
  default: Unreachable();
  }
}

void MotherBrain_Phase3_WalkHandler(void) {  // 0xA9C25A
  if (!Get_MotherBrain(0)->mbn_var_02) {
    CallMotherBrainWallkFunc1A(Get_MotherBrain(0x40)->mbn_var_1A | 0xA90000);
  }
}

void MotherBrain_Phase3_Walk_TryToInchForward(void) {  // 0xA9C26A
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  uint16 mbn_var_07 = E->mbn_var_07;
  if (mbn_var_07) {
    uint16 v3 = mbn_var_07 + 32;
    E->mbn_var_07 = v3;
    if (v3 >= 0x100) {
      uint16 v4 = E->base.x_pos + 1;
      Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
      E1->mbn_var_1B = v4;
      if (MotherBrain_MakeWalkForwards((random_number & 2) + 4, E1->mbn_var_1B) & 1)
        E->mbn_var_07 = 128;
    }
  } else {
    uint16 v6 = E->base.x_pos - 14;
    Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
    E1->mbn_var_1B = v6;
    E1->mbn_var_1A = FUNC16(MotherBrain_Phase3_Walk_RetreatQuickly);
    MotherBrain_Phase3_Walk_RetreatQuickly();
  }
}

void MotherBrain_Phase3_Walk_RetreatQuickly(void) {  // 0xA9C2B3
  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  if (MotherBrain_MakeWalkBackwards(E->mbn_var_1B, 2) & 1) {
    E->mbn_var_1B = Get_MotherBrain(0)->base.x_pos - 14;
    E->mbn_var_1A = FUNC16(MotherBrain_Phase3_Walk_RetreatSlowly);
  }
}

void MotherBrain_Phase3_Walk_RetreatSlowly(void) {  // 0xA9C2D2
  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  if (MotherBrain_MakeWalkBackwards(E->mbn_var_1B, 4) & 1)
    MotherBrain_SetToTryToInchForward(0x40);
}

void MotherBrain_SetToTryToInchForward(uint16 a) {  // 0xA9C313
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->mbn_var_07 = a;
  Enemy_MotherBrain *E1 = Get_MotherBrain(0x40);
  E1->mbn_var_1A = FUNC16(MotherBrain_Phase3_Walk_TryToInchForward);
  E1->mbn_var_1B = E->base.x_pos + 1;
}

void CallMotherBrainNeckFunc(uint32 ea) {
  switch (ea) {
  case fnnullsub_369: return;
  case fnMotherBrain_Phase3_Neck_Normal: MotherBrain_Phase3_Neck_Normal(); return;
  case fnMotherBrain_Phase3_Neck_SetupRecoilRecovery: MotherBrain_Phase3_Neck_SetupRecoilRecovery(); return;
  case fnMotherBrain_Phase3_Neck_RecoilRecovery: MotherBrain_Phase3_Neck_RecoilRecovery(); return;
  case fnMotherBrain_Phase3_Neck_SetupHyperBeamRecoil: MotherBrain_Phase3_Neck_SetupHyperBeamRecoil(); return;
  case fnMotherBrain_Phase3_Neck_HyperBeamRecoil: MotherBrain_Phase3_Neck_HyperBeamRecoil(); return;
  default: Unreachable();
  }
}

void MotherBrain_Phase3_NeckHandler(void) {  // 0xA9C327
  CallMotherBrainNeckFunc(Get_MotherBrain(0x40)->mbn_var_18 | 0xA90000);
}

void MotherBrain_Phase3_Neck_Normal(void) {  // 0xA9C330
  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  E->mbn_var_32 = 1;
  E->mbn_var_34 = 128;
  E->mbn_var_32 = 2;
  E->mbn_var_33 = 4;
  E->mbn_var_18 = addr_locret_A9C353;
}

void MotherBrain_Phase3_Neck_SetupRecoilRecovery(void) {  // 0xA9C354
  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  E->mbn_var_31 = 1;
  E->mbn_var_34 = 1280;
  E->mbn_var_32 = 6;
  E->mbn_var_33 = 6;
  E->mbn_var_18 = FUNC16(MotherBrain_Phase3_Neck_RecoilRecovery);
  E->mbn_var_19 = 16;
  MotherBrain_Phase3_Neck_RecoilRecovery();
}

void MotherBrain_Phase3_Neck_RecoilRecovery(void) {  // 0xA9C37B
  int16 v1;

  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  v1 = E->mbn_var_19 - 1;
  if (v1 < 0) {
    MotherBrain_SetBrainInstrs(addr_kMotherBrain_Ilist_9DBB);
    E->mbn_var_18 = FUNC16(MotherBrain_Phase3_Neck_Normal);
  } else {
    E->mbn_var_19 = v1;
  }
}

void MotherBrain_Phase3_Neck_SetupHyperBeamRecoil(void) {  // 0xA9C395
  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  E->mbn_var_31 = 1;
  Get_MotherBrain(0)->mbn_var_06 = 1;
  MotherBrain_SetBrainInstrs(addr_kMotherBrain_Ilist_9BE7);
  E->mbn_var_00 = 50;
  E->mbn_var_34 = 2304;
  E->mbn_var_32 = 8;
  E->mbn_var_33 = 8;
  E->mbn_var_18 = FUNC16(MotherBrain_Phase3_Neck_HyperBeamRecoil);
  E->mbn_var_19 = 11;
  MotherBrain_Phase3_Neck_HyperBeamRecoil();
}

void MotherBrain_Phase3_Neck_HyperBeamRecoil(void) {  // 0xA9C3CD
  int16 v2;

  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  v2 = E->mbn_var_19 - 1;
  if (v2 < 0) {
    E->mbn_var_34 = 128;
    Get_MotherBrain(0)->mbn_var_06 = 0;
    E->mbn_var_18 = FUNC16(MotherBrain_Phase3_Neck_SetupRecoilRecovery);
  } else {
    E->mbn_var_19 = v2;
  }
}

void MoveEnemyWithVelocity(void) {  // 0xA9C3EF
  EnemyData *v0 = gEnemyData(cur_enemy_index);
  uint16 ai_var_B = v0->ai_var_B;
  int carry = HIBYTE(v0->x_subpos) + LOBYTE(ai_var_B);
  HIBYTE(v0->x_subpos) = carry;
  v0->x_pos += (int8)(ai_var_B >> 8) + (carry >> 8);

  uint16 ai_var_C = v0->ai_var_C;
  carry = HIBYTE(v0->y_subpos) + LOBYTE(ai_var_C);
  HIBYTE(v0->y_subpos) = carry;
  v0->y_pos += (int8)(ai_var_C >> 8) + (carry >> 8);
}

void MotherBrain_SetBodyInstrs(uint16 a) {  // 0xA9C42D
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->base.current_instruction = a;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
}

void MotherBrain_SetBrainUnusedInstrs(uint16 a) {  // 0xA9C43A
  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  E->base.current_instruction = a;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
}

void MotherBrain_SetBrainInstrs(uint16 a) {  // 0xA9C447
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->mbn_var_21 = a;
  E->mbn_var_20 = 1;
}

void Enemy_SetInstrList(uint16 k, uint16 a) {  // 0xA9C453
  EnemyData *v2 = gEnemyData(k);
  v2->current_instruction = a;
  v2->instruction_timer = 1;
  v2->timer = 0;
}

uint16 ComputeSinMult(uint16 a, uint16 r18) {  // 0xA9C460
  return sub_A9C46C(r18, a);
}

uint16 ComputeCosMult(uint16 a, uint16 r18) {  // 0xA9C465
  return sub_A9C46C(r18 + 64, a);
}

uint16 sub_A9C46C(uint16 a, uint16 j) {  // 0xA9C46C
  uint16 v2 = kSinCosTable8bit_Sext[(a & 0xff) + 64];
  WriteReg(M7A, v2);
  WriteReg(M7A, HIBYTE(v2));
  WriteReg(M7B, j);
  return ReadRegWord(MPYM);
}

void Enemy_IncreaseYpos(uint16 k, uint16 a) {  // 0xA9C4A9
  EnemyData *v2 = gEnemyData(k);
  int carry = HIBYTE(v2->y_subpos) + LOBYTE(a);
  HIBYTE(v2->y_subpos) = carry;
  v2->y_pos += (int8)(a >> 8) + (carry >> 8);
}

void Samus_DecrementAmmoDueToRainbowBeam(void) {  // 0xA9C4C4
  if ((random_enemy_counter & 3) == 0 && samus_missiles) {
    uint16 v0 = samus_missiles - g_word_A9C544;
    if (sign16(samus_missiles - g_word_A9C544 - 1)) {
      if (hud_item_index == 1)
        hud_item_index = 0;
      v0 = 0;
      samus_auto_cancel_hud_item_index = 0;
    }
    samus_missiles = v0;
  }
  if ((random_enemy_counter & 3) == 0 && samus_super_missiles) {
    uint16 v1 = samus_super_missiles - g_word_A9C544;
    if (sign16(samus_super_missiles - g_word_A9C544 - 1)) {
      if (hud_item_index == 2)
        hud_item_index = 0;
      v1 = 0;
      samus_auto_cancel_hud_item_index = 0;
    }
    samus_super_missiles = v1;
  }
  if (samus_power_bombs) {
    uint16 v2 = samus_power_bombs - g_word_A9C544;
    if (sign16(samus_power_bombs - g_word_A9C544 - 1)) {
      if (hud_item_index == 3)
        hud_item_index = 0;
      v2 = 0;
      samus_auto_cancel_hud_item_index = 0;
    }
    samus_power_bombs = v2;
  }
}

void Samus_PlayGainingLosingHealthSfx(void) {  // 0xA9C546
  if (!sign16(samus_health - 81) && (random_enemy_counter & 7) == 0)
    QueueSfx3_Max3(0x2D);
}

void Samus_DamageDueToRainbowBeam(void) {  // 0xA9C57D
  uint16 v0 = samus_health + (equipped_items & 1) - 2;
  if (sign16(v0 - 1))
    v0 = 0;
  samus_health = v0;
  Samus_PlayGainingLosingHealthSfx();
}

uint8 ProcessSpriteTilesTransfers(uint8 db, uint16 k) {  // 0xA9C5BE
  VramWriteEntry *v7;

  Enemy_MotherBrain *E = Get_MotherBrain(0);
  uint16 src = E->mbn_var_22;
  if (!src)
    src = k;
  uint16 v4 = vram_write_queue_tail;
  const uint8 *v5 = RomPtrWithBank(db, src);
  uint16 v6 = GET_WORD(v5);
  if (v6) {
    v7 = gVramWriteEntry(vram_write_queue_tail);
    v7->size = v6;
    *(VoidP *)((uint8 *)&v7->src.addr + 1) = GET_WORD(v5 + 3);
    v7->src.addr = GET_WORD(v5 + 2);
    v7->vram_dst = GET_WORD(v5 + 5);
    vram_write_queue_tail = v4 + 7;
    E->mbn_var_22 = src + 7;
    v6 = *(uint16 *)RomPtrWithBank(db, E->mbn_var_22);
    if (v6)
      return 0;
  }
  E->mbn_var_22 = v6;
  return 1;
}

uint8 MotherBrain_MakeWalkForwards(uint16 j, uint16 a) {  // 0xA9C601
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((int16)(a - E->base.x_pos) >= 0) {
    if (E->mbn_var_02)
      return 0;
    if (sign16(E->base.x_pos - 128)) {
      MotherBrain_SetBodyInstrs(g_off_A9C61E[j >> 1]);
      return 0;
    }
  }
  return 1;
}

uint8 MotherBrain_MakeWalkBackwards(uint16 a, uint16 j) {  // 0xA9C647
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if ((int16)(a - E->base.x_pos) < 0) {
    if (E->mbn_var_02)
      return 0;
    if (!sign16(E->base.x_pos - 48)) {
      MotherBrain_SetBodyInstrs(g_off_A9C664[j >> 1]);
      return 0;
    }
  }
  return 1;
}

uint8 MotherBrain_MakeHerStandUp(void) {  // 0xA9C670
  uint16 mbn_var_02 = Get_MotherBrain(0)->mbn_var_02;
  if (mbn_var_02) {
    uint16 v1 = addr_kMotherBrain_Ilist_99C6;
    if (mbn_var_02 != 3) {
      if (mbn_var_02 != 6)
        return 0;
      v1 = addr_kMotherBrain_Ilist_99E2;
    }
    MotherBrain_SetBodyInstrs(v1);
    return 0;
  }
  return 1;
}

uint8 MotherBrain_MakeHerCrouch(void) {  // 0xA9C68E
  uint16 mbn_var_02 = Get_MotherBrain(0)->mbn_var_02;
  if (mbn_var_02) {
    if (mbn_var_02 == 3)
      return 1;
  } else {
    MotherBrain_SetBodyInstrs(addr_kMotherBrain_Ilist_9A0A);
  }
  return 0;
}

uint8 MotherBrain_MakeHerLeanDown(void) {  // 0xA9C6A3
  uint16 mbn_var_02 = Get_MotherBrain(0)->mbn_var_02;
  if (mbn_var_02) {
    if (mbn_var_02 == 6)
      return 1;
  } else {
    MotherBrain_SetBodyInstrs(addr_kMotherBrain_Ilist_99F2);
  }
  return 0;
}

void MotherBrain_HandleWalking(void) {  // 0xA9C6B8
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if (!E->mbn_var_02) {
    uint16 mbn_var_07 = E->mbn_var_07;
    if (mbn_var_07) {
      uint16 v2;
      v2 = mbn_var_07 + 6;
      E->mbn_var_07 = v2;
      if (v2 >= 0x100) {
LABEL_4:
        E->mbn_var_07 = 128;
        if (sign16(E->base.x_pos - 128))
          MotherBrain_SetBodyInstrs(addr_kMotherBrain_Ilist_97A4);
        return;
      }
      if (!sign16(E->base.x_pos - 48))
        return;
    } else {
      E->mbn_var_07 = 1;
      if (!sign16(E->base.x_pos - 48)) {
        MotherBrain_SetBodyInstrs(addr_kMotherBrain_Ilist_98C6);
        return;
      }
    }
    if (sign16((random_number & 0xFFF) - 4032))
      return;
    goto LABEL_4;
  }
}

void MotherBrain_Pal_ProcessInvincibility(void) {  // 0xA9CFD4
  if (Get_MotherBrain(0)->mbn_var_00 == 4) {
    uint16 flash_timer = Get_MotherBrain(0x40)->base.flash_timer;
    if (flash_timer >> 1) {
      if (!(flash_timer & 1)) {
        for (int i = 28; i >= 0; i -= 2) {
          int v2 = i >> 1;
          uint16 v3 = palette_buffer[v2 + 129];
          palette_buffer[v2 + 145] = v3;
          palette_buffer[v2 + 177] = v3;
          palette_buffer[v2 + 65] = v3;
        }
      }
    }
  }
}

void MotherBrain_Pal_BeginScreenFlash(void) {  // 0xA9CFFD
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->mbn_var_0E = addr_stru_A9D046;
  E->mbn_var_0F = 1;
}

void MotherBrain_Pal_EndScreenFlash(void) {  // 0xA9D00C
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->mbn_var_0E = 0;
  E->mbn_var_0F = 0;
  MotherBrain_Pal_WriteRoomPal(addr_word_A9D082);
}

void MotherBrain_Pal_HandleRoomPal(void) {  // 0xA9D01C
  uint16 j = HandleMotherBrainInstructionList(ADDR16_OF_RAM(*enemy_ram7800) + 28);
  if (j)
    MotherBrain_Pal_WriteRoomPal(j);
}

void MotherBrain_Pal_WriteRoomPal(uint16 j) {  // 0xA9D025
  WriteColorsToPalette(0x68, 0xa9, j, 12);
  WriteColorsToPalette(0xA6, 0xa9, j + 24, 12);
  WriteColorsToPalette(0xE6, 0xa9, j + 24, 12);
}

typedef struct MotherBrainInstrExecState {
  uint16 ip;
  uint16 timer;
} MotherBrainInstrExecState;

uint16 HandleMotherBrainInstructionList(uint16 a) {  // 0xA9D192
  MotherBrainInstrExecState *st = (MotherBrainInstrExecState *)&g_ram[a];
  if ((st->ip & 0x8000) == 0)
    return 0;
  uint16 v2 = st->ip;
  const uint16 *v3 = (uint16 *)RomPtr_A9(v2);
  if (sign16(v3[0]))
    goto LABEL_8;
  if (st->timer != v3[0]) {
    st->timer++;
    return v3[1];
  }
  v2 += 4;
  for (; ; ) {
    v3 = (uint16 *)RomPtr_A9(v2);
    if (!sign16(v3[0]))
      break;
LABEL_8:
    v2 = CallMotherBrainInstr(v3[0] | 0xA90000, v2 + 2);
  }
  if (!v3[0]) {
    st->ip = 0;
    st->timer = 0;
    return 0;
  } else {
    st->timer = 1;
    st->ip = v2;
    return v3[1];
  }
}

void MotherBrain_HandlePalette(void) {  // 0xA9D1E4
  Enemy_MotherBrain *E = Get_MotherBrain(0x40);
  if (E->mbn_var_10)
    MotherBrain_HandleBrainPal();
  if (E->mbn_var_11)
    MotherBrain_HealthBasedPaletteHandling();
}

void MotherBrain_SetupBrainNormalPal(void) {  // 0xA9D1F8
  Get_MotherBrain(0)->mbn_var_E = 10;
}

void MotherBrain_SetupBrainPalForLaser(void) {  // 0xA9D1FF
  Get_MotherBrain(0)->mbn_var_E = 514;
}

void MotherBrain_HandleBrainPal(void) {  // 0xA9D206
  int8 mbn_var_D_high;
  int8 mbn_var_D;

  Enemy_MotherBrain *E = Get_MotherBrain(0);
  if (E->mbn_var_00 != 1) {
    mbn_var_D_high = HIBYTE(E->mbn_var_D);
    if (mbn_var_D_high) {
      HIBYTE(E->mbn_var_D) = mbn_var_D_high - 1;
    } else {
      uint16 r18 = g_off_A9D260[HIBYTE(E->mbn_var_E) >> 1];
      HIBYTE(E->mbn_var_D) = E->mbn_var_E;
      mbn_var_D = E->mbn_var_D;
      if (mbn_var_D || Get_MotherBrain(0x40)->mbn_var_A == FUNC16(MotherBrainsBrain_SetupBrainAndNeckToDraw)) {
        uint8 v4 = (mbn_var_D + 1) & 7;
        LOBYTE(E->mbn_var_D) = v4;
        uint16 v5 = 290;
        if (E->mbn_var_0D != 512)
          v5 = 482;
        WriteColorsToPalette(v5, 0xa9, r18 + 8 * v4, 3);
      }
    }
  }
}

void WriteColorsToPalette(uint16 k, uint8 db, uint16 j, uint16 a) {  // 0xA9D2E4
  int n = a;
  do {
    palette_buffer[k >> 1] = *(uint16 *)RomPtrWithBank(db, j);
    k += 2;
    j += 2;
  } while (--n);
}

void WriteColorsToTargetPalette(uint8 db, uint16 k, uint16 j, uint16 a) {  // 0xA9D2F6
  int n = a;
  do {
    target_palettes[k >> 1] = *(uint16 *)RomPtrWithBank(db, j);
    k += 2;
    j += 2;
  } while (--n);
}

void MotherBrain_CorpseRottingInitFunc(void) {  // 0xA9E08B
  uint8 *base = (uint8*)&kraid_unk9000;
  const uint8 *src = RomPtr_B7(addr_kMotherBrain_Misc_TileData);
  MemCpy(base, src + 192, 0xC0);
  MemCpy(base + 0xe0, src + 704, 0xC0);
  MemCpy(base + 0x1C0, src + 1216, 0xC0);
  MemCpy(base + 0x2a0, src + 1728, 0xC0);
  MemCpy(base + 0x380, src + 2240, 0xE0);
  MemCpy(base + 0x460, src + 2752, 0xE0);
}

void MotherBrain_CorpseRottingMoveFunc(uint16 j, uint16 k) {  // 0xA9EA40
  Enemy_DeadMonsters *E = Get_DeadMonsters(0);
  if (E->dms_var_41 >= 0x10) {
    if (sign16(E->dms_var_41 - 46)) {
      *(uint16 *)((uint8 *)&g_word_7E9002 + j) = *(uint16 *)((uint8 *)&kraid_unk9000 + k);
      *(uint16 *)((uint8 *)&g_word_7E9012 + j) = *(uint16 *)((uint8 *)&g_word_7E900F + k + 1);
    }
    *(uint16 *)((uint8 *)&kraid_unk9000 + k) = 0;
    *(uint16 *)((uint8 *)&g_word_7E900F + k + 1) = 0;
  }
  if (E->dms_var_41 >= 8) {
    if (sign16(E->dms_var_41 - 46)) {
      *(uint16 *)((uint8 *)&g_word_7E9022 + j) = *(uint16 *)((uint8 *)&g_word_7E9020 + k);
      *(uint16 *)((uint8 *)&g_word_7E9032 + j) = *(uint16 *)((uint8 *)&g_word_7E9030 + k);
    }
    *(uint16 *)((uint8 *)&g_word_7E9020 + k) = 0;
    *(uint16 *)((uint8 *)&g_word_7E9030 + k) = 0;
  }
  if (sign16(E->dms_var_41 - 46)) {
    *(uint16 *)((uint8 *)&g_word_7E9042 + j) = *(uint16 *)((uint8 *)&g_word_7E9040 + k);
    *(uint16 *)((uint8 *)&g_word_7E9052 + j) = *(uint16 *)((uint8 *)&g_word_7E9050 + k);
  }
  *(uint16 *)((uint8 *)&g_word_7E9040 + k) = 0;
  *(uint16 *)((uint8 *)&g_word_7E9050 + k) = 0;
  if (sign16(E->dms_var_41 - 46)) {
    *(uint16 *)((uint8 *)&g_word_7E9062 + j) = *(uint16 *)((uint8 *)&g_word_7E9060 + k);
    *(uint16 *)((uint8 *)&g_word_7E9072 + j) = *(uint16 *)((uint8 *)&g_word_7E9070 + k);
  }
  *(uint16 *)((uint8 *)&g_word_7E9060 + k) = 0;
  *(uint16 *)((uint8 *)&g_word_7E9070 + k) = 0;
  if (sign16(E->dms_var_41 - 46)) {
    *(uint16 *)((uint8 *)&g_word_7E9082 + j) = *(uint16 *)((uint8 *)&g_word_7E9080 + k);
    *(uint16 *)((uint8 *)&g_word_7E9092 + j) = *(uint16 *)((uint8 *)&g_word_7E9090 + k);
  }
  *(uint16 *)((uint8 *)&g_word_7E9080 + k) = 0;
  *(uint16 *)((uint8 *)&g_word_7E9090 + k) = 0;
  if (E->dms_var_41 >= 8) {
    if (sign16(E->dms_var_41 - 46)) {
      *(uint16 *)((uint8 *)&g_word_7E90A2 + j) = *(uint16 *)((uint8 *)&g_word_7E90A0 + k);
      *(uint16 *)((uint8 *)&g_word_7E90B2 + j) = *(uint16 *)((uint8 *)&g_word_7E90B0 + k);
    }
    *(uint16 *)((uint8 *)&g_word_7E90A0 + k) = 0;
    *(uint16 *)((uint8 *)&g_word_7E90B0 + k) = 0;
  }
  if (E->dms_var_41 >= 0x20) {
    if (sign16(E->dms_var_41 - 46)) {
      *(uint16 *)((uint8 *)&g_word_7E90C2 + j) = *(uint16 *)((uint8 *)&g_word_7E90C0 + k);
      *(uint16 *)((uint8 *)&g_word_7E90D2 + j) = *(uint16 *)((uint8 *)&g_word_7E90D0 + k);
    }
    *(uint16 *)((uint8 *)&g_word_7E90C0 + k) = 0;
    *(uint16 *)((uint8 *)&g_word_7E90D0 + k) = 0;
  }
}

void MotherBrain_CorpseRottingCopyFunc(uint16 j, uint16 k) {  // 0xA9EB0B
  Enemy_DeadMonsters *E = Get_DeadMonsters(0);
  if (E->dms_var_41 >= 0x10 && sign16(E->dms_var_41 - 46)) {
    *(uint16 *)((uint8 *)&g_word_7E9002 + j) = *(uint16 *)((uint8 *)&kraid_unk9000 + k);
    *(uint16 *)((uint8 *)&g_word_7E9012 + j) = *(uint16 *)((uint8 *)&g_word_7E900F + k + 1);
  }
  if (E->dms_var_41 >= 8 && sign16(E->dms_var_41 - 46)) {
    *(uint16 *)((uint8 *)&g_word_7E9022 + j) = *(uint16 *)((uint8 *)&g_word_7E9020 + k);
    *(uint16 *)((uint8 *)&g_word_7E9032 + j) = *(uint16 *)((uint8 *)&g_word_7E9030 + k);
  }
  if (sign16(E->dms_var_41 - 46)) {
    *(uint16 *)((uint8 *)&g_word_7E9042 + j) = *(uint16 *)((uint8 *)&g_word_7E9040 + k);
    *(uint16 *)((uint8 *)&g_word_7E9052 + j) = *(uint16 *)((uint8 *)&g_word_7E9050 + k);
  }
  if (sign16(E->dms_var_41 - 46)) {
    *(uint16 *)((uint8 *)&g_word_7E9062 + j) = *(uint16 *)((uint8 *)&g_word_7E9060 + k);
    *(uint16 *)((uint8 *)&g_word_7E9072 + j) = *(uint16 *)((uint8 *)&g_word_7E9070 + k);
  }
  if (sign16(E->dms_var_41 - 46)) {
    *(uint16 *)((uint8 *)&g_word_7E9082 + j) = *(uint16 *)((uint8 *)&g_word_7E9080 + k);
    *(uint16 *)((uint8 *)&g_word_7E9092 + j) = *(uint16 *)((uint8 *)&g_word_7E9090 + k);
  }
  if (E->dms_var_41 >= 8 && sign16(E->dms_var_41 - 46)) {
    *(uint16 *)((uint8 *)&g_word_7E90A2 + j) = *(uint16 *)((uint8 *)&g_word_7E90A0 + k);
    *(uint16 *)((uint8 *)&g_word_7E90B2 + j) = *(uint16 *)((uint8 *)&g_word_7E90B0 + k);
  }
  if (E->dms_var_41 >= 0x20) {
    if (sign16(E->dms_var_41 - 46)) {
      *(uint16 *)((uint8 *)&g_word_7E90C2 + j) = *(uint16 *)((uint8 *)&g_word_7E90C0 + k);
      *(uint16 *)((uint8 *)&g_word_7E90D2 + j) = *(uint16 *)((uint8 *)&g_word_7E90D0 + k);
    }
  }
}
