/*
 * Desktop Mode 1 scanline renderer unit test (no game kernel, no SDL).
 *
 *   gcc -O2 -Werror -I. -Isrc/pico tests/test_pico_scanline.c \
 *       src/pico/scanline_mode1.c -o sm_rev_pico_scanline_test
 *   ./sm_rev_pico_scanline_test
 *
 * Do not pass -Isrc: it shadows glibc's <features.h> with src/features.h.
 *
 * Optional Landing Site dump (out/landing_site_scanlines.json) is metadata
 * only, not VRAM/CGRAM/OAM, so it is not loaded here.
 */

#include <stdio.h>
#include <string.h>

#include "scanline_mode1.h"
#include "scanline_test_tiles.inc"

static int g_failures;

static void expect_u16(const char *name, uint16_t got, uint16_t want) {
  if (got != want) {
    fprintf(stderr, "FAIL %s: got 0x%04X want 0x%04X\n", name, got, want);
    g_failures++;
  }
}

static void poke_tile(uint8_t *vram, int tile_index, const uint8_t *tile) {
  memcpy(vram + (size_t)tile_index * 32, tile, 32);
}

static void poke_map_word(uint8_t *vram, uint16_t word_addr, uint16_t value) {
  size_t off = (size_t)word_addr << 1;
  vram[off] = (uint8_t)value;
  vram[off + 1] = (uint8_t)(value >> 8);
}

static void fill_checker_maps(uint8_t *vram) {
  /* BG1SC=0x10 => word base 0x1000, 32x32. Even (tx+ty) = tile 2 pal 1. */
  for (int ty = 0; ty < 32; ty++) {
    for (int tx = 0; tx < 32; tx++) {
      uint16_t word = 0;
      if (((tx + ty) & 1) == 0)
        word = (uint16_t)(2 | (1 << 10));
      poke_map_word(vram, (uint16_t)(0x1000 + ty * 32 + tx), word);
    }
  }
  /* BG2SC=0x14 => word base 0x1400. All tile 1 pal 0. */
  for (int i = 0; i < 32 * 32; i++)
    poke_map_word(vram, (uint16_t)(0x1400 + i), 1);
}

int main(void) {
  static uint8_t vram[kPicoVramSize];
  static uint16_t cgram[kPicoCgramColors];
  static uint8_t oam[kPicoOamSize];
  static uint8_t oam_hi[kPicoOamHiSize];
  uint16_t line[kPicoScreenWidth];

  memset(vram, 0, sizeof(vram));
  memset(cgram, 0, sizeof(cgram));
  memset(oam, 0, sizeof(oam));
  memset(oam_hi, 0, sizeof(oam_hi));

  poke_tile(vram, 1, kTestTileSolid1);
  poke_tile(vram, 2, kTestTileQuadrant);
  poke_tile(vram, 3, kTestTileSprite);
  poke_tile(vram, 4, kTestTileSolid1);
  fill_checker_maps(vram);

  cgram[0] = kTestCgramBackdrop;
  cgram[1] = kTestCgramRed;
  cgram[17] = kTestCgramGreen;
  cgram[18] = kTestCgramYellow;
  cgram[129] = kTestCgramBlue;
  cgram[130] = kTestCgramWhite;

  /* Sprite 0: 8x8 at (20,8), tile 3, pal 0, priority 3 (in front). */
  oam[0] = 20;
  oam[1] = 8;
  oam[2] = 3;
  oam[3] = 0x30;

  /* Sprite 1: 8x8 at (0,0), tile 3, pal 0, priority 0 (behind BG1 pri 0). */
  oam[4] = 0;
  oam[5] = 0;
  oam[6] = 3;
  oam[7] = 0x00;

  /* Sprite 2: 16x16 at (200,16), tile 3, pal 0, priority 3, size bit set. */
  oam[8] = 200;
  oam[9] = 16;
  oam[10] = 3;
  oam[11] = 0x30;
  oam_hi[0] = (uint8_t)(2 << 4); /* sprite 2 size=large */

  PicoPpuState ppu;
  memset(&ppu, 0, sizeof(ppu));
  ppu.vram = vram;
  ppu.vram_size = sizeof(vram);
  ppu.cgram = cgram;
  ppu.oam = oam;
  ppu.oam_hi = oam_hi;
  ppu.sprite_count = 3;
  ppu.bgmode = 9;
  ppu.bg1sc = 0x10;
  ppu.bg2sc = 0x14;
  ppu.bg12nba = 0;
  ppu.obsel = 0;

  expect_u16("bgr555 red", PicoBgr555ToRgb565(kTestCgramRed), 0xF800);
  expect_u16("bgr555 green", PicoBgr555ToRgb565(kTestCgramGreen), 0x07E0);
  expect_u16("bgr555 blue", PicoBgr555ToRgb565(kTestCgramBlue), 0x001F);
  expect_u16("bgr555 white", PicoBgr555ToRgb565(kTestCgramWhite), 0xFFFF);

  uint16_t red = PicoBgr555ToRgb565(kTestCgramRed);
  uint16_t green = PicoBgr555ToRgb565(kTestCgramGreen);
  uint16_t yellow = PicoBgr555ToRgb565(kTestCgramYellow);
  uint16_t blue = PicoBgr555ToRgb565(kTestCgramBlue);
  uint16_t white = PicoBgr555ToRgb565(kTestCgramWhite);

  PicoScanline_Mode1(&ppu, 0, line);
  expect_u16("y0 x0 BG1 quadrant color1", line[0], green);
  expect_u16("y0 x4 BG1 quadrant color2", line[4], yellow);
  expect_u16("y0 x8 BG2 through hole", line[8], red);
  expect_u16("y0 x12 BG2 through hole", line[12], red);
  /* Sprite 1 pri 0 sits under opaque BG1 at (0,0). */
  expect_u16("y0 x0 sprite pri0 behind BG1", line[0], green);
  expect_u16("y0 no sprite at x20", line[20], yellow);

  PicoScanline_Mode1(&ppu, 4, line);
  expect_u16("y4 x0 BG1 lower-left color2", line[0], yellow);
  expect_u16("y4 x4 BG1 lower-right color1", line[4], green);

  PicoScanline_Mode1(&ppu, 8, line);
  expect_u16("y8 x20 sprite marker white", line[20], white);
  expect_u16("y8 x21 sprite body blue", line[21], blue);
  expect_u16("y8 x8 BG1 even (tx+ty)", line[8], green);

  PicoScanline_Mode1(&ppu, 16, line);
  expect_u16("y16 x200 16x16 left marker", line[200], white);
  expect_u16("y16 x201 16x16 left body", line[201], blue);
  expect_u16("y16 x208 16x16 right solid", line[208], blue);
  expect_u16("y16 x215 16x16 right edge", line[215], blue);

  ppu.bg1hofs = 4;
  PicoScanline_Mode1(&ppu, 0, line);
  expect_u16("scroll x0 samples BG1 col4", line[0], yellow);
  expect_u16("scroll x4 sprite pri0 over BG2", line[4], blue);
  ppu.bg1hofs = 0;

  ppu.bg1vofs = 4;
  PicoScanline_Mode1(&ppu, 0, line);
  expect_u16("vscroll x0 samples BG1 row4", line[0], yellow);
  ppu.bg1vofs = 0;

  if (g_failures) {
    fprintf(stderr, "%d failure(s)\n", g_failures);
    return 1;
  }
  puts("ok");
  return 0;
}
