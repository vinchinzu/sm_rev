// Enemy AI - Mochtroid
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"

static const int16 kMochtroidStepX[4] = { 2, 0, -2, 0 };
static const int16 kMochtroidStepY[4] = { 0, -2, 0, 2 };

static void Mochtroid_ChaseSamus(void);
static void Mochtroid_RecoverFromContact(void);
static void Mochtroid_DriftTowardSamus(void);
static void Mochtroid_SetInstruction(uint16 k, uint16 instr);

void Mochtroid_Init(void) {  // 0xA3A77D
  Enemy_Mochtroid *E = Get_Mochtroid(cur_enemy_index);
  E->base.layer = 2;
  Mochtroid_SetInstruction(cur_enemy_index, addr_kMochtroid_Ilist_A745);
  E->mochtr_var_F = 0;
}

static Func_V *const kMochtroidHandlers[3] = {
    Mochtroid_ChaseSamus,
    Mochtroid_DriftTowardSamus,
    Mochtroid_RecoverFromContact,
};

void Mochtroid_Main(void) {  // 0xA3A790
  uint16 state = Get_Mochtroid(cur_enemy_index)->mochtr_var_F;
  Get_Mochtroid(cur_enemy_index)->mochtr_var_F = 0;
  kMochtroidHandlers[state]();
}

static void Mochtroid_ChaseSamus(void) {  // 0xA3A7AA
  int16 y_velocity;
  int16 x_velocity;

  Enemy_Mochtroid *E = Get_Mochtroid(cur_enemy_index);
  int32 y_accel = INT16_SHL8((int16)(E->base.y_pos - samus_y_pos) >> 2);
  AddToHiLo(&E->mochtr_var_D, &E->mochtr_var_C, -y_accel);
  if ((int16)E->mochtr_var_D < 0) {
    if ((uint16)E->mochtr_var_D < 0xfffd) {
      y_velocity = -3;
      goto clamp_y_velocity;
    }
  } else if ((uint16)E->mochtr_var_D >= 3) {
    y_velocity = 3;
clamp_y_velocity:
    E->mochtr_var_D = y_velocity;
    E->mochtr_var_C = 0;
  }
  if (Enemy_MoveDown(cur_enemy_index, __PAIR32__(E->mochtr_var_D, E->mochtr_var_C))) {
    E->mochtr_var_C = 0;
    E->mochtr_var_D = 0;
  }

  int32 x_accel = INT16_SHL8((int16)(E->base.x_pos - samus_x_pos) >> 2);
  AddToHiLo(&E->mochtr_var_B, &E->mochtr_var_A, -x_accel);
  if ((int16)E->mochtr_var_B < 0) {
    if ((uint16)E->mochtr_var_B < 0xfffd) {
      x_velocity = -3;
      goto clamp_x_velocity;
    }
  } else if ((uint16)E->mochtr_var_B >= 3) {
    x_velocity = 3;
clamp_x_velocity:
    E->mochtr_var_B = x_velocity;
    E->mochtr_var_A = 0;
  }
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, __PAIR32__(E->mochtr_var_B, E->mochtr_var_A))) {
    E->mochtr_var_A = 0;
    E->mochtr_var_B = 0;
  }
  Mochtroid_SetInstruction(cur_enemy_index, addr_kMochtroid_Ilist_A745);
}

static void Mochtroid_RecoverFromContact(void) {  // 0xA3A88F
  Enemy_Mochtroid *E = Get_Mochtroid(cur_enemy_index);
  int step_index = (E->mochtr_var_E & 6) >> 1;
  E->base.x_pos += kMochtroidStepX[step_index];
  E->base.y_pos += kMochtroidStepY[step_index];
  E->mochtr_var_A = 0;
  E->mochtr_var_B = 0;
  E->mochtr_var_C = 0;
  E->mochtr_var_D = 0;
  if (E->mochtr_var_E-- == 1)
    E->mochtr_var_F = 0;
  Mochtroid_SetInstruction(cur_enemy_index, addr_kMochtroid_Ilist_A745);
}

static void Mochtroid_DriftTowardSamus(void) {  // 0xA3A8C8
  Enemy_Mochtroid *E = Get_Mochtroid(cur_enemy_index);
  if (E->base.x_pos == samus_x_pos) {
    E->mochtr_var_A = 0;
    E->mochtr_var_B = 0;
  } else if ((int16)(E->base.x_pos - samus_x_pos) >= 0) {
    E->mochtr_var_A = 0;
    E->mochtr_var_B = -1;
  } else {
    E->mochtr_var_A = 0;
    E->mochtr_var_B = 1;
  }
  Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, __PAIR32__(E->mochtr_var_B, E->mochtr_var_A));

  if (E->base.y_pos == samus_y_pos) {
    E->mochtr_var_C = 0;
    E->mochtr_var_D = 0;
  } else if ((int16)(E->base.y_pos - samus_y_pos) >= 0) {
    E->mochtr_var_C = 0;
    E->mochtr_var_D = -1;
  } else {
    E->mochtr_var_C = 0;
    E->mochtr_var_D = 1;
  }
  Enemy_MoveDown(cur_enemy_index, __PAIR32__(E->mochtr_var_D, E->mochtr_var_C));
}

static void Mochtroid_SetInstruction(uint16 k, uint16 instr) {  // 0xA3A93C
  Enemy_Mochtroid *E = Get_Mochtroid(k);
  if (instr != E->mochtr_var_01) {
    E->mochtr_var_01 = instr;
    E->base.current_instruction = instr;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
  }
}

void Mochtroid_Touch(void) {  // 0xA3A953
  Enemy_Mochtroid *E = Get_Mochtroid(cur_enemy_index);
  E->mochtr_var_F = 1;
  Mochtroid_SetInstruction(cur_enemy_index, addr_kMochtroid_Ilist_A759);
  ++E->mochtr_var_20;
  if (samus_contact_damage_index)
    goto deal_damage;
  if ((random_enemy_counter & 7) == 7 && !sign16(samus_health - 30))
    QueueSfx3_Max6(0x2d);
  if (!sign16(E->mochtr_var_20 - 80)) {
    E->mochtr_var_20 = 0;
deal_damage:
    Enemy_NormalTouchAI_A3();
    samus_invincibility_timer = 0;
    samus_knockback_timer = 0;
  }
}

void Mochtroid_Shot(void) {  // 0xA3A9A8
  Enemy_NormalShotAI_A3();
}
