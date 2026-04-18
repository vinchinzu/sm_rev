// HUD tilemap, item highlight, and digit rendering.
#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"

static const uint16 kHudTilemaps[32] = {  // 0x8099CF
  0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f,
  0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c1d, 0x2c1d, 0x2c1d, 0x2c1d, 0x2c1d, 0x2c1c,
};
static const uint16 kHudTilemaps_Row1to3[96] = {
  0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f,
  0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c12, 0x2c12, 0x2c23, 0x2c12, 0x2c12, 0x2c1e,
  0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f,
  0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2822, 0x2822, 0x2823, 0x2813, 0x2c14, 0x2c1e,
  0x2c0f, 0x2c0b, 0x2c0c, 0x2c0d, 0x2c32, 0x2c0f, 0x2c09, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f,
  0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c0f, 0x2c12, 0x2c12, 0xa824, 0x2815, 0x2c16, 0x2c1e,
};
static const uint16 kHudTilemaps_AutoReserve[12] = { 0x3c33, 0x3c46, 0x3c47, 0x3c48, 0xbc33, 0xbc46, 0x2c33, 0x2c46, 0x2c47, 0x2c48, 0xac33, 0xac46 };
static const uint16 kHudTilemaps_Missiles[22] = {
  0x344b, 0x3449, 0x744b, 0x344c, 0x344a, 0x744c, 0x3434, 0x7434, 0x3435, 0x7435, 0x3436, 0x7436, 0x3437, 0x7437, 0x3438, 0x7438,
  0x3439, 0x7439, 0x343a, 0x743a, 0x343b, 0x743b,
};

void AddMissilesToHudTilemap(void) {
  if ((hud_tilemap[10] & 0x3FF) == 15) {
    hud_tilemap[10] = kHudTilemaps_Missiles[0];
    hud_tilemap[11] = kHudTilemaps_Missiles[1];
    hud_tilemap[12] = kHudTilemaps_Missiles[2];
    hud_tilemap[42] = kHudTilemaps_Missiles[3];
    hud_tilemap[43] = kHudTilemaps_Missiles[4];
    hud_tilemap[44] = kHudTilemaps_Missiles[5];
  }
}

void AddSuperMissilesToHudTilemap(void) {  // 0x809A0E
  AddToTilemapInner(0x1C, (const uint16*)RomPtr_80(addr_kHudTilemaps_Missiles + 12));
}

void AddPowerBombsToHudTilemap(void) {  // 0x809A1E
  AddToTilemapInner(0x22, (const uint16 *)RomPtr_80(addr_kHudTilemaps_Missiles + 20));
}

void AddGrappleToHudTilemap(void) {  // 0x809A2E
  AddToTilemapInner(0x28, (const uint16 *)RomPtr_80(addr_kHudTilemaps_Missiles + 28));
}

void AddXrayToHudTilemap(void) {  // 0x809A3E
  AddToTilemapInner(0x2E, (const uint16 *)RomPtr_80(addr_kHudTilemaps_Missiles + 36));
}

void AddToTilemapInner(uint16 k, const uint16 *j) {  // 0x809A4C
  int v2 = k >> 1;
  if ((hud_tilemap[v2] & 0x3FF) == 15) {
    hud_tilemap[v2] = j[0];
    hud_tilemap[v2 + 1] = j[1];
    hud_tilemap[v2 + 32] = j[2];
    hud_tilemap[v2 + 33] = j[3];
  }
}

void InitializeHud(void) {  // 0x809A79
  WriteRegWord(VMADDL, addr_unk_605800);
  WriteRegWord(VMAIN, 0x80);
  static const StartDmaCopy unk_809A8F = { 1, 1, 0x18, LONGPTR(0x80988b), 0x0040 };
  SetupDmaTransfer(&unk_809A8F);
  WriteReg(MDMAEN, 2);
  for (int i = 0; i != 192; i += 2)
    hud_tilemap[i >> 1] = kHudTilemaps_Row1to3[i >> 1];
  if ((equipped_items & 0x8000) != 0)
    AddXrayToHudTilemap();
  if ((equipped_items & 0x4000) != 0)
    AddGrappleToHudTilemap();
  if (samus_max_missiles)
    AddMissilesToHudTilemap();
  if (samus_max_super_missiles)
    AddSuperMissilesToHudTilemap();
  if (samus_max_power_bombs)
    AddPowerBombsToHudTilemap();
  samus_prev_health = 0;
  samus_prev_missiles = 0;
  samus_prev_super_missiles = 0;
  samus_prev_power_bombs = 0;
  samus_prev_hud_item_index = 0;
  InitializeMiniMapBroken();
  if (samus_max_missiles)
    DrawThreeHudDigits(addrl_kDigitTilesetsWeapon, samus_missiles, 0x94);
  if (samus_max_super_missiles)
    DrawTwoHudDigits(addrl_kDigitTilesetsWeapon, samus_super_missiles, 0x9C);
  if (samus_max_power_bombs)
    DrawTwoHudDigits(addrl_kDigitTilesetsWeapon, samus_power_bombs, 0xA2);
  ToggleHudItemHighlight(hud_item_index, 0x1000);
  ToggleHudItemHighlight(samus_prev_hud_item_index, 0x1400);
  HandleHudTilemap();
}

static const uint16 kEnergyTankIconTilemapOffsets[14] = { 0x42, 0x44, 0x46, 0x48, 0x4a, 0x4c, 0x4e, 2, 4, 6, 8, 0xa, 0xc, 0xe };

void HandleHudTilemap(void) {  // 0x809B44
  if (reserve_health_mode == 1) {
    const uint16 *v1 = (const uint16 *)RomPtr_80(addr_kHudTilemaps_AutoReserve);
    if (!samus_reserve_health)
      v1 += 6;
    hud_tilemap[8] = v1[0];
    hud_tilemap[9] = v1[1];
    hud_tilemap[40] = v1[2];
    hud_tilemap[41] = v1[3];
    hud_tilemap[72] = v1[4];
    hud_tilemap[73] = v1[5];
  }
  if (samus_health != samus_prev_health) {
    samus_prev_health = samus_health;

    uint16 r20 = SnesDivide(samus_health, 100);
    uint16 r18 = SnesModulus(samus_health, 100);
    int v2 = 0;
    int n = SnesDivide(samus_max_health, 100) + 1;
    do {
      if (!--n)
        break;
      uint16 v3 = 13360;
      if (r20) {
        --r20;
        v3 = 10289;
      }
      hud_tilemap[kEnergyTankIconTilemapOffsets[v2 >> 1] >> 1] = v3;
      v2 += 2;
    } while ((int16)(v2 - 28) < 0);
    DrawTwoHudDigits(addrl_kDigitTilesetsHealth, r18, 0x8C);
  }
  if (samus_max_missiles && samus_missiles != samus_prev_missiles) {
    samus_prev_missiles = samus_missiles;
    DrawThreeHudDigits(addrl_kDigitTilesetsWeapon, samus_missiles, 0x94);
  }
  if (samus_max_super_missiles && samus_super_missiles != samus_prev_super_missiles) {
    samus_prev_super_missiles = samus_super_missiles;
    if ((joypad_dbg_flags & 0x1F40) != 0)
      DrawThreeHudDigits(addrl_kDigitTilesetsWeapon, samus_prev_super_missiles, 0x9C);
    else
      DrawTwoHudDigits(addrl_kDigitTilesetsWeapon, samus_prev_super_missiles, 0x9C);
  }
  if (samus_max_power_bombs && samus_power_bombs != samus_prev_power_bombs) {
    samus_prev_power_bombs = samus_power_bombs;
    DrawTwoHudDigits(addrl_kDigitTilesetsWeapon, samus_power_bombs, 0xA2);
  }
  if (hud_item_index != samus_prev_hud_item_index) {
    ToggleHudItemHighlight(hud_item_index, 0x1000);
    ToggleHudItemHighlight(samus_prev_hud_item_index, 0x1400);
    samus_prev_hud_item_index = hud_item_index;
    if (samus_movement_type != 3
        && samus_movement_type != 20
        && grapple_beam_function == 0xC4F0
        && !time_is_frozen_flag) {
      QueueSfx1_Max6(0x39);
    }
  }
  uint16 v4 = 5120;
  if ((nmi_frame_counter_byte & 0x10) != 0)
    v4 = 4096;
  ToggleHudItemHighlight(samus_auto_cancel_hud_item_index, v4);
  uint16 v5 = vram_write_queue_tail;
  gVramWriteEntry(vram_write_queue_tail)->size = 192;
  v5 += 2;
  gVramWriteEntry(v5)->size = ADDR16_OF_RAM(*hud_tilemap);
  v5 += 2;
  gVramWriteEntry(v5++)->size = 126;
  gVramWriteEntry(v5)->size = addr_unk_605820;
  vram_write_queue_tail = v5 + 2;
}

static const uint16 kHudItemTilemapOffsets[5] = { 0x14, 0x1c, 0x22, 0x28, 0x2e };

void ToggleHudItemHighlight(uint16 a, uint16 k) {  // 0x809CEA
  int16 v2;

  hud_item_tilemap_palette_bits = k;
  v2 = a - 1;
  if (v2 >= 0) {
    int v3 = kHudItemTilemapOffsets[v2] >> 1;
    if (hud_tilemap[v3] != 11279)
      hud_tilemap[v3] = hud_item_tilemap_palette_bits | hud_tilemap[v3] & 0xE3FF;
    if (hud_tilemap[v3 + 1] != 11279)
      hud_tilemap[v3 + 1] = hud_item_tilemap_palette_bits | hud_tilemap[v3 + 1] & 0xE3FF;
    if (hud_tilemap[v3 + 32] != 11279)
      hud_tilemap[v3 + 32] = hud_item_tilemap_palette_bits | hud_tilemap[v3 + 32] & 0xE3FF;
    if (hud_tilemap[v3 + 33] != 11279)
      hud_tilemap[v3 + 33] = hud_item_tilemap_palette_bits | hud_tilemap[v3 + 33] & 0xE3FF;
    if (!(2 * v2)) {
      if (hud_tilemap[v3 + 2] != 11279)
        hud_tilemap[v3 + 2] = hud_item_tilemap_palette_bits | hud_tilemap[v3 + 2] & 0xE3FF;
      if (hud_tilemap[v3 + 34] != 11279)
        hud_tilemap[v3 + 34] = hud_item_tilemap_palette_bits | hud_tilemap[v3 + 34] & 0xE3FF;
    }
  }
}

void DrawThreeHudDigits(uint32 addr, uint16 a, uint16 k) {  // 0x809D78
  hud_tilemap[k >> 1] = GET_WORD(RomPtr(addr + 2 * (a / 100)));
  DrawTwoHudDigits(addr, a % 100, k + 2);
}

void DrawTwoHudDigits(uint32 addr, uint16 a, uint16 k) {  // 0x809D98
  int v3 = k >> 1;
  hud_tilemap[v3] = GET_WORD(RomPtr(addr + 2 * (a / 10)));
  hud_tilemap[v3 + 1] = GET_WORD(RomPtr(addr + 2 * (a % 10)));
}
