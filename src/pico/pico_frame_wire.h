#ifndef SM_PICO_FRAME_WIRE_H_
#define SM_PICO_FRAME_WIRE_H_

#include "pico_frame_packet.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Pico frame wire v1 — little-endian, no struct padding, no framebuffer.
 *
 *   offset  size  field
 *        0     4  magic        'PFR1' = 0x31524650
 *        4     2  version      1
 *        6     2  flags        bit0 oam_full, bit1 cgram_full,
 *                              bit2 vram_blob present, bit3 vram_full
 *        8     4  frame_id
 *       12     2  vsync_token  kPicoFramePacketVsync (0x1D15)
 *       14     2  joypad_echo
 *       16     2  bg1hofs
 *       18     2  bg1vofs
 *       20     2  bg2hofs
 *       22     2  bg2vofs
 *       24     1  inidisp
 *       25     1  obsel
 *       26     1  bgmode
 *       27     1  bg1sc
 *       28     1  bg2sc
 *       29     1  bg12nba
 *       30     1  tm
 *       31     1  reserved     0
 *       32     4  vram_size    scene VRAM bytes (0 if none)
 *       36        -- header end --
 *       36   544  oam          512 main + 32 high table
 *      580   512  cgram        256 × u16 LE BGR555
 *     1092     n  vram blob    only if flags bit2; n = vram_size
 *
 * Live frames are ~1.1 KB (header + OAM + CGRAM). Room load may append a
 * VRAM blob (tens of KB). Do not stream a 256×224 framebuffer.
 *
 * Hardware UART (follow-up, not a blocker here): 1 000 000 baud 8N1,
 * game Pico TX → display Pico RX, one encoded packet per vsync. At 1 Mbaud
 * ~1.1 KB fits 60 Hz; a 64 KB VRAM burst is a room-load, not a per-frame
 * stream. SPI/PIO is sm_rev-of6.
 */
enum {
  kPicoFrameWireMagic = 0x31524650u, /* 'PFR1' LE */
  kPicoFrameWireVersion = 1,
  kPicoFrameWireHeaderSize = 36,
  kPicoFrameWireBaseSize = 36 + 544 + 512, /* no VRAM blob */
  kPicoFrameWireMaxSize = 36 + 544 + 512 + 0x10000,
  kPicoFrameWireFlagOam = 0x0001,
  kPicoFrameWireFlagCgram = 0x0002,
  kPicoFrameWireFlagVramBlob = 0x0004,
  kPicoFrameWireFlagVramFull = 0x0008
};

/* Bytes Encode will write. 0 if pkt is unusable. */
size_t PicoFrameWire_EncodedSize(const PicoFramePacket *pkt, int include_vram_blob);

/* Serialize pkt into buf. include_vram_blob copies vram[] after CGRAM.
 * Returns bytes written, or 0 on error. */
size_t PicoFrameWire_Encode(const PicoFramePacket *pkt, uint8_t *buf, size_t cap,
                            int include_vram_blob);

/* Unpack buf into pkt. If a VRAM blob is present, copy it into vram
 * (cap must be >= vram_size) and set pkt->vram = vram. If no blob,
 * pkt->vram = vram (display-side retained scene; may be NULL).
 * Returns 1 on success, 0 on error. Trailing bytes past the packet are ok. */
int PicoFrameWire_Decode(const uint8_t *buf, size_t len, PicoFramePacket *pkt,
                         uint8_t *vram, size_t vram_cap);

#ifdef __cplusplus
}
#endif

#endif /* SM_PICO_FRAME_WIRE_H_ */
