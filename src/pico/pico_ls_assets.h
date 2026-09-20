#ifndef SM_PICO_LS_ASSETS_H_
#define SM_PICO_LS_ASSETS_H_

#include "pico_frame_packet.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Display-side Landing Site assets packed from assets/local_mini (tileset 00,
 * spawn-window BG1/BG2). No sm.smc. Ship 1 keeps editor-export BG.
 *
 * PicoFramePacket_InitLandingSiteExtracted fills a 64 KB VRAM block + CGRAM
 * for Mode 1 (same PPU regs as MiniPpu_InitGameplay / InitDummy).
 *
 * Packed BG maps are a spawn window whose (0,0) is world pixel
 * (kPicoLsExtractCameraX, kPicoLsExtractCameraY). Live scrolls are
 * PicoViewport_ExtractScroll(layer1_*, origin), clamped to the extract
 * (hofs 0..256, vofs 0..32) — not samus-128 and not the full 144×80 room.
 * Walking past the window has no BG tiles; Samus goes to the screen edge.
 * Follow-up: VRAM window / dirty upload. Y is 32px below JSON cameraY 976
 * so pal4 platforms sit in the 224-line viewport at scroll 0.
 */
enum {
  kPicoLsExtractCameraX = 1024,
  kPicoLsExtractCameraY = 1008,
  /* room_91F8.json camera.spawnX/spawnY: where vanilla puts Samus, standing
   * on the gunship. Asserted against the packed values in the .c. */
  kPicoLsRoomSpawnX = 1153,
  kPicoLsRoomSpawnY = 1088,
  /* room_91F8.json scroll.bgScrolling = 0x0181. layer2_scroll_y == 1 makes
   * vanilla CalculateLayer2Ypos() leave reg_BG2VOFS alone, so BG2 never
   * scrolls vertically and holds its room-load value. Asserted against the
   * packed value in the .c; the packed map is expanded from that same row. */
  kPicoLsBg2VerticalScroll = 0
};

size_t PicoLsAssets_PackedSize(void);
size_t PicoLsAssets_UnpackedSourceSize(void);
void PicoFramePacket_InitLandingSiteExtracted(PicoFramePacket *pkt);

#ifdef __cplusplus
}
#endif

#endif /* SM_PICO_LS_ASSETS_H_ */
