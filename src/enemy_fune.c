// Enemy AI - Fune — peeled from Bank $A8

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

const uint16 *Fune_Instr_2(uint16 k, const uint16 *jp) {  // 0xA89625
  QueueSfx2_Max9(0x1F);
  return jp;
}

const uint16 *Fune_Instr_6(uint16 k, const uint16 *jp) {  // 0xA89631
  eproj_unk1995 = LOBYTE(Get_Fune(cur_enemy_index)->fune_parameter_2);
  SpawnEprojWithGfx(0, cur_enemy_index, addr_stru_86DFBC);
  return jp;
}

const uint16 *Fune_Instr_7(uint16 k, const uint16 *jp) {  // 0xA8964A
  eproj_unk1995 = LOBYTE(Get_Fune(cur_enemy_index)->fune_parameter_2);
  SpawnEprojWithGfx(1, cur_enemy_index, addr_stru_86DFBC);
  return jp;
}

const uint16 *Fune_Instr_1(uint16 k, const uint16 *jp) {  // 0xA89663
  eproj_unk1995 = LOBYTE(Get_Fune(cur_enemy_index)->fune_parameter_2);
  SpawnEprojWithGfx(0, cur_enemy_index, addr_stru_86DFCA);
  return jp;
}

const uint16 *Fune_Instr_4(uint16 k, const uint16 *jp) {  // 0xA8967C
  eproj_unk1995 = LOBYTE(Get_Fune(cur_enemy_index)->fune_parameter_2);
  SpawnEprojWithGfx(1, cur_enemy_index, addr_stru_86DFCA);
  return jp;
}

const uint16 *Fune_Instr_3(uint16 k, const uint16 *jp) {  // 0xA89695
  Enemy_Fune *E = Get_Fune(cur_enemy_index);
  E->fune_var_A += 4;
  E->fune_var_B = FUNC16(Fune_Func_1);
  if (E->fune_var_D)
    E->fune_var_B = FUNC16(Fune_Func_2);
  return jp;
}

const uint16 *Fune_Instr_5(uint16 k, const uint16 *jp) {  // 0xA896B4
  Enemy_Fune *E = Get_Fune(cur_enemy_index);
  E->fune_var_A += 4;
  E->fune_var_B = FUNC16(Fune_Func_1);
  if (E->fune_var_D)
    E->fune_var_B = FUNC16(Fune_Func_2);
  return jp;
}

void Fune_Init(void) {  // 0xA896E3
  Enemy_Fune *E = Get_Fune(cur_enemy_index);
  E->fune_var_A = addr_off_A896D7;
  E->fune_var_B = FUNC16(Fune_Func_1);
  uint16 v1 = E->fune_parameter_1 & 0xF;
  E->fune_var_D = v1;
  if (v1) {
    E->fune_var_A = addr_off_A896DF;
    E->fune_var_B = FUNC16(Fune_Func_2);
  }
  if ((E->fune_parameter_1 & 0xF0) != 0)
    E->fune_var_A += 2;
  Fune_Func_4();
  E->fune_var_C = HIBYTE(E->fune_parameter_2);
  E->fune_var_F = HIBYTE(E->fune_parameter_1);
  E->fune_var_E = 0;
}

void CallFuneFunc(uint32 ea) {
  switch (ea) {
  case fnFune_Func_1: Fune_Func_1(); return;  // 0xa89737
  case fnFune_Func_2: Fune_Func_2(); return;  // 0xa8975c
  case fnnullsub_247: return;  // 0xa8978e
  case fnnullsub_248: return;  // 0xa8978f
  default: Unreachable();
  }
}

void Fune_Main(void) {  // 0xA89730
  Enemy_Fune *E = Get_Fune(cur_enemy_index);
  CallFuneFunc(E->fune_var_B | 0xA80000);
}

void Fune_Func_1(void) {  // 0xA89737
  Enemy_Fune *E = Get_Fune(cur_enemy_index);
  if ((int16)(++E->fune_var_E - E->fune_var_F) >= 0) {
    E->fune_var_A -= 4;
    Fune_Func_4();
    E->fune_var_B = FUNC16(nullsub_247);
    E->fune_var_E = 0;
  }
}

void Fune_Func_2(void) {  // 0xA8975C
  Enemy_Fune *E = Get_Fune(cur_enemy_index);
  E->fune_var_00 = E->fune_var_A;
  if (Fune_Func_3()) {
    uint16 fune_var_00 = E->fune_var_00;
    E->fune_var_A = fune_var_00;
    fune_var_00 -= 4;
    E->fune_var_00 = fune_var_00;
    E->fune_var_A = fune_var_00;
    Fune_Func_4();
    E->fune_var_B = FUNC16(nullsub_248);
  }
  E->fune_var_A = E->fune_var_00;
}

uint16 Fune_Func_3(void) {  // 0xA89790
  Enemy_Fune *E = Get_Fune(cur_enemy_index);
  return IsSamusWithinEnemy_Y(cur_enemy_index, E->fune_var_C);
}

void Fune_Func_4(void) {  // 0xA8979B
  Enemy_Fune *E = Get_Fune(cur_enemy_index);
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->base.current_instruction = *(uint16 *)RomPtr_A8(E->fune_var_A);
}

