// Enemy AI - Wrecked Ship Ghost/Orbs/Spark/Robot — peeled from Bank $A8

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

#define g_off_A8CC30 ((uint16*)RomFixedPtr(0xa8cc30))
#define g_off_A8E380 ((uint16*)RomFixedPtr(0xa8e380))
#define g_off_A8E682 ((uint16*)RomFixedPtr(0xa8e682))
#define g_off_A8E688 ((uint16*)RomFixedPtr(0xa8e688))

static const uint16 g_word_A89A9C = 0x10;
static const uint16 g_word_A89A9E = 0x40;
static const uint16 g_word_A89AA0 = 0x1800;
static const uint16 g_word_A89AA2 = 1;
static const uint16 g_word_A89AA4 = 0x78;
static const uint16 g_word_A89AA6 = 0x78;
static const uint16 g_word_A89AA8[18] = { 0xffc0, 0xffc0, 0, 0xffc0, 0x40, 0, 0xffc0, 0, 0, 0, 0x40, 0, 0xffc0, 0x40, 0, 0x40, 0x40, 0x40 };
static const uint16 g_word_A89ACC[17] = { 1, 8, 1, 8, 1, 7, 1, 7, 2, 6, 2, 6, 3, 5, 3, 5, 0xffff };
static const uint16 kWreckedShipGhost_Palette[16] = { 0x3800, 0x57ff, 0x42f7, 0x929, 0xa5, 0x4f5a, 0x36b5, 0x2610, 0x1dce, 0x1df, 0x1f, 0x18, 0xa, 0x6b9, 0xea, 0x45 };
static const uint16 g_word_A89D32 = 1;
static const uint16 g_word_A89D34 = 1;
static const int16 g_word_A8CCC1[31] = {
  0x1f, 0x18,  0xf,    8, 0x40,
  0x18,  0xf,    8, 0x1f, 0x10,
   0xf,    8, 0x1f, 0x18, 0x10,
     8, 0x1f, 0x18,  0xf, 0x40,
   0xf,    8, 0x1f, 0x18, 0x10,
  0x18,  0xf,    8, 0x1f, 0x10,
    -1,
};

void WreckedShipGhost_Init(void) {  // 0xA89AEE
  Enemy_WreckedShipGhost *E = Get_WreckedShipGhost(cur_enemy_index);
  E->base.properties |= kEnemyProps_ProcessInstructions | kEnemyProps_Intangible | kEnemyProps_Invisible;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->base.current_instruction = addr_kWreckedShipGhost_Ilist_9A8C;
  E->wsgt_var_A = FUNC16(WreckedShipGhost_Func_6);
  E->wsgt_var_B = g_word_A89AA4 + 160;

  uint16 v2 = swap16(E->base.palette_index);
  uint16 v3 = 16 * v2 + 256;
  int n = 16;
  do {
    target_palettes[v3 >> 1] = 0;
    v3 += 2;
  } while (--n >= 0);
}

void CallWreckedShipGhost(uint32 ea, uint16 k) {
  switch (ea) {
  case fnWreckedShipGhost_Func_1: WreckedShipGhost_Func_1(k); return;
  case fnWreckedShipGhost_Func_2: WreckedShipGhost_Func_2(k); return;
  case fnWreckedShipGhost_Func_4: WreckedShipGhost_Func_4(k); return;
  case fnWreckedShipGhost_Func_5: WreckedShipGhost_Func_5(k); return;
  case fnWreckedShipGhost_Func_6: WreckedShipGhost_Func_6(k); return;
  case fnWreckedShipGhost_Func_7: WreckedShipGhost_Func_7(k); return;
  default: Unreachable();
  }
}

void WreckedShipGhost_Main(void) {  // 0xA89B3C
  Enemy_WreckedShipGhost *E = Get_WreckedShipGhost(cur_enemy_index);
  CallWreckedShipGhost(E->wsgt_var_A | 0xA80000, cur_enemy_index);
}

void WreckedShipGhost_Func_1(uint16 k) {  // 0xA89B42
  WreckedShipGhost_Func_3(k);
  Enemy_WreckedShipGhost *E = Get_WreckedShipGhost(cur_enemy_index);
  uint16 v3 = swap16(E->base.palette_index);
  uint16 v4 = 16 * v3 + 256;
  int n = 16, v1 = 16;
  do {
    int v5 = v4 >> 1;
    if (sign16((palette_buffer[v5] & 0x1F) - 31)) {
      palette_buffer[v5] += 1057;
      --v1;
    }
    v4 += 2;
  } while (--n);
  if ((int16)(v1 - 16) >= 0) {
    E->wsgt_var_A = FUNC16(WreckedShipGhost_Func_2);
    uint16 v7 = swap16(E->base.palette_index);
    uint16 v8 = 16 * v7 + 256;
    uint16 v9 = 0;
    do {
      target_palettes[v8 >> 1] = kWreckedShipGhost_Palette[v9 >> 1];
      v8 += 2;
      v9 += 2;
    } while ((int16)(v9 - 32) < 0);
  }
}

void WreckedShipGhost_Func_2(uint16 k) {  // 0xA89BAD
  uint16 v4 = WreckedShipGhost_Func_8();
  WreckedShipGhost_Func_3(cur_enemy_index);
  Enemy_WreckedShipGhost *E = Get_WreckedShipGhost(cur_enemy_index);
  if (!(E->wsgt_var_B | v4)) {
    E->base.properties &= 0xFAFF;
    E->wsgt_var_A = FUNC16(WreckedShipGhost_Func_5);
    E->wsgt_var_00 = E->base.y_pos;
    E->wsgt_var_B = 1;
    E->wsgt_var_C = 2;
    E->wsgt_var_01 = 0;
    E->wsgt_var_02 = g_word_A89AA2;
    E->wsgt_var_B = g_word_A89AA6;
    E->wsgt_var_05 = 4;
    uint16 v2 = samus_x_pos;
    E->wsgt_var_06 = samus_x_pos;
    E->wsgt_var_07 = v2;
    E->wsgt_var_08 = v2;
    E->wsgt_var_09 = 12;
    uint16 v3 = *(uint16 *)((uint8 *)&samus_y_pos + cur_enemy_index);
    E->wsgt_var_0A = v3;
    E->wsgt_var_0B = v3;
    E->wsgt_var_0C = v3;
    E->wsgt_var_0D = g_word_A89A9E;
    E->wsgt_var_0E = g_word_A89A9C;
  }
}

void WreckedShipGhost_Func_3(uint16 k) {  // 0xA89C31
  int16 v5;

  Enemy_WreckedShipGhost *E = Get_WreckedShipGhost(k);
  uint16 wsgt_var_B = E->wsgt_var_B;
  if (!wsgt_var_B)
    goto LABEL_6;
  uint16 v3;
  v3 = wsgt_var_B - 1;
  E->wsgt_var_B = v3;
  if (!v3) {
    uint16 wsgt_var_C;
    wsgt_var_C = E->wsgt_var_C;
    v5 = g_word_A89ACC[wsgt_var_C >> 1];
    if (v5 < 0) {
      E->wsgt_var_B = 0;
      E->wsgt_var_C = 0;
      return;
    }
    E->wsgt_var_B = v5;
    E->wsgt_var_C = wsgt_var_C + 2;
    if ((wsgt_var_C & 2) == 0) {
LABEL_6:
      E->base.properties &= ~kEnemyProps_Invisible;
    }
  }
}

void WreckedShipGhost_Func_4(uint16 k) {  // 0xA89C69
  if (!WreckedShipGhost_Func_8()) {
    Enemy_WreckedShipGhost *E = Get_WreckedShipGhost(cur_enemy_index);
    E->wsgt_var_A = FUNC16(WreckedShipGhost_Func_6);
    E->base.properties |= kEnemyProps_Invisible;
    E->wsgt_var_B = g_word_A89AA4;
  }
}

void WreckedShipGhost_Func_5(uint16 k) {  // 0xA89C8A
  Enemy_WreckedShipGhost *E = Get_WreckedShipGhost(k);
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(E->wsgt_var_02, E->wsgt_var_01));
  AddToHiLo(&E->wsgt_var_02, &E->wsgt_var_01, ((int16)(E->base.y_pos - E->wsgt_var_00) < 0) ? g_word_A89AA0 : -g_word_A89AA0);
  if (--E->wsgt_var_B == 0) {
    E->wsgt_var_A = FUNC16(WreckedShipGhost_Func_4);
    E->base.properties |= kEnemyProps_Intangible;
    // todo: this must be swap16 it seems
    int t = swap16(E->base.palette_index);
    uint16 v9 = 16 * t + 256;
    int n = 16;
    do {
      target_palettes[v9 >> 1] = 0x7FFF;
      v9 += 2;
    } while ((--n & 0x8000) == 0);
  }
}

void WreckedShipGhost_Func_6(uint16 k) {  // 0xA89D13
  uint16 v3;

  Enemy_WreckedShipGhost *E = Get_WreckedShipGhost(k);
  uint16 wsgt_var_B = E->wsgt_var_B;
  if (!wsgt_var_B || (v3 = wsgt_var_B - 1, (E->wsgt_var_B = v3) == 0)) {
    E->wsgt_var_B = 1;
    E->wsgt_var_C = 2;
    E->wsgt_var_A = FUNC16(WreckedShipGhost_Func_7);
  }
}

void WreckedShipGhost_Func_7(uint16 k) {  // 0xA89D36
  int16 v2;
  int16 v4;
  int16 v7;
  int16 v9;

  Enemy_WreckedShipGhost *E = Get_WreckedShipGhost(k);
  if ((int16)(samus_x_pos - E->wsgt_var_07) < 0
      || (int16)(samus_x_pos - E->wsgt_var_08) >= 0
      || (v2 = *(uint16 *)((uint8 *)&samus_y_pos + k), (int16)(v2 - E->wsgt_var_0B) < 0)
      || (int16)(v2 - E->wsgt_var_0C) >= 0) {
    E->wsgt_var_0D = g_word_A89A9E;
    v4 = 0;
    if ((int16)(samus_x_pos - E->wsgt_var_06) >= 0) {
      v4 = (samus_x_pos == E->wsgt_var_06) ? 4 : 8;
    }
    if (v4 == E->wsgt_var_05) {
      v7 = 0;
      v9 = samus_y_pos - E->wsgt_var_0A;
      if (v9 >= 0) {
        v7 = v9 ? 24 : 12;
      }
      if (v7 == E->wsgt_var_09) {
        uint16 v10 = E->wsgt_var_0E - 1;
        E->wsgt_var_0E = v10;
        if (!v10)
          goto LABEL_13;
      } else {
        E->wsgt_var_09 = v7;
        E->wsgt_var_0E = g_word_A89A9C;
      }
    } else {
      E->wsgt_var_05 = v4;
      E->wsgt_var_0E = g_word_A89A9C;
    }
  } else {
    uint16 v3;
    v3 = E->wsgt_var_0D - 1;
    E->wsgt_var_0D = v3;
    if (!v3) {
      E->wsgt_var_05 = 4;
      E->wsgt_var_09 = 12;
LABEL_13:
      E->wsgt_var_A = FUNC16(WreckedShipGhost_Func_1);
      E->wsgt_var_0D = g_word_A89A9E;
      E->wsgt_var_0E = g_word_A89A9C;
      int v6 = (uint16)(E->wsgt_var_09 + E->wsgt_var_05) >> 1;
      E->base.x_pos = g_word_A89AA8[v6] + *(uint16 *)((uint8 *)&samus_x_pos + k);
      E->base.y_pos = g_word_A89AA8[v6 + 1] + *(uint16 *)((uint8 *)&samus_y_pos + k);
      return;
    }
  }
  uint16 v11 = samus_x_pos;
  E->wsgt_var_06 = samus_x_pos;
  E->wsgt_var_07 = v11 - g_word_A89D32;
  E->wsgt_var_08 = g_word_A89D32 + samus_x_pos;
  uint16 v13 = samus_y_pos;
  E->wsgt_var_0A = samus_y_pos;
  E->wsgt_var_0B = v13 - g_word_A89D34;
  E->wsgt_var_0C = g_word_A89D34 + samus_y_pos;
}

uint16 WreckedShipGhost_Func_8(void) {  // 0xA89E88
  uint16 v0 = 0;
  if (!door_transition_flag_enemies) {
    Enemy_WreckedShipGhost *E = Get_WreckedShipGhost(cur_enemy_index);
    uint16 v2 = swap16(E->base.palette_index);
    uint16 v3 = 16 * v2 + 256;
    uint16 r20 = 16 * v2 + 288;
    do {
      int v4 = v3 >> 1;
      if (target_palettes[v4] != palette_buffer[v4]) {
        uint16 r18 = target_palettes[v4] & 0x1F;
        uint16 v5 = palette_buffer[v4] & 0x1F;
        if (v5 != r18) {
          uint16 v6;
          if ((int16)(v5 - r18) >= 0)
            v6 = v5 - 1;
          else
            v6 = v5 + 1;
          r18 = v6;
          palette_buffer[v4] = v6 | palette_buffer[v4] & 0xFFE0;
          ++v0;
        }
        r18 = target_palettes[v4] & 0x3E0;
        uint16 v7 = palette_buffer[v4] & 0x3E0;
        if (v7 != r18) {
          uint16 v8;
          if ((int16)(v7 - r18) >= 0)
            v8 = v7 - 32;
          else
            v8 = v7 + 32;
          palette_buffer[v4] = v8 | palette_buffer[v4] & 0xFC1F;
          ++v0;
        }
        r18 = target_palettes[v4] & 0x7C00;
        uint16 v9 = palette_buffer[v4] & 0x7C00;
        if (v9 != r18) {
          uint16 v10;
          if ((int16)(v9 - r18) >= 0)
            v10 = v9 - 1024;
          else
            v10 = v9 + 1024;
          r18 = v10;
          palette_buffer[v4] = v10 | palette_buffer[v4] & 0x83FF;
          ++v0;
        }
      }
      v3 += 2;
    } while ((int16)(v3 - r20) < 0);
  }
  return v0;
}

void WreckedShipRobot_Init(void) {  // 0xA8CB77
  if ((*(uint16 *)&boss_bits_for_area[area_index] & 1) != 0) {
    enemy_gfx_drawn_hook.bank = -88;
    enemy_gfx_drawn_hook.addr = FUNC16(WreckedShipRobot_Func_1);
    Enemy_WreckedShipRobot *E = Get_WreckedShipRobot(cur_enemy_index);
    E->base.properties |= 0xA000;
    E->base.instruction_timer = 4;
    E->base.timer = 0;
    E->base.current_instruction = addr_kWreckedShipRobot_Ilist_C6E5;
    E->wsrt_var_A = -512;
    wrecked_ship_robot_palanim_timer = 1;
    *(uint16 *)&wrecked_ship_robot_palanim_table_index = 0;
    E->wsrt_var_B = 0;
    wrecked_ship_robot_palanim_palindex = E->base.palette_index;
  } else {
    WreckedShipRobotDeactivated_Init();
  }
}

void WreckedShipRobotDeactivated_Init(void) {  // 0xA8CBCC
  EnemyData *v0 = gEnemyData(cur_enemy_index);
  uint16 parameter_1 = v0->parameter_1;
  if (sign16(parameter_1) || !sign16(parameter_1 - 4))
    parameter_1 = 0;
  v0->parameter_1 = parameter_1;
  v0->current_instruction = g_off_A8CC30[parameter_1];
  v0->properties |= kEnemyProps_SolidToSamus;
  v0->instruction_timer = 1;
  v0->timer = 0;
  wrecked_ship_robot_palanim_timer = 0;
  v0->ai_var_E = 0;
  v0->ai_preinstr = 1;
  int v2 = (16 * ((wrecked_ship_robot_palanim_palindex & 0xFF00) >> 8)) >> 1;
  palette_buffer[v2 + 137] = 10;
  palette_buffer[v2 + 138] = 10;
  palette_buffer[v2 + 139] = 10;
  palette_buffer[v2 + 140] = 10;
}

void WreckedShipRobot_Main(void) {  // 0xA8CC36
  Enemy_WreckedShipRobot *E = Get_WreckedShipRobot(cur_enemy_index);
  if (!Enemy_MoveDown(cur_enemy_index, __PAIR32__(E->wsrt_var_F, E->wsrt_var_E))) {
    ++E->base.instruction_timer;
    AddToHiLo(&E->wsrt_var_F, &E->wsrt_var_E, 0x8000);
  }
}

void WreckedShipRobot_Func_1(void) {  // 0xA8CC67
  uint16 i;
  int16 v2;

  if (!door_transition_flag_enemies) {
    if (wrecked_ship_robot_palanim_timer) {
      if (!--wrecked_ship_robot_palanim_timer) {
        int v1;
        for (i = *(uint16 *)&wrecked_ship_robot_palanim_table_index; ; i = 0) {
          v1 = i >> 1;
          v2 = g_word_A8CCC1[v1];
          if (v2 >= 0)
            break;
        }
        int v3 = (16 * ((wrecked_ship_robot_palanim_palindex & 0xFF00) >> 8)) >> 1;
        palette_buffer[v3 + 137] = v2;
        palette_buffer[v3 + 138] = g_word_A8CCC1[v1 + 1];
        palette_buffer[v3 + 139] = g_word_A8CCC1[v1 + 2];
        palette_buffer[v3 + 140] = g_word_A8CCC1[v1 + 3];
        wrecked_ship_robot_palanim_timer = g_word_A8CCC1[v1 + 4];
        *(uint16 *)&wrecked_ship_robot_palanim_table_index = i + 10;
      }
    }
  }
}

void WreckedShipRobot_Func_2(uint16 k) {  // 0xA8CCFF
  Enemy_WreckedShipRobot *E = Get_WreckedShipRobot(k);
  uint16 wsrt_var_B = E->wsrt_var_B;
  if (wsrt_var_B)
    E->wsrt_var_B = wsrt_var_B - 1;
}

const uint16 *WreckedShipRobot_Instr_4(uint16 k, const uint16 *jp) {  // 0xA8CD09
  WreckedShipRobot_Func_2(cur_enemy_index);
  Enemy_WreckedShipRobot *E = Get_WreckedShipRobot(cur_enemy_index);
  E->wsrt_var_A = -512;
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, INT16_SHL16(-4))) {
    E->wsrt_var_B += 8;
    return INSTR_RETURN_ADDR(addr_kWreckedShipRobot_Ilist_C73F);
  } else {
    if (CheckIfEnemyTouchesSamus(cur_enemy_index)) {
      extra_samus_x_subdisplacement = 0;
      extra_samus_x_displacement = -4;
    }
    E->wsrt_var_D = E->base.y_pos;
    uint16 x_pos = E->base.x_pos;
    E->wsrt_var_C = x_pos;
    E->base.x_pos = x_pos - E->base.x_width - E->base.x_width;
    if (Enemy_MoveDown(cur_enemy_index, INT16_SHL16(1))) {
      E->base.x_pos = E->wsrt_var_C;
      E->base.y_pos = E->wsrt_var_D;
    } else {
      E->wsrt_var_B += 8;
      E->base.x_pos = E->wsrt_var_C;
      E->base.y_pos = E->wsrt_var_D;
      E->wsrt_var_A = 512;
      return INSTR_RETURN_ADDR(addr_kWreckedShipRobot_Ilist_CB65);
    }
  }
  return jp;
}

const uint16 *WreckedShipRobot_Instr_9(uint16 k, const uint16 *jp) {  // 0xA8CDA4
  WreckedShipRobot_Func_2(cur_enemy_index);
  Enemy_WreckedShipRobot *E = Get_WreckedShipRobot(cur_enemy_index);
  E->wsrt_var_A = -512;
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, INT16_SHL16(-4))) {
    E->wsrt_var_B += 8;
    return INSTR_RETURN_ADDR(addr_kWreckedShipRobot_Ilist_C73F);
  } else if (CheckIfEnemyTouchesSamus(cur_enemy_index)) {
    extra_samus_x_subdisplacement = 0;
    extra_samus_x_displacement = -4;
  }
  return jp;
}

const uint16 *WreckedShipRobot_Instr_6(uint16 k, const uint16 *jp) {  // 0xA8CDEA
  WreckedShipRobot_Func_2(cur_enemy_index);
  Enemy_WreckedShipRobot *E = Get_WreckedShipRobot(cur_enemy_index);
  E->wsrt_var_A = -512;
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, INT16_SHL16(4))) {
    E->wsrt_var_B += 8;
    return INSTR_RETURN_ADDR(addr_stru_A8C6E9);
  } else {
    if (CheckIfEnemyTouchesSamus(cur_enemy_index)) {
      extra_samus_x_subdisplacement = 0;
      extra_samus_x_displacement = 4;
    }
    E->wsrt_var_D = E->base.y_pos;
    uint16 x_pos = E->base.x_pos;
    E->wsrt_var_C = x_pos;
    E->base.x_pos = E->base.x_width + E->base.x_width + x_pos;
    if (Enemy_MoveDown(cur_enemy_index, INT16_SHL16(1))) {
      E->base.x_pos = E->wsrt_var_C;
      E->base.y_pos = E->wsrt_var_D;
    } else {
      E->wsrt_var_B += 8;
      E->base.x_pos = E->wsrt_var_C;
      E->base.y_pos = E->wsrt_var_D;
      E->wsrt_var_A = -512;
      return INSTR_RETURN_ADDR(addr_kWreckedShipRobot_Ilist_C91B);
    }
  }
  return jp;
}

const uint16 *WreckedShipRobot_Instr_8(uint16 k, const uint16 *jp) {  // 0xA8CE85
  WreckedShipRobot_Func_2(cur_enemy_index);
  Enemy_WreckedShipRobot *E = Get_WreckedShipRobot(cur_enemy_index);
  E->wsrt_var_A = -512;
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, INT16_SHL16(4))) {
    E->wsrt_var_B += 8;
    return INSTR_RETURN_ADDR(addr_stru_A8C6E9);
  } else if (CheckIfEnemyTouchesSamus(cur_enemy_index)) {
    extra_samus_x_subdisplacement = 0;
    extra_samus_x_displacement = 4;
  }
  return jp;
}

const uint16 *WreckedShipRobot_Instr_7(uint16 k, const uint16 *jp) {  // 0xA8CECB
  return INSTR_RETURN_ADDR(addr_kWreckedShipRobot_Ilist_C92D);
}

const uint16 *WreckedShipRobot_Instr_15(uint16 k, const uint16 *jp) {  // 0xA8CECF
  WreckedShipRobot_Func_2(cur_enemy_index);
  Enemy_WreckedShipRobot *E = Get_WreckedShipRobot(cur_enemy_index);
  E->wsrt_var_A = 512;
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, INT16_SHL16(4))) {
    E->wsrt_var_B += 8;
    return INSTR_RETURN_ADDR(addr_kWreckedShipRobot_Ilist_C985);
  } else {
    if (CheckIfEnemyTouchesSamus(cur_enemy_index)) {
      extra_samus_x_subdisplacement = 0;
      extra_samus_x_displacement = 4;
    }
    E->wsrt_var_D = E->base.y_pos;
    uint16 x_pos = E->base.x_pos;
    E->wsrt_var_C = x_pos;
    E->base.x_pos = E->base.x_width + E->base.x_width + x_pos;
    if (Enemy_MoveDown(cur_enemy_index, INT16_SHL16(1))) {
      E->base.x_pos = E->wsrt_var_C;
      E->base.y_pos = E->wsrt_var_D;
    } else {
      E->wsrt_var_B += 8;
      E->base.x_pos = E->wsrt_var_C;
      E->base.y_pos = E->wsrt_var_D;
      E->wsrt_var_A = -512;
      return INSTR_RETURN_ADDR(addr_kWreckedShipRobot_Ilist_C91B);
    }
  }
  return jp;
}

const uint16 *WreckedShipRobot_Instr_18(uint16 k, const uint16 *jp) {  // 0xA8CF6A
  WreckedShipRobot_Func_2(cur_enemy_index);
  Enemy_WreckedShipRobot *E = Get_WreckedShipRobot(cur_enemy_index);
  E->wsrt_var_A = 512;
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, INT16_SHL16(4))) {
    E->wsrt_var_B += 8;
    return INSTR_RETURN_ADDR(addr_kWreckedShipRobot_Ilist_C985);
  } else if (CheckIfEnemyTouchesSamus(cur_enemy_index)) {
    extra_samus_x_subdisplacement = 0;
    extra_samus_x_displacement = 4;
  }
  return jp;
}

const uint16 *WreckedShipRobot_Instr_16(uint16 k, const uint16 *jp) {  // 0xA8CFB0
  WreckedShipRobot_Func_2(cur_enemy_index);
  Enemy_WreckedShipRobot *E = Get_WreckedShipRobot(cur_enemy_index);
  E->wsrt_var_A = 512;
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, INT16_SHL16(-4))) {
    E->wsrt_var_B += 8;
    return INSTR_RETURN_ADDR(addr_stru_A8C6E9);
  } else {
    if (CheckIfEnemyTouchesSamus(cur_enemy_index)) {
      extra_samus_x_subdisplacement = 0;
      extra_samus_x_displacement = -4;
    }
    E->wsrt_var_D = E->base.y_pos;
    uint16 x_pos = E->base.x_pos;
    E->wsrt_var_C = x_pos;
    E->base.x_pos = x_pos - E->base.x_width - E->base.x_width;
    if (Enemy_MoveDown(cur_enemy_index, INT16_SHL16(1))) {
      E->base.y_pos = E->wsrt_var_D;
      E->base.x_pos = E->wsrt_var_C;
    } else {
      E->wsrt_var_B += 8;
      E->base.y_pos = E->wsrt_var_D;
      E->base.x_pos = E->wsrt_var_C;
      E->wsrt_var_A = 512;
      return INSTR_RETURN_ADDR(addr_kWreckedShipRobot_Ilist_CB65);
    }
  }
  return jp;
}

const uint16 *WreckedShipRobot_Instr_17(uint16 k, const uint16 *jp) {  // 0xA8D04B
  WreckedShipRobot_Func_2(cur_enemy_index);
  Enemy_WreckedShipRobot *E = Get_WreckedShipRobot(cur_enemy_index);
  E->wsrt_var_A = 512;
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, INT16_SHL16(-4))) {
    E->wsrt_var_B += 8;
    return INSTR_RETURN_ADDR(addr_stru_A8C6E9);
  } else if (CheckIfEnemyTouchesSamus(cur_enemy_index)) {
    extra_samus_x_subdisplacement = 0;
    extra_samus_x_displacement = -4;
  }
  return jp;
}

const uint16 *WreckedShipRobot_Instr_3(uint16 k, const uint16 *jp) {  // 0xA8D091
  Enemy_WreckedShipRobot *E = Get_WreckedShipRobot(cur_enemy_index);
  if ((int16)(layer1_x_pos - E->base.x_pos) < 0
      && (int16)(layer1_x_pos + 256 - E->base.x_pos) >= 0
      && (int16)(layer1_y_pos - E->base.y_pos) < 0
      && (int16)(layer1_y_pos + 224 - E->base.y_pos) >= 0) {
    QueueSfx2_Max6(0x68);
  }
  return jp;
}

const uint16 *WreckedShipRobot_Instr_10(uint16 k, const uint16 *jp) {  // 0xA8D0C2
  return INSTR_RETURN_ADDR(addr_stru_A8C6E9);
}

const uint16 *WreckedShipRobot_Instr_14(uint16 k, const uint16 *jp) {  // 0xA8D0C6
  return WreckedShipRobot_CommonInstr(k, jp,
      addr_kWreckedShipRobot_Ilist_CB1D, addr_kEproj_WreckedShipRobotLaserUpRight);
}

const uint16 *WreckedShipRobot_Instr_2(uint16 k, const uint16 *jp) {  // 0xA8D0D2
  return WreckedShipRobot_CommonInstr(k, jp,
      addr_kWreckedShipRobot_Ilist_C8D1, addr_kEproj_WreckedShipRobotLaserUpLeft);
}

const uint16 *WreckedShipRobot_CommonInstr(uint16 k, const uint16 *jp, uint16 r50, uint16 r48) {  // 0xA8D0DC
  Enemy_WreckedShipRobot *E = Get_WreckedShipRobot(k);
  if (E->wsrt_var_B) {
    WreckedShipRobot_Func_2(k);
  } else {
    uint16 v3 = (random_number & 0x1F) + 16;
    E->wsrt_var_B = v3;
    SpawnEprojWithGfx(v3, cur_enemy_index, r48);
    return INSTR_RETURN_ADDR(r50);
  }
  return jp;
}

const uint16 *WreckedShipRobot_Instr_13(uint16 k, const uint16 *jp) {  // 0xA8D100
  return WreckedShipRobot_D10C(k, jp, addr_kWreckedShipRobot_Ilist_CB09);
}

const uint16 *WreckedShipRobot_Instr_1(uint16 k, const uint16 *jp) {  // 0xA8D107
  return WreckedShipRobot_D10C(k, jp, addr_kWreckedShipRobot_Ilist_C8BD);
}

const uint16 *WreckedShipRobot_D10C(uint16 k, const uint16 *jp, uint16 r50) {  // 0xA8D10C
  Enemy_WreckedShipRobot *E = Get_WreckedShipRobot(k);
  if (E->wsrt_var_B) {
    WreckedShipRobot_Func_2(k);
  } else {
    uint16 v3 = (random_number & 0x1F) + 16;
    E->wsrt_var_B = v3;
    SpawnEprojWithGfx(v3, cur_enemy_index, addr_kEproj_WreckedShipRobotLaserHorizontal);
    return INSTR_RETURN_ADDR(r50);
  }
  return jp;
}

const uint16 *WreckedShipRobot_Instr_12(uint16 k, const uint16 *jp) {  // 0xA8D131
  return WreckedShipRobot_D147(k, jp,
      addr_kEproj_WreckedShipRobotLaserDownRight, addr_kWreckedShipRobot_Ilist_CAFD);
}

const uint16 *WreckedShipRobot_Instr_5(uint16 k, const uint16 *jp) {  // 0xA8D13D
  return WreckedShipRobot_D147(k, jp,
      addr_kEproj_WreckedShipRobotLaserDownLeft, addr_kWreckedShipRobot_Ilist_C8B1);
}

const uint16 *WreckedShipRobot_D147(uint16 k, const uint16 *jp, uint16 r48, uint16 r50) {  // 0xA8D147
  Enemy_WreckedShipRobot *E = Get_WreckedShipRobot(k);
  if (E->wsrt_var_B) {
    WreckedShipRobot_Func_2(k);
  } else {
    uint16 v3 = (random_number & 0x1F) + 16;
    E->wsrt_var_B = v3;
    SpawnEprojWithGfx(v3, cur_enemy_index, r48);
    return INSTR_RETURN_ADDR(r50);
  }
  return jp;
}

const uint16 *WreckedShipRobot_Instr_11(uint16 k, const uint16 *jp) {  // 0xA8D16B
  WreckedShipRobot_Func_2(cur_enemy_index);
  return jp;
}

void WreckedShipRobotDeactivated_Touch(void) {  // 0xA8D174
  if ((int16)(gEnemyData(cur_enemy_index)->x_pos - samus_x_pos) < 0)
    extra_samus_x_displacement = 4;
  else
    extra_samus_x_displacement = -4;
}

void WreckedShipRobotDeactivated_Shot(void) {  // 0xA8D18D
  NormalEnemyShotAi();
}

void WreckedShipRobot_Shot(void) {  // 0xA8D192
  if ((*(uint16 *)&boss_bits_for_area[area_index] & 1) != 0) {
    NormalEnemyShotAi();
    Enemy_WreckedShipRobot *E = Get_WreckedShipRobot(cur_enemy_index);
    if (E->base.health) {
      uint16 v1;
      if ((E->wsrt_var_A & 0x8000) != 0) {
        if ((int16)(samus_x_pos - E->base.x_pos) >= 0)
          v1 = addr_kWreckedShipRobot_Ilist_C833;
        else
          v1 = addr_kWreckedShipRobot_Ilist_C7BB;
      } else if ((int16)(samus_x_pos - E->base.x_pos) < 0) {
        v1 = addr_kWreckedShipRobot_Ilist_CA7D;
      } else {
        v1 = addr_kWreckedShipRobot_Ilist_CA01;
      }
      E->base.current_instruction = v1;
      E->base.instruction_timer = 1;
      E->wsrt_var_B += 64;
    }
  }
}

void WreckedShipOrbs_Init(void) {  // 0xA8E388
  Enemy_WreckedShipOrbs *E = Get_WreckedShipOrbs(cur_enemy_index);
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->base.current_instruction = g_off_A8E380[E->wsos_parameter_1];
  int v1 = (8 * E->wsos_parameter_2) >> 1;
  E->wsos_var_01 = kCommonEnemySpeeds_Linear[v1];
  E->wsos_var_00 = kCommonEnemySpeeds_Linear[v1 + 1];
  E->wsos_var_03 = kCommonEnemySpeeds_Linear[v1 + 2];
  E->wsos_var_02 = kCommonEnemySpeeds_Linear[v1 + 3];
}

void CallWreckedShipOrbsA(uint32 ea) {
  switch (ea) {
  case fnWreckedShipOrbs_Func_3: WreckedShipOrbs_Func_3(); return;
  case fnWreckedShipOrbs_Func_4: WreckedShipOrbs_Func_4(); return;
  default: Unreachable();
  }
}
void CallWreckedShipOrbsB(uint32 ea) {
  switch (ea) {
  case fnWreckedShipOrbs_Func_5: WreckedShipOrbs_Func_5(); return;
  case fnWreckedShipOrbs_Func_6: WreckedShipOrbs_Func_6(); return;
  default: Unreachable();
  }
}
void WreckedShipOrbs_Main(void) {  // 0xA8E3C3
  WreckedShipOrbs_Func_1();
  WreckedShipOrbs_Func_2();
  Enemy_WreckedShipOrbs *E = Get_WreckedShipOrbs(cur_enemy_index);
  CallWreckedShipOrbsA(E->wsos_var_A | 0xA80000);
  CallWreckedShipOrbsB(E->wsos_var_B | 0xA80000);
}

void WreckedShipOrbs_Func_1(void) {  // 0xA8E3D9
  Enemy_WreckedShipOrbs *E = Get_WreckedShipOrbs(cur_enemy_index);
  E->wsos_var_A = FUNC16(WreckedShipOrbs_Func_3);
  if ((GetSamusEnemyDelta_Y(cur_enemy_index) & 0x8000) == 0)
    E->wsos_var_A = FUNC16(WreckedShipOrbs_Func_4);
}

void WreckedShipOrbs_Func_2(void) {  // 0xA8E3EF
  Enemy_WreckedShipOrbs *E = Get_WreckedShipOrbs(cur_enemy_index);
  E->wsos_var_B = FUNC16(WreckedShipOrbs_Func_5);
  if ((GetSamusEnemyDelta_X(cur_enemy_index) & 0x8000) == 0)
    E->wsos_var_B = FUNC16(WreckedShipOrbs_Func_6);
}

void WreckedShipOrbs_Func_3(void) {  // 0xA8E405
  Enemy_WreckedShipOrbs *E = Get_WreckedShipOrbs(cur_enemy_index);
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(E->wsos_var_03, E->wsos_var_02));
}

void WreckedShipOrbs_Func_4(void) {  // 0xA8E424
  Enemy_WreckedShipOrbs *E = Get_WreckedShipOrbs(cur_enemy_index);
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(E->wsos_var_01, E->wsos_var_00));
}

void WreckedShipOrbs_Func_5(void) {  // 0xA8E443
  Enemy_WreckedShipOrbs *E = Get_WreckedShipOrbs(cur_enemy_index);
  AddToHiLo(&E->base.x_pos, &E->base.x_subpos, __PAIR32__(E->wsos_var_03, E->wsos_var_02));
}

void WreckedShipOrbs_Func_6(void) {  // 0xA8E462
  Enemy_WreckedShipOrbs *E = Get_WreckedShipOrbs(cur_enemy_index);
  AddToHiLo(&E->base.x_pos, &E->base.x_subpos, __PAIR32__(E->wsos_var_01, E->wsos_var_00));
}

const uint16 *WreckedShipSpark_Instr_2(uint16 k, const uint16 *jp) {  // 0xA8E61D
  Enemy_WreckedShipSpark *E = Get_WreckedShipSpark(cur_enemy_index);
  E->base.properties |= kEnemyProps_Intangible;
  return jp;
}

const uint16 *WreckedShipSpark_Instr_1(uint16 k, const uint16 *jp) {  // 0xA8E62A
  Enemy_WreckedShipSpark *E = Get_WreckedShipSpark(cur_enemy_index);
  E->base.properties &= ~kEnemyProps_Intangible;
  return jp;
}


static void WreckedShipSpark_Func_4(uint16 k, uint16 r18) {  // 0xA8E6F6
  int16 wssk_var_E;

  Enemy_WreckedShipSpark *E = Get_WreckedShipSpark(k);
  wssk_var_E = E->wssk_var_E;
  if (wssk_var_E < 0)
    wssk_var_E = (NextRandom() & 0x3F) + 4;
  E->wssk_var_F = r18 + wssk_var_E;
}

void WreckedShipSpark_Init(void) {  // 0xA8E637
  Enemy_WreckedShipSpark *E = Get_WreckedShipSpark(cur_enemy_index);
  int v2 = E->wssk_parameter_1 & 3;
  E->wssk_var_B = g_off_A8E688[v2];
  E->wssk_var_E = E->wssk_parameter_2;
  WreckedShipSpark_Func_4(cur_enemy_index, 0);
  E->base.instruction_timer = 1;
  E->base.current_instruction = g_off_A8E682[v2];
  E->base.timer = 0;
  E->base.instruction_timer = 1;
  if ((boss_bits_for_area[area_index] & 1) == 0) {
    E->base.properties |= *(uint16 *)((uint8 *)&gVramWriteEntry(0)[6].vram_dst + 1);
  }
}

void CallWreckedShipSpark(uint32 ea, uint16 k) {
  switch (ea) {
  case fnWreckedShipSpark_Func_1: WreckedShipSpark_Func_1(k); return;
  case fnWreckedShipSpark_Func_2: WreckedShipSpark_Func_2(k); return;
  case fnWreckedShipSpark_Func_3: WreckedShipSpark_Func_3(k); return;
  case fnnullsub_259: return;
  default: Unreachable();
  }
}

void WreckedShipSpark_Main(void) {  // 0xA8E68E
  Enemy_WreckedShipSpark *E = Get_WreckedShipSpark(cur_enemy_index);
  CallWreckedShipSpark(E->wssk_var_B | 0xA80000, cur_enemy_index);
}

void WreckedShipSpark_Func_1(uint16 k) {  // 0xA8E695
  Enemy_WreckedShipSpark *E = Get_WreckedShipSpark(k);
  if (E->wssk_var_F == 1) {
    E->wssk_var_B = FUNC16(WreckedShipSpark_Func_2);
    E->base.current_instruction = addr_kWreckedShipSpark_Ilist_E5A7;
    E->base.instruction_timer = 1;
    WreckedShipSpark_Func_4(k, 0);
  } else {
    --E->wssk_var_F;
  }
}

void WreckedShipSpark_Func_2(uint16 k) {  // 0xA8E6B7
  Enemy_WreckedShipSpark *E = Get_WreckedShipSpark(k);
  if (E->wssk_var_F == 1) {
    E->wssk_var_B = FUNC16(WreckedShipSpark_Func_1);
    E->base.current_instruction = addr_kWreckedShipSpark_Ilist_E5E5;
    E->base.instruction_timer = 1;
    WreckedShipSpark_Func_4(k, 8);
  } else {
    --E->wssk_var_F;
  }
}

void WreckedShipSpark_Func_3(uint16 k) {  // 0xA8E6DC
  Enemy_WreckedShipSpark *E = Get_WreckedShipSpark(k);
  if (E->wssk_var_F == 1) {
    uint16 v2 = cur_enemy_index;
    SpawnEprojWithGfx(0, cur_enemy_index, addr_kEproj_Sparks);
    WreckedShipSpark_Func_4(v2, 0);
  } else {
    --E->wssk_var_F;
  }
}


void WreckedShipSpark_Shot(void) {  // 0xA8E70E
  projectile_dir[collision_detection_index] &= ~0x10;
}

