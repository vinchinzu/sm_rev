// Enemy AI - FireZoomer, WreckedShipOrangeZoomer, StoneZoomer — peeled from Bank $A3
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

#define g_off_A3E03B ((uint16*)RomFixedPtr(0xa3e03b))
#define g_word_A3E5F0 ((uint16*)RomFixedPtr(0xa3e5f0))
#define g_off_A3E2CC ((uint16*)RomFixedPtr(0xa3e2cc))
#define g_off_A3E630 ((uint16*)RomFixedPtr(0xa3e630))
#define g_off_A3E63C ((uint16*)RomFixedPtr(0xa3e63c))
#define g_off_A3E648 ((uint16*)RomFixedPtr(0xa3e648))
#define g_off_A3E654 ((uint16*)RomFixedPtr(0xa3e654))
#define g_word_A3E931 ((uint16*)RomFixedPtr(0xa3e931))

static uint32 Shift8AddMagn(int16 a, int s) {
  int32 r = (a << 8);
  r += (r < 0) ? -(s << 16) : (s << 16);
  return r;
}

const uint16 *WreckedShipOrangeZoomer_Func_1(uint16 k, const uint16 *jp) {  // 0xA3DFC2
  Get_WreckedShipOrangeZoomer(k)->wsozr_var_F = jp[0];
  return jp + 1;
}

void WreckedShipOrangeZoomer_Init(void) {  // 0xA3E043
  Enemy_WreckedShipOrangeZoomer *E = Get_WreckedShipOrangeZoomer(cur_enemy_index);
  E->base.current_instruction = g_off_A3E03B[E->base.current_instruction & 3];
  E->wsozr_var_F = addr_locret_A3E08A;
  uint16 v1 = g_word_A3E5F0[E->wsozr_parameter_1];
  E->wsozr_var_A = v1;
  E->wsozr_var_B = v1;
  if ((E->base.properties & 3) != 0) {
    if ((E->base.properties & 3) == 2)
      E->wsozr_var_B = -E->wsozr_var_B;
  } else {
    E->wsozr_var_A = -E->wsozr_var_A;
  }
}

void WreckedShipOrangeZoomer_Main(void) {  // 0xA3E08B
  Enemy_WreckedShipOrangeZoomer *E = Get_WreckedShipOrangeZoomer(cur_enemy_index);
  EnemyRunPreInstr(E->wsozr_var_F);
}

void WreckedShipOrangeZoomer_Func_2(uint16 k) {  // 0xA3E091
  int16 v7;
  uint16 v9;
  uint16 v12;
  Enemy_WreckedShipOrangeZoomer *E = Get_WreckedShipOrangeZoomer(k);

  if (earthquake_timer == 30 && earthquake_type == 20) {
    E->wsozr_var_03 = E->wsozr_var_F;
    E->wsozr_var_F = FUNC16(FireZoomer_Func_2);
  }
  if (Enemy_MoveRight_ProcessSlopes(k, Shift8AddMagn(E->wsozr_var_A, 1))) {
    E->wsozr_var_04 = 0;
    EnemyFunc_C8AD(k);
    if (!(Enemy_MoveDown(k, INT16_SHL8(E->wsozr_var_B)))) {
      if ((int16)(samus_y_pos - E->base.y_pos) >= 0) {
        v7 = E->wsozr_var_B;
        if (v7 < 0)
          LABEL_17:
        v7 = -v7;
      } else {
        v7 = E->wsozr_var_B;
        if (v7 >= 0)
          goto LABEL_17;
      }
      E->wsozr_var_B = v7;
      return;
    }
    E->wsozr_var_A = -E->wsozr_var_A;
    v9 = addr_kWreckedShipOrangeZoomer_Ilist_E01F;
    if ((E->wsozr_var_B & 0x8000) != 0)
      v9 = addr_kWreckedShipOrangeZoomer_Ilist_E003;
    E->base.current_instruction = v9;
    E->base.instruction_timer = 1;
  } else {
    uint16 v11 = E->wsozr_var_04 + 1;
    E->wsozr_var_04 = v11;
    if (sign16(v11 - 4)) {
      E->wsozr_var_B = -E->wsozr_var_B;
      v12 = addr_kWreckedShipOrangeZoomer_Ilist_E01F;
      if ((E->wsozr_var_B & 0x8000) != 0)
        v12 = addr_kWreckedShipOrangeZoomer_Ilist_E003;
      E->base.current_instruction = v12;
      E->base.instruction_timer = 1;
    } else {
      E->wsozr_var_03 = E->wsozr_var_F;
      E->wsozr_var_F = FUNC16(FireZoomer_Func_2);
    }
  }
}

void sub_A3E168(uint16 k) {  // 0xA3E168
  int16 v7;
  uint16 v9;
  uint16 v12;
  Enemy_WreckedShipOrangeZoomer *E = Get_WreckedShipOrangeZoomer(k);

  if (earthquake_timer == 30 && earthquake_type == 20) {
    E->wsozr_var_03 = E->wsozr_var_F;
    E->wsozr_var_F = FUNC16(FireZoomer_Func_2);
  }
  if (Enemy_MoveDown(k, Shift8AddMagn(E->wsozr_var_B, 1))) {
    E->wsozr_var_04 = 0;
    if (!(Enemy_MoveRight_ProcessSlopes(k, INT16_SHL8(E->wsozr_var_A)))) {
      EnemyFunc_C8AD(k);
      if ((int16)(samus_x_pos - E->base.x_pos) >= 0) {
        v7 = E->wsozr_var_A;
        if (v7 < 0)
          v7 = -v7;
      } else {
        v7 = E->wsozr_var_A;
        if (v7 >= 0)
          v7 = -v7;
      }
      E->wsozr_var_A = v7;
      return;
    }
    E->wsozr_var_B = -E->wsozr_var_B;
    v9 = addr_kWreckedShipOrangeZoomer_Ilist_DFE7;
    if ((E->wsozr_var_A & 0x8000) != 0)
      v9 = addr_kWreckedShipOrangeZoomer_Ilist_DFCB;
    E->base.current_instruction = v9;
    E->base.instruction_timer = 1;
  } else {
    uint16 v11 = E->wsozr_var_04 + 1;
    E->wsozr_var_04 = v11;
    if (sign16(v11 - 4)) {
      E->wsozr_var_A = -E->wsozr_var_A;
      v12 = addr_kWreckedShipOrangeZoomer_Ilist_DFE7;
      if ((E->wsozr_var_A & 0x8000) != 0)
        v12 = addr_kWreckedShipOrangeZoomer_Ilist_DFCB;
      E->base.current_instruction = v12;
      E->base.instruction_timer = 1;
    } else {
      E->wsozr_var_03 = E->wsozr_var_F;
      E->wsozr_var_F = FUNC16(FireZoomer_Func_2);
    }
  }
}

void FireZoomer_Init(void) {  // 0xA3E59C
  Enemy_FireZoomer *E = Get_FireZoomer(cur_enemy_index);
  E->base.current_instruction = g_off_A3E2CC[(E->base.current_instruction & 3)];
  StoneZoomer_E67A(cur_enemy_index);
}

const uint16 *Zoomer_Instr_SetPreinstr(uint16 k, const uint16 *jp) {  // 0xA3E660
  gEnemyData(k)->ai_preinstr = jp[0];
  return jp + 1;
}

void StoneZoomer_Init(void) {  // 0xA3E669
  Enemy_StoneZoomer *E = Get_StoneZoomer(cur_enemy_index);
  E->base.current_instruction = g_off_A3E2CC[(E->base.current_instruction & 3)];
  StoneZoomer_E67A(cur_enemy_index);
}

void StoneZoomer_E67A(uint16 k) {  // 0xA3E67A
  Enemy_StoneZoomer *E = Get_StoneZoomer(k);
  E->base.spritemap_pointer = addr_kSpritemap_Nothing_A3;
  E->base.instruction_timer = 1;
  E->szr_var_F = FUNC16(nullsub_304);
  if (E->szr_parameter_1 != 255) {
    uint16 v2 = g_word_A3E5F0[E->szr_parameter_1];
    E->szr_var_A = v2;
    E->szr_var_B = v2;
  }
  if ((E->base.properties & 3) != 0) {
    if ((E->base.properties & 3) == 2)
      E->szr_var_B = -E->szr_var_B;
  } else {
    E->szr_var_A = -E->szr_var_A;
  }
}

void StoneZoomer_Main(void) {  // 0xA3E6C2
  Enemy_StoneZoomer *E = Get_StoneZoomer(cur_enemy_index);
  EnemyRunPreInstr(E->szr_var_F);
}

void FireZoomer_Func_1(uint16 k) {  // 0xA3E6C8
  Enemy_FireZoomer *E = Get_FireZoomer(k);
  if (earthquake_timer == 30 && earthquake_type == 20) {
    E->fzr_var_03 = E->fzr_var_F;
    E->fzr_var_F = FUNC16(FireZoomer_Func_2);
  }
  if (Enemy_MoveRight_IgnoreSlopes(k, Shift8AddMagn(E->fzr_var_A, 1))) {
    E->fzr_var_04 = 0;
    EnemyFunc_C8AD(k);
    if (Enemy_MoveDown(k, INT16_SHL8(E->fzr_var_B))) {
      E->fzr_var_A = -E->fzr_var_A;
      uint16 fzr_parameter_2 = E->fzr_parameter_2, v7;
      if ((E->fzr_var_B & 0x8000) == 0)
        v7 = g_off_A3E63C[fzr_parameter_2 >> 1];
      else
        v7 = g_off_A3E630[fzr_parameter_2 >> 1];
      E->base.current_instruction = v7;
      E->base.instruction_timer = 1;
    }
  } else {
    uint16 v8 = E->fzr_var_04 + 1;
    E->fzr_var_04 = v8;
    if (sign16(v8 - 4)) {
      E->fzr_var_B = -E->fzr_var_B;
      uint16 v9 = E->fzr_parameter_2, v10;
      if ((E->fzr_var_B & 0x8000) == 0)
        v10 = g_off_A3E63C[v9 >> 1];
      else
        v10 = g_off_A3E630[v9 >> 1];
      E->base.current_instruction = v10;
      E->base.instruction_timer = 1;
    } else {
      E->fzr_var_03 = E->fzr_var_F;
      E->fzr_var_F = FUNC16(FireZoomer_Func_2);
    }
  }
}

void FireZoomer_Func_2(uint16 k) {  // 0xA3E785
  Enemy_FireZoomer *E = Get_FireZoomer(k);
  if (Enemy_MoveDown(k, __PAIR32__(E->fzr_var_02, E->fzr_var_01))) {
    if (E->fzr_parameter_1 == 255) {
      E->fzr_var_A = 128;
      E->fzr_var_B = 128;
    }
    E->fzr_var_01 = 0;
    E->fzr_var_02 = 0;
    E->fzr_var_04 = 0;
    E->fzr_var_F = E->fzr_var_03;
  } else {
    if (sign16(E->fzr_var_02 - 4))
      AddToHiLo(&E->fzr_var_02, &E->fzr_var_01, 0x8000);
    if (!E->fzr_var_01 && !E->fzr_var_02)
      E->fzr_var_F = FUNC16(FireZoomer_Func_1);
  }
}

void FireZoomer_Func_3(uint16 k) {  // 0xA3E7F2
  Enemy_FireZoomer *E = Get_FireZoomer(k);

  if (earthquake_timer == 30 && earthquake_type == 20) {
    E->fzr_var_03 = E->fzr_var_F;
    E->fzr_var_F = FUNC16(FireZoomer_Func_2);
  }
  if (Enemy_MoveDown(k, Shift8AddMagn(E->fzr_var_B, 1))) {
    E->fzr_var_04 = 0;
    int32 amt = FireZoomer_E8A5(k);
    if (Enemy_MoveRight_IgnoreSlopes(k, amt)) {
      E->fzr_var_B = -E->fzr_var_B;
      uint16 fzr_parameter_2 = E->fzr_parameter_2, v6;
      if ((E->fzr_var_A & 0x8000) == 0)
        v6 = g_off_A3E654[fzr_parameter_2 >> 1];
      else
        v6 = g_off_A3E648[fzr_parameter_2 >> 1];
      E->base.current_instruction = v6;
      E->base.instruction_timer = 1;
    } else {
      EnemyFunc_C8AD(k);
    }
  } else {
    uint16 v7 = E->fzr_var_04 + 1;
    E->fzr_var_04 = v7;
    if (sign16(v7 - 4)) {
      E->fzr_var_A = -E->fzr_var_A;
      uint16 v8 = E->fzr_parameter_2, v9;
      if ((E->fzr_var_A & 0x8000) == 0)
        v9 = g_off_A3E654[v8 >> 1];
      else
        v9 = g_off_A3E648[v8 >> 1];
      E->base.current_instruction = v9;
      E->base.instruction_timer = 1;
    } else {
      E->fzr_var_03 = E->fzr_var_F;
      E->fzr_var_F = FUNC16(FireZoomer_Func_2);
    }
  }
}

int32 FireZoomer_E8A5(uint16 k) {  // 0xA3E8A5
  Enemy_FireZoomer *E = Get_FireZoomer(cur_enemy_index);

  uint16 xpos = E->base.x_pos;
  uint16 ypos = sign16(E->fzr_var_B) ?
    E->base.y_pos - E->base.y_height :
    E->base.y_pos + E->base.y_height - 1;
  CalculateBlockContainingPixelPos(xpos, ypos);

  if ((level_data[cur_block_index] & 0xF000) == 4096 && (BTS[cur_block_index] & 0x1F) >= 5) {
    uint16 v1 = g_word_A3E931[(uint16)(4 * (BTS[cur_block_index] & 0x1F)) >> 1];
    if ((int16)E->fzr_var_A >= 0)
      return Multiply16x16(E->fzr_var_A, v1);
    else
      return -(int32)Multiply16x16(-E->fzr_var_A, v1);
  } else {
    return INT16_SHL8(E->fzr_var_A);
  }
}
