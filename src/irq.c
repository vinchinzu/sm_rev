// IRQ vector dispatch and HUD/door-transition IRQ handlers.
#include "sm_rtl.h"
#include "variables.h"
#include "funcs.h"

static void IrqHandler_SetResult(uint16 a, uint16 y, uint16 x) {
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
