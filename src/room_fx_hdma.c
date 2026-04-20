// Bank $88 room FX HDMA families: scrolling sky, liquid FX, weather, and haze.

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"

#define g_word_88A938 ((uint16*)RomFixedPtr(0x88a938))
#define kHdmaScrollEntrys ((HdmaScrollEntry*)RomFixedPtr(0x88aec1))
#define g_word_88B589 ((uint16*)RomFixedPtr(0x88b589))
#define g_word_88B60A ((uint16*)RomFixedPtr(0x88b60a))
#define g_word_88D992 ((uint16*)RomFixedPtr(0x88d992))

static uint16 SetupSomeHdmaTablesBG3(uint16 r24, uint16 r0, uint16 r3, uint16 r6, uint16 r9) {  // 0x88A81C
  int16 v1;

  g_word_7E0598 = 32;
  *(uint16 *)mother_brain_indirect_hdma = 31;
  *(uint16 *)&mother_brain_indirect_hdma[1] = 0;
  uint16 v0 = 3;
  v1 = layer1_y_pos + 32;
  uint16 r18 = layer1_y_pos + 32;
  uint16 v2 = 0;
  while ((int16)(v1 - *(uint16 *)&RomPtr_88(r0)[v2]) < 0
      || (int16)(v1 - *(uint16 *)&RomPtr_88(r9)[v2]) >= 0) {
    v2 += 6;
    if ((int16)(v2 - r24) >= 0)
      return v0;
  }
  uint16 v3;
  if (sign16(r18 - 1248))
    v3 = r18 & 0xF;
  else
    v3 = r18 & 0x1F;
  uint16 R22_ = v3;
  *(uint16 *)&mother_brain_indirect_hdma[3] = *(uint16 *)&RomPtr_88(r3)[v2] - v3;
  *(uint16 *)&mother_brain_indirect_hdma[4] = R22_ + *(uint16 *)&RomPtr_88(r6)[v2] - g_word_7E0598;
  while (1) {
    uint16 r20 = mother_brain_indirect_hdma[v0];
    r18 += r20;
    v0 += 3;
    g_word_7E0598 += r20;
    if (!sign16(g_word_7E0598 - 224))
      break;
    while ((int16)(r18 - *(uint16 *)&RomPtr_88(r0)[v2]) < 0
        || (int16)(r18 - *(uint16 *)&RomPtr_88(r9)[v2]) >= 0) {
      v2 += 6;
      if ((int16)(v2 - r24) >= 0)
        return v0;
    }
    *(uint16 *)&mother_brain_indirect_hdma[v0] = *(uint16 *)&RomPtr_88(r3)[v2];
    *(uint16 *)&mother_brain_indirect_hdma[v0 + 1] = *(uint16 *)&RomPtr_88(r6)[v2] - g_word_7E0598;
  }
  return v0;
}

static void CallFxRisingFunction88(uint32 ea) {
  switch (ea) {
  case fnFxRisingFunction_LavaAcid: FxRisingFunction_LavaAcid(); return;
  case fnFxRisingFunction_LavaAcid_WaitToRise: FxRisingFunction_LavaAcid_WaitToRise(); return;
  case fnFxRisingFunction_LavaAcid_Raising: FxRisingFunction_LavaAcid_Raising(); return;
  case fnFxRisingFunction_C428_WaterNormal: FxRisingFunction_C428_WaterNormal(); return;
  case fnFxRisingFunction_WaterWaitToRise: FxRisingFunction_WaterWaitToRise(); return;
  case fnFxRisingFunction_WaterRising: FxRisingFunction_WaterRising(); return;
  default: Unreachable();
  }
}

uint8 RaiseOrLowerFx(void) {  // 0x88868C
  if ((fx_target_y_pos & 0x8000) != 0)
    return 1;
  uint32 r24_r22 = (int16)fx_y_vel << 8;
  if ((fx_y_vel & 0x8000) == 0) {
    uint16 v2 = (r24_r22 + __PAIR32__(fx_base_y_pos, fx_base_y_subpos)) >> 16;
    fx_base_y_subpos += r24_r22;
    if ((v2 & 0x8000) != 0)
      v2 = -1;
    fx_base_y_pos = v2;
    if (fx_target_y_pos >= v2) {
      return 0;
    } else {
      fx_base_y_pos = fx_target_y_pos;
      fx_base_y_subpos = 0;
      return 1;
    }
  } else {
    uint16 v0 = (r24_r22 + __PAIR32__(fx_base_y_pos, fx_base_y_subpos)) >> 16;
    fx_base_y_subpos += r24_r22;
    if ((v0 & 0x8000) != 0)
      v0 = 0;
    fx_base_y_pos = v0;
    if (fx_target_y_pos >= v0) {
      fx_base_y_pos = fx_target_y_pos;
      fx_base_y_subpos = 0;
    }
    return fx_target_y_pos >= v0;
  }
}

void FxTypeFunc_22_ScrollingSky(void) {  // 0x88A61B
  fx_y_pos = 1248;
  fx_type = 6;

  static const SpawnHdmaObject_Args unk_88A62E = { 0x02, 0x12, 0xad63 };
  static const SpawnHdmaObject_Args unk_88A636 = { 0x42, 0x11, 0xad4e };
  static const SpawnHdmaObject_Args unk_88A63E = { 0x42, 0x0d, 0xad39 };
  SpawnHdmaObject(0x88, &unk_88A62E);
  SpawnHdmaObject(0x88, &unk_88A636);
  SpawnHdmaObject(0x88, &unk_88A63E);
  HdmaobjPreInstr_FxType22_BG3Yscroll(0);
}

void HdmaobjPreInstr_FxType22_BG3Yscroll(uint16 k) {  // 0x88A643
  DamageSamusInTopRow();
  k = SetupSomeHdmaTablesBG3(78, addr_word_88A8E8, addr_word_88A8E8 + 2, addr_word_88A8E8 + 4, addr_word_88A8E8 + 6);
  *(uint16 *)&mother_brain_indirect_hdma[k] = 0;
}

const uint8 *HdmaobjInstr_SetFlagB(uint16 k, const uint8 *hdp) {  // 0x88A66C
  hdma_object_B[k >> 1] = 1;
  return hdp;
}

static const int16 g_word_88C46E[16] = {  // 0x88A673
  0, 1, 1, 0, 0, -1, -1, 0,
  0, 1, 1, 0, 0, -1, -1, 0,
};

void HdmaobjPreInstr_BG3Xscroll(uint16 k) {
  int16 v8;
  int16 v9;

  if (sign16(layer1_y_pos - 1024)) {
    if ((nmi_frame_counter_byte & 1) == 0)
      g_word_7E0596 = (g_word_7E0596 + 2) & 0x1E;
    uint16 v1 = g_word_7E0596;
    uint16 v2 = 0;
    do {
      g_word_7E9E80[v2 >> 1] = g_word_88A938[v1 >> 1];
      v2 += 2;
      v1 += 2;
    } while ((int16)(v1 - 32) < 0);
  } else {
    int v3 = k >> 1;
    if (hdma_object_B[v3]-- == 1) {
      hdma_object_B[v3] = 6;
      hdma_object_A[v3] = (hdma_object_A[v3] + 2) & 0x1F;
    }
    uint16 v5 = hdma_object_A[v3];
    for (int i = 30; i >= 0; i -= 2) {
      g_word_7E9E80[v5 >> 1] = g_word_88C46E[i >> 1] + reg_BG1HOFS;
      v5 = (v5 - 2) & 0x1F;
    }
  }
  *(uint16 *)&hdma_window_1_left_pos[0].field_0 = 0;
  *(uint16 *)scrolling_sky_bg2_indirect_hdma = 0;
  *(uint16 *)&hdma_window_1_left_pos[0].field_2 = 31;
  *(uint16 *)&scrolling_sky_bg2_indirect_hdma[2] = 31;
  uint16 r18 = 31;
  *(uint16 *)&hdma_window_1_left_pos[1].field_0 = -25088;
  *(uint16 *)&scrolling_sky_bg2_indirect_hdma[3] = -25088;
  if (sign16(layer1_y_pos - 1024)) {
    *(uint16 *)&scrolling_sky_bg2_indirect_hdma[2] = 0;
  } else {
    r18 = HdmaFunc_A786(0x105, r18, 177, 0);
  }
  uint16 v7 = 5;
  r18 = HdmaFunc_A786(5, r18, -24960, 128);
  v8 = 224 - r18;
  while ((int16)(v8 - 16) >= 0) {
    v8 -= 16;
    *(uint16 *)(&hdma_window_1_left_pos[0].field_0 + v7) = 144;
    *(uint16 *)&scrolling_sky_bg2_indirect_hdma[v7] = 144;
    *(uint16 *)(&hdma_window_1_left_pos[0].field_1 + v7) = -24960;
    *(uint16 *)&scrolling_sky_bg2_indirect_hdma[v7 + 1] = -24960;
    v7 += 3;
  }
  v9 = v8;
  if (v8)
    v9 = v8 + 128;
  *(uint16 *)(&hdma_window_1_left_pos[0].field_0 + v7) = v9;
  *(uint16 *)&scrolling_sky_bg2_indirect_hdma[v7] = v9;
  *(uint16 *)(&hdma_window_1_left_pos[0].field_1 + v7) = -24960;
  *(uint16 *)&scrolling_sky_bg2_indirect_hdma[v7 + 1] = -24960;
  *(uint16 *)(&hdma_window_1_left_pos[1].field_0 + v7) = 0;
  *(uint16 *)&scrolling_sky_bg2_indirect_hdma[v7 + 3] = 0;
}

uint16 HdmaFunc_A786(uint16 k, uint16 r18, uint16 r20, uint16 r22) {  // 0x88A786
  uint16 v0 = k;
  int16 v2;

  uint16 v1 = 1216 - layer1_y_pos;
  uint16 r24 = 1216 - layer1_y_pos;
  if ((int16)(1216 - layer1_y_pos) >= 0 && layer1_y_pos != 1216) {
    if (sign16(v1 - 128)) {
      r18 = 1216 - layer1_y_pos;
    } else {
      r18 = 1216 - layer1_y_pos;
      if (!sign16(v1 - 193))
        v1 = 193;
      r18 = v1;
      r24 = v1;
      while (1) {
        v2 = v1 - 16;
        if (v2 < 0)
          break;
        r24 = v2;
        *(uint16 *)(&hdma_window_1_left_pos[0].field_0 + v0) = r22 | 0x10;
        *(uint16 *)(&hdma_window_1_left_pos[0].field_1 + v0) = r20;
        v1 = r24;
        v0 += 3;
      }
    }
    *(uint16 *)(&hdma_window_1_left_pos[0].field_0 + v0) = r22 | r24;
    *(uint16 *)(&hdma_window_1_left_pos[0].field_1 + v0) = r20;
  }
  return r18;
}

static const SpawnHdmaObject_Args unk_88A7EF = { 0x42, 0x0f, 0xad76 };
static const SpawnHdmaObject_Args unk_88A80B = { 0x42, 0x0f, 0xad89 };

void FxTypeFunc_20(void) {  // 0x88A7D8
  layer2_scroll_x |= 1;
  layer2_scroll_y |= 1;
  SpawnHdmaObject(0x88, &unk_88A7EF);
  set_to_e0_by_scrolling_sky = 224;
  UNUSED_word_7E059C = 0;
}

void RoomSetupAsm_ScrollingSkyOcean(void) {  // 0x88A800
  reg_BG2SC = 74;
  SpawnHdmaObject(0x88, &unk_88A80B);
  set_to_e0_by_scrolling_sky = 224;
  UNUSED_word_7E059C = 0;
}

void DamageSamusInTopRow(void) {  // 0x88A8C4
  if ((int16)(samus_y_pos - samus_y_radius) < 0 || sign16(samus_y_pos - samus_y_radius - 17))
    samus_periodic_damage = 8;
}

void HdmaobjPreInstr_SkyLandBG2XscrollInner(uint16 k) {  // 0x88ADC2
  uint16 i;
  uint16 r24 = 0;
  reg_BG2SC = 74;
  uint16 v1 = 0;
  do {
    uint16 *v3 = (uint16 *)&g_ram[kHdmaScrollEntrys[v1].hdma_data_table_entry];
    AddToHiLo(&v3[1], &v3[0], __PAIR32__(kHdmaScrollEntrys[v1].scroll_speed, kHdmaScrollEntrys[v1].scroll_subspeed));
    v1++;
  } while (sign16(v1 * 8 - 184));
  scrolling_sky_bg2_hdma_data[44] = 0;
  scrolling_sky_bg2_hdma_data[45] = 0;
  *(uint16 *)scrolling_sky_bg2_indirect_hdma = 31;
  *(uint16 *)&scrolling_sky_bg2_indirect_hdma[1] = 1438;
  uint16 r18 = layer1_y_pos + 32;
  uint16 r20 = layer1_y_pos + 224;
  uint16 v5 = 0;
  uint16 v6 = 3;
  do {
    while ((int16)(r18 - kHdmaScrollEntrys[v5].top_pos) >= 0 && (int16)(r18 - kHdmaScrollEntrys[v5 + 1].top_pos) < 0) {
      uint16 v8 = kHdmaScrollEntrys[v5 + 1].top_pos - r18;
      r24 = v8;
      if (!sign16(v8 - 128)) {
        *(uint16 *)&scrolling_sky_bg2_indirect_hdma[v6] = 127;
        *(uint16 *)&scrolling_sky_bg2_indirect_hdma[v6 + 1] = kHdmaScrollEntrys[v5].hdma_data_table_entry + 2;
        v6 += 3;
        v8 = r24 - 127;
      }
      *(uint16 *)&scrolling_sky_bg2_indirect_hdma[v6] = v8;
      *(uint16 *)&scrolling_sky_bg2_indirect_hdma[v6 + 1] = kHdmaScrollEntrys[v5].hdma_data_table_entry + 2;
      r18 += r24;
      v6 += 3;
      if (!sign16(r18 - r20)) {
        *(uint16 *)&scrolling_sky_bg2_indirect_hdma[v6 + 3] = 0;
        return;
      }
    }
    ++v5;
  } while (sign16(v5 * 8 - 184));
  for (i = 1535 - r18; ; i = r24 - 127) {
    r24 = i;
    if (sign16(i - 128))
      break;
    *(uint16 *)&scrolling_sky_bg2_indirect_hdma[v6] = 127;
    *(uint16 *)&scrolling_sky_bg2_indirect_hdma[v6 + 1] = 181;
    v6 += 3;
  }
  *(uint16 *)&scrolling_sky_bg2_indirect_hdma[v6] = i;
  *(uint16 *)&scrolling_sky_bg2_indirect_hdma[v6 + 1] = 181;
  *(uint16 *)&scrolling_sky_bg2_indirect_hdma[v6 + 3] = 0;
}

void HdmaobjPreInstr_SkyLandBG2Xscroll(uint16 k) {  // 0x88ADB2
  if (!time_is_frozen_flag)
    HdmaobjPreInstr_SkyLandBG2XscrollInner(k);
}

void HdmaobjPreInstr_SkyLandBG2Xscroll2(uint16 k) {  // 0x88ADBA
  if (!time_is_frozen_flag)
    HdmaobjPreInstr_SkyLandBG2XscrollInner(k);
}

void RoomMainAsm_ScrollingSky(const uint16 *src) {  // 0x88AFA3
  if (time_is_frozen_flag) {
    WORD(scrolling_sky_bg2_indirect_hdma[0]) = 0;
  } else {
    reg_BG2VOFS = layer1_y_pos;
    uint16 v0 = vram_write_queue_tail;
    VramWriteEntry *v1 = gVramWriteEntry(vram_write_queue_tail);
    v1[0].size = 64;
    v1[1].size = 64;
    v1[2].size = 64;
    v1[3].size = 64;
    int tt = (layer1_y_pos & 0x7F8) - 16;
    // fixed: prevent out of bounds read when tt is negative.
    if (sign16(tt))
      tt = 0;
    int v2 = 8 * (uint8)tt;
    int y1 = (uint8)(tt >> 8);
    int y2 = (uint8)((tt >> 8) + 1);
    VoidP v3 = src[y1] + v2;
    v1[0].src.addr = v3;
    v1[1].src.addr = v3 + 64;
    VoidP v5 = src[y2] + v2;
    v1[2].src.addr = v5;
    v1[3].src.addr = v5 + 64;
    v1[0].src.bank = 0x8a;
    v1[1].src.bank = 0x8a;
    v1[2].src.bank = 0x8a;
    v1[3].src.bank = 0x8a;
    int t = (reg_BG2SC & 0xFC) << 8;
    uint16 v6 = t + 4 * ((layer1_y_pos - 16) & 0x1F8);
    v1[0].vram_dst = v6;
    v1[1].vram_dst = v6 + 32;
    uint16 v7 = t + 4 * ((layer1_y_pos + 240) & 0x1F8);
    v1[2].vram_dst = v7;
    v1[3].vram_dst = v7 + 32;
    vram_write_queue_tail = v0 + 28;
  }
}

void RoomCode_ScrollingSkyLand(void) {  // 0x88AF8D
  RoomMainAsm_ScrollingSky((const uint16 *)RomPtr_88(addr_off_88AD9C));
}

void RoomMainAsm_ScrollingSkyOcean(void) {  // 0x88AF99
  RoomMainAsm_ScrollingSky((const uint16 *)RomPtr_88(addr_off_88ADA6));
}

static const SpawnHdmaObject_Args unk_88B08C = { 0x42, 0x11, 0xb0ac };

static const uint16 kFirefleaFlashingShades[12] = { 0, 0x100, 0x200, 0x300, 0x400, 0x500, 0x600, 0x500, 0x400, 0x300, 0x200, 0x100 };
static const uint16 kFirefleaDarknessShades[6] = { 0, 0x600, 0xc00, 0x1200, 0x1800, 0x1900 };

void FxTypeFunc_24(void) {  // 0x88B07C
  fireflea_flashing_timer = 6;
  fireflea_flashing_index = 0;
  SpawnHdmaObject(0x88, &unk_88B08C);
  *(uint16 *)&hdma_window_1_left_pos[0].field_0 = 0;
  UNUSED_word_7E177C = 0;
  fireflea_darkness_level = 0;
  UNUSED_word_7E1780 = 24;
  UNUSED_word_7E1782 = kFirefleaFlashingShades[0];
}

void HdmaobjPreInstr_FirefleaBG3XScroll(uint16 k) {  // 0x88B0BC
  int8 v2;
  uint16 v1;

  fx_layer_blending_config_c = fx_layer_blending_config_c & 0xFF00 | 0xC;
  if (!time_is_frozen_flag) {
    if (!--fireflea_flashing_timer) {
      fireflea_flashing_timer = 6;
      if (sign16(fireflea_darkness_level - 10)) {
        v1 = fireflea_flashing_index + 1;
        if ((uint16)(fireflea_flashing_index + 1) >= 0xC)
          v1 = 0;
      } else {
        v1 = 6;
      }
      fireflea_flashing_index = v1;
    }
    v2 = (uint16)(kFirefleaDarknessShades[fireflea_darkness_level >> 1]
                  + kFirefleaFlashingShades[fireflea_flashing_index]) >> 8;
    reg_COLDATA[1] = v2 | 0x80;
    reg_COLDATA[2] = v2 | 0x40;
    reg_COLDATA[0] = v2 | 0x20;
  }
}

void ExpandingContractingHdmaEffect(void) {  // 0x88B17F
  if (!sign16(++set_to_e0_by_scrolling_sky - 4)) {
    set_to_e0_by_scrolling_sky = 0;
    if (UNUSED_hdma_contracting_flag) {
      message_box_animation_y_radius -= 1024;
      if (sign16(message_box_animation_y_radius - 0x2000)) {
        message_box_animation_y_radius = 0x2000;
        UNUSED_hdma_contracting_flag = 0;
      }
    } else {
      message_box_animation_y_radius += 1024;
      if (!sign16(message_box_animation_y_radius + 0x8000))
        ++UNUSED_hdma_contracting_flag;
    }
    message_box_animation_y0 = 128;
    message_box_animation_y1 = 128;
    message_box_animation_y2 = 127;
    message_box_animation_y3 = 127;
  }
  uint16 v0 = 2 * message_box_animation_y0;
  uint16 v1 = 2 * message_box_animation_y2;
  uint16 r18 = 0, r20 = 32;
  do {
    *(uint16 *)&mother_brain_indirect_hdma[v0] = message_box_animation_y1 - message_box_animation_y0;
    *(uint16 *)&mother_brain_indirect_hdma[v1] = message_box_animation_y3 - message_box_animation_y2;
    r18 += message_box_animation_y_radius;
    if (Unreachable()) {
      ++message_box_animation_y1;
      --message_box_animation_y3;
    }
    ++message_box_animation_y0;
    --message_box_animation_y2;
    v1 -= 2;
    v0 += 2;
  } while (--r20);
}

static const uint16 g_word_88B256[17] = { 0x46, 1, 0x46, 3, 0x46, 2, 0x46, 1, 0x46, 1, 0x46, 2, 0x46, 2, 0x46, 1, 0x8000 };

void HandleEarthquakeSoundEffect(void) {  // 0x88B21D
  int16 v1;

  if ((earthquake_sfx_timer & 0x8000) == 0 && (--earthquake_sfx_timer & 0x8000) != 0) {
    uint16 v0 = earthquake_sfx_index;
    v1 = g_word_88B256[earthquake_sfx_index >> 1];
    if (v1 < 0) {
      v0 = 0;
      v1 = g_word_88B256[0];
    }
    QueueSfx2_Max6(v1);
    earthquake_sfx_timer = g_word_88B256[(v0 >> 1) + 1] + (v0 & 1) + (random_number & 3);
    earthquake_sfx_index = v0 + 4;
  }
}

static const SpawnHdmaObject_Args unk_88B289 = { 0x42, 0x12, 0xc3e1 };
static const SpawnHdmaObject_Args unk_88B291 = { 0x42, 0x10, 0xc3f0 };
static const SpawnHdmaObject_Args unk_88B2B1 = { 0x42, 0x12, 0xc3e1 };
static const SpawnHdmaObject_Args unk_88B2B9 = { 0x42, 0x10, 0xc3f0 };

void FxTypeFunc_2_Lava(void) {  // 0x88B279
  fx_rising_function_bank_88 = FUNC16(FxRisingFunction_LavaAcid);
  lava_acid_y_pos = fx_base_y_pos;
  SpawnHdmaObject(0x88, &unk_88B289);
  SpawnHdmaObject(0x88, &unk_88B291);
  SpawnBG3ScrollHdmaObject();
  SpawnAnimtiles(addr_kAnimtiles_Lava);
}

void FxTypeFunc_4_Acid(void) {  // 0x88B2A1
  fx_rising_function_bank_88 = FUNC16(FxRisingFunction_LavaAcid);
  lava_acid_y_pos = fx_base_y_pos;
  SpawnHdmaObject(0x88, &unk_88B2B1);
  SpawnHdmaObject(0x88, &unk_88B2B9);
  SpawnBG3ScrollHdmaObject();
  SpawnAnimtiles(addr_kAnimtiles_Acid);
}

void FxHandleTide(void) {  // 0x88B2C9
  uint16 v2, v5;

  if (*(int16 *)((uint8 *)&fx_y_vel + 1) < 0) {
    fx_y_suboffset = 0;
    fx_y_offset = 0;
    int v0 = HIBYTE(tide_phase);
    uint16 v1 = 8 * kSinCosTable8bit_Sext[v0];
    if ((kSinCosTable8bit_Sext[v0] & 0x1000) != 0)
      --fx_y_offset;
    *(uint16 *)((uint8 *)&fx_y_suboffset + 1) = v1;
    if ((kSinCosTable8bit_Sext[v0] & 0x8000) == 0)
      v2 = tide_phase + 288;
    else
      v2 = tide_phase + 192;
    tide_phase = v2;
  } else if ((*(uint16 *)((uint8 *)&fx_y_vel + 1) & 0x4000) != 0) {
    fx_y_suboffset = 0;
    fx_y_offset = 0;
    int v3 = HIBYTE(tide_phase);
    uint16 v4 = 32 * kSinCosTable8bit_Sext[v3];
    if ((kSinCosTable8bit_Sext[v3] & 0x400) != 0)
      --fx_y_offset;
    *(uint16 *)((uint8 *)&fx_y_suboffset + 1) = v4;
    if ((kSinCosTable8bit_Sext[v3] & 0x8000) == 0)
      v5 = tide_phase + 224;
    else
      v5 = tide_phase + 128;
    tide_phase = v5;
  }
}

void FxRisingFunction_LavaAcid(void) {  // 0x88B343
  if (fx_y_vel) {
    if ((fx_y_vel & 0x8000) == 0) {
      if (fx_target_y_pos < fx_base_y_pos || fx_target_y_pos == fx_base_y_pos)
        return;
LABEL_8:
      fx_rising_function_bank_88 = FUNC16(FxRisingFunction_LavaAcid_WaitToRise);
      return;
    }
    if (fx_target_y_pos != fx_base_y_pos && fx_target_y_pos < fx_base_y_pos)
      goto LABEL_8;
  }
}

void FxRisingFunction_LavaAcid_WaitToRise(void) {  // 0x88B367
  HandleEarthquakeSoundEffect();
  earthquake_type = 21;
  earthquake_timer |= 0x20;
  if (!--fx_timer)
    fx_rising_function_bank_88 = FUNC16(FxRisingFunction_LavaAcid_Raising);
}

void FxRisingFunction_LavaAcid_Raising(void) {  // 0x88B382
  HandleEarthquakeSoundEffect();
  earthquake_type = 21;
  earthquake_timer |= 0x20;
  if (RaiseOrLowerFx() & 1) {
    fx_y_vel = 0;
    fx_rising_function_bank_88 = FUNC16(FxRisingFunction_LavaAcid);
  }
}

const uint8 *HdmaobjInstr_B3A9(uint16 k, const uint8 *hdp) {  // 0x88B3A9
  hdma_object_C[k >> 1] = 112;
  return hdp;
}

static const uint8 kLavaSoundEffects[8] = { 0x12, 0x13, 0x14, 0x12, 0x13, 0x14, 0x12, 0x13 };

void HdmaobjPreInstr_LavaAcidBG3YScroll(uint16 k) {  // 0x88B3B0
  int16 v2;
  int16 v3;
  int16 v4;
  int16 v8;

  fx_layer_blending_config_c = fx_layer_blending_config_b;
  if (!time_is_frozen_flag) {
    CallFxRisingFunction88(fx_rising_function_bank_88 | 0x880000);
    FxHandleTide();
    SetHiLo(&lava_acid_y_pos, &lava_acid_y_subpos, __PAIR32__(fx_y_offset, fx_y_suboffset) + __PAIR32__(fx_base_y_pos, fx_base_y_subpos));
    bg3_xpos = reg_BG1HOFS;
    *(uint16 *)mother_brain_indirect_hdma = 0;
    bg3_ypos = 0;
    v2 = (__PAIR32__(fx_y_offset, fx_y_suboffset) + __PAIR32__(fx_base_y_pos, fx_base_y_subpos)) >> 16;
    if (v2 >= 0) {
      v3 = v2 - layer1_y_pos;
      if (v3 <= 0) {
        v4 = (v3 ^ 0x1F) & 0x1F | 0x100;
LABEL_8:
        *(uint16 *)&mother_brain_indirect_hdma[2] = v4;
        if (fx_type == 2 && (lava_acid_y_pos & 0x8000) == 0) {
          int v5 = (uint8)hdma_object_index >> 1;
          if (hdma_object_C[v5]-- == 1) {
            hdma_object_C[v5] = 112;
            QueueSfx2_Max6(kLavaSoundEffects[random_number & 7]);
          }
        }
        random_number = swap16(random_number);
        if ((lava_acid_y_pos & 0x8000) == 0) {
          v8 = lava_acid_y_pos - layer1_y_pos + 256;
          if (v8 < 0) {
            v8 = 255;
LABEL_17:
            hdma_object_table_pointers[(uint8)hdma_object_index >> 1] = 3 * (((v8 ^ 0x1FF) + 1) & 0x3FF) + 0xB62A;
            return;
          }
          if ((uint16)v8 < 0x200)
            goto LABEL_17;
        }
        v8 = 511;
        goto LABEL_17;
      }
      if ((uint16)v3 < 0x100) {
        v4 = (uint8)~v3;
        goto LABEL_8;
      }
    }
    v4 = 0;
    goto LABEL_8;
  }
}

const uint8 *HdmaobjInstr_SetFlagB_Copy(uint16 k, const uint8 *hdp) {  // 0x88B4CE
  hdma_object_B[k >> 1] = 1;
  return hdp;
}

void HdmaobjPreInstr_LavaAcidBG2YScroll(uint16 k) {  // 0x88B4D5
  g_word_7E9C44 = reg_BG2VOFS;
  if (!time_is_frozen_flag && (fx_liquid_options & 6) != 0) {
    if ((fx_liquid_options & 2) != 0)
      Handle_LavaAcidBG2YScroll_Func3(k);
    else
      Handle_LavaAcidBG2YScroll_Func2(k);
  } else {
    Handle_LavaAcidBG2YScroll_Func1(k);
  }
  hdma_object_table_pointers[(uint8)hdma_object_index >> 1] = 3 * (reg_BG2VOFS & 0xF) + 0xC0B1;
}

void Handle_LavaAcidBG2YScroll_Func1(uint16 v0) {  // 0x88B51D
  WriteReg((SnesRegs)(*((uint8 *)hdma_object_bank_slot + v0) + 17153), 0x10);
  uint8 v1 = 30;
  uint16 v2 = reg_BG2VOFS & 0x1FF;
  do {
    g_word_7E9C46[v1 >> 1] = v2;
    v1 -= 2;
  } while ((v1 & 0x80) == 0);
}

void Handle_LavaAcidBG2YScroll_Func2(uint16 v0) {  // 0x88B53B
  WriteReg((SnesRegs)(*((uint8 *)hdma_object_bank_slot + v0) + 17153), 0xF);
  int v1 = v0 >> 1;
  if (hdma_object_B[v1]-- == 1) {
    hdma_object_B[v1] = 6;
    hdma_object_A[v1] = (hdma_object_A[v1] - 2) & 0x1E;
  }
  uint8 v3 = *((uint8 *)hdma_object_A + v0);
  uint8 v4 = 30;
  int n = 15;
  do {
    g_word_7E9C46[v4 >> 1] = (g_word_88B589[v3 >> 1] + reg_BG2HOFS) & 0x1FF;
    v3 = (v3 - 2) & 0x1E;
    v4 = (v4 - 2) & 0x1E;
  } while (--n >= 0);
}

void Handle_LavaAcidBG2YScroll_Func3(uint16 v0) {  // 0x88B5A9
  WriteReg((SnesRegs)(*((uint8 *)hdma_object_bank_slot + v0) + BBAD0), 0x10);
  int v1 = v0 >> 1;
  if (hdma_object_B[v1]-- == 1) {
    hdma_object_B[v1] = 4;
    hdma_object_A[v1] = (hdma_object_A[v1] - 2) & 0x1E;
  }
  uint8 v3 = (LOBYTE(hdma_object_A[v1]) + 2 * (reg_BG2VOFS & 0xF)) & 0x1E;
  uint8 v4 = (2 * (reg_BG2VOFS & 0xF) + 30) & 0x1E;
  int n = 15;
  do {
    g_word_7E9C46[v4 >> 1] = (g_word_88B60A[v3 >> 1] + reg_BG2VOFS) & 0x1FF;
    v4 = (v4 - 2) & 0x1E;
    v3 = (v3 - 2) & 0x1E;
  } while (--n >= 0);
}

static const SpawnHdmaObject_Args unk_88C40F = { 0x42, 0x11, 0xd856 };
static const SpawnHdmaObject_Args unk_88C41F = { 0x42, 0x0f, 0xd847 };

void FxTypeFunc_6_Water(void) {  // 0x88C3FF
  fx_rising_function_bank_88 = FUNC16(FxRisingFunction_C428_WaterNormal);
  fx_y_pos = fx_base_y_pos;
  SpawnHdmaObject(0x88, &unk_88C40F);
  if ((fx_liquid_options & 2) != 0)
    SpawnHdmaObject(0x88, &unk_88C41F);
  SpawnBG3ScrollHdmaObject();
}

void FxRisingFunction_C428_WaterNormal(void) {  // 0x88C428
  if (fx_y_vel) {
    if ((fx_y_vel & 0x8000) == 0) {
      if (fx_target_y_pos < fx_base_y_pos || fx_target_y_pos == fx_base_y_pos)
        return;
LABEL_8:
      fx_rising_function_bank_88 = FUNC16(FxRisingFunction_WaterWaitToRise);
      return;
    }
    if (fx_target_y_pos != fx_base_y_pos && fx_target_y_pos < fx_base_y_pos)
      goto LABEL_8;
  }
}

void FxRisingFunction_WaterWaitToRise(void) {  // 0x88C44C
  if (!--fx_timer)
    fx_rising_function_bank_88 = FUNC16(FxRisingFunction_WaterRising);
}

void FxRisingFunction_WaterRising(void) {  // 0x88C458
  if (RaiseOrLowerFx() & 1) {
    fx_rising_function_bank_88 = FUNC16(FxRisingFunction_C428_WaterNormal);
    fx_y_vel = 0;
  }
}

const uint8 *HdmaobjInstr_SetFlagB_Copy2(uint16 k, const uint8 *hdp) {  // 0x88C467
  hdma_object_B[k >> 1] = 1;
  return hdp;
}

void HdmaobjPreInstr_WaterBG3XScroll(uint16 k) {  // 0x88C48E
  fx_layer_blending_config_c = fx_layer_blending_config_b;
  if (time_is_frozen_flag)
    return;
  CallFxRisingFunction88(fx_rising_function_bank_88 | 0x880000);
  FxHandleTide();
  SetHiLo(&fx_y_pos, &fx_y_subpos, __PAIR32__(fx_y_offset, fx_y_suboffset) + __PAIR32__(fx_base_y_pos, fx_base_y_subpos));
  int16 v3 = fx_y_pos - layer1_y_pos;
  if ((int16)fx_y_pos < 0) {
    bg3_ypos = 0;
  } else if (v3 <= 0) {
    bg3_ypos = (v3 ^ 0x1F) & 0x1F | 0x100;
  } else if ((uint16)v3 < 0x100) {
    bg3_ypos = (uint8)~v3;
  } else {
    bg3_ypos = 0;
  }
  int v5 = k >> 1;
  uint16 r20 = layer1_x_pos + (int8)HIBYTE(hdma_object_C[v5]);
  if (hdma_object_B[v5]-- == 1) {
    hdma_object_B[v5] = 10;
    hdma_object_A[v5] = (hdma_object_A[v5] + 2) & 0x1E;
  }
  uint8 v7 = hdma_object_A[v5];
  for (int i = 30; (i & 0x80) == 0; i -= 2) {
    *(uint16 *)&mother_brain_indirect_hdma[v7 + 4] = g_word_88C46E[i >> 1] + r20;
    v7 = (v7 - 2) & 0x1E;
  }
  if ((fx_liquid_options & 1) != 0)
    hdma_object_C[k >> 1] += 64;
  int16 v9 = fx_y_pos - layer1_y_pos + 256;
  if ((int16)fx_y_pos < 0) {
    v9 = 511;
  } else if (v9 >= 0) {
    if ((uint16)v9 >= 0x200)
      v9 = 511;
  } else {
    v9 = v9 & 0xF | 0x100;
  }
  hdma_object_table_pointers[k >> 1] = 3 * (((v9 ^ 0x1FF) + 1) & 0x3FF) + 0xC645;
}

const uint8 *HdmaobjInstr_SetFlagB_Copy3(uint16 k, const uint8 *hdp) {  // 0x88C582
  hdma_object_B[k >> 1] = 1;
  return hdp;
}

void HdmaobjPreInstr_WaterBG2XScroll(uint16 k) {  // 0x88C589
  g_word_7E9C44 = reg_BG2HOFS;
  if (time_is_frozen_flag) {
    HdmaobjPreInstr_WaterBG2XScroll_Func1(k);
  } else if ((fx_liquid_options & 2) == 0) {
    hdma_object_channels_bitmask[(uint8)k >> 1] = 0;
    HdmaobjPreInstr_WaterBG2XScroll_Func1(k);
  } else {
    HdmaobjPreInstr_WaterBG2XScroll_Func2(k);
  }
  int16 v1 = fx_y_pos - layer1_y_pos + 256;
  if ((int16)fx_y_pos < 0) {
    v1 = 511;
  } else if (v1 >= 0) {
    if ((uint16)v1 >= 0x200)
      v1 = 511;
  } else {
    v1 = (fx_y_pos - layer1_y_pos) & 0xF | 0x100;
  }
  hdma_object_table_pointers[(uint8)k >> 1] = 3 * ((v1 ^ 0x1FF) & 0x3FF) + 0xCF46;
}

void HdmaobjPreInstr_WaterBG2XScroll_Func2(uint16 k) {  // 0x88C5E4
  int v1 = (uint8)k >> 1;
  if (hdma_object_B[v1]-- == 1) {
    hdma_object_B[v1] = 6;
    hdma_object_A[v1] = (hdma_object_A[v1] + 2) & 0x1E;
  }
  uint8 v3 = (LOBYTE(hdma_object_A[v1]) + 2 * (reg_BG2VOFS & 0xF)) & 0x1E;
  uint8 v4 = (2 * (reg_BG2VOFS & 0xF) + 30) & 0x1E;
  int n = 15;
  do {
    g_word_7E9C46[(v4 >> 1) + 1] = g_word_88C46E[v3 >> 1] + reg_BG2HOFS;
    v4 = (v4 - 2) & 0x1E;
    v3 = (v3 - 2) & 0x1E;
  } while (--n >= 0);
}

void HdmaobjPreInstr_WaterBG2XScroll_Func1(uint16 k) {  // 0x88C636
  uint8 v1 = 30;
  uint16 v2 = reg_BG2HOFS;
  do {
    g_word_7E9C46[(v1 >> 1) + 1] = v2;
    v1 -= 2;
  } while ((v1 & 0x80) == 0);
}

void SpawnBG3ScrollHdmaObject(void) {  // 0x88D865
  static const SpawnHdmaObject_Args unk_88D869 = { 0x43, 0x11, 0xd8d0 };
  SpawnHdmaObjectToSlot0xA(0x88, &unk_88D869);
}

void FxTypeFunc_28_CeresRidley(void) {  // 0x88D8DE
  static const SpawnHdmaObject_Args unk_88D8E2 = { 0x40, 0x05, 0xd906 };
  static const SpawnHdmaObject_Args unk_88D8EA = { 0x00, 0x2c, 0xd91d };
  SpawnHdmaObject(0x88, &unk_88D8E2);
  SpawnHdmaObject(0x88, &unk_88D8EA);
}

void sub_88D916(void) {  // 0x88D916
  hdma_data_table_in_ceres = 9;
}

void FxTypeFunc_CeresElevator(void) {  // 0x88D928
  static const SpawnHdmaObject_Args unk_88D92C = { 0x40, 0x05, 0xd939 };
  SpawnHdmaObject(0x88, &unk_88D92C);
}

const uint8 *HdmaobjInstr_SetVideoMode1(uint16 k, const uint8 *hdp) {  // 0x88D949
  hdma_data_table_in_ceres = 9;
  return hdp;
}

void FxTypeFunc_A_Rain(void) {  // 0x88D950
  static const SpawnHdmaObject_Args unk_88D95C = { 0x43, 0x11, 0xd96c };
  gameplay_BG3SC = 92;
  SpawnHdmaObject(0x88, &unk_88D95C);
  SpawnBG3ScrollHdmaObject();
  SpawnAnimtiles(addr_kAnimtiles_Rain);
}

const uint8 *HdmaobjInstr_1938_RandomNumber(uint16 k, const uint8 *hdp) {  // 0x88D981
  hdma_object_D[k >> 1] = g_word_88D992[(uint16)((random_number >> 1) & 6) >> 1];
  return hdp;
}

void HdmaobjPreInstr_RainBg3Scroll(uint16 k) {  // 0x88D9A1
  gameplay_BG3SC = 92;
  fx_layer_blending_config_c = fx_layer_blending_config_b;
  if (!time_is_frozen_flag) {
    int v1 = (uint8)k >> 1;
    bg3_ypos = hdma_object_E[v1] - layer1_y_pos + (int8)HIBYTE(hdma_object_A[v1]);
    hdma_object_A[v1] -= 1536;
    hdma_object_E[v1] = layer1_y_pos;
    bg3_xpos = hdma_object_F[v1] - layer1_x_pos + (int8)HIBYTE(hdma_object_B[v1]);
    hdma_object_B[v1] += hdma_object_D[v1];
    hdma_object_F[v1] = layer1_x_pos;
  }
}

void FxTypeFunc_8_Spores(void) {  // 0x88DA11
  gameplay_BG3SC = 92;
  static const SpawnHdmaObject_Args unk_88DA1D = { 0x42, 0x11, 0xda2d };
  SpawnHdmaObject(0x88, &unk_88DA1D);
  SpawnBG3ScrollHdmaObject();
  SpawnAnimtiles(addr_kAnimtiles_Spores);
}

void HdmaobjPreInstr_SporesBG3Xscroll(uint16 k) {  // 0x88DA47
  gameplay_BG3SC = 92;
  fx_layer_blending_config_c = fx_layer_blending_config_b;
  if (!time_is_frozen_flag) {
    int v1 = (uint8)k >> 1;
    bg3_ypos = layer1_y_pos + (int8)HIBYTE(hdma_object_C[v1]);
    uint16 v3 = hdma_object_C[v1] - 64;
    hdma_object_C[v1] = v3;
    bg3_xpos = layer1_x_pos + (int8)HIBYTE(hdma_object_D[v1]);
    hdma_object_D[v1] = hdma_object_D[v1];
  }
}

void FxTypeFunc_C(void) {  // 0x88DB08
  gameplay_BG3SC = 92;
  static const SpawnHdmaObject_Args unk_88DB14 = { 0x43, 0x11, 0xdb19 };
  SpawnHdmaObject(0x88, &unk_88DB14);
}

void HdmaobjPreInstr_FogBG3Scroll(uint16 k) {  // 0x88DB36
  gameplay_BG3SC = 92;
  fx_layer_blending_config_c = fx_layer_blending_config_b;
  if (!time_is_frozen_flag) {
    int v1 = (uint8)k >> 1;
    bg3_ypos = layer1_y_pos + (int8)HIBYTE(hdma_object_A[v1]);
    hdma_object_A[v1] -= 64;
    bg3_xpos = layer1_x_pos + (int8)HIBYTE(hdma_object_B[v1]);
    hdma_object_B[v1] += 80;
  }
}

void FxTypeFunc_26_TourianEntranceStatue(void) {  // 0x88DB8A
  static const SpawnHdmaObject_Args unk_88DBBA = { 0x42, 0x11, 0xd856 };
  static const SpawnHdmaObject_Args unk_88DBC2 = { 0x42, 0x10, 0xdcfa };

  if (CheckEventHappened(0xA) & 1) {
    SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x06, 0x0c, 0xb777 });
    *(uint16 *)scrolls = 514;
  }
  reg_BG2SC = 74;
  fx_rising_function_bank_88 = FUNC16(FxRisingFunction_C428_WaterNormal);
  fx_y_pos = fx_base_y_pos;
  SpawnHdmaObject(0x88, &unk_88DBBA);
  SpawnHdmaObject(0x88, &unk_88DBC2);
  SpawnBG3ScrollHdmaObject();
}

void sub_88DBCB(uint16 k) {  // 0x88DBCB
  *(uint16 *)&hdma_window_1_left_pos[0].field_0 = layer1_y_pos + hdma_object_B[k >> 1];
}

void HdmaobjPreInstr_CheckLotsOfEventsHappened(uint16 v0) {  // 0x88DBD7
  if (CheckEventHappened(6) & 1) {
    if (CheckEventHappened(7) & 1) {
      if (CheckEventHappened(8) & 1) {
        if (CheckEventHappened(9) & 1) {
          tourian_entrance_statue_animstate |= 0x10;
          if ((tourian_entrance_statue_animstate & 0x8000) == 0) {
            hdma_object_C[v0 >> 1] = 300;
            v0 = hdma_object_index;
            int v1 = hdma_object_index >> 1;
            hdma_object_instruction_timers[v1] = 1;
            hdma_object_instruction_list_pointers[v1] += 2;
          }
        }
      }
    }
  }
  sub_88DBCB(v0);
}

void HdmaobjPreInstr_DC23(uint16 k) {  // 0x88DC23
  uint16 v0 = k;

  HandleEarthquakeSoundEffect();
  earthquake_type = 13;
  earthquake_timer |= 0x20;
  int v1 = (uint8)v0 >> 1;
  if ((--hdma_object_C[v1] & 0x8000) != 0) {
    SpawnEprojWithRoomGfx(addr_kEproj_TourianStatueDustClouds, 0);
    SpawnEprojWithRoomGfx(addr_kEproj_TourianStatueDustClouds, 0);
    SpawnEprojWithRoomGfx(addr_kEproj_TourianStatueDustClouds, 0);
    SpawnEprojWithRoomGfx(addr_kEproj_TourianStatueDustClouds, 0);
    hdma_object_instruction_timers[v1] = 1;
    hdma_object_instruction_list_pointers[v1] += 2;
  }
  sub_88DBCB(v0);
}

void HdmaobjPreInstr_DC69(uint16 k) {  // 0x88DC69
  HandleEarthquakeSoundEffect();
  earthquake_type = 13;
  earthquake_timer |= 0x20;
  if (!time_is_frozen_flag) {
    int v1 = k >> 1;
    AddToHiLo(&hdma_object_B[v1], &hdma_object_A[v1], -0x4000);
    if (hdma_object_B[v1] == 0xFF10) {
      SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x06, 0x0c, 0xb773 });
      SetEventHappened(0xA);
      hdma_object_instruction_timers[v1] = 1;
      hdma_object_instruction_list_pointers[v1] += 2;
    }
    sub_88DBCB(k);
  }
}

void HdmaobjPreInstr_DCBA(uint16 v0) {  // 0x88DCBA
  tourian_entrance_statue_finished = 0x8000;
  *(uint16 *)scrolls = 514;
  sub_88DBCB(v0);
}

const uint8 *HdmaobjInstr_GotoIfEventHappened(uint16 k, const uint8 *hdp) {  // 0x88DCCB
  int v2 = k >> 1;
  hdma_object_C[v2] = 0;
  hdma_object_A[v2] = 0;
  if (CheckEventHappened(0xA)) {
    hdma_object_B[v2] = -240;
    *(uint16 *)&hdma_window_1_left_pos[0].field_0 = -240;
    return INSTRB_RETURN_ADDR(GET_WORD(hdp));
  } else {
    hdma_object_B[v2] = 0;
    *(uint16 *)&hdma_window_1_left_pos[0].field_0 = 0;
    *(uint16 *)scrolls = 1;
    return hdp + 2;
  }
}

void SpawnBombTorizoHaze(void) {  // 0x88DD32
  static const SpawnHdmaObject_Args unk_88DD36 = { 0x02, 0x32, 0xdd4a };
  static const SpawnHdmaObject_Args unk_88DD3E = { 0x00, 0x32, 0xdd62 };
  SpawnHdmaObject(0x88, &unk_88DD36);
  SpawnHdmaObject(0x88, &unk_88DD3E);
}

void HdmaobjPreInstr_BombTorizoHazeColorMathBgColor(uint16 k) {  // 0x88DD43
  fx_layer_blending_config_c = 44;
}

void FxTypeFunc_2C_Haze(void) {  // 0x88DDC7
  static const SpawnHdmaObject_Args unk_88DDD4 = { 0x40, 0x32, 0xded3 };
  static const SpawnHdmaObject_Args unk_88DDDD = { 0x40, 0x32, 0xdeeb };
  if (CheckBossBitForCurArea(1) & 1)
    SpawnHdmaObject(0x88, &unk_88DDDD);
  else
    SpawnHdmaObject(0x88, &unk_88DDD4);
}

void HdmaobjPreInstr_HazeColorMathSubscreen_CeresRidleyAlive(uint16 k) {  // 0x88DE10
  sub_88DE18(k, 0x80);
}

void HdmaobjPreInstr_HazeColorMathSubscreen_CeresRidleyDead(uint16 k) {  // 0x88DE15
  sub_88DE18(k, 0x20);
}

void sub_88DE18(uint16 k, uint16 a) {  // 0x88DE18
  int v2 = (uint8)k >> 1;
  hdma_object_B[v2] = a;
  hdma_object_A[v2] = 0;
  if (door_transition_function == FUNC16(DoorTransition_FadeInScreenAndFinish)) {
    hdma_object_pre_instructions[v2] = FUNC16(HdmaobjPreInstr_HazeColorMathSubscreen_FadingIn);
    HdmaobjPreInstr_HazeColorMathSubscreen_FadingIn(k);
  }
}

void HdmaobjPreInstr_HazeColorMathSubscreen_FadingIn(uint16 k) {  // 0x88DE2D
  int8 v1;

  reg_COLDATA[0] = 32;
  reg_COLDATA[1] = 64;
  reg_COLDATA[2] = 0x80;
  fx_layer_blending_config_c = 44;
  if (hdma_object_A[(uint8)k >> 1] == 16) {
    hdma_object_pre_instructions[(uint8)k >> 1] = FUNC16(HdmaobjPreInstr_HazeColorMathSubscreen_FadedIn);
  } else {
    uint8 v3 = k;
    uint8 r20 = *((uint8 *)hdma_object_B + (uint8)k);
    v1 = *((uint8 *)hdma_object_A + (uint8)k);
    for (int i = 15; i >= 0; --i) {
      uint8 r18 = v1;
      *((uint8 *)hdma_table_2 + (uint8)i) = r20 | v1;
      v1 = r18 - 1;
      if ((int8)(r18 - 1) < 0)
        v1 = 0;
    }
    ++hdma_object_A[v3 >> 1];
  }
}

void HdmaobjPreInstr_HazeColorMathSubscreen_FadedIn(uint16 k) {  // 0x88DE74
  reg_COLDATA[0] = 32;
  reg_COLDATA[1] = 64;
  reg_COLDATA[2] = 0x80;
  fx_layer_blending_config_c = 44;
  if (door_transition_function == FUNC16(DoorTransitionFunction_FadeOutScreen))
    hdma_object_pre_instructions[(uint8)k >> 1] = FUNC16(HdmaobjPreInstr_HazeColorMathSubscreen_FadingOut);
}

void HdmaobjPreInstr_HazeColorMathSubscreen_FadingOut(uint16 k) {  // 0x88DE96
  int8 v1;

  reg_COLDATA[0] = 32;
  reg_COLDATA[1] = 64;
  reg_COLDATA[2] = 0x80;
  fx_layer_blending_config_c = 44;
  if (hdma_object_A[(uint8)k >> 1]) {
    uint8 v3 = k;
    uint8 r20 = *((uint8 *)hdma_object_B + (uint8)k);
    v1 = *((uint8 *)hdma_object_A + (uint8)k);
    for (int i = 15; i >= 0; --i) {
      uint8 r18 = v1;
      *((uint8 *)hdma_table_2 + (uint8)i) = r20 | v1;
      v1 = r18 - 1;
      if ((int8)(r18 - 1) < 0)
        v1 = 0;
    }
    --hdma_object_A[v3 >> 1];
  }
}
