// Door setup callbacks and scroll mutators extracted from Bank $8F.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"

enum RoomScrollColor {
  kRoomScroll_Red = 0,
  kRoomScroll_Blue = 1,
  kRoomScroll_Green = 2,
};

enum {
  kRoomMainAsmVar0 = 0,
  kRoomMainAsmVar2 = 2,
  kElevatubeVelocity = 4,
  kElevatubeAcceleration = 6,
  kCeresElevatorRotationStep = 34,
  kCeresElevatorFrameDelay = 60,
};

static inline void SetScrollColor(uint16 scroll_idx, uint8 color) {
  scrolls[scroll_idx] = color;
}

static inline void WriteRoomMainAsmWord(uint16 offset, uint16 value) {
  *(uint16 *)(room_main_asm_variables + offset) = value;
}

static void DoorCode_StartWreckedShipTreadmillWest(void) {  // 0x8FB971
  SpawnAnimtiles(addr_kAnimtiles_WreckedShipTradmillRight);
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x04, 0x09, 0xb64b });
}

static void DoorCode_Scroll6_Green(void) {  // 0x8FB981
  SetScrollColor(6, kRoomScroll_Green);
}

static void DoorCode_Scroll0_Blue(void) {  // 0x8FB98C
  SetScrollColor(0, kRoomScroll_Blue);
}

static void DoorCode_Scroll13_Blue(void) {  // 0x8FB997
  SetScrollColor(19, kRoomScroll_Blue);
}

static void DoorCode_Scroll_4Blue_8Green(void) {  // 0x8FB9A2
  SetScrollColor(4, kRoomScroll_Red);
  SetScrollColor(8, kRoomScroll_Green);
}

static void DoorCode_Scroll_8toB_Red(void) {  // 0x8FB9B3
  SetScrollColor(8, kRoomScroll_Red);
  SetScrollColor(9, kRoomScroll_Red);
  SetScrollColor(10, kRoomScroll_Red);
  SetScrollColor(11, kRoomScroll_Red);
}

static void DoorCode_Scroll_LotsRed(void) {  // 0x8FB9CA
  SetScrollColor(2, kRoomScroll_Red);
  SetScrollColor(3, kRoomScroll_Red);
  SetScrollColor(4, kRoomScroll_Red);
  SetScrollColor(5, kRoomScroll_Red);
  SetScrollColor(11, kRoomScroll_Red);
  SetScrollColor(12, kRoomScroll_Red);
  SetScrollColor(13, kRoomScroll_Red);
  SetScrollColor(17, kRoomScroll_Red);
}

static void DoorCode_Scroll_1_4_Green(void) {  // 0x8FB9F1
  SetScrollColor(1, kRoomScroll_Green);
  SetScrollColor(4, kRoomScroll_Green);
}

static void DoorCode_Scroll_2_Blue(void) {  // 0x8FBA00
  SetScrollColor(2, kRoomScroll_Blue);
}

static void DoorCode_Scroll_17_Blue(void) {  // 0x8FBA0B
  SetScrollColor(23, kRoomScroll_Blue);
}

static void DoorCode_Scroll_4_Blue(void) {  // 0x8FBA16
  SetScrollColor(4, kRoomScroll_Blue);
}

static void DoorCode_Scroll_6_Green(void) {  // 0x8FBA21
  SetScrollColor(6, kRoomScroll_Green);
}

static void DoorCode_Scroll_3_Green(void) {  // 0x8FBA2C
  SetScrollColor(3, kRoomScroll_Green);
}

static void DoorCode_SetScroll_0(void) {  // 0x8FBD07
  scrolls[24] = 2;
  scrolls[28] = 2;
}

static void DoorCode_SetScroll_1(void) {  // 0x8FBD16
  scrolls[5] = 1;
  scrolls[6] = 1;
}

static void DoorCode_SetScroll_2(void) {  // 0x8FBD25
  scrolls[29] = 1;
}

static void DoorCode_SetScroll_3(void) {  // 0x8FBD30
  scrolls[2] = 2;
  scrolls[3] = 2;
}

static void DoorCode_SetScroll_4(void) {  // 0x8FBD3F
  scrolls[0] = 0;
  scrolls[1] = 2;
}

static void DoorCode_SetScroll_5(void) {  // 0x8FBD50
  scrolls[11] = 2;
}

static void DoorCode_SetScroll_6(void) {  // 0x8FBD5B
  scrolls[28] = 0;
  scrolls[29] = 1;
}

static void DoorCode_SetScroll_7(void) {  // 0x8FBD6C
  scrolls[4] = 0;
}

static void DoorCode_SetScroll_8(void) {  // 0x8FBD77
  scrolls[32] = 2;
  scrolls[36] = 2;
  scrolls[37] = 2;
}

static void DoorCode_SetScroll_9(void) {  // 0x8FBD8A
  scrolls[2] = 1;
}

static void DoorCode_SetScroll_10(void) {  // 0x8FBD95
  scrolls[0] = 2;
}

static void DoorCode_SetScroll_11(void) {  // 0x8FBDA0
  scrolls[6] = 2;
  scrolls[7] = 2;
}

static void DoorCode_SetScroll_12(void) {  // 0x8FBDAF
  scrolls[1] = 1;
  scrolls[2] = 0;
}

static void DoorCode_SetScroll_13(void) {  // 0x8FBDC0
  scrolls[1] = 1;
  scrolls[3] = 0;
}

static void DoorCode_SetScroll_14(void) {  // 0x8FBDD1
  scrolls[0] = 0;
  scrolls[4] = 1;
}

static void DoorCode_SetScroll_15(void) {  // 0x8FBDE2
  scrolls[2] = 1;
  scrolls[3] = 1;
}

static void DoorCode_SetScroll_16(void) {  // 0x8FBDF1
  scrolls[0] = 2;
  scrolls[1] = 2;
}

static void DoorCode_SetScroll_17(void) {  // 0x8FBE00
  scrolls[1] = 2;
}

static void DoorCode_SetScroll_18(void) {  // 0x8FBE0B
  scrolls[15] = 2;
  scrolls[18] = 2;
}

static void DoorCode_SetScroll_19(void) {  // 0x8FBE1A
  scrolls[6] = 2;
}

static void DoorCode_SetScroll_20(void) {  // 0x8FBE25
  scrolls[0] = 2;
  scrolls[1] = 1;
}

static void DoorCode_SetScroll_21(void) {  // 0x8FBE36
  scrolls[2] = 2;
}

static void DoorCode_SetScroll_22(void) {  // 0x8FBF9E
  scrolls[3] = 0;
  scrolls[4] = 0;
  scrolls[6] = 1;
  scrolls[7] = 1;
  scrolls[8] = 1;
}

static void DoorCode_SetScroll_23(void) {  // 0x8FBFBB
  scrolls[1] = 1;
  scrolls[2] = 1;
  scrolls[3] = 1;
  scrolls[4] = 2;
  scrolls[6] = 0;
}

static void DoorCode_SetScroll_24(void) {  // 0x8FBFDA
  scrolls[0] = 1;
  scrolls[1] = 1;
}

static void DoorCode_SetScroll_25(void) {  // 0x8FBFE9
  scrolls[1] = 0;
  scrolls[0] = 1;
}

static void DoorCode_SetScroll_26(void) {  // 0x8FBFFA
  scrolls[10] = 2;
}

static void DoorCode_SetScroll_27(void) {  // 0x8FC005
  scrolls[0] = 1;
  scrolls[2] = 0;
}

static void DoorCode_SetScroll_28(void) {  // 0x8FC016
  scrolls[0] = 2;
  scrolls[2] = 2;
}

static void DoorCode_SetScroll_29(void) {  // 0x8FC025
  scrolls[6] = 1;
  scrolls[7] = 1;
  scrolls[8] = 0;
}

static void DoorCode_SetScroll_30(void) {  // 0x8FC03A
  scrolls[2] = 0;
  scrolls[3] = 1;
}

static void DoorCode_SetScroll_31(void) {  // 0x8FC04B
  scrolls[7] = 2;
}

static void DoorCode_SetScroll_32(void) {  // 0x8FC056
  scrolls[1] = 0;
  scrolls[2] = 1;
}

static void DoorCode_SetScroll_33(void) {  // 0x8FC067
  scrolls[3] = 0;
  scrolls[0] = 1;
}

static void DoorCode_SetScroll_34(void) {  // 0x8FC078
  scrolls[1] = 1;
  scrolls[4] = 0;
}

static void DoorCode_SetScroll_35(void) {  // 0x8FC089
  scrolls[0] = 1;
  scrolls[1] = 0;
  scrolls[2] = 0;
  scrolls[3] = 0;
}

static void DoorCode_SetScroll_36(void) {  // 0x8FC0A2
  scrolls[0] = 2;
}

static void DoorCode_SetScroll_37(void) {  // 0x8FC0AD
  scrolls[0] = 1;
  scrolls[1] = 1;
  scrolls[4] = 0;
}

static void DoorCode_SetScroll_38(void) {  // 0x8FC0C2
  scrolls[0] = 1;
  scrolls[3] = 0;
}

static void DoorCode_SetScroll_39(void) {  // 0x8FC0D3
  scrolls[0] = 1;
}

static void DoorCode_SetScroll_40(void) {  // 0x8FC0DE
  scrolls[0] = 1;
  scrolls[1] = 0;
}

static void DoorCode_SetScroll_41(void) {  // 0x8FC0EF
  scrolls[24] = 1;
}

static void DoorCode_SetScroll_42(void) {  // 0x8FC0FA
  scrolls[3] = 0;
  scrolls[2] = 1;
}

static void DoorCode_SetScroll_43(void) {  // 0x8FC10B
  scrolls[14] = 0;
}

static void DoorCode_StartWreckedSkipTreadmill_East(void) {  // 0x8FE1D8
  SpawnAnimtiles(addr_kAnimtiles_WreckedShipTradmillLeft);
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x04, 0x09, 0xb64f });
}

static void DoorCode_SetScroll_44(void) {  // 0x8FE1E8
  scrolls[1] = 1;
}

static void DoorCode_SetScroll_45(void) {  // 0x8FE1F3
  scrolls[0] = 2;
}

static void DoorCode_SetScroll_46(void) {  // 0x8FE1FE
  scrolls[3] = 0;
  scrolls[4] = 1;
}

static void DoorCode_SetScroll_47(void) {  // 0x8FE20F
  scrolls[41] = 1;
}

static void DoorCode_SetScroll_48(void) {  // 0x8FE21A
  scrolls[40] = 2;
  scrolls[46] = 2;
}

static void DoorCode_SetScroll_49(void) {  // 0x8FE229
  scrolls[6] = 0;
  scrolls[7] = 0;
  scrolls[8] = 0;
  scrolls[9] = 0;
  scrolls[10] = 0;
  scrolls[11] = 0;
}

static void DoorCode_SetupElevatubeFromSouth(void) {  // 0x8FE26C
  WriteRoomMainAsmWord(kElevatubeVelocity, -256);
  WriteRoomMainAsmWord(kRoomMainAsmVar2, 2496);
  WriteRoomMainAsmWord(kElevatubeAcceleration, -32);
  CallSomeSamusCode(0);
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x01, 0x00, 0xb8f9 });
}

static void DoorCode_SetupElevatubeFromNorth(void) {  // 0x8FE291
  WriteRoomMainAsmWord(kElevatubeVelocity, 256);
  WriteRoomMainAsmWord(kRoomMainAsmVar2, '@');
  WriteRoomMainAsmWord(kElevatubeAcceleration, ' ');
  CallSomeSamusCode(0);
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x01, 0x00, 0xb8f9 });
}

static void DoorCode_ResetElevatubeNorthExit(void) {  // 0x8FE301
  CallSomeSamusCode(1);
}

static void DoorCode_ResetElevatubeSouthExit(void) {  // 0x8FE309
  SetScrollColor(0, kRoomScroll_Green);
  SetScrollColor(1, kRoomScroll_Blue);
  CallSomeSamusCode(1);
}

static void DoorCode_SetScroll_50(void) {  // 0x8FE318
  scrolls[10] = 0;
  scrolls[11] = 1;
}

static void DoorCode_SetScroll_53(void) {  // 0x8FE345
  scrolls[0] = 0;
  scrolls[4] = 1;
}

static void DoorCode_SetScroll_54(void) {  // 0x8FE356
  scrolls[0] = 0;
  scrolls[1] = 1;
}

static void DoorCode_SetScroll_55(void) {  // 0x8FE367
  scrolls[9] = 0;
  scrolls[10] = 1;
}

static void DoorCode_SetScroll_56(void) {  // 0x8FE378
  scrolls[0] = 0;
  scrolls[2] = 0;
  scrolls[1] = 1;
}

static void DoorCode_SetScroll_57(void) {  // 0x8FE38D
  scrolls[1] = 1;
}

static void DoorCode_SetScroll_58(void) {  // 0x8FE398
  scrolls[6] = 1;
}

static void DoorCode_SetScroll_59(void) {  // 0x8FE3A3
  scrolls[4] = 0;
}

static void DoorCode_SetScroll_61(void) {  // 0x8FE3B9
  scrolls[4] = 0;
  scrolls[7] = 0;
}

static void DoorCode_SetScroll_62(void) {  // 0x8FE3C8
  scrolls[2] = 0;
  scrolls[1] = 1;
}

static void DoorCode_SetScroll_63(void) {  // 0x8FE3D9
  scrolls[0] = 2;
  scrolls[2] = 2;
}

static void DoorCode_SetScroll_64(void) {  // 0x8FE4C0
  scrolls[0] = 2;
  scrolls[1] = 2;
}

static void DoorCode_SetScroll_65(void) {  // 0x8FE4CF
  scrolls[24] = 1;
  scrolls[25] = 0;
}

static void DoorCode_CeresElevatorShaft(void) {  // 0x8FE4E0
  reg_BGMODE_fake = 7;
  WriteReg(BGMODE, 7);
  reg_M7A = 256;
  reg_M7D = 256;
  reg_M7B = 0;
  reg_M7C = 0;
  reg_M7X = 128;
  reg_M7Y = 1008;
  irq_enable_mode7 = 1;
  WriteRoomMainAsmWord(kRoomMainAsmVar0, kCeresElevatorRotationStep);
  WriteRoomMainAsmWord(kRoomMainAsmVar2, kCeresElevatorFrameDelay);
}

static void DoorCode_CeresElevatorShaft_2(void) {  // 0x8FE513
  reg_BGMODE_fake = 9;
  irq_enable_mode7 = 0;
}

static void CallDoorDefSetupCode(uint32 ea) {
  switch (ea) {
  case fnDoorCode_StartWreckedShipTreadmillWest: DoorCode_StartWreckedShipTreadmillWest(); return;
  case fnDoorCode_Scroll6_Green: DoorCode_Scroll6_Green(); return;
  case fnDoorCode_Scroll0_Blue: DoorCode_Scroll0_Blue(); return;
  case fnDoorCode_Scroll13_Blue: DoorCode_Scroll13_Blue(); return;
  case fnDoorCode_Scroll_4Blue_8Green: DoorCode_Scroll_4Blue_8Green(); return;
  case fnDoorCode_Scroll_8toB_Red: DoorCode_Scroll_8toB_Red(); return;
  case fnDoorCode_Scroll_LotsRed: DoorCode_Scroll_LotsRed(); return;
  case fnDoorCode_Scroll_1_4_Green: DoorCode_Scroll_1_4_Green(); return;
  case fnDoorCode_Scroll_2_Blue: DoorCode_Scroll_2_Blue(); return;
  case fnDoorCode_Scroll_17_Blue: DoorCode_Scroll_17_Blue(); return;
  case fnDoorCode_Scroll_4_Blue: DoorCode_Scroll_4_Blue(); return;
  case fnDoorCode_Scroll_6_Green: DoorCode_Scroll_6_Green(); return;
  case fnDoorCode_Scroll_3_Green: DoorCode_Scroll_3_Green(); return;
  case fnDoorCode_SetScroll_0: DoorCode_SetScroll_0(); return;
  case fnDoorCode_SetScroll_1: DoorCode_SetScroll_1(); return;
  case fnDoorCode_SetScroll_2: DoorCode_SetScroll_2(); return;
  case fnDoorCode_SetScroll_3: DoorCode_SetScroll_3(); return;
  case fnDoorCode_SetScroll_4: DoorCode_SetScroll_4(); return;
  case fnDoorCode_SetScroll_5: DoorCode_SetScroll_5(); return;
  case fnDoorCode_SetScroll_6: DoorCode_SetScroll_6(); return;
  case fnDoorCode_SetScroll_7: DoorCode_SetScroll_7(); return;
  case fnDoorCode_SetScroll_8: DoorCode_SetScroll_8(); return;
  case fnDoorCode_SetScroll_9: DoorCode_SetScroll_9(); return;
  case fnDoorCode_SetScroll_10: DoorCode_SetScroll_10(); return;
  case fnDoorCode_SetScroll_11: DoorCode_SetScroll_11(); return;
  case fnDoorCode_SetScroll_12: DoorCode_SetScroll_12(); return;
  case fnDoorCode_SetScroll_13: DoorCode_SetScroll_13(); return;
  case fnDoorCode_SetScroll_14: DoorCode_SetScroll_14(); return;
  case fnDoorCode_SetScroll_15: DoorCode_SetScroll_15(); return;
  case fnDoorCode_SetScroll_16: DoorCode_SetScroll_16(); return;
  case fnDoorCode_SetScroll_17: DoorCode_SetScroll_17(); return;
  case fnDoorCode_SetScroll_18: DoorCode_SetScroll_18(); return;
  case fnDoorCode_SetScroll_19: DoorCode_SetScroll_19(); return;
  case fnDoorCode_SetScroll_20: DoorCode_SetScroll_20(); return;
  case fnDoorCode_SetScroll_21: DoorCode_SetScroll_21(); return;
  case fnDoorCode_SetScroll_22: DoorCode_SetScroll_22(); return;
  case fnDoorCode_SetScroll_23: DoorCode_SetScroll_23(); return;
  case fnDoorCode_SetScroll_24: DoorCode_SetScroll_24(); return;
  case fnDoorCode_SetScroll_25: DoorCode_SetScroll_25(); return;
  case fnDoorCode_SetScroll_26: DoorCode_SetScroll_26(); return;
  case fnDoorCode_SetScroll_28: DoorCode_SetScroll_28(); return;
  case fnDoorCode_SetScroll_29: DoorCode_SetScroll_29(); return;
  case fnDoorCode_SetScroll_30: DoorCode_SetScroll_30(); return;
  case fnDoorCode_SetScroll_31: DoorCode_SetScroll_31(); return;
  case fnDoorCode_SetScroll_32: DoorCode_SetScroll_32(); return;
  case fnDoorCode_SetScroll_33: DoorCode_SetScroll_33(); return;
  case fnDoorCode_SetScroll_34: DoorCode_SetScroll_34(); return;
  case fnDoorCode_SetScroll_35: DoorCode_SetScroll_35(); return;
  case fnDoorCode_SetScroll_36: DoorCode_SetScroll_36(); return;
  case fnDoorCode_SetScroll_37: DoorCode_SetScroll_37(); return;
  case fnDoorCode_SetScroll_38: DoorCode_SetScroll_38(); return;
  case fnDoorCode_SetScroll_39: DoorCode_SetScroll_39(); return;
  case fnDoorCode_SetScroll_40: DoorCode_SetScroll_40(); return;
  case fnDoorCode_SetScroll_41: DoorCode_SetScroll_41(); return;
  case fnDoorCode_SetScroll_42: DoorCode_SetScroll_42(); return;
  case fnDoorCode_SetScroll_43: DoorCode_SetScroll_43(); return;
  case fnDoorCode_StartWreckedSkipTreadmill_East: DoorCode_StartWreckedSkipTreadmill_East(); return;
  case fnDoorCode_SetScroll_44: DoorCode_SetScroll_44(); return;
  case fnDoorCode_SetScroll_45: DoorCode_SetScroll_45(); return;
  case fnDoorCode_SetScroll_46: DoorCode_SetScroll_46(); return;
  case fnDoorCode_SetScroll_47: DoorCode_SetScroll_47(); return;
  case fnDoorCode_SetScroll_48: DoorCode_SetScroll_48(); return;
  case fnDoorCode_SetScroll_49: DoorCode_SetScroll_49(); return;
  case fnDoorCode_SetupElevatubeFromSouth: DoorCode_SetupElevatubeFromSouth(); return;
  case fnDoorCode_SetupElevatubeFromNorth: DoorCode_SetupElevatubeFromNorth(); return;
  case fnDoorCode_ResetElevatubeNorthExit: DoorCode_ResetElevatubeNorthExit(); return;
  case fnDoorCode_ResetElevatubeSouthExit: DoorCode_ResetElevatubeSouthExit(); return;
  case fnDoorCode_SetScroll_50: DoorCode_SetScroll_50(); return;
  case fnDoorCode_SetScroll_53: DoorCode_SetScroll_53(); return;
  case fnDoorCode_SetScroll_54: DoorCode_SetScroll_54(); return;
  case fnDoorCode_SetScroll_55: DoorCode_SetScroll_55(); return;
  case fnDoorCode_SetScroll_56: DoorCode_SetScroll_56(); return;
  case fnDoorCode_SetScroll_57: DoorCode_SetScroll_57(); return;
  case fnDoorCode_SetScroll_58: DoorCode_SetScroll_58(); return;
  case fnDoorCode_SetScroll_59: DoorCode_SetScroll_59(); return;
  case fnDoorCode_SetScroll_61: DoorCode_SetScroll_61(); return;
  case fnDoorCode_SetScroll_62: DoorCode_SetScroll_62(); return;
  case fnDoorCode_SetScroll_63: DoorCode_SetScroll_63(); return;
  case fnDoorCode_SetScroll_64: DoorCode_SetScroll_64(); return;
  case fnDoorCode_SetScroll_65: DoorCode_SetScroll_65(); return;
  case fnDoorCode_CeresElevatorShaft: DoorCode_CeresElevatorShaft(); return;
  case fnDoorCode_CeresElevatorShaft_2: DoorCode_CeresElevatorShaft_2(); return;
  default: Unreachable();
  }
}

void RunDoorSetupCode(void) {  // 0x8FE8A3
  DoorDef *door_def = get_DoorDef(door_def_ptr);
  if (door_def->door_setup_code)
    CallDoorDefSetupCode(door_def->door_setup_code | 0x8F0000);
}
