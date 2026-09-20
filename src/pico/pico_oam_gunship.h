#ifndef SM_PICO_OAM_GUNSHIP_H_
#define SM_PICO_OAM_GUNSHIP_H_

#include "pico_frame_packet.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Packed Landing Site gunship (roomSprites) in extract-space OAM.
 * CHR is the 144-tile enemy sheet (tileNum 256-357 → local 0-101) at the
 * OBJ name-select bank so it does not collide with the blob at name base.
 * Sprite pal 1/2 at CGRAM 144+; blob keeps pal 0 / CGRAM 129.
 * Slots 64.. sit above the range PicoOam_WriteSamusFrame owns (0..63). Lower
 * slot draws on top, so Samus renders in front of the ship she stands on.
 * The OAM is packed in extract space (world minus the packed window origin),
 * so callers pass the current scroll back out as (-hofs, -vofs).
 */
enum {
  kPicoLsGunshipOamCount = 56,
  kPicoLsGunshipChrSize = 4608,
  kPicoLsGunshipPalCount = 32,
  kPicoLsGunshipFirstSlot = 64,
  kPicoLsGunshipCgramIndex = 144
};

extern const uint8_t kPicoLsGunshipChr[kPicoLsGunshipChrSize];
extern const uint16_t kPicoLsGunshipPal[kPicoLsGunshipPalCount];
extern const uint8_t kPicoLsGunshipOam[kPicoLsGunshipOamCount * 4];
extern const uint8_t kPicoLsGunshipOamHi[kPicoLsGunshipOamCount];

int PicoOam_PlantGunshipGfx(PicoFramePacket *pkt);
void PicoOam_WriteGunship(PicoFramePacket *pkt, int dx, int dy);

#ifdef __cplusplus
}
#endif

#endif /* SM_PICO_OAM_GUNSHIP_H_ */
