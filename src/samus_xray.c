// X-ray scope logic
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "sm_rtl.h"

#define unk_91CAF2 (*(SpawnHdmaObject_Args*)RomFixedPtr(0x91caf2))
#define stru_91D2D6 ((XrayBlockData*)RomFixedPtr(0x91d2d6))

static void Xray_Func12(uint16 dst_r22, const uint8 *jp);
static void Xray_Func13(uint16 dst_r22, uint16 j);
static void Xray_Func14(uint16 dst_r22, const uint8 *jp);
static void Xray_Func15(uint16 dst_r22, const uint8 *jp);
static void Xray_Func16(uint16 dst_r22, uint16 j);

typedef struct XrayHdmaCtx {
  uint16 r22;
  uint16 r24;
  uint16 r26;
  uint16 r28;
  uint16 r30;
  uint16 r32;
  uint16 r18_avoid;
  uint16 *dest;
} XrayHdmaCtx;

static void XrayFillUp(uint16 *dest, uint16 fill_with, uint16 v1) {
  do {
    dest[v1 >> 1] = fill_with;
  } while (!sign16(v1 -= 2));
}

static void XrayFillDown(uint16 *dest, uint16 fill_with, uint16 v1) {
  do {
    dest[v1 >> 1] = fill_with;
  } while ((v1 += 2) < 460);
}

static void XrayFillUpRamped(uint16 *dest, int pos, uint32 left, uint32 left_step, uint32 right, uint32 right_step, int adjust) {
  do {
    left += left_step;
    right += right_step;
    uint32 left_val = (left & ~0xffff) ? (((int32)left < 0) ? 0 : 0xffff) : left;
    uint32 right_val = (right & ~0xffff) ? (((int32)right < 0) ? 0 : 0xffff) : right;
    // If both left and right reached the end, fill the remainder with 0xff
    bool finished = (left & ~0xffff) && (right & ~0xffff) && (((left_step ^ left) | (right_step ^ right)) >> 31) == 0;
    dest[pos >> 1] = ((int32)right < 0 || (int32)left >= 0x10000) ? 0xff : (left_val >> 8 | right_val & ~0xff);
    if (finished) {
      pos += adjust;
      while ((pos -= 2) >= 0)
        dest[pos >> 1] = 0xff;
      return;
    }
  } while ((pos -= 2) >= 0);
}

static void XrayFillDownRamped(uint16 *dest, int pos, uint32 left, uint32 left_step, uint32 right, uint32 right_step, int adjust) {
  do {
    left += left_step;
    right += right_step;
    uint32 left_val = (left & ~0xffff) ? (((int32)left < 0) ? 0 : 0xffff) : left;
    uint32 right_val = (right & ~0xffff) ? (((int32)right < 0) ? 0 : 0xffff) : right;
    bool finished = (left & ~0xffff) && (right & ~0xffff) && (((left_step ^ left) | (right_step ^ right)) >> 31) == 0;
    // If both left and right got clamped, fill the remainder with 0xff
    dest[pos >> 1] = ((int32)right < 0 || (int32)left >= 0x10000) ? 0xff : (left_val >> 8 | right_val & ~0xff);
    if (finished) {
      pos += adjust;
      while ((pos += 2) < 460)
        dest[pos >> 1] = 0xff;
      return;
    }
  } while ((pos += 2) < 460);
}

typedef void XrayHdma0Func(XrayHdmaCtx *ctx);
static void XrayHdmaFunc_BeamAimedD(XrayHdmaCtx *ctx);
static void XrayHdmaFunc_BeamAimedL(XrayHdmaCtx *ctx);
static void XrayHdmaFunc_BeamAimedR(XrayHdmaCtx *ctx);
static void XrayHdmaFunc_BeamAimedU(XrayHdmaCtx *ctx);
static void XrayHdmaFunc_BeamHoriz(XrayHdmaCtx *ctx);
static void XrayHdmaOnScreen_BeamAimedD(XrayHdmaCtx *ctx);
static void XrayHdmaOnScreen_BeamAimedL(XrayHdmaCtx *ctx);
static void XrayHdmaOnScreen_BeamAimedR(XrayHdmaCtx *ctx);
static void XrayHdmaOnScreen_BeamAimedU(XrayHdmaCtx *ctx);
static void XrayHdmaOnScreen_BeamHoriz(XrayHdmaCtx *ctx);

static void XrayHdmaFunc_BeamAimedR(XrayHdmaCtx *ctx) {  // 0x91BEC2
  int p = 2 * ctx->r24;
  XrayFillUpRamped(ctx->dest, p - 2, ctx->r22 - 0x10000, ctx->r30, 65535, 1, 2);
  XrayFillDownRamped(ctx->dest, p, ctx->r22 - 0x10000, ctx->r32, 65535, 1, -2);
}

static void XrayHdmaFunc_BeamAimedL(XrayHdmaCtx *ctx) {  // 0x91BF72
  int t = 2 * ctx->r24;
  XrayFillUpRamped(ctx->dest, t - 2, 0, -1, ctx->r22 + 0x10000, -ctx->r32, 2);
  XrayFillDownRamped(ctx->dest, t, 0, -1, ctx->r22 + 0x10000, -ctx->r30, -2);
}

static void XrayHdmaFunc_BeamAimedU(XrayHdmaCtx *ctx) {
  int p = 2 * ctx->r24;
  if (sign16(ctx->r26 - 192)) {
    XrayFillUpRamped(ctx->dest, p - 2, ctx->r22 - 0x10000, ctx->r30, ctx->r22 - 0x10000, ctx->r32, 0);
  } else if (sign16(ctx->r28 - 192)) {
    XrayFillUpRamped(ctx->dest, p - 2, ctx->r22 + 0x10000, -ctx->r30, ctx->r22 - 0x10000, ctx->r32, 0);
  } else {
    XrayFillUpRamped(ctx->dest, p - 2, ctx->r22 + 0x10000, -ctx->r30, ctx->r22 + 0x10000, -ctx->r32, 2);
  }
  XrayFillDown(ctx->dest, 0xff, p);
}

static void XrayHdmaFunc_BeamAimedD(XrayHdmaCtx *ctx) {
  int p = 2 * ctx->r24;
  if (sign16(ctx->r28 - 128)) {
    XrayFillDownRamped(ctx->dest, p, ctx->r22 - 0x10000, ctx->r32, ctx->r22 - 0x10000, ctx->r30, 0);
  } else if (sign16(ctx->r26 - 128)) {
    XrayFillDownRamped(ctx->dest, p, ctx->r22 + 0x10000, -ctx->r32, ctx->r22 - 0x10000, ctx->r30, 0);
  } else {
    XrayFillDownRamped(ctx->dest, p, ctx->r22 + 0x10000, -ctx->r32, ctx->r22 + 0x10000, -ctx->r30, 0);
  }
  XrayFillUp(ctx->dest, 0xff, p - 2);
}

static void XrayHdmaFunc_BeamHoriz(XrayHdmaCtx *ctx) {  // 0x91C505
  uint16 v0 = 2 * (ctx->r24 - 1);
  ctx->dest[v0 >> 1] = 0xff00;
  XrayFillUp(ctx->dest, 0xff, v0 - 2);
  XrayFillDown(ctx->dest, 0xff, v0 + 2);
}

static void XrayHdmaOnScreen_BeamAimedR(XrayHdmaCtx *ctx) {  // 0x91C5FF
  int p = 2 * ctx->r24;
  ctx->dest[(p - 2) >> 1] = ctx->r22 >> 8 | 0xff00;
  XrayFillUpRamped(ctx->dest, p - 4, ctx->r22, ctx->r30, 65535, 1, 2);
  XrayFillDownRamped(ctx->dest, p, ctx->r22, ctx->r32, 65535, 1, -2);
}

static void XrayHdmaOnScreen_BeamAimedL(XrayHdmaCtx *ctx) {  // 0x91C660
  int v0 = 2 * ctx->r24;
  ctx->dest[(v0 - 2) >> 1] = (ctx->r22 >> 8) << 8;
  XrayFillUpRamped(ctx->dest, v0 - 4, 0, -1, ctx->r22, -ctx->r32, 2);
  XrayFillDownRamped(ctx->dest, v0, 0, -1, ctx->r22, -ctx->r30, -2);
}

static void XrayHdmaOnScreen_BeamAimedU(XrayHdmaCtx *ctx) {  // 0x91C6C1
  int v0 = 2 * ctx->r24;
  ctx->dest[(v0 - 2) >> 1] = (ctx->r22 >> 8) * 0x101;
  if (sign16(ctx->r26 - 192)) {
    XrayFillUpRamped(ctx->dest, v0 - 4, ctx->r22, ctx->r30, ctx->r22, ctx->r32, 0);
  } else if (sign16(ctx->r28 - 192)) {
    XrayFillUpRamped(ctx->dest, v0 - 4, ctx->r22, -ctx->r30, ctx->r22, ctx->r32, 0);
  } else {
    XrayFillUpRamped(ctx->dest, v0 - 4, ctx->r22, -ctx->r30, ctx->r22, -ctx->r32, 2);
  }
  XrayFillDown(ctx->dest, 0xff, v0);
}

static void XrayHdmaOnScreen_BeamAimedD(XrayHdmaCtx *ctx) {  // 0x91C822
  int v0 = 2 * ctx->r24;
  ctx->dest[(v0 - 2) >> 1] = (ctx->r22 >> 8) * 0x101;
  if (sign16(ctx->r28 - 128)) {
    XrayFillDownRamped(ctx->dest, v0, ctx->r22, ctx->r32, ctx->r22, ctx->r30, 0);
  } else if (sign16(ctx->r26 - 128)) {
    XrayFillDownRamped(ctx->dest, v0, ctx->r22, -ctx->r32, ctx->r22, ctx->r30, 0);
  } else {
    XrayFillDownRamped(ctx->dest, v0, ctx->r22, -ctx->r32, ctx->r22, -ctx->r30, 0);
  }
  XrayFillUp(ctx->dest, 0xff, v0 - 4);
}

static void XrayHdmaOnScreen_BeamHoriz(XrayHdmaCtx *ctx) {  // 0x91C998
  uint16 v0 = 2 * (ctx->r24 - 1);
  uint32 t = (ctx->r22 >> 8);
  ctx->dest[v0 >> 1] = (ctx->r18_avoid == 64) ? t | 0xFF00 : t << 8;
  XrayFillUp(ctx->dest, 0xff, v0 - 2);
  XrayFillDown(ctx->dest, 0xff, v0 + 2);
}

void CalculateXrayHdmaTableInner(uint16 k, uint16 j, uint16 r18, uint16 r20, bool off_screen, uint16 *dest_addr) {
  int v3;
  XrayHdmaCtx ctx[1];
  ctx->dest = (uint16*)dest_addr;
  ctx->r18_avoid = r18;
  ctx->r22 = k << 8;
  ctx->r24 = j;
  ctx->r26 = r18 - r20;
  ctx->r26 += (ctx->r26 & 0x8000) != 0 ? 0x100 : 0;
  ctx->r28 = r20 + r18;
  if (!sign16(r20 + r18 - 257))
    ctx->r28 = r20 + r18 - 256;
  ctx->r30 = kTanTable[sign16(ctx->r26 - 128) ? ctx->r26 : ctx->r26 - 128];
  ctx->r32 = kTanTable[sign16(ctx->r28 - 128) ? ctx->r28 : ctx->r28 - 128];

  if (!r20 && (r18 == 64 || r18 == 192)) {
    v3 = 8;
  } else if (sign16(ctx->r26 - 128)) {
    if (!sign16(ctx->r26 - 64)) {
      v3 = 4;
    } else if (!sign16(ctx->r28 - 64)) {
      v3 = 0;
    } else {
      v3 = 2;
    }
  } else {
    if (!sign16(ctx->r26 - 192)) {
      v3 = 2;
    } else if (!sign16(ctx->r28 - 192)) {
      v3 = 6;
    } else {
      v3 = 4;
    }
  }

  if (off_screen) {
    static XrayHdma0Func *const kXrayHdmaFuncs[5] = {  // 0x91BE11
      XrayHdmaFunc_BeamAimedR,
      XrayHdmaFunc_BeamAimedU,
      XrayHdmaFunc_BeamAimedD,
      XrayHdmaFunc_BeamAimedL,
      XrayHdmaFunc_BeamHoriz,
    };
    kXrayHdmaFuncs[v3 >> 1](ctx);
  } else {
    static XrayHdma0Func *const kXrayHdmaOnScreen_Funcs[5] = {
      XrayHdmaOnScreen_BeamAimedR,
      XrayHdmaOnScreen_BeamAimedU,
      XrayHdmaOnScreen_BeamAimedD,
      XrayHdmaOnScreen_BeamAimedL,
      XrayHdmaOnScreen_BeamHoriz,
    };
    kXrayHdmaOnScreen_Funcs[v3 >> 1](ctx);
  }
}

void HdmaobjPreInstr_XrayFunc0_NoBeam(uint16 k) {  // 0x888732
  if ((button_config_run_b & joypad1_lastkeys) != 0) {
    CalculateXrayHdmaTable();
    ++demo_input_pre_instr;
  } else {
    demo_input_pre_instr = 3;
  }
}

void HdmaobjPreInstr_XrayFunc1_BeamWidening(uint16 k) {  // 0x888754
  if ((button_config_run_b & joypad1_lastkeys) != 0) {
    AddToHiLo(&demo_input_instr_timer, &demo_input_instr_ptr, 2048);
    AddToHiLo(&demo_input, &demo_input_new, __PAIR32__(demo_input_instr_timer, demo_input_instr_ptr));
    if (!sign16(demo_input - 11)) {
      demo_input_new = 0;
      demo_input = 10;
      ++demo_input_pre_instr;
    }
    CalculateXrayHdmaTable();
  } else {
    demo_input_pre_instr = 3;
  }
}

void HdmaobjPreInstr_XrayFunc2_FullBeam(uint16 k) {  // 0x8887AB
  if ((button_config_run_b & joypad1_lastkeys) != 0) {
    HandleMovingXrayUpDown();
    CalculateXrayHdmaTable();
  } else {
    ++demo_input_pre_instr;
  }
}

void HandleMovingXrayUpDown(void) {  // 0x8887C5
  if ((button_config_up & joypad1_lastkeys) != 0) {
    MoveXrayUp();
  } else if ((button_config_down & joypad1_lastkeys) != 0) {
    MoveXrayDown();
  }
}

void MoveXrayUp(void) {  // 0x8887E0
  bool v0; // cf
  int16 v1;

  if (sign16(xray_angle - 128)) {
    if (xray_angle != demo_input) {
      if ((int16)(xray_angle - demo_input) < 0
          || (v0 = xray_angle != 0, --xray_angle, (int16)(xray_angle - (!v0 + demo_input)) < 0)) {
        xray_angle = demo_input;
      }
    }
  } else if (demo_input + xray_angle != 256) {
    if ((int16)(demo_input + xray_angle - 256) >= 0
        || (v1 = (__PAIR32__(demo_input, xray_angle) + __PAIR32__(xray_angle, 1)) >> 16, ++xray_angle, v1 != 256)
        && (int16)(v1 - 256) >= 0) {
      xray_angle = 256 - demo_input;
    }
  }
}

void MoveXrayDown(void) {  // 0x888835
  int16 v0;
  bool v1; // cf

  if (sign16(xray_angle - 128)) {
    if (demo_input + xray_angle != 128) {
      if ((int16)(demo_input + xray_angle - 128) >= 0
          || (v0 = (__PAIR32__(demo_input, xray_angle) + __PAIR32__(xray_angle, 1)) >> 16, ++xray_angle, v0 != 128)
          && (int16)(v0 - 128) >= 0) {
        xray_angle = 128 - demo_input;
      }
    }
  } else if (xray_angle - demo_input != 128) {
    if ((int16)(xray_angle - demo_input - 128) < 0
        || (v1 = xray_angle != 0, --xray_angle, xray_angle - (!v1 + demo_input) != 128)
        && (int16)(xray_angle - (!v1 + demo_input) - 128) < 0) {
      xray_angle = demo_input + 128;
    }
  }
}

void CalculateXrayHdmaTable(void) {  // 0x888896
  int16 v0;
  if (samus_pose_x_dir == 4)
    v0 = samus_x_pos - layer1_x_pos - 3;
  else
    v0 = samus_x_pos - layer1_x_pos + 3;
  uint16 v1;
  if (samus_movement_type == 5)
    v1 = samus_y_pos - layer1_y_pos - 12;
  else
    v1 = samus_y_pos - layer1_y_pos - 16;
  if (v0 < 0) {
    if (samus_pose_x_dir != 4) {
off_screen:
      CalculateXrayHdmaTableInner(v0, v1, xray_angle, demo_input, true, hdma_table_1);
      return;
    }
  } else {
    if ((int16)(v0 - 256) < 0) {
      CalculateXrayHdmaTableInner(v0, v1, xray_angle, demo_input, false, hdma_table_1);
      return;
    }
    if (samus_pose_x_dir != 8)
      goto off_screen;
  }

  for (int i = 510; i >= 0; i -= 2)
    hdma_table_1[i >> 1] = 255;
}

void HdmaobjPreInstr_XrayFunc3_DeactivateBeam(uint16 k) {  // 0x888934
  int16 v1;
  VramWriteEntry *v4;

  mov24(&hdma_ptr_1, 0x980001);
  *(uint16 *)((uint8 *)&demo_num_input_frames + 1) = 0;
  demo_input_prev = 0;
  demo_input_prev_new = 0;
  demo_backup_prev_controller_input = 0;
  hdma_table_1[0] = 255;
  v1 = 4096;
  if (fx_type != 36) {
    v1 = 0x2000;
    if (CanXrayShowBlocks())
      v1 = 0x4000;
  }
  fx_layer_blending_config_c |= v1;
  palette_buffer[0] = 0;
  int v2 = hdma_object_index >> 1;
  reg_BG2HOFS = hdma_object_A[v2];
  reg_BG2VOFS = hdma_object_B[v2];
  reg_BG2SC = *((uint8 *)hdma_object_C + hdma_object_index);
  uint16 v3 = vram_write_queue_tail;
  if ((int16)(vram_write_queue_tail - 240) < 0) {
    v4 = gVramWriteEntry(vram_write_queue_tail);
    v4->size = 2048;
    v4->src.addr = ADDR16_OF_RAM(ram4000) + 4096;
    *(uint16 *)&v4->src.bank = 126;
    v4->vram_dst = (reg_BG2SC & 0xFC) << 8;
    vram_write_queue_tail = v3 + 7;
    ++demo_input_pre_instr;
  }
}

void HdmaobjPreInstr_XrayFunc4_DeactivateBeam(uint16 k) {  // 0x8889BA
  int16 v1;
  VramWriteEntry *v3;

  v1 = 4096;
  if (fx_type != 36) {
    v1 = 0x2000;
    if (CanXrayShowBlocks())
      v1 = 0x4000;
  }
  fx_layer_blending_config_c |= v1;
  uint16 v2 = vram_write_queue_tail;
  if ((int16)(vram_write_queue_tail - 240) < 0) {
    v3 = gVramWriteEntry(vram_write_queue_tail);
    v3->size = 2048;
    v3->src.addr = ADDR16_OF_RAM(ram4000) + 6144;
    *(uint16 *)&v3->src.bank = 126;
    v3->vram_dst = ((reg_BG2SC & 0xFC) << 8) + 1024;
    vram_write_queue_tail = v2 + 7;
    ++demo_input_pre_instr;
  }
}

void HdmaobjPreInstr_XrayFunc5_DeactivateBeam(uint16 k) {  // 0x888A08
  int16 v1;
  v1 = 4096;
  if (fx_type != 36) {
    v1 = 0x2000;
    if (CanXrayShowBlocks())
      v1 = 0x4000;
  }
  fx_layer_blending_config_c |= v1;
  if (time_is_frozen_flag) {
    time_is_frozen_flag = 0;
    demo_input_pre_instr = 0;
    demo_input_instr_timer = 0;
    demo_input_instr_ptr = 0;
    xray_angle = 0;
    demo_input = 0;
    demo_input_new = 0;
    mov24(&hdma_ptr_1, 0x980001);
    *(uint16 *)((uint8 *)&demo_num_input_frames + 1) = 0;
    demo_input_prev = 0;
    demo_input_prev_new = 0;
    demo_backup_prev_controller_input = 0;
    ResponsibleForXrayStandupGlitch();
    hdma_object_channels_bitmask[hdma_object_index >> 1] = 0;
    QueueSfx1_Max6(0xA);
    if ((uint8)fx_type != 36) {
      reg_COLDATA[2] = 0x80;
      reg_COLDATA[1] = 64;
      reg_COLDATA[0] = 32;
    }
    for (int i = 510 / 2; i >= 0; i--)
      hdma_table_1[i] = 0xff;
    if (samus_auto_cancel_hud_item_index) {
      hud_item_index = 0;
      samus_auto_cancel_hud_item_index = 0;
    }
  }
}

static Func_Y_V *const kHdmaobjPreInstr_XrayFuncs[6] = {  // 0x8886EF
  HdmaobjPreInstr_XrayFunc0_NoBeam,
  HdmaobjPreInstr_XrayFunc1_BeamWidening,
  HdmaobjPreInstr_XrayFunc2_FullBeam,
  HdmaobjPreInstr_XrayFunc3_DeactivateBeam,
  HdmaobjPreInstr_XrayFunc4_DeactivateBeam,
  HdmaobjPreInstr_XrayFunc5_DeactivateBeam,
};

void HdmaobjPreInstr_Xray(uint16 k) {
  int16 v1;

  v1 = 4096;
  if (fx_type != 36) {
    v1 = 0x2000;
    if (CanXrayShowBlocks()) {
      v1 = 0x4000;
      *(uint16 *)&reg_COLDATA[0] = 0x27;
      *(uint16 *)&reg_COLDATA[1] = 0x47;
      *(uint16 *)&reg_COLDATA[2] = 0x87;
    }
  }
  fx_layer_blending_config_c |= v1;
  kHdmaobjPreInstr_XrayFuncs[demo_input_pre_instr](2 * demo_input_pre_instr);
}

void XrayRunHandler(void) {  // 0x91CAD6
  if (!time_is_frozen_flag && (button_config_run_b & joypad1_lastkeys) != 0) {
    if (Xray_Initialize() & 1)
      SpawnHdmaObject(0x91, &unk_91CAF2);
  }
}

void Xray_SetupStage1_FreezeTimeBackup(uint16 k) {  // 0x91CAF9
  LOBYTE(time_is_frozen_flag) = 1;
  *((uint8 *)hdma_object_A + (uint8)k) = reg_BG2HOFS;
  *((uint8 *)hdma_object_A + (uint8)k + 1) = HIBYTE(reg_BG2HOFS);
  *((uint8 *)hdma_object_B + (uint8)k) = reg_BG2VOFS;
  *((uint8 *)hdma_object_B + (uint8)k + 1) = HIBYTE(reg_BG2VOFS);
  *((uint8 *)hdma_object_C + (uint8)k) = reg_BG2SC;
}

void Xray_SetupStage2_ReadBg1_2ndScreen(void) {  // 0x91CB1C
  uint16 v0 = vram_read_queue_tail;
  *(uint16 *)((uint8 *)&vram_read_queue[0].vram_target + v0) = ((reg_BG1SC & 0xFC) << 8) + 1024;
  *(uint16 *)((uint8 *)&vram_read_queue[0].dma_parameters + v0) = 129;
  *(uint16 *)((uint8 *)&vram_read_queue[0].dma_parameters + v0 + 1) = 57;
  *(VoidP *)((uint8 *)&vram_read_queue[0].src.addr + v0) = ADDR16_OF_RAM(ram4000) + 0x2800;
  *(uint16 *)(&vram_read_queue[0].src.bank + v0) = 126;
  *(uint16 *)((uint8 *)&vram_read_queue[0].size + v0) = 2048;
  vram_read_queue_tail = v0 + 9;
}

void Xray_SetupStage3_ReadBg1_1stScreen(void) {  // 0x91CB57

  uint16 v0 = vram_read_queue_tail;
  *(uint16 *)((uint8 *)&vram_read_queue[0].vram_target + v0) = (reg_BG1SC & 0xFC) << 8;
  *(uint16 *)((uint8 *)&vram_read_queue[0].dma_parameters + v0) = 129;
  *(uint16 *)((uint8 *)&vram_read_queue[0].dma_parameters + v0 + 1) = 57;
  *(VoidP *)((uint8 *)&vram_read_queue[0].src.addr + v0) = ADDR16_OF_RAM(ram4000) + 0x2000;
  *(uint16 *)(&vram_read_queue[0].src.bank + v0) = 126;
  *(uint16 *)((uint8 *)&vram_read_queue[0].size + v0) = 2048;
  vram_read_queue_tail = v0 + 9;
}

void Xray_SetupStage4(void) {  // 0x91CB8E
  uint16 r24 = 4 * ((layer1_y_pos + bg1_y_offset) & 0xF0);
  r24 += ((layer1_x_pos + bg1_x_offset) & 0xF0) >> 3;
  r24 += 4 * ((layer1_x_pos + bg1_x_offset) & 0x100);
  uint16 r22 = 0;
  int m = 16;
  do {
    int n = 16;
    uint16 r26 = r24 & 0x7E0;
    uint16 r28 = r24 & 0x1F;
    uint16 r30 = 0;
    do {
      if (!sign16(r30 + r28 - 32)) {
        r26 = (r26 + 1024) & 0x7E0;
        r28 = 0;
        r30 = 0;
      }
      uint16 v0 = r30 + r28 + r26;
      uint16 v11 = ram4000.xray_tilemaps[v0 + 4096];
      uint16 v10 = ram4000.xray_tilemaps[v0 + 4097];
      uint16 v9 = ram4000.xray_tilemaps[v0 + 4128];
      uint16 v1 = ram4000.xray_tilemaps[v0 + 4129];
      *(uint16 *)((uint8 *)&ram4000.xray_tilemaps[33] + r22) = v1;
      *(uint16 *)((uint8 *)&ram4000.xray_tilemaps[32] + r22) = v9;
      *(uint16 *)((uint8 *)&ram4000.xray_tilemaps[1] + r22) = v10;
      *(uint16 *)((uint8 *)ram4000.xray_tilemaps + r22) = v11;
      r22 += 4;
      r30 += 2;
    } while (--n);
    Xray_SetupStage4_Func1(r22, r26, r28, r30);
    r24 = (r24 & 0x400) + ((r24 + 64) & 0x3FF);
    r22 += 64;
  } while (--m);
  assert((uint16)(layer1_y_pos >> 12) == 0);
  uint16 r34 = Mult8x8(layer1_y_pos >> 4, room_width_in_blocks) + (layer1_x_pos >> 4);
  r22 = 0;
  m = 16;
  do {
    Xray_SetupStage4_Func2(r22, r34);
    int n = 16;
    uint16 r36 = r34;
    do {
      Xray_SetupStage4_Func3(n, r22, r36);
      r22 += 4;
      r36 += 1;
    } while (--n);
    uint16 v12 = r22;
    r22 += 1984;
    Xray_SetupStage4_Func3(0, r22, r36);
    r22 = v12 + 64;
    r34 += room_width_in_blocks;
  } while (--m);
  LoadXrayBlocks();
  uint16 v7 = vram_read_queue_tail;
  *(uint16 *)((uint8 *)&vram_read_queue[0].vram_target + v7) = (reg_BG2SC & 0xFC) << 8;
  *(uint16 *)((uint8 *)&vram_read_queue[0].dma_parameters + v7) = 129;
  *(uint16 *)((uint8 *)&vram_read_queue[0].dma_parameters + v7 + 1) = 57;
  *(VoidP *)((uint8 *)&vram_read_queue[0].src.addr + v7) = 0x5000;
  *(uint16 *)(&vram_read_queue[0].src.bank + v7) = 126;
  *(uint16 *)((uint8 *)&vram_read_queue[0].size + v7) = 2048;
  vram_read_queue_tail = v7 + 9;
}

void Xray_SetupStage4_Func1(uint16 dst_r22, uint16 r26, uint16 r28, uint16 r30) {  // 0x91CCF1
  int16 v0;

  uint16 R32 = r26;
  v0 = r30 + r28;
  if (!sign16(r30 + r28 - 32)) {
    R32 = (R32 + 1024) & 0x7E0;
    v0 = 0;
  }
  uint16 v1 = R32 + v0;
  uint16 v6 = ram4000.xray_tilemaps[v1 + 4096];
  uint16 v5 = ram4000.xray_tilemaps[v1 + 4097];
  uint16 v4 = ram4000.xray_tilemaps[v1 + 4128];
  uint16 v3 = ram4000.xray_tilemaps[v1 + 4129];
  uint16 v2 = dst_r22 + 1984;
  *(uint16 *)((uint8 *)&ram4000.xray_tilemaps[33] + v2) = v3;
  *(uint16 *)((uint8 *)&ram4000.xray_tilemaps[32] + v2) = v4;
  *(uint16 *)((uint8 *)&ram4000.xray_tilemaps[1] + v2) = v5;
  *(uint16 *)((uint8 *)ram4000.xray_tilemaps + v2) = v6;
}

static const uint8 *Xray_GetXrayedBlock(uint16 k) {  // 0x91CDD6
  uint16 value;

  uint16 bts = BTS[k];
  uint16 r40 = level_data[k] & 0xF000;
  for (int i = 0; ; ++i) {
    value = stru_91D2D6[i].value;
    if (value == 0xFFFF)
      break;
    if (value == r40) {
      for (const uint8 *p = RomPtr_91(stru_91D2D6[i].addr); ; p += 4) {
        value = GET_WORD(p);
        if (value == 0xFFFF)
          break;
        if (value == 0xFF00 || value == bts)
          return RomPtr_91(GET_WORD(p + 2));
      }
      return NULL;
    }
  }
  return NULL;
}


void Xray_SetupStage4_Func2(uint16 dst_r22, uint16 r34) {  // 0x91CD42
  // bug: passing 0xffff to this function is invalid and will read invalid memory.
  if (r34 == 0)
    return;
  const uint8 *jp = Xray_GetXrayedBlock(r34 - 1);
  uint16 *dst = (uint16 *)((uint8 *)ram4000.xray_tilemaps + dst_r22);

  if (jp == NULL)
    return;

  if (GET_WORD(jp) != FUNC16(Xray_Func9)) {
    if (GET_WORD(jp) != FUNC16(Xray_Func11))
      return;
    TileTable *src = tile_table.tables + GET_WORD(jp + 8);
    dst[64] = src->top_left;
    dst[65] = src->top_right;
    dst[96] = src->bottom_left;
    dst[97] = src->bottom_right;
  }
  TileTable *src = tile_table.tables + GET_WORD(jp + 4);
  dst[0] = src->top_left;
  dst[1] = src->top_right;
  dst[32] = src->bottom_left;
  dst[33] = src->bottom_right;
}

static void Xray_CombinedMove(uint16 dst_r22, uint16 r36, bool which_dir) {
  uint16 y = r36 / room_width_in_blocks;
  uint16 x = r36 % room_width_in_blocks;
  uint16 r48 = level_data[r36] & 0xF000;
  int step = (int8)BTS[r36];
  if (step) {
    uint16 *variable = which_dir ? &y : &x;
    for (;;) {
      if ((int16)(*variable + step) < 0) {
        Xray_Func13(dst_r22, 0xFF);
        return;
      }
      *variable += step;
      int pos = y * room_width_in_blocks + x;
      step = (int8)BTS[pos];
      r48 = level_data[pos] & 0xF000;
      if (r48 == 0xd000) {
        variable = &y;
        continue;
      }
      if (r48 != 0x5000)
        break;
      if (step < 0)
        variable = &x; // wtf?
    }
  }

  if (stru_91D2D6[1].value == r48) {
    uint16 *t = (uint16 *)RomPtr_91(stru_91D2D6[1].addr);
    for (; t[0] != 0xffff; t += 2) {
      if (t[0] == 0xff00 || t[0] == (step & 0xff)) {
        Xray_Func12(dst_r22, RomPtr_91(t[1] + 2));
        return;
      }
    }
  }
}

static void Xray_Func7(uint16 dst_r22, const uint8 *jp) {  // 0x91CF36
  Xray_Func12(dst_r22, jp + 2);
}

static void Xray_Func8(uint16 dst_r22, const uint8 *jp) {  // 0x91CF3E
  if (area_index == 1)
    Xray_Func12(dst_r22, jp + 2);
}

static void Xray_Func9(uint16 r18, uint16 dst_r22, const uint8 *jp) {  // 0x91CF4E
  Xray_Func12(dst_r22, jp + 2);
  if (r18 != 1)
    Xray_Func14(dst_r22, jp + 4);
}

static void Xray_Func10(uint16 dst_r22, const uint8 *jp) {  // 0x91CF62
  Xray_Func12(dst_r22, jp + 2);
  Xray_Func15(dst_r22, jp + 2);
}

static void Xray_Func11(uint16 r18, uint16 dst_r22, const uint8 *jp) {  // 0x91CF6F
  Xray_Func12(dst_r22, jp + 2);
  if (r18 != 1)
    Xray_Func14(dst_r22, jp + 4);
  Xray_Func15(dst_r22, jp + 6);
  if (r18 != 1) {
    uint16 v5 = GET_WORD(jp + 8);
    uint16 top_left = tile_table.tables[v5].top_left;
    uint16 top_right = tile_table.tables[v5].top_right;
    uint16 bottom_left = tile_table.tables[v5].bottom_left;
    uint16 bottom_right = tile_table.tables[v5].bottom_right;
    *(uint16 *)((uint8 *)&ram4000.xray_tilemaps[99] + dst_r22) = bottom_right;
    *(uint16 *)((uint8 *)&ram4000.xray_tilemaps[98] + dst_r22) = bottom_left;
    *(uint16 *)((uint8 *)&ram4000.xray_tilemaps[67] + dst_r22) = top_right;
    *(uint16 *)((uint8 *)&ram4000.xray_tilemaps[66] + dst_r22) = top_left;
  }
}

static void CallXrayFunc(uint32 ea, const uint8 *jp, uint16 r18, uint16 r22, uint16 r36) {
  switch (ea) {
  case fnXray_Func6: Xray_CombinedMove(r22, r36, true); return;
  case fnXray_Func6B: Xray_CombinedMove(r22, r36, false); return;
  case fnXray_Func7: Xray_Func7(r22, jp); return;
  case fnXray_Func8: Xray_Func8(r22, jp); return;
  case fnXray_Func9: Xray_Func9(r18, r22, jp); return;
  case fnXray_Func10: Xray_Func10(r22, jp); return;
  case fnXray_Func11: Xray_Func11(r18, r22, jp); return;
  default: Unreachable();
  }
}

void Xray_SetupStage4_Func3(uint16 r18, uint16 r22, uint16 r36) {  // 0x91CDBE
  const uint8 *jp = Xray_GetXrayedBlock(r36);
  if (jp != NULL) {
    CallXrayFunc(GET_WORD(jp) | 0x910000, jp, r18, r22, r36);
  }
}

static void Xray_Func12(uint16 dst_r22, const uint8 *jp) {  // 0x91CFBF
  Xray_Func13(dst_r22, GET_WORD(jp));
}

static void Xray_Func13(uint16 dst_r22, uint16 a) {  // 0x91CFC1
  uint16 top_left = tile_table.tables[a].top_left;
  uint16 top_right = tile_table.tables[a].top_right;
  uint16 bottom_left = tile_table.tables[a].bottom_left;
  uint16 bottom_right = tile_table.tables[a].bottom_right;
  *(uint16 *)((uint8 *)&ram4000.xray_tilemaps[33] + dst_r22) = bottom_right;
  *(uint16 *)((uint8 *)&ram4000.xray_tilemaps[32] + dst_r22) = bottom_left;
  *(uint16 *)((uint8 *)&ram4000.xray_tilemaps[1] + dst_r22) = top_right;
  *(uint16 *)((uint8 *)&ram4000.xray_tilemaps[0] + dst_r22) = top_left;
}

static void Xray_Func14(uint16 dst_r22, const uint8 *jp) {  // 0x91CFEE
  uint16 a = GET_WORD(jp);
  uint16 top_left = tile_table.tables[a].top_left;
  uint16 top_right = tile_table.tables[a].top_right;
  uint16 bottom_left = tile_table.tables[a].bottom_left;
  uint16 bottom_right = tile_table.tables[a].bottom_right;
  *(uint16 *)((uint8 *)&ram4000.xray_tilemaps[35] + dst_r22) = bottom_right;
  *(uint16 *)((uint8 *)&ram4000.xray_tilemaps[34] + dst_r22) = bottom_left;
  *(uint16 *)((uint8 *)&ram4000.xray_tilemaps[3] + dst_r22) = top_right;
  *(uint16 *)((uint8 *)&ram4000.xray_tilemaps[2] + dst_r22) = top_left;
}

static void Xray_Func15(uint16 dst_r22, const uint8 *jp) {  // 0x91D01D
  uint16 a = GET_WORD(jp);
  uint16 top_left = tile_table.tables[a].top_left;
  uint16 top_right = tile_table.tables[a].top_right;
  uint16 bottom_left = tile_table.tables[a].bottom_left;
  uint16 bottom_right = tile_table.tables[a].bottom_right;
  *(uint16 *)((uint8 *)&ram4000.xray_tilemaps[97] + dst_r22) = bottom_right;
  *(uint16 *)((uint8 *)&ram4000.xray_tilemaps[96] + dst_r22) = bottom_left;
  *(uint16 *)((uint8 *)&ram4000.xray_tilemaps[65] + dst_r22) = top_right;
  *(uint16 *)((uint8 *)&ram4000.xray_tilemaps[64] + dst_r22) = top_left;
}

static void Xray_Func16(uint16 dst_r22, uint16 a) {  // 0x91D0A6
  uint16 top_left = tile_table.tables[a].top_left;
  uint16 top_right = tile_table.tables[a].top_right;
  uint16 bottom_left = tile_table.tables[a].bottom_left;
  uint16 bottom_right = tile_table.tables[a].bottom_right;
  *(uint16 *)((uint8 *)&ram4000.xray_tilemaps[1] + dst_r22) = bottom_right;
  *(uint16 *)((uint8 *)&ram4000.xray_tilemaps[0] + dst_r22) = bottom_left;
  *(uint16 *)((uint8 *)&ram4000.xray_tilemaps[33] + dst_r22) = top_right;
  *(uint16 *)((uint8 *)&ram4000.xray_tilemaps[32] + dst_r22) = top_left;
}

void LoadBlockToXrayTilemap(uint16 a, uint16 k, uint16 j) {  // 0x91D04C
  uint16 R24 = k - (layer1_x_pos >> 4);
  uint16 R26 = j - (layer1_y_pos >> 4);
  if (!sign16(R24) && sign16(R24 - 16) && !sign16(R26) && sign16(R26 - 16)) {
    uint16 R22 = 4 * (R24 + 32 * R26);
    if ((a & 0x800) != 0)
      Xray_Func16(R22, a & 0x3FF);
    else
      Xray_Func13(R22, a & 0x3FF);
  }
}

void Xray_SetupStage5(void) {  // 0x91D0D3
  unsigned int v1;

  if (CanXrayShowBlocks()) {
    if (earthquake_timer) {
      reg_BG1HOFS = layer1_x_pos + bg1_x_offset;
      reg_BG1VOFS = layer1_y_pos + bg1_y_offset;
    }
    reg_BG2HOFS = reg_BG1HOFS & 0xF;
    reg_BG2VOFS = reg_BG1VOFS & 0xF;
    reg_BG2SC = 73;
  }
  uint16 v0 = vram_read_queue_tail;
  v1 = vram_read_queue_tail;
  *(uint16 *)((uint8 *)&vram_read_queue[0].vram_target + vram_read_queue_tail) = ((reg_BG2SC & 0xFC) << 8) + 1024;
  *(uint16 *)((uint8 *)&vram_read_queue[0].dma_parameters + v1 + 1) = 57;
  *(VoidP *)((uint8 *)&vram_read_queue[0].src.addr + v0) = 0x5800;
  *(uint16 *)(&vram_read_queue[0].src.bank + v0) = 126;
  *(uint16 *)((uint8 *)&vram_read_queue[0].size + v0) = 2048;
  vram_read_queue_tail = v0 + 9;
}

bool CanXrayShowBlocks(void) {  // 0x91D143
  if (room_ptr == addr_kRoom_a66a || room_ptr == addr_kRoom_cefb)
    return false;
  if (fx_type == 0x24)
    return false;
  if (boss_id == 3 || boss_id == 6 || boss_id == 7 || boss_id == 8 || boss_id == 10)
    return false;
  return true;
}

void Xray_SetupStage6(void) {  // 0x91D173
  VramWriteEntry *v1;

  if (CanXrayShowBlocks()) {
    uint16 v0 = vram_write_queue_tail;
    v1 = gVramWriteEntry(vram_write_queue_tail);
    v1->size = 2048;
    v1->src.addr = ADDR16_OF_RAM(ram4000);
    *(uint16 *)&v1->src.bank = 126;
    v1->vram_dst = (reg_BG2SC & 0xFC) << 8;
    vram_write_queue_tail = v0 + 7;
  }
}

void Xray_SetupStage7(void) {  // 0x91D1A0
  VramWriteEntry *v1;

  if (CanXrayShowBlocks()) {
    uint16 v0 = vram_write_queue_tail;
    v1 = gVramWriteEntry(vram_write_queue_tail);
    v1->size = 2048;
    v1->src.addr = 0x4800;
    *(uint16 *)&v1->src.bank = 126;
    v1->vram_dst = ((reg_BG2SC & 0xFC) << 8) + 1024;
    vram_write_queue_tail = v0 + 7;
  }
  mov24(&hdma_ptr_1, 0x9800E4);
  mov24(&hdma_ptr_2, 0x98C8E4);
  mov24(&hdma_ptr_3, 0x999098);
  hdma_var_1 = 0;
  demo_input_pre_instr = 0;
  demo_input_instr_timer = 0;
  demo_input_instr_ptr = 0;
  demo_input = 0;
  demo_input_new = 0;
  if (samus_pose_x_dir == 4)
    xray_angle = 192;
  else
    xray_angle = 64;
}

void HdmaobjPreInstr_XraySetup(uint16 k) {  // 0x91D27F
  uint16 v1;

  v1 = 4096;
  if (fx_type == 36) {
    if (!sign16((reg_COLDATA[0] & 0x1F) - 7))
      goto LABEL_5;
    goto LABEL_4;
  }
  v1 = 0x2000;
  if (CanXrayShowBlocks()) {
    v1 = 0x4000;
LABEL_4:
    *(uint16 *)&reg_COLDATA[0] = 0x27;
    *(uint16 *)&reg_COLDATA[1] = 0x47;
    *(uint16 *)&reg_COLDATA[2] = 0x87;
  }
LABEL_5:
  fx_layer_blending_config_c |= v1;
}

void Xray_SetupStage8_SetBackdropColor(void) {  // 0x91D2BC
  palette_buffer[0] = 3171;
}

uint8 Xray_Initialize(void) {  // 0x91E16D
  static const uint8 byte_91E291[28] = {
  1, 1, 0, 0, 0, 2, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 1, 0, 0,
  0, 0, 0, 0,
  };

  if (cooldown_timer == 7 && bomb_counter == 5 && samus_x_speed_divisor == 2
      || !sign16(samus_pose - kPose_A4_FaceR_LandJump)
      && (sign16(samus_pose - kPose_A8_FaceR_Grappling)
          || !sign16(samus_pose - kPose_E0_FaceR_LandJump_AimU) && sign16(samus_pose - kPose_E8_FaceR_Drained_CrouchFalling))
      || game_state != 8
      || power_bomb_explosion_status
      || samus_y_speed
      || samus_y_subspeed
      || !byte_91E291[samus_prev_movement_type]) {
    return 0;
  }
  if (byte_91E291[samus_movement_type] == 1) {
    if (samus_pose_x_dir == 4)
      samus_new_pose_interrupted = kPose_D6_FaceL_Xray_Stand;
    else
      samus_new_pose_interrupted = kPose_D5_FaceR_Xray_Stand;
  } else {
    if (byte_91E291[samus_movement_type] != 2)
      return 0;
    if (samus_pose_x_dir == 4)
      samus_new_pose_interrupted = kPose_DA_FaceL_Xray_Crouch;
    else
      samus_new_pose_interrupted = kPose_D9_FaceR_Xray_Crouch;
  }
  time_is_frozen_flag = 1;
  samus_special_transgfx_index = 5;
  for (int i = 510; i >= 0; i -= 2)
    hdma_table_1[i >> 1] = 255;
  DisableEprojs();
  DisablePLMs();
  DisableAnimtiles();
  DisablePaletteFx();
  mov24(&hdma_ptr_1, 0x980001);
  *(uint16 *)((uint8 *)&demo_num_input_frames + 1) = 0;
  demo_input_prev = -26424;
  mov24(&hdma_ptr_3, 0x999098);
  hdma_var_1 = 0;
  demo_input_pre_instr = 0;
  demo_input_instr_timer = 0;
  demo_input_instr_ptr = 0;
  demo_input = 0;
  demo_input_new = 0;
  if (samus_pose_x_dir == 4)
    xray_angle = 192;
  else
    xray_angle = 64;
  return 1;
}

void ResponsibleForXrayStandupGlitch(void) {  // 0x91E2AD
  if (samus_movement_type == kMovementType_05_Crouching) {
    if (samus_pose_x_dir == 4)
      samus_pose = kPose_28_FaceL_Crouch;
    else
      samus_pose = kPose_27_FaceR_Crouch;
  } else if (samus_pose_x_dir == kPose_04_FaceL_AimU) {
    samus_pose = kPose_02_FaceL_Normal;
  } else {
    samus_pose = kPose_01_FaceR_Normal;
  }
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
  samus_last_different_pose = samus_prev_pose;
  *(uint16 *)&samus_last_different_pose_x_dir = *(uint16 *)&samus_prev_pose_x_dir;
  samus_prev_pose = samus_pose;
  *(uint16 *)&samus_prev_pose_x_dir = *(uint16 *)&samus_pose_x_dir;
  samus_movement_handler = FUNC16(Samus_MovementHandler_Normal);
  samus_input_handler = FUNC16(Samus_InputHandler_E913);
  demo_timer_counter = -1;
  uint16 r18 = kPoseParams[samus_pose].y_radius - samus_y_radius;
  if ((r18 & 0x8000) == 0) {
    samus_y_pos -= r18;
    samus_prev_y_pos = samus_y_pos;
  }
  EnableEprojs();
  EnablePLMs();
  EnableAnimtiles();
  EnablePaletteFx();
}
