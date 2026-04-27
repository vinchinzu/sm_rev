// File-select menu extracted from Bank 81.

#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "menu_common.h"
#include "enemy_types.h"
#include "samus_full_spec.h"

#define kOffsetToSaveSlot ((uint16*)RomFixedPtr(0x81812b))
#define kZebesAndStarsTilemap ((uint16*)RomFixedPtr(0x8edc00))

void FileSelectMenu_16(void);

static uint8 LoadFromSram_(uint16 a) {  // 0x81A053
  return LoadFromSram(a);
}

static void FileSelectClearRestOfMenuTilemapRow(uint16 v0) {  // 0x81B3C5
  int n = 32 - ((v0 & 0x3F) >> 1);
  do {
    *(uint16 *)((uint8 *)&ram3000.pause_menu_map_tilemap[768] + v0) = 15;
    v0 += 2;
  } while (--n);
}

static void DrawFileSelectionHealth(uint16 a, uint16 k) {  // 0x81A087
  int16 v2;
  int16 v4;

  if (a) {
    FileSelectClearRestOfMenuTilemapRow(k);
    LoadMenuTilemap(k + 64, addr_kMenuTilemap_NoData);
  } else {
    LoadMenuTilemap(k, addr_kMenuTilemap_Energy);
    v2 = k + 8;

    int n = samus_health / 100;
    int q = samus_health % 100;
    int mh = samus_max_health / 100;
    int m = 7;
    uint16 v3 = v2 + 64;
    while (--mh >= 0) {
      v4 = 153;
      if (n) {
        --n;
        v4 = 152;
      }
      *(uint16 *)((uint8 *)&ram3000.pause_menu_map_tilemap[768] + v3) = enemy_data[0].palette_index | v4;
      v3 += 2;
      if (!--m) {
        v3 -= 78;
        m = 8;
      }
    }
    n = q / 10;
    q = q % 10;
    *(uint16 *)((uint8 *)&ram3000.pause_menu_map_tilemap[802] + k) = enemy_data[0].palette_index | (q + 8288);
    *(uint16 *)((uint8 *)&ram3000.pause_menu_map_tilemap[801] + k) = enemy_data[0].palette_index | (n + 8288);
  }
}

static void DrawFileSelectionTime(uint16 a, uint16 k) {  // 0x81A14E
  if (!a) {
    uint16 r26 = k;
    int div_val = game_time_hours / 10;
    int mod_val = game_time_hours % 10;

    uint16 v2 = r26;
    *(uint16 *)((uint8 *)&ram3000.pause_menu_map_tilemap[769] + v2) = enemy_data[0].palette_index | (mod_val + 8288);
    *(uint16 *)((uint8 *)&ram3000.pause_menu_map_tilemap[768] + v2) = enemy_data[0].palette_index | (div_val + 8288);
    LoadMenuTilemap(r26 + 4, addr_word_81B4A8);

    int div_min = game_time_minutes / 10;
    int mod_min = game_time_minutes % 10;

    uint16 v3 = r26;
    *(uint16 *)((uint8 *)&ram3000.pause_menu_map_tilemap[772] + v3) = enemy_data[0].palette_index | (mod_min + 8288);
    *(uint16 *)((uint8 *)&ram3000.pause_menu_map_tilemap[771] + v3) = enemy_data[0].palette_index | (div_min + 8288);
  }
}

static void DrawFileSelectSlotSamusHelmet(uint16 k) {  // 0x819DE4
  uint16 v0 = k;
  int16 v1;

  v1 = *(uint16 *)((uint8 *)&eproj_enable_flag + v0);
  if (v1) {
    uint16 v2 = v1 - 1;
    *(uint16 *)((uint8 *)&eproj_enable_flag + v0) = v2;
    if (!v2) {
      *(uint16 *)((uint8 *)&eproj_enable_flag + v0) = 8;
      int v3 = v0 >> 1;
      uint16 v4 = eproj_id[v3] + 1;
      if (!sign16(eproj_id[v3] - 7)) {
        *(uint16 *)((uint8 *)&eproj_enable_flag + v0) = 0;
        v4 = 7;
      }
      eproj_id[v3] = v4;
    }
  }
  int v5 = v0 >> 1;
  uint16 v6 = 2 * eproj_id[v5];
  static const uint16 kDrawFileSlotHelmet_Spritemaps[9] = { 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x33 };
  DrawMenuSpritemap(kDrawFileSlotHelmet_Spritemaps[v6 >> 1], eproj_id[v5 + 5], eproj_id[v5 + 10], 3584);
}

static void DrawFileSelectSamusHelmets(void) {  // 0x819DC3
  eproj_index = 0;
  DrawFileSelectSlotSamusHelmet(4);
  eproj_init_param_1 = 0;
  DrawFileSelectSlotSamusHelmet(6);
  eproj_unk1995 = 0;
  DrawFileSelectSlotSamusHelmet(8);
}

static void DrawFileCopySaveSlotAInfo(void) {  // 0x81960F
  DrawFileSelectionHealth((nonempty_save_slots & 1) == 0, 0x218);
  DrawFileSelectionTime((nonempty_save_slots & 1) == 0, 0x272);
  LoadMenuTilemap(0x234, addr_kMenuTilemap_TIME);
  LoadMenuTilemap(0x208, addr_kMenuTilemap_SamusA);
}

static void DrawFileCopySaveSlotBInfo(void) {  // 0x81963F
  DrawFileSelectionHealth(~nonempty_save_slots & 2, 0x318);
  DrawFileSelectionTime(~nonempty_save_slots & 2, 0x372);
  LoadMenuTilemap(0x334, addr_kMenuTilemap_TIME);
  LoadMenuTilemap(0x308, addr_kMenuTilemap_SamusB);
}

static void DrawFileCopySaveSlotCInfo(void) {  // 0x81966F
  DrawFileSelectionHealth(~nonempty_save_slots & 4, 0x418);
  DrawFileSelectionTime(~nonempty_save_slots & 4, 0x472);
  LoadMenuTilemap(0x434, addr_kMenuTilemap_TIME);
  LoadMenuTilemap(0x408, addr_kMenuTilemap_SamusC);
}

static void DrawFileCopySaveFileInfo(void) {  // 0x8195BE
  LoadFromSram_(0);
  enemy_data[0].palette_index = 0;
  if ((nonempty_save_slots & 1) == 0)
    enemy_data[0].palette_index = 1024;
  DrawFileCopySaveSlotAInfo();
  LoadFromSram_(1);
  enemy_data[0].palette_index = 0;
  if ((nonempty_save_slots & 2) == 0)
    enemy_data[0].palette_index = 1024;
  DrawFileCopySaveSlotBInfo();
  LoadFromSram_(2);
  enemy_data[0].palette_index = 0;
  if ((nonempty_save_slots & 4) == 0)
    enemy_data[0].palette_index = 1024;
  DrawFileCopySaveSlotCInfo();
  QueueTransferOfMenuTilemapToVramBG1();
}

static void SetInitialFileCopyMenuSelection(void) {  // 0x819593
  int v0 = nonempty_save_slots;
  int v1 = 0;
  do {
    if (v0 & 1)
      break;
    v0 >>= 1;
    ++v1;
  } while ((int16)(v1 - 3) < 0);
  eproj_id[15] = v1;
}

void sub_819591(void) {  // 0x819591
  SetInitialFileCopyMenuSelection();
}

static void SetFileCopyMenuSelectionMissilePosition(void) {  // 0x81975E
  static const uint16 kFileCopyMissileY[4] = { 72, 104, 136, 211 };
  eproj_id[10] = kFileCopyMissileY[eproj_id[15]];
  eproj_id[5] = 22;
}

static void FileSelectMenu_Func1(void) {  // 0x819566
  ClearMenuTilemap();
  enemy_data[0].palette_index = 0;
  LoadMenuTilemap(0x52, addr_kMenuTilemap_DataCopyMode);
  LoadMenuTilemap(0x150, addr_kMenuTilemap_CopyWhichData);
  LoadMenuExitTilemap();
  DrawFileCopySaveFileInfo();
  SetInitialFileCopyMenuSelection();
  SetFileCopyMenuSelectionMissilePosition();
  eproj_id[16] = 0;
  eproj_id[17] = 0;
}

static void DrawFileCopySelectDestinationSaveFileInfo(void) {  // 0x819799
  ClearMenuTilemap();
  enemy_data[0].palette_index = 0;
  LoadMenuTilemap(0x52, addr_kMenuTilemap_DataCopyMode);
  enemy_data[0].palette_index = 0;
  LoadMenuTilemap(0x148, addr_kMenuTilemap_CopySamusToWhere);
  ram3000.pause_menu_map_tilemap[944] = eproj_id[16] + 8298;
  LoadMenuExitTilemap();
  LoadFromSram_(0);
  uint16 v0 = 1024;
  if (eproj_id[16])
    v0 = 0;
  enemy_data[0].palette_index = v0;
  DrawFileCopySaveSlotAInfo();
  LoadFromSram_(1);
  uint16 v1 = 1024;
  if (eproj_id[16] != 1)
    v1 = 0;
  enemy_data[0].palette_index = v1;
  DrawFileCopySaveSlotBInfo();
  LoadFromSram_(2);
  uint16 v2 = 1024;
  if (eproj_id[16] != 2)
    v2 = 0;
  enemy_data[0].palette_index = v2;
  DrawFileCopySaveSlotCInfo();
  QueueTransferOfMenuTilemapToVramBG1();
}

static void DrawFileCopyConfirmationSaveFileInfo(void) {  // 0x819922
  LoadFromSram_(0);
  int v0 = 0;
  if (eproj_id[16] && eproj_id[17])
    v0 = 1024;
  enemy_data[0].palette_index = v0;
  DrawFileCopySaveSlotAInfo();
  LoadFromSram_(1);
  int v1 = 0;
  if (eproj_id[16] != 1 && eproj_id[17] != 1)
    v1 = 1024;
  enemy_data[0].palette_index = v1;
  DrawFileCopySaveSlotBInfo();
  LoadFromSram_(2);
  int v2 = 0;
  if (eproj_id[16] != 2 && eproj_id[17] != 2)
    v2 = 1024;
  enemy_data[0].palette_index = v2;
  DrawFileCopySaveSlotCInfo();
  QueueTransferOfMenuTilemapToVramBG1();
}

static void DrawFileCopyClearConfirmation(void) {  // 0x8198ED
  enemy_data[0].palette_index = 0;
  LoadMenuTilemap(0x514, addr_kMenuTilemap_IsThisOk);
  enemy_data[0].palette_index = 0;
  LoadMenuTilemap(0x59C, addr_kMenuTilemap_Yes);
  int v0 = 832;
  do {
    ram3000.pause_menu_map_tilemap[v0 + 768] = 15;
    ++v0;
  } while ((int16)(v0 * 2 - 1728) < 0);
  enemy_data[0].palette_index = 0;
  LoadMenuTilemap(0x65C, addr_kMenuTilemap_No);
  DrawFileCopyConfirmationSaveFileInfo();
}

static void HandleFileCopyArrowPalette(void) {  // 0x8199FE
  if (eproj_unk198F) {
    if (!--eproj_unk198F) {
      eproj_unk198F = 4;
      uint16 v0 = palette_buffer[145];
      uint16 v1 = 0;
      do {
        palette_buffer[(v1 >> 1) + 145] = palette_buffer[(v1 >> 1) + 146];
        v1 += 2;
      } while ((int16)(v1 - 12) < 0);
      palette_buffer[151] = v0;
    }
  }
}

static void SetFileClearMenuMissilePos(void) {  // 0x819BEF
  static const uint16 kFileClear_MissileY[4] = { 72, 104, 136, 211 };
  eproj_id[10] = kFileClear_MissileY[eproj_id[15]];
  eproj_id[5] = 22;
}

static void InitFileSelectMenuFileClear(void) {  // 0x819B3C
  ClearMenuTilemap();
  enemy_data[0].palette_index = 0;
  LoadMenuTilemap(0x50, addr_kMenuTilemap_DataClearMode);
  LoadMenuTilemap(0x140, addr_kMenuTilemap_ClearWhichData);
  LoadMenuExitTilemap();
  DrawFileCopySaveFileInfo();
  eproj_id[16] = 0;
  SetInitialFileCopyMenuSelection();
  SetFileClearMenuMissilePos();
}

static void NewSaveFile(void) {  // 0x81B2CB
  samus_max_health = 99;
  samus_health = 99;
  samus_max_missiles = 0;
  samus_missiles = 0;
  samus_max_super_missiles = 0;
  samus_super_missiles = 0;
  samus_max_power_bombs = 0;
  samus_power_bombs = 0;
  hud_item_index = 0;
  collected_beams = 0;
  equipped_beams = 0;
  collected_items = 0;
  equipped_items = 0;
  reserve_health_mode = 0;
  samus_max_reserve_health = 0;
  samus_reserve_health = 0;
  samus_reserve_missiles = 0;
  button_config_up = kButton_Up;
  button_config_down = kButton_Down;
  button_config_left = kButton_Left;
  button_config_right = kButton_Right;
  button_config_jump_a = kButton_A;
  button_config_run_b = kButton_B;
  button_config_shoot_x = kButton_X;
  button_config_itemcancel_y = kButton_Y;
  button_config_itemswitch = kButton_Select;
  button_config_aim_up_R = kButton_R;
  button_config_aim_down_L = kButton_L;
  game_time_frames = 0;
  game_time_seconds = 0;
  game_time_minutes = 0;
  game_time_hours = 0;
  japanese_text_flag = 0;
  moonwalk_flag = 0;
  hud_auto_cancel_flag = 0;
  debug_flag = 1;
  UNUSED_word_7E09E8 = 1;
  uint16 v0 = 0;
  do {
    int v1 = v0 >> 1;
    WORD(room_chozo_bits[v0]) = 0;
    *(uint16 *)&item_bit_array[v0] = 0;
    *(uint16 *)&item_bit_array[v0] = 0;
    WORD(opened_door_bit_array[v0]) = 0;
    UNUSED_word_7ED8F0[v1] = 0;
    *(uint16 *)&map_station_byte_array[v0] = 0;
    *(uint16 *)&used_save_stations_and_elevators[v0] = 0;
    *(uint16 *)&used_save_stations_and_elevators[v0 + 8] = 0;
    v0 += 2;
  } while ((int16)(v0 - 8) < 0);
  do {
    int v2 = v0 >> 1;
    WORD(room_chozo_bits[v0]) = 0;
    *(uint16 *)&item_bit_array[v0] = 0;
    *(uint16 *)&item_bit_array[v0] = 0;
    WORD(opened_door_bit_array[v0]) = 0;
    v0 += 2;
  } while ((int16)(v0 - 64) < 0);
  uint16 v3 = 0;
  do {
    explored_map_tiles_saved[v3 >> 1] = 0;
    v3 += 2;
  } while ((int16)(v3 - 1792) < 0);
  SamusFullSpec_ApplyIfEnabled();
}

static const uint16 kMenuSelectionMissileXY[12] = {  // 0x81951E
  0x30, 0xe,
  0x58, 0xe,
  0x80, 0xe,
  0xa3, 0xe,
  0xbb, 0xe,
  0xd3, 0xe,
};

void FileSelectMenu_15_FadeOutToMain(void) {  // 0x8194F4
  HandleFadeOut();
  reg_MOSAIC = reg_MOSAIC & 0x0F | (16 * (reg_INIDISP & 0xF)) ^ 0xF0;
  if ((reg_INIDISP & 0xF) == 0) {
    ScreenOff();
    ++menu_index;
  }
}

void FileSelectMenu_7_FadeInFromMain(void) {  // 0x819532
  DrawMenuSelectionMissile();
  HandleFadeIn();
  reg_MOSAIC = reg_MOSAIC & 0x0F | (16 * (reg_INIDISP & 0xF)) ^ 0xF0;
  if ((reg_INIDISP & 0xF) == 15)
    ++menu_index;
}

void FileSelectMenu_0_FadeOutConfigGfx(void) {  // 0x81944E
  HandleFadeOut();
  if ((reg_INIDISP & 0xF) != 0)
    return;
  ScreenOff();
  QueueSfx3_Max6(1);
  DisableHdmaObjects();
  WaitUntilEndOfVblankAndClearHdma();
  ++menu_index;
  MapVramForMenu();
  LoadInitialMenuTiles();
  reg_BG1HOFS = 0;
  reg_BG2HOFS = 0;
  reg_BG3HOFS = 0;
  reg_BG1VOFS = 0;
  reg_BG2VOFS = 0;
  reg_BG3VOFS = 0;
  LoadFileSelectPalettes();
}

void FileSelectMenu_32_FadeOutToOptions(void) {  // 0x8194A3
  DrawMenuSelectionMissile();
  DrawBorderAroundSamusData();
  DrawFileSelectSamusHelmets();
  HandleFadeOut();
  if ((reg_INIDISP & 0xF) == 0) {
    game_state = kGameState_2_GameOptionsMenu;
    menu_index = 0;
    int v0 = 0;
    do {
      *(uint16 *)((uint8 *)&eproj_enable_flag + v0) = 0;
      v0 += 2;
    } while ((int16)(v0 - 48) < 0);
  }
}

void FileSelectMenu_33_FadeOutToTitle(void) {  // 0x8194D5
  DrawBorderAroundSamusData();
  DrawFileSelectSamusHelmets();
  HandleFadeOut();
  if ((reg_INIDISP & 0xF) == 0)
    SoftReset();
}

void FileSelectMenu_5_FadeOutFromMain(void) {  // 0x8194EE
  DrawMenuSelectionMissile();
  FileSelectMenu_15_FadeOutToMain();
}

void FileSelectMenu_17_FadeInToMain(void) {
  int v0 = (uint16)(4 * selected_save_slot) >> 1;
  eproj_id[10] = kMenuSelectionMissileXY[v0];
  eproj_id[5] = kMenuSelectionMissileXY[v0 + 1];
  FileSelectMenu_7_FadeInFromMain();
}

void FileSelectMenu_6_FileCopyInit(void) {  // 0x819561
  ++menu_index;
  FileSelectMenu_Func1();
}

void FileSelectMenu_8(void) {
  static const uint8 kBitShl[3] = { 1, 2, 4 };
  uint8 v0;

  DrawBorderAroundDataCopyMode();
  DrawMenuSelectionMissile();
  if ((joypad1_newkeys & (kButton_Start | kButton_A)) != 0) {
    QueueSfx1_Max6(0x37);
    if (eproj_id[15] == 3) {
      menu_index += 7;
    } else {
      eproj_id[16] = eproj_id[15];
      ++menu_index;
    }
  } else if ((joypad1_newkeys & 0x8000) != 0) {
    menu_index += 7;
    QueueSfx1_Max6(0x37);
  } else {
    if ((joypad1_newkeys & kButton_Up) != 0) {
      v0 = eproj_id[15];
      while ((--v0 & 0x80) == 0) {
        if ((kBitShl[v0] & nonempty_save_slots) != 0) {
LABEL_16:
          LOBYTE(eproj_id[15]) = v0;
          QueueSfx1_Max6(0x37);
          SetFileCopyMenuSelectionMissilePosition();
          return;
        }
      }
    } else if ((joypad1_newkeys & kButton_Down) != 0) {
      v0 = eproj_id[15];
      while ((int8)(++v0 - 4) < 0) {
        if (v0 == 3 || (kBitShl[v0] & nonempty_save_slots) != 0)
          goto LABEL_16;
      }
    }
    SetFileCopyMenuSelectionMissilePosition();
  }
}

void FileSelectMenu_9_InitializeSelectDest(void) {  // 0x81977A
  DrawBorderAroundDataCopyMode();
  DrawFileCopySelectDestinationSaveFileInfo();
  ++menu_index;
  uint16 v0 = 0;
  do {
    if (v0 != eproj_id[16])
      break;
    ++v0;
  } while (sign16(v0 - 3));
  eproj_id[15] = v0;
  SetFileCopyMenuSelectionMissilePosition();
}

void FileSelectMenu_10_FileCopySelectDest(void) {  // 0x819813
  static const uint16 kFileCopySelectDest_MissileY[4] = { 72, 104, 136, 212 };
  uint16 v0;

  DrawBorderAroundDataCopyMode();
  DrawMenuSelectionMissile();
  if ((joypad1_newkeys & (kButton_Start | kButton_A)) != 0) {
    QueueSfx1_Max6(0x37);
    if (eproj_id[15] != 3) {
      eproj_id[17] = eproj_id[15];
      ++menu_index;
LABEL_9:
      eproj_id[10] = kFileCopySelectDest_MissileY[eproj_id[15]];
      eproj_id[5] = 22;
      return;
    }
    menu_index += 5;
  } else {
    if ((joypad1_newkeys & 0x8000) == 0) {
      if ((joypad1_newkeys & kButton_Up) != 0) {
        QueueSfx1_Max6(0x37);
        v0 = eproj_id[15];
        while ((--v0 & 0x8000) == 0) {
          if (v0 != eproj_id[16]) {
LABEL_8:
            eproj_id[15] = v0;
            goto LABEL_9;
          }
        }
      } else if ((joypad1_newkeys & kButton_Down) != 0) {
        QueueSfx1_Max6(0x37);
        v0 = eproj_id[15];
        while (++v0 != 4) {
          if (v0 != eproj_id[16])
            goto LABEL_8;
        }
      }
      goto LABEL_9;
    }
    menu_index -= 2;
    eproj_id[15] = eproj_id[16];
    QueueSfx1_Max6(0x37);
    FileSelectMenu_Func1();
  }
}

void FileSelectMenu_11_InitializeConfirm(void) {  // 0x8198B7
  DrawBorderAroundDataCopyMode();
  enemy_data[0].palette_index = 0;
  LoadMenuTilemap(0x144, addr_kMenuTilemap_CopySamusToSamus);
  ram3000.pause_menu_map_tilemap[942] = eproj_id[16] + 8298;
  ram3000.pause_menu_map_tilemap[955] = eproj_id[17] + 8298;
  DrawFileCopyClearConfirmation();
  ++menu_index;
  eproj_id[15] = 0;
  eproj_unk198F = 8;
}

void FileSelectMenu_12_FileCopyConfirm(void) {  // 0x819984
  DrawBorderAroundDataCopyMode();
  DrawMenuSelectionMissile();
  HandleFileCopyArrowPalette();
  DrawFileCopyArrow();
  if ((joypad1_newkeys & (kButton_Up | kButton_Down)) != 0) {
    eproj_id[15] ^= 1;
    QueueSfx1_Max6(0x37);
  } else {
    if ((joypad1_newkeys & 0x8000) != 0) {
      menu_index -= 3;
      eproj_id[15] = eproj_id[17];
      QueueSfx1_Max6(0x37);
      return;
    }
    if ((joypad1_newkeys & (kButton_Start | kButton_A)) != 0) {
      QueueSfx1_Max6(0x38);
      if (eproj_id[15]) {
        menu_index -= 4;
        FileSelectMenu_Func1();
      } else {
        ++menu_index;
      }
      return;
    }
  }
  uint16 v0 = 184;
  if (eproj_id[15])
    v0 = 208;
  eproj_id[10] = v0;
  eproj_id[5] = 94;
}

void FileSelectMenu_13_FileCopyDoIt(void) {  // 0x819A2C
  static const uint16 kBitShl_16bit[3] = { 1, 2, 4 };

  DrawBorderAroundDataCopyMode();
  DrawMenuSelectionMissile();
  HandleFileCopyArrowPalette();
  DrawFileCopyArrow();
  uint16 src_addr = kOffsetToSaveSlot[eproj_id[16]];
  uint16 dst_addr = kOffsetToSaveSlot[eproj_id[17]];
  memcpy(&g_sram[dst_addr], &g_sram[src_addr], 1628);
  int v2 = eproj_id[16];
  int v10 = *(uint16 *)(&g_sram[2 * v2 + 0x1FF0]);
  int v9 = *(uint16 *)(&g_sram[2 * v2 + 0x1FF8]);
  int v8 = *(uint16 *)(&g_sram[2 * v2 + 0]);
  int v4 = eproj_id[17];
  *(uint16 *)(&g_sram[2 * v4 + 8]) = *(uint16 *)&g_sram[2 * v2 + 8];
  *(uint16 *)(&g_sram[2 * v4]) = v8;
  *(uint16 *)(&g_sram[2 * v4 + 0x1FF8]) = v9;
  *(uint16 *)(&g_sram[2 * v4 + 0x1FF0]) = v10;
  ++menu_index;
  int v5 = 640;
  do {
    ram3000.pause_menu_map_tilemap[v5 + 768] = 15;
    ++v5;
  } while ((int16)(v5 * 2 - 1856) < 0);
  nonempty_save_slots |= kBitShl_16bit[eproj_id[17]];
  int v6 = ((4 * eproj_id[17] + 9) << 6) + 24;
  int v7 = 0;
  do {
    *(uint16 *)((uint8 *)&ram3000.pause_menu_map_tilemap[768] + v6) = 15;
    *(uint16 *)((uint8 *)&ram3000.pause_menu_map_tilemap[736] + v6) = 15;
    v6 += 2;
    v7 += 2;
  } while ((int16)(v7 - 22) < 0);
  enemy_data[0].palette_index = 0;
  LoadMenuTilemap(0x510, addr_kMenuTilemap_CopyCompleted);
  DrawFileCopyConfirmationSaveFileInfo();
  RtlWriteSram();
}

void FileSelectMenu_14_CopyCompleted(void) {  // 0x819AFA
  DrawBorderAroundDataCopyMode();
  if (joypad1_newkeys) {
    QueueSfx1_Max6(0x37);
    ++menu_index;
    int slot = sram_save_slot_selected;
    if (sign16(slot) || !sign16(sram_save_slot_selected - 3) ||
        (slot & sram_save_slot_selected_complement)) {
      slot = 0;
    }
    selected_save_slot = slot;
  }
}

void FileSelectMenu_18(void) {  // 0x819B28
  menu_index -= 14;
}

void FileSelectMenu_20_FileClearInit(void) {  // 0x819B33
  DrawBorderAroundDataClearMode();
  ++menu_index;
  InitFileSelectMenuFileClear();
}

void FileSelectMenu_22_FileClearSelectSlot(void) {  // 0x819B64
  static const uint8 kBitShl_[3] = { 1, 2, 4 };
  uint16 v0;
  uint8 v1;

  DrawBorderAroundDataClearMode();
  DrawMenuSelectionMissile();
  HIBYTE(v0) = HIBYTE(joypad1_newkeys);
  if ((joypad1_newkeys & (kButton_Start | kButton_A)) != 0) {
    QueueSfx1_Max6(0x37);
    if (eproj_id[15] != 3) {
      eproj_id[16] = eproj_id[15];
      ++menu_index;
      return;
    }
LABEL_11:
    QueueSfx1_Max6(0x37);
    menu_index += 5;
    return;
  }
  if ((joypad1_newkeys & 0x8000) != 0)
    goto LABEL_11;
  if ((joypad1_newkeys & kButton_Up) != 0) {
    v1 = eproj_id[15];
    while ((--v1 & 0x80) == 0) {
      if ((kBitShl_[v1] & nonempty_save_slots) != 0) {
LABEL_16:
        LOBYTE(eproj_id[15]) = v1;
        LOBYTE(v0) = 55;
        QueueSfx1_Max6(v0);
        SetFileClearMenuMissilePos();
        return;
      }
    }
  } else if ((joypad1_newkeys & kButton_Down) != 0) {
    v1 = eproj_id[15];
    while ((int8)(++v1 - 4) < 0) {
      if (v1 == 3 || (kBitShl_[v1] & nonempty_save_slots) != 0)
        goto LABEL_16;
    }
  }
  SetFileClearMenuMissilePos();
}

void FileSelectMenu_23_FileClearInitConfirm(void) {  // 0x819C0B
  DrawBorderAroundDataClearMode();
  enemy_data[0].palette_index = 0;
  LoadMenuTilemap(0x140, addr_kMenuTilemap_ClearSamusA);
  ram3000.pause_menu_map_tilemap[949] = eproj_id[16] + 8298;
  eproj_id[17] = 3;
  DrawFileCopyClearConfirmation();
  ++menu_index;
  eproj_id[15] = 0;
}

void FileSelectMenu_24_FileClearConfirm(void) {  // 0x819C36
  uint16 v0;
  DrawBorderAroundDataClearMode();
  DrawMenuSelectionMissile();
  if ((joypad1_newkeys & (kButton_Up | kButton_Down)) != 0) {
    eproj_id[15] ^= 1;
    QueueSfx1_Max6(0x37);
LABEL_8:
    v0 = 184;
    if (eproj_id[15])
      v0 = 208;
    eproj_id[10] = v0;
    eproj_id[5] = 94;
    return;
  }
  if ((joypad1_newkeys & 0x8000) != 0) {
LABEL_5:
    menu_index -= 2;
    eproj_id[15] = eproj_id[16];
    QueueSfx1_Max6(0x37);
    InitFileSelectMenuFileClear();
    return;
  }
  if ((joypad1_newkeys & (kButton_Start | kButton_A)) == 0)
    goto LABEL_8;
  QueueSfx1_Max6(0x38);
  if (eproj_id[15])
    goto LABEL_5;
  ++menu_index;
}

void FileSelectMenu_25_FileClearDoClear(void) {  // 0x819C9E
  static const uint16 kBitShl_Not[3] = { 0xfffe, 0xfffd, 0xfffb };

  DrawBorderAroundDataClearMode();
  int sram_addr = kOffsetToSaveSlot[eproj_id[16]];
  memset(&g_sram[sram_addr], 0, 1628);

  int v2 = eproj_id[16];
  *(uint16 *)(&g_sram[2 * v2]) = 0;
  *(uint16 *)(&g_sram[2 * v2 + 8]) = 0;
  *(uint16 *)(&g_sram[2 * v2 + 0x1FF0]) = 0;
  *(uint16 *)(&g_sram[2 * v2 + 0x1FF8]) = 0;
  ++menu_index;
  NewSaveFile();
  LoadFromSram(eproj_id[16]);
  area_index = eproj_id[16];
  LoadMirrorOfExploredMapTiles();
  uint16 v3 = 640;
  do {
    ram3000.pause_menu_map_tilemap[v3 + 768] = 15;
    ++v3;
  } while ((int16)(v3 * 2 - 1856) < 0);
  nonempty_save_slots &= kBitShl_Not[eproj_id[16]];
  enemy_data[0].palette_index = 0;
  LoadMenuTilemap(0x500, addr_kMenuTilemap_DataCleared);
  DrawFileCopyConfirmationSaveFileInfo();
  RtlWriteSram();
}

void FileSelectMenu_26_ClearCompleted(void) {  // 0x819D26
  DrawBorderAroundDataClearMode();
  if (joypad1_newkeys) {
    QueueSfx1_Max6(0x37);
    ++menu_index;
    if (!(LoadFromSram(0))) {
LABEL_3:
      selected_save_slot = 0;
      return;
    }
    if (LoadFromSram(1)) {
      if (LoadFromSram(2))
        goto LABEL_3;
      selected_save_slot = 2;
    } else {
      selected_save_slot = 1;
    }
  }
}

void FileSelectMenu_30(void) {  // 0x819D68
  DrawBorderAroundSamusData();
  menu_index -= 26;
}

void FileSelectMenu_31_TurnSamusHelmet(void) {  // 0x819D77
  DrawMenuSelectionMissile();
  DrawBorderAroundSamusData();
  DrawFileSelectSlotSamusHelmet(4);
  DrawFileSelectSlotSamusHelmet(6);
  DrawFileSelectSlotSamusHelmet(8);
  if ((joypad1_newkeys & (kButton_Start | kButton_A)) != 0
      || eproj_id[2] == 7 && !eproj_index
      || eproj_id[3] == 7 && !eproj_init_param_1
      || eproj_id[4] == 7 && !eproj_unk1995) {
    ++menu_index;
  }
}

void FileSelectMenu_1_LoadFileSelectMenuBG2(void) {  // 0x819E93
  for (int i = 1023; i >= 0; --i)
    ram3000.pause_menu_map_tilemap[i + 768] = kZebesAndStarsTilemap[i];
  uint16 v1 = vram_write_queue_tail;
  VramWriteEntry *v2 = gVramWriteEntry(vram_write_queue_tail);
  v2->size = 2048;
  v2->src.addr = ADDR16_OF_RAM(ram3000.menu.menu_tilemap);
  *(uint16 *)&v2->src.bank = 126;
  v2->vram_dst = (reg_BG2SC & 0xFC) << 8;
  vram_write_queue_tail = v1 + 7;
  ++menu_index;
  eproj_enable_flag = 1;
  eproj_id[0] = 0;
}

void FileSelectMenu_2_InitMain(void) {  // 0x819ED6
  uint16 v0;

  if (sign16(sram_save_slot_selected)
      || !sign16(sram_save_slot_selected - 3)
      || (v0 = sram_save_slot_selected, (sram_save_slot_selected_complement & sram_save_slot_selected) != 0)) {
    v0 = 0;
  }
  selected_save_slot = v0;
  FileSelectMenu_16();
}

void FileSelectMenu_16(void) {  // 0x819EF3
  for (int i = 2046; i >= 0; i -= 2)
    *(uint16 *)((uint8 *)&ram3000.pause_menu_map_tilemap[768] + (uint16)i) = 15;
  nonempty_save_slots = -1;
  enemy_data[0].palette_index = 0;
  LoadMenuTilemap(0x56, addr_kMenuTilemap_SamusData);
  LoadMenuTilemap(0x146, addr_kMenuTilemap_SamusA);
  uint8 c = LoadFromSram_(0);
  nonempty_save_slots = (nonempty_save_slots >> 1) | (c << 15);
  DrawFileSelectionHealth(nonempty_save_slots & 0x8000, 0x15C);
  DrawFileSelectionTime(nonempty_save_slots & 0x8000, 0x1B4);
  LoadMenuTilemap(0x176, addr_kMenuTilemap_TIME);
  enemy_data[0].palette_index = 0;
  LoadMenuTilemap(0x286, addr_kMenuTilemap_SamusB);
  c = LoadFromSram_(1);
  nonempty_save_slots = (nonempty_save_slots >> 1) | (c << 15);
  DrawFileSelectionHealth(nonempty_save_slots & 0x8000, 0x29C);
  DrawFileSelectionTime(nonempty_save_slots & 0x8000, 0x2F4);
  LoadMenuTilemap(0x2B6, addr_kMenuTilemap_TIME);
  enemy_data[0].palette_index = 0;
  LoadMenuTilemap(0x3C6, addr_kMenuTilemap_SamusC);
  c = LoadFromSram_(2);
  nonempty_save_slots = (nonempty_save_slots >> 1) | (c << 15);
  DrawFileSelectionHealth(nonempty_save_slots & 0x8000, 0x3DC);
  DrawFileSelectionTime(nonempty_save_slots & 0x8000, 0x434);
  LoadMenuTilemap(0x3F6, addr_kMenuTilemap_TIME);
  nonempty_save_slots = swap16(~nonempty_save_slots) >> 5;
  if (nonempty_save_slots) {
    enemy_data[0].palette_index = 0;
    LoadMenuTilemap(0x508, addr_kMenuTilemap_DataCopy);
    enemy_data[0].palette_index = 0;
    LoadMenuTilemap(0x5C8, addr_kMenuTilemap_DataClear);
  }
  enemy_data[0].palette_index = 0;
  LoadMenuTilemap(0x688, addr_kMenuTilemap_Exit);
  QueueTransferOfMenuTilemapToVramBG1();
  eproj_enable_flag = 1;
  eproj_unk198F = 0;
  eproj_index = 0;
  eproj_init_param_1 = 0;
  eproj_unk1995 = 0;
  eproj_id[0] = 0;
  eproj_id[1] = 0;
  eproj_id[2] = 0;
  eproj_id[3] = 0;
  eproj_id[4] = 0;
  eproj_id[5] = 0;
  eproj_id[10] = 0;
  eproj_id[6] = 0;
  eproj_id[11] = 0;
  eproj_id[7] = 100;
  eproj_id[8] = 100;
  eproj_id[9] = 100;
  eproj_id[12] = 47;
  eproj_id[13] = 87;
  eproj_id[14] = 127;
  screen_fade_delay = 1;
  screen_fade_counter = 1;
  ScreenOn();
  ++menu_index;
  eproj_id[15] = 0;
  eproj_id[16] = 0;
  eproj_id[17] = 0;
}

void FileSelectMenu_3_FadeInToMain(void) {  // 0x81A058
  DrawFileSelectSamusHelmets();
  int v0 = (uint16)(4 * selected_save_slot) >> 1;
  eproj_id[10] = kMenuSelectionMissileXY[v0];
  eproj_id[5] = kMenuSelectionMissileXY[v0 + 1];
  DrawMenuSelectionMissile();
  DrawBorderAroundSamusData();
  HandleFadeIn();
  if ((reg_INIDISP & 0xF) == 15)
    ++menu_index;
}

void FileSelectMenu_4_Main(void) {  // 0x81A1C2
  int16 v0;
  uint16 v1, v2;
  int v3;

  DrawBorderAroundSamusData();
  DrawMenuSelectionMissile();
  DrawFileSelectSlotSamusHelmet(4);
  DrawFileSelectSlotSamusHelmet(6);
  DrawFileSelectSlotSamusHelmet(8);
  if ((joypad1_newkeys & (kButton_Start | kButton_A)) == 0) {
    if ((joypad1_newkeys & kButton_Up) != 0) {
      if (nonempty_save_slots) {
        v1 = selected_save_slot - 1;
        if ((int16)(selected_save_slot - 1) < 0)
          v1 = 5;
      } else {
        v1 = selected_save_slot - 1;
        if ((int16)(selected_save_slot - 1) >= 0) {
          if (!sign16(selected_save_slot - 5))
            v1 = 2;
        } else {
          v1 = 5;
        }
      }
      selected_save_slot = v1;
    } else {
      if ((joypad1_newkeys & kButton_Down) == 0) {
        if ((joypad1_newkeys & kButton_B) != 0) {
          QueueSfx1_Max6(0x37);
          menu_index = 33;
          QueueSfx1_Max6(0x37);
        }
        goto LABEL_28;
      }
      if (nonempty_save_slots) {
        v2 = selected_save_slot + 1;
        if (!sign16(selected_save_slot - 5))
          v2 = 0;
      } else {
        v2 = selected_save_slot + 1;
        if (!sign16(selected_save_slot - 2)) {
          if (sign16(selected_save_slot - 5))
            v2 = 5;
          else
            v2 = 0;
        }
      }
      selected_save_slot = v2;
    }
    QueueSfx1_Max6(0x37);
LABEL_28:
    v3 = (uint16)(4 * selected_save_slot) >> 1;
    eproj_id[10] = kMenuSelectionMissileXY[v3];
    eproj_id[5] = kMenuSelectionMissileXY[v3 + 1];
    return;
  }
  v0 = selected_save_slot;
  if (sign16(selected_save_slot - 3)) {
    QueueSfx1_Max6(0x2A);
    menu_index += 27;
    *(uint16 *)((uint8 *)&eproj_enable_flag + (uint16)(2 * (selected_save_slot + 2))) = 1;
    *(uint16 *)&g_sram[0x1FEC] = selected_save_slot;
    *(uint16 *)&g_sram[0x1FEE] = ~selected_save_slot;
    RtlWriteSram();
    if (LoadFromSram(selected_save_slot)) {
      NewSaveFile();
      has_area_map = 0;
    } else {
      LoadMirrorOfExploredMapTiles();
    }
    goto LABEL_28;
  }
  if (selected_save_slot == 3) {
    QueueSfx1_Max6(0x37);
    ++menu_index;
    *(uint16 *)&reg_MOSAIC = *(uint16 *)&reg_MOSAIC & 0xFF0C | 3;
  } else {
    if (selected_save_slot == 4) {
      QueueSfx1_Max6(0x37);
      menu_index += 15;
      v0 = *(uint16 *)&reg_MOSAIC & 0xFF0C | 3;
      *(uint16 *)&reg_MOSAIC = v0;
    }
    if (v0 == 5)
      menu_index = 33;
  }
}

static Func_V *const kFileSelectMenuFuncs[34] = {  // 0x8193FB
  FileSelectMenu_0_FadeOutConfigGfx,
  FileSelectMenu_1_LoadFileSelectMenuBG2,
  FileSelectMenu_2_InitMain,
  FileSelectMenu_3_FadeInToMain,
  FileSelectMenu_4_Main,
  FileSelectMenu_5_FadeOutFromMain,
  FileSelectMenu_6_FileCopyInit,
  FileSelectMenu_7_FadeInFromMain,
  FileSelectMenu_8,
  FileSelectMenu_9_InitializeSelectDest,
  FileSelectMenu_10_FileCopySelectDest,
  FileSelectMenu_11_InitializeConfirm,
  FileSelectMenu_12_FileCopyConfirm,
  FileSelectMenu_13_FileCopyDoIt,
  FileSelectMenu_14_CopyCompleted,
  FileSelectMenu_15_FadeOutToMain,
  FileSelectMenu_16,
  FileSelectMenu_17_FadeInToMain,
  FileSelectMenu_18,
  FileSelectMenu_5_FadeOutFromMain,
  FileSelectMenu_20_FileClearInit,
  FileSelectMenu_7_FadeInFromMain,
  FileSelectMenu_22_FileClearSelectSlot,
  FileSelectMenu_23_FileClearInitConfirm,
  FileSelectMenu_24_FileClearConfirm,
  FileSelectMenu_25_FileClearDoClear,
  FileSelectMenu_26_ClearCompleted,
  FileSelectMenu_15_FadeOutToMain,
  FileSelectMenu_16,
  FileSelectMenu_17_FadeInToMain,
  FileSelectMenu_30,
  FileSelectMenu_31_TurnSamusHelmet,
  FileSelectMenu_32_FadeOutToOptions,
  FileSelectMenu_33_FadeOutToTitle,
};

void FileSelectMenu(void) {
  kFileSelectMenuFuncs[menu_index]();
}
