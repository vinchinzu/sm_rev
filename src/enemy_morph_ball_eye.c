// Enemy AI - Morph Ball Eye — peeled from Bank $A8

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

#define g_off_A890DA ((uint16*)RomFixedPtr(0xa890da))

static const int16 g_word_A890CA[4] = { -8, 8, 0, 0 };
static const int16 g_word_A890D2[4] = { 0, 0, -8, 8 };
static const uint16 g_word_A89050 = 0x80;
static const uint16 g_word_A89052 = 0xb0;
static const uint16 g_word_A89054 = 0x80;
static const uint16 g_word_A89056 = 0x80;

void MorphBallEye_Init(void) {  // 0xA89058
  Enemy_MorphBallEye *E = Get_MorphBallEye(cur_enemy_index);
  E->base.properties |= kEnemyProps_ProcessInstructions;
  E->base.spritemap_pointer = addr_kSpritemap_Nothing_A8;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  if ((E->mbee_parameter_2 & 0x8000) != 0) {
    int v1 = E->mbee_parameter_2 & 0xF;
    E->base.x_pos += g_word_A890CA[v1];
    E->base.y_pos += g_word_A890D2[v1];
    E->mbee_var_F = FUNC16(nullsub_244);
    E->base.current_instruction = g_off_A890DA[v1];
    for (int i = 510; i >= 0; i -= 2)
      *(uint16 *)((uint8 *)&g_word_7E9100 + (uint16)i) = 255;
  } else {
    E->mbee_var_F = FUNC16(MorphBallEye_Func_1);
    if ((E->mbee_parameter_1 & 1) != 0)
      E->base.current_instruction = addr_stru_A8900E;
    else
      E->base.current_instruction = addr_stru_A88FFC;
  }
}

void MorphBallEye_Main(void) {  // 0xA890E2
  if ((collected_items & 4) != 0) {
    Enemy_MorphBallEye *E = Get_MorphBallEye(cur_enemy_index);
    EnemyRunPreInstr(E->mbee_var_F);
  }
}

void MorphBallEye_Func_1(uint16 k) {  // 0xA890F1
  if (IsSamusWithinEnemy_Y(k, g_word_A89054) && IsSamusWithinEnemy_X(k, g_word_A89050)) {
    Enemy_MorphBallEye *E = Get_MorphBallEye(k);
    E->mbee_var_E = 32;
    E->base.instruction_timer = 1;
    if ((E->mbee_parameter_1 & 1) != 0)
      E->base.current_instruction = addr_kMorphBallEye_Ilist_9026;
    else
      E->base.current_instruction = addr_kMorphBallEye_Ilist_9014;
    E->mbee_var_F = FUNC16(MorphBallEye_Func_2);
  }
}

void MorphBallEye_Func_2(uint16 k) {  // 0xA8912E
  Enemy_MorphBallEye *E = Get_MorphBallEye(k);
  bool v2 = E->mbee_var_E == 1;
  bool v3 = (--E->mbee_var_E & 0x8000) != 0;
  if (v2 || v3) {
    QueueSfx2_Max6(0x17);
    SpawnMorphBallEyeBeamHdma();
    E->mbee_var_F = FUNC16(MorphBallEye_Func_3);
    E->mbee_var_D = CalculateAngleFromXY(samus_x_pos - E->base.x_pos, samus_y_pos - E->base.y_pos);
  }
}

void MorphBallEye_Func_3(uint16 k) {  // 0xA89160
  Enemy_MorphBallEye *E = Get_MorphBallEye(k);
  if (IsSamusWithinEnemy_Y(k, g_word_A89056) && IsSamusWithinEnemy_X(k, g_word_A89052)) {
    uint16 v3 = CalculateAngleFromXY(samus_x_pos - E->base.x_pos, samus_y_pos - E->base.y_pos);
    E->mbee_var_D = v3;
    E->base.current_instruction = ((v3 & 0xF0) >> 2) - 28756;
  } else {
    QueueSfx2_Max6(0x71);
    E->mbee_var_C = 0;
    E->mbee_var_E = 32;
    if ((E->mbee_parameter_1 & 1) != 0)
      E->base.current_instruction = addr_kMorphBallEye_Ilist_9002;
    else
      E->base.current_instruction = addr_kMorphBallEye_Ilist_8FF0;
    E->mbee_var_F = FUNC16(MorphBallEye_Func_4);
  }
  E->base.instruction_timer = 1;
}

void MorphBallEye_Func_4(uint16 k) {  // 0xA891CE
  Enemy_MorphBallEye *E = Get_MorphBallEye(k);
  bool v2 = E->mbee_var_E == 1;
  bool v3 = (--E->mbee_var_E & 0x8000) != 0;
  if (v2 || v3)
    E->mbee_var_F = FUNC16(MorphBallEye_Func_1);
}

