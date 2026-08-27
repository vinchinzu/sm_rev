// Enemy AI - Fake Kraid — peeled from Bank $A6
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"

static const uint16 g_word_A69A48[8] = { 0xfe00, 0xfb00, 0xfc00, 0xfb00, 0x200, 0xfb00, 0x400, 0xfb00 };

void FakeKraid_Init(void) {  // 0xA69A58
  uint16 v0 = (random_number & 3) + 2;
  Enemy_FakeKraid *E = Get_FakeKraid(cur_enemy_index);
  E->fkd_var_D = v0;
  E->fkd_var_E = v0;
  v0 += 64;
  E->fkd_var_03 = v0;
  v0 += 32;
  E->fkd_var_04 = v0;
  E->fkd_var_05 = v0 - 48;
  E->fkd_var_07 = 0;
  E->base.properties |= kEnemyProps_ProcessInstructions;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->fkd_var_B = -4;
  E->fkd_var_C = -4;
  uint16 v2 = addr_stru_A699AE;
  if ((int16)(E->base.x_pos - samus_x_pos) < 0) {
    E->fkd_var_B = 4;
    E->fkd_var_C = 4;
    v2 = addr_stru_A699FC;
  }
  E->base.current_instruction = v2;
}

void FakeKraid_Main(void) {  // 0xA69AC2
  Enemy_FakeKraid *E = Get_FakeKraid(cur_enemy_index);
  uint16 fkd_var_07 = E->fkd_var_07;
  uint16 v3 = fkd_var_07 + 2;
  if (!sign16(fkd_var_07 - 4))
    v3 = 0;
  E->fkd_var_07 = v3;
  FakeKraid_Func_1(cur_enemy_index, fkd_var_07);
}

void FakeKraid_Func_1(uint16 k, uint16 j) {  // 0xA69ADC
  Enemy_FakeKraid *ET = Get_FakeKraid(k + j);
  uint16 fkd_var_03 = ET->fkd_var_03;
  if (fkd_var_03) {
    ET->fkd_var_03 = fkd_var_03 - 1;
  } else {
    ET->fkd_var_03 = (random_number & 0x3F) + 16;
    Enemy_FakeKraid *E = Get_FakeKraid(k);
    E->fkd_var_06 = j;
    uint16 v6 = addr_kEproj_MiniKraidSpikesLeft;
    int16 fkd_var_C = E->fkd_var_C;
    if (fkd_var_C >= 0)
      v6 = addr_kEproj_MiniKraidSpikesRight;
    SpawnEprojWithGfx(0, k, v6);
    if (!CheckIfEnemyIsOnScreen())
      QueueSfx2_Max6(0x3F);
  }
}

const uint16 *FakeKraid_Instr_2(uint16 k, const uint16 *jp) {  // 0xA69B26
  Enemy_FakeKraid *E = Get_FakeKraid(cur_enemy_index);
  if (E->fkd_var_E)
    --E->fkd_var_E;
  if (E->fkd_var_D-- == 1) {
    E->fkd_var_D = (random_number & 3) + 7;
  } else {
    if (!(Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, INT16_SHL16(E->fkd_var_B))))
      goto LABEL_7;
  }
  E->fkd_var_B = -E->fkd_var_B;
LABEL_7:
  E->fkd_var_C = -4;
  if ((int16)(E->base.x_pos - samus_x_pos) < 0)
    E->fkd_var_C = 4;
  return jp;
}

const uint16 *FakeKraid_Instr_1(uint16 k, const uint16 *jp) {  // 0xA69B74
  Enemy_FakeKraid *E = Get_FakeKraid(cur_enemy_index);
  if (E->fkd_var_E) {
    if ((E->fkd_var_C & 0x8000) != 0) {
      if ((Get_FakeKraid(cur_enemy_index)->fkd_var_B & 0x8000) == 0)
        return INSTR_RETURN_ADDR(addr_stru_A699C6);
      return INSTR_RETURN_ADDR(addr_stru_A699AE);
    } else {
      if ((E->fkd_var_B & 0x8000) != 0)
        return INSTR_RETURN_ADDR(addr_stru_A69A14);
      return INSTR_RETURN_ADDR(addr_stru_A699FC);
    }
  } else {
    E->fkd_var_E = (random_number & 3) + 3;
    if ((E->fkd_var_C & 0x8000) != 0)
      return INSTR_RETURN_ADDR(addr_kFakeKraid_Ilist_99DC);
    return INSTR_RETURN_ADDR(addr_kFakeKraid_Ilist_9A2A);
  }
}

const uint16 *FakeKraid_Instr_3(uint16 k, const uint16 *jp) {  // 0xA69BB2
  if (CheckIfEnemyIsOnScreen() == 0)
    QueueSfx2_Max6(0x16);
  return jp;
}

const uint16 *FakeKraid_Instr_4(uint16 k, const uint16 *jp) {  // 0xA69BC4
  FakeKraid_InstrHelper_45(k, 0, 0xFFFC);
  return jp;
}

void FakeKraid_InstrHelper_45(uint16 k, uint16 j, uint16 a) {  // 0xA69BCB
  uint16 ka = cur_enemy_index;
  Enemy_FakeKraid *E = Get_FakeKraid(cur_enemy_index);
  E->fkd_var_02 = a;
  int v5 = j >> 1;
  E->fkd_var_00 = g_word_A69A48[v5];
  uint16 v6 = g_word_A69A48[v5 + 1];
  E->fkd_var_01 = v6;
  SpawnEprojWithGfx(v6, cur_enemy_index, addr_kEproj_MiniKraidSpit);
  E->fkd_var_00 = g_word_A69A48[v5 + 2];
  uint16 v8 = g_word_A69A48[v5 + 3];
  E->fkd_var_01 = v8;
  SpawnEprojWithGfx(v8, ka, addr_kEproj_MiniKraidSpit);
}

const uint16 *FakeKraid_Instr_5(uint16 k, const uint16 *jp) {  // 0xA69C02
  FakeKraid_InstrHelper_45(k, 8, 4);
  return jp;
}

void sub_A69C0B(void) {  // 0xA69C0B
  EnemyData *v0 = gEnemyData(cur_enemy_index);
  special_death_item_drop_x_origin_pos = v0->x_pos;
  special_death_item_drop_y_origin_pos = v0->y_pos;
  NormalEnemyPowerBombAiSkipDeathAnim_CurEnemy();
  FakeKraid_9C50();
}

void FakeKraid_Touch(void) {  // 0xA69C22
  Enemy_FakeKraid *E = Get_FakeKraid(cur_enemy_index);
  special_death_item_drop_x_origin_pos = E->base.x_pos;
  special_death_item_drop_y_origin_pos = E->base.y_pos;
  NormalEnemyTouchAiSkipDeathAnim_CurEnemy();
  FakeKraid_9C50();
}

void FakeKraid_Shot(void) {  // 0xA69C39
  Enemy_FakeKraid *E = Get_FakeKraid(cur_enemy_index);
  special_death_item_drop_x_origin_pos = E->base.x_pos;
  special_death_item_drop_y_origin_pos = E->base.y_pos;
  NormalEnemyShotAiSkipDeathAnim_CurEnemy();
  FakeKraid_9C50();
}

void FakeKraid_9C50(void) {  // 0xA69C50
  if (!Get_FakeKraid(cur_enemy_index)->base.health) {
    EnemyDeathAnimation(cur_enemy_index, 3);
    Enemy_ItemDrop_MiniKraid(cur_enemy_index);
  }
}
