// Game timer (Ceres escape, Mother Brain escape) state machine and sprite rendering.
#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"

#define kTimerDigitsSpritemapPtr ((uint16*)RomFixedPtr(0x809fd4))

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

static bool DecrementDecimal(uint8 *number, uint8 value) {
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
