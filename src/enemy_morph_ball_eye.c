// Enemy AI - Morph Ball Eye — peeled from Bank $A8

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

enum {
  kCollectedItems_MorphBall = 0x04,
  kMorphBallEyeParam_Mount = 0x8000,
  kMorphBallEyeParam_FaceRight = 1,
  kMorphBallEyeDirMask = 0xF,
  kMorphBallEyeActivateDelay = 32,
  kSfx2_MorphBallEyeRay = 0x17,
  kSfx2_Silence = 0x71,
};

static const int16 kMorphBallEyeMountXOff[4] = { -8, 8, 0, 0 };
static const int16 kMorphBallEyeMountYOff[4] = { 0, 0, -8, 8 };
static const uint16 kMorphBallEyeMountIlists[4] = {
  addr_kMorphBallEye_Ilist_9044,
  addr_kMorphBallEye_Ilist_9038,
  addr_kMorphBallEye_Ilist_904A,
  addr_kMorphBallEye_Ilist_903E,
};
static const uint16 kMorphBallEyeActivateXRadius = 0x80;
static const uint16 kMorphBallEyeDeactivateXRadius = 0xb0;
static const uint16 kMorphBallEyeActivateYRadius = 0x80;
static const uint16 kMorphBallEyeDeactivateYRadius = 0x80;

void MorphBallEye_Init(void) {  // 0xA89058
  Enemy_MorphBallEye *E = Get_MorphBallEye(cur_enemy_index);
  E->base.properties |= kEnemyProps_ProcessInstructions;
  E->base.spritemap_pointer = addr_kSpritemap_Nothing_A8;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  if (E->mbee_parameter_2 & kMorphBallEyeParam_Mount) {
    int dir = E->mbee_parameter_2 & kMorphBallEyeDirMask;
    E->base.x_pos += kMorphBallEyeMountXOff[dir];
    E->base.y_pos += kMorphBallEyeMountYOff[dir];
    E->mbee_var_F = FUNC16(nullsub_244);
    E->base.current_instruction = kMorphBallEyeMountIlists[dir];
    for (int i = 510; i >= 0; i -= 2)
      *(uint16 *)((uint8 *)&g_word_7E9100 + (uint16)i) = 255;
  } else {
    E->mbee_var_F = FUNC16(MorphBallEye_Func_1);
    if (E->mbee_parameter_1 & kMorphBallEyeParam_FaceRight)
      E->base.current_instruction = addr_kMorphBallEye_Ilist_900E;
    else
      E->base.current_instruction = addr_kMorphBallEye_Ilist_8FFC;
  }
}

void MorphBallEye_Main(void) {  // 0xA890E2
  if (collected_items & kCollectedItems_MorphBall) {
    Enemy_MorphBallEye *E = Get_MorphBallEye(cur_enemy_index);
    EnemyRunPreInstr(E->mbee_var_F);
  }
}

void MorphBallEye_Func_1(uint16 k) {  // 0xA890F1
  if (IsSamusWithinEnemy_Y(k, kMorphBallEyeActivateYRadius) && IsSamusWithinEnemy_X(k, kMorphBallEyeActivateXRadius)) {
    Enemy_MorphBallEye *E = Get_MorphBallEye(k);
    E->mbee_var_E = kMorphBallEyeActivateDelay;
    E->base.instruction_timer = 1;
    if (E->mbee_parameter_1 & kMorphBallEyeParam_FaceRight)
      E->base.current_instruction = addr_kMorphBallEye_Ilist_9026;
    else
      E->base.current_instruction = addr_kMorphBallEye_Ilist_9014;
    E->mbee_var_F = FUNC16(MorphBallEye_Func_2);
  }
}

void MorphBallEye_Func_2(uint16 k) {  // 0xA8912E
  Enemy_MorphBallEye *E = Get_MorphBallEye(k);
  bool expired = E->mbee_var_E == 1;
  bool underflow = sign16(--E->mbee_var_E);
  if (expired || underflow) {
    QueueSfx2_Max6(kSfx2_MorphBallEyeRay);
    SpawnMorphBallEyeBeamHdma();
    E->mbee_var_F = FUNC16(MorphBallEye_Func_3);
    E->mbee_var_D = CalculateAngleFromXY(samus_x_pos - E->base.x_pos, samus_y_pos - E->base.y_pos);
  }
}

void MorphBallEye_Func_3(uint16 k) {  // 0xA89160
  Enemy_MorphBallEye *E = Get_MorphBallEye(k);
  if (IsSamusWithinEnemy_Y(k, kMorphBallEyeDeactivateYRadius) && IsSamusWithinEnemy_X(k, kMorphBallEyeDeactivateXRadius)) {
    uint16 angle = CalculateAngleFromXY(samus_x_pos - E->base.x_pos, samus_y_pos - E->base.y_pos);
    E->mbee_var_D = angle;
    E->base.current_instruction = addr_kMorphBallEye_Ilist_8FAC + ((angle & 0xF0) >> 2);
  } else {
    QueueSfx2_Max6(kSfx2_Silence);
    E->mbee_var_C = 0;
    E->mbee_var_E = kMorphBallEyeActivateDelay;
    if (E->mbee_parameter_1 & kMorphBallEyeParam_FaceRight)
      E->base.current_instruction = addr_kMorphBallEye_Ilist_9002;
    else
      E->base.current_instruction = addr_kMorphBallEye_Ilist_8FF0;
    E->mbee_var_F = FUNC16(MorphBallEye_Func_4);
  }
  E->base.instruction_timer = 1;
}

void MorphBallEye_Func_4(uint16 k) {  // 0xA891CE
  Enemy_MorphBallEye *E = Get_MorphBallEye(k);
  bool expired = E->mbee_var_E == 1;
  bool underflow = sign16(--E->mbee_var_E);
  if (expired || underflow)
    E->mbee_var_F = FUNC16(MorphBallEye_Func_1);
}
