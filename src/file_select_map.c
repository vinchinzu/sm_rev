// File-select area/room map flow extracted from Bank 81.

#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "menu_common.h"
#include "enemy_types.h"

#define kAreaMapForegroundSetDefs ((uint16*)RomFixedPtr(0x81a4e6))
#define kAreaMapForegroundColors ((uint16*)RomFixedPtr(0x81a40e))
#define kBg2RoomSelectMapTilemap ((uint16*)RomFixedPtr(0xb6e000))
#define kFileSelectExpandingSquareTilemap ((uint16*)RomFixedPtr(0x81b14b))
#define kMapIconDataPointers ((MapIconDataPointers*)RomFixedPtr(0x82c7cb))
#define g_word_82C749 ((uint16*)RomFixedPtr(0x82c749))
#define kLeftMapScrollArrowData (*(MapScrollArrowData*)RomFixedPtr(0x81af32))
#define kRightMapScrollArrowData (*(MapScrollArrowData*)RomFixedPtr(0x81af3c))
#define kUpMapScrollArrowData (*(MapScrollArrowData*)RomFixedPtr(0x81af46))
#define kDownMapScrollArrowData (*(MapScrollArrowData*)RomFixedPtr(0x81af50))
#define kExpandingSquareVels ((ExpandingSquareVels*)RomFixedPtr(0x81aa34))

static const uint16 kFileSelectMap_AreaIndexes[6] = { 0, 3, 5, 1, 4, 2 };
static const uint16 kActiveAreaMapForegroundColors[6] = { 0, 0xa, 0x10, 0x16, 0x24, 0x2a };
static const uint16 kInactiveAreaMapForegroundColors[6] = {  // 0x81A3D3
  0x30, 0x3a,
  0x40, 0x46,
  0x54, 0x5a,
};
static const uint16 kAreaSelectMapLabelPositions[12] = {  // 0x81A97E
  0x5b, 0x32,
  0x2a, 0x7f,
  0x5e, 0xb5,
  0xce, 0x50,
  0xce, 0x9f,
  0x87, 0x8b,
};
static const uint16 kRoomSelectMapExpandingSquareTimers[6] = { 0x33, 0x35, 0x2d, 0x33, 0x33, 0x22 };
static const uint8 kExpandingSquareTransitionSpeed = 4;

static void LoadAreaMapForegroundColors(uint16 j) {  // 0x81A3E3
  while (1) {
    int v1 = j >> 1;
    if (kAreaMapForegroundSetDefs[v1] == 0xFFFF)
      break;
    uint16 v4 = j;
    uint16 v2 = kAreaMapForegroundSetDefs[v1 + 1];
    uint16 v3 = kAreaMapForegroundSetDefs[v1];
    int n = 5;
    do {
      palette_buffer[v2 >> 1] = kAreaMapForegroundColors[v3 >> 1];
      v2 += 2;
      v3 += 2;
    } while (--n);
    j = v4 + 4;
  }
}

static void LoadActiveAreaMapForegroundColors(uint16 v0) {
  LoadAreaMapForegroundColors(kActiveAreaMapForegroundColors[v0]);
}

static void LoadInactiveAreaMapForegroundColors(uint16 v0) {  // 0x81A3DC
  LoadAreaMapForegroundColors(kInactiveAreaMapForegroundColors[v0]);
}

void sub_81A3D1(uint16 k) {  // 0x81A3D1
  LoadActiveAreaMapForegroundColors(k);
}

static void LoadAreaSelectBackgroundTilemap(uint16 j) {  // 0x81A58A
  uint16 v1 = vram_write_queue_tail;
  VramWriteEntry *v2 = gVramWriteEntry(vram_write_queue_tail);
  v2->size = 2048;
  v2->src.addr = 8 * 256 * j + addr_kAreaSelectBackgroundTilemaps;
  v2->src.bank = 0x81;
  v2->vram_dst = (reg_BG3SC & 0xFC) << 8;
  vram_write_queue_tail = v1 + 7;
}

static void ConfigureWindow1ForExpandingSquare(void) {  // 0x81A5F6
  reg_TM = 19;
  WriteReg(TM, 0x13);
  reg_TMW = 19;
  WriteReg(TMW, 0x13);
  reg_W12SEL = 35;
  WriteReg(W12SEL, 0x23);
  reg_W34SEL = 3;
  WriteReg(W34SEL, 3);
  reg_WOBJSEL = 35;
  WriteReg(WOBJSEL, 0x23);
}

static void SetupInitialExpandingSquareHDMA(void) {  // 0x81A61C
  expand_sq_topbottom_margin_right_pos = 0;
  expand_sq_topbottom_margin_left_pos = -1;
  LOBYTE(expand_sq_left_pos) = 127;
  LOBYTE(expand_sq_right_pos) = -127;
  hdma_window_1_left_pos[0].field_0 = 111;
  hdma_window_1_left_pos[0].field_1 = 34;
  hdma_window_1_left_pos[0].field_2 = -98;
  hdma_window_1_left_pos[1].field_0 = 1;
  hdma_window_1_left_pos[1].field_1 = 50;
  hdma_window_1_left_pos[1].field_2 = -98;
  hdma_window_1_left_pos[2].field_0 = 1;
  hdma_window_1_left_pos[2].field_1 = 50;
  hdma_window_1_left_pos[2].field_2 = -98;
  hdma_window_1_left_pos[3].field_0 = 111;
  hdma_window_1_left_pos[3].field_1 = 32;
  hdma_window_1_left_pos[3].field_2 = -98;
  WriteReg(DMAP2, 0x40);
  WriteReg(BBAD2, 0x26);
  WriteReg(A1T2L, 0);
  WriteReg(A1T2H, 0x9E);
  WriteReg(A1B2, 0x7E);
  WriteReg(DAS20, 0x7E);
  WriteReg(DAS2L, 0);
  WriteReg(DAS2H, 0);
  WriteReg(A2A2L, 0);
  WriteReg(A2A2H, 0);
  WriteReg(NTRL2, 0);
  hdma_window_1_right_pos[0].field_0 = 111;
  hdma_window_1_right_pos[0].field_1 = 32;
  hdma_window_1_right_pos[0].field_2 = -98;
  hdma_window_1_right_pos[1].field_0 = 1;
  hdma_window_1_right_pos[1].field_1 = 54;
  hdma_window_1_right_pos[1].field_2 = -98;
  hdma_window_1_right_pos[2].field_0 = 1;
  hdma_window_1_right_pos[2].field_1 = 54;
  hdma_window_1_right_pos[2].field_2 = -98;
  hdma_window_1_right_pos[3].field_0 = 111;
  hdma_window_1_right_pos[3].field_1 = 32;
  hdma_window_1_right_pos[3].field_2 = -98;
  WriteReg(DMAP3, 0x40);
  WriteReg(BBAD3, 0x27);
  WriteReg(A1T3L, 0x10);
  WriteReg(A1T3H, 0x9E);
  WriteReg(A1B3, 0x7E);
  WriteReg(DAS30, 0x7E);
  WriteReg(DAS3L, 0);
  WriteReg(DAS3H, 0);
  WriteReg(A2A3L, 0);
  WriteReg(A2A3H, 0);
  WriteReg(NTRL3, 0);
}

static uint16 DecAndWraparoundTo5(uint16 a) {  // 0x81A898
  uint16 result = a - 1;
  if ((result & 0x8000) != 0)
    return 5;
  return result;
}

static uint16 WraparoundFrom6to0(uint16 a) {  // 0x81A89F
  uint16 result = a + 1;
  if (!sign16(result - 6))
    return 0;
  return result;
}

static uint16 CheckIfFileSelectMapAreaCanBeSelected(uint16 a) {  // 0x81A931
  int v1 = 2 * kFileSelectMap_AreaIndexes[a];
  if (*(uint16 *)&used_save_stations_and_elevators[v1])
    return true;
  uint16 t = *(uint16 *)((uint8 *)&kMapIconDataPointers[4].crateria + v1) + 64;
  return t != 0xffff;
}

static void SwitchActiveFileSelectMapArea(uint16 R28) {  // 0x81A958
  LoadInactiveAreaMapForegroundColors(kFileSelectMap_AreaIndexes[file_select_map_area_index]);
  file_select_map_area_index = R28;
  LoadActiveAreaMapForegroundColors(kFileSelectMap_AreaIndexes[R28]);
  LoadAreaSelectBackgroundTilemap(kFileSelectMap_AreaIndexes[file_select_map_area_index]);
}

static void DrawAreaSelectMapLabels(void) {
  uint16 r3 = 0;
  DrawMenuSpritemap(g_word_82C749[0], 0x80, 0x10, r3);
  for (int i = 0; i < 6; i++) {
    r3 = (i == file_select_map_area_index) ? 0 : 512;
    uint16 v1 = 2 * kFileSelectMap_AreaIndexes[i];
    uint16 r36 = *(uint16 *)&used_save_stations_and_elevators[v1];
    const uint16 *v2 = (const uint16 *)RomPtr_82(*(VoidP *)((uint8 *)&kMapIconDataPointers[4].crateria + v1));
    int R30 = 16;
    while (*v2 != 0xffff) {
      int v4 = r36 & 1;
      r36 >>= 1;
      if (v4 && *v2 != 0xfffe)
        goto LABEL_11;
      v2 += 2;
      if (!--R30) {
        if (enable_debug && *v2 != 0xFFFF) {
LABEL_11:;
          int j = 4 * kFileSelectMap_AreaIndexes[i] >> 1;
          DrawMenuSpritemap(
            g_word_82C749[0] + kFileSelectMap_AreaIndexes[i] + 1,
            kAreaSelectMapLabelPositions[j],
            kAreaSelectMapLabelPositions[j + 1], r3);
          break;
        }
        break;
      }
    }
  }
}

static void SelectFileSelectMapArea(void) {  // 0x81A8A9
  int8 v2;
  int16 v3;
  int16 v4;

  ++menu_index;
  if (!enable_debug) {
    area_index = sram_area_index;
    load_station_index = sram_save_station_index;
    DrawAreaSelectMapLabels();
    return;
  }
  area_index = kFileSelectMap_AreaIndexes[file_select_map_area_index];
  uint16 q = *(uint16 *)&used_save_stations_and_elevators[2 * area_index];
  uint16 v0 = 0;
  const uint16 *r0 = (const uint16 *)RomPtr_82(GET_WORD(RomPtr_82(addr_kMapIconDataPointers + 64 + 2 * area_index)));
  uint16 r20 = 16;
  while (1) {
    v2 = q & 1;
    q >>= 1;
    if (!v2)
      goto LABEL_10;
    v3 = r0[2 * v0];
    if (v3 == -2)
      goto LABEL_10;
    if (v3 != -1)
      break;
    v0 = -1;
LABEL_10:
    ++v0;
    if (!--r20) {
      while (1) {
        v4 = r0[2 * v0];
        if (v4 != -2) {
          if (v4 != -1)
            goto LABEL_16;
          v0 = -1;
        }
        ++v0;
        if (!--r20)
          Unreachable();
      }
    }
  }
LABEL_16:
  load_station_index = v0;
  DrawAreaSelectMapLabels();
}

static void AddExpandingSqTransLeftIndirHDMA(uint16 a, uint16 k, uint16 j) {  // 0x81ABF7
  if ((a & 0x80) != 0) {
    *(&hdma_window_1_left_pos[0].field_0 + k) = a - 127;
    *(&hdma_window_1_left_pos[1].field_0 + k) = 127;
    *(uint16 *)(&hdma_window_1_left_pos[0].field_1 + k) = j;
    *(uint16 *)(&hdma_window_1_left_pos[1].field_1 + k) = j;
  } else {
    *(&hdma_window_1_left_pos[0].field_0 + k) = a;
    *(uint16 *)(&hdma_window_1_left_pos[0].field_1 + k) = j;
  }
}

static uint16 AddExpandingSqTransRightIndirHDMA(uint16 a, uint16 k, uint16 j) {  // 0x81AC2D
  if ((a & 0x80) != 0) {
    *(&hdma_window_1_right_pos[0].field_0 + k) = a - 127;
    *(&hdma_window_1_right_pos[1].field_0 + k) = 127;
    *(uint16 *)(&hdma_window_1_right_pos[0].field_1 + k) = j;
    *(uint16 *)(&hdma_window_1_right_pos[1].field_1 + k) = j;
    return k + 6;
  } else {
    *(&hdma_window_1_right_pos[0].field_0 + k) = a;
    *(uint16 *)(&hdma_window_1_right_pos[0].field_1 + k) = j;
    return k + 3;
  }
}

static void SetupRoomSelectMapExpandingSquareTransHDMA(void) {  // 0x81ABA7
  uint16 k = 0;
  uint16 v0 = expand_sq_top_pos;
  AddExpandingSqTransLeftIndirHDMA(v0, k, 0x9E22);
  k = AddExpandingSqTransRightIndirHDMA(v0, k, 0x9E20);
  uint16 v2 = expand_sq_bottom_pos - expand_sq_top_pos;
  if ((uint8)expand_sq_bottom_pos == (uint8)expand_sq_top_pos)
    v2 = 1;
  AddExpandingSqTransLeftIndirHDMA(v2, k, 0x9E32);
  k = AddExpandingSqTransRightIndirHDMA(v2, k, 0x9E36);
  uint16 v4 = -32 - expand_sq_bottom_pos;
  if ((uint8)expand_sq_bottom_pos == 0xE0)
    v4 = 1;
  AddExpandingSqTransLeftIndirHDMA(v4, k, 0x9E22);
  k = AddExpandingSqTransRightIndirHDMA(v4, k, 0x9E20);
  *(&hdma_window_1_left_pos[0].field_0 + k) = 0;
  *(&hdma_window_1_right_pos[0].field_0 + k) = 0;
}

static uint16 HandleRoomSelectMapExpandingSquareTrans(void) {  // 0x81AC84
  AddToHiLo(&expand_sq_left_pos, &expand_sq_left_subpos, __PAIR32__(expand_sq_left_vel, expand_sq_left_subvel));
  if (sign16(expand_sq_left_pos - 1))
    expand_sq_left_pos = 1;
  AddToHiLo(&expand_sq_right_pos, &expand_sq_right_subpos, __PAIR32__(expand_sq_right_vel, expand_sq_right_subvel));
  if (!sign16(expand_sq_right_pos - 256))
    expand_sq_right_pos = 255;
  AddToHiLo(&expand_sq_top_pos, &expand_sq_top_subpos, __PAIR32__(expand_sq_top_vel, expand_sq_top_subvel));
  if (sign16(expand_sq_top_pos - 1))
    expand_sq_top_pos = 1;
  AddToHiLo(&expand_sq_bottom_pos, &expand_sq_bottom_subpos, __PAIR32__(expand_sq_bottom_vel, expand_sq_bottom_subvel));
  if (!sign16(expand_sq_bottom_pos - 224))
    expand_sq_bottom_pos = 224;
  SetupRoomSelectMapExpandingSquareTransHDMA();
  return --expand_sq_timer;
}

void sub_81AEC8(void) {  // 0x81AEC8
  if (sign16(map_min_x_scroll - 24 - reg_BG1HOFS))
    DrawMapScrollArrowAndCheckToScroll(0x81, addr_kLeftMapScrollArrowData);
  if (!sign16(map_max_x_scroll - 232 - reg_BG1HOFS))
    DrawMapScrollArrowAndCheckToScroll(0x81, addr_kRightMapScrollArrowData);
  if (sign16(map_min_y_scroll - 64 - reg_BG1VOFS))
    DrawMapScrollArrowAndCheckToScroll(0x81, addr_kUpMapScrollArrowData);
  if (sign16(map_max_y_scroll - 145 - reg_BG1VOFS)) {
    if (map_scrolling_direction == kDownMapScrollArrowData.map_scroll_dir) {
      map_scrolling_gear_switch_timer = 0;
      map_scrolling_direction = 0;
      map_scrolling_speed_index = 0;
    }
  } else {
    DrawMapScrollArrowAndCheckToScroll(0x81, addr_kDownMapScrollArrowData);
  }
}

static void HandleFileSelectMapScrollArrows(void) {  // 0x81AECA
  sub_81AEC8();
}

void FileSelectMap_22(void) {  // 0x819E7B
  DrawAreaSelectMapLabels();
  HandleFadeOut();
  if ((reg_INIDISP & 0xF) == 0) {
    game_state = kGameState_2_GameOptionsMenu;
    menu_index = 0;
  }
}

void FileSelectMap_0(void) {  // 0x81A32A
  ClearMenuTilemap();
  uint16 v0 = vram_write_queue_tail;
  VramWriteEntry *v1 = gVramWriteEntry(vram_write_queue_tail);
  v1->size = 2048;
  v1->src.addr = ADDR16_OF_RAM(ram3000.menu.menu_tilemap);
  *(uint16 *)&v1->src.bank = 126;
  v1->vram_dst = (reg_BG2SC & 0xFC) << 8;
  vram_write_queue_tail = v0 + 7;
  palette_change_num = 0;
  LoadFileSelectPalettes();
  uint16 v2 = 0;
  do {
    target_palettes[v2 >> 1] = palette_buffer[v2 >> 1];
    v2 += 2;
  } while ((int16)(v2 - 64) < 0);
  target_palettes[14] = 0;
  target_palettes[30] = 0;
  ++menu_index;
}

void FileSelectMap_1(void) {  // 0x81A37C
  if (AdvanceGradualColorChangeOfPalette(0, 0x40)) {
    reg_BG1VOFS = 0;
    reg_BG1HOFS = 0;
    reg_BG2VOFS = 0;
    reg_BG2HOFS = 0;
    uint16 v0 = 0;
    while (area_index != kFileSelectMap_AreaIndexes[v0 >> 1]) {
      v0 += 2;
      if ((int16)(v0 - 12) >= 0) {
        v0 = 0;
        break;
      }
    }
    file_select_map_area_index = v0 >> 1;
    int16 v1 = 0;
    int16 v2;
    do {
      v2 = v1;
      if (v1 == area_index)
        LoadActiveAreaMapForegroundColors(v1);
      else
        LoadInactiveAreaMapForegroundColors(v1);
      ++v1;
    } while ((int16)(v2 - 5) < 0);
    ++menu_index;
    g_word_7E0787 = 0;
    reg_TM = 2;
  }
}

void FileSelectMap_2_LoadAreaSelectForegroundTilemap(void) {  // 0x81A546
  uint16 v0 = vram_write_queue_tail;
  VramWriteEntry *v1 = gVramWriteEntry(vram_write_queue_tail);
  v1->size = 2048;
  v1->src.addr = addr_kAreaSelectForegroundTilemap;
  *(uint16 *)&v1->src.bank = 129;
  v1->vram_dst = (reg_BG1SC & 0xFC) << 8;
  vram_write_queue_tail = v0 + 7;
  *(uint16 *)&reg_INIDISP = (reg_OBSEL << 8) | 0xF;
  ++menu_index;
}

void FileSelectMap_18(void) {  // 0x81A578
  ++menu_index;
  LoadAreaSelectBackgroundTilemap(area_index);
}

void FileSelectMap_3_LoadAreaSelectBackgroundTilemap(void) {  // 0x81A582
  ++menu_index;
  LoadAreaSelectBackgroundTilemap(area_index);
}

void FileSelectMap_4_SetupExpandingSquareTransition(void) {  // 0x81A5B3
  reg_TS = 4;
  WriteReg(TS, 4);
  reg_TSW = 4;
  WriteReg(TSW, 4);
  reg_CGWSEL = 2;
  WriteReg(CGWSEL, 2);
  reg_CGADSUB = 37;
  WriteReg(CGADSUB, 0x25);
  reg_COLDATA[2] = 0x80;
  reg_COLDATA[1] = 64;
  reg_COLDATA[0] = 32;
  SetupInitialExpandingSquareHDMA();
  reg_HDMAEN = 12;
  WriteReg(HDMAEN, 0xC);
  ConfigureWindow1ForExpandingSquare();
  hdma_objects_enable_flag = 0;
  LOBYTE(menu_index) = menu_index + 1;
}

void FileSelectMap_5_ExpandingSquare(void) {  // 0x81A725
  DrawAreaSelectMapLabels();
  uint8 v0 = hdma_window_1_left_pos[0].field_0 - kExpandingSquareTransitionSpeed;
  if ((int8)(hdma_window_1_left_pos[0].field_0 - kExpandingSquareTransitionSpeed) < 0) {
    LOBYTE(menu_index) = menu_index + 1;
    reg_TM &= ~2;
    reg_TMW = 0;
    reg_TSW = 0;
    reg_BG2VOFS = 24;
    pausemenu_palette_animation_timer = 1;
    uint16 v1 = 0;
    do {
      ram4000.xray_tilemaps[v1] = kBg2RoomSelectMapTilemap[v1];
      ++v1;
    } while ((int16)(v1 * 2 - 1600) < 0);
    do
      ram4000.xray_tilemaps[v1++] = 10241;
    while ((int16)(v1 * 2 - 2048) < 0);
    DrawRoomSelectMapAreaLabel(ram4000.bg2_room_select_map_tilemap + 170);
    uint16 v2 = 320;
    uint16 v3 = 959;
    do {
      ram4000.xray_tilemaps[v3--] = kFileSelectExpandingSquareTilemap[v2 >> 1];
      v2 -= 2;
    } while (v2);
    uint16 v4 = vram_write_queue_tail;
    VramWriteEntry *v5 = gVramWriteEntry(vram_write_queue_tail);
    v5->size = 2048;
    v5->src.addr = ADDR16_OF_RAM(ram4000);
    v5->src.bank = 126;
    v5->vram_dst = (reg_BG2SC & 0xFC) << 8;
    vram_write_queue_tail = v4 + 7;
  } else {
    hdma_window_1_left_pos[0].field_0 -= kExpandingSquareTransitionSpeed;
    hdma_window_1_left_pos[3].field_0 = v0;
    hdma_window_1_right_pos[0].field_0 = v0;
    hdma_window_1_right_pos[3].field_0 = v0;
    hdma_window_1_left_pos[1].field_0 += kExpandingSquareTransitionSpeed;
    hdma_window_1_left_pos[2].field_0 = hdma_window_1_left_pos[1].field_0;
    hdma_window_1_right_pos[1].field_0 = hdma_window_1_left_pos[1].field_0;
    hdma_window_1_right_pos[2].field_0 = hdma_window_1_left_pos[1].field_0;
    LOBYTE(expand_sq_left_pos) = expand_sq_left_pos - kExpandingSquareTransitionSpeed;
    LOBYTE(expand_sq_right_pos) = kExpandingSquareTransitionSpeed + expand_sq_right_pos;
  }
}

void FileSelectMap_6_AreaSelectMap(void) {  // 0x81A800
  int16 v0 = joypad1_newkeys;
  if ((joypad1_newkeys & (kButton_Up | kButton_Left)) != 0) {
    v0 = enable_debug;
    if (enable_debug) {
      uint16 R28 = DecAndWraparoundTo5(file_select_map_area_index);
      if (CheckIfFileSelectMapAreaCanBeSelected(R28)
          || (R28 = DecAndWraparoundTo5(R28), CheckIfFileSelectMapAreaCanBeSelected(R28))
          || (R28 = DecAndWraparoundTo5(R28), CheckIfFileSelectMapAreaCanBeSelected(R28))) {
        QueueSfx1_Max6(0x37);
        SwitchActiveFileSelectMapArea(R28);
        DrawAreaSelectMapLabels();
        return;
      }
      goto LABEL_18;
    }
LABEL_6:
    if (v0 < 0) {
      menu_index = 22;
      DrawAreaSelectMapLabels();
      return;
    }
    if ((v0 & (kButton_Start | kButton_A)) != 0) {
      QueueSfx1_Max6(0x38);
      SelectFileSelectMapArea();
      return;
    }
LABEL_18:
    DrawAreaSelectMapLabels();
    return;
  }
  if ((joypad1_newkeys & (kButton_Select | kButton_Down | kButton_Right)) == 0)
    goto LABEL_6;
  v0 = enable_debug;
  if (!enable_debug)
    goto LABEL_6;
  int n = 6;
  uint16 R28 = file_select_map_area_index;
  while (1) {
    R28 = WraparoundFrom6to0(R28);
    if (CheckIfFileSelectMapAreaCanBeSelected(R28))
      break;
    if (!--n)
      goto LABEL_18;
  }
  SwitchActiveFileSelectMapArea(R28);
  QueueSfx1_Max6(0x37);
  DrawAreaSelectMapLabels();
}

void FileSelectMap_7_PrepExpandSquareTransToRoomMap(void) {  // 0x81AAAC
  DrawAreaSelectMapLabels();
  SetupInitialExpandingSquareHDMA();
  reg_TM = 19;
  reg_TMW = 19;
  reg_W12SEL = 50;
  WriteReg(W12SEL, 0x32);
  reg_W34SEL = 2;
  WriteReg(W34SEL, 2);
  reg_CGADSUB = 5;
  WriteReg(CGADSUB, 5);
  reg_WOBJSEL = 34;
  WriteReg(WOBJSEL, 0x22);
  reg_BG12NBA = 48;
  hdma_window_1_left_pos[3].field_0 = 0;
  hdma_window_1_right_pos[3].field_0 = 0;
  DrawRoomSelectMapAreaLabel(ram4000.bg2_room_select_map_tilemap + 170);
  uint16 v0 = vram_write_queue_tail;
  VramWriteEntry *v1 = gVramWriteEntry(vram_write_queue_tail);
  v1->size = 512;
  v1->src.addr = ADDR16_OF_RAM(ram4000);
  v1->src.bank = 126;
  v1->vram_dst = (reg_BG2SC & 0xFC) << 8;
  vram_write_queue_tail = v0 + 7;

  expand_sq_timer = kRoomSelectMapExpandingSquareTimers[area_index];
  int v2 = (uint16)(4 * area_index) >> 1;
  expand_sq_left_pos = kAreaSelectMapLabelPositions[v2];
  expand_sq_right_pos = expand_sq_left_pos;
  expand_sq_top_pos = kAreaSelectMapLabelPositions[v2 + 1];
  expand_sq_bottom_pos = expand_sq_top_pos;
  expand_sq_left_subpos = 0;
  expand_sq_right_subpos = 0;
  expand_sq_top_subpos = 0;
  expand_sq_bottom_subpos = 0;

  const ExpandingSquareVels *vels = &kExpandingSquareVels[area_index];
  expand_sq_left_subvel = vels->left_subvel;
  expand_sq_left_vel = vels->left_vel;
  expand_sq_right_subvel = vels->right_subvel;
  expand_sq_right_vel = vels->right_vel;
  expand_sq_top_subvel = vels->top_subvel;
  expand_sq_top_vel = vels->top_vel;
  expand_sq_bottom_subvel = vels->bottom_subvel;
  expand_sq_bottom_vel = vels->bottom_vel;
  SetupRoomSelectMapExpandingSquareTransHDMA();
  reg_HDMAEN = 12;
  WriteReg(HDMAEN, 0xC);
  ++menu_index;
  QueueSfx1_Max6(0x3B);
}

void FileSelectMap_8_ExpandSquareTransToRoomSelectMap(void) {  // 0x81AC66
  if ((HandleRoomSelectMapExpandingSquareTrans() & 0x8000) != 0) {
    ++menu_index;
    reg_TM = 2;
    reg_TMW = 0;
    reg_TSW = 0;
    reg_TS = 0;
  }
  DrawAreaSelectMapLabels();
}

void FileSelectMap_9_InitRoomSelectMap(void) {  // 0x81AD17
  LoadMirrorOfExploredMapTiles();
  DrawRoomSelectMap();
  LoadFromLoadStation();
  DisableHdmaObjects();
  WaitUntilEndOfVblankAndClearHdma();
  RoomDefHeader *room_def_header = get_RoomDefHeader(room_ptr);
  LOBYTE(area_index) = room_def_header->area_index_;
  LOBYTE(room_x_coordinate_on_map) = room_def_header->x_coordinate_on_map;
  LOBYTE(room_y_coordinate_on_map) = room_def_header->y_coordinate_on_map;
  SetupMapScrollingForFileSelectMap();
  map_min_y_scroll += 24;
  reg_BG2VOFS = 24;
  *(uint16 *)&reg_TM &= ~4;
  ++menu_index;
  map_scrolling_direction = 0;
  map_scrolling_speed_index = 0;
  samus_position_indicator_animation_frame = 0;
  samus_position_indicator_animation_timer = 0;
  samus_position_indicator_animation_loop_counter = 0;
}

void FileSelectMap_10_RoomSelectMap(void) {  // 0x81AD7F
  DrawFileSelectMapIcons();
  HandleFileSelectMapScrollArrows();
  MapScrolling();
  DisplayMapElevatorDestinations();
  if (enable_debug && (joypad2_new_keys & kButton_Select) != 0) {
    QueueSfx1_Max6(0x38);

    uint16 r24 = *(uint16 *)&used_save_stations_and_elevators[2 * area_index];
    int16 v1 = load_station_index;
    do {
      r24 >>= 1;
      --v1;
    } while (v1 >= 0);

    const uint16 *r0 = (const uint16 *)RomPtr_82(GET_WORD(RomPtr_82(addr_kMapIconDataPointers + 64 + 2 * area_index)));
    int16 v3 = 4 * load_station_index;
    uint16 r18 = r0[v3 >> 1];
    uint16 r20 = r0[(v3 >> 1) + 1];
    int8 v4;
    int16 v5;
    if (!sign16(load_station_index - 16))
      goto LABEL_23;
    do {
      if (!sign16(++load_station_index - 16)) {
        while (1) {
          v5 = r0[2 * load_station_index];
          if (v5 == -1)
            break;
          if (v5 != -2)
            goto LABEL_25;
LABEL_23:
          ++load_station_index;
        }
        load_station_index = 0;
        r24 = *(uint16 *)&used_save_stations_and_elevators[2 * area_index];
      }
      v4 = r24 & 1;
      r24 >>= 1;
    } while (!v4 || r0[2 * load_station_index] >= 0xFFFE);
LABEL_25:;
    uint16 v6 = 4 * load_station_index;
    uint16 w7 = r0[2 * load_station_index];
    if (sign16(w7 - reg_BG1HOFS) || !sign16(w7 - 256 - reg_BG1HOFS)) {
      int16 v8 = reg_BG1HOFS + r0[v6 >> 1] - r18;
      if (v8 >= 0) {
        if ((int16)(v8 - map_min_x_scroll) >= 0)
          v8 = map_min_x_scroll;
      } else {
        v8 = 0;
      }
      reg_BG1HOFS = v8;
    }
    uint16 v9 = v6 + 2;
    uint16 v10 = r0[v9 >> 1];
    if (sign16(v10 - reg_BG1VOFS) || !sign16(v10 - 161 - reg_BG1VOFS)) {
      uint16 v11 = reg_BG1VOFS + r0[v9 >> 1] - r20;
      if ((int16)(v11 - map_min_y_scroll) >= 0)
        v11 = map_min_y_scroll;
      reg_BG1VOFS = v11;
    }
  } else if ((joypad1_newkeys & 0x8000) != 0) {
    menu_index += 5;
    uint16 v0 = 0;
    while (area_index != kFileSelectMap_AreaIndexes[v0 >> 1]) {
      v0 += 2;
      if ((int16)(v0 - 12) >= 0) {
        file_select_map_area_index = 0;
        return;
      }
    }
    file_select_map_area_index = v0 >> 1;
    QueueSfx1_Max6(0x3C);
  } else if ((joypad1_newkeys & (kButton_Start | kButton_A)) != 0) {
    ++menu_index;
    QueueSfx1_Max6(0x38);
  }
}

void FileSelectMap_11(void) {  // 0x81AF5A
  DrawFileSelectMapIcons();
  DisplayMapElevatorDestinations();
  ++menu_index;
}

void FileSelectMap_13(void) {  // 0x81AF66
  DrawFileSelectMapIcons();
  DisplayMapElevatorDestinations();
  HandleFadeOut();
  if ((reg_INIDISP & 0xF) == 0) {
    ++menu_index;
    enemy_data[0].x_pos = 32;
  }
}

void FileSelectMap_14(void) {  // 0x81AF83
  if (!--enemy_data[0].x_pos) {
    ScreenOff();
    ++game_state;
    menu_index = 0;
  }
}

void FileSelectMap_15_ClearTileMap(void) {  // 0x81AF97
  reg_TM = 18;
  for (int i = 2046; i >= 0; i -= 2)
    *(uint16 *)((uint8 *)ram3000.pause_menu_map_tilemap + (uint16)i) = 15;
  uint16 v1 = vram_write_queue_tail;
  VramWriteEntry *v2 = gVramWriteEntry(vram_write_queue_tail);
  v2->size = 2048;
  v2->src.addr = ADDR16_OF_RAM(ram3000.pause_menu_map_tilemap);
  *(uint16 *)&v2->src.bank = 126;
  v2->vram_dst = (reg_BG1SC & 0xFC) << 8;
  vram_write_queue_tail = v1 + 7;
  ++menu_index;
}

void FileSelectMap_16_LoadPalettes(void) {  // 0x81AFD3
  int16 v0 = 0;
  int16 v1;
  LoadMenuPalettes();
  do {
    v1 = v0;
    LoadInactiveAreaMapForegroundColors(v0++);
  } while ((int16)(v1 - 5) < 0);
  LoadActiveAreaMapForegroundColors(area_index);
  LoadAreaSelectBackgroundTilemap(area_index);
  ++menu_index;
}

void FileSelectMap_20_SetupExpandingSquare(void) {  // 0x81AFF6
  reg_HDMAEN = 0;
  QueueSfx1_Max6(0x3C);
  expand_sq_timer = kRoomSelectMapExpandingSquareTimers[area_index] - 12;
  expand_sq_left_subvel = *(uint16 *)((uint8 *)&kExpandingSquareVels[0].left_subvel + (uint16)(16 * area_index));
  expand_sq_left_vel = *(uint16 *)((uint8 *)&kExpandingSquareVels[0].left_vel + (uint16)(16 * area_index));
  expand_sq_right_subvel = *(uint16 *)((uint8 *)&kExpandingSquareVels[0].right_subvel + (uint16)(16 * area_index));
  expand_sq_right_vel = *(uint16 *)((uint8 *)&kExpandingSquareVels[0].right_vel + (uint16)(16 * area_index));
  expand_sq_top_subvel = *(uint16 *)((uint8 *)&kExpandingSquareVels[0].top_subvel + (uint16)(16 * area_index));
  expand_sq_top_vel = *(uint16 *)((uint8 *)&kExpandingSquareVels[0].top_vel + (uint16)(16 * area_index));
  expand_sq_bottom_subvel = *(uint16 *)((uint8 *)&kExpandingSquareVels[0].bottom_subvel + (uint16)(16 * area_index));
  expand_sq_bottom_vel = *(uint16 *)((uint8 *)&kExpandingSquareVels[0].bottom_vel + (uint16)(16 * area_index));
  expand_sq_left_pos = 8;
  expand_sq_right_pos = 248;
  expand_sq_top_pos = 8;
  expand_sq_bottom_pos = 216;
  expand_sq_left_subpos = 0;
  expand_sq_right_subpos = 0;
  expand_sq_top_subpos = 0;
  expand_sq_bottom_subpos = 0;
  SetupRoomSelectMapExpandingSquareTransHDMA();
  ++menu_index;
  reg_HDMAEN = 12;
  WriteReg(HDMAEN, 0xC);
  reg_W12SEL = 50;
  WriteReg(W12SEL, 0x32);
  reg_W34SEL = 2;
  WriteReg(W34SEL, 2);
  reg_BG12NBA = 48;
  reg_WOBJSEL = 34;
  hdma_window_1_left_pos[3].field_0 = 0;
  hdma_window_1_right_pos[3].field_0 = 0;
  reg_BG1VOFS = 0;
  reg_BG1HOFS = 0;
}

void FileSelectMap_21_MoveExpandingSquare(void) {  // 0x81B0BB
  AddToHiLo(&expand_sq_left_pos, &expand_sq_left_subpos, -IPAIR32(expand_sq_left_vel, expand_sq_left_subvel));
  AddToHiLo(&expand_sq_right_pos, &expand_sq_right_subpos, -IPAIR32(expand_sq_right_vel, expand_sq_right_subvel));
  AddToHiLo(&expand_sq_top_pos, &expand_sq_top_subpos, -IPAIR32(expand_sq_top_vel, expand_sq_top_subvel));
  AddToHiLo(&expand_sq_bottom_pos, &expand_sq_bottom_subpos, -IPAIR32(expand_sq_bottom_vel, expand_sq_bottom_subvel));
  SetupRoomSelectMapExpandingSquareTransHDMA();
  DrawAreaSelectMapLabels();
  if ((--expand_sq_timer & 0x8000) != 0) {
    menu_index -= 15;
    reg_TM = 17;
    reg_TMW = 0;
    reg_TSW = 0;
  }
}

static Func_V *const kFileSelectMapFuncs[23] = {  // 0x819E3E
  FileSelectMap_0,
  FileSelectMap_1,
  FileSelectMap_2_LoadAreaSelectForegroundTilemap,
  FileSelectMap_3_LoadAreaSelectBackgroundTilemap,
  FileSelectMap_4_SetupExpandingSquareTransition,
  FileSelectMap_5_ExpandingSquare,
  FileSelectMap_6_AreaSelectMap,
  FileSelectMap_7_PrepExpandSquareTransToRoomMap,
  FileSelectMap_8_ExpandSquareTransToRoomSelectMap,
  FileSelectMap_9_InitRoomSelectMap,
  FileSelectMap_10_RoomSelectMap,
  FileSelectMap_11,
  FileSelectMap_11,
  FileSelectMap_13,
  FileSelectMap_14,
  FileSelectMap_15_ClearTileMap,
  FileSelectMap_16_LoadPalettes,
  FileSelectMap_2_LoadAreaSelectForegroundTilemap,
  FileSelectMap_18,
  FileSelectMap_4_SetupExpandingSquareTransition,
  FileSelectMap_20_SetupExpandingSquare,
  FileSelectMap_21_MoveExpandingSquare,
  FileSelectMap_22,
};

void FileSelectMap(void) {
  kFileSelectMapFuncs[menu_index]();
}
