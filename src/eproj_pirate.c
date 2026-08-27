// Pirate, Ceres, Shaktool, and Ceres Ridley enemy-projectile families split out of sm_86.c.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "eproj_internal.h"

#define off_86A2E2 ((uint16*)RomFixedPtr(0x86a2e2))

void sub_86A301(uint16 j);

static const int16 kEprojInit_9634_Xvel[4] = { -0x200, -0x1f0, -0x1bc, -0x16a };
static const int16 kEprojInit_9634_Yvel[4] = { 0, 0x88, 0xfc, 0x16a };

void EprojInit_9634(uint16 j) {  // 0x86934D
  int v1 = j >> 1;
  eproj_E[v1] = 0;
  eproj_F[v1] = 0;
  eproj_x_pos[v1] = enemy_data[0].x_pos - 29;
  eproj_y_pos[v1] = enemy_data[0].y_pos - 35;
  eproj_gfx_idx[v1] = 2560;
  int v2 = enemy_data[0].parameter_1;
  eproj_x_vel[v1] = kEprojInit_9634_Xvel[v2];
  eproj_y_vel[v1] = kEprojInit_9634_Yvel[v2];
}

void EprojPreInstr_9634(uint16 k) {  // 0x869392
  int v1 = k >> 1;
  if (eproj_E[v1] >= 8) {
    MoveEprojWithVelocity(k);
    if (EprojBlockCollisition_Vertical(k) & 1) {
      eproj_instr_list_ptr[v1] = addr_off_869574;
      ++eproj_E[v1];
      eproj_instr_timers[v1] = 1;
      eproj_x_vel[v1] = 0;
      eproj_y_vel[v1] = 0;
      eproj_gfx_idx[v1] = 2560;
      QueueSfx2_Max6(0x2B);
    }
  } else {
    ++eproj_E[v1];
  }
}

void EprojInit_9642_RidleysFireball(uint16 j) {  // 0x8693CA
  int v1 = j >> 1;
  eproj_F[v1] = eproj_unk1995;
  int16 v2 = eproj_init_param_1 ? 25 : -25;
  eproj_x_pos[v1] = enemy_data[0].x_pos + v2;
  eproj_y_pos[v1] = enemy_data[0].y_pos - 43;
  eproj_gfx_idx[v1] = 2560;

  Ram7800_Default *v4 = gRam7800_Default(0);
  eproj_x_vel[v1] = v4->var_19;
  eproj_y_vel[v1] = v4->var_1A;
  SetAreaDependentEprojPropertiesEx(addr_kRidleysFireball_Tab0, j);
}

void EprojPreInstr_9642_RidleysFireball(uint16 k) {  // 0x86940E
  uint16 v1;
  if (EprojBlockCollisition_Horiz(k) & 1) {
    v1 = -27042;
  } else {
    if (!(EprojBlockCollisition_Vertical(k) & 1))
      return;
    v1 = -27056;
  }
  int v2 = k >> 1;
  eproj_id[v2] = 0;
  if (!eproj_F[v2]) {
    eproj_spawn_pt = (Point16U){ eproj_x_pos[v2], eproj_y_pos[v2] };
    SpawnEprojWithRoomGfx(v1, 3);
    QueueSfx2_Max6(0x2B);
  }
}

const uint8 *EprojInstr_DisableCollisionsWithSamus(uint16 k, const uint8 *epjp) {  // 0x869475
  eproj_properties[k >> 1] |= 0x2000;
  return epjp;
}

void EprojInit_9660_FireballExplosion(uint16 j) {  // 0x86947F
  int v1 = j >> 1;
  eproj_gfx_idx[v1] = 2560;
  eproj_x_pos[v1] = eproj_spawn_pt.x;
  eproj_y_pos[v1] = eproj_spawn_pt.y;
  eproj_E[v1] = eproj_init_param_1;
  eproj_F[v1] = 0;
  eproj_x_vel[v1] = 0;
  eproj_y_vel[v1] = 0;
}

static void EprojInit_Common(uint16 j, uint16 k, uint16 a) {  // 0x8694EE
  int v3 = j >> 1;
  eproj_F[v3] = a;
  int v4 = k >> 1;
  eproj_x_pos[v3] = eproj_x_pos[v4];
  eproj_y_pos[v3] = eproj_y_pos[v4];
  eproj_E[v3] = eproj_E[v4];
  eproj_gfx_idx[v3] = 2560;
  SetAreaDependentEprojProperties(j);
}

void EprojInit_9688(uint16 j) {  // 0x8694A0
  uint16 v1 = eproj_init_param_1;
  int v2 = j >> 1;
  eproj_x_vel[v2] = 0;
  eproj_y_vel[v2] = -3584;
  EprojInit_Common(j, v1, 0x9688);
}

void EprojInit_9696(uint16 j) {  // 0x8694B4
  uint16 v1 = eproj_init_param_1;
  int v2 = j >> 1;
  eproj_x_vel[v2] = 0;
  eproj_y_vel[v2] = 3584;
  EprojInit_Common(j, v1, 0x9696);
}

void EprojInit_966C(uint16 j) {  // 0x8694C8
  uint16 v1 = eproj_init_param_1;
  int v2 = j >> 1;
  eproj_x_vel[v2] = 3584;
  eproj_y_vel[v2] = 0;
  EprojInit_Common(j, v1, 0x966C);
}

void EprojInit_967A(uint16 j) {  // 0x8694DC
  uint16 v1 = eproj_init_param_1;
  int v2 = j >> 1;
  eproj_x_vel[v2] = -3584;
  eproj_y_vel[v2] = 0;
  EprojInit_Common(j, v1, 0x967A);
}

void EprojPreInstr_966C(uint16 k) {  // 0x86950D
  MoveEprojWithVelocityX(k);
  if (EprojBlockCollisition_Vertical(k) & 1) {
    int v1 = k >> 1;
    eproj_instr_list_ptr[v1] = addr_off_869574;
    eproj_instr_timers[v1] = 1;
  }
}

void EprojPreInstr_9688(uint16 k) {  // 0x869522
  MoveEprojWithVelocityY(k);
  if (EprojBlockCollisition_Horiz(k) & 1) {
    int v1 = k >> 1;
    eproj_instr_list_ptr[v1] = addr_off_869574;
    eproj_instr_timers[v1] = 1;
  }
}

void EprojPreInstr_96A4(uint16 k) {  // 0x869537
  if (EprojBlockCollisition_Horiz(k) & 1)
    eproj_id[k >> 1] = 0;
}

void EprojPreInstr_96C0(uint16 k) {  // 0x869540
  if (EprojBlockCollisition_Vertical(k) & 1)
    eproj_id[k >> 1] = 0;
}

void EprojPreInstr_96CE(uint16 k) {  // 0x869549
  if (EprojBlockCollisition_Vertical(k) & 1)
    eproj_id[k >> 1] = 0;
}

const uint8 *EprojInstr_95BA(uint16 k, const uint8 *epjp) {  // 0x8695BA
  eproj_unk1995 = 0;
  SpawnEprojWithRoomGfx(addr_stru_86966C, k);
  eproj_unk1995 = 0;
  SpawnEprojWithRoomGfx(addr_stru_86967A, k);
  return epjp;
}

const uint8 *EprojInstr_95ED(uint16 k, const uint8 *epjp) {  // 0x8695ED
  eproj_unk1995 = 0;
  SpawnEprojWithRoomGfx(addr_stru_869688, k);
  eproj_unk1995 = 0;
  SpawnEprojWithRoomGfx(addr_stru_869696, k);
  return epjp;
}

const uint8 *EprojInstr_9620(uint16 k, const uint8 *epjp) {  // 0x869620
  if ((int8)-- * ((uint8 *)eproj_E + k) >= 0)
    SpawnEprojWithRoomGfx(eproj_F[k >> 1], k);
  return epjp;
}

void EprojInit_9734_CeresFallingDebris(uint16 j) {  // 0x8696DC
  int v1 = j >> 1;
  eproj_E[v1] = 0;
  eproj_F[v1] = 0;
  eproj_x_vel[v1] = 0;
  eproj_gfx_idx[v1] = 3584;
  eproj_x_pos[v1] = eproj_init_param_1;
  eproj_y_pos[v1] = 42;
  eproj_y_vel[v1] = 16;
}

void EprojPreInstr_9734_CeresFallingDebris(uint16 k) {  // 0x869701
  int v1 = k >> 1;
  eproj_y_vel[v1] += 16;
  if (EprojBlockCollisition_Vertical(k) & 1) {
    eproj_id[v1] = 0;
    eproj_spawn_pt = (Point16U) { eproj_x_pos[v1], eproj_y_pos[v1] };
    SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 9);
    QueueSfx2_Max6(0x6D);
  }
}

const uint8 *EprojInstr_980E(uint16 k, const uint8 *epjp) {  // 0x86980E
  int v2 = k >> 1;
  eproj_spawn_pt = (Point16U) { eproj_x_pos[v2], eproj_y_pos[v2] };
  SpawnEnemyDrops(addr_kEnemyDef_E4FF, k, 0);
  return epjp;
}

void EprojInit_PirateMotherBrainLaser(uint16 j) {  // 0x86A009
  int v2 = j >> 1;
  eproj_instr_list_ptr[v2] = eproj_spawn_r22 ? addr_word_869F7D : addr_word_869F41;
  eproj_pre_instr[v2] = FUNC16(nullsub_87);
  eproj_x_pos[v2] = eproj_spawn_pt.x;
  eproj_y_pos[v2] = eproj_spawn_pt.y;
  EnemyData *v3 = gEnemyData(cur_enemy_index);
  eproj_properties[v2] = *((uint16 *)RomPtr_A0(v3->enemy_ptr) + 3) | 0x1000;
  eproj_E[v2] = v3->parameter_1;
  QueueSfx2_Max6(0x67);
}

const uint8 *EprojInstr_SetPreInstrAndRun(uint16 k, const uint8 *epjp) {  // 0x86A050
  eproj_pre_instr[k >> 1] = GET_WORD(epjp);
  return epjp;
}

void EprojPreInstr_PirateMotherBrainLaser_MoveLeft(uint16 k) {  // 0x86A05C
  int v1 = k >> 1;
  eproj_x_pos[v1] -= 2;
  if ((eproj_E[v1] & 0x8000) == 0)
    eproj_x_pos[v1] -= 2;
  if (CheckIfEprojIsOffScreen(k))
    eproj_id[v1] = 0;
}

void EprojPreInstr_PirateMotherBrainLaser_MoveRight(uint16 k) {  // 0x86A07A
  int v1 = k >> 1;
  eproj_x_pos[v1] += 2;
  if ((eproj_E[v1] & 0x8000) == 0)
    eproj_x_pos[v1] += 2;
  if (CheckIfEprojIsOffScreen(k))
    eproj_id[v1] = 0;
}

void EprojInit_PirateClaw(uint16 j) {  // 0x86A098
  Rect16U r = eproj_spawn_rect;
  int v1 = j >> 1;
  eproj_y_pos[v1] = r.y + r.h;
  eproj_x_pos[v1] = r.x + r.w;
  eproj_instr_list_ptr[v1] = eproj_init_param_1 ? addr_off_869FE1 : addr_off_869FB9;
  eproj_pre_instr[v1] = FUNC16(nullsub_87);
  eproj_E[v1] = 2048;
  eproj_F[v1] = 1;
}

void EprojPreInstr_PirateClawThrownLeft(uint16 k) {  // 0x86A0D1
  int v1 = k >> 1;
  if (eproj_F[v1]) {
    eproj_x_pos[v1] -= HIBYTE(eproj_E[v1]);
    uint16 v2 = eproj_E[v1] - 32;
    eproj_E[v1] = v2;
    if (!v2)
      eproj_F[v1] = 0;
  } else {
    eproj_x_pos[v1] += HIBYTE(eproj_E[v1]);
    eproj_E[v1] += 32;
  }
  ++eproj_y_pos[v1];
  if (CheckIfEprojIsOffScreen(k))
    eproj_id[v1] = 0;
}

void EprojPreInstr_PirateClawThrownRight(uint16 k) {  // 0x86A124
  int v1 = k >> 1;
  if (eproj_F[v1]) {
    eproj_x_pos[v1] += HIBYTE(eproj_E[v1]);
    uint16 v2 = eproj_E[v1] - 32;
    eproj_E[v1] = v2;
    if (!v2)
      eproj_F[v1] = 0;
  } else {
    eproj_x_pos[v1] -= HIBYTE(eproj_E[v1]);
    eproj_E[v1] += 32;
  }
  ++eproj_y_pos[v1];
  if (CheckIfEprojIsOffScreen(k))
    eproj_id[v1] = 0;
}

static const int16 word_86A2D6[6] = { 64, 72, 80, -64, -72, -80 };

void EprojInit_A379(uint16 j) {  // 0x86A2A1
  int v1 = j >> 1;
  eproj_x_subpos[v1] = 0;
  eproj_y_subpos[v1] = 0;
  eproj_x_vel[v1] = 0;
  eproj_y_vel[v1] = 0;
  uint16 v2 = eproj_init_param_1;
  int v3 = eproj_init_param_1 >> 1;
  eproj_x_pos[v1] = word_86A2D6[v3] + samus_x_pos;
  eproj_y_pos[v1] = samus_y_pos + 80;
  eproj_instr_list_ptr[v1] = off_86A2E2[v3];
  eproj_E[v1] = v2;
}

void EprojInit_CeresElevatorPad(uint16 j) {  // 0x86A2EE
  int v1 = j >> 1;
  eproj_y_pos[v1] = samus_y_pos + 28;
  eproj_E[v1] = 60;
  sub_86A301(j);
}

void sub_86A301(uint16 j) {  // 0x86A301
  int v1 = j >> 1;
  eproj_x_subpos[v1] = 0;
  eproj_y_subpos[v1] = 0;
  eproj_x_vel[v1] = 0;
  eproj_y_vel[v1] = 0;
  eproj_gfx_idx[v1] = 0;
  eproj_x_pos[v1] = samus_x_pos;
}


void EprojInit_CeresElevatorPlatform(uint16 j) {  // 0x86A31B
  eproj_y_pos[j >> 1] = 97;
  sub_86A301(j);
}


void EprojPreInstr_CeresElevatorPad(uint16 k) {  // 0x86A328
  bool v2; // zf
  bool v3; // sf

  int v1 = k >> 1;
  if (!eproj_E[v1]
      || (v2 = eproj_E[v1] == 1, v3 = (int16)(eproj_E[v1] - 1) < 0, --eproj_E[v1], v2)
      || v3) {
    eproj_y_pos[v1] = samus_y_pos + 28;
    if (!sign16(++samus_y_pos - 73)) {
      samus_y_pos = 72;
      eproj_instr_timers[v1] = 1;
      eproj_instr_list_ptr[v1] = addr_off_86A28B;
      CallSomeSamusCode(0xE);
    }
  }
}

void EprojPreInstr_CeresElevatorPlatform(uint16 k) {  // 0x86A364
  if (samus_y_pos == 72) {
    int v1 = k >> 1;
    eproj_instr_timers[v1] = 1;
    eproj_instr_list_ptr[v1] = addr_off_86A28B;
  }
}

void EprojPreInstr_PrePhantomRoom(uint16 j) {  // 0x86A3A3
  bg2_y_scroll = 0;
}

const uint8 *EprojInstr_A3BE(uint16 k, const uint8 *epjp) {  // 0x86A3BE
  int v1 = k >> 1;
  eproj_x_pos[v1] = eproj_E[v1];
  eproj_y_pos[v1] = eproj_F[v1];
  return epjp;
}

static const int16 kEprojInit_ShaktoolAttackMiddleBackCircle_X[8] = { 0, 12, 16, 12, 0, -12, -16, -12 };
static const int16 kEprojInit_ShaktoolAttackMiddleBackCircle_Y[8] = { -16, -12, 0, 12, 16, 12, 0, -12 };

void EprojInit_BDA2(uint16 j) {  // 0x86BDA2
  int v1 = j >> 1;
  EnemyData *v2 = gEnemyData(cur_enemy_index);
  eproj_x_pos[v1] = v2->x_pos;
  eproj_y_pos[v1] = v2->y_pos;
  uint16 v3 = 2 * LOBYTE(v2->ai_var_D);
  int v4 = v3 >> 1;
  eproj_x_vel[v1] = kSinCosTable8bit_Sext[v4 + 64];
  eproj_y_vel[v1] = kSinCosTable8bit_Sext[v4];
  int v5 = (uint16)(v3 >> 5) >> 1;
  eproj_x_pos[v1] += kEprojInit_ShaktoolAttackMiddleBackCircle_X[v5];
  eproj_y_pos[v1] += kEprojInit_ShaktoolAttackMiddleBackCircle_Y[v5];
}

void EprojInit_ShaktoolAttackMiddleBackCircle(uint16 j) {  // 0x86BD9C
  eproj_E[j >> 1] = eproj_init_param_1;
  EprojInit_BDA2(j);
}

void EprojInit_ShaktoolAttackFrontCircle(uint16 v1) {  // 0x86BE03
  if (EprojBlockCollisition_Horiz(v1) & 1 || EprojBlockCollisition_Vertical(v1) & 1)
    eproj_id[v1 >> 1] = 0;
}

void EprojPreInstr_BE12(uint16 k) {  // 0x86BE12
  int v1 = k >> 1;
  if (eproj_id[eproj_E[v1] >> 1]) {
    EprojBlockCollisition_Horiz(k);
    EprojBlockCollisition_Vertical(k);
  } else {
    eproj_id[v1] = 0;
  }
}
