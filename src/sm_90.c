// Samus

#include "ida_types.h"
#include "variables.h"
#include "variables_extra.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "multi_samus.h"
#include "physics_config.h"

#define kSamusFramesForUnderwaterSfx ((uint8*)RomFixedPtr(0x90a514))
#define kPauseMenuMapData ((uint16*)RomFixedPtr(0x829717))
#define kPauseMenuMapTilemaps ((LongPtr*)RomFixedPtr(0x82964a))

static uint8 GetPackedOamExtBits(uint16 idx);
static void SetPackedOamExtBits(uint16 idx, uint8 bits);
static void DrawPad2DronePing(const OamEnt *src, uint8 src_ext_bits, int x, int y);
static void DrawPad2Drone(uint16 samus_oam_start, uint16 samus_oam_end);

static void Samus_HandleAnimDelay(void);

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

static uint8 Samus_HandleSpeedBoosterAnimDelay(const uint8 *jp);


typedef bool Func_AnimDelay(const uint8 *jp);

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


#define kDefaultAnimFramePtr ((uint16 *)RomFixedPtr(0x91B5D1))

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
      // The original code forgets to preserve A here.
      samus_echoes_sound_flag = 1;
      QueueSfx3_Max6(3);
      //v2 = 0x103; // bug!
    }
  }
  int v3 = HIBYTE(v2);
  speed_boost_counter = kSpeedBoostToCtr[v3] | speed_boost_counter & 0xFF00;
  samus_anim_frame = 0;
  samus_anim_frame_timer = samus_anim_frame_buffer + RomPtr_91(kSpeedBoostToAnimFramePtr[v3])[0];
  return 0;
}

static void Samus_HandleAnimDelay(void) {
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

static Func_U8 *const kSamusIsBottomDrawnFuncs[28] = {  // 0x9085E2
  SamusBottomDrawn_0_Standing,
  SamusBottomDrawn_1,
  SamusBottomDrawn_1,
  SamusBottomDrawn_3_SpinJump,
  SamusBottomDrawn_4,
  SamusBottomDrawn_1,
  SamusBottomDrawn_1,
  SamusBottomDrawn_4,
  SamusBottomDrawn_4,
  SamusBottomDrawn_4,
  SamusBottomDrawn_A_Knockback,
  SamusBottomDrawn_1,
  SamusBottomDrawn_1,
  UNUSED_SamusBottomDrawn_D,
  SamusBottomDrawn_1,
  SamusBottomDrawn_F_Transitions,
  SamusBottomDrawn_1,
  SamusBottomDrawn_4,
  SamusBottomDrawn_4,
  SamusBottomDrawn_4,
  SamusBottomDrawn_14_WallJump,
  SamusBottomDrawn_1,
  SamusBottomDrawn_1,
  SamusBottomDrawn_1,
  SamusBottomDrawn_1,
  SamusBottomDrawn_19_DamageBoost,
  SamusBottomDrawn_1,
  SamusBottomDrawn_1B,
};

void Samus_Draw(void) {
  PairU16 v0;

  if (samus_knockback_timer || !samus_invincibility_timer || samus_shine_timer || (nmi_frame_counter_word & 1) == 0) {
    uint16 v2 = 2 * samus_pose;
    samus_top_half_spritemap_index = samus_anim_frame
      + kSamusPoseToBaseSpritemapIndexTop[samus_pose];
    uint16 a = samus_top_half_spritemap_index;
    v0 = Samus_CalcSpritemapPos(2 * samus_pose);
    DrawSamusSpritemap(a, v0.k, v0.j);
    uint16 R36 = v2;
    if (kSamusIsBottomDrawnFuncs[samus_movement_type]() & 1) {
      samus_bottom_half_spritemap_index = samus_anim_frame + kSamusPoseToBaseSpritemapIndexBottom[R36 >> 1];
      DrawSamusSpritemap(samus_bottom_half_spritemap_index, samus_spritemap_x_pos, samus_spritemap_y_pos);
    }
  }
  SetSamusTilesDefsForCurAnim();
}

uint8 SamusBottomDrawn_1(void) {  // 0x908686
  return 1;
}

uint8 SamusBottomDrawn_4(void) {  // 0x908688
  samus_bottom_half_spritemap_index = 0;
  return 0;
}

uint8 SamusBottomDrawn_0_Standing(void) {  // 0x90868D
  OamEnt *v1;

  if (samus_pose == kPose_00_FaceF_Powersuit) {
    uint16 v0 = oam_next_ptr;
    v1 = gOamEnt(oam_next_ptr);
    *(uint16 *)&v1->xcoord = samus_x_pos - 7 - layer1_x_pos;
    *(uint16 *)&v1->ycoord = samus_y_pos - 17 - layer1_y_pos;
    *(uint16 *)&v1->charnum = 14369;
    oam_next_ptr = v0 + 4;
  }
  return 1;
}

uint8 SamusBottomDrawn_3_SpinJump(void) {  // 0x9086C6
  if (samus_pose == kPose_81_FaceR_Screwattack
      || samus_pose == kPose_82_FaceL_Screwattack
      || samus_pose == kPose_1B_FaceR_SpaceJump
      || samus_pose == kPose_1C_FaceL_SpaceJump
      || !samus_anim_frame
      || !sign16(samus_anim_frame - kPose_0B_MoveR_Gun)) {
    return 1;
  }
  samus_bottom_half_spritemap_index = 0;
  return 0;
}

uint8 SamusBottomDrawn_A_Knockback(void) {  // 0x9086EE
  if (samus_pose != kPose_D7_FaceR_CrystalFlashEnd && samus_pose != kPose_D8_FaceL_CrystalFlashEnd
      || !sign16(samus_anim_frame - 3)) {
    return 1;
  }
  samus_bottom_half_spritemap_index = 0;
  return 0;
}

uint8 SamusBottomDrawn_F_Transitions(void) {  // 0x90870C
  if (!sign16(samus_pose - kPose_F1_FaceR_CrouchTrans_AimU))
    return 1;
  if (!sign16(samus_pose - 219)) {
    if (sign16(samus_pose - 221)) {
      if (samus_anim_frame)
        goto LABEL_8;
    } else if (samus_anim_frame != 2) {
      goto LABEL_8;
    }
    return 1;
  }
  if (samus_pose == kPose_35_FaceR_CrouchTrans
      || samus_pose == kPose_36_FaceL_CrouchTrans
      || samus_pose == kPose_3B_FaceR_StandTrans
      || samus_pose == kPose_3C_FaceL_StandTrans) {
    return 1;
  }
LABEL_8:
  samus_bottom_half_spritemap_index = 0;
  return 0;
}

uint8 UNUSED_SamusBottomDrawn_D(void) {  // 0x90874C
  if (samus_pose != (kPose_44_FaceL_Turn_Crouch | kPose_01_FaceR_Normal | 0x20)
      && samus_pose != (kPose_44_FaceL_Turn_Crouch | kPose_02_FaceL_Normal | 0x20)
      || sign16(samus_anim_frame - 1)) {
    return 1;
  }
  samus_bottom_half_spritemap_index = 0;
  return 0;
}

uint8 SamusBottomDrawn_14_WallJump(void) {  // 0x908768
  if (sign16(samus_anim_frame - 3) || !sign16(samus_anim_frame - 13))
    return 1;
  samus_bottom_half_spritemap_index = 0;
  return 0;
}

uint8 SamusBottomDrawn_19_DamageBoost(void) {  // 0x90877C
  if (sign16(samus_anim_frame - 2) || !sign16(samus_anim_frame - 9))
    return 1;
  samus_bottom_half_spritemap_index = 0;
  return 0;
}

uint8 SamusBottomDrawn_1B(void) {  // 0x908790
  if (sign16(samus_pose - kPose_CF_FaceR_Ranintowall_AimUR)) {
    if (samus_pose != kPose_CB_FaceR_Shinespark_Vert && samus_pose != kPose_CC_FaceL_Shinespark_Vert)
      return 1;
  } else if (samus_pose != kPose_E8_FaceR_Drained_CrouchFalling && samus_pose != kPose_E9_FaceL_Drained_CrouchFalling
             || !sign16(samus_anim_frame - 2)) {
    return 1;
  }
  samus_bottom_half_spritemap_index = 0;
  return 0;
}

void Samus_DrawEchoes(void) {  // 0x9087BD
  if ((speed_echoes_index & 0x8000) == 0) {
    if ((speed_boost_counter & 0xFF00) == 1024) {
      if (speed_echo_xpos[1])
        Samus_DrawEcho(2);
      if (speed_echo_xpos[0])
        Samus_DrawEcho(0);
    }
    return;
  }
  for (int i = 2; i >= 0; i -= 2) {
    int v1 = i >> 1;
    if (!speed_echo_xpos[v1])
      continue;
    uint16 v2 = speed_echo_ypos[v1], v3;
    if (v2 != samus_y_pos) {
      if ((int16)(v2 - samus_y_pos) < 0)
        v3 = v2 + 2;
      else
        v3 = v2 - 2;
      speed_echo_ypos[v1] = v3;
    }
    if ((speed_echo_xspeed[v1] & 0x8000) != 0) {
      uint16 v5 = speed_echo_xspeed[v1] + speed_echo_xpos[v1];
      speed_echo_xpos[v1] = v5;
      if ((int16)(v5 - samus_x_pos) < 0) {
        speed_echo_xpos[v1] = 0;
        continue;
      }
    } else {
      uint16 v4 = speed_echo_xspeed[v1] + speed_echo_xpos[v1];
      speed_echo_xpos[v1] = v4;
      if ((int16)(v4 - samus_x_pos) >= 0) {
        speed_echo_xpos[v1] = 0;
        continue;
      }
    }
    Samus_DrawEcho(i);
  }
  if (!speed_echo_xpos[1] && !speed_echo_xpos[0])
    speed_echoes_index = 0;
}

void Samus_DrawEcho(uint16 j) {  // 0x908855
  int v1 = j >> 1;
  int16 v2 = speed_echo_ypos[v1] - kPoseParams[samus_pose].y_offset_to_gfx - layer1_y_pos;
  if (v2 >= 0 && sign16(v2 - 248)) {
    DrawSamusSpritemap(samus_top_half_spritemap_index, speed_echo_xpos[v1] - layer1_x_pos, v2);
    if (samus_bottom_half_spritemap_index)
      DrawSamusSpritemap(samus_bottom_half_spritemap_index, speed_echo_xpos[v1] - layer1_x_pos, v2);
  }
}

void Samus_DrawShinesparkCrashEchoes(uint16 k) {  // 0x9088BA
  if ((nmi_frame_counter_word & 1) != 0) {
    uint16 a = samus_anim_frame + kSamusPoseToBaseSpritemapIndexTop[samus_pose];
    int16 v2 = speed_echo_ypos[k >> 1] - kPoseParams[samus_pose].y_offset_to_gfx - layer1_y_pos;
    if (v2 >= 0 && sign16(v2 - 248)) {
      DrawSamusSpritemap(a, speed_echo_xpos[k >> 1] - layer1_x_pos, v2);
      if (kSamusIsBottomDrawnFuncs[samus_movement_type]() & 1) {
        uint16 v5 = samus_anim_frame + kSamusPoseToBaseSpritemapIndexBottom[samus_pose];
        DrawSamusSpritemap(v5, speed_echo_xpos[k >> 1] - layer1_x_pos, v2);
      }
    }
  }
}

void Samus_DrawShinesparkCrashEchoProjectiles(void) {  // 0x908953
  if ((nmi_frame_counter_word & 1) != 0) {
    if (speed_echo_xspeed[3])
      Samus_DrawEcho(6);
    if (speed_echo_xspeed[2])
      Samus_DrawEcho(4);
  }
}

void Samus_DrawStartingDeathAnim(void) {  // 0x908976
  bool v0 = (--samus_anim_frame_timer & 0x8000) != 0;
  if (!samus_anim_frame_timer || v0) {
    ++samus_anim_frame;
    Samus_HandleAnimDelay();
  }
  Samus_DrawDuringDeathAnim();
}

void Samus_DrawDuringDeathAnim(void) {  // 0x908998
  uint16 v1 = 2 * samus_pose;
  uint16 a = samus_anim_frame + kSamusPoseToBaseSpritemapIndexTop[samus_pose];
  Samus_CalcSpritemapPos(2 * samus_pose);
  DrawSamusSpritemap(a, layer1_x_pos + samus_spritemap_x_pos, layer1_y_pos + samus_spritemap_y_pos);
  uint16 R36 = v1;
  if (kSamusIsBottomDrawnFuncs[samus_movement_type]() & 1)
    DrawSamusSpritemap(
      samus_anim_frame + kSamusPoseToBaseSpritemapIndexBottom[R36 >> 1],
      layer1_x_pos + samus_spritemap_x_pos,
      layer1_y_pos + samus_spritemap_y_pos);
  SetSamusTilesDefsForCurAnim();
}

void Samus_DrawWhenNotAnimatingOrDying(void) {  // 0x908A00
  PairU16 v0;

  uint16 v2 = 2 * samus_pose;
  uint16 a = samus_anim_frame + kSamusPoseToBaseSpritemapIndexTop[samus_pose];
  v0 = Samus_CalcSpritemapPos(2 * samus_pose);
  DrawSamusSpritemap(a, v0.k, v0.j);
  uint16 R36 = v2;
  if (kSamusIsBottomDrawnFuncs[samus_movement_type]() & 1)
    DrawSamusSpritemap(
      samus_anim_frame + kSamusPoseToBaseSpritemapIndexBottom[R36 >> 1],
      samus_spritemap_x_pos,
      samus_spritemap_y_pos);
  SetSamusTilesDefsForCurAnim();
}

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

static Func_Y_To_PairU16 *const kSamus_CalcSpritemapPos[28] = {  // 0x908C1F
  &Samus_CalcSpritemapPos_Standing,
  &Samus_CalcSpritemapPos_Default,
  &Samus_CalcSpritemapPos_Default,
  &Samus_CalcSpritemapPos_Default,
  &Samus_CalcSpritemapPos_Default,
  &Samus_CalcSpritemapPos_Default,
  &Samus_CalcSpritemapPos_Default,
  &Samus_CalcSpritemapPos_Default,
  &Samus_CalcSpritemapPos_Default,
  &Samus_CalcSpritemapPos_Default,
  &Samus_CalcSpritemapPos_Default,
  &Samus_CalcSpritemapPos_Default,
  &Samus_CalcSpritemapPos_Default,
  &Samus_CalcSpritemapPos_Default,
  &Samus_CalcSpritemapPos_Default,
  &Samus_CalcSpritemapPos_Crouch,
  &Samus_CalcSpritemapPos_Default,
  &Samus_CalcSpritemapPos_Default,
  &Samus_CalcSpritemapPos_Default,
  &Samus_CalcSpritemapPos_Default,
  &Samus_CalcSpritemapPos_Default,
  &Samus_CalcSpritemapPos_Default,
  &Samus_CalcSpritemapPos_Default,
  &Samus_CalcSpritemapPos_Default,
  &Samus_CalcSpritemapPos_Default,
  &Samus_CalcSpritemapPos_Default,
  &Samus_CalcSpritemapPos_Default,
  &Samus_CalcSpritemapPos_Special,
};

PairU16 Samus_CalcSpritemapPos(uint16 k) {
  PairU16 v1;

  if ((ceres_status & 0x8000) == 0) {
    v1 = kSamus_CalcSpritemapPos[samus_movement_type](k);
  } else {
    uint16 old_x = samus_x_pos, old_y = samus_y_pos;
    Samus_CalcPos_Mode7();
    v1 = kSamus_CalcSpritemapPos[samus_movement_type](k);
    samus_y_pos = old_y;
    samus_x_pos = old_x;
  }
  return MakePairU16(v1.k, v1.j);
}

PairU16 Samus_CalcSpritemapPos_Default(uint16 j) {  // 0x908C94
  int v1 = (int8)*(&kPoseParams[0].y_offset_to_gfx + 4 * j);
  samus_spritemap_y_pos = samus_y_pos - v1 - layer1_y_pos;
  samus_spritemap_x_pos = samus_x_pos - layer1_x_pos;
  return MakePairU16(samus_spritemap_x_pos, samus_spritemap_y_pos);
}

PairU16 Samus_CalcSpritemapPos_Standing(uint16 j) {  // 0x908CC3
  static const uint8 g_byte_908D28[16] = {
    3, 6, 0, 0,
    3, 6, 0, 0,
    3, 3, 6, 0,
    3, 3, 6, 0,
  };

  int16 v1;
  PairU16 v3;

  v1 = j >> 1;
  if (!(j >> 1) || v1 == kPose_9B_FaceF_VariaGravitySuit) {
    if (!sign16(samus_anim_frame - 2)) {
      samus_spritemap_y_pos = samus_y_pos - 1 - layer1_y_pos;
      samus_spritemap_x_pos = samus_x_pos - layer1_x_pos;
      return MakePairU16(samus_x_pos - layer1_x_pos, samus_y_pos - 1 - layer1_y_pos);
    }
  } else if (!sign16(v1 - kPose_A4_FaceR_LandJump) && sign16(v1 - kPose_A8_FaceR_Grappling)) {
    uint16 r18 = *(uint16 *)&g_byte_908D28[(uint16)(samus_anim_frame + 4 * (v1 - 164))];
    samus_spritemap_y_pos = samus_y_pos - r18 - layer1_y_pos;
    samus_spritemap_x_pos = samus_x_pos - layer1_x_pos;
    return MakePairU16(samus_x_pos - layer1_x_pos, samus_y_pos - r18 - layer1_y_pos);
  }
  v3 = Samus_CalcSpritemapPos_Default(j);
  return MakePairU16(v3.k, v3.j);
}

PairU16 Samus_CalcSpritemapPos_Crouch(uint16 j) {  // 0x908D3C
  static const int8 byte_908D80[24] = {
    -8,  0, -8,  0,
    -4, -2, -4, -2,
     0,  0,  0,  0,
    -4,  0, -4,  0,
     5,  4,  5,  4,
     0,  0,  0,  0,
  };

  int16 v1;

  v1 = j >> 1;
  if (sign16((j >> 1) - kPose_35_FaceR_CrouchTrans) || !sign16(v1 - kPose_41_FaceL_Morphball_Ground)) {
    return Samus_CalcSpritemapPos_Default(j);
  } else {
    int v4 = byte_908D80[(uint16)(samus_anim_frame + 2 * (v1 - 53))];
    samus_spritemap_y_pos = v4 + samus_y_pos - layer1_y_pos;
    samus_spritemap_x_pos = samus_x_pos - layer1_x_pos;
    return MakePairU16(samus_x_pos - layer1_x_pos, samus_spritemap_y_pos);
  }
}

PairU16 Samus_CalcSpritemapPos_Special(uint16 j) {  // 0x908D98
  static const int8 byte_908DEF[32] = {
   7, 5, -8, -8, -8, -8, -8, -5,
   4, 4,  4,  4,  0,  0,  4, -3,
  -5, 0,  0,  4, -3, -5, -3,  4,
   0, 0,  4,  0,  0,  4,  0,  0,
  };
  int v1, v2;

  v1 = j >> 1;
  if (v1 == kPose_E8_FaceR_Drained_CrouchFalling || v1 == kPose_E9_FaceL_Drained_CrouchFalling) {
    v2 = byte_908DEF[samus_anim_frame];
  } else {
    if (v1 != kPose_EA_FaceR_Drained_Stand && v1 != kPose_EB_FaceL_Drained_Stand || (int16)(samus_anim_frame - 5) < 0) {
      return Samus_CalcSpritemapPos_Default(j);
    }
    v2 = -3;
  }
  samus_spritemap_y_pos = v2 + samus_y_pos - layer1_y_pos;
  samus_spritemap_x_pos = samus_x_pos - layer1_x_pos;
  return MakePairU16(samus_x_pos - layer1_x_pos, v2 + samus_y_pos - layer1_y_pos);
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


void CallScrollingFinishedHook(uint32 ea) {
  switch (ea) {
  case fnSamus_ScrollFinishedHook_SporeSpawnFight: Samus_ScrollFinishedHook_SporeSpawnFight(); return;
  default: Unreachable();
  }
}

void MainScrollingRoutine(void) {  // 0x9094EC
  if (slow_grabble_scrolling_flag) {
    if ((samus_x_pos & 0x8000) != 0)
      goto LABEL_14;
    uint16 v0;
    v0 = samus_x_pos - layer1_x_pos;
    if (samus_x_pos < layer1_x_pos)
      goto LABEL_7;
    if (v0 >= 0xA0) {
      layer1_x_pos += 3;
      goto LABEL_8;
    }
    if (v0 < 0x60)
      LABEL_7:
    layer1_x_pos -= 3;
LABEL_8:
    if ((samus_y_pos & 0x8000) == 0) {
      uint16 v1 = samus_y_pos - layer1_y_pos;
      if (samus_y_pos >= layer1_y_pos) {
        if (v1 >= 0x90) {
          layer1_y_pos += 3;
          goto LABEL_14;
        }
        if (v1 >= 0x70)
          goto LABEL_14;
      }
      layer1_y_pos -= 3;
    }
LABEL_14:
    HandleAutoscrolling_X();
    HandleAutoscrolling_Y();
    goto LABEL_16;
  }
  Samus_CalcDistanceMoved_X();
  Samus_HandleScroll_X();
  Samus_CalcDistanceMoved_Y();
  Samus_HandleScroll_Y();
LABEL_16:
  if (scrolling_finished_hook)
    CallScrollingFinishedHook(scrolling_finished_hook | 0x900000);
  samus_prev_x_pos = samus_x_pos;
  samus_prev_x_subpos = samus_x_subpos;
  samus_prev_y_pos = samus_y_pos;
  samus_prev_y_subpos = samus_y_subpos;
}

void Samus_ScrollFinishedHook_SporeSpawnFight(void) {  // 0x909589
  if (layer1_y_pos <= 0x1D0)
    layer1_y_pos = 464;
}

void Samus_HandleScroll_X(void) {  // 0x9095A0
  static const uint16 kSamus_HandleScroll_X_FaceLeft[4] = { 0xa0, 0x50, 0x20, 0xe0 };
  static const uint16 kSamus_HandleScroll_X_FaceRight[4] = { 0x60, 0x40, 0x20, 0xe0 };
  if (samus_prev_x_pos == samus_x_pos) {
    HandleAutoscrolling_X();
    return;
  }
  if ((knockback_dir || samus_movement_type == 16 || samus_x_accel_mode == 1) ^ (samus_pose_x_dir != 4)) {
    ideal_layer1_xpos = samus_x_pos - kSamus_HandleScroll_X_FaceRight[camera_distance_index >> 1];
  } else {
    ideal_layer1_xpos = samus_x_pos - kSamus_HandleScroll_X_FaceLeft[camera_distance_index >> 1];
  }
  if (ideal_layer1_xpos != layer1_x_pos) {
    if ((int16)(ideal_layer1_xpos - layer1_x_pos) < 0) {
      AddToHiLo(&layer1_x_pos, &layer1_x_subpos, -IPAIR32(absolute_moved_last_frame_x, absolute_moved_last_frame_x_fract));
      HandleScrollingWhenTriggeringScrollLeft();
    } else {
      AddToHiLo(&layer1_x_pos, &layer1_x_subpos, __PAIR32__(absolute_moved_last_frame_x, absolute_moved_last_frame_x_fract));
      HandleScrollingWhenTriggeringScrollRight();
    }
  }
}

void Samus_HandleScroll_Y(void) {  // 0x90964F
  if (samus_prev_y_pos == samus_y_pos) {
    HandleAutoscrolling_Y();
  } else {
    //r18 = layer1_y_pos;
    if (samus_y_dir == 1)
      ideal_layer1_ypos = samus_y_pos - down_scroller;
    else
      ideal_layer1_ypos = samus_y_pos - up_scroller;
    if (ideal_layer1_ypos != layer1_y_pos) {
      if ((int16)(ideal_layer1_ypos - layer1_y_pos) < 0) {
        AddToHiLo(&layer1_y_pos, &layer1_y_subpos, -IPAIR32(absolute_moved_last_frame_y, absolute_moved_last_frame_y_fract));
        HandleScrollingWhenTriggeringScrollUp();
      } else {
        AddToHiLo(&layer1_y_pos, &layer1_y_subpos, __PAIR32__(absolute_moved_last_frame_y, absolute_moved_last_frame_y_fract));
        HandleScrollingWhenTriggeringScrollDown();
      }
    }
  }
}

void Samus_CalcDistanceMoved_X(void) {  // 0x9096C0
  if ((int16)(samus_x_pos - samus_prev_x_pos) >= 0) {
    SetHiLo(&absolute_moved_last_frame_x, &absolute_moved_last_frame_x_fract,
      __PAIR32__(samus_x_pos, samus_x_subpos) - __PAIR32__(samus_prev_x_pos, samus_prev_x_subpos) + (1 << 16));
  } else {
    SetHiLo(&absolute_moved_last_frame_x, &absolute_moved_last_frame_x_fract,
      __PAIR32__(samus_prev_x_pos, samus_prev_x_subpos) - __PAIR32__(samus_x_pos, samus_x_subpos) + (1 << 16));
  }
}

void Samus_CalcDistanceMoved_Y(void) {  // 0x9096FF
  if ((int16)(samus_y_pos - samus_prev_y_pos) >= 0) {
    SetHiLo(&absolute_moved_last_frame_y, &absolute_moved_last_frame_y_fract,
      __PAIR32__(samus_y_pos, samus_y_subpos) - __PAIR32__(samus_prev_y_pos, samus_prev_y_subpos) + (1 << 16));
  } else {
    SetHiLo(&absolute_moved_last_frame_y, &absolute_moved_last_frame_y_fract,
      __PAIR32__(samus_prev_y_pos, samus_prev_y_subpos) - __PAIR32__(samus_y_pos, samus_y_subpos) + (1 << 16));
  }
}

static const uint8 kShr0x80[8] = { 0x80, 0x40, 0x20, 0x10, 8, 4, 2, 1 };
static const uint16 kShr0xFc00[8] = { 0xfc00, 0x7e00, 0x3f00, 0x1f80, 0xfc0, 0x7e0, 0x3f0, 0x1f8 };

static void MarkMapTileAsExplored(uint16 r18, uint16 r24) {  // 0x90A8A6
  uint8 v0 = (uint16)(r18 & 0xFF00) >> 8;
  uint16 R34 = (room_x_coordinate_on_map + v0) & 0x20;
  uint16 r20 = ((room_x_coordinate_on_map + v0) & 0x1F) >> 3;
  uint16 R22 = room_y_coordinate_on_map + ((r24 & 0xFF00) >> 8) + 1;
  map_tiles_explored[(uint16)(r20 + 4 * (R34 + R22))] |= kShr0x80[(room_x_coordinate_on_map + v0) & 7];
}

void DisableMinimapAndMarkBossRoomAsExplored(void) {  // 0x90A7E2
  debug_disable_minimap = 1;
  uint16 v0 = 0;
  do {
    int v1 = v0 >> 1;
    hud_tilemap[v1 + 26] = 11295;
    hud_tilemap[v1 + 58] = 11295;
    hud_tilemap[v1 + 90] = 11295;
    v0 += 2;
  } while ((int16)(v0 - 10) < 0);
  uint16 v2 = 5;
  while (boss_id != g_stru_90A83A[v2].boss_id_) {
    if ((--v2 & 0x8000) != 0)
      return;
  }
  for (int i = g_stru_90A83A[v2].ptrs; ; i += 4) {
    const uint16 *v4 = (const uint16 *)RomPtr_90(i);
    if ((*v4 & 0x8000) != 0)
      break;
    MarkMapTileAsExplored(v4[0], v4[1]);
  }
}

void InitializeMiniMapBroken(void) {  // 0x90A8EF
  //SetR18_R20(room_x_coordinate_on_map + ((samus_x_pos & 0xFF00) >> 8), r18 >> 3);
  //R22_ = room_y_coordinate_on_map + ((samus_y_pos & 0xFF00) >> 8) + 1;
  //UpdateMinimapInside();
}


void UpdateMinimap(void) {  // 0x90A91B
  int16 v4;
  int16 v10;

  if (debug_disable_minimap || (samus_x_pos >> 4) >= room_width_in_blocks ||
      (samus_y_pos >> 4) >= room_height_in_blocks)
    return;
  uint16 r46 = 0;
  uint8 v0 = (uint16)(samus_x_pos & 0xFF00) >> 8;
  uint16 r34 = (room_x_coordinate_on_map + v0) & 0x20;
  uint16 r18 = (room_x_coordinate_on_map + v0) & 0x1F;
  uint16 v1 = (room_x_coordinate_on_map + v0) & 7;
  uint16 r20 = ((room_x_coordinate_on_map + v0) & 0x1F) >> 3;
  uint16 r22 = room_y_coordinate_on_map + ((samus_y_pos & 0xFF00) >> 8) + 1;
  uint16 v2 = r20 + 4 * (r34 + r22);
  map_tiles_explored[v2] |= kShr0x80[(room_x_coordinate_on_map + v0) & 7];
  uint16 r32 = v1;
  uint16 r30 = v2;
  uint16 v3 = v2 - 4;
  v4 = v1 - 2;
  if ((int16)(v1 - 2) < 0) {
    v4 &= 7;
    --v3;
    ++r46;
  }
  uint16 R50 = v3;
  int r52 = 2 * v4;
  int v6 = r52 >> 1;
  uint16 r24 = kShr0xFc00[v6] & swap16(*(uint16 *)&map_tiles_explored[v3]);
  uint16 r26 = kShr0xFc00[v6] & swap16(*(uint16 *)&map_tiles_explored[v3 + 4]);
  uint16 r28 = kShr0xFc00[v6] & swap16(*(uint16 *)&map_tiles_explored[v3 + 8]);
  const uint8 *r9 = RomPtr_82(kPauseMenuMapData[area_index]);
  const uint8 *r15 = r9;
  r9 += v3;
  uint16 r38 = swap16(GET_WORD(r9));
  r9 += 4;
  uint16 r40 = swap16(GET_WORD(r9));
  r9 += 4;
  uint16 r42 = swap16(GET_WORD(r9));
  if ((R50 & 3) == 3) {
    v10 = r46 ? r52 >> 1 : r32;
    if (!sign16(v10 - 6)) {
      uint8 R48 = r34 ? (R50 - 124) : (R50 + 125);

      uint16 v0 = (uint8)R48;
      uint16 r44 = 0;
      r9 = r15 + (uint8)R48;
      LOBYTE(r44) = map_tiles_explored[(uint8)R48];
      HIBYTE(r44) = *r9;
      if ((uint8)r34 == 32) {
        HIBYTE(r38) = HIBYTE(r44);
        HIBYTE(r24) = r44;
      } else {
        LOBYTE(r38) = HIBYTE(r44);
        LOBYTE(r24) = r44;
      }
      LOBYTE(r44) = map_tiles_explored[v0 + 4];
      r9 += 4;
      HIBYTE(r44) = *r9;
      if ((uint8)r34 == 32) {
        HIBYTE(r40) = HIBYTE(r44);
        HIBYTE(r26) = r44;
      } else {
        LOBYTE(r40) = HIBYTE(r44);
        LOBYTE(r26) = r44;
      }
      LOBYTE(r44) = map_tiles_explored[v0 + 8];
      r9 += 4;
      HIBYTE(r44) = *r9;
      if ((uint8)r34 == 32) {
        HIBYTE(r42) = HIBYTE(r44);
        HIBYTE(r28) = r44;
      } else {
        LOBYTE(r42) = HIBYTE(r44);
        LOBYTE(r28) = r44;
      }
    }
  }
  for (int n = r52 >> 1; n; n--) {
    r24 *= 2;
    r38 *= 2;
    r26 *= 2;
    r40 *= 2;
    r28 *= 2;
    r42 *= 2;
  }
  UpdateMinimapInside(r18, r22, r34, r30, r32, r38, r24, r40, r26, r42, r28);
}

void UpdateMinimapInside(uint16 r18, uint16 r22, uint16 r34, uint16 r30, uint16 r32,
                         uint16 r38, uint16 r24, uint16 r40,
                         uint16 r26, uint16 r42, uint16 r28) {  // 0x90AA43
  uint16 v0;
  int16 v1;
  int16 v5;
  int16 v7;
  int16 v8;

  LOBYTE(v0) = (uint16)(r34 + r22) >> 8;
  HIBYTE(v0) = r34 + r22;
  uint16 t = r18 + (v0 >> 3);
  if (r34 && sign16((t & 0x1F) - 2))
    v1 = t - 1026;
  else
    v1 = t - 34;
  uint16 v2 = 2 * v1;
  const uint16 *r0 = (const uint16 *)RomPtr(Load24(&kPauseMenuMapTilemaps[area_index]));
  const uint16 *r3 = r0 + 32;
  const uint16 *r6 = r0 + 64;
  int n = 5;
  uint16 v3 = 0;
  do {
    bool v4 = r38 >> 15;
    r38 *= 2;
    if (!v4 || (v5 = r0[v2 >> 1], !has_area_map))
      v5 = 31;
    int v6 = v3 >> 1;
    hud_tilemap[v6 + 26] = v5 & 0xC3FF | 0x2C00;

    v4 = r24 >> 15;
    r24 *= 2;
    if (v4)
      hud_tilemap[v6 + 26] = r0[v2 >> 1] & 0xC3FF | 0x2800;
    
    v4 = r40 >> 15;
    r40 *= 2;
    if (!v4 || (v7 = r3[v2 >> 1], !has_area_map))
      v7 = 31;
    hud_tilemap[v6 + 58] = v7 & 0xC3FF | 0x2C00;

    v4 = r26 >> 15;
    r26 *= 2;
    if (v4) {
      hud_tilemap[v6 + 58] = r3[v2 >> 1] & 0xC3FF | 0x2800;
      if (n == 3 && (hud_tilemap[v6 + 58] & 0x1FF) == 40) {
        // MarkMapTileAboveSamusAsExplored
        *((uint8 *)&music_data_index + r30) |= kShr0x80[r32];
      }
    }

    v4 = r42 >> 15;
    r42 *= 2;
    if (!v4 || (v8 = r6[v2 >> 1], !has_area_map))
      v8 = 31;
    hud_tilemap[v6 + 90] = v8 & 0xC3FF | 0x2C00;

    v4 = r28 >> 15;
    r28 *= 2;
    if (v4)
      hud_tilemap[v6 + 90] = r6[v2 >> 1] & 0xC3FF | 0x2800;

    v3 += 2;
    v2 += 2;
    if ((v2 & 0x3F) == 0)
      v2 += 1984;
  } while (--n);
  if ((nmi_frame_counter_byte & 8) == 0)
    hud_tilemap[60] |= 0x1C00;
}



void HandleArmCannonOpenState(void) {  // 0x90C5C4
  if (flag_arm_cannon_opening_or_closing || UpdateArmCannonIsOpenFlag() & 1)
    AdvanceArmCannonFrame();
  arm_cannon_drawing_mode = RomPtr_90(kPlayerPoseToPtr[samus_pose])[1];
}

uint8 UpdateArmCannonIsOpenFlag(void) {  // 0x90C5EB
  static const int8 kFlagShouldArmCannonBeOpen[6] = { 0, 1, 1, 0, 1, 0 };
  if (sign16(hud_item_changed_this_frame - 2))
    return 0;
  uint16 r18 = kFlagShouldArmCannonBeOpen[hud_item_index];
  if (flag_arm_cannon_open_or_opening == r18)
    return 0;
  if (r18)
    arm_cannon_frame = 0;
  else
    arm_cannon_frame = 4;
  *(uint16 *)&flag_arm_cannon_open_or_opening = r18 | 0x100;
  return 1;
}

void AdvanceArmCannonFrame(void) {  // 0x90C627
  if (flag_arm_cannon_open_or_opening) {
    if (sign16(arm_cannon_frame - 2)) {
      ++arm_cannon_frame;
      return;
    }
    arm_cannon_frame = 3;
  } else {
    if (arm_cannon_frame != 1 && (int16)(arm_cannon_frame - 1) >= 0) {
      --arm_cannon_frame;
      return;
    }
    arm_cannon_frame = 0;
  }
  *(uint16 *)&flag_arm_cannon_open_or_opening = flag_arm_cannon_open_or_opening;
}

void Samus_ArmCannon_Draw(void) {  // 0x90C663
  static const uint16 kDrawArmCannon_Char[10] = { 0x281f, 0x281f, 0x281f, 0x681f, 0xa81f, 0xe81f, 0x281f, 0x681f, 0x681f, 0x681f };
  uint16 v3;
  uint16 r22;

  if (arm_cannon_frame && (!samus_invincibility_timer || (nmi_frame_counter_word & 1) == 0)) {
    uint16 v0 = kPlayerPoseToPtr[samus_pose];
    const uint8 *v1 = RomPtr_90(v0);
    int16 v2 = *v1;
    if ((v2 & 0x80) != 0) {
      if (samus_anim_frame)
        v3 = 2 * (v1[2] & 0x7F);
      else
        v3 = 2 * (*v1 & 0x7F);
      r22 = v0 + 4;
    } else {
      v3 = 2 * v2;
      r22 = v0 + 2;
    }
    uint16 r24 = kDrawArmCannon_Char[v3 >> 1];
    const uint8 *v4 = RomPtr_90(r22 + 2 * samus_anim_frame);
    uint16 r18 = (int8)v4[0], r20 = (int8)v4[1];
    r22 = kPoseParams[samus_pose].y_offset_to_gfx;
    uint16 v7 = oam_next_ptr;
    int16 v8 = r18 + samus_x_pos - layer1_x_pos;
    if (v8 >= 0) {
      if (sign16(v8 - 256)) {
        OamEnt *v9 = gOamEnt(oam_next_ptr);
        v9->xcoord = v8;
        int16 v10 = r20 + samus_y_pos - r22 - layer1_y_pos;
        if (v10 >= 0) {
          if (sign16(v10 - 256)) {
            v9->ycoord = v10;
            *(uint16 *)&v9->charnum = r24;
            oam_next_ptr = v7 + 4;
          }
        }
      }
    }
    const uint8 *v11 = RomPtr_90(kPlayerPoseToPtr[samus_pose]);
    uint8 v12 = *v11;
    if ((v12 & 0x80) != 0) {
      v12 = samus_anim_frame ? v11[2] : v11[0];
    }
    uint16 v13 = kDrawArmCannon_Tab2[v12 & 0x7F] + 2 * arm_cannon_frame;
    uint16 v14 = vram_write_queue_tail;
    gVramWriteEntry(vram_write_queue_tail)->size = 32;
    v14 += 2;
    uint16 v15 = *(uint16 *)RomPtr_90(v13);
    gVramWriteEntry(v14)->size = v15;
    v14 += 2;
    LOBYTE(gVramWriteEntry(v14++)->size) = -102;
    gVramWriteEntry(v14)->size = addr_unk_6061F0;
    vram_write_queue_tail = v14 + 2;
  }
}

void CallFrameHandlerGamma(uint32 ea) {
  switch (ea) {
  case fnSamus_Func1: Samus_Func1(); return;
  case fnSamus_Func2: Samus_Func2(); return;
  case fnSamus_Func3: Samus_Func3(); return;
  case fnDrawTimer_: DrawTimer_(); return;
  case fnSamus_PushOutOfRidleysWay: Samus_PushOutOfRidleysWay(); return;
  case fnSamus_Func4: Samus_Func4(); return;
  case fnSamus_GrabbedByDraygonFrameHandler: Samus_GrabbedByDraygonFrameHandler(); return;
  case fnnullsub_151: return;
  case fnSamus_Func7: Samus_Func7(); return;
  case fnSamus_Func9: Samus_Func9(); return;
  case fnnullsub_152: return;
  default: Unreachable();
  }
}
void RunFrameHandlerGamma(void) {  // 0x90E097
  CallFrameHandlerGamma(frame_handler_gamma | 0x900000);
}

void Samus_Func1(void) {  // 0x90E09B
  if (samus_pose == kPose_E9_FaceL_Drained_CrouchFalling
      && !sign16(samus_anim_frame - 8)
      && (joypad1_newkeys & kButton_Up) != 0) {
    samus_anim_frame_timer = 1;
    samus_anim_frame = 13;
    frame_handler_gamma = FUNC16(nullsub_152);
  }
}

void Samus_Func2(void) {  // 0x90E0C5
  if (!sign16(samus_anim_frame - 8) && sign16(samus_anim_frame - 12) && (joypad1_newkeys & kButton_Up) != 0) {
    samus_anim_frame_timer = 1;
    samus_anim_frame = 18;
  }
}

void Samus_Func3(void) {  // 0x90E0E6
  if (ProcessTimer() & 1) {
    game_state = kGameState_35_TimeUp;
    for (int i = 510; i >= 0; i -= 2)
      target_palettes[i >> 1] = 0x7FFF;
    frame_handler_gamma = FUNC16(DrawTimer_);
    DisablePaletteFx();
  }
  if (timer_status)
    DrawTimer();
}

void DrawTimer_(void) {  // 0x90E114
  DrawTimer();
}

void Samus_SetPushedOutOfCeresRidley(void) {  // 0x90E119
  samus_movement_handler = FUNC16(nullsub_152);
  frame_handler_gamma = FUNC16(Samus_PushOutOfRidleysWay);
}

void Samus_PushOutOfRidleysWay(void) {  // 0x90E12E
  if (samus_pose_x_dir == 4)
    samus_pose = kPose_54_FaceL_Knockback;
  else
    samus_pose = kPose_53_FaceR_Knockback;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
  samus_last_different_pose = samus_prev_pose;
  *(uint16 *)&samus_last_different_pose_x_dir = *(uint16 *)&samus_prev_pose_x_dir;
  samus_prev_pose = samus_pose;
  *(uint16 *)&samus_prev_pose_x_dir = *(uint16 *)&samus_pose_x_dir;
  samus_y_pos -= 21 - samus_y_radius;
  if (sign16(samus_x_pos - layer1_x_pos - 128))
    samus_var62 = 1;
  else
    samus_var62 = 2;
  samus_y_speed = 5;
  samus_y_subspeed = 0;
  bomb_jump_dir = 0;
  frame_handler_gamma = FUNC16(Samus_Func4);
  samus_new_pose = -1;
  samus_new_pose_interrupted = -1;
  samus_new_pose_transitional = -1;
  samus_momentum_routine_index = 0;
  samus_special_transgfx_index = 0;
  samus_hurt_switch_index = 0;
  ProcessTimer();
  if (timer_status)
    DrawTimer();
}

void Samus_Func4(void) {  // 0x90E1C8
  static Func_V *const off_90E1F7[3] = {
    0,
    Samus_Func5,
    Samus_Func6,
  };

  if (samus_new_pose == kPose_4F_FaceL_Dmgboost || samus_new_pose == kPose_50_FaceR_Dmgboost) {
    samus_new_pose = -1;
    samus_momentum_routine_index = 0;
  }
  off_90E1F7[samus_var62]();
  input_to_pose_calc = 0;
  ProcessTimer();
  if (timer_status)
    DrawTimer();
}

void Samus_Func5(void) {  // 0x90E1FD
  Samus_BombJumpFallingXMovement_();
  if (samus_collision_flag) {
    samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
    frame_handler_gamma = FUNC16(Samus_Func3);
    samus_var62 = 0;
    Samus_ClearMoveVars();
  } else {
    Samus_BombJumpFallingYMovement_();
  }
}
void Samus_Func6(void) {  // 0x90E21C
  Samus_BombJumpFallingXMovement_();
  if (samus_collision_flag) {
    samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
    frame_handler_gamma = FUNC16(Samus_Func3);
    samus_var62 = 0;
    Samus_ClearMoveVars();
  } else {
    Samus_BombJumpFallingYMovement_();
  }
}
void Samus_GrabbedByDraygonFrameHandler(void) {  // 0x90E2A1
  if (grapple_beam_function == FUNC16(GrappleBeamFunc_ConnectedLockedInPlace)) {
    samus_new_pose = -1;
    samus_momentum_routine_index = 0;
  }
  if ((joypad1_newkeys & (kButton_Up | kButton_Down | kButton_Left | kButton_Right)) != 0
      && (joypad1_newkeys & (kButton_Up | kButton_Down | kButton_Left | kButton_Right)) != suit_pickup_light_beam_pos) {
    suit_pickup_light_beam_pos = joypad1_newkeys & (kButton_Up | kButton_Down | kButton_Left | kButton_Right);
    if ((int16)(++substate - 60) >= 0)
      Samus_ReleaseFromDraygon();
  }
}

void Samus_SetGrabbedByDraygonPose(uint16 a) {  // 0x90E23B
  if ((a & 1) != 0)
    samus_pose = kPose_EC_FaceR_Draygon_NoMove_NoAim;
  else
    samus_pose = kPose_BA_FaceL_Draygon_NoMove_NoAim;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
  frame_handler_gamma = FUNC16(Samus_GrabbedByDraygonFrameHandler);
  samus_movement_handler = FUNC16(nullsub_152);
  substate = 0;
  suit_pickup_light_beam_pos = 0;
  *(uint16 *)&suit_pickup_color_math_R = 0;
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

void Samus_ReleaseFromDraygon_(void) {  // 0x90E2D4
  Samus_ReleaseFromDraygon();
}

void Samus_ReleaseFromDraygon(void) {  // 0x90E2DE
  if (samus_pose_x_dir == 4)
    samus_pose = kPose_02_FaceL_Normal;
  else
    samus_pose = kPose_01_FaceR_Normal;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
  samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
  frame_handler_gamma = FUNC16(nullsub_152);
  samus_last_different_pose = samus_prev_pose;
  *(uint16 *)&samus_last_different_pose_x_dir = *(uint16 *)&samus_prev_pose_x_dir;
  samus_prev_pose = samus_pose;
  *(uint16 *)&samus_prev_pose_x_dir = *(uint16 *)&samus_pose_x_dir;
  samus_x_base_speed = 0;
  samus_x_base_subspeed = 0;
  samus_new_pose = -1;
  samus_new_pose_interrupted = -1;
  samus_new_pose_transitional = -1;
  samus_momentum_routine_index = 0;
  samus_special_transgfx_index = 0;
  samus_hurt_switch_index = 0;
  samus_y_speed = 0;
  samus_y_subspeed = 0;
  samus_y_dir = 0;
  used_for_ball_bounce_on_landing = 0;
  samus_x_accel_mode = 0;
  samus_grapple_flags = samus_grapple_flags & 0xFFFD | 2;
}

void Samus_Func7(void) {  // 0x90E3A3
  Samus_BombJumpFallingXMovement_();
  if (samus_collision_flag || (Samus_BombJumpFallingYMovement_(), samus_collision_flag)) {
    samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
    frame_handler_gamma = FUNC16(nullsub_152);
    samus_var62 = 0;
    Samus_ClearMoveVars();
    samus_new_pose_interrupted = 65;
    samus_special_transgfx_index = 0;
  }
}

void Samus_Func8(void) {  // 0x90E400
  samus_y_subspeed = 0;
  samus_y_speed = 0;
  samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
  frame_handler_gamma = FUNC16(nullsub_152);
}

void Samus_Func9(void) {  // 0x90E41B
  if (sign16(samus_y_speed - 5))
    AddToHiLo(&samus_y_speed, &samus_y_subspeed, __PAIR32__(samus_y_accel, samus_y_subaccel));
  if ((samus_pose == kPose_29_FaceR_Fall || samus_pose == kPose_2A_FaceL_Fall
       || samus_pose == kPose_67_FaceR_Fall_Gun || samus_pose == kPose_68_FaceL_Fall_Gun)
      && !sign16(samus_y_speed - 5)) {
    samus_anim_frame_timer = 16;
    samus_anim_frame = 4;
  }
}

uint32 Samus_CalcSpeed_X(uint32 amt) {  // 0x90E4E6
  amt += __PAIR32__(samus_x_extra_run_speed, samus_x_extra_run_subspeed);
  amt >>= (samus_x_speed_divisor <= 4) ? samus_x_speed_divisor : 4;
  SetHiLo(&samus_total_x_speed, &samus_total_x_subspeed, amt);
  return amt;
}

void Samus_MoveUp_SetPoseCalcInput(void) {  // 0x90E606
  if (samus_collision_flag)
    input_to_pose_calc = 4;
  else
    input_to_pose_calc = 0;
}

static const uint8 kSamus_MoveDown_SetPoseCalcInput_Tab0[28] = {  // 0x90E61B
  0, 0, 4, 4, 1, 0, 4, 2,
  4, 4, 0, 0, 0, 0, 4, 4,
  0, 3, 4, 4, 4, 0, 4, 4,
  4, 4, 4, 4,
};
static const uint8 kSamus_MoveDown_SetPoseCalcInput_Tab1[28] = {
  4, 4, 0, 0, 4, 4, 0, 4,
  1, 2, 0, 4, 4, 0, 4, 4,
  4, 4, 3, 3, 0, 4, 4, 4,
  4, 0, 4, 4,
};
void Samus_MoveDown_SetPoseCalcInput(void) {
  if (samus_collision_flag) {
    input_to_pose_calc = 1;
    HIBYTE(input_to_pose_calc) = kSamus_MoveDown_SetPoseCalcInput_Tab1[samus_movement_type];
  } else if ((uint8)input_to_pose_calc != 5) {
    input_to_pose_calc = 2;
    HIBYTE(input_to_pose_calc) = kSamus_MoveDown_SetPoseCalcInput_Tab0[samus_movement_type];
  }
}

void CallFrameHandlerAlfa(uint32 ea) {
  switch (ea) {
  case fnSamus_FrameHandlerAlfa_Func11: Samus_FrameHandlerAlfa_Func11(); return;
  case fnSamus_FrameHandlerAlfa_Func12: Samus_FrameHandlerAlfa_Func12(); return;
  case fnSamus_FrameHandlerAlfa_Func13: Samus_FrameHandlerAlfa_Func13(); return;
  case fnEmptyFunction: return;
  default: Unreachable();
  }
}
void HandleControllerInputForGamePhysics(void) {  // 0x90E692
  CallFrameHandlerAlfa(frame_handler_alfa | 0x900000);
}

void Samus_FrameHandlerAlfa_Func11(void) {  // 0x90E695
  samus_new_pose = -1;
  samus_new_pose_interrupted = -1;
  samus_new_pose_transitional = -1;
  samus_momentum_routine_index = 0;
  samus_special_transgfx_index = 0;
  samus_hurt_switch_index = 0;
  Samus_SetRadius();
  Samus_CallInputHandler();
  Samus_UpdateSuitPaletteIndex();
  Samus_DetermineAccel_Y();
  BlockInsideDetection();
  Samus_HandleHudSpecificBehaviorAndProjs();
  Samus_Func10();
}

void Samus_FrameHandlerAlfa_Func12(void) {  // 0x90E6C9
  samus_new_pose = -1;
  samus_new_pose_interrupted = -1;
  samus_new_pose_transitional = -1;
  samus_momentum_routine_index = 0;
  samus_special_transgfx_index = 0;
  samus_hurt_switch_index = 0;
  controller1_input_for_demo = joypad1_lastkeys;
  controller1_new_input_for_demo = joypad1_newkeys;
  demo_backup_prev_controller_input = joypad1_input_samusfilter;
  demo_backup_prev_controller_input_new = joypad1_newinput_samusfilter;
  Samus_SetRadius();
  Samus_UpdateSuitPaletteIndex();
  Samus_CallInputHandler();
  Samus_DetermineAccel_Y();
  BlockInsideDetection();
  Samus_HandleHudSpecificBehaviorAndProjs();
  Samus_Func10();
}

void Samus_FrameHandlerAlfa_Func13(void) {  // 0x90E713
  HandleProjectile();
  Samus_Func10();
}

void CallFrameHandlerBeta(uint32 ea) {
  switch (ea) {
  case fnSamus_FrameHandlerBeta_Func17: Samus_FrameHandlerBeta_Func17(); return;
  case fnHandleDemoRecorder_3: HandleDemoRecorder_3(); return;
  case fnSamus_FrameHandlerBeta_Func14: Samus_FrameHandlerBeta_Func14(); return;
  case fnSamus_Func15: Samus_Func15(); return;
  case fnSamus_Func16: Samus_Func16(); return;
  case fnSamus_Func18: Samus_Func18(); return;
  case fnEmptyFunction: return;
  case fnj_HandleDemoRecorder_2: return;
  case fnj_HandleDemoRecorder_2_0: return;
  case fnSetContactDamageIndexAndUpdateMinimap: SetContactDamageIndexAndUpdateMinimap(); return;
  case fnSamus_Func19: Samus_Func19(); return;
  case fnSamus_LowHealthCheck: Samus_LowHealthCheck(); return;
  default: Unreachable();
  }
}

void HandleSamusMovementAndPause(void) {  // 0x90E722
  CallFrameHandlerBeta(frame_handler_beta | 0x900000);
}

void Samus_FrameHandlerBeta_Func17(void) {  // 0x90E725
  samus_contact_damage_index = 0;
  RunSamusMovementHandler();
  UpdateMinimap();
  RunFrameHandlerGamma();
  Samus_Animate();
  Samus_HitInterruption();
  Samus_HandleTransFromBlockColl();
  Samus_HandleTransitions();
  Samus_HandlePalette();
  Samus_HandlePeriodicDamage();
  Samus_PauseCheck();
  Samus_LowHealthCheck_();
}

void HandleDemoRecorder_1(void) {  // 0x90E786
  if ((joypad2_new_keys & 0x8000) == 0) {
    if (!debug_flag && (joypad2_new_keys & 0x80) != 0) {
      DisableEprojs();
      time_is_frozen_flag = 1;
      frame_handler_alfa = FUNC16(EmptyFunction);
      frame_handler_beta = FUNC16(HandleDemoRecorder_3);
    }
  } else if (debug_flag) {
    samus_draw_handler = FUNC16(nullsub_152);
    debug_flag = 0;
  } else {
    debug_flag = 1;
    samus_draw_handler = FUNC16(SamusDrawHandler_Default);
  }
}

void HandleDemoRecorder_3(void) {  // 0x90E7D2
  if ((joypad2_new_keys & 0x80) != 0) {
    frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func11);
    frame_handler_beta = FUNC16(Samus_FrameHandlerBeta_Func17);
    EnableEprojs();
    time_is_frozen_flag = 0;
  }
}

void Samus_FrameHandlerBeta_Func14(void) {  // 0x90E7F5
  samus_contact_damage_index = 0;
  RunSamusMovementHandler();
  UpdateMinimap();
  Samus_Animate();
  Samus_HitInterruption();
  Samus_HandleTransFromBlockColl();
  Samus_HandleTransitions();
  Samus_HandlePalette();
  joypad1_lastkeys = controller1_input_for_demo;
  joypad1_newkeys = controller1_new_input_for_demo;
  joypad1_input_samusfilter = demo_backup_prev_controller_input;
  joypad1_newinput_samusfilter = demo_backup_prev_controller_input_new;
}

void Samus_Func15(void) {  // 0x90E833
  samus_contact_damage_index = 0;
  RunSamusMovementHandler();
  Samus_Animate();
  Samus_HitInterruption();
  Samus_HandleTransFromBlockColl();
  Samus_HandleTransitions();
  Samus_HandlePalette();
  joypad1_lastkeys = controller1_input_for_demo;
  joypad1_newkeys = controller1_new_input_for_demo;
  joypad1_input_samusfilter = demo_backup_prev_controller_input;
  joypad1_newinput_samusfilter = demo_backup_prev_controller_input_new;
}

void Samus_Func16(void) {  // 0x90E86A
  Samus_SetRadius();
  UpdateMinimap();
  Samus_Animate();
  elevator_status = 0;
  samus_prev_y_pos = samus_y_pos;
  if (PlaySamusFanfare() & 1) {
    if (sign16(debug_invincibility - 7) || (joypad2_last & 0x8000) == 0)
      debug_invincibility = 0;
    frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func11);
    frame_handler_beta = FUNC16(Samus_FrameHandlerBeta_Func17);
  }
}

void Samus_Func18(void) {  // 0x90E8AA
  Samus_FrameHandlerBeta_Func17();
  if (frame_handler_gamma == FUNC16(DrawTimer_) && game_state != kGameState_35_TimeUp)
    game_state = kGameState_35_TimeUp;
}

void SetContactDamageIndexAndUpdateMinimap(void) {  // 0x90E8DC
  samus_contact_damage_index = 0;
  UpdateMinimap();
}

void Samus_Func19(void) {  // 0x90E8EC
  samus_contact_damage_index = 0;
  RunSamusMovementHandler();
  UpdateMinimap();
  Samus_Animate();
}

void Samus_LowHealthCheck(void) {  // 0x90E902
  Samus_LowHealthCheck_();
}

void CallSamusInputHandler(uint32 ea) {
  switch (ea) {
  case fnnullsub_152: return;
  case fnSamus_InputHandler_E913: Samus_InputHandler_E913(); return;
  case fnSamus_Func20_: Samus_Func20_(); return;
  case fnSamus_InputHandler_E91D: Samus_InputHandler_E91D(); return;
  case fnHandleAutoJumpHack: HandleAutoJumpHack(); return;
  default: Unreachable();
  }
}

void Samus_CallInputHandler(void) {  // 0x90E90F
  CallSamusInputHandler(samus_input_handler | 0x900000);
}

void Samus_InputHandler_E913(void) {  // 0x90E913
  Samus_InputHandler();
}

void Samus_Func20_(void) {  // 0x90E918
  Samus_Func20();
}

void Samus_InputHandler_E91D(void) {  // 0x90E91D
  DemoObjectInputHandler();
  Samus_InputHandler();
}

void HandleAutoJumpHack(void) {  // 0x90E926
  uint16 v0 = joypad1_newkeys;
  if (autojump_timer && sign16(autojump_timer - 9)) {
    joypad1_newkeys |= button_config_jump_a;
    autojump_timer = 0;
  }
  Samus_InputHandler();
  joypad1_newkeys = v0;
  samus_input_handler = FUNC16(Samus_InputHandler_E913);
}

void CallSamusMovementHandler(uint32 ea) {
  switch (ea) {
  case fnSamus_MoveHandler_ReleaseFromGrapple: Samus_MoveHandler_ReleaseFromGrapple(); return;
  case fnSamus_HandleMovement_DrainedCrouching: Samus_HandleMovement_DrainedCrouching(); return;
  case fnSamus_MovementHandler_Normal: Samus_MovementHandler_Normal(); return;
  case fnSamus_MoveHandlerShinesparkWindup: Samus_MoveHandlerShinesparkWindup(); return;
  case fnSamus_MoveHandlerVerticalShinespark: Samus_MoveHandlerVerticalShinespark(); return;
  case fnSamus_MoveHandler_Shinespark_Diag: Samus_MoveHandler_Shinespark_Diag(); return;
  case fnSamus_MoveHandler_Shinespark_Horiz: Samus_MoveHandler_Shinespark_Horiz(); return;
  case fnSamus_MoveHandler_ShinesparkCrash: Samus_MoveHandler_ShinesparkCrash(); return;
  case fnSamus_MoveHandler_ShinesparkCrash_0: Samus_MoveHandler_ShinesparkCrash_0(); return;
  case fnSamus_MoveHandler_ShinesparkCrash_1: Samus_MoveHandler_ShinesparkCrash_1(); return;
  case fnSamus_MoveHandler_ShinesparkCrash_2: Samus_MoveHandler_ShinesparkCrash_2(); return;
  case fnSamus_MoveHandler_ShinesparkCrashEchoCircle: Samus_MoveHandler_ShinesparkCrashEchoCircle(); return;
  case fnSamus_MoveHandler_ShinesparkCrashFinish: Samus_MoveHandler_ShinesparkCrashFinish(); return;
  case fnSamusMoveHandler_CrystalFlashStart: SamusMoveHandler_CrystalFlashStart(); return;
  case fnSamusMoveHandler_CrystalFlashMain: SamusMoveHandler_CrystalFlashMain(); return;
  case fnkSamusMoveHandler_CrystalFlashFinish: kSamusMoveHandler_CrystalFlashFinish(); return;
  case fnSamus_MoveHandler_Knockback: Samus_MoveHandler_Knockback(); return;
  case fnSamus_MoveHandler_Knockback_0: Samus_MoveHandler_Knockback_0(); return;
  case fnSamus_MoveHandler_Knockback_Up: Samus_MoveHandler_Knockback_Up(); return;
  case fnSamus_MoveHandler_Knockback_3: Samus_MoveHandler_Knockback_3(); return;
  case fnSamus_MoveHandler_Knockback_Down: Samus_MoveHandler_Knockback_Down(); return;
  case fnSamus_MoveHandler_BombJumpStart: Samus_MoveHandler_BombJumpStart(); return;
  case fnSamus_MoveHandler_BombJumpMain: Samus_MoveHandler_BombJumpMain(); return;
  case fnSamus_MoveHandler_BombJumpFunc1: Samus_MoveHandler_BombJumpFunc1(); return;
  case fnnullsub_152: return;
  case fnSamusMovementType_Xray: SamusMovementType_Xray(); return;
  case fnSamus_Func25_ShineSpark: Samus_Func25_ShineSpark(); return;
  case fnSamus_MoveHandler_F072: Samus_MoveHandler_F072(); return;
  default: Unreachable();
  }
}

void RunSamusMovementHandler(void) {  // 0x90E94B
  CallSamusMovementHandler(samus_movement_handler | 0x900000);
}

void SamusMovementType_Xray(void) {  // 0x90E94F
  uint16 v0;
  if (samus_movement_type != kMovementType_0E_TurningAroundOnGround) {
    samus_anim_frame_timer = 15;
    if (samus_pose_x_dir == 4) {
      if (sign16(xray_angle - 153)) {
        v0 = 4;
      } else if (sign16(xray_angle - 178)) {
        v0 = 3;
      } else if (sign16(xray_angle - 203)) {
        v0 = 2;
      } else {
        v0 = sign16(xray_angle - 228) != 0;
      }
    } else if (sign16(xray_angle - 25)) {
      v0 = 0;
    } else if (sign16(xray_angle - 50)) {
      v0 = 1;
    } else if (sign16(xray_angle - 75)) {
      v0 = 2;
    } else if (sign16(xray_angle - 100)) {
      v0 = 3;
    } else {
      v0 = 4;
    }
    samus_anim_frame = v0;
  }
}

void Samus_HandlePeriodicDamage(void) {  // 0x90E9CE
  if (time_is_frozen_flag) {
    samus_periodic_damage = samus_periodic_subdamage = 0;
    return;
  }
  int32 t = __PAIR32__(samus_periodic_damage, samus_periodic_subdamage);
  if (t < 0) {
    InvalidInterrupt_Crash();
    return;
  }
  if ((equipped_items & 0x20) != 0)
    t = (t >> 2) & 0xffff00;
  else if ((equipped_items & 1) != 0)
    t = (t >> 1) & 0xffff00;
  AddToHiLo(&samus_health, &samus_subunit_health, -t);
  if ((int16)samus_health < 0)
    samus_health = samus_subunit_health = 0;
  samus_periodic_damage = samus_periodic_subdamage = 0;
}

void Samus_PauseCheck(void) {  // 0x90EA45
  if (!power_bomb_flag
      && !time_is_frozen_flag
      && !door_transition_flag_enemies
      && area_index != 6
      && game_state == kGameState_8_MainGameplay
      && (joypad1_newkeys & kButton_Start) != 0) {
    screen_fade_delay = 1;
    screen_fade_counter = 1;
    game_state = kGameState_12_Pausing;
  }
}

void Samus_LowHealthCheck_(void) {  // 0x90EA7F
  if (sign16(samus_health - 31)) {
    if (!samus_health_warning) {
      QueueSfx3_Max6(2);
      samus_health_warning = 1;
    }
  } else if (samus_health_warning) {
    samus_health_warning = 0;
    QueueSfx3_Max6(1);
  }
}

void Samus_LowHealthCheck_0(void) {  // 0x90EAAB
  Samus_LowHealthCheck_();
}

void Samus_JumpCheck(void) {  // 0x90EAB3
  if ((button_config_jump_a & joypad1_lastkeys) != 0 && (button_config_jump_a & joypad1_input_samusfilter) != 0)
    ++autojump_timer;
  else
    autojump_timer = 0;
  joypad1_input_samusfilter = joypad1_lastkeys;
  joypad1_newinput_samusfilter = joypad1_newkeys;
  if ((int16)(samus_health - samus_prev_health_for_flash) >= 0)
    goto LABEL_10;
  if (!samus_hurt_flash_counter)
    samus_hurt_flash_counter = 1;
  if (sign16(debug_invincibility - 7))
    LABEL_10:
  samus_prev_health_for_flash = samus_health;
  else
    samus_health = samus_prev_health_for_flash;
}

void Samus_Func10(void) {  // 0x90EB02
  projectile_init_speed_samus_moved_left = 0;
  projectile_init_speed_samus_moved_left_fract = 0;
  projectile_init_speed_samus_moved_right = 0;
  projectile_init_speed_samus_moved_right_fract = 0;
  projectile_init_speed_samus_moved_up = 0;
  projectile_init_speed_samus_moved_up_fract = 0;
  projectile_init_speed_samus_moved_down = 0;
  projectile_init_speed_samus_moved_down_fract = 0;
  samus_anim_frame_skip = 0;
  new_projectile_direction_changed_pose = 0;
  UNUSED_word_7E0DFA <<= 8;
  g_word_7E0A10 = WORD(samus_pose_x_dir);
}

static uint8 GetPackedOamExtBits(uint16 idx) {
  int shift = 2 * ((idx >> 2) & 7);
  return (oam_ext[idx >> 5] >> shift) & 3;
}

static void SetPackedOamExtBits(uint16 idx, uint8 bits) {
  int shift = 2 * ((idx >> 2) & 7);
  uint16 mask = 3 << shift;
  oam_ext[idx >> 5] = (oam_ext[idx >> 5] & ~mask) | ((bits & 3) << shift);
}

static void DrawPad2DronePing(const OamEnt *src, uint8 src_ext_bits, int x, int y) {
  if ((joypad2_last & (kButton_R | kButton_Select)) == 0)
    return;

  int dx = 0;
  int dy = 0;
  int len = 2 + ((nmi_frame_counter_word >> 1) & 1);

  if (joypad2_last & kButton_Left)
    dx = -1;
  else if (joypad2_last & kButton_Right)
    dx = 1;
  else
    dx = (samus_pose & 1) ? 1 : -1;

  if (joypad2_last & kButton_Up)
    dy = -1;
  else if (joypad2_last & kButton_Down)
    dy = 1;

  if ((joypad2_new_keys & (kButton_R | kButton_Select)) != 0)
    len = 4;

  for (int i = 1; i <= len && oam_next_ptr <= 0x1FB; i++) {
    int px = x + dx * (6 * i);
    int py = y + dy * (6 * i);

    if (dx == 0)
      px += (i & 1) ? -1 : 1;
    if (dy == 0)
      py += (i & 1) ? -1 : 1;
    if (px < 0 || px >= 256 || py < 0 || py >= 224)
      break;

    uint16 idx = oam_next_ptr;
    OamEnt *ping = gOamEnt(idx);
    *ping = *src;
    ping->xcoord = px;
    ping->ycoord = py;
    ping->flags ^= (i & 1) ? 0x40 : 0x80;
    SetPackedOamExtBits(idx, src_ext_bits);
    oam_next_ptr = idx + 4;
  }
}

static void DrawPad2Drone(uint16 samus_oam_start, uint16 samus_oam_end) {
  if (samus_oam_start == samus_oam_end || oam_next_ptr > 0x1FB)
    return;

  uint16 src_idx = (samus_oam_end - 4) & 0x1FF;
  OamEnt *src = gOamEnt(src_idx);
  uint16 firefly_phase = nmi_frame_counter_word & 0xF;
  bool firefly_bright = firefly_phase < 6 || (joypad2_last & (kButton_A | kButton_B | kButton_X | kButton_Y)) != 0;
  int x = (int)samus_x_pos - (int)layer1_x_pos + ((samus_pose & 1) ? -20 : 20);
  int y = (int)samus_y_pos - (int)layer1_y_pos - 24 + ((nmi_frame_counter_word >> 2) & 1);

  // Prototype only: keep the helper as a hover sprite until we have real actor state.
  if (joypad2_last & kButton_Left)
    x -= 12;
  if (joypad2_last & kButton_Right)
    x += 12;
  if (joypad2_last & kButton_Up)
    y -= 12;
  if (joypad2_last & kButton_Down)
    y += 12;
  if (joypad2_last & kButton_Y)
    y -= 6;
  if (joypad2_last & kButton_B)
    y += 6;
  if (joypad2_last & kButton_A)
    x += (samus_pose & 1) ? 6 : -6;
  if (joypad2_last & kButton_X)
    x += (samus_pose & 1) ? -6 : 6;

  // Give the helper a cheap firefly read: soft drift, blink, and a brief twin sparkle.
  x += (firefly_phase & 4) ? 1 : -1;
  if ((firefly_phase & 2) != 0)
    y += 1;
  if (!firefly_bright && firefly_phase >= 12)
    return;

  if (x < 0 || x >= 256 || y < 0 || y >= 224)
    return;

  uint8 src_ext_bits = GetPackedOamExtBits(src_idx) & 2;
  uint16 dst_idx = oam_next_ptr;
  OamEnt *dst = gOamEnt(dst_idx);
  *dst = *src;
  dst->xcoord = x;
  dst->ycoord = y;
  dst->flags ^= (nmi_frame_counter_word & 8) ? 0x40 : 0;
  SetPackedOamExtBits(dst_idx, src_ext_bits);
  oam_next_ptr = dst_idx + 4;

  if (firefly_bright && firefly_phase < 2 && oam_next_ptr <= 0x1FB) {
    uint16 glow_idx = oam_next_ptr;
    OamEnt *glow = gOamEnt(glow_idx);
    *glow = *src;
    glow->xcoord = x + 1;
    glow->ycoord = y + 1;
    glow->flags ^= 0xC0;
    SetPackedOamExtBits(glow_idx, src_ext_bits);
    oam_next_ptr = glow_idx + 4;
  }

  DrawPad2DronePing(src, src_ext_bits, x, y);
}

void DrawSamusAndProjectiles(void) {  // 0x90EB35
  for (int i = 0, n = MultiSamus_GetNumSamus(); i < n; i++) {
    MultiSamus_Switch(i);
    uint16 samus_oam_start = oam_next_ptr;
    SamusDrawSprites();
    DrawPad2Drone(samus_oam_start, oam_next_ptr);
    DrawPlayerExplosions2();
  }
  MultiSamus_Switch(0);
  Samus_JumpCheck();
  Samus_ShootCheck();
}

void CallSamusDrawHandler(uint32 ea) {
  switch (ea) {
  case fnSamusDrawHandler_Default: SamusDrawHandler_Default(); return;
  case fnsub_90EB86: sub_90EB86(); return;
  case fnnullsub_152:
  case fnnullsub_156: return;
  case fnSamus_DrawHandler_EndOfShinespark: Samus_DrawHandler_EndOfShinespark(); return;
  case fnSamusDisplayHandler_UsingElevator: SamusDisplayHandler_UsingElevator(); return;
  case fnSamusDisplayHandler_SamusReceivedFatal: SamusDisplayHandler_SamusReceivedFatal(); return;
  default: Unreachable();
  }
}

void SamusDrawSprites(void) {  // 0x90EB4B
  HandleArmCannonOpenState();
  CallSamusDrawHandler(samus_draw_handler | 0x900000);
}

void SamusDrawHandler_Default(void) {  // 0x90EB52
  HandleChargingBeamGfxAudio();
  sub_90EB55();
}

void sub_90EB55(void) {  // 0x90EB55
  if ((arm_cannon_drawing_mode & 0xF) != 0) {
    if ((arm_cannon_drawing_mode & 0xF) == 2) {
      HandleAtmosphericEffects();
      Samus_Draw();
      Samus_ArmCannon_Draw();
    } else {
      HandleAtmosphericEffects();
      Samus_ArmCannon_Draw();
      Samus_Draw();
    }
    Samus_DrawEchoes();
  } else {
    HandleAtmosphericEffects();
    Samus_Draw();
    Samus_DrawEchoes();
  }
}

void sub_90EB86(void) {  // 0x90EB86
  if (!sign16(grapple_beam_function + 0x37AA)) {
    sub_90EB55();
    return;
  }
  if (grapple_beam_function == FUNC16(GrappleBeamFunc_Firing)
      || grapple_beam_function == FUNC16(UNUSED_sub_9BC759)) {
    GrappleBeamFunc_BF1B();
  }
  HandleGrappleBeamFlare();
  if ((arm_cannon_drawing_mode & 0xF) != 0) {
    if ((arm_cannon_drawing_mode & 0xF) == 2) {
      HandleAtmosphericEffects();
      Samus_Draw();
      Samus_ArmCannon_Draw();
      UpdateGrappleBeamTilesAndIncrFlameCtr();
      if (!grapple_beam_length)
        return;
    } else {
      HandleAtmosphericEffects();
      Samus_ArmCannon_Draw();
      Samus_Draw();
      UpdateGrappleBeamTilesAndIncrFlameCtr();
      if (!grapple_beam_length)
        return;
    }
  } else {
    HandleAtmosphericEffects();
    Samus_Draw();
    UpdateGrappleBeamTilesAndIncrFlameCtr();
    if (!grapple_beam_length)
      return;
  }
  HandleGrappleBeamGfx();
}

void Samus_DrawManyEchoes(void) {  // 0x90EC03
  SamusDrawHandler_Default();
  for (int i = 2; i >= 0; i -= 2)
    Samus_DrawEchoes();
}

void SamusDisplayHandler_UsingElevator(void) {  // 0x90EC14
  if ((nmi_frame_counter_word & 1) == 0)
    SamusDisplayHandler_SamusReceivedFatal();
}

void SamusDisplayHandler_SamusReceivedFatal(void) {  // 0x90EC1D
  Samus_DrawWhenNotAnimatingOrDying();
}

void Samus_SetRadius(void) {  // 0x90EC22
  samus_y_radius = kPoseParams[samus_pose].y_radius;
  samus_x_radius = 5;
}

uint16 Samus_GetBottom_R18(void) {
  return samus_y_pos + kPoseParams[samus_pose].y_radius - 1;
}

uint16 Samus_GetTop_R20(void) {
  return samus_y_pos - kPoseParams[samus_pose].y_radius;
}

void Samus_AlignBottomWithPrevPose(void) {  // 0x90EC7E
  uint16 r18 = kPoseParams[samus_pose].y_radius;
  r18 = kPoseParams[samus_prev_pose].y_radius - r18;
  samus_y_pos += r18;
  samus_prev_y_pos += r18;
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

void MoveSamusWithControlPad(void) {  // 0x90ECD5
  if ((joypad1_lastkeys & 0x800) != 0)
    Samus_MoveUp(INT16_SHL16(-4));
  if ((joypad1_lastkeys & 0x400) != 0)
    Samus_MoveDown(INT16_SHL16(4));
  if ((joypad1_lastkeys & 0x200) != 0)
    Samus_MoveLeft(INT16_SHL16(-4));
  if ((joypad1_lastkeys & 0x100) != 0)
    Samus_MoveRight(INT16_SHL16(4));
}

static const uint8 byte_90ED50[28] = {  // 0x90ED26
  0, 0, 0, 0, 1, 0, 0, 1,
  1, 1, 0, 0, 0, 0, 0, 0,
  0, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 0,
};

void SwappedAmmoRoutine(void) {
  if (byte_90ED50[samus_movement_type]) {
    samus_missiles = samus_x_pos >> 4;
    samus_max_missiles = samus_x_pos >> 4;
    samus_super_missiles = samus_y_pos >> 4;
    samus_max_super_missiles = samus_y_pos >> 4;
  }
}

void UNUSEDsub_90ED6C(void) {  // 0x90ED6C
  samus_missiles = game_time_hours;
  samus_max_missiles = game_time_hours;
  samus_super_missiles = game_time_minutes;
  samus_max_super_missiles = game_time_minutes;
  samus_power_bombs = game_time_seconds;
  samus_max_power_bombs = game_time_seconds;
}

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
void Samus_FootstepGraphics(void) {
  kSamus_FootstepGraphics[area_index]();
}

static const uint8 byte_90EDC9[16] = {  // 0x90EDA1
  1, 0, 0, 0, 0, 2, 0, 4,
  0, 4, 4, 4, 4, 0, 4, 0,
};
void Samus_FootstepGraphics_Crateria(void) {
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

void DEPRECATED_Samus_UpdateSpeedEchoPos(void) {  // 0x90EEE7
  if ((speed_boost_counter & 0xFF00) == 1024 && (speed_echoes_index & 0x8000) == 0 && (game_time_frames & 3) == 0) {
    uint16 v0 = speed_echoes_index;
    int v1 = speed_echoes_index >> 1;
    speed_echo_xpos[v1] = samus_x_pos;
    speed_echo_ypos[v1] = samus_y_pos;
    uint16 v2 = v0 + 2;
    if ((int16)(v2 - 4) >= 0)
      v2 = 0;
    speed_echoes_index = v2;
  }
}

void ProjPreInstr_UnknownProj8027(uint16 k) {  // 0x90EFD3
  static const int16 kProjPreInstr_UnknownProj8027_X[4] = { -4, -4, 4, 4 };
  static const int16 kProjPreInstr_UnknownProj8027_Y[4] = { 4, -4, -4, 4 };
  static const int16 kProjPreInstr_UnknownProj8027_X2[4] = { 0x80, 0x80, -0x80, -0x80 };
  static const int16 kProjPreInstr_UnknownProj8027_Y2[4] = { -0x80, 0x80, 0x80, -0x80 };

  int v1 = k >> 1;
  projectile_x_pos[v1] += kProjPreInstr_UnknownProj8027_X[v1];
  uint16 v2 = kProjPreInstr_UnknownProj8027_Y[v1] + projectile_y_pos[v1];
  projectile_y_pos[v1] = v2;
  if (v2 == samus_y_pos) {
    if (projectile_variables[v1] == 1) {
      if (k == 6)
        samus_movement_handler = FUNC16(Samus_Func25_ShineSpark);
      ClearProjectile(k);
    } else {
      int v3 = k >> 1;
      ++projectile_variables[v3];
      samus_shine_timer = 180;
      timer_for_shine_timer = 1;
      special_samus_palette_frame = 0;
      projectile_x_pos[v3] = kProjPreInstr_UnknownProj8027_X2[v3] + samus_x_pos;
      projectile_y_pos[v3] = kProjPreInstr_UnknownProj8027_Y2[v3] + samus_y_pos;
    }
  }
}

static Func_U8 *const kSamusCodeHandlers[32] = {  // 0x90F084
  SamusCode_00_LockSamus,
  SamusCode_01_UnlockSamus,
  SamusCode_02_ReachCeresElevator,
  SamusCode_03,
  SamusCode_04,
  SamusCode_05_SetupDrained,
  SamusCode_06_LockToStation,
  SamusCode_07_SetupForElevator,
  SamusCode_08_SetupForCeresStart,
  SamusCode_08_SetupForZebesStart,
  SamusCode_0A_ClearDrawHandler,
  SamusCode_0B_DrawHandlerDefault,
  SamusCode_0C_UnlockFromMapStation,
  SamusCode_0D_IsGrappleActive,
  SamusCode_0E,
  SamusCode_0F_EnableTimerHandling,
  SamusCode_10,
  SamusCode_11_SetupForDeath,
  SamusCode_12_SetSuperPaletteFlag1,
  SamusCode_12_SetSuperPaletteFlag0,
  SamusCode_14,
  SamusCode_15_CalledBySuitAcquision,
  SamusCode_16,
  SamusCode_17_DisableRainbowSamusAndStandUp,
  SamusCode_18,
  SamusCode_17_FreezeDrainedSamus,
  SamusCode_1A,
  SamusCode_1B_CheckedLockSamus,
  SamusCode_1C,
  SamusCode_1D_ClearSoundInDoor,
  SamusCode_1E,
  SamusCode_1F,
};

uint16 CallSomeSamusCode(uint16 a) {
  uint16 code = kSamusCodeHandlers[a & 0x1F]();
  if (!(code & 1))
    return code >> 1;
  samus_new_pose = -1;
  samus_new_pose_interrupted = -1;
  samus_new_pose_transitional = -1;
  samus_momentum_routine_index = 0;
  samus_special_transgfx_index = 0;
  samus_hurt_switch_index = 0;
  return -1;
}

void Samus_UpdatePreviousPose(void) {  // 0x90F0EE
  samus_last_different_pose = samus_prev_pose;
  *(uint16 *)&samus_last_different_pose_x_dir = *(uint16 *)&samus_prev_pose_x_dir;
  samus_prev_pose = samus_pose;
  *(uint16 *)&samus_prev_pose_x_dir = *(uint16 *)&samus_pose_x_dir;
}

uint8 ClearCarry(void) {  // 0x90F107
  return 0;
}

uint8 SamusCode_00_LockSamus(void) {  // 0x90F109
  frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func13);
  frame_handler_beta = FUNC16(SetContactDamageIndexAndUpdateMinimap);
  return 1;
}

uint8 SamusCode_01_UnlockSamus(void) {  // 0x90F117
  frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func11);
  frame_handler_beta = FUNC16(Samus_FrameHandlerBeta_Func17);
  return 1;
}

uint8 SamusCode_02_ReachCeresElevator(void) {  // 0x90F125
  if (samus_pose_x_dir == 4)
    samus_pose = kPose_02_FaceL_Normal;
  else
    samus_pose = kPose_01_FaceR_Normal;
  samus_anim_frame_skip = 0;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
  reached_ceres_elevator_fade_timer = 60;
  return SamusCode_00_LockSamus();
}

uint8 SamusCode_03(void) {  // 0x90F152
  if (grapple_beam_function != FUNC16(GrappleBeamFunc_Inactive)) {
    grapple_beam_function = FUNC16(GrappleBeam_Func2);
    return 0;
  }
  if (samus_movement_type != kMovementType_03_SpinJumping && samus_movement_type != kMovementType_14_WallJumping)
    return 0;
  if (samus_pose_x_dir == kMovementType_04_MorphBallOnGround)
    samus_pose = kPose_02_FaceL_Normal;
  else
    samus_pose = kPose_01_FaceR_Normal;
  samus_anim_frame_skip = 0;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
  Samus_UpdatePreviousPose();
  return 1;
}

uint8 SamusCode_04_06_Common(void) {  // 0x90F19E
  flare_counter = 0;
  ClearFlareAnimationState();
  Samus_LoadSuitPalette();
  return 0;
}

uint8 SamusCode_04(void) {  // 0x90F19B
  samus_charge_palette_index = 0;
  return SamusCode_04_06_Common();
}

uint8 SamusCode_06_LockToStation(void) {  // 0x90F1AA
  frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func13);
  frame_handler_beta = FUNC16(j_HandleDemoRecorder_2);
  if (!sign16(flare_counter - 15))
    QueueSfx1_Max15(2);
  return SamusCode_04_06_Common();
}

uint8 SamusCode_07_SetupForElevator(void) {  // 0x90F1C8
  MakeSamusFaceForward();
  frame_handler_beta = FUNC16(Samus_Func19);
  samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
  samus_draw_handler = FUNC16(SamusDisplayHandler_UsingElevator);
  samus_input_handler = FUNC16(Samus_InputHandler_E913);
  bomb_jump_dir = 0;
  return 1;
}

uint8 SamusCode_08_SetupForCeresStart(void) {  // 0x90F1E9
  frame_handler_alfa = FUNC16(EmptyFunction);
  frame_handler_beta = FUNC16(SetContactDamageIndexAndUpdateMinimap);
  samus_pose = kPose_00_FaceF_Powersuit;
  samus_anim_frame_skip = 0;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
  Samus_LoadSuitPalette();
  samus_draw_handler = FUNC16(SamusDrawHandler_Default);
  samus_prev_pose = samus_pose;
  samus_last_different_pose = samus_pose;
  *(uint16 *)&samus_prev_pose_x_dir = *(uint16 *)&samus_pose_x_dir;
  *(uint16 *)&samus_last_different_pose_x_dir = *(uint16 *)&samus_pose_x_dir;
  samus_y_pos = 0;
  SpawnEprojWithGfx(0, 0, addr_kEproj_CeresElevatorPad);
  SpawnEprojWithGfx(0, 0, addr_kEproj_CeresElevatorPlatform);
  debug_disable_minimap = 0;
  PlayRoomMusicTrackAfterAFrames(0x20);
  return 1;
}

uint8 SamusCode_08_SetupForZebesStart(void) {  // 0x90F23C
  if ((equipped_items & 0x20) != 0) {
    SpawnPalfxObject(addr_stru_8DE1FC);
    samus_pose = kPose_9B_FaceF_VariaGravitySuit;
  } else if ((equipped_items & 1) != 0) {
    SpawnPalfxObject(addr_stru_8DE1F8);
    samus_pose = kPose_9B_FaceF_VariaGravitySuit;
  } else {
    SpawnPalfxObject(addr_stru_8DE1F4);
    samus_pose = kPose_00_FaceF_Powersuit;
  }
  Samus_LoadSuitPalette();
  SamusFunc_F433();
  samus_anim_frame_timer = 3;
  samus_anim_frame = 2;
  substate = 0;
  return 1;
}

uint8 SamusCode_0A_ClearDrawHandler(void) {  // 0x90F28D
  samus_draw_handler = FUNC16(nullsub_152);
  return 0;
}

uint8 SamusCode_0B_DrawHandlerDefault(void) {  // 0x90F295
  samus_draw_handler = FUNC16(SamusDrawHandler_Default);
  return SamusCode_01_UnlockSamus();
}

uint8 SamusCode_0C_UnlockFromMapStation(void) {  // 0x90F29E
  SamusFunc_E633();
  if (frame_handler_beta == FUNC16(j_HandleDemoRecorder_2)) {
    frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func11);
    frame_handler_beta = FUNC16(Samus_FrameHandlerBeta_Func17);
  }
  return 1;
}

uint8 SamusCode_0D_IsGrappleActive(void) {  // 0x90F2B8
  return (grapple_beam_function != FUNC16(GrappleBeamFunc_Inactive)) ? 2 : 0;
}

uint8 SamusCode_0D_IsGrappleActive_A(void) {
  return grapple_beam_function != FUNC16(GrappleBeamFunc_Inactive);
}

uint8 SamusCode_0E(void) {  // 0x90F2CA
  frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func11);
  frame_handler_beta = FUNC16(Samus_Func18);
  return 1;
}

uint8 SamusCode_0F_EnableTimerHandling(void) {  // 0x90F2D8
  frame_handler_gamma = FUNC16(Samus_Func3);
  return 0;
}

uint8 SamusCode_10(void) {  // 0x90F2E0
  if (frame_handler_beta != FUNC16(j_HandleDemoRecorder_2_0)) {
    frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func11);
    frame_handler_beta = FUNC16(Samus_FrameHandlerBeta_Func17);
  }
  return 1;
}

uint8 SamusCode_11_15_Common(void) {  // 0x90F2FC
  frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func13);
  frame_handler_beta = FUNC16(EmptyFunction);
  samus_draw_handler = FUNC16(SamusDisplayHandler_SamusReceivedFatal);
  return 1;
}

uint8 SamusCode_11_SetupForDeath(void) {  // 0x90F2F8
  DisablePaletteFx();
  return SamusCode_11_15_Common();
}

uint8 SamusCode_15_CalledBySuitAcquision(void) {  // 0x90F310
  Samus_UpdatePreviousPose();
  flare_counter = 0;
  ClearFlareAnimationState();
  Samus_LoadSuitPalette();
  return SamusCode_11_15_Common();
}

uint8 SamusCode_12_SetSuperPaletteFlag1(void) {  // 0x90F320
  samus_special_super_palette_flags = 1;
  return 0;
}

uint8 SamusCode_12_SetSuperPaletteFlag0(void) {  // 0x90F328
  samus_special_super_palette_flags = 0;
  Samus_LoadSuitPalette();
  return 0;
}

uint8 SamusCode_14(void) {  // 0x90F331
  if (sign16(samus_health - 31))
    QueueSfx3_Max6(2);
  if (!SamusCode_0D_IsGrappleActive_A()) {
    if (samus_pose_x_dir == 3) {
      if (samus_pose == kPose_81_FaceR_Screwattack || samus_pose == kPose_82_FaceL_Screwattack) {
        QueueSfx1_Max6(0x33);
      } else if (samus_pose == kPose_1B_FaceR_SpaceJump || samus_pose == kPose_1C_FaceL_SpaceJump) {
        QueueSfx1_Max6(0x3E);
      } else {
        QueueSfx1_Max6(0x31);
      }
    }
  } else {
    QueueSfx1_Max6(6);
  }
  return 0;
}

uint8 SamusCode_05_SetupDrained(void) {  // 0x90F38E
  frame_handler_gamma = FUNC16(Samus_Func1);
  return Samus_SetupForBeingDrained();
}

uint8 Samus_SetupForBeingDrained(void) {  // 0x90F394
  samus_pose = kPose_54_FaceL_Knockback;
  samus_anim_frame_skip = 0;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
  Samus_UpdatePreviousPose();
  flare_counter = 0;
  ClearFlareAnimationState();
  Samus_LoadSuitPalette();
  frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func13);
  frame_handler_beta = FUNC16(j_HandleDemoRecorder_2_0);
  return 1;
}

uint8 SamusCode_18(void) {  // 0x90F3C0
  frame_handler_gamma = FUNC16(Samus_Func2);
  return Samus_SetupForBeingDrained();
}

uint8 SamusCode_16(void) {  // 0x90F3C9
  samus_special_super_palette_flags = 0x8000;
  special_samus_palette_frame = 1;
  special_samus_palette_timer = 1;
  samus_charge_palette_index = 0;
  return 0;
}

uint8 SamusCode_17_DisableRainbowSamusAndStandUp(void) {  // 0x90F3DD
  samus_special_super_palette_flags = 0;
  special_samus_palette_frame = 0;
  special_samus_palette_timer = 0;
  samus_charge_palette_index = 0;
  Samus_LoadSuitPalette();
  samus_anim_frame_timer = 1;
  samus_anim_frame = 13;
  return 0;
}

uint8 SamusCode_17_FreezeDrainedSamus(void) {  // 0x90F3FB
  samus_anim_frame_timer = 1;
  samus_anim_frame = 28;
  return 1;
}

uint8 SamusCode_1A(void) {  // 0x90F409
  frame_handler_beta = FUNC16(Samus_LowHealthCheck);
  return 0;
}

uint8 SamusCode_1B_CheckedLockSamus(void) {  // 0x90F411
  if (frame_handler_beta == FUNC16(j_HandleDemoRecorder_2_0))
    return 1;
  else
    return SamusCode_00_LockSamus();
}

uint8 SamusCode_1C(void) {  // 0x90F41E
  if (samus_movement_type == kMovementType_14_WallJumping) {
    if (sign16(samus_anim_frame - 23)) {
      if (sign16(samus_anim_frame - 13)) {
LABEL_11:
        QueueSfx1_Max9(0x31);
        return 0;
      }
      goto LABEL_12;
    }
  } else {
    if (samus_movement_type != kMovementType_03_SpinJumping)
      return 0;
    if (samus_pose != kPose_81_FaceR_Screwattack && samus_pose != kPose_82_FaceL_Screwattack) {
      if (samus_pose != kPose_1B_FaceR_SpaceJump && samus_pose != kPose_1C_FaceL_SpaceJump)
        goto LABEL_11;
LABEL_12:
      QueueSfx1_Max9(0x3E);
      return 0;
    }
  }
  QueueSfx1_Max9(0x33);
  return 0;
}

uint8 SamusCode_1D_ClearSoundInDoor(void) {  // 0x90F471
  if (samus_movement_type == 3 || samus_movement_type == 20) {
    QueueSfx1_Max15(0x32);
    return 0;
  } else {
    if ((button_config_shoot_x & joypad1_lastkeys) == 0) {
      if (sign16(flare_counter - 16))
        QueueSfx1_Max15(2);
    }
    return 0;
  }
}

uint8 SamusCode_1E(void) {  // 0x90F4A2
  if (game_state == 8) {
    if (samus_movement_type == 3 || samus_movement_type == 20) {
      SamusCode_1C();
      return 0;
    }
    if (!sign16(flare_counter - 16))
      QueueSfx1_Max9(0x41);
  }
  return 0;
}

uint8 SamusCode_1F(void) {  // 0x90F4D0
  if (grapple_beam_function != (uint16)addr_loc_90C4F0) {
    grapple_beam_unkD1E = 0;
    grapple_beam_unkD20 = 0;
    grapple_beam_direction = 0;
    grapple_beam_unkD36 = 0;
    grapple_walljump_timer = 0;
    slow_grabble_scrolling_flag = 0;
    grapple_varCF6 = 0;
    grapple_beam_flags = 0;
    LoadProjectilePalette(equipped_beams);
    grapple_beam_function = -15120;
    samus_draw_handler = FUNC16(SamusDrawHandler_Default);
  }
  return 0;
}

uint8 Samus_Func26(void) {  // 0x90F507
  if (samus_movement_type != kMovementType_03_SpinJumping
      && samus_movement_type != kMovementType_14_WallJumping
      && (button_config_shoot_x & joypad1_lastkeys) != 0
      && !sign16(flare_counter - 16)) {
    QueueSfx1_Max9(0x41);
  }
  return 0;
}

void Samus_ShootCheck(void) {  // 0x90F576
  if ((play_resume_charging_beam_sfx & 0x8000) != 0)
    goto LABEL_15;
  if (play_resume_charging_beam_sfx) {
    if ((button_config_shoot_x & joypad1_lastkeys) != 0)
      QueueSfx1_Max9(0x41);
    play_resume_charging_beam_sfx = 0;
  }
  if (samus_echoes_sound_flag && (speed_boost_counter & 0x400) == 0) {
    samus_echoes_sound_flag = 0;
    QueueSfx3_Max15(0x25);
  }
  if ((samus_prev_movement_type == 3 || samus_prev_movement_type == 20)
      && samus_movement_type != kMovementType_03_SpinJumping
      && samus_movement_type != kMovementType_14_WallJumping) {
    QueueSfx1_Max15(0x32);
    if (!sign16(flare_counter - 16) && (button_config_shoot_x & joypad1_lastkeys) != 0)
      LABEL_15:
    play_resume_charging_beam_sfx = 1;
  }
  if (enable_debug) {
    if (samus_pose == kPose_00_FaceF_Powersuit || samus_pose == kPose_9B_FaceF_VariaGravitySuit) {
      if ((joypad2_last & 0x30) == 48 && (joypad2_new_keys & 0x80) != 0)
        debug_invincibility = 7;
    } else {
      if (!sign16(debug_invincibility - 7))
        return;
      debug_invincibility = 0;
    }
  }
  if (CheckEventHappened(0xE) & 1
      && frame_handler_gamma == FUNC16(DrawTimer_)
      && game_state != kGameState_35_TimeUp) {
    game_state = kGameState_35_TimeUp;
  }
}
