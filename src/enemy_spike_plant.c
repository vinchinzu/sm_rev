// SpikeShootingPlant extracted from sm_a2.c.
#include "ida_types.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "variables.h"
#include "enemy_ai_canon.h"

#define g_word_A29F36 ((uint16*)RomFixedPtr(0xa29f36))
#define g_off_A29F42 ((uint16*)RomFixedPtr(0xa29f42))


const uint16 *SpikeShootingPlant_Instr_9F2A(uint16 k, const uint16 *jp) {  // 0xA29F2A
  QueueSfx2_Max6(0x34);
  return jp;
}

void SpikeShootingPlant_Init(void) {  // 0xA29F48
  Enemy_SpikeShootingPlant *E = Get_SpikeShootingPlant(cur_enemy_index);
  E->base.spritemap_pointer = addr_kSpritemap_Nothing_A2;
  if (HIBYTE(E->sspt_parameter_1))
    SpikeShootingPlant_5();
  else
    SpikeShootingPlant_7();
  uint16 sspt_parameter_1_low = LOBYTE(E->sspt_parameter_1);
  E->sspt_var_E = sspt_parameter_1_low;
  E->sspt_var_F = g_off_A29F42[sspt_parameter_1_low];
  int v2 = LOBYTE(E->sspt_parameter_2);
  E->sspt_var_01 = g_word_A29F36[v2] + E->base.x_pos;
  E->sspt_var_00 = E->base.x_pos - g_word_A29F36[v2];
  int v3 = (8 * HIBYTE(E->sspt_parameter_2)) >> 1;
  E->sspt_var_B = kCommonEnemySpeeds_Linear[v3];
  E->sspt_var_A = kCommonEnemySpeeds_Linear[v3 + 1];
  E->sspt_var_D = kCommonEnemySpeeds_Linear[v3 + 2];
  E->sspt_var_C = kCommonEnemySpeeds_Linear[v3 + 3];
}

void SpikeShootingPlant_Main(void) {  // 0xA29FB3
  Enemy_SpikeShootingPlant *E = Get_SpikeShootingPlant(cur_enemy_index);
  EnemyRunPreInstr(E->sspt_var_F);
}

void SpikeShootingPlant_2(uint16 k) {  // 0xA29FBA
  Enemy_SpikeShootingPlant *E = Get_SpikeShootingPlant(cur_enemy_index);
  AddToHiLo(&E->base.x_pos, &E->base.x_subpos, __PAIR32__(E->sspt_var_D, E->sspt_var_C));
  if ((int16)(E->base.x_pos - E->sspt_var_00) < 0) {
    E->sspt_var_F = FUNC16(SpikeShootingPlant_3);
    E->sspt_var_E = 1;
  }
  SpikeShootingPlant_4();
}

void SpikeShootingPlant_3(uint16 k) {  // 0xA29FEC
  Enemy_SpikeShootingPlant *E = Get_SpikeShootingPlant(cur_enemy_index);
  AddToHiLo(&E->base.x_pos, &E->base.x_subpos, __PAIR32__(E->sspt_var_B, E->sspt_var_A));
  if ((int16)(E->base.x_pos - E->sspt_var_01) >= 0) {
    E->sspt_var_F = FUNC16(SpikeShootingPlant_2);
    E->sspt_var_E = 0;
  }
  SpikeShootingPlant_4();
}

void SpikeShootingPlant_4(void) {  // 0xA2A01C

  NextRandom();
  Enemy_SpikeShootingPlant *E = Get_SpikeShootingPlant(cur_enemy_index);
  if (sign16((uint8)(LOBYTE(E->base.frame_counter) + random_number) - 3)) {
    E->sspt_var_F = FUNC16(nullsub_182);
    if (HIBYTE(E->sspt_parameter_1))
      SpikeShootingPlant_6();
    else
      SpikeShootingPlant_8();
  }
}

void SpikeShootingPlant_5(void) {  // 0xA2A049
  Enemy_SpikeShootingPlant *E = Get_SpikeShootingPlant(cur_enemy_index);
  E->base.current_instruction = addr_kSpikeShootingPlant_Ilist_9E8A;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
}

void SpikeShootingPlant_6(void) {  // 0xA2A05C
  Enemy_SpikeShootingPlant *E = Get_SpikeShootingPlant(cur_enemy_index);
  E->base.current_instruction = addr_kSpikeShootingPlant_Ilist_9EB0;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
}

void SpikeShootingPlant_7(void) {  // 0xA2A06F
  Enemy_SpikeShootingPlant *E = Get_SpikeShootingPlant(cur_enemy_index);
  E->base.current_instruction = addr_kSpikeShootingPlant_Ilist_9EDA;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
}

void SpikeShootingPlant_8(void) {  // 0xA2A082
  Enemy_SpikeShootingPlant *E = Get_SpikeShootingPlant(cur_enemy_index);
  E->base.current_instruction = addr_kSpikeShootingPlant_Ilist_9F00;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
}

const uint16 *SpikeShootingPlant_Instr_A095(uint16 k, const uint16 *jp) {  // 0xA2A095
  Enemy_SpikeShootingPlant *E = Get_SpikeShootingPlant(k);
  E->sspt_var_F = FUNC16(SpikeShootingPlant_2);
  if (E->sspt_var_E)
    E->sspt_var_F = FUNC16(SpikeShootingPlant_3);
  return jp;
}

const uint16 *SpikeShootingPlant_Instr_A0A7(uint16 k, const uint16 *jp) {  // 0xA2A0A7
  SpawnEprojWithGfx(*jp, cur_enemy_index, addr_kEproj_SpikeShootingPlantSpikes);
  return jp + 1;
}
