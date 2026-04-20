// Boss and room-event HDMA families extracted from Bank 88: Draygon's room
// gate, Phantoon's wave effect, Mother Brain's rainbow beam, and the Morph
// Ball eye beam.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"

#define kMotherBrainRainbowBeamColors ((uint16*)RomFixedPtr(0x88e833))
#define kMorphBallEyeBeamColors ((uint8*)RomFixedPtr(0x88ea8b))

static const SpawnHdmaObject_Args kDraygonHdmaObject = { 0x00, 0x2c, 0xdf4f };
static const SpawnHdmaObject_Args kUnusedBossHdmaObjectA = { 0x00, 0x2c, 0xdf6b };
static const SpawnHdmaObject_Args kUnusedBossHdmaObjectB = { 0x02, 0x12, 0xdf77 };
static const SpawnHdmaObject_Args kPhantoonWavyHdmaObject = { 0x42, 0x0f, 0xe4a8 };
static const SpawnHdmaObject_Args kMotherBrainRisingHdmaArgs = { 0x00, 0x2c, 0xe727 };
static const SpawnHdmaObject_Args kMotherBrainRainbowBeamHdmaArgs = { 0x41, 0x26, 0xe751 };
static const SpawnHdmaObject_Args kMorphBallEyeBeamHdmaArgs = { 0x41, 0x26, 0xe8ec };

static void UpdateMotherBrainRainbowBeamColor(void);
static void UpdateMorphBallEyeBeamHdma(uint16 k);

void sub_88DF34(void) {  // 0x88DF34
  SpawnHdmaObject(0x88, &kDraygonHdmaObject);
}

void sub_88DF3D(void) {  // 0x88DF3D
  SpawnHdmaObject(0x88, &kUnusedBossHdmaObjectA);
}

void sub_88DF46(void) {  // 0x88DF46
  SpawnHdmaObject(0x88, &kUnusedBossHdmaObjectB);
}

void HdmaobjPreInstr_DF94(uint16 v0) {  // 0x88DF94
  int16 v1;

  if ((enemy_data[0].properties & 0x200) == 0
      && (int16)(enemy_data[0].x_pos - layer1_x_pos + 64) >= 0
      && sign16(enemy_data[0].x_pos - layer1_x_pos - 320)
      && (int16)(enemy_data[0].y_pos - layer1_y_pos + 16) >= 0
      && (v1 = enemy_data[0].y_pos - layer1_y_pos, sign16(enemy_data[0].y_pos - layer1_y_pos - 304))) {
    if (sign16(v1 - 40)) {
      int v2 = v0 >> 1;
      hdma_object_instruction_timers[v2] = 1;
      hdma_object_instruction_list_pointers[v2] = addr_word_88DF6B;
    } else {
      int v4 = v0 >> 1;
      hdma_object_instruction_timers[v4] = 1;
      if (sign16(v1 - 192))
        hdma_object_instruction_list_pointers[v4] = addr_word_88DF5F;
      else
        hdma_object_instruction_list_pointers[v4] = addr_word_88DF65;
    }
  } else {
    int v3 = v0 >> 1;
    hdma_object_instruction_timers[v3] = 1;
    hdma_object_instruction_list_pointers[v3] = addr_word_88DF71;
  }
}

void HdmaobjPreInstr_E449(uint16 k) {  // 0x88E449
  if ((phantom_related_layer_flag & 0x4000) != 0) {
    fx_layer_blending_config_c = 26;
  } else if (LOBYTE(enemy_data[3].parameter_1)) {
    if (LOBYTE(enemy_data[3].parameter_1) == 0xFF) {
      fx_layer_blending_config_c = 4;
      int v0 = hdma_object_index >> 1;
      hdma_object_instruction_list_pointers[v0] += 2;
      hdma_object_instruction_timers[v0] = 1;
    }
  } else {
    fx_layer_blending_config_c = 4;
  }
}

void sub_88E487(uint16 v0, uint16 r22) {  // 0x88E487
  enemy_data[2].parameter_1 = v0;
  enemy_data[3].ai_var_D = 0;
  enemy_data[3].ai_var_E = 0;
  enemy_data[3].ai_preinstr = r22;
  SpawnHdmaObject(0x88, &kPhantoonWavyHdmaObject);
}

const uint8 *HdmaobjInstr_E4BD(uint16 k, const uint8 *hdp) {  // 0x88E4BD
  unsigned int v2; // kr00_4
  unsigned int v3; // kr04_4
  unsigned int v4; // kr08_4
  unsigned int v5; // kr0C_4

  enemy_data[1].parameter_1 = enemy_data[2].parameter_1;
  if ((enemy_data[2].parameter_1 & 1) != 0) {
    kraid_unk9000 = 192;
    g_word_7E9006 = 192;
    v4 = 12620032;
    *(uint16 *)((uint8 *)&g_word_7E9002 + 1) = HIWORD(v4);
    *(uint16 *)((uint8 *)&kraid_unk9000 + 1) = v4;
    g_word_7E9004 = -28288;
    v5 = 12620032;
    *(uint16 *)((uint8 *)&g_word_7E9008 + 1) = HIWORD(v5);
    *(uint16 *)((uint8 *)&g_word_7E9006 + 1) = v5;
    g_word_7E900A = addr_loc_889180;
    g_word_7E900C = 0;
  } else {
    kraid_unk9000 = 160;
    g_word_7E9006 = 160;
    g_word_7E900C = 160;
    g_word_7E900F = 160;
    g_word_7E9012 = 160;
    g_word_7E9015 = 160;
    v2 = 10522880;
    *(uint16 *)((uint8 *)&g_word_7E9002 + 1) = HIWORD(v2);
    *(uint16 *)((uint8 *)&kraid_unk9000 + 1) = v2;
    v3 = 10522880;
    *(uint16 *)((uint8 *)&g_word_7E9008 + 1) = HIWORD(v3);
    *(uint16 *)((uint8 *)&g_word_7E9006 + 1) = v3;
    *(uint16 *)((uint8 *)&g_word_7E900C + 1) = -28416;
    *(uint16 *)((uint8 *)&g_word_7E9012 + 1) = -28416;
    g_word_7E9004 = -28352;
    g_word_7E900A = -28352;
    *(uint16 *)((uint8 *)&g_word_7E900F + 1) = -28352;
    *(uint16 *)((uint8 *)&g_word_7E9015 + 1) = -28352;
    g_word_7E9018 = 0;
  }
  int v6 = k >> 1;
  hdma_object_A[v6] = -2;
  hdma_object_B[v6] = 1;
  hdma_object_C[v6] = 0;
  hdma_object_D[v6] = 0;
  return hdp;
}

void HdmaobjPreInstr_E567(uint16 v0) {  // 0x88E567
  int16 v7;
  uint16 j;
  uint16 v8;
  uint16 r28;
  uint16 r30;

  if (enemy_data[1].parameter_1) {
    if ((enemy_data[1].parameter_1 & 1) != 0) {
      r28 = 4;
      r30 = 128;
    } else {
      r28 = 8;
      r30 = 64;
    }
    int v2 = v0 >> 1;
    if (hdma_object_B[v2]-- == 1) {
      hdma_object_B[v2] = 1;
      hdma_object_A[v2] = (2 * enemy_data[3].ai_preinstr + hdma_object_A[v2]) & 0x1FF;
    }
    uint16 r20 = hdma_object_A[v2];
    uint16 v4 = 0;
    do {
      uint16 v11 = v4;
      uint16 v5 = r20;
      int v6 = r20 >> 1;
      v7 = kSinCosTable8bit_Sext[v6 + 64];
      if (v7 < 0) {
        uint16 r18 = -v7;
        uint16 r22 = Mult8x8(-(int8)v7, enemy_data[3].ai_var_D) >> 8;
        r22 += Mult8x8(HIBYTE(r18), enemy_data[3].ai_var_D);
        uint16 r24 = Mult8x8(r18, HIBYTE(enemy_data[3].ai_var_D));
        uint8 mult = Mult8x8(HIBYTE(r18), HIBYTE(enemy_data[3].ai_var_D));
        r22 += r24;
        r18 = ((r22 + (mult << 8)) & 0xFF00) >> 8;
        r20 = (r28 + v5) & 0x1FF;
        v8 = v11;
        *(uint16 *)((uint8 *)&g_word_7E9100 + v11) = reg_BG2HOFS - r18;
      } else {
        uint16 r18 = kSinCosTable8bit_Sext[v6 + 64];
        uint16 r22 = Mult8x8(v7, enemy_data[3].ai_var_D) >> 8;
        r22 += Mult8x8(HIBYTE(r18), enemy_data[3].ai_var_D);
        uint16 r24 = Mult8x8(r18, HIBYTE(enemy_data[3].ai_var_D));
        uint8 mult = Mult8x8(HIBYTE(r18), HIBYTE(enemy_data[3].ai_var_D));
        r22 += r24;
        r18 = ((r22 + (mult << 8)) & 0xFF00) >> 8;
        r20 = (r28 + v5) & 0x1FF;
        v8 = v11;
        *(uint16 *)((uint8 *)&g_word_7E9100 + v11) = r18 + reg_BG2HOFS;
      }
      v4 = v8 + 2;
    } while ((int16)(v4 - r30) < 0);
    if ((enemy_data[1].parameter_1 & 1) != 0) {
      for (int i = 126; i >= 0; i -= 2)
        *(uint16 *)((uint8 *)&g_word_7E9180 + i) = reg_BG2HOFS + reg_BG2HOFS - *(uint16 *)((uint8 *)&g_word_7E9100 + i);
    } else {
      for (j = 62; (j & 0x8000) == 0; j -= 2)
        *(uint16 *)((uint8 *)&g_word_7E9140 + j) = reg_BG2HOFS + reg_BG2HOFS - *(uint16 *)((uint8 *)&g_word_7E9100 + j);
    }
  } else {
    int v1 = hdma_object_index >> 1;
    hdma_object_instruction_list_pointers[v1] += 2;
    hdma_object_instruction_timers[v1] = 1;
  }
}

uint16 MotherBrainRisingHdmaObject(void) {  // 0x88E71E
  return SpawnHdmaObject(0x88, &kMotherBrainRisingHdmaArgs);
}

uint16 SpawnMotherBrainRainbowBeamHdma(void) {  // 0x88E748
  return SpawnHdmaObject(0x88, &kMotherBrainRainbowBeamHdmaArgs);
}

void InitializeRainbowBeam(void) {  // 0x88E767
  reg_COLDATA[0] = 32;
  reg_COLDATA[1] = 71;
  reg_COLDATA[2] = 0x8f;
  fx_layer_blending_config_c = 36;
  mother_brain_indirect_hdma[0] = 100;
  *(uint16 *)&mother_brain_indirect_hdma[1] = -25344;
  *(uint16 *)&mother_brain_indirect_hdma[3] = 0;
  hdma_table_2[0] = 0;
  hdma_table_2[1] = -32736;
  hdma_table_2[2] = 0;
  MotherBrain_CalculateRainbowBeamHdma();
}

void HdmaobjPreInstr_E7BC(uint16 k) {  // 0x88E7BC
  if (game_state == 19) {
    hdma_object_channels_bitmask[hdma_object_index >> 1] = 0;
  } else {
    fx_layer_blending_config_c = 36;
    if (game_state != 27) {
      MotherBrain_CalculateRainbowBeamHdma();
      UpdateMotherBrainRainbowBeamColor();
    }
  }
}

static void UpdateMotherBrainRainbowBeamColor(void) {  // 0x88E7ED
  uint16 v0 = kMotherBrainRainbowBeamColors[hdma_object_A[0] >> 1];
  if ((v0 & 0x8000) == 0) {
    ++hdma_object_A[0];
    ++hdma_object_A[0];
    ++hdma_object_A[0];
    ++hdma_object_A[0];
  } else {
    hdma_object_A[0] = 0;
    v0 = kMotherBrainRainbowBeamColors[0];
  }
  reg_COLDATA[0] = v0 & 0x1F | 0x20;
  reg_COLDATA[1] = (v0 >> 5) & 0x1F | 0x40;
  reg_COLDATA[2] = ((uint16)(v0 >> 2) >> 8) & 0x1F | 0x80;
}

void SpawnMorphBallEyeBeamHdma(void) {  // 0x88E8D9
  SpawnHdmaObject(0x88, &kMorphBallEyeBeamHdmaArgs);
}

const uint8 *HdmaobjInstr_InitMorphBallEyeBeamHdma(uint16 k, const uint8 *hdp) {  // 0x88E917
  unsigned int v2; // kr00_4
  unsigned int v3; // kr04_4

  *((uint8 *)hdma_object_A + k) = 48;
  reg_COLDATA[0] = 48;
  *((uint8 *)hdma_object_A + k + 1) = 80;
  reg_COLDATA[1] = 80;
  *((uint8 *)hdma_object_B + k) = 0x80;
  reg_COLDATA[2] = 0x80;
  fx_layer_blending_config_c = 16;
  kraid_unk9000 = 228;
  v2 = 14979328;
  *(uint16 *)((uint8 *)&g_word_7E9002 + 1) = HIWORD(v2);
  *(uint16 *)((uint8 *)&kraid_unk9000 + 1) = v2;
  g_word_7E9004 = -28216;
  g_word_7E9006 = 152;
  v3 = 37520;
  *(uint16 *)((uint8 *)&g_word_7E9008 + 1) = HIWORD(v3);
  *(uint16 *)((uint8 *)&g_word_7E9006 + 1) = v3;
  enemy_data[1].ai_var_C = 1;
  int v4 = k >> 1;
  hdma_object_C[v4] = 0;
  hdma_object_D[v4] = 0;
  g_word_7E9080 = 0;
  g_word_7E9082 = 0;
  g_word_7E9090 = 0;
  return hdp;
}

static void UpdateMorphBallEyeBeamHdma(uint16 v0) {  // 0x88E987
  uint16 v3 = v0;
  uint16 r18 = enemy_data[1].ai_var_D;
  uint16 r20 = hdma_object_C[v0 >> 1];
  uint16 v1 = enemy_data[1].y_pos - layer1_y_pos;
  uint16 v2 = enemy_data[1].x_pos - layer1_x_pos;
  if ((int16)(enemy_data[1].x_pos - layer1_x_pos) >= 0 && sign16(enemy_data[1].x_pos - layer1_x_pos - 256)) {
    CalculateXrayHdmaTableInner(v2, v1, r18, r20, false, &g_word_7E9100);
  } else {
    CalculateXrayHdmaTableInner(v2, v1, r18, r20, true, &g_word_7E9100);
  }
  reg_COLDATA[0] = *((uint8 *)hdma_object_A + v3);
  reg_COLDATA[1] = *((uint8 *)hdma_object_A + v3 + 1);
  reg_COLDATA[2] = *((uint8 *)hdma_object_B + v3);
}

void HdmaobjPreInstr_E9E6(uint16 k) {  // 0x88E9E6
  int v2 = k >> 1;

  fx_layer_blending_config_c = 16;
  AddToHiLo(&g_word_7E9080, &g_word_7E9082, 0x4000);
  AddToHiLo(&hdma_object_C[v2], &hdma_object_D[v2], __PAIR32__(g_word_7E9080, g_word_7E9082));
  if (!sign16(hdma_object_C[v2] - 4)) {
    hdma_object_C[v2] = 4;
    int v5 = hdma_object_index >> 1;
    hdma_object_instruction_list_pointers[v5] += 2;
    hdma_object_instruction_timers[v5] = 1;
  }
  UpdateMorphBallEyeBeamHdma(k);
}

void HdmaobjPreInstr_EA3C(uint16 k) {  // 0x88EA3C
  fx_layer_blending_config_c = 16;
  if (enemy_data[1].ai_var_C) {
    UpdateMorphBallEyeBeamHdma(k);
    uint16 v2 = 4 * g_word_7E9090;
    *((uint8 *)hdma_object_A + k) = kMorphBallEyeBeamColors[(uint16)(4 * g_word_7E9090)];
    *((uint8 *)hdma_object_A + k + 1) = kMorphBallEyeBeamColors[v2 + 1];
    *((uint8 *)hdma_object_B + k) = kMorphBallEyeBeamColors[v2 + 2];
    g_word_7E9090 = (g_word_7E9090 + 1) & 0xF;
  } else {
    int v1 = hdma_object_index >> 1;
    hdma_object_instruction_list_pointers[v1] += 2;
    hdma_object_instruction_timers[v1] = 1;
  }
}

void HdmaobjPreInstr_EACB(uint16 k) {  // 0x88EACB
  fx_layer_blending_config_c = 16;
  if (*((uint8 *)hdma_object_A + k + 1) == 64) {
    reg_COLDATA[0] = 32;
    reg_COLDATA[1] = 64;
    reg_COLDATA[2] = 0x80;
    int v1 = k >> 1;
    hdma_object_D[v1] = 0;
    hdma_object_C[v1] = 0;
    for (int i = 510; i >= 0; i -= 2)
      *(uint16 *)((uint8 *)&g_word_7E9100 + (uint16)i) = 255;
    kraid_unk9000 = 0;
    g_word_7E9002 = 0;
    g_word_7E9004 = 0;
    g_word_7E9006 = 0;
    g_word_7E9008 = 0;
    int v3 = hdma_object_index >> 1;
    hdma_object_instruction_list_pointers[v3] += 2;
    hdma_object_instruction_timers[v3] = 1;
  } else {
    UpdateMorphBallEyeBeamHdma(k);
    uint16 v4 = hdma_object_index;
    if (*((uint8 *)hdma_object_A + hdma_object_index) != 32)
      --*((uint8 *)hdma_object_A + hdma_object_index);
    if (*((uint8 *)hdma_object_A + v4 + 1) != 64)
      --*((uint8 *)hdma_object_A + v4 + 1);
    if (*((uint8 *)hdma_object_B + v4) != 0x80)
      --*((uint8 *)hdma_object_B + v4);
  }
}
