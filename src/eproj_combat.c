// Leftover Bank $86 enemy-projectile dispatch and unpeeled families.
// Drain plan:
// - Phantoon fireballs -> eproj_phantoon.c
// - Walking lava seahorse fireball -> eproj_environment.c
// - Remaining Bomb Torizo / Chozo / statue-dust handlers -> enemy_torizo_projectiles.c / eproj_tourian.c
// - Shared off-screen helpers Eproj_FuncE722 / Eproj_FuncE73E_MoveXY -> eproj_core.c
// - CallEprojInit / CallEprojPreInstr / CallEprojInstr stay here until the address ABI is retired
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "eproj_internal.h"

#define kAlignPos_Tab1 ((uint8*)RomFixedPtr(0x94892b))
#define off_86A64D ((uint16*)RomFixedPtr(0x86a64d))
#define off_86BB1E ((uint16*)RomFixedPtr(0x86bb1e))
#define kCommonEnemySpeeds_Quadratic_Copy ((uint16*)RomFixedPtr(0xa0cbc7))
#define off_86C040 ((uint16*)RomFixedPtr(0x86c040))
#define kEprojInit_BombTorizoStatueBreaking_InstrList ((uint16*)RomFixedPtr(0x86a7ab))
#define word_86DEB6 ((uint16*)RomFixedPtr(0x86deb6))
#define off_86E42C ((uint16*)RomFixedPtr(0x86e42c))
#define word_86E47E ((uint16*)RomFixedPtr(0x86e47e))

void sub_86A887(uint16 k);
void sub_86A91A(uint16 v0);
void sub_86BB30(uint16 j);

static const uint8 byte_8698B4[16] = { 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0xf0, 0xe0, 0xd0, 0xc0, 0xb0, 0xa0, 0x90, 0x80 };
static const uint8 byte_8698F7[9] = { 0x30, 0x44, 0x58, 0x6c, 0x80, 0x94, 0xa8, 0xbc, 0xd0 };
static const uint8 byte_869979[8] = { 0, 0x20, 0x40, 0x60, 0x80, 0xa0, 0xc0, 0xe0 };


static void EprojInit_PhantoonDestroyableFireballs(uint16 j) {  // 0x869824
  int8 v1;

  v1 = (uint16)(eproj_init_param_1 & 0xFF00) >> 8;
  switch (v1) {
  case 0: {
    int v2 = j >> 1;
    eproj_x_subpos[v2] = 0;
    eproj_y_subpos[v2] = 0;
    eproj_x_vel[v2] = 0;
    eproj_y_vel[v2] = 0;
    eproj_x_pos[v2] = enemy_data[0].x_pos;
    eproj_y_pos[v2] = enemy_data[0].y_pos + 32;
    eproj_instr_list_ptr[v2] = addr_off_8697B4;
    eproj_properties[v2] = eproj_properties[v2] & 0xFFF | 0x2000;
    break;
  }
  case 2: {
    int v3 = j >> 1;
    eproj_x_subpos[v3] = 0;
    eproj_y_subpos[v3] = 0;
    eproj_y_vel[v3] = 0;
    uint16 v4 = (uint8)eproj_init_param_1;
    if ((int16)((uint8)eproj_init_param_1 - 8) >= 0)
      eproj_x_vel[v3] = -2;
    else
      eproj_x_vel[v3] = 2;
    eproj_E[v3] = byte_8698B4[v4];
    eproj_x_pos[v3] = enemy_data[0].x_pos;
    eproj_y_pos[v3] = enemy_data[0].y_pos + 32;
    eproj_pre_instr[v3] = FUNC16(EprojPreInstr_PhantoonDestroyableFireballs);
    break;
  }
  case 4: {
    int v5 = j >> 1;
    eproj_x_subpos[v5] = 0;
    eproj_y_subpos[v5] = 0;
    eproj_y_vel[v5] = 0;
    eproj_x_vel[v5] = (eproj_init_param_1 & 0xF0) >> 1;
    eproj_x_pos[v5] = byte_8698F7[eproj_init_param_1 & 0xF];
    eproj_y_pos[v5] = 40;
    eproj_pre_instr[v5] = FUNC16(EprojPreInstr_PhantoonDestroyableFireballs_2);
    break;
  }
  case 6: {
    int v7 = j >> 1;
    eproj_x_subpos[v7] = 0;
    eproj_y_subpos[v7] = 0;
    eproj_y_vel[v7] = 0;
    eproj_x_vel[v7] = 128;
    eproj_E[v7] = byte_869979[(uint8)eproj_init_param_1];
    eproj_x_pos[v7] = enemy_data[0].x_pos;
    eproj_y_pos[v7] = enemy_data[0].y_pos + 16;
    eproj_pre_instr[v7] = FUNC16(EprojPreInstr_PhantoonDestroyableFireballs_3);
    break;
  }
  default:
    Unreachable();
  }
}

static void EprojInit_PhantoonStartingFireballs(uint16 j) {  // 0x86993A
  int v1 = j >> 1;
  eproj_x_subpos[v1] = 0;
  eproj_y_subpos[v1] = 0;
  eproj_x_vel[v1] = 0;
  eproj_y_vel[v1] = 0;
  uint16 v2 = byte_869979[eproj_init_param_1];
  eproj_E[v1] = v2;
  Point16U pt = Eproj_PhantomFireballs_Func1(v2, 0x30);
  eproj_x_pos[v1] = pt.x + enemy_data[0].x_pos;
  eproj_y_pos[v1] = pt.y + enemy_data[0].y_pos + 16;
}

void EprojPreInstr_PhantoonStartingFireballs(uint16 k) {  // 0x869981
  int v1 = k >> 1;
  eproj_y_vel[v1] += 16;
  if (EprojBlockCollisition_Vertical(k) & 1) {
    eproj_properties[v1] = eproj_properties[v1] & 0xFFF | 0x8000;
    eproj_pre_instr[v1] = FUNC16(EprojPreInstr_PhantoonStartingFireballs2);
    eproj_instr_list_ptr[v1] = addr_word_86976C;
    eproj_instr_timers[v1] = 1;
    eproj_E[v1] = 8;
    eproj_y_pos[v1] += 8;
  }
}

void EprojPreInstr_PhantoonStartingFireballs2(uint16 k) {  // 0x8699BF
  int v1 = k >> 1;
  bool v2 = eproj_E[v1] == 1;
  bool v3 = (--eproj_E[v1] & 0x8000) != 0;
  if (v2 || v3) {
    eproj_pre_instr[v1] = FUNC16(EprojPreInstr_PhantoonStartingFireballs3);
    eproj_instr_list_ptr[v1] = addr_word_869772;
    eproj_instr_timers[v1] = 1;
    eproj_y_pos[v1] -= 8;
    eproj_y_vel[v1] = -768;
    eproj_E[v1] = 0;
    if ((nmi_frame_counter_word & 1) != 0)
      eproj_x_vel[v1] = -128;
    else
      eproj_x_vel[v1] = 128;
  }
}

static const uint16 word_869A3E[3] = { 0xfd00, 0xfe00, 0xff00 };

void EprojPreInstr_PhantoonStartingFireballs3(uint16 k) {  // 0x869A01
  int v1 = k >> 1;
  eproj_y_vel[v1] += 16;
  if (EprojBlockCollisition_Vertical(k) & 1) {
    uint16 v2 = eproj_E[v1] + 1;
    eproj_E[v1] = v2;
    if (sign16(v2 - 3)) {
      eproj_y_vel[v1] = word_869A3E[v2];
      return;
    }
    goto LABEL_6;
  }
  if (EprojBlockCollisition_Horiz(k) & 1) {
LABEL_6:
    eproj_instr_list_ptr[v1] = addr_word_869782;
    eproj_instr_timers[v1] = 1;
    eproj_pre_instr[v1] = FUNC16(nullsub_86);
  }
}

void EprojPreInstr_PhantoonDestroyableFireballs(uint16 k) {  // 0x869A45
  int16 v4;
  uint16 v5;
  int v1 = k >> 1;
  eproj_y_vel[v1] += 4;
  uint16 v2 = (uint8)(LOBYTE(eproj_x_vel[v1]) + eproj_E[v1]);
  eproj_E[v1] = v2;
  Point16U pt = Eproj_PhantomFireballs_Func1(v2, eproj_y_vel[v1]);
  bool v3 = (int16)(pt.x + enemy_data[0].x_pos) < 0;
  v4 = pt.x + enemy_data[0].x_pos;
  eproj_x_pos[v1] = pt.x + enemy_data[0].x_pos;
  if (v3
      || !sign16(v4 - 256)
      || (v3 = (int16)(pt.y + enemy_data[0].y_pos + 16) < 0,
          v5 = pt.y + enemy_data[0].y_pos + 16,
          eproj_y_pos[v1] = v5,
          v3)
      || !sign16(v5 - 256)) {
    eproj_instr_list_ptr[v1] = addr_off_8697F8;
    eproj_instr_timers[v1] = 1;
  }
}

void EprojPreInstr_PhantoonDestroyableFireballs_2(uint16 k) {  // 0x869A94
  int v1 = k >> 1;
  if (!eproj_x_vel[v1])
    goto LABEL_5;
  bool v2, v3;
  v2 = eproj_x_vel[v1] == 1;
  v3 = (--eproj_x_vel[v1] & 0x8000) != 0;
  if (v2 || v3) {
    QueueSfx3_Max6(0x1D);
LABEL_5:
    eproj_y_vel[v1] += 16;
    if (EprojBlockCollisition_Vertical(k) & 1) {
      eproj_instr_list_ptr[v1] = addr_word_8697AC;
      eproj_instr_timers[v1] = 1;
      eproj_y_pos[v1] += 8;
      eproj_pre_instr[v1] = FUNC16(nullsub_86);
      QueueSfx3_Max6(0x1D);
    }
  }
}

void EprojPreInstr_PhantoonDestroyableFireballs_3(uint16 k) {  // 0x869ADA
  int16 v4;
  uint16 v5;
  int v1 = k >> 1;
  eproj_y_vel[v1] += 2;
  uint16 v2 = (uint8)(eproj_E[v1] + 2);
  eproj_E[v1] = v2;
  Point16U pt = Eproj_PhantomFireballs_Func1(v2, eproj_y_vel[v1]);
  bool v3 = (int16)(pt.x + enemy_data[0].x_pos) < 0;
  v4 = pt.x + enemy_data[0].x_pos;
  eproj_x_pos[v1] = pt.x + enemy_data[0].x_pos;
  if (v3
      || !sign16(v4 - 256)
      || (v3 = (int16)(pt.y + enemy_data[0].y_pos + 16) < 0,
          v5 = pt.y + enemy_data[0].y_pos + 16,
          eproj_y_pos[v1] = v5,
          v3)
      || !sign16(v5 - 256)) {
    eproj_instr_list_ptr[v1] = addr_off_8697F8;
    eproj_instr_timers[v1] = 1;
  }
}

void EprojPreInstr_PhantoonStartingFireballsB(uint16 k) {  // 0x869B29
  if (enemy_data[0].ai_var_B) {
    int v1 = k >> 1;
    eproj_pre_instr[v1] = FUNC16(EprojPreInstr_PhantoonStartingFireballsB_2);
    eproj_x_vel[v1] = 180;
    eproj_y_vel[v1] = 48;
  }
}

void EprojPreInstr_PhantoonStartingFireballsB_2(uint16 k) {  // 0x869B41
  int v1 = k >> 1;
  uint16 v2 = eproj_x_vel[v1];
  if (v2) {
    eproj_x_vel[v1] = v2 - 1;
LABEL_5:;
    uint8 v4 = eproj_E[v1] + 1;
    eproj_E[v1] = v4;
    Point16U pt = Eproj_PhantomFireballs_Func1(v4, eproj_y_vel[v1]);
    eproj_x_pos[v1] = pt.x + enemy_data[0].x_pos;
    eproj_y_pos[v1] = pt.y + enemy_data[0].y_pos + 16;
    return;
  }
  if ((nmi_frame_counter_word & 1) == 0)
    goto LABEL_5;
  uint16 v3 = eproj_y_vel[v1] - 1;
  eproj_y_vel[v1] = v3;
  if (v3)
    goto LABEL_5;
  eproj_x_pos[v1] = enemy_data[0].x_pos;
  eproj_y_pos[v1] = enemy_data[0].y_pos + 16;
  eproj_instr_timers[v1] = 1;
  eproj_instr_list_ptr[v1] = addr_off_8697F8;
}

Point16U Eproj_PhantomFireballs_Func1(uint16 j, uint16 a) {  // 0x869BA2
  int16 v3;
  uint16 v2, v4;

  uint16 r24 = a;
  uint16 r26 = j;
  if (sign16(j - 128))
    v2 = Eproj_PhantomFireballs_Func2(2 * j, r24);
  else
    v2 = -Eproj_PhantomFireballs_Func2(2 * (uint8)(j + 0x80), r24);
  uint16 r20 = v2;
  v3 = (uint8)(r26 - 64);
  if (sign16(v3 - 128))
    v4 = Eproj_PhantomFireballs_Func2(2 * v3, r24);
  else
    v4 = -Eproj_PhantomFireballs_Func2(2 * (uint8)(v3 + 0x80), r24);
  uint16 r22 = v4;
  return (Point16U) { r20, r22 };
}


uint16 Eproj_PhantomFireballs_Func2(uint16 k, uint16 r24) {  // 0x869BF3
  uint16 r18 = Mult8x8(*((uint8 *)&kSinCosTable8bit_Sext[64] + k), r24) >> 8;
  return r18 + Mult8x8(*((uint8 *)&kSinCosTable8bit_Sext[64] + k + 1), r24);
}

static void EprojInit_WalkingLavaSeahorseFireball(uint16 j) {  // 0x869EB2
  static const int16 word_869EF9[3] = { -0x100, 0, 0x100 };

  EnemyData *v1 = gEnemyData(cur_enemy_index);
  int v2 = j >> 1;
  eproj_y_pos[v2] = v1->y_pos - 12;
  eproj_x_vel[v2] = -1024;
  eproj_x_pos[v2] = v1->x_pos - 16;
  if ((v1->ai_var_D & 0x8000) == 0) {
    eproj_x_vel[v2] = 1024;
    eproj_x_pos[v2] = v1->x_pos + 16;
  }
  eproj_y_vel[v2] = word_869EF9[eproj_init_param_1 >> 1];
  eproj_y_subpos[v2] = 0;
  eproj_x_subpos[v2] = 0;
}

void EprojPreInstr_WalkingLavaSeahorseFireball(uint16 k) {  // 0x869EFF
  uint16 v2;
  if (EprojBlockCollisition_Vertical(k) & 1 || EprojBlockCollisition_Horiz(k) & 1) {
    eproj_id[k >> 1] = 0;
  } else {
    int v1 = k >> 1;
    if ((eproj_x_vel[v1] & 0x8000) == 0) {
      v2 = eproj_x_vel[v1] - 64;
      eproj_x_vel[v1] = v2;
      if (sign16(v2 - 512))
        v2 = 512;
    } else {
      v2 = eproj_x_vel[v1] + 64;
      eproj_x_vel[v1] = v2;
      if (!sign16(v2 + 512))
        v2 = -512;
    }
    eproj_x_vel[v1] = v2;
  }
}

const uint8 *EprojInstr_GotoWithProbability25(uint16 k, const uint8 *epjp) {  // 0x86A456
  if ((NextRandom() & 0xC000) == 0xC000)
    return INSTRB_RETURN_ADDR(GET_WORD(epjp));
  else
    return epjp + 2;
}

static void EprojInit_BombTorizoLowHealthDrool(uint16 j) {  // 0x86A5D3
  int16 parameter_1; // dx
  uint16 v4, v5, v8;
  int v1 = j >> 1;
  eproj_gfx_idx[v1] = 0;
  eproj_instr_list_ptr[v1] = off_86A64D[(uint16)((NextRandom() >> 1) & 0xE) >> 1];
  NextRandom();
  EnemyData *v2 = gEnemyData(cur_enemy_index);
  eproj_y_pos[v1] = v2->y_pos - 5;
  parameter_1 = v2->parameter_1;
  if ((parameter_1 & 0x4000) != 0) {
    v4 = random_number & 0x1FE;
  } else {
    if (parameter_1 < 0)
      v5 = 32;
    else
      v5 = 224;
    v4 = 2 * (v5 + (random_number & 0xF) - 8);
  }
  int v6 = v4 >> 1;
  eproj_x_vel[v1] = kSinCosTable8bit_Sext[v6 + 64];
  eproj_y_vel[v1] = kSinCosTable8bit_Sext[v6];
  EnemyData *v7 = gEnemyData(cur_enemy_index);
  if ((v7->parameter_1 & 0x8000) != 0)
    v8 = v7->x_pos + 8;
  else
    v8 = v7->x_pos - 8;
  eproj_x_pos[v1] = v8;
}

static void EprojInit_BombTorizoLowHealthInitialDrool(uint16 j) {  // 0x86A65D
  int16 v3;
  int16 parameter_1; // dx

  int v1 = j >> 1;
  eproj_gfx_idx[v1] = 0;
  NextRandom();
  EnemyData *v2 = gEnemyData(cur_enemy_index);
  eproj_y_pos[v1] = v2->y_pos + (random_number & 3) - 5;
  eproj_y_vel[v1] = (random_number & 0x1F) + 48;
  NextRandom();
  v3 = random_number & 3;
  parameter_1 = v2->parameter_1;
  if ((parameter_1 & 0x4000) != 0) {
    eproj_x_pos[v1] = v2->x_pos + v3;
    eproj_x_vel[v1] = 0;
  } else {
    if (parameter_1 < 0)
      eproj_x_pos[v1] = v2->x_pos + v3 + 8;
    else
      eproj_x_pos[v1] = v2->x_pos + v3 - 8;
    eproj_x_vel[v1] = 0;
  }
}

static void EprojInit_A977(uint16 j) {  // 0x86A6C7
  int v1 = j >> 1;
  eproj_gfx_idx[v1] = 0;
  CalculatePlmBlockCoords(plm_id);
  eproj_x_pos[v1] = 16 * plm_x_block;
  eproj_y_pos[v1] = 16 * plm_y_block - 4;
  eproj_x_subpos[v1] = 0;
  eproj_y_subpos[v1] = 0;
}

static const int16 kEprojInit_BombTorizoExplosiveSwipe_Tab0[11] = { -30, -40, -47, -31, -21, -1, -28, -43, -48, -31, -21 };
static const int16 kEprojInit_BombTorizoExplosiveSwipe_Tab1[11] = { -52, -28, -11, 9, 21, 20, -52, -27, -10, 9, 20 };
static const int16 kEprojInit_BombTorizoStatueBreaking_Xpos[16] = { 8, 0x18, -8, 8, 0x18, -8, 8, 0x18, 8, -8, 0x18, 8, -8, 0x18, 8, -8 };
static const int16 kEprojInit_BombTorizoStatueBreaking_Ypos[8] = { -8, -8, 8, 8, 8, 0x18, 0x18, 0x18 };
static const int16 kEprojInit_BombTorizoStatueBreaking_Yvel[8] = { 256, 256, 256, 256, 256, 256, 256, 256 };
static const int16 kEprojInit_BombTorizoStatueBreaking_F[8] = { 16, 16, 16, 16, 16, 16, 16, 16 };
static void EprojInit_BombTorizoExplosiveSwipe(uint16 j) {  // 0x86A6F6
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  if ((v1->parameter_1 & 0x8000) != 0) {
    int v4 = eproj_init_param_1 >> 1;
    int v5 = j >> 1;
    eproj_x_pos[v5] = v1->x_pos - kEprojInit_BombTorizoExplosiveSwipe_Tab0[v4];
    eproj_y_pos[v5] = v1->y_pos + kEprojInit_BombTorizoExplosiveSwipe_Tab1[v4];
  } else {
    int v2 = eproj_init_param_1 >> 1;
    int v3 = j >> 1;
    eproj_x_pos[v3] = v1->x_pos + kEprojInit_BombTorizoExplosiveSwipe_Tab0[v2];
    eproj_y_pos[v3] = v1->y_pos + kEprojInit_BombTorizoExplosiveSwipe_Tab1[v2];
  }
}

static void EprojInit_BombTorizoStatueBreaking(uint16 j) {  // 0x86A764
  int8 v1;

  CalculatePlmBlockCoords(plm_id);
  v1 = eproj_init_param_1;
  int v2 = eproj_init_param_1 >> 1;
  int v3 = j >> 1;
  eproj_instr_list_ptr[v3] = kEprojInit_BombTorizoStatueBreaking_InstrList[v2];
  eproj_x_pos[v3] = kEprojInit_BombTorizoStatueBreaking_Xpos[v2] + 16 * plm_x_block;
  int v4 = (v1 & 0xF) >> 1;
  eproj_y_pos[v3] = kEprojInit_BombTorizoStatueBreaking_Ypos[v4] + 16 * plm_y_block;
  eproj_y_vel[v3] = kEprojInit_BombTorizoStatueBreaking_Yvel[v4];
  eproj_F[v3] = kEprojInit_BombTorizoStatueBreaking_F[v4];
  eproj_properties[v3] |= 0x1000;
}

void sub_86A887(uint16 v0) {  // 0x86A887
  int16 v2;
  int16 v3;
  int16 v4;

  if (EprojBlockCollisition_Horiz(v0) & 1) {
    int v6 = v0 >> 1;
    eproj_instr_list_ptr[v6] = 0xA48A;
    eproj_instr_timers[v6] = 1;
  } else {
    int v1 = v0 >> 1;
    v2 = eproj_x_vel[v1];
    if (v2 >= 0) {
      v4 = v2 - 4;
      if (v4 < 0)
        v4 = 3;
      eproj_x_vel[v1] = v4;
    } else {
      v3 = v2 + 4;
      if (v3 >= 0)
        v3 = 3;
      eproj_x_vel[v1] = v3;
    }
    uint8 carry = EprojBlockCollisition_Vertical(v0);
    if ((eproj_y_vel[v1] & 0x8000) != 0 || !carry) {
      uint16 v5 = eproj_y_vel[v1] + 16;
      eproj_y_vel[v1] = v5;
      if ((v5 & 0xF000) == 4096)
        eproj_id[v1] = 0;
    } else {
      int v7 = v0 >> 1;
      eproj_y_pos[v7] -= 3;
      eproj_instr_list_ptr[v7] = 0xA48E;
      eproj_instr_timers[v7] = 1;
    }
  }
}

void EprojPreInstr_A977(uint16 k) {  // 0x86A8EF
  uint8 carry = EprojBlockCollisition_Vertical(k);
  int v1 = k >> 1;
  if ((eproj_y_vel[v1] & 0x8000) != 0 || !carry) {
    uint16 v2 = eproj_F[0] + eproj_y_vel[v1];
    eproj_y_vel[v1] = v2;
    if ((v2 & 0xF000) == 4096)
      eproj_y_vel[v1] = 4096;
  } else {
    eproj_pre_instr[v1] = 0xA918;
  }
}

void sub_86A91A(uint16 v0) {  // 0x86A91A
  int v1 = v0 >> 1;
  eproj_x_vel[v1] = 0;
  if ((joypad2_last & 0x100) != 0)
    eproj_x_vel[v1] = 256;
  if ((joypad2_last & 0x200) != 0)
    eproj_x_vel[v1] = -256;
  EprojBlockCollisition_Horiz(v0);
  eproj_y_vel[v1] = 0;
  if ((joypad2_last & 0x400) != 0)
    eproj_y_vel[v1] = 256;
  if ((joypad2_last & 0x800) != 0)
    eproj_y_vel[v1] = -256;
  EprojBlockCollisition_Vertical(v0);
}

static void EprojInit_AB07(uint16 j) {  // 0x86AA3D
  uint16 v1 = vram_write_queue_tail;
  VramWriteEntry *v2 = gVramWriteEntry(vram_write_queue_tail);
  v2->size = 64;
  v2->src.addr = addr_kEprojInit_AB07_Tile0;
  *(uint16 *)&v2->src.bank = 134;
  v2->vram_dst = 28160;
  v1 += 7;
  VramWriteEntry *v3 = gVramWriteEntry(v1);
  v3->size = 64;
  v3->src.addr = addr_kEprojInit_AB07_Tile1;
  *(uint16 *)&v3->src.bank = 134;
  v3->vram_dst = 28416;
  vram_write_queue_tail = v1 + 7;
  int v4 = j >> 1;
  eproj_x_pos[v4] = samus_x_pos;
  eproj_y_pos[v4] = samus_y_pos - 36;
}

void EprojPreInstr_AB07(uint16 k) {  // 0x86AA8C
  int v1 = k >> 1;
  int32 amt = INT16_SHL8(8 * (samus_x_pos - eproj_x_pos[v1]));
  AddToHiLo(&eproj_x_pos[v1], &eproj_x_subpos[v1], amt);
  
  amt = INT16_SHL8(8 * (samus_y_pos - 36 - eproj_y_pos[v1]));
  AddToHiLo(&eproj_y_pos[v1], &eproj_y_subpos[v1], amt);
}

const uint8 *EprojInstr_SpawnEnemyDrops(uint16 k, const uint8 *epjp) {  // 0x86AB8A
  int v2 = k >> 1;
  eproj_spawn_pt = (Point16U) { eproj_x_pos[v2], eproj_y_pos[v2] };
  SpawnEnemyDrops(area_index ? GET_WORD(epjp + 2) : GET_WORD(epjp), k, 0);
  return epjp + 4;
}

static void EprojInit_WreckedShipChozoSpikeFootsteps(uint16 j) {  // 0x86AEFC
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  int v2 = j >> 1;
  eproj_x_pos[v2] = eproj_init_param_1 + v1->x_pos;
  eproj_y_pos[v2] = v1->y_pos + 28;
}

const uint8 *EprojInstr_ResetXYpos1(uint16 k, const uint8 *epjp) {  // 0x86AF36
  int v2 = k >> 1;
  eproj_x_pos[v2] = eproj_E[v2];
  eproj_y_pos[v2] = eproj_F[v2];
  return epjp;
}

static void EprojInit_TourianStatueDustClouds(uint16 j) {  // 0x86AF43
  int v1 = j >> 1;
  eproj_E[v1] = 128;
  eproj_F[v1] = 188;
}

static void EprojInit_TourianLandingDustCloudsRightFoot(uint16 j) {  // 0x86AF50
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  int v2 = j >> 1;
  eproj_y_pos[v2] = v1->y_pos + 48;
  eproj_x_pos[v2] = v1->x_pos + 24;
}

const uint8 *EprojInstr_MoveY_Minus4(uint16 k, const uint8 *epjp) {  // 0x86AF92
  eproj_y_pos[k >> 1] -= 4;
  return epjp;
}

const uint8 *EprojInstr_GotoIfFunc1(uint16 k, const uint8 *epjp) {  // 0x86B3B8
  if ((gExtraEnemyRam7800(eproj_F[k >> 1])->kraid.kraid_healths_8ths[0] & 0x8000) == 0)
    return INSTRB_RETURN_ADDR(GET_WORD(epjp));
  else
    return epjp + 2;
}

const uint8 *EprojInstr_ResetXYpos2(uint16 k, const uint8 *epjp) {  // 0x86B436
  int v2 = k >> 1;
  eproj_x_pos[v2] = eproj_E[v2];
  eproj_y_pos[v2] = eproj_F[v2];
  return epjp;
}

const uint8 *EprojInstr_SpawnTourianStatueUnlockingParticle(uint16 k, const uint8 *epjp) {  // 0x86B7EA
  SpawnEprojWithRoomGfx(addr_kEproj_TourianStatueUnlockingParticle, k);
  return epjp;
}

void sub_86BB30(uint16 j) {  // 0x86BB30
  ExtraEnemyRam8000 *v1 = gExtraEnemyRam8000(cur_enemy_index);
  int v2 = j >> 1;
  eproj_x_pos[v2] = *(uint16 *)&v1->pad[34];
  eproj_y_pos[v2] = *(uint16 *)&v1->pad[36];
  eproj_instr_list_ptr[v2] = off_86BB1E[eproj_init_param_1];
}

const uint8 *EprojInstr_SetYVel(uint16 k, const uint8 *epjp) {  // 0x86E533
  eproj_y_vel[k >> 1] = GET_WORD(epjp);
  return epjp + 2;
}

uint16 Eproj_FuncE722(uint16 k) {  // 0x86E722
  int16 v2;
  int16 v3;

  int v1 = k >> 1;
  v2 = eproj_x_pos[v1];
  uint16 result = 1;
  if (v2 >= 0) {
    if (sign16(v2 - 512)) {
      v3 = eproj_y_pos[v1];
      if (v3 >= 0) {
        if (sign16(v3 - 512))
          return 0;
      }
    }
  }
  return result;
}

void Eproj_FuncE73E_MoveXY(uint16 k) {  // 0x86E73E
  int v1 = k >> 1;
  if (((g_word_7E97DC[v1] + 64) & 0x80) != 0) {
    AddToHiLo(&eproj_x_pos[v1], &eproj_x_subpos[v1], -IPAIR32(eproj_x_vel[v1], eproj_E[v1]));
  } else {
    AddToHiLo(&eproj_x_pos[v1], &eproj_x_subpos[v1], __PAIR32__(eproj_x_vel[v1], eproj_E[v1]));
  }
  if (((g_word_7E97DC[v1] + 128) & 0x80) != 0) {
    AddToHiLo(&eproj_y_pos[v1], &eproj_y_subpos[v1], -IPAIR32(eproj_y_vel[v1], eproj_F[v1]));
  } else {
    AddToHiLo(&eproj_y_pos[v1], &eproj_y_subpos[v1], __PAIR32__(eproj_y_vel[v1], eproj_F[v1]));
  }
}

void CallEprojInit(uint32 ea, uint16 j) {
  switch (ea) {
  case fnEproj_Init_0x8aaf: Eproj_Init_0x8aaf(j); return;
  case fnEproj_Init_0x8bc2_SkreeDownRight: Eproj_Init_0x8bc2_SkreeDownRight(j); return;
  case fnEproj_Init_0x8bd0_SkreeUpRight: Eproj_Init_0x8bd0_SkreeUpRight(j); return;
  case fnEproj_Init_0x8bde_SkreeDownLeft: Eproj_Init_0x8bde_SkreeDownLeft(j); return;
  case fnEproj_Init_0x8bec_SkreeUpLeft: Eproj_Init_0x8bec_SkreeUpLeft(j); return;
  case fnEprojInit_DraygonsGunk: EprojInit_DraygonsGunk(j); return;
  case fnEprojInit_DraygonsWallTurretProjs: EprojInit_DraygonsWallTurretProjs(j); return;
  case fnEprojInit_CrocomireProjectile: EprojInit_CrocomireProjectile(j); return;
  case fnEprojInit_CrocomireSpikeWallPieces: EprojInit_CrocomireSpikeWallPieces(j); return;
  case fnEprojInit_CrocomireBridgeCrumbling: EprojInit_CrocomireBridgeCrumbling(j); return;
  case fnEprojInit_9634: EprojInit_9634(j); return;
  case fnEprojInit_9642_RidleysFireball: EprojInit_9642_RidleysFireball(j); return;
  case fnEprojInit_9660_FireballExplosion: EprojInit_9660_FireballExplosion(j); return;
  case fnEprojInit_9688: EprojInit_9688(j); return;
  case fnEprojInit_9696: EprojInit_9696(j); return;
  case fnEprojInit_966C: EprojInit_966C(j); return;
  case fnEprojInit_967A: EprojInit_967A(j); return;
  case fnEprojInit_9734_CeresFallingDebris: EprojInit_9734_CeresFallingDebris(j); return;
  case fnEprojInit_PhantoonDestroyableFireballs: EprojInit_PhantoonDestroyableFireballs(j); return;
  case fnEprojInit_PhantoonStartingFireballs: EprojInit_PhantoonStartingFireballs(j); return;
  case fnEprojInit_RocksKraidSpits: EprojInit_RocksKraidSpits(j); return;
  case fnEprojInit_RocksFallingKraidCeiling: EprojInit_RocksFallingKraidCeiling(j); return;
  case fnEprojInit_RocksWhenKraidRises: EprojInit_RocksWhenKraidRises(j); return;
  case fnEprojInit_MiniKraidSpit: EprojInit_MiniKraidSpit(j); return;
  case fnEprojInit_MiniKraidSpikesLeft: EprojInit_MiniKraidSpikesLeft(j); return;
  case fnEprojInit_MiniKraidSpikesRight: EprojInit_MiniKraidSpikesRight(j); return;
  case fnEprojInit_WalkingLavaSeahorseFireball: EprojInit_WalkingLavaSeahorseFireball(j); return;
  case fnEprojInit_PirateMotherBrainLaser: EprojInit_PirateMotherBrainLaser(j); return;
  case fnEprojInit_PirateClaw: EprojInit_PirateClaw(j); return;
  case fnEprojInit_A379: EprojInit_A379(j); return;
  case fnEprojInit_CeresElevatorPad: EprojInit_CeresElevatorPad(j); return;
  case fnEprojInit_CeresElevatorPlatform: EprojInit_CeresElevatorPlatform(j); return;
  case fnEprojPreInstr_PrePhantomRoom: EprojPreInstr_PrePhantomRoom(j); return;
  case fnEprojInit_BombTorizoLowHealthDrool: EprojInit_BombTorizoLowHealthDrool(j); return;
  case fnEprojInit_BombTorizoLowHealthInitialDrool: EprojInit_BombTorizoLowHealthInitialDrool(j); return;
  case fnEprojInit_A977: EprojInit_A977(j); return;
  case fnEprojInit_BombTorizoExplosiveSwipe: EprojInit_BombTorizoExplosiveSwipe(j); return;
  case fnEprojInit_BombTorizoStatueBreaking: EprojInit_BombTorizoStatueBreaking(j); return;
  case fnEprojInit_BombTorizoLowHealthExplode: EprojInit_BombTorizoLowHealthExplode(j); return;
  case fnEprojInit_BombTorizoDeathExplosion: EprojInit_BombTorizoDeathExplosion(j); return;
  case fnEprojInit_AB07: EprojInit_AB07(j); return;
  case fnEprojInit_BombTorizosChozoOrbs: EprojInit_BombTorizosChozoOrbs(j); return;
  case fnEprojInit_GoldenTorizosChozoOrbs: EprojInit_GoldenTorizosChozoOrbs(j); return;
  case fnEprojInit_TorizoSonicBoom: EprojInit_TorizoSonicBoom(j); return;
  case fnEprojInit_WreckedShipChozoSpikeFootsteps: EprojInit_WreckedShipChozoSpikeFootsteps(j); return;
  case fnEprojInit_TourianStatueDustClouds: EprojInit_TourianStatueDustClouds(j); return;
  case fnEprojInit_TourianLandingDustCloudsRightFoot: EprojInit_TourianLandingDustCloudsRightFoot(j); return;
  case fnEprojInit_TorizoLandingDustCloudLeftFoot: EprojInit_TorizoLandingDustCloudLeftFoot(j); return;
  case fnEprojInit_GoldenTorizoEgg: EprojInit_GoldenTorizoEgg(j); return;
  case fnEprojInit_GoldenTorizoSuperMissile: EprojInit_GoldenTorizoSuperMissile(j); return;
  case fnEprojInit_GoldenTorizoEyeBeam: EprojInit_GoldenTorizoEyeBeam(j); return;
  case fnEprojInit_TourianEscapeShaftFakeWallExplode: EprojInit_TourianEscapeShaftFakeWallExplode(j); return;
  case fnEprojInit_LavaSeahorseFireball: EprojInit_LavaSeahorseFireball(j); return;
  case fnEprojInit_EyeDoorProjectile: EprojInit_EyeDoorProjectile(j); return;
  case fnEprojInit_EyeDoorSweat: EprojInit_EyeDoorSweat(j); return;
  case fnEprojInit_TourianStatueUnlockingParticleWaterSplash: EprojInit_TourianStatueUnlockingParticleWaterSplash(j); return;
  case fnEprojInit_TourianStatueEyeGlow: EprojInit_TourianStatueEyeGlow(j); return;
  case fnEprojInit_TourianStatueUnlockingParticle: EprojInit_TourianStatueUnlockingParticle(j); return;
  case fnEprojIni_TourianStatueUnlockingParticleTail: EprojIni_TourianStatueUnlockingParticleTail(j); return;
  case fnEprojInit_TourianStatueSoul: EprojInit_TourianStatueSoul(j); return;
  case fnEprojInit_TourianStatueBaseDecoration: EprojInit_TourianStatueBaseDecoration(j); return;
  case fnEprojInit_TourianStatueRidley: EprojInit_TourianStatueRidley(j); return;
  case fnEprojInit_TourianStatuePhantoon: EprojInit_TourianStatuePhantoon(j); return;
  case fnsub_86BB30: sub_86BB30(j); return;
  case fnEprojInit_NuclearWaffleBody: EprojInit_NuclearWaffleBody(j); return;
  case fnEprojInit_NorfairLavaquakeRocks: EprojInit_NorfairLavaquakeRocks(j); return;
  case fnEprojInit_ShaktoolAttackMiddleBackCircle: EprojInit_ShaktoolAttackMiddleBackCircle(j); return;
  case fnEprojInit_BDA2: EprojInit_BDA2(j); return;
  case fnEprojInit_MotherBrainRoomTurrets: EprojInit_MotherBrainRoomTurrets(j); return;
  case fnEprojInit_MotherBrainRoomTurretBullets: EprojInit_MotherBrainRoomTurretBullets(j); return;
  case fnEproj_MotherBrainsBlueRingLasers: Eproj_MotherBrainsBlueRingLasers(j); return;
  case fnEprojInit_MotherBrainBomb: EprojInit_MotherBrainBomb(j); return;
  case fnsub_86C605: sub_86C605(j); return;
  case fnEprojInit_MotherBrainDeathBeemFired: EprojInit_MotherBrainDeathBeemFired(j); return;
  case fnEprojInit_MotherBrainRainbowBeam: EprojInit_MotherBrainRainbowBeam(j); return;
  case fnEprojInit_MotherBrainsDrool: EprojInit_MotherBrainsDrool(j); return;
  case fnEprojInit_MotherBrainsDeathExplosion: EprojInit_MotherBrainsDeathExplosion(j); return;
  case fnEprojInit_MotherBrainsRainbowBeamExplosion: EprojInit_MotherBrainsRainbowBeamExplosion(j); return;
  case fnEprojInit_MotherBrainEscapeDoorParticles: EprojInit_MotherBrainEscapeDoorParticles(j); return;
  case fnEprojInit_MotherBrainPurpleBreathBig: EprojInit_MotherBrainPurpleBreathBig(j); return;
  case fnEprojInit_MotherBrainPurpleBreathSmall: EprojInit_MotherBrainPurpleBreathSmall(j); return;
  case fnEprojInit_TimeBombSetJapaneseText: EprojInit_TimeBombSetJapaneseText(j); return;
  case fnEprojInit_MotherBrainTubeFalling: EprojInit_MotherBrainTubeFalling(j); return;
  case fnEprojInit_MotherBrainGlassShatteringShard: EprojInit_MotherBrainGlassShatteringShard(j); return;
  case fnEprojInit_MotherBrainGlassShatteringSparkle: EprojInit_MotherBrainGlassShatteringSparkle(j); return;
  case fnEprojInit_KiHunterAcidSpitLeft: EprojInit_KiHunterAcidSpitLeft(j); return;
  case fnEprojInit_KiHunterAcidSpitRight: EprojInit_KiHunterAcidSpitRight(j); return;
  case fnEprojInit_KagosBugs: EprojInit_KagosBugs(j); return;
  case fnEprojInit_MaridiaFloatersSpikes: EprojInit_MaridiaFloatersSpikes(j); return;
  case fnEprojInit_WreckedShipRobotLaserDown: EprojInit_WreckedShipRobotLaserDown(j); return;
  case fnEprojInit_WreckedShipRobotLaserHorizontal: EprojInit_WreckedShipRobotLaserHorizontal(j); return;
  case fnEprojInit_WreckedShipRobotLaserUp: EprojInit_WreckedShipRobotLaserUp(j); return;
  case fnEprojInit_N00bTubeCrack: EprojInit_N00bTubeCrack(j); return;
  case fnEprojInit_N00bTubeShards: EprojInit_N00bTubeShards(j); return;
  case fnEprojInit_N00bTubeReleasedAirBubbles: EprojInit_N00bTubeReleasedAirBubbles(j); return;
  case fnsub_86D992: sub_86D992(j); return;
  case fnEprojInit_DBF2: EprojInit_DBF2(j); return;
  case fnEprojInit_Spores: EprojInit_Spores(j); return;
  case fnEprojInit_SporeSpawnStalk: EprojInit_SporeSpawnStalk(j); return;
  case fnEprojInit_SporeSpawners: EprojInit_SporeSpawners(j); return;
  case fnEprojInit_NamiFuneFireball: EprojInit_NamiFuneFireball(j); return;
  case fnEprojInit_LavaThrownByLavaman: EprojInit_LavaThrownByLavaman(j); return;
  case fnEprojInit_DustCloudOrExplosion: EprojInit_DustCloudOrExplosion(j); return;
  case fnEprojInit_EyeDoorSmoke: EprojInit_EyeDoorSmoke(j); return;
  case fnEprojInit_SpawnedShotGate: EprojInit_SpawnedShotGate(j); return;
  case fnEprojInit_ClosedDownwardsShotGate: EprojInit_ClosedDownwardsShotGate(j); return;
  case fnEprojInit_ClosedUpwardsShotGate: EprojInit_ClosedUpwardsShotGate(j); return;
  case fnEprojInit_SaveStationElectricity: EprojInit_SaveStationElectricity(j); return;
  case fnEprojInit_BotwoonsBody: EprojInit_BotwoonsBody(j); return;
  case fnEprojInit_BotwoonsSpit: EprojInit_BotwoonsSpit(j); return;
  case fnEprojInit_YappingMawsBody: EprojInit_YappingMawsBody(j); return;
  case fnEprojInit_F337: EprojInit_F337(j); return;
  case fnEprojInit_EnemyDeathExplosion: EprojInit_EnemyDeathExplosion(j); return;
  case fnEprojInit_Sparks: EprojInit_Sparks(j); return;
  default: Unreachable();
  }
}

void CallEprojPreInstr(uint32 ea, uint16 k) {
  switch (ea) {
  case fnnullsub_352: return;
  case fnEprojPreInstr_nullsub_297: return;
  case fnEprojPreInstr_nullsub_83: return;
  case fnEproj_PreInit_0x8aaf: Eproj_PreInit_0x8aaf(k); return;
  case fnEproj_PreInstr_SkreeParticle: Eproj_PreInstr_SkreeParticle(k); return;
  case fnnullsub_84: return;
  case fnEprojPreInstr_8DCA: EprojPreInstr_8DCA(k); return;
  case fnEprojPreInstr_DraygonsTurret_8DFF: EprojPreInstr_DraygonsTurret_8DFF(k); return;
  case fnEprojPreInstr_DraygonsGunk_8E0F: EprojPreInstr_DraygonsGunk_8E0F(k); return;
  case fnEprojPreInstr_CrocomireProjectile: EprojPreInstr_CrocomireProjectile(k); return;
  case fnsub_8690B3: sub_8690B3(k); return;
  case fnEprojPreInstr_CrocomireSpikeWallPieces: EprojPreInstr_CrocomireSpikeWallPieces(k); return;
  case fnEprojPreInstr_CrocomireBridgeCrumbling: EprojPreInstr_CrocomireBridgeCrumbling(k); return;
  case fnEprojPreInstr_9634: EprojPreInstr_9634(k); return;
  case fnEprojPreInstr_9642_RidleysFireball: EprojPreInstr_9642_RidleysFireball(k); return;
  case fnnullsub_85: return;
  case fnEprojPreInstr_966C: EprojPreInstr_966C(k); return;
  case fnEprojPreInstr_9688: EprojPreInstr_9688(k); return;
  case fnEprojPreInstr_96A4: EprojPreInstr_96A4(k); return;
  case fnEprojPreInstr_96C0: EprojPreInstr_96C0(k); return;
  case fnEprojPreInstr_96CE: EprojPreInstr_96CE(k); return;
  case fnEprojPreInstr_9734_CeresFallingDebris: EprojPreInstr_9734_CeresFallingDebris(k); return;
  case fnEprojPreInstr_PhantoonStartingFireballs: EprojPreInstr_PhantoonStartingFireballs(k); return;
  case fnEprojPreInstr_PhantoonStartingFireballs2: EprojPreInstr_PhantoonStartingFireballs2(k); return;
  case fnEprojPreInstr_PhantoonStartingFireballs3: EprojPreInstr_PhantoonStartingFireballs3(k); return;
  case fnnullsub_86: return;
  case fnEprojPreInstr_PhantoonDestroyableFireballs: EprojPreInstr_PhantoonDestroyableFireballs(k); return;
  case fnEprojPreInstr_PhantoonDestroyableFireballs_2: EprojPreInstr_PhantoonDestroyableFireballs_2(k); return;
  case fnEprojPreInstr_PhantoonDestroyableFireballs_3: EprojPreInstr_PhantoonDestroyableFireballs_3(k); return;
  case fnEprojPreInstr_PhantoonStartingFireballsB: EprojPreInstr_PhantoonStartingFireballsB(k); return;
  case fnEprojPreInstr_PhantoonStartingFireballsB_2: EprojPreInstr_PhantoonStartingFireballsB_2(k); return;
  case fnEprojPreInstr_KraidRocks: EprojPreInstr_KraidRocks(k); return;
  case fnEprojPreInstr_RocksFallingKraidCeiling: EprojPreInstr_RocksFallingKraidCeiling(k); return;
  case fnsub_869DA5: sub_869DA5(k); return;
  case fnEprojPreInit_MiniKraidSpit: EprojPreInit_MiniKraidSpit(k); return;
  case fnEprojPreInstr_MiniKraidSpikes: EprojPreInstr_MiniKraidSpikes(k); return;
  case fnEprojPreInstr_WalkingLavaSeahorseFireball: EprojPreInstr_WalkingLavaSeahorseFireball(k); return;
  case fnnullsub_87: return;
  case fnEprojPreInstr_PirateMotherBrainLaser_MoveLeft: EprojPreInstr_PirateMotherBrainLaser_MoveLeft(k); return;
  case fnEprojPreInstr_PirateMotherBrainLaser_MoveRight: EprojPreInstr_PirateMotherBrainLaser_MoveRight(k); return;
  case fnEprojPreInstr_PirateClawThrownLeft: EprojPreInstr_PirateClawThrownLeft(k); return;
  case fnEprojPreInstr_PirateClawThrownRight: EprojPreInstr_PirateClawThrownRight(k); return;
  case fnnullsub_88: return;
  case fnEprojPreInstr_CeresElevatorPad: EprojPreInstr_CeresElevatorPad(k); return;
  case fnEprojPreInstr_CeresElevatorPlatform: EprojPreInstr_CeresElevatorPlatform(k); return;
  case fnEprojPreInstr_PrePhantomRoom: EprojPreInstr_PrePhantomRoom(k); return;
  case fnsub_86A887: sub_86A887(k); return;
  case fnEprojPreInstr_A977: EprojPreInstr_A977(k); return;
  case fnnullsub_89: return;
  case fnEprojPreInstr_AB07: EprojPreInstr_AB07(k); return;
  case fnEprojPreInstr_BombTorizosChozoOrbs: EprojPreInstr_BombTorizosChozoOrbs(k); return;
  case fnEprojPreInstr_GoldenTorizosChozoOrbs: EprojPreInstr_GoldenTorizosChozoOrbs(k); return;
  case fnEprojPreInstr_TorizoSonicBoom: EprojPreInstr_TorizoSonicBoom(k); return;
  case fnEprojPreInstr_GoldenTorizoEgg: EprojPreInstr_GoldenTorizoEgg(k); return;
  case fnsub_86B0B9: sub_86B0B9(k); return;
  case fnsub_86B0DD: sub_86B0DD(k); return;
  case fnEprojPreInstr_GoldenTorizoSuperMissile: EprojPreInstr_GoldenTorizoSuperMissile(k); return;
  case fnEprojPreInstr_B237: EprojPreInstr_B237(k); return;
  case fnEprojPreInstr_GoldenTorizoEyeBeam: EprojPreInstr_GoldenTorizoEyeBeam(k); return;
  case fnnullsub_90: return;
  case fnsub_86B535: sub_86B535(k); return;
  case fnEprojPreInstr_EyeDoorProjectile: EprojPreInstr_EyeDoorProjectile(k); return;
  case fnEprojPreInstr_EyeDoorSweat: EprojPreInstr_EyeDoorSweat(k); return;
  case fnEprojPreInstr_TourianStatueUnlockingParticleWaterSplash: EprojPreInstr_TourianStatueUnlockingParticleWaterSplash(k); return;
  case fnEprojPreInstr_TourianStatueUnlockingParticle: EprojPreInstr_TourianStatueUnlockingParticle(k); return;
  case fnEprojPreInstr_TourianStatueSoul: EprojPreInstr_TourianStatueSoul(k); return;
  case fnEprojPreInstr_TourianStatueStuff: EprojPreInstr_TourianStatueStuff(k); return;
  case fnEprojPreInstr_BA42: EprojPreInstr_BA42(k); return;
  case fnnullsub_91: return;
  case fnnullsub_92: return;
  case fnEprojPreInstr_NorfairLavaquakeRocks: EprojPreInstr_NorfairLavaquakeRocks(k); return;
  case fnEprojPreInstr_NorfairLavaquakeRocks_Inner: EprojPreInstr_NorfairLavaquakeRocks_Inner(k); return;
  case fnEprojPreInstr_NorfairLavaquakeRocks_Inner2: EprojPreInstr_NorfairLavaquakeRocks_Inner2(k); return;
  case fnEprojInit_ShaktoolAttackFrontCircle: EprojInit_ShaktoolAttackFrontCircle(k); return;
  case fnEprojPreInstr_BE12: EprojPreInstr_BE12(k); return;
  case fnEprojPreInstr_MotherBrainRoomTurrets: EprojPreInstr_MotherBrainRoomTurrets(k); return;
  case fnEprojPreInstr_MotherBrainRoomTurretBullets: EprojPreInstr_MotherBrainRoomTurretBullets(k); return;
  case fnEproj_MoveToBlueRingSpawnPosition: Eproj_MoveToBlueRingSpawnPosition(k); return;
  case fnEprojPreInstr_MotherBrainBomb: EprojPreInstr_MotherBrainBomb(k); return;
  case fnEprojPreInstr_MotherBrainRainbowBeam: EprojPreInstr_MotherBrainRainbowBeam(k); return;
  case fnEprojPreInstr_C84D: EprojPreInstr_C84D(k); return;
  case fnEprojInit_MotherBrainsDrool_Falling: EprojInit_MotherBrainsDrool_Falling(k); return;
  case fnEprojPreInit_MotherBrainsDeathExplosion_0: EprojPreInit_MotherBrainsDeathExplosion_0(k); return;
  case fnEprojPreInstr_MotherBrainsRainbowBeamExplosion: EprojPreInstr_MotherBrainsRainbowBeamExplosion(k); return;
  case fnEprojPreInstr_MotherBrainsExplodedDoorParticles: EprojPreInstr_MotherBrainsExplodedDoorParticles(k); return;
  case fnnullsub_94: return;
  case fnEprojPreInstr_TimeBombSetJapaneseText: EprojPreInstr_TimeBombSetJapaneseText(k); return;
  case fnEprojPreInstr_MotherBrainTubeFalling: EprojPreInstr_MotherBrainTubeFalling(k); return;
  case fnEprojPreInstr_MotherBrainGlassShatteringShard: EprojPreInstr_MotherBrainGlassShatteringShard(k); return;
  case fnsub_86CFD5: sub_86CFD5(k); return;
  case fnsub_86CFE6: sub_86CFE6(k); return;
  case fnnullsub_95: return;
  case fnsub_86CFF8: sub_86CFF8(k); return;
  case fnEprojPreInstr_KagosBugs_Func1: EprojPreInstr_KagosBugs_Func1(k); return;
  case fnEprojPreInstr_KagosBugs: EprojPreInstr_KagosBugs(k); return;
  case fnnullsub_302: return;
  case fnEprojPreInstr_D0EC: EprojPreInstr_D0EC(k); return;
  case fnEprojPreInstr_D128: EprojPreInstr_D128(k); return;
  case fnEprojPreInstr_KagosBugs_Func2: EprojPreInstr_KagosBugs_Func2(k); return;
  case fnEprojPreInstr_MaridiaFloatersSpikes: EprojPreInstr_MaridiaFloatersSpikes(k); return;
  case fnEprojPreInstr_WreckedShipRobotLaser: EprojPreInstr_WreckedShipRobotLaser(k); return;
  case fnsub_86D7BF: sub_86D7BF(k); return;
  case fnsub_86D7DE: sub_86D7DE(k); return;
  case fnEprojPreInstr_N00bTubeShards: EprojPreInstr_N00bTubeShards(k); return;
  case fnsub_86D83D: sub_86D83D(k); return;
  case fnsub_86D89F: sub_86D89F(k); return;
  case fnsub_86D8DF: sub_86D8DF(k); return;
  case fnEprojPreInstr_SpikeShootingPlantSpikes: EprojPreInstr_SpikeShootingPlantSpikes(k); return;
  case fnEprojPreInstr_SpikeShootingPlantSpikes_MoveY1: EprojPreInstr_SpikeShootingPlantSpikes_MoveY1(k); return;
  case fnEprojPreInstr_SpikeShootingPlantSpikes_MoveY2: EprojPreInstr_SpikeShootingPlantSpikes_MoveY2(k); return;
  case fnEprojPreInstr_SpikeShootingPlantSpikes_MoveX1: EprojPreInstr_SpikeShootingPlantSpikes_MoveX1(k); return;
  case fnEprojPreInstr_SpikeShootingPlantSpikes_MoveX2: EprojPreInstr_SpikeShootingPlantSpikes_MoveX2(k); return;
  case fnEprojPreInstr_SpikeShootingPlantSpikes_0_MoveX1: EprojPreInstr_SpikeShootingPlantSpikes_0_MoveX1(k); return;
  case fnEprojPreInstr_SpikeShootingPlantSpikes_2_MoveX2: EprojPreInstr_SpikeShootingPlantSpikes_2_MoveX2(k); return;
  case fnEprojPreInstr_SpikeShootingPlantSpikes_1_MoveY1: EprojPreInstr_SpikeShootingPlantSpikes_1_MoveY1(k); return;
  case fnEprojPreInstr_SpikeShootingPlantSpikes_4_MoveY2: EprojPreInstr_SpikeShootingPlantSpikes_4_MoveY2(k); return;
  case fnEprojPreInstr_SpikeShootingPlantSpikes_6_MoveX1Y1: EprojPreInstr_SpikeShootingPlantSpikes_6_MoveX1Y1(k); return;
  case fnEprojPreInstr_SpikeShootingPlantSpikes_8_MoveX1Y2: EprojPreInstr_SpikeShootingPlantSpikes_8_MoveX1Y2(k); return;
  case fnEprojPreInstr_SpikeShootingPlantSpikes_7_MoveX2Y1: EprojPreInstr_SpikeShootingPlantSpikes_7_MoveX2Y1(k); return;
  case fnEprojPreInstr_SpikeShootingPlantSpikes_9_MoveX2Y2: EprojPreInstr_SpikeShootingPlantSpikes_9_MoveX2Y2(k); return;
  case fnEprojPreInstr_DBF2: EprojPreInstr_DBF2(k); return;
  case fnEprojPreInstr_DBF2_MoveX1: EprojPreInstr_DBF2_MoveX1(k); return;
  case fnEprojPreInstr_DBF2_MoveX2: EprojPreInstr_DBF2_MoveX2(k); return;
  case fnEprojPreInstr_DBF2_Func1: EprojPreInstr_DBF2_Func1(k); return;
  case fnEprojPreInstr_Spores: EprojPreInstr_Spores(k); return;
  case fnnullsub_96: return;
  case fnEprojPreInstr_SporeSpawners: EprojPreInstr_SporeSpawners(k); return;
  case fnEprojPreInstr_NamiFuneFireball: EprojPreInstr_NamiFuneFireball(k); return;
  case fnsub_86E049: sub_86E049(k); return;
  case fnEprojPreInstr_DustCloudOrExplosion: EprojPreInstr_DustCloudOrExplosion(k); return;
  case fnEprojPreInstr_nullsub_98: return;
  case fnEprojPreInstr_nullsub_99: return;
  case fnEprojPreInstr_E605: EprojPreInstr_E605(k); return;
  case fnnullsub_100: return;
  case fnEprojPreInstr_BotwoonsBody: EprojPreInstr_BotwoonsBody(k); return;
  case fnEprojPreInstr_BotwoonsSpit: EprojPreInstr_BotwoonsSpit(k); return;
  case fnEprojPreInstr_Empty2: return;
  case fnEprojPreInstr_Empty: return;
  case fnEprojPreInstr_Pickup: EprojPreInstr_Pickup(k); return;
  case fnEprojPreInstr_Sparks: EprojPreInstr_Sparks(k); return;
  case fnnullsub_366: return;
  default: Unreachable();
  }
}

const uint8 *CallEprojInstr(uint32 ea, uint16 k, const uint8 *j) {
  switch (ea) {
  case fnEprojInstr_Delete: return EprojInstr_Delete(k, j);
  case fnEprojInstr_Sleep: return EprojInstr_Sleep(k, j);
  case fnEprojInstr_SetPreInstr_: return EprojInstr_SetPreInstr_(k, j);
  case fnEprojInstr_ClearPreInstr: return EprojInstr_ClearPreInstr(k, j);
  case fnEprojInstr_CallFunc: return EprojInstr_CallFunc(k, j);
  case fnEprojInstr_Goto: return EprojInstr_Goto(k, j);
  case fnEprojInstr_GotoRel: return EprojInstr_GotoRel(k, j);
  case fnEprojInstr_DecTimerAndGotoIfNonZero: return EprojInstr_DecTimerAndGotoIfNonZero(k, j);
  case fnEprojInstr_DecTimerAndGotoRelIfNonZero: return EprojInstr_DecTimerAndGotoRelIfNonZero(k, j);
  case fnEprojInstr_SetTimer: return EprojInstr_SetTimer(k, j);
  case fnEprojInstr_MoveRandomlyWithinRadius: return EprojInstr_MoveRandomlyWithinRadius(k, j);
  case fnEprojInstr_SetProjectileProperties: return EprojInstr_SetProjectileProperties(k, j);
  case fnEprojInstr_ClearProjectileProperties: return EprojInstr_ClearProjectileProperties(k, j);
  case fnEprojInstr_EnableCollisionWithSamusProj: return EprojInstr_EnableCollisionWithSamusProj(k, j);
  case fnEprojInstr_DisableCollisionWithSamusProj: return EprojInstr_DisableCollisionWithSamusProj(k, j);
  case fnEprojInstr_DisableCollisionWithSamus: return EprojInstr_DisableCollisionWithSamus(k, j);
  case fnEprojInstr_EnableCollisionWithSamus: return EprojInstr_EnableCollisionWithSamus(k, j);
  case fnEprojInstr_SetToNotDieOnContact: return EprojInstr_SetToNotDieOnContact(k, j);
  case fnEprojInstr_SetToDieOnContact: return EprojInstr_SetToDieOnContact(k, j);
  case fnEprojInstr_SetLowPriority: return EprojInstr_SetLowPriority(k, j);
  case fnEprojInstr_SetHighPriority: return EprojInstr_SetHighPriority(k, j);
  case fnEprojInstr_SetXyRadius: return EprojInstr_SetXyRadius(k, j);
  case fnEprojInstr_SetXyRadiusZero: return EprojInstr_SetXyRadiusZero(k, j);
  case fnEprojInstr_CalculateDirectionTowardsSamus: return EprojInstr_CalculateDirectionTowardsSamus(k, j);
  case fnEprojInstr_WriteColorsToPalette: return EprojInstr_WriteColorsToPalette(k, j);
  case fnEprojInstr_QueueMusic: return EprojInstr_QueueMusic(k, j);
  case fnEprojInstr_QueueSfx1_Max6: return EprojInstr_QueueSfx1_Max6(k, j);
  case fnEprojInstr_QueueSfx2_Max6: return EprojInstr_QueueSfx2_Max6(k, j);
  case fnEprojInstr_QueueSfx3_Max6: return EprojInstr_QueueSfx3_Max6(k, j);
  case fnEprojInstr_QueueSfx1_Max15: return EprojInstr_QueueSfx1_Max15(k, j);
  case fnEprojInstr_QueueSfx2_Max15: return EprojInstr_QueueSfx2_Max15(k, j);
  case fnEprojInstr_QueueSfx3_Max15: return EprojInstr_QueueSfx3_Max15(k, j);
  case fnEprojInstr_QueueSfx1_Max3: return EprojInstr_QueueSfx1_Max3(k, j);
  case fnEprojInstr_QueueSfx2_Max3: return EprojInstr_QueueSfx2_Max3(k, j);
  case fnEprojInstr_QueueSfx3_Max3: return EprojInstr_QueueSfx3_Max3(k, j);
  case fnEprojInstr_QueueSfx1_Max9: return EprojInstr_QueueSfx1_Max9(k, j);
  case fnEprojInstr_QueueSfx2_Max9: return EprojInstr_QueueSfx2_Max9(k, j);
  case fnEprojInstr_QueueSfx3_Max9: return EprojInstr_QueueSfx3_Max9(k, j);
  case fnEprojInstr_QueueSfx1_Max1: return EprojInstr_QueueSfx1_Max1(k, j);
  case fnEprojInstr_QueueSfx2_Max1: return EprojInstr_QueueSfx2_Max1(k, j);
  case fnEprojInstr_QueueSfx3_Max1: return EprojInstr_QueueSfx3_Max1(k, j);
  case fnEprojInstr_SpawnEnemyDropsWithDraygonsEyeDrops: return EprojInstr_SpawnEnemyDropsWithDraygonsEyeDrops(k, j);
  case fnEprojInstr_868D99: return EprojInstr_868D99(k, j);
  case fnEprojInstr_DisableCollisionsWithSamus: return EprojInstr_DisableCollisionsWithSamus(k, j);
  case fnEprojInstr_95BA: return EprojInstr_95BA(k, j);
  case fnEprojInstr_95ED: return EprojInstr_95ED(k, j);
  case fnEprojInstr_9620: return EprojInstr_9620(k, j);
  case fnEprojInstr_980E: return EprojInstr_980E(k, j);
  case fnEprojInstr_SetPreInstrAndRun: return EprojInstr_SetPreInstrAndRun(k, j);
  case fnEprojInstr_GotoWithProbability25: return EprojInstr_GotoWithProbability25(k, j);
  case fnEprojInstr_SpawnEnemyDrops: return EprojInstr_SpawnEnemyDrops(k, j);
  case fnEprojInstr_GotoDependingOnXDirection: return EprojInstr_GotoDependingOnXDirection(k, j);
  case fnEprojInstr_ResetXYpos1: return EprojInstr_ResetXYpos1(k, j);
  case fnEprojInstr_MoveY_Minus4: return EprojInstr_MoveY_Minus4(k, j);
  case fnEprojInstr_SetVelTowardsSamus1: return EprojInstr_SetVelTowardsSamus1(k, j);
  case fnEprojInstr_SetVelTowardsSamus2: return EprojInstr_SetVelTowardsSamus2(k, j);
  case fnEprojInstr_GotoIfFunc1: return EprojInstr_GotoIfFunc1(k, j);
  case fnEprojInstr_ResetXYpos2: return EprojInstr_ResetXYpos2(k, j);
  case fnEprojInstr_SpawnTourianStatueUnlockingParticle: return EprojInstr_SpawnTourianStatueUnlockingParticle(k, j);
  case fnEprojInstr_Earthquake: return EprojInstr_Earthquake(k, j);
  case fnEprojInstr_SpawnTourianStatueUnlockingParticleTail: return EprojInstr_SpawnTourianStatueUnlockingParticleTail(k, j);
  case fnEprojInstr_AddToYpos: return EprojInstr_AddToYpos(k, j);
  case fnEprojInstr_SwitchJump: return EprojInstr_SwitchJump(k, j);
  case fnEprojInstr_UserPalette0: return EprojInstr_UserPalette0(k, j);
  case fnEprojInstr_Add12ToY: return EprojInstr_Add12ToY(k, j);
  case fnEprojInstr_MotherBrainPurpleBreathIsActive: return EprojInstr_MotherBrainPurpleBreathIsActive(k, j);
  case fnEprojInstr_D15C: return EprojInstr_D15C(k, j);
  case fnEprojInstr_D1B6: return EprojInstr_D1B6(k, j);
  case fnEprojInstr_D1C7: return EprojInstr_D1C7(k, j);
  case fnEprojInstr_D1CE: return EprojInstr_D1CE(k, j);
  case fnEprojInstr_AssignNewN00bTubeShardVelocity: return EprojInstr_AssignNewN00bTubeShardVelocity(k, j);
  case fnEprojInstr_SetN00bTubeShardX: return EprojInstr_SetN00bTubeShardX(k, j);
  case fnEprojInstr_D62A: return EprojInstr_D62A(k, j);
  case fnEprojInstr_SetXvelRandom: return EprojInstr_SetXvelRandom(k, j);
  case fnEprojInstr_DC5A: return EprojInstr_DC5A(k, j);
  case fnEprojInstr_SpawnEnemyDrops_0: return EprojInstr_SpawnEnemyDrops_0(k, j);
  case fnEprojInstr_SpawnSporesEproj: return EprojInstr_SpawnSporesEproj(k, j);
  case fnEprojInstr_DFEA: return EprojInstr_DFEA(k, j);
  case fnEprojInstr_SetYVel: return EprojInstr_SetYVel(k, j);
  case fnEprojInstr_ECE3: return EprojInstr_ECE3(k, j);
  case fnEprojInstr_ED17: return EprojInstr_ED17(k, j);
  case fnEprojInstr_QueueSfx2_9: return EprojInstr_QueueSfx2_9(k, j);
  case fnEprojInstr_QueueSfx2_24: return EprojInstr_QueueSfx2_24(k, j);
  case fnEprojInstr_QueueSfx2_B: return EprojInstr_QueueSfx2_B(k, j);
  case fnEprojInstr_EEAF: return EprojInstr_EEAF(k, j);
  case fnEprojInstr_HandleRespawningEnemy: return EprojInstr_HandleRespawningEnemy(k, j);
  case fnEprojInstr_SetPreInstrA: return EprojInstr_SetPreInstrA(k, j);
  case fnEprojInstr_SetPreInstrB: return EprojInstr_SetPreInstrB(k, j);
  case fnEprojPreInstr_PirateMotherBrainLaser_MoveRight: EprojPreInstr_PirateMotherBrainLaser_MoveRight(k); return j;
  case fnEprojPreInstr_PirateMotherBrainLaser_MoveLeft: EprojPreInstr_PirateMotherBrainLaser_MoveLeft(k); return j;
  case fnEprojInstr_A3BE: return EprojInstr_A3BE(k, j);
  case fnEprojInstr_9270: return EprojInstr_9270(k, j);
  case fnnullsub_82: return j; // really j
  case fnsub_86B13E: return sub_86B13E(k, j);
  default: Unreachable(); return NULL;
  }
}
