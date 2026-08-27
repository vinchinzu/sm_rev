// Ki-Hunter, Kago, Maridia, Wrecked Ship robot, n00b tube, and spore enemy-projectile families split out of sm_86.c.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "eproj_internal.h"

#define kEprojInit_N00bTubeShards_InstrPtrs ((uint16*)RomFixedPtr(0x86d760))
#define off_86D96A ((uint16*)RomFixedPtr(0x86d96a))
#define kSporeMovementData ((uint8*)RomFixedPtr(0x86dd6c))

void sub_86CFD5(uint16 k);
void sub_86CFE6(uint16 k);
void sub_86CFF8(uint16 k);
void sub_86D7BF(uint16 k);
void sub_86D7DE(uint16 k);
void sub_86D83D(uint16 k);
void sub_86D89F(uint16 k);
void sub_86D8DF(uint16 k);
void sub_86D992(uint16 k);
void sub_86DCC3(uint16 j);

static void EprojInit_KiHunterAcidSpitCommon(uint16 k, uint16 j) {  // 0x86CFBA
  int v2 = j >> 1;
  eproj_y_vel[v2] = 0;
  eproj_y_pos[v2] = gEnemyData(k)->y_pos - 16;
  eproj_y_subpos[v2] = 0;
  eproj_x_subpos[v2] = 0;
}

void EprojInit_KiHunterAcidSpitLeft(uint16 j) {  // 0x86CF90
  int v2 = j >> 1;
  eproj_x_vel[v2] = -768;
  eproj_x_pos[v2] = gEnemyData(cur_enemy_index)->x_pos - 22;
  EprojInit_KiHunterAcidSpitCommon(cur_enemy_index, j);
}

void EprojInit_KiHunterAcidSpitRight(uint16 j) {  // 0x86CFA6
  int v2 = j >> 1;
  eproj_x_vel[v2] = 768;
  eproj_x_pos[v2] = gEnemyData(cur_enemy_index)->x_pos + 22;
  EprojInit_KiHunterAcidSpitCommon(cur_enemy_index, j);
}

void sub_86CFD5(uint16 k) {  // 0x86CFD5
  int v1 = k >> 1;
  eproj_pre_instr[v1] = FUNC16(sub_86CFF8);
  eproj_x_pos[v1] -= 19;
}

void sub_86CFE6(uint16 k) {  // 0x86CFE6
  int v1 = k >> 1;
  eproj_pre_instr[v1] = FUNC16(sub_86CFF8);
  eproj_x_pos[v1] += 19;
}

void sub_86CFF8(uint16 k) {  // 0x86CFF8
  int v1 = k >> 1;
  if (EprojBlockCollisition_Vertical(k) & 1) {
    eproj_instr_list_ptr[v1] = addr_off_86CF56;
    eproj_instr_timers[v1] = 1;
  } else if (EprojBlockCollisition_Horiz(k) & 1) {
    eproj_x_vel[k >> 1] = 0;
  } else {
    uint16 v2 = eproj_y_vel[v1] + 16;
    eproj_y_vel[v1] = v2;
    if (!sign16(v2 - 512))
      v2 = 512;
    eproj_y_vel[v1] = v2;
  }
}

void EprojInit_KagosBugs(uint16 j) {  // 0x86D088
  int v2 = j >> 1;
  eproj_F[v2] = cur_enemy_index;
  EnemyData *v3 = gEnemyData(cur_enemy_index);
  eproj_x_pos[v2] = v3->x_pos;
  eproj_y_pos[v2] = v3->y_pos;
  uint16 v4 = (random_number & 7) + 1;
  eproj_G[v2] = v4;
  eproj_E[v2] = v4 + 4;
  eproj_pre_instr[v2] = FUNC16(EprojPreInstr_KagosBugs);
}

void EprojPreInstr_KagosBugs_Func1(uint16 k) {  // 0x86D0B3
  int v1 = k >> 1;
  uint16 v2 = eproj_E[v1];
  if (v2) {
    uint16 v3 = v2 - 1;
    eproj_E[v1] = v3;
    if (!v3)
      QueueSfx2_Max6(0x6C);
  }
}

void EprojPreInstr_KagosBugs_Func2(uint16 k) {  // 0x86D1E4
  int v1 = k >> 1;
  EnemyData *v2 = gEnemyData(eproj_F[v1]);
  uint16 v3 = abs16(v2->x_pos - eproj_x_pos[v1]);
  if (!sign16(v3 - 23))
    eproj_properties[v1] |= 0x8000;
}

void EprojPreInstr_KagosBugs(uint16 k) {  // 0x86D0CA
  EprojPreInstr_KagosBugs_Func1(k);
  EprojPreInstr_KagosBugs_Func2(k);
  int v1 = k >> 1;
  if (eproj_G[v1]) {
    --eproj_G[v1];
  } else {
    eproj_instr_list_ptr[v1] = addr_word_86D052;
    eproj_instr_timers[v1] = 1;
    eproj_pre_instr[v1] = FUNC16(nullsub_302);
  }
}

static const uint16 word_86D082 = 0xe0;

void EprojPreInstr_D0EC(uint16 k) {  // 0x86D0EC
  EprojPreInstr_KagosBugs_Func1(k);
  EprojPreInstr_KagosBugs_Func2(k);
  if (EprojBlockCollisition_Horiz(k)) {
    eproj_x_vel[k >> 1] = 0;
    goto LABEL_6;
  }
  if (EprojBlockCollisition_Vertical(k)) {
LABEL_6:
    eproj_y_vel[k >> 1] = 256;
LABEL_7:;
    eproj_pre_instr[k >> 1] = FUNC16(EprojPreInstr_D128);
    eproj_instr_list_ptr[k >> 1] = addr_word_86D04A;
    eproj_instr_timers[k >> 1] = 1;
    return;
  }
  uint16 v2 = eproj_y_vel[k >> 1];
  bool v3 = (int16)(word_86D082 + v2) < 0;
  eproj_y_vel[k >> 1] = word_86D082 + v2;
  if (!v3) {
    goto LABEL_7;
  }
}

void EprojPreInstr_D128(uint16 v0) {  // 0x86D128
  EprojPreInstr_KagosBugs_Func1(v0);
  EprojPreInstr_KagosBugs_Func2(v0);
  if (EprojBlockCollisition_Horiz(v0) & 1) {
    eproj_x_vel[v0 >> 1] = 0;
  } else if (EprojBlockCollisition_Vertical(v0) & 1) {
    int v1 = v0 >> 1;
    eproj_pre_instr[v1] = FUNC16(nullsub_302);
    eproj_instr_list_ptr[v1] = addr_word_86D03C;
    eproj_instr_timers[v1] = 1;
  } else {
    eproj_y_vel[v0 >> 1] += word_86D082;
  }
}

static const uint16 g_word_86D086 = 0x200;
static const uint16 g_word_86D084 = 0x30;

const uint8 *EprojInstr_D15C(uint16 k, const uint8 *epjp) {  // 0x86D15C
  EprojPreInstr_KagosBugs_Func1(k);
  EprojPreInstr_KagosBugs_Func2(k);
  int v2 = k >> 1;

  uint16 t = (random_number & 0x300) + 2048;
  eproj_y_vel[v2] = -t;
  EnemyData *v3 = gEnemyData(eproj_F[v2]);
  uint16 t2 = v3->x_pos - eproj_x_pos[v2];

  if ((int16)(abs16(t2) - g_word_86D084) >= 0)
    t = sign16(t2) ? -1 : 0;

  uint16 v4 = g_word_86D086;
  if (t & 0x100)
    v4 = -v4;
  eproj_x_vel[v2] = v4;
  eproj_pre_instr[v2] = FUNC16(EprojPreInstr_D0EC);
  return epjp;
}

const uint8 *EprojInstr_D1B6(uint16 k, const uint8 *epjp) {  // 0x86D1B6
  int v1 = k >> 1;
  eproj_G[v1] = (random_number & 0x1F) + 1;
  eproj_pre_instr[v1] = FUNC16(EprojPreInstr_KagosBugs);
  return epjp;
}

const uint8 *EprojInstr_D1C7(uint16 k, const uint8 *epjp) {  // 0x86D1C7
  eproj_gfx_idx[k >> 1] = 0;
  return epjp;
}

const uint8 *EprojInstr_D1CE(uint16 k, const uint8 *epjp) {  // 0x86D1CE
  eproj_spawn_pt = (Point16U){ eproj_x_pos[k >> 1], eproj_y_pos[k >> 1] };
  SpawnEnemyDrops(addr_kEnemyDef_E7FF, k, 0);
  return epjp;
}

void EprojInit_MaridiaFloatersSpikes(uint16 j) {  // 0x86D23A
  int v1 = j >> 1;
  eproj_x_subpos[v1] = 0;
  eproj_y_subpos[v1] = 0;
  eproj_x_vel[v1] = 0;
  eproj_y_vel[v1] = 0;
  EnemyData *v2 = gEnemyData(cur_enemy_index);
  eproj_x_pos[v1] = v2->x_pos;
  eproj_y_pos[v1] = v2->y_pos;
  eproj_E[v1] = eproj_init_param_1;
}

static const int16 word_86D21A[8] = { 0, 32, 32, 32, 0, -32, -32, -32 };
static const int16 word_86D22A[8] = { -32, -32, 0, 32, 32, 32, 0, -32 };

void EprojPreInstr_MaridiaFloatersSpikes(uint16 k) {  // 0x86D263
  int v1 = k >> 1;
  eproj_x_vel[v1] += word_86D21A[eproj_E[v1]];
  if (EprojBlockCollisition_Horiz(k) & 1
      || (eproj_y_vel[v1] += word_86D22A[eproj_E[v1]],  EprojBlockCollisition_Vertical(k) & 1)) {
    eproj_instr_list_ptr[v1] = addr_off_86D218;
    eproj_instr_timers[v1] = 1;
  }
}

static void EprojInit_WreckedShipRobotLaserCommon(uint16 k, uint16 j) {  // 0x86D35B
  EnemyData *v2 = gEnemyData(k);
  int v3 = j >> 1;
  eproj_y_pos[v3] = v2->y_pos - 16;
  eproj_x_pos[v3] = v2->x_pos + (sign16(eproj_x_vel[v3]) ? -4 : 4);
  eproj_y_subpos[v3] = 0;
  eproj_x_subpos[v3] = 0;
  if ((int16)(v2->x_width + v2->x_pos - layer1_x_pos) >= 0
    && (int16)(v2->x_pos - v2->x_width - 257 - layer1_x_pos) < 0
    && (int16)(v2->y_height + v2->y_pos - layer1_y_pos) >= 0
    && (int16)(v2->y_pos - v2->y_height - 224) < 0) {
    QueueSfx2_Max6(0x67);
  }
}

void EprojInit_WreckedShipRobotLaserDown(uint16 j) {  // 0x86D30C
  int v2 = j >> 1;
  eproj_x_vel[v2] = gEnemyData(cur_enemy_index)->ai_var_A;
  eproj_y_vel[v2] = 128;
  eproj_gfx_idx[v2] = 0;
  EprojInit_WreckedShipRobotLaserCommon(cur_enemy_index, j);
}

void EprojInit_WreckedShipRobotLaserHorizontal(uint16 j) {  // 0x86D32E
  int v2 = j >> 1;
  eproj_x_vel[v2] = gEnemyData(cur_enemy_index)->ai_var_A;
  eproj_y_vel[v2] = 0;
  EprojInit_WreckedShipRobotLaserCommon(cur_enemy_index, j);
}

void EprojInit_WreckedShipRobotLaserUp(uint16 j) {  // 0x86D341
  int v2 = j >> 1;
  eproj_x_vel[v2] = gEnemyData(cur_enemy_index)->ai_var_A;
  eproj_y_vel[v2] = -128;
  EprojInit_WreckedShipRobotLaserCommon(cur_enemy_index, j);
}

void EprojPreInstr_WreckedShipRobotLaser(uint16 k) {  // 0x86D3BF
  int v1 = k >> 1;
  eproj_gfx_idx[v1] = 0;
  if (EprojBlockCollisition_Horiz(k) & 1 || EprojBlockCollisition_Vertical(k) & 1)
    eproj_id[v1] = 0;
}

const uint8 *EprojInstr_AssignNewN00bTubeShardVelocity(uint16 k, const uint8 *epjp) {  // 0x86D5E1
  NextRandom();
  int v2 = k >> 1;
  eproj_x_vel[v2] = *(uint16 *)((uint8 *)&random_number + 1);
  eproj_y_vel[v2] = 192;
  return epjp;
}

const uint8 *EprojInstr_SetN00bTubeShardX(uint16 k, const uint8 *epjp) {  // 0x86D5F2
  int v2 = k >> 1;
  if (nmi_frame_counter_word & 1) {
    eproj_x_pos[v2] = eproj_F[v2];
    eproj_spritemap_ptr[v2] = GET_WORD(epjp);
  } else {
    eproj_x_pos[v2] = 128 - eproj_F[v2] + 128;
    eproj_spritemap_ptr[v2] = GET_WORD(epjp + 2);
  }
  eproj_instr_list_ptr[v2] = epjp - RomBankBase(0x86) + 4;
  eproj_instr_timers[v2] = 1;
  return 0;
}

const uint8 *EprojInstr_D62A(uint16 k, const uint8 *epjp) {  // 0x86D62A
  if (nmi_frame_counter_word & 1)
    eproj_x_pos[k >> 1] = eproj_F[k >> 1];
  else
    eproj_x_pos[k >> 1] = -4608;
  int v2 = k >> 1;
  eproj_spritemap_ptr[v2] = GET_WORD(epjp);
  eproj_instr_list_ptr[v2] = epjp - RomBankBase(0x86) + 2;
  eproj_instr_timers[v2] = 1;
  return 0;
}

const uint8 *EprojInstr_SetXvelRandom(uint16 k, const uint8 *epjp) {  // 0x86D69A
  NextRandom();
  eproj_x_vel[k >> 1] = *(uint16 *)((uint8 *)&random_number + 1);
  return epjp;
}

void EprojInit_N00bTubeCrack(uint16 j) {  // 0x86D6A5
  CalculatePlmBlockCoords(plm_id);
  int v1 = j >> 1;
  eproj_x_pos[v1] = 16 * plm_x_block + 96;
  eproj_y_pos[v1] = 16 * plm_y_block + 48;
}

static const int16 kEprojInit_N00bTubeShards_X[10] = { -56, -64, -20, -40, -64, -48, -24, -40, 0, -8 };
static const int16 kEprojInit_N00bTubeShards_Y[10] = { 8, -12, -26, -24, -32, 28, 16, -8, -24, 16 };
static const int16 kEprojInit_N00bTubeShards_Xvel[10] = { -384, -384, -160, -288, -288, -320, -96, -352, 0, -64 };
static const int16 kEprojInit_N00bTubeShards_Yvel[10] = { 320, -256, -416, -288, -288, 448, 576, -96, -288, 384 };

void EprojInit_N00bTubeShards(uint16 j) {  // 0x86D6C9
  CalculatePlmBlockCoords(plm_id);
  int v1 = eproj_init_param_1 >> 1;
  int v2 = j >> 1;
  eproj_F[v2] = kEprojInit_N00bTubeShards_X[v1] + 16 * plm_x_block + 96;
  eproj_E[v2] = 0;
  eproj_y_pos[v2] = kEprojInit_N00bTubeShards_Y[v1] + 16 * plm_y_block + 48;
  eproj_instr_list_ptr[v2] = kEprojInit_N00bTubeShards_InstrPtrs[v1];
  eproj_x_vel[v2] = kEprojInit_N00bTubeShards_Xvel[v1];
  eproj_y_vel[v2] = kEprojInit_N00bTubeShards_Yvel[v1];
}

static const uint16 kEprojInit_N00bTubeReleasedAirBubbles_X[6] = { 40, 80, 104, 120, 152, 184 };
static const uint16 kEprojInit_N00bTubeReleasedAirBubbles_Y[6] = { 80, 72, 84, 32, 64, 84 };

void EprojInit_N00bTubeReleasedAirBubbles(uint16 j) {  // 0x86D774
  CalculatePlmBlockCoords(plm_id);
  int v1 = eproj_init_param_1 >> 1;
  int v2 = j >> 1;
  eproj_F[v2] = kEprojInit_N00bTubeReleasedAirBubbles_X[v1] + 16 * plm_x_block;
  eproj_E[v2] = 0;
  eproj_y_pos[v2] = kEprojInit_N00bTubeReleasedAirBubbles_Y[v1] + 16 * plm_y_block;
  eproj_y_vel[v2] = -1280;
}

void sub_86D7BF(uint16 k) {  // 0x86D7BF
  int v1 = k >> 1;
  if (eproj_x_pos[v1] != 0xEE00)
    eproj_E[v1] = eproj_x_pos[v1];
  if (nmi_frame_counter_word & 1)
    eproj_x_pos[v1] = -4608;
  else
    eproj_x_pos[v1] = eproj_E[v1];
}

void sub_86D7DE(uint16 k) {  // 0x86D7DE
  int v1 = k >> 1;
  int32 amt = INT16_SHL8(192);
  AddToHiLo(&eproj_y_pos[v1], &eproj_y_subpos[v1], amt);
}

void EprojPreInstr_N00bTubeShards(uint16 k) {  // 0x86D7FD
  int v1 = k >> 1;
  int32 amt = INT16_SHL8(eproj_x_vel[v1]);
  AddToHiLo(&eproj_F[v1], &eproj_E[v1], amt);
  amt = INT16_SHL8(eproj_y_vel[v1]);
  AddToHiLo(&eproj_y_pos[v1], &eproj_y_subpos[v1], amt);
  Eproj_DeleteIfYposOutside(k);
}

void sub_86D83D(uint16 k) {  // 0x86D83D
  int v2 = (eproj_x_vel[k >> 1] & 0x17E | 0x80) >> 1;
  int v6 = eproj_index >> 1;
  int32 amt = INT16_SHL8((int16)kSinCosTable8bit_Sext[v2 + 64] >> 2);
  AddToHiLo(&eproj_F[v6], &eproj_E[v6], amt);
  eproj_x_vel[v6] += 2;
  amt = INT16_SHL8(eproj_y_vel[v6]);
  AddToHiLo(&eproj_y_pos[v6], &eproj_y_subpos[v6], amt);
  Eproj_DeleteIfYposOutside(eproj_index);
}

void sub_86D89F(uint16 k) {  // 0x86D89F
  uint16 v1 = eproj_x_vel[k >> 1] & 0x17E | 0x80;
  int v2 = v1 >> 1;
  int32 amt = INT16_SHL8((int16)kSinCosTable8bit_Sext[v2 + 64] >> 2);
  int v5 = eproj_index >> 1;
  AddToHiLo(&eproj_F[v5], &eproj_E[v5], amt);
  eproj_x_vel[v5] += 4;
  sub_86D8DF(eproj_index);
}

void sub_86D8DF(uint16 k) {  // 0x86D8DF
  int v5 = k >> 1;
  int32 amt = INT16_SHL8(eproj_y_vel[v5]);
  AddToHiLo(&eproj_y_pos[v5], &eproj_y_subpos[v5], amt);
  eproj_x_pos[v5] = eproj_F[v5];
}

void sub_86D992(uint16 v0) {  // 0x86D992
  uint16 v1 = eproj_init_param_1;
  int v2 = v0 >> 1;
  eproj_E[v2] = eproj_init_param_1;
  eproj_instr_list_ptr[v2] = off_86D96A[v1 >> 1];
  EnemyData *v3 = gEnemyData(cur_enemy_index);
  eproj_x_pos[v2] = v3->x_pos;
  eproj_x_subpos[v2] = v3->x_subpos;
  eproj_y_pos[v2] = v3->y_pos;
  eproj_y_subpos[v2] = v3->y_subpos;
  eproj_y_vel[v2] = -512;
  eproj_x_vel[v2] = 512;
  if (!sign16(eproj_init_param_1 - 12)) {
    eproj_y_vel[v2] = -384;
    eproj_x_vel[v2] = 384;
  }
}

void EprojPreInstr_SpikeShootingPlantSpikes_MoveY1(uint16 k) {  // 0x86D9E6
  int v1 = k >> 1;
  AddToHiLo(&eproj_y_pos[v1], &eproj_y_subpos[v1], INT16_SHL8(eproj_y_vel[v1]));
}

void EprojPreInstr_SpikeShootingPlantSpikes_MoveY2(uint16 k) {  // 0x86DA10
  int v1 = k >> 1;
  AddToHiLo(&eproj_y_pos[v1], &eproj_y_subpos[v1], INT16_SHL8(eproj_x_vel[v1]));
}

void EprojPreInstr_SpikeShootingPlantSpikes_MoveX1(uint16 k) {  // 0x86DA3A
  int v1 = k >> 1;
  AddToHiLo(&eproj_x_pos[v1], &eproj_x_subpos[v1], INT16_SHL8(eproj_y_vel[v1]));
}

void EprojPreInstr_SpikeShootingPlantSpikes_MoveX2(uint16 k) {  // 0x86DA64
  int v1 = k >> 1;
  AddToHiLo(&eproj_x_pos[v1], &eproj_x_subpos[v1], INT16_SHL8(eproj_x_vel[v1]));
}

void EprojPreInstr_SpikeShootingPlantSpikes_0_MoveX1(uint16 k) {  // 0x86DA8E
  EprojPreInstr_SpikeShootingPlantSpikes_MoveX1(k);
}

void EprojPreInstr_SpikeShootingPlantSpikes_2_MoveX2(uint16 k) {  // 0x86DA93
  EprojPreInstr_SpikeShootingPlantSpikes_MoveX2(k);
}

void EprojPreInstr_SpikeShootingPlantSpikes_1_MoveY1(uint16 k) {  // 0x86DA98
  EprojPreInstr_SpikeShootingPlantSpikes_MoveY1(k);
}

void EprojPreInstr_SpikeShootingPlantSpikes_4_MoveY2(uint16 k) {  // 0x86DA9D
  EprojPreInstr_SpikeShootingPlantSpikes_MoveY2(k);
}

void EprojPreInstr_SpikeShootingPlantSpikes_6_MoveX1Y1(uint16 k) {  // 0x86DAA2
  EprojPreInstr_SpikeShootingPlantSpikes_MoveX1(k);
  EprojPreInstr_SpikeShootingPlantSpikes_MoveY1(k);
}

void EprojPreInstr_SpikeShootingPlantSpikes_8_MoveX1Y2(uint16 k) {  // 0x86DAAA
  EprojPreInstr_SpikeShootingPlantSpikes_MoveX1(k);
  EprojPreInstr_SpikeShootingPlantSpikes_MoveY2(k);
}

void EprojPreInstr_SpikeShootingPlantSpikes_7_MoveX2Y1(uint16 k) {  // 0x86DAB2
  EprojPreInstr_SpikeShootingPlantSpikes_MoveX2(k);
  EprojPreInstr_SpikeShootingPlantSpikes_MoveY1(k);
}

void EprojPreInstr_SpikeShootingPlantSpikes_9_MoveX2Y2(uint16 k) {  // 0x86DABA
  EprojPreInstr_SpikeShootingPlantSpikes_MoveX2(k);
  EprojPreInstr_SpikeShootingPlantSpikes_MoveY2(k);
}

static Func_Y_V *const kEprojPreInstr_SpikeShootingPlantSpikes[10] = {  // 0x86D9DB
  EprojPreInstr_SpikeShootingPlantSpikes_0_MoveX1,
  EprojPreInstr_SpikeShootingPlantSpikes_1_MoveY1,
  EprojPreInstr_SpikeShootingPlantSpikes_2_MoveX2,
  EprojPreInstr_SpikeShootingPlantSpikes_0_MoveX1,
  EprojPreInstr_SpikeShootingPlantSpikes_4_MoveY2,
  EprojPreInstr_SpikeShootingPlantSpikes_2_MoveX2,
  EprojPreInstr_SpikeShootingPlantSpikes_6_MoveX1Y1,
  EprojPreInstr_SpikeShootingPlantSpikes_7_MoveX2Y1,
  EprojPreInstr_SpikeShootingPlantSpikes_8_MoveX1Y2,
  EprojPreInstr_SpikeShootingPlantSpikes_9_MoveX2Y2,
};

void EprojPreInstrHelper_SpikeShootingPlantSpikes_Func1(uint16 k) {  // 0x86DAC2
  if (EprojPreInstrHelper_SpikeShootingPlantSpikes_Func2(k))
    eproj_id[k >> 1] = 0;
}

void EprojPreInstr_SpikeShootingPlantSpikes(uint16 k) {
  kEprojPreInstr_SpikeShootingPlantSpikes[eproj_E[k >> 1] >> 1](k);
  EprojPreInstrHelper_SpikeShootingPlantSpikes_Func1(k);
}

uint16 EprojPreInstrHelper_SpikeShootingPlantSpikes_Func2(uint16 k) {  // 0x86DACE
  int v1 = k >> 1;
  return (int16)(eproj_x_pos[v1] - layer1_x_pos) < 0
    || (int16)(layer1_x_pos + 256 - eproj_x_pos[v1]) < 0
    || (int16)(eproj_y_pos[v1] - layer1_y_pos) < 0
    || (int16)(layer1_y_pos + 256 - eproj_y_pos[v1]) < 0;
}

void EprojInit_DBF2(uint16 j) {  // 0x86DB18
  int v2 = j >> 1;
  eproj_instr_list_ptr[v2] = addr_word_86DB0C;
  eproj_E[v2] = FUNC16(EprojPreInstr_DBF2_MoveX1);
  if (eproj_init_param_1)
    eproj_E[v2] = FUNC16(EprojPreInstr_DBF2_MoveX2);
  EnemyData *v3 = gEnemyData(cur_enemy_index);
  eproj_x_pos[v2] = v3->x_pos;
  eproj_x_subpos[v2] = v3->x_subpos;
  eproj_y_pos[v2] = v3->y_pos + 2;
  eproj_y_subpos[v2] = v3->y_subpos;
  eproj_y_vel[v2] = -256;
  eproj_x_vel[v2] = 256;
}

void EprojPreInstr_DBF2_Func1(uint16 k) {  // 0x86DBB6
  if (EprojPreInstrHelper_DBF2_Func2(k))
    eproj_id[k >> 1] = 0;
}

void EprojPreInstr_DBF2(uint16 k) {  // 0x86DB5B
  CallEprojPreInstr(eproj_E[k >> 1] | 0x860000, k);
  EprojPreInstr_DBF2_Func1(k);
}

void EprojPreInstr_DBF2_MoveX1(uint16 k) {  // 0x86DB62
  int v1 = k >> 1;
  AddToHiLo(&eproj_x_pos[v1], &eproj_x_subpos[v1], INT16_SHL8(eproj_y_vel[v1]));
}

void EprojPreInstr_DBF2_MoveX2(uint16 k) {  // 0x86DB8C
  int v1 = k >> 1;
  AddToHiLo(&eproj_x_pos[v1], &eproj_x_subpos[v1], INT16_SHL8(eproj_x_vel[v1]));
}

uint16 EprojPreInstrHelper_DBF2_Func2(uint16 k) {  // 0x86DBC2
  int v1 = k >> 1;
  return (int16)(eproj_x_pos[v1] - layer1_x_pos) < 0
    || (int16)(layer1_x_pos + 256 - eproj_x_pos[v1]) < 0
    || (int16)(eproj_y_pos[v1] - layer1_y_pos) < 0
    || (int16)(layer1_y_pos + 256 - eproj_y_pos[v1]) < 0;
}

const uint8 *EprojInstr_DC5A(uint16 k, const uint8 *epjp) {  // 0x86DC5A
  eproj_properties[k >> 1] = 12288;
  return epjp;
}

const uint8 *EprojInstr_SpawnEnemyDrops_0(uint16 k, const uint8 *epjp) {  // 0x86DC61
  int v2 = k >> 1;
  eproj_spawn_pt = (Point16U){ eproj_x_pos[v2], eproj_y_pos[v2] };
  SpawnEnemyDrops(addr_kEnemyDef_DF7F, k, 0);
  return epjp;
}

const uint8 *EprojInstr_SpawnSporesEproj(uint16 k, const uint8 *epjp) {  // 0x86DC77
  int v2 = k >> 1;
  eproj_spawn_pt = (Point16U) { eproj_x_pos[v2], eproj_y_pos[v2] };
  SpawnEprojWithRoomGfx(addr_kEproj_Spores, 0);
  return epjp;
}

void EprojInit_Spores(uint16 j) {  // 0x86DC8D
  int v2 = j >> 1;
  eproj_F[v2] = eproj_x_pos[v2] = eproj_spawn_pt.x;
  eproj_y_pos[v2] = eproj_spawn_pt.y;
  eproj_gfx_idx[v2] = 512;
}

static const int16 word_86DCB9[5] = { -64, -56, -48, -40, -32 };

void EprojInit_SporeSpawnStalk(uint16 j) {  // 0x86DCA3
  int v1 = j >> 1;
  eproj_y_pos[v1] = enemy_data[0].y_pos + word_86DCB9[eproj_init_param_1];
  eproj_x_pos[v1] = enemy_data[0].x_pos;
}

void sub_86DCC3(uint16 v0) {  // 0x86DCC3
  int v1 = v0 >> 1;
  eproj_y_pos[v1] = enemy_data[0].y_pos - 96;
  eproj_x_pos[v1] = enemy_data[0].x_pos;
}

static const int16 kEprojInit_SporeSpawners_X[4] = { 0x20, 0x60, 0xa0, 0xe0 };

void EprojInit_SporeSpawners(uint16 j) {  // 0x86DCD4
  int v1 = j >> 1;
  eproj_x_pos[v1] = kEprojInit_SporeSpawners_X[eproj_init_param_1];
  eproj_y_pos[v1] = 520;
}

void EprojPreInstr_Spores(uint16 k) {  // 0x86DCEE
  int v1 = k >> 1;
  uint16 v2 = LOBYTE(eproj_E[v1]);
  uint16 r18 = SignExtend8(kSporeMovementData[LOBYTE(eproj_E[v1])]);
  if ((eproj_F[v1] & 0x80) != 0)
    r18 = -r18;
  eproj_x_pos[v1] += r18;
  r18 = SignExtend8(kSporeMovementData[v2 + 1]);
  uint16 v3 = r18 + eproj_y_pos[v1] + r18;
  eproj_y_pos[v1] = v3;
  if (!sign16(v3 - 768))
    eproj_id[v1] = 0;
  eproj_E[v1] = (uint8)(LOBYTE(eproj_E[v1]) + 2);
}

void EprojPreInstr_SporeSpawners(uint16 k) {  // 0x86DD46
  if (!kraid_unk9000) {
    int v1 = k >> 1;
    if (!eproj_F[v1]) {
      eproj_instr_list_ptr[v1] = addr_word_86DC06;
      eproj_instr_timers[v1] = 1;
      eproj_F[v1] = NextRandom() & 0x1FF;
    }
    --eproj_F[v1];
  }
}

const uint8 *EprojInstr_DFEA(uint16 k, const uint8 *epjp) {  // 0x86DFEA
  int v1 = k >> 1;
  eproj_spawn_pt = (Point16U){ eproj_x_pos[v1], eproj_y_pos[v1] };
  SpawnEnemyDrops(addr_kEnemyDef_E83F, k, 0);
  return epjp;
}
