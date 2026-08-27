#include "samus_status.h"

#include "samus_env.h"

static uint16 g_samus_lockout_timer;

static void SamusStatus_ClearMotion(void) {
  input_to_pose_calc = 0;
  joypad1_input_samusfilter = 0;
  joypad1_newinput_samusfilter = 0;
  samus_x_extra_run_speed = 0;
  samus_x_extra_run_subspeed = 0;
  samus_x_base_speed = 0;
  samus_x_base_subspeed = 0;
  samus_x_accel_mode = kSamusXAccelMode_None;
  samus_y_speed = 0;
  samus_y_subspeed = 0;
}

void SamusStatus_RequestLockout(uint16 frames) {
  if (frames == 0)
    return;
  if (g_samus_lockout_timer < frames)
    g_samus_lockout_timer = frames;
  if (samus_knockback_timer < frames)
    samus_knockback_timer = frames;
  SamusStatus_ClearMotion();
}

void SamusStatus_Tick(void) {
  if (g_samus_lockout_timer == 0)
    return;
  SamusStatus_ClearMotion();
  --g_samus_lockout_timer;
}

bool SamusStatus_LockoutActive(void) {
  return g_samus_lockout_timer != 0;
}

uint16 SuitDamageDivision(uint16 a) {  // 0xA0A45E
  if ((equipped_items & kSamusEquip_GravitySuit) != 0)
    return a >> 2;
  if (equipped_items & kSamusEquip_VariaSuit)
    return a >> 1;
  return a;
}
