#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "menu_assets.h"

static Func_V *const kEquipmentScreenCategories[4] = {  // 0x82AC4F
  EquipmentScreenCategory_Tanks,
  EquipmentScreenCategory_Weapons,
  EquipmentScreenCategory_Suit,
  EquipmentScreenCategory_Boots,
};

static Func_V *const kEquipmentScreenCategory_TanksFuncs[2] = {  // 0x82AC70
  EquipmentScreenCategory_Tanks_0,
  EquipmentScreenCategory_Tanks_1,
};

static Func_V *const kEquipmentScreenGlowingArrowFuncs[2] = {  // 0x82AD0A
  EquipmentScreenGlowingArrowAnimated,
  EquipmentScreenGlowingArrowSolidOn,
};

static const uint16 kEquipmentScreenGlowingArrowPalettes0[32] = { 0x39e, 0x77d, 0xb5c, 0xf5b, 0x133a, 0x171a, 0x1f19, 0x22f8, 0x26d7, 0x2ad6, 0x2eb6, 0x3695, 0x3a94, 0x3e73, 0x4253, 0x4a52, 0x4a52, 0x4253, 0x3e73, 0x3a94, 0x3695, 0x2eb6, 0x2ad6, 0x26d7, 0x22f8, 0x1f19, 0x171a, 0x133a, 0xf5b, 0xb5c, 0x77d, 0x39e };
static const uint16 kEquipmentScreenGlowingArrowPalettes1[32] = { 0x156, 0x155, 0x554, 0x954, 0xd53, 0xd52, 0x1152, 0x1551, 0x1970, 0x1d70, 0x1d6f, 0x216e, 0x256e, 0x296d, 0x296c, 0x318c, 0x318c, 0x296c, 0x296d, 0x256e, 0x216e, 0x1d6f, 0x1d70, 0x1970, 0x1551, 0x1152, 0xd52, 0xd53, 0x954, 0x554, 0x155, 0x156 };

static const uint16 kEquipmentScreenTilemap_MANUAL[4] = { 0x3d46, 0x3d47, 0x3d48, 0x3d49 };
static const uint16 kEquipmentScreenTilemap_AUTO[4] = { 0x3d56, 0x3d57, 0x3d58, 0x3d59 };
static const uint16 kEquipmentScreenReserveTank_X[6] = { 0x18, 0x20, 0x28, 0x30, 0x38, 0x40 };
static const uint16 kEquipmentScreenReserveTank_Y = 0x60;
static const uint16 kPartialReserveTankSpritemapIds[16] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27 };
static const uint16 kEquipmentScreenWireframeCmp[3] = { 0x0101, 0x0001, 0x0000 };
static const uint16 kEquipmentScreenWireframePtrs[3] = { 0xb251, 0xb231, 0xb211 };

static void ChangePaletteValues(uint16 *tilemap, uint16 palette_mask, uint16 count) {
  for (int i = 0; i < count; ++i)
    tilemap[i] = (tilemap[i] & ~0x1C00) | palette_mask;
}

void PauseMenu_1_EquipmentScreen(void) {  // 0x829142
  reg_BG1HOFS = 0;
  reg_BG1VOFS = 0;
  EquipmentScreenMain();
  HandlePauseScreenLR();
  HandlePauseScreenStart();
  pause_screen_mode = 1;
}

void LoadEquipmentScreenEquipmentTilemaps(void) {  // 0x82A12B
  if (samus_max_reserve_health) {
    memcpy(g_ram + kEquipmentTilemapOffs_Tanks[0], RomPtr_82(kEquipmentTilemaps_Tanks[0]), 14);
    memcpy(g_ram + kEquipmentTilemapOffs_Tanks[1], RomPtr_82(kEquipmentTilemaps_Tanks[1]), 14);
  }
  const uint16 *p = (uint16*)RomPtr_82(addr_kEquipmentTilemapOffs_Weapons);
  if (hyper_beam_flag) {
    for(int i = 0; i < 5; i++)
      memcpy(g_ram + p[i], RomPtr_82(kHyperBeamWeaponsTilemaps[i]), 10);
  } else {
    for (int i = 0; i < 6; i++) {
      uint16 *target = (uint16 *)(g_ram + p[i]);
      if ((collected_beams & kEquipmentBitmasks_Weapons[i]) != 0) {
        memcpy(target, RomPtr_82(kEquipmentTilemaps_Weapons[i]), 10);
        if ((equipped_beams & kEquipmentBitmasks_Weapons[i]) == 0)
          ChangePaletteValues(target, 3072, 5);
      } else {
        memcpy(target, RomPtr_82(addr_kEquipmentScreenTilemap_Blank), 10);
      }
    }
  }

  p = (uint16*)RomPtr_82(addr_kEquipmentTilemapOffs_Suits);
  for(int i = 0; i < 6; i++) {
    uint16 *target = (uint16*)(g_ram + p[i]);
    if ((collected_items & kEquipmentBitmasks_Suits[i]) != 0) {
      memcpy(target, RomPtr_82(kEquipmentTilemaps_Suits[i]), 18);
      if ((equipped_items & kEquipmentBitmasks_Suits[i]) == 0)
        ChangePaletteValues(target, 3072, 9);
    } else {
      memcpy(target, RomPtr_82(addr_kEquipmentScreenTilemap_Blank), 18);
    }
  }
  
  p = (uint16*)RomPtr_82(addr_kEquipmentTilemapOffs_Boots);
  for (int i = 0; i < 3; i++) {
    uint16 *target = (uint16*)(g_ram + p[i]);
    if ((collected_items & kEquipmentBitmasks_Boots[i]) != 0) {
      memcpy(target, RomPtr_82(kEquipmentTilemaps_Boots[i]), 18);
      if ((equipped_items & kEquipmentBitmasks_Boots[i]) == 0)
        ChangePaletteValues(target, 3072, 9);
    } else {
      memcpy(target, RomPtr_82(addr_kEquipmentScreenTilemap_Blank), 18);
    }
  }
}

void EquipmentScreenSetupReserveMode(void) {  // 0x82AB47
  VoidP v0;
  int16 v5;
  int16 v7;
  int16 v8;

  reg_BG4HOFS = reg_BG1HOFS;
  reg_BG4VOFS = reg_BG1VOFS;
  reg_BG1HOFS = 0;
  reg_BG1VOFS = 0;
  if (samus_max_reserve_health && reserve_health_mode) {
    v0 = addr_kEquipmentScreenTilemap_AUTO;
    if (reserve_health_mode != 1)
      v0 = addr_kEquipmentScreenTilemap_MANUAL;
    uint16 *table = (uint16 *)RomPtr_82(v0);
    for (int i = 0; i < 4; i++)
      ram3800.cinematic_bg_tilemap[i + 327] = ram3800.cinematic_bg_tilemap[i + 327] & 0xFC00 | table[i];
  }
  pausemenu_item_selector_animation_frame = 0;
  pausemenu_item_selector_animation_timer = kPauseLrHighlightAnimData[0];
  pausemenu_reserve_tank_animation_frame = 0;
  pausemenu_reserve_tank_animation_timer = kPauseReserveTankAnimationData[0];
  if (samus_max_reserve_health) {
    pausemenu_equipment_category_item = 0;
  } else if (hyper_beam_flag) {
LABEL_15:;
    uint16 v6 = 0;
    do {
      if ((kEquipmentBitmasks_Suits[v6 >> 1] & collected_items) != 0) {
        LOBYTE(v7) = (uint16)(v6 >> 1) >> 8;
        HIBYTE(v7) = v6 >> 1;
        pausemenu_equipment_category_item = v7 | 2;
        goto LABEL_21;
      }
      v6 += 2;
    } while ((int16)(v6 - 12) < 0);
    v8 = 0;
    do
      v8 += 2;
    while ((int16)(v8 - 6) < 0);
  } else {
    uint16 v4 = 0;
    while ((kEquipmentBitmasks_Weapons[v4 >> 1] & collected_beams) == 0) {
      v4 += 2;
      if ((int16)(v4 - 10) >= 0)
        goto LABEL_15;
    }
    LOBYTE(v5) = (uint16)(v4 >> 1) >> 8;
    HIBYTE(v5) = v4 >> 1;
    pausemenu_equipment_category_item = v5 | 1;
  }
LABEL_21:
  if (samus_reserve_health) {
    EquipmentScreenGlowingArrowSolidOn();
    WriteSamusWireframeTilemapAndQueue();
  }
}

void EquipmentScreenTransferBG1Tilemap(void) {  // 0x82AC22
  WriteReg(VMADDL, 0);
  WriteReg(VMADDH, 0x30);
  WriteReg(VMAIN, 0x80);
  static const StartDmaCopy unk_82AC3B = { 1, 1, 0x18, LONGPTR(0x7e3800), 0x0800 };
  SetupDmaTransfer(&unk_82AC3B);
  WriteReg(MDMAEN, 2);
  reg_BG1VOFS = 0;
}

void EquipmentScreenMain(void) {
  kEquipmentScreenCategories[(uint8)pausemenu_equipment_category_item]();
  EquipmentScreenDrawItemSelector();
  EquipmentScreenDisplayReserveTankAmount();
  WriteSamusWireframeTilemapAndQueue();
}

void EquipmentScreenCategory_Tanks(void) {
  kEquipmentScreenCategory_TanksFuncs[HIBYTE(pausemenu_equipment_category_item)]();
  EquipmentScreenHandleDpad();
  EquipmentScreenGlowingArrow();
}

void EquipmentScreenHandleDpad(void) {  // 0x82AC8B
  uint16 r18 = pausemenu_equipment_category_item;
  if ((joypad1_newkeys & kButton_Right) != 0) {
    if ((joypad1_newkeys & kButton_Down) != 0 || EquipmentScreenMoveLowerOnSuitsMisc(0) == 0xFFFF)
      EquipmentScreenMoveToHighJumpOrLowerInBoots(0, r18);
  } else if ((joypad1_newkeys & kButton_Up) != 0) {
    if ((pausemenu_equipment_category_item & 0xFF00) != 0) {
      QueueSfx1_Max6(0x37);
      pausemenu_equipment_category_item -= 256;
    }
  } else if ((joypad1_newkeys & kButton_Down) != 0) {
    if ((pausemenu_equipment_category_item & 0xFF00) == 256
        || reserve_health_mode == 1
        || (pausemenu_equipment_category_item += 256, !samus_reserve_health)) {
      EquipmentScreenMoveToBeams(0, r18);
    } else {
      QueueSfx1_Max6(0x37);
    }
  }
}

void EquipmentScreenGlowingArrow(void) {
  if ((uint8)pausemenu_equipment_category_item)
    EquipmentScreenGlowingArrowSolidOff();
  else
    kEquipmentScreenGlowingArrowFuncs[HIBYTE(pausemenu_equipment_category_item)]();
}

void EquipmentScreenGlowingArrowAnimated(void) {  // 0x82AD29
  if (reserve_health_mode == 1) {
    int v0 = nmi_frame_counter_byte & 0x1F;
    palette_buffer[102] = kEquipmentScreenGlowingArrowPalettes0[v0];
    palette_buffer[107] = kEquipmentScreenGlowingArrowPalettes1[v0];
    EquipmentScreenEnergyArrowGlow_On();
  } else {
    palette_buffer[107] = 926;
    palette_buffer[102] = 342;
    EquipmentScreenEnergyArrowGlow_Off();
  }
}

void EquipmentScreenGlowingArrowSolidOn(void) {  // 0x82ADDD
  palette_buffer[107] = 926;
  palette_buffer[102] = 342;
  EquipmentScreenEnergyArrowGlow_On();
}

void EquipmentScreenGlowingArrowSolidOff(void) {  // 0x82ADEF
  palette_buffer[107] = 926;
  palette_buffer[102] = 342;
  EquipmentScreenEnergyArrowGlow_Off();
}

void EquipmentScreenEnergyArrowGlow_On(void) {  // 0x82AE01
  for (int i = 0; i < 8; i++)
    ram3800.equipment_screen_bg1_tilemap[129 + i * 32] = ram3800.equipment_screen_bg1_tilemap[129 + i * 32] & ~0x1C00 | 0x1800;
  for(int i = 0; i < 2; i++)
    ram3800.equipment_screen_bg1_tilemap[385 + i] = ram3800.equipment_screen_bg1_tilemap[385 + i] & ~0x1C00 | 0x1800;
}

void EquipmentScreenEnergyArrowGlow_Off(void) {  // 0x82AE46
  for(int i = 0; i < 8; i++)
    ram3800.equipment_screen_bg1_tilemap[129 + i * 32] = ram3800.equipment_screen_bg1_tilemap[129 + i * 32] & ~0x1C00 | 0x1C00;
  for (int i = 0; i < 2; i++)
    ram3800.equipment_screen_bg1_tilemap[385 + i] = ram3800.equipment_screen_bg1_tilemap[385 + i] & ~0x1C00 | 0x1C00;
}

void EquipmentScreenCategory_Tanks_0(void) {
  int16 v0;
  int16 v2;

  if ((joypad1_newkeys & kButton_A) != 0 && samus_max_reserve_health) {
    QueueSfx1_Max6(0x37);
    if (reserve_health_mode == 1) {
      reserve_health_mode = 2;
      EquipmentScreenHudReserveAutoTilemap_Off();
      v0 = 8;
      uint16 v1 = 0;
      do {
        ram3800.cinematic_bg_tilemap[v1 + 327] = kEquipmentScreenTilemap_MANUAL[v1] | ram3800.cinematic_bg_tilemap[v1 + 327] & 0xFC00;
        ++v1;
        v0 -= 2;
      } while (v0);
    } else {
      reserve_health_mode = 1;
      EquipmentScreenHudReserveAutoTilemap_On_BUGGY();
      v2 = 8;
      uint16 v3 = 0;
      do {
        ram3800.cinematic_bg_tilemap[v3 + 327] = kEquipmentScreenTilemap_AUTO[v3] | ram3800.cinematic_bg_tilemap[v3 + 327] & 0xFC00;
        ++v3;
        v2 -= 2;
      } while (v2);
    }
  }
}

void EquipmentScreenHudReserveAutoTilemap_On_BUGGY(void) {  // 0x82AEFD
  uint16 v0 = -26229;
  if (!samus_reserve_health)
    v0 = -26217;
  const uint16 *v1 = (const uint16 *)RomPtr_82(v0);
  hud_tilemap[8] = *v1;
  hud_tilemap[9] = v1[1];
  hud_tilemap[40] = v1[2];
  hud_tilemap[41] = v1[3];
  hud_tilemap[72] = v1[4];
  hud_tilemap[73] = v1[5];
}

void EquipmentScreenHudReserveAutoTilemap_Off(void) {  // 0x82AF33
  hud_tilemap[8] = 11279;
  hud_tilemap[9] = 11279;
  hud_tilemap[40] = 11279;
  hud_tilemap[41] = 11279;
  hud_tilemap[72] = 11279;
  hud_tilemap[73] = 11279;
}

void EquipmentScreenCategory_Tanks_1(void) {  // 0x82AF4F
  if (!pausemenu_reserve_tank_delay_ctr) {
    if ((joypad1_newkeys & kButton_A) == 0)
      return;
    pausemenu_reserve_tank_delay_ctr = (samus_reserve_health + 7) & 0xFFF8;
  }
  if ((--pausemenu_reserve_tank_delay_ctr & 7) == 7)
    QueueSfx3_Max6(0x2D);
  samus_health += 1;
  if ((int16)(samus_health - samus_max_health) < 0) {
    samus_reserve_health -= 1;
    if ((int16)samus_reserve_health > 0)
      return;
    samus_health += samus_reserve_health;
  } else {
    samus_health = samus_max_health;
  }
  samus_reserve_health = 0;
  pausemenu_reserve_tank_delay_ctr = 0;
  EquipmentScreenEnergyArrowGlow_Off();
  pausemenu_equipment_category_item = 0;
}

void EquipmentScreenCategory_Weapons(void) {  // 0x82AFBE
  EquipmentScreenCategory_Weapons_MoveButtons();
  uint16 R36 = equipped_beams;
  if (collected_beams) {
    if ((uint8)pausemenu_equipment_category_item == 1) {
      EquipmentScreenCategory_ButtonResponse(10);
      EquipmentScreenCategory_Weapons_PlazmaSpazerCheck(R36);
    }
  }
}

void EquipmentScreenCategory_Weapons_MoveButtons(void) {  // 0x82AFDB
  uint16 r18 = pausemenu_equipment_category_item;
  if ((joypad1_newkeys & kButton_Right) != 0) {
    if ((joypad1_newkeys & kButton_Down) != 0) {
      EquipmentScreenMoveLowerOnSuitsMisc(0);
    } else if (EquipmentScreenMoveLowerOnSuitsMisc(4)) {
      EquipmentScreenMoveToHighJumpOrLowerInBoots(0, r18);
    }
  } else if ((joypad1_newkeys & kButton_Down) != 0) {
    if (!hyper_beam_flag && pausemenu_equipment_category_item != 1025) {
      pausemenu_equipment_category_item += 256;
      EquipmentScreenMoveToBeams(2 * HIBYTE(pausemenu_equipment_category_item), r18);
    }
  } else if ((joypad1_newkeys & kButton_Up) != 0) {
    if ((pausemenu_equipment_category_item & 0xFF00) == 0
        || (pausemenu_equipment_category_item -= 256,
            EquipmentScreenMoveToBottomOfBeams(2 * HIBYTE(pausemenu_equipment_category_item)) == 0xFFFF)) {
      if (!EquipmentScreenMoveToReserveTanks())
        pausemenu_equipment_category_item = r18;
    }
  }
}

void EquipmentScreenCategory_Weapons_PlazmaSpazerCheck(uint16 R36) {  // 0x82B068
  int t = equipped_beams & ~R36;
  if ((t & 4) != 0) {
    if ((R36 & 4) == 0 && (equipped_beams & 8) != 0) {
      equipped_beams &= ~8;
      ChangePaletteValues((uint16 *)(g_ram + kEquipmentTilemapOffs_Weapons[4]), 3072, 5);
    }
  } else if ((t & 8) != 0 && (R36 & 8) == 0 && (equipped_beams & 4) != 0) {
    equipped_beams &= ~4;
    ChangePaletteValues((uint16 *)(g_ram + kEquipmentTilemapOffs_Weapons[3]), 3072, 5);
  }
}

void EquipmentScreenCategory_Suit(void) {  // 0x82B0C2
  EquipmentScreenCategory_Suit_MoveResponse();
  if ((uint8)pausemenu_equipment_category_item == 2)
    EquipmentScreenCategory_ButtonResponse(18);
}

void EquipmentScreenCategory_Suit_MoveResponse(void) {  // 0x82B0D2
  uint16 r18 = pausemenu_equipment_category_item;
  if ((joypad1_newkeys & kButton_Left) != 0) {
    if ((joypad1_newkeys & kButton_Down) == 0) {
      if (EquipmentScreenMoveToReserveTanks())
        return;
      pausemenu_equipment_category_item = r18;
    }
    EquipmentScreenMoveToBeams(0, r18);
  } else if ((joypad1_newkeys & kButton_Up) != 0) {
    if ((pausemenu_equipment_category_item & 0xFF00) != 0) {
      pausemenu_equipment_category_item -= 256;
      EquipmentScreenMoveToScrewOrHigherOnSuits(2 * HIBYTE(pausemenu_equipment_category_item), r18);
    }
  } else if ((joypad1_newkeys & kButton_Down) != 0) {
    if ((pausemenu_equipment_category_item & 0xFF00) == 1280
        || (pausemenu_equipment_category_item += 256,
            EquipmentScreenMoveLowerOnSuitsMisc(2 * HIBYTE(pausemenu_equipment_category_item)) == 0xFFFF)) {
      EquipmentScreenMoveToHighJumpOrLowerInBoots(0, r18);
    }
  }
}

void EquipmentScreenCategory_Boots(void) {  // 0x82B150
  EquipmentScreenCategory_Boots_MoveResponse();
  if ((uint8)pausemenu_equipment_category_item == 3)
    EquipmentScreenCategory_ButtonResponse(18);
}

void EquipmentScreenCategory_Boots_MoveResponse(void) {  // 0x82B160
  uint16 r18 = pausemenu_equipment_category_item;
  if ((joypad1_newkeys & kButton_Left) != 0) {
    if (((joypad1_newkeys & kButton_Up) != 0 || EquipmentScreenMoveToBottomOfBeams(8) == 0xFFFF)
        && !EquipmentScreenMoveToReserveTanks()) {
      pausemenu_equipment_category_item = r18;
    }
  } else if ((joypad1_newkeys & kButton_Down) != 0) {
    if (pausemenu_equipment_category_item != 515) {
      pausemenu_equipment_category_item += 256;
      EquipmentScreenMoveToHighJumpOrLowerInBoots(2 * HIBYTE(pausemenu_equipment_category_item), r18);
    }
  } else if ((joypad1_newkeys & kButton_Up) != 0) {
    if ((pausemenu_equipment_category_item & 0xFF00) == 0
        || (pausemenu_equipment_category_item -= 256,
            EquipmentScreenCategory_Boots_MoveUpInBoots(2 * HIBYTE(pausemenu_equipment_category_item)) == 0xFFFF)) {
      EquipmentScreenMoveToScrewOrHigherOnSuits(0xA, r18);
    }
  }
}

void WriteSamusWireframeTilemapAndQueue(void) {  // 0x82B1E0
  WriteSamusWireframeTilemap();
  uint16 v0 = vram_write_queue_tail;
  gVramWriteEntry(vram_write_queue_tail)->size = 1280;
  v0 += 2;
  gVramWriteEntry(v0)->size = ADDR16_OF_RAM(ram3800) + 256;
  v0 += 2;
  LOBYTE(gVramWriteEntry(v0++)->size) = 126;
  gVramWriteEntry(v0)->size = 0x3080;
  vram_write_queue_tail = v0 + 2;
}

void WriteSamusWireframeTilemap(void) {  // 0x82B20C
  uint16 i;
  for (i = 0; (equipped_items & 0x101) != kEquipmentScreenWireframeCmp[i >> 1]; i += 2) ;
  const uint16 *src = (const uint16*)RomPtr_82(kEquipmentScreenWireframePtrs[i >> 1]);
  int m = 17;
  uint16 v2 = 472;
  do {
    uint16 v3 = v2;
    int n = 8;
    do {
      *(uint16 *)((uint8 *)ram3800.cinematic_bg_tilemap + v2) = *src++;
      v2 += 2;
    } while (--n);
    v2 = v3 + 64;
  } while (--m);
}

void EquipmentScreenDrawItemSelector(void) {  // 0x82B267
  if (samus_max_reserve_health | (uint16)(collected_items | collected_beams)) {
    uint16 *t = (uint16*)RomPtr_82(kEquipmentScreenPtrsToItemXYpos[(uint8)pausemenu_equipment_category_item] + 
        4 * HIBYTE(pausemenu_equipment_category_item));
    DrawPauseScreenSpriteAnim(3, t[0] - 1, t[1]);
  }
}

void EquipmentScreenDisplayReserveTankAmount(void) {  // 0x82B2A2
  EquipmentScreenDisplayReserveTankAmount_();
}

void EquipmentScreenDisplayReserveTankAmount_(void) {
  uint16 r3 = EquipmentScreenDisplayReserves_PaletteSetup();
  int r52 = 0;
  if (!samus_max_reserve_health) return;
  uint16 R44 = samus_max_reserve_health / 100;
  int mod_value = samus_reserve_health % 100;
  uint16 R50 = mod_value;
  int r42 = samus_reserve_health / 100;
  uint16 R48 = r42;
  int r46 = r42;
  if (r42) {
    uint16 v0 = 0;
    do {
      DrawMenuSpritemap(0x1B, kEquipmentScreenReserveTank_X[v0 >> 1], kEquipmentScreenReserveTank_Y - 1, r3);
      v0 += 2;
    } while (--r46);
    r52 = v0;
  }
  uint16 RegWord = mod_value;
  if (RegWord) {
    uint16 v2 = 2 * (RegWord / 14);
    if (sign16(v2 - 7) && RegWord % 14 && (nmi_frame_counter_byte & 4) == 0) v2 += 2;
    if (!sign16(samus_reserve_health - 100)) v2 += 16;
    DrawMenuSpritemap(kPartialReserveTankSpritemapIds[v2 >> 1], kEquipmentScreenReserveTank_X[r52 >> 1], kEquipmentScreenReserveTank_Y - 1, r3);
    ++R48;
    r52 += 2;
  }
  while (sign16(R48 - R44)) {
    DrawMenuSpritemap(0x20, kEquipmentScreenReserveTank_X[r52 >> 1], kEquipmentScreenReserveTank_Y - 1, r3);
    r52 += 2;
    ++R48;
  }
  DrawMenuSpritemap(0x1F, kEquipmentScreenReserveTank_X[r52 >> 1], kEquipmentScreenReserveTank_Y - 1, r3);
  int div_val = R50 / 10;
  int mod_val = R50 % 10;
  ram3800.cinematic_bg_tilemap[394] = mod_val + 2052;
  ram3800.cinematic_bg_tilemap[393] = div_val + 2052;
  ram3800.cinematic_bg_tilemap[392] = r42 + 2052;
}

uint16 EquipmentScreenDisplayReserves_PaletteSetup(void) {  // 0x82B3F9
  if (samus_reserve_health) {
    bool v0 = (--pausemenu_reserve_tank_animation_timer & 0x8000) != 0;
    if (!pausemenu_reserve_tank_animation_timer || v0) {
      ++pausemenu_reserve_tank_animation_frame;
      uint16 v1 = kPauseReserveTankAnimationData[2 * pausemenu_reserve_tank_animation_frame];
      if (v1 == 255) {
        pausemenu_reserve_tank_animation_frame = 0;
        v1 = kPauseReserveTankAnimationData[0];
      }
      pausemenu_reserve_tank_animation_timer = v1;
    }
    return kPAuseSpritePaletteIndexValues[3];
  } else {
    return 1536;
  }
}

uint16 EquipmentScreenMoveToReserveTanks(void) {  // 0x82B43F
  if (samus_max_reserve_health) {
    pausemenu_equipment_category_item = 0;
    QueueSfx1_Max6(0x37);
    return 1;
  }
  return 0;
}

void EquipmentScreenMoveToBeams(uint16 v0, uint16 r18) {  // 0x82B456
  if (hyper_beam_flag) {
    pausemenu_equipment_category_item = r18;
  } else {
    while ((kEquipmentBitmasks_Weapons[v0 >> 1] & collected_beams) == 0) {
      v0 += 2;
      if ((int16)(v0 - 10) >= 0) {
        pausemenu_equipment_category_item = r18;
        return;
      }
    }
    pausemenu_equipment_category_item = ((v0 >> 1) << 8) | 1;
    QueueSfx1_Max6(0x37);
  }
}

uint16 EquipmentScreenMoveToBottomOfBeams(uint16 k) {  // 0x82B489
  if (hyper_beam_flag) return -1;
  while ((kEquipmentBitmasks_Weapons[k >> 1] & collected_beams) == 0) {
    k -= 2;
    if ((k & 0x8000) != 0) return -1;
  }
  QueueSfx1_Max6(0x37);
  uint16 result = (k >> 1) << 8 | 1;
  pausemenu_equipment_category_item = result;
  return result;
}

uint16 EquipmentScreenMoveLowerOnSuitsMisc(uint16 v0) {  // 0x82B4B7
  while ((kEquipmentBitmasks_Suits[v0 >> 1] & collected_items) == 0) {
    v0 += 2;
    if ((int16)(v0 - 10) >= 0) return -1;
  }
  QueueSfx1_Max6(0x37);
  pausemenu_equipment_category_item = ((v0 >> 1) << 8) | 2;
  return 0;
}

void EquipmentScreenMoveToScrewOrHigherOnSuits(uint16 v0, uint16 r18) {  // 0x82B4E6
  while ((kEquipmentBitmasks_Suits[v0 >> 1] & collected_items) == 0) {
    v0 -= 2;
    if ((v0 & 0x8000) != 0) {
      pausemenu_equipment_category_item = r18;
      return;
    }
  }
  QueueSfx1_Max6(0x37);
  pausemenu_equipment_category_item = (v0 >> 1) << 8 | 2;
}

void EquipmentScreenMoveToHighJumpOrLowerInBoots(uint16 v0, uint16 r18) {  // 0x82B511
  while ((kEquipmentBitmasks_Boots[v0 >> 1] & collected_items) == 0) {
    v0 += 2;
    if ((int16)(v0 - 6) >= 0) {
      pausemenu_equipment_category_item = r18;
      return;
    }
  }
  QueueSfx1_Max6(0x37);
  pausemenu_equipment_category_item = (v0 >> 1) << 8 | 3;
}

uint16 EquipmentScreenCategory_Boots_MoveUpInBoots(uint16 k) {  // 0x82B53F
  while ((kEquipmentBitmasks_Boots[k >> 1] & collected_items) == 0) {
    k -= 2;
    if ((k & 0x8000) != 0) return -1;
  }
  QueueSfx1_Max6(0x37);
  uint16 result = (k >> 1) << 8 | 3;
  pausemenu_equipment_category_item = result;
  return result;
}

void EquipmentScreenCategory_ButtonResponse(uint16 r24) {  // 0x82B568
  if ((joypad1_newkeys & kButton_A) != 0) {
    QueueSfx1_Max6(0x38);
    int item = HIBYTE(pausemenu_equipment_category_item);
    int category = (uint8)pausemenu_equipment_category_item;
    uint8 *target = g_ram + *(uint16 *)RomPtr_82(kEquipmentPtrsToRamTilemapOffsets[category] + item * 2);
    uint16 *var = (uint16 *)RomPtr_RAM(kEquipmentPtrsToBitsets[category]);
    uint16 mask = *(uint16 *)RomPtr_82(kEquipmentPtrsToBitmasks[category] + item * 2);
    if ((*var & mask) != 0) {
      *var &= ~mask;
      ChangePaletteValues((uint16*)target, 0xc00, r24 >> 1);
    } else {
      *var |= mask;
      uint16 src = *(uint16 *)RomPtr_82(kEquipmentPtrsToEquipmentTilemaps[category] + item * 2);
      memcpy(target, RomPtr_82(src), r24);
    }
  }
}
