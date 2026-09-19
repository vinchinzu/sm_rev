#ifndef SM_PICO_OAM_FROM_SAMUS_H_
#define SM_PICO_OAM_FROM_SAMUS_H_

#include "pico_frame_packet.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Authored 16x16 OAM blob for ship-1 (kernel does not fill SNES OAM).
 * Screen position is Samus world minus camera minus hitbox radius.
 * Sprite CHR is planted at the packet's OBJ name base; CGRAM 129 is magenta.
 */
enum {
  kPicoOamSamusBlobPx = 16,
  kPicoOamSamusBlobCgramIndex = 129,
  kPicoOamSamusBlobBgr555 = 0x7C1F
};

/* pkt->vram must be writable. Plants 4 solid 4bpp tiles + sprite pal 0 color 1. */
int PicoOam_PlantSamusBlobGfx(PicoFramePacket *pkt);

/* One large (16x16 at OBSEL size 0) sprite 0 at screen (x, y), pri 3. */
void PicoOam_WriteSamusBlob(PicoFramePacket *pkt, int screen_x, int screen_y);

#ifdef __cplusplus
}
#endif

#endif /* SM_PICO_OAM_FROM_SAMUS_H_ */
