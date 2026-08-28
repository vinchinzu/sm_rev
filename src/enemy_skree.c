// Enemy AI - Skree — peeled from Bank $A3
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

enum {
  kSkreeDetectX = 0x30,
  kSkreeFallYSpeed = 6,
  kSkreeBurrowTimer = 21,
  kSkreeParticleTimer = 8,
  kSkreeBurrowPalette = 2560,
  kSfx2_SkreeLaunch = 0x5B,
  kSfx2_SkreeHitGround = 0x5C,
};

static const uint16 kSkreeIlists[4] = {
  addr_kSkree_Ilist_C65E,
  addr_kSkree_Ilist_C672,
  addr_kSkree_Ilist_C67E,
  addr_kSkree_Ilist_C694,
};

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
  if (abs16(E->base.x_pos - samus_x_pos) < kSkreeDetectX) {
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
    QueueSfx2_Max6(kSfx2_SkreeLaunch);
  }
}

void Skree_Func_3(void) {  // 0xA3C716
  Enemy_Skree *E = Get_Skree(cur_enemy_index);
  E->skree_var_A = kSkreeBurrowTimer;
  uint16 props = E->base.properties | 3;
  E->base.properties = props;
  if (EnemyFunc_BF8A(cur_enemy_index, props, INT16_SHL16(kSkreeFallYSpeed)) & 1) {
    E->base.instruction_timer = 1;
    E->base.timer = 0;
    E->skree_var_B = FUNC16(Skree_Func_4);
    QueueSfx2_Max6(kSfx2_SkreeHitGround);
  } else {
    E->base.y_pos += kSkreeFallYSpeed;
    E->base.x_pos += ((int16)(E->base.x_pos - samus_x_pos) >= 0) ? -1 : 1;
  }
}

void Skree_Func_4(void) {  // 0xA3C77F
  Enemy_Skree *E = Get_Skree(cur_enemy_index);
  if (E->skree_var_A-- == 1) {
    gEnemySpawnData(cur_enemy_index)->vram_tiles_index = E->base.vram_tiles_index | E->base.palette_index;
    E->base.palette_index = kSkreeBurrowPalette;
    E->base.vram_tiles_index = 0;
    E->base.properties |= kEnemyProps_Deleted;
  } else {
    if (E->skree_var_A == kSkreeParticleTimer) {
      uint16 idx = cur_enemy_index;
      SpawnEprojWithGfx(8, cur_enemy_index, addr_stru_868BC2);
      SpawnEprojWithGfx(0, idx, addr_stru_868BD0);
      SpawnEprojWithGfx(0, idx, addr_stru_868BDE);
      SpawnEprojWithGfx(0, idx, addr_stru_868BEC);
    }
    ++E->base.y_pos;
  }
}

void Skree_Func_5(void) {  // 0xA3C7D5
  Enemy_Skree *E = Get_Skree(cur_enemy_index);
  uint16 ilist_idx = E->skree_var_C;
  if (ilist_idx != E->skree_var_D) {
    E->skree_var_D = ilist_idx;
    E->base.current_instruction = kSkreeIlists[ilist_idx];
    E->base.instruction_timer = 1;
    E->base.timer = 0;
  }
}

void Skree_Shot(void) {  // 0xA3C7F5
  NormalEnemyShotAiSkipDeathAnim_CurEnemy();
  Enemy_Skree *E = Get_Skree(cur_enemy_index);
  if (!E->base.health) {
    uint16 idx = cur_enemy_index;
    SpawnEprojWithGfx(E->skree_var_A, cur_enemy_index, addr_stru_868BC2);
    SpawnEprojWithGfx(0, idx, addr_stru_868BD0);
    SpawnEprojWithGfx(0, idx, addr_stru_868BDE);
    SpawnEprojWithGfx(0, idx, addr_stru_868BEC);
    uint16 explosion = 2;
    if ((projectile_type[collision_detection_index] & kProjectileType_TypeMask) != kProjectileType_SuperMissile)
      explosion = 0;
    EnemyDeathAnimation(2 * collision_detection_index, explosion);
  }
}
