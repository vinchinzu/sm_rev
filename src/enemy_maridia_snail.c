// Enemy AI - Maridia snail — peeled from Bank $A3
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

#define g_stru_A3CD42 ((MaridiaSnailData2*)RomFixedPtr(0xa3cd42))
#define g_word_A3CD82 ((uint16*)RomFixedPtr(0xa3cd82))
#define g_word_A3CDC2 ((uint16*)RomFixedPtr(0xa3cdc2))
#define g_word_A3CCA2 ((uint16*)RomFixedPtr(0xa3cca2))
#define g_off_A3CDD2 ((uint16*)RomFixedPtr(0xa3cdd2))
#define g_off_A3D1AB ((uint16*)RomFixedPtr(0xa3d1ab))
#define g_off_A3D30D ((uint16*)RomFixedPtr(0xa3d30d))
#define g_off_A3D50F ((uint16*)RomFixedPtr(0xa3d50f))
#define g_word_A3D517 ((uint16*)RomFixedPtr(0xa3d517))
#define g_off_A3D5A4 ((uint16*)RomFixedPtr(0xa3d5a4))

const uint16 *MaridiaSnail_Instr_1(uint16 k, const uint16 *jp) {  // 0xA3CC36
  Get_MaridiaSnail(k)->msl_var_F = *jp;
  return jp + 1;
}

const uint16 *MaridiaSnail_Instr_2(uint16 k, const uint16 *jp) {  // 0xA3CC3F
  Get_MaridiaSnail(k)->msl_var_D = *jp;
  return jp + 1;
}

const uint16 *MaridiaSnail_Instr_4(uint16 k, const uint16 *jp) {  // 0xA3CC48
  Enemy_MaridiaSnail *E = Get_MaridiaSnail(k);
  E->msl_var_07 = jp[0];
  E->msl_var_C = *(uint16 *)((uint8 *)&g_stru_A3CD42[0].field_6 + 8 * E->msl_var_07);
  return jp + 1;
}

const uint16 *MaridiaSnail_Instr_3(uint16 k, const uint16 *jp) {  // 0xA3CC5F
  Enemy_MaridiaSnail *E = Get_MaridiaSnail(k);
  E->base.x_pos += jp[0];
  E->base.y_pos += jp[1];
  return jp + 2;
}

const uint16 *MaridiaSnail_Instr_5(uint16 k, const uint16 *jp) {  // 0xA3CC78
  if (Get_MaridiaSnail(k)->msl_var_08 == 2 || (NextRandom() & 1) != 0)
    jp -= 3;
  return jp;
}

void MaridiaSnail_Func_1(uint16 k) {  // 0xA3CC92
  MaridiaSnail_Func_17(k);
}

void MaridiaSnail_Init(void) {  // 0xA3CDE2
  Enemy_MaridiaSnail *E = Get_MaridiaSnail(cur_enemy_index);
  E->msl_var_F = FUNC16(nullsub_215);
  E->base.spritemap_pointer = addr_kSpritemap_Nothing_A3;
  E->base.instruction_timer = 1;
  uint16 current_instruction = E->base.current_instruction;
  E->base.current_instruction = g_stru_A3CD42[current_instruction].field_0;
  E->base.properties |= g_stru_A3CD42[current_instruction].field_2;
  E->msl_var_D = g_stru_A3CD42[current_instruction].field_4;
  E->msl_var_C = g_stru_A3CD42[current_instruction].field_6;
  E->msl_var_08 = 0;
  E->msl_var_06 = E->msl_parameter_1;
  MaridiaSnail_Func_2(cur_enemy_index, current_instruction * 8);
}

void MaridiaSnail_Func_2(uint16 k, uint16 j) {  // 0xA3CE27
  int v2 = j >> 1;
  uint16 r18 = g_word_A3CD82[v2];
  uint16 r20 = g_word_A3CD82[v2 + 1];
  uint16 r22 = g_word_A3CD82[v2 + 2];
  uint16 r24 = g_word_A3CD82[v2 + 3];
  Enemy_MaridiaSnail *E = Get_MaridiaSnail(k);
  E->msl_var_A = r20 + (r18 ^ g_word_A3CCA2[E->msl_parameter_1]);
  E->msl_var_B = r24 + (r22 ^ g_word_A3CCA2[E->msl_parameter_1]);
}

void MaridiaSnail_Func_3(uint16 k) {  // 0xA3CE57
  Enemy_MaridiaSnail *E = Get_MaridiaSnail(k);
  E->msl_var_F = g_off_A3CDD2[E->msl_var_07];
}

void MaridiaSnail_Main(void) {  // 0xA3CE64
  MaridiaSnail_Func_4(cur_enemy_index);
  MaridiaSnail_Func_5(cur_enemy_index);
  MaridiaSnail_Func_6(cur_enemy_index);
  Enemy_MaridiaSnail *E = Get_MaridiaSnail(cur_enemy_index);
  EnemyRunPreInstr(E->msl_var_F);
}

void MaridiaSnail_Func_4(uint16 k) {  // 0xA3CE73
  uint16 msl_var_08 = Get_MaridiaSnail(k)->msl_var_08;
  if (msl_var_08 != 3 && msl_var_08 != 4 && msl_var_08 != 5 && earthquake_timer == 30 && earthquake_type == 20)
    MaridiaSnail_Func_14(k);
}

void MaridiaSnail_Func_5(uint16 k) {  // 0xA3CE9A
  Enemy_MaridiaSnail *E = Get_MaridiaSnail(k);
  uint16 msl_var_08 = E->msl_var_08;
  if (msl_var_08 != 1 && msl_var_08 != 3 && msl_var_08 != 4 && msl_var_08 != 5) {
    if (!sign16(E->base.y_pos - samus_y_pos + 96)) {
      if ((int16)(E->base.x_pos - samus_x_pos) < 0) {
        if (samus_pose_x_dir != 4)
          goto LABEL_14;
      } else if (samus_pose_x_dir != 8) {
        goto LABEL_14;
      }
      if (E->msl_var_08 == 2)
        return;
      if (E->msl_var_D != FUNC16(nullsub_215)) {
        E->base.current_instruction = E->msl_var_D;
        E->base.spritemap_pointer = addr_kSpritemap_Nothing_A3;
        E->base.instruction_timer = 1;
        E->base.timer = 0;
        E->msl_var_08 = 2;
        return;
      }
    }
LABEL_14:
    E->msl_var_08 = 0;
  }
}

void MaridiaSnail_Func_6(uint16 k) {  // 0xA3CF11
  Enemy_MaridiaSnail *E = Get_MaridiaSnail(k);
  if (E->msl_var_F == FUNC16(MaridiaSnail_Func_15))
    goto LABEL_11;
  if (E->msl_var_08 == 1) {
    if (samus_has_momentum_flag || E->msl_var_F != FUNC16(nullsub_215))
      goto LABEL_11;
LABEL_10:
    E->base.properties |= kEnemyProps_SolidToSamus;
    return;
  }
  if (!samus_has_momentum_flag) {
    uint16 msl_var_08 = E->msl_var_08;
    if (msl_var_08 == 2 || msl_var_08 != 3 && msl_var_08 != 5)
      goto LABEL_10;
  }
LABEL_11:
  E->base.properties &= ~kEnemyProps_SolidToSamus;
}

static uint32 Shift8AddMagn(int16 a, int s) {
  int32 r = (a << 8);
  r += (r < 0) ? -(s << 16) : (s << 16);
  return r;
}

void MaridiaSnail_Func_7(uint16 k) {  // 0xA3CF60
  Enemy_MaridiaSnail *E = Get_MaridiaSnail(k);
  if (E->msl_var_07 >= 4) {
    if (EnemyFunc_BC76(k, Shift8AddMagn(E->msl_var_B, 7)))
      return;
  } else {
    if (EnemyFunc_BBBF(k, Shift8AddMagn(E->msl_var_A, 7)))
      return;
  }
  MaridiaSnail_Func_14(k);
}

void MaridiaSnail_Func_9(uint16 k) {  // 0xA3CFA6
  uint16 v1 = Get_MaridiaSnail(k)->msl_var_05 ? addr_stru_A3CD22 : addr_stru_A3CCE2;
  MaridiaSnail_D07E(k, RomPtr_A3(v1));
}

void MaridiaSnail_CFB7(uint16 k) {  // 0xA3CFB7
  MaridiaSnail_D002(k, RomPtr_A3(addr_stru_A3CCEA));
}

void MaridiaSnail_CFBD(uint16 k) {  // 0xA3CFBD
  uint16 v1 = Get_MaridiaSnail(k)->msl_var_05 ? addr_stru_A3CD2A : addr_stru_A3CCF2;
  MaridiaSnail_D07E(k, RomPtr_A3(v1));
}

void MaridiaSnail_CFCE(uint16 k) {  // 0xA3CFCE
  MaridiaSnail_D002(k, RomPtr_A3(addr_stru_A3CCFA));
}

void MaridiaSnail_CFD4(uint16 k) {  // 0xA3CFD4
  uint16 v1 = Get_MaridiaSnail(k)->msl_var_05 ? addr_stru_A3CD32 : addr_stru_A3CD02;
  MaridiaSnail_D07E(k, RomPtr_A3(v1));
}

void MaridiaSnail_CFE5(uint16 k) {  // 0xA3CFE5
  MaridiaSnail_D002(k, RomPtr_A3(addr_stru_A3CD0A));
}

void MaridiaSnail_CFEB(uint16 k) {  // 0xA3CFEB
  uint16 v1;
  if (Get_MaridiaSnail(k)->msl_var_05)
    v1 = addr_stru_A3CD3A;
  else
    v1 = addr_stru_A3CD12;
  MaridiaSnail_D07E(k, RomPtr_A3(v1));
}

void MaridiaSnail_CFFC(uint16 k) {  // 0xA3CFFC
  MaridiaSnail_D002(k, RomPtr_A3(addr_stru_A3CD1A));
}

void MaridiaSnail_D002(uint16 k, const uint8 *j) {  // 0xA3D002
  MaridiaSnail_Func_10(k, j);
  Enemy_MaridiaSnail *E = Get_MaridiaSnail(k);
  uint8 rv = Enemy_MoveRight_ProcessSlopes(k, Shift8AddMagn(E->msl_var_A, 1));
  MaridiaSnail_Func_11(k, j);
  if (rv) {
    E->msl_var_E = 0;
    uint8 carry = EnemyFunc_C8AD(k);
    MaridiaSnail_Func_12(k, carry);
    if (Enemy_MoveDown(k, INT16_SHL8(E->msl_var_B))) {
      E->msl_var_A = -E->msl_var_A;
      MaridiaSnail_Func_13(k, GET_WORD(j + 6));
    }
  } else {
    uint16 v7 = E->msl_var_E + 1;
    E->msl_var_E = v7;
    if (sign16(v7 - 4)) {
      E->msl_var_B = -E->msl_var_B;
      MaridiaSnail_Func_13(k, GET_WORD(j + 4));
    } else {
      MaridiaSnail_Func_14(k);
    }
  }
}

void MaridiaSnail_D07E(uint16 k, const uint8 *j) {  // 0xA3D07E
  MaridiaSnail_Func_10(k, j);
  Enemy_MaridiaSnail *E = Get_MaridiaSnail(k);
  uint8 rv = Enemy_MoveDown(k, Shift8AddMagn(E->msl_var_B, 1));
  MaridiaSnail_Func_11(k, j);
  if (rv) {
    E->msl_var_E = 0;
    if (Enemy_MoveRight_ProcessSlopes(k, INT16_SHL8(E->msl_var_A))) {
      E->msl_var_B = -E->msl_var_B;
      MaridiaSnail_Func_13(k, GET_WORD(j + 6));
    } else {
      uint8 carry = EnemyFunc_C8AD(k);
      MaridiaSnail_Func_12(k, carry);
    }
  } else {
    uint16 v7 = E->msl_var_E + 1;
    E->msl_var_E = v7;
    if (sign16(v7 - 4)) {
      E->msl_var_A = -E->msl_var_A;
      MaridiaSnail_Func_13(k, GET_WORD(j + 4));
    } else {
      MaridiaSnail_Func_14(k);
    }
  }
}

void MaridiaSnail_Func_10(uint16 k, const uint8 *j) {  // 0xA3D0F8
  Enemy_MaridiaSnail *E = Get_MaridiaSnail(k);
  E->base.x_pos += GET_WORD(j);
  E->base.y_pos += GET_WORD(j + 2);
}

void MaridiaSnail_Func_11(uint16 k, const uint8 *j) {  // 0xA3D10D
  Enemy_MaridiaSnail *E = Get_MaridiaSnail(k);
  E->base.x_pos -= GET_WORD(j);
  E->base.y_pos -= GET_WORD(j + 2);
}

void MaridiaSnail_Func_12(uint16 k, uint16 carry) {  // 0xA3D124
  Enemy_MaridiaSnail *E = Get_MaridiaSnail(k);
  if (!carry) {
    if ((uint16)(E->msl_var_04 + 1) >= 0x10)
      E->msl_var_05 = 0;
    else
      ++E->msl_var_04;
  } else {
    E->msl_var_05 = 1;
    E->msl_var_04 = 0;
  }
}

void MaridiaSnail_Func_13(uint16 k, uint16 a) {  // 0xA3D14C
  Enemy_MaridiaSnail *E = Get_MaridiaSnail(k);
  E->base.current_instruction = a;
  E->base.instruction_timer = 1;
  E->msl_var_05 = 1;
  E->msl_var_04 = 0;
}

void MaridiaSnail_Func_14(uint16 k) {  // 0xA3D164
  Enemy_MaridiaSnail *E = Get_MaridiaSnail(k);
  if (E->msl_var_08 != 3) {
    E->msl_var_08 = 3;
    E->msl_var_F = FUNC16(MaridiaSnail_Func_15);
    int v2 = 2 * E->msl_var_C;
    E->base.current_instruction = g_off_A3D1AB[v2];
    E->msl_var_D = g_off_A3D1AB[v2 + 1];
    E->base.instruction_timer = 1;
    E->base.timer = 0;
    E->msl_var_02 = 0;
    E->msl_var_03 = 0;
    E->msl_var_00 = 0;
    E->msl_var_01 = 0;
  }
}

void MaridiaSnail_Func_15(uint16 k) {  // 0xA3D1B3
  Enemy_MaridiaSnail *E = Get_MaridiaSnail(k);
  if (E->msl_var_08 != 3) {
    if (Enemy_MoveRight_IgnoreSlopes(k, __PAIR32__(E->msl_var_03, E->msl_var_02))) {
      E->msl_var_02 = -E->msl_var_02;
      E->msl_var_03 = -E->msl_var_03;
      E->msl_var_20 = 1;
      QueueSfx2_Max3(0x70);
    } else {
      uint32 delta;
      if ((E->msl_var_03 & 0x8000) != 0)
        delta = 4096;
      else
        delta = -4096;
      uint32 t = __PAIR32__(E->msl_var_03, E->msl_var_02) + delta;
      E->msl_var_02 = t;
      if (t >> 16)
        E->msl_var_03 = (t >> 16);
    }
  }
  if (Enemy_MoveDown(k, __PAIR32__(E->msl_var_01, E->msl_var_00))) {
    int16 msl_var_01 = E->msl_var_01;
    if (msl_var_01 >= 0 && sign16(msl_var_01 - 3)) {
      E->msl_var_03 = 0;
      E->msl_var_02 = 0;
      E->msl_var_01 = 0;
      E->msl_var_00 = 0;
      E->msl_var_E = 0;
      E->msl_var_04 = 0;
      E->msl_var_05 = 1;
      if (E->msl_var_08 == 3) {
        E->msl_var_08 = 0;
      } else {
        E->msl_var_08 = 1;
        E->msl_parameter_1 = 8;
        MaridiaSnail_Func_18(k);
        MaridiaSnail_Func_16(k);
      }
      E->base.properties = E->msl_var_C | E->base.properties & ~3;
      StoneZoomer_E67A(k);
      E->msl_var_F = FUNC16(nullsub_215);
      E->base.current_instruction = E->msl_var_D;
      E->base.instruction_timer = 1;
      E->base.timer = 0;
    } else {
      E->msl_var_00 = -E->msl_var_00;
      E->msl_var_01 = -E->msl_var_01;
      E->msl_var_20 = 0;
    }
  } else {
    uint32 t = __PAIR32__(E->msl_var_01, E->msl_var_00) + 0x2000;
    E->msl_var_00 = t;
    if (sign16((t >> 16) - 4))
      E->msl_var_01 = (t >> 16);
  }
}

void MaridiaSnail_Func_16(uint16 k) {  // 0xA3D2FA
  Enemy_MaridiaSnail *E = Get_MaridiaSnail(k);
  int v2 = (uint16)(4 * E->msl_var_C) >> 1;
  E->base.current_instruction = g_off_A3D30D[v2];
  E->msl_var_D = g_off_A3D30D[v2 + 1];
}

uint8 MaridiaSnail_Func_17(uint16 k) {  // 0xA3D315
  Enemy_MaridiaSnail *E = Get_MaridiaSnail(k);
  if (E->msl_var_05)
    return 0;
  if (E->msl_var_07 >= 4)
    return MaridiaSnail_Func_18(k) & 1;
  if ((E->msl_var_07 & 1) != 0) {
    return (E->base.y_pos >= samus_y_pos) ? MaridiaSnail_Func_19(k) : 0;
  } else {
    return (E->base.y_pos < samus_y_pos) ? MaridiaSnail_Func_19(k) : 0;
  }
}

uint8 MaridiaSnail_Func_18(uint16 k) {  // 0xA3D33E
  Enemy_MaridiaSnail *E = Get_MaridiaSnail(k);
  if (E->msl_var_C) {
    return (E->base.x_pos >= samus_x_pos) ? MaridiaSnail_Func_19(k) : 0;
  } else {
    return (E->base.x_pos < samus_x_pos) ? MaridiaSnail_Func_19(k) : 0;
  }
}

uint8 MaridiaSnail_Func_19(uint16 k) {  // 0xA3D356
  Enemy_MaridiaSnail *E = Get_MaridiaSnail(k);
  if (E->msl_var_08 == 2 || E->msl_var_F == 0xCF5F)
    return 0;
  uint16 v2 = g_word_A3CDC2[E->msl_var_07];
  E->msl_var_07 = v2;
  E->base.current_instruction = *(VoidP *)((uint8 *)&g_stru_A3CD42[0].field_0 + (8 * v2));
  E->base.properties = *(uint16 *)((uint8 *)&g_stru_A3CD42[0].field_2 + (8 * v2)) | E->base.properties & ~3;
  E->msl_var_D = *(VoidP *)((uint8 *)&g_stru_A3CD42[0].field_4 + (8 * v2));
  E->msl_var_C = *(uint16 *)((uint8 *)&g_stru_A3CD42[0].field_6 + (8 * v2));
  MaridiaSnail_Func_2(k, 8 * v2);
  MaridiaSnail_Func_3(k);
  E->msl_var_05 = 1;
  E->msl_var_04 = 0;
  return 1;
}

void MaridiaSnail_Touch(void) {  // 0xA3D3B0
  Enemy_MaridiaSnail *E = Get_MaridiaSnail(cur_enemy_index);
  if ((E->msl_var_08 != 1
       || E->msl_var_F == 0xCF5F
       || !(MaridiaSnail_Func_20(cur_enemy_index) & 1))
      && (E->msl_var_F == 0xD1B3 || samus_has_momentum_flag)) {
    MaridiaSnail_Func_22(cur_enemy_index);
    if (E->msl_var_F == 0xD1B3)
      QueueSfx2_Max3(0x70);
  } else if (E->msl_var_F != 0xCF5F && E->msl_var_08 != 4 && E->msl_var_08 != 3) {
    NormalEnemyTouchAi();
    E->msl_parameter_1 = E->msl_var_06;
    if (E->msl_var_08)
      MaridiaSnail_Func_19(cur_enemy_index);
    E->msl_var_08 = 0;
  }
}

uint8 MaridiaSnail_Func_20(uint16 k) {  // 0xA3D421
  uint16 r20 = Get_MaridiaSnail(k)->msl_var_C & 1;
  if ((uint16)(joypad1_lastkeys & 0x300) >> 8 == 1) {
    if (r20)
      return 0;
  } else if (!r20) {
    return 0;
  }
  return 1;
}

uint8 MaridiaSnail_Func_21(uint16 k) {  // 0xA3D446
  if ((Get_MaridiaSnail(k)->msl_var_A & 0x8000) != 0)
    return samus_pose_x_dir == 8;
  return samus_pose_x_dir == 4;
}

void MaridiaSnail_Shot(void) {  // 0xA3D469
  uint16 v0 = projectile_type[collision_detection_index] & 0xFF00;
  if (v0 == 768 || v0 == 1280) {
    NormalEnemyShotAi();
  } else {
    uint16 msl_var_08 = Get_MaridiaSnail(cur_enemy_index)->msl_var_08;
    if (msl_var_08 != 3 && msl_var_08 != 4)
      MaridiaSnail_Func_23(cur_enemy_index);
    QueueSfx2_Max3(0x70);
  }
}

void MaridiaSnail_Func_22(uint16 k) {  // 0xA3D49F
  Enemy_MaridiaSnail *E = Get_MaridiaSnail(k);
  E->msl_var_08 = 4;
  E->msl_var_F = FUNC16(MaridiaSnail_Func_15);
  int v2 = (uint16)(4 * E->msl_var_C) >> 1;
  E->base.current_instruction = g_off_A3D50F[v2];
  E->msl_var_D = g_off_A3D50F[v2 + 1];
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->msl_var_02 = absolute_moved_last_frame_x_fract;
  uint16 v3 = absolute_moved_last_frame_x;
  E->msl_var_03 = absolute_moved_last_frame_x;
  if (v3 >= 0x10)
    v3 = 15;
  int v4 = (uint16)(4 * v3) >> 1;
  E->msl_var_00 = g_word_A3D517[v4];
  E->msl_var_01 = g_word_A3D517[v4 + 1];
  if ((samus_pose_x_dir & 4) != 0) {
    E->msl_var_02 = -E->msl_var_02;
    E->msl_var_03 = -E->msl_var_03;
  }
}

void MaridiaSnail_Func_23(uint16 k) {  // 0xA3D557
  Enemy_MaridiaSnail *E = Get_MaridiaSnail(k);
  E->msl_var_08 = 5;
  E->msl_var_F = FUNC16(MaridiaSnail_Func_15);
  int v2 = (uint16)(4 * E->msl_var_C) >> 1;
  E->base.current_instruction = g_off_A3D5A4[v2];
  E->msl_var_D = g_off_A3D5A4[v2 + 1];
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->msl_var_01 = -1;
  E->msl_var_03 = (samus_pose_x_dir == 4) ? -1 : 1;
}
