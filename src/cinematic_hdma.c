// Cinematic/title HDMA families extracted from Bank 88.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"

static const SpawnHdmaObject_Args kTitleGradientBackdropHdmaArgs = { 0x00, 0x31, 0xeb85 };
static const SpawnHdmaObject_Args kTitleGradientColorMathHdmaArgs = { 0x00, 0x32, 0xeb73 };
static const SpawnHdmaObject_Args kIntroCutsceneCrossfadeHdmaArgs = { 0x00, 0x31, 0xec03 };
static const SpawnHdmaObject_Args kIntroFunc133HdmaArgs = { 0x42, 0x11, 0xec8a };

void SpawnTitleScreenGradientObjs(void) {  // 0x88EB58
  SpawnHdmaObject(0x88, &kTitleGradientColorMathHdmaArgs);
  SpawnHdmaObject(0x88, &kTitleGradientBackdropHdmaArgs);
}

const uint8 *HdmaobjInsr_ConfigTitleSequenceGradientHDMA(uint16 k, const uint8 *hdp) {  // 0x88EB9F
  reg_CGWSEL = 0;
  ConfigureTitleSequenceGradientHDMA();
  return hdp;
}

void HdmaobjPreInstr_Backdrop_TitleSequenceGradient(uint16 k) {  // 0x88EBB0
  ConfigureTitleSequenceGradientHDMA();
  if (cinematic_function == FUNC16(CinematicFunctionOpening)) {
    int v1 = hdma_object_index >> 1;
    hdma_object_instruction_list_pointers[v1] += 2;
    hdma_object_instruction_timers[v1] = 1;
  }
}

void HdmaobjPreInstr_ColorMathControlB_TitleGradient(uint16 k) {  // 0x88EBD2
  if (cinematic_function == FUNC16(CinematicFunctionOpening)) {
    int v1 = hdma_object_index >> 1;
    hdma_object_instruction_list_pointers[v1] += 2;
    hdma_object_instruction_timers[v1] = 1;
  }
}

void SpawnIntroCutsceneCrossfadeHdma(void) {  // 0x88EBF0
  SpawnHdmaObject(0x88, &kIntroCutsceneCrossfadeHdmaArgs);
}

void HdmaobjPreInstr_IntroCutsceneCrossfade(uint16 k) {  // 0x88EC1D
  if (eproj_x_pos[0] == 1) {
    int v1 = hdma_object_index >> 1;
    hdma_object_instruction_list_pointers[v1] += 2;
    hdma_object_instruction_timers[v1] = 1;
  }
}

void CinematicFunction_Intro_Func133(void) {  // 0x88EC3B
  unsigned int v0; // kr00_4
  unsigned int v1; // kr04_4

  g_word_7E0D9C = 1;
  *(uint16 *)&g_byte_7E0D9E = 0x4000;
  loop_counter_transfer_enemies_to_vram = 8;
  button_config_shoot_x_saved = 192;
  button_config_itemcancel_y_saved = 192;
  v0 = 12621824;
  *(uint16 *)((uint8 *)&button_config_jump_a_saved + 1) = HIWORD(v0);
  *(uint16 *)((uint8 *)&button_config_shoot_x_saved + 1) = v0;
  button_config_run_b_saved = -26496;
  v1 = 12621824;
  *(uint16 *)((uint8 *)&button_config_itemswitch_saved + 1) = HIWORD(v1);
  *(uint16 *)((uint8 *)&button_config_itemcancel_y_saved + 1) = v1;
  button_config_aim_down_L_saved = -26496;
  button_config_aim_up_R_saved = 0;
  SpawnHdmaObject(0x88, &kIntroFunc133HdmaArgs);
}

const uint8 *HdmaobjInstr_EC9F_ClearVars(uint16 k, const uint8 *hdp) {  // 0x88EC9F
  int v2 = k >> 1;
  hdma_object_A[v2] = -2;
  hdma_object_B[v2] = 1;
  hdma_object_C[v2] = 0;
  hdma_object_D[v2] = 0;
  return hdp;
}

void HdmaobjPreInstr_ECB6(uint16 k) {  // 0x88ECB6
  int16 v7;
  uint16 v8;

  if (g_word_7E0D9C) {
    uint16 r28 = 4;
    int n = 128;
    int v3 = k >> 1;
    hdma_object_A[v3] = (2 * loop_counter_transfer_enemies_to_vram + hdma_object_A[v3]) & 0x1FF;
    uint16 r20 = hdma_object_A[v3];
    uint16 v4 = 0;
    do {
      uint16 v10 = v4;
      uint16 v5 = r20;
      int v6 = r20 >> 1;
      v7 = kSinCosTable8bit_Sext[v6 + 64];
      if (v7 < 0) {
        uint16 r18 = -v7;
        uint16 r22 = Mult8x8(-(int8)v7, g_byte_7E0D9E) >> 8;
        r22 += Mult8x8(HIBYTE(r18), g_byte_7E0D9E);
        uint16 r24 = Mult8x8(r18, g_byte_7E0D9F);
        uint8 mult = Mult8x8(HIBYTE(r18), g_byte_7E0D9F);
        r22 += r24;
        r18 = ((r22 + (mult << 8)) & 0xFF00) >> 8;
        r20 = (r28 + v5) & 0x1FF;
        v8 = v10;
        hdma_table_1[v10 >> 1] = reg_BG3HOFS - r18;
      } else {
        uint16 r18 = kSinCosTable8bit_Sext[v6 + 64];
        uint16 r22 = Mult8x8(v7, g_byte_7E0D9E) >> 8;
        r22 += Mult8x8(HIBYTE(r18), g_byte_7E0D9E);
        uint16 r24 = Mult8x8(r18, g_byte_7E0D9F);
        uint8 mult = Mult8x8(HIBYTE(r18), g_byte_7E0D9F);
        r22 += r24;
        r18 = ((r22 + (mult << 8)) & 0xFF00) >> 8;
        r20 = (r28 + v5) & 0x1FF;
        v8 = v10;
        hdma_table_1[v10 >> 1] = r18 + reg_BG3HOFS;
      }
      v4 = v8 + 2;
    } while ((int16)(v4 - n) < 0);
    for (int i = 126; i >= 0; i -= 2)
      hdma_table_1[(i >> 1) + 64] = reg_BG3HOFS + reg_BG3HOFS - hdma_table_1[i >> 1];
  } else {
    int v2 = hdma_object_index >> 1;
    hdma_object_instruction_list_pointers[v2] += 2;
    hdma_object_instruction_timers[v2] = 1;
  }
}
