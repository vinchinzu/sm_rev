// SpikeShootingPlant extracted from sm_a2.c.
#include "ida_types.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "variables.h"
#include "enemy_ai_canon.h"

enum {
  kSfx2_SpikeShootingPlantSpit = 0x34,
  kSpikeShootingPlantShootChance = 3,
};

static const uint16 kSpikeShootingPlantTravel[6] = {
  0x0010, 0x0040, 0x0050, 0x0060, 0x0070, 0x0080,
};
static const uint16 kSpikeShootingPlantMoveFuncs[3] = {
  0x9FBA, 0x9FEC, 0xA01B,
};

const uint16 *SpikeShootingPlant_Instr_9F2A(uint16 k, const uint16 *jp) {  // 0xA29F2A
  QueueSfx2_Max6(kSfx2_SpikeShootingPlantSpit);
  return jp;
}

void SpikeShootingPlant_Init(void) {  // 0xA29F48
  Enemy_SpikeShootingPlant *E = Get_SpikeShootingPlant(cur_enemy_index);
  E->base.spritemap_pointer = addr_kSpritemap_Nothing_A2;
  if (HIBYTE(E->sspt_parameter_1))
    SpikeShootingPlant_5();
  else
    SpikeShootingPlant_7();
  uint16 dir = LOBYTE(E->sspt_parameter_1);
  E->sspt_var_E = dir;
  E->sspt_var_F = kSpikeShootingPlantMoveFuncs[dir];
  int travel = LOBYTE(E->sspt_parameter_2);
  E->sspt_var_01 = kSpikeShootingPlantTravel[travel] + E->base.x_pos;
  E->sspt_var_00 = E->base.x_pos - kSpikeShootingPlantTravel[travel];
  int speed = (8 * HIBYTE(E->sspt_parameter_2)) >> 1;
  E->sspt_var_B = kCommonEnemySpeeds_Linear[speed];
  E->sspt_var_A = kCommonEnemySpeeds_Linear[speed + 1];
  E->sspt_var_D = kCommonEnemySpeeds_Linear[speed + 2];
  E->sspt_var_C = kCommonEnemySpeeds_Linear[speed + 3];
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
  if (sign16((uint8)(LOBYTE(E->base.frame_counter) + random_number) - kSpikeShootingPlantShootChance)) {
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
