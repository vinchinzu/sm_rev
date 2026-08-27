// Enemy AI - Slug, Crab, NorfairSlowFireball, BigEyeBugs — peeled from Bank $A3
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

enum {
  kCrabCrawlyIlistIndex = 8,
  kSlugCrawlyIlistIndex = 10,
  kNorfairSlowFireballCrawlyIlistIndex = 6,
  kCreepyCrawlyInitDirMask = 3,
};

static const uint16 kCrabInitIlists[4] = {
  addr_kCrab_Ilist_967B,
  addr_kCrab_Ilist_9693,
  addr_kCrab_Ilist_96AB,
  addr_kCrab_Ilist_96C3,
};
static const uint16 kSlugInitIlists[4] = {
  addr_kSlug_Ilist_984B,
  addr_kSlug_Ilist_988B,
  addr_kSlug_Ilist_98AB,
  addr_kSlug_Ilist_990B,
};
static const uint16 kNorfairSlowFireballInitIlists[4] = {
  addr_kNorfairSlowFireball_Ilist_B5E3,
  addr_kNorfairSlowFireball_Ilist_B5EB,
  addr_kNorfairSlowFireball_Ilist_B5D3,
  addr_kNorfairSlowFireball_Ilist_B5DB,
};
static const uint16 kBigEyeBugsInitIlists[4] = {
  addr_kBigEyeBugs_Ilist_E25C,
  addr_kBigEyeBugs_Ilist_E278,
  addr_kBigEyeBugs_Ilist_E294,
  addr_kBigEyeBugs_Ilist_E2B0,
};

void Crab_Init(void) {  // 0xA396E3
  EnemyData *E = gEnemyData(cur_enemy_index);
  E->parameter_2 = kCrabCrawlyIlistIndex;
  E->current_instruction = kCrabInitIlists[E->current_instruction & kCreepyCrawlyInitDirMask];
  StoneZoomer_E67A(cur_enemy_index);
}

void Crab_Func_1(void) {  // 0xA396FD
  ;
}

void Slug_Init(void) {  // 0xA3993B
  EnemyData *E = gEnemyData(cur_enemy_index);
  E->parameter_2 = kSlugCrawlyIlistIndex;
  E->current_instruction = kSlugInitIlists[E->current_instruction & kCreepyCrawlyInitDirMask];
  StoneZoomer_E67A(cur_enemy_index);
}

void Slug_Func_1(void) {  // 0xA39955
  ;
}

void NorfairSlowFireball_Init(void) {  // 0xA3B66F
  EnemyData *E = gEnemyData(cur_enemy_index);
  E->parameter_2 = kNorfairSlowFireballCrawlyIlistIndex;
  E->current_instruction = kNorfairSlowFireballInitIlists[E->properties & kCreepyCrawlyInitDirMask];
  StoneZoomer_E67A(cur_enemy_index);
}

void NorfairSlowFireball_Func_1(void) {  // 0xA3B6F9
  uint16 current_instruction = gEnemyData(cur_enemy_index)->current_instruction;
  if (current_instruction)
    Unreachable();
}

void BigEyeBugs_Init(void) {  // 0xA3E2D4
  EnemyData *E = gEnemyData(cur_enemy_index);
  E->current_instruction = kBigEyeBugsInitIlists[E->current_instruction & kCreepyCrawlyInitDirMask];
  StoneZoomer_E67A(cur_enemy_index);
}
