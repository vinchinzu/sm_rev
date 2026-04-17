#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "sm_82_data.h"

#define kDemoRoomData ((uint16*)RomFixedPtr(0x82876c))

void CheckForNextDemo(void) {  // 0x828637
  if (*(uint16 *)RomPtr_82(kDemoRoomData[demo_set] + 18 * demo_scene) == 0xFFFF) {
    substate = 0;
    uint16 v0 = demo_set + 1;
    if (v0 >= num_demo_sets)
      v0 = 0;
    demo_set = v0;
    demo_scene = 0;
  } else {
    substate = 0x8000;
  }
}

void LoadDemoRoomData(void) {  // 0x828679
  DemoRoomData *drd;

  door_def_ptr = 0;
  drd = get_DemoRoomData(18 * demo_scene + kDemoRoomData[demo_set]);
  room_ptr = drd->room_ptr_;
  door_def_ptr = drd->door_ptr;
  layer1_x_pos = drd->screen_x_pos;
  bg1_x_offset = layer1_x_pos;
  layer1_y_pos = drd->screen_y_pos;
  bg1_y_offset = layer1_y_pos;
  samus_y_pos = layer1_y_pos + drd->samus_x_offs;
  samus_prev_y_pos = samus_y_pos;
  samus_x_pos = drd->samus_y_offs + layer1_x_pos + 128;
  samus_prev_x_pos = samus_x_pos;
  demo_timer = drd->demo_length;
  LOBYTE(area_index) = get_RoomDefHeader(room_ptr)->area_index_;
  reg_BG1HOFS = 0;
  reg_BG1VOFS = 0;
  ++demo_scene;
  uint16 v1 = 0;
  do {
    int v2 = v1 >> 1;
    WORD(room_chozo_bits[v1]) = -1;
    *(uint16 *)&item_bit_array[v1] = -1;
    UNUSED_word_7ED8F0[v2] = -1;
    *(uint16 *)&map_station_byte_array[v1] = -1;
    *(uint16 *)&used_save_stations_and_elevators[v1] = -1;
    *(uint16 *)&used_save_stations_and_elevators[v1 + 8] = -1;
    WORD(opened_door_bit_array[v1]) = 0;
    WORD(events_that_happened[v1]) = 0;
    *(uint16 *)&boss_bits_for_area[v1] = 0;
    v1 += 2;
  } while ((int16)(v1 - 8) < 0);
  do {
    int v3 = v1 >> 1;
    WORD(room_chozo_bits[v1]) = -1;
    *(uint16 *)&item_bit_array[v1] = -1;
    WORD(opened_door_bit_array[v1]) = 0;
    v1 += 2;
  } while ((int16)(v1 - 64) < 0);
  uint16 v4 = 0;
  do {
    explored_map_tiles_saved[v4 >> 1] = 0;
    v4 += 2;
  } while ((int16)(v4 - 1536) < 0);
  samus_max_reserve_health = 0;
  samus_reserve_health = 0;
  reserve_health_mode = 0;
  loading_game_state = kLoadingGameState_0_Intro;
  debug_disable_minimap = 0;
}

void DemoRoom_ChargeBeamRoomScroll21(void) {  // 0x82891A
  scrolls[33] = 0;
}

void DemoRoom_SetBG2TilemapBase(void) {  // 0x828925
  *(uint16 *)&reg_BG2SC = 74;
}

void DemoRoom_SetKraidFunctionTimer(void) {  // 0x82892B
  enemy_data[0].ai_preinstr = 60;
}

void DemoRoom_SetBrinstarBossBits(void) {  // 0x828932
  boss_bits_for_area[1] = 1;
}

CoroutineRet GameState_41_TransitionToDemo(void) {  // 0x82852D
  COROUTINE_AWAIT_ONLY(GameState_8_MainGameplay());
  HdmaObjectHandler();
  ++game_state;
  reg_INIDISP = 15;
  return kCoroutineNone;
}

CoroutineRet GameState_42_PlayingDemo_Async(void) {  // 0x828548
  COROUTINE_BEGIN(coroutine_state_3, 0)
  COROUTINE_AWAIT(2, GameState_8_MainGameplay());
  if (joypad1_newkeys) {
    substate = 1;
    goto LABEL_10;
  }
  demo_timer--;
  if (!demo_timer || (demo_timer & 0x8000) != 0) {
    substate = 0;
    my_counter = 90;
    while (1) {
      COROUTINE_AWAIT(1, WaitForNMI_Async());
      if (joypad1_newkeys)
        break;
      if (!--my_counter)
        goto LABEL_10;
    }
    substate = 1;

LABEL_10:
    ++game_state;
    debug_disable_sounds = 0;
    reg_INIDISP = 0x80;
    screen_fade_delay = 1;
    screen_fade_counter = 1;
  }
  COROUTINE_END(0);
}

CoroutineRet GameState_43_TransitionFromDemo(void) {  // 0x828593
  if (substate != 1)
    CheckForNextDemo();
  EnableNMI();
  ++game_state;
  screen_fade_delay = 0;
  screen_fade_counter = 0;
  WaitUntilEndOfVblankAndClearHdma();
  DisableIrqInterrupts();
  fx_layer_blending_config_a = 0;
  cur_irq_handler = 0;
  irqhandler_next_handler = 0;
  DisablePaletteFx();
  ClearPaletteFXObjects();
  for (int i = 656; i >= 0; i -= 2)
    *(uint16 *)((uint8 *)&eproj_enable_flag + i) = 0;
  for (int j = 538; j >= 0; j -= 2)
    *(uint16 *)((uint8 *)&hud_item_tilemap_palette_bits + j) = 0;
  next_gameplay_CGWSEL = 0;
  next_gameplay_CGADSUB = 0;
  reg_TM = 16;
  reg_TS = 0;
  reg_TMW = 0;
  reg_TSW = 0;
  return kCoroutineNone;
}

CoroutineRet GameState_44_TransitionFromDemo(void) {  // 0x8285FB
  game_state = kGameState_1_OpeningCinematic;
  if ((substate & 0x8000) != 0) {
    game_state = kGameState_40_TransitionToDemo;
  } else if (substate) {
    LoadTitleSequenceGraphics();
    eproj_x_pos[4] = 2;
    cinematic_function = FUNC16(CinematicFunctionNone);
  } else {
    QueueMusic_Delayed8(0);
    debug_disable_sounds = 0;
    cinematic_function = FUNC16(CinematicFunctionOpening);
  }
  return kCoroutineNone;
}
