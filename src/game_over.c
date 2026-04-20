#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"

void DrawBabyMetroid(uint16 k) {  // 0x82BB9E
  uint16 v1 = *((uint16 *)RomPtr_82(k) + 2);
  for(int i = 0; i < 32; i += 2)
    palette_buffer[(i >> 1) + 192] = *(uint16 *)RomPtr_82(v1 + i);
  const uint8 *v3 = RomPtr_82(k);
  DrawMenuSpritemap(GET_WORD(v3 + 2), 0x7C, 0x50, 2048);
  DrawMenuSpritemap(0x64, 0x7C, 0x50, 2560);
}

void sub_82BBDD(void) {  // 0x82BBDD
  enemy_data[0].current_instruction = addr_kgameOverBabyMetridInstructionList;
  enemy_data[0].instruction_timer = 10;
  sub_82BB7F(0xA);
}

void CallBabyMetroidPlaySfx(uint32 ea) {
  switch (ea) {
  case fnBabyMetroidPlaySfx0x23: BabyMetroidPlaySfx0x23(); return;
  case fnBabyMetroidPlaySfx0x26: BabyMetroidPlaySfx0x26(); return;
  case fnBabyMetroidPlaySfx0x27: BabyMetroidPlaySfx0x27(); return;
  default: Unreachable();
  }
}

void sub_82BB7F(uint16 a) {  // 0x82BB7F
  uint16 current_instruction = enemy_data[0].current_instruction;
  enemy_data[0].instruction_timer = a - 1;
  if (a == 1) {
    const uint8 *v2 = RomPtr_82(enemy_data[0].current_instruction);
    if (GET_WORD(v2 + 6) == 0xFFFF) {
      sub_82BBDD();
    } else if ((int16)(GET_WORD(v2 + 6) + 1) >= 0) {
      enemy_data[0].instruction_timer = GET_WORD(v2 + 6);
      enemy_data[0].current_instruction += 6;
      DrawBabyMetroid(current_instruction + 6);
    } else {
      CallBabyMetroidPlaySfx(GET_WORD(v2 + 6) | 0x820000);
    }
  } else {
    DrawBabyMetroid(enemy_data[0].current_instruction);
  }
}


void HandleGameOverBabyMetroid(void) {  // 0x82BB75
  if (enemy_data[0].instruction_timer)
    sub_82BB7F(enemy_data[0].instruction_timer);
  else
    sub_82BBDD();
}

void FinishProcessingGameOverBabyMetroidAsm(void) {  // 0x82BBF0
  uint16 t = *((uint16 *)RomPtr_82(enemy_data[0].current_instruction) + 4);
  enemy_data[0].instruction_timer = t;
  enemy_data[0].current_instruction += 8;
  if (t == 0xFFFF)
    sub_82BBDD();
  else
    DrawBabyMetroid(enemy_data[0].current_instruction);
}

void BabyMetroidPlaySfx0x23(void) {  // 0x82BC0C
  QueueSfx3_Max6(0x23);
  FinishProcessingGameOverBabyMetroidAsm();
}

void BabyMetroidPlaySfx0x26(void) {  // 0x82BC15
  QueueSfx3_Max6(0x26);
  FinishProcessingGameOverBabyMetroidAsm();
}

void BabyMetroidPlaySfx0x27(void) {  // 0x82BC1E
  QueueSfx3_Max6(0x27);
  FinishProcessingGameOverBabyMetroidAsm();
}

void CancelSoundEffects(void) {  // 0x82BE17
  QueueSfx1_Max6(2);
  QueueSfx2_Max6(0x71);
  QueueSfx3_Max6(1);
}

void HandleSamusOutOfHealthAndGameTile(void) {  // 0x82DB69
  if ((int16)samus_health <= 0) {
    if ((reserve_health_mode & 1) != 0 && samus_reserve_health) {
      time_is_frozen_flag = 0x8000;
      game_state = kGameState_27_ReserveTanksAuto;
      CallSomeSamusCode(0x1B);
    } else {
      if (game_state != kGameState_8_MainGameplay)
        return;
      time_is_frozen_flag = 0x8000;
      CallSomeSamusCode(0x11);
      game_state = kGameState_19_SamusNoHealth;
    }
  }
  if (!sign16(++game_time_frames - 60)) {
    game_time_frames = 0;
    if (!sign16(++game_time_seconds - 60)) {
      game_time_seconds = 0;
      if (!sign16(++game_time_minutes - 60)) {
        game_time_minutes = 0;
        ++game_time_hours;
      }
    }
  }
  if (!sign16(game_time_hours - 100)) {
    game_time_frames = 59;
    game_time_seconds = 59;
    game_time_minutes = 59;
    game_time_hours = 99;
  }
}

uint8 RefillHealthFromReserveTanks(void) {  // 0x82DC31
  if (samus_reserve_health) {
    if ((nmi_frame_counter_word & 7) == 0)
      QueueSfx3_Max3(0x2D);
    if ((int16)(++samus_health - samus_max_health) >= 0) {
      samus_health = samus_max_health;
LABEL_9:
      samus_reserve_health = 0;
      return samus_reserve_health == 0;
    }
    bool v0 = (--samus_reserve_health & 0x8000) != 0;
    if (!samus_reserve_health)
      goto LABEL_9;
    if (v0) {
      samus_health += samus_reserve_health;
      goto LABEL_9;
    }
  }
  return samus_reserve_health == 0;
}

CoroutineRet GameState_27_ReserveTanksAuto(void) {  // 0x82DC10
  if (coroutine_state_1 == 0 && RefillHealthFromReserveTanks()) {
    time_is_frozen_flag = 0;
    game_state = kGameState_8_MainGameplay;
    CallSomeSamusCode(0x10);
  }
  COROUTINE_AWAIT_ONLY(GameState_8_MainGameplay());
  Samus_LowHealthCheck_0();
  return kCoroutineNone;
}

CoroutineRet GameState_19_SamusNoHealth(void) {  // 0x82DC80
  COROUTINE_AWAIT_ONLY(GameState_8_MainGameplay());
  for (int i = 255; i >= 0; --i)
    ram3000.pause_menu_map_tilemap[i + 384] = palette_buffer[i];
  for (int j = 382; (j & 0x8000) == 0; j -= 2)
    target_palettes[j >> 1] = 0;
  for (int k = 94; (k & 0x8000) == 0; k -= 2)
    target_palettes[(k >> 1) + 208] = 0;
  for (int m = 30; (m & 0x8000) == 0; m -= 2)
    target_palettes[(m >> 1) + 192] = palette_buffer[(m >> 1) + 192];
  game_options_screen_index = 3;
  g_word_7E0DE4 = 0;
  g_word_7E0DE6 = 0;
  g_word_7E0DE8 = 0;
  hud_item_index = 0;
  samus_auto_cancel_hud_item_index = 0;
  samus_invincibility_timer = 0;
  samus_knockback_timer = 0;
  ++game_state;

  return kCoroutineNone;
}

CoroutineRet GameState_20_SamusNoHealth_BlackOut(void) {  // 0x82DCE0
  COROUTINE_AWAIT_ONLY(GameState_8_MainGameplay());
  palette_change_denom = 6;
  if (AdvancePaletteFadeForAllPalettes()) {
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
    game_options_screen_index = 0;
    screen_fade_delay = 0;
    screen_fade_counter = 0;
    for (int i = 254; i >= 0; i -= 2)
      eproj_y_subpos[(i >> 1) + 15] = 0;
    g_word_7E0DE8 = 16;
    game_options_screen_index = 3;
    g_word_7E0DE4 = 0;
    g_word_7E0DE6 = 0;
    ++game_state;
    power_bomb_explosion_status = 0;
    QueueSfx1_Max15(2);
    QueueSfx2_Max15(0x71);
    QueueSfx3_Max15(1);
    QueueMusic_Delayed8(0);
    QueueMusic_Delayed8(0xFF39);
    QueueMusic_DelayedY(5, 0xE);
  }
  return kCoroutineNone;
}

CoroutineRet GameState_21_SamusNoHealth(void) {  // 0x82DD71
  Samus_DrawWhenNotAnimatingOrDying();
  if (!HasQueuedMusic()) {
    StartSamusDeathAnimation();
    ++game_state;
  }
  return kCoroutineNone;
}

CoroutineRet GameState_22_SamusNoHealth_Dying(void) {  // 0x82DD87
  DrawSamusStartingDeathAnim_();
  bool v0 = (--g_word_7E0DE8 & 0x8000) != 0;
  if (!g_word_7E0DE8 || v0)
    ++game_state;
  return kCoroutineNone;
}

CoroutineRet GameState_23_SamusNoHealth_Flashing(void) {  // 0x82DD9A
  if (HandleSamusDeathSequence())
    ++game_state;
  else
    Samus_DrawDuringDeathAnim();
  return kCoroutineNone;
}

CoroutineRet GameState_24_SamusNoHealth_Explosion(void) {  // 0x82DDAF
  if (GameState_24_SamusNoHealth_Explosion_Helper()) {
    screen_fade_delay = 1;
    screen_fade_counter = 1;
    ++game_state;
  }
  return kCoroutineNone;
}

CoroutineRet GameState_25_SamusNoHealth_BlackOut(void) {  // 0x82DDC7
  HandleFadeOut();
  if (reg_INIDISP == 0x80) {
    EnableNMI();
    screen_fade_delay = 0;
    screen_fade_counter = 0;
    ++game_state;
    menu_index = 0;
    debug_disable_sounds = 0;
  }
  return kCoroutineNone;
}
