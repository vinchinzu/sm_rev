#ifndef SM_PICO_OAM_FROM_SAMUS_H_
#define SM_PICO_OAM_FROM_SAMUS_H_

#include "pico_frame_packet.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Packed Samus animation frames (stand / run / turn, plus jump and spin heads).
 * Screen position is the spritemap origin:
 *   sx = samus_x_pos - layer1_x_pos
 *   sy = samus_y_pos - layer1_y_pos - PicoOam_SamusYOffset(pose)
 * CHR is DMAd to the OBJ name base once per frame, the way the vanilla NMI
 * handler does it; the power-suit palette goes to CGRAM 192 (sprite pal 4).
 *
 * OAM budget: Samus owns slots 0..kPicoOamSamusMaxSlots-1, the gunship owns
 * the slots above it (see pico_oam_gunship.h). Lower slot draws on top, so
 * Samus stays in front of the ship she is standing on.
 */
enum {
  kPicoOamSamusBlobBgr555 = 0x7C1F, /* leftover magenta; must not appear */
  kPicoOamSamusCgramIndex = 192,
  kPicoOamSamusMaxSlots = 64,
  kPicoOamSamusFallbackPose = 1 /* kPose_01_FaceR_Normal */
};

/* Non-zero when this pose has packed frames. */
int PicoOam_SamusPoseIsPacked(int pose);

/* Total frames packed across every pose. */
int PicoOam_SamusFrameTotal(void);

/* Frames packed for this pose, or 0 when it is not packed. */
int PicoOam_SamusPoseFrames(int pose);

/*
 * Resolve a live (pose, anim_frame) to a packed frame index. Unpacked poses
 * fall back to kPicoOamSamusFallbackPose so Samus never vanishes; frame wraps
 * within the pose. Always returns a valid index.
 */
int PicoOam_SamusFrameIndex(int pose, int anim_frame);

/*
 * kPoseParams[pose].y_offset_to_gfx, packed verbatim from ROM bank 0x91
 * (0x91B629). Vanilla draws Samus at samus_y_pos - y_offset_to_gfx
 * (Samus_CalcSpritemapPos_Default, src/samus_draw.c), so this is the same
 * number the ROM uses, for airborne poses as well as grounded ones.
 * Signed: vanilla reads it as int8.
 */
int PicoOam_SamusYOffset(int pose);

/* kPoseParams[pose].y_radius, same table. Vanilla's collision half-height:
 * the foot line is samus_y_pos + y_radius - 1. */
int PicoOam_SamusYRadius(int pose);

/*
 * Points the sim's bank 0x91 accessors (kPoseParams, kSamusAnimationDelayData
 * and the delay streams they chase) at the packed $91B000..$91BFFF window.
 * g_rom is NULL on the Pico, so without this the vanilla animation clock
 * cannot read its tables and samus_anim_frame never moves. Call once before
 * MiniCreate. Returns non-zero on success.
 */
int PicoOam_InstallSamusBank91(void);

/* pkt->vram must be writable. Plants the frame's 4bpp tiles + sprite pal 4. */
int PicoOam_PlantSamusFrame(PicoFramePacket *pkt, int frame_index);

/* Writes the frame's spritemap at origin (x, y) into slots 0..63. */
void PicoOam_WriteSamusFrame(PicoFramePacket *pkt, int frame_index,
                             int screen_x, int screen_y);

/* Parks every sprite slot off-screen. Call once before the first gunship
 * write; the per-frame Samus write only clears its own slots. */
void PicoOam_ParkAllSprites(PicoFramePacket *pkt);

#ifdef __cplusplus
}
#endif

#endif /* SM_PICO_OAM_FROM_SAMUS_H_ */
