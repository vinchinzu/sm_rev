// System routines 

#include <stdio.h>
#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "enemy_types.h"
#include "spc_player.h"


#define kTimerDigitsSpritemapPtr ((uint16*)RomFixedPtr(0x809fd4))
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


void IrqHandler_SetResult(uint16 a, uint16 y, uint16 x) {
  //  printf("Setting irq next: %d, %d, %d\n", a, x, y);
  cur_irq_handler = a;
  WriteRegWord(VTIMEL, y);
  WriteRegWord(HTIMEL, x);
}

void IrqHandler_0_Nothing(void) {  // 0x80966E
  uint16 a = irqhandler_next_handler;
  irqhandler_next_handler = 0;
  IrqHandler_SetResult(a, 0, 0);
}

void IrqHandler_2_DisableIRQ(void) {  // 0x809680
  *(uint16 *)&reg_NMITIMEN &= ~0x30;
  IrqHandler_SetResult(0, 0, 0);
}

void IrqHandler_4_Main_BeginHudDraw(void) {  // 0x80968B
  WriteReg(BG3SC, 0x5A);
  WriteReg(CGWSEL, 0);
  WriteReg(CGADSUB, 0);
  WriteReg(TM, 4);
  IrqHandler_SetResult(6, 31, 152);
}

void IrqHandler_6_Main_EndHudDraw(void) {  // 0x8096A9
  WriteReg(CGWSEL, gameplay_CGWSEL);
  WriteReg(CGADSUB, gameplay_CGADSUB);
  WriteReg(BG3SC, gameplay_BG3SC);
  WriteReg(TM, gameplay_TM);

  uint16 a = irqhandler_next_handler;
  irqhandler_next_handler = 0;
  IrqHandler_SetResult(a ? a : 4, 0, 152);
}

void IrqHandler_8_StartOfDoor_BeginHud(void) {  // 0x8096D3
  WriteReg(BG3SC, 0x5A);
  WriteReg(TM, 4);
  WriteReg(CGWSEL, 0);
  WriteReg(CGADSUB, 0);
  IrqHandler_SetResult(10, 31, 152);
}

void IrqHandler_10_StartOfDoor_EndHud(void) {  // 0x8096F1
  uint8 v0;
  if (((previous_cre_bitset | cre_bitset) & 1) != 0)
    v0 = 16;
  else
    v0 = 17;
  WriteReg(TM, v0);
  uint16 a = irqhandler_next_handler;
  irqhandler_next_handler = 0;
  IrqHandler_SetResult(a ? a : 8, 0, 152);
}

void IrqHandler_12_Draygon_BeginHud(void) {  // 0x80971A
  WriteReg(TM, 4);
  WriteReg(CGWSEL, 0);
  WriteReg(CGADSUB, 0);
  IrqHandler_SetResult(14, 31, 152);
}

void IrqHandler_14_Draygon_EndHud(void) {  // 0x809733
  WriteReg(BG3SC, gameplay_BG3SC);
  WriteReg(CGWSEL, gameplay_CGWSEL);
  WriteReg(CGADSUB, gameplay_CGADSUB);
  uint16 a = irqhandler_next_handler;
  irqhandler_next_handler = 0;
  IrqHandler_SetResult(a ? a : 12, 0, 152);
}

void IrqHandler_16_VerticalDoor_BeginHud(void) {  // 0x809758
  WriteReg(TM, 4);
  WriteReg(CGWSEL, 0);
  WriteReg(CGADSUB, 0);
  IrqHandler_SetResult(18, 31, 152);
}

void IrqHandler_18_VerticalDoor_EndHud(void) {  // 0x809771
  uint8 v0;
  if (((previous_cre_bitset | cre_bitset) & 1) != 0)
    v0 = 16;
  else
    v0 = 17;
  WriteReg(TM, v0);
  WriteReg(CGWSEL, 0);
  WriteReg(CGADSUB, 0);
  if ((door_transition_flag & 0x8000) == 0)
    Irq_FollowDoorTransition();
  IrqHandler_SetResult(20, 216, 152);
}

void IrqHandler_20_VerticalDoor_EndDraw(void) {  // 0x8097A9
  uint16 a = irqhandler_next_handler;
  irqhandler_next_handler = 0;
  IrqHandler_SetResult(a ? a : 16, 0, 152);
}

void IrqHandler_22_HorizDoor_BeginHud(void) {  // 0x8097C1
  WriteReg(TM, 4);
  WriteReg(CGWSEL, 0);
  WriteReg(CGADSUB, 0);
  IrqHandler_SetResult(24, 31, 152);
}

void IrqHandler_24_HorizDoor_EndHud(void) {  // 0x8097DA
  uint8 v0;

  if (((previous_cre_bitset | cre_bitset) & 1) != 0)
    v0 = 16;
  else
    v0 = 17;
  WriteReg(TM, v0);
  WriteReg(CGWSEL, 0);
  WriteReg(CGADSUB, 0);
  if ((door_transition_flag & 0x8000) == 0)
    Irq_FollowDoorTransition();
  IrqHandler_SetResult(26, 160, 152);
}

void IrqHandler_26_HorizDoor_EndDraw(void) {  // 0x80980A
  uint16 a = irqhandler_next_handler;
  irqhandler_next_handler = 0;
  IrqHandler_SetResult(a ? a : 22, 0, 152);
}

void EnableIrqInterrupts(void) {  // 0x80982A
  WriteRegWord(VTIMEL, 0);
  WriteRegWord(HTIMEL, 152);
  *(uint16 *)&reg_NMITIMEN |= 0x30;
}

void EnableIrqInterruptsNow(void) {  // 0x809841
  WriteRegWord(VTIMEL, 0);
  WriteRegWord(HTIMEL, 152);
  *(uint16 *)&reg_NMITIMEN |= 0x30;
  WriteReg(NMITIMEN, reg_NMITIMEN);
}

void DisableIrqInterrupts(void) {  // 0x80985F
  *(uint16 *)&reg_NMITIMEN &= ~0x30;
}

static Func_V *const kIrqHandlers[14] = {  // 0x80986A
  IrqHandler_0_Nothing,
  IrqHandler_2_DisableIRQ,
  IrqHandler_4_Main_BeginHudDraw,
  IrqHandler_6_Main_EndHudDraw,
  IrqHandler_8_StartOfDoor_BeginHud,
  IrqHandler_10_StartOfDoor_EndHud,
  IrqHandler_12_Draygon_BeginHud,
  IrqHandler_14_Draygon_EndHud,
  IrqHandler_16_VerticalDoor_BeginHud,
  IrqHandler_18_VerticalDoor_EndHud,
  IrqHandler_20_VerticalDoor_EndDraw,
  IrqHandler_22_HorizDoor_BeginHud,
  IrqHandler_24_HorizDoor_EndHud,
  IrqHandler_26_HorizDoor_EndDraw,
};

void Vector_IRQ(void) {
  kIrqHandlers[cur_irq_handler >> 1]();
}

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

static Func_U8 *const kTimerProcessFuncs[7] = {  // 0x809DE7
  ProcessTimer_Empty,
  ProcessTimer_CeresStart,
  ProcessTimer_MotherBrainStart,
  ProcessTimer_InitialDelay,
  ProcessTimer_MovementDelayed,
  ProcessTimer_MovingIntoPlace,
  ProcessTimer_Decrement,
};

uint8 ProcessTimer(void) {
  return kTimerProcessFuncs[(uint8)timer_status]();
}

uint8 ProcessTimer_CeresStart(void) {  // 0x809E09
  ClearTimerRam();
  SetTimerMinutes(0x100);
  timer_status = -32765;
  return 0;
}

uint8 ProcessTimer_Empty(void) {  // 0x809E1A
  return 0;
}

uint8 ProcessTimer_MotherBrainStart(void) {  // 0x809E1C
  ClearTimerRam();
  SetTimerMinutes(0x300);
  timer_status = -32765;
  return 0;
}

uint8 ProcessTimer_InitialDelay(void) {  // 0x809E2F
  LOBYTE(timer_x_pos) = timer_x_pos + 1;
  if ((uint8)timer_x_pos >= 0x10)
    LOBYTE(timer_status) = timer_status + 1;
  return 0;
}

uint8 ProcessTimer_MovementDelayed(void) {  // 0x809E41
  LOBYTE(timer_x_pos) = timer_x_pos + 1;
  if ((uint8)timer_x_pos >= 0x60) {
    LOBYTE(timer_x_pos) = 0;
    LOBYTE(timer_status) = timer_status + 1;
  }
  return ProcessTimer_Decrement();
}

uint8 ProcessTimer_MovingIntoPlace(void) {  // 0x809E58
  int v0 = 0;
  uint16 v1 = timer_x_pos + 224;
  if ((uint16)(timer_x_pos + 224) >= 0xDC00) {
    v0 = 1;
    v1 = -9216;
  }
  timer_x_pos = v1;
  uint16 v2 = timer_y_pos - 193;
  if ((uint16)(timer_y_pos - 193) < 0x3000) {
    ++v0;
    v2 = 12288;
  }
  timer_y_pos = v2;
  if (v0 == 2)
    ++timer_status;
  return ProcessTimer_Decrement();
}

void SetTimerMinutes(uint16 a) {  // 0x809E8C
  *(uint16 *)&timer_centiseconds = 0;
  *(uint16 *)&timer_seconds = a;
}

void ClearTimerRam(void) {  // 0x809E93
  timer_x_pos = 0x8000;
  timer_y_pos = 0x8000;
  *(uint16 *)&timer_centiseconds = 0;
  *(uint16 *)&timer_seconds = 0;
  timer_status = 0;
}

bool DecrementDecimal(uint8 *number, uint8 value) {
  int result = (*number & 0xf) + (~value & 0xf) + 1;
  if (result < 0x10) result = (result - 0x6) & ((result - 0x6 < 0) ? 0xf : 0x1f);
  result = (*number & 0xf0) + (~value & 0xf0) + result;
  if (result < 0x100) result -= 0x60;
  *number = result;
  return result < 0x100;
}

uint8 ProcessTimer_Decrement(void) {  // 0x809EA9
  static const uint8 kTimerCentisecondDecrements[128] = {
    1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 2, 1, 2, 2,
    1, 2, 2, 1, 2, 2, 1, 2, 1, 2, 2, 1, 2, 2, 1, 2,
    1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 2, 1, 2, 2,
    1, 2, 2, 1, 2, 2, 1, 2, 1, 2, 2, 1, 2, 2, 1, 2,
    1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 2, 1, 2, 2,
    1, 2, 2, 1, 2, 2, 1, 2, 1, 2, 2, 1, 2, 2, 1, 2,
    1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 2, 1, 2, 2,
    1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 2, 1, 2, 2,
  };
  if (DecrementDecimal(&timer_centiseconds, kTimerCentisecondDecrements[nmi_frame_counter_word & 0x7f])) {
    if (DecrementDecimal(&timer_seconds, 1)) {
      if (DecrementDecimal(&timer_minutes, 1)) {
        timer_centiseconds = 0;
        timer_seconds = 0;
        timer_minutes = 0;
      } else {
        timer_seconds = 0x59;
      }
    }
  }
  return (timer_centiseconds | timer_seconds | timer_minutes) == 0;

}

void DrawTimer(void) {  // 0x809F6C
  DrawTimerSpritemap(0, addr_word_80A060);
  DrawTwoTimerDigits(*(uint16 *)&timer_minutes, 0xFFE4);
  DrawTwoTimerDigits(*(uint16 *)&timer_seconds, 0xFFFC);
  DrawTwoTimerDigits(*(uint16 *)&timer_centiseconds, 0x14);
}


void DrawTwoTimerDigits(uint16 a, uint16 k) {  // 0x809F95
  DrawTimerSpritemap(k, kTimerDigitsSpritemapPtr[(a & 0xF0) >> 4]);
  DrawTimerSpritemap(k + 8, kTimerDigitsSpritemapPtr[a & 0xF]);
}

void DrawTimerSpritemap(uint16 a, uint16 j) {  // 0x809FB3
  DrawSpritemap(0x80, j, a + HIBYTE(timer_x_pos), HIBYTE(timer_y_pos), 2560);
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

static uint32 decompress_src;

static uint8 DecompNextByte() {
  uint8 b = *RomPtr(decompress_src);
  if ((decompress_src++ & 0xffff) == 0xffff)
    decompress_src += 0x8000;
  return b;
}

void DecompressToMem(uint32 src, uint8 *decompress_dst) {  // 0x80B119
  decompress_src = src;

  int src_pos, dst_pos = 0;
  while (1) {
    int len;
    uint8 cmd, b;
    b = DecompNextByte();
    if (b == 0xFF)
      break;
    if ((b & 0xE0) == 0xE0) {
      cmd = (8 * b) & 0xE0;
      len = ((b & 3) << 8 | DecompNextByte()) + 1;
    } else {
      cmd = b & 0xE0;
      len = (b & 0x1F) + 1;
    }
    if (cmd & 0x80) {
      uint8 want_xor = cmd & 0x20 ? 0xff : 0;
      if (cmd >= 0xC0) {
        src_pos = dst_pos - DecompNextByte();
      } else {
        src_pos = DecompNextByte();
        src_pos += DecompNextByte() * 256;
      }
      do {
        b = decompress_dst[src_pos++] ^ want_xor;
        decompress_dst[dst_pos++] = b;
      } while (--len);
    } else {
      switch (cmd) {
      case 0x20:
        b = DecompNextByte();
        do {
          decompress_dst[dst_pos++] = b;
        } while (--len);
        break;
      case 0x40: {
        b = DecompNextByte();
        uint8 b2 = DecompNextByte();
        do {
          decompress_dst[dst_pos++] = b;
          if (!--len)
            break;
          decompress_dst[dst_pos++] = b2;
        } while (--len);
        break;
      }
      case 0x60:
        b = DecompNextByte();
        do {
          decompress_dst[dst_pos++] = b++;
        } while (--len);
        break;
      default:
        do {
          b = DecompNextByte();
          decompress_dst[dst_pos++] = b;
        } while (--len);
        break;
      }
    }
  }
}

static uint8 ReadPpuByte(uint16 addr) {
  WriteRegWord(VMADDL, addr >> 1);
  ReadRegWord(RDVRAML);  // latch
  uint16 data = ReadRegWord(RDVRAML);
  return (addr & 1) ? GET_HIBYTE(data) : data;
}

void DecompressToVRAM(uint32 src, uint16 dst_addr) {  // 0x80B271
  decompress_src = src;
  int src_pos, dst_pos = dst_addr;
  while (1) {
    int len;
    uint8 b = DecompNextByte(), cmd;
    if (b == 0xFF)
      break;
    if ((b & 0xE0) == 0xE0) {
      cmd = (8 * b) & 0xE0;
      len = ((b & 3) << 8 | DecompNextByte()) + 1;
    } else {
      cmd = b & 0xE0;
      len = (b & 0x1F) + 1;
    }
    if (cmd & 0x80) {
      uint8 want_xor = cmd & 0x20 ? 0xff : 0;
      if (cmd >= 0xC0) {
        src_pos = dst_pos - DecompNextByte();
      } else {
        src_pos = DecompNextByte();
        src_pos += DecompNextByte() * 256;
        src_pos += dst_addr;
      }
      do {
        b = ReadPpuByte(src_pos++) ^ want_xor;
        WriteRegWord(VMADDL, dst_pos >> 1);
        WriteReg(VMDATAL + (dst_pos++ & 1), b);
      } while (--len);
    } else {
      switch (cmd) {
      case 0x20:
        b = DecompNextByte();
        do {
          WriteReg(VMDATAL + (dst_pos++ & 1), b);
        } while (--len);
        break;
      case 0x40: {
        b = DecompNextByte();
        uint8 b2 = DecompNextByte();
        do {
          WriteReg(VMDATAL + (dst_pos++ & 1), b);
          if (!--len)
            break;
          WriteReg(VMDATAL + (dst_pos++ & 1), b2);
        } while (--len);
        break;
      }
      case 0x60:
        b = DecompNextByte();
        do {
          WriteReg(VMDATAL + (dst_pos++ & 1), b++);
        } while (--len);
        break;
      default:
        do {
          b = DecompNextByte();
          WriteReg(VMDATAL + (dst_pos++ & 1), b++);
        } while (--len);
        break;
      }
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
