// Enemy AI - Ceres door + Ceres steam — peeled from Bank $A6
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"

#define g_off_A6EFF5 ((uint16*)RomFixedPtr(0xa6eff5))
#define g_off_A6F001 ((uint16*)RomFixedPtr(0xa6f001))
#define g_off_A6F72B ((uint16*)RomFixedPtr(0xa6f72b))
#define g_off_A6F52C ((uint16*)RomFixedPtr(0xa6f52c))
#define g_word_A6F840 ((uint16*)RomFixedPtr(0xa6f840))
#define g_off_A6F900 ((uint16*)RomFixedPtr(0xa6f900))

void CeresSteam_Init(void) {  // 0xA6EFB1
  Enemy_CeresSteam *E = Get_CeresSteam(cur_enemy_index);
  E->base.vram_tiles_index = 0;
  E->base.properties |= kEnemyProps_ProcessInstructions;
  E->base.extra_properties |= 4;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->base.palette_index = 2560;
  E->csm_var_D = (NextRandom() & 0x1F) + 1;
  int v1 = E->csm_parameter_1;
  E->base.current_instruction = g_off_A6EFF5[v1];
  E->csm_var_A = g_off_A6F001[v1];
}

void CallCeresSteamFunc(uint32 ea, uint16 k) {
  switch (ea) {
  case fnnullsub_350: return;  // 0xa6eff4
  case fnCeresSteam_Func_1: CeresSteam_Func_1(k); return;  // 0xa6f019
  default: Unreachable();
  }
}

void CeresSteam_Main(void) {  // 0xA6F00D
  Enemy_CeresSteam *E = Get_CeresSteam(cur_enemy_index);
  E->base.health = 0x7FFF;
  CallCeresSteamFunc(E->csm_var_A | 0xA60000, cur_enemy_index);
}

void CeresSteam_Func_1(uint16 k) {  // 0xA6F019
  Enemy_CeresSteam *E = Get_CeresSteam(k);
  Point16U pt = CalcCeresSteamPos_Mode7((Point16U) { E->base.x_pos, E->base.y_pos });
  EnemySpawnData *v3 = gEnemySpawnData(cur_enemy_index);
  v3->xpos2 = pt.x - E->base.x_pos;
  v3->ypos2 = pt.y - E->base.y_pos;
}

void CeresSteam_Touch(void) {  // 0xA6F03F
  Get_CeresSteam(cur_enemy_index)->base.health = 0x7FFF;
  NormalEnemyTouchAi();
}

const uint16 *CeresSteam_Instr_1(uint16 k, const uint16 *jp) {  // 0xA6F11D
  Enemy_CeresSteam *E = Get_CeresSteam(k);
  E->base.properties |= kEnemyProps_Intangible | kEnemyProps_Invisible;
  return jp;
}

const uint16 *CeresSteam_Instr_2(uint16 k, const uint16 *jp) {  // 0xA6F127
  Enemy_CeresSteam *E = Get_CeresSteam(k);
  if (E->csm_var_D-- != 1)
    return INSTR_RETURN_ADDR(jp[0]);
  return CeresSteam_Instr_3(k, (uint16 *)RomPtr_A6(jp[1]));
}

const uint16 *CeresSteam_Instr_3(uint16 k, const uint16 *jp) {  // 0xA6F135
  Enemy_CeresSteam *E = Get_CeresSteam(k);
  E->base.properties &= 0xFAFF;
  return jp;
}

const uint16 *CeresDoor_Instr_6(uint16 k, const uint16 *jp) {  // 0xA6F63E
  Enemy_CeresDoor *E = Get_CeresDoor(k);
  if (abs16(E->base.x_pos - samus_x_pos) >= 0x30 || abs16(E->base.y_pos - samus_y_pos) >= 0x30)
    return INSTR_RETURN_ADDR(*jp);
  else
    return jp + 1;
}

const uint16 *CeresDoor_Instr_4(uint16 k, const uint16 *jp) {  // 0xA6F66A
  if (*(uint16 *)&boss_bits_for_area[area_index] & 1)
    return jp + 1;
  else
    return INSTR_RETURN_ADDR(*jp);
}

const uint16 *CeresDoor_Instr_8(uint16 k, const uint16 *jp) {  // 0xA6F678
  if (ceres_status)
    return jp + 1;
  else
    return INSTR_RETURN_ADDR(*jp);
}

void CeresDoor_Func_6b(void) {  // 0xA6F67F
  if (ceres_status)
    ceres_status = 0x8000;
}

const uint16 *CeresSteam_Instr_4(uint16 k, const uint16 *jp) {  // 0xA6F68B
  Enemy_CeresSteam *E = Get_CeresSteam(k);
  E->base.properties |= kEnemyProps_Intangible;
  return jp;
}

const uint16 *CeresDoor_Instr_1(uint16 k, const uint16 *jp) {  // 0xA6F695
  Enemy_CeresDoor *E = Get_CeresDoor(k);
  E->base.properties &= ~kEnemyProps_Intangible;
  return jp;
}

const uint16 *CeresDoor_Instr_3(uint16 k, const uint16 *jp) {  // 0xA6F69F
  Get_CeresDoor(k)->cdr_var_B = 1;
  return jp;
}

const uint16 *CeresSteam_Instr_5(uint16 k, const uint16 *jp) {  // 0xA6F6A6
  Enemy_CeresSteam *E = Get_CeresSteam(k);
  E->base.properties |= kEnemyProps_Invisible;
  return jp;
}

const uint16 *CeresDoor_Instr_5(uint16 k, const uint16 *jp) {  // 0xA6F6B0
  Get_CeresDoor(k)->cdr_var_B = 0;
  return CeresDoor_Instr_2(k, jp);
}

const uint16 *CeresDoor_Instr_2(uint16 k, const uint16 *jp) {  // 0xA6F6B3
  Enemy_CeresDoor *E = Get_CeresDoor(k);
  E->base.properties &= ~kEnemyProps_Invisible;
  return jp;
}

const uint16 *CeresDoor_Instr_7(uint16 k, const uint16 *jp) {  // 0xA6F6BD
  QueueSfx3_Max6(0x2C);
  return jp;
}

void CeresDoor_Init(void) {  // 0xA6F6C5
  Enemy_CeresDoor *E = Get_CeresDoor(cur_enemy_index);
  E->base.spritemap_pointer = addr_kCeresDoor_Sprmap_FAC7;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->base.vram_tiles_index = 0;
  E->base.palette_index = 1024;
  int v2 = E->cdr_parameter_1;
  E->cdr_var_A = g_off_A6F72B[v2];
  E->base.current_instruction = g_off_A6F52C[v2];
  E->cdr_var_B = 0;
  CeresDoor_Func_1(cur_enemy_index);
  uint16 v3;
  if (ceres_status) {
    v3 = addr_word_A6F50C + 2;
  } else {
    if (E->cdr_parameter_1 == 3) {
      WriteColorsToTargetPalette(0xa6, 0x142, addr_kCeresDoor_Palette + 2, 0xF);
      return;
    }
    v3 = addr_kCeresDoor_Palette + 2;
  }
  Get_CeresDoor(cur_enemy_index)->base.palette_index = 3584;
  WriteColorsToTargetPalette(0xa6, 0x1E2, v3, 0xF);
}

void CeresDoor_Func_1(uint16 k) {  // 0xA6F739
  VramWriteEntry *v2;

  if (Get_CeresDoor(k)->cdr_parameter_1 == 2) {
    uint16 v1 = vram_write_queue_tail;
    v2 = gVramWriteEntry(vram_write_queue_tail);
    v2->size = 1024;
    *(VoidP *)((uint8 *)&v2->src.addr + 1) = -20480;
    v2->src.addr = -15360;
    v2->vram_dst = 28672;
    vram_write_queue_tail = v1 + 7;
  }
}

void CallCeresDoor(uint32 ea) {
  uint16 k = cur_enemy_index;
  switch (ea) {
  case fnCeresDoor_Func_2: CeresDoor_Func_2(); return;  // 0xa6f76b
  case fnCeresDoor_Func_3: CeresDoor_Func_3(); return;  // 0xa6f770
  case fnCeresDoor_Func_4: CeresDoor_Func_4(); return;  // 0xa6f7a5
  case fnCeresDoor_Func_5: CeresDoor_Func_5(k); return;  // 0xa6f7bd
  case fnCeresDoor_Func_6: CeresDoor_Func_6(k); return;  // 0xa6f7dc
  case fnCeresDoor_Func_7: CeresDoor_Func_7(); return;  // 0xa6f850
  default: Unreachable();
  }
}

void CeresDoor_Main(void) {  // 0xA6F765
  Enemy_CeresDoor *E = Get_CeresDoor(cur_enemy_index);
  CallCeresDoor(E->cdr_var_A | 0xA60000);
}

void CeresDoor_Func_2(void) {  // 0xA6F76B
  CeresDoor_F773(0x14);
}

void CeresDoor_Func_3(void) {  // 0xA6F770
  CeresDoor_F773(0x1D);
}

void CeresDoor_F773(uint16 j) {  // 0xA6F773
  if (ceres_status >= 2 && !earthquake_timer) {
    if ((random_number & 0xFFF) < 0x80) {
      earthquake_timer = 4;
      earthquake_type = j + 6;
    } else {
      earthquake_timer = 2;
      earthquake_type = j;
    }
  }
}

void CeresDoor_Func_4(void) {  // 0xA6F7A5
  CeresSteam_Instr_5(cur_enemy_index, 0);
  if (ceres_status & 1) {
    Get_CeresDoor(cur_enemy_index)->base.palette_index = 3584;
    CeresDoor_Instr_2(cur_enemy_index, 0);
  }
}

void CeresDoor_Func_5(uint16 k) {  // 0xA6F7BD
  CeresDoor_Func_7();
  if (ceres_status >= 2) {
    Enemy_CeresDoor *E = Get_CeresDoor(k);
    E->cdr_var_A = FUNC16(CeresDoor_Func_6);
    E->cdr_var_D = 48;
    Enemy_CeresDoor *E0 = Get_CeresDoor(0);
    E0->cdr_var_E = 0;
    E0->cdr_var_F = 0;
  }
}

void CeresDoor_Func_6(uint16 k) {  // 0xA6F7DC
  Enemy_CeresDoor *E = Get_CeresDoor(k);
  bool v3 = (--E->cdr_var_D & 0x8000) != 0;
  if (v3) {
    E->base.properties |= kEnemyProps_Invisible;
    E->cdr_var_A = FUNC16(CeresDoor_Func_7);
    CeresDoor_Func_6b();
  } else {
    Enemy_CeresDoor *E0 = Get_CeresDoor(0);
    v3 = (--E0->cdr_var_E & 0x8000) != 0;
    if (v3) {
      E0->cdr_var_E = 4;
      v3 = (--E0->cdr_var_F & 0x8000) != 0;
      if (v3)
        E0->cdr_var_F = 3;
      int v5 = (uint16)(4 * E0->cdr_var_F) >> 1;
      eproj_spawn_pt = (Point16U){ E->base.x_pos + g_word_A6F840[v5], E->base.y_pos + g_word_A6F840[v5 + 1] };
      uint16 v6 = 3;
      if (NextRandom() < 0x4000)
        v6 = 12;
      SpawnEprojWithRoomGfx(0xE509, v6);
      QueueSfx2_Max6(0x25);
    }
  }
}

void CeresDoor_Func_7(void) {  // 0xA6F850
  CeresDoor_Func_8();
  if (!palette_change_num)
    WriteColorsToPalette(
      0x52,
      0xa6, 2 * (nmi_frame_counter_word & 0x38) - 0x78F,
      6);
}

void CeresDoor_Func_8(void) {  // 0xA6F8F1
  QueueMode7Transfers(0xA6, g_off_A6F900[(nmi_frame_counter_word & 2) >> 1]);
}
