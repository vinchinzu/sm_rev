// Shared menu tilemap, palette, and VRAM helpers extracted from Bank 81.

#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "menu_common.h"
#include "enemy_types.h"

#define kMenuPalettes ((uint16*)RomFixedPtr(0x8ee400))

void MapVramForMenu(void) {  // 0x818DBA
  reg_TS = 0;
  reg_OBSEL = 3;
  reg_BG1SC = 81;
  reg_BG2SC = 88;
  reg_BG3SC = 92;
  reg_BG12NBA = 0;
  reg_BG34NBA = 4;
  reg_TM = 19;
}

void LoadInitialMenuTiles(void) {  // 0x818DDB
  WriteReg(VMADDL, 0);
  WriteReg(VMADDH, 0);
  WriteReg(VMAIN, 0x80);
  static const StartDmaCopy unk_818DF1 = { 1, 1, 0x18, LONGPTR(0x8e8000), 0x5600 };
  SetupDmaTransfer(&unk_818DF1);
  WriteReg(MDMAEN, 2);
  WriteReg(VMADDL, 0);
  WriteReg(VMADDH, 0x30);
  WriteReg(VMAIN, 0x80);
  static const StartDmaCopy unk_818E11 = { 1, 1, 0x18, LONGPTR(0xb68000), 0x2000 };
  SetupDmaTransfer(&unk_818E11);
  WriteReg(MDMAEN, 2);
  WriteReg(VMADDL, 0);
  WriteReg(VMADDH, 0x60);
  WriteReg(VMAIN, 0x80);
  static const StartDmaCopy unk_818E31 = { 1, 1, 0x18, LONGPTR(0xb6c000), 0x2000 };
  SetupDmaTransfer(&unk_818E31);
  WriteReg(MDMAEN, 2);
  WriteReg(VMADDL, 0);
  WriteReg(VMADDH, 0x40);
  WriteReg(VMAIN, 0x80);
  static const StartDmaCopy unk_818E51 = { 1, 1, 0x18, LONGPTR(0x8ed600), 0x0600 };
  SetupDmaTransfer(&unk_818E51);
  WriteReg(MDMAEN, 2);
}

void LoadMenuPalettes(void) {  // 0x818E60
  uint16 v0 = 0;
  do {
    int v1 = v0 >> 1;
    palette_buffer[v1] = kMenuPalettes[v1];
    palette_buffer[v1 + 1] = kMenuPalettes[v1 + 1];
    v0 += 4;
  } while ((int16)(v0 - 512) < 0);
}

void LoadFileSelectPalettes(void) {  // 0x819486
  int v0 = 0;
  do {
    int v1 = v0 >> 1;
    palette_buffer[v1] = kMenuPalettes[v1];
    palette_buffer[v1 + 1] = kMenuPalettes[v1 + 1];
    v0 += 4;
  } while ((int16)(v0 - 512) < 0);
}

void ClearMenuTilemap(void) {  // 0x8195A6
  for (int i = 2046; i >= 0; i -= 2)
    *(uint16 *)((uint8 *)&ram3000.pause_menu_map_tilemap[768] + (uint16)i) = 15;
}

void LoadMenuExitTilemap(void) {  // 0x8195B5
  LoadMenuTilemap(0x688, addr_kMenuTilemap_Exit);
}

void QueueTransferOfMenuTilemapToVramBG1(void) {  // 0x81969F
  uint16 v0 = vram_write_queue_tail;
  VramWriteEntry *v1 = gVramWriteEntry(vram_write_queue_tail);
  v1->size = 2048;
  v1->src.addr = ADDR16_OF_RAM(ram3000.menu.menu_tilemap);
  *(uint16 *)&v1->src.bank = 126;
  v1->vram_dst = (reg_BG1SC & 0xFC) << 8;
  vram_write_queue_tail = v0 + 7;
}

void LoadMenuTilemap(uint16 k, uint16 j) {  // 0x81B3E2
  int16 v2;
  int k_bak = k;

  while (1) {
    while (1) {
      v2 = *(uint16 *)RomPtr_81(j);
      if (v2 != -2)
        break;
      j += 2;
      k = (k_bak += 64);
    }
    if (v2 == -1)
      break;
    *(uint16 *)((uint8 *)&ram3000.pause_menu_map_tilemap[768] + k) = enemy_data[0].palette_index | v2;
    k += 2;
    j += 2;
  }
}
