#ifndef SM_MINI_PPU_STUB_H_
#define SM_MINI_PPU_STUB_H_

#include <stddef.h>

#include "types.h"

void MiniPpu_Reset(void);
void MiniPpu_InitGameplay(void);
void MiniPpu_CopyVram(uint16 vram_dst, const void *src, size_t size);
uint8 *MiniPpu_GetVram(void);

#endif  // SM_MINI_PPU_STUB_H_
