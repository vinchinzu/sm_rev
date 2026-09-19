#include "pico_frame_packet.h"

#include <string.h>

#include "scanline_test_tiles.inc"

enum {
  kTileBytes4bpp = 32,
  /* MiniPpu_InitGameplay / Super Metroid gameplay: Landing Site Mode 1. */
  kLsInidisp = 0x0F,
  kLsObsel = 0x03,
  kLsBgmode = 0x09,
  kLsBg1sc = 0x51,
  kLsBg2sc = 0x49,
  kLsBg12nba = 0x00,
  kLsTm = 0x13,
  kLsBg1MapWord = 0x5000, /* (BG1SC & 0xFC) << 8 */
  kLsBg2MapWord = 0x4800,
  kLsObjTileBase = 0xC000 / kTileBytes4bpp /* OBSEL name base 3 << 14 */
};

static uint8_t s_dummy_vram[kPicoVramSize];

static void poke_tile(uint8_t *vram, int tile_index, const uint8_t *tile) {
  memcpy(vram + (size_t)tile_index * kTileBytes4bpp, tile, kTileBytes4bpp);
}

static void poke_map_word(uint8_t *vram, uint16_t word_addr, uint16_t value) {
  size_t off = (size_t)word_addr << 1;
  vram[off] = (uint8_t)value;
  vram[off + 1] = (uint8_t)(value >> 8);
}

static void poke_chr(uint8_t *vram, int tile_base) {
  poke_tile(vram, tile_base + 1, kTestTileSolid1);
  poke_tile(vram, tile_base + 2, kTestTileQuadrant);
  poke_tile(vram, tile_base + 3, kTestTileSprite);
  poke_tile(vram, tile_base + 4, kTestTileSolid1);
}

static void fill_ls_checker_maps(uint8_t *vram) {
  /* BG1 64x32 checker: even (tx+ty) = tile 2 pal 1 (quadrant). */
  for (int ty = 0; ty < 32; ty++) {
    for (int tx = 0; tx < 64; tx++) {
      uint16_t word = 0;
      if (((tx + ty) & 1) == 0)
        word = (uint16_t)(2 | (1 << 10));
      uint16_t addr = (uint16_t)(kLsBg1MapWord + ty * 32 + (tx & 31));
      if (tx & 32)
        addr = (uint16_t)(addr + 0x400);
      poke_map_word(vram, addr, word);
    }
  }
  /* BG2 64x32: solid tile 1 pal 0. */
  for (int ty = 0; ty < 32; ty++) {
    for (int tx = 0; tx < 64; tx++) {
      uint16_t addr = (uint16_t)(kLsBg2MapWord + ty * 32 + (tx & 31));
      if (tx & 32)
        addr = (uint16_t)(addr + 0x400);
      poke_map_word(vram, addr, 1);
    }
  }
}

void PicoFramePacket_InitDummy(PicoFramePacket *pkt) {
  if (pkt == NULL)
    return;

  memset(pkt, 0, sizeof(*pkt));
  memset(s_dummy_vram, 0, sizeof(s_dummy_vram));

  poke_chr(s_dummy_vram, 0);
  poke_chr(s_dummy_vram, kLsObjTileBase);
  fill_ls_checker_maps(s_dummy_vram);

  pkt->frame_id = 1;
  pkt->vsync_token = (uint16_t)kPicoFramePacketVsync;
  pkt->joypad_echo = 0;

  pkt->inidisp = kLsInidisp;
  pkt->obsel = kLsObsel;
  pkt->bgmode = kLsBgmode;
  pkt->bg1sc = kLsBg1sc;
  pkt->bg2sc = kLsBg2sc;
  pkt->bg12nba = kLsBg12nba;
  pkt->tm = kLsTm;

  pkt->cgram[0] = kTestCgramBackdrop;
  pkt->cgram[1] = kTestCgramRed;
  pkt->cgram[17] = kTestCgramGreen;
  pkt->cgram[18] = kTestCgramYellow;
  pkt->cgram[129] = kTestCgramBlue;
  pkt->cgram[130] = kTestCgramWhite;

  /* Sprite 0: 8x8 at (20,8), tile 3, pal 0, priority 3. */
  pkt->oam[0] = 20;
  pkt->oam[1] = 8;
  pkt->oam[2] = 3;
  pkt->oam[3] = 0x30;

  /* Sprite 1: 8x8 at (0,0), tile 3, pal 0, priority 0 (under BG1). */
  pkt->oam[4] = 0;
  pkt->oam[5] = 0;
  pkt->oam[6] = 3;
  pkt->oam[7] = 0x00;

  /* Sprite 2: 16x16 at (200,16), tile 3, pal 0, priority 3, size=large. */
  pkt->oam[8] = 200;
  pkt->oam[9] = 16;
  pkt->oam[10] = 3;
  pkt->oam[11] = 0x30;
  pkt->oam_hi[0] = (uint8_t)(2 << 4);

  pkt->oam_full = 1;
  pkt->cgram_full = 1;
  pkt->vram_full = 1;
  pkt->vram = s_dummy_vram;
  pkt->vram_size = sizeof(s_dummy_vram);
}

void PicoFramePacket_ToPpu(const PicoFramePacket *pkt, PicoPpuState *ppu) {
  if (pkt == NULL || ppu == NULL)
    return;

  memset(ppu, 0, sizeof(*ppu));
  ppu->vram = pkt->vram;
  ppu->vram_size = pkt->vram_size;
  ppu->cgram = pkt->cgram_full ? pkt->cgram : NULL;
  ppu->oam = pkt->oam_full ? pkt->oam : NULL;
  ppu->oam_hi = pkt->oam_full ? pkt->oam_hi : NULL;
  ppu->sprite_count = kPicoSpriteCount;
  ppu->bg1hofs = pkt->bg1hofs;
  ppu->bg1vofs = pkt->bg1vofs;
  ppu->bg2hofs = pkt->bg2hofs;
  ppu->bg2vofs = pkt->bg2vofs;
  ppu->bgmode = pkt->bgmode;
  ppu->bg1sc = pkt->bg1sc;
  ppu->bg2sc = pkt->bg2sc;
  ppu->bg12nba = pkt->bg12nba;
  ppu->obsel = pkt->obsel;
}
