// Samus draw/render helpers extracted from sm_90.c: sprite composition,
// arm-cannon rendering, draw-handler dispatch, and multiplayer drone mirroring.

#include "ida_types.h"
#include "variables.h"
#include "variables_extra.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "multi_samus.h"
#include "physics_config.h"

static uint8 GetPackedOamExtBits(uint16 idx);
static void SetPackedOamExtBits(uint16 idx, uint8 bits);
static void DrawPad2DronePing(const OamEnt *src, uint8 src_ext_bits, int x, int y);
static void DrawPad2Drone(uint16 samus_oam_start, uint16 samus_oam_end);
static void CallSamusDrawHandler(uint32 ea);

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
  int num_samus = MultiSamus_GetNumSamus();
  bool draw_pad2_drone = num_samus > 1;
  for (int i = 0; i < num_samus; i++) {
    MultiSamus_Switch(i);
    uint16 samus_oam_start = oam_next_ptr;
    SamusDrawSprites();
    if (draw_pad2_drone)
      DrawPad2Drone(samus_oam_start, oam_next_ptr);
    DrawPlayerExplosions2();
  }
  MultiSamus_Switch(0);
  Samus_JumpCheck();
  Samus_ShootCheck();
}

static void CallSamusDrawHandler(uint32 ea) {
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

// Extracted from sm_92.c: Samus animation tile definitions

#define kSamus_AnimationDefinitionPtrs ((uint16*)RomFixedPtr(0x92d94e))
#define kSamus_TileDefs_TopHalf ((uint16*)RomFixedPtr(0x92d91e))
#define kSamus_TileDefs_BottomHalf ((uint16*)RomFixedPtr(0x92d938))

void SetSamusTilesDefsForCurAnim(void) {  // 0x928000
  uint16 v0 = 4 * samus_anim_frame + kSamus_AnimationDefinitionPtrs[samus_pose];
  SamusTileAnimationDefs *AD = get_SamusTileAnimationDefs(v0);
  nmi_copy_samus_top_half_src = 7 * AD->top_half_pos + kSamus_TileDefs_TopHalf[AD->top_half_idx];
  LOBYTE(nmi_copy_samus_halves) = 1;
  if (AD->bottom_half_idx != 255) {
    nmi_copy_samus_bottom_half_src = 7 * AD->bottom_half_pos + kSamus_TileDefs_BottomHalf[AD->bottom_half_idx];
    HIBYTE(nmi_copy_samus_halves) = 1;
  }
}

void Unused_SamusTileViewer(void) {  // 0x92ED7A
  DrawSamusSpritemap(0x182, 0x40, 0x40);
  DrawSamusSpritemap(0x183, 0xC0, 0x40);
  DrawSamusSpritemap(0x184, 0x80, 0x60);
  DrawSamusSpritemap(0x185, 0x80, 0x50);
}

void DrawSamusSuitExploding(void) {  // 0x92EDBE
  uint16 r18 = (samus_pose_x_dir == 4) ? g_word_7E0DE4 + 2085 : g_word_7E0DE4 + 2076;
  DrawSamusSpritemap(r18, samus_x_pos, samus_y_pos);
}
