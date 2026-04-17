#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"

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
