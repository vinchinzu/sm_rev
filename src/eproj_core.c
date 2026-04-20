// Enemy projectile core runtime, collision helpers, and generic instruction handlers.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "eproj_internal.h"

#define kAlignYPos_Tab0 ((uint8*)RomFixedPtr(0x948b2b))
#define kScreenShakeOffsets ((uint16*)RomFixedPtr(0x86846b))
#define CHECK_locret_868728(i) (unk_868729[i] & 0x80 ? -1 : 0)

typedef struct EprojCollInfo {
  uint16 eci_r20;
  uint16 eci_r24;
  uint16 eci_r26;
  uint16 eci_r28;
  uint16 eci_r30;
  uint16 eci_r32;
  uint16 eci_r34;
} EprojCollInfo;

Rect16U eproj_spawn_rect;
uint16 eproj_spawn_r22;
Point16U eproj_spawn_pt; // R18/R20
uint16 eproj_spawn_varE24;

static uint8 EprojColl_85C2(EprojCollInfo *eci, uint16 a, uint16 k);
static uint8 EprojColl_8676(EprojCollInfo *eci, uint16 a, uint16 k, uint16 j);
static uint8 EprojColl_874E(EprojCollInfo *eci);
static uint8 EprojColl_8506(EprojCollInfo *eci);
static uint8 EprojColl_Unknown8536(EprojCollInfo *eci);
static uint8 EprojColl_ClearCarry(EprojCollInfo *eci);
static uint8 EprojColl_SetCarry(EprojCollInfo *eci);
static uint8 EprojColl_858E(EprojCollInfo *eci);
static uint8 EprojColl_85AD(EprojCollInfo *eci);
static uint8 EprojBlockCollisition_CheckHorizontal(EprojCollInfo *eci, uint16 k);
static uint8 EprojBlockCollisition_CheckVertical(EprojCollInfo *eci, uint16 k);

static const uint8 unk_868729[20] = {  // 0x8685C2
     0,    1, 0x82, 0x83, 0, 0x81, 2, 0x83, 0, 1, 2, 0x83, 0, 0x81, 0x82, 0x83,
  0x80, 0x81, 0x82, 0x83,
};

static void SpawnEprojInner(uint16 j, uint16 gfx_idx) {  // 0x868027
  uint16 v3 = 34;
  while (eproj_id[v3 >> 1]) {
    v3 -= 2;
    if ((v3 & 0x8000) != 0)
      return;
  }
  int v4 = v3 >> 1;
  eproj_gfx_idx[v4] = gfx_idx;
  eproj_id[v4] = j;
  EprojDef *Edef = get_EprojDef(j);
  eproj_pre_instr[v4] = Edef->pre_instr_ptr;
  eproj_instr_list_ptr[v4] = Edef->instr_list;
  eproj_radius[v4] = Edef->radius;
  eproj_properties[v4] = Edef->properties;
  eproj_instr_timers[v4] = 1;
  eproj_spritemap_ptr[v4] = 0x8000;
  eproj_E[v4] = 0;
  eproj_F[v4] = 0;
  eproj_timers[v4] = 0;
  eproj_x_subpos[v4] = 0;
  eproj_y_subpos[v4] = 0;
  eproj_G[v4] = 0;
  CallEprojInit(Edef->init_code_ptr | 0x860000, v3);
}

static void EprojRunOne(uint16 k) {  // 0x868125
  CallEprojPreInstr(eproj_pre_instr[k >> 1] | 0x860000, k);
  uint16 v1 = eproj_index;
  int v2 = eproj_index >> 1;
  if (eproj_instr_timers[v2]-- == 1) {
    const uint8 *base = RomBankBase(0x86);
    const uint8 *p = base + eproj_instr_list_ptr[v2];
    while (1) {
      if ((GET_WORD(p) & 0x8000) == 0)
        break;
      p = CallEprojInstr(GET_WORD(p) | 0x860000, v1, p + 2);
      if ((uintptr_t)p < 0x10000) {
        if (!p)
          return;
        p = base + (uintptr_t)p;
      }
    }
    int v7 = v1 >> 1;
    eproj_instr_timers[v7] = GET_WORD(p);
    eproj_spritemap_ptr[v7] = GET_WORD(p + 2);
    eproj_instr_list_ptr[v7] = p + 4 - base;
  }
}

static void CallEprojFunc(uint32 ea, uint32 k) {
  switch (ea) {
  case fnSpawnMotherBrainDeathBeam: SpawnMotherBrainDeathBeam(k); return;
  default: Unreachable();
  }
}

void EnableEprojs(void) {  // 0x868000
  eproj_enable_flag |= 0x8000;
}

void DisableEprojs(void) {  // 0x86800B
  eproj_enable_flag &= ~0x8000;
}

void ClearEprojs(void) {  // 0x868016
  for (int i = 34; i >= 0; i -= 2)
    eproj_id[i >> 1] = 0;
}

void SpawnEprojWithGfx(uint16 a, uint16 k, uint16 j) {  // 0x868027
  eproj_init_param_1 = a;
  EnemyData *E = gEnemyData(k);
  SpawnEprojInner(j, E->vram_tiles_index | E->palette_index);
}

void SpawnEprojWithRoomGfx(uint16 j, uint16 a) {  // 0x868097
  eproj_init_param_1 = a;
  SpawnEprojInner(j, 0);
}

void EprojRunAll(void) {  // 0x868104
  if ((eproj_enable_flag & 0x8000) != 0) {
    for (int i = 34; i >= 0; i -= 2) {
      eproj_index = i;
      if (eproj_id[i >> 1]) {
        EprojRunOne(i);
        i = eproj_index;
      }
    }
  }
}

static uint8 EprojColl_8506(EprojCollInfo *eci) {  // 0x868506
  if (BTS[cur_block_index]) {
    cur_block_index += (int8)BTS[cur_block_index];
    return 0xff;
  }
  return 0;
}

static uint8 EprojColl_Unknown8536(EprojCollInfo *eci) {  // 0x86853C
  if (BTS[cur_block_index]) {
    cur_block_index += (int8)BTS[cur_block_index] * room_width_in_blocks;
    return 0xff;
  }
  return 0;
}

static uint8 EprojColl_ClearCarry(EprojCollInfo *eci) {  // 0x86858A
  return 0;
}

static uint8 EprojColl_SetCarry(EprojCollInfo *eci) {  // 0x86858C
  return 1;
}

static uint8 EprojColl_858E(EprojCollInfo *eci) {  // 0x86858E
  uint16 v0 = BTS[cur_block_index] & 0x1F;
  if (v0 < 5)
    return EprojColl_85C2(eci, v0, cur_block_index);
  current_slope_bts = BTS[cur_block_index];
  return EprojColl_873D();
}

static uint8 EprojColl_85AD(EprojCollInfo *eci) {  // 0x8685AD
  uint16 v0 = BTS[cur_block_index] & 0x1F;
  if (v0 >= 5)
    return EprojColl_874E(eci);
  else
    return EprojColl_8676(eci, v0, cur_block_index, 0);
}

static uint8 EprojColl_85C2(EprojCollInfo *eci, uint16 a, uint16 k) {
  uint16 temp_collision_DD4 = 4 * a;
  uint16 temp_collision_DD6 = BTS[k] >> 6;
  uint16 v2 = 4 * a + (temp_collision_DD6 ^ ((eci->eci_r34 & 8) >> 3));
  if (!eci->eci_r32) {
    int v3 = eproj_index >> 1;
    if (((eproj_y_pos[v3] - eci->eci_r30) & 8) != 0
        || CHECK_locret_868728(v2) >= 0) {
      uint16 v4 = v2 ^ 2;
      if (((eci->eci_r30 + eproj_y_pos[v3] - 1) & 8) == 0
          || CHECK_locret_868728(v4) >= 0) {
        return 0;
      }
    }
    goto LABEL_17;
  }
  if (!eci->eci_r26) {
    if (((eci->eci_r30 + eproj_y_pos[eproj_index >> 1] - 1) & 8) == 0) {
      if (CHECK_locret_868728(v2) >= 0)
        return 0;
      goto LABEL_17;
    }
    goto LABEL_14;
  }
  if (eci->eci_r26 != eci->eci_r32 || ((eproj_y_pos[eproj_index >> 1] - eci->eci_r30) & 8) == 0) {
LABEL_14:
    if (CHECK_locret_868728(v2) < 0)
      goto LABEL_17;
  }
  if (CHECK_locret_868728(v2 ^ 2) >= 0)
    return 0;
LABEL_17:;
  int v6 = eproj_index >> 1;
  uint16 v7;
  eproj_x_subpos[v6] = 0;
  if ((eci->eci_r20 & 0x8000) != 0)
    v7 = eci->eci_r28 + (eci->eci_r34 | 7) + 1;
  else
    v7 = (eci->eci_r34 & 0xFFF8) - eci->eci_r28;
  eproj_x_pos[v6] = v7;
  return 1;
}

static uint8 EprojColl_8676(EprojCollInfo *eci, uint16 a, uint16 k, uint16 j) {  // 0x868676
  uint16 v2 = eproj_index;

  uint16 temp_collision_DD4 = 4 * a;
  uint16 temp_collision_DD6 = BTS[k] >> 6;
  uint16 v3 = 4 * a + (temp_collision_DD6 ^ ((eci->eci_r34 & 8) >> 2));
  if (!eci->eci_r32) {
    int v4 = v2 >> 1;
    if (((eproj_x_pos[v4] - eci->eci_r28) & 8) != 0
        || CHECK_locret_868728(v3) >= 0) {
      uint16 v5 = v3 ^ 1;
      if (((eci->eci_r28 + eproj_x_pos[v4] - 1) & 8) == 0
          || CHECK_locret_868728(v5) >= 0) {
        return 0;
      }
    }
    goto LABEL_17;
  }
  if (!eci->eci_r26) {
    if (((eci->eci_r28 + eproj_x_pos[eproj_index >> 1] - 1) & 8) == 0) {
      if (CHECK_locret_868728(v3) >= 0)
        return 0;
      goto LABEL_17;
    }
    goto LABEL_14;
  }
  if (eci->eci_r26 != eci->eci_r32 || ((eproj_x_pos[eproj_index >> 1] - eci->eci_r28) & 8) == 0) {
LABEL_14:
    if (CHECK_locret_868728(v3) < 0)
      goto LABEL_17;
  }
  if (CHECK_locret_868728(v3 ^ 1) >= 0)
    return 0;
LABEL_17:;
  int v7 = eproj_index >> 1;
  uint16 v8;
  eproj_y_subpos[v7] = 0;
  if ((eci->eci_r20 & 0x8000) != 0)
    v8 = eci->eci_r30 + (eci->eci_r34 | 7) + 1;
  else
    v8 = (eci->eci_r34 & 0xFFF8) - eci->eci_r30;
  eproj_y_pos[v7] = v8;
  return 1;
}

uint8 EprojColl_873D(void) {  // 0x86873D
  return 0;
}

static uint8 EprojColl_874E(EprojCollInfo *eci) {  // 0x86874E
  int16 v3;
  int16 v5;
  uint16 v6;
  uint16 v7;
  int16 v8;
  int16 v11;
  int16 v12;
  uint16 v13;
  uint16 v14;
  int16 v15;

  uint16 v0 = eproj_index;
  if ((eci->eci_r20 & 0x8000) != 0) {
    uint16 v9 = cur_block_index;
    uint16 mod = SnesModulus(cur_block_index, room_width_in_blocks);
    int v10 = v0 >> 1;
    v11 = eproj_x_pos[v10] >> 4;
    if (v11 == mod) {
      uint16 temp_collision_DD4 = (eci->eci_r24 - eci->eci_r30) & 0xF ^ 0xF;
      uint16 temp_collision_DD6 = 16 * (BTS[v9] & 0x1F);
      v12 = BTS[v9] << 8;
      if (v12 < 0
          && ((v12 & 0x4000) != 0 ? (v13 = eproj_x_pos[v10] ^ 0xF) : (v13 = eproj_x_pos[v10]),
              (v14 = temp_collision_DD6 + (v13 & 0xF),
               v15 = (kAlignYPos_Tab0[v14] & 0x1F) - temp_collision_DD4 - 1,
               (kAlignYPos_Tab0[v14] & 0x1F) - temp_collision_DD4 == 1)
              || v15 < 0)) {
        eproj_y_pos[v10] = eci->eci_r24 - v15;
        eproj_y_subpos[v10] = 0;
        return 1;
      } else {
        return 0;
      }
    } else {
      return 0;
    }
  } else {
    uint16 v1 = cur_block_index;
    uint16 mod = SnesModulus(cur_block_index, room_width_in_blocks);
    int v2 = v0 >> 1;
    v3 = eproj_x_pos[v2] >> 4;
    if (v3 == mod) {
      uint16 temp_collision_DD4 = (eci->eci_r30 + eci->eci_r24 - 1) & 0xF;
      uint16 temp_collision_DD6 = 16 * (BTS[v1] & 0x1F);
      v5 = BTS[v1] << 8;
      if (v5 >= 0
          && ((v5 & 0x4000) != 0 ? (v6 = eproj_x_pos[v2] ^ 0xF) : (v6 = eproj_x_pos[v2]),
              (v7 = temp_collision_DD6 + (v6 & 0xF),
               v8 = (kAlignYPos_Tab0[v7] & 0x1F) - temp_collision_DD4 - 1,
               (kAlignYPos_Tab0[v7] & 0x1F) - temp_collision_DD4 == 1)
              || v8 < 0)) {
        eproj_y_pos[v2] = eci->eci_r24 + v8;
        eproj_y_subpos[v2] = -1;
        return 1;
      } else {
        return 0;
      }
    } else {
      return 0;
    }
  }
}

typedef uint8 Func_EprojCollInfo_U8(EprojCollInfo *eci);

static Func_EprojCollInfo_U8 *const kEprojBlockCollisition_FuncA[16] = {  // 0x868886
  EprojColl_ClearCarry,
  EprojColl_858E,
  EprojColl_ClearCarry,
  EprojColl_ClearCarry,
  EprojColl_ClearCarry,
  EprojColl_8506,
  EprojColl_ClearCarry,
  EprojColl_ClearCarry,
  EprojColl_SetCarry,
  EprojColl_SetCarry,
  EprojColl_SetCarry,
  EprojColl_SetCarry,
  EprojColl_SetCarry,
  EprojColl_Unknown8536,
  EprojColl_ClearCarry,
  EprojColl_SetCarry,
};

static Func_EprojCollInfo_U8 *const kEprojBlockCollisition_FuncB[16] = {
  EprojColl_ClearCarry,
  EprojColl_85AD,
  EprojColl_ClearCarry,
  EprojColl_ClearCarry,
  EprojColl_ClearCarry,
  EprojColl_8506,
  EprojColl_ClearCarry,
  EprojColl_ClearCarry,
  EprojColl_SetCarry,
  EprojColl_SetCarry,
  EprojColl_SetCarry,
  EprojColl_SetCarry,
  EprojColl_SetCarry,
  EprojColl_Unknown8536,
  EprojColl_ClearCarry,
  EprojColl_SetCarry,
};

static uint8 EprojBlockCollisition_CheckHorizontal(EprojCollInfo *eci, uint16 k) {
  uint8 rv;
  cur_block_index = k >> 1;
  do {
    rv = kEprojBlockCollisition_FuncA[(level_data[cur_block_index] & 0xF000) >> 12](eci);
  } while (rv & 0x80);
  return rv;
}

static uint8 EprojBlockCollisition_CheckVertical(EprojCollInfo *eci, uint16 k) {  // 0x86889E
  uint8 rv;
  cur_block_index = k >> 1;
  do {
    rv = kEprojBlockCollisition_FuncB[(level_data[cur_block_index] & 0xF000) >> 12](eci);
  } while (rv & 0x80);
  return rv;
}

uint8 EprojBlockCollisition_Horiz(uint16 k) {  // 0x8688B6
  int v1 = k >> 1;
  int32 amt = INT16_SHL8(eproj_x_vel[v1]);
  uint16 R30 = HIBYTE(eproj_radius[v1]);
  uint16 R28 = LOBYTE(eproj_radius[v1]);
  uint16 r26 = (eproj_y_pos[v1] - R30) & 0xFFF0;
  r26 = (uint16)(R30 + eproj_y_pos[v1] - 1 - r26) >> 4;
  uint16 R32 = r26;
  uint16 prod = Mult8x8((uint16)(eproj_y_pos[v1] - R30) >> 4, room_width_in_blocks);
  uint16 v3 = (amt + __PAIR32__(eproj_x_pos[v1], eproj_x_subpos[v1])) >> 16;
  uint16 r22 = amt + eproj_x_subpos[v1];
  uint16 r24 = v3;
  uint16 v4;
  if (amt < 0)
    v4 = v3 - R28;
  else
    v4 = R28 + v3 - 1;
  EprojCollInfo eci = { .eci_r20 = amt >> 16, .eci_r24 = r24, .eci_r26 = r26, .eci_r28 = R28, .eci_r30 = R30, .eci_r32 = R32, .eci_r34 = v4 };
  uint16 v5 = 2 * (prod + (v4 >> 4));
  while (!(EprojBlockCollisition_CheckHorizontal(&eci, v5) & 1)) {
    v5 += room_width_in_blocks * 2;
    if ((--eci.eci_r26 & 0x8000) != 0) {
      int v6 = k >> 1;
      eproj_x_subpos[v6] = r22;
      eproj_x_pos[v6] = r24;
      return 0;
    }
  }
  int v8 = k >> 1;
  eproj_x_subpos[v8] = 0;
  if (amt < 0) {
    uint16 v10 = eci.eci_r28 + (eci.eci_r34 | 0xF) + 1;
    if (v10 <= eproj_x_pos[v8])
      eproj_x_pos[v8] = v10;
  } else {
    uint16 v9 = (eci.eci_r34 & 0xFFF0) - eci.eci_r28;
    if (v9 >= eproj_x_pos[v8])
      eproj_x_pos[v8] = v9;
  }
  return 1;
}

uint8 EprojBlockCollisition_Vertical(uint16 k) {  // 0x86897B
  int16 v5;

  int v1 = k >> 1;
  int32 amt = INT16_SHL8(eproj_y_vel[v1]);
  uint16 R30 = HIBYTE(eproj_radius[v1]);
  uint16 R28 = LOBYTE(eproj_radius[v1]);
  uint16 r26 = (eproj_x_pos[v1] - R28) & 0xFFF0;
  r26 = (uint16)(R28 + eproj_x_pos[v1] - 1 - r26) >> 4;
  uint16 R32 = r26;
  uint16 v3 = (amt + __PAIR32__(eproj_y_pos[v1], eproj_y_subpos[v1])) >> 16;
  uint16 r22 = amt + eproj_y_subpos[v1];
  uint16 r24 = v3;
  uint16 v4;
  if (amt < 0)
    v4 = v3 - R30;
  else
    v4 = R30 + v3 - 1;
  EprojCollInfo eci = { .eci_r20 = amt >> 16, .eci_r24 = r24, .eci_r26 = r26, .eci_r28 = R28, .eci_r30 = R30, .eci_r32 = R32, .eci_r34 = v4 };
  uint16 prod = Mult8x8(v4 >> 4, room_width_in_blocks);
  v5 = (uint16)(eproj_x_pos[v1] - R28) >> 4;
  for (int i = 2 * (prod + v5); !(EprojBlockCollisition_CheckVertical(&eci, i) & 1); i += 2) {
    if ((--eci.eci_r26 & 0x8000) != 0) {
      int v7 = k >> 1;
      eproj_y_subpos[v7] = r22;
      eproj_y_pos[v7] = r24;
      return 0;
    }
  }
  int v9 = k >> 1;
  eproj_y_subpos[v9] = 0;
  if (amt < 0) {
    uint16 v11 = eci.eci_r30 + (eci.eci_r34 | 0xF) + 1;
    if (v11 <= eproj_y_pos[v9])
      eproj_y_pos[v9] = v11;
  } else {
    uint16 v10 = (eci.eci_r34 & 0xFFF0) - eci.eci_r30;
    if (v10 >= eproj_y_pos[v9])
      eproj_y_pos[v9] = v10;
  }
  return 1;
}

uint16 MoveEprojWithVelocity(uint16 k) {  // 0x8692D6
  int v1 = k >> 1;
  uint16 v2 = eproj_x_vel[v1];
  int carry = *((uint8 *)eproj_x_subpos + k + 1) + (v2 & 0xff);
  *((uint8 *)eproj_x_subpos + k + 1) = carry;
  eproj_x_pos[v1] += (int8)(v2 >> 8) + (carry >> 8);
  return MoveEprojWithVelocityY(k);
}

uint16 MoveEprojWithVelocityY(uint16 k) {  // 0x8692F3
  int v1 = k >> 1;
  uint16 v2 = eproj_y_vel[v1];
  int carry = *((uint8 *)eproj_y_subpos + k + 1) + (v2 & 0xff);
  *((uint8 *)eproj_y_subpos + k + 1) = carry;
  uint16 result = eproj_y_pos[v1] + (int8)(v2 >> 8) + (carry >> 8);
  eproj_y_pos[v1] = result;
  return result;
}

uint16 MoveEprojWithVelocityX(uint16 k) {  // 0x869311
  int v1 = k >> 1;
  uint16 v2 = eproj_x_vel[v1];
  int carry = *((uint8 *)eproj_x_subpos + k + 1) + (v2 & 0xff);
  *((uint8 *)eproj_x_subpos + k + 1) = carry;
  uint16 result = eproj_x_pos[v1] + (int8)(v2 >> 8) + (carry >> 8);
  eproj_x_pos[v1] = result;
  return result;
}

void SetAreaDependentEprojPropertiesEx(uint16 k, uint16 j) {  // 0x86932F
  uint16 v2;
  uint16 *p = (uint16 *)RomPtr_86(k);
  if (area_index == 2) {
    v2 = p[1];
  } else if (area_index == 5) {
    v2 = p[2];
  } else {
    v2 = p[0];
  }
  eproj_properties[j >> 1] = v2;
}

void SetAreaDependentEprojProperties(uint16 j) {  // 0x869402
  SetAreaDependentEprojPropertiesEx(addr_kRidleysFireball_Tab0, j);
}

void Eproj_DeleteIfYposOutside(uint16 k) {  // 0x86B5B9
  int v1 = k >> 1;
  if ((int16)(eproj_y_pos[v1] - layer1_y_pos) >= 288)
    eproj_id[v1] = 0;
}

const uint8 *EprojInstr_Delete(uint16 k, const uint8 *epjp) {  // 0x868154
  eproj_id[k >> 1] = 0;
  return 0;
}

const uint8 *EprojInstr_Sleep(uint16 k, const uint8 *epjp) {  // 0x868159
  eproj_instr_list_ptr[k >> 1] = epjp - RomBankBase(0x86) - 2;
  return 0;
}

const uint8 *EprojInstr_SetPreInstr_(uint16 k, const uint8 *epjp) {  // 0x868161
  eproj_pre_instr[k >> 1] = GET_WORD(epjp);
  return epjp + 2;
}

const uint8 *EprojInstr_ClearPreInstr(uint16 k, const uint8 *epjp) {  // 0x86816A
  eproj_pre_instr[k >> 1] = 0x8170;
  return epjp;
}

const uint8 *EprojInstr_CallFunc(uint16 k, const uint8 *epjp) {  // 0x868171
  CallEprojFunc(Load24((LongPtr *)epjp), k);
  return epjp + 3;
}

const uint8 *EprojInstr_Goto(uint16 k, const uint8 *epjp) {  // 0x8681AB
  return INSTRB_RETURN_ADDR(GET_WORD(epjp));
}

const uint8 *EprojInstr_GotoRel(uint16 k, const uint8 *epjp) {  // 0x8681B0
  return epjp + (int8)*epjp;
}

const uint8 *EprojInstr_DecTimerAndGotoIfNonZero(uint16 k, const uint8 *epjp) {  // 0x8681C6
  int v2 = k >> 1;
  if (eproj_timers[v2]-- == 1)
    return epjp + 2;
  else
    return EprojInstr_Goto(k, epjp);
}

const uint8 *EprojInstr_DecTimerAndGotoRelIfNonZero(uint16 k, const uint8 *epjp) {  // 0x8681CE
  int v2 = k >> 1;
  if (eproj_timers[v2]-- == 1)
    return epjp + 1;
  else
    return EprojInstr_GotoRel(k, epjp);
}

const uint8 *EprojInstr_SetTimer(uint16 k, const uint8 *epjp) {  // 0x8681D5
  eproj_timers[k >> 1] = GET_WORD(epjp);
  return epjp + 2;
}

const uint8 *EprojInstr_MoveRandomlyWithinRadius(uint16 k, const uint8 *epjp) {  // 0x8681DF
  int8 Random;
  int8 v4;
  int16 v6;

  uint16 r18 = NextRandom();
  do {
    Random = NextRandom();
    v4 = (*epjp & Random) - epjp[1];
  } while (v4 < 0);
  eproj_x_pos[k >> 1] += sign16(r18) ? -(uint8)v4 : (uint8)v4;
  do {
    LOBYTE(v6) = NextRandom();
    LOBYTE(v6) = (epjp[2] & v6) - epjp[3];
  } while ((v6 & 0x80) != 0);
  v6 = (uint8)v6;
  if ((r18 & 0x4000) != 0)
    v6 = -(uint8)v6;
  eproj_y_pos[k >> 1] += v6;
  return epjp + 4;
}

const uint8 *EprojInstr_SetProjectileProperties(uint16 k, const uint8 *epjp) {  // 0x868230
  eproj_properties[k >> 1] |= GET_WORD(epjp);
  return epjp + 2;
}

const uint8 *EprojInstr_ClearProjectileProperties(uint16 k, const uint8 *epjp) {  // 0x86823C
  eproj_properties[k >> 1] &= GET_WORD(epjp);
  return epjp + 2;
}

const uint8 *EprojInstr_EnableCollisionWithSamusProj(uint16 k, const uint8 *epjp) {  // 0x868248
  eproj_properties[k >> 1] |= 0x8000;
  return epjp;
}

const uint8 *EprojInstr_DisableCollisionWithSamusProj(uint16 k, const uint8 *epjp) {  // 0x868252
  eproj_properties[k >> 1] &= ~0x8000;
  return epjp;
}

const uint8 *EprojInstr_DisableCollisionWithSamus(uint16 k, const uint8 *epjp) {  // 0x86825C
  eproj_properties[k >> 1] |= 0x2000;
  return epjp;
}

const uint8 *EprojInstr_EnableCollisionWithSamus(uint16 k, const uint8 *epjp) {  // 0x868266
  eproj_properties[k >> 1] &= ~0x2000;
  return epjp;
}

const uint8 *EprojInstr_SetToNotDieOnContact(uint16 k, const uint8 *epjp) {  // 0x868270
  eproj_properties[k >> 1] |= 0x4000;
  return epjp;
}

const uint8 *EprojInstr_SetToDieOnContact(uint16 k, const uint8 *epjp) {  // 0x86827A
  eproj_properties[k >> 1] &= ~0x4000;
  return epjp;
}

const uint8 *EprojInstr_SetLowPriority(uint16 k, const uint8 *epjp) {  // 0x868284
  eproj_properties[k >> 1] |= 0x1000;
  return epjp;
}

const uint8 *EprojInstr_SetHighPriority(uint16 k, const uint8 *epjp) {  // 0x86828E
  eproj_properties[k >> 1] &= ~0x1000;
  return epjp;
}

const uint8 *EprojInstr_SetXyRadius(uint16 k, const uint8 *epjp) {  // 0x868298
  eproj_radius[k >> 1] = GET_WORD(epjp);
  return epjp + 2;
}

const uint8 *EprojInstr_SetXyRadiusZero(uint16 k, const uint8 *epjp) {  // 0x8682A1
  eproj_radius[k >> 1] = 0;
  return epjp;
}

const uint8 *EprojInstr_CalculateDirectionTowardsSamus(uint16 k, const uint8 *epjp) {  // 0x8682A5
  int v2 = k >> 1;
  uint16 x = samus_x_pos - eproj_x_pos[v2];
  uint16 y = samus_y_pos - eproj_y_pos[v2];
  uint16 v3 = 2 * CalculateAngleFromXY(x, y);
  int v4 = eproj_index >> 1;
  eproj_E[v4] = v3;
  int v5 = v3 >> 1;
  eproj_x_vel[v4] = kSinCosTable8bit_Sext[v5 + 64];
  eproj_y_vel[v4] = kSinCosTable8bit_Sext[v5];
  return epjp;
}

const uint8 *EprojInstr_WriteColorsToPalette(uint16 k, const uint8 *epjp) {  // 0x8682D5
  uint16 v3 = GET_WORD(epjp + 2);
  int n = epjp[4];
  uint16 v4 = GET_WORD(epjp);
  do {
    palette_buffer[v3 >> 1] = *(uint16 *)RomPtr_86(v4);
    v4 += 2;
    v3 += 2;
  } while ((--n & 0x8000) == 0);
  return epjp + 5;
}

const uint8 *EprojInstr_QueueMusic(uint16 k, const uint8 *epjp) {  // 0x8682FD
  QueueMusic_Delayed8(*epjp);
  return epjp + 1;
}

const uint8 *EprojInstr_QueueSfx1_Max6(uint16 k, const uint8 *epjp) {  // 0x868309
  QueueSfx1_Max6(*epjp);
  return epjp + 1;
}

const uint8 *EprojInstr_QueueSfx2_Max6(uint16 k, const uint8 *epjp) {  // 0x868312
  QueueSfx2_Max6(*epjp);
  return epjp + 1;
}

const uint8 *EprojInstr_QueueSfx3_Max6(uint16 k, const uint8 *epjp) {  // 0x86831B
  QueueSfx3_Max6(*epjp);
  return epjp + 1;
}

const uint8 *EprojInstr_QueueSfx1_Max15(uint16 k, const uint8 *epjp) {  // 0x868324
  QueueSfx1_Max15(*epjp);
  return epjp + 1;
}

const uint8 *EprojInstr_QueueSfx2_Max15(uint16 k, const uint8 *epjp) {  // 0x86832D
  QueueSfx2_Max15(*epjp);
  return epjp + 1;
}

const uint8 *EprojInstr_QueueSfx3_Max15(uint16 k, const uint8 *epjp) {  // 0x868336
  QueueSfx3_Max15(*epjp);
  return epjp + 1;
}

const uint8 *EprojInstr_QueueSfx1_Max3(uint16 k, const uint8 *epjp) {  // 0x86833F
  QueueSfx1_Max3(*epjp);
  return epjp + 1;
}

const uint8 *EprojInstr_QueueSfx2_Max3(uint16 k, const uint8 *epjp) {  // 0x868348
  QueueSfx2_Max3(*epjp);
  return epjp + 1;
}

const uint8 *EprojInstr_QueueSfx3_Max3(uint16 k, const uint8 *epjp) {  // 0x868351
  QueueSfx3_Max3(*epjp);
  return epjp + 1;
}

const uint8 *EprojInstr_QueueSfx1_Max9(uint16 k, const uint8 *epjp) {  // 0x86835A
  QueueSfx1_Max9(*epjp);
  return epjp + 1;
}

const uint8 *EprojInstr_QueueSfx2_Max9(uint16 k, const uint8 *epjp) {  // 0x868363
  QueueSfx2_Max9(*epjp);
  return epjp + 1;
}

const uint8 *EprojInstr_QueueSfx3_Max9(uint16 k, const uint8 *epjp) {  // 0x86836C
  QueueSfx3_Max9(*epjp);
  return epjp + 1;
}

const uint8 *EprojInstr_QueueSfx1_Max1(uint16 k, const uint8 *epjp) {  // 0x868375
  QueueSfx1_Max1(*epjp);
  return epjp + 1;
}

const uint8 *EprojInstr_QueueSfx2_Max1(uint16 k, const uint8 *epjp) {  // 0x86837E
  QueueSfx2_Max1(*epjp);
  return epjp + 1;
}

const uint8 *EprojInstr_QueueSfx3_Max1(uint16 k, const uint8 *epjp) {  // 0x868387
  QueueSfx3_Max1(*epjp);
  return epjp + 1;
}

void DrawLowPriorityEprojs(void) {  // 0x868390
  Point16U pt = GetValuesForScreenShaking();
  for (int i = 34; i >= 0; i -= 2) {
    int v1 = i >> 1;
    if (eproj_id[v1]) {
      if ((eproj_properties[v1] & 0x1000) != 0)
        DrawEprojs(i, pt);
    }
  }
}

void DrawHighPriorityEprojs(void) {  // 0x8683B2
  Point16U pt = GetValuesForScreenShaking();
  for (int i = 34; i >= 0; i -= 2) {
    int v1 = i >> 1;
    if (eproj_id[v1]) {
      if ((eproj_properties[v1] & 0x1000) == 0)
        DrawEprojs(i, pt);
    }
  }
}

void DrawEprojs(uint16 k, Point16U pt) {  // 0x8683D6
  int v1 = k >> 1;
  uint16 v2 = eproj_spritemap_ptr[v1];
  uint16 r26 = LOBYTE(eproj_gfx_idx[v1]);
  uint16 R28 = eproj_gfx_idx[v1] & 0xFF00;
  uint16 r20 = pt.x + eproj_x_pos[v1] - layer1_x_pos;
  if (((r20 + 128) & 0xFE00) == 0) {
    uint16 v3 = pt.y + eproj_y_pos[v1] - layer1_y_pos;
    uint16 r18 = v3;
    if ((v3 & 0xFF00) != 0) {
      if (((v3 + 128) & 0xFE00) == 0)
        DrawEprojSpritemapWithBaseTileOffscreen(0x8D, v2, r20, r18, r26, R28);
    } else {
      DrawEprojSpritemapWithBaseTile(0x8D, v2, r20, r18, r26, R28);
    }
  }
}

Point16U GetValuesForScreenShaking(void) {  // 0x868427
  uint16 R34, R36;
  if (earthquake_timer && !time_is_frozen_flag && sign16(earthquake_type - 36)) {
    int v0 = (uint16)(4 * earthquake_type) >> 1;
    if ((earthquake_timer & 2) != 0) {
      R36 = -kScreenShakeOffsets[v0];
      R34 = -kScreenShakeOffsets[v0 + 1];
    } else {
      R36 = kScreenShakeOffsets[v0];
      R34 = kScreenShakeOffsets[v0 + 1];
    }
  } else {
    R34 = 0;
    R36 = 0;
  }
  return (Point16U){R36, R34};
}
