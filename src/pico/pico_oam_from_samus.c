#include "pico_oam_from_samus.h"

#include <string.h>

enum {
  kTileBytes4bpp = 32,
  kSamusBlobAttr = 0x30, /* pal 0, priority 3 */
  kSamusBlobHiLarge = 0x02
};

/* Solid color 1 (bitplane 0). */
static const uint8_t kSolidColor1[kTileBytes4bpp] = {
    0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
    0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static size_t obj_name_base(uint8_t obsel) {
  return (size_t)(obsel & 7) << 14;
}

int PicoOam_PlantSamusBlobGfx(PicoFramePacket *pkt) {
  uint8_t *vram;
  size_t base;
  size_t n;
  int i;
  static const int kTiles[4] = {0, 1, 16, 17};

  if (pkt == NULL || pkt->vram == NULL)
    return 0;
  n = pkt->vram_size ? pkt->vram_size : (size_t)kPicoVramSize;
  base = obj_name_base(pkt->obsel);
  if (base + (size_t)17 * kTileBytes4bpp + kTileBytes4bpp > n)
    return 0;

  vram = (uint8_t *)pkt->vram;
  for (i = 0; i < 4; i++)
    memcpy(vram + base + (size_t)kTiles[i] * kTileBytes4bpp, kSolidColor1,
           kTileBytes4bpp);

  pkt->cgram[kPicoOamSamusBlobCgramIndex] = (uint16_t)kPicoOamSamusBlobBgr555;
  pkt->cgram_full = 1;
  return 1;
}

void PicoOam_WriteSamusBlob(PicoFramePacket *pkt, int screen_x, int screen_y) {
  int x;
  int i;
  uint8_t hi;

  if (pkt == NULL)
    return;

  memset(pkt->oam, 0, sizeof(pkt->oam));
  memset(pkt->oam_hi, 0, sizeof(pkt->oam_hi));
  /* Y=224 is below the 224-line screen so unused slots do not draw tile 0. */
  for (i = 0; i < kPicoSpriteCount; i++)
    pkt->oam[(size_t)i * 4u + 1u] = 224;

  x = screen_x;
  hi = (uint8_t)kSamusBlobHiLarge;
  if (x < 0) {
    x += 512;
    hi = (uint8_t)(hi | 1);
  }

  pkt->oam[0] = (uint8_t)x;
  pkt->oam[1] = (uint8_t)screen_y;
  pkt->oam[2] = 0;
  pkt->oam[3] = (uint8_t)kSamusBlobAttr;
  pkt->oam_hi[0] = hi;
  pkt->oam_full = 1;
}
