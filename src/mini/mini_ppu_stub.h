#ifndef SM_MINI_PPU_STUB_H_
#define SM_MINI_PPU_STUB_H_

#include <stddef.h>

#include "types.h"

enum {
  kMiniPpuVramSize = 0x10000,
};

typedef struct MiniPpuSnapshot {
  uint16 vram_addr;
  uint8 vmain;
  uint16 a1t1;
  uint8 a1b1;
  uint16 das1;
  uint8 dmap1;
  uint8 bbad1;
  uint8 cgadd;
  uint8 cgram_latch;
  bool cgram_low_pending;
  uint8 vram[kMiniPpuVramSize];
} MiniPpuSnapshot;

void MiniPpu_Reset(void);
void MiniPpu_InitGameplay(void);
void MiniPpu_CopyVram(uint16 vram_dst, const void *src, size_t size);
uint8 *MiniPpu_GetVram(void);
void MiniPpu_SaveSnapshot(MiniPpuSnapshot *snapshot);
void MiniPpu_LoadSnapshot(const MiniPpuSnapshot *snapshot);

#endif  // SM_MINI_PPU_STUB_H_
