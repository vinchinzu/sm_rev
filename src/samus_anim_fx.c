// Samus animation and atmospheric FX helpers: animation-delay state,
// liquid interaction visuals, footstep effects, and suit-palette updates.

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"

void nullsub_12(void) {}

static Func_V *const kSamusFxHandlers[8] = {
  Samus_Animate_NoFx,
  Samus_Animate_LavaFx,
  Samus_Animate_AcidFx,
  Samus_Animate_WaterFx,
  nullsub_12,
  nullsub_12,
  nullsub_12,
  nullsub_12,
};

typedef bool Func_AnimDelay(const uint8 *jp);

static uint8 Samus_HandleSpeedBoosterAnimDelay(const uint8 *jp);
static bool Samus_AnimDelayFunc_0to5(const uint8 *jp);
static bool Samus_AnimDelayFunc_6_GotoStartIfLittleHealth(const uint8 *jp);
static bool Samus_AnimDelayFunc_7(const uint8 *jp);
static bool Samus_AnimDelayFunc_13_TransToPose(const uint8 *jp);
static bool Samus_AnimDelayFunc_8_AutoJumpHack(const uint8 *jp);
static bool Samus_AnimDelayFunc_9_TransToPose(const uint8 *jp);
static bool UNUSED_Samus_AnimDelayFunc_10(const uint8 *jp);
static bool Samus_AnimDelayFunc_11_SelectDelaySequenceWalljump(const uint8 *jp);
static bool Samus_AnimDelayFunc_12_TransToPose(const uint8 *jp);
static bool Samus_AnimDelayFunc_14_Goto(const uint8 *jp);
static bool Samus_AnimDelayFunc_15_GotoStart(const uint8 *jp);

static Func_AnimDelay *const kAnimDelayFuncs[16] = {  // 0x9082DC
  Samus_AnimDelayFunc_0to5,
  Samus_AnimDelayFunc_0to5,
  Samus_AnimDelayFunc_0to5,
  Samus_AnimDelayFunc_0to5,
  Samus_AnimDelayFunc_0to5,
  Samus_AnimDelayFunc_0to5,
  Samus_AnimDelayFunc_6_GotoStartIfLittleHealth,
  Samus_AnimDelayFunc_7,
  Samus_AnimDelayFunc_8_AutoJumpHack,
  Samus_AnimDelayFunc_9_TransToPose,
  UNUSED_Samus_AnimDelayFunc_10,
  Samus_AnimDelayFunc_11_SelectDelaySequenceWalljump,
  Samus_AnimDelayFunc_12_TransToPose,
  Samus_AnimDelayFunc_13_TransToPose,
  Samus_AnimDelayFunc_14_Goto,
  Samus_AnimDelayFunc_15_GotoStart,
};

static FuncXY_V *const kAtmosphericTypeFuncs[8] = {  // 0x908A4C
  0,
  AtmosphericTypeFunc_1_FootstepSplash,
  AtmosphericTypeFunc_1_FootstepSplash,
  AtmosphericTypeFunc_3_DivingSplash,
  AtmosphericTypeFunc_4_LavaSurfaceDmg,
  AtmosphericTypeFunc_5_Bubbles,
  AtmosphericTypeFunc_67_Dust,
  AtmosphericTypeFunc_67_Dust,
};

static Func_V *const kSamus_FootstepGraphics[8] = {  // 0x90ED88
  Samus_FootstepGraphics_Crateria,
  Samus_FootstepGraphics_1,
  Samus_FootstepGraphics_1,
  Samus_FootstepGraphics_1,
  Samus_FootstepGraphics_Maridia,
  Samus_FootstepGraphics_1,
  Samus_FootstepGraphics_1,
  Samus_FootstepGraphics_1,
};

#define kDefaultAnimFramePtr ((uint16 *)RomFixedPtr(0x91B5D1))

void Samus_Animate(void) {  // 0x908000
  kSamusFxHandlers[(fx_type & 0xF) >> 1]();
  if (samus_pose == kPose_4D_FaceR_Jump_NoAim_NoMove_NoGun || samus_pose == kPose_4E_FaceL_Jump_NoAim_NoMove_NoGun) {
    if (samus_y_dir != 2 && samus_anim_frame == 1 && samus_anim_frame_timer == 1)
      samus_anim_frame_timer = 4;
    bool v1 = (--samus_anim_frame_timer & 0x8000) != 0;
    if (!samus_anim_frame_timer || v1) {
      ++samus_anim_frame;
      Samus_HandleAnimDelay();
    }
  } else {
    bool v0 = (--samus_anim_frame_timer & 0x8000) != 0;
    if (!samus_anim_frame_timer || v0) {
      ++samus_anim_frame;
      Samus_HandleAnimDelay();
    }
  }
}

void Samus_Animate_NoFx(void) {  // 0x908078
  uint16 r18 = Samus_GetBottom_R18();
  samus_anim_frame_buffer = samus_x_speed_divisor;
  if (liquid_physics_type) {
    if ((liquid_physics_type & 1) != 0) {
      liquid_physics_type = 0;
      QueueSfx2_Max6(0xE);
      if ((samus_suit_palette_index & 4) == 0 && (samus_movement_type == 3 || samus_movement_type == 20))
        QueueSfx1_Max6(0x30);
      Samus_SpawnWaterSplash(r18);
    } else {
      liquid_physics_type = 0;
    }
  }
}

void Samus_Animate_WaterFx(void) {  // 0x9080B8
  static const uint16 kSamusPhys_AnimDelayInWater = 3;
  uint16 r18 = Samus_GetBottom_R18();
  if ((fx_y_pos & 0x8000) == 0 && sign16(fx_y_pos - r18) && (fx_liquid_options & 4) == 0) {
    samus_anim_frame_buffer = kSamusPhys_AnimDelayInWater;
    if (liquid_physics_type == kLiquidPhysicsType_Water) {
      Samus_SpawnAirBubbles();
    } else {
      liquid_physics_type = kLiquidPhysicsType_Water;
      QueueSfx2_Max6(0xD);
      Samus_SpawnWaterSplash(r18);
    }
  } else {
    Samus_Animate_NoFx();
  }
}

void Samus_SpawnWaterSplash(uint16 r18) {  // 0x9080E6
  static const uint8 kWaterSplashTypeTable[28] = {
    1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
    1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
  };
  if (kWaterSplashTypeTable[samus_movement_type]) {
    atmospheric_gfx_frame_and_type[0] = 256;
    atmospheric_gfx_frame_and_type[1] = 256;
    atmospheric_gfx_anim_timer[0] = 3;
    atmospheric_gfx_anim_timer[1] = 3;
    atmospheric_gfx_x_pos[0] = samus_x_pos + 4;
    atmospheric_gfx_x_pos[1] = samus_x_pos - 3;
    atmospheric_gfx_y_pos[0] = r18 - 4;
    atmospheric_gfx_y_pos[1] = r18 - 4;
  } else {
    atmospheric_gfx_frame_and_type[0] = 768;
    atmospheric_gfx_anim_timer[0] = 2;
    atmospheric_gfx_x_pos[0] = samus_x_pos;
    atmospheric_gfx_y_pos[0] = fx_y_pos;
  }
  Samus_SpawnAirBubbles();
}

void Samus_SpawnAirBubbles() {  // 0x90813E
  uint16 r20 = Samus_GetTop_R20();
  if ((int16)(r20 - 24 - fx_y_pos) >= 0 && (nmi_frame_counter_word & 0x7F) == 0
      && !atmospheric_gfx_frame_and_type[2]) {
    atmospheric_gfx_frame_and_type[2] = 1280;
    atmospheric_gfx_anim_timer[2] = 3;
    atmospheric_gfx_x_pos[2] = samus_x_pos;
    atmospheric_gfx_y_pos[2] = samus_y_pos - samus_y_radius + 6;
    QueueSfx2_Max6((NextRandom() & 1) ? 15 : 17);
  }
  if (samus_pose == kPose_00_FaceF_Powersuit || samus_pose == kPose_9B_FaceF_VariaGravitySuit
      || (equipped_items & 0x20) != 0) {
    samus_anim_frame_buffer = 0;
  }
}

void Samus_Animate_LavaFx(void) {  // 0x9081C0
  static const uint16 kSamusPhys_LavaDamagePerFrame = 0;
  static const uint16 kSamusPhys_LavaSubdamagePerFrame = 0x8000;

  uint16 r18 = Samus_GetBottom_R18();
  if ((lava_acid_y_pos & 0x8000) == 0 && sign16(lava_acid_y_pos - r18)) {
    if (speed_boost_counter) {
      Samus_CancelSpeedBoost();
      samus_x_extra_run_speed = 0;
      samus_x_extra_run_subspeed = 0;
    }
    if ((equipped_items & 0x20) != 0) {
      samus_anim_frame_buffer = 0;
      liquid_physics_type = 2;
    } else {
      if ((game_time_frames & 7) == 0 && !sign16(samus_health - 71))
        QueueSfx3_Max3(0x2D);
      AddToHiLo(&samus_periodic_damage, &samus_periodic_subdamage, __PAIR32__(kSamusPhys_LavaDamagePerFrame, kSamusPhys_LavaSubdamagePerFrame));
      Samus_Animate_SubmergedLavaAcid();
    }
  } else {
    Samus_Animate_NoFx();
  }
}

void Samus_Animate_AcidFx(void) {  // 0x908219
  static const uint16 kSamusPhys_AcidSubdamagePerFrame = 0x8000;
  static const uint16 kSamusPhys_AcidDamagePerFrame = 1;
  uint16 r18 = Samus_GetBottom_R18();

  if ((lava_acid_y_pos & 0x8000) == 0 && sign16(lava_acid_y_pos - r18)) {
    if ((game_time_frames & 7) == 0 && !sign16(samus_health - 71))
      QueueSfx3_Max3(0x2D);
    AddToHiLo(&samus_periodic_damage, &samus_periodic_subdamage, __PAIR32__(kSamusPhys_AcidDamagePerFrame, kSamusPhys_AcidSubdamagePerFrame));
    Samus_Animate_SubmergedLavaAcid();
  } else {
    Samus_Animate_NoFx();
  }
}

void Samus_Animate_SubmergedLavaAcid(void) {  // 0x90824C
  static const uint16 kSamusPhys_AnimDelayInAcid = 2;
  uint16 r20 = Samus_GetTop_R20();
  samus_anim_frame_buffer = kSamusPhys_AnimDelayInAcid;
  liquid_physics_type = 2;
  if ((int16)(r20 - lava_acid_y_pos) < 0 && (atmospheric_gfx_frame_and_type[0] & 0x400) == 0) {
    atmospheric_gfx_frame_and_type[0] = 1024;
    atmospheric_gfx_frame_and_type[1] = 1024;
    atmospheric_gfx_frame_and_type[2] = 1024;
    atmospheric_gfx_frame_and_type[3] = 1024;
    atmospheric_gfx_anim_timer[0] = 3;
    atmospheric_gfx_anim_timer[3] = 3;
    atmospheric_gfx_anim_timer[1] = -32766;
    atmospheric_gfx_anim_timer[2] = -32766;
    atmospheric_gfx_y_pos[0] = lava_acid_y_pos;
    atmospheric_gfx_y_pos[1] = lava_acid_y_pos;
    atmospheric_gfx_y_pos[2] = lava_acid_y_pos;
    atmospheric_gfx_y_pos[3] = lava_acid_y_pos;
    atmospheric_gfx_x_pos[0] = samus_x_pos + 6;
    atmospheric_gfx_x_pos[1] = samus_x_pos;
    atmospheric_gfx_x_pos[2] = samus_x_pos;
    atmospheric_gfx_x_pos[3] = samus_x_pos - 6;
    if ((game_time_frames & 1) == 0)
      QueueSfx2_Max6(0x10);
  }
  if (samus_pose == kPose_00_FaceF_Powersuit || samus_pose == kPose_9B_FaceF_VariaGravitySuit || (equipped_items & 0x20) != 0) {
    samus_anim_frame_buffer = 0;
  }
}

static bool Samus_AnimDelayFunc_0to5(const uint8 *jp) {  // 0x908344
  return false;
}

static bool Samus_AnimDelayFunc_6_GotoStartIfLittleHealth(const uint8 *jp) {  // 0x908346
  samus_anim_frame = sign16(samus_health - 30) ? samus_anim_frame + 1 : 0;
  return true;
}

static bool Samus_AnimDelayFunc_7(const uint8 *jp) {  // 0x908360
  samus_movement_handler = FUNC16(Samus_HandleMovement_DrainedCrouching);
  samus_anim_frame++;
  return true;
}

static bool Samus_AnimDelayFunc_13_TransToPose(const uint8 *jp) {  // 0x9084B6
  samus_new_pose_transitional = jp[1];
  samus_hurt_switch_index = 3;
  return false;
}

static bool Samus_AnimDelayFunc_8_AutoJumpHack(const uint8 *jp) {  // 0x908370
  if (samus_input_handler == FUNC16(Samus_InputHandler_E91D))
    return Samus_AnimDelayFunc_13_TransToPose(jp);
  if (samus_new_pose != kPose_4B_FaceR_Jumptrans
      && samus_new_pose != kPose_4C_FaceL_Jumptrans
      && samus_new_pose != kPose_19_FaceR_SpinJump
      && samus_new_pose != kPose_1A_FaceL_SpinJump) {
    samus_input_handler = FUNC16(HandleAutoJumpHack);
    return Samus_AnimDelayFunc_13_TransToPose(jp);
  }
  return false;
}

static bool Samus_AnimDelayFunc_9_TransToPose(const uint8 *jp) {  // 0x90839A
  if ((GET_WORD(jp + 1) & equipped_items) != 0) {
    if (samus_y_speed || samus_y_subspeed)
      samus_new_pose_transitional = jp[6];
    else
      samus_new_pose_transitional = jp[5];
  } else if (samus_y_speed || samus_y_subspeed) {
    samus_new_pose_transitional = jp[4];
  } else {
    samus_new_pose_transitional = jp[3];
  }
  samus_hurt_switch_index = 3;
  return false;
}

static bool UNUSED_Samus_AnimDelayFunc_10(const uint8 *jp) {  // 0x9083F6
  if (samus_y_speed || samus_y_subspeed)
    samus_new_pose_transitional = jp[2];
  else
    samus_new_pose_transitional = jp[1];
  samus_hurt_switch_index = 3;
  return false;
}

static bool Samus_AnimDelayFunc_11_SelectDelaySequenceWalljump(const uint8 *jp) {  // 0x90841D
  if ((equipped_items & 0x20) == 0) {
    uint16 r20 = Samus_GetTop_R20();
    if ((fx_y_pos & 0x8000) != 0) {
      if ((lava_acid_y_pos & 0x8000) == 0 && sign16(lava_acid_y_pos - r20))
        goto LABEL_10;
    } else if (sign16(fx_y_pos - r20) && (fx_liquid_options & 4) == 0) {
      goto LABEL_10;
    }
  }
  if ((equipped_items & 8) != 0) {
    QueueSfx1_Max6(0x33);
    samus_anim_frame += 21;
  } else {
    if ((equipped_items & 0x200) == 0) {
LABEL_10:
      QueueSfx1_Max6(0x31);
      samus_anim_frame += 1;
    } else {
      QueueSfx1_Max6(0x3E);
      samus_anim_frame += 11;
    }
  }
  return true;
}

static bool Samus_AnimDelayFunc_12_TransToPose(const uint8 *jp) {  // 0x90848B
  if ((GET_WORD(jp + 1) & equipped_items) != 0)
    samus_new_pose_transitional = jp[4];
  else
    samus_new_pose_transitional = jp[3];
  samus_hurt_switch_index = 3;
  return false;
}

static bool Samus_AnimDelayFunc_14_Goto(const uint8 *jp) {  // 0x9084C7
  samus_anim_frame -= jp[1];
  return true;
}

static bool Samus_AnimDelayFunc_15_GotoStart(const uint8 *jp) {  // 0x9084DB
  samus_anim_frame = 0;
  return true;
}

static uint8 Samus_HandleSpeedBoosterAnimDelay(const uint8 *jp) {  // 0x90852C
  if (!samus_has_momentum_flag || (button_config_run_b & joypad1_lastkeys) == 0 || samus_movement_type != 1)
    return jp[0];
  if ((equipped_items & 0x2000) == 0) {
    samus_anim_frame = 0;
    samus_anim_frame_timer = samus_anim_frame_buffer + RomPtr_91(*kDefaultAnimFramePtr)[0];
    return 0;
  }
  if ((uint8)--speed_boost_counter)
    return jp[0];
  uint16 v2 = speed_boost_counter;
  if ((speed_boost_counter & 0x400) == 0) {
    v2 = speed_boost_counter + 256;
    speed_boost_counter = v2;
    if ((v2 & 0x400) != 0) {
      samus_echoes_sound_flag = 1;
      QueueSfx3_Max6(3);
    }
  }
  int v3 = HIBYTE(v2);
  speed_boost_counter = kSpeedBoostToCtr[v3] | speed_boost_counter & 0xFF00;
  samus_anim_frame = 0;
  samus_anim_frame_timer = samus_anim_frame_buffer + RomPtr_91(kSpeedBoostToAnimFramePtr[v3])[0];
  return 0;
}

void Samus_HandleAnimDelay(void) {
  const uint8 *p = RomPtr_91(kSamusAnimationDelayData[samus_pose]);
  if ((p[samus_anim_frame] & 0x80) != 0) {
    uint8 v1 = Samus_HandleSpeedBoosterAnimDelay(p + samus_anim_frame);
    if (kAnimDelayFuncs[v1 & 0xF](p + samus_anim_frame))
      samus_anim_frame_timer = samus_anim_frame_buffer + p[samus_anim_frame];
  } else {
    if (samus_has_momentum_flag && samus_movement_type == 1) {
      uint16 addr = ((equipped_items & 0x2000) != 0) ? kSpeedBoostToAnimFramePtr[HIBYTE(speed_boost_counter)] : *kDefaultAnimFramePtr;
      p = RomPtr_91(addr);
    }
    samus_anim_frame_timer = samus_anim_frame_buffer + p[samus_anim_frame];
  }
}

void HandleAtmosphericEffects(void) {
  int16 v4;

  for (int i = 6; i >= 0; i -= 2) {
    int v1 = i >> 1;
    uint16 v2 = atmospheric_gfx_frame_and_type[v1];
    if (!v2)
      continue;
    uint16 r18 = 2 * (uint8)v2;
    uint16 v3 = 2 * HIBYTE(v2);
    v4 = atmospheric_gfx_anim_timer[v1] - 1;
    atmospheric_gfx_anim_timer[v1] = v4;
    if (v4) {
      if (v4 < 0) {
        if (v4 != (int16)0x8000)
          continue;
        atmospheric_gfx_anim_timer[v1] = *(uint16 *)RomPtr_90(r18 + kAtmosphericGraphicAnimationTimers[v3 >> 1]);
      }
    } else {
      atmospheric_gfx_anim_timer[v1] = *(uint16 *)RomPtr_90(r18 + kAtmosphericGraphicAnimationTimers[v3 >> 1]);
      uint16 v5 = atmospheric_gfx_frame_and_type[v1] + 1;
      atmospheric_gfx_frame_and_type[v1] = v5;
      if ((int16)((uint8)v5 - kAtmosphericTypeNumFrames[v3 >> 1]) >= 0) {
        atmospheric_gfx_frame_and_type[v1] = 0;
        continue;
      }
    }
    kAtmosphericTypeFuncs[v3 >> 1](v3, i);
  }
}

void AtmosphericTypeFunc_1_FootstepSplash(uint16 k, uint16 j) {  // 0x908AC5
  int16 v4;
  OamEnt *v5;
  int16 v6;

  int v2 = j >> 1;
  uint16 r18 = 2 * LOBYTE(atmospheric_gfx_frame_and_type[v2]);
  uint16 v3 = oam_next_ptr;
  v4 = atmospheric_gfx_x_pos[v2] - layer1_x_pos - 4;
  if (v4 >= 0) {
    if (sign16(atmospheric_gfx_x_pos[v2] - layer1_x_pos - 260)) {
      v5 = gOamEnt(oam_next_ptr);
      v5->xcoord = v4;
      v6 = atmospheric_gfx_y_pos[v2] - layer1_y_pos - 4;
      if (v6 >= 0) {
        if (sign16(atmospheric_gfx_y_pos[v2] - layer1_y_pos - 260)) {
          v5->ycoord = v6;
          *(uint16 *)&v5->charnum = *(uint16 *)RomPtr_90(r18 + g_off_908BFF[k >> 1]);
          oam_next_ptr = v3 + 4;
        }
      }
    }
  }
}

void AtmosphericTypeFunc_Common(uint16 j, uint16 a) {  // 0x908B74
  int v1 = j >> 1;
  if (((atmospheric_gfx_y_pos[v1] - layer1_y_pos) & 0xFF00) == 0)
    DrawSamusSpritemap(a, atmospheric_gfx_x_pos[v1] - layer1_x_pos, atmospheric_gfx_y_pos[v1] - layer1_y_pos);
}

void AtmosphericTypeFunc_3_DivingSplash(uint16 k, uint16 j) {  // 0x908B16
  int v2 = j >> 1;
  uint16 r18 = LOBYTE(atmospheric_gfx_frame_and_type[v2]);
  atmospheric_gfx_y_pos[v2] = fx_y_pos;
  AtmosphericTypeFunc_Common(j, r18 + 399);
}

void AtmosphericTypeFunc_4_LavaSurfaceDmg(uint16 k, uint16 j) {  // 0x908B2E
  int v2 = j >> 1;
  uint16 v3;
  if ((j & 4) != 0)
    v3 = atmospheric_gfx_x_pos[v2] - 1;
  else
    v3 = atmospheric_gfx_x_pos[v2] + 1;
  atmospheric_gfx_x_pos[v2] = v3;
  --atmospheric_gfx_y_pos[j >> 1];
  AtmosphericTypeFunc_1_FootstepSplash(k, j);
}

void AtmosphericTypeFunc_67_Dust(uint16 k, uint16 j) {  // 0x908B57
  --atmospheric_gfx_y_pos[j >> 1];
  AtmosphericTypeFunc_1_FootstepSplash(k, j);
}

void AtmosphericTypeFunc_5_Bubbles(uint16 k, uint16 j) {  // 0x908B64
  uint16 r18 = LOBYTE(atmospheric_gfx_frame_and_type[j >> 1]);
  AtmosphericTypeFunc_Common(j, r18 + 390);
}

void SetLiquidPhysicsType(void) {
  uint16 r18 = Samus_GetBottom_R18();
  switch ((fx_type & 0xF) >> 1) {
  case 1:
  case 2:
    if ((lava_acid_y_pos & 0x8000) == 0 && sign16(lava_acid_y_pos - r18))
      liquid_physics_type = kLiquidPhysicsType_LavaAcid;
    else
      liquid_physics_type = 0;
    break;
  case 3:
    if ((fx_y_pos & 0x8000) == 0 && sign16(fx_y_pos - r18) && (fx_liquid_options & 4) == 0)
      liquid_physics_type = kLiquidPhysicsType_Water;
    else
      liquid_physics_type = 0;
    break;
  default:
    liquid_physics_type = 0;
  }
}

void Samus_UpdateSuitPaletteIndex(void) {  // 0x90ECB6
  if ((equipped_items & 0x20) != 0) {
    samus_suit_palette_index = 4;
  } else if ((equipped_items & 1) != 0) {
    samus_suit_palette_index = 2;
  } else {
    samus_suit_palette_index = 0;
  }
}

void Samus_FootstepGraphics(void) {
  kSamus_FootstepGraphics[area_index]();
}

void Samus_FootstepGraphics_Crateria(void) {
  static const uint8 byte_90EDC9[16] = {  // 0x90EDA1
    1, 0, 0, 0, 0, 2, 0, 4,
    0, 4, 4, 4, 4, 0, 4, 0,
  };
  if (cinematic_function || (int16)(room_index - 16) >= 0)
    goto LABEL_11;
  if ((byte_90EDC9[room_index] & 1) == 0) {
    if ((byte_90EDC9[room_index] & 2) != 0) {
      if (!sign16(samus_y_pos - 944))
        goto LABEL_12;
    } else if ((byte_90EDC9[room_index] & 4) != 0) {
      goto LABEL_12;
    }
LABEL_11:
    Samus_FootstepGraphics_1();
    return;
  }
  if (fx_type != 10)
    goto LABEL_11;
LABEL_12:
  Samus_FootstepGraphics_Maridia();
}

void Samus_FootstepGraphics_Maridia(void) {  // 0x90EDEC
  uint16 r18 = Samus_GetBottom_R18();
  if ((fx_y_pos & 0x8000) != 0) {
    if ((lava_acid_y_pos & 0x8000) == 0 && sign16(lava_acid_y_pos - r18))
      return;
  } else if (sign16(fx_y_pos - r18) && (fx_liquid_options & 4) == 0) {
    return;
  }
  if (samus_pose_x_dir == 4) {
    atmospheric_gfx_x_pos[0] = samus_x_pos - 12;
    atmospheric_gfx_x_pos[1] = samus_x_pos + 8;
  } else {
    atmospheric_gfx_x_pos[0] = samus_x_pos + 12;
    atmospheric_gfx_x_pos[1] = samus_x_pos - 8;
  }
  atmospheric_gfx_y_pos[0] = samus_y_pos + 16;
  atmospheric_gfx_y_pos[1] = samus_y_pos + 16;
  atmospheric_gfx_frame_and_type[0] = 256;
  atmospheric_gfx_frame_and_type[1] = 256;
  atmospheric_gfx_anim_timer[0] = -32766;
  atmospheric_gfx_anim_timer[1] = 3;
}

void Samus_FootstepGraphics_1(void) {  // 0x90EE64
  if ((speed_boost_counter & 0xFF00) == 1024) {
    uint16 r18 = Samus_GetBottom_R18();
    if ((fx_y_pos & 0x8000) != 0) {
      if ((lava_acid_y_pos & 0x8000) == 0 && sign16(lava_acid_y_pos - r18))
        return;
    } else if (sign16(fx_y_pos - r18) && (fx_liquid_options & 4) == 0) {
      return;
    }
    if (samus_pose_x_dir == 4) {
      atmospheric_gfx_x_pos[0] = samus_x_pos - 12;
      atmospheric_gfx_x_pos[1] = samus_x_pos + 8;
    } else {
      atmospheric_gfx_x_pos[0] = samus_x_pos + 12;
      atmospheric_gfx_x_pos[1] = samus_x_pos - 8;
    }
    atmospheric_gfx_y_pos[0] = samus_y_pos + 16;
    atmospheric_gfx_y_pos[1] = samus_y_pos + 16;
    atmospheric_gfx_frame_and_type[0] = 1792;
    atmospheric_gfx_frame_and_type[1] = 1792;
    atmospheric_gfx_anim_timer[0] = -32766;
    atmospheric_gfx_anim_timer[1] = 3;
  }
}
