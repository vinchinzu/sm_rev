// Game-over menu extracted from Bank 81.

#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "menu_common.h"
#include "enemy_types.h"

static void RestorePalettesAndIoAfterDebugGameover(void) {  // 0x81905B
  int v0 = 0;
  do {
    palette_buffer[v0] = ram3000.pause_menu_map_tilemap[v0 + 384];
    ++v0;
  } while ((int16)(v0 * 2 - 512) < 0);
  int v1 = 0;
  do {
    *(uint16 *)(&reg_INIDISP + v1 * 2) = ram3000.pause_menu_map_tilemap[v1 + 640];
    ++v1;
  } while ((int16)(v1 * 2 - 54) < 0);
}

void GameOverMenu_0_FadeOutConfigGfx(void) {  // 0x818D0F
  HandleFadeOut();
  if ((reg_INIDISP & 0xF) == 0) {
    ScreenOff();
    QueueSfx3_Max6(1);
    DisableHdmaObjects();
    WaitUntilEndOfVblankAndClearHdma();
    uint16 v0 = 0;
    do {
      ram3000.pause_menu_map_tilemap[v0 + 384] = palette_buffer[v0];
      ++v0;
    } while ((int16)(v0 * 2 - 512) < 0);
    uint16 v1 = 0;
    do {
      ram3000.pause_menu_map_tilemap[v1 + 640] = *(uint16 *)(&reg_INIDISP + v1 * 2);
      ++v1;
    } while ((int16)(v1 * 2 - 54) < 0);
    MapVramForMenu();
    LoadInitialMenuTiles();
    reg_BG1HOFS = 0;
    reg_BG2HOFS = 0;
    reg_BG3HOFS = 0;
    reg_BG1VOFS = 0;
    reg_BG2VOFS = 0;
    reg_BG3VOFS = 0;
    LoadMenuPalettes();
    ++menu_index;
  }
}

void GameOverMenu_24_FadeIn(void) {  // 0x818DA6
  HandleFadeIn();
  if ((reg_INIDISP & 0xF) == 15)
    ++menu_index;
}

void GameOverMenu_3_Main(void) {  // 0x819003
  int16 v0;
  int16 v1;

  if ((joypad1_newkeys & kButton_Select) != 0
      || (joypad1_newkeys & kButton_Up) != 0
      || (joypad1_newkeys & kButton_Down) != 0) {
    file_select_map_area_index ^= 1;
  } else if ((joypad1_newkeys & (uint16)(kButton_B | kButton_Start | kButton_A)) != 0) {
    if (file_select_map_area_index) {
      ++menu_index;
    } else {
      SaveToSram(selected_save_slot);
      SoftReset();
    }
    return;
  }
  v0 = 30720;
  if (file_select_map_area_index)
    v0 = -30720;
  v1 = v0 | 0x28;
  OamEnt *v3 = gOamEnt(oam_next_ptr);
  *(uint16 *)&v3->xcoord = v1;
  *(uint16 *)&v3->charnum = 182;
  oam_next_ptr += 4;
}

void GameOverMenu_5_Continue(void) {  // 0x81907E
  WriteReg(VMADDL, 0);
  WriteReg(VMADDH, 0x40);
  WriteReg(VMAIN, 0x80);
  static const StartDmaCopy unk_819093 = { 1, 1, 0x18, LONGPTR(0x9ab200), 0x2000 };
  SetupDmaTransfer(&unk_819093);
  WriteReg(MDMAEN, 2);
  game_state = 16;
  file_select_map_area_index = 0;
  RestorePalettesAndIoAfterDebugGameover();
}

void GameOverMenu_3_FadeIn(void) {  // 0x8190CD
  HandleGameOverBabyMetroid();
  DrawMenuSelectionMissile();
  HandleFadeIn();
  if ((reg_INIDISP & 0xF) == 15)
    ++menu_index;
}

void GameOverMenu_5_FadeOutToGameMap(void) {  // 0x8190E7
  HandleGameOverBabyMetroid();
  DrawMenuSelectionMissile();
  HandleFadeOut();
  if ((reg_INIDISP & 0xF) == 0)
    ++menu_index;
}

void GameOverMenu_7_FadeOutToSoftReset(void) {  // 0x8190FE
  DrawMenuSelectionMissile();
  HandleGameOverBabyMetroid();
  HandleFadeOut();
  if ((reg_INIDISP & 0xF) == 0)
    SoftReset();
}

void GameOverMenu_6_LoadGameMapView(void) {  // 0x819116
  DisableHdmaObjects();
  WaitUntilEndOfVblankAndClearHdma();
  game_state = kGameState_5_FileSelectMap;
  menu_index = 0;
}

void GameOverMenu_4_Main(void) {  // 0x81912B
  HandleGameOverBabyMetroid();
  DrawMenuSelectionMissile();
  if ((joypad1_newkeys & kButton_Select) != 0
      || (joypad1_newkeys & kButton_Up) != 0
      || (joypad1_newkeys & kButton_Down) != 0) {
    QueueSfx1_Max6(0x37);
    file_select_map_area_index ^= 1;
  } else if ((joypad1_newkeys & kButton_A) != 0) {
    enemy_data[0].instruction_timer = 180;
    if (file_select_map_area_index) {
      menu_index = 7;
    } else {
      if (loading_game_state == kGameState_31_SetUpNewGame)
        game_state = loading_game_state;
      else
        ++menu_index;
      LoadFromSram(selected_save_slot);
    }
    return;
  }
  uint16 v0 = 160;
  if (file_select_map_area_index)
    v0 = 192;
  eproj_id[5] = 40;
  eproj_id[10] = v0;
}

void GameOverMenu_1_Init(void) {  // 0x8191A4
  reg_TM = 17;
  reg_COLDATA[0] = 32;
  reg_COLDATA[1] = 64;
  reg_COLDATA[2] = 0x80;
  QueueMusic_Delayed8(0);
  QueueMusic_Delayed8(0xFF03);
  eproj_enable_flag = 1;
  eproj_id[0] = 0;
  int v0 = 0;
  do {
    ((uint16*)ram3000.menu.menu_tilemap)[v0] = 15;
    ++v0;
  } while ((int16)(v0 * 2 - 2048) < 0);
  int v1 = vram_write_queue_tail;
  VramWriteEntry *v2 = gVramWriteEntry(vram_write_queue_tail);
  v2->size = 2048;
  v2->src.addr = ADDR16_OF_RAM(ram3000.menu.menu_tilemap);
  *(uint16 *)&v2->src.bank = 126;
  v2->vram_dst = (reg_BG1SC & 0xFC) << 8;
  vram_write_queue_tail = v1 + 7;
  enemy_data[0].palette_index = 0;
  LoadMenuTilemap(0x156, addr_kMenuTilemap_GameOver);
  LoadMenuTilemap(0x38A, addr_kMenuTilemap_FindMetroidLarva);
  LoadMenuTilemap(0x414, addr_kMenuTilemap_TryAgain);
  LoadMenuTilemap(0x4CE, addr_kMenuTilemap_YesReturnToGame);
  LoadMenuTilemap(0x5CE, addr_kMenuTilemap_NoGoToTitle);
  enemy_data[0].current_instruction = 0;
  enemy_data[0].instruction_timer = 0;
  HandleGameOverBabyMetroid();
  *(uint16 *)&reg_CGWSEL = gameplay_CGWSEL << 8;
  DisableHdmaObjects();
  WaitUntilEndOfVblankAndClearHdma();
  EnableHdmaObjects();
  static const SpawnHdmaObject_Args unk_819254 = { 0x00, 0x32, 0x927d };
  static const SpawnHdmaObject_Args unk_81925C = { 0x00, 0x31, 0x928d };
  SpawnHdmaObject(0x81, &unk_819254);
  SpawnHdmaObject(0x81, &unk_81925C);
  ScreenOn();
  ++menu_index;
  screen_fade_delay = 0;
  screen_fade_counter = 0;
  file_select_map_area_index = 0;
  eproj_id[5] = 40;
  eproj_id[10] = 160;
}

void GameOverMenu_2_PlayMusic(void) {  // 0x8193E8
  if (!(HasQueuedMusic())) {
    ++menu_index;
    QueueMusic_Delayed8(4);
  }
}

static Func_V *const kGameOverMenuFuncs[8] = {  // 0x8190AE
  GameOverMenu_0_FadeOutConfigGfx,
  GameOverMenu_1_Init,
  GameOverMenu_2_PlayMusic,
  GameOverMenu_3_FadeIn,
  GameOverMenu_4_Main,
  GameOverMenu_5_FadeOutToGameMap,
  GameOverMenu_6_LoadGameMapView,
  GameOverMenu_7_FadeOutToSoftReset,
};

void GameOverMenu(void) {
  kGameOverMenuFuncs[menu_index]();
}
