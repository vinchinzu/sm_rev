// Enemy AI - Zebetites — peeled from Bank $A6
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"

#define g_word_A6FC03 ((uint16*)RomFixedPtr(0xa6fc03))
#define g_word_A6FC0B ((uint16*)RomFixedPtr(0xa6fc0b))
#define g_off_A6FC13 ((uint16*)RomFixedPtr(0xa6fc13))
#define g_word_A6FC1B ((uint16*)RomFixedPtr(0xa6fc1b))
#define g_word_A6FC23 ((uint16*)RomFixedPtr(0xa6fc23))
#define g_word_A6FC2B ((uint16*)RomFixedPtr(0xa6fc2b))
#define g_off_A6FD4A ((uint16*)RomFixedPtr(0xa6fd4a))
#define g_off_A6FD54 ((uint16*)RomFixedPtr(0xa6fd54))

void Zebetites_Init(void) {  // 0xA6FB72
  Enemy_Zebetites *E = Get_Zebetites(cur_enemy_index);
  E->base.properties |= 0xA000;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->base.palette_index = 1024;
  E->base.vram_tiles_index = 128;
  E->zebet_var_C = 0;
  uint16 v1 = FUNC16(Zebetites_Func_1);
  if (E->zebet_parameter_1)
    v1 = FUNC16(Zebetites_Func_2);
  E->zebet_var_A = v1;
  uint16 r18 = 0;
  r18 = (r18 << 1) | CheckEventHappened(5);
  r18 = (r18 << 1) | CheckEventHappened(4);
  r18 = (r18 << 1) | CheckEventHappened(3);
  uint16 v2 = r18;
  E->zebet_var_D = r18;
  if (sign16(v2 - 4)) {
    int v3 = v2;
    E->zebet_var_F = g_word_A6FC03[v3];
    E->base.y_height = g_word_A6FC0B[v3];
    E->base.current_instruction = g_off_A6FC13[v3];
    E->base.x_pos = g_word_A6FC1B[v3];
    uint16 v4;
    if (E->zebet_parameter_1)
      v4 = g_word_A6FC2B[v3];
    else
      v4 = g_word_A6FC23[v3];
    E->base.y_pos = v4;
  } else {
    E->base.properties |= kEnemyProps_Deleted;
  }
}

void CallZebetitesFunc(uint32 ea, uint16 k) {
  switch (ea) {
  case fnZebetites_Func_1: Zebetites_Func_1(k); return;
  case fnZebetites_Func_2: Zebetites_Func_2(k); return;
  case fnZebetites_Func_3: Zebetites_Func_3(); return;
  default: Unreachable();
  }
}

void Zebetites_Main(void) {  // 0xA6FC33
  if (!earthquake_timer)
    Get_Zebetites(cur_enemy_index)->base.shake_timer = 0;
  Enemy_Zebetites *E = Get_Zebetites(cur_enemy_index);
  CallZebetitesFunc(E->zebet_var_A | 0xA60000, cur_enemy_index);
}

void Zebetites_Func_1(uint16 k) {  // 0xA6FC41
  Enemy_Zebetites *E = Get_Zebetites(k);
  if ((E->zebet_var_F & 0x8000) != 0) {
    uint16 new_k = Zebetites_Func_9();
    Get_Zebetites(new_k)->zebet_parameter_2 = k;
    Get_Zebetites(k)->zebet_parameter_2 = new_k;
  }
  Get_Zebetites(k)->zebet_var_A = FUNC16(Zebetites_Func_2);
  Zebetites_Func_2(k);
}

void Zebetites_Func_2(uint16 k) {  // 0xA6FC5B
  if (!door_transition_flag_elevator_zebetites) {
    Get_Zebetites(k)->zebet_var_A = FUNC16(Zebetites_Func_3);
    Zebetites_Func_3();
  }
}

void Zebetites_Func_3(void) {  // 0xA6FC67
  int16 v3;

  Zebetites_Func_4();
  Zebetites_Func_5();
  Enemy_Zebetites *E = Get_Zebetites(cur_enemy_index);
  uint16 health = E->base.health;
  if (health) {
    v3 = health + 1;
    if (!sign16(v3 - 1000))
      v3 = 1000;
    E->base.health = v3;
  } else if (E->zebet_parameter_1
             || (Zebetites_Func_6(), !sign16(E->zebet_var_D - 4))) {
    EnemyDeathAnimation(cur_enemy_index, 0);
  } else {
    EnemyDeathAnimation(cur_enemy_index, 0);
    Zebetites_Func_8();
  }
}

void Zebetites_Func_6(void) {  // 0xA6FCAA
  int16 v1;

  Enemy_Zebetites *E = Get_Zebetites(cur_enemy_index);
  v1 = E->zebet_var_D + 1;
  E->zebet_var_D = v1;
  uint16 r18 = v1;
  Zebetites_Func_7(3, (r18 >> 0) & 1);
  Zebetites_Func_7(4, (r18 >> 1) & 1);
  Zebetites_Func_7(5, (r18 >> 2) & 1);
}

void Zebetites_Func_7(uint16 j, uint8 a) {  // 0xA6FCCB
  if (a)
    SetEventHappened(j);
  else
    ClearEventHappened(j);
}

void Zebetites_Func_8(void) {  // 0xA6FCD9
  SpawnEnemy(0xA6, addr_stru_A6FCE1);
}

uint16 Zebetites_Func_9(void) {  // 0xA6FCF1
  return SpawnEnemy(0xA6, addr_stru_A6FCF9);
}

void Zebetites_Func_5(void) {  // 0xA6FD09
  uint16 v0 = 0;
  Enemy_Zebetites *E = Get_Zebetites(cur_enemy_index);
  uint16 health = E->base.health;
  if (sign16(health - 800)) {
    v0 = 2;
    if (sign16(health - 600)) {
      v0 = 4;
      if (sign16(health - 400)) {
        v0 = 6;
        if (sign16(health - 200))
          v0 = 8;
      }
    }
  }
  int v3 = v0 >> 1;
  uint16 v4 = g_off_A6FD4A[v3];
  if ((E->zebet_var_F & 0x8000) != 0)
    v4 = g_off_A6FD54[v3];
  E->base.current_instruction = v4;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
}

void Zebetites_Func_4(void) {  // 0xA6FD5E
  if (!palette_change_num && !Get_Zebetites(cur_enemy_index)->zebet_parameter_1) {
    Enemy_Zebetites *E = Get_Zebetites(0);
    uint16 v1 = (E->zebet_var_C + 1) & 7;
    E->zebet_var_C = v1;
    WriteColorsToPalette(0x158, 0xa6, 4 * v1 - 0x279, 2);
  }
}

void Zebetites_Touch(void) {  // 0xA6FDA7
  NormalEnemyTouchAiSkipDeathAnim_CurEnemy();
}

void Zebetites_Shot(void) {  // 0xA6FDAC
  QueueSfx3_Max6(9);
  NormalEnemyShotAiSkipDeathAnim_CurEnemy();
  Enemy_Zebetites *E = Get_Zebetites(cur_enemy_index);
  Enemy_Zebetites *G = Get_Zebetites(E->zebet_parameter_2);
  G->base.health = E->base.health;
  G->base.flash_timer = E->base.flash_timer;
}
