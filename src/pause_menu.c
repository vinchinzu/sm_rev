#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "menu_assets.h"

static Func_V *const kPauseMenuFuncs[8] = {
  PauseMenu_0_MapScreen,
  PauseMenu_1_EquipmentScreen,
  PauseMenu_2,
  PauseMenu_3_MapToEquipment_Load,
  PauseMenu_4_MapToEquipment_FadeIn,
  PauseMenu_5,
  PauseMenu_6_EquipmentToMap_Load,
  PauseMenu_7_EquipmentToMap_FadeIn,
};

void CallPauseHook(uint32 ea);
void LoadPauseMenuTilesAndClearBG2(void);
void LoadPauseScreenBaseTilemaps(void);
void BackupGameplayPalettesAndLoadForPause(void);
void ContinueInitPauseMenu(void);
void LoadEqupmentScreenReserveHealthTilemap(void);

void BackupSomeGfxStateForPause(void) {  // 0x828DBD
  pausemenu_bak_BG1SC = reg_BG1SC;
  pausemenu_bak_BG2SC = reg_BG2SC;
  pausemenu_bak_BG3SC = reg_BG3SC;
  pausemenu_bak_BG12NBA = reg_BG12NBA;
  pausemenu_bak_BG34NBA = reg_BG34NBA;
  pausemenu_bak_OBSEL = reg_OBSEL;
  pausemenu_bak_BG1HOFS = reg_BG1HOFS;
  pausemenu_bak_BG2HOFS = reg_BG2HOFS;
  pausemenu_bak_BG3HOFS = reg_BG3HOFS;
  pausemenu_bak_BG1VOFS = reg_BG1VOFS;
  pausemenu_bak_BG2VOFS = reg_BG2VOFS;
  pausemenu_bak_BG3VOFS = reg_BG3VOFS;
  pausemenu_bak_BGMODE = reg_BGMODE;
  pausemenu_bak_layer2_scroll_x = layer2_scroll_x;
  pausemenu_bak_layer2_scroll_y = layer2_scroll_y;
  pausemenu_bak_MOSAIC = reg_MOSAIC;
  pausemenu_bak_CGADSUB = next_gameplay_CGADSUB;
}

void RestoreSomeGfxStateForPause(void) {  // 0x828E19
  layer2_scroll_x = pausemenu_bak_layer2_scroll_x;
  layer2_scroll_y = pausemenu_bak_layer2_scroll_y;
  reg_BGMODE = pausemenu_bak_BGMODE;
  LOBYTE(reg_BG3VOFS) = pausemenu_bak_BG3VOFS;
  LOBYTE(reg_BG2VOFS) = pausemenu_bak_BG2VOFS;
  LOBYTE(reg_BG1VOFS) = pausemenu_bak_BG1VOFS;
  LOBYTE(reg_BG3HOFS) = pausemenu_bak_BG3HOFS;
  LOBYTE(reg_BG2HOFS) = pausemenu_bak_BG2HOFS;
  LOBYTE(reg_BG1HOFS) = pausemenu_bak_BG1HOFS;
  reg_OBSEL = pausemenu_bak_OBSEL;
  reg_BG34NBA = pausemenu_bak_BG34NBA;
  reg_BG12NBA = pausemenu_bak_BG12NBA;
  reg_BG3SC = pausemenu_bak_BG3SC;
  reg_BG2SC = pausemenu_bak_BG2SC;
  reg_BG1SC = pausemenu_bak_BG1SC;
  reg_MOSAIC = pausemenu_bak_MOSAIC;
  next_gameplay_CGADSUB = pausemenu_bak_CGADSUB;
}

void BackupBG2TilemapForPauseMenu(void) {  // 0x828D51
  WriteRegWord(VMADDL, (reg_BG2SC & 0xFC) << 8 | 1);
  WriteReg(VMAIN, 0x80);
  WriteReg(DMAP1, 0x81);
  WriteReg(BBAD1, 0x39);
  WriteRegWord(A1T1L, 0xDF5C);
  WriteReg(A1B1, 0x7E);
  WriteRegWord(DAS1L, 0x1000);
  WriteReg(DAS10, 0);
  WriteRegWord(A2A1L, 0);
  WriteReg(NTRL1, 0);
  WriteReg(MDMAEN, 2);
}

void RestoreBG2TilemapFromPauseScreen(void) {  // 0x828D96
  WriteRegWord(VMADDL, (reg_BG2SC & 0xFC) << 8);
  WriteReg(VMAIN, 0x80);
  static const StartDmaCopy unk_828DAE = { 1, 1, 0x18, LONGPTR(0x7edf5c), 0x1000 };
  SetupDmaTransfer(&unk_828DAE);
  WriteReg(MDMAEN, 2);
}

CoroutineRet GameState_12_Pausing_Darkening_Async(void) {  // 0x828CCF
  COROUTINE_AWAIT_ONLY(GameState_8_MainGameplay());
  HandleFadeOut();
  if ((reg_INIDISP & 0xF) == 0) {
    EnableNMI();
    screen_fade_delay = 0;
    screen_fade_counter = 0;
    ++game_state;
  }
  return kCoroutineNone;
}

CoroutineRet GameState_13_Pausing_Async(void) {  // 0x828CEF
  DisableHdmaObjects();
  reg_HDMAEN = 0;
  WriteReg(HDMAEN, 0);
  DisableAnimtiles();
  BackupBG2TilemapForPauseMenu();
  CallPauseHook(Load24(&pause_hook));
  CancelSoundEffects();
  BackupSomeGfxStateForPause();
  LoadPauseMenuTilesAndClearBG2();
  LoadPauseScreenBaseTilemaps();
  LoadPauseMenuMapTilemapAndAreaLabel();
  BackupGameplayPalettesAndLoadForPause();
  ContinueInitPauseMenu();
  screen_fade_delay = 1;
  screen_fade_counter = 1;
  pausemenu_leftarrow_animation_frame = 0;
  pausemenu_palette_animation_timer = 1;
  map_scrolling_direction = 0;
  map_scrolling_speed_index = 0;
  QueueClearingOfFxTilemap();
  ++game_state;
  return kCoroutineNone;
}

void CallPauseHook(uint32 ea) {
  switch (ea) {
  case fnPauseHook_Empty: return;
  case fnPauseHook_DraygonRoom: PauseHook_DraygonRoom(); return;
  case fnPauseHook_Kraid: PauseHook_Kraid(); return;
  default: Unreachable();
  }
}

void LoadPauseMenuTilesAndClearBG2(void) {  // 0x828E75
  WriteReg(VMADDL, 0);
  WriteReg(VMADDH, 0);
  WriteReg(VMAIN, 0x80);
  static const StartDmaCopy unk_828E8B = { 1, 1, 0x18, LONGPTR(0xb68000), 0x4000 };
  SetupDmaTransfer(&unk_828E8B);
  WriteReg(MDMAEN, 2);
  WriteReg(VMADDL, 0);
  WriteReg(VMADDH, 0x20);
  WriteReg(VMAIN, 0x80);
  static const StartDmaCopy unk_828EAB = { 1, 1, 0x18, LONGPTR(0xb6c000), 0x2000 };
  SetupDmaTransfer(&unk_828EAB);
  WriteReg(MDMAEN, 2);
  WriteReg(VMADDL, 0);
  WriteReg(VMADDH, 0x40);
  WriteReg(VMAIN, 0x80);
  static const StartDmaCopy unk_828ECB = { 1, 1, 0x18, LONGPTR(0x9ab200), 0x2000 };
  SetupDmaTransfer(&unk_828ECB);
  WriteReg(MDMAEN, 2);
}

void LoadPauseScreenBaseTilemaps(void) {  // 0x828EDA
  WriteReg(VMADDL, 0);
  WriteReg(VMADDH, 0x38);
  WriteReg(VMAIN, 0x80);
  static const StartDmaCopy unk_828EF0 = { 1, 1, 0x18, LONGPTR(0xb6e000), 0x0800 };
  SetupDmaTransfer(&unk_828EF0);
  WriteReg(MDMAEN, 2);
  WriteReg(WMADDL, 0);
  WriteReg(WMADDM, 0x34);
  WriteReg(WMADDH, 0x7E);
  static const StartDmaCopy unk_828F10 = { 1, 0, 0x80, LONGPTR(0xb6e400), 0x0400 };
  SetupDmaTransfer(&unk_828F10);
  WriteReg(MDMAEN, 2);
  WriteReg(WMADDL, 0);
  WriteReg(WMADDM, 0x38);
  WriteReg(WMADDH, 0x7E);
  static const StartDmaCopy unk_828F30 = { 1, 0, 0x80, LONGPTR(0xb6e800), 0x0800 };
  SetupDmaTransfer(&unk_828F30);
  WriteReg(MDMAEN, 2);
  uint16 v0 = addr_kDummySamusWireframeTilemap;
  uint16 v1 = 236;
  int m = 17;
  do {
    int n = 8;
    do {
      ram3000.pause_menu_map_tilemap[v1++] = *(uint16 *)RomPtr_82(v0);
      v0 += 2;
    } while (--n);
    v1 += 8;
  } while (--m);
  WriteSamusWireframeTilemapAndQueue();
  LoadEqupmentScreenReserveHealthTilemap();
}

void LoadEqupmentScreenReserveHealthTilemap(void) {  // 0x828F70
  if (samus_max_reserve_health) {
    int r42 = samus_reserve_health / 100;
    int rest = samus_reserve_health % 100;
    int div10 = rest / 10;
    int mod10 = rest % 10;

    ram3800.cinematic_bg_tilemap[394] = mod10 + 2052;
    ram3800.cinematic_bg_tilemap[393] = div10 + 2052;
    ram3800.cinematic_bg_tilemap[392] = r42 + 2052;
  }
}

void BackupGameplayPalettesAndLoadForPause(void) {  // 0x828FD4
  int16 v0;
  int16 v2;

  v0 = 512;
  uint16 v1 = 0;
  do {
    ram3000.pause_menu_map_tilemap[v1 + 384] = palette_buffer[v1];
    ++v1;
    v0 -= 2;
  } while (v0);
  v2 = 512;
  uint16 v3 = 0;
  do {
    palette_buffer[v3 >> 1] = kPauseScreenPalettes[v3 >> 1];
    v3 += 2;
    v2 -= 2;
  } while (v2);
}

void ContinueInitPauseMenu(void) {  // 0x829009
  SetupPpuForPauseMenu();
  ResetPauseMenuAnimations();
  LoadEquipmentScreenEquipmentTilemaps();
  SetPauseScreenButtonLabelPalettes_0();
  UpdatePauseMenuLRStartVramTilemap();
  DetermineMapScrollLimits();
  SetupMapScrollingForPauseMenu(0x80);
}

CoroutineRet GameState_14_Paused_Async(void) {  // 0x8290C8
  DrawPauseMenuDuringFadeIn();
  HandleFadeIn();
  if (reg_INIDISP == 15) {
    screen_fade_delay = 0;
    screen_fade_counter = 0;
    ++game_state;
  }
  return kCoroutineNone;
}

CoroutineRet GameState_15_Paused_Async(void) {  // 0x8290E8
  ReleaseButtonsFilter(3);
  MainPauseRoutine();
  HandleHudTilemap();
  HandlePauseScreenPaletteAnimation();
  return kCoroutineNone;
}

void MainPauseRoutine(void) {
  kPauseMenuFuncs[menu_index]();
}

void PauseMenu_2(void) {  // 0x829156
  DisplayMapElevatorDestinations();
  MapScreenDrawSamusPositionIndicator();
  DrawMapIcons();
  HandlePauseMenuLRPressHighlight();
  pause_screen_mode = 0;
  HandleFadeOut();
  if (reg_INIDISP == 0x80) {
    EnableNMI();
    screen_fade_delay = 0;
    screen_fade_counter = 0;
    ++menu_index;
  }
}

void PauseMenu_5(void) {  // 0x829186
  EquipmentScreenDrawItemSelector();
  EquipmentScreenDisplayReserveTankAmount();
  HandlePauseMenuLRPressHighlight();
  HandleFadeOut();
  if (reg_INIDISP == 0x80) {
    EnableNMI();
    screen_fade_delay = 0;
    screen_fade_counter = 0;
    ++menu_index;
  }
}

void PauseMenu_3_MapToEquipment_Load(void) {  // 0x8291AB
  DisplayMapElevatorDestinations();
  EquipmentScreenSetupReserveMode();
  EquipmentScreenTransferBG1Tilemap();
  pause_screen_mode = 1;
  SetPauseScreenButtonLabelPalettes();
  pausemenu_lr_animation_frame = 0;
  pausemenu_lr_animation_timer = *(uint16 *)kPauseLrHighlightAnimData;
  screen_fade_delay = 1;
  screen_fade_counter = 1;
  ++menu_index;
}

void PauseMenu_6_EquipmentToMap_Load(void) {  // 0x8291D7
  DisplayMapElevatorDestinations();
  LoadPauseMenuMapTilemapAndAreaLabel();
  SetPauseScreenButtonLabelPalettes();
  pausemenu_lr_animation_frame = 0;
  pausemenu_lr_animation_timer = *(uint16 *)kPauseLrHighlightAnimData;
  screen_fade_delay = 1;
  screen_fade_counter = 1;
  pause_screen_mode = 0;
  ++menu_index;
}

void PauseMenu_7_EquipmentToMap_FadeIn(void) {  // 0x829200
  MapScreenDrawSamusPositionIndicator();
  DrawMapIcons();
  DisplayMapElevatorDestinations();
  pause_screen_mode = 0;
  HandleFadeIn();
  if (reg_INIDISP == 15) {
    screen_fade_delay = 0;
    screen_fade_counter = 0;
    uint16 v0 = pausemenu_button_label_mode;
    if (pausemenu_button_label_mode)
      v0 = 1;
    menu_index = v0;
  }
}

void PauseMenu_4_MapToEquipment_FadeIn(void) {  // 0x829231
  EquipmentScreenDrawItemSelector();
  EquipmentScreenDisplayReserveTankAmount();
  pause_screen_mode = 1;
  HandleFadeIn();
  if (reg_INIDISP == 15) {
    screen_fade_delay = 0;
    screen_fade_counter = 0;
    uint16 v0 = pausemenu_button_label_mode;
    if (pausemenu_button_label_mode)
      v0 = 1;
    menu_index = v0;
  }
}

CoroutineRet GameState_16_Unpausing_Async(void) {  // 0x829324
  HighlightPauseScreenButton();
  DrawPauseMenuDuringFadeout();
  HandleFadeOut();
  if (reg_INIDISP == 0x80) {
    EnableNMI();
    screen_fade_delay = 0;
    screen_fade_counter = 0;
    ++game_state;
  }
  return kCoroutineNone;
}

void DrawPauseMenuDuringFadeout(void) {  // 0x82934B
  if (pause_screen_mode == 1) {
    EquipmentScreenDrawItemSelector();
    EquipmentScreenDisplayReserveTankAmount();
    HandlePauseMenuLRPressHighlight();
  } else {
    DisplayMapElevatorDestinations();
    DrawMapIcons();
    MapScreenDrawSamusPositionIndicator();
  }
}

CoroutineRet GameState_17_Unpausing_Async(void) {  // 0x829367
  COROUTINE_BEGIN(coroutine_state_1, 0); 
  ClearSamusBeamTiles();
  ContinueInitGameplayResume();
  ResumeGameplay();
  RestoreSomeGfxStateForPause();
  RestoreBG2TilemapFromPauseScreen();
  screen_fade_delay = 1;
  screen_fade_counter = 1;
  COROUTINE_AWAIT(1, CallUnpauseHook_Async(Load24(&unpause_hook)));
  EnableHdmaObjects();
  EnableAnimtiles();
  QueueSamusMovementSfx();
  ++game_state;
  COROUTINE_END(0);
}

CoroutineRet CallUnpauseHook_Async(uint32 ea) {
  switch (ea) {
  case fnPauseHook_Empty: return kCoroutineNone;
  case fnUnpauseHook_DraygonRoom: return UnpauseHook_DraygonRoom();
  case fnUnpauseHook_Kraid_IsDead: return UnpauseHook_Kraid_IsDead();
  case fnUnpauseHook_Kraid_IsAlive: return UnpauseHook_Kraid_IsAlive();
  case fnKraid_UnpauseHook_IsSinking: return Kraid_UnpauseHook_IsSinking();
  case fnMotherBrainsBody_UnpauseHook: return MotherBrainsBody_UnpauseHook();
  default: Unreachable(); return kCoroutineNone;
  }
}

CoroutineRet GameState_18_Unpausing(void) {  // 0x8293A1
  HandleFadeIn();
  if (reg_INIDISP == 15) {
    screen_fade_delay = 0;
    screen_fade_counter = 0;
    game_state = kGameState_8_MainGameplay;
  }
  return kCoroutineNone;
}

void ClearSamusBeamTiles(void) {  // 0x82A2BE
  WriteReg(VMADDL, 0);
  WriteReg(VMADDH, 0x60);
  WriteReg(VMAIN, 0x80);
  static const StartDmaCopy unk_82A2D4 = { 1, 1, 0x18, LONGPTR(0x9ad200), 0x1000 };
  SetupDmaTransfer(&unk_82A2D4);
  WriteReg(MDMAEN, 2);
}

void ContinueInitGameplayResume(void) {  // 0x82A2E3
  int16 v0;

  v0 = 512;
  uint16 v1 = 0;
  do {
    palette_buffer[v1] = ram3000.pause_menu_map_tilemap[v1 + 384];
    ++v1;
    v0 -= 2;
  } while (v0);
  SetupPpuForGameplayResume();
  CalculateBgScrolls_Unpause();
  UpdateBeamTilesAndPalette_Unpause();
  ClearPauseMenuData();
  CallSomeSamusCode(0xC);
}

void SetupPpuForGameplayResume(void) {  // 0x82A313
  WriteReg(OBSEL, 3);
  reg_OBSEL = 3;
  WriteReg(BGMODE, 9);
  reg_BGMODE = 9;
  reg_BG12NBA = 0;
  WriteReg(BG12NBA, 0);
  reg_BG34NBA = 4;
  WriteReg(BG34NBA, 4);
  reg_BG1SC = 81;
  WriteReg(BG1SC, 0x51);
  reg_BG2SC = 73;
  WriteReg(BG2SC, 0x49);
  reg_BG3SC = 90;
  WriteReg(BG3SC, 0x5A);
  reg_BG4SC = 0;
  WriteReg(BG4SC, 0);
}

void CalculateBgScrolls_Unpause(void) {  // 0x82A34E
  reg_BG1HOFS = bg1_x_offset + layer1_x_pos;
  reg_BG1VOFS = bg1_y_offset + layer1_y_pos;
  reg_BG2HOFS = bg2_x_scroll + layer2_x_pos;
  reg_BG2VOFS = bg2_y_scroll + layer2_y_pos;
}

void UpdateBeamTilesAndPalette_Unpause(void) {  // 0x82A377
  UpdateBeamTilesAndPalette();
}

void ClearPauseMenuData(void) {  // 0x82A380
  menu_index = 0;
  pausemenu_start_lr_pressed_highlight_timer = 0;
  pausemenu_lr_animation_timer = 0;
  pausemenu_item_selector_animation_timer = 0;
  pausemenu_reserve_tank_animation_timer = 0;
  UNUSED_word_7E0731 = 0;
  pausemenu_uparrow_animation_timer = 0;
  pausemenu_downarrow_animation_timer = 0;
  pausemenu_rightarrow_animation_timer = 0;
  pausemenu_leftarrow_animation_timer = 0;
  UNUSED_word_7E073D = 0;
  pausemenu_lr_animation_frame = 0;
  pausemenu_item_selector_animation_frame = 0;
  pausemenu_reserve_tank_animation_frame = 0;
  UNUSED_word_7E0745 = 0;
  pausemenu_uparrow_animation_frame = 0;
  pausemenu_downarrow_animation_frame = 0;
  pausemenu_rightarrow_animation_frame = 0;
  pausemenu_leftarrow_animation_frame = 0;
  pausemenu_shoulder_button_highlight = 0;
  pausemenu_button_label_mode = 0;
  pausemenu_equipment_category_item = 0;
  pausemenu_reserve_tank_delay_ctr = 0;
  UNUSED_word_7E0759 = 0;
  UNUSED_word_7E075B = 0;
  UNUSED_word_7E075D = 0;
  UNUSED_word_7E075F = 0;
  UNUSED_word_7E0761 = 0;
}

void HandlePauseScreenLR(void) {  // 0x82A505
  HandlePauseScreenLrInput();
  DrawLrHighlight();
}

void HandlePauseScreenLrInput(void) {  // 0x82A50C
  if ((newly_held_down_timed_held_input & kButton_L) != 0) {
    if (!pausemenu_button_label_mode)
      return;
    pausemenu_start_lr_pressed_highlight_timer = 5;
    menu_index = 5;
    pausemenu_shoulder_button_highlight = 1;
    pausemenu_button_label_mode = 0;
    SetPauseScreenButtonLabelPalettes();
    goto LABEL_7;
  }
  if ((newly_held_down_timed_held_input & kButton_R) != 0 && pausemenu_button_label_mode != 2) {
    pausemenu_start_lr_pressed_highlight_timer = 5;
    menu_index = 2;
    pausemenu_shoulder_button_highlight = 2;
    pausemenu_button_label_mode = 2;
    SetPauseScreenButtonLabelPalettes();
LABEL_7:
    QueueSfx1_Max6(0x38);
  }
}

static const uint16 kPauseLrButtonPressedHighlight_Spritemap[2] = { 0x28, 0x29 };
static const uint16 kPauseLrButtonPressedHighlight_Y[2] = { 0x18, 0xe8 };
static const uint16 kPauseLrButtonPressedHighlight_X[2] = { 0xd0, 0xd0 };

void HandlePauseMenuLRPressHighlight(void) {  // 0x82A56D
  if (pausemenu_start_lr_pressed_highlight_timer) {
    --pausemenu_start_lr_pressed_highlight_timer;
    if (pausemenu_shoulder_button_highlight) {
      int v0 = pausemenu_shoulder_button_highlight - 1;
      DrawMenuSpritemap(
        kPauseLrButtonPressedHighlight_Spritemap[v0],
        kPauseLrButtonPressedHighlight_Y[v0],
        kPauseLrButtonPressedHighlight_X[v0] - 1, 0);
    }
  }
}

void DrawLrHighlight(void) {  // 0x82A59A
  DrawPauseScreenSpriteAnim(2, 0x18, 0xD0);
  DrawPauseScreenSpriteAnim(2, 0xE8, 0xD0);
}

void HandlePauseScreenStart(void) {  // 0x82A5B7
  if ((newly_held_down_timed_held_input & kButton_Start) != 0) {
    QueueSfx1_Max6(0x38);
    screen_fade_delay = 1;
    screen_fade_counter = 1;
    uint16 v0 = pausemenu_button_label_mode;
    pausemenu_button_label_mode = 1;
    SetPauseScreenButtonLabelPalettes();
    pausemenu_button_label_mode = v0;
    pausemenu_start_lr_pressed_highlight_timer = 11;
    ++game_state;
  }
  UpdatePauseMenuLRStartVramTilemap();
}

void HighlightPauseScreenButton(void) {  // 0x82A5F1
  if (pausemenu_start_lr_pressed_highlight_timer) {
    --pausemenu_start_lr_pressed_highlight_timer;
    DrawMenuSpritemap(0x2B, 0x90, 0xD0, 0);
  }
}

static Func_V *const kSetPauseScreenButtonLabelPalettes_Funcs[3] = {  // 0x82A615
  SetPauseScreenButtonLabelPalettes_0,
  SetPauseScreenButtonLabelPalettes_1,
  SetPauseScreenButtonLabelPalettes_2,
};

void SetPauseScreenButtonLabelPalettes(void) {
  kSetPauseScreenButtonLabelPalettes_Funcs[pausemenu_button_label_mode]();
}

void SetPauseScreenButtonLabelPalettes_2(void) {  // 0x82A628
  int16 v0 = 10;
  uint16 v1 = 0;
  do {
    ram3000.pause_menu_map_tilemap[v1 + 805] = ram3000.pause_menu_map_tilemap[v1 + 805] & 0xE3FF | 0x800;
    ++v1;
    v0 -= 2;
  } while (v0);
  int16 v2 = 10;
  uint16 v3 = 0;
  do {
    ram3000.pause_menu_map_tilemap[v3 + 837] = ram3000.pause_menu_map_tilemap[v3 + 837] & 0xE3FF | 0x800;
    ++v3;
    v2 -= 2;
  } while (v2);
  int16 v4 = 8;
  uint16 v5 = 0;
  do {
    ram3000.pause_menu_map_tilemap[v5 + 812] = ram3000.pause_menu_map_tilemap[v5 + 812] & 0xE3FF | 0x800;
    ++v5;
    v4 -= 2;
  } while (v4);
  int16 v6 = 8;
  uint16 v7 = 0;
  do {
    ram3000.pause_menu_map_tilemap[v7 + 844] = ram3000.pause_menu_map_tilemap[v7 + 844] & 0xE3FF | 0x800;
    ++v7;
    v6 -= 2;
  } while (v6);
  int16 v8 = 10;
  uint16 v9 = 0;
  do {
    ram3000.pause_menu_map_tilemap[v9 + 822] = ram3000.pause_menu_map_tilemap[v9 + 822] & 0xE3FF | 0x1400;
    ++v9;
    v8 -= 2;
  } while (v8);
  int16 v10 = 10;
  uint16 v11 = 0;
  do {
    ram3000.pause_menu_map_tilemap[v11 + 854] = ram3000.pause_menu_map_tilemap[v11 + 854] & 0xE3FF | 0x1400;
    ++v11;
    v10 -= 2;
  } while (v10);
}

void SetPauseScreenButtonLabelPalettes_1(void) {  // 0x82A6DF
  int16 v0 = 8;
  uint16 v1 = 0;
  do {
    ram3000.pause_menu_map_tilemap[v1 + 812] = ram3000.pause_menu_map_tilemap[v1 + 812] & 0xE3FF | 0x800;
    ++v1;
    v0 -= 2;
  } while (v0);
  int16 v2 = 8;
  uint16 v3 = 0;
  do {
    ram3000.pause_menu_map_tilemap[v3 + 844] = ram3000.pause_menu_map_tilemap[v3 + 844] & 0xE3FF | 0x800;
    ++v3;
    v2 -= 2;
  } while (v2);
  int16 v4 = 10;
  uint16 v5 = 0;
  do {
    ram3000.pause_menu_map_tilemap[v5 + 805] = ram3000.pause_menu_map_tilemap[v5 + 805] & 0xE3FF | 0x1400;
    ++v5;
    v4 -= 2;
  } while (v4);
  int16 v6 = 10;
  uint16 v7 = 0;
  do {
    ram3000.pause_menu_map_tilemap[v7 + 837] = ram3000.pause_menu_map_tilemap[v7 + 837] & 0xE3FF | 0x1400;
    ++v7;
    v6 -= 2;
  } while (v6);
  int16 v8 = 10;
  uint16 v9 = 0;
  do {
    ram3000.pause_menu_map_tilemap[v9 + 822] = ram3000.pause_menu_map_tilemap[v9 + 822] & 0xE3FF | 0x1400;
    ++v9;
    v8 -= 2;
  } while (v8);
  int16 v10 = 10;
  uint16 v11 = 0;
  do {
    ram3000.pause_menu_map_tilemap[v11 + 854] = ram3000.pause_menu_map_tilemap[v11 + 854] & 0xE3FF | 0x1400;
    ++v11;
    v10 -= 2;
  } while (v10);
}

void SetPauseScreenButtonLabelPalettes_0(void) {  // 0x82A796
  int16 v0 = 10;
  uint16 v1 = 0;
  do {
    ram3000.pause_menu_map_tilemap[v1 + 822] = ram3000.pause_menu_map_tilemap[v1 + 822] & 0xE3FF | 0x800;
    ++v1;
    v0 -= 2;
  } while (v0);
  int16 v2 = 10;
  uint16 v3 = 0;
  do {
    ram3000.pause_menu_map_tilemap[v3 + 854] = ram3000.pause_menu_map_tilemap[v3 + 854] & 0xE3FF | 0x800;
    ++v3;
    v2 -= 2;
  } while (v2);
  int16 v4 = 8;
  uint16 v5 = 0;
  do {
    ram3000.pause_menu_map_tilemap[v5 + 812] = ram3000.pause_menu_map_tilemap[v5 + 812] & 0xE3FF | 0x800;
    ++v5;
    v4 -= 2;
  } while (v4);
  int16 v6 = 8;
  uint16 v7 = 0;
  do {
    ram3000.pause_menu_map_tilemap[v7 + 844] = ram3000.pause_menu_map_tilemap[v7 + 844] & 0xE3FF | 0x800;
    ++v7;
    v6 -= 2;
  } while (v6);
  int16 v8 = 10;
  uint16 v9 = 0;
  do {
    ram3000.pause_menu_map_tilemap[v9 + 805] = ram3000.pause_menu_map_tilemap[v9 + 805] & 0xE3FF | 0x1400;
    ++v9;
    v8 -= 2;
  } while (v8);
  int16 v10 = 10;
  uint16 v11 = 0;
  do {
    ram3000.pause_menu_map_tilemap[v11 + 837] = ram3000.pause_menu_map_tilemap[v11 + 837] & 0xE3FF | 0x1400;
    ++v11;
    v10 -= 2;
  } while (v10);
}

void UpdatePauseMenuLRStartVramTilemap(void) {  // 0x82A84D
  uint16 v0 = vram_write_queue_tail;
  gVramWriteEntry(vram_write_queue_tail)->size = 128;
  v0 += 2;
  gVramWriteEntry(v0)->size = 0x3640;
  v0 += 2;
  LOBYTE(gVramWriteEntry(v0++)->size) = 126;
  gVramWriteEntry(v0)->size = ((reg_BG2SC & 0xFC) << 8) + 800;
  vram_write_queue_tail = v0 + 2;
}

void DrawPauseScreenSpriteAnim(uint16 a, uint16 input_k, uint16 input_j) {  // 0x82A881
  int t = a - 1;
  uint16 *v8 = (uint16*)RomPtr_RAM(kPauseScreenSpriteAnimationData_1.arr[t]);
  uint16 *v5 = (uint16*)RomPtr_RAM(kPauseScreenSpriteAnimationData_0.arr[t]);
  const uint8 *v3 = RomPtr_82(kPauseScreenSpriteAnimationData_3.arr[t]);
  if ((int16)--(*v5) <= 0) {
    uint16 v10 = v3[3 * ++(*v8)];
    if (v10 == 255) {
      *v8 = 0;
      v10 = v3[0];
    }
    *v5 = v10;
  }
  uint16 r3 = kPAuseSpritePaletteIndexValues[3];
  int r24 = v3[3 * *v8 + 2];
  int r26 = 2 * *RomPtr_RAM(kPauseScreenSpriteAnimationData_2.arr[t]);
  const uint8 *v11 = RomPtr_82(r26 + kPausePtsToAnimationSpritemapBaseIds[t]);
  DrawMenuSpritemap(r24 + GET_WORD(v11), input_k, input_j - 1, r3);
}

void HandlePauseScreenPaletteAnimation(void) {  // 0x82A92B
  uint16 v2;
  if ((uint8)pausemenu_palette_animation_timer) {
    LOBYTE(pausemenu_palette_animation_timer) = pausemenu_palette_animation_timer - 1;
    if (!(uint8)pausemenu_palette_animation_timer) {
      for (int i = pausemenu_palette_animation_frame + 1; ; i = 0) {
        LOBYTE(pausemenu_palette_animation_frame) = i;
        uint8 v1 = ((uint8*)kPauseLrHighlightAnimData)[3 * i];
        if (v1 != 0xFF) {
          LOBYTE(pausemenu_palette_animation_timer) = v1;
          break;
        }
        QueueSfx3_Max6(0x2A);
      }
      HIBYTE(v2) = pausemenu_palette_animation_frame;
      LOBYTE(v2) = 0;
      for (int j = 0; j < 8; ++j)
        palette_buffer[j + 152] = kPauseAnimatedPalette[(v2 >> 1) + j];
    }
  }
}

void DrawPauseMenuDuringFadeIn(void) {  // 0x82B62B
  if (menu_index) {
    EquipmentScreenDrawItemSelector();
    EquipmentScreenDisplayReserveTankAmount();
  } else {
    MapScreenDrawSamusPositionIndicator();
    DrawMapIcons();
    DisplayMapElevatorDestinations();
  }
}

void UNKNOWN_sub_82B650(void) {  // 0x82B650
  if (pausemenu_button_label_mode) {
    DisplayMapElevatorDestinations();
    MapScreenDrawSamusPositionIndicator();
    DrawMapIcons();
  } else {
    EquipmentScreenDrawItemSelector();
    EquipmentScreenDisplayReserveTankAmount();
  }
}
