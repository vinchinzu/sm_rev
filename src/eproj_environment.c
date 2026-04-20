// Enemy projectile environment/facility families split out of sm_86.c.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "eproj_internal.h"

#define off_86E42C ((uint16*)RomFixedPtr(0x86e42c))
#define word_86E47E ((uint16*)RomFixedPtr(0x86e47e))
#define word_86DEB6 ((uint16*)RomFixedPtr(0x86deb6))
#define kCommonEnemySpeeds_Quadratic_Copy ((uint16*)RomFixedPtr(0xa0cbc7))
#define kCommonEnemySpeeds_Quadratic32 ((uint32*)RomFixedPtr(0xa0cbc7))

static const int16 kEprojInit_EyeDoorProjectile_Positions[20] = {
  -16,  16, -96, -64, -128, -32, -96, 64, -128, 32,
  16, 16, 96, -64, 112, -64, 128, -64, 144, -64,
};

static const int16 kEprojInit_EyeDoorSweat_Velocities[4] = { -64, 512, 64, 512 };

uint16 CheckIfEprojIsOffScreen(uint16 k) {  // 0x86E6E0
  int eproj_idx = k >> 1;
  if ((int16)(eproj_x_pos[eproj_idx] - layer1_x_pos) >= 0) {
    if ((int16)(eproj_x_pos[eproj_idx] - (layer1_x_pos + 256)) < 0
        && (int16)(eproj_y_pos[eproj_idx] - layer1_y_pos) >= 0) {
      if ((int16)(eproj_y_pos[eproj_idx] - (layer1_y_pos + 256)) < 0)
        return 0;
    }
  }
  return 1;
}

void EprojInit_TourianEscapeShaftFakeWallExplode(uint16 j) {  // 0x86B49D
  int eproj_idx = j >> 1;
  eproj_x_pos[eproj_idx] = 272;
  eproj_E[eproj_idx] = 272;
  eproj_y_pos[eproj_idx] = 2184;
  eproj_F[eproj_idx] = 2184;
}

void EprojInit_LavaSeahorseFireball(uint16 j) {  // 0x86B4EF
  EnemyData *enemy = gEnemyData(cur_enemy_index);
  int eproj_idx = j >> 1;
  eproj_y_pos[eproj_idx] = enemy->y_pos - 28;
  eproj_y_vel[eproj_idx] = -961;
  if ((enemy->ai_var_A & 0x8000) == 0) {
    eproj_x_pos[eproj_idx] = enemy->x_pos + 12;
    eproj_x_vel[eproj_idx] = 704;
    eproj_instr_list_ptr[eproj_idx] = addr_word_86B4CB;
  } else {
    eproj_x_pos[eproj_idx] = enemy->x_pos - 12;
    eproj_x_vel[eproj_idx] = -704;
    eproj_instr_list_ptr[eproj_idx] = addr_word_86B4BF;
  }
}

void sub_86B535(uint16 k) {  // 0x86B535
  int eproj_idx = k >> 1;
  AddToHiLo(&eproj_x_pos[eproj_idx], &eproj_x_subpos[eproj_idx], INT16_SHL8(eproj_x_vel[eproj_idx]));
  AddToHiLo(&eproj_y_pos[eproj_idx], &eproj_y_subpos[eproj_idx], INT16_SHL8(eproj_y_vel[eproj_idx]));

  int16 y_vel = eproj_y_vel[eproj_idx];
  eproj_y_vel[eproj_idx] = y_vel + 32;
  if (y_vel >= 0) {
    Eproj_DeleteIfYposOutside(k);
  } else if ((int16)(y_vel + 32) >= 0) {
    eproj_instr_list_ptr[eproj_idx] = (eproj_x_vel[eproj_idx] & 0x8000) == 0 ? addr_word_86B4E3 : addr_word_86B4D7;
    eproj_instr_timers[eproj_idx] = 1;
  }
}

void EprojInit_EyeDoorProjectile(uint16 j) {  // 0x86B62D
  uint16 door_plm = plm_id;
  int eproj_idx = j >> 1;
  eproj_F[eproj_idx] = plm_room_arguments[door_plm >> 1];
  CalculatePlmBlockCoords(door_plm);
  int pos_idx = eproj_init_param_1 >> 1;
  eproj_x_pos[eproj_idx] = kEprojInit_EyeDoorProjectile_Positions[pos_idx] + 8 * (2 * plm_x_block + 1);
  eproj_y_pos[eproj_idx] = kEprojInit_EyeDoorProjectile_Positions[pos_idx + 1] + 16 * plm_y_block;
}

void EprojInit_EyeDoorSweat(uint16 j) {  // 0x86B683
  CalculatePlmBlockCoords(plm_id);
  int eproj_idx = j >> 1;
  eproj_x_pos[eproj_idx] = 8 * (2 * (plm_x_block - 1) + 1);
  eproj_y_pos[eproj_idx] = 16 * (plm_y_block + 1);
  int vel_idx = eproj_init_param_1 >> 1;
  eproj_x_vel[eproj_idx] = kEprojInit_EyeDoorSweat_Velocities[vel_idx];
  eproj_y_vel[eproj_idx] = kEprojInit_EyeDoorSweat_Velocities[vel_idx + 1];
}

void EprojPreInstr_EyeDoorProjectile(uint16 k) {  // 0x86B6B9
  int eproj_idx = k >> 1;
  if (EprojBlockCollisition_Horiz(k) & 1 || EprojBlockCollisition_Vertical(k) & 1) {
    eproj_instr_list_ptr[eproj_idx] = addr_off_86B5F3;
    eproj_instr_timers[eproj_idx] = 1;
    return;
  }
  int angle_idx = eproj_E[eproj_idx] >> 1;
  eproj_x_vel[eproj_idx] += kSinCosTable8bit_Sext[angle_idx + 64] >> 4;
  eproj_y_vel[eproj_idx] += kSinCosTable8bit_Sext[angle_idx] >> 4;
  int bit_index = PrepareBitAccess(eproj_F[eproj_idx]);
  if ((bitmask & opened_door_bit_array[bit_index]) != 0) {
    eproj_instr_list_ptr[eproj_idx] = addr_off_86B5F3;
    eproj_instr_timers[eproj_idx] = 1;
  }
}

void EprojPreInstr_EyeDoorSweat(uint16 k) {  // 0x86B714
  EprojBlockCollisition_Horiz(k);
  uint8 collided_vertically = EprojBlockCollisition_Vertical(k);
  int eproj_idx = k >> 1;
  if ((eproj_y_vel[eproj_idx] & 0x8000) != 0 || !collided_vertically) {
    eproj_y_vel[eproj_idx] += 12;
  } else {
    eproj_y_pos[eproj_idx] -= 4;
    eproj_instr_list_ptr[eproj_idx] = addr_off_86B61D;
    eproj_instr_timers[eproj_idx] = 1;
  }
}

void EprojInit_NuclearWaffleBody(uint16 j) {  // 0x86BB92
  EnemyData *enemy = gEnemyData(cur_enemy_index);
  int eproj_idx = j >> 1;
  eproj_x_pos[eproj_idx] = enemy->x_pos;
  eproj_x_subpos[eproj_idx] = enemy->x_subpos;
  eproj_y_pos[eproj_idx] = enemy->y_pos;
  eproj_y_subpos[eproj_idx] = enemy->y_subpos;
  ExtraEnemyRam8000 *extra_enemy_ram = gExtraEnemyRam8000(cur_enemy_index);
  gExtraEnemyRam7800(cur_enemy_index + *(uint16 *)&extra_enemy_ram->pad[20])->kraid.kraid_next = j;
  eproj_flags[eproj_idx] = 1;
}

static void CallNorfairLavaquakeRocksFunc(uint32 ea, uint16 k) {
  switch (ea) {
  case fnEproj_NorfairLavaquakeRocks_Func1: Eproj_NorfairLavaquakeRocks_Func1(k); return;
  case fnEproj_NorfairLavaquakeRocks_Func2: Eproj_NorfairLavaquakeRocks_Func2(k); return;
  default: Unreachable();
  }
}

void EprojInit_NorfairLavaquakeRocks(uint16 j) {  // 0x86BBDB
  int v1 = j >> 1;
  eproj_instr_list_ptr[v1] = addr_word_86BBD5;
  eproj_E[v1] = FUNC16(Eproj_NorfairLavaquakeRocks_Func1);
  eproj_y_vel[v1] = eproj_init_param_1;
  eproj_x_vel[v1] = eproj_unk1995;
  EnemyData *v2 = gEnemyData(cur_enemy_index);
  eproj_x_pos[v1] = v2->x_pos;
  eproj_x_subpos[v1] = v2->x_subpos;
  eproj_y_pos[v1] = v2->y_pos;
  eproj_y_subpos[v1] = v2->y_subpos;
}

void EprojPreInstr_NorfairLavaquakeRocks(uint16 k) {  // 0x86BC0F
  CallNorfairLavaquakeRocksFunc(eproj_E[k >> 1] | 0x860000, k);
  EprojPreInstr_NorfairLavaquakeRocks_Inner(k);
}

void Eproj_NorfairLavaquakeRocks_Func1(uint16 k) {  // 0x86BC16
  int16 v2;
  int16 v3;

  int v1 = k >> 1;
  v2 = eproj_y_vel[v1] - 2;
  eproj_y_vel[v1] = v2;
  if (v2 >= 0) {
    int n = 2;
    do {
      v3 = n + eproj_y_vel[v1] - 1;
      if (v3 < 0)
        v3 = 0;
      int t = (8 * v3 + 4) >> 1;
      AddToHiLo(&eproj_y_pos[v1], &eproj_y_subpos[v1], kCommonEnemySpeeds_Quadratic32[t >> 1]);
      eproj_F[v1] = kCommonEnemySpeeds_Quadratic_Copy[t + 1];  // junk
    } while (--n);
    Eproj_NorfairLavaquakeRocks_Func3(k);
  } else {
    eproj_y_vel[v1] = 0;
    eproj_E[v1] = FUNC16(Eproj_NorfairLavaquakeRocks_Func2);
  }
}

void Eproj_NorfairLavaquakeRocks_Func2(uint16 k) {  // 0x86BC8F
  int v1 = k >> 1;
  uint16 v2 = eproj_y_vel[v1] + 2;
  eproj_y_vel[v1] = v2;
  if (!sign16(v2 - 64))
    eproj_y_vel[v1] = 64;
  int n = 2;
  do {
    int t = (8 * (eproj_y_vel[v1] - n + 1)) >> 1;
    AddToHiLo(&eproj_y_pos[v1], &eproj_y_subpos[v1], kCommonEnemySpeeds_Quadratic32[t >> 1]);
    eproj_F[v1] = kCommonEnemySpeeds_Quadratic_Copy[t + 1];  // junk
  } while (--n);
  Eproj_NorfairLavaquakeRocks_Func3(k);
}

void Eproj_NorfairLavaquakeRocks_Func3(uint16 k) {  // 0x86BCF4
  int v1 = k >> 1;
  AddToHiLo(&eproj_x_pos[v1], &eproj_x_subpos[v1], INT16_SHL8(eproj_x_vel[v1]));
}

uint16 EprojPreInstr_NorfairLavaquakeRocks_Inner2(uint16 k) {  // 0x86BD2A
  int v1 = k >> 1;
  return (int16)(eproj_x_pos[v1] - layer1_x_pos) < 0
    || (int16)(layer1_x_pos + 256 - eproj_x_pos[v1]) < 0
    || (int16)(eproj_y_pos[v1] - layer1_y_pos) < 0
    || (int16)(layer1_y_pos + 256 - eproj_y_pos[v1]) < 0;
}

void EprojPreInstr_NorfairLavaquakeRocks_Inner(uint16 k) {  // 0x86BD1E
  if (EprojPreInstr_NorfairLavaquakeRocks_Inner2(k))
    eproj_id[k >> 1] = 0;
}

static void EprojInit_NamiFuneFireball_MoveX1(uint16 k) {  // 0x86DF40
  int eproj_idx = k >> 1;
  AddToHiLo(&eproj_x_pos[eproj_idx], &eproj_x_subpos[eproj_idx], INT16_SHL8(eproj_y_vel[eproj_idx]));
}

static void EprojInit_NamiFuneFireball_MoveX2(uint16 k) {  // 0x86DF6A
  int eproj_idx = k >> 1;
  AddToHiLo(&eproj_x_pos[eproj_idx], &eproj_x_subpos[eproj_idx], INT16_SHL8(eproj_x_vel[eproj_idx]));
}

static void CallNamiFuneFireballFunc(uint32 ea, uint16 k) {
  switch (ea) {
  case fnEprojInit_NamiFuneFireball_MoveX1: EprojInit_NamiFuneFireball_MoveX1(k); return;
  case fnEprojInit_NamiFuneFireball_MoveX2: EprojInit_NamiFuneFireball_MoveX2(k); return;
  default: Unreachable();
  }
}

void Eproj_NamiFuneFireball_After(uint16 k) {  // 0x86DF94
  if (EprojPreInstrHelper_DBF2_Func2(k))
    eproj_id[k >> 1] = 0;
}

void EprojInit_NamiFuneFireball(uint16 j) {  // 0x86DED6
  int eproj_idx = j >> 1;
  eproj_instr_list_ptr[eproj_idx] = addr_word_86DE96;
  eproj_E[eproj_idx] = FUNC16(EprojInit_NamiFuneFireball_MoveX1);
  if (eproj_init_param_1) {
    eproj_instr_list_ptr[eproj_idx] = addr_word_86DEA6;
    eproj_E[eproj_idx] = FUNC16(EprojInit_NamiFuneFireball_MoveX2);
  }
  EnemyData *enemy = gEnemyData(cur_enemy_index);
  eproj_x_pos[eproj_idx] = enemy->x_pos;
  eproj_x_subpos[eproj_idx] = enemy->x_subpos;
  eproj_y_pos[eproj_idx] = enemy->y_pos;
  eproj_y_subpos[eproj_idx] = enemy->y_subpos;
  if ((enemy->parameter_1 & 0xF) != 0)
    eproj_y_pos[eproj_idx] += 4;
  int vel_idx = (uint16)(4 * LOBYTE(enemy->parameter_2)) >> 1;
  eproj_y_vel[eproj_idx] = word_86DEB6[vel_idx];
  eproj_x_vel[eproj_idx] = word_86DEB6[vel_idx + 1];
}

void EprojPreInstr_NamiFuneFireball(uint16 k) {  // 0x86DF39
  CallNamiFuneFireballFunc(eproj_E[k >> 1] | 0x860000, k);
  Eproj_NamiFuneFireball_After(k);
}

void Eproj_LavaThrownByLavaman_MoveX1(uint16 k) {  // 0x86E050
  int eproj_idx = k >> 1;
  AddToHiLo(&eproj_x_pos[eproj_idx], &eproj_x_subpos[eproj_idx], INT16_SHL8(eproj_y_vel[eproj_idx]));
}

void Eproj_LavaThrownByLavaman_MoveX2(uint16 k) {  // 0x86E07A
  int eproj_idx = k >> 1;
  AddToHiLo(&eproj_x_pos[eproj_idx], &eproj_x_subpos[eproj_idx], INT16_SHL8(eproj_x_vel[eproj_idx]));
}

static void CallLavamanFunc(uint32 ea, uint16 k) {
  switch (ea) {
  case fnEproj_LavaThrownByLavaman_MoveX1: Eproj_LavaThrownByLavaman_MoveX1(k); return;
  case fnEproj_LavaThrownByLavaman_MoveX2: Eproj_LavaThrownByLavaman_MoveX2(k); return;
  default: Unreachable();
  }
}

void EprojInit_LavaThrownByLavaman(uint16 j) {  // 0x86E000
  int eproj_idx = j >> 1;
  eproj_instr_list_ptr[eproj_idx] = addr_word_86DFD8;
  eproj_E[eproj_idx] = FUNC16(Eproj_LavaThrownByLavaman_MoveX1);
  if (eproj_init_param_1) {
    eproj_instr_list_ptr[eproj_idx] = addr_word_86DFDE;
    eproj_E[eproj_idx] = FUNC16(Eproj_LavaThrownByLavaman_MoveX2);
  }
  EnemyData *enemy = gEnemyData(cur_enemy_index);
  eproj_x_pos[eproj_idx] = enemy->x_pos;
  eproj_x_subpos[eproj_idx] = enemy->x_subpos;
  eproj_y_pos[eproj_idx] = enemy->y_pos + 2;
  eproj_y_subpos[eproj_idx] = enemy->y_subpos;
  eproj_y_vel[eproj_idx] = -768;
  eproj_x_vel[eproj_idx] = 768;
}

void sub_86E049(uint16 k) {  // 0x86E049
  CallLavamanFunc(eproj_E[k >> 1] | 0x860000, k);
  if (CheckIfEprojIsOffScreen(k))
    eproj_id[k >> 1] = 0;
}

void EprojInit_DustCloudOrExplosion(uint16 j) {  // 0x86E468
  int eproj_idx = j >> 1;
  eproj_instr_list_ptr[eproj_idx] = off_86E42C[eproj_init_param_1];
  eproj_x_pos[eproj_idx] = eproj_spawn_pt.x;
  eproj_y_pos[eproj_idx] = eproj_spawn_pt.y;
}

void EprojPreInstr_DustCloudOrExplosion(uint16 k) {  // 0x86E4FE
  if (CheckIfEprojIsOffScreen(k))
    eproj_id[k >> 1] = 0;
}

void EprojInit_EyeDoorSmoke(uint16 j) {  // 0x86E4A6
  int eproj_idx = j >> 1;
  eproj_instr_list_ptr[eproj_idx] = off_86E42C[(uint8)eproj_init_param_1];
  int smoke_idx = (8 * HIBYTE(eproj_init_param_1)) >> 1;
  uint16 x = word_86E47E[smoke_idx + 2] + (word_86E47E[smoke_idx] & random_number);
  uint16 y = word_86E47E[smoke_idx + 3] + (word_86E47E[smoke_idx + 1] & (random_number >> 8));
  CalculatePlmBlockCoords(plm_id);
  eproj_x_pos[eproj_idx] = x + 8 * (2 * plm_x_block + 1);
  eproj_y_pos[eproj_idx] = y + 8 * (2 * plm_y_block + 1);
  NextRandom();
}

static void EprojInit_SpawnedGate_Common(uint16 j, uint16 y_offset) {  // 0x86E5DD
  uint16 gate_plm = plm_id;
  CalculatePlmBlockCoords(gate_plm);
  int eproj_idx = j >> 1;
  eproj_E[eproj_idx] = plm_block_indices[gate_plm >> 1];
  eproj_x_pos[eproj_idx] = 16 * plm_x_block;
  eproj_y_pos[eproj_idx] = y_offset + 16 * plm_y_block;
}

void EprojInit_SpawnedShotGate(uint16 j) {  // 0x86E5D0
  EprojInit_SpawnedGate_Common(j, 0);
}

void EprojInit_ClosedDownwardsShotGate(uint16 j) {  // 0x86E5D5
  EprojInit_SpawnedGate_Common(j, 0x40);
}

void EprojInit_ClosedUpwardsShotGate(uint16 j) {  // 0x86E5DA
  EprojInit_SpawnedGate_Common(j, -0x40);
}

void EprojPreInstr_E605(uint16 k) {  // 0x86E605
  int eproj_idx = k >> 1;
  uint16 timer_accum = eproj_timers[eproj_idx] + abs16(eproj_y_vel[eproj_idx]);
  if (timer_accum >= 0x1000) {
    eproj_instr_timers[eproj_idx] = 1;
    ++eproj_instr_list_ptr[eproj_idx];
    ++eproj_instr_list_ptr[eproj_idx];
    timer_accum = 0;
  }
  eproj_timers[eproj_idx] = timer_accum;
  eproj_y_subpos[eproj_idx] += LOBYTE(eproj_y_vel[eproj_idx]) << 8;
  eproj_y_pos[eproj_idx] += (int8)HIBYTE(eproj_y_vel[eproj_idx]);
}

void EprojInit_SaveStationElectricity(uint16 j) {  // 0x86E6AD
  CalculatePlmBlockCoords(plm_id);
  int eproj_idx = j >> 1;
  eproj_x_pos[eproj_idx] = 16 * (plm_x_block + 1);
  eproj_y_pos[eproj_idx] = 16 * (plm_y_block - 2);
}
