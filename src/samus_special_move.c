// Samus special-movement state helpers: shinespark/crystal-flash state,
// hit interruption, knockback, bomb-jump setup/handlers, and the small
// grapple-adjacent helpers that feed those movement states.

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "samus_env.h"

void Samus_Movement_1B_ShinesparkEtc(void) {
  input_to_pose_calc = 0;
}

void Samus_Movement_0A_KnockbackOrCrystalFlashEnding(void) {
  input_to_pose_calc = 0;
  Samus_Move_NoSpeedCalc_Y();
}

uint8 Samus_GrappleWallJumpCheck(int32 amt) {  // 0x909CAC
  enemy_index_to_shake = -1;
  if (samus_pose_x_dir != kSamusPoseXDir_FaceRight) {
    if (samus_pose_x_dir != kSamusPoseXDir_FaceLeft)
      return 0;
    samus_collision_direction = 1;
  } else {
    samus_collision_direction = 0;
  }
  CheckEnemyColl_Result cres = Samus_CheckSolidEnemyColl(amt);
  if (!cres.collision) {
    amt = WallJumpBlockCollDetect(cres.amt);
    return samus_collision_flag && (button_config_jump_a & joypad1_newkeys) != 0;
  }
  if ((button_config_jump_a & joypad1_newkeys) == 0)
    return 0;
  enemy_index_to_shake = collision_detection_index;
  return 1;
}

void Projectile_Func7_Shinespark(void) {  // 0x90CFFA
  samus_movement_handler = FUNC16(Samus_MoveHandlerShinesparkWindup);
  samus_y_dir = 1;
  speed_boost_counter = kSpeedBoostCounter_Charged;
  samus_y_subspeed = 0;
  samus_y_speed = 0;
  knockback_dir = 0;
  samus_x_extra_run_speed = 8;
  samus_x_extra_run_subspeed = 0;
  samus_x_base_speed = 0;
  samus_x_base_subspeed = 0;
  substate = 7;
  suit_pickup_light_beam_pos = 0;
  cooldown_timer = 0;
  timer_for_shinesparks_startstop = 30;
  samus_shine_timer = 60;
  timer_for_shine_timer = 6;
  special_samus_palette_frame = 0;
  bomb_jump_dir = 0;
  if (flare_counter) {
    if (!sign16(flare_counter - 16))
      QueueSfx1_Max9(2);
    flare_counter = 0;
    ClearFlareAnimationState();
  }
}

void Samus_MoveHandlerShinesparkWindup(void) {  // 0x90D068
  bool v0 = (--timer_for_shinesparks_startstop & 0x8000) != 0;
  if (!timer_for_shinesparks_startstop || v0) {
    if (samus_pose_x_dir == kSamusPoseXDir_FaceRight)
      samus_new_pose_interrupted = 204;
    else
      samus_new_pose_interrupted = 203;
    samus_movement_handler = FUNC16(Samus_MoveHandlerVerticalShinespark);
    samus_input_handler = FUNC16(nullsub_152);
    speed_echoes_index = 0;
    speed_echo_xspeed[0] = 0;
    speed_echo_xspeed[1] = 0;
    speed_echo_xpos[0] = 0;
    speed_echo_xpos[1] = 0;
    QueueSfx3_Max9(0xF);
  }
}

void Samus_MoveHandlerVerticalShinespark(void) {  // 0x90D0AB
  samus_contact_damage_index = kSamusContactDamage_Shinespark;
  samus_hurt_flash_counter = 8;
  Samus_UpdateSpeedEchoPos();
  Samus_ShinesparkMove_Y();
  Samus_EndSuperJump();
  if (!sign16(samus_health - 30) && (--samus_health & 0x8000) != 0)
    samus_health = 0;
}

void Samus_MoveHandler_Shinespark_Diag(void) {  // 0x90D0D7
  samus_contact_damage_index = kSamusContactDamage_Shinespark;
  samus_hurt_flash_counter = 8;
  Samus_UpdateSpeedEchoPos();
  Samus_ShinesparkMove_X();
  Samus_ShinesparkMove_Y();
  Samus_EndSuperJump();
  if (!sign16(samus_health - 30) && (--samus_health & 0x8000) != 0)
    samus_health = 0;
}

void Samus_MoveHandler_Shinespark_Horiz(void) {  // 0x90D106
  samus_contact_damage_index = kSamusContactDamage_Shinespark;
  samus_hurt_flash_counter = 8;
  Samus_UpdateSpeedEchoPos();
  Samus_ShinesparkMove_X();
  Samus_EndSuperJump();
  if (!sign16(samus_health - 30) && (--samus_health & 0x8000) != 0)
    samus_health = 0;
}

static uint32 Samus_ClampSpeedHi(int32 amt, int val) {
  if (!sign16((amt >> 16) - val))
    amt = val << 16 | (amt & 0xffff);
  return amt;
}

void Samus_ShinesparkMove_X(void) {  // 0x90D132
  int16 v4;

  samus_shine_timer = 15;
  AddToHiLo(&samus_x_extra_run_speed, &samus_x_extra_run_subspeed, __PAIR32__(samus_y_accel, samus_y_subaccel));
  if (!sign16(samus_x_extra_run_speed - 15))
    SetHiLo(&samus_x_extra_run_speed, &samus_x_extra_run_subspeed, INT16_SHL16(15));
  int32 amt = 0;
  if (samus_pose_x_dir == kSamusPoseXDir_FaceRight) {
    amt = Samus_ClampSpeedHi(-(int32)Samus_CalcDisplacementMoveLeft(amt), 15);
    CheckEnemyColl_Result cres = Samus_CheckSolidEnemyColl(amt);
    amt = cres.amt;
    if (cres.collision) {
      samus_collision_flag = cres.collision;
      goto LABEL_18;
    }
    amt = -(int32)amt;
  } else {
    amt = Samus_ClampSpeedHi(Samus_CalcDisplacementMoveRight(amt), 25);
    CheckEnemyColl_Result cres = Samus_CheckSolidEnemyColl(amt);
    amt = cres.amt;
    if (cres.collision) {
      samus_collision_flag = cres.collision;
      goto LABEL_18;
    }
  }
  Samus_MoveRight_NoSolidColl(amt);
  Samus_AlignYPosSlope();
LABEL_18:
  v4 = samus_x_pos - samus_prev_x_pos;
  if ((int16)(samus_x_pos - samus_prev_x_pos) < 0) {
    if (sign16(v4 + 15))
      samus_prev_x_pos = samus_x_pos + 15;
  } else if (!sign16(v4 - 16)) {
    samus_prev_x_pos = samus_x_pos - 15;
  }
}

void Samus_ShinesparkMove_Y(void) {  // 0x90D1FF
  samus_shine_timer = 15;

  AddToHiLo(&substate, &suit_pickup_light_beam_pos, __PAIR32__(samus_y_accel, samus_y_subaccel));
  AddToHiLo(&samus_y_speed, &samus_y_subspeed, __PAIR32__(substate, suit_pickup_light_beam_pos));
  int32 amt = Samus_ClampSpeedHi(__PAIR32__(samus_y_speed, samus_y_subspeed), 14);
  amt -= __PAIR32__(extra_samus_y_displacement, extra_samus_y_subdisplacement);
  samus_collision_direction = 2;
  CheckEnemyColl_Result cres = Samus_CheckSolidEnemyColl(Samus_ClampSpeedHi(amt, 15));
  amt = cres.amt;
  if (cres.collision) {
    samus_collision_flag = cres.collision;
  } else {
    Samus_MoveDown_NoSolidColl(-(int32)amt);
  }
  if (sign16(samus_y_pos - samus_prev_y_pos + 14))
    samus_prev_y_pos = samus_y_pos + 14;
}

uint8 Samus_EndSuperJump(void) {  // 0x90D2BA
  if (!sign16(samus_health - 30) && !samus_collision_flag)
    return 0;
  if (samus_pose_x_dir == kSamusPoseXDir_FaceRight) {
    speed_echo_xspeed[0] = 32;
    speed_echo_xspeed[1] = 160;
    speed_echo_xpos[2] = 4;
  } else {
    speed_echo_xspeed[0] = 224;
    speed_echo_xspeed[1] = 96;
    speed_echo_xpos[2] = -4;
  }
  samus_y_subspeed = 0;
  samus_y_speed = 0;
  samus_x_extra_run_speed = 0;
  samus_x_extra_run_subspeed = 0;
  speed_boost_counter = 0;
  UNUSED_word_7E0B1A = 0;
  samus_y_dir = 0;
  samus_movement_handler = FUNC16(Samus_MoveHandler_ShinesparkCrash);
  samus_draw_handler = FUNC16(Samus_DrawHandler_EndOfShinespark);
  speed_echoes_index = 0;
  speed_echo_xpos[0] = samus_x_pos;
  speed_echo_xpos[1] = samus_x_pos;
  speed_echo_ypos[0] = samus_y_pos;
  speed_echo_ypos[1] = samus_y_pos;
  speed_echo_xspeed[2] = 0;
  samus_hurt_flash_counter = 0;
  QueueSfx1_Max6(0x35);
  QueueSfx3_Max6(0x10);
  return 1;
}

static Func_V *const kSamus_MoveHandler_ShinesparkCrash[3] = {  // 0x90D346
  Samus_MoveHandler_ShinesparkCrash_0,
  Samus_MoveHandler_ShinesparkCrash_1,
  Samus_MoveHandler_ShinesparkCrash_2,
};

void Samus_MoveHandler_ShinesparkCrash(void) {
  samus_shine_timer = 15;
  kSamus_MoveHandler_ShinesparkCrash[HIBYTE(speed_echoes_index)]();
  for (int i = 2; i >= 0; i -= 2) {
    int v1 = i >> 1;
    Point16U pt = Projectile_SinLookup(speed_echo_xspeed[v1], (uint8)speed_echoes_index);
    speed_echo_xpos[v1] = pt.x + samus_x_pos;
    speed_echo_ypos[v1] = pt.y + samus_y_pos;
  }
}

void Samus_MoveHandler_ShinesparkCrash_0(void) {  // 0x90D383
  uint16 v0 = speed_echoes_index + 4;
  if (!sign16(speed_echoes_index - 12))
    v0 |= 0x100;
  speed_echoes_index = v0;
}

void Samus_MoveHandler_ShinesparkCrash_1(void) {  // 0x90D396
  speed_echo_xspeed[0] = (uint8)(LOBYTE(speed_echo_xpos[2]) + LOBYTE(speed_echo_xspeed[0]));
  speed_echo_xspeed[1] = (uint8)(LOBYTE(speed_echo_xpos[2]) + LOBYTE(speed_echo_xspeed[1]));
  speed_echo_ypos[2] += 4;
  if (!sign16(speed_echo_ypos[2] - 128))
    speed_echoes_index = (uint8)speed_echoes_index | 0x200;
}

void Samus_MoveHandler_ShinesparkCrash_2(void) {  // 0x90D3CC
  speed_echoes_index -= 4;
  if (!(uint8)speed_echoes_index) {
    samus_movement_handler = FUNC16(Samus_MoveHandler_ShinesparkCrashEchoCircle);
    timer_for_shinesparks_startstop = 30;
    speed_echoes_index = 0;
    speed_echo_xspeed[0] = 0;
    speed_echo_xspeed[1] = 0;
  }
}

void Samus_MoveHandler_ShinesparkCrashEchoCircle(void) {  // 0x90D3F3
  samus_shine_timer = 15;
  bool v0 = (--timer_for_shinesparks_startstop & 0x8000) != 0;
  if (!timer_for_shinesparks_startstop || v0) {
    samus_movement_handler = FUNC16(Samus_MoveHandler_ShinesparkCrashFinish);
    samus_draw_handler = FUNC16(SamusDrawHandler_Default);
  }
}

void Samus_MoveHandler_ShinesparkCrashFinish(void) {  // 0x90D40D
  static const uint8 kShinesparkCrashFinish_Tab0[12] = { 0, 0x80, 0, 0x80, 0x40, 0xc0, 0x40, 0xc0, 0xe0, 0x60, 0x20, 0xa0 };
  speed_echoes_index = 0;
  if (sign16(projectile_counter - 5)) {
    if (sign16(projectile_counter - 4)) {
      ++projectile_counter;
      speed_echo_xspeed[2] = 64;
      speed_echo_xpos[2] = samus_x_pos;
      speed_echo_ypos[2] = samus_y_pos;
      projectile_type[3] = addr_loc_908029;
      InitializeShinesparkEchoOrSpazerSba(6);
      projectile_bomb_pre_instructions[3] = FUNC16(ProjPreInstr_SpeedEcho);
      projectile_variables[3] = kShinesparkCrashFinish_Tab0[(uint16)(2 * (samus_pose - 201))];
      projectile_bomb_x_speed[3] = 0;
    }
    ++projectile_counter;
    speed_echo_xspeed[3] = 64;
    speed_echo_xpos[3] = samus_x_pos;
    speed_echo_ypos[3] = samus_y_pos;
    projectile_type[4] = addr_loc_908029;
    InitializeShinesparkEchoOrSpazerSba(8);
    projectile_bomb_pre_instructions[4] = FUNC16(ProjPreInstr_SpeedEcho);
    projectile_variables[4] = kShinesparkCrashFinish_Tab0[(uint16)(2 * (samus_pose - 201)) + 1];
    projectile_bomb_x_speed[4] = 0;
  }
  cooldown_timer = 0;
  samus_shine_timer = 1;
  if (samus_pose_x_dir == kSamusPoseXDir_FaceRight)
    samus_new_pose_transitional = 2;
  else
    samus_new_pose_transitional = 1;
  samus_hurt_switch_index = 2;
  substate = 0;
  suit_pickup_light_beam_pos = 0;
}

void ProjPreInstr_SpeedEcho(uint16 k) {  // 0x90D4D2
  int16 v3;
  uint16 v4;
  int16 v5;

  int v1 = k >> 1;
  projectile_bomb_x_speed[v1] += 8;
  Point16U pt = Projectile_SinLookup(projectile_variables[v1], LOBYTE(projectile_bomb_x_speed[v1]));
  uint16 v2 = pt.x + samus_x_pos;
  *(uint16 *)((uint8 *)&speed_echoes_index + k) = pt.x + samus_x_pos;
  projectile_x_pos[v1] = v2;
  v3 = v2 - layer1_x_pos;
  if (v3 < 0
      || !sign16(v3 - 256)
      || (v4 = pt.y + samus_y_pos,
          speed_echo_xpos[v1 + 3] = pt.y + samus_y_pos,
          projectile_y_pos[v1] = v4,
          v5 = v4 - layer1_y_pos,
          v5 < 0)
      || !sign16(v5 - 256)) {
    speed_echo_ypos[v1 + 3] = 0;
    *(uint16 *)((uint8 *)&speed_echoes_index + k) = 0;
    speed_echo_xpos[v1 + 3] = 0;
    ClearProjectile(k);
  }
}

void Grapple_Func1(void) {  // 0x90D525
  bool v0;

  if ((button_config_shoot_x & joypad1_lastkeys) != 0
      && (v0 = (int16)(grapple_walljump_timer - 1) < 0, --grapple_walljump_timer, grapple_walljump_timer)
      && !v0) {
    if ((grapple_beam_length_delta & 0x8000) == 0) {
      grapple_beam_length += grapple_beam_length_delta;
      if (!sign16(grapple_beam_length - 96))
        grapple_beam_length_delta = -16;
    }
    Point16U pt = Projectile_SinLookup(HIBYTE(grapple_beam_end_angle16), grapple_beam_length);
    grapple_beam_end_x_pos = pt.x + x_pos_of_start_of_grapple_beam;
    grapple_beam_end_y_pos = pt.y + y_pos_of_start_of_grapple_beam;
    grapple_beam_end_angle16 += 2048;
    GrappleBeamFunc_BF1B();
  } else {
    grapple_beam_function = FUNC16(GrappleBeamFunc_Cancel);
  }
}

uint8 Hdmaobj_CrystalFlash(void) {  // 0x90D5A2
  SamusPose v1;

  if (sign16(game_state - 40)) {
    if (joypad1_lastkeys != (button_config_shoot_x | 0x430))
      return 1;
  }
  if (samus_y_speed
      || samus_y_subspeed
      || !sign16(samus_health - 51)
      || samus_reserve_health
      || sign16(samus_missiles - 10)
      || sign16(samus_super_missiles - 10)
      || sign16(samus_power_bombs - 10)) {
    return 1;
  }
  v1 = samus_pose_x_dir == kSamusPoseXDir_FaceRight ? kPose_D4_FaceL_CrystalFlash : kPose_D3_FaceR_CrystalFlash;
  samus_pose = v1;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
  if (samus_movement_type != 27)
    return 1;
  samus_last_different_pose = samus_prev_pose;
  *(uint16 *)&samus_last_different_pose_x_dir = *(uint16 *)&samus_prev_pose_x_dir;
  samus_prev_pose = samus_pose;
  *(uint16 *)&samus_prev_pose_x_dir = *(uint16 *)&samus_pose_x_dir;
  samus_movement_handler = FUNC16(SamusMoveHandler_CrystalFlashStart);
  if (samus_input_handler != FUNC16(Samus_InputHandler_E91D))
    samus_input_handler = FUNC16(nullsub_152);
  timer_for_shinesparks_startstop = 9;
  which_item_to_pickup = 0;
  substate = 10;
  suit_pickup_light_beam_pos = 0;
  *(uint16 *)&suit_pickup_color_math_R = 0;
  timer_for_shine_timer = 7;
  special_samus_palette_frame = 0;
  samus_shine_timer = 1;
  *(uint16 *)&suit_pickup_color_math_B = 1;
  samus_invincibility_timer = 0;
  samus_knockback_timer = 0;
  knockback_dir = 0;
  return 0;
}

void SamusMoveHandler_CrystalFlashStart(void) {  // 0x90D678
  samus_y_pos -= 2;
  if ((--timer_for_shinesparks_startstop & 0x8000) != 0) {
    samus_anim_frame_timer = 3;
    samus_anim_frame = 6;
    *(uint16 *)&suit_pickup_color_math_R = samus_y_pos;
    samus_movement_handler = FUNC16(SamusMoveHandler_CrystalFlashMain);
    samus_invincibility_timer = 0;
    samus_knockback_timer = 0;
    QueueSfx3_Max15(1);
    power_bomb_flag = 0;
    power_bomb_explosion_x_pos = samus_x_pos;
    power_bomb_explosion_y_pos = samus_y_pos;
    EnableHdmaObjects();
    SpawnCrystalFlashHdmaObjs();
  }
}

static Func_V *const kSamusMoveHandler_CrystalFlashMainFuncs[3] = {  // 0x90D6CE
  SamusMoveHandler_CrystalFlashMain_0,
  SamusMoveHandler_CrystalFlashMain_1,
  SamusMoveHandler_CrystalFlashMain_2,
};

void SamusMoveHandler_CrystalFlashMain(void) {
  kSamusMoveHandler_CrystalFlashMainFuncs[which_item_to_pickup]();
  samus_invincibility_timer = 0;
  samus_knockback_timer = 0;
}

void SamusMoveHandler_CrystalFlashMain_0(void) {  // 0x90D6E3
  if ((nmi_frame_counter_word & 7) == 0) {
    --samus_missiles;
    Samus_RestoreHealth(0x32);
    bool v0 = (int16)(substate - 1) < 0;
    if (!--substate || v0) {
      substate = 10;
      ++which_item_to_pickup;
    }
  }
}

void SamusMoveHandler_CrystalFlashMain_1(void) {  // 0x90D706
  if ((nmi_frame_counter_word & 7) == 0) {
    --samus_super_missiles;
    Samus_RestoreHealth(0x32);
    bool v0 = (int16)(substate - 1) < 0;
    if (!--substate || v0) {
      substate = 10;
      ++which_item_to_pickup;
    }
  }
}

void SamusMoveHandler_CrystalFlashMain_2(void) {  // 0x90D729
  if ((nmi_frame_counter_word & 7) == 0) {
    --samus_power_bombs;
    Samus_RestoreHealth(0x32);
    bool v0 = (int16)(substate - 1) < 0;
    if (!--substate || v0) {
      samus_movement_handler = FUNC16(kSamusMoveHandler_CrystalFlashFinish);
      samus_draw_handler = FUNC16(SamusDrawHandler_Default);
      samus_anim_frame_timer = 3;
      samus_anim_frame = 12;
    }
  }
}

void kSamusMoveHandler_CrystalFlashFinish(void) {  // 0x90D75B
  if (samus_y_pos != *(uint16 *)&suit_pickup_color_math_R)
    ++samus_y_pos;
  if (!samus_movement_type) {
    power_bomb_flag = 0;
    samus_shine_timer = -1;
    samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
    if (samus_input_handler != FUNC16(Samus_InputHandler_E91D)) {
      samus_input_handler = FUNC16(Samus_InputHandler_E913);
      samus_invincibility_timer = 0;
      samus_knockback_timer = 0;
    }
  }
}

static Func_U8 *const kSamusHitInterrupts[28] = {  // 0x90DDE9
  Samus_HitInterrupt_Stand,
  Samus_HitInterrupt_Stand,
  Samus_HitInterrupt_Stand,
  Samus_HitInterrupt_Stand,
  Samus_HitInterrupt_Ball,
  Samus_HitInterrupt_Stand,
  Samus_HitInterrupt_Falling,
  Samus_HitInterrupt_Unused,
  Samus_HitInterrupt_Ball,
  Samus_HitInterrupt_Ball,
  Samus_HitInterrupt_KnockbackGrapple,
  Samus_HitInterrupt_Turning,
  Samus_HitInterrupt_Turning,
  Samus_HitInterrupt_Stand,
  Samus_HitInterrupt_Turning,
  Samus_HitInterrupt_Turning,
  Samus_HitInterrupt_Stand,
  Samus_HitInterrupt_Ball,
  Samus_HitInterrupt_Ball,
  Samus_HitInterrupt_Ball,
  Samus_HitInterrupt_Stand,
  Samus_HitInterrupt_Stand,
  Samus_HitInterrupt_KnockbackGrapple,
  Samus_HitInterrupt_Turning,
  Samus_HitInterrupt_Turning,
  Samus_HitInterrupt_Turning,
  Samus_HitInterrupt_Turning,
  Samus_HitInterrupt_Shinespark,
};

static Func_V *const kSamus_MoveHandler_Knockback[6] = {  // 0x90DF38
  Samus_MoveHandler_Knockback_0,
  Samus_MoveHandler_Knockback_Up,
  Samus_MoveHandler_Knockback_Up,
  Samus_MoveHandler_Knockback_3,
  Samus_MoveHandler_Knockback_Down,
  Samus_MoveHandler_Knockback_Down,
};

static Func_U8 *const kSetupBombJump[28] = {  // 0x90DF99
  SetupBombJump_StandCrouch,
  SetupBombJump_1,
  SetupBombJump_2,
  SetupBombJump_2,
  SetupBombJump_4,
  SetupBombJump_StandCrouch,
  SetupBombJump_1,
  SetupBombJump_4,
  SetupBombJump_4,
  SetupBombJump_4,
  SetupBombJump_4,
  SetupBombJump_1,
  SetupBombJump_1,
  SetupBombJump_1,
  SetupBombJump_2,
  SetupBombJump_2,
  SetupBombJump_1,
  SetupBombJump_4,
  SetupBombJump_4,
  SetupBombJump_4,
  SetupBombJump_1,
  SetupBombJump_1,
  SetupBombJump_1,
  SetupBombJump_2,
  SetupBombJump_2,
  SetupBombJump_2,
  SetupBombJump_1A,
  SetupBombJump_1A,
};

static Func_V *const kSamus_MoveHandler_BombJumpMain[4] = {  // 0x90E032
  Samus_MoveHandler_Knockback_0,
  Samus_HorizontalBombJump,
  Samus_VerticalBombJump,
  Samus_HorizontalBombJump,
};

void Samus_HitInterruption(void) {
  if (samus_knockback_timer) {
    if (sign16(debug_invincibility - 7)) {
      if (!time_is_frozen_flag && !knockback_dir) {
        if (kSamusHitInterrupts[samus_movement_type]() & 1)
          samus_special_transgfx_index = 1;
      }
    } else {
      samus_invincibility_timer = 0;
      samus_knockback_timer = 0;
    }
  } else if (knockback_dir) {
    if (samus_movement_type == kPose_0A_MoveL_NoAim) {
      if (!sign16(flare_counter - 16))
        QueueSfx1_Max6(0x41);
      if (samus_pose_x_dir == kSamusPoseXDir_FaceRight)
        samus_new_pose_transitional = kPose_2A_FaceL_Fall;
      else
        samus_new_pose_transitional = kPose_29_FaceR_Fall;
    } else {
      if (samus_hurt_switch_index == 3) {
        samus_hurt_switch_index = 8;
        return;
      }
      samus_new_pose_transitional = samus_pose;
    }
    samus_hurt_switch_index = 1;
  } else if (bomb_jump_dir) {
    SetupBombJump();
  }
}

uint8 Samus_HitInterrupt_Shinespark(void) {  // 0x90DEBA
  if (samus_pose == kPose_E8_FaceR_Drained_CrouchFalling || samus_pose == kPose_E9_FaceL_Drained_CrouchFalling) {
    samus_anim_frame_timer = 17;
    samus_anim_frame = 26;
  }
  samus_special_transgfx_index = 0;
  knockback_dir = 0;
  return 0;
}

uint8 Samus_HitInterrupt_KnockbackGrapple(void) {  // 0x90DEDD
  samus_special_transgfx_index = 0;
  return 0;
}

uint8 Samus_HitInterrupt_Turning(void) {  // 0x90DEE2
  samus_special_transgfx_index = 0;
  knockback_dir = 0;
  return 0;
}

uint8 Samus_HitInterrupt_Falling(void) {  // 0x90DEEA
  if (frame_handler_gamma != FUNC16(Samus_Func9))
    return Samus_HitInterrupt_Stand();
  samus_special_transgfx_index = 0;
  knockback_dir = 0;
  return 0;
}

uint8 Samus_HitInterrupt_Stand(void) {  // 0x90DEFA
  if (samus_pose_x_dir == kSamusPoseXDir_FaceRight)
    samus_new_pose_interrupted = 84;
  else
    samus_new_pose_interrupted = 83;
  return 1;
}

uint8 Samus_HitInterrupt_Ball(void) {  // 0x90DF15
  samus_new_pose_interrupted = samus_pose;
  return 1;
}

uint8 Samus_HitInterrupt_Unused(void) {  // 0x90DF1D
  if (samus_pose_x_dir == kSamusPoseXDir_FaceRight)
    samus_new_pose_interrupted = 52;
  else
    samus_new_pose_interrupted = 51;
  return 1;
}

void Samus_MoveHandler_Knockback(void) {
  kSamus_MoveHandler_Knockback[knockback_dir]();
  input_to_pose_calc = 0;
}

void Samus_MoveHandler_Knockback_0(void) {  // 0x90DF50
  InvalidInterrupt_Crash();
}

void Samus_MoveHandler_Knockback_Up(void) {  // 0x90DF53
  Samus_BombJumpRisingXMovement_();
  Samus_MoveY_WithSpeedCalc();
  Samus_ClearMoveVars();
}

void Samus_MoveHandler_Knockback_3(void) {  // 0x90DF5D
  Samus_MoveY_WithSpeedCalc();
  Samus_ClearMoveVars();
}

void Samus_MoveHandler_Knockback_Down(void) {  // 0x90DF64
  Samus_BombJumpRisingXMovement_();
  Samus_Move_NoSpeedCalc_Y();
  Samus_ClearMoveVars();
}

void Samus_ClearMoveVars(void) {  // 0x90DF6E
  if (samus_collision_flag) {
    samus_x_accel_mode = 0;
    samus_collides_with_solid_enemy = 0;
    samus_is_falling_flag = 0;
    UNUSED_word_7E0B1A = 0;
    UNUSED_word_7E0B2A = 0;
    samus_y_subspeed = 0;
    samus_y_speed = 0;
    samus_y_dir = 0;
    UNUSED_word_7E0B38 = 0;
    samus_x_base_speed = 0;
    samus_x_base_subspeed = 0;
    Samus_AlignBottomWithPrevPose();
  }
}

void SetupBombJump(void) {
  if (!HIBYTE(bomb_jump_dir)) {
    if (kSetupBombJump[samus_movement_type]() & 1)
      samus_special_transgfx_index = 3;
  }
}

uint8 SetupBombJump_StandCrouch(void) {  // 0x90DFED
  if (!time_is_frozen_flag)
    return SetupBombJump_1();
  bomb_jump_dir = 0;
  return 0;
}

uint8 SetupBombJump_1(void) {  // 0x90DFF7
  if (samus_pose_x_dir == kSamusPoseXDir_FaceRight)
    samus_new_pose_interrupted = kPose_52_FaceL_Jump_NoAim_MoveF;
  else
    samus_new_pose_interrupted = kPose_51_FaceR_Jump_NoAim_MoveF;
  return 1;
}

uint8 SetupBombJump_4(void) {  // 0x90E012
  samus_new_pose_interrupted = samus_pose;
  return 1;
}

uint8 SetupBombJump_2(void) {  // 0x90E01A
  samus_input_handler = FUNC16(Samus_InputHandler_E913);
  return SetupBombJump_1A();
}

uint8 SetupBombJump_1A(void) {  // 0x90E020
  bomb_jump_dir = 0;
  return 0;
}

void Samus_MoveHandler_BombJumpStart(void) {  // 0x90E025
  Samus_InitBombJump();
  samus_movement_handler = FUNC16(Samus_MoveHandler_BombJumpMain);
  input_to_pose_calc = 0;
}

void Samus_MoveHandler_BombJumpMain(void) {
  if (bomb_jump_dir)
    kSamus_MoveHandler_BombJumpMain[(uint8)bomb_jump_dir]();
  else
    Samus_MoveHandler_BombJumpFunc1();
}

void Samus_HorizontalBombJump(void) {  // 0x90E04C
  Samus_BombJumpRisingXMovement_();
  Samus_BombJumpRisingYMovement_();
  if (samus_y_dir == 2 || (Samus_MoveY_WithSpeedCalc(), samus_collision_flag))
    Samus_MoveHandler_BombJumpFunc1();
}

void Samus_VerticalBombJump(void) {  // 0x90E066
  Samus_BombJumpRisingYMovement_();
  if (samus_y_dir == 2 || (Samus_MoveY_WithSpeedCalc(), samus_collision_flag))
    Samus_MoveHandler_BombJumpFunc1();
}

void Samus_MoveHandler_BombJumpFunc1(void) {  // 0x90E07D
  samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
  if (samus_input_handler != FUNC16(Samus_InputHandler_E91D))
    samus_input_handler = FUNC16(Samus_InputHandler_E913);
  bomb_jump_dir = 0;
}

void Samus_DrawHandler_EndOfShinespark(void) {  // 0x90EBF3
  Samus_Draw();
  for (int i = 2; i >= 0; i -= 2)
    Samus_DrawShinesparkCrashEchoes(i);
}

void Samus_PostGrappleCollisionDetect(void) {  // 0x90EF22
  PostGrappleCollisionDetect_X();
  PostGrappleCollisionDetect_Y();
  if (!distance_to_eject_samus_down || !distance_to_eject_samus_up) {
    if (distance_to_eject_samus_up) {
      samus_y_pos -= distance_to_eject_samus_up;
      if (!sign16(samus_y_radius - 16)) {
        PostGrappleCollisionDetect_Y();
        samus_y_pos -= distance_to_eject_samus_up;
      }
    }
  }
}

void Samus_Func25_ShineSpark(void) {  // 0x90F04B
  if (!samus_shine_timer) {
    special_samus_palette_frame = 6;
    special_samus_palette_timer = 1;
    demo_timer_counter = 1;
    timer_for_shine_timer = 10;
    samus_shine_timer = 120;
    samus_movement_handler = FUNC16(Samus_MoveHandler_F072);
  }
}

void Samus_MoveHandler_F072(void) {  // 0x90F072
  if (!samus_shine_timer) {
    frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func11);
    samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
  }
}

void MakeSamusFaceForward(void) {  // 0x91E3F6
  if (Samus_HasEquip(kSamusEquip_GravitySuit | kSamusEquip_VariaSuit))
    samus_pose = kPose_9B_FaceF_VariaGravitySuit;
  else
    samus_pose = kPose_00_FaceF_Powersuit;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
  samus_last_different_pose = samus_prev_pose;
  *(uint16 *)&samus_last_different_pose_x_dir = *(uint16 *)&samus_prev_pose_x_dir;
  samus_prev_pose = samus_pose;
  *(uint16 *)&samus_prev_pose_x_dir = *(uint16 *)&samus_pose_x_dir;
  if (samus_y_radius != 24) {
    samus_y_pos -= 3;
    samus_prev_y_pos = samus_y_pos;
  }
  frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func13);
  frame_handler_beta = FUNC16(SetContactDamageIndexAndUpdateMinimap);
  samus_new_pose = -1;
  samus_new_pose_interrupted = -1;
  samus_new_pose_transitional = -1;
  samus_momentum_routine_index = 0;
  samus_special_transgfx_index = 0;
  samus_hurt_switch_index = 0;
  CallSomeSamusCode(0x1F);
  SetHiLo(&samus_x_extra_run_speed, &samus_x_extra_run_subspeed, 0);
  samus_x_base_speed = 0;
  samus_x_base_subspeed = 0;
  samus_y_subspeed = 0;
  samus_y_speed = 0;
  samus_y_dir = 0;
  used_for_ball_bounce_on_landing = 0;
  samus_x_accel_mode = 0;
  flare_counter = 0;
  flare_animation_frame = 0;
  flare_slow_sparks_anim_frame = 0;
  flare_fast_sparks_anim_frame = 0;
  flare_animation_timer = 0;
  flare_slow_sparks_anim_timer = 0;
  flare_fast_sparks_anim_timer = 0;
  Samus_LoadSuitPalette();
}

static Func_U8 *const kSomeMotherBrainScripts[5] = {  // 0x91E4AD
  SomeMotherBrainScripts_0,
  SomeMotherBrainScripts_1,
  SomeMotherBrainScripts_2,
  SomeMotherBrainScripts_3_EnableHyperBeam,
  SomeMotherBrainScripts_4,
};

void SomeMotherBrainScripts(uint16 a) {
  if (kSomeMotherBrainScripts[a]() & 1) {
    samus_last_different_pose = samus_prev_pose;
    *(uint16 *)&samus_last_different_pose_x_dir = *(uint16 *)&samus_prev_pose_x_dir;
    samus_prev_pose = samus_pose;
    *(uint16 *)&samus_prev_pose_x_dir = *(uint16 *)&samus_pose_x_dir;
    samus_new_pose = -1;
    samus_new_pose_interrupted = -1;
    samus_new_pose_transitional = -1;
    samus_momentum_routine_index = 0;
    samus_special_transgfx_index = 0;
    samus_hurt_switch_index = 0;
  }
}

uint8 SomeMotherBrainScripts_0(void) {  // 0x91E4F8
  samus_y_pos -= 21 - samus_y_radius;
  samus_prev_y_pos = samus_y_pos;
  if (samus_pose_x_dir == kSamusPoseXDir_FaceRight)
    samus_pose = kPose_E9_FaceL_Drained_CrouchFalling;
  else
    samus_pose = kPose_E8_FaceR_Drained_CrouchFalling;
  samus_anim_frame_skip = 2;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
  samus_y_radius = kPoseParams[samus_pose].y_radius;
  samus_x_base_speed = 0;
  samus_x_base_subspeed = 0;
  samus_y_subspeed = 0;
  samus_y_speed = 0;
  samus_y_dir = 2;
  flare_counter = 0;
  flare_animation_frame = 0;
  flare_slow_sparks_anim_frame = 0;
  flare_fast_sparks_anim_frame = 0;
  flare_animation_timer = 0;
  flare_slow_sparks_anim_timer = 0;
  flare_fast_sparks_anim_timer = 0;
  Samus_LoadSuitPalette();
  return 1;
}

uint8 SomeMotherBrainScripts_1(void) {  // 0x91E571
  samus_anim_frame_timer = 16;
  samus_anim_frame = 0;
  if (samus_pose_x_dir == kSamusPoseXDir_FaceRight)
    samus_pose = kPose_EB_FaceL_Drained_Stand;
  else
    samus_pose = kPose_EA_FaceR_Drained_Stand;
  frame_handler_gamma = FUNC16(nullsub_152);
  return 1;
}

uint8 SomeMotherBrainScripts_2(void) {  // 0x91E59B
  if (samus_pose == kPose_E8_FaceR_Drained_CrouchFalling || samus_pose == kPose_E9_FaceL_Drained_CrouchFalling) {
    samus_anim_frame_timer = 1;
    samus_anim_frame = 13;
  } else if (samus_pose == kPose_EA_FaceR_Drained_Stand || samus_pose == kPose_EB_FaceL_Drained_Stand) {
    samus_anim_frame_timer = 1;
    samus_anim_frame = 4;
  }
  samus_y_radius = kPoseParams[samus_pose].y_radius;
  samus_x_base_speed = 0;
  samus_x_base_subspeed = 0;
  samus_y_subspeed = 0;
  samus_y_speed = 0;
  samus_y_dir = 2;
  return 1;
}

uint8 SomeMotherBrainScripts_3_EnableHyperBeam(void) {  // 0x91E5F0
  equipped_beams = 4105;
  UpdateBeamTilesAndPalette();
  SpawnPalfxObject(addr_stru_8DE1F0);
  hyper_beam_flag = FUNC16(Samus_InputHandler);
  play_resume_charging_beam_sfx = 0;
  return 0;
}

uint8 SomeMotherBrainScripts_4(void) {  // 0x91E60C
  samus_anim_frame_timer = 16;
  samus_anim_frame = 8;
  if (samus_pose_x_dir == kSamusPoseXDir_FaceRight)
    samus_pose = kPose_E9_FaceL_Drained_CrouchFalling;
  else
    samus_pose = kPose_E8_FaceR_Drained_CrouchFalling;
  return 1;
}
