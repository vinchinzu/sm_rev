// MiniCrocomire extracted from sm_a2.c.
#include "ida_types.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "variables.h"
#include "enemy_ai_canon.h"


const uint16 *MiniCrocomire_Instr_897E(uint16 k, const uint16 *jp) {  // 0xA2897E
  SpawnEprojWithGfx(*jp, cur_enemy_index, addr_Eproj_DBF2);
  return jp + 1;
}

const uint16 *MiniCrocomire_Instr_8990(uint16 k, const uint16 *jp) {  // 0xA28990
  Enemy_MiniCrocomire *E = Get_MiniCrocomire(cur_enemy_index);
  E->mce_var_F = FUNC16(MiniCrocomire_PreInstr5);
  E->mce_var_E = 0;
  return jp;
}

const uint16 *MiniCrocomire_Instr_899D(uint16 k, const uint16 *jp) {  // 0xA2899D
  Enemy_MiniCrocomire *E = Get_MiniCrocomire(cur_enemy_index);
  E->mce_var_F = FUNC16(MiniCrocomire_PreInstr6);
  E->mce_var_E = 1;
  return jp;
}

void MiniCrocomire_Init(void) {  // 0xA289AD
  Enemy_MiniCrocomire *E = Get_MiniCrocomire(cur_enemy_index);
  E->base.spritemap_pointer = addr_kSpritemap_Nothing_A2;
  int v1 = (8 * E->mce_parameter_2) >> 1;
  E->mce_var_B = kCommonEnemySpeeds_Linear[v1];
  E->mce_var_A = kCommonEnemySpeeds_Linear[v1 + 1];
  E->mce_var_D = kCommonEnemySpeeds_Linear[v1 + 2];
  E->mce_var_C = kCommonEnemySpeeds_Linear[v1 + 3];
  MiniCrocomire_Func1();
  E->mce_var_F = FUNC16(MiniCrocomire_PreInstr5);
  uint16 mce_parameter_1 = E->mce_parameter_1;
  E->mce_var_E = mce_parameter_1;
  if (mce_parameter_1) {
    MiniCrocomire_Func3();
    E->mce_var_F = FUNC16(MiniCrocomire_PreInstr6);
  }
}

void MiniCrocomire_Main(void) {  // 0xA289F0
  Enemy_MiniCrocomire *E = Get_MiniCrocomire(cur_enemy_index);
  EnemyRunPreInstr(E->mce_var_F);
}

void MiniCrocomire_Func1(void) {  // 0xA289F7
  Enemy_MiniCrocomire *E = Get_MiniCrocomire(cur_enemy_index);
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->base.current_instruction = addr_kMiniCrocomire_Ilist_8932;
}

void MiniCrocomire_Func2(void) {  // 0xA28A0A
  Enemy_MiniCrocomire *E = Get_MiniCrocomire(cur_enemy_index);
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->base.current_instruction = addr_kMiniCrocomire_Ilist_8948;
}

void MiniCrocomire_Func3(void) {  // 0xA28A1D
  Enemy_MiniCrocomire *E = Get_MiniCrocomire(cur_enemy_index);
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->base.current_instruction = addr_kMiniCrocomire_Ilist_8958;
}

void MiniCrocomire_Func4(void) {  // 0xA28A30
  Enemy_MiniCrocomire *E = Get_MiniCrocomire(cur_enemy_index);
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->base.current_instruction = addr_kMiniCrocomire_Ilist_896E;
}

void MiniCrocomire_PreInstr5(uint16 k) {  // 0xA28A43
  Enemy_MiniCrocomire *E = Get_MiniCrocomire(cur_enemy_index);
  MiniCrocomire_Func7(__PAIR32__(E->mce_var_D, E->mce_var_C));
  if (MiniCrocomire_Func9())
    MiniCrocomire_Func2();
}

void MiniCrocomire_PreInstr6(uint16 k) {  // 0xA28A5C
  Enemy_MiniCrocomire *E = Get_MiniCrocomire(cur_enemy_index);
  MiniCrocomire_Func7(__PAIR32__(E->mce_var_B, E->mce_var_A));
  if (MiniCrocomire_Func9())
    MiniCrocomire_Func4();
}

void MiniCrocomire_Func7(int32 amt) {  // 0xA28A76
  if (!Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, amt)) {
    if (EnemyFunc_BC76(cur_enemy_index, INT16_SHL16(2)))
      return;
  }
  MiniCrocomire_Func8();
}

void MiniCrocomire_Func8(void) {  // 0xA28A95
  MiniCrocomire_Func1();
  if (Get_MiniCrocomire(cur_enemy_index)->mce_var_E != 1)
    MiniCrocomire_Func3();
}

uint8 MiniCrocomire_Func9(void) {  // 0xA28AA7
  NextRandom();
  Enemy_MiniCrocomire *E = Get_MiniCrocomire(cur_enemy_index);
  if (sign16((uint8)(LOBYTE(E->base.frame_counter) + random_number) - 2)) {
    E->mce_var_F = FUNC16(nullsub_175);
    return 1;
  }
  return 0;
}
