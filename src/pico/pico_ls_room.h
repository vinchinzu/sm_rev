#ifndef SM_PICO_LS_ROOM_H_
#define SM_PICO_LS_ROOM_H_

#include "mini/mini_room_adapter.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Landing Site (0x91F8) gameplay room baked into the binary.
 *
 * On-device there is no filesystem (MiniEditorPath_Exists is false), so
 * MiniCreate would otherwise build its own fallback room: one flat floor at
 * the bottom of the viewport, in viewport coordinates. That floor is not the
 * gunship deck and not the terrain, which is why Samus used to walk off the
 * ship without falling and could never reach the ground.
 *
 * PicoLsRoom_Install registers the packed room with mini. Call it BEFORE
 * MiniCreate; after it, samus_x_pos / samus_y_pos / layer1_* are Landing Site
 * world coordinates and need no re-basing.
 *
 * Measured room facts, re-checked by the packer before it writes:
 *   spawn block (72,68) = world (1153,1088)
 *   gunship deck collision rows by 69..73 (blockWord 0x80FF: solid collision,
 *     metatile 0xFF, air graphics - the ship itself is drawn by sprites)
 *   terrain floor rows by 77..79, world y 1232+
 */
enum {
  kPicoLsRoomBlockPx = 16,
  kPicoLsRoomSpawnBlockX = 72,
  kPicoLsRoomSpawnBlockY = 68,
  kPicoLsRoomDeckTopRow = 69,
  kPicoLsRoomDeckBottomRow = 73,
  kPicoLsRoomFloorTopRow = 77
};

/* True once the packed room is registered. Safe to call more than once. */
bool PicoLsRoom_Install(void);
void PicoLsRoom_Uninstall(void);
const MiniBakedRoom *PicoLsRoom_Baked(void);
size_t PicoLsRoom_PackedSize(void);

#ifdef __cplusplus
}
#endif

#endif /* SM_PICO_LS_ROOM_H_ */
