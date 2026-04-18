// NMI / PPU transfer routines

#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"

static void NMI_ProcessMode7QueueInner(const uint8 *p);
static void Nmi_ProcessHorizScrollingDma(void);
static void Nmi_ProcessVertScrollingDma(void);
static void NmiTransferSamusHalfToVram(uint16 tile_src, uint16 top_vram_addr, uint16 bottom_vram_addr);

void QueueMode7Transfers(uint8 db, uint16 k) {  // 0x808B4F
  const uint8 *p = RomPtrWithBank(db, k);
  uint8 *dst = (uint8 *)mode7_write_queue + mode7_vram_write_queue_tail;
  for (;;) {
    int f = GET_BYTE(p);
    if (f & 0x80) {
      dst[9] = 0;
      memcpy(dst, p, 9);
      p += 9, dst += 9;
    } else if (f & 0x40) {
      dst[7] = 0;
      memcpy(dst, p, 7);
      p += 7, dst += 7;
    } else {
      break;
    }
  }
  mode7_vram_write_queue_tail = dst - (uint8 *)mode7_write_queue;
}

void NMI_ProcessMode7Queue(void) {  // 0x808BBA
  if (mode7_vram_write_queue_tail) {
    NMI_ProcessMode7QueueInner((uint8 *)mode7_write_queue);
    *(uint16 *)&mode7_write_queue[0].field_0 = 0;
    mode7_vram_write_queue_tail = 0;
  }
}

static void NMI_ProcessMode7QueueInner(const uint8 *p) {  // 0x808BD3
  while (1) {
    uint8 v2;
    while (1) {
      v2 = *p;
      if ((v2 & 0x80) == 0)
        break;
      WriteReg(DMAP1, v2 & 0x1F);
      Mode7CgvmWriteQueue *v5 = (Mode7CgvmWriteQueue *)p;
      WriteRegWord(A1T1L, v5->src_addr.addr);
      WriteReg(A1B1, v5->src_addr.bank);
      WriteRegWord(DAS1L, v5->count);
      if (v2 & 0x40)
        WriteReg(BBAD1, 0x19);
      else
        WriteReg(BBAD1, 0x18);
      WriteRegWord(VMADDL, v5->vram_addr);
      WriteReg(VMAIN, v5->vmain);
      WriteReg(MDMAEN, 2);
      p += sizeof(Mode7CgvmWriteQueue);
    }
    if (!(v2 & 0x40))
      break;
    WriteReg(DMAP1, v2 & 0x1F);
    WriteRegWord(A1T1L, *(uint16 *)(p + 1));
    WriteReg(A1B1, p[3]);
    WriteRegWord(DAS1L, *((uint16 *)p + 2));
    WriteReg(BBAD1, 0x22);
    WriteReg(CGADD, p[6]);
    WriteReg(MDMAEN, 2);
    p += 7;
  }
}

void NMI_ProcessVramWriteQueue(void) {  // 0x808C83
  if (vram_write_queue_tail) {
    gVramWriteEntry(vram_write_queue_tail)->size = 0;
    WriteRegWord(DMAP1, 0x1801);
    for (int i = 0;; i += 7) {
      VramWriteEntry *e = gVramWriteEntry(i);
      if (!e->size)
        break;
      WriteRegWord(DAS1L, e->size);
      WriteRegWord(A1T1L, e->src.addr);
      WriteReg(A1B1, e->src.bank);
      WriteRegWord(VMAIN, sign16(e->vram_dst) ? 0x81 : 0x80);
      WriteRegWord(VMADDL, e->vram_dst);
      WriteReg(MDMAEN, 2);
    }
  }
  vram_write_queue_tail = 0;
  Nmi_ProcessHorizScrollingDma();
  Nmi_ProcessVertScrollingDma();
}

static void Nmi_ProcessHorizScrollingDma(void) {  // 0x808CD8
  WriteReg(VMAIN, 0x81);
  if ((uint8)bg1_update_col_enable) {
    LOBYTE(bg1_update_col_enable) = 0;
    uint16 v0 = bg1_update_col_unwrapped_dst;
    WriteRegWord(VMADDL, bg1_update_col_unwrapped_dst);
    WriteRegWord(DMAP1, 0x1801);
    WriteRegWord(A1T1L, ADDR16_OF_RAM(*bg1_column_update_tilemap_left_halves));
    WriteReg(A1B1, 0x7E);
    uint16 v1 = bg1_update_col_unwrapped_size;
    WriteRegWord(DAS1L, bg1_update_col_unwrapped_size);
    WriteReg(MDMAEN, 2);
    WriteRegWord(VMADDL, v0 + 1);
    WriteRegWord(DAS1L, v1);
    WriteRegWord(A1T1L, ADDR16_OF_RAM(*bg1_column_update_tilemap_right_halves));
    WriteReg(MDMAEN, 2);
    WriteRegWord(A1T1L, bg1_update_col_wrapped_left_src);
    uint16 v2 = bg1_update_col_wrapped_size;
    if (bg1_update_col_wrapped_size) {
      WriteRegWord(DAS1L, bg1_update_col_wrapped_size);
      uint16 v3 = bg1_update_col_wrapped_dst;
      WriteRegWord(VMADDL, bg1_update_col_wrapped_dst);
      WriteReg(MDMAEN, 2);
      WriteRegWord(VMADDL, v3 + 1);
      WriteRegWord(DAS1L, v2);
      WriteRegWord(A1T1L, bg1_update_col_wrapped_right_src);
      WriteReg(MDMAEN, 2);
    }
  }
  if ((uint8)bg2_update_col_enable) {
    LOBYTE(bg2_update_col_enable) = 0;
    uint16 v4 = bg2_update_col_unwrapped_dst;
    WriteRegWord(VMADDL, bg2_update_col_unwrapped_dst);
    WriteRegWord(DMAP1, 0x1801);
    WriteRegWord(A1T1L, ADDR16_OF_RAM(*bg2_column_update_tilemap_left_halves));
    WriteReg(A1B1, 0x7E);
    uint16 v5 = bg2_update_col_unwrapped_size;
    WriteRegWord(DAS1L, bg2_update_col_unwrapped_size);
    WriteReg(MDMAEN, 2);
    WriteRegWord(VMADDL, v4 + 1);
    WriteRegWord(DAS1L, v5);
    WriteRegWord(A1T1L, ADDR16_OF_RAM(*bg2_column_update_tilemap_right_halves));
    WriteReg(MDMAEN, 2);
    WriteRegWord(A1T1L, bg2_update_col_wrapped_left_src);
    uint16 v6 = bg2_update_col_wrapped_size;
    if (bg2_update_col_wrapped_size) {
      WriteRegWord(DAS1L, bg2_update_col_wrapped_size);
      uint16 v7 = bg2_update_col_wrapped_dst;
      WriteRegWord(VMADDL, bg2_update_col_wrapped_dst);
      WriteReg(MDMAEN, 2);
      WriteRegWord(VMADDL, v7 + 1);
      WriteRegWord(DAS1L, v6);
      WriteRegWord(A1T1L, bg2_update_col_wrapped_right_src);
      WriteReg(MDMAEN, 2);
    }
  }
}

static void Nmi_ProcessVertScrollingDma(void) {  // 0x808DAC
  WriteReg(VMAIN, 0x80);
  if ((uint8)bg1_update_row_enable) {
    LOBYTE(bg1_update_row_enable) = 0;
    uint16 v0 = bg1_update_row_unwrapped_dst;
    WriteRegWord(VMADDL, bg1_update_row_unwrapped_dst);
    WriteRegWord(DMAP1, 0x1801);
    WriteRegWord(A1T1L, ADDR16_OF_RAM(*bg1_column_update_tilemap_top_halves));
    WriteReg(A1B1, 0x7E);
    uint16 v1 = bg1_update_row_unwrapped_size;
    WriteRegWord(DAS1L, bg1_update_row_unwrapped_size);
    WriteReg(MDMAEN, 2);
    WriteRegWord(VMADDL, v0 | 0x20);
    WriteRegWord(DAS1L, v1);
    WriteRegWord(A1T1L, ADDR16_OF_RAM(*bg1_column_update_tilemap_bottom_halves));
    WriteReg(MDMAEN, 2);
    WriteRegWord(A1T1L, bg1_update_row_wrapped_top_src);
    uint16 v2 = bg1_update_row_wrapped_size;
    if (bg1_update_row_wrapped_size) {
      WriteRegWord(DAS1L, bg1_update_row_wrapped_size);
      uint16 v3 = bg1_update_row_wrapped_dst;
      WriteRegWord(VMADDL, bg1_update_row_wrapped_dst);
      WriteReg(MDMAEN, 2);
      WriteRegWord(VMADDL, v3 | 0x20);
      WriteRegWord(DAS1L, v2);
      WriteRegWord(A1T1L, bg1_update_row_wrapped_bottom_src);
      WriteReg(MDMAEN, 2);
    }
  }
  if ((uint8)bg2_update_row_enable) {
    LOBYTE(bg2_update_row_enable) = 0;
    uint16 v4 = bg2_update_row_unwrapped_dst;
    WriteRegWord(VMADDL, bg2_update_row_unwrapped_dst);
    WriteRegWord(DMAP1, 0x1801);
    WriteRegWord(A1T1L, ADDR16_OF_RAM(*bg2_column_update_tilemap_top_halves));
    WriteReg(A1B1, 0x7E);
    uint16 v5 = bg2_update_row_unwrapped_size;
    WriteRegWord(DAS1L, bg2_update_row_unwrapped_size);
    WriteReg(MDMAEN, 2);
    WriteRegWord(VMADDL, v4 | 0x20);
    WriteRegWord(DAS1L, v5);
    WriteRegWord(A1T1L, ADDR16_OF_RAM(*bg2_column_update_tilemap_bottom_halves));
    WriteReg(MDMAEN, 2);
    WriteRegWord(A1T1L, bg2_update_row_wrapped_top_src);
    uint16 v6 = bg2_update_row_wrapped_size;
    if (bg2_update_row_wrapped_size) {
      WriteRegWord(DAS1L, bg2_update_row_wrapped_size);
      uint16 v7 = bg2_update_row_wrapped_dst;
      WriteRegWord(VMADDL, bg2_update_row_wrapped_dst);
      WriteReg(MDMAEN, 2);
      WriteRegWord(VMADDL, v7 | 0x20);
      WriteRegWord(DAS1L, v6);
      WriteRegWord(A1T1L, bg2_update_row_wrapped_bottom_src);
      WriteReg(MDMAEN, 2);
    }
  }
}

void NMI_ProcessVramReadQueue(void) {  // 0x808EA2
  if ((uint8)vram_read_queue_tail) {
    *((uint8 *)&vram_read_queue[0].vram_target + (uint8)vram_read_queue_tail) = 0;
    int v0 = 0;
    WriteReg(VMAIN, 0x80);
    while (vram_read_queue[v0].vram_target) {
      WriteRegWord(VMADDL, vram_read_queue[v0].vram_target);
      ReadRegWord(RDVRAML);
      WriteRegWord(DMAP1, vram_read_queue[v0].dma_parameters);
      WriteRegWord(A1T1L, vram_read_queue[v0].src.addr);
      WriteRegWord(A1T1H, *(VoidP *)((uint8 *)&vram_read_queue[v0].src.addr + 1));
      WriteRegWord(DAS1L, vram_read_queue[v0].size);
      WriteRegWord(DAS10, 0);
      WriteRegWord(A2A1H, 0);
      WriteReg(MDMAEN, 2);
      ++v0;
    }
    LOBYTE(vram_read_queue_tail) = 0;
  }
}

void SetupDmaTransfer(const void *p) {  // 0x8091A9
  const StartDmaCopy *s = (const StartDmaCopy *)p;

  int v2 = s->chan * 16;
  WriteRegWord((SnesRegs)(v2 + DMAP0), *(uint16 *)&s->dmap);
  WriteRegWord((SnesRegs)(v2 + A1T0L), *(uint16 *)&s->a1.addr);
  WriteReg((SnesRegs)(v2 + A1B0), s->a1.bank);
  WriteRegWord((SnesRegs)(v2 + DAS0L), s->das);
}

void NmiUpdateIoRegisters(void) {  // 0x8091EE
  WriteReg(NMITIMEN, reg_NMITIMEN);
  WriteReg(INIDISP, reg_INIDISP);
  WriteReg(OBSEL, reg_OBSEL);
  WriteReg(BGMODE, reg_BGMODE);
  WriteReg(MOSAIC, reg_MOSAIC);
  WriteReg(BG1SC, reg_BG1SC);
  WriteReg(BG2SC, reg_BG2SC);
  WriteReg(BG3SC, reg_BG3SC);
  WriteReg(BG4SC, reg_BG4SC);
  WriteReg(BG12NBA, reg_BG12NBA);
  WriteReg(BG34NBA, reg_BG34NBA);
  WriteReg(M7SEL, reg_M7SEL);
  WriteReg(W12SEL, reg_W12SEL);
  WriteReg(W34SEL, reg_W34SEL);
  WriteReg(WOBJSEL, reg_WOBJSEL);
  WriteReg(WH0, reg_WH0);
  WriteReg(WH1, reg_WH1);
  WriteReg(WH2, reg_WH2);
  WriteReg(WH3, reg_WH3);
  WriteReg(WBGLOG, reg_WBGLOG);
  WriteReg(WOBJLOG, reg_WOBJLOG);
  gameplay_TM = reg_TM;
  WriteReg(TM, reg_TM);
  WriteReg(TMW, reg_TMW);
  WriteReg(TS, reg_TS);
  WriteReg(TSW, reg_TSW);
  WriteReg(CGWSEL, reg_CGWSEL);
  WriteReg(CGADSUB, reg_CGADSUB);
  gameplay_CGWSEL = next_gameplay_CGWSEL;
  gameplay_CGADSUB = next_gameplay_CGADSUB;
  WriteReg(COLDATA, reg_COLDATA[0]);
  WriteReg(COLDATA, reg_COLDATA[1]);
  WriteReg(COLDATA, reg_COLDATA[2]);
  WriteReg(SETINI, reg_SETINI);
  WriteReg(BG1HOFS, reg_BG1HOFS);
  WriteReg(BG1HOFS, HIBYTE(reg_BG1HOFS));
  WriteReg(BG1VOFS, reg_BG1VOFS);
  WriteReg(BG1VOFS, HIBYTE(reg_BG1VOFS));
  WriteReg(BG2HOFS, reg_BG2HOFS);
  WriteReg(BG2HOFS, HIBYTE(reg_BG2HOFS));
  WriteReg(BG2VOFS, reg_BG2VOFS);
  WriteReg(BG2VOFS, HIBYTE(reg_BG2VOFS));
  WriteReg(BG3HOFS, reg_BG3HOFS);
  WriteReg(BG3HOFS, HIBYTE(reg_BG3HOFS));
  WriteReg(BG3VOFS, reg_BG3VOFS);
  WriteReg(BG3VOFS, HIBYTE(reg_BG3VOFS));
  WriteReg(BG4HOFS, reg_BG4HOFS);
  WriteReg(BG4HOFS, HIBYTE(reg_BG4HOFS));
  WriteReg(BG4VOFS, reg_BG4VOFS);
  WriteReg(BG4VOFS, HIBYTE(reg_BG4VOFS));
  HIBYTE(hdma_data_table_in_ceres) = reg_BGMODE_fake;
  if ((reg_BGMODE & 7) == 7 || (reg_BGMODE_fake & 7) == 7) {
    WriteReg(M7A, reg_M7A);
    WriteReg(M7A, HIBYTE(reg_M7A));
    WriteReg(M7B, reg_M7B);
    WriteReg(M7B, HIBYTE(reg_M7B));
    WriteReg(M7C, reg_M7C);
    WriteReg(M7C, HIBYTE(reg_M7C));
    WriteReg(M7D, reg_M7D);
    WriteReg(M7D, HIBYTE(reg_M7D));
    WriteReg(M7X, reg_M7X);
    WriteReg(M7X, HIBYTE(reg_M7X));
    WriteReg(M7Y, reg_M7Y);
    WriteReg(M7Y, HIBYTE(reg_M7Y));
  }
}

void NmiUpdatePalettesAndOam(void) {  // 0x80933A
  WriteRegWord(DMAP0, 0x400);
  WriteRegWord(A1T0L, ADDR16_OF_RAM(*oam_ent));
  WriteReg(A1B0, 0);
  WriteRegWord(DAS0L, 0x220);
  WriteRegWord(OAMADDL, 0);
  WriteRegWord(DMAP1, 0x2200);
  WriteRegWord(A1T1L, ADDR16_OF_RAM(*palette_buffer));
  WriteReg(A1B1, 0x7E);
  WriteRegWord(DAS1L, 0x200);
  WriteReg(CGADD, 0);
  WriteReg(MDMAEN, 3);
}

static void NmiTransferSamusHalfToVram(uint16 tile_src, uint16 top_vram_addr, uint16 bottom_vram_addr) {
  SamusTileAnimationTileDefs *td = (SamusTileAnimationTileDefs *)RomPtr_92(tile_src);
  WriteRegWord(VMADDL, top_vram_addr);
  WriteRegWord(DMAP1, 0x1801);
  WriteRegWord(A1T1L, td->src.addr);
  WriteReg(A1B1, td->src.bank);
  WriteRegWord(DAS1L, td->part1_size);
  WriteReg(MDMAEN, 2);
  WriteRegWord(VMADDL, bottom_vram_addr);
  WriteRegWord(A1T1L, td->src.addr + td->part1_size);
  if (td->part2_size) {
    WriteRegWord(DAS1L, td->part2_size);
    WriteReg(MDMAEN, 2);
  }
}

void NmiTransferSamusToVram(void) {  // 0x809376
  WriteReg(VMAIN, 0x80);
  if ((uint8)nmi_copy_samus_halves)
    NmiTransferSamusHalfToVram(nmi_copy_samus_top_half_src, 0x6000, 0x6100);
  if (HIBYTE(nmi_copy_samus_halves))
    NmiTransferSamusHalfToVram(nmi_copy_samus_bottom_half_src, 0x6080, 0x6180);
}

void NmiProcessAnimtilesVramTransfers(void) {  // 0x809416
  if ((animtiles_enable_flag & 0x8000) != 0) {
    for (int i = 10; (i & 0x80) == 0; i -= 2) {
      int v1 = i >> 1;
      if (animtiles_ids[v1]) {
        if (animtiles_src_ptr[v1]) {
          WriteRegWord(A1T0L, animtiles_src_ptr[v1]);
          WriteReg(A1B0, 0x87);
          WriteRegWord(DMAP0, 0x1801);
          WriteRegWord(DAS0L, animtiles_sizes[v1]);
          WriteRegWord(VMADDL, animtiles_vram_ptr[v1]);
          WriteReg(VMAIN, 0x80);
          WriteReg(MDMAEN, 1);
          animtiles_src_ptr[v1] = 0;
        }
      }
    }
  }
}

void Vector_NMI(void) {  // 0x809583
  if (waiting_for_nmi) {
    NmiUpdatePalettesAndOam();
    NmiTransferSamusToVram();
    NmiProcessAnimtilesVramTransfers();
    NmiUpdateIoRegisters();
    for (int i = 0; i != 6; ++i) {
      if (hdma_object_channels_bitmask[i])
        WriteRegWord((SnesRegs)(LOBYTE(hdma_object_bank_slot[i]) + 17154), hdma_object_table_pointers[i]);
    }
    if (reg_BGMODE == 7 || reg_BGMODE_fake == 7)
      NMI_ProcessMode7Queue();
    NMI_ProcessVramWriteQueue();
    NMI_ProcessVramReadQueue();
    WriteReg(HDMAEN, reg_HDMAEN);
    waiting_for_nmi = 0;
    nmi_frames_missed = 0;
    ++nmi_frame_counter_byte;
    ++nmi_frame_counter_word;
  } else if (++nmi_frames_missed >= nmi_frames_missed_max) {
    nmi_frames_missed_max = nmi_frames_missed;
  }
  ++nmi_frame_counter_including_lag;
}

void CopyToVramNow(uint16 vram_dst, uint32 src, uint16 size) {
  // src can point either to ram or rom
  WriteReg(INIDISP, 0x80);
  WriteRegWord(VMADDL, vram_dst);
  WriteRegWord(DMAP1, 0x1801);
  WriteRegWord(A1T1L, (uint16)src);
  WriteReg(A1B1, src >> 16);
  WriteRegWord(DAS1L, size);
  WriteReg(VMAIN, 0x80);
  WriteReg(MDMAEN, 2);
  WriteReg(INIDISP, 0xF);
}
