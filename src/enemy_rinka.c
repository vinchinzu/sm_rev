// Rinka enemy runtime extracted from sm_a2.c.
#include "ida_types.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "variables.h"
#include "enemy_ai_canon.h"

#define g_word_A2B75B ((uint16*)RomFixedPtr(0xa2b75b))


void Rinka_Init(void) {  // 0xA2B602
  Enemy_Rinka *E = Get_Rinka(cur_enemy_index);
  if (E->rinka_parameter_1) {
    Rinka_1(cur_enemy_index);
    E->base.properties = E->base.properties & ~(kEnemyProps_RespawnIfKilled | kEnemyProps_ProcessInstructions | kEnemyProps_ProcessedOffscreen | kEnemyProps_Intangible) | 0x2C00;
  } else {
    E->base.properties = E->base.properties & ~(kEnemyProps_RespawnIfKilled | kEnemyProps_ProcessInstructions | kEnemyProps_ProcessedOffscreen | kEnemyProps_Intangible) | 0x6400;
  }
  E->base.palette_index = 1024;
  Rinka_Init3(cur_enemy_index);
}

void Rinka_Init2(uint16 k) {  // 0xA2B63E
  EnemySpawnData *v2;

  Enemy_Rinka *E = Get_Rinka(k);
  if (E->rinka_parameter_1)
    Rinka_1(k);
  v2 = gEnemySpawnData(k);
  E->base.x_pos = v2->x_pos;
  E->base.y_pos = v2->y_pos;
  Rinka_Init3(k);
}

void Rinka_Init3(uint16 k) {  // 0xA2B654
  Enemy_Rinka *E = Get_Rinka(k);
  E->rinka_var_A = FUNC16(Rinka_5);
  E->rinka_var_F = 26;
  E->rinka_var_B = 0;
  E->rinka_var_C = 0;
  if (E->rinka_parameter_1) {
    if (Get_Rinka(0)->rinka_var_1D) {
      E->base.properties |= kEnemyProps_Deleted;
    } else {
      E->base.current_instruction = addr_kRinka_Ilist_BA0C;
      E->base.instruction_timer = 1;
      E->base.timer = 0;
    }
  } else {
    E->base.current_instruction = addr_kRinka_Ilist_B9E0;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
  }
}

void Rinka_1(uint16 k) {  // 0xA2B69B
  uint16 v2;
  EnemySpawnData *ES = gEnemySpawnData(k);
  if (Rinka_10(ES->x_pos, ES->y_pos) || (v2 = Rinka_2(k), gRam8000_Default(v2)[31].var_3F & 1)) {
    uint16 v3 = 0;
    do {
      int v4 = v3 >> 1;
      uint16 x = g_word_A2B75B[v4];
      uint16 y = g_word_A2B75B[v4 + 1];
      if (!Rinka_10(x, y) && !(gRam8000_Default(g_word_A2B75B[v4 + 2])[31].var_3F & 1)) {
        Enemy_Rinka *E = Get_Rinka(k);
        ES->x_pos = x;
        E->base.x_pos = x;
        ES->y_pos = y;
        E->base.y_pos = y;
        uint16 v9 = g_word_A2B75B[(v3 >> 1) + 2];
        gRam8000_Default(v9)[31].var_3F = -1;
        E->rinka_var_D = v9;
        return;
      }
      v3 += 6;
    } while ((int16)(v3 - 66) < 0);
    uint16 v10 = 0;
    while (gRam8000_Default(g_word_A2B75B[(v10 >> 1) + 2])[31].var_3F & 1) {
      v10 += 6;
      if ((int16)(v10 - 66) >= 0)
        return;
    }
    int v11 = v10 >> 1;
    uint16 v12 = g_word_A2B75B[v11];
    ES->x_pos = v12;
    Enemy_Rinka *E = Get_Rinka(k);
    E->base.x_pos = v12;
    uint16 v15 = g_word_A2B75B[v11 + 1];
    ES->y_pos = v15;
    E->base.y_pos = v15;
    uint16 v16 = g_word_A2B75B[v11 + 2];
    E->rinka_var_D = v16;
    gRam8000_Default(v16)[31].var_3F = -1;
  } else {
    Get_Rinka(k)->rinka_var_D = v2;
    gRam8000_Default(v2)[31].var_3F = -1;
  }
}

uint16 Rinka_2(uint16 k) {  // 0xA2B79D
  EnemySpawnData *v3;

  uint16 v1 = 0;
  while (1) {
    int v2 = v1 >> 1;
    v3 = gEnemySpawnData(k);
    if (g_word_A2B75B[v2] == v3->x_pos && g_word_A2B75B[v2 + 1] == v3->y_pos)
      break;
    v1 += 6;
    if (!sign16(v1 - 66)) {
      v1 = 0;
      return g_word_A2B75B[(v1 >> 1) + 2];
    }
  }
  return g_word_A2B75B[(v1 >> 1) + 2];
}

void CallRinkaFunc(uint32 ea, uint16 k) {
  switch (ea) {
  case fnRinka_3: Rinka_3(k); return;
  case fnRinka_4: Rinka_4(k); return;
  case fnRinka_5: Rinka_5(k); return;
  case fnRinka_B85B: Rinka_B85B(k); return;
  default: Unreachable();
  }
}

void Rinka_Main(void) {  // 0xA2B7C4
  uint16 v2;

  Enemy_Rinka *E = Get_Rinka(cur_enemy_index);
  if (E->rinka_parameter_1 && Get_Rinka(0)->rinka_var_1D) {
    Rinka_6(cur_enemy_index);
    Rinka_8(cur_enemy_index);
    //printf("A unknown\n");
    v2 = 0;
    RinkasDeathAnimation(v2);
  } else {
    CallRinkaFunc(E->rinka_var_A | 0xA20000, cur_enemy_index);
  }
}

void Rinka_3(uint16 k) {  // 0xA2B7DF
  Enemy_Rinka *E = Get_Rinka(k);
  if ((--E->rinka_var_F & 0x8000) != 0) {
    E->rinka_var_A = FUNC16(Rinka_B85B);
    uint16 v3;
    if (E->rinka_parameter_1)
      v3 = E->base.properties & ~kEnemyProps_Intangible;
    else
      v3 = E->base.properties & 0xF3FF | 0x800;
    E->base.properties = v3;
    uint16 r18 = (uint8)-(CalculateAngleFromXY(samus_x_pos - E->base.x_pos, samus_y_pos - E->base.y_pos) + 0x80);
    E->rinka_var_B = Math_MultBySin(0x120, r18);
    E->rinka_var_C = Math_MultByCos(0x120, r18);
  }
}

void Rinka_4(uint16 k) {  // 0xA2B844
  Enemy_Rinka *E = Get_Rinka(k);
  if ((--E->rinka_var_F & 0x8000) != 0) {
    E->base.health = 10;
    Rinka_Init2(k);
  }
}

void Rinka_5(uint16 k) {  // 0xA2B852
  if (Rinka_9(k) & 1)
    Rinka_B865(k);
}

void Rinka_B85B(uint16 k) {  // 0xA2B85B
  MoveEnemyWithVelocity();
  if (Rinka_9(k) & 1)
    Rinka_B865(k);
}

void Rinka_B865(uint16 k) {  // 0xA2B865
  if (Get_Rinka(k)->rinka_parameter_1 && (Rinka_8(k), Get_Rinka(0)->rinka_var_1D)) {
    Rinka_6(k);
    DeleteEnemyAndConnectedEnemies();
  } else {
    Rinka_6(k);
    Rinka_Init2(k);
  }
}

void Rinka_6(uint16 k) {  // 0xA2B880
  int16 v3;

  Enemy_Rinka *E = Get_Rinka(k);
  if (E->rinka_parameter_1 && (E->base.properties & kEnemyProps_Invisible) == 0) {
    Enemy_Rinka *E0 = Get_Rinka(0);
    v3 = E0->rinka_var_1E - 1;
    if (v3 < 0)
      v3 = 0;
    E0->rinka_var_1E = v3;
  }
}

void Rinka_7(uint16 k) {  // 0xA2B89C
  Enemy_Rinka *E = Get_Rinka(k);
  if ((random_enemy_counter & 3) == E->rinka_parameter_1)
    E->base.properties &= ~kEnemyProps_Intangible;
  else
    E->base.properties |= kEnemyProps_Intangible;
}

void Rinka_8(uint16 k) {  // 0xA2B8BB
  Enemy_Rinka *E = Get_Rinka(k);
  if (E->rinka_parameter_1) {
    uint16 rinka_var_D = E->rinka_var_D;
    if (rinka_var_D) {
      gRam8000_Default(rinka_var_D)[31].var_3F = 0;
      E->rinka_var_D = 0;
    }
  }
}

uint8 Rinka_9(uint16 k) {  // 0xA2B8D3
  int16 y_pos;
  int16 v3;
  int16 x_pos;
  int16 v5;

  Enemy_Rinka *E = Get_Rinka(k);
  y_pos = E->base.y_pos;
  uint8 result = 1;
  if (y_pos >= 0) {
    v3 = y_pos + 16 - layer1_y_pos;
    if (v3 >= 0) {
      if (sign16(v3 - 256)) {
        x_pos = E->base.x_pos;
        if (x_pos >= 0) {
          v5 = x_pos + 16 - layer1_x_pos;
          if (v5 >= 0) {
            if (sign16(v5 - 288))
              return 0;
          }
        }
      }
    }
  }
  return result;
}

bool Rinka_10(uint16 r18, uint16 r20) {  // 0xA2B8FF
  return (r20 & 0x8000) != 0 || (int16)(r20 - layer1_y_pos) < 0 || !sign16(r20 - layer1_y_pos - 224)
       || (r20 & 0x8000) != 0 || (int16)(r18 - layer1_x_pos) < 0 || !sign16(r18 - layer1_x_pos - 256);
}

void Rinka_Frozen(uint16 k) {  // 0xA2B929
  if (Rinka_9(k) & 1)
    Get_Rinka(k)->base.frozen_timer = 0;
  NormalEnemyFrozenAI();
  if (Get_Rinka(0)->rinka_var_1D) {
    Rinka_6(k);
    Rinka_8(k);
//    printf("A undefined!\n");
    uint16 v1 = 0;
    RinkasDeathAnimation(v1);
  }
}

void Rinka_Touch(void) {  // 0xA2B947
  NormalEnemyTouchAiSkipDeathAnim_CurEnemy();
  Rinka_B960(cur_enemy_index);
}

void Rinka_Shot(void) {  // 0xA2B94D
  NormalEnemyShotAiSkipDeathAnim_CurEnemy();
  Rinka_B960(cur_enemy_index);
}

void Rinka_Powerbomb(uint16 k) {  // 0xA2B953
  if ((Get_Rinka(k)->base.properties & kEnemyProps_Invisible) == 0) {
    NormalEnemyPowerBombAiSkipDeathAnim_CurEnemy();
    Rinka_B960(k);
  }
}

void Rinka_B960(uint16 k) {  // 0xA2B960
  Enemy_Rinka *E = Get_Rinka(k);
  if (!E->base.health) {
    Rinka_6(k);
    Rinka_8(k);
    if (E->rinka_parameter_1) {
      E->base.properties |= kEnemyProps_Intangible | kEnemyProps_Invisible;
      eproj_spawn_pt = (Point16U){ E->base.x_pos, E->base.y_pos };
      SpawnEprojWithRoomGfx(0xE509, 3);
      E->rinka_var_A = FUNC16(Rinka_4);
      E->rinka_var_F = 1;
    } else {
      RinkasDeathAnimation(0);
    }
  }
}

uint16 Rinka_Instr_B9A2(uint16 k, uint16 j) {  // 0xA2B9A2
  Enemy_Rinka *E = Get_Rinka(0);
  if (sign16(E->rinka_var_1E - 3))
    return j + 2;
  else
    return *(uint16 *)RomPtr_A2(j);
}

const uint16 *Rinka_Instr_B9B3(uint16 k, const uint16 *jp) {  // 0xA2B9B3
  Enemy_Rinka *E = Get_Rinka(k);
  E->base.properties |= kEnemyProps_Intangible | kEnemyProps_Invisible;
  return jp;
}

const uint16 *Rinka_Instr_B9BD(uint16 k, const uint16 *jp) {  // 0xA2B9BD
  Enemy_Rinka *E = Get_Rinka(k);
  E->base.properties |= kEnemyProps_ProcessedOffscreen | kEnemyProps_Intangible | kEnemyProps_Invisible;
  return jp;
}

const uint16 *Rinka_Instr_B9C7(uint16 k, const uint16 *jp) {  // 0xA2B9C7
  Enemy_Rinka *E = Get_Rinka(k);
  E->base.properties &= ~(kEnemyProps_Intangible | kEnemyProps_Invisible);
  E->rinka_var_A = FUNC16(Rinka_3);
  Enemy_Rinka *E0 = Get_Rinka(0);
  ++E0->rinka_var_1E;
  return jp;
}
