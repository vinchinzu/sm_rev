// Enemy AI - Skree — peeled from Bank $A3
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

#define g_off_A3C69C ((uint16*)RomFixedPtr(0xa3c69c))

const uint16 *Skree_Instr_1(uint16 k, const uint16 *jp) {  // 0xA3C6A4
  Get_Skree(cur_enemy_index)->skree_var_E = 1;
  return jp;
}

void Skree_Init(void) {  // 0xA3C6AE
  Enemy_Skree *E = Get_Skree(cur_enemy_index);
  E->skree_var_C = 0;
  E->skree_var_D = 0;
  E->skree_var_E = 0;
  E->base.current_instruction = addr_kSkree_Ilist_C65E;
  E->skree_var_B = FUNC16(Skree_Func_1);
}

void CallSkreeFunc(uint32 ea) {
  switch (ea) {
  case fnSkree_Func_1: Skree_Func_1(); return;
  case fnSkree_Func_2: Skree_Func_2(cur_enemy_index); return;
  case fnSkree_Func_3: Skree_Func_3(); return;
  case fnSkree_Func_4: Skree_Func_4(); return;
  default: Unreachable();
  }
}
void Skree_Main(void) {  // 0xA3C6C7
  Enemy_Skree *E = Get_Skree(cur_enemy_index);
  CallSkreeFunc(E->skree_var_B | 0xA30000);
}

void Skree_Func_1(void) {  // 0xA3C6D5
  Enemy_Skree *E = Get_Skree(cur_enemy_index);
  if (abs16(E->base.x_pos - samus_x_pos) < 0x30) {
    ++E->skree_var_C;
    Skree_Func_5();
    E->skree_var_B = FUNC16(Skree_Func_2);
  }
}

void Skree_Func_2(uint16 k) {  // 0xA3C6F7
  Enemy_Skree *E = Get_Skree(k);
  if (E->skree_var_E) {
    E->skree_var_E = 0;
    ++E->skree_var_C;
    Skree_Func_5();
    E->skree_var_B = FUNC16(Skree_Func_3);
    QueueSfx2_Max6(0x5B);
  }
}

void Skree_Func_3(void) {  // 0xA3C716
  Enemy_Skree *E = Get_Skree(cur_enemy_index);
  E->skree_var_A = 21;
  uint16 v1 = E->base.properties | 3;
  E->base.properties = v1;
  if (EnemyFunc_BF8A(cur_enemy_index, v1, INT16_SHL16(6)) & 1) {
    E->base.instruction_timer = 1;
    E->base.timer = 0;
    E->skree_var_B = FUNC16(Skree_Func_4);
    QueueSfx2_Max6(0x5C);
  } else {
    E->base.y_pos += 6;
    E->base.x_pos += ((int16)(E->base.x_pos - samus_x_pos) >= 0) ? -1 : 1;
  }
}

void Skree_Func_4(void) {  // 0xA3C77F
  Enemy_Skree *E = Get_Skree(cur_enemy_index);
  if (E->skree_var_A-- == 1) {
    gEnemySpawnData(cur_enemy_index)->vram_tiles_index = E->base.vram_tiles_index | E->base.palette_index;
    E->base.palette_index = 2560;
    E->base.vram_tiles_index = 0;
    E->base.properties |= kEnemyProps_Deleted;
  } else {
    if (E->skree_var_A == 8) {
      uint16 v2 = cur_enemy_index;
      SpawnEprojWithGfx(8, cur_enemy_index, addr_stru_868BC2);
      SpawnEprojWithGfx(0, v2, addr_stru_868BD0);
      SpawnEprojWithGfx(0, v2, addr_stru_868BDE);
      SpawnEprojWithGfx(0, v2, addr_stru_868BEC);
    }
    ++E->base.y_pos;
  }
}

void Skree_Func_5(void) {  // 0xA3C7D5
  Enemy_Skree *E = Get_Skree(cur_enemy_index);
  uint16 skree_var_C = E->skree_var_C;
  if (skree_var_C != E->skree_var_D) {
    E->skree_var_D = skree_var_C;
    E->base.current_instruction = g_off_A3C69C[skree_var_C];
    E->base.instruction_timer = 1;
    E->base.timer = 0;
  }
}

void Skree_Shot(void) {  // 0xA3C7F5
  NormalEnemyShotAiSkipDeathAnim_CurEnemy();
  Enemy_Skree *E = Get_Skree(cur_enemy_index);
  if (!E->base.health) {
    uint16 v1 = cur_enemy_index;
    SpawnEprojWithGfx(E->skree_var_A, cur_enemy_index, addr_stru_868BC2);
    SpawnEprojWithGfx(0, v1, addr_stru_868BD0);
    SpawnEprojWithGfx(0, v1, addr_stru_868BDE);
    SpawnEprojWithGfx(0, v1, addr_stru_868BEC);
    uint16 v5 = 2;
    if ((projectile_type[collision_detection_index] & 0xF00) != 512)
      v5 = 0;
    EnemyDeathAnimation(2 * collision_detection_index, v5);
  }
}
