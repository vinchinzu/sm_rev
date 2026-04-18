#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "multi_samus.h"

CoroutineRet GameState_0_Reset_Async(void) {  // 0x828AE4
  UNUSED_word_7E0DF8 = 0;
  UNUSED_word_7E0DFA = 0;
  UNUSED_word_7E0DFC = 0;
  cinematic_function = FUNC16(CinematicFunctionOpening);
  demo_set = 0;
  if (num_demo_sets == 4)
    demo_set = 3;
  ++game_state;
  return kCoroutineNone;
}

CoroutineRet GameState_1_OpeningCinematic(void) {  // 0x828B08
  return GameState_1_OpeningCinematic_();
}

CoroutineRet GameState_7_MainGameplayFadeIn(void) {  // 0x828B20
  COROUTINE_AWAIT_ONLY(GameState_8_MainGameplay());
  HandleFadeIn();
  if (reg_INIDISP == 15) {
    screen_fade_delay = 0;
    screen_fade_counter = 0;
    ++game_state;
  }
  return kCoroutineNone;
}

CoroutineRet GameState_32_MadeItToCeresElevator(void) {  // 0x828367
  if (timer_status)
    DrawTimer();
  COROUTINE_AWAIT_ONLY(GameState_8_MainGameplay());
  bool v0 = (--reached_ceres_elevator_fade_timer & 0x8000) != 0;
  if (!reached_ceres_elevator_fade_timer || v0) {
    ++game_state;
    screen_fade_delay = 0;
    screen_fade_counter = 0;
  }
  return kCoroutineNone;
}

CoroutineRet GameState_33_BlackoutFromCeres(void) {  // 0x828388
  if (timer_status)
    DrawTimer();
  COROUTINE_AWAIT_ONLY(GameState_8_MainGameplay());
  HandleFadeOut();
  if (reg_INIDISP == 0x80) {
    EnableNMI();
    WaitUntilEndOfVblankAndClearHdma();
    DisableIrqInterrupts();
    fx_layer_blending_config_a = 0;
    cur_irq_handler = 0;
    next_gameplay_CGWSEL = 0;
    next_gameplay_CGADSUB = 0;
    reg_TM = 16;
    reg_TS = 0;
    reg_TMW = 0;
    reg_TSW = 0;
    reg_BGMODE = 9;
    screen_fade_delay = 0;
    screen_fade_counter = 0;
    loading_game_state = kLoadingGameState_22_EscapingCeres;
    game_state = 34;
    SaveToSram(selected_save_slot);
    cinematic_function = FUNC16(CinematicFunctionBlackoutFromCeres);
    ceres_status = 0;
    timer_status = 0;
    QueueMusic_Delayed8(0);
    QueueSfx1_Max15(2);
    QueueSfx2_Max15(0x71);
    QueueSfx3_Max15(1);
  }
  return kCoroutineNone;
}

CoroutineRet GameState_35_TimeUp(void) {  // 0x828411
  COROUTINE_AWAIT_ONLY(GameState_8_MainGameplay());
  palette_change_denom = 8;
  if (AdvancePaletteFadeForAllPalettes()) {
    game_state = kGameState_36_WhitingOutFromTimeUp;
    screen_fade_delay = 0;
    screen_fade_counter = 0;
  }
  return kCoroutineNone;
}

CoroutineRet GameState_36_WhitingOutFromTimeUp(void) {  // 0x828431
  HandleFadeOut();
  if (reg_INIDISP == 0x80) {
    EnableNMI();
    WaitUntilEndOfVblankAndClearHdma();
    DisableIrqInterrupts();
    fx_layer_blending_config_a = 0;
    cur_irq_handler = 0;
    next_gameplay_CGWSEL = 0;
    next_gameplay_CGADSUB = 0;
    reg_TM = 16;
    reg_TS = 0;
    reg_TMW = 0;
    reg_TSW = 0;
    reg_BGMODE = 9;
    screen_fade_delay = 0;
    screen_fade_counter = 0;
    ceres_status = 0;
    timer_status = 0;
    QueueSfx1_Max15(2);
    QueueSfx2_Max15(0x71);
    QueueSfx3_Max15(1);
    if (CheckEventHappened(0xE)) {
      game_options_screen_index = 0;
      menu_index = 0;
      for (int i = 254; i >= 0; i -= 2)
        eproj_y_subpos[(i >> 1) + 15] = 0;
      game_state = kGameState_25_SamusNoHealth;
    } else {
      game_state = kGameState_37_CeresGoesBoomWithSamus;
      cinematic_function = FUNC16(CinematicFunctionBlackoutFromCeres);
    }
  }
  return kCoroutineNone;
}

CoroutineRet GameState_37_CeresGoesBoomWithSamus(void) {  // 0x828B0E
  return GameState_37_CeresGoesBoomWithSamus_();
}

CoroutineRet GameState_38_SamusEscapesFromZebes(void) {  // 0x8284BD
  COROUTINE_AWAIT_ONLY(GameState_8_MainGameplay());
  HandleFadeOut();
  if (reg_INIDISP == 0x80) {
    EnableNMI();
    WaitUntilEndOfVblankAndClearHdma();
    DisableIrqInterrupts();
    fx_layer_blending_config_a = 0;
    next_gameplay_CGWSEL = 0;
    next_gameplay_CGADSUB = 0;
    reg_TM = 16;
    reg_TS = 0;
    reg_TMW = 0;
    reg_TSW = 0;
    reg_BGMODE = 9;
    screen_fade_delay = 0;
    screen_fade_counter = 0;
    game_state = kGameState_39_EndingAndCredits;
    cinematic_function = FUNC16(CinematicFunctionEscapeFromCebes);
    timer_status = 0;
    QueueMusic_Delayed8(0);
    QueueSfx1_Max15(2);
    QueueSfx2_Max15(0x71);
    QueueSfx3_Max15(1);
  }
  return kCoroutineNone;
}

CoroutineRet GameState_39_EndingAndCredits(void) {  // 0x828B13
  return GameState_39_EndingAndCredits_();
}

void ShowSpareCpu(void) {  // 0x828AB0
  joypad1_input_samusfilter = joypad1_lastkeys;
}

CoroutineRet GameState_3_Null(void) {
  return kCoroutineNone;
}

CoroutineRet GameState_4_FileSelectMenus(void) {  // 0x8289E5
  FileSelectMenu();
  return kCoroutineNone;
}

CoroutineRet GameState_5_FileSelectMap(void) {  // 0x8289EA
  FileSelectMap();
  return kCoroutineNone;
}

CoroutineRet GameState_26_GameOverMenu(void) {  // 0x8289E0
  GameOverMenu();
  return kCoroutineNone;
}

CoroutineRet GameState_28_Unused(void) {  // 0x828B3F
  GameState_28_Unused_();
  return kCoroutineNone;
}

CoroutineRet GameState_29_DebugGameOverMenu(void) {  // 0x8289DB
  DebugGameOverMenu();
  return kCoroutineNone;
}

CoroutineRet GameState_8_MainGameplay(void) {  // 0x828B44
  COROUTINE_BEGIN(coroutine_state_1, 0);
  DetermineWhichEnemiesToProcess();
  if (1) { // !DebugHandler()) {
    PaletteFxHandler();
    for (int i = 0, n = MultiSamus_GetNumSamus(); i < n; i++) {
      MultiSamus_Switch(i);
      HandleControllerInputForGamePhysics();
      if (!debug_disable_sprite_interact)
        SamusProjectileInteractionHandler();
    }
    MultiSamus_Switch(0);

    EnemyMain();

    if (queued_message_box_index) {
      COROUTINE_AWAIT(1, DisplayMessageBox_Async(queued_message_box_index));
      queued_message_box_index = 0;
    }

    for (int i = 0, n = MultiSamus_GetNumSamus(); i < n; i++) {
      MultiSamus_Switch(i);
      HandleSamusMovementAndPause();
    }
    MultiSamus_Switch(0);

    EprojRunAll();
    COROUTINE_AWAIT(2, PlmHandler_Async());

    AnimtilesHandler();
    if (!debug_disable_sprite_interact) {
      for (int i = 0, n = MultiSamus_GetNumSamus(); i < n; i++) {
        MultiSamus_Switch(i);
        EprojSamusCollDetect();
      }
      MultiSamus_Switch(0);

      EprojProjCollDet();
      ProcessEnemyPowerBombInteraction();
    }
    MainScrollingRoutine();
    int debug_scrolling_enabled = 0;
    if (debug_scrolling_enabled)
      DebugScrollPosSaveLoad();
    DrawSamusEnemiesAndProjectiles();
    QueueEnemyBG2TilemapTransfers();
  }
  HandleHudTilemap();
  CalculateLayer2PosAndScrollsWhenScrolling();
  RunRoomMainCode();

  for (int i = 0, n = MultiSamus_GetNumSamus(); i < n; i++) {
    MultiSamus_Switch(i);
    HandleSamusOutOfHealthAndGameTile();
    DecrementSamusTimers();
  }
  MultiSamus_Switch(0);

  HandleRoomShaking();
  COROUTINE_END(0);
}

static Func_V_Coroutine *const kGameStateFuncs[45] = {
  GameState_0_Reset_Async,
  GameState_1_OpeningCinematic,
  GameState_2_GameOptionsMenu,
  GameState_3_Null,
  GameState_4_FileSelectMenus,
  GameState_5_FileSelectMap,
  InitAndLoadGameData_Async,
  GameState_7_MainGameplayFadeIn,
  GameState_8_MainGameplay,
  GameState_9_HitDoorBlock,
  GameState_10_LoadingNextRoom_Async,
  GameState_11_LoadingNextRoom_Async,
  GameState_12_Pausing_Darkening_Async,
  GameState_13_Pausing_Async,
  GameState_14_Paused_Async,
  GameState_15_Paused_Async,
  GameState_16_Unpausing_Async,
  GameState_17_Unpausing_Async,
  GameState_18_Unpausing,
  GameState_19_SamusNoHealth,
  GameState_20_SamusNoHealth_BlackOut,
  GameState_21_SamusNoHealth,
  GameState_22_SamusNoHealth_Dying,
  GameState_23_SamusNoHealth_Flashing,
  GameState_24_SamusNoHealth_Explosion,
  GameState_25_SamusNoHealth_BlackOut,
  GameState_26_GameOverMenu,
  GameState_27_ReserveTanksAuto,
  GameState_28_Unused,
  GameState_29_DebugGameOverMenu,
  GameState_37_CeresGoesBoomWithSamus,
  InitAndLoadGameData_Async,
  GameState_32_MadeItToCeresElevator,
  GameState_33_BlackoutFromCeres,
  GameState_37_CeresGoesBoomWithSamus,
  GameState_35_TimeUp,
  GameState_36_WhitingOutFromTimeUp,
  GameState_37_CeresGoesBoomWithSamus,
  GameState_38_SamusEscapesFromZebes,
  GameState_39_EndingAndCredits,
  InitAndLoadGameData_Async,
  GameState_41_TransitionToDemo,
  GameState_42_PlayingDemo_Async,
  GameState_43_TransitionFromDemo,
  GameState_44_TransitionFromDemo,
};

static CoroutineRet RunOneFrameOfGameInner(void) {
  int st = coroutine_state_0;
  if (st >= 1 && st <= 3) {
    COROUTINE_AWAIT_ONLY(Vector_RESET_Async())
  } else if (st >= 10) {
    goto RESUME_AT_SWITCH;
  } else if (st != 0) {
    Die("Incorrect coroutine_state_0");
  }

  coroutine_state_1 = coroutine_state_2 = coroutine_state_3 = coroutine_state_4 = 0;

  ReadJoypadInputs();
  HdmaObjectHandler();
  NextRandom();
  ClearOamExt();
  oam_next_ptr = 0;
  nmi_copy_samus_halves = 0;
  nmi_copy_samus_top_half_src = 0;
  nmi_copy_samus_bottom_half_src = 0;

  coroutine_state_0 = st = game_state + 10;
RESUME_AT_SWITCH:
  COROUTINE_AWAIT_ONLY(kGameStateFuncs[st - 10]());

  HandleSoundEffects();
  ClearUnusedOam();
  ShowSpareCpu();
  
  if (coroutine_state_1 | coroutine_state_2 | coroutine_state_3 | coroutine_state_4) {
    printf("Coroutine state: %d, %d, %d, %d\n",
      coroutine_state_1, coroutine_state_2, coroutine_state_3, coroutine_state_4);
    Warning("Coroutine State is broken!");
  }

  coroutine_state_0 = 0;
  return kCoroutineNone;
}

void RunOneFrameOfGame(void) {  // 0x828948
  CoroutineRet ret = RunOneFrameOfGameInner();
  if (ret == 0)
    waiting_for_nmi = 1;

  Vector_NMI();
}

uint16 NextRandom(void) {  // 0x808111
  uint16 RegWord = LOBYTE(random_number) * 5;
  uint8 Reg = HIBYTE(random_number) * 5;

  int carry = HIBYTE(RegWord) + Reg + 1;
  HIBYTE(RegWord) = carry;
  uint16 result = (carry >> 8) + RegWord + 17;
  random_number = result;
  return result;
}

uint16 PrepareBitAccess(uint16 a) {  // 0x80818E
  bitmask = 1 << (a & 7);
  return a >> 3;
}

void SetBossBitForCurArea(uint16 a) {  // 0x8081A6
  boss_bits_for_area[area_index] |= a;
}

void ClearBossBitForCurArea(uint16 a) {  // 0x8081C0
  boss_bits_for_area[area_index] &= ~a;
}

uint8 CheckBossBitForCurArea(uint16 a) {  // 0x8081DC
  return (a & boss_bits_for_area[area_index]) != 0;
}

void SetEventHappened(uint16 a) {  // 0x8081FA
  uint16 v1 = PrepareBitAccess(a);
  events_that_happened[v1] |= bitmask;
}

void ClearEventHappened(uint16 v0) {  // 0x808212
  uint16 v1 = PrepareBitAccess(v0);
  events_that_happened[v1] &= ~bitmask;
}

uint16 CheckEventHappened(uint16 a) {  // 0x808233
  uint16 idx = PrepareBitAccess(a);
  return (bitmask & events_that_happened[idx]) != 0;
}

void DebugScrollPosSaveLoad(void) {  // 0x80A9AC
  if ((joypad2_new_keys & 0x40) != 0)
    ++debug_saveload_scrollpos_toggle;
  if (debug_saveload_scrollpos_toggle & 1) {
    layer1_x_pos = debug_saved_xscroll;
    layer1_y_pos = debug_saved_yscroll;
  } else {
    debug_saved_xscroll = layer1_x_pos;
    debug_saved_yscroll = layer1_y_pos;
  }
}
