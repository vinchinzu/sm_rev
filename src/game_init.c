#include <stdio.h>
#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "spc_player.h"

#define kInitialPalette ((uint16*)RomFixedPtr(0x9a8000))
#define kDemoRoomData ((uint16*)RomFixedPtr(0x82876c))

void CallDemoRoomDataFunc(uint32 ea) {
  switch (ea) {
  case fnDemoRoom_ChargeBeamRoomScroll21: DemoRoom_ChargeBeamRoomScroll21(); return;
  case fnnullsub_291: return;
  case fnDemoRoom_SetBG2TilemapBase: DemoRoom_SetBG2TilemapBase(); return;
  case fnDemoRoom_SetKraidFunctionTimer: DemoRoom_SetKraidFunctionTimer(); return;
  case fnDemoRoom_SetBrinstarBossBits: DemoRoom_SetBrinstarBossBits(); return;
  default: Unreachable();
  }
}

CoroutineRet InitAndLoadGameData_Async(void) {  // 0x828000
  int16 v0, v2, v4, v7;

  COROUTINE_BEGIN(coroutine_state_1, 0)

  if (game_state == kGameState_40_TransitionToDemo) {
    InitIoForGameplay();
    LoadStdBG3andSpriteTilesClearTilemaps();
    LoadInitialPalette();
    Samus_Initialize();
    LoadDemoRoomData();
  } else {
    if (loading_game_state != kLoadingGameState_5_Main) {
      if (loading_game_state == kLoadingGameState_1F_StartingAtCeres) {
        area_index = 6;
        load_station_index = 0;
        ClearTimerRam();
      } else if (loading_game_state == kLoadingGameState_22_EscapingCeres) {
        area_index = 0;
        load_station_index = 18;
        LoadMirrorOfExploredMapTiles();
      }
    }
    InitIoForGameplay();
    LoadStdBG3andSpriteTilesClearTilemaps();
    LoadInitialPalette();
    Samus_Initialize();
    LoadFromLoadStation();
  }
  COROUTINE_AWAIT(1, StartGameplay_Async());
  InitializeHud();
  {
    v0 = 32;
    uint16 v1 = 0;
    do {
      target_palettes[(v1 >> 1) + 192] = palette_buffer[(v1 >> 1) + 192];
      v1 += 2;
      v0 -= 2;
    } while (v0);
  }
  screen_fade_delay = 1;
  screen_fade_counter = 1;
  EnableNMI();
  EnableEprojs();
  EnablePLMs();
  EnablePaletteFx();
  EnableHdmaObjects();
  EnableAnimtiles();
  SetLiquidPhysicsType();
  if (game_state == kGameState_40_TransitionToDemo) {
    loop_counter_transfer_enemies_to_vram = 6;
    do {
      TransferEnemyTilesToVramAndInit();
      COROUTINE_AWAIT(2, WaitForNMI_Async());
      --loop_counter_transfer_enemies_to_vram;
    } while ((loop_counter_transfer_enemies_to_vram & 0x8000) == 0);
    uint16 r18 = get_DemoRoomData(kDemoRoomData[demo_set] + 18 * (demo_scene - 1))->demo_code_ptr;
    CallDemoRoomDataFunc(r18 | 0x820000);
    ++game_state;
    v7 = 512;
    uint16 v8 = 0;
    do {
      palette_buffer[v8 >> 1] = target_palettes[v8 >> 1];
      v8 += 2;
      v7 -= 2;
    } while (v7);
  } else if (loading_game_state == kLoadingGameState_22_EscapingCeres) {
    QueueMusic_Delayed8(5);
    loop_counter_transfer_enemies_to_vram = 15;
    do {
      TransferEnemyTilesToVramAndInit();
      COROUTINE_AWAIT(3, WaitForNMI_Async());
      --loop_counter_transfer_enemies_to_vram;
    } while ((loop_counter_transfer_enemies_to_vram & 0x8000) == 0);
    ++game_state;
    v2 = 512;
    uint16 v3 = 0;
    do {
      palette_buffer[v3 >> 1] = target_palettes[v3 >> 1];
      v3 += 2;
      v2 -= 2;
    } while (v2);
  } else {
    loop_counter_transfer_enemies_to_vram = 6;
    do {
      TransferEnemyTilesToVramAndInit();
      COROUTINE_AWAIT(4, WaitForNMI_Async());
      --loop_counter_transfer_enemies_to_vram;
    } while ((loop_counter_transfer_enemies_to_vram & 0x8000) == 0);
    game_state = kGameState_7_MainGameplayFadeIn;
    v4 = 512;
    uint16 v5 = 0;
    do {
      palette_buffer[v5 >> 1] = target_palettes[v5 >> 1];
      v5 += 2;
      v4 -= 2;
    } while (v4);
    if (loading_game_state == kLoadingGameState_1F_StartingAtCeres) {
      palette_buffer[223] = 0;
      CallSomeSamusCode(8);
    } else {
      CallSomeSamusCode(9);
    }
  }

  COROUTINE_END(0);
}

void InitCpuForGameplay(void) {  // 0x8281A4
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

void InitPpuForGameplay(void) {  // 0x8281DD
  WriteReg(INIDISP, 0x80);
  reg_INIDISP = 0x80;
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
  WriteReg(VMAIN, 0);
  WriteReg(W12SEL, 0);
  reg_W12SEL = 0;
  WriteReg(W12SEL, 0);
  reg_W12SEL = 0;
  WriteReg(W34SEL, 0);
  reg_W34SEL = 0;
  WriteReg(WOBJSEL, 0);
  reg_WOBJSEL = 0;
  WriteReg(WH0, 0);
  reg_WH0 = 0;
  WriteReg(WH1, 0);
  reg_WH1 = 0;
  WriteReg(WH2, 0);
  reg_WH2 = 0;
  WriteReg(WH3, 0);
  reg_WH3 = 0;
  WriteReg(WBGLOG, 0);
  reg_WBGLOG = 0;
  WriteReg(WOBJLOG, 0);
  reg_WOBJLOG = 0;
  WriteReg(TM, 0x17);
  reg_TM = 23;
  WriteReg(TMW, 0);
  reg_TMW = 0;
  WriteReg(TS, 0);
  reg_TS = 0;
  WriteReg(TSW, 0);
  reg_TSW = 0;
  WriteReg(CGWSEL, 0);
  next_gameplay_CGWSEL = 0;
  WriteReg(CGADSUB, 0);
  next_gameplay_CGADSUB = 0;
  WriteReg(COLDATA, 0xE0);
  WriteReg(SETINI, 0);
  reg_SETINI = 0;
  oam_next_ptr = 0;
  memset7E((uint16*)&ram3000, 0, 0x7FE);
  memset7E((uint16 *)&ram4000, 0x6F, 0x7FE);
  memset7E(ram4000.bg2_tilemap, 0x2C0F, 0xFE);
}

void InitIoForGameplay(void) {  // 0x82819B
  InitCpuForGameplay();
  InitPpuForGameplay();
}

void LoadInitialPalette(void) {  // 0x8282C5
  int16 v0 = 512;
  uint16 v1 = 0;
  do {
    palette_buffer[v1 >> 1] = kInitialPalette[v1 >> 1];
    v1 += 2;
    v0 -= 2;
  } while (v0);
}

void LoadStdBG3andSpriteTilesClearTilemaps(void) {  // 0x8282E2
  WriteReg(VMADDL, 0);
  WriteReg(VMADDH, 0x40);
  WriteReg(VMAIN, 0x80);
  static const StartDmaCopy unk_8282F8 = { 1, 1, 0x18, LONGPTR(0x9ab200), 0x2000 };
  SetupDmaTransfer(&unk_8282F8);
  WriteReg(MDMAEN, 2);
  WriteReg(VMADDL, 0);
  WriteReg(VMADDH, 0x60);
  WriteReg(VMAIN, 0x80);
  static const StartDmaCopy unk_828318 = { 1, 1, 0x18, LONGPTR(0x9ad200), 0x2e00 };
  SetupDmaTransfer(&unk_828318);
  WriteReg(MDMAEN, 2);
  WriteReg(VMADDL, 0);
  WriteReg(VMADDH, 0x50);
  WriteReg(VMAIN, 0x80);
  static const StartDmaCopy unk_828338 = { 1, 1, 0x18, LONGPTR(0x7e4000), 0x1000 };
  SetupDmaTransfer(&unk_828338);
  WriteReg(MDMAEN, 2);
  WriteReg(VMADDL, 0);
  WriteReg(VMADDH, 0x58);
  WriteReg(VMAIN, 0x80);
  static const StartDmaCopy unk_828358 = { 1, 1, 0x18, LONGPTR(0x7e4000), 0x0800 };
  SetupDmaTransfer(&unk_828358);
  WriteReg(MDMAEN, 2);
}

void APU_UploadBank(uint32 addr) {  // 0x808028
  if (!g_use_my_apu_code)
    return;
  RtlApuUpload(RomPtr(addr));
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
