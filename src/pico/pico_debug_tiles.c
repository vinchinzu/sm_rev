#include "pico_debug_tiles.h"

#include <stdio.h>

#include "block_reaction.h"
#include "mini/mini_game.h"

enum {
  kPicoDebugAirRgb = 0x2104,
  kPicoDebugSolidRgb = 0xF800,
  kPicoDebugSamusRgb = 0xFFFF,
  kPicoDebugBeamRgb = 0xFFE0
};

static const uint16_t kPicoDebugMaterialRgb[16] = {
    kPicoDebugAirRgb, /* Air */
    0x07FF,           /* Slope */
    0xFD20,           /* SpikeAir */
    0xA81F,           /* SpecialAir */
    0x7BEF,           /* ShootableAir */
    0xFC10,           /* HorizontalExtension */
    0x4208,           /* UnusedAir */
    0x6B4D,           /* BombableAir */
    kPicoDebugSolidRgb, /* Solid */
    0xF81F,           /* Door */
    0xFDA0,           /* SpikeBlock */
    0x07E0,           /* SpecialBlock */
    0x001F,           /* ShootableBlock */
    0xFA80,           /* VerticalExtension */
    0xAFE5,           /* GrappleBlock */
    0xFBE0            /* BombableBlock */
};

static int PicoDebugClamp(int value, int lo, int hi) {
  if (value < lo)
    return lo;
  if (value > hi)
    return hi;
  return value;
}

static void PicoDebugFillRect(uint16_t *rgb565, int width, int height,
                              int left, int top, int fill_w, int fill_h,
                              uint16_t color) {
  int x0, y0, x1, y1;
  int y;

  if (rgb565 == NULL || width <= 0 || height <= 0 || fill_w <= 0 || fill_h <= 0)
    return;

  x0 = PicoDebugClamp(left, 0, width);
  y0 = PicoDebugClamp(top, 0, height);
  x1 = PicoDebugClamp(left + fill_w, 0, width);
  y1 = PicoDebugClamp(top + fill_h, 0, height);
  for (y = y0; y < y1; y++) {
    int x;
    uint16_t *row = rgb565 + (size_t)y * (size_t)width;
    for (x = x0; x < x1; x++)
      row[x] = color;
  }
}

uint16_t PicoDebugTiles_Rgb565(unsigned r, unsigned g, unsigned b) {
  return (uint16_t)(((r & 0xF8u) << 8) | ((g & 0xFCu) << 3) | (b >> 3));
}

uint16_t PicoDebugTiles_ColorForMaterial(uint16_t block_type) {
  unsigned index = BlockTypeIndexFromTile(block_type);
  if (index >= 16)
    index = 0;
  return kPicoDebugMaterialRgb[index];
}

uint16_t PicoDebugTiles_AirColor(void) {
  return kPicoDebugAirRgb;
}

uint16_t PicoDebugTiles_SolidColor(void) {
  return kPicoDebugSolidRgb;
}

uint16_t PicoDebugTiles_SamusColor(void) {
  return kPicoDebugSamusRgb;
}

uint16_t PicoDebugTiles_BeamColor(void) {
  return kPicoDebugBeamRgb;
}

void PicoDebugTiles_Composite(const MiniGameState *state, uint16_t *rgb565,
                              int width, int height) {
  const MiniCollisionMapView *map;
  int block_size;
  int tile_size = kPicoDebugTileSize;
  int room_w;
  int room_h;
  int cam_x;
  int cam_y;
  int first_bx;
  int first_by;
  int last_bx;
  int last_by;
  int bx;
  int by;
  int rx;
  int ry;
  int i;

  if (state == NULL || rgb565 == NULL || width <= 0 || height <= 0)
    return;

  map = &state->collision_map;
  block_size = map->block_size > 0 ? map->block_size : kMiniBlockSize;
  room_w = map->width_blocks * block_size;
  room_h = map->height_blocks * block_size;
  cam_x = PicoDebugClamp(state->viewport.camera_x, 0,
                         room_w > width ? room_w - width : 0);
  cam_y = PicoDebugClamp(state->viewport.camera_y, 0,
                         room_h > height ? room_h - height : 0);

  PicoDebugFillRect(rgb565, width, height, 0, 0, width, height,
                    PicoDebugTiles_SolidColor());

  first_bx = cam_x / block_size;
  first_by = cam_y / block_size;
  last_bx = (cam_x + width - 1) / block_size;
  last_by = (cam_y + height - 1) / block_size;
  for (by = first_by; by <= last_by; by++) {
    for (bx = first_bx; bx <= last_bx; bx++) {
      uint16_t color = PicoDebugTiles_ColorForMaterial(
          MiniStubs_GetCollisionMaterial(bx, by));
      int origin_x = bx * block_size - cam_x;
      int origin_y = by * block_size - cam_y;
      int qy;
      for (qy = 0; qy < block_size; qy += tile_size) {
        int qx;
        int th = block_size - qy;
        if (th > tile_size)
          th = tile_size;
        for (qx = 0; qx < block_size; qx += tile_size) {
          int tw = block_size - qx;
          if (tw > tile_size)
            tw = tile_size;
          PicoDebugFillRect(rgb565, width, height, origin_x + qx, origin_y + qy,
                            tw, th, color);
        }
      }
    }
  }

  rx = state->samus.x_radius > 0 ? (int)state->samus.x_radius : 1;
  ry = state->samus.y_radius > 0 ? (int)state->samus.y_radius : 1;
  PicoDebugFillRect(rgb565, width, height,
                    state->samus.world_x - rx - cam_x,
                    state->samus.world_y - ry - cam_y,
                    rx * 2 + 1, ry * 2 + 1,
                    PicoDebugTiles_SamusColor());

  for (i = 0; i < state->projectile_state.count; i++) {
    const SamusProjectileView *beam = &state->projectile_state.views[i];
    int brx;
    int bry;
    if (!beam->active || !beam->is_beam)
      continue;
    brx = beam->x_radius > 0 ? (int)beam->x_radius : (kPicoDebugTileSize / 2);
    bry = beam->y_radius > 0 ? (int)beam->y_radius : (kPicoDebugTileSize / 2);
    PicoDebugFillRect(rgb565, width, height,
                      (int)beam->x_pos - brx - cam_x,
                      (int)beam->y_pos - bry - cam_y,
                      brx * 2 + 1, bry * 2 + 1,
                      PicoDebugTiles_BeamColor());
  }
}

bool PicoDebugTiles_WritePpm(const char *path, const uint16_t *rgb565,
                             int width, int height) {
  FILE *f;
  int y;

  if (path == NULL || path[0] == '\0' || rgb565 == NULL || width <= 0 || height <= 0)
    return false;

  f = fopen(path, "wb");
  if (f == NULL)
    return false;

  if (fprintf(f, "P6\n%d %d\n255\n", width, height) < 0) {
    fclose(f);
    return false;
  }

  for (y = 0; y < height; y++) {
    int x;
    const uint16_t *row = rgb565 + (size_t)y * (size_t)width;
    for (x = 0; x < width; x++) {
      uint16_t c = row[x];
      unsigned r = (unsigned)((c >> 11) & 0x1F) * 255u / 31u;
      unsigned g = (unsigned)((c >> 5) & 0x3F) * 255u / 63u;
      unsigned b = (unsigned)(c & 0x1F) * 255u / 31u;
      if (fputc((int)r, f) == EOF || fputc((int)g, f) == EOF || fputc((int)b, f) == EOF) {
        fclose(f);
        return false;
      }
    }
  }

  return fclose(f) == 0;
}
