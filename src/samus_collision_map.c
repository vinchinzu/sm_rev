// Shared Samus collision map probe: world-pixel coordinates to room block index.
// This is the smallest reusable map-facing helper needed by both the full
// block-collision layer and the mini tilemap-backed collision seam.

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"

void CalculateBlockAt(uint16 r26, uint16 r28, uint16 r30, uint16 r32) {  // 0x949C1D
  int16 x = r30 + r26;
  int16 y;
  uint16 block_x;

  if ((int16)(r30 + r26) >= 0
      && sign16(x - 4096)
      && (block_x = (uint16)(x & 0xFFF0) >> 4,
          y = r32 + r28,
          (int16)(r32 + r28) >= 0)
      && sign16(y - 4096)) {
    uint16 block_y = (uint16)(y & 0xFFF0) >> 4;
    cur_block_index = block_x + block_y * room_width_in_blocks;
  } else {
    cur_block_index = -1;
  }
}
