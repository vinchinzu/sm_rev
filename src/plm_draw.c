// PLM draw instruction processing and tile rendering.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"

static inline uint8 *RomPtr_84orRAM(uint16_t addr) {
  if (addr & 0x8000) {
    return (uint8*)RomPtr(0x840000 | addr);
  } else {
    assert(addr < 0x2000);
    return RomPtr_RAM(addr);
  }
}

void ProcessPlmDrawInstruction(uint16 v0) {  // 0x84861E
  int v1 = v0 >> 1;
  uint16 *p = (uint16 *)RomPtr_84orRAM(plm_instruction_draw_ptr[v1]);
  uint16 dst = plm_block_indices[v1], dst_base = dst;
  while (1) {
    uint16 v4 = *p++;
    if (v4 & 0x8000) {
      for (int i = 0, t = room_width_in_blocks * 2; i < (uint8)v4; i++)
        level_data[(dst + i * t) >> 1] = *p++;
    } else {
      for (int i = 0; i < (uint8)v4; i++)
        level_data[(dst + i * 2) >> 1] = *p++;
    }
    if (!*p)
      break;
    dst = dst_base + 2 * ((int8)*p + (int8)(*p >> 8) * room_width_in_blocks);
    p++;
  }
}

void CallPlmInstrFunc(uint32 ea) {
  switch (ea) {
  case fnVariaSuitPickup: VariaSuitPickup(); return;
  case fnGravitySuitPickup: GravitySuitPickup(); return;
  default: Unreachable();
  }
}

static void PartiallyQueueVramForSingleScreenPlm(uint16 a, uint16 k, uint16 r20, uint16 r28) {  // 0x849220
  VramWriteEntry *ent = gVramWriteEntry(k);
  ent->vram_dst = r28 | a;
  uint16 size = 4 * r20;
  VoidP base = plm_draw_tilemap_index + 0xC6C8;
  ent[0].size = size;
  ent[1].size = size;
  ent[0].src = (LongPtr){ base, 126 };
  ent[1].src = (LongPtr){ base + size, 126 };
  // R0/R6 assignments move to caller
}

static void CalculatePlmDrawTilemapVramDst(uint16 k, uint16 r9, uint16 r12, uint16 r20, uint16 r24, uint16 r26, uint16 r28) {  // 0x8491DC
  uint16 a;
  uint16 prod = (r26 & 0xF) * 0x40;
  uint16 v1 = r24 & 0x1F;
  if (v1 >= 0x10) {
    a = prod + r12 + 2 * v1;
    if ((bg1_x_offset & 0x100) != 0)
      a -= 1024;
  } else {
    a = prod + r9 + 2 * v1;
    if ((bg1_x_offset & 0x100) != 0)
      a += 1024;
  }
  PartiallyQueueVramForSingleScreenPlm(a, k, r20, r28);
}

void DrawPlm(uint16 k) {  // 0x848DAA
  int16 v2;
  int16 v5;
  int16 v8;
  VramWriteEntry *v10;
  VoidP v12;
  VoidP v13;
  VramWriteEntry *v15;
  int16 v18;
  int16 v20;
  bool v23; // sf
  VramWriteEntry *v25;
  int16 v28;
  int16 v31;
  uint16 v35, r26, r22;
  uint16 a, r24;
  uint16 r20, r18;
  uint16 r9 = addr_unk_605000;
  uint16 r12 = addr_unk_6053E0;
  uint16 v1 = plm_instruction_draw_ptr[k >> 1];
  uint16 x = plm_x_block;
  uint16 y = plm_y_block;
  uint16 r3;
  //LongPtr r0, r6;
  uint16 *r0, *r6;
LABEL_2:
  r26 = layer1_y_pos >> 4;
  if (sign16((layer1_y_pos >> 4) + 15 - y))
    return;
  v2 = *(uint16 *)RomPtr_84orRAM(v1);
  if (v2 < 0) {
    r20 = v2 & 0x7FFF;
    if (layer1_x_pos >> 4 == x || (int16)((layer1_x_pos >> 4) - x) < 0) {
      v20 = (layer1_x_pos >> 4) + 17;
      if (v20 != x && (int16)(v20 - x) >= 0) {
        r24 = x;
        r18 = 0;
        r22 = r26 + 16;
        if ((int16)(r26 - y) < 0) {
          r26 = y;
        } else {
          r18 = r26 - y;
          if (sign16(y + r20 - r26))
            return;
          bool v21 = r20 == r18;
          bool v22 = (int16)(r20 - r18) < 0;
          r20 -= r18;
          if (v22)
            Unreachable();
          if (v21)
            return;
        }
        r22 = r20 + r26 - r22;
        if ((r22 & 0x8000) != 0 || (v23 = (int16)(r20 - r22) < 0, (r20 -= r22) != 0) && !v23) {
          v35 = k;
          uint16 v24;
          v24 = vram_write_queue_tail;
          if ((int16)(vram_write_queue_tail - 240) < 0
              && !sign16(((uint16)(512 - plm_draw_tilemap_index) >> 3) - r20)) {
            CalculatePlmDrawTilemapVramDst(vram_write_queue_tail, r9, r12, r20, r24, r26, 0x8000);
            v25 = gVramWriteEntry(v24);
            v25[1].vram_dst = v25[0].vram_dst + 1;
            r0 = (uint16 *)(g_ram + v25[0].src.addr);
            r6 = (uint16 *)(g_ram + v25[1].src.addr);
            vram_write_queue_tail = v24 + 14;
            r18 *= 2;
            r3 = r18 + v1 + 2;
            uint16 v26;
            v26 = 0;
            while (1) {
              x = *(uint16 *)RomPtr_84orRAM(r3);
              uint16 v27, v29;
              v27 = x & 0x3FF;
              v28 = x & 0xC00;
              if ((x & 0xC00) != 0) {
                if (v28 == 1024) {
                  r0[v26 >> 1] = tile_table.tables[v27].top_right ^ 0x4000;
                  r6[v26 >> 1] = tile_table.tables[v27].top_left ^ 0x4000;
                  v29 = v26 + 2;
                  r0[v29 >> 1] = tile_table.tables[v27].bottom_right ^ 0x4000;
                  r6[v29 >> 1] = tile_table.tables[v27].bottom_left ^ 0x4000;
                } else if (v28 == 2048) {
                  r0[v26 >> 1] = tile_table.tables[v27].bottom_left ^ 0x8000;
                  r6[v26 >> 1] = tile_table.tables[v27].bottom_right ^ 0x8000;
                  v29 = v26 + 2;
                  r0[v29 >> 1] = tile_table.tables[v27].top_left ^ 0x8000;
                  r6[v29 >> 1] = tile_table.tables[v27].top_right ^ 0x8000;
                } else {
                  r0[v26 >> 1] = tile_table.tables[v27].bottom_right ^ 0xC000;
                  r6[v26 >> 1] = tile_table.tables[v27].bottom_left ^ 0xC000;
                  v29 = v26 + 2;
                  r0[v29 >> 1] = tile_table.tables[v27].top_right ^ 0xC000;
                  r6[v29 >> 1] = tile_table.tables[v27].top_left ^ 0xC000;
                }
              } else {
                r0[v26 >> 1] = tile_table.tables[v27].top_left;
                r6[v26 >> 1] = tile_table.tables[v27].top_right;
                v29 = v26 + 2;
                r0[v29 >> 1] = tile_table.tables[v27].bottom_left;
                r6[v29 >> 1] = tile_table.tables[v27].bottom_right;
              }
              v26 = v29 + 2;
              ++r3;
              ++r3;
              plm_draw_tilemap_index += 8;
              if (!sign16(plm_draw_tilemap_index - 512))
                break;
              if (!--r20) {
LABEL_70:
                k = v35;
                uint16 addr = r3;
                if ((r22 & 0x8000) == 0)
                  addr = r3 + 2 * r22;
                v31 = *(uint16 *)RomPtr_84orRAM(addr);
                if (v31) {
                  x = plm_x_block + (int8)v31;
                  uint16 v32 = addr + 1;
                  y = plm_y_block + (int8)*RomPtr_84orRAM(v32);
                  v1 = v32 + 1;
                  goto LABEL_2;
                }
                return;
              }
            }
          }
        }
      }
    }
  } else {
    r20 = v2 & 0x7FFF;
    if (sign16(y - r26))
      return;
    r26 = y;
    r18 = 0;
    r24 = x;
    r22 = ((uint16)(layer1_x_pos + 15) >> 4) - 1;
    if ((int16)(r22 - x) >= 0 && r22 != x) {
      r18 = r22 - x;
      if (x + r20 == r22 || (int16)(x + r20 - r22) < 0)
        return;
      r20 -= r18;
      r24 = r22;
    }
    r22 += 17;
    if (!sign16(r22 - x)) {
      r22 = r20 + r24 - 1 - r22;
      if ((r22 & 0x8000) != 0 || (r20 -= r22) != 0) {
        v35 = k;
        uint16 v3 = vram_write_queue_tail;
        if ((int16)(vram_write_queue_tail - 480) < 0
            && !sign16(((uint16)(512 - plm_draw_tilemap_index) >> 3) - r20)) {
          uint16 prod = (r26 & 0xF) * 0x40;
          uint16 v4 = r24 & 0x1F;
          if (v4 >= 0x10) {
            v8 = r12 + 2 * v4;
            a = prod + v8;
            if ((bg1_x_offset & 0x100) != 0)
              a -= 1024;
          } else {
            v5 = r9 + 2 * v4;
            uint16 RegWord = prod;
            a = RegWord + v5;
            if ((bg1_x_offset & 0x100) != 0)
              a += 1024;
          }
          x = 2 * r20;
          uint16 R34 = a & 0x1F;
          if (((2 * r20 + (a & 0x1F) - 1) & 0xFFE0) != 0) {
            if ((int16)(v3 - 228) >= 0 || (int16)(32 - R34) < 0)
              return;
            uint16 v9 = 2 * (32 - R34);
            v10 = gVramWriteEntry(v3);
            v10->size = v9;
            v10[2].size = v9;
            v10[0].vram_dst = a;
            v10[1].vram_dst = a & 0xFFE0 ^ 0x400;
            v10[3].vram_dst = v10[1].vram_dst + 32;
            v10[2].vram_dst = v10->vram_dst + 32;
            x = 4 * r20;
            uint16 v11 = 4 * r20 - v10->size;
            v10[1].size = v11;
            v10[3].size = v11;
            v12 = plm_draw_tilemap_index - 14648;
            v10->src.addr = plm_draw_tilemap_index - 14648;
            r0 = (uint16 *)(g_ram + v12);
            v13 = v10->size + v12;
            v10[1].src.addr = v13;
            uint16 v14 = v10[1].size + v13;
            v10[2].src.addr = v14;
            r6 = (uint16 *)(g_ram + v14);
            v10[3].src.addr = v10[2].size + v14;
            v10->src.bank = 126;
            v10[1].src.bank = 126;
            v10[2].src.bank = 126;
            v10[3].src.bank = 126;
            vram_write_queue_tail = v3 + 28;
          } else {
            PartiallyQueueVramForSingleScreenPlm(a, v3, r20, 0);
            v15 = gVramWriteEntry(v3);
            v15[1].vram_dst = v15[0].vram_dst + 32;
            r0 = (uint16 *)(g_ram + v15[0].src.addr);
            r6 = (uint16 *)(g_ram + v15[1].src.addr);
            vram_write_queue_tail = v3 + 14;
          }
          r18 *= 2;
          r3 = r18 + v1 + 2;
          uint16 v16 = 0;
          while (1) {
            x = *(uint16 *)RomPtr_84orRAM(r3);
            uint16 v17 = x & 0x3FF, v19;
            v18 = x & 0xC00;
            if ((x & 0xC00) != 0) {
              if (v18 == 1024) {
                r0[v16 >> 1] = tile_table.tables[v17].top_right ^ 0x4000;
                r6[v16 >> 1] = tile_table.tables[v17].bottom_right ^ 0x4000;
                v19 = v16 + 2;
                r0[v19 >> 1] = tile_table.tables[v17].top_left ^ 0x4000;
                r6[v19 >> 1] = tile_table.tables[v17].bottom_left ^ 0x4000;
              } else if (v18 == 2048) {
                r0[v16 >> 1] = tile_table.tables[v17].bottom_left ^ 0x8000;
                r6[v16 >> 1] = tile_table.tables[v17].top_left ^ 0x8000;
                v19 = v16 + 2;
                r0[v19 >> 1] = tile_table.tables[v17].bottom_right ^ 0x8000;
                r6[v19 >> 1] = tile_table.tables[v17].top_right ^ 0x8000;
              } else {
                r0[v16 >> 1] = tile_table.tables[v17].bottom_right ^ 0xC000;
                r6[v16 >> 1] = tile_table.tables[v17].top_right ^ 0xC000;
                v19 = v16 + 2;
                r0[v19 >> 1] = tile_table.tables[v17].bottom_left ^ 0xC000;
                r6[v19 >> 1] = tile_table.tables[v17].top_left ^ 0xC000;
              }
            } else {
              r0[v16 >> 1] = tile_table.tables[v17].top_left;
              r6[v16 >> 1] = tile_table.tables[v17].bottom_left;
              v19 = v16 + 2;
              r0[v19 >> 1] = tile_table.tables[v17].top_right;
              r6[v19 >> 1] = tile_table.tables[v17].bottom_right;
            }
            v16 = v19 + 2;
            r3 += 2;
            plm_draw_tilemap_index += 8;
            if (!sign16(plm_draw_tilemap_index - 512))
              break;
            if (!--r20)
              goto LABEL_70;
          }
        }
      }
    }
  }
}
