// MaridiaLargeSnail extracted from sm_a2.c.
#include "ida_types.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "variables.h"
#include "enemy_ai_canon.h"

#define g_off_A2CB77 ((uint16*)RomFixedPtr(0xa2cb77))


const uint16 *MaridiaLargeSnail_Instr_CB6B(uint16 k, const uint16 *jp) {  // 0xA2CB6B
  QueueSfx2_Max6(0xE);
  return jp;
}

const uint16 *MaridiaLargeSnail_Instr_CCB3(uint16 k, const uint16 *jp) {  // 0xA2CCB3
  Get_MaridiaLargeSnail(cur_enemy_index)->mlsl_var_02 = 1;
  return jp;
}

const uint16 *MaridiaLargeSnail_Instr_CCBE(uint16 k, const uint16 *jp) {  // 0xA2CCBE
  Get_MaridiaLargeSnail(cur_enemy_index)->mlsl_var_03 = 1;
  return jp;
}

const uint16 *MaridiaLargeSnail_Instr_CCC9(uint16 k, const uint16 *jp) {  // 0xA2CCC9
  Get_MaridiaLargeSnail(cur_enemy_index)->mlsl_var_03 = 0;
  return jp;
}

void MaridiaLargeSnail_Init(void) {  // 0xA2CCD4
  Enemy_MaridiaLargeSnail *E = Get_MaridiaLargeSnail(cur_enemy_index);
  E->mlsl_var_B = 0;
  E->mlsl_var_D = 0;
  E->mlsl_var_00 = 0;
  E->mlsl_var_01 = 0;
  E->mlsl_var_02 = 0;
  E->mlsl_var_03 = 0;
  E->mlsl_var_0A = 0;
  E->mlsl_var_E = 3;
  E->mlsl_var_C = 128;
  E->base.current_instruction = addr_kMaridiaLargeSnail_Ilist_CA4B;
  E->mlsl_var_A = FUNC16(MaridiaLargeSnail_Func_4);
  E->mlsl_var_F = FUNC16(MaridiaLargeSnail_Func_7);
}


void CallMaridiaLargeSnailFunc(uint32 ea) {
  switch (ea) {
  case fnMaridiaLargeSnail_Func_4: MaridiaLargeSnail_Func_4(); return;
  case fnMaridiaLargeSnail_Func_5: MaridiaLargeSnail_Func_5(); return;
  case fnMaridiaLargeSnail_Func_6: MaridiaLargeSnail_Func_6(); return;
  default: Unreachable();
  }
}
void MaridiaLargeSnail_Main(void) {  // 0xA2CD13
  MaridiaLargeSnail_Func_1(cur_enemy_index);
  Enemy_MaridiaLargeSnail *E = Get_MaridiaLargeSnail(cur_enemy_index);
  CallMaridiaLargeSnailFunc(E->mlsl_var_A | 0xA20000);
  MaridiaLargeSnail_Func_2(cur_enemy_index);
  MaridiaLargeSnail_Func_3(cur_enemy_index);
}

void MaridiaLargeSnail_Func_1(uint16 k) {  // 0xA2CD23
  MaridiaLargeSnail_Func_11();
  Enemy_MaridiaLargeSnail *E = Get_MaridiaLargeSnail(k);
  E->mlsl_var_06 = E->base.x_pos;
  E->mlsl_var_08 = E->base.y_pos;
}

void MaridiaLargeSnail_Func_2(uint16 k) {  // 0xA2CD35
  Enemy_MaridiaLargeSnail *E = Get_MaridiaLargeSnail(k);
  E->mlsl_var_0B = 0;
  if (CheckIfEnemyTouchesSamus(k))
    extra_samus_y_displacement = E->base.y_pos - E->mlsl_var_08;
  if (E->mlsl_var_04) {
    uint16 r18 = E->base.x_pos - E->mlsl_var_06;
    if ((r18 & 0x8000) != 0) {
      if (!E->mlsl_var_05)
        return;
    } else if (E->mlsl_var_05) {
      return;
    }
    extra_samus_x_displacement += r18;
  }
}

void MaridiaLargeSnail_Func_3(uint16 k) {  // 0xA2CD77
  int16 v2;

  Enemy_MaridiaLargeSnail *E = Get_MaridiaLargeSnail(k);
  if (E->mlsl_var_04) {
    v2 = E->base.x_pos - E->mlsl_var_06;
    if (!v2) {
      if (E->mlsl_var_05) {
        if (!E->mlsl_var_D || (joypad1_lastkeys & 0x100) == 0)
          return;
      } else if (E->mlsl_var_D || (joypad1_lastkeys & 0x200) == 0) {
        return;
      }
      E->mlsl_var_0B = 1;
      E->base.x_pos = E->mlsl_var_06;
      return;
    }
    if (v2 < 0) {
      if ((joypad1_lastkeys & 0x100) != 0) {
        E->mlsl_var_0B = 1;
        E->base.x_pos = E->mlsl_var_06;
      }
    } else if ((joypad1_lastkeys & 0x200) != 0) {
      E->mlsl_var_0B = 1;
      E->base.x_pos = E->mlsl_var_06;
    }
  }
}

void MaridiaLargeSnail_Func_4(void) {  // 0xA2CDE6
  Enemy_MaridiaLargeSnail *E = Get_MaridiaLargeSnail(cur_enemy_index);
  if (E->mlsl_var_E) {
    EnemyRunPreInstr(E->mlsl_var_F);
  } else {
    E->mlsl_var_00 = 0;
    if ((GetSamusEnemyDelta_X(cur_enemy_index) & 0x8000) != 0) {
      E->mlsl_var_D = 1;
      E->mlsl_var_00 = 1;
    }
    MaridiaLargeSnail_Func_10(cur_enemy_index);
    if (IsSamusWithinEnemy_X(cur_enemy_index, 0x18)) {
      E->mlsl_var_00 |= 2;
      MaridiaLargeSnail_Func_10(cur_enemy_index);
      E->mlsl_var_A = FUNC16(MaridiaLargeSnail_Func_5);
    }
  }
}

void MaridiaLargeSnail_Func_5(void) {  // 0xA2CE2B
  Enemy_MaridiaLargeSnail *E = Get_MaridiaLargeSnail(cur_enemy_index);
  if (E->mlsl_var_E) {
    EnemyRunPreInstr(E->mlsl_var_F);
  } else {
    if (!(Enemy_MoveDown(cur_enemy_index, INT16_SHL16(1)))) {
      E->mlsl_var_B = 0;
      E->mlsl_var_F = FUNC16(MaridiaLargeSnail_Func_7);
      E->mlsl_var_E = 3;
    }
  }
  if (!E->mlsl_var_0B) {
    uint16 R36 = 0;
    bool v4 = (--E->mlsl_var_C & 0x8000) != 0;
    if (v4 && (E->mlsl_var_C = 0, IsSamusWithinEnemy_X(cur_enemy_index, 0x20)) && E->mlsl_var_03 && !E->mlsl_var_E) {
      E->mlsl_var_00 = 0;
      E->mlsl_var_D = 0;
      if ((GetSamusEnemyDelta_X(cur_enemy_index) & 0x8000) != 0) {
        E->mlsl_var_00 = 1;
        E->mlsl_var_D = 1;
      }
      E->mlsl_var_C = 128;
      E->mlsl_var_00 = E->mlsl_var_00 & 1 | 4;
      MaridiaLargeSnail_Func_10(cur_enemy_index);
      E->mlsl_var_A = FUNC16(MaridiaLargeSnail_Func_6);
    } else {
      uint16 v5 = 128;
      if (E->mlsl_var_D)
        v5 = 132;
      int v6 = v5 >> 1;
      uint16 r18 = kCommonEnemySpeeds_Linear[v6 + 1];
      uint16 r20 = kCommonEnemySpeeds_Linear[v6];
      if (E->mlsl_var_04 && E->mlsl_var_D == E->mlsl_var_05) {
        if (E->mlsl_var_D)
          r20 -= 16;
        else
          r20 += 16;
      }
      if (Enemy_MoveRight_SlopesAsWalls(cur_enemy_index, __PAIR32__(r20, r18)))
        ++R36;
      if (E->mlsl_var_04 && E->mlsl_var_D == E->mlsl_var_05) {
        uint16 v7;
        if (E->mlsl_var_D)
          v7 = E->base.x_pos + 16;
        else
          v7 = E->base.x_pos - 16;
        E->base.x_pos = v7;
      }
      if (R36) {
        E->mlsl_var_D ^= 1;
        E->mlsl_var_00 ^= 4;
        MaridiaLargeSnail_Func_10(cur_enemy_index);
      }
    }
  }
}

void MaridiaLargeSnail_Func_6(void) {  // 0xA2CF40
  Enemy_MaridiaLargeSnail *E = Get_MaridiaLargeSnail(cur_enemy_index);
  if (E->mlsl_var_02) {
    E->mlsl_var_02 = 0;
    E->mlsl_var_00 -= 2;
    MaridiaLargeSnail_Func_10(cur_enemy_index);
    E->mlsl_var_A = FUNC16(MaridiaLargeSnail_Func_5);
  }
}

void MaridiaLargeSnail_Func_7(uint16 k) {  // 0xA2CF66
  Enemy_MaridiaLargeSnail *E = Get_MaridiaLargeSnail(k);
  uint16 v2 = E->mlsl_var_B + 384;
  if (!sign16(v2 - 0x4000))
    v2 = 0x4000;
  E->mlsl_var_B = v2;
  
  int v5 = (8 * (v2 >> 8)) >> 1;
  if (Enemy_MoveDown(k, kCommonEnemySpeeds_Quadratic32[v5 >> 1])) {
    --E->mlsl_var_E;
    int16 v6 = E->mlsl_var_B - 4096;
    if (v6 < 0)
      E->mlsl_var_E = 0;
    E->mlsl_var_B = v6;
    E->mlsl_var_F = FUNC16(MaridiaLargeSnail_Func_8);
  }
}

void MaridiaLargeSnail_Func_8(uint16 k) {  // 0xA2CFA9
  Enemy_MaridiaLargeSnail *E = Get_MaridiaLargeSnail(k);
  int16 v2 = E->mlsl_var_B - 384;
  E->mlsl_var_B = v2;
  if (v2 >= 0) {
    uint16 v3 = (v2 & 0x7F00) >> 8;
    int v5 = (8 * v3) >> 1;
    Enemy_MoveDown(k, kCommonEnemySpeeds_Quadratic32[(v5 + 2) >> 1]);
  } else {
    E->mlsl_var_B = 0;
    E->mlsl_var_F = FUNC16(MaridiaLargeSnail_Func_7);
  }
}
// 7DD25: mask 0xFF00 is shortened because ax.2 <= 0x7FFF

void MaridiaLargeSnail_Func_9(void) {  // 0xA2CFD7
  QueueSfx2_Max6(0x63);
}

void MaridiaLargeSnail_Func_10(uint16 k) {  // 0xA2CFDF
  Enemy_MaridiaLargeSnail *E = Get_MaridiaLargeSnail(k);
  uint16 mlsl_var_00 = E->mlsl_var_00;
  if (mlsl_var_00 != E->mlsl_var_01) {
    E->mlsl_var_01 = mlsl_var_00;
    E->base.current_instruction = g_off_A2CB77[mlsl_var_00];
    E->base.instruction_timer = 1;
    E->base.timer = 0;
  }
}

void MaridiaLargeSnail_Func_11(void) {  // 0xA2CFFF
  Enemy_MaridiaLargeSnail *E = Get_MaridiaLargeSnail(cur_enemy_index);
  E->mlsl_var_04 = 0;
  E->mlsl_var_05 = 0;
  if (IsSamusWithinEnemy_Y(cur_enemy_index, 0x20) && IsSamusWithinEnemy_X(cur_enemy_index, 0x18)) {
    E->mlsl_var_04 = 1;
    if ((GetSamusEnemyDelta_X(cur_enemy_index) & 0x8000) != 0)
      E->mlsl_var_05 = 1;
  }
}

void MaridiaLargeSnail_Func_12(void) {  // 0xA2D388
  NormalEnemyTouchAi();
  MaridiaLargeSnail_Touch();
}

void MaridiaLargeSnail_Touch() {  // 0xA2D38C
  uint16 k = cur_enemy_index;
  if (!CheckIfEnemyTouchesSamus(k)) {
    if ((int16)(samus_x_pos - Get_MaridiaLargeSnail(cur_enemy_index)->base.x_pos) < 0)
      extra_samus_x_displacement -= 4;
    else
      extra_samus_x_displacement += 4;
  }
}

void MaridiaLargeSnail_Shot(void) {  // 0xA2D3B4
  NormalEnemyShotAi();
  QueueSfx2_Max6(0x57);
}
