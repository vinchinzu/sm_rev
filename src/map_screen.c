#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "sm_82_data.h"

void SetupMapScrollingForFileSelectMap(void) {  // 0x829028
  ResetPauseMenuAnimations();
  DetermineMapScrollLimits();
  reg_BG1HOFS = map_min_x_scroll + ((uint16)(map_max_x_scroll - map_min_x_scroll) >> 1) - 128;
  uint16 r18 = 8 * (room_x_coordinate_on_map + ((samus_x_pos & 0xFF00) >> 8)) - reg_BG1HOFS;
  int16 v0 = 224 - r18;
  if ((int16)(224 - r18) >= 0) {
    r18 = 32 - r18;
    if ((r18 & 0x8000) == 0)
      reg_BG1HOFS -= r18;
  } else {
    r18 = 224 - r18;
    reg_BG1HOFS -= v0;
  }
  r18 = map_min_y_scroll + ((uint16)(map_max_y_scroll - map_min_y_scroll) >> 1) + 16;
  reg_BG1VOFS = -((112 - r18) & 0xFFF8);
  r18 = 8 * (room_y_coordinate_on_map + HIBYTE(samus_y_pos) + 1) + ((112 - r18) & 0xFFF8);
  int16 v1 = 64 - r18;
  if ((int16)(64 - r18) >= 0) {
    r18 = 64 - r18;
    reg_BG1VOFS -= v1;
    if (sign16(reg_BG1VOFS + 40))
      reg_BG1VOFS = -40;
  }
}

static Func_V *const kMapScrollingFuncs[5] = {  // 0x82925D
  MapScrolling_0_None,
  MapScrolling_1_Left,
  MapScrolling_2_Right,
  MapScrolling_3_Up,
  MapScrolling_4_Down,
};

static const uint16 kMapScrollingSpeedTable[32] = {  // 0x82928E
  0, 0, 0, 8,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 8,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
};

static const uint16 kMap_SamusPositionIndicator_Delays[4] = { 8, 4, 8, 4 };
static const uint16 kMap_SamusPositionIndicator_SpritemapIds[4] = { 0x5f, 0x60, 0x61, 0x60 };

static const uint8 k0x80Shr[8] = { 0x80, 0x40, 0x20, 0x10, 8, 4, 2, 1 };
static const uint8 k0x80Shr_0[8] = { 0x80, 0x40, 0x20, 0x10, 8, 4, 2, 1 };

void PauseMenu_0_MapScreen(void) {  // 0x829120
  HandlePauseScreenLR();
  HandlePauseScreenStart();
  HandleMapScrollArrows();
  MapScrolling();
  MapScreenDrawSamusPositionIndicator();
  DrawMapIcons();
  DisplayMapElevatorDestinations();
  pause_screen_mode = 0;
}

void MapScrolling(void) {
  kMapScrollingFuncs[map_scrolling_direction]();
}

void MapScrolling_0_None(void) {  // 0x829278
  map_scrolling_gear_switch_timer = 4;
}

uint16 MapScrolling_GetSpeedIndex(void) {  // 0x82927F
  uint16 result = map_scrolling_speed_index;
  if (!map_scrolling_gear_switch_timer)
    return map_scrolling_speed_index + 32;
  return result;
}

void MapScrolling_1_Left(void) {
  reg_BG1HOFS -= kMapScrollingSpeedTable[MapScrolling_GetSpeedIndex() >> 1];
  MapScrolling_Common();
}

void MapScrolling_Common(void) {  // 0x829299
  ++map_scrolling_speed_index;
  if ((++map_scrolling_speed_index & 0xF) == 0) {
    QueueSfx1_Max6(0x36);
    map_scrolling_direction = 0;
    map_scrolling_speed_index = 0;
    if (map_scrolling_gear_switch_timer)
      --map_scrolling_gear_switch_timer;
  }
}

void MapScrolling_2_Right(void) {  // 0x8292BD
  reg_BG1HOFS += kMapScrollingSpeedTable[MapScrolling_GetSpeedIndex() >> 1];
  MapScrolling_Common();
}

void MapScrolling_3_Up(void) {  // 0x8292CA
  reg_BG1VOFS -= kMapScrollingSpeedTable[MapScrolling_GetSpeedIndex() >> 1];
  MapScrolling_Common();
}

void MapScrolling_4_Down(void) {  // 0x8292D7
  reg_BG1VOFS += kMapScrollingSpeedTable[MapScrolling_GetSpeedIndex() >> 1];
  MapScrolling_Common();
}

void LoadPauseMenuMapTilemapAndAreaLabel(void) {  // 0x8293C3
  reg_BG1HOFS = reg_BG4HOFS;
  reg_BG1VOFS = reg_BG4VOFS;
  WriteReg(VMADDL, 0);
  WriteReg(VMADDH, 0x30);
  WriteReg(VMAIN, 0x80);
  LoadPauseMenuMapTilemap();
  static const StartDmaCopy unk_8293E9 = { 1, 1, 0x18, LONGPTR(0x7e4000), 0x1000 };
  SetupDmaTransfer(&unk_8293E9);
  WriteReg(MDMAEN, 2);
  WriteReg(VMADDL, 0xAA);
  WriteReg(VMADDH, 0x38);
  WriteReg(VMAIN, 0x80);
  WriteReg(DMAP1, 1);
  WriteReg(BBAD1, 0x18);
  WriteReg(DAS1L, 0x18);
  WriteReg(DAS1H, 0);
  uint16 v0 = area_index;
  if (!sign16(area_index - 7))
    v0 = 0;
  WriteRegWord(A1T1L, kPauseAreaLabelTilemap[v0]);
  WriteReg(A1B1, 0x82);
  WriteReg(MDMAEN, 2);
}

void LoadPauseMenuMapTilemap(void) {  // 0x82943D
  uint16 v0 = area_index;
  if (!sign16(area_index - 7))
    v0 = 0;
  const uint16 *r0 = (const uint16 *)RomPtr(Load24(&kPauseMenuMapTilemaps[v0]));
  uint16 *r3 = (uint16 *)&ram4000;
  const uint16 *r6 = (const uint16 *)RomPtr_82(kPauseMenuMapData[v0]);
  const uint16 *r9 = (const uint16 *)map_tiles_explored;
  if (map_station_byte_array[area_index]) {
    uint16 r38 = swap16(*r6++);
    uint16 r40 = swap16(*r9++);
    uint16 v8 = 0;
    int v9 = 16;
    do {
      uint16 v10 = r0[v8 >> 1];
      bool v11 = r40 >> 15;
      r40 *= 2;
      if (v11) {
        v10 &= ~0x400;
        r38 *= 2;
      } else {
        v11 = r38 >> 15;
        r38 *= 2;
        if (!v11)
          v10 = 31;
      }
      r3[v8 >> 1] = v10;
      if (!--v9) {
        v9 = 16;
        r38 = swap16(*r6++);
        r40 = swap16(*r9++);
      }
      v8 += 2;
    } while ((int16)(v8 - 4096) < 0);
  } else {
    uint16 v1 = 0;
    uint16 v2 = 0;
    uint8 r18 = 0;
    while (1) {
      uint8 t = map_tiles_explored[v2];
      map_tiles_explored[v2] <<= 1;
      if (!(t & 0x80)) {
        r3[v1 >> 1] = 0x1f;
      } else {
        ++map_tiles_explored[v2];
        r3[v1 >> 1] = r0[v1 >> 1] & 0xFBFF;
      }
      v1 += 2;
      if (++r18 >= 8) {
        r18 = 0;
        if ((int16)(++v2 - 256) >= 0)
          break;
      }
    }
  }
}

void DrawRoomSelectMap(void) {  // 0x829517
  reg_BG12NBA = 51;
  reg_TM = 19;
  reg_BG1VOFS = -40;
  uint16 v0 = area_index;
  if (!sign16(area_index - 7))
    v0 = 0;

  const uint16 *r0 = (const uint16 *)RomPtr(Load24(&kPauseMenuMapTilemaps[v0]));
  uint16 *r3 = (uint16 *)&ram3000;
  const uint16 *r6 = (const uint16 *)RomPtr_82(kPauseMenuMapData[v0]);
  const uint16 *r9 = (const uint16 *)map_tiles_explored;

  if (map_station_byte_array[area_index]) {
    uint16 r38 = swap16(*r6++);
    uint16 r40 = swap16(*r9++);
    uint16 v9 = 0;
    int v10 = 16;
    do {
      uint16 v11 = r0[v9 >> 1];
      bool v12 = r40 >> 15;
      r40 *= 2;
      if (v12) {
        v11 &= ~0x400;
        r38 *= 2;
      } else {
        v12 = r38 >> 15;
        r38 *= 2;
        if (!v12)
          v11 = 31;
      }
      r3[v9 >> 1] = v11;
      if (!--v10) {
        v10 = 16;
        r38 = swap16(*r6++);
        r40 = swap16(*r9++);
      }
      v9 += 2;
    } while ((int16)(v9 - 4096) < 0);
  } else {
    uint16 v1 = 0;
    uint16 v2 = 0;
    uint8 r18 = 0;
    while (1) {
      uint8 what = map_tiles_explored[v2];
      map_tiles_explored[v2] <<= 1;
      if (!(what & 0x80)) {
        r3[v1 >> 1] = 0xf;
      } else {
        ++map_tiles_explored[v2];
        r3[v1 >> 1] = r0[v1 >> 1] & ~0x400;
      }
      v1 += 2;
      r18++;
      if (!sign8(r18 - 8)) {
        r18 = 0;
        if ((int16)(++v2 - 256) >= 0)
          break;
      }
    }
  }
  uint16 v16 = vram_write_queue_tail;
  VramWriteEntry *v17 = gVramWriteEntry(vram_write_queue_tail);
  v17->size = 4096;
  v17->src.addr = ADDR16_OF_RAM(ram3000);
  v17->src.bank = 126;
  v17->vram_dst = (reg_BG1SC & 0xFC) << 8;
  vram_write_queue_tail = v16 + 7;
}

void DrawRoomSelectMapAreaLabel(uint16 *dst) {  // 0x829628
  const uint16 *v2 = (const uint16 * )RomPtr_82(kPauseAreaLabelTilemap[area_index]);
  for(int i = 0; i < 12; i++)
    dst[i] = v2[i] & 0xEFFF;
}

void SetupMapScrollingForPauseMenu(uint16 a) {  // 0x829E27
  int16 v1;
  int16 v2;

  reg_BG1HOFS = map_min_x_scroll + ((uint16)(map_max_x_scroll - map_min_x_scroll) >> 1) - 128;
  uint16 r18 = 8 * (room_x_coordinate_on_map + ((samus_x_pos & 0xFF00) >> 8)) - reg_BG1HOFS;
  v1 = 224 - r18;
  if ((int16)(224 - r18) >= 0) {
    r18 = 32 - r18;
    if ((r18 & 0x8000) == 0)
      reg_BG1HOFS -= r18;
  } else {
    r18 = 224 - r18;
    reg_BG1HOFS -= v1;
  }
  r18 = map_min_y_scroll + ((uint16)(map_max_y_scroll - map_min_y_scroll) >> 1) + 16;
  reg_BG1VOFS = -((a - r18) & 0xFFF8);
  r18 = 8 * (room_y_coordinate_on_map + HIBYTE(samus_y_pos) + 1) + ((a - r18) & 0xFFF8);
  v2 = 64 - r18;
  if ((int16)v2 >= 0) {
    reg_BG1VOFS -= v2;
    if (sign16(reg_BG1VOFS + 40))
      reg_BG1VOFS = -40;
  }
}

static uint16 DetermineLeftmostMapColumn(const uint8 *r0) {
  uint16 v0;

  uint16 result = 0;
LABEL_2:
  v0 = result & 7;
  uint8 r18 = k0x80Shr[v0];
  uint16 v2 = 0;
  while ((r18 & r0[v2]) == 0) {
    v2 += 4;
    if ((int16)(v2 - 128) >= 0) {
      if ((int16)(++result - 64) >= 0)
        return 26;
      if ((result & 7) == 0) {
        r0 += 1;
      }
      if (result == 32) {
        r0 += 123;
      }
      goto LABEL_2;
    }
  }
  return result;
}

static uint16 DetermineRightmostMapColumn(const uint8 *r0) {  // 0x829FA9
  uint16 result = 63;
LABEL_2:;
  uint8 r18 = k0x80Shr_0[result & 7];
  uint16 v1 = 0;
  while ((r18 & r0[v1]) == 0) {
    v1 += 4;
    if ((int16)(v1 - 128) >= 0) {
      if ((--result & 0x8000) != 0)
        return 28;
      if ((result & 7) == 7) {
        r0--;
      }
      if (result == 31)
        r0 -= 124;
      goto LABEL_2;
    }
  }
  return result;
}

static uint16 DetermineTopmostMapColumn(const uint8 *r0) {  // 0x82A009
  const uint8 *r3 = r0 + 128;
  uint16 result = 0;
  uint16 v1 = 0;
  while (!r0[v1] && !r3[v1]) {
    if ((int16)(++v1 - 4) >= 0) {
      v1 = 0;
      r0 += 4;
      r3 += 4;
      if ((int16)(++result - 31) >= 0)
        return 1;
    }
  }
  return result;
}

static uint16 DetermineBottommostMapColumn(const uint8 *r0) {  // 0x82A053
  const uint8 *r3 = r0 + 128;
  uint16 result = 31;
  uint16 v1 = 0;
  while (!r0[v1] && !r3[v1]) {
    if ((int16)(++v1 - 4) >= 0) {
      v1 = 0;
      r0 -= 4;
      r3 -= 4;
      if (!--result)
        return 11;
    }
  }
  return result;
}

void DetermineMapScrollLimits(void) {  // 0x829EC4
  const uint8 *r6;
  if (has_area_map) {
    r6 = RomPtr_82(GET_WORD(RomPtr_82(addr_kPauseMenuMapData + 2 * area_index)));
  } else {
    r6 = map_tiles_explored;
  }
  map_min_x_scroll = DetermineLeftmostMapColumn(r6) * 8;
  if (area_index == 4)
    map_min_x_scroll -= 24;
  map_max_x_scroll = DetermineRightmostMapColumn(r6 + 131) * 8;
  map_min_y_scroll = DetermineTopmostMapColumn(r6) * 8;
  map_max_y_scroll = DetermineBottommostMapColumn(r6 + 124) * 8;
}

void SetupPpuForPauseMenu(void) {  // 0x82A09A
  WriteReg(OBSEL, 1);
  reg_OBSEL = 1;
  WriteReg(BGMODE, 9);
  reg_BGMODE = 9;
  reg_BG12NBA = 0;
  WriteReg(BG12NBA, 0);
  reg_BG34NBA = 4;
  WriteReg(BG34NBA, 4);
  reg_BG1SC = 49;
  WriteReg(BG1SC, 0x31);
  reg_BG2SC = 56;
  WriteReg(BG2SC, 0x38);
  reg_BG3SC = 88;
  WriteReg(BG3SC, 0x58);
  reg_BG4SC = 0;
  WriteReg(BG4SC, 0);
  WriteReg(TM, 0x17);
  reg_TM = 23;
  WriteReg(MOSAIC, 0);
  reg_MOSAIC = 0;
  reg_COLDATA[0] &= 0xE0;
  reg_COLDATA[1] &= 0xE0;
  reg_COLDATA[2] &= 0xE0;
  next_gameplay_CGADSUB = 0;
}

void ResetPauseMenuAnimations(void) {  // 0x82A0F7
  pausemenu_reserve_tank_delay_ctr = 0;
  reg_BG1HOFS = 0;
  reg_BG2HOFS = 0;
  reg_BG3HOFS = 0;
  reg_BG2VOFS = 0;
  reg_BG3VOFS = 0;
  pausemenu_button_label_mode = 0;
  pausemenu_lr_animation_frame = 0;
  UNUSED_word_7E0745 = 0;
  samus_position_indicator_animation_frame = 0;
  samus_position_indicator_animation_timer = 0;
  samus_position_indicator_animation_loop_counter = 0;
  pausemenu_lr_animation_timer = *(uint16 *)kPauseLrHighlightAnimData;
  pausemenu_palette_animation_timer = 1;
  pausemenu_palette_animation_frame = 0;
}

void DrawMapIcons(void) {  // 0x82B672
  DrawBossMapIcons(9, addr_kMapIconDataPointers);
  DrawSimpleMapIcons(0xB, addr_kMapIconDataPointers + 0x10, 3584);
  DrawSimpleMapIcons(0xA, addr_kMapIconDataPointers + 0x20, 3584);
  DrawSimpleMapIcons(0x4E, addr_kMapIconDataPointers + 0x30, 3584);
  DrawSaveStationMapIcon(8, 0xC80B, 1024);
  if (enable_debug)
    DrawSimpleMapIcons(8, 0xC82B, 1024);
  if (!area_index) {
    DrawMenuSpritemap(0x63, kMap_Criteria_SavePoints[0] - reg_BG1HOFS, kMap_Criteria_SavePoints[1] - reg_BG1VOFS, 3584);
  }
}

void DrawFileSelectMapIcons(void) {  // 0x82B6DD
  HandlePauseScreenPaletteAnimation();
  DrawBossMapIcons(9, 0xC7CB);
  DrawSimpleMapIcons(0xB, 0xC7DB, 3584);
  DrawSimpleMapIcons(0xA, 0xC7EB, 3584);
  DrawSimpleMapIcons(0x4E, 0xC7FB, 3584);
  uint16 a = UpdateSamusPositionIndicatorAnimation();
  const uint16 *data = (uint16 *)RomPtr_82(*(VoidP *)((uint8 *)&kMapIconDataPointers[4].crateria + 2 * area_index)) + load_station_index * 2;
  uint16 v1 = data[0] - reg_BG1HOFS;
  uint16 v2 = data[1] - reg_BG1VOFS;
  if ((samus_position_indicator_animation_loop_counter & 1) == 0)
    DrawMenuSpritemap(0x12, v1, v2, 3584);
  DrawMenuSpritemap(a, v1, v2, 3584);
  if (enable_debug) {
    DrawDebugSaveMapIcons(0xC, 0xC80B, 1536);
    DrawDebugElevatorMapIcons(0x17, 0xC81B, 1536);
    DrawSimpleMapIcons(0xC, 0xC82B, 1536);
  }
  if (!area_index)
    DrawMenuSpritemap(0x63, kMap_Criteria_SavePoints[0] - reg_BG1HOFS, kMap_Criteria_SavePoints[1] - reg_BG1VOFS, 3584);
}

void DrawSaveStationMapIcon(uint16 a, uint16 k, uint16 r3) {  // 0x82B798
  static const uint8 kShlBit[8] = { 1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80 };
  uint16 R34 = a;
  if (area_index == sram_area_index) {
    uint16 R36 = *(uint16 *)&used_save_stations_and_elevators[2 * area_index];
    LOBYTE(R36) = R36 & kShlBit[load_station_index];
    uint16 v2 = *(uint16 *)RomPtr_82(k + 2 * area_index);
    if (v2)
      DrawMapIconsOfType(v2, R34, R36, r3);
  }
}

void DrawDebugSaveMapIcons(uint16 a, uint16 k, uint16 r3) {  // 0x82B7D1
  uint16 R34 = a;
  uint16 R36 = *(uint16 *)&used_save_stations_and_elevators[2 * area_index];
  uint16 v2 = *(uint16 *)RomPtr_82(k + 2 * area_index);
  if (v2)
    DrawMapIconsOfType(v2, R34, R36, r3);
}

void DrawDebugElevatorMapIcons(uint16 a, uint16 k, uint16 r3) {  // 0x82B7EB
  uint16 R34 = a;
  uint16 R36 = *(uint16 *)&used_save_stations_and_elevators[2 * area_index + 1];
  uint16 v2 = *(uint16 *)RomPtr_82(k + 2 * area_index);
  if (v2)
    DrawMapIconsOfType(v2, R34, R36, r3);
}

void DrawSimpleMapIcons(uint16 a, uint16 k, uint16 r3) {  // 0x82B805
  uint16 R34 = a;
  uint16 v2 = *(uint16 *)RomPtr_82(k + 2 * area_index);
  if (v2)
    DrawMapIconsOfType(v2, R34, -1, r3);
}

void DrawMapIconsOfType(uint16 a, uint16 r34, uint16 r36, uint16 r3) {  // 0x82B81C
  int8 v3; // cf
  int16 v4;

  while (1) {
    const uint16 *v2 = (const uint16 *)RomPtr_82(a);
    if ((*v2 & 0x8000) != 0)
      break;
    v3 = r36 & 1;
    r36 >>= 1;
    if (v3) {
      v4 = CheckIfMapPositionIsExplored(v2[0], v2[1]);
      if (v4)
        DrawMenuSpritemap(r34, v2[0] - reg_BG1HOFS, v2[1] - reg_BG1VOFS, r3);
    }
    a += 4;
  }
}

uint16 CheckIfMapPositionIsExplored(uint16 k, uint16 j) {  // 0x82B855
  static const uint8 kBits0x80Shr[8] = { 0x80, 0x40, 0x20, 0x10, 8, 4, 2, 1 };
  int r18 = (uint16)(k & 0xFF00) >> 1;
  r18 += (uint8)k >> 6;
  r18 += (uint16)(j & 0xFFF8) >> 1;
  uint16 v3 = (k >> 3) & 7;
  return (kBits0x80Shr[v3] & map_tiles_explored[r18]) != 0;
}

void DrawBossMapIcons(uint16 a, uint16 k) {  // 0x82B892
  int bits = boss_bits_for_area[area_index];
  int t = *(uint16 *)RomPtr_82(k + 2 * area_index);
  if (t == 0)
    return;
  const uint16 *v4 = (const uint16 *)RomPtr_82(t);
  for (; ; v4 += 2) {
    if (v4[0] == 0xffff)
      return;
    if (v4[0] != 0xfffe) {
      int v5 = bits & 1;
      bits >>= 1;
      if (v5) {
        DrawMenuSpritemap(0x62, v4[0] - reg_BG1HOFS, v4[1] - reg_BG1VOFS, 3584);
        DrawMenuSpritemap(a, v4[0] - reg_BG1HOFS, v4[1] - reg_BG1VOFS, 3072);
        continue;
      }
      if (has_area_map) {
        DrawMenuSpritemap(a, v4[0] - reg_BG1HOFS, v4[1] - reg_BG1VOFS, 3584);
        continue;
      }
    }
    bits >>= 1;
  }
}

void DrawMapScrollArrowAndCheckToScroll(uint8 db, uint16 k) {  // 0x82B90A
  const uint16 *v1 = (const uint16 *)RomPtrWithBank(db, k);
  DrawPauseScreenSpriteAnim(v1[2], *v1, v1[1]);
  const uint8 *v2 = RomPtrWithBank(db, k);
  if ((joypad1_lastkeys & GET_WORD(v2 + 6)) != 0 && !map_scrolling_direction)
    map_scrolling_direction = GET_WORD(v2 + 8);
}

void HandleMapScrollArrows(void) {  // 0x82B934
  if (sign16(map_min_x_scroll - 24 - reg_BG1HOFS))
    DrawMapScrollArrowAndCheckToScroll(0x82, addr_stru_82B9A0);
  if (!sign16(map_max_x_scroll - 232 - reg_BG1HOFS))
    DrawMapScrollArrowAndCheckToScroll(0x82, addr_stru_82B9AA);
  if (sign16(map_min_y_scroll - 56 - reg_BG1VOFS))
    DrawMapScrollArrowAndCheckToScroll(0x82, addr_stru_82B9B4);
  if (sign16(map_max_y_scroll - 177 - reg_BG1VOFS)) {
    if (map_scrolling_direction == g_stru_82B9BE.map_scroll_dir) {
      map_scrolling_gear_switch_timer = 0;
      map_scrolling_direction = 0;
      map_scrolling_speed_index = 0;
    }
  } else {
    DrawMapScrollArrowAndCheckToScroll(0x82, addr_stru_82B9BE);
  }
}

void MapScreenDrawSamusPositionIndicator(void) {  // 0x82B9C8
  uint16 a = UpdateSamusPositionIndicatorAnimation();
  DrawMenuSpritemap(a, 8 * (room_x_coordinate_on_map + HIBYTE(samus_x_pos)) - reg_BG1HOFS,
                       8 * (room_y_coordinate_on_map + HIBYTE(samus_y_pos) + 1) - reg_BG1VOFS, 3584);
}

uint16 UpdateSamusPositionIndicatorAnimation(void) {  // 0x82B9FC
  uint16 v0 = samus_position_indicator_animation_timer;
  if (!v0) {
    uint16 v1 = samus_position_indicator_animation_frame + 2;
    if ((int16)(v1 - 8) >= 0) {
      ++samus_position_indicator_animation_loop_counter;
      v1 = 0;
    }
    samus_position_indicator_animation_frame = v1;
    v0 = kMap_SamusPositionIndicator_Delays[v1 >> 1];
    samus_position_indicator_animation_timer = v0;
  }
  samus_position_indicator_animation_timer = v0 - 1;
  return kMap_SamusPositionIndicator_SpritemapIds[samus_position_indicator_animation_frame >> 1];
}

void DisplayMapElevatorDestinations(void) {  // 0x82BB30
  if (map_station_byte_array[area_index]) {
    for (int i = kMapElevatorDests[area_index]; ; i += 6) {
      const uint8 *v1 = RomPtr_82(i);
      if (GET_WORD(v1) == 0xFFFF)
        break;
      DrawMenuSpritemap(GET_WORD(v1 + 4), GET_WORD(v1) - reg_BG1HOFS, GET_WORD(v1 + 2) - reg_BG1VOFS, 0);
    }
  }
}
