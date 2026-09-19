#ifndef SM_PICO_FRAME_PACKET_H_
#define SM_PICO_FRAME_PACKET_H_

#include "scanline_mode1.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Game → display Pico frame packet (one 1/60 s snapshot).
 *
 * Payload is PPU state, not a framebuffer: scroll, OAM, CGRAM, a handful of
 * PPU regs, and a VRAM pointer (dirty/full upload). Most live frames are a
 * few KB; a room load can be tens of KB of VRAM. UART/SPI is sm_rev-qcf.
 *
 * Dummy host path: PicoFramePacket_InitDummy bakes a static Landing Site-ish
 * Mode 1 checker (BG1/BG2 4bpp + a couple of sprites) and points vram at a
 * static test block. Raster via PicoFramePacket_ToPpu + PicoScanline_Mode1.
 *
 * Extracted LS tiles (no sm.smc): PicoFramePacket_InitLandingSiteExtracted in
 * pico_ls_assets.c packs tileset 00 + BG2 + a spawn-window BG1 tilemap.
 */
enum {
  kPicoFramePacketVsync = 0x1D15
};

typedef struct PicoFramePacket {
  uint32_t frame_id;     /* 1-based; bumps once per vsync / 1/60 s */
  uint16_t vsync_token;  /* kPicoFramePacketVsync */
  uint16_t joypad_echo;  /* display Pico echo; dummy 0 */

  uint16_t bg1hofs;
  uint16_t bg1vofs;
  uint16_t bg2hofs;
  uint16_t bg2vofs;

  uint8_t inidisp;   /* $2100: dummy 0x0F (full brightness, not blank) */
  uint8_t obsel;     /* $2101 */
  uint8_t bgmode;    /* $2105 */
  uint8_t bg1sc;     /* $2107 */
  uint8_t bg2sc;     /* $2108 */
  uint8_t bg12nba;   /* $210B */
  uint8_t tm;        /* $212C: dummy 0x13 = BG1+BG2+OBJ */

  uint8_t oam_full;    /* 1 = oam[] + oam_hi[] valid this frame */
  uint8_t cgram_full;  /* 1 = cgram[] valid this frame */
  uint8_t vram_full;   /* 1 = vram[] is a complete scene (dummy) */

  uint8_t oam[kPicoOamSize];
  uint8_t oam_hi[kPicoOamHiSize];
  uint16_t cgram[kPicoCgramColors]; /* SNES BGR555 */

  /* Display Pico keeps VRAM; packet carries a pointer, not 64 KB inline. */
  const uint8_t *vram;
  size_t vram_size;
} PicoFramePacket;

void PicoFramePacket_InitDummy(PicoFramePacket *pkt);
void PicoFramePacket_ToPpu(const PicoFramePacket *pkt, PicoPpuState *ppu);

#ifdef __cplusplus
}
#endif

#endif /* SM_PICO_FRAME_PACKET_H_ */
