// Enemy AI - Shitroid + Shitroid-in-cutscene runtime — peeled from Bank $A9

#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "enemy_types.h"
#include "sm_rtl.h"

#define kShitroid_FadingToBlack ((uint16*)RomFixedPtr(0xade8e2))
#define g_word_A9CDFC ((uint16*)RomFixedPtr(0xa9cdfc))
#define kShitroid_HealthBasedPalettes_Shell ((uint16*)RomFixedPtr(0xade7e2))
#define kShitroid_HealthBasedPalettes_Innards ((uint16*)RomFixedPtr(0xade882))

static const int16 g_word_A993BB[4] = { 0, -1, 0, 1 };
static const int16 g_word_A993C3[4] = { 0, 1, -1, 1 };
static const int8 g_byte_A9F56A[16] = { 0x10, 0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 9, 8, 7, 6, 5, 4, 3, 2, 1 };


void Samus_DamageDueToShitroid(void) {  // 0xA9C560
  int16 v0;

  v0 = -4;
  if (equipped_items & 1)
    v0 = -3;
  uint16 v1 = samus_health + (equipped_items & 1) + v0;
  if (sign16(v1 - 2))
    v1 = 1;
  samus_health = v1;
}

uint8 Samus_HealDueToShitroid(void) {  // 0xA9C59F
  if ((int16)(samus_health + 1 - samus_max_health) < 0) {
    ++samus_health;
    Samus_PlayGainingLosingHealthSfx();
    return 1;
  } else {
    samus_health = samus_max_health;
    Samus_PlayGainingLosingHealthSfx();
    return 0;
  }
}

void ShitroidInCutscene_Init(void) {  // 0xA9C710
  Enemy_ShitroidInCutscene *E = Get_ShitdroidInCutscene(cur_enemy_index);
  E->base.properties |= kEnemyProps_ProcessInstructions | kEnemyProps_BlockPlasmaBeam;
  E->base.palette_index = 3584;
  E->base.current_instruction = addr_kShitroid_Ilist_CFA2;
  E->base.instruction_timer = 1;
  E->sice_var_04 = 1;
  E->base.timer = 0;
  E->sice_var_E = 10;
  E->base.vram_tiles_index = 160;
  E->base.x_pos = 320;
  E->base.y_pos = 96;
  E->sice_var_B = 0;
  E->sice_var_C = 0;
  E->sice_var_09 = 0;
  E->sice_var_A = FUNC16(ShitroidInCutscene_DashOntoScreen);
  E->sice_var_F = 248;
  E->sice_var_0F = FUNC16(Shitroid_HandleCutscenePalette);
  WriteColorsToPalette(0x1E2, 0xa9, addr_kMotherBrainPalette_2 + 2, 0xF);
}

void CallShitroidCutsceneFuncA(uint32 ea, uint16 k) {
  switch (ea) {
  case fnnullsub_367: return;
  case fnShitroidInCutscene_DashOntoScreen: ShitroidInCutscene_DashOntoScreen(k); return;
  case fnShitroidInCutscene_CurveTowardsBrain: ShitroidInCutscene_CurveTowardsBrain(k); return;
  case fnShitroidInCutscene_GetIntoFace: ShitroidInCutscene_GetIntoFace(k); return;
  case fnShitroidInCutscene_LatchOntoBrain: ShitroidInCutscene_LatchOntoBrain(k); return;
  case fnShitroidInCutscene_SetMotherBrainToStumbleBack: ShitroidInCutscene_SetMotherBrainToStumbleBack(k); return;
  case fnShitroidInCutscene_ActivateRainbowBeam: ShitroidInCutscene_ActivateRainbowBeam(k); return;
  case fnShitroidInCutscene_BrainTurnsToCorpse: ShitroidInCutscene_BrainTurnsToCorpse(k); return;
  case fnShitroidInCutscene_StopDraining: ShitroidInCutscene_StopDraining(k); return;
  case fnShitroidInCutscene_LetGoAndSpawnDust: ShitroidInCutscene_LetGoAndSpawnDust(k); return;
  case fnShitroidInCutscene_MoveUpToCeiling: ShitroidInCutscene_MoveUpToCeiling(k); return;
  case fnShitroidInCutscene_MoveToSamus: ShitroidInCutscene_MoveToSamus(k); return;
  case fnShitroidInCutscene_LatchOntoSamus: ShitroidInCutscene_LatchOntoSamus(k); return;
  case fnShitroidInCutscene_HealSamusToFullHealth: ShitroidInCutscene_HealSamusToFullHealth(k); return;
  case fnShitroidInCutscene_IdleUntilToNoHealth: ShitroidInCutscene_IdleUntilToNoHealth(k); return;
  case fnShitroidInCutscene_ReleaseSamus: ShitroidInCutscene_ReleaseSamus(k); return;
  case fnShitroidInCutscene_StareDownMotherBrain: ShitroidInCutscene_StareDownMotherBrain(k); return;
  case fnShitroidInCutscene_FlyOffScreen: ShitroidInCutscene_FlyOffScreen(k); return;
  case fnShitroidInCutscene_MoveToFinalChargeStart: ShitroidInCutscene_MoveToFinalChargeStart(k); return;
  case fnShitroidInCutscene_InitiateFinalCharge: ShitroidInCutscene_InitiateFinalCharge(k); return;
  case fnShitroidInCutscene_FinalCharge: ShitroidInCutscene_FinalCharge(k); return;
  case fnShitroidInCutscene_ShitroidFinalBelow: ShitroidInCutscene_ShitroidFinalBelow(k); return;
  case fnShitroidInCutscene_PlaySamusTheme: ShitroidInCutscene_PlaySamusTheme(k); return;
  case fnShitroidInCutscene_PrepareSamusHyperbeam: ShitroidInCutscene_PrepareSamusHyperbeam(k); return;
  case fnShitroidInCutscene_DeathSequence: ShitroidInCutscene_DeathSequence(k); return;
  case fnShitroidInCutscene_UnloadShitroid: ShitroidInCutscene_UnloadShitroid(k); return;
  case fnShitroidInCutscene_LetSamusRainbowMore: ShitroidInCutscene_LetSamusRainbowMore(k); return;
  case fnShitroidInCutscene_FinishCutscene: ShitroidInCutscene_FinishCutscene(k); return;
  default: Unreachable();
  }
}

void CallShitroidCutsceneFunc0F(uint32 ea) {
  switch (ea) {
  case fnnullsub_368: return;
  case fnnullsub_338: return;
  case fnShitroid_HandleCutscenePalette: Shitroid_HandleCutscenePalette(); return;
  case fnShitroid_HandleCutscenePalette_LowHealth: Shitroid_HandleCutscenePalette_LowHealth(); return;
  default: Unreachable();
  }
}

void ShitroidInCutscene_Main(void) {  // 0xA9C779
  Enemy_ShitroidInCutscene *E = Get_ShitdroidInCutscene(cur_enemy_index);
  E->base.shake_timer = 0;
  CallShitroidCutsceneFuncA(E->sice_var_A | 0xA90000, cur_enemy_index);
  MoveEnemyWithVelocity();
  ShitroidInCutscene_Flashing(cur_enemy_index);
  ShitroidInCutscene_HandleHealthBasedPalette(cur_enemy_index);
  uint16 r18 = Get_ShitdroidInCutscene(cur_enemy_index)->sice_var_0F;
  CallShitroidCutsceneFunc0F(r18 | 0xA90000);
}

void ShitroidInCutscene_Flashing(uint16 k) {  // 0xA9C79C
  int16 v1;

  v1 = 3584;
  Enemy_ShitroidInCutscene *E = Get_ShitdroidInCutscene(k);
  uint16 sice_var_06 = E->sice_var_06;
  if (sice_var_06) {
    uint16 v4 = sice_var_06 - 1;
    E->sice_var_06 = v4;
    if ((v4 & 2) != 0)
      v1 = 0;
  }
  E->base.palette_index = v1;
}

void ShitroidInCutscene_HandleCry(void) {  // 0xA9C7B7
  Enemy_ShitroidInCutscene *E = Get_ShitdroidInCutscene(0);
  if (E->sice_var_14) {
    E->sice_var_14 = 0;
    QueueSfx2_Max6(0x72);
  }
}

void ShitroidInCutscene_DashOntoScreen(uint16 k) {  // 0xA9C7CC
  Enemy_ShitroidInCutscene *E = Get_ShitdroidInCutscene(k);
  if ((--E->sice_var_F & 0x8000) != 0) {
    E->sice_var_0A = -10240;
    E->sice_var_0B = 2560;
    E->sice_var_A = FUNC16(ShitroidInCutscene_CurveTowardsBrain);
    E->sice_var_F = 10;
    ShitroidInCutscene_CurveTowardsBrain(k);
  }
}

void ShitroidInCutscene_CurveTowardsBrain(uint16 k) {  // 0xA9C7EC
  ShitroidInCutscene_UpdateSpeedAndAngle(k, -384, -20480, 2560);
  Enemy_ShitroidInCutscene *E = Get_ShitroidInCutscene(k);
  if ((--E->sice_var_F & 0x8000) != 0) {
    E->sice_var_A = FUNC16(ShitroidInCutscene_GetIntoFace);
    E->sice_var_F = 9;
  }
}

void ShitroidInCutscene_GetIntoFace(uint16 k) {  // 0xA9C811
  bool v3; // sf
  ShitroidInCutscene_UpdateSpeedAndAngle(k, -1536, -32256, 3584);
  Enemy_ShitroidInCutscene *E = Get_ShitroidInCutscene(0x40);
  Rect16U rect = { E->base.x_pos, E->base.y_pos, 4, 4 };
  if (!Shitroid_Func_2(k, rect)
      || (E = Get_ShitroidInCutscene(k), v3 = (int16)(E->sice_var_F - 1) < 0, --E->sice_var_F, v3)) {
    Get_ShitroidInCutscene(k)->sice_var_A = FUNC16(ShitroidInCutscene_LatchOntoBrain);
    SomeMotherBrainScripts(1);
  }
}

void ShitroidInCutscene_LatchOntoBrain(uint16 k) {  // 0xA9C851
  Enemy_ShitroidInCutscene *E = Get_ShitroidInCutscene(0x40);
  Rect16U rect = { E->base.x_pos, E->base.y_pos - 24, 8, 8 };
  Shitroid_Func_GraduallyAccelerateTowards0x400(k, 0, rect.x, rect.y);
  if (!Shitroid_Func_2(k, rect))
    Get_ShitroidInCutscene(k)->sice_var_A = FUNC16(ShitroidInCutscene_SetMotherBrainToStumbleBack);
}

void ShitroidInCutscene_SetMotherBrainToStumbleBack(uint16 k) {  // 0xA9C879
  Enemy_ShitroidInCutscene *E = Get_ShitroidInCutscene(0);
  MotherBrain_MakeWalkBackwards(E->base.x_pos - 1, 2);
  Get_ShitroidInCutscene(k)->sice_var_A = FUNC16(ShitroidInCutscene_ActivateRainbowBeam);
  ShitroidInCutscene_ActivateRainbowBeam(k);
}

void ShitroidInCutscene_ActivateRainbowBeam(uint16 k) {  // 0xA9C889
  Enemy_ShitroidInCutscene *E1 = Get_ShitdroidInCutscene(0x40);
  if (Shitroid_AccelerateTowardsPoint(k, 0x200, E1->base.x_pos, E1->base.y_pos - 24) & 1) {
    Enemy_ShitroidInCutscene *E = Get_ShitdroidInCutscene(k);
    E->sice_var_B = 0;
    E->sice_var_C = 0;
    E->base.x_pos = E1->base.x_pos;
    E->base.y_pos = E1->base.y_pos - 24;
    Enemy_SetInstrList(k, addr_kShitroid_Ilist_CFB8);
    E->sice_var_A = FUNC16(ShitroidInCutscene_BrainTurnsToCorpse);
    E->sice_var_E = 1;
    Enemy_ShitroidInCutscene *E0 = Get_ShitdroidInCutscene(0);
    E0->sice_var_A = FUNC16(MotherBrain_DrainedByShitroid_0);
    QueueSfx1_Max6(0x40);
    E0->sice_var_16 = 1;
  }
}

void ShitroidInCutscene_BrainTurnsToCorpse(uint16 k) {  // 0xA9C8E2
  Enemy_ShitroidInCutscene *E = Get_ShitdroidInCutscene(k);
  Enemy_ShitroidInCutscene *E1 = Get_ShitdroidInCutscene(0x40);

  int v2 = (E->base.frame_counter & 6) >> 1;
  E->base.x_pos = E1->base.x_pos + g_word_A993BB[v2];
  E->base.y_pos = E1->base.y_pos + g_word_A993C3[v2] - 24;
  if (Get_ShitdroidInCutscene(0)->sice_var_1F) {
    E->sice_var_A = FUNC16(ShitroidInCutscene_StopDraining);
    E->sice_var_F = 64;
  }
}

void ShitroidInCutscene_StopDraining(uint16 k) {  // 0xA9C915
  Enemy_ShitroidInCutscene *E = Get_ShitroidInCutscene(k);
  Enemy_ShitroidInCutscene *E1 = Get_ShitroidInCutscene(0x40);
  E->base.x_pos = E1->base.x_pos;
  E->base.y_pos = E1->base.y_pos - 24;
  if ((--E->sice_var_F & 0x8000) != 0) {
    Enemy_SetInstrList(k, addr_kShitroid_Ilist_CFA2);
    E->sice_var_E = 10;
    E->sice_var_A = FUNC16(ShitroidInCutscene_LetGoAndSpawnDust);
    E->sice_var_F = 32;
    E->sice_var_B = 0;
    E->sice_var_C = 0;
  }
}

void ShitroidInCutscene_LetGoAndSpawnDust(uint16 k) {  // 0xA9C94B
  Enemy_ShitroidInCutscene *E = Get_ShitdroidInCutscene(k);
  if ((--E->sice_var_F & 0x8000) != 0) {
    ShitroidInCutscene_SpawnThreeDustClouds();
    E->sice_var_A = addr_loc_A9C959;
  }
  ShitroidInCutscene_MoveUpToCeiling(k);
}

void ShitroidInCutscene_MoveUpToCeiling(uint16 k) {  // 0xA9C959
  Rect16U rect = { Get_ShitdroidInCutscene(0x40)->base.x_pos, 0, 4, 4 };
  Shitroid_Func_GraduallyAccelerateTowards0x400(k, 0, rect.x, rect.y);
  if (!Shitroid_Func_2(k, rect)) {
    SomeMotherBrainScripts(4);
    Enemy_ShitroidInCutscene *E = Get_ShitdroidInCutscene(k);
    E->sice_var_A = FUNC16(ShitroidInCutscene_MoveToSamus);
    E->sice_var_0E = addr_stru_A9CA24;
  }
}

void ShitroidInCutscene_SpawnOneDustCloudAt(uint16 xd, uint16 yd) {  // 0xA9C9AA
  Enemy_ShitroidInCutscene *E = Get_ShitroidInCutscene(0x40);
  eproj_spawn_pt = (Point16U){ E->base.x_pos + xd, E->base.y_pos + yd };
  SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 9);
}

void ShitroidInCutscene_SpawnThreeDustClouds(void) {  // 0xA9C98C
  ShitroidInCutscene_SpawnOneDustCloudAt(-16, -8);
  ShitroidInCutscene_SpawnOneDustCloudAt(0, -16);
  ShitroidInCutscene_SpawnOneDustCloudAt(16, -8);
}

void CallShitroidMoveFunc(uint32 ea, uint16 k, uint16 j, uint16 r18, uint16 r20) {
  switch (ea) {
  case fnShitroid_GraduallyAccelerateTowards0x8: Shitroid_GraduallyAccelerateTowards0x8(k, j, r18, r20); return;
  case fnShitroid_GraduallyAccelerateTowards0x10: Shitroid_GraduallyAccelerateTowards0x10(k, j, r18, r20); return;
  default: Unreachable();
  }
}

void ShitroidInCutscene_MoveToSamus(uint16 k) {  // 0xA9C9C3
  int16 v4;

  Enemy_ShitroidInCutscene *E = Get_ShitdroidInCutscene(k);
  E->sice_var_04 = 0;
  E->sice_var_09 = 1;
  if ((random_number & 0xFFF) >= 0xFA0)
    QueueSfx2_Max6(0x52);
  uint16 sice_var_0E = E->sice_var_0E;
  const uint16 *v3 = (const uint16 *)RomPtr_A9(sice_var_0E);
  Rect16U rect = { v3[0], v3[1], 4, 4 };
  CallShitroidMoveFunc(v3[3] | 0xA90000, k, v3[2], rect.x, rect.y);
  if (!Shitroid_Func_2(k, rect)) {
    v4 = *((uint16 *)RomPtr_A9(sice_var_0E) + 4);
    if (v4 < 0) {
      E->sice_var_A = v4;
    } else {
      E->sice_var_0E += 8;
    }
  }
}

void ShitroidInCutscene_LatchOntoSamus(uint16 k) {  // 0xA9CA66
  Shitroid_Func_GraduallyAccelerateTowards0x400(k, 0, samus_x_pos, samus_y_pos - 20);
}

void ShitroidInCutscene_HealSamusToFullHealth(uint16 k) {  // 0xA9CA7A
  Enemy_ShitroidInCutscene *E = Get_ShitdroidInCutscene(k);
  E->sice_var_04 = 0;
  ShitroidInCutscene_HandleCry();
  int v2 = (E->base.frame_counter & 6) >> 1;
  E->base.x_pos = samus_x_pos + g_word_A993BB[v2];
  E->base.y_pos = samus_y_pos + g_word_A993C3[v2] - 20;
  if (!(Samus_HealDueToShitroid() & 1)) {
    samus_reserve_health = samus_max_reserve_health;
    E->sice_var_A = FUNC16(ShitroidInCutscene_IdleUntilToNoHealth);
    E->sice_var_0F = FUNC16(nullsub_368);
  }
}

void ShitroidInCutscene_IdleUntilToNoHealth(uint16 k) {  // 0xA9CABD
  ShitroidInCutscene_HandleCry();
  Enemy_ShitroidInCutscene *E = Get_ShitdroidInCutscene(k);
  uint16 sice_var_06 = E->sice_var_06;
  if (sice_var_06) {
    int v3 = (sice_var_06 & 6) >> 1;
    E->base.x_pos = samus_x_pos + 2 * g_word_A993BB[v3];
    E->base.y_pos = samus_y_pos + 2 * g_word_A993C3[v3] - 20;
  }
  if (!E->base.health) {
    E->base.health = 320;
    E->sice_var_A = FUNC16(ShitroidInCutscene_ReleaseSamus);
    E->sice_var_E = 10;
    E->sice_var_D = 0;
    E->sice_var_0F = FUNC16(Shitroid_HandleCutscenePalette_LowHealth);
    E->sice_var_04 = 1;
    E->sice_var_09 = 0;
  }
}

void ShitroidInCutscene_ReleaseSamus(uint16 k) {  // 0xA9CB13
  QueueSfx2_Max6(0x72);
  Get_ShitdroidInCutscene(0x40)->sice_var_0B = 1;
  Get_ShitdroidInCutscene(k)->sice_var_A = FUNC16(ShitroidInCutscene_StareDownMotherBrain);
  Get_ShitdroidInCutscene(0)->sice_var_A = FUNC16(MotherBrain_Phase2_PrepareForFinalShitroid);
  ShitroidInCutscene_StareDownMotherBrain(k);
}

void ShitroidInCutscene_StareDownMotherBrain(uint16 k) {  // 0xA9CB2D
  Rect16U rect = { samus_x_pos - 4, 96, 4, 4 };
  Shitroid_GraduallyAccelerateTowards0x10(k, 0, rect.x, rect.y);
  if (!Shitroid_Func_2(k, rect))
    Get_ShitroidInCutscene(k)->sice_var_A = FUNC16(ShitroidInCutscene_FlyOffScreen);
}

void ShitroidInCutscene_FlyOffScreen(uint16 k) {  // 0xA9CB56
  Rect16U rect = { 272, 64, 4, 4 };
  Shitroid_GraduallyAccelerateTowards0x10(k, 0, rect.x, rect.y);
  if (!Shitroid_Func_2(k, rect))
    Get_ShitroidInCutscene(k)->sice_var_A = FUNC16(ShitroidInCutscene_MoveToFinalChargeStart);
}

void ShitroidInCutscene_MoveToFinalChargeStart(uint16 k) {  // 0xA9CB7B
  Rect16U rect = { 305, 160, 4, 4 };
  Shitroid_GraduallyAccelerateTowards0x10(k, 0, rect.x, rect.y);
  if (!Shitroid_Func_2(k, rect)) {
    Enemy_ShitroidInCutscene *E = Get_ShitdroidInCutscene(k);
    E->base.health = 79;
    Get_ShitdroidInCutscene(0x40)->sice_var_0B = 0;
    Get_ShitdroidInCutscene(0)->sice_var_A = FUNC16(MotherBrain_Phase2_ExecuteFinalkShitroid);
    E->sice_var_A = FUNC16(ShitroidInCutscene_InitiateFinalCharge);
  }
}

void ShitroidInCutscene_InitiateFinalCharge(uint16 k) {  // 0xA9CBB3
  Rect16U rect = { 290, 128, 4, 4 };
  Shitroid_Func_GraduallyAccelerateTowards0x400(k, 0xA, rect.x, rect.y);
  if (!Shitroid_Func_2(k, rect))
    Get_ShitroidInCutscene(k)->sice_var_A = FUNC16(ShitroidInCutscene_FinalCharge);
}

void ShitroidInCutscene_FinalCharge(uint16 k) {  // 0xA9CBD8
  Enemy_ShitroidInCutscene *E1 = Get_ShitdroidInCutscene(0x40);
  Shitroid_Func_GraduallyAccelerateTowards0x400(k, 0xC, E1->base.x_pos, E1->base.y_pos - 32);
  Enemy_ShitroidInCutscene *E = Get_ShitdroidInCutscene(k);
  if (!E->base.health) {
    E->sice_var_0F = FUNC16(nullsub_338);
    E->base.vram_tiles_index = 4256;
    QueueSfx3_Max6(0x19);
    TurnOffLightsForShitroidDeath();
    Enemy_SetInstrList(k, addr_kShitroid_Ilist_CFCE);
    E->sice_var_B = 0;
    E->sice_var_C = 0;
    Get_ShitdroidInCutscene(0)->sice_var_A = addr_locret_A9C18D;
    QueueMusic_Delayed8(0);
    E->sice_var_A = FUNC16(ShitroidInCutscene_ShitroidFinalBelow);
    E->sice_var_F = 16;
    E->sice_var_10 = E->base.x_pos;
    E->sice_var_11 = E->base.y_pos;
    ShitroidInCutscene_ShitroidFinalBelow(k);
  }
}

void ShitroidInCutscene_ShitroidFinalBelow(uint16 k) {  // 0xA9CC3E
  ShitroidInCutscene_Shake(k);
  Enemy_ShitroidInCutscene *E = Get_ShitdroidInCutscene(k);
  if ((--E->sice_var_F & 0x8000) != 0) {
    E->base.x_pos = E->sice_var_10;
    E->base.y_pos = E->sice_var_11;
    E->sice_var_A = FUNC16(ShitroidInCutscene_PlaySamusTheme);
    E->sice_var_F = 56;
    ShitroidInCutscene_PlaySamusTheme(k);
  }
}

void ShitroidInCutscene_PlaySamusTheme(uint16 k) {  // 0xA9CC60
  Enemy_ShitroidInCutscene *E = Get_ShitroidInCutscene(k);
  if ((--E->sice_var_F & 0x8000) != 0) {
    QueueMusic_Delayed8(0xFF48);
    QueueMusic_Delayed8(5);
    E->sice_var_A = FUNC16(ShitroidInCutscene_PrepareSamusHyperbeam);
    E->sice_var_F = 12;
    ShitroidInCutscene_PrepareSamusHyperbeam(k);
  }
}

void ShitroidInCutscene_PrepareSamusHyperbeam(uint16 k) {  // 0xA9CC7F
  Enemy_ShitroidInCutscene *E = Get_ShitdroidInCutscene(k);
  if ((--E->sice_var_F & 0x8000) != 0) {
    CallSomeSamusCode(0x19);
    Get_ShitdroidInCutscene(0x40)->sice_var_16 = FUNC16(SamusRainbowPaletteFunc_ActivateWhenEnemyLow);
    E->sice_var_A = FUNC16(ShitroidInCutscene_DeathSequence);
  }
}

void ShitroidInCutscene_DeathSequence(uint16 k) {  // 0xA9CC99
  HandleSamusRainbowPaletteAnimation(k);
  ShitroidInCutscene_AccelerateDownwards(k);
  if (ShitroidInCutscene_FadeShitroidToBlack(k) & 1) {
    Enemy_ShitroidInCutscene *E = Get_ShitroidInCutscene(k);
    E->base.properties |= kEnemyProps_Invisible;
    E->sice_var_A = FUNC16(ShitroidInCutscene_UnloadShitroid);
    E->sice_var_F = 128;
  } else {
    ShitroidInCutscene_HandleShitroidDeathExplosions(k);
    ShitroidInCutscene_HandleEnemyBlinking(k);
  }
}

void ShitroidInCutscene_UnloadShitroid(uint16 k) {  // 0xA9CCC0
  HandleSamusRainbowPaletteAnimation(k);
  Enemy_ShitroidInCutscene *E = Get_ShitroidInCutscene(k);
  bool v2 = (--E->sice_var_F & 0x8000) != 0;
  if (v2 && ProcessSpriteTilesTransfers(0xa9, addr_stru_A98FC7)) {
    E->sice_var_A = FUNC16(ShitroidInCutscene_LetSamusRainbowMore);
    E->sice_var_F = 176;
    ShitroidInCutscene_LetSamusRainbowMore(k);
  }
}

void ShitroidInCutscene_LetSamusRainbowMore(uint16 k) {  // 0xA9CCDE
  Enemy_ShitroidInCutscene *E = Get_ShitdroidInCutscene(k);
  if ((--E->sice_var_F & 0x8000) != 0) {
    E->sice_var_A = FUNC16(ShitroidInCutscene_FinishCutscene);
    Get_ShitdroidInCutscene(0)->sice_var_37 = 0;
    ShitroidInCutscene_FinishCutscene(k);
  }
}

void ShitroidInCutscene_FinishCutscene(uint16 k) {  // 0xA9CCF0
  Enemy_ShitroidInCutscene *E = Get_ShitdroidInCutscene(0);
  uint16 v2 = E->sice_var_37 + 1;
  E->sice_var_37 = v2;
  if (MotherBrain_Phase3_TurnLightsBackOn(v2 - 1) & 1) {
    E->sice_var_A = FUNC16(MotherBrain_Phase3_Recover_MakeDistance);
    CallSomeSamusCode(0x17);
    SomeMotherBrainScripts(3);
    Get_ShitdroidInCutscene(k)->base.properties |= kEnemyProps_Deleted;
    Get_ShitdroidInCutscene(0x40)->sice_var_0A = 0;
  }
}

void CallSamusRainbowPaletteFunc(uint32 ea, uint16 k) {
  switch (ea) {
  case fnSamusRainbowPaletteFunc_ActivateWhenEnemyLow: SamusRainbowPaletteFunc_ActivateWhenEnemyLow(k); return;
  case fnSamusRainbowPaletteFunc_GraduallySlowDown: SamusRainbowPaletteFunc_GraduallySlowDown(); return;
  default: Unreachable();
  }
}

void HandleSamusRainbowPaletteAnimation(uint16 k) {  // 0xA9CD27
  uint16 r18 = Get_ShitdroidInCutscene(0x40)->sice_var_16;
  CallSamusRainbowPaletteFunc(r18 | 0xA90000, k);
}

void SamusRainbowPaletteFunc_ActivateWhenEnemyLow(uint16 k) {  // 0xA9CD30
  if ((int16)(Get_ShitdroidInCutscene(k)->base.y_pos + 16 - samus_y_pos) >= 0) {
    CallSomeSamusCode(0x16);
    Get_ShitdroidInCutscene(0x40)->sice_var_16 = FUNC16(SamusRainbowPaletteFunc_GraduallySlowDown);
  }
}

void SamusRainbowPaletteFunc_GraduallySlowDown(void) {  // 0xA9CD4B
  Enemy_ShitroidInCutscene *E = Get_ShitdroidInCutscene(0x40);
  uint16 sice_var_1D = E->sice_var_1D;
  E->sice_var_1D = sice_var_1D + 768;
  if (sice_var_1D >= 0xFD00) {
    uint16 v2 = special_samus_palette_frame + 1;
    if (!sign16(special_samus_palette_frame - 9))
      v2 = 10;
    special_samus_palette_frame = v2;
  }
}

uint8 ShitroidInCutscene_FadeShitroidToBlack(uint16 k) {  // 0xA9CD69
  int16 v2;

  Enemy_ShitroidInCutscene *E = Get_ShitdroidInCutscene(k);
  if (sign16(E->base.y_pos - 128))
    return 0;
  v2 = E->sice_var_0C - 1;
  if (v2 >= 0) {
    E->sice_var_0C = v2;
    return 0;
  }
  E->sice_var_0C = 8;
  uint16 v4 = E->sice_var_0D + 1;
  if (!sign16(E->sice_var_0D - 6))
    return 1;
  E->sice_var_0D = v4;
  WriteColorsToPalette(0x1E2, 0xad, kShitroid_FadingToBlack[v4], 0xE);
  return 0;
}

void ShitroidInCutscene_HandleShitroidDeathExplosions(uint16 k) {  // 0xA9CDB1
  int16 v2;

  Enemy_ShitroidInCutscene *E = Get_ShitdroidInCutscene(k);
  v2 = E->sice_var_08 - 1;
  if (v2 < 0) {
    E->sice_var_08 = 4;
    uint16 v3 = E->sice_var_07 + 1;
    if (!sign16(E->sice_var_07 - 9))
      v3 = 0;
    E->sice_var_07 = v3;
    int v4 = (uint16)(4 * v3) >> 1;
    eproj_spawn_pt = (Point16U){ E->base.x_pos + g_word_A9CDFC[v4], E->base.y_pos + g_word_A9CDFC[v4 + 1] };
    SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 3);
    QueueSfx3_Max3(0x13);
  } else {
    E->sice_var_08 = v2;
  }
}

void ShitroidInCutscene_HandleEnemyBlinking(uint16 k) {  // 0xA9CE24
  Enemy_ShitroidInCutscene *E = Get_ShitroidInCutscene(k);
  if ((E->base.frame_counter & 1) != 0)
    E->base.properties &= ~kEnemyProps_Invisible;
  else
    E->base.properties |= kEnemyProps_Invisible;
}

void ShitroidInCutscene_AccelerateDownwards(uint16 k) {  // 0xA9CE40
  int16 v2;

  Enemy_ShitroidInCutscene *E = Get_ShitroidInCutscene(k);
  v2 = abs16(E->sice_var_B) - 32;
  if (v2 < 0)
    v2 = 0;
  E->sice_var_B = sign16(E->sice_var_B) ? -v2 : v2;
  E->sice_var_C += 2;
}

void ShitroidInCutscene_HandleHealthBasedPalette(uint16 k) {  // 0xA9CE69
  Enemy_ShitroidInCutscene *E = Get_ShitdroidInCutscene(k);
  if (E->sice_var_09) {
    uint16 health = E->base.health;
    if (sign16(health - 2560)) {
      uint16 v3 = 2;
      if (sign16(health - 2240)) {
        v3 = 4;
        if (sign16(health - 1920)) {
          v3 = 6;
          if (sign16(health - 1600)) {
            v3 = 8;
            if (sign16(health - 1280)) {
              v3 = 10;
              if (sign16(health - 960)) {
                v3 = 12;
                if (sign16(health - 640))
                  v3 = 14;
              }
            }
          }
        }
      }
      uint16 v5 = v3;
      uint16 v4 = kShitroid_HealthBasedPalettes_Shell[v3 >> 1];
      WriteColorsToPalette(0x1E2, 0xad, v4, 4);
      WriteColorsToPalette(0x1F4, 0xad, v4 + 8, 5);
      WriteColorsToPalette(0x1EA, 0xad, kShitroid_HealthBasedPalettes_Innards[v5 >> 1], 5);
    }
  }
}

void ShitroidInCutscene_Shake(uint16 k) {  // 0xA9CEDB
  Enemy_ShitroidInCutscene *E = Get_ShitdroidInCutscene(k);
  --E->sice_var_C;
  int v2 = (E->base.frame_counter & 6) >> 1;
  E->base.x_pos = E->sice_var_10 + g_word_A993BB[v2];
  E->base.y_pos = E->sice_var_11 + g_word_A993C3[v2];
}

void ShitroidInCutscene_Touch(void) {  // 0xA9CF03
  Enemy_ShitroidInCutscene *E = Get_ShitroidInCutscene(cur_enemy_index);
  if (E->sice_var_A == FUNC16(ShitroidInCutscene_LatchOntoSamus)) {
    if (Shitroid_AccelerateTowardsPoint(cur_enemy_index, 0x10, samus_x_pos, samus_y_pos - 20) & 1) {
      E->sice_var_B = 0;
      E->sice_var_C = 0;
      E->sice_var_A = FUNC16(ShitroidInCutscene_HealSamusToFullHealth);
    }
  }
}

void ShitroidInCutscene_UpdateSpeedAndAngle(uint16 k, uint16 r18, uint16 r20, uint16 r22) {  // 0xA9CF31
  Enemy_ShitroidInCutscene *E = Get_ShitdroidInCutscene(k);
  if (r22 != E->sice_var_0B) {
    if ((int16)(r22 - E->sice_var_0B) >= 0) {
      uint16 v3 = E->sice_var_0B + 32;
      if (!sign16(v3 - r22))
        v3 = r22;
      E->sice_var_0B = v3;
    } else {
      uint16 v2 = E->sice_var_0B - 32;
      if (sign16(v2 - r22))
        v2 = r22;
      E->sice_var_0B = v2;
    }
  }
  uint16 v4;
  if ((r18 & 0x8000) != 0) {
    v4 = E->sice_var_0A + r18;
    if (sign16(v4 - r20))
      LABEL_13:
    v4 = r20;
  } else {
    v4 = E->sice_var_0A + r18;
    if (!sign16(v4 - r20))
      goto LABEL_13;
  }
  E->sice_var_0A = v4;
  r18 = HIBYTE(v4);
  E->sice_var_B = Math_MultBySin(E->sice_var_0B, r18);
  E->sice_var_C = Math_MultByCos(E->sice_var_0B, r18);
}

const uint16 *Shitroid_Instr_1(uint16 k, const uint16 *jp) {  // 0xA9CFB4
  return INSTR_RETURN_ADDR(addr_kShitroid_Ilist_CFA2);
}

const uint16 *Shitroid_Instr_2(uint16 k, const uint16 *jp) {  // 0xA9CFCA
  return INSTR_RETURN_ADDR(addr_kShitroid_Ilist_CFB8);
}

uint8 Shitroid_Func_1(uint16 k, uint16 j) {  // 0xA9EED1
  Enemy_Shitroid *E = Get_Shitroid(j);
  Enemy_Shitroid *G = Get_Shitroid(k);
  uint16 r18 = G->base.x_width + E->base.x_width + 1;
  uint16 v4 = abs16(E->base.x_pos - G->base.x_pos);
  bool v5 = v4 >= r18;
  if (v4 < r18) {
    r18 = G->base.y_height + E->base.y_height + 1;
    return abs16(E->base.y_pos - G->base.y_pos) >= r18;
  }
  return v5;
}

uint8 Shitroid_Func_2(uint16 k, Rect16U rect) {  // 0xA9EF06
  Enemy_Shitroid *E = Get_Shitroid(k);
  uint16 R26 = E->base.x_width + rect.w + 1;
  uint16 v2 = abs16(rect.x - E->base.x_pos);
  bool v3 = v2 >= R26;
  if (v2 < R26) {
    R26 = E->base.y_height + rect.h + 1;
    return abs16(rect.y - E->base.y_pos) >= R26;
  }
  return v3;
}

void Shitroid_Init(void) {  // 0xA9EF37
  for (int i = 4094; i >= 0; i -= 2)
    tilemap_stuff[i >> 1] = 0;
  Enemy_Shitroid *E = Get_Shitroid(cur_enemy_index);
  E->base.properties |= kEnemyProps_ProcessInstructions | kEnemyProps_BlockPlasmaBeam;
  E->base.palette_index = 1024;
  E->base.current_instruction = addr_kShitroid_Ilist_F90E;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  uint16 v2 = FUNC16(Shitroid_Func_4);
  if ((layer1_x_pos & 0x8000) != 0) {
    E->base.properties |= kEnemyProps_Intangible | kEnemyProps_Invisible;
    v2 = FUNC16(Shitroid_Func_3);
  }
  E->shitr_var_A = v2;
  E->shitr_var_B = 0;
  E->shitr_var_C = 0;
  E->shitr_var_E = 10;
  E->shitr_parameter_2 = 0;
  WriteColorsToTargetPalette(0xa9, 0x120, addr_word_A9F8C6, 0x10);
  WriteColorsToTargetPalette(0xa9, 0x140, addr_kShitroidInCutscene_Palette, 0x10);
  WriteColorsToTargetPalette(0xa9, 0x1E0, addr_kDeadSidehopper_Palette_0, 0x10);
}

void Shitroid_Powerbomb(void) {  // 0xA9EFBA
  if (Get_Shitroid(cur_enemy_index)->shitr_parameter_2)
    Shitroid_Func_26(cur_enemy_index);
  Shitroid_Main();
}

void CallShitroidFunc(uint32 ea, uint16 k) {
  switch (ea) {
  case fnShitroid_Func_3: Shitroid_Func_3(k); return;
  case fnShitroid_Func_4: Shitroid_Func_4(); return;
  case fnShitroid_Func_5: Shitroid_Func_5(k); return;
  case fnShitroid_Func_6: Shitroid_Func_6(k); return;
  case fnShitroid_Func_7: Shitroid_Func_7(k); return;
  case fnShitroid_Func_8: Shitroid_Func_8(k); return;
  case fnShitroid_Func_9: Shitroid_Func_9(k); return;
  case fnShitroid_Func_10: Shitroid_Func_10(k); return;
  case fnShitroid_Func_11: Shitroid_Func_11(k); return;
  case fnShitroid_Func_12: Shitroid_Func_12(k); return;
  case fnShitroid_Func_13: Shitroid_Func_13(k); return;
  case fnShitroid_Func_14: Shitroid_Func_14(k); return;
  case fnShitroid_Func_15: Shitroid_Func_15(k); return;
  case fnShitroid_Func_16: Shitroid_Func_16(k); return;
  case fnShitroid_Func_17: Shitroid_Func_17(k); return;
  case fnShitroid_Func_18: Shitroid_Func_18(k); return;
  case fnShitroid_Func_19: Shitroid_Func_19(k); return;
  case fnShitroid_Func_20: Shitroid_Func_20(); return;
  case fnShitroid_Func_21: Shitroid_Func_21(); return;
  case fnShitroid_Func_22: Shitroid_Func_22(k); return;
  case fnShitroid_Func_23: Shitroid_Func_23(); return;
  case fnShitroid_Func_24: Shitroid_Func_24(k); return;
  case fnShitroid_Func_25: Shitroid_Func_25(k); return;
  default: Unreachable();
  }
}

void Shitroid_Main(void) {  // 0xA9EFC5
  Enemy_Shitroid *E = Get_Shitroid(cur_enemy_index);
  E->base.health = 0x7FFF;
  CallShitroidFunc(E->shitr_var_A | 0xA90000, cur_enemy_index);
  MoveEnemyWithVelocity();
  if (!palette_change_num)
    Shitroid_HandleNormalPalette();
}

void Shitroid_Func_3(uint16 k) {  // 0xA9EFDF
  Enemy_Shitroid *E = Get_Shitroid(k);
  E->shitr_var_B = 0;
  E->shitr_var_C = 0;
}

void Shitroid_Func_4(void) {  // 0xA9EFE6
  if (sign16(layer1_x_pos - 513)) {
    layer1_x_pos = 512;
    *(uint16 *)scrolls = scrolls[0];
    *(uint16 *)&scrolls[2] = scrolls[2];
    SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x30, 0x03, 0xb767 });
    SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x1f, 0x03, 0xb767 });
    Enemy_Shitroid *E = Get_Shitroid(cur_enemy_index);
    E->shitr_var_A = FUNC16(Shitroid_Func_5);
    E->shitr_var_04 = 1;
  }
}

void Shitroid_Func_5(uint16 k) {  // 0xA9F02B
  Enemy_Shitroid *E = Get_Shitroid(k);
  E->shitr_var_A = FUNC16(Shitroid_Func_6);
  E->shitr_var_F = 464;
  Shitroid_Func_6(k);
}

void Shitroid_Func_6(uint16 k) {  // 0xA9F037
  Enemy_Shitroid *E = Get_Shitroid(k);
  if ((--E->shitr_var_F & 0x8000) != 0) {
    QueueMusic_Delayed8(5);
    E->shitr_var_A = FUNC16(Shitroid_Func_7);
    Shitroid_Func_7(k);
  }
}

void Shitroid_Func_7(uint16 k) {  // 0xA9F049
  Rect16U rect = { 584, 74, 1, 1 };
  Shitroid_Func_GraduallyAccelerateTowards0x400(k, 0xF, rect.x, rect.y);
  if (!Shitroid_Func_2(k, rect))
    Get_Shitroid(k)->shitr_var_A = FUNC16(Shitroid_Func_8);
}

void Shitroid_Func_8(uint16 k) {  // 0xA9F06D
  Enemy_Shitroid *E = Get_Shitroid(k + 64);
  Shitroid_Func_GraduallyAccelerateTowards0x400(k, 0xF, E->base.x_pos, E->base.y_pos - 32);
  if (!(Shitroid_Func_1(k, k + 64) & 1))
    Get_Shitroid(k)->shitr_var_A = FUNC16(Shitroid_Func_9);
}

void Shitroid_Func_9(uint16 k) {  // 0xA9F094
  Enemy_Shitroid *E1 = Get_Shitroid(k + 64);
  if (Shitroid_AccelerateTowardsPoint(k, 0x200, E1->base.x_pos, E1->base.y_pos - 32) & 1) {
    Enemy_Shitroid *E = Get_Shitroid(k);
    E->shitr_var_B = 0;
    E->shitr_var_C = 0;
    E->base.x_pos = E1->base.x_pos;
    E->base.y_pos = E1->base.y_pos - 32;
    E->base.current_instruction = addr_kShitroid_Ilist_F924;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
    E->shitr_var_A = FUNC16(Shitroid_Func_10);
    E->shitr_var_E = 1;
    E->shitr_parameter_2 = 0;
    E->shitr_var_F = 320;
  }
}

void Shitroid_Func_10(uint16 k) {  // 0xA9F0E6
  Enemy_Shitroid *E = Get_Shitroid(k);
  int v2 = (E->base.frame_counter & 6) >> 1;
  Enemy_Shitroid *E1 = Get_Shitroid(k + 64);
  E->base.x_pos = E1->base.x_pos + g_word_A993BB[v2];
  E->base.y_pos = E1->base.y_pos + g_word_A993C3[v2] - 32;
  if (E->shitr_var_F-- == 1) {
    E->shitr_var_A = FUNC16(Shitroid_Func_11);
    E->base.current_instruction = addr_kShitroid_Ilist_F906;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
    E->shitr_var_E = 10;
  }
}

void Shitroid_Func_11(uint16 k) {  // 0xA9F125
  Get_Shitroid(0x40)->shitr_var_08 = 1;
  Enemy_Shitroid *E = Get_Shitroid(k);
  E->shitr_var_A = FUNC16(Shitroid_Func_12);
  E->shitr_var_F = 192;
  Shitroid_Func_12(k);
}

void Shitroid_Func_12(uint16 k) {  // 0xA9F138
  Enemy_Shitroid *E = Get_Shitroid(k);
  Shitroid_Func_GraduallyAccelerateTowards0x400(k, 0, E->base.x_pos, 104);
  if ((--E->shitr_var_F & 0x8000) != 0) {
    E->shitr_var_A = FUNC16(Shitroid_Func_13);
    E->shitr_parameter_2 = 1;
    *(uint16 *)scrolls |= 0x100;
    *(uint16 *)&scrolls[2] |= 0x100;
    SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x30, 0x03, 0xb763 });
    SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x1f, 0x03, 0xb763 });
  }
}

void Shitroid_Func_13(uint16 k) {  // 0xA9F180
  Enemy_Shitroid *E = Get_Shitroid(k);
  uint16 v2 = abs16(E->base.x_pos - samus_x_pos);
  uint16 shitro_var_02;
  if (v2 >= 8) {
    shitro_var_02 = E->shitr_var_02;
    if (shitro_var_02) {
      if ((--shitro_var_02 & 0x8000) != 0)
        shitro_var_02 = 0;
    }
  } else {
    shitro_var_02 = (v2 >= 8) + E->shitr_var_02 + 2;
  }
  E->shitr_var_02 = shitro_var_02;
  if (shitro_var_02 >= 0x100 || sign16(samus_x_pos - 512)) {
    Get_Shitroid(cur_enemy_index)->shitr_var_A = FUNC16(Shitroid_Func_14);
  } else {
    uint16 shitro_var_01 = E->shitr_var_01;
    uint16 v5;
    if (shitro_var_01) {
      E->shitr_var_01 = shitro_var_01 - 1;
      v5 = samus_y_pos;
    } else {
      v5 = 80;
      if ((random_number & 0xFFF) >= 0xFE0)
        E->shitr_var_01 = 32;
    }
    Shitroid_Func_GraduallyAccelerateTowards0x400(cur_enemy_index, 0xA, samus_x_pos, v5);
  }
}

void Shitroid_Func_14(uint16 k) {  // 0xA9F1FA
  Shitroid_Func_GraduallyAccelerateTowards0x400(k, 0xF, samus_x_pos, samus_y_pos - 32);
}

void Shitroid_Func_15(uint16 k) {  // 0xA9F20E
  CallSomeSamusCode(0x12);
  Get_Shitroid(0)->shitr_var_A = FUNC16(Shitroid_Func_16);
  Shitroid_Func_16(k);
}

void Shitroid_Func_16(uint16 k) {  // 0xA9F21B
  Enemy_Shitroid *E = Get_Shitroid(k);
  if (samus_health < 2) {
    samus_x_speed_divisor = 0;
    bomb_counter = 0;
    E->shitr_var_A = FUNC16(Shitroid_Func_17);
    E->shitr_var_B = 0;
    E->shitr_var_C = 0;
    Enemy_SetInstrList(k, addr_kShitroid_Ilist_F906);
    E->shitr_var_E = 10;
    CallSomeSamusCode(0x13);
    SomeMotherBrainScripts(0);
    E->shitr_var_04 = 0;
    QueueMusic_Delayed8(7);
  } else {
    cooldown_timer = 8;
    bomb_counter = 5;
    samus_x_speed_divisor = 2;
    if (!sign16(samus_y_speed - 4))
      samus_y_speed = 2;
    int v2 = (E->base.frame_counter & 6) >> 1;
    E->base.x_pos = samus_x_pos + g_word_A993BB[v2];
    E->base.y_pos = samus_y_pos + g_word_A993C3[v2] - 20;
    Samus_DamageDueToShitroid();
  }
}

void Shitroid_Func_17(uint16 k) {  // 0xA9F2A2
  Enemy_Shitroid *E = Get_Shitroid(k);
  E->shitr_var_A = FUNC16(Shitroid_Func_18);
  E->shitr_var_F = 120;
  Shitroid_Func_18(k);
}

void Shitroid_Func_18(uint16 k) {  // 0xA9F2AE
  Enemy_Shitroid *E = Get_Shitroid(k);
  if ((--E->shitr_var_F & 0x8000) != 0) {
    E->shitr_var_A = FUNC16(Shitroid_Func_19);
    E->shitr_var_F = 192;
    Shitroid_Func_19(k);
  }
}

void Shitroid_Func_19(uint16 k) {  // 0xA9F2C0
  Shitroid_Func_GraduallyAccelerateTowards0x400(cur_enemy_index, 0, samus_x_pos, 104);
  Enemy_Shitroid *E = Get_Shitroid(cur_enemy_index);
  if ((--E->shitr_var_F & 0x8000) != 0) {
    QueueSfx2_Max6(0x7D);
    E->shitr_var_A = FUNC16(Shitroid_Func_20);
    E->shitr_var_F = 88;
    E->base.current_instruction = addr_kShitroid_Ilist_F924;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
    Shitroid_Func_20();
  }
}

void Shitroid_Func_20(void) {  // 0xA9F2FB
  Shitroid_Func_GraduallyAccelerateTowards0x400(cur_enemy_index, 0, samus_x_pos - 64, 100);
  Enemy_Shitroid *E = Get_Shitroid(cur_enemy_index);
  if ((--E->shitr_var_F & 0x8000) != 0) {
    E->shitr_var_A = FUNC16(Shitroid_Func_21);
    E->shitr_var_F = 88;
    Shitroid_Func_21();
  }
}

void Shitroid_Func_21(void) {  // 0xA9F324
  Shitroid_Func_GraduallyAccelerateTowards0x400(cur_enemy_index, 0, samus_x_pos + 96, 104);
  Enemy_Shitroid *E = Get_Shitroid(cur_enemy_index);
  if ((--E->shitr_var_F & 0x8000) != 0) {
    E->shitr_var_A = FUNC16(Shitroid_Func_24);
    E->shitr_var_F = 256;
    E->base.current_instruction = addr_kShitroid_Ilist_F93A;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
  }
}

void Shitroid_Func_22(uint16 k) {  // 0xA9F360
  QueueSfx2_Max6(0x52);
  Get_Shitroid(k)->shitr_var_A = FUNC16(Shitroid_Func_23);
  Shitroid_Func_23();
}

void Shitroid_Func_23(void) {  // 0xA9F36D
  Rect16U rect = { -128, 64, 8, 8 };
  Shitroid_Func_GraduallyAccelerateTowards0x400(cur_enemy_index, 0, rect.x, rect.y);
  if (!Shitroid_Func_2(cur_enemy_index, rect)) {
    Enemy_Shitroid *E = Get_Shitroid(cur_enemy_index);
    E->shitr_var_B = 0;
    E->shitr_var_C = 0;
    E->base.properties &= ~(kEnemyProps_ProcessInstructions | kEnemyProps_Invisible);
    E->shitr_var_A = FUNC16(Shitroid_Func_3);
  }
}

void Shitroid_Func_24(uint16 k) {  // 0xA9F3A3
  Enemy_Shitroid *E = Get_Shitroid(k);
  if ((--E->shitr_var_F & 0x8000) != 0) {
    SomeMotherBrainScripts(2);
    E->shitr_parameter_2 = 1;
    E->shitr_var_A = FUNC16(Shitroid_Func_25);
    Shitroid_Func_25(k);
  } else {
    Shitroid_Func_27(k);
  }
}

void Shitroid_Func_25(uint16 k) {  // 0xA9F3BE
  if (Shitroid_Func_27(k) & 1)
    Shitroid_F3C4(k);
}

void Shitroid_F3C4(uint16 k) {  // 0xA9F3C4
  Get_Shitroid(k)->shitr_var_A = FUNC16(Shitroid_Func_22);
}

void Shitroid_Func_26(uint16 k) {  // 0xA9F3CB
  if (Get_Shitroid(k)->shitr_var_A == FUNC16(Shitroid_Func_25))
    Shitroid_F3C4(k);
}

uint8 Shitroid_Func_27(uint16 k) {  // 0xA9F3D4
  Enemy_Shitroid *E = Get_Shitroid(k);
  uint16 v2 = abs16(E->base.x_pos - samus_x_pos);
  uint16 shitro_var_02;
  if (v2 >= 2) {
    shitro_var_02 = E->shitr_var_02;
    if (shitro_var_02) {
      if ((--shitro_var_02 & 0x8000) != 0)
        shitro_var_02 = 0;
    }
  } else {
    shitro_var_02 = (v2 >= 2) + E->shitr_var_02 + 2;
  }
  E->shitr_var_02 = shitro_var_02;
  uint16 shitro_var_01 = E->shitr_var_01;
  uint16 v5;
  if (shitro_var_01) {
    E->shitr_var_01 = shitro_var_01 - 1;
    v5 = samus_y_pos - 18;
  } else {
    v5 = 80;
    if ((random_number & 0xFFF) >= 0xFE0)
      E->shitr_var_01 = 32;
  }
  Shitroid_Func_GraduallyAccelerateTowards0x400(cur_enemy_index, 8, samus_x_pos, v5);
  return Get_Shitroid(cur_enemy_index)->shitr_var_02 >= 0x400 || sign16(samus_x_pos - 128);
}

void Shitroid_GraduallyAccelerateTowardsPt(uint16 k, uint16 j, uint16 r18, uint16 r20, uint16 r26) {  // 0xA9F46B
  int16 v3;
  int16 v5;
  int16 v6;
  int16 shitr_var_C;
  int16 v9;

  uint16 r24 = g_byte_A9F56A[j];
  Shitroid_GraduallyAccelerateHoriz(k, r18, r24, r26);
  Enemy_Shitroid *E = Get_Shitroid(k);
  v3 = E->base.y_pos - r20;
  if (v3) {
    if (v3 >= 0) {
      uint16 RegWord = SnesDivide(v3, r24);
      if (!RegWord)
        RegWord = 1;
      uint16 r22 = RegWord;
      shitr_var_C = E->shitr_var_C;
      if (shitr_var_C >= 0)
        shitr_var_C = shitr_var_C - 8 - r22;
      v9 = shitr_var_C - r22;
      if (sign16(v9 + 1280))
        v9 = -1280;
      E->shitr_var_C = v9;
    } else {
      uint16 v4 = SnesDivide(r20 - E->base.y_pos, r24);
      if (!v4)
        v4 = 1;
      uint16 r22 = v4;
      v5 = E->shitr_var_C;
      if (v5 < 0)
        v5 += r22 + 8;
      v6 = r22 + v5;
      if (!sign16(v6 - 1280))
        v6 = 1280;
      E->shitr_var_C = v6;
    }
  }
}

void Shitroid_Func_GraduallyAccelerateTowards0x400(uint16 k, uint16 j, uint16 r18, uint16 r20) {  // 0xA9F451
  Shitroid_GraduallyAccelerateTowardsPt(k, j, r18, r20, 1024);
}

void Shitroid_GraduallyAccelerateTowards0x4(uint16 k, uint16 j, uint16 r18, uint16 r20) {  // 0xA9F458
  Shitroid_GraduallyAccelerateTowardsPt(k, j, r18, r20, 4);
}

void Shitroid_GraduallyAccelerateTowards0x8(uint16 k, uint16 j, uint16 r18, uint16 r20) {  // 0xA9F45F
  Shitroid_GraduallyAccelerateTowardsPt(k, j, r18, r20, 8);
}

void Shitroid_GraduallyAccelerateTowards0x10(uint16 k, uint16 j, uint16 r18, uint16 r20) {  // 0xA9F466
  Shitroid_GraduallyAccelerateTowardsPt(k, j, r18, r20, 16);
}

void Shitroid_GraduallyAccelerateHoriz(uint16 k, uint16 r18, uint16 r24, uint16 r26) {  // 0xA9F4E6
  int16 v2;
  int16 v4;
  int8 v5; // cf
  int16 v7;
  int16 shitr_var_B;
  int16 v11;

  Enemy_Shitroid *E = Get_Shitroid(k);
  v2 = E->base.x_pos - r18;
  if (v2) {
    if (v2 >= 0) {
      uint16 RegWord = SnesDivide(v2,  r24);
      if (!RegWord)
        RegWord = 1;
      uint16 r22 = RegWord;
      shitr_var_B = E->shitr_var_B;
      if (shitr_var_B >= 0) {
        uint16 v13 = E->shitr_var_B;
        v5 = Shitroid_CheckIfOnScreen(k) & 1;
        uint16 v10 = v13;
        if (v5)
          v10 = v13 - r26;
        shitr_var_B = v10 - 8 - r22;
      }
      v11 = shitr_var_B - r22;
      if (sign16(v11 + 2048))
        v11 = -2048;
      E->shitr_var_B = v11;
    } else {
      uint16 v3 = SnesDivide(r18 - E->base.x_pos, r24);
      if (!v3)
        v3 = 1;
      uint16 r22 = v3;
      v4 = E->shitr_var_B;
      if (v4 < 0) {
        uint16 v12 = E->shitr_var_B;
        v5 = Shitroid_CheckIfOnScreen(k) & 1;
        uint16 v6 = v12;
        if (v5)
          v6 = r26 + v12;
        v4 = r22 + v6 + 8;
      }
      v7 = r22 + v4;
      if (!sign16(v7 - 2048))
        v7 = 2048;
      E->shitr_var_B = v7;
    }
  }
}


uint8 Shitroid_CheckIfOnScreen(uint16 k) {  // 0xA9F57A
  int16 y_pos;
  int16 v3;
  int16 x_pos;
  int16 v5;

  Enemy_Shitroid *E = Get_Shitroid(k);
  y_pos = E->base.y_pos;
  uint8 result = 1;
  if (y_pos >= 0) {
    v3 = y_pos + 96 - layer1_y_pos;
    if (v3 >= 0) {
      if (sign16(v3 - 416)) {
        x_pos = E->base.x_pos;
        if (x_pos >= 0) {
          v5 = x_pos + 16 - layer1_x_pos;
          if (v5 >= 0) {
            if (sign16(v5 - 288))
              return 0;
          }
        }
      }
    }
  }
  return result;
}

uint8 Shitroid_AccelerateTowardsPoint(uint16 k, uint16 a, uint16 r18, uint16 r20) {  // 0xA9F5A6
  uint16 R28 = 0;
  R28 += Shitroid_AccelerateTowardsX(k, r18, a);
  R28 += Shitroid_AccelerateTowardsY(k, r20, a);
  R28 >>= 1;
  return R28 & 1;
}

uint16 Shitroid_AccelerateTowardsY(uint16 k, uint16 r20, uint16 r22) {  // 0xA9F5B5
  Enemy_Shitroid *E = Get_Shitroid(k);
  uint16 result = 0;
  int16 v2 = E->base.y_pos - r20;
  if (!v2) {
    ++result;
  } else if (v2 >= 0) {
    uint16 v7 = E->shitr_var_C - r22;
    if (sign16(v7 + 1280))
      v7 = -1280;
    E->shitr_var_C = v7;
    if ((int16)(E->base.y_pos + (int8)(v7 >> 8) - r20) <= 0) {
      E->shitr_var_C = 0;
      ++result;
    }
  } else {
    uint16 v3 = r22 + E->shitr_var_C;
    if (!sign16(v3 - 1280))
      v3 = 1280;
    E->shitr_var_C = v3;
    if ((int16)(E->base.y_pos + (int8)(v3 >> 8) - r20) >= 0) {
      E->shitr_var_C = 0;
      ++result;
    }
  }
  return result;
}

uint16 Shitroid_AccelerateTowardsX(uint16 k, uint16 r18, uint16 r22) {  // 0xA9F615
  Enemy_Shitroid *E = Get_Shitroid(k);
  uint16 result = 0;
  if ((int16)(E->base.x_pos - r18) >= 0) {
    uint16 v6 = E->shitr_var_B - r22;
    if (sign16(v6 + 1280))
      v6 = -1280;
    E->shitr_var_B = v6;
    if ((int16)(E->base.x_pos + (int8)(v6 >> 8) - r18) <= 0) {
      E->shitr_var_B = 0;
      ++result;
    }
  } else {
    uint16 v2 = r22 + E->shitr_var_B;
    if (!sign16(v2 - 1280))
      v2 = 1280;
    E->shitr_var_B = v2;
    if ((int16)(E->base.x_pos + (int8)(v2 >> 8) - r18) >= 0) {
      E->shitr_var_B = 0;
      ++result;
    }
  }
  return result;
}

void Shitroid_HandleNormalPalette(void) {  // 0xA9F677
  Shitroid_HandleCutscenePalette_Common(addr_word_A9F6D1, 330);
}

void Shitroid_HandleCutscenePalette(void) {  // 0xA9F683
  Shitroid_HandleCutscenePalette_Common(addr_word_A9F6D1, 490);
}

void Shitroid_HandleCutscenePalette_LowHealth(void) {  // 0xA9F68F
  Shitroid_HandleCutscenePalette_Common(addr_word_A9F711, 490);
}

void Shitroid_HandleCutscenePalette_Common(uint16 r22, uint16 r18) {  // 0xA9F699
  int8 shitro_var_D_high;

  Enemy_Shitroid *E = Get_Shitroid(cur_enemy_index);
  shitro_var_D_high = HIBYTE(E->shitr_var_D);
  if (shitro_var_D_high) {
    HIBYTE(E->shitr_var_D) = shitro_var_D_high - 1;
  } else {
    HIBYTE(E->shitr_var_D) = E->shitr_var_E;
    uint8 v3 = (LOBYTE(E->shitr_var_D) + 1) & 7;
    LOBYTE(E->shitr_var_D) = v3;
    uint16 v4 = Shitroid_HandleCrySoundEffect(cur_enemy_index, v3);
    WriteColorsToPalette(r18, 0xa9, r22 + 8 * v4, 4);
  }
}

uint16 Shitroid_HandleCrySoundEffect(uint16 k, uint16 a) {  // 0xA9F751
  if (a == 5) {
    Enemy_Shitroid *E = Get_Shitroid(k);
    if (E->shitr_var_04) {
      uint16 v3 = E->shitr_var_05 + 1;
      E->shitr_var_05 = v3;
      if (v3 >= 4) {
        E->shitr_var_05 = 0;
        uint16 v4 = 114;
        if (E->shitr_var_E < 0xA)
          v4 = 120;
        QueueSfx2_Max6(v4);
      }
    }
    return 5;
  }
  return a;
}

void Shitroid_Touch(void) {  // 0xA9F789
  Enemy_Shitroid *E = Get_Shitroid(cur_enemy_index);
  if (E->shitr_parameter_2) {
    Shitroid_Func_26(cur_enemy_index);
    if (samus_movement_type == 3 && !sign16(samus_x_pos - 512)) {
      uint16 r18 = (uint8)(0x80 - CalculateAngleFromXY(samus_x_pos - E->base.x_pos, samus_y_pos - E->base.y_pos) + 0x80);
      E->shitr_var_B += Math_MultBySin(0x40, r18);
      E->shitr_var_C += Math_MultByCos(0x40, r18);
    } else if (E->shitr_var_A == FUNC16(Shitroid_Func_14)) {
      if (Shitroid_AccelerateTowardsPoint(cur_enemy_index, 0x200, samus_x_pos, samus_y_pos - 32) & 1) {
        E->base.current_instruction = addr_kShitroid_Ilist_F924;
        E->base.instruction_timer = 1;
        E->base.timer = 0;
        E->shitr_var_E = 1;
        E->shitr_parameter_2 = 0;
        E->shitr_var_B = 0;
        E->shitr_var_C = 0;
        E->shitr_var_A = FUNC16(Shitroid_Func_15);
      }
    } else if (E->shitr_var_A == FUNC16(Shitroid_Func_13)) {
      E->shitr_var_A = FUNC16(Shitroid_Func_14);
    }
  }
}

void Shitroid_Shot(void) {  // 0xA9F842
  Enemy_Shitroid *E = Get_Shitroid(cur_enemy_index);
  if (E->shitr_parameter_2) {
    Shitroid_Func_26(cur_enemy_index);
    uint16 r18 = (uint8)-CalculateAngleFromXY(projectile_x_pos[0] - E->base.x_pos, projectile_y_pos[0] - E->base.y_pos);
    uint16 v1 = 8 * projectile_damage[collision_detection_index];
    if (v1 >= 0xF0)
      v1 = 240;
    E->shitr_var_B += Math_MultBySin(v1, r18);
    E->shitr_var_C += Math_MultByCos(v1, r18);
  }
}

const uint16 *Shitroid_Instr_3(uint16 k, const uint16 *jp) {  // 0xA9F920
  return INSTR_RETURN_ADDR(addr_kShitroid_Ilist_F90E);
}

const uint16 *Shitroid_Instr_4(uint16 k, const uint16 *jp) {  // 0xA9F936
  return INSTR_RETURN_ADDR(addr_kShitroid_Ilist_F924);
}

const uint16 *Shitroid_Instr_6(uint16 k, const uint16 *jp) {  // 0xA9F990
  return INSTR_RETURN_ADDR(addr_kShitroid_Ilist_F93A);
}

const uint16 *Shitroid_Instr_5(uint16 k, const uint16 *jp) {  // 0xA9F994
  if ((random_number & 0x8000) == 0)
    return INSTR_RETURN_ADDR(jp[0]);
  QueueSfx2_Max6(0x52);
  return jp + 1;
}
