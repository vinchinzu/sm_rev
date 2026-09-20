#ifndef SM_PICO_SCANLINE_MODE1_H_
#define SM_PICO_SCANLINE_MODE1_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
  kPicoVramSize = 0x10000,
  kPicoCgramColors = 256,
  kPicoOamSize = 512,
  kPicoOamHiSize = 32,
  kPicoSpriteCount = 128,
  kPicoScreenWidth = 256,
  kPicoScreenHeight = 224
};

/*
 * SNES Mode 1 scanline renderer (BG1/BG2 4bpp + sprites).
 *
 * Skipped (later beads): BG3 2bpp, Mode 7, windows, color math, mosaic,
 * HDMA, 16x16 BG tiles, INIDISP brightness, and the 32 OBJ-tile/line
 * budget (sprites may overdraw a scanline).
 *
 * Output is RGB565. CGRAM is SNES BGR555 (0bbbbbgggggrrrrr).
 * No persistent framebuffer — one 256-pixel line per call.
 */
typedef struct PicoPpuState {
  const uint8_t *vram;
  size_t vram_size;          /* 0 = 64 KB */
  const uint16_t *cgram;     /* 256 BGR555 colors; NULL => black */
  const uint8_t *oam;        /* 512-byte main table; NULL => no sprites */
  const uint8_t *oam_hi;     /* 32-byte high table; NULL => all zero */
  int sprite_count;          /* 0 = 128 entries */
  uint16_t bg1hofs;
  uint16_t bg1vofs;
  uint16_t bg2hofs;
  uint16_t bg2vofs;
  uint8_t bgmode;            /* bits 0-2 ignored (always Mode 1 here) */
  uint8_t bg1sc;
  uint8_t bg2sc;
  uint8_t bg12nba;
  uint8_t obsel;
} PicoPpuState;

uint16_t PicoBgr555ToRgb565(uint16_t bgr555);

/* Render scanline y (0..223) into out256[256] as RGB565. */
void PicoScanline_Mode1(const PicoPpuState *ppu, int y, uint16_t *out256);

/*
 * As PicoScanline_Mode1, but only computes columns [x0, x1) of the line.
 * Pixels are still written at their absolute index in out256, and columns
 * outside the range are left untouched -- so a caller that only ever reads a
 * sub-range (the 240-column Explorer panel drops x 0..7 and 248..255) can skip
 * the raster work for the columns it throws away. x0/x1 are clamped to
 * [0, 256]; Mode1() is exactly Mode1Range(..., 0, 256).
 */
void PicoScanline_Mode1Range(const PicoPpuState *ppu, int y, uint16_t *out256,
                             int x0, int x1);

#ifdef __cplusplus
}
#endif

#endif  /* SM_PICO_SCANLINE_MODE1_H_ */
