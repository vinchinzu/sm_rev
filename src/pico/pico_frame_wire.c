#include "pico_frame_wire.h"

#include <string.h>

static void wr_u16(uint8_t *p, uint16_t v) {
  p[0] = (uint8_t)v;
  p[1] = (uint8_t)(v >> 8);
}

static void wr_u32(uint8_t *p, uint32_t v) {
  p[0] = (uint8_t)v;
  p[1] = (uint8_t)(v >> 8);
  p[2] = (uint8_t)(v >> 16);
  p[3] = (uint8_t)(v >> 24);
}

static uint16_t rd_u16(const uint8_t *p) {
  return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t rd_u32(const uint8_t *p) {
  return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) |
         ((uint32_t)p[3] << 24);
}

static size_t wire_size(const PicoFramePacket *pkt, int include_vram_blob) {
  size_t n = (size_t)kPicoFrameWireBaseSize;
  if (include_vram_blob && pkt->vram_size > 0)
    n += pkt->vram_size;
  return n;
}

size_t PicoFrameWire_EncodedSize(const PicoFramePacket *pkt, int include_vram_blob) {
  if (pkt == NULL)
    return 0;
  if (pkt->vram_size > (size_t)kPicoVramSize)
    return 0;
  if (include_vram_blob && pkt->vram_size > 0 && pkt->vram == NULL)
    return 0;
  return wire_size(pkt, include_vram_blob);
}

size_t PicoFrameWire_Encode(const PicoFramePacket *pkt, uint8_t *buf, size_t cap,
                            int include_vram_blob) {
  uint16_t flags;
  size_t need;
  size_t cg;
  int i;

  need = PicoFrameWire_EncodedSize(pkt, include_vram_blob);
  if (need == 0 || buf == NULL || need > cap)
    return 0;

  flags = 0;
  if (pkt->oam_full)
    flags |= (uint16_t)kPicoFrameWireFlagOam;
  if (pkt->cgram_full)
    flags |= (uint16_t)kPicoFrameWireFlagCgram;
  if (pkt->vram_full)
    flags |= (uint16_t)kPicoFrameWireFlagVramFull;
  if (include_vram_blob && pkt->vram_size > 0)
    flags |= (uint16_t)kPicoFrameWireFlagVramBlob;

  wr_u32(buf + 0, kPicoFrameWireMagic);
  wr_u16(buf + 4, (uint16_t)kPicoFrameWireVersion);
  wr_u16(buf + 6, flags);
  wr_u32(buf + 8, pkt->frame_id);
  wr_u16(buf + 12, pkt->vsync_token);
  wr_u16(buf + 14, pkt->joypad_echo);
  wr_u16(buf + 16, pkt->bg1hofs);
  wr_u16(buf + 18, pkt->bg1vofs);
  wr_u16(buf + 20, pkt->bg2hofs);
  wr_u16(buf + 22, pkt->bg2vofs);
  buf[24] = pkt->inidisp;
  buf[25] = pkt->obsel;
  buf[26] = pkt->bgmode;
  buf[27] = pkt->bg1sc;
  buf[28] = pkt->bg2sc;
  buf[29] = pkt->bg12nba;
  buf[30] = pkt->tm;
  buf[31] = 0;
  wr_u32(buf + 32, (uint32_t)pkt->vram_size);

  memcpy(buf + kPicoFrameWireHeaderSize, pkt->oam, kPicoOamSize);
  memcpy(buf + kPicoFrameWireHeaderSize + kPicoOamSize, pkt->oam_hi, kPicoOamHiSize);

  cg = (size_t)kPicoFrameWireHeaderSize + kPicoOamSize + kPicoOamHiSize;
  for (i = 0; i < kPicoCgramColors; i++)
    wr_u16(buf + cg + (size_t)i * 2u, pkt->cgram[i]);

  if (flags & (uint16_t)kPicoFrameWireFlagVramBlob)
    memcpy(buf + (size_t)kPicoFrameWireBaseSize, pkt->vram, pkt->vram_size);

  return need;
}

int PicoFrameWire_Decode(const uint8_t *buf, size_t len, PicoFramePacket *pkt,
                         uint8_t *vram, size_t vram_cap) {
  uint16_t version;
  uint16_t flags;
  uint32_t vram_size;
  size_t need;
  size_t cg;
  int i;

  if (buf == NULL || pkt == NULL || len < (size_t)kPicoFrameWireBaseSize)
    return 0;
  if (rd_u32(buf + 0) != kPicoFrameWireMagic)
    return 0;

  version = rd_u16(buf + 4);
  if (version != (uint16_t)kPicoFrameWireVersion)
    return 0;

  flags = rd_u16(buf + 6);
  vram_size = rd_u32(buf + 32);
  if (vram_size > (uint32_t)kPicoVramSize)
    return 0;

  need = (size_t)kPicoFrameWireBaseSize;
  if (flags & (uint16_t)kPicoFrameWireFlagVramBlob)
    need += (size_t)vram_size;
  if (len < need)
    return 0;

  if ((flags & (uint16_t)kPicoFrameWireFlagVramBlob) && vram_size > 0) {
    if (vram == NULL || (size_t)vram_size > vram_cap)
      return 0;
  }

  memset(pkt, 0, sizeof(*pkt));
  pkt->frame_id = rd_u32(buf + 8);
  pkt->vsync_token = rd_u16(buf + 12);
  pkt->joypad_echo = rd_u16(buf + 14);
  pkt->bg1hofs = rd_u16(buf + 16);
  pkt->bg1vofs = rd_u16(buf + 18);
  pkt->bg2hofs = rd_u16(buf + 20);
  pkt->bg2vofs = rd_u16(buf + 22);
  pkt->inidisp = buf[24];
  pkt->obsel = buf[25];
  pkt->bgmode = buf[26];
  pkt->bg1sc = buf[27];
  pkt->bg2sc = buf[28];
  pkt->bg12nba = buf[29];
  pkt->tm = buf[30];
  pkt->oam_full = (uint8_t)((flags & (uint16_t)kPicoFrameWireFlagOam) != 0);
  pkt->cgram_full = (uint8_t)((flags & (uint16_t)kPicoFrameWireFlagCgram) != 0);
  pkt->vram_full = (uint8_t)((flags & (uint16_t)kPicoFrameWireFlagVramFull) != 0);
  pkt->vram_size = (size_t)vram_size;

  memcpy(pkt->oam, buf + kPicoFrameWireHeaderSize, kPicoOamSize);
  memcpy(pkt->oam_hi, buf + kPicoFrameWireHeaderSize + kPicoOamSize, kPicoOamHiSize);

  cg = (size_t)kPicoFrameWireHeaderSize + kPicoOamSize + kPicoOamHiSize;
  for (i = 0; i < kPicoCgramColors; i++)
    pkt->cgram[i] = rd_u16(buf + cg + (size_t)i * 2u);

  if (flags & (uint16_t)kPicoFrameWireFlagVramBlob) {
    if (vram_size > 0)
      memcpy(vram, buf + (size_t)kPicoFrameWireBaseSize, (size_t)vram_size);
    pkt->vram = vram;
  } else {
    pkt->vram = vram;
  }

  return 1;
}
