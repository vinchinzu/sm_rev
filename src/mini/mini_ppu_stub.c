#include "mini_ppu_stub.h"

#include <string.h>

#include "sm_rtl.h"
#include "variables.h"

static struct {
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
} g_mini_ppu;

static uint8 g_mini_vram[kMiniPpuVramSize];

static const uint8 *MiniBusPtr(uint8 bank, uint16 addr) {
  if (bank == 0x7E || bank == 0x7F)
    return g_ram + (((size_t)bank - 0x7E) << 16) + addr;
  return RomPtr(((uint32)bank << 16) | addr);
}

static uint16 MiniVramIncrement(void) {
  switch (g_mini_ppu.vmain & 3) {
  case 0:
    return 1;
  case 1:
    return 32;
  default:
    return 128;
  }
}

static void MiniAdvanceVramAddr(bool accessed_high) {
  if (((g_mini_ppu.vmain & 0x80) != 0) == accessed_high)
    g_mini_ppu.vram_addr += MiniVramIncrement();
}

static void MiniWriteVramData(bool high, uint8 value) {
  size_t offset = ((size_t)g_mini_ppu.vram_addr << 1) + (high ? 1 : 0);
  if (offset < kMiniPpuVramSize)
    g_mini_vram[offset] = value;
  MiniAdvanceVramAddr(high);
}

static uint8 MiniReadVramData(bool high) {
  size_t offset = ((size_t)g_mini_ppu.vram_addr << 1) + (high ? 1 : 0);
  uint8 value = offset < kMiniPpuVramSize ? g_mini_vram[offset] : 0;
  MiniAdvanceVramAddr(high);
  return value;
}

static void MiniWriteCgData(uint8 value) {
  if (!g_mini_ppu.cgram_low_pending) {
    g_mini_ppu.cgram_latch = value;
    g_mini_ppu.cgram_low_pending = true;
    return;
  }
  target_palettes[g_mini_ppu.cgadd & 0xFF] = g_mini_ppu.cgram_latch | (value << 8);
  g_mini_ppu.cgadd++;
  g_mini_ppu.cgram_low_pending = false;
}

static void MiniWriteBbus(uint8 reg, uint8 value) {
  switch (reg) {
  case 0x18:
    MiniWriteVramData(false, value);
    break;
  case 0x19:
    MiniWriteVramData(true, value);
    break;
  case 0x22:
    MiniWriteCgData(value);
    break;
  default:
    break;
  }
}

static void MiniRunDmaChannel1(void) {
  const uint8 *src = MiniBusPtr(g_mini_ppu.a1b1, g_mini_ppu.a1t1);
  bool fixed_source = (g_mini_ppu.dmap1 & 8) != 0;
  switch (g_mini_ppu.dmap1 & 7) {
  case 0:
    for (uint16 i = 0; i < g_mini_ppu.das1; i++) {
      MiniWriteBbus(g_mini_ppu.bbad1, *src);
      if (!fixed_source)
        src++;
    }
    break;
  case 1:
    for (uint16 i = 0; i < g_mini_ppu.das1; i++) {
      MiniWriteBbus(g_mini_ppu.bbad1 + (i & 1), *src);
      if (!fixed_source)
        src++;
    }
    break;
  default:
    break;
  }
}

void MiniPpu_Reset(void) {
  memset(g_mini_vram, 0, sizeof(g_mini_vram));
  memset(&g_mini_ppu, 0, sizeof(g_mini_ppu));
}

void MiniPpu_InitGameplay(void) {
  reg_OBSEL = 3;
  reg_BGMODE = 9;
  reg_BG1SC = 0x51;
  reg_BG2SC = 0x49;
  reg_BG3SC = 0x5A;
  reg_BG4SC = 0;
  reg_BG12NBA = 0;
  reg_BG34NBA = 4;
  reg_BG1HOFS = layer1_x_pos;
  reg_BG1VOFS = layer1_y_pos;
  reg_BG2HOFS = layer1_x_pos;
  reg_BG2VOFS = layer1_y_pos;
}

void MiniPpu_CopyVram(uint16 vram_dst, const void *src, size_t size) {
  size_t dst = (size_t)vram_dst << 1;
  if (dst >= kMiniPpuVramSize || size == 0)
    return;
  if (size > kMiniPpuVramSize - dst)
    size = kMiniPpuVramSize - dst;
  memcpy(g_mini_vram + dst, src, size);
}

uint8 *MiniPpu_GetVram(void) {
  return g_mini_vram;
}

void MiniPpu_SaveSnapshot(MiniPpuSnapshot *snapshot) {
  snapshot->vram_addr = g_mini_ppu.vram_addr;
  snapshot->vmain = g_mini_ppu.vmain;
  snapshot->a1t1 = g_mini_ppu.a1t1;
  snapshot->a1b1 = g_mini_ppu.a1b1;
  snapshot->das1 = g_mini_ppu.das1;
  snapshot->dmap1 = g_mini_ppu.dmap1;
  snapshot->bbad1 = g_mini_ppu.bbad1;
  snapshot->cgadd = g_mini_ppu.cgadd;
  snapshot->cgram_latch = g_mini_ppu.cgram_latch;
  snapshot->cgram_low_pending = g_mini_ppu.cgram_low_pending;
  memcpy(snapshot->vram, g_mini_vram, sizeof(snapshot->vram));
}

void MiniPpu_LoadSnapshot(const MiniPpuSnapshot *snapshot) {
  g_mini_ppu.vram_addr = snapshot->vram_addr;
  g_mini_ppu.vmain = snapshot->vmain;
  g_mini_ppu.a1t1 = snapshot->a1t1;
  g_mini_ppu.a1b1 = snapshot->a1b1;
  g_mini_ppu.das1 = snapshot->das1;
  g_mini_ppu.dmap1 = snapshot->dmap1;
  g_mini_ppu.bbad1 = snapshot->bbad1;
  g_mini_ppu.cgadd = snapshot->cgadd;
  g_mini_ppu.cgram_latch = snapshot->cgram_latch;
  g_mini_ppu.cgram_low_pending = snapshot->cgram_low_pending;
  memcpy(g_mini_vram, snapshot->vram, sizeof(snapshot->vram));
}

void WriteReg(uint16 reg, uint8 value) {
  switch (reg) {
  case VMAIN:
    g_mini_ppu.vmain = value;
    break;
  case VMADDL:
    g_mini_ppu.vram_addr = (g_mini_ppu.vram_addr & 0xFF00) | value;
    break;
  case VMADDH:
    g_mini_ppu.vram_addr = (g_mini_ppu.vram_addr & 0x00FF) | (value << 8);
    break;
  case VMDATAL:
    MiniWriteVramData(false, value);
    break;
  case VMDATAH:
    MiniWriteVramData(true, value);
    break;
  case DMAP1:
    g_mini_ppu.dmap1 = value;
    break;
  case BBAD1:
    g_mini_ppu.bbad1 = value;
    break;
  case A1T1L:
    g_mini_ppu.a1t1 = (g_mini_ppu.a1t1 & 0xFF00) | value;
    break;
  case A1T1H:
    g_mini_ppu.a1t1 = (g_mini_ppu.a1t1 & 0x00FF) | (value << 8);
    break;
  case A1B1:
    g_mini_ppu.a1b1 = value;
    break;
  case DAS1L:
    g_mini_ppu.das1 = (g_mini_ppu.das1 & 0xFF00) | value;
    break;
  case DAS1H:
    g_mini_ppu.das1 = (g_mini_ppu.das1 & 0x00FF) | (value << 8);
    break;
  case CGADD:
    g_mini_ppu.cgadd = value;
    g_mini_ppu.cgram_low_pending = false;
    break;
  case CGDATA:
    MiniWriteCgData(value);
    break;
  case MDMAEN:
    if (value & 2)
      MiniRunDmaChannel1();
    break;
  default:
    break;
  }
}

uint8 ReadReg(uint16 reg) {
  switch (reg) {
  case RDVRAML:
    return MiniReadVramData(false);
  case RDVRAMH:
    return MiniReadVramData(true);
  default:
    return 0;
  }
}

uint16 ReadRegWord(uint16 reg) {
  uint16 value = ReadReg(reg);
  value |= (uint16)ReadReg(reg + 1) << 8;
  return value;
}

void WriteRegWord(uint16 reg, uint16 value) {
  WriteReg(reg, (uint8)value);
  WriteReg(reg + 1, value >> 8);
}
