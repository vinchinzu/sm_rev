// Enemy AI - Metalee — peeled from Bank $A3
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

#define g_off_A3894E ((uint16*)RomFixedPtr(0xa3894e))

const uint16 *Metalee_Instr_1(uint16 k, const uint16 *jp) {  // 0xA38956
  Get_Metalee(cur_enemy_index)->metalee_var_E = 1;
  return jp;
}

void Metalee_Init(void) {  // 0xA38960
  Enemy_Metalee *E = Get_Metalee(cur_enemy_index);
  E->metalee_var_C = 0;
  E->metalee_var_D = 0;
  E->metalee_var_E = 0;
  E->base.current_instruction = addr_kMetalee_Ilist_8910;
  E->metalee_var_B = FUNC16(Metalee_Func_1);
}

void CallMetaleeFunc(uint32 ea) {
  switch (ea) {
  case fnMetalee_Func_1: Metalee_Func_1(); return;
  case fnMetalee_Func_3: Metalee_Func_3(cur_enemy_index); return;
  case fnMetalee_Func_4: Metalee_Func_4(); return;
  case fnMetalee_Func_5: Metalee_Func_5(); return;
  default: Unreachable();
  }
}

void Metalee_Main(void) {  // 0xA38979
  Enemy_Metalee *E = Get_Metalee(cur_enemy_index);
  CallMetaleeFunc(E->metalee_var_B | 0xA30000);
}

void Metalee_Func_1(void) {  // 0xA38987
  Enemy_Metalee *E = Get_Metalee(cur_enemy_index);
  if (abs16(E->base.x_pos - samus_x_pos) < 0x48) {
    Metalee_Func_2(cur_enemy_index);
    ++E->metalee_var_C;
    Metalee_Func_6();
    E->metalee_var_B = FUNC16(Metalee_Func_3);
  }
}

void Metalee_Func_2(uint16 k) {  // 0xA389AC
  Enemy_Metalee *E = Get_Metalee(k);
  uint16 div = (uint8)SnesDivide(samus_y_pos - E->base.y_pos, 24);
  E->metalee_var_F = div + 4;
}

void Metalee_Func_3(uint16 k) {  // 0xA389D4
  Enemy_Metalee *E = Get_Metalee(k);
  if (E->metalee_var_E) {
    E->metalee_var_E = 0;
    ++E->metalee_var_C;
    Metalee_Func_6();
    E->metalee_var_B = FUNC16(Metalee_Func_4);
    QueueSfx2_Max6(0x5B);
  }
}

void Metalee_Func_4(void) {  // 0xA389F3
  Enemy_Metalee *E = Get_Metalee(cur_enemy_index);
  E->metalee_var_A = 21;
  uint16 v1 = E->base.properties | 3;
  E->base.properties = v1;
  if (EnemyFunc_BF8A(cur_enemy_index, v1, INT16_SHL16(E->metalee_var_F)) & 1) {
    E->base.instruction_timer = 1;
    E->base.timer = 0;
    E->metalee_var_B = FUNC16(Metalee_Func_5);
    QueueSfx2_Max6(0x5C);
  } else {
    E->base.y_pos += E->metalee_var_F;
    E->base.x_pos += ((int16)(E->base.x_pos - samus_x_pos) >= 0) ? -2 : 2;
  }
}

void Metalee_Func_5(void) {  // 0xA38A5C
  Enemy_Metalee *E = Get_Metalee(cur_enemy_index);
  if (E->metalee_var_A-- == 1) {
    gEnemySpawnData(cur_enemy_index)->vram_tiles_index = E->base.vram_tiles_index | E->base.palette_index;
    E->base.palette_index = 2560;
    E->base.vram_tiles_index = 0;
    E->base.properties |= kEnemyProps_Deleted;
  } else {
    if (E->metalee_var_A == 8) {
      uint16 v2 = cur_enemy_index;
      SpawnEprojWithGfx(0, v2, addr_stru_868BFA);
      SpawnEprojWithGfx(0, v2, addr_stru_868C08);
      SpawnEprojWithGfx(0, v2, addr_stru_868C16);
      SpawnEprojWithGfx(0, v2, addr_stru_868C24);
    }
    ++E->base.y_pos;
  }
}

void Metalee_Func_6(void) {  // 0xA38AB2
  Enemy_Metalee *E = Get_Metalee(cur_enemy_index);
  uint16 metalee_var_C = E->metalee_var_C;
  if (metalee_var_C != E->metalee_var_D) {
    E->metalee_var_D = metalee_var_C;
    E->base.current_instruction = g_off_A3894E[metalee_var_C];
    E->base.instruction_timer = 1;
    E->base.timer = 0;
  }
}

void Metalee_Shot(void) {  // 0xA38B0F
  Enemy_Metalee *E = Get_Metalee(cur_enemy_index);
  uint16 varE2A = E->base.vram_tiles_index;
  uint16 varE2C = E->base.palette_index;
  NormalEnemyShotAi();
  if (!Get_Metalee(cur_enemy_index)->base.health) {
    E->base.vram_tiles_index = varE2A;
    E->base.palette_index = varE2C;
    uint16 v2 = cur_enemy_index;
    SpawnEprojWithGfx(E->metalee_var_A, cur_enemy_index, addr_stru_868BFA);
    SpawnEprojWithGfx(0, v2, addr_stru_868C08);
    SpawnEprojWithGfx(0, v2, addr_stru_868C16);
    SpawnEprojWithGfx(0, v2, addr_stru_868C24);
    E->base.vram_tiles_index = 0;
    E->base.palette_index = 0;
  }
}
