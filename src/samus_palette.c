// Samus palette and suit-pickup helpers: the suit pickup setup shared with
// HDMA, the per-frame palette dispatcher, beam/visor/hurt/x-ray palette
// effects, and the suit palette load/cancel helpers. Extracted from sm_91.c.

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "samus_env.h"

#define off_91D727 ((uint16*)RomFixedPtr(0x91d727))
#define kSamusPalette_HyperBeam ((uint16*)RomFixedPtr(0x91d829))
#define kSamusPalette_NonPseudoScrew ((uint16*)RomFixedPtr(0x91d7d5))
#define kSamusPalette_PseudoScrew ((uint16*)RomFixedPtr(0x91d7ff))
#define word_9BA3C0 ((uint16*)RomFixedPtr(0x9ba3c0))
#define kSamus_SpeedBoostingPalettes ((uint16*)RomFixedPtr(0x91d998))
#define kSamus_HyperBeamPalettes ((uint16*)RomFixedPtr(0x91d99e))
#define kSamusPal_ScrewAttack ((uint16*)RomFixedPtr(0x91da4a))
#define kSamusPal_SpeedBoost ((uint16*)RomFixedPtr(0x91daa9))
#define kSamusPal_SpeedBoostShine ((uint16*)RomFixedPtr(0x91db10))
#define kSamusPal_Shinespark ((uint16*)RomFixedPtr(0x91db75))
#define stru_91DC00 ((SamusCrystalFlashPalTable*)RomFixedPtr(0x91dc00))
#define off_91DC28 ((uint16*)RomFixedPtr(0x91dc28))
#define kSuitPickupBeamOffsets ((uint8*)RomFixedPtr(0x88e3c9))

void VariaSuitPickup(void) {  // 0x91D4E4
  suit_pickup_color_math_R = 48;
  suit_pickup_color_math_G = 80;
  suit_pickup_color_math_B = 0x80;
  suit_pickup_palette_transition_color = 0;
  Samus_CancelSpeedBoost();
  SetHiLo(&samus_x_extra_run_speed, &samus_x_extra_run_subspeed, 0);
  samus_x_base_speed = 0;
  samus_x_base_subspeed = 0;
  samus_y_subspeed = 0;
  samus_y_speed = 0;
  samus_y_dir = 0;
  used_for_ball_bounce_on_landing = 0;
  samus_x_accel_mode = kSamusXAccelMode_None;
  elevator_status = 0;
  substate = 0;
  suit_pickup_light_beam_pos = 0;
  suit_pickup_light_beam_widening_speed = 256;
  for (int i = 510; i >= 0; i -= 2)
    hdma_table_1[i >> 1] = 255;
  if (samus_movement_type == 3 || samus_movement_type == 20)
    QueueSfx1_Max9(0x32);
  if ((equipped_items & 0x20) != 0)
    samus_pose = kPose_9B_FaceF_VariaGravitySuit;
  else
    samus_pose = kPose_00_FaceF_Powersuit;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
  CallSomeSamusCode(0x15);
  samus_x_pos = layer1_x_pos + 120;
  samus_prev_x_pos = layer1_x_pos + 120;
  samus_y_pos = layer1_y_pos + 136;
  samus_prev_y_pos = layer1_y_pos + 136;
  QueueSfx2_Max6(0x56);
  static const SpawnHdmaObject_Args unk_91D59B = { 0x41, 0x26, 0xd5a2 };
  SpawnHdmaObject(0x91, &unk_91D59B);
}

void GravitySuitPickup(void) {  // 0x91D5BA
  suit_pickup_color_math_R = 48;
  suit_pickup_color_math_G = 73;
  suit_pickup_color_math_B = -112;
  suit_pickup_palette_transition_color = 1;
  Samus_CancelSpeedBoost();
  SetHiLo(&samus_x_extra_run_speed, &samus_x_extra_run_subspeed, 0);
  samus_x_base_speed = 0;
  samus_x_base_subspeed = 0;
  samus_y_subspeed = 0;
  samus_y_speed = 0;
  samus_y_dir = 0;
  used_for_ball_bounce_on_landing = 0;
  samus_x_accel_mode = kSamusXAccelMode_None;
  elevator_status = 0;
  substate = 0;
  suit_pickup_light_beam_pos = 0;
  suit_pickup_light_beam_widening_speed = 256;
  for (int i = 510; i >= 0; i -= 2)
    hdma_table_1[i >> 1] = 255;
  if (samus_movement_type == kMovementType_03_SpinJumping || samus_movement_type == kMovementType_14_WallJumping)
    QueueSfx1_Max9(0x32);
  if ((equipped_items & 1) != 0)
    samus_pose = kPose_9B_FaceF_VariaGravitySuit;
  else
    samus_pose = kPose_00_FaceF_Powersuit;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
  CallSomeSamusCode(0x15);
  samus_x_pos = layer1_x_pos + 120;
  samus_prev_x_pos = layer1_x_pos + 120;
  samus_y_pos = layer1_y_pos + 136;
  samus_prev_y_pos = layer1_y_pos + 136;
  QueueSfx2_Max6(0x56);
  static const SpawnHdmaObject_Args unk_91D673 = { 0x41, 0x26, 0xd67a };
  SpawnHdmaObject(0x91, &unk_91D673);
}

void InitializeSuitPickupHdma(void) {  // 0x91D692
  reg_TM = 19;
  reg_TMW = 19;
  reg_TS = 4;
  reg_TSW = 4;
  next_gameplay_CGWSEL = 16;
  reg_W12SEL = 0;
  reg_W34SEL = 2;
  reg_WOBJSEL = 32;
  next_gameplay_CGADSUB = 51;
  reg_COLDATA[0] = suit_pickup_color_math_R;
  reg_COLDATA[1] = suit_pickup_color_math_G;
  reg_COLDATA[2] = suit_pickup_color_math_B;
  mov24(&hdma_ptr_1, 0x9800E4);
  mov24(&hdma_ptr_2, 0x98C8E4);
  mov24(&hdma_ptr_3, 0x999098);
  hdma_var_1 = 0;
}

static const uint16 kSuitPickupLightBeamWindow = 0x7878;

static Func_U8 *const kVariaSuitPickupFuncs[7] = {  // 0x88E026
  VariaSuitPickup_0_LightBeamAppears,
  VariaSuitPickup_1,
  VariaSuitPickup_2_LightBeamWidens,
  VariaSuitPickup_3,
  VariaSuitPickup_4_LightBeamShrinks,
  VariaSuitPickup_5_LightBeamDissipates,
  VariaSuitPickup_6,
};

void HdmaobjPreInstr_VariaSuitPickup(uint16 k) {
  if (kVariaSuitPickupFuncs[substate]() & 1) {
    reg_COLDATA[0] = suit_pickup_color_math_R;
    reg_COLDATA[1] = suit_pickup_color_math_G;
    reg_COLDATA[2] = suit_pickup_color_math_B;
    fx_layer_blending_config_c = 18;
  }
}

static Func_U8 *const kGravitySuitPickupFuncs[7] = {  // 0x88E05C
  VariaSuitPickup_0_LightBeamAppears,
  VariaSuitPickup_1,
  VariaSuitPickup_2_LightBeamWidens,
  GravitySuitPickup_3,
  VariaSuitPickup_4_LightBeamShrinks,
  VariaSuitPickup_5_LightBeamDissipates,
  GravitySuitPickup_6,
};

void HdmaobjPreInstr_GravitySuitPickup(uint16 k) {
  if (kGravitySuitPickupFuncs[substate]() & 1) {
    reg_COLDATA[0] = suit_pickup_color_math_R;
    reg_COLDATA[1] = suit_pickup_color_math_G;
    reg_COLDATA[2] = suit_pickup_color_math_B;
    fx_layer_blending_config_c = 18;
  }
}

uint8 VariaSuitPickup_0_LightBeamAppears(void) {  // 0x88E092
  int16 v0;
  int16 v3;

  suit_pickup_light_beam_pos += 8;
  v0 = suit_pickup_light_beam_pos;
  uint16 v1 = 0;
  do {
    hdma_table_1[v1 >> 1] = kSuitPickupLightBeamWindow;
    v1 += 2;
    --v0;
  } while (v0 > 0);
  uint16 v2 = 510;
  v3 = suit_pickup_light_beam_pos;
  do {
    hdma_table_1[v2 >> 1] = kSuitPickupLightBeamWindow;
    v2 -= 2;
    --v3;
  } while (v3 > 0);
  if (!sign16(suit_pickup_light_beam_pos - 128)) {
    ++substate;
    suit_pickup_light_beam_pos = 30840;
  }
  return 1;
}

uint8 VariaSuitPickup_1(void) {  // 0x88E0D7
  LOBYTE(suit_pickup_light_beam_pos) = suit_pickup_light_beam_pos - HIBYTE(suit_pickup_light_beam_widening_speed);
  HIBYTE(suit_pickup_light_beam_pos) += HIBYTE(suit_pickup_light_beam_widening_speed);
  for (int i = 510; i >= 0; i -= 2)
    hdma_table_1[i >> 1] = suit_pickup_light_beam_pos;
  if (sign16((uint8)suit_pickup_light_beam_pos - 97)) {
    ++substate;
    suit_pickup_light_beam_pos = 0x846C;
  }
  return 1;
}

uint8 VariaSuitPickup_2_LightBeamWidens(void) {  // 0x88E113
  int8 v1;
  int16 v2;
  int8 v4;
  int8 v6;
  int8 v8;
  int8 v10;

  AdvanceSuitPickupColorMathSubscreenToWhite();
  bool v0 = (int8)(suit_pickup_light_beam_pos - HIBYTE(suit_pickup_light_beam_widening_speed)) < 0;
  LOBYTE(suit_pickup_light_beam_pos) = suit_pickup_light_beam_pos - HIBYTE(suit_pickup_light_beam_widening_speed);
  if (!(uint8)suit_pickup_light_beam_pos || v0) {
    suit_pickup_light_beam_pos = -256;
  } else {
    v1 = HIBYTE(suit_pickup_light_beam_widening_speed) + HIBYTE(suit_pickup_light_beam_pos);
    if (__CFADD__uint8(HIBYTE(suit_pickup_light_beam_widening_speed), HIBYTE(suit_pickup_light_beam_pos)))
      v1 = -1;
    HIBYTE(suit_pickup_light_beam_pos) = v1;
  }
  v2 = 0;
  uint16 v3 = 0;
  do {
    v4 = suit_pickup_light_beam_pos - kSuitPickupBeamOffsets[v3];
    if (v4 < 0)
      v4 = 0;
    *((uint8 *)hdma_table_1 + (uint16)v2) = v4;
    uint16 v5 = v2 + 1;
    v6 = kSuitPickupBeamOffsets[v3] + HIBYTE(suit_pickup_light_beam_pos);
    if (__CFADD__uint8(kSuitPickupBeamOffsets[v3], HIBYTE(suit_pickup_light_beam_pos)))
      v6 = -1;
    *((uint8 *)hdma_table_1 + v5) = v6;
    v2 = v5 + 1;
    ++v3;
  } while (v2 < 0x100);
  uint16 v7 = v3 - 1;
  do {
    v8 = suit_pickup_light_beam_pos - kSuitPickupBeamOffsets[v7];
    if (v8 < 0)
      v8 = 0;
    *((uint8 *)hdma_table_1 + (uint16)v2) = v8;
    uint16 v9 = v2 + 1;
    v10 = kSuitPickupBeamOffsets[v7] + HIBYTE(suit_pickup_light_beam_pos);
    if (__CFADD__uint8(kSuitPickupBeamOffsets[v7], HIBYTE(suit_pickup_light_beam_pos)))
      v10 = -1;
    *((uint8 *)hdma_table_1 + v9) = v10;
    v2 = v9 + 1;
    --v7;
  } while (v2 < 0x200);
  suit_pickup_light_beam_widening_speed += 96;
  if (suit_pickup_light_beam_pos == 0xFF00) {
    ++substate;
    suit_pickup_light_beam_widening_speed >>= 1;
    suit_pickup_light_beam_pos = 0;
  }
  return 1;
}

uint8 VariaSuitPickup_4_LightBeamShrinks(void) {  // 0x88E1BA
  if (suit_pickup_palette_transition_color)
    AdvanceSuitPickupColorMathToBlue();
  else
    AdvanceSuitPickupColorMathToOrange();
  suit_pickup_light_beam_pos += (suit_pickup_light_beam_widening_speed & 0xFF00) >> 8;
  int16 v0 = suit_pickup_light_beam_pos;
  uint16 v1 = 0;
  do {
    hdma_table_1[v1 >> 1] = 255;
    v1 += 2;
    --v0;
  } while (v0 > 0);
  uint16 v2 = 510;
  int16 v3 = suit_pickup_light_beam_pos - 1;
  do {
    hdma_table_1[v2 >> 1] = 255;
    v2 -= 2;
    --v3;
  } while (v3 > 0);
  suit_pickup_light_beam_widening_speed -= 32;
  if (sign16(suit_pickup_light_beam_widening_speed - 256))
    suit_pickup_light_beam_widening_speed = 256;
  if (!sign16(suit_pickup_light_beam_pos - 128)) {
    ++substate;
    suit_pickup_light_beam_pos = -1793;
  }
  return 1;
}

uint8 VariaSuitPickup_5_LightBeamDissipates(void) {  // 0x88E22B
  LOBYTE(suit_pickup_light_beam_pos) = suit_pickup_light_beam_pos + 8;
  HIBYTE(suit_pickup_light_beam_pos) -= 8;
  hdma_table_1[128] = suit_pickup_light_beam_pos;
  if (!sign16((uint8)suit_pickup_light_beam_pos - 112))
    ++substate;
  return 1;
}

uint8 VariaSuitPickup_6(void) {  // 0x88E258
  QueueMusic_Delayed8(3);
  return GravitySuitPickup_6();
}

uint8 GravitySuitPickup_6(void) {  // 0x88E25F
  reg_COLDATA[2] = 0x80;
  reg_COLDATA[1] = 64;
  reg_COLDATA[0] = 32;
  mov24(&hdma_ptr_1, 0x980001);
  *(uint16 *)((uint8 *)&demo_num_input_frames + 1) = 0;
  demo_input_prev = 0;
  demo_input_prev_new = 0;
  demo_backup_prev_controller_input = 0;
  hdma_table_1[0] = 255;
  substate = 0;
  suit_pickup_light_beam_pos = 0;
  *(uint16 *)&suit_pickup_color_math_R = 0;
  *(uint16 *)&suit_pickup_color_math_B = 0;
  int v0 = hdma_object_index >> 1;
  hdma_object_instruction_list_pointers[v0] += 2;
  hdma_object_instruction_timers[v0] = 1;
  CallSomeSamusCode(0xB);
  return 0;
}

uint8 AdvanceSuitPickupColorMathSubscreenToWhite(void) {  // 0x88E2B4
  if (suit_pickup_color_math_R != 63) {
    suit_pickup_color_math_R += 2;
    if (!sign8(suit_pickup_color_math_R - 64))
      suit_pickup_color_math_R = 63;
  }
  if (suit_pickup_color_math_G != 95) {
    suit_pickup_color_math_G += 2;
    if (!sign8(suit_pickup_color_math_G - 96))
      suit_pickup_color_math_G = 95;
  }
  if (suit_pickup_color_math_B != 0x9F) {
    suit_pickup_color_math_B += 2;
    if (!sign8(suit_pickup_color_math_B + 96))
      suit_pickup_color_math_B = -97;
  }
  return 1;
}

uint8 AdvanceSuitPickupColorMathToOrange(void) {  // 0x88E2F9
  if (suit_pickup_color_math_R != 63)
    --suit_pickup_color_math_R;
  if (suit_pickup_color_math_G != 77)
    --suit_pickup_color_math_G;
  if (suit_pickup_color_math_B != 0x83)
    --suit_pickup_color_math_B;
  return 1;
}

uint8 VariaSuitPickup_3(void) {  // 0x88E320
  equipped_items |= 1;
  collected_items |= 1;
  samus_pose = kPose_9B_FaceF_VariaGravitySuit;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
  samus_last_different_pose = samus_prev_pose;
  *(uint16 *)&samus_last_different_pose_x_dir = *(uint16 *)&samus_prev_pose_x_dir;
  samus_prev_pose = samus_pose;
  *(uint16 *)&samus_prev_pose_x_dir = *(uint16 *)&samus_pose_x_dir;
  Samus_LoadSuitPalette();
  ++substate;
  return 1;
}

uint8 GravitySuitPickup_3(void) {  // 0x88E361
  equipped_items |= 0x20;
  collected_items |= 0x20;
  samus_pose = kPose_9B_FaceF_VariaGravitySuit;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
  samus_last_different_pose = samus_prev_pose;
  *(uint16 *)&samus_last_different_pose_x_dir = *(uint16 *)&samus_prev_pose_x_dir;
  samus_prev_pose = samus_pose;
  *(uint16 *)&samus_prev_pose_x_dir = *(uint16 *)&samus_pose_x_dir;
  Samus_LoadSuitPalette();
  ++substate;
  return 1;
}

uint8 AdvanceSuitPickupColorMathToBlue(void) {  // 0x88E3A2
  if (suit_pickup_color_math_R != 48)
    --suit_pickup_color_math_R;
  if (suit_pickup_color_math_G != 73)
    --suit_pickup_color_math_G;
  if (suit_pickup_color_math_B != 0x90)
    --suit_pickup_color_math_B;
  return 1;
}

static Func_U8 *const off_91D72D[11] = {  // 0x91D6F7
  Samus_HandleScrewAttackSpeedBoostingPals,
  Samus_SpeedBoosterShinePals,
  (Func_U8 *)HandleMiscSamusPalette,
  0,
  0,
  0,
  Samus_HandleShinesparkingPals,
  Samus_HandleCrystalFlashPals,
  Samus_HandleXrayPals,
  HandleVisorPalette,
  nullsub_164,
};

void Samus_HandlePalette(void) {
  if ((samus_special_super_palette_flags & 0x8000) == 0
      && (HandleBeamChargePalettes() & 1
          || !(off_91D72D[timer_for_shine_timer]() & 1))) {
    CopyToSamusSuitPalette(off_91D727[samus_suit_palette_index >> 1]);
  }
  HandleMiscSamusPalette();
}

uint8 HandleBeamChargePalettes(void) {  // 0x91D743
  if (charged_shot_glow_timer) {
    if (hyper_beam_flag) {
      if ((charged_shot_glow_timer & 1) == 0) {
        if ((charged_shot_glow_timer & 0x1E) == 0) {
          charged_shot_glow_timer = 0;
          return 1;
        }
        CopyToSamusSuitPalette(kSamusPalette_HyperBeam[(charged_shot_glow_timer & 0x1E) >> 1]);
      }
      --charged_shot_glow_timer;
      return 0;
    }
    if (--charged_shot_glow_timer) {
      for (int i = 28; i >= 0; i -= 2)
        palette_buffer[(i >> 1) + 193] = 1023;
      return 0;
    } else {
      return 1;
    }
  } else if (grapple_beam_function == FUNC16(GrappleBeamFunc_Inactive)
             && flare_counter
             && !sign16(flare_counter - 60)) {
    uint16 R36;
    if (samus_contact_damage_index == 4)
      R36 = kSamusPalette_PseudoScrew[samus_suit_palette_index >> 1];
    else
      R36 = kSamusPalette_NonPseudoScrew[samus_suit_palette_index >> 1];
    CopyToSamusSuitPalette(*(uint16 *)RomPtr_91(R36 + samus_charge_palette_index));
    uint16 v1 = samus_charge_palette_index + 2;
    if (!sign16(samus_charge_palette_index - 10))
      v1 = 0;
    samus_charge_palette_index = v1;
    return 0;
  } else {
    samus_charge_palette_index = 0;
    return HandleVisorPalette();
  }
}

uint8 HandleVisorPalette(void) {  // 0x91D83F
  if (timer_for_shine_timer == 8)
    return 0;
  if (fx_layer_blending_config_a == 40 || fx_layer_blending_config_a == 42) {
    uint16 v1 = samus_visor_palette_timer_index - 1;
    samus_visor_palette_timer_index = v1;
    if ((uint8)v1)
      return 0;
    samus_visor_palette_timer_index = v1 | 5;
    palette_buffer[196] = word_9BA3C0[HIBYTE(v1) >> 1];
    int v2 = HIBYTE(v1) + 2;
    if (sign16(v2 - 12)) {
      samus_visor_palette_timer_index = swap16(v2) | (uint8)samus_visor_palette_timer_index;
    } else {
      samus_visor_palette_timer_index = (uint8)samus_visor_palette_timer_index | 0x600;
    }
    return 0;
  } else {
    samus_visor_palette_timer_index = 1537;
    return 0;
  }
}

static bool Samus_ShouldQueueHurtFlashDamageSfx(void) {
  return !cinematic_function &&
         (frame_handler_beta != FUNC16(j_HandleDemoRecorder_2_0) ||
          samus_pose != kPose_54_FaceL_Knockback);
}

static void Samus_RestoreSuitPaletteForHurtFlash(void) {
  if (cinematic_function)
    CopyToSamusSuitPalette(addr_word_9BA3A0);
  else
    Samus_LoadSuitPalette();
}

static void Samus_ApplyHurtFlashPaletteFrame(uint16 hurt_counter) {
  if (hurt_counter == 2 && Samus_ShouldQueueHurtFlashDamageSfx())
    QueueSfx1_Max6(0x35);

  if (!sign16(hurt_counter - 7))
    return;
  if ((hurt_counter & 1) != 0) {
    CopyToSamusSuitPalette(addr_word_9BA380);
    return;
  }
  Samus_RestoreSuitPaletteForHurtFlash();
}

static void Samus_HandleHurtFlashCounter40(void) {
  if (grapple_beam_function == FUNC16(GrappleBeamFunc_Inactive)) {
    if (samus_movement_type == kMovementType_03_SpinJumping ||
        samus_movement_type == kMovementType_14_WallJumping) {
      CallSomeSamusCode(0x1C);
    } else if (!sign16(flare_counter - 16) && (button_config_shoot_x & joypad1_lastkeys) != 0) {
      play_resume_charging_beam_sfx = 1;
    }
  } else if (sign16(grapple_beam_function + 0x37AA)) {
    QueueSfx1_Max9(6);
  }
}

static void Samus_AdvanceHurtFlashCounter(void) {
  uint16 next_counter = samus_hurt_flash_counter + 1;
  samus_hurt_flash_counter = next_counter;
  if (next_counter == 40) {
    Samus_HandleHurtFlashCounter40();
  } else if (!sign16(next_counter - 60)) {
    samus_hurt_flash_counter = 0;
  }
}

void HandleMiscSamusPalette(void) {  // 0x91D8A5
  if (!samus_special_super_palette_flags) {
    if (!samus_hurt_flash_counter)
      return;
    Samus_ApplyHurtFlashPaletteFrame(samus_hurt_flash_counter);
    Samus_AdvanceHurtFlashCounter();
    return;
  }
  if ((samus_special_super_palette_flags & 0x8000) != 0) {
    CopyToSamusSuitPalette(kSamus_HyperBeamPalettes[samus_charge_palette_index]);
    bool v2 = (--special_samus_palette_timer & 0x8000) != 0;
    if (!special_samus_palette_timer || v2) {
      special_samus_palette_timer = special_samus_palette_frame;
      if (!sign16(++samus_charge_palette_index - 10))
        samus_charge_palette_index = 0;
    }
  } else {
    if ((samus_special_super_palette_flags & 1) != 0)
      CopyToSamusSuitPalette(kSamus_SpeedBoostingPalettes[samus_suit_palette_index >> 1]);
    else
      Samus_LoadSuitPalette();
    ++samus_special_super_palette_flags;
  }
}

static bool Samus_ScrewSpeedPaletteBlockedByLiquid(void) {
  return (samus_suit_palette_index & kSamusSuitPalette_Gravity) == 0 &&
         Samus_IsSubmergedInRoomLiquid(Samus_GetTop_R20());
}

static uint8 Samus_ApplyScrewAttackPalette(void) {
  uint16 R36 = kSamusPal_ScrewAttack[samus_suit_palette_index >> 1];
  uint16 palette = *(uint16 *)RomPtr_91(R36 + special_samus_palette_frame);
  CopyToSamusSuitPalette(palette);
  uint16 next_frame = special_samus_palette_frame + 2;
  if (special_samus_palette_frame >= 10)
    next_frame = 0;
  special_samus_palette_frame = next_frame;
  return 1;
}

static uint8 Samus_ResetScrewAttackPaletteFrame(void) {
  special_samus_palette_frame = 0;
  return 1;
}

static uint8 Samus_ApplySpeedBoostPalette(void) {
  if ((speed_boost_counter & 0xFF00) != 1024)
    return 1;
  bool timer_expired = (--special_samus_palette_timer & 0x8000) != 0;
  if (!special_samus_palette_timer || timer_expired) {
    special_samus_palette_timer = 4;
    uint16 R36 = kSamusPal_SpeedBoost[samus_suit_palette_index >> 1];
    // Bugfix: The original game can do an out of bounds read here.
    if (special_samus_palette_frame > 6)
      special_samus_palette_frame = 6;
    uint16 palette = *(uint16 *)RomPtr_91(R36 + special_samus_palette_frame);
    CopyToSamusSuitPalette(palette);
    uint16 next_frame = special_samus_palette_frame + 2;
    if (special_samus_palette_frame >= 6)
      next_frame = 6;
    special_samus_palette_frame = next_frame;
  }
  return 1;
}

uint8 Samus_HandleScrewAttackSpeedBoostingPals(void) {  // 0x91D9B2
  if (Samus_ScrewSpeedPaletteBlockedByLiquid())
    return 1;
  if (samus_movement_type == kMovementType_03_SpinJumping) {
    if (!Samus_HasEquip(kSamusEquip_ScrewAttack))
      return Samus_ApplySpeedBoostPalette();
    if (samus_anim_frame == 0)
      return Samus_ResetScrewAttackPaletteFrame();
    if (!sign16(samus_anim_frame - 27))
      return 0;
    return Samus_ApplyScrewAttackPalette();
  }
  if (samus_movement_type == kMovementType_14_WallJumping) {
    if (!Samus_HasEquip(kSamusEquip_ScrewAttack))
      return 1;
    if (!sign16(samus_anim_frame - 3))
      return Samus_ApplyScrewAttackPalette();
    return Samus_ResetScrewAttackPaletteFrame();
  }
  return Samus_ApplySpeedBoostPalette();
}

uint8 Samus_SpeedBoosterShinePals(void) {  // 0x91DAC7
  uint16 v0 = samus_shine_timer;
  if (samus_shine_timer == 170) {
    uint16 v4 = samus_shine_timer;
    QueueSfx3_Max9(0xC);
    v0 = v4;
  }
  samus_shine_timer = v0 - 1;
  if ((int16)(v0 - 1) <= 0) {
    special_samus_palette_frame = 0;
    timer_for_shine_timer = 0;
    return 0;
  } else {
    uint16 R36 = kSamusPal_SpeedBoostShine[samus_suit_palette_index >> 1];
    uint16 v1 = *(uint16 *)RomPtr_91(R36 + special_samus_palette_frame);
    CopyToSamusSuitPalette(v1);
    uint16 v2 = special_samus_palette_frame + 2;
    if (!sign16(special_samus_palette_frame - 10))
      v2 = 0;
    special_samus_palette_frame = v2;
    return 1;
  }
}

uint8 Samus_HandleShinesparkingPals(void) {  // 0x91DB3A
  bool v0 = (--samus_shine_timer & 0x8000) != 0;
  if (!samus_shine_timer || v0) {
    timer_for_shine_timer = 0;
    special_samus_palette_frame = 0;
    return 0;
  } else {
    uint16 R36 = kSamusPal_Shinespark[samus_suit_palette_index >> 1];
    uint16 v1 = *(uint16 *)RomPtr_91(R36 + special_samus_palette_frame);
    CopyToSamusSuitPalette(v1);
    uint16 v2 = special_samus_palette_frame + 2;
    if (!sign16(special_samus_palette_frame - 6))
      v2 = 0;
    special_samus_palette_frame = v2;
    return 1;
  }
}

uint8 Samus_HandleCrystalFlashPals(void) {  // 0x91DB93
  if ((samus_shine_timer & 0x8000) != 0) {
    WriteBeamPalette_A(equipped_beams);
    timer_for_shine_timer = 0;
    special_samus_palette_frame = 0;
    special_samus_palette_timer = 0;
    samus_shine_timer = 0;
    return 0;
  } else {
    if ((int16)--samus_shine_timer <= 0) {
      samus_shine_timer = 5;
      Samus_Copy6PalColors(off_91DC28[special_samus_palette_frame >> 1]);
      uint16 v0 = special_samus_palette_frame + 2;
      if (!sign16(special_samus_palette_frame - 10))
        v0 = 0;
      special_samus_palette_frame = v0;
    }
    bool v1 = (int16)-- * (uint16 *)&suit_pickup_color_math_B < 0;
    if (!*(uint16 *)&suit_pickup_color_math_B || v1) {
      *(uint16 *)&suit_pickup_color_math_B = *(uint16 *)((uint8 *)&stru_91DC00[0].timer + special_samus_palette_timer);
      Samus_Copy10PalColors(*(VoidP *)((uint8 *)&stru_91DC00[0].ptr + special_samus_palette_timer));
      uint16 v2 = special_samus_palette_timer + 4;
      if (!sign16(special_samus_palette_timer - 36))
        v2 = 0;
      special_samus_palette_timer = v2;
    }
    return 1;
  }
}

void Samus_Copy10PalColors(uint16 v0) {  // 0x91DC34
  memcpy(&palette_buffer[224], RomPtr_9B(v0), 20);
}

void Samus_Copy6PalColors(uint16 j) {  // 0x91DC82
  memcpy(&palette_buffer[234], RomPtr_9B(j), 12);
}

uint8 Samus_HandleXrayPals(void) {  // 0x91DCB4
  if ((demo_timer_counter & 0x8000) != 0) {
    timer_for_shine_timer = 0;
    special_samus_palette_frame = 0;
    special_samus_palette_timer = 0;
    demo_timer_counter = 0;
    return 0;
  } else {
    if (!demo_timer_counter) {
      if (sign16(demo_input_pre_instr - 2)) {
        bool v0 = (--special_samus_palette_timer & 0x8000) != 0;
        if (!special_samus_palette_timer || v0) {
          special_samus_palette_timer = 5;
          palette_buffer[196] = word_9BA3C0[special_samus_palette_frame >> 1];
          if (sign16(special_samus_palette_frame - 4))
            special_samus_palette_frame += 2;
        }
        return 1;
      }
      special_samus_palette_frame = 6;
      special_samus_palette_timer = 1;
      demo_timer_counter = 1;
    }
    bool v2 = (--special_samus_palette_timer & 0x8000) != 0;
    if (special_samus_palette_timer && !v2)
      return 1;
    special_samus_palette_timer = 5;
    palette_buffer[196] = word_9BA3C0[special_samus_palette_frame >> 1];
    uint16 v3 = special_samus_palette_frame + 2;
    if (!sign16(special_samus_palette_frame - 10))
      v3 = 6;
    special_samus_palette_frame = v3;
    return 1;
  }
}

uint8 nullsub_164(void) {  // 0x91DD31
  return 0;
}

void CopyToSamusSuitPalette(uint16 k) {  // 0x91DD5B
  memcpy(&palette_buffer[192], (uint16 *)RomPtr_9B(k), 32);
}

void CopyToSamusSuitTargetPalette(uint16 k) {  // 0x91DDD7
  memcpy(&target_palettes[192], (uint16 *)RomPtr_9B(k), 32);
}

void Samus_CancelSpeedBoost(void) {  // 0x91DE53
  if (samus_has_momentum_flag) {
    samus_has_momentum_flag = 0;
    speed_boost_counter = 0;
    special_samus_palette_frame = 0;
    special_samus_palette_timer = 0;
    if ((equipped_items & 0x20) != 0) {
      CopyToSamusSuitPalette(addr_kSamusPalette_GravitySuit);
    } else if ((equipped_items & 1) != 0) {
      CopyToSamusSuitPalette(addr_kSamusPalette_VariaSuit);
    } else {
      CopyToSamusSuitPalette(addr_kSamusPalette_PowerSuit);
    }
  }
  if ((speed_echoes_index & 0x8000) == 0) {
    speed_echoes_index = -1;
    if (samus_pose_x_dir == 4) {
      speed_echo_xspeed[0] = -8;
      speed_echo_xspeed[1] = -8;
    } else {
      speed_echo_xspeed[0] = 8;
      speed_echo_xspeed[1] = 8;
    }
  }
}

void Samus_LoadSuitPalette(void) {  // 0x91DEBA
  if ((equipped_items & 0x20) != 0) {
    CopyToSamusSuitPalette(addr_kSamusPalette_GravitySuit);
  } else if ((equipped_items & 1) != 0) {
    CopyToSamusSuitPalette(addr_kSamusPalette_VariaSuit);
  } else {
    CopyToSamusSuitPalette(addr_kSamusPalette_PowerSuit);
  }
}

void Samus_LoadSuitTargetPalette(void) {  // 0x91DEE6
  if ((equipped_items & 0x20) != 0) {
    CopyToSamusSuitTargetPalette(addr_kSamusPalette_GravitySuit);
  } else if ((equipped_items & 1) != 0) {
    CopyToSamusSuitTargetPalette(addr_kSamusPalette_VariaSuit);
  } else {
    CopyToSamusSuitTargetPalette(addr_kSamusPalette_PowerSuit);
  }
}
