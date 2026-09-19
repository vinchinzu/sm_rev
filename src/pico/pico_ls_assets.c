#include "pico_ls_assets.h"

#include <string.h>

#include "assets/pico_ls_extracted.inc"

enum {
  kTileBytes4bpp = 32,
  kLsInidisp = 0x0F,
  kLsObsel = 0x03,
  kLsBgmode = 0x09,
  kLsBg1sc = 0x51,
  kLsBg2sc = 0x49,
  kLsBg12nba = 0x00,
  kLsTm = 0x13,
  kLsBg1MapWord = 0x5000,
  kLsBg2MapWord = 0x4800
};

static uint8_t s_ls_vram[kPicoVramSize];

static void copy_words_le(uint8_t *vram, uint16_t word_addr, const uint16_t *words, size_t count) {
  size_t off = (size_t)word_addr << 1;
  size_t n = count * 2u;
  if (off >= sizeof(s_ls_vram) || n > sizeof(s_ls_vram) - off)
    return;
  memcpy(vram + off, words, n);
}

size_t PicoLsAssets_PackedSize(void) {
  return (size_t)kPicoLsPackedSize;
}

size_t PicoLsAssets_UnpackedSourceSize(void) {
  return (size_t)kPicoLsUnpackedSourceSize;
}

void PicoFramePacket_InitLandingSiteExtracted(PicoFramePacket *pkt) {
  if (pkt == NULL)
    return;

  memset(pkt, 0, sizeof(*pkt));
  memset(s_ls_vram, 0, sizeof(s_ls_vram));

  memcpy(s_ls_vram, kPicoLsTiles4bpp, sizeof(kPicoLsTiles4bpp));
  copy_words_le(s_ls_vram, kLsBg2MapWord, kPicoLsBg2Tilemap, kPicoLsPackedTilemapWords);
  copy_words_le(s_ls_vram, kLsBg1MapWord, kPicoLsBg1Tilemap, kPicoLsPackedTilemapWords);

  pkt->frame_id = 1;
  pkt->vsync_token = (uint16_t)kPicoFramePacketVsync;
  pkt->joypad_echo = 0;
  pkt->bg1hofs = 0;
  pkt->bg1vofs = 0;
  pkt->bg2hofs = 0;
  pkt->bg2vofs = 0;

  pkt->inidisp = kLsInidisp;
  pkt->obsel = kLsObsel;
  pkt->bgmode = kLsBgmode;
  pkt->bg1sc = kLsBg1sc;
  pkt->bg2sc = kLsBg2sc;
  pkt->bg12nba = kLsBg12nba;
  pkt->tm = kLsTm;

  memcpy(pkt->cgram, kPicoLsPalette, sizeof(kPicoLsPalette));

  pkt->oam_full = 1;
  pkt->cgram_full = 1;
  pkt->vram_full = 1;
  pkt->vram = s_ls_vram;
  pkt->vram_size = sizeof(s_ls_vram);
}
