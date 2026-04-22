// Elevator runtime extracted from Bank $A3 enemy code.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"

#define kElevatorActivationInputMask ((uint16 *)RomFixedPtr(0xa394e2))

enum {
  kElevatorState_WaitForInput = 0,
  kElevatorState_MovePlatform = 1,
  kElevatorState_StartReturn = 2,
  kElevatorState_Returning = 3,
  kElevatorMoveSpeed = 0x18000,
  kElevatorSamusYOffset = 26,
  kSfx3_ElevatorActivate = 0x0B,
  kSfx1_ElevatorTravel = 0x32,
  kSfx3_ElevatorArrive = 0x25,
  kSamusCode_SetupForElevator = 7,
  kSamusCode_ExitElevator = 0x0B,
};

static void Elevator_WaitForInput(void);
static void Elevator_MovePlatform(void);
static void Elevator_StartReturn(void);
static void Elevator_ReturnPlatform(void);
static void Elevator_SyncSamusToPlatform(void);

static bool ElevatorIsActive(void) {
  return elevator_flags != 0 || elevator_status != 0;
}

void Elevator_Init(void) {  // 0xA394E6
  Enemy_Elevator *E = Get_Elevator(cur_enemy_index);
  E->base.spritemap_pointer = addr_kSpritemap_Nothing_A3;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->base.current_instruction = addr_kElevator_Ilist_94D6;
  E->elevat_parameter_1 *= 2;
  E->elevat_var_A = E->base.y_pos;
  if (elevator_status != kElevatorState_StartReturn)
    elevator_flags = elevator_status = 0;
  if (ElevatorIsActive()) {
    E->base.y_pos = E->elevat_parameter_2;
    Elevator_SyncSamusToPlatform();
  }
}

void Elevator_Frozen(void) {  // 0xA3952A
  static Func_V *const kElevatorStateHandlers[4] = {
    Elevator_WaitForInput,
    Elevator_MovePlatform,
    Elevator_StartReturn,
    Elevator_ReturnPlatform,
  };

  if (!door_transition_flag_elevator_zebetites && ElevatorIsActive())
    kElevatorStateHandlers[elevator_status]();
}

static void Elevator_WaitForInput(void) {  // 0xA39548
  Enemy_Elevator *E = Get_Elevator(cur_enemy_index);
  uint16 input_mask = kElevatorActivationInputMask[E->elevat_parameter_1 >> 1];
  if ((input_mask & joypad1_newkeys) == 0) {
    elevator_flags = 0;
    return;
  }

  QueueSfx3_Max6(kSfx3_ElevatorActivate);
  QueueSfx1_Max6(kSfx1_ElevatorTravel);
  CallSomeSamusCode(kSamusCode_SetupForElevator);
  ResetProjectileData();
  Elevator_SyncSamusToPlatform();
  ++elevator_status;
}

static void Elevator_MovePlatform(void) {  // 0xA39579
  Enemy_Elevator *E = Get_Elevator(cur_enemy_index);
  if (E->elevat_parameter_1) {
    elevator_direction = 0x8000;
    AddToHiLo(&E->base.y_pos, &E->base.y_subpos, -kElevatorMoveSpeed);
  } else {
    elevator_direction = 0;
    AddToHiLo(&E->base.y_pos, &E->base.y_subpos, kElevatorMoveSpeed);
  }
  Elevator_SyncSamusToPlatform();
}

static void Elevator_StartReturn(void) {  // 0xA395B9
  ++elevator_status;
  Elevator_ReturnPlatform();
}

static void Elevator_ReturnPlatform(void) {  // 0xA395BC
  Enemy_Elevator *E = Get_Elevator(cur_enemy_index);
  if (E->elevat_parameter_1) {
    AddToHiLo(&E->base.y_pos, &E->base.y_subpos, kElevatorMoveSpeed);
    if (E->base.y_pos < E->elevat_var_A) {
      Elevator_SyncSamusToPlatform();
      return;
    }
  } else {
    AddToHiLo(&E->base.y_pos, &E->base.y_subpos, -kElevatorMoveSpeed);
    if (E->base.y_pos >= E->elevat_var_A) {
      Elevator_SyncSamusToPlatform();
      return;
    }
  }

  elevator_status = 0;
  elevator_flags = 0;
  QueueSfx3_Max6(kSfx3_ElevatorArrive);
  E->base.y_pos = E->elevat_var_A;
  CallSomeSamusCode(kSamusCode_ExitElevator);
  Elevator_SyncSamusToPlatform();
}

static void Elevator_SyncSamusToPlatform(void) {  // 0xA39612
  Enemy_Elevator *E = Get_Elevator(cur_enemy_index);
  samus_y_pos = E->base.y_pos - kElevatorSamusYOffset;
  samus_y_subpos = 0;
  samus_x_pos = E->base.x_pos;
  samus_y_speed = 0;
  samus_y_subspeed = 0;
}
