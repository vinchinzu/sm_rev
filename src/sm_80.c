// System routines 

#include <stdio.h>
#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "enemy_types.h"
#include "spc_player.h"


#define kLoadStationLists ((uint16*)RomFixedPtr(0x80c4b5))
#define off_80CD46 ((uint16*)RomFixedPtr(0x80cd46))

void APU_UploadBank(uint32 addr) {  // 0x808028
  if (!g_use_my_apu_code)
    return;
  RtlApuUpload(RomPtr(addr));
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

void ReleaseButtonsFilter(uint16 v0) {  // 0x808146
  timed_held_input_timer_reset = v0;
  bool v1 = ((uint16)~joypad1_newkeys & joypad1_lastkeys) == joypad_released_keys;
  joypad_released_keys = ~joypad1_newkeys & joypad1_lastkeys;
  if (!v1) {
    timed_held_input_timer = timed_held_input_timer_reset;
    timed_held_input = 0;
    goto LABEL_6;
  }
  if ((--timed_held_input_timer & 0x8000) == 0) {
    timed_held_input = 0;
    goto LABEL_6;
  }
  timed_held_input_timer = 0;
  previous_timed_held_input = timed_held_input;
  timed_held_input = ~joypad1_newkeys & joypad1_lastkeys;
LABEL_6:
  newly_held_down_timed_held_input = timed_held_input & (previous_timed_held_input ^ timed_held_input);
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

void CopySuperMetroidString(void) {  // 0x80824F
  memcpy(&g_sram[0x1fe0], "supermetroid", 12);
  RtlWriteSram();
}

void VerifySRAM(void) {  // 0x808261
  num_demo_sets = 3;
  if (LoadFromSram(0) && LoadFromSram(1) && LoadFromSram(2)) {
    memcpy(&g_sram[0x1fe0], "madadameyohn", 12);
    RtlWriteSram();
  } else {
    if (!memcmp(&g_sram[0x1fe0], "supermetroid", 12))
      num_demo_sets = 4;
  }
}

uint32 Multiply16x16(uint16 a, uint16 j) {  // 0x8082D6
  uint32 result = (uint32)a * (uint32)j;
  return result;
}

CoroutineRet WaitForNMI_Async(void) {  // 0x808338
  // Return 0 from this routine as soon as the coroutine has finished
  if (coroutine_completion_flags) {
    coroutine_completion_flags = 0;
    return 0;
  }
  waiting_for_nmi = 1;
  coroutine_completion_flags = 1;
  return 1;
}

CoroutineRet WaitForNMI_NoUpdate_Async(void) {
  // Return 0 from this routine as soon as the coroutine has finished
  if (coroutine_completion_flags) {
    coroutine_completion_flags = 0;
    return 0;
  }
  coroutine_completion_flags = 1;
  return 1;
}

void EnableNMI(void) {  // 0x80834B
  uint8 v0 = reg_NMITIMEN | 0x80;
  WriteReg(NMITIMEN, reg_NMITIMEN | 0x80);
  reg_NMITIMEN = v0;
}

void DisableNMI(void) {  // 0x80835D
  uint8 v0 = reg_NMITIMEN & 0x7F;
  WriteReg(NMITIMEN, reg_NMITIMEN & 0x7F);
  reg_NMITIMEN = v0;
}

void ScreenOff(void) {
  reg_INIDISP |= 0x80;
}

void ScreenOn(void) {
  reg_INIDISP &= ~0x80;
}

void memset7E(uint16 *k, uint16 a, uint16 j) {  // 0x8083F6
  do {
    *k++ = a;
  } while (j -= 2);
}

CoroutineRet Vector_RESET_Async(void) {  // 0x80841C
  COROUTINE_BEGIN(coroutine_state_0, 1)
  WriteReg(MEMSEL, 1);
  reg_MEMSEL = 1;
  // Removed code to wait 4 frames
  uint16 bak = bug_fix_counter;
  memset(g_ram, 0, 8192);
  bug_fix_counter = bak;
  COROUTINE_AWAIT(2, InitializeIoDisplayLogo_Async());
  COROUTINE_MANUAL_POS(3); // Soft reset position
  APU_UploadBank(0xCF8000);
  WriteReg(INIDISP, 0x8F);
  bak = bug_fix_counter;
  memset(g_ram, 0, 0x10000);
  bug_fix_counter = bak;
  WriteReg(NMITIMEN, 0);
  reg_NMITIMEN = 0;
  reg_INIDISP = 0x8f;
  InitializeCpuIoRegs();
  InitializePpuIoRegs();
  WriteLotsOf0x1c2f();
  sfx_readpos[0] = 0;
  sfx_readpos[1] = 0;
  sfx_readpos[2] = 0;
  sfx_writepos[0] = 0;
  sfx_writepos[1] = 0;
  sfx_writepos[2] = 0;
  sfx_state[0] = 0;
  sfx_state[1] = 0;
  sfx_state[2] = 0;
  sfx_cur[0] = 0;
  sfx_cur[1] = 0;
  sfx_cur[2] = 0;
  sfx_clear_delay[0] = 0;
  sfx_clear_delay[1] = 0;
  sfx_clear_delay[2] = 0;
  sfx1_queue[0] = 0;
  sfx2_queue[0] = 0;
  sfx3_queue[0] = 0;
  oam_next_ptr = 0;
  reg_OAMaddr_UNUSED = 0;
  ClearOamExt();
  ClearUnusedOam();
  nmi_copy_samus_halves = 0;
  nmi_copy_samus_top_half_src = 0;
  nmi_copy_samus_bottom_half_src = 0;
  EnableNMI();
  RtlApuWrite(APUI00, 0);
  RtlApuWrite(APUI02, 0);

  // Removed code to wait 4 frames
  random_number = 97;
  music_timer = 0;
  music_queue_delay[0] = 0;
  music_queue_delay[1] = 0;
  music_queue_delay[2] = 0;
  music_queue_delay[3] = 0;
  music_queue_delay[4] = 0;
  music_queue_delay[5] = 0;
  music_queue_delay[6] = 0;
  music_queue_delay[7] = 0;
  enable_debug = 0;
  VerifySRAM();
  debug_disable_sounds = 0;
  sound_handler_downtime = 0;
  COROUTINE_END(2);
}

void InvalidInterrupt_Crash(void) {  // 0x808573
  printf("InvalidInterrupt_Crash\n");
  Unreachable();
  for (;;);
}

void LoadMirrorOfExploredMapTiles(void) {  // 0x80858C
  uint16 v1 = swap16(area_index);
  uint16 v2 = 0;
  do {
    *(uint16 *)&map_tiles_explored[v2] = explored_map_tiles_saved[v1 >> 1];
    v1 += 2;
    v2 += 2;
  } while ((int16)(v2 - 256) < 0);
  has_area_map = map_station_byte_array[area_index];
}

void SaveExploredMapTilesToSaved(void) {  // 0x8085C6
  uint16 v1 = swap16(area_index);
  uint16 v2 = 0;
  do {
    explored_map_tiles_saved[v1 >> 1] = *(uint16 *)&map_tiles_explored[v2];
    v1 += 2;
    v2 += 2;
  } while ((int16)(v2 - 256) < 0);
  if (has_area_map)
    *(uint16 *)&map_station_byte_array[area_index] |= 0xFF;
}

void InitializeCpuIoRegs(void) {  // 0x80875D
  WriteReg(NMITIMEN, 1);
  reg_NMITIMEN = 1;
  WriteReg(WRIO, 0);
  WriteReg(WRMPYA, 0);
  WriteReg(WRMPYB, 0);
  WriteReg(WRDIVL, 0);
  WriteReg(WRDIVH, 0);
  WriteReg(WRDIVB, 0);
  WriteReg(HTIMEL, 0);
  WriteReg(HTIMEH, 0);
  WriteReg(VTIMEL, 0);
  WriteReg(VTIMEH, 0);
  WriteReg(MDMAEN, 0);
  WriteReg(HDMAEN, 0);
  reg_HDMAEN = 0;
  WriteReg(MEMSEL, 1);
  reg_MEMSEL = 1;
}

void InitializePpuIoRegs(void) {  // 0x808792
  WriteReg(INIDISP, 0x8F);
  reg_INIDISP = 0x8f;
  WriteReg(OBSEL, 3);
  reg_OBSEL = 3;
  WriteReg(OAMADDL, 0);
  LOBYTE(reg_OAMaddr_UNUSED) = 0;
  WriteReg(OAMADDH, 0x80);
  HIBYTE(reg_OAMaddr_UNUSED) = 0x80;
  WriteReg(OAMDATA, 0);
  WriteReg(OAMDATA, 0);
  WriteReg(BGMODE, 9);
  reg_BGMODE = 9;
  WriteReg(MOSAIC, 0);
  reg_MOSAIC = 0;
  WriteReg(BG1SC, 0x40);
  reg_BG1SC = 64;
  WriteReg(BG2SC, 0x44);
  reg_BG2SC = 68;
  WriteReg(BG3SC, 0x48);
  reg_BG3SC = 72;
  WriteReg(BG4SC, 0);
  reg_BG4SC = 0;
  WriteReg(BG12NBA, 0);
  reg_BG12NBA = 0;
  WriteReg(BG34NBA, 5);
  reg_BG34NBA = 5;
  WriteReg(BG1HOFS, 0);
  WriteReg(BG1HOFS, 0);
  WriteReg(BG1VOFS, 0);
  WriteReg(BG1VOFS, 0);
  WriteReg(BG2HOFS, 0);
  WriteReg(BG2HOFS, 0);
  WriteReg(BG2VOFS, 0);
  WriteReg(BG2VOFS, 0);
  WriteReg(BG3HOFS, 0);
  WriteReg(BG3HOFS, 0);
  WriteReg(BG3VOFS, 0);
  WriteReg(BG3VOFS, 0);
  WriteReg(BG4HOFS, 0);
  WriteReg(BG4HOFS, 0);
  WriteReg(BG4VOFS, 0);
  WriteReg(BG4VOFS, 0);
  WriteReg(VMAIN, 0);
  WriteReg(M7SEL, 0);
  reg_M7SEL = 0;
  WriteReg(M7A, 0);
  WriteReg(M7B, 0);
  WriteReg(M7C, 0);
  WriteReg(M7D, 0);
  WriteReg(M7X, 0);
  WriteReg(M7Y, 0);
  WriteReg(W12SEL, 0);
  reg_W12SEL = 0;
  WriteReg(W34SEL, 0);
  reg_W34SEL = 0;
  WriteReg(WOBJSEL, 0);
  reg_WOBJSEL = 0;
  WriteReg(WH0, 0);
  reg_WH0 = 0;
  WriteReg(WH1, 0xF8);
  reg_WH1 = -8;
  WriteReg(WH2, 0);
  reg_WH2 = 0;
  WriteReg(WH3, 0);
  reg_WH3 = 0;
  WriteReg(WBGLOG, 0);
  reg_WBGLOG = 0;
  WriteReg(WOBJLOG, 0);
  reg_WOBJLOG = 0;
  WriteReg(TM, 0x11);
  reg_TM = 17;
  WriteReg(TMW, 0x11);
  reg_TMW = 17;
  WriteReg(TS, 2);
  reg_TS = 2;
  WriteReg(TSW, 2);
  reg_TSW = 2;
  WriteReg(CGWSEL, 2);
  next_gameplay_CGWSEL = 2;
  WriteReg(CGADSUB, 0xA1);
  next_gameplay_CGADSUB = -95;
  WriteReg(COLDATA, 0xE0);
  WriteReg(COLDATA, 0xE0);
  WriteReg(COLDATA, 0x80);
  reg_COLDATA[0] = 0x80;
  WriteReg(COLDATA, 0x40);
  reg_COLDATA[1] = 64;
  WriteReg(COLDATA, 0x20);
  reg_COLDATA[2] = 32;
  WriteReg(SETINI, 0);
  reg_SETINI = 0;
}

void WriteLotsOf0x1c2f(void) {  // 0x8088D1
  sub_8088EB(0x1C2F);
  sub_8088FE(0x1C2F);
  sub_808911(0x1C2F);
}

void sub_8088EB(uint16 a) {  // 0x8088EB
  memset7E((uint16*)&ram3000, a, 0x800);
}

void sub_8088FE(uint16 a) {  // 0x8088FE
  memset7E((uint16*)&ram4000, a, 0x800);
}

void sub_808911(uint16 a) {  // 0x808911
  memset7E(ram6000, a, 0x800);
}

void HandleFadeOut(void) {  // 0x808924
  if ((int16)(screen_fade_counter - 1) < 0) {
    screen_fade_counter = screen_fade_delay;
    if ((reg_INIDISP & 0xF) != 0) {
      if ((reg_INIDISP & 0xF) == 1)
        reg_INIDISP = 0x80;
      else
        reg_INIDISP = (reg_INIDISP & 0xF) - 1;
    }
  } else {
    --screen_fade_counter;
  }
}

void HandleFadeIn(void) {  // 0x80894D
  if ((int16)(screen_fade_counter - 1) < 0) {
    screen_fade_counter = screen_fade_delay;
    if (((reg_INIDISP + 1) & 0xF) != 0)
      reg_INIDISP = (reg_INIDISP + 1) & 0xF;
  } else {
    --screen_fade_counter;
  }
}

void ClearUnusedOam(void) {
  for (int i = oam_next_ptr >> 2; i < 0x80; i++)
    oam_ent[i].ycoord = 0xf0;
  oam_next_ptr = 0;
}

void ClearOamExt(void) {  // 0x808B1A
  memset(oam_ext, 0, sizeof(oam_ext[0]) * 16);
}


void ReadJoypadInputs(void) {  // 0x809459
  uint16 RegWord = ReadRegWord(JOY1L);
  uint16 v1 = ReadRegWord(JOY2L);
  joypad1_lastkeys = RegWord;
  joypad1_newkeys = RegWord & (joypad1_prev ^ RegWord);
  joypad1_newkeys2_UNUSED = RegWord & (joypad1_prev ^ RegWord);
  if (RegWord && RegWord == joypad1_prev) {
    if (!--joypad1_keyrepeat_ctr) {
      joypad1_newkeys2_UNUSED = joypad1_lastkeys;
      joypad1_keyrepeat_ctr = joypad_ctr_repeat_next;
    }
  } else {
    joypad1_keyrepeat_ctr = joypad_ctr_repeat_first;
  }
  joypad1_prev = joypad1_lastkeys;
  joypad2_last = v1;
  joypad2_new_keys = v1 & (joypad2_prev ^ v1);
  joypad2_newkeys2 = v1 & (joypad2_prev ^ v1);
  if (v1 && v1 == joypad2_prev) {
    if (!--joypad2_keyrepeat_ctr) {
      joypad2_newkeys2 = joypad2_last;
      joypad2_keyrepeat_ctr = joypad_ctr_repeat_next;
    }
  } else {
    joypad2_keyrepeat_ctr = joypad_ctr_repeat_first;
  }
  joypad2_prev = joypad2_last;
  if (enable_debug) {
    if (is_uploading_apu || joypad1_lastkeys != (kButton_Select | kButton_Start | kButton_L | kButton_R)) {
      joypad_dbg_1 = 0;
      joypad_dbg_2 = 0;
      if ((joypad_dbg_flags & 0x4000) == 0) {
        if ((joypad1_lastkeys & (kButton_Select | kButton_L)) == (kButton_Select | kButton_L)) {
          joypad_dbg_1 = joypad1_newkeys;
          joypad1_lastkeys = 0;
          joypad1_newkeys = 0;
        }
        if ((joypad1_lastkeys & (kButton_Select | kButton_R)) == (kButton_Select | kButton_R)) {
          joypad_dbg_2 = joypad1_newkeys;
          joypad1_lastkeys = 0;
          joypad1_newkeys = 0;
        }
        if ((joypad_dbg_2 & 0x80) != 0)
          *(uint16 *)&reg_NMITIMEN ^= 0x30;
        if ((joypad_dbg_2 & 0x8000) != 0) {
          bool v2 = (~joypad_dbg_flags & 0x8000) != 0;
          joypad_dbg_flags ^= 0x8000;
          if (v2) {
            joypad_dbg_missiles_swap = samus_missiles;
            joypad_dbg_super_missiles_swap = samus_super_missiles;
            joypad_dbg_power_bombs_swap = samus_power_bombs;
            samus_missiles = 0;
            samus_super_missiles = 0;
            samus_power_bombs = 0;
          } else {
            samus_missiles = joypad_dbg_missiles_swap;
            samus_super_missiles = joypad_dbg_super_missiles_swap;
            samus_power_bombs = joypad_dbg_power_bombs_swap;
          }
        }
        if ((joypad_dbg_2 & 0x40) != 0)
          joypad_dbg_flags ^= 0x2000;
      }
    } else {
      debug_disable_sounds = 0;
      SoftReset();
    }
  }
}


CoroutineRet StartGameplay_Async(void) {  // 0x80A07B
  COROUTINE_BEGIN(coroutine_state_2, 0)
  WriteRegWord(MDMAEN, 0);
  scrolling_finished_hook = 0;
  music_data_index = 0;
  music_track_index = 0;
  timer_status = 0;
  ResetSoundQueues();
  debug_disable_sounds = -1;
  DisableNMI();
  DisableIrqInterrupts();
  LoadDestinationRoomThings();
  COROUTINE_AWAIT(1, Play20FramesOfMusic_Async());
  ClearAnimtiles();
  WaitUntilEndOfVblankAndClearHdma();
  InitializeSpecialEffectsForNewRoom();
  ClearPLMs();
  ClearEprojs();
  ClearPaletteFXObjects();
  UpdateBeamTilesAndPalette();
  LoadColorsForSpritesBeamsAndEnemies();
  LoadEnemies();
  LoadRoomMusic();
  COROUTINE_AWAIT(2, Play20FramesOfMusic_Async());
  UpdateMusicTrackIndex();
  NullFunc();
  ClearBG2Tilemap();
  LoadLevelDataAndOtherThings();
  LoadFXHeader();
  LoadLibraryBackground();
  CalculateLayer2Xpos();
  CalculateLayer2Ypos();
  bg2_x_scroll = layer2_x_pos;
  bg2_y_scroll = layer2_y_pos;
  CalculateBgScrolls();
  DisplayViewablePartOfRoom();
  EnableNMI();
  irqhandler_next_handler = (room_loading_irq_handler == 0) ? 4 : room_loading_irq_handler;
  EnableIrqInterrupts();
  COROUTINE_AWAIT(3, Play20FramesOfMusic_Async());
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x08, 0x08, 0xb7eb });
  door_transition_function = FUNC16(DoorTransition_FadeInScreenAndFinish);
  COROUTINE_END(0);
}

CoroutineRet Play20FramesOfMusic_Async(void) {  // 0x80A12B
  COROUTINE_BEGIN(coroutine_state_3, 0)
  EnableNMI();
  for(my_counter = 0; my_counter != 20; my_counter++) {
    HandleMusicQueue();
    COROUTINE_AWAIT(1, WaitForNMI_Async());
  }
  DisableNMI();
  COROUTINE_END(0);
}

void ResumeGameplay(void) {  // 0x80A149
  WriteRegWord(MDMAEN, 0);
  DisableNMI();
  DisableIrqInterrupts();
  LoadCRETilesTilesetTilesAndPalette();
  LoadLibraryBackground();
  DisplayViewablePartOfRoom();
  LoadRoomPlmGfx();
  EnableNMI();
  EnableIrqInterrupts();
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

void ConfigureMode7RotationMatrix(void) {  // 0x80B0C2
  if (irq_enable_mode7) {
    if ((nmi_frame_counter_word & 7) == 0) {
      reg_M7B = kSinCosTable8bit_Sext[((uint8)mode7_rotation_angle) + 64];
      reg_M7C = -reg_M7B;
      reg_M7A = kSinCosTable8bit_Sext[((uint8)(mode7_rotation_angle + 64)) + 64];
      reg_M7D = reg_M7A;
      ++mode7_rotation_angle;
    }
  }
}


void LoadFromLoadStation(void) {  // 0x80C437
  save_station_lockout_flag = 1;
  const LoadStationList *L = (LoadStationList *)RomPtr_80(kLoadStationLists[area_index] + 14 * load_station_index);

  room_ptr = L->room_ptr_;
  door_def_ptr = L->door_ptr;
//  door_bts = v0[2];
  bg1_x_offset = layer1_x_pos = L->screen_x_pos;
  bg1_y_offset = layer1_y_pos = L->screen_y_pos;
  samus_y_pos = layer1_y_pos + L->samus_y_offset;
  samus_prev_y_pos = samus_y_pos;
  samus_x_pos = layer1_x_pos + 128 + L->samus_x_offset;
  samus_prev_x_pos = samus_x_pos;
  reg_BG1HOFS = 0;
  reg_BG1VOFS = 0;
  LOBYTE(area_index) = get_RoomDefHeader(room_ptr)->area_index_;
  LOBYTE(debug_disable_minimap) = 0;
}


void SetElevatorsAsUsed(void) {  // 0x80CD07
  const uint8 *v0 = RomPtr_80(off_80CD46[area_index] + 4 * ((elevator_door_properties_orientation & 0xF) - 1));
  used_save_stations_and_elevators[v0[0]] |= v0[1];
  used_save_stations_and_elevators[v0[2]] |= v0[3];
}
