#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "menu_assets.h"
#include "variables_extra.h"

static Func_V *const kGameOptionsMenuFuncs[13] = {  // 0x82EB9F
  GameOptionsMenuFunc_0,
  GameOptionsMenu_1_LoadingOptionsScreen,
  GameOptionsMenu_2_FadeInOptionsScreen,
  GameOptionsMenu_3_OptionsScreen,
  GameOptionsMenu_4_StartGame,
  GameOptionsMenu_5_DissolveOutScreen,
  GameOptionsMenu_6_DissolveInScreen,
  GameOptionsMenu_7_ControllerSettings,
  GameOptionsMenu_8_SpecialSettings,
  GameOptionsMenu_9_ScrollControllerSettingsDown,
  GameOptionsMenu_A_ScrollControllerSettingsUp,
  GameOptionsMenu_B_TransitionBackToFileSelect,
  GameOptionsMenu_C_FadeOutOptionsScreenToStart,
};

static Func_V *const kGameOptionsMenuItemFuncs[5] = {  // 0x82ED42
  GameOptionsMenuItemFunc_0,
  GameOptionsMenuItemFunc_2_ToggleJapanese,
  GameOptionsMenuItemFunc_2_ToggleJapanese,
  GameOptionsMenuItemFunc_4,
  GameOptionsMenuItemFunc_4,
};

static Func_V *const kGameOptionsMenuSpecialSettings[3] = {  // 0x82F024
  GameOptionsMenuSpecialSettings_0,
  GameOptionsMenuSpecialSettings_0,
  GameOptionsMenuSpecialSettings_2,
};

static const Buttons kOptionsMenuButtonMasks[9] = {
  0x0080, 0x0040, 0x0020, 0x0010, 0x0100, 0x0200, 0x0400, 0x0800, 0x1000,
};
static const uint16 kOptionsMenuPairLeftOffsets[4] = { 0x0001, 0x0002, 0x0004, 0x0008 };
static const uint16 kOptionsMenuPairRightOffsets[4] = { 0x0010, 0x0020, 0x0040, 0x0080 };
static const uint16 kDebugInvincibilityButtonMasks[16] = {
  0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
  0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000, 0x8000,
};
static const uint16 kOptionsMenuDigitGlyphs[6] = { 0, 1, 2, 3, 4, 5 };
static const uint16 kMenuSelectionMissileEnableMasks[4] = { 0x0080, 0x0100, 0x0200, 0x0400 };
static const uint16 kMenuSelectionMissileSpritemaps[4] = { 0x0001, 0x0002, 0x0003, 0x0004 };



CoroutineRet GameState_2_GameOptionsMenu(void) {
  kGameOptionsMenuFuncs[game_options_screen_index]();
  OptionsMenuFunc1();
  DrawOptionsMenuSpritemaps();
  if (!sign16(game_options_screen_index - 2))
    OptionsMenu_AddToVramQueue();
  return kCoroutineNone;
}

void GameOptionsMenuFunc_0(void) {  // 0x82EBDB
  HandleFadeOut();
  if (reg_INIDISP == 0x80) {
    EnableNMI();
    screen_fade_delay = 0;
    screen_fade_counter = 0;
    ++game_options_screen_index;
  } else if (reg_INIDISP == 14) {
    uint16 v0 = reg_TS;
    if ((reg_TS & 4) == 0)
      CreateOptionsMenuObject_(v0, addr_stru_82F4D6);
  }
}

void GameOptionsMenu_1_LoadingOptionsScreen(void) {  // 0x82EC11
  uint16 j;
  reg_BG12NBA = 0;
  reg_TM = 19;
  reg_TS = 0;
  reg_TMW = 0;
  reg_TSW = 0;
  next_gameplay_CGWSEL = 0;
  next_gameplay_CGADSUB = 0;
  reg_CGWSEL = 0;
  reg_CGADSUB = 0;
  WriteReg(VMADDL, 0);
  WriteReg(VMADDH, 0x58);
  WriteReg(VMAIN, 0x80);
  static const StartDmaCopy unk_82EC3D = { 1, 1, 0x18, LONGPTR(0x8edc00), 0x0800 };
  SetupDmaTransfer(&unk_82EC3D);
  WriteReg(MDMAEN, 2);
  reg_BG1HOFS = 0;
  reg_BG1VOFS = 0;
  reg_BG2HOFS = 0;
  reg_BG2VOFS = 0;
  debug_invincibility = 0;
  for (int i = 510; i >= 0; i -= 2)
    palette_buffer[i >> 1] = kMenuPalettes[i >> 1];
  DecompressToMem(0x978DF4, intro_gfx_buf_1c000);
  DecompressToMem(0x978FCD, intro_gfx_buf_1c800);
  DecompressToMem(0x9791C4, intro_gfx_buf_1d000);
  DecompressToMem(0x97938D, intro_gfx_buf_1d800);
  DecompressToMem(0x97953A, intro_gfx_buf_1e000);
  for (j = 1023; (j & 0x8000) == 0; --j)
    ram3000.pause_menu_map_tilemap[j] = custom_background[j + 5375];
  menu_option_index = 0;
  DeleteAllOptionsMenuObjects_();
  CreateOptionsMenuObject_(0, addr_stru_82F4B8);
  CreateOptionsMenuObject_(0, addr_stru_82F4C4);
  ++game_options_screen_index;
  OptionsMenuFunc4();
}

void GameOptionsMenu_2_FadeInOptionsScreen(void) {  // 0x82ECE4
  HandleFadeIn();
  if (reg_INIDISP == 15) {
    screen_fade_delay = 0;
    screen_fade_counter = 0;
    ++game_options_screen_index;
  }
}

void OptionsMenu_AddToVramQueue(void) {  // 0x82ECFF
  uint16 v0 = vram_write_queue_tail;
  gVramWriteEntry(vram_write_queue_tail)->size = 2048;
  v0 += 2;
  gVramWriteEntry(v0)->size = 12288;
  v0 += 2;
  LOBYTE(gVramWriteEntry(v0++)->size) = 126;
  gVramWriteEntry(v0)->size = 20480;
  vram_write_queue_tail = v0 + 2;
}

void OptionsMenuFunc5(uint16 a, uint16 k, uint16 j) {  // 0x82ED28
  do {
    *(uint16 *)((uint8 *)ram3000.pause_menu_map_tilemap + k) = a | *(uint16 *)((uint8 *)ram3000.pause_menu_map_tilemap + k) & 0xE3FF;
    k += 2;
    j -= 2;
  } while (j);
}

void GameOptionsMenu_3_OptionsScreen(void) {
  if ((joypad1_newkeys & kButton_Up) != 0) {
    QueueSfx1_Max6(0x37);
    if ((--menu_option_index & 0x8000) != 0)
      menu_option_index = 4;
  } else if ((joypad1_newkeys & kButton_Down) != 0) {
    QueueSfx1_Max6(0x37);
    if (++menu_option_index == 5)
      menu_option_index = 0;
  }
  if ((joypad1_newkeys & 0x8000) != 0) {
    game_options_screen_index = 11;
  } else if ((joypad1_newkeys & kButton_A) != 0 || (joypad1_newkeys & kButton_Start) != 0) {
    QueueSfx1_Max6(0x38);
    kGameOptionsMenuItemFuncs[menu_option_index]();
  }
}

void GameOptionsMenuItemFunc_0(void) {  // 0x82EDB1
  if (enable_debug && (joypad1_lastkeys & kButton_L) == 0 || loading_game_state == kGameState_5_FileSelectMap) {
    game_options_screen_index = 4;
  } else {
    screen_fade_delay = 0;
    screen_fade_counter = 0;
    game_options_screen_index = 12;
  }
}

void GameOptionsMenuItemFunc_2_ToggleJapanese(void) {  // 0x82EDDA
  menu_option_index = 0;
  japanese_text_flag = japanese_text_flag == 0;
  OptionsMenuFunc4();
}

void OptionsMenuFunc4(void) {  // 0x82EDED
  if (japanese_text_flag) {
    OptionsMenuFunc5(0x400, 0x288, 0x18);
    OptionsMenuFunc5(0x400, 0x2C8, 0x18);
    OptionsMenuFunc5(0, 0x348, 0x32);
    OptionsMenuFunc5(0, 0x388, 0x32);
  } else {
    OptionsMenuFunc5(0, 0x288, 0x18);
    OptionsMenuFunc5(0, 0x2C8, 0x18);
    OptionsMenuFunc5(0x400, 0x348, 0x32);
    OptionsMenuFunc5(0x400, 0x388, 0x32);
  }
}

void GameOptionsMenuItemFunc_4(void) {  // 0x82EE55
  reg_MOSAIC = 3;
  screen_fade_delay = 0;
  screen_fade_counter = 0;
  game_options_screen_index = 5;
}

void GameOptionsMenu_B_TransitionBackToFileSelect(void) {  // 0x82EE6A
  HandleFadeOut();
  if (reg_INIDISP == 0x80) {
    EnableNMI();
    screen_fade_delay = 0;
    screen_fade_counter = 0;
    game_state = kGameState_4_FileSelectMenus;
    menu_index = 0;
    game_options_screen_index = 0;
  }
}

void GameOptionsMenu_C_FadeOutOptionsScreenToStart(void) {  // 0x82EE92
  HandleFadeOut();
  if (reg_INIDISP == 0x80) {
    EnableNMI();
    screen_fade_delay = 0;
    screen_fade_counter = 0;
    game_options_screen_index = 4;
  }
}

void GameOptionsMenu_4_StartGame(void) {  // 0x82EEB4
  game_options_screen_index = 0;
  if (enable_debug && (joypad1_lastkeys & 0x20) == 0) {
    game_state = kGameState_5_FileSelectMap;
    if (loading_game_state != kLoadingGameState_5_Main) {
      loading_game_state = kGameState_5_FileSelectMap;
      SaveToSram(selected_save_slot);
    }
  } else if (loading_game_state) {
    game_state = loading_game_state;
    if (loading_game_state == kGameState_34_CeresGoesBoom)
      cinematic_function = FUNC16(CinematicFunctionBlackoutFromCeres);
    menu_option_index = 0;
    game_options_screen_index = 0;
  } else {
    game_state = kGameState_30_IntroCinematic;
    cinematic_function = FUNC16(CinematicFunction_Intro_Initial);
    menu_option_index = 0;
    screen_fade_delay = 0;
    screen_fade_counter = 0;
  }
}

void GameOptionsMenu_5_DissolveOutScreen(void) {  // 0x82EF18
  uint16 m, k, j, n, v1;
  HandleFadeOut();
  if (reg_MOSAIC != 0xF3)
    reg_MOSAIC += 16;
  if (reg_INIDISP == 0x80) {
    EnableNMI();
    screen_fade_delay = 0;
    screen_fade_counter = 0;
    reg_BG1VOFS = 0;
    ++game_options_screen_index;
    if (menu_option_index) {
      if ((menu_option_index & 4) != 0) {
        if (japanese_text_flag) {
          for (int i = 1023; i >= 0; --i)
            ram3000.pause_menu_map_tilemap[i] = custom_background[i + 9471];
        } else {
          for (j = 1023; (j & 0x8000) == 0; --j)
            ram3000.pause_menu_map_tilemap[j] = custom_background[j + 8447];
        }
        menu_option_index = 0;
        OptionsMenuFunc7();
        menu_option_index = 1;
        OptionsMenuFunc7();
        menu_option_index = 4;
        CreateOptionsMenuObject_(4, addr_stru_82F4D0);
      } else {
        if (japanese_text_flag) {
          for (k = 1023; (k & 0x8000) == 0; --k) {
            v1 = custom_background[k + 7423];
            ram3000.pause_menu_map_tilemap[k] = v1;
          }
        } else {
          for (m = 1023; (m & 0x8000) == 0; --m) {
            v1 = custom_background[m + 6399];
            ram3000.pause_menu_map_tilemap[m] = v1;
          }
        }
        CreateOptionsMenuObject_(v1, addr_stru_82F4CA);
        LoadControllerOptionsFromControllerBindings();
        OptionsMenuFunc6();
      }
    } else {
      for (n = 1023; (n & 0x8000) == 0; --n)
        ram3000.pause_menu_map_tilemap[n] = custom_background[n + 5375];
      OptionsMenuFunc4();
      CreateOptionsMenuObject_(0, addr_stru_82F4C4);
    }
  }
}

void GameOptionsMenu_6_DissolveInScreen(void) {  // 0x82EFDB
  HandleFadeIn();
  if (reg_MOSAIC != 3)
    reg_MOSAIC -= 16;
  if (reg_INIDISP == 15) {
    reg_MOSAIC = 0;
    screen_fade_delay = 0;
    screen_fade_counter = 0;
    if (menu_option_index) {
      if ((menu_option_index & 4) != 0)
        game_options_screen_index = 8;
      else
        game_options_screen_index = 7;
      menu_option_index = 0;
    } else {
      game_options_screen_index = 3;
    }
  }
}

void GameOptionsMenu_8_SpecialSettings(void) {
  if ((joypad1_newkeys & kButton_Up) != 0) {
    QueueSfx1_Max6(0x37);
    if ((--menu_option_index & 0x8000) != 0)
      menu_option_index = 2;
  } else if ((joypad1_newkeys & kButton_Down) != 0) {
    QueueSfx1_Max6(0x37);
    if (++menu_option_index == 3)
      menu_option_index = 0;
  }
  if ((joypad1_newkeys & 0x8000) == 0) {
    if ((joypad1_newkeys & (kButton_Start | kButton_Left | kButton_Right | kButton_A)) != 0) {
      QueueSfx1_Max6(0x38);
      kGameOptionsMenuSpecialSettings[menu_option_index]();
    }
  } else {
    QueueSfx1_Max6(0x38);
    menu_option_index = 0;
    GameOptionsMenuItemFunc_4();
  }
}

void GameOptionsMenuSpecialSettings_0(void) {  // 0x82F08E
  uint8 *v0 = RomPtr_RAM(kOptionsMenuSpecialPtrs[menu_option_index]);
  if (GET_WORD(v0))
    *(uint16 *)v0 = 0;
  else
    *(uint16 *)v0 = 1;
  OptionsMenuFunc7();
}

void GameOptionsMenuSpecialSettings_2(void) {  // 0x82F0B2
  menu_option_index = 0;
  GameOptionsMenuItemFunc_4();
}

void OptionsMenuFunc7(void) {
  uint16 v0 = 4 * menu_option_index;
  if (*(uint16 *)RomPtr_RAM(kOptionsMenuSpecialPtrs[menu_option_index])) {
    OptionsMenuFunc5(0, kOptionsMenuPairLeftOffsets[(uint16)(4 * menu_option_index) >> 1], 0xC);
    OptionsMenuFunc5(0, kOptionsMenuPairLeftOffsets[(v0 >> 1) + 1], 0xC);
    OptionsMenuFunc5(0x400, kOptionsMenuPairRightOffsets[v0 >> 1], 0xC);
    OptionsMenuFunc5(0x400, kOptionsMenuPairRightOffsets[(v0 >> 1) + 1], 0xC);
  } else {
    OptionsMenuFunc5(0x400, kOptionsMenuPairLeftOffsets[(uint16)(4 * menu_option_index) >> 1], 0xC);
    OptionsMenuFunc5(0x400, kOptionsMenuPairLeftOffsets[(v0 >> 1) + 1], 0xC);
    OptionsMenuFunc5(0, kOptionsMenuPairRightOffsets[v0 >> 1], 0xC);
    OptionsMenuFunc5(0, kOptionsMenuPairRightOffsets[(v0 >> 1) + 1], 0xC);
  }
}

void GameOptionsMenu_7_ControllerSettings(void) {
  if ((joypad1_newkeys & kButton_Up) != 0) {
    QueueSfx1_Max6(0x37);
    uint16 v0 = --menu_option_index;
    if ((menu_option_index & 0x8000) == 0) {
      if (v0 != 6) return;
      game_options_screen_index = 10;
      return;
    }
    menu_option_index = 8;
    game_options_screen_index = 9;
    return;
  }
  if ((joypad1_newkeys & kButton_Down) != 0) {
    QueueSfx1_Max6(0x37);
    uint16 v1 = menu_option_index + 1;
    menu_option_index = v1;
    if (v1 == 7) {
      game_options_screen_index = 9;
      return;
    }
    if (v1 == 9) {
      menu_option_index = 0;
      game_options_screen_index = 10;
      return;
    }
  } else if (joypad1_newkeys) {
    QueueSfx1_Max6(0x38);
    static Func_V *const kOptionsMenuControllerFuncs[9] = {
      OptionsMenuControllerFunc_0, OptionsMenuControllerFunc_0, OptionsMenuControllerFunc_0,
      OptionsMenuControllerFunc_0, OptionsMenuControllerFunc_0, OptionsMenuControllerFunc_0,
      OptionsMenuControllerFunc_0, OptionsMenuControllerFunc_7, OptionsMenuControllerFunc_8,
    };
    kOptionsMenuControllerFuncs[menu_option_index]();
  } else if (joypad2_new_keys && menu_option_index == 8 && sign16(debug_invincibility - 16)) {
    if ((kDebugInvincibilityButtonMasks[debug_invincibility] & joypad2_new_keys) == kDebugInvincibilityButtonMasks[debug_invincibility])
      ++debug_invincibility;
    else
      debug_invincibility = 0;
  }
}

void OptionsMenuControllerFunc_8(void) {  // 0x82F224
  if ((joypad1_newkeys & (kButton_Start | kButton_A)) != 0) {
    button_config_shoot_x = 64;
    button_config_jump_a = 128;
    button_config_run_b = 0x8000;
    button_config_itemcancel_y = 0x4000;
    button_config_itemswitch = 0x2000;
    button_config_aim_up_R = 16;
    button_config_aim_down_L = 32;
    LoadControllerOptionsFromControllerBindings();
    OptionsMenuFunc6();
  }
}

void OptionsMenuControllerFunc_7(void) {  // 0x82F25D
  if ((joypad1_newkeys & (kButton_Start | kButton_A)) != 0 && !OptionsMenuFunc8()) {
    menu_option_index = 0;
    GameOptionsMenuItemFunc_4();
  }
}

void GameOptionsMenu_9_ScrollControllerSettingsDown(void) {  // 0x82F271
  reg_BG1VOFS += 2;
  if (reg_BG1VOFS == 32)
    game_options_screen_index = 7;
}

void GameOptionsMenu_A_ScrollControllerSettingsUp(void) {  // 0x82F285
  reg_BG1VOFS -= 2;
  if (!reg_BG1VOFS)
    game_options_screen_index = 7;
}

void sub_82F296(uint16 j) {  // 0x82F296
  int v1 = j >> 1;
  eproj_y_pos[v1 + 13] = 24;
  eproj_x_vel[v1 + 3] = 56;
  eproj_x_vel[v1 + 11] = 3584;
}

void OptionsPreInstr_F2A9(uint16 v0) {  // 0x82F2A9
  if (game_state == kGameState_2_GameOptionsMenu) {
    int v2 = game_options_screen_index;
    uint16 v3 = off_82F2ED[v2];
    if (v3) {
      const uint16 *v4 = (const uint16 *)RomPtr_82(v3 + 4 * menu_option_index);
      int v5 = v0 >> 1;
      eproj_y_pos[v5 + 13] = *v4;
      eproj_x_vel[v5 + 3] = v4[1];
    } else {
      int v6 = v0 >> 1;
      eproj_y_pos[v6 + 13] = 384;
      eproj_x_vel[v6 + 3] = 16;
    }
  } else {
    int v1 = v0 >> 1;
    eproj_E[v1 + 15] = 1;
    eproj_y_vel[v1 + 17] = addr_off_82F4B6;
  }
}

void sub_82F34B(uint16 j) {  // 0x82F34B
  eproj_y_pos[(j >> 1) + 13] = 124;
  sub_82F369(j);
}
void sub_82F353(uint16 j) {  // 0x82F353
  eproj_y_pos[(j >> 1) + 13] = 132;
  sub_82F369(j);
}
void sub_82F35B(uint16 j) {  // 0x82F35B
  eproj_y_pos[(j >> 1) + 13] = 128;
  sub_82F369(j);
}
void sub_82F363(uint16 j) {  // 0x82F363
  eproj_y_pos[(j >> 1) + 13] = 128;
  sub_82F369(j);
}

void sub_82F369(uint16 j) {  // 0x82F369
  int v1 = j >> 1;
  eproj_x_vel[v1 + 3] = 16;
  eproj_x_vel[v1 + 11] = 3584;
}

void OptionsPreInstr_F376(uint16 k) {  // 0x82F376
  if (game_state != kGameState_2_GameOptionsMenu || (game_options_screen_index == 6 && reg_INIDISP == 0x80)) {
    int v1 = k >> 1;
    eproj_E[v1 + 15] = 1;
    eproj_y_vel[v1 + 17] = addr_off_82F4B6;
  }
}

void OptionsPreInstr_F3A0(uint16 k) {  // 0x82F3A0
  switch (game_options_screen_index) {
  case 6:
    if (reg_INIDISP == 0x80) {
      int v1 = k >> 1;
      eproj_E[v1 + 15] = 1;
      eproj_y_vel[v1 + 17] = -2890;
    }
    break;
  case 9: eproj_x_vel[(k >> 1) + 3] -= 2; break;
  case 0xA: eproj_x_vel[(k >> 1) + 3] += 2; break;
  }
}

void OptionsPreInstr_F3E2(uint16 k) {  // 0x82F3E2
  if (game_options_screen_index == 6 && reg_INIDISP == 0x80) {
    int v1 = k >> 1;
    eproj_E[v1 + 15] = 1;
    eproj_y_vel[v1 + 17] = -2890;
  }
}

void sub_82F404(uint16 k) {  // 0x82F404
  if (game_options_screen_index == 1) {
    int v1 = k >> 1;
    eproj_E[v1 + 15] = 1;
    eproj_y_vel[v1 + 17] = -2890;
  }
}

void sub_82F419(uint16 j) {  // 0x82F419
  int v1 = j >> 1;
  eproj_y_pos[v1 + 13] = 216;
  eproj_x_vel[v1 + 3] = 16;
  eproj_x_vel[v1 + 11] = 3584;
}

void OptionsPreInstr_F42C(uint16 k) {  // 0x82F42C
  if (game_state != 2) {
    int v1 = k >> 1;
    eproj_E[v1 + 15] = 1;
    eproj_y_vel[v1 + 17] = addr_off_82F4B6;
  }
}

void LoadControllerOptionsFromControllerBindings(void) {  // 0x82F4DC
  uint16 v0 = 0;
  do {
    int v1 = v0 >> 1;
    uint16 v2 = *(uint16 *)&g_ram[off_82F54A[v1]];
    if ((v2 & kButton_X) != 0) {
LABEL_9:
      eproj_F[v1 + 13] = 0;
      goto LABEL_16;
    }
    if ((v2 & kButton_A) != 0) eproj_F[v1 + 13] = 1;
    else if (v2 & 0x8000) eproj_F[v1 + 13] = 2;
    else if ((v2 & kButton_Select) != 0) eproj_F[v1 + 13] = 3;
    else if ((v2 & kButton_Y) != 0) eproj_F[v1 + 13] = 4;
    else if ((v2 & kButton_L) != 0) eproj_F[v1 + 13] = 5;
    else {
      if ((v2 & kButton_R) == 0) goto LABEL_9;
      eproj_F[v1 + 13] = 6;
    }
LABEL_16:
    v0 += 2;
  } while ((int16)(v0 - 14) < 0);
}

uint8 OptionsMenuFunc8(void) {
  int v0 = 0, v2;
  do {
    v2 = v0;
    *(uint16 *)&g_ram[off_82F54A[v0 >> 1]] = kOptionsMenuButtonMasks[eproj_F[(v0 >> 1) + 13]];
    v0 += 2;
  } while (v2 < 12);
  return 0;
}

void OptionsMenuFunc6(void) {
  uint16 v0 = 0, v4;
  do {
    v4 = v0;
    int v1 = v0 >> 1;
    uint16 v2 = g_word_82F639[v1];
    const uint16 *v3 = (const uint16 *)RomPtr_82(g_off_82F647[eproj_F[v1 + 13]]);
    *(uint16 *)((uint8 *)ram3000.pause_menu_map_tilemap + v2) = *v3;
    *(uint16 *)((uint8 *)&ram3000.pause_menu_map_tilemap[1] + v2) = v3[1];
    *(uint16 *)((uint8 *)&ram3000.pause_menu_map_tilemap[2] + v2) = v3[2];
    *(uint16 *)((uint8 *)&ram3000.pause_menu_map_tilemap[32] + v2) = v3[3];
    *(uint16 *)((uint8 *)&ram3000.pause_menu_map_tilemap[33] + v2) = v3[4];
    *(uint16 *)((uint8 *)&ram3000.pause_menu_map_tilemap[34] + v2) = v3[5];
    v0 = v4 + 2;
  } while ((int16)(v4 - 12) < 0);
  if (eproj_instr_list_ptr[0] != 5 && eproj_instr_list_ptr[0] != 6) {
    *(uint32 *)&ram3000.menu.backup_of_io_registers_in_gameover[46] = *(uint32 *)kOptionsMenuDigitGlyphs;
    ram3000.pause_menu_map_tilemap[665] = kOptionsMenuDigitGlyphs[2];
    *(uint32 *)&ram3000.menu.field_536[56] = *(uint32 *)&kOptionsMenuDigitGlyphs[3];
    ram3000.pause_menu_map_tilemap[697] = kOptionsMenuDigitGlyphs[5];
  }
  if (eproj_instr_list_ptr[1] != 5 && eproj_instr_list_ptr[1] != 6) {
    *(uint32 *)&ram3000.menu.field_536[184] = *(uint32 *)kOptionsMenuDigitGlyphs;
    ram3000.pause_menu_map_tilemap[761] = kOptionsMenuDigitGlyphs[2];
    *(uint32 *)&ram3000.menu.menu_tilemap[46] = *(uint32 *)&kOptionsMenuDigitGlyphs[3];
    ram3000.pause_menu_map_tilemap[793] = kOptionsMenuDigitGlyphs[5];
  }
}

void OptionsMenuControllerFunc_0(void) {  // 0x82F6B9
  uint16 v0 = 12;
  while ((kOptionsMenuButtonMasks[v0 >> 1] & joypad1_newkeys) == 0) {
    v0 -= 2;
    if ((v0 & 0x8000) != 0) return;
  }
  uint16 r18 = v0 >> 1;
  uint16 v1 = 2 * menu_option_index + 2;
  if (!sign16(2 * menu_option_index - 12)) v1 = 0;
  uint16 v2 = v1;
  for (int i = 5; i >= 0; --i) {
    if (eproj_F[(v2 >> 1) + 13] == r18) break;
    v2 += 2;
    if ((int16)(v2 - 14) >= 0) v2 = 0;
  }
  int v4 = menu_option_index;
  uint16 r20 = eproj_F[v4 + 13];
  eproj_F[v4 + 13] = r18;
  eproj_F[(v2 >> 1) + 13] = r20;
  OptionsMenuFunc6();
}

void DrawBorderAroundSamusData(void) {  // 0x82BA35
  DrawMenuSpritemap(0x48, 0x80, 0x10, 3584);
}

void DrawBorderAroundDataCopyMode(void) {  // 0x82BA48
  DrawMenuSpritemap(0x49, 0x80, 0x10, 3584);
}

void DrawBorderAroundDataClearMode(void) {  // 0x82BA5B
  DrawMenuSpritemap(0x4A, 0x7C, 0x10, 3584);
}

void DrawMenuSelectionMissile(void) {  // 0x82BA6E
  if (eproj_enable_flag) {
    if (!--eproj_enable_flag) {
      eproj_id[0] = (LOBYTE(eproj_id[0]) + 1) & 3;
      eproj_enable_flag = kMenuSelectionMissileEnableMasks[eproj_id[0]];
    }
  }
  DrawMenuSpritemap(kMenuSelectionMissileSpritemaps[eproj_id[0]],
      eproj_id[5], eproj_id[10], 3584);
}

void DrawFileCopyArrow(void) {  // 0x82BABA
  uint16 v0;
  if ((int16)(eproj_id[16] - eproj_id[17]) >= 0) {
    v0 = 3;
    if (sign16(eproj_id[16] - eproj_id[17] - 2)) {
      v0 = 4;
      if (eproj_id[16] != 1)
        v0 = 5;
    }
  } else {
    v0 = 0;
    if (sign16(eproj_id[17] - eproj_id[16] - 2)) {
      v0 = 1;
      if (eproj_id[16])
        v0 = 2;
    }
  }
  DrawMenuSpritemap(file_copy_arrow_stuff[v0].spritemap, file_copy_arrow_stuff[v0].xpos, file_copy_arrow_stuff[v0].ypos, 512);
}


void DeleteAllOptionsMenuObjects_(void) {
  for (int i = 14; i >= 0; i -= 2) {
    int v1 = i >> 1;
    optionsmenu_instr_ptr[v1] = 0;
    optionsmenu_cur_data[v1] = 0;
  }
}

void CallOptionsEntryFunc(uint32 ea, uint16 j) {
  switch (ea) {
  case fnsub_82F296: sub_82F296(j); return;
  case fnsub_82F34B: sub_82F34B(j); return;
  case fnsub_82F353: sub_82F353(j); return;
  case fnsub_82F35B: sub_82F35B(j); return;
  case fnsub_82F363: sub_82F363(j); return;
  case fnsub_82F419: sub_82F419(j); return;
  default: Unreachable();
  }
}

uint8 CreateOptionsMenuObject_(uint16 a, uint16 j) {
  options_menu_init_param = a;
  uint16 v3 = 14;
  while (optionsmenu_instr_ptr[v3 >> 1]) {
    v3 -= 2;
    if ((v3 & 0x8000) != 0)
      return 1;
  }
  const uint8 *v5 = RomPtr_82(j);
  int v6 = v3 >> 1;
  optionsmenu_arr1[v6] = GET_WORD(v5 + 2);
  optionsmenu_instr_ptr[v6] = GET_WORD(v5 + 4);
  optionsmenu_instr_timer[v6] = 1;
  optionsmenu_cur_data[v6] = 0;
  optionsmenu_arr5[v6] = 0;
  optionsmenu_arr6[v6] = 0;
  optionsmenu_arr7[v6] = 0;
  CallOptionsEntryFunc(GET_WORD(v5) | 0x820000, v3);
  return 0;
}

void OptionsPreInstr_nullsub_57(uint16 k) {
}

void OptionsMenuFunc1(void) {
  for (int i = 14; i >= 0; i -= 2) {
    optionsmenu_index = i;
    if (optionsmenu_instr_ptr[i >> 1]) {
      OptionsMenuFunc2(i);
      i = optionsmenu_index;
    }
  }
}

void CallOptionsPreInstr(uint32 ea, uint16 k) {
  switch (ea) {
  case fnOptionsPreInstr_nullsub_57: OptionsPreInstr_nullsub_57(k); return;
  case fnOptionsPreInstr_F2A9: OptionsPreInstr_F2A9(k); return;
  case fnOptionsPreInstr_F376: OptionsPreInstr_F376(k); return;
  case fnOptionsPreInstr_F3A0: OptionsPreInstr_F3A0(k); return;
  case fnOptionsPreInstr_F3E2: OptionsPreInstr_F3E2(k); return;
  case fnOptionsPreInstr_F42C: OptionsPreInstr_F42C(k); return;
  default: Unreachable();
  }
}

uint16 CallOptionsInstr(uint32 ea, uint16 k, uint16 j) {
  switch (ea) {
  case fnOptionsInstr_Destroy: return OptionsInstr_Destroy(k, j);
  case fnOptionsInstr_8C64: return OptionsInstr_8C64(k, j);
  case fnOptionsInstr_SetPreInstr: return OptionsInstr_SetPreInstr(k, j);
  case fnOptionsInstr_8C79: return OptionsInstr_8C79(k, j);
  case fnOptionsInstr_Goto: return OptionsInstr_Goto(k, j);
  case fnOptionsInstr_8C89: return OptionsInstr_8C89(k, j);
  case fnOptionsInstr_8C93: return OptionsInstr_8C93(k, j);
  default: return Unreachable();
  }
}

void OptionsMenuFunc2(uint16 k) {
  CallOptionsPreInstr(optionsmenu_arr1[k >> 1] | 0x820000, k);
  uint16 v1 = optionsmenu_index;
  int v2 = optionsmenu_index >> 1;
  if (optionsmenu_instr_timer[v2]-- == 1) {
    uint16 v4 = optionsmenu_instr_ptr[v2];
    uint16 *v5;
    while (1) {
      v5 = (uint16 *)RomPtr_82(v4);
      if ((v5[0] & 0x8000) == 0)
        break;
      v4 = CallOptionsInstr(v5[0] | 0x820000, v1, v4 + 2);
      if (!v4)
        return;
    }
    int v7 = v1 >> 1;
    optionsmenu_instr_timer[v7] = v5[0];
    optionsmenu_cur_data[v7] = v5[1];
    optionsmenu_instr_ptr[v7] = v4 + 4;
  }
}

uint16 OptionsInstr_Destroy(uint16 k, uint16 j) {
  int v2 = k >> 1;
  optionsmenu_cur_data[v2] = 0;
  optionsmenu_instr_ptr[v2] = 0;
  return 0;
}

uint16 OptionsInstr_8C64(uint16 k, uint16 j) {
  optionsmenu_instr_ptr[k >> 1] = j - 2;
  return 0;
}

uint16 OptionsInstr_SetPreInstr(uint16 k, uint16 j) {
  optionsmenu_arr1[k >> 1] = *(uint16 *)RomPtr_82(j);
  return j + 2;
}

uint16 OptionsInstr_8C79(uint16 k, uint16 j) {
  optionsmenu_arr1[k >> 1] = FUNC16(OptionsPreInstr_nullsub_57);
  return j;
}

uint16 OptionsInstr_Goto(uint16 k, uint16 j) {
  return *(uint16 *)RomPtr_82(j);
}

uint16 OptionsInstr_8C89(uint16 k, uint16 j) {
  int v2 = k >> 1;
  if (optionsmenu_arr5[v2]-- == 1)
    return j + 2;
  else
    return OptionsInstr_Goto(k, j);
}

uint16 OptionsInstr_8C93(uint16 k, uint16 j) {
  optionsmenu_arr5[k >> 1] = *(uint16 *)RomPtr_82(j);
  return j + 2;
}

void DrawOptionsMenuSpritemaps(void) {
  for (int i = 14; i >= 0; i -= 2) {
    int v1 = i >> 1;
    if (optionsmenu_cur_data[v1]) {
      uint16 v2 = optionsmenu_cur_data[v1];
      DrawSpritemap(0x82, v2, optionsmenu_arr8[v1], optionsmenu_arr9[v1], optionsmenu_arr10[v1]);
    }
  }
}
