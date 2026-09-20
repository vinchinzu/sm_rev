#include "pico_oam_gunship.h"

#include <string.h>

#include "assets/pico_ls_gunship.inc"

static size_t obj_name_select_base(uint8_t obsel) {
  size_t base = (size_t)(obsel & 7) << 14;
  base += (size_t)(((obsel & 0x18) + 8) << 10);
  return base;
}

int PicoOam_PlantGunshipGfx(PicoFramePacket *pkt) {
  uint8_t *vram;
  size_t base;
  size_t n;
  int i;

  if (pkt == NULL || pkt->vram == NULL)
    return 0;
  n = pkt->vram_size ? pkt->vram_size : (size_t)kPicoVramSize;
  base = obj_name_select_base(pkt->obsel);
  if (base + (size_t)kPicoLsGunshipChrSize > n)
    return 0;
  if ((unsigned)kPicoLsGunshipCgramIndex + (unsigned)kPicoLsGunshipPalCount >
      (unsigned)kPicoCgramColors)
    return 0;

  vram = (uint8_t *)pkt->vram;
  memcpy(vram + base, kPicoLsGunshipChr, (size_t)kPicoLsGunshipChrSize);
  for (i = 0; i < kPicoLsGunshipPalCount; i++)
    pkt->cgram[kPicoLsGunshipCgramIndex + i] = kPicoLsGunshipPal[i];
  pkt->cgram_full = 1;
  return 1;
}

void PicoOam_WriteGunship(PicoFramePacket *pkt, int dx, int dy) {
  int i;

  if (pkt == NULL)
    return;

  for (i = 0; i < kPicoLsGunshipOamCount; i++) {
    int slot = kPicoLsGunshipFirstSlot + i;
    size_t dst = (size_t)slot * 4u;
    size_t src = (size_t)i * 4u;
    int shift = (slot & 3) * 2;
    int hi_i = slot >> 2;
    uint8_t bits = (uint8_t)(kPicoLsGunshipOamHi[i] & 3u);
    /* Rebuild the 9-bit X, shift it, and re-split it across oam/oam_hi. */
    int x = kPicoLsGunshipOam[src] | ((bits & 1) << 8);
    int y = kPicoLsGunshipOam[src + 1u];

    if (slot >= kPicoSpriteCount)
      break;
    x = (x + dx) & 0x1FF;
    y = (y + dy) & 0xFF;
    bits = (uint8_t)((bits & 2u) | ((x >> 8) & 1u));

    pkt->oam[dst] = (uint8_t)x;
    pkt->oam[dst + 1u] = (uint8_t)y;
    pkt->oam[dst + 2u] = kPicoLsGunshipOam[src + 2u];
    pkt->oam[dst + 3u] = kPicoLsGunshipOam[src + 3u];
    pkt->oam_hi[hi_i] = (uint8_t)((pkt->oam_hi[hi_i] & ~(3u << shift)) | (bits << shift));
  }
  pkt->oam_full = 1;
}
