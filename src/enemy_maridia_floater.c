// Enemy AI - Maridia Floater — peeled from Bank $A8

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

#define g_off_A8C599 ((uint16*)RomFixedPtr(0xa8c599))

static const int16 g_word_A8C277[3] = { -12, -16, -20 };
static const int16 g_word_A8C27D[3] = { -20, -16, -12 };
static const uint16 g_word_A8C19F = 0x40;
static const uint16 g_word_A8C1A1[12] = { 0, 1, 2, 3, 2, 1, 0, 0xffff, 0xfffe, 0xfffd, 0xfffe, 0xffff };
static const uint16 g_word_A8C1B9 = 0;
static const uint16 g_word_A8C1BB = 0;
static const uint16 g_word_A8C1BD = 0;
static const uint16 g_word_A8C1BF = 0;
static const uint16 g_word_A8C1C1 = 1;
static const uint16 g_word_A8C1C3 = 0;
static const uint16 g_word_A8C1C5 = 0xffff;
static const uint16 g_word_A8C1C7 = 0x8000;

void MaridiaFloater_Init(void) {  // 0xA8C1C9
  Enemy_MaridiaFloater *E = Get_MaridiaFloater(cur_enemy_index);
  E->base.properties |= kEnemyProps_ProcessInstructions;
  E->base.spritemap_pointer = addr_kSpritemap_Nothing_A8;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  if (E->mfr_parameter_1) {
    E->mfr_var_A = E->base.x_pos;
    E->mfr_var_B = E->base.y_pos;
    E->mfr_var_F = FUNC16(nullsub_256);
    E->base.current_instruction = addr_stru_A8C199;
    E->mfr_var_D = E->mfr_parameter_2;
  } else {
    E->mfr_var_E = 60;
    E->mfr_var_F = FUNC16(MaridiaFloater_Func_3);
    E->base.current_instruction = addr_kMaridiaFloater_Ilist_C173;
  }
}

void MaridiaFloater_Main(void) {  // 0xA8C21C
  Enemy_MaridiaFloater *E = Get_MaridiaFloater(cur_enemy_index);
  EnemyRunPreInstr(E->mfr_var_F);
}

void MaridiaFloater_Func_1(uint16 k) {  // 0xA8C223
  for (int i = 7; i >= 0; --i)
    SpawnEprojWithGfx(i, k, addr_kEproj_MaridiaFloatersSpikes);
}

void MaridiaFloater_Func_2(uint16 k) {  // 0xA8C234
  int v1 = k >> 1;
  uint16 v2 = enemy_drawing_queue[v1 + 103];
  if (sign16(v2 + 0x3E6F)) {
    uint16 v3 = (uint16)(v2 - 4 + 15997) >> 1;
    if (!sign16(v3 - 6))
      v3 = 0;
    enemy_drawing_queue[v1 + 93] = g_word_A8C277[v3 >> 1] + Get_MaridiaFloater(k)->base.y_pos;
  } else {
    uint16 v4 = (uint16)(v2 - 4 + 15983) >> 1;
    if (!sign16(v4 - 6))
      v4 = 0;
    enemy_drawing_queue[v1 + 93] = g_word_A8C27D[v4 >> 1] + Get_MaridiaFloater(k)->base.y_pos;
  }
}

void MaridiaFloater_Func_3(uint16 k) {  // 0xA8C283
  Enemy_MaridiaFloater *E = Get_MaridiaFloater(k);
  bool v2 = E->mfr_var_E == 1;
  bool v3 = (--E->mfr_var_E & 0x8000) != 0;
  if (v2 || v3) {
    int v4 = k >> 1;
    enemy_drawing_queue[v4 + 104] = 1;
    enemy_drawing_queue[v4 + 103] = addr_kMaridiaFloater_Ilist_C183;
    E->mfr_var_F = FUNC16(MaridiaFloater_Func_4);
    E->mfr_var_E = 10;
  }
  MaridiaFloater_Func_2(k);
}

void MaridiaFloater_Func_4(uint16 k) {  // 0xA8C2A6
  Enemy_MaridiaFloater *E = Get_MaridiaFloater(k);
  bool v2 = E->mfr_var_E == 1;
  bool v3 = (--E->mfr_var_E & 0x8000) != 0;
  if (v2 || v3) {
    E->mfr_var_F = FUNC16(MaridiaFloater_Func_5);
    E->mfr_var_A = g_word_A8C1C5;
    E->mfr_var_B = g_word_A8C1C7;
    E->base.instruction_timer = 1;
    E->base.current_instruction = addr_kMaridiaFloater_Ilist_C163;
  }
  MaridiaFloater_Func_2(k);
}

void MaridiaFloater_Func_5(uint16 k) {  // 0xA8C2CF
  Enemy_MaridiaFloater *E = Get_MaridiaFloater(k);
  if ((E->base.ai_handler_bits & 1) != 0) {
    E->mfr_var_F = FUNC16(MaridiaFloater_Func_7);
    MaridiaFloater_Func_7(k);
  } else {
    bool v2 = E->mfr_var_D == 1;
    bool v3 = (--E->mfr_var_D & 0x8000) != 0;
    if (v2 || v3) {
      E->mfr_var_D = 5;
      int v4 = k >> 1;
      uint16 v5 = g_word_A8C1A1[E->mfr_var_C] + enemy_drawing_queue_sizes[v4];
      enemy_drawing_queue[v4 + 91] = v5;
      E->base.x_pos = v5;
      uint16 v6 = E->mfr_var_C + 1;
      if (!sign16(E->mfr_var_C - 11))
        v6 = 0;
      E->mfr_var_C = v6;
    }
    AddToHiLo(&E->mfr_var_A, &E->mfr_var_B, __PAIR32__(g_word_A8C1B9, g_word_A8C1BB));
    if (Enemy_MoveDown(k, __PAIR32__(E->mfr_var_A, E->mfr_var_B))
        || (int16)(enemy_drawing_queue_sizes[(k >> 1) + 1] - g_word_A8C19F - E->base.y_pos) >= 0) {
      uint16 mfr_var_C = E->mfr_var_C;
      if (!mfr_var_C || mfr_var_C == 6) {
        E->mfr_var_F = FUNC16(MaridiaFloater_Func_10);
        E->mfr_var_E = 10;
        int v10 = k >> 1;
        enemy_drawing_queue[v10 + 104] = 1;
        enemy_drawing_queue[v10 + 103] = addr_kMaridiaFloater_Ilist_C191;
      } else {
        E->mfr_var_F = FUNC16(MaridiaFloater_Func_6);
      }
    }
    MaridiaFloater_Func_2(k);
  }
}

void MaridiaFloater_Func_6(uint16 k) {  // 0xA8C36B
  Enemy_MaridiaFloater *E = Get_MaridiaFloater(k);
  bool v2 = E->mfr_var_D == 1;
  bool v3 = (--E->mfr_var_D & 0x8000) != 0;
  if (!v2 && !v3) {
LABEL_9:;
    AddToHiLo(&E->mfr_var_A, &E->mfr_var_B, __PAIR32__(g_word_A8C1B9, g_word_A8C1BB));
    Enemy_MoveDown(k, __PAIR32__(E->mfr_var_A, E->mfr_var_B));
    goto LABEL_10;
  }
  E->mfr_var_D = 5;
  int v4;
  v4 = k >> 1;
  {
    uint16 v5 = g_word_A8C1A1[E->mfr_var_C] + enemy_drawing_queue_sizes[v4];
    enemy_drawing_queue[v4 + 91] = v5;
    E->base.x_pos = v5;
  }
  if (E->mfr_var_C && E->mfr_var_C != 6) {
    uint16 v7 = E->mfr_var_C + 1;
    if (!sign16(v7 - 12))
      v7 = 0;
    E->mfr_var_C = v7;
    goto LABEL_9;
  }
  E->mfr_var_F = FUNC16(MaridiaFloater_Func_10);
  E->mfr_var_E = 10;
  enemy_drawing_queue[v4 + 104] = 1;
  enemy_drawing_queue[v4 + 103] = addr_kMaridiaFloater_Ilist_C191;
LABEL_10:
  MaridiaFloater_Func_2(k);
}

void MaridiaFloater_Func_7(uint16 k) {  // 0xA8C3E1
  Enemy_MaridiaFloater *E = Get_MaridiaFloater(k);
  if ((E->base.ai_handler_bits & 1) != 0) {
    bool v2 = E->mfr_var_D == 1;
    bool v3 = (--E->mfr_var_D & 0x8000) != 0;
    if (v2 || v3) {
      E->mfr_var_D = 5;
      int v4 = k >> 1;
      uint16 v5 = g_word_A8C1A1[E->mfr_var_C] + enemy_drawing_queue_sizes[v4];
      enemy_drawing_queue[v4 + 91] = v5;
      E->base.x_pos = v5;
      uint16 v6 = E->mfr_var_C + 1;
      if (!sign16(E->mfr_var_C - 11))
        v6 = 0;
      E->mfr_var_C = v6;
    }
    AddToHiLo(&E->mfr_var_A, &E->mfr_var_B, __PAIR32__(g_word_A8C1B9, g_word_A8C1BB));
    if (Enemy_MoveDown(k, __PAIR32__(E->mfr_var_A, E->mfr_var_B))
        || (int16)(enemy_drawing_queue_sizes[(k >> 1) + 1] - enemy_drawing_queue_sizes[(k >> 1) + 3] - E->base.y_pos) >= 0) {
      if (!E->mfr_var_C || E->mfr_var_C == 6)
        E->mfr_var_F = FUNC16(MaridiaFloater_Func_9);
      else
        E->mfr_var_F = FUNC16(MaridiaFloater_Func_8);
    }
    MaridiaFloater_Func_2(k);
  } else {
    E->mfr_var_F = FUNC16(MaridiaFloater_Func_6);
  }
}

void MaridiaFloater_Func_8(uint16 k) {  // 0xA8C469
  Enemy_MaridiaFloater *E = Get_MaridiaFloater(k);
  if ((E->base.ai_handler_bits & 1) == 0) {
    E->mfr_var_F = FUNC16(MaridiaFloater_Func_6);
    return;
  }
  bool v2 = E->mfr_var_D == 1;
  bool v3 = (--E->mfr_var_D & 0x8000) != 0;
  if (!v2 && !v3)
    goto LABEL_11;
  E->mfr_var_D = 5;
  int v4;
  v4 = k >> 1;
  uint16 v5;
  v5 = g_word_A8C1A1[E->mfr_var_C] + enemy_drawing_queue_sizes[v4];
  enemy_drawing_queue[v4 + 91] = v5;
  E->base.x_pos = v5;
  if (E->mfr_var_C && E->mfr_var_C != 6) {
    {
      uint16 v7 = E->mfr_var_C + 1;
      if (!sign16(v7 - 12))
        v7 = 0;
      E->mfr_var_C = v7;
    }
LABEL_11:;
    AddToHiLo(&E->mfr_var_A, &E->mfr_var_B, __PAIR32__(g_word_A8C1B9, g_word_A8C1BB));
    Enemy_MoveDown(k, __PAIR32__(E->mfr_var_A, E->mfr_var_B));
    goto LABEL_12;
  }
  E->mfr_var_F = FUNC16(MaridiaFloater_Func_9);
LABEL_12:
  MaridiaFloater_Func_2(k);
}

void MaridiaFloater_Func_9(uint16 k) {  // 0xA8C4DC
  Enemy_MaridiaFloater *E = Get_MaridiaFloater(k);
  if ((E->base.ai_handler_bits & 1) == 0) {
    E->mfr_var_F = FUNC16(MaridiaFloater_Func_10);
    E->mfr_var_E = 10;
    int v2 = k >> 1;
    enemy_drawing_queue[v2 + 104] = 1;
    enemy_drawing_queue[v2 + 103] = addr_kMaridiaFloater_Ilist_C191;
  }
  MaridiaFloater_Func_2(k);
}

void MaridiaFloater_Func_10(uint16 k) {  // 0xA8C500
  Enemy_MaridiaFloater *E = Get_MaridiaFloater(k);
  bool v2 = E->mfr_var_E == 1;
  bool v3 = (--E->mfr_var_E & 0x8000) != 0;
  if (v2 || v3) {
    E->mfr_var_F = FUNC16(MaridiaFloater_Func_11);
    E->mfr_var_A = g_word_A8C1C1;
    E->mfr_var_B = g_word_A8C1C3;
  }
  MaridiaFloater_Func_2(k);
}

void MaridiaFloater_Func_11(uint16 k) {  // 0xA8C51D
  Enemy_MaridiaFloater *E = Get_MaridiaFloater(k);
  AddToHiLo(&E->mfr_var_A, &E->mfr_var_B, __PAIR32__(g_word_A8C1BD, g_word_A8C1BF));
  Enemy_MoveDown(k, __PAIR32__(E->mfr_var_A, E->mfr_var_B));
  int v4 = k >> 1;
  if ((int16)(E->base.y_pos - enemy_drawing_queue_sizes[v4 + 1]) >= 0) {
    E->base.y_pos = enemy_drawing_queue_sizes[v4 + 1];
    E->mfr_var_F = FUNC16(MaridiaFloater_Func_3);
    E->mfr_var_E = 60;
    E->base.instruction_timer = 1;
    E->base.current_instruction = addr_kMaridiaFloater_Ilist_C173;
  }
  MaridiaFloater_Func_2(k);
}

void MaridiaFloater_Func_12(uint16 k) {  // 0xA8C569
  int v1 = k >> 1;
  if (!sign16(enemy_drawing_queue[v1 + 103] + 0x3E6F)) {
    uint16 v2 = (uint16)(enemy_drawing_queue[v1 + 103] - 4 + 15983) >> 1;
    if (v2) {
      enemy_drawing_queue[v1 + 103] = g_off_A8C599[v2 >> 1];
      enemy_drawing_queue[v1 + 104] = 1;
    }
  }
  Enemy_MaridiaFloater *E = Get_MaridiaFloater(k);
  E->mfr_var_F = FUNC16(MaridiaFloater_Func_13);
  E->mfr_var_E = 32;
  MaridiaFloater_Func_2(k);
}

void MaridiaFloater_Func_13(uint16 k) {  // 0xA8C59F
  uint16 v5;

  Enemy_MaridiaFloater *E = Get_MaridiaFloater(k);
  bool v2 = E->mfr_var_E == 1;
  bool v3 = (--E->mfr_var_E & 0x8000) != 0;
  if (v2 || v3) {
    int v4 = k >> 1;
    enemy_drawing_queue[v4 + 100] = 0;
    enemy_drawing_queue[v4 + 97] |= 0x200;
    MaridiaFloater_Func_1(k);
    //printf("A undefined!\n");
    v5 = 0;
    EnemyDeathAnimation(k, v5);
  } else {
    MaridiaFloater_Func_2(k);
  }
}

void MaridiaFloater_Touch(void) {  // 0xA8C5BE
  Enemy_MaridiaFloater *E = Get_MaridiaFloater(cur_enemy_index);
  if (!E->mfr_parameter_2) {
    uint16 palette_index = E->base.palette_index;
    NormalEnemyTouchAi();
    if (!E->base.health) {
      enemy_drawing_queue[(cur_enemy_index >> 1) + 97] |= 0x200;
      E->base.palette_index = palette_index;
      MaridiaFloater_Func_1(cur_enemy_index);
      E->base.palette_index = 2560;
    }
  }
}

void MaridiaFloater_Shot(void) {  // 0xA8C5EF
  Enemy_MaridiaFloater *E = Get_MaridiaFloater(cur_enemy_index);
  if (!E->mfr_parameter_2) {
    NormalEnemyShotAiSkipDeathAnim_CurEnemy();
    if ((E->base.ai_handler_bits & 4) != 0) {
      int v2 = cur_enemy_index >> 1;
      enemy_drawing_queue[v2 + 109] = E->base.frozen_timer;
      enemy_drawing_queue[v2 + 99] |= 4;
    }
    if ((E->base.ai_handler_bits & 2) != 0) {
      int v3 = cur_enemy_index >> 1;
      enemy_drawing_queue[v3 + 108] = E->base.flash_timer;
      enemy_drawing_queue[v3 + 99] |= 2;
    }
    if (!E->base.health) {
      E->mfr_var_F = FUNC16(MaridiaFloater_Func_12);
      E->mfr_parameter_2 = 1;
    }
  }
}

void MaridiaFloater_Powerbomb(void) {  // 0xA8C63F
  NormalEnemyPowerBombAi();
  Enemy_MaridiaFloater *E = Get_MaridiaFloater(cur_enemy_index);
  if (E->base.health) {
    int v1 = cur_enemy_index >> 1;
    enemy_drawing_queue[v1 + 111] = E->base.shake_timer;
    enemy_drawing_queue[v1 + 110] = E->base.invincibility_timer;
    enemy_drawing_queue[v1 + 108] = E->base.flash_timer;
    enemy_drawing_queue[v1 + 109] = E->base.frozen_timer;
    enemy_drawing_queue[v1 + 99] = E->base.ai_handler_bits;
  } else {
    enemy_drawing_queue[(cur_enemy_index >> 1) + 97] |= 0x200;
  }
}

