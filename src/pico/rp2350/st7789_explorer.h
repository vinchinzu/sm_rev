#ifndef SM_PICO_ST7789_EXPLORER_H_
#define SM_PICO_ST7789_EXPLORER_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Pimoroni Pico Explorer ST7789 240×240. Pin map and init match the proven
 * pico2_explorer_bringup/st7789_fill.c (gnuboy Explorer map, not CircuitPython).
 *
 *   SCK=GP18 MOSI=GP19 CS=GP17 DC=GP16 BL=GP20 PWM  RESET=none
 *   window (0,0,240,240)  MADCTL RGB|COL_ORDER|SWAP_XY|SCAN_ORDER
 *   no rowstart=80
 */
enum {
  kSt7789ExplorerWidth = 240,
  kSt7789ExplorerHeight = 240
};

void St7789Explorer_Init(void);
void St7789Explorer_Fill(uint16_t color);
void St7789Explorer_BeginFrame(void);
void St7789Explorer_WriteRgb565Line(const uint16_t *px, int count);
void St7789Explorer_WriteSolidLine(uint16_t color, int count);
void St7789Explorer_EndFrame(void);

#ifdef __cplusplus
}
#endif

#endif /* SM_PICO_ST7789_EXPLORER_H_ */
