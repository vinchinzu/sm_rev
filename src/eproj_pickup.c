// Pickup, death-explosion, and spark enemy-projectile families split out of sm_86.c.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "eproj_internal.h"
#include "enemy_ai_canon.h"

#define off_86EF04 ((uint16*)RomFixedPtr(0x86ef04))
#define off_86EFD5 ((uint16*)RomFixedPtr(0x86efd5))
#define kEnemyDef_F3D3 (*(EnemyDef_B2*)RomFixedPtr(0xa0f3d3))
#define g_word_86F3D4 ((uint16*)RomFixedPtr(0x86f3d4))  // bug:: oob read

const uint8 *EprojInstr_ECE3(uint16 k, const uint8 *epjp) {  // 0x86ECE3
  int v2 = k >> 1;
  uint16 x = eproj_x_pos[v2] + (NextRandom() & 0x3F) - 32;
  uint16 y = eproj_y_pos[v2] + ((uint16)(random_number & 0x3F00) >> 8) - 32;
  CreateSpriteAtPos(x, y, GET_WORD(epjp), 0);
  return epjp + 2;
}

const uint8 *EprojInstr_ED17(uint16 k, const uint8 *epjp) {  // 0x86ED17
  int v2 = k >> 1;
  uint16 x = eproj_x_pos[v2] + (NextRandom() & 0x1F) - 16;
  uint16 y = eproj_y_pos[v2] + ((uint16)(random_number & 0x1F00) >> 8) - 16;
  CreateSpriteAtPos(x, y, GET_WORD(epjp), 0);
  return epjp + 2;
}

const uint8 *EprojInstr_QueueSfx2_9(uint16 k, const uint8 *epjp) {  // 0x86EE8B
  QueueSfx2_Max1(9);
  return epjp;
}

const uint8 *EprojInstr_QueueSfx2_24(uint16 k, const uint8 *epjp) {  // 0x86EE97
  QueueSfx2_Max1(0x24);
  return epjp;
}

const uint8 *EprojInstr_QueueSfx2_B(uint16 k, const uint8 *epjp) {  // 0x86EEA3
  QueueSfx2_Max1(0xB);
  return epjp;
}

const uint8 *EprojInstr_EEAF(uint16 k, const uint8 *epjp) {  // 0x86EEAF
  uint16 v2 = RandomDropRoutine(k);
  if (k != 0 && sign16(v2 - 6)) {
    uint16 v3 = 2 * v2;
    int v4 = k >> 1;
    eproj_E[v4] = v3;
    eproj_instr_list_ptr[v4] = off_86EF04[v3 >> 1];
    eproj_instr_timers[v4] = 1;
    eproj_F[v4] = 400;
    eproj_pre_instr[v4] = FUNC16(EprojPreInstr_Pickup);
    eproj_properties[v4] &= ~0x4000;
    return INSTRB_RETURN_ADDR(eproj_instr_list_ptr[v4]);
  } else {
    int v6 = k >> 1;
    eproj_instr_timers[v6] = 1;
    eproj_properties[v6] = 12288;
    eproj_pre_instr[v6] = FUNC16(EprojPreInstr_Empty);
    eproj_instr_list_ptr[v6] = 0xECA3;
    return INSTRB_RETURN_ADDR(0xECA3);
  }
}

const uint8 *EprojInstr_HandleRespawningEnemy(uint16 k, const uint8 *epjp) {  // 0x86EF10
  if ((int16)eproj_killed_enemy_index[k >> 1] <= -2)
    RespawnEnemy(eproj_killed_enemy_index[k >> 1] & 0x7fff);
  return epjp;
}

void EprojInit_F337(uint16 j) {  // 0x86EF29
  int v2 = j >> 1;
  eproj_x_pos[v2] = eproj_spawn_pt.x;
  eproj_y_pos[v2] = eproj_spawn_pt.y;
  eproj_gfx_idx[v2] = 0;
  eproj_enemy_header_ptr[v2] = eproj_spawn_varE24; // this is X?!
  uint16 v3 = RandomDropRoutine(j);
  if (v3 != 0 && sign16(v3 - 6)) { // bug, why does it compare x here.
    uint16 v4 = 2 * v3;
    eproj_E[v2] = v4;
    eproj_instr_list_ptr[v2] = off_86EF04[v4 >> 1];
    eproj_instr_timers[v2] = 1;
    eproj_F[v2] = 400;
    eproj_killed_enemy_index[j >> 1] = -1;
  } else {
    int v5 = j >> 1;
    eproj_instr_list_ptr[v5] = addr_word_86ECA3;
    eproj_instr_timers[v5] = 1;
    eproj_properties[v5] = 12288;
    eproj_pre_instr[v5] = FUNC16(EprojPreInstr_Empty);
  }
}


void EprojInit_EnemyDeathExplosion(uint16 j) {  // 0x86EF89
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  int v2 = j >> 1;
  eproj_x_pos[v2] = v1->x_pos;
  eproj_y_pos[v2] = v1->y_pos;
  eproj_killed_enemy_index[v2] = cur_enemy_index;
  if ((v1->properties & 0x4000) != 0)
    eproj_killed_enemy_index[v2] = cur_enemy_index | 0x8000;
  eproj_enemy_header_ptr[v2] = v1->enemy_ptr;
  eproj_gfx_idx[v2] = 0;
  eproj_instr_list_ptr[v2] = off_86EFD5[eproj_init_param_1];
  eproj_instr_timers[v2] = 1;
}

static Func_V *const off_86F0AD[7] = {  // 0x86EFE0
  0,
  Eproj_Pickup_SmallHealth,
  Eproj_Pickup_BigHealth,
  Eproj_Pickup_PowerBombs,
  Eproj_Pickup_Missiles,
  Eproj_Pickup_SuperMissiles,
  0,
};

void EprojPreInstr_Pickup(uint16 k) {
  int v1 = k >> 1;
  if (!--eproj_F[v1])
    goto LABEL_7;
  if (CallSomeSamusCode(0xD)) {
    if (sign16(eproj_F[v1] - 384)) {
      uint16 v2, v3;
      v2 = abs16(eproj_x_pos[v1] - grapple_beam_end_x_pos);
      if (sign16(v2 - 16)) {
        v3 = abs16(eproj_y_pos[v1] - grapple_beam_end_y_pos);
        if (sign16(v3 - 16)) {
          off_86F0AD[eproj_E[v1] >> 1]();
LABEL_7:;
          int v4 = k >> 1;
          eproj_instr_list_ptr[v4] = addr_word_86ECA3;
          eproj_instr_timers[v4] = 1;
          eproj_properties[v4] = 12288;
          eproj_pre_instr[v4] = FUNC16(EprojPreInstr_Empty);
          return;
        }
      }
    }
  }
  int v5 = k >> 1;
  uint16 varE20 = LOBYTE(eproj_radius[v5]);
  uint16 varE22 = HIBYTE(eproj_radius[v5]);
  uint16 v6 = abs16(samus_x_pos - eproj_x_pos[v5]);
  bool v7 = v6 < samus_x_radius;
  uint16 v8 = v6 - samus_x_radius;
  if (v7 || v8 < varE20) {
    uint16 v9 = abs16(samus_y_pos - eproj_y_pos[v5]);
    v7 = v9 < samus_y_radius;
    uint16 v10 = v9 - samus_y_radius;
    if (v7 || v10 < varE22) {
      off_86F0AD[eproj_E[v5] >> 1]();
      int v11 = k >> 1;
      eproj_instr_list_ptr[v11] = addr_word_86ECA3;
      eproj_instr_timers[v11] = 1;
      eproj_properties[v11] = 12288;
      eproj_pre_instr[v11] = FUNC16(EprojPreInstr_Empty);
    }
  }
}

void Eproj_Pickup_SmallHealth(void) {  // 0x86F0BB
  Samus_RestoreHealth(5);
  QueueSfx2_Max1(1);
}

void Eproj_Pickup_BigHealth(void) {  // 0x86F0CA
  Samus_RestoreHealth(0x14);
  QueueSfx2_Max1(2);
}

void Eproj_Pickup_PowerBombs(void) {  // 0x86F0D9
  Samus_RestorePowerBombs(1);
  QueueSfx2_Max1(5);
}

void Eproj_Pickup_Missiles(void) {  // 0x86F0E8
  Samus_RestoreMissiles(2);
  QueueSfx2_Max1(3);
}

void Eproj_Pickup_SuperMissiles(void) {  // 0x86F0F7
  Samus_RestoreSuperMissiles(1);
  QueueSfx2_Max1(4);
}

static const uint8 byte_86F25E[6] = { 1, 2, 4, 6, 5, 3 };

uint16 RandomDropRoutine(uint16 k) {  // 0x86F106
  int8 v9; // cf
  uint8 r18, r22, r20;
  uint16 r24;

  int v1 = k >> 1;
  //varE2A = eproj_killed_enemy_index[v1] & 0x7FFF;
  uint16 varE28 = eproj_enemy_header_ptr[v1];
  if (varE28 == 0)
    goto LABEL_30;
  uint16 v2;
  v2 = get_EnemyDef_A2(varE28)->item_drop_chances_ptr;
  if (!v2)
    goto LABEL_30;
  uint8 Random;
  do
    Random = NextRandom();
  while (!Random);
  r20 = 255;
  r24 = 0;
  uint16 v5;
  v5 = 1;
  if ((uint16)(samus_reserve_health + samus_health) >= 0x1E) {
    if ((uint16)(samus_reserve_health + samus_health) < 0x32)
      goto LABEL_7;
    v5 = 0;
  }
  health_drop_bias_flag = v5;
LABEL_7:;
  const uint8 *v7 = RomPtr_B4(v2);
  if ((uint8)health_drop_bias_flag) {
    r18 = v7[1] + *v7;
    r22 = 3;
  } else {
    r18 = v7[3];
    r22 = 8;
    if (samus_health != samus_max_health || samus_reserve_health != samus_max_reserve_health) {
      r18 = v7[1] + *v7 + r18;
      r22 = r22 | 3;
    }
    if (samus_missiles != samus_max_missiles) {
      r18 = v7[2] + r18;
      r22 = r22 | 4;
    }
    if (samus_super_missiles != samus_max_super_missiles) {
      r20 = r20 - v7[4];
      r22 = r22 | 0x10;
    }
    if (samus_power_bombs != samus_max_power_bombs) {
      r20 = r20 - v7[5];
      r22 = r22 | 0x20;
    }
  }
  int i;
  for (i = 0; i != 4; ++i) {
    if (!(uint8)r18) {
      r22 = (uint8)r22 >> 4;
      v2 += 4;
      i = 4;
      goto LABEL_26;
    }
    v9 = r22 & 1;
    r22 = (uint8)r22 >> 1;
    if (v9) {
      const uint8 *v10 = RomPtr_B4(v2);
      uint16 RegWord = Mult8x8(r20, *v10);
      int divved = SnesDivide(RegWord, r18);
      uint16 v12 = r24;
      uint16 v13 = divved + v12;
      if (v13 >= Random)
        return byte_86F25E[i];
      r24 = v13;
    }
    ++v2;
  }
  do {
LABEL_26:
    v9 = r22 & 1;
    r22 = (uint8)r22 >> 1;
    if (v9) {
      const uint8 *v14 = RomPtr_B4(v2);
      if ((uint16)(r24 + *v14) >= Random)
        return byte_86F25E[i];
      r24 += *v14;
    }
    ++v2;
    ++i;
  } while (i != 6);
LABEL_30:
  i = 3;
  return byte_86F25E[i];
}

void RespawnEnemy(uint16 v0) {  // 0x86F264
  cur_enemy_index = v0;
  const uint8 *v1 = RomPtr_A1(room_enemy_population_ptr + (v0 >> 2));
  EnemyData *E = gEnemyData(v0);
  E->enemy_ptr = GET_WORD(v1);
  E->x_pos = GET_WORD(v1 + 2);
  E->y_pos = GET_WORD(v1 + 4);
  E->current_instruction = GET_WORD(v1 + 6);
  E->properties = GET_WORD(v1 + 8);
  E->extra_properties = GET_WORD(v1 + 10);
  E->parameter_1 = GET_WORD(v1 + 12);
  E->parameter_2 = GET_WORD(v1 + 14);
  EnemySpawnData *ESD = gEnemySpawnData(v0);
  E->palette_index = ESD->palette_index;
  E->vram_tiles_index = ESD->vram_tiles_index;
  E->frozen_timer = 0;
  E->flash_timer = 0;
  E->invincibility_timer = 0;
  E->timer = 0;
  E->frame_counter = 0;
  E->ai_var_A = 0;
  E->ai_var_B = 0;
  E->ai_var_C = 0;
  E->ai_var_D = 0;
  E->ai_var_E = 0;
  E->ai_preinstr = 0;
  E->instruction_timer = 1;
  EnemyDef *Edef = get_EnemyDef_A2(E->enemy_ptr);
  E->x_width = Edef->x_radius;
  E->y_height = Edef->y_radius;
  E->health = Edef->health;
  E->layer = Edef->layer;
  *(uint16 *)&E->bank = *(uint16 *)&Edef->bank;
  RunEnemyAiFn(GetEnemyDefAiFns(E->enemy_ptr)->ai_init);
}


void EprojInit_Sparks(uint16 j) {  // 0x86F391
  static const uint16 word_86F3D4[14] = { 0xffff, 0xb800, 0xffff, 0xc000, 0xffff, 0xe000, 0xffff, 0xff00, 0, 0x100, 0, 0x2000, 0, 0x4000 };
  int v2 = j >> 1;
  eproj_instr_list_ptr[v2] = addr_kEnemyDef_F353;
  EnemyData *v3 = gEnemyData(cur_enemy_index);
  eproj_x_pos[v2] = v3->x_pos;
  eproj_x_subpos[v2] = v3->x_subpos;
  eproj_y_pos[v2] = v3->y_pos + 8;
  eproj_y_subpos[v2] = v3->y_subpos;
  eproj_x_vel[v2] = 0;
  eproj_y_vel[v2] = 0;
  uint16 v4 = (NextRandom() & 0x1C) >> 1;
  eproj_F[v2] = g_word_86F3D4[v4 + 0];
  eproj_E[v2] = g_word_86F3D4[v4 + 1];
}

void EprojPreInstr_Sparks(uint16 k) {  // 0x86F3F0
  int v1 = k >> 1;
  if ((eproj_y_vel[v1] & 0x8000) == 0) {
    if (EprojBlockCollisition_Vertical(k) & 1) {
      eproj_instr_list_ptr[v1] = 0xF363;
      eproj_instr_timers[v1] = 1;

      eproj_F[v1] = eproj_F[v1] * 2 + (eproj_E[v1] >> 15);
      eproj_E[v1] *= 2;

      eproj_F[v1] = eproj_F[v1] * 2 + (eproj_E[v1] >> 15);
      eproj_E[v1] *= 2;

      eproj_x_vel[v1] = 0x8000;
      eproj_y_vel[v1] = -1;
      eproj_y_pos[v1] -= 2;
      return;
    }
    uint16 v2 = eproj_x_vel[v1];
    eproj_x_vel[v1] = v2 + 0x4000;
    uint16 v3 = __CFADD__uint16(v2, 0x4000) + eproj_y_vel[v1];
    if (v3 < 4)
      eproj_y_vel[v1] = v3;
  }
  AddToHiLo(&eproj_y_pos[v1], &eproj_y_subpos[v1], __PAIR32__(eproj_y_vel[v1], eproj_x_vel[v1]));
  AddToHiLo(&eproj_x_pos[v1], &eproj_x_subpos[v1], __PAIR32__(eproj_F[v1], eproj_E[v1]));
  if ((nmi_frame_counter_byte & 3) == 0)
    CreateSpriteAtPos(eproj_x_pos[v1], eproj_y_pos[v1], 48, eproj_gfx_idx[v1]);
}
