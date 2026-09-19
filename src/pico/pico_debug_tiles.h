#ifndef SM_PICO_DEBUG_TILES_H_
#define SM_PICO_DEBUG_TILES_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Host debug compositor for pico-kernel. Not mini_renderer.c.
 *
 * Paints collision materials as colored 8x8 tiles, Samus as a rectangle,
 * and live beam slots as colored sprites. Caller owns the RGB565 dump
 * buffer — this TU has no 112 KB framebuffer.
 */
struct MiniGameState;

enum {
  kPicoDebugTileSize = 8,
  kPicoDebugWidth = 256,
  kPicoDebugHeight = 224
};

uint16_t PicoDebugTiles_Rgb565(unsigned r, unsigned g, unsigned b);
uint16_t PicoDebugTiles_ColorForMaterial(uint16_t block_type);
uint16_t PicoDebugTiles_AirColor(void);
uint16_t PicoDebugTiles_SolidColor(void);
uint16_t PicoDebugTiles_SamusColor(void);
uint16_t PicoDebugTiles_BeamColor(void);

void PicoDebugTiles_Composite(const struct MiniGameState *state,
                              uint16_t *rgb565, int width, int height);

bool PicoDebugTiles_WritePpm(const char *path, const uint16_t *rgb565,
                             int width, int height);

#ifdef __cplusplus
}
#endif

#endif /* SM_PICO_DEBUG_TILES_H_ */
