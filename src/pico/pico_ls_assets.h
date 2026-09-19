#ifndef SM_PICO_LS_ASSETS_H_
#define SM_PICO_LS_ASSETS_H_

#include "pico_frame_packet.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Display-side Landing Site assets packed from assets/local_mini (tileset 00,
 * default BG2, spawn-window BG1). No sm.smc. Ship 1 keeps editor-export BG.
 *
 * PicoFramePacket_InitLandingSiteExtracted fills a 64 KB VRAM block + CGRAM
 * for Mode 1 (same PPU regs as MiniPpu_InitGameplay / InitDummy).
 *
 * Packed BG maps are a spawn window whose (0,0) is world pixel
 * (kPicoLsExtractCameraX, kPicoLsExtractCameraY). Live scrolls should be
 * layer1_* minus these origins, not raw camera.
 */
enum {
  kPicoLsExtractCameraX = 1024,
  kPicoLsExtractCameraY = 976
};

size_t PicoLsAssets_PackedSize(void);
size_t PicoLsAssets_UnpackedSourceSize(void);
void PicoFramePacket_InitLandingSiteExtracted(PicoFramePacket *pkt);

#ifdef __cplusplus
}
#endif

#endif /* SM_PICO_LS_ASSETS_H_ */
