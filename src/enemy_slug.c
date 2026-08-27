// Enemy AI - Slug, Crab, NorfairSlowFireball, BigEyeBugs — peeled from Bank $A3
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

#define g_off_A396DB ((uint16*)RomFixedPtr(0xa396db))
#define g_off_A3992B ((uint16*)RomFixedPtr(0xa3992b))
#define g_off_A3B667 ((uint16*)RomFixedPtr(0xa3b667))
#define g_off_A3E2CC ((uint16*)RomFixedPtr(0xa3e2cc))

void Crab_Init(void) {  // 0xA396E3
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  v1->parameter_2 = 8;
  v1->current_instruction = g_off_A396DB[v1->current_instruction & 3];
  StoneZoomer_E67A(cur_enemy_index);
}

void Crab_Func_1(void) {  // 0xA396FD
  ;
}

void Slug_Init(void) {  // 0xA3993B
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  v1->parameter_2 = 10;
  v1->current_instruction = g_off_A3992B[v1->current_instruction & 3];
  StoneZoomer_E67A(cur_enemy_index);
}

void Slug_Func_1(void) {  // 0xA39955
  ;
}

void NorfairSlowFireball_Init(void) {  // 0xA3B66F
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  v1->parameter_2 = 6;
  v1->current_instruction = g_off_A3B667[v1->properties & 3];
  StoneZoomer_E67A(cur_enemy_index);
}

void NorfairSlowFireball_Func_1(void) {  // 0xA3B6F9
  uint16 current_instruction = gEnemyData(cur_enemy_index)->current_instruction;
  if (current_instruction)
    Unreachable();
}

void BigEyeBugs_Init(void) {  // 0xA3E2D4
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  v1->current_instruction = g_off_A3E2CC[v1->current_instruction & 3];
  StoneZoomer_E67A(cur_enemy_index);
}
