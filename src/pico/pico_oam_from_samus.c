#include "pico_oam_from_samus.h"

#include <string.h>

#include "sm_rtl.h"

#include "assets/pico_samus_anim.inc"

_Static_assert((int)kPicoOamSamusCgramIndex == (int)kPicoLsSamusCgramIndex,
               "power-suit pal must land at sprite pal 4 / CGRAM 192");
_Static_assert((int)kPicoOamSamusFallbackPose < (int)kPicoLsSamusPoseCount,
               "fallback pose must be inside the pose table");
/* The window the packer emitted must be the one sm_rtl.h maps bank 0x91
 * addresses into, or kPoseParams would resolve to the wrong bytes. */
_Static_assert((int)kPicoLsSamusBank91Base == (int)kSamusBank91WindowBase,
               "packed bank 0x91 window base must match sm_rtl.h");
_Static_assert((int)kPicoLsSamusBank91Size == (int)kSamusBank91WindowSize,
               "packed bank 0x91 window size must match sm_rtl.h");
/* Pose 1 is the reference the packer asserts against the ROM table. */
_Static_assert((int)kPicoLsSamusYOffsetToGfx == 6, "reference y_offset_to_gfx");
_Static_assert((int)kPicoLsSamusYRadius == 21, "reference y_radius");

static size_t obj_name_base(uint8_t obsel) {
  return (size_t)(obsel & 7) << 14;
}

int PicoOam_SamusPoseIsPacked(int pose) {
  if ((unsigned)pose >= (unsigned)kPicoLsSamusPoseCount)
    return 0;
  return kPicoLsSamusPoseFrameCount[pose] != 0;
}

int PicoOam_SamusFrameTotal(void) {
  return (int)kPicoLsSamusFrameCount;
}

int PicoOam_SamusPoseFrames(int pose) {
  if ((unsigned)pose >= (unsigned)kPicoLsSamusPoseCount)
    return 0;
  return (int)kPicoLsSamusPoseFrameCount[pose];
}

int PicoOam_SamusFrameIndex(int pose, int anim_frame) {
  int count;

  if (!PicoOam_SamusPoseIsPacked(pose))
    pose = kPicoOamSamusFallbackPose;
  count = (int)kPicoLsSamusPoseFrameCount[pose];
  if (count <= 0)
    return 0;
  if (anim_frame < 0)
    anim_frame = 0;
  return (int)kPicoLsSamusPoseFirstFrame[pose] + (anim_frame % count);
}

int PicoOam_SamusYOffset(int pose) {
  if ((unsigned)pose >= (unsigned)kPicoLsSamusPoseCount)
    pose = kPicoOamSamusFallbackPose;
  /* int8 because that is how vanilla reads it (src/samus_draw.c:101). */
  return (int)(int8_t)kPicoLsSamusPoseYOffset[pose];
}

int PicoOam_SamusYRadius(int pose) {
  if ((unsigned)pose >= (unsigned)kPicoLsSamusPoseCount)
    pose = kPicoOamSamusFallbackPose;
  return (int)kPicoLsSamusPoseYRadius[pose];
}

int PicoOam_InstallSamusBank91(void) {
  SamusBank91_Install(kPicoLsSamusBank91, (uint32)kPicoLsSamusBank91Size);
  return g_samus_bank91 != NULL;
}

void PicoOam_ParkAllSprites(PicoFramePacket *pkt) {
  int i;

  if (pkt == NULL)
    return;
  /* Y=224 is below the 224-line viewport, so tile 0 never draws. */
  for (i = 0; i < kPicoSpriteCount; i++)
    pkt->oam[(size_t)i * 4u + 1u] = 224;
  memset(pkt->oam_hi, 0, sizeof(pkt->oam_hi));
  pkt->oam_full = 1;
}

int PicoOam_PlantSamusFrame(PicoFramePacket *pkt, int frame_index) {
  uint8_t *vram;
  size_t base;
  size_t n;
  size_t src;
  int i;

  if (pkt == NULL || pkt->vram == NULL)
    return 0;
  if ((unsigned)frame_index >= (unsigned)kPicoLsSamusFrameCount)
    return 0;
  n = pkt->vram_size ? pkt->vram_size : (size_t)kPicoVramSize;
  base = obj_name_base(pkt->obsel);
  if (base + (size_t)kPicoLsSamusChrFrameSize > n)
    return 0;
  if ((unsigned)kPicoLsSamusCgramIndex + (unsigned)kPicoLsSamusPalCount >
      (unsigned)kPicoCgramColors)
    return 0;

  vram = (uint8_t *)pkt->vram;
  src = (size_t)frame_index * (size_t)kPicoLsSamusChrFrameSize;
  memcpy(vram + base, kPicoLsSamusChr + src, (size_t)kPicoLsSamusChrFrameSize);
  for (i = 0; i < kPicoLsSamusPalCount; i++)
    pkt->cgram[kPicoLsSamusCgramIndex + i] = kPicoLsSamusPal[i];
  pkt->cgram_full = 1;
  pkt->vram_full = 1;
  return 1;
}

void PicoOam_WriteSamusFrame(PicoFramePacket *pkt, int frame_index,
                             int screen_x, int screen_y) {
  const uint8_t *pp;
  uint16_t n;
  int i;

  if (pkt == NULL)
    return;
  if ((unsigned)frame_index >= (unsigned)kPicoLsSamusFrameCount)
    return;

  /* Clear only our own slots; the gunship owns the ones above. */
  for (i = 0; i < kPicoOamSamusMaxSlots; i++) {
    pkt->oam[(size_t)i * 4u + 1u] = 224;
    pkt->oam_hi[i >> 2] = (uint8_t)(pkt->oam_hi[i >> 2] & ~(3u << ((i & 3) * 2)));
  }

  pp = kPicoLsSamusSpritemaps + kPicoLsSamusFrameSmOffset[frame_index];
  n = (uint16_t)(pp[0] | ((uint16_t)pp[1] << 8));
  /* Bound by the slots we own, not by the count we just read from the blob. */
  if (n > (unsigned)kPicoOamSamusMaxSlots)
    n = (uint16_t)kPicoOamSamusMaxSlots;
  pp += 2;

  for (i = 0; i < (int)n; i++) {
    uint16_t xword = (uint16_t)(pp[0] | ((uint16_t)pp[1] << 8));
    uint16_t charnum = (uint16_t)(pp[3] | ((uint16_t)pp[4] << 8));
    uint16_t xsum = (uint16_t)screen_x + xword;
    uint8_t y = (uint8_t)((uint8_t)screen_y + pp[2]);
    uint8_t hi = (uint8_t)((xsum >> 8) & 1u);
    int shift = (i & 3) * 2;
    int hi_i = i >> 2;
    size_t dst = (size_t)i * 4u;

    if ((int16_t)xword < 0)
      hi = (uint8_t)(hi | 2u);

    pkt->oam[dst] = (uint8_t)xsum;
    pkt->oam[dst + 1u] = y;
    pkt->oam[dst + 2u] = (uint8_t)charnum;
    pkt->oam[dst + 3u] = (uint8_t)(charnum >> 8);
    pkt->oam_hi[hi_i] =
        (uint8_t)((pkt->oam_hi[hi_i] & ~(3u << shift)) | (hi << shift));
    pp += 5;
  }
  pkt->oam_full = 1;
}
