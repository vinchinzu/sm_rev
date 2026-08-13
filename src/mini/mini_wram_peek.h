#ifndef SM_MINI_WRAM_PEEK_H_
#define SM_MINI_WRAM_PEEK_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "types.h"

// Key WRAM addresses for validation
enum {
  kWramAddr_RoomId = 0x079B,
  kWramAddr_SamusX = 0x0AF6,
  kWramAddr_SamusXSub = 0x0AF8,
  kWramAddr_SamusY = 0x0AFA,
  kWramAddr_SamusYSub = 0x0AFC,
  kWramAddr_SamusPose = 0x0A1C,
  kWramAddr_Health = 0x09C2,
};

#endif  // SM_MINI_WRAM_PEEK_H_
