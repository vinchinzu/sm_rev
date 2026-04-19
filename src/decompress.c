// ROM decompression routines (0x80B119, 0x80B271)

#include "sm_rtl.h"
#include "variables.h"
#include "funcs.h"

static uint32 decompress_src;

static uint8 DecompNextByte() {
  uint8 b = *RomPtr(decompress_src);
  if ((decompress_src++ & 0xffff) == 0xffff)
    decompress_src += 0x8000;
  return b;
}

void DecompressToMem(uint32 src, uint8 *decompress_dst) {  // 0x80B119
  decompress_src = src;

  int src_pos, dst_pos = 0;
  while (1) {
    int len;
    uint8 cmd, b;
    b = DecompNextByte();
    if (b == 0xFF)
      break;
    if ((b & 0xE0) == 0xE0) {
      cmd = (8 * b) & 0xE0;
      len = ((b & 3) << 8 | DecompNextByte()) + 1;
    } else {
      cmd = b & 0xE0;
      len = (b & 0x1F) + 1;
    }
    if (cmd & 0x80) {
      uint8 want_xor = cmd & 0x20 ? 0xff : 0;
      if (cmd >= 0xC0) {
        src_pos = dst_pos - DecompNextByte();
      } else {
        src_pos = DecompNextByte();
        src_pos += DecompNextByte() * 256;
      }
      do {
        b = decompress_dst[src_pos++] ^ want_xor;
        decompress_dst[dst_pos++] = b;
      } while (--len);
    } else {
      switch (cmd) {
      case 0x20:
        b = DecompNextByte();
        do {
          decompress_dst[dst_pos++] = b;
        } while (--len);
        break;
      case 0x40: {
        b = DecompNextByte();
        uint8 b2 = DecompNextByte();
        do {
          decompress_dst[dst_pos++] = b;
          if (!--len)
            break;
          decompress_dst[dst_pos++] = b2;
        } while (--len);
        break;
      }
      case 0x60:
        b = DecompNextByte();
        do {
          decompress_dst[dst_pos++] = b++;
        } while (--len);
        break;
      default:
        do {
          b = DecompNextByte();
          decompress_dst[dst_pos++] = b;
        } while (--len);
        break;
      }
    }
  }
}

static uint8 ReadPpuByte(uint16 addr) {
  WriteRegWord(VMADDL, addr >> 1);
  ReadRegWord(RDVRAML);  // latch
  uint16 data = ReadRegWord(RDVRAML);
  return (addr & 1) ? GET_HIBYTE(data) : data;
}

void DecompressToVRAM(uint32 src, uint16 dst_addr) {  // 0x80B271
  decompress_src = src;
  int src_pos, dst_pos = dst_addr;
  while (1) {
    int len;
    uint8 b = DecompNextByte(), cmd;
    if (b == 0xFF)
      break;
    if ((b & 0xE0) == 0xE0) {
      cmd = (8 * b) & 0xE0;
      len = ((b & 3) << 8 | DecompNextByte()) + 1;
    } else {
      cmd = b & 0xE0;
      len = (b & 0x1F) + 1;
    }
    if (cmd & 0x80) {
      uint8 want_xor = cmd & 0x20 ? 0xff : 0;
      if (cmd >= 0xC0) {
        src_pos = dst_pos - DecompNextByte();
      } else {
        src_pos = DecompNextByte();
        src_pos += DecompNextByte() * 256;
        src_pos += dst_addr;
      }
      do {
        b = ReadPpuByte(src_pos++) ^ want_xor;
        WriteRegWord(VMADDL, dst_pos >> 1);
        WriteReg(VMDATAL + (dst_pos++ & 1), b);
      } while (--len);
    } else {
      switch (cmd) {
      case 0x20:
        b = DecompNextByte();
        do {
          WriteReg(VMDATAL + (dst_pos++ & 1), b);
        } while (--len);
        break;
      case 0x40: {
        b = DecompNextByte();
        uint8 b2 = DecompNextByte();
        do {
          WriteReg(VMDATAL + (dst_pos++ & 1), b);
          if (!--len)
            break;
          WriteReg(VMDATAL + (dst_pos++ & 1), b2);
        } while (--len);
        break;
      }
      case 0x60:
        b = DecompNextByte();
        do {
          WriteReg(VMDATAL + (dst_pos++ & 1), b++);
        } while (--len);
        break;
      default:
        do {
          b = DecompNextByte();
          WriteReg(VMDATAL + (dst_pos++ & 1), b);
        } while (--len);
        break;
      }
    }
  }
}
