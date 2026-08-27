// Enemy AI - Maridia Puffer — peeled from Bank $A8

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

static const int16 g_word_A8D871[10] = {
  0xc0, 0xe0,    0, 0x20, 0x40,
  0x40, 0x60, 0x80, 0xa0, 0xc0,
};
static const int16 g_word_A8D885[8] = {
  0x3ff, 0x4ff, 0x5ff, 0x6ff, 0x7ff,
  0x8ff, 0x9ff, 0xaff,
};
static const int16 g_word_A8D895[26] = {
    3,   1,   4,   1,   5,
    2,   6,   2,   7,   2,
    8,   3,   9,   3, 0xa,
    4, 0xb,   4, 0xc,   5,
  0xd,   5, 0xe,   6, 0xf,
    6,
};

void MaridiaPuffer_Init(void) {  // 0xA8D8C9
  Enemy_MaridiaPuffer *E = Get_MaridiaPuffer(cur_enemy_index);
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->base.current_instruction = addr_kMaridiaPuffer_Ilist_D841;
  int v1 = (uint16)(4 * E->mpr_parameter_1) >> 1;
  uint16 v2 = g_word_A8D895[v1];
  E->mpr_var_05 = v2;
  E->mpr_var_F = v2;
  E->mpr_var_06 = g_word_A8D895[v1 + 1];
  E->mpr_var_E = 16;
  E->mpr_var_A = FUNC16(MaridiaPuffer_Func_1);
  E->mpr_var_00 = g_word_A8D885[E->mpr_parameter_2];
}

void CallMaridiaPuffer(uint32 ea) {
  switch (ea) {
  case fnMaridiaPuffer_Func_1: MaridiaPuffer_Func_1(); return;
  case fnMaridiaPuffer_Func_2: MaridiaPuffer_Func_2(); return;
  case fnMaridiaPuffer_Func_3: MaridiaPuffer_Func_3(); return;
  case fnMaridiaPuffer_Func_4: MaridiaPuffer_Func_4(); return;
  default: Unreachable();
  }
}

void MaridiaPuffer_Main(void) {  // 0xA8D90B
  Enemy_MaridiaPuffer *E = Get_MaridiaPuffer(cur_enemy_index);
  uint16 v1 = E->mpr_var_07 - 1;
  E->mpr_var_07 = v1;
  if (!v1) {
    E->mpr_var_07 = 1;
    E->mpr_var_03 = 0;
  }
  CallMaridiaPuffer(E->mpr_var_A | 0xA80000);
}

void MaridiaPuffer_Func_1(void) {  // 0xA8D92B
  Enemy_MaridiaPuffer *E = Get_MaridiaPuffer(cur_enemy_index);
  if (E->mpr_var_E-- == 1) {
    E->mpr_var_E = 16;
    E->mpr_var_A = FUNC16(MaridiaPuffer_Func_2);
  }
}

void MaridiaPuffer_Func_2(void) {  // 0xA8D940
  uint8 v1 = CalculateAngleOfSamusFromEnemy(cur_enemy_index) - 64;
  Enemy_MaridiaPuffer *E = Get_MaridiaPuffer(cur_enemy_index);
  E->mpr_var_01 = v1;
  E->mpr_var_02 = v1;
  E->mpr_var_A = FUNC16(MaridiaPuffer_Func_3);
  E->mpr_var_C = 24;
}

void MaridiaPuffer_Func_3(void) {  // 0xA8D963
  Enemy_MaridiaPuffer *E = Get_MaridiaPuffer(cur_enemy_index);
  if ((int16)(E->mpr_var_D - E->mpr_var_00) < 0)
    MaridiaPuffer_Func_8(cur_enemy_index);
  MaridiaPuffer_Func_6();
  MaridiaPuffer_Func_7();
  MaridiaPuffer_Func_5();
}

void MaridiaPuffer_Func_4(void) {  // 0xA8D97C
  Enemy_MaridiaPuffer *E = Get_MaridiaPuffer(cur_enemy_index);
  if ((int16)E->mpr_var_D > 0 && (int16)E->mpr_var_B > 0) {
    MaridiaPuffer_Func_6();
    MaridiaPuffer_Func_7();
    MaridiaPuffer_Func_9(cur_enemy_index);
  } else {
    E->mpr_var_A = FUNC16(MaridiaPuffer_Func_1);
    E->mpr_var_B = 0;
    E->mpr_var_C = 0;
    E->mpr_var_D = 0;
  }
}

void MaridiaPuffer_Func_5(void) {  // 0xA8D9AA
  uint8 v1 = CalculateAngleOfSamusFromEnemy(cur_enemy_index) - 64;
  Enemy_MaridiaPuffer *E = Get_MaridiaPuffer(cur_enemy_index);
  E->mpr_var_01 = v1;
  uint16 v3 = SignExtend8(v1 - E->mpr_var_02);
  uint16 v4 = Abs16(v3);
  if (!sign16(v4 - 48)) {
    E->mpr_var_A = FUNC16(MaridiaPuffer_Func_4);
    E->mpr_var_C = 24;
  }
}

void MaridiaPuffer_Func_6(void) {  // 0xA8D9DB
  Enemy_MaridiaPuffer *E = Get_MaridiaPuffer(cur_enemy_index);
  uint16 R26 = 0;
  uint16 r18 = kSine16bit[(uint8)(E->mpr_var_02 + 64)];
  if ((r18 & 0x8000) != 0)
    ++R26;
  uint32 t = ((Abs16(r18) & 0xFF00) >> 8) * E->mpr_var_D;
  if (R26)
    t = -(int32)t;
  AddToHiLo(&E->base.x_pos, &E->base.x_subpos, t);
}

void MaridiaPuffer_Func_7(void) {  // 0xA8DA28
  Enemy_MaridiaPuffer *E = Get_MaridiaPuffer(cur_enemy_index);
  uint16 R26 = 0;
  uint16 r18 = kSine16bit[(uint8)E->mpr_var_02];
  if ((r18 & 0x8000) != 0)
    ++R26;
  uint32 t = ((Abs16(r18) & 0xFF00) >> 8) * E->mpr_var_D;
  if (R26)
    t = -(int32)t;
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, t);
}

void MaridiaPuffer_Func_8(uint16 k) {  // 0xA8DA71
  Enemy_MaridiaPuffer *E = Get_MaridiaPuffer(k);
  if (E->mpr_var_F-- == 1) {
    E->mpr_var_F = E->mpr_var_05;
    E->mpr_var_B += E->mpr_var_C;
    E->mpr_var_D += E->mpr_var_B;
  }
}

void MaridiaPuffer_Func_9(uint16 k) {  // 0xA8DA92
  Enemy_MaridiaPuffer *E = Get_MaridiaPuffer(k);
  if (E->mpr_var_F-- == 1) {
    E->mpr_var_F = E->mpr_var_06;
    E->mpr_var_B -= E->mpr_var_C;
    E->mpr_var_D -= E->mpr_var_B;
  }
}

void MaridiaPuffer_Shot(void) {  // 0xA8DB14
  Enemy_MaridiaPuffer *E = Get_MaridiaPuffer(cur_enemy_index);
  E->mpr_var_40 = E->base.health;
  NormalEnemyShotAi();
  if (E->base.health == E->mpr_var_40 && !E->mpr_var_03) {
    E->base.instruction_timer = 1;
    E->base.timer = 0;
    E->base.current_instruction = addr_kMaridiaPuffer_Ilist_D855;
    E->mpr_var_02 = g_word_A8D871[projectile_dir[collision_detection_index] & 0xF];
    E->mpr_var_B = 256;
    E->mpr_var_D = 1536;
    E->mpr_var_A = FUNC16(MaridiaPuffer_Func_4);
    E->mpr_var_07 = 48;
    E->mpr_var_03 = 1;
  }
}

