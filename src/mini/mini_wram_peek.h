#ifndef SM_MINI_WRAM_PEEK_H_
#define SM_MINI_WRAM_PEEK_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "types.h"

// Key WRAM addresses for validation and observation
enum {
  kWramAddr_RoomId = 0x079B,            // uint16: current room ID
  kWramAddr_SamusX = 0x0AF6,            // uint16: Samus X position (pixels)
  kWramAddr_SamusXSub = 0x0AF8,         // uint16: Samus X subpixel (16-bit)
  kWramAddr_SamusY = 0x0AFA,            // uint16: Samus Y position (pixels)
  kWramAddr_SamusYSub = 0x0AFC,         // uint16: Samus Y subpixel (16-bit)
  kWramAddr_SamusPose = 0x0A1C,         // uint16: Samus pose
  kWramAddr_Health = 0x09C2,            // uint16: current health (energy)
  kWramAddr_FrameCounter1 = 0x1842,     // uint32: first frame counter
  kWramAddr_FrameCounter2 = 0x09DA,     // uint16: second frame counter
};

#endif  // SM_MINI_WRAM_PEEK_H_
