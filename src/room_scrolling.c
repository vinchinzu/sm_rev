// Room scrolling, background streaming, and door-transition scrolling helpers
// extracted from Bank $80.

#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"

static void UpdateScrollVarsUpdateMap(void);
static void UpdateBgGraphicsWhenScrolling(void);
static void CalculateBgScrollAndLayerPositionBlocks(void);
static void UploadBackgroundDataColumn(void);
static void UploadLevelDataColumn(void);
static void UpdateLevelOrBackgroundDataColumn(uint16 k);
static void UpdateBackgroundDataRow(void);
static void UpdateLevelDataRow(void);
static void UpdateLevelOrBackgroundDataRow(uint16 k);
static void UpdatePreviousLayerBlocks(void);
static void DoorTransitionScrollingSetup_Right(void);
static void DoorTransitionScrollingSetup_Left(void);
static void DoorTransitionScrollingSetup_Down(void);
static void DoorTransitionScrollingSetup_Up(void);
static void UpdateBgScrollOffsets(void);
static uint8 DoorTransition_Right(void);
static uint8 DoorTransition_Left(void);
static uint8 DoorTransition_Down(void);
static uint8 DoorTransition_Up(void);

void DisplayViewablePartOfRoom(void) {  // 0x80A176
  int v1, v2, v3;

  v1 = (reg_BG1SC - reg_BG2SC) << 8;
  size_of_bg2 = v1 & 0xF800;
  CalculateBgScrollAndLayerPositionBlocks();
  v2 = 0;
  do {
    v3 = v2;
    blocks_to_update_x_block = layer1_x_block;
    blocks_to_update_y_block = layer1_y_block;
    vram_blocks_to_update_x_block = bg1_x_block;
    vram_blocks_to_update_y_block = bg1_y_block;
    UploadLevelDataColumn();
    if (!(layer2_scroll_x & 1)) {
      blocks_to_update_x_block = layer2_x_block;
      blocks_to_update_y_block = layer2_y_block;
      vram_blocks_to_update_x_block = bg2_x_block;
      vram_blocks_to_update_y_block = bg2_y_block;
      UploadBackgroundDataColumn();
    }
    NMI_ProcessVramWriteQueue();
    ++layer1_x_block;
    ++bg1_x_block;
    ++layer2_x_block;
    ++bg2_x_block;
    ++v2;
  } while (v3 != 16);
}

void QueueClearingOfFxTilemap(void) {  // 0x80A211
  for (int i = 3838; i >= 0; i -= 2)
    *(uint16 *)((uint8 *)ram4000.xray_tilemaps + (uint16)i) = 6222;
  uint16 v1 = vram_write_queue_tail;
  VramWriteEntry *v2 = gVramWriteEntry(vram_write_queue_tail);
  v2->size = 3840;
  v2->src.addr = ADDR16_OF_RAM(ram4000);
  *(uint16 *)&v2->src.bank = 126;
  v2->vram_dst = addr_unk_605880;
  vram_write_queue_tail = v1 + 7;
}

void ClearBG2Tilemap(void) {  // 0x80A23F
  WriteRegWord(VMADDL, 0x4800);
  WriteRegWord(DMAP1, 0x1808);
  WriteRegWord(A1T1L, 0xA29A);
  WriteRegWord(A1B1, 0x80);
  WriteRegWord(DAS1L, 0x800);
  WriteReg(VMAIN, 0);
  WriteReg(MDMAEN, 2);
  WriteRegWord(VMADDL, 0x4800);
  WriteRegWord(DMAP1, 0x1908);
  WriteRegWord(A1T1L, 0xA29A);
  WriteRegWord(A1B1, 0x80);
  WriteRegWord(DAS1L, 0x800);
  WriteReg(VMAIN, 0x80);
  WriteReg(MDMAEN, 2);
}

void ClearFXTilemap(void) {  // 0x80A29C
  WriteRegWord(VMADDL, 0x5880);
  WriteRegWord(DMAP1, 0x1808);
  WriteRegWord(A1T1L, 0xA2F7);
  WriteRegWord(A1B1, 0x80);
  WriteRegWord(DAS1L, 0x780);
  WriteReg(VMAIN, 0);
  WriteReg(MDMAEN, 2);
  WriteRegWord(VMADDL, 0x5880);
  WriteRegWord(DMAP1, 0x1908);
  WriteRegWord(A1T1L, 0xA2F8);
  WriteRegWord(A1B1, 0x80);
  WriteRegWord(DAS1L, 0x780);
  WriteReg(VMAIN, 0x80);
  WriteReg(MDMAEN, 2);
}

uint8 CalculateLayer2Xpos(void) {  // 0x80A2F9
  if (!layer2_scroll_x) {
    layer2_x_pos = layer1_x_pos;
    return 0;
  }
  if (layer2_scroll_x != 1) {
    int t = layer2_scroll_x & 0xFE;
    uint8 var933 = t * LOBYTE(layer1_x_pos) >> 8;
    layer2_x_pos = t * HIBYTE(layer1_x_pos) + var933;
    return 0;
  }
  return 1;
}

uint8 CalculateLayer2Ypos(void) {  // 0x80A33A
  if (!layer2_scroll_y) {
    layer2_y_pos = layer1_y_pos;
    return 0;
  }
  if (layer2_scroll_y != 1) {
    int t = layer2_scroll_y & 0xFE;
    uint8 var933 = t * (uint8)layer1_y_pos >> 8;
    layer2_y_pos = t * HIBYTE(layer1_y_pos) + var933;
    return 0;
  }
  return 1;
}

void CalculateBgScrolls(void) {  // 0x80A37B
  reg_BG1HOFS = bg1_x_offset + layer1_x_pos;
  reg_BG1VOFS = bg1_y_offset + layer1_y_pos;
  reg_BG2HOFS = bg2_x_scroll + layer2_x_pos;
  reg_BG2VOFS = bg2_y_scroll + layer2_y_pos;
}

static void UpdateScrollVarsUpdateMap(void) {  // 0x80A3A0
  CalculateBgScrolls();
  UpdateBgGraphicsWhenScrolling();
}

void CalculateLayer2PosAndScrollsWhenScrolling(void) {  // 0x80A3AB
  if (!time_is_frozen_flag) {
    reg_BG1HOFS = bg1_x_offset + layer1_x_pos;
    reg_BG1VOFS = bg1_y_offset + layer1_y_pos;
    if (!CalculateLayer2Xpos())
      reg_BG2HOFS = bg2_x_scroll + layer2_x_pos;
    if (!CalculateLayer2Ypos())
      reg_BG2VOFS = bg2_y_scroll + layer2_y_pos;
    UpdateBgGraphicsWhenScrolling();
  }
}

static void UpdateBgGraphicsWhenScrolling(void) {  // 0x80A3DF
  CalculateBgScrollAndLayerPositionBlocks();
  int v0 = 0;
  bool v1 = (int16)(layer1_x_block - previous_layer1_x_block) < 0;
  if (layer1_x_block != previous_layer1_x_block) {
    previous_layer1_x_block = layer1_x_block;
    if (!v1)
      v0 = 16;
    blocks_to_update_x_block = layer1_x_block + v0;
    vram_blocks_to_update_x_block = bg1_x_block + v0;
    blocks_to_update_y_block = layer1_y_block;
    vram_blocks_to_update_y_block = bg1_y_block;
    UploadLevelDataColumn();
  }
  if (!(layer2_scroll_x & 1)) {
    int v2 = 0;
    bool v3 = (int16)(layer2_x_block - previous_layer2_x_block) < 0;
    if (layer2_x_block != previous_layer2_x_block) {
      previous_layer2_x_block = layer2_x_block;
      if (!v3)
        v2 = 16;
      blocks_to_update_x_block = layer2_x_block + v2;
      vram_blocks_to_update_x_block = bg2_x_block + v2;
      blocks_to_update_y_block = layer2_y_block;
      vram_blocks_to_update_y_block = bg2_y_block;
      UploadBackgroundDataColumn();
    }
  }
  int v4 = 1;
  bool v5 = (int16)(layer1_y_block - previous_layer1_y_block) < 0;
  if (layer1_y_block != previous_layer1_y_block) {
    previous_layer1_y_block = layer1_y_block;
    if (!v5)
      v4 = 15;
    blocks_to_update_y_block = layer1_y_block + v4;
    vram_blocks_to_update_y_block = bg1_y_block + v4;
    blocks_to_update_x_block = layer1_x_block;
    vram_blocks_to_update_x_block = bg1_x_block;
    UpdateLevelDataRow();
  }
  if (!(layer2_scroll_y & 1)) {
    int v6 = 1;
    bool v7 = (int16)(layer2_y_block - previous_layer2_y_block) < 0;
    if (layer2_y_block != previous_layer2_y_block) {
      previous_layer2_y_block = layer2_y_block;
      if (!v7)
        v6 = 15;
      blocks_to_update_y_block = layer2_y_block + v6;
      vram_blocks_to_update_y_block = bg2_y_block + v6;
      blocks_to_update_x_block = layer2_x_block;
      vram_blocks_to_update_x_block = bg2_x_block;
      UpdateBackgroundDataRow();
    }
  }
}

static void CalculateBgScrollAndLayerPositionBlocks(void) {  // 0x80A4BB
  bg1_x_block = reg_BG1HOFS >> 4;
  bg2_x_block = reg_BG2HOFS >> 4;
  uint16 v0 = layer1_x_pos >> 4;
  if (((layer1_x_pos >> 4) & 0x800) != 0)
    v0 |= 0xF000;
  layer1_x_block = v0;
  uint16 v1 = layer2_x_pos >> 4;
  if (((layer2_x_pos >> 4) & 0x800) != 0)
    v1 |= 0xF000;
  layer2_x_block = v1;
  bg1_y_block = reg_BG1VOFS >> 4;
  bg2_y_block = reg_BG2VOFS >> 4;
  uint16 v2 = layer1_y_pos >> 4;
  if (((layer1_y_pos >> 4) & 0x800) != 0)
    v2 |= 0xF000;
  layer1_y_block = v2;
  uint16 v3 = layer2_y_pos >> 4;
  if (((layer2_y_pos >> 4) & 0x800) != 0)
    v3 |= 0xF000;
  layer2_y_block = v3;
}

void HandleAutoscrolling_X(void) {  // 0x80A528
  int16 v1;
  int16 v3;
  int16 v6;
  uint16 v4;
  uint16 var933;

  if (!time_is_frozen_flag) {
    uint16 var939 = layer1_x_pos;
    if ((layer1_x_pos & 0x8000) != 0)
      layer1_x_pos = 0;
    uint16 v0 = swap16(room_width_in_scrolls - 1);
    if (v0 < layer1_x_pos)
      layer1_x_pos = v0;
    v1 = HIBYTE(layer1_x_pos);
    uint16 v2 = Mult8x8((uint16)(layer1_y_pos + 128) >> 8, room_width_in_scrolls) + v1;
    if (scrolls[v2]) {
      if (scrolls[(uint16)(v2 + 1)])
        return;
      var933 = layer1_x_pos & 0xFF00;
      uint16 v5 = var939 - absolute_moved_last_frame_x - 2;
      if ((int16)(v5 - (layer1_x_pos & 0xFF00)) < 0) {
        v4 = var933;
      } else {
        var939 = v5;
        v6 = HIBYTE(var939);
        if (scrolls[(uint16)(Mult8x8((uint16)(layer1_y_pos + 128) >> 8, room_width_in_scrolls) + v6)])
          v4 = var939;
        else
          v4 = (var939 & 0xFF00) + 256;
      }
    } else {
      var933 = (layer1_x_pos & 0xFF00) + 256;
      if ((uint16)(absolute_moved_last_frame_x + var939 + 2) >= var933) {
        v4 = var933;
      } else {
        var939 += absolute_moved_last_frame_x + 2;
        v3 = (uint8)(HIBYTE(var939) + 1);
        if (scrolls[(uint16)(Mult8x8((uint16)(layer1_y_pos + 128) >> 8, room_width_in_scrolls) + v3)])
          v4 = var939;
        else
          v4 = var939 & 0xFF00;
      }
    }
    layer1_x_pos = v4;
  }
}

void HandleScrollingWhenTriggeringScrollRight(void) {  // 0x80A641
  int16 v1;

  uint16 var939 = layer1_x_pos;
  if ((int16)(ideal_layer1_xpos - layer1_x_pos) < 0) {
    layer1_x_pos = ideal_layer1_xpos;
    layer1_x_subpos = 0;
  }
  uint16 v0 = swap16(room_width_in_scrolls - 1);
  if (v0 >= layer1_x_pos) {
    v1 = HIBYTE(layer1_x_pos);
    uint16 RegWord = Mult8x8((uint16)(layer1_y_pos + 128) >> 8, room_width_in_scrolls);
    if (!scrolls[(uint16)(RegWord + 1 + v1)]) {
      uint16 var933 = layer1_x_pos & 0xFF00;
      uint16 v4 = var939 - absolute_moved_last_frame_x - 2;
      if ((int16)(v4 - (layer1_x_pos & 0xFF00)) < 0)
        v4 = var933;
      layer1_x_pos = v4;
    }
  } else {
    layer1_x_pos = v0;
  }
}

void HandleScrollingWhenTriggeringScrollLeft(void) {  // 0x80A6BB
  int16 v0;

  uint16 var939 = layer1_x_pos;
  if ((int16)(layer1_x_pos - ideal_layer1_xpos) < 0) {
    layer1_x_pos = ideal_layer1_xpos;
    layer1_x_subpos = 0;
  }
  if ((layer1_x_pos & 0x8000) == 0) {
    v0 = HIBYTE(layer1_x_pos);
    uint16 prod = Mult8x8((uint16)(layer1_y_pos + 128) >> 8, room_width_in_scrolls);
    if (!scrolls[(uint16)(prod + v0)]) {
      uint16 var933 = (layer1_x_pos & 0xFF00) + 256;
      uint16 v1 = absolute_moved_last_frame_x + var939 + 2;
      if (v1 >= var933)
        v1 = var933;
      layer1_x_pos = v1;
    }
  } else {
    layer1_x_pos = 0;
  }
}

void HandleAutoscrolling_Y(void) {  // 0x80A731
  int16 v1;
  int16 v2;
  int16 v4;
  int16 v7;
  int16 v10;
  uint16 v8;

  if (!time_is_frozen_flag) {
    uint16 v0 = 0;
    v1 = (uint16)(layer1_x_pos + 128) >> 8;
    uint16 r20 = Mult8x8(HIBYTE(layer1_y_pos), room_width_in_scrolls) + v1;
    if (scrolls[r20] != 1)
      v0 = 31;
    uint16 var933 = v0;
    uint16 var939 = layer1_y_pos;
    if ((layer1_y_pos & 0x8000) != 0)
      layer1_y_pos = 0;
    LOBYTE(v2) = (uint16)(room_height_in_scrolls - 1) >> 8;
    HIBYTE(v2) = room_height_in_scrolls - 1;
    uint16 v3 = var933 + v2;
    if (v3 < layer1_y_pos)
      layer1_y_pos = v3;
    v4 = (uint16)(layer1_x_pos + 128) >> 8;
    uint16 v5 = Mult8x8(HIBYTE(layer1_y_pos), room_width_in_scrolls) + v4;
    if (!scrolls[v5]) {
      uint16 var935 = (layer1_y_pos & 0xFF00) + 256;
      uint16 v6 = absolute_moved_last_frame_y + var939 + 2;
      if (v6 >= var935) {
        v8 = var935;
      } else {
        var939 += absolute_moved_last_frame_y + 2;
        v7 = (uint16)(layer1_x_pos + 128) >> 8;
        uint16 prod = Mult8x8(HIBYTE(v6) + 1, room_width_in_scrolls);
        if (scrolls[(uint16)(prod + v7)])
          v8 = var939;
        else
          v8 = var939 & 0xFF00;
      }
      layer1_y_pos = v8;
      return;
    }
    if (!scrolls[(uint16)(room_width_in_scrolls + v5)]) {
      uint16 var937 = var933 + (layer1_y_pos & 0xFF00);
      if (var937 < layer1_y_pos) {
        uint16 v9 = var939 - absolute_moved_last_frame_y - 2;
        if ((int16)(v9 - var937) < 0) {
          v8 = var937;
        } else {
          var939 = v9;
          uint16 prod = Mult8x8(HIBYTE(v9), room_width_in_scrolls);
          v10 = (uint16)(layer1_x_pos + 128) >> 8;
          if (scrolls[(uint16)(prod + v10)])
            v8 = var939;
          else
            v8 = (var939 & 0xFF00) + 256;
        }
        layer1_y_pos = v8;
        return;
      }
    }
  }
}

void HandleScrollingWhenTriggeringScrollDown(void) {  // 0x80A893
  uint16 var939 = layer1_y_pos;
  int v0 = 0;
  uint16 prod = Mult8x8(HIBYTE(layer1_y_pos), room_width_in_scrolls);
  uint16 v1 = (uint16)(layer1_x_pos + 128) >> 8;
  uint16 r20 = prod + v1;
  if (scrolls[r20] != 1)
    v0 = 31;
  uint16 var933 = v0;
  if ((int16)(ideal_layer1_ypos - layer1_y_pos) < 0) {
    layer1_y_pos = ideal_layer1_ypos;
    layer1_y_subpos = 0;
  }
  uint16 v2 = swap16(room_height_in_scrolls - 1);
  uint16 var937 = var933 + v2;
  if ((uint16)(var933 + v2) < layer1_y_pos
      || !scrolls[(uint16)(room_width_in_scrolls + r20)]
      && (var937 = var933 + (layer1_y_pos & 0xFF00), var937 < layer1_y_pos)) {
    uint16 v3 = var939 - absolute_moved_last_frame_y - 2;
    if ((int16)(v3 - var937) < 0)
      v3 = var937;
    layer1_y_pos = v3;
  }
}

void HandleScrollingWhenTriggeringScrollUp(void) {  // 0x80A936
  int16 v0;

  uint16 var939 = layer1_y_pos;
  if ((int16)(layer1_y_pos - ideal_layer1_ypos) < 0) {
    layer1_y_pos = ideal_layer1_ypos;
    layer1_y_subpos = 0;
  }
  if ((layer1_y_pos & 0x8000) == 0) {
    uint16 prod = Mult8x8(HIBYTE(layer1_y_pos), room_width_in_scrolls);
    v0 = (uint16)(layer1_x_pos + 128) >> 8;
    if (!scrolls[(uint16)(prod + v0)]) {
      uint16 var933 = (layer1_y_pos & 0xFF00) + 256;
      uint16 v1 = absolute_moved_last_frame_y + var939 + 2;
      if (v1 >= var933)
        v1 = var933;
      layer1_y_pos = v1;
    }
  } else {
    layer1_y_pos = 0;
  }
}

static void UploadBackgroundDataColumn(void) {  // 0x80A9D6
  UpdateLevelOrBackgroundDataColumn(0x1c);
}

static void UploadLevelDataColumn(void) {  // 0x80A9DB
  UpdateLevelOrBackgroundDataColumn(0);
}

static void UpdateLevelOrBackgroundDataColumn(uint16 k) {  // 0x80A9DE
  if (irq_enable_mode7)
    return;

  uint16 prod = Mult8x8(blocks_to_update_y_block, room_width_in_blocks);
  uint16 v1 = blocks_to_update_x_block;
  uint16 v2 = 2 * (prod + v1) + 2;
  if (k)
    v2 += 0x9600;
  const uint16 *r54 = (const uint16 *)((uint8*)&ram7F_start + v2);
  uint16 v3 = (4 * vram_blocks_to_update_y_block) & 0x3C;
  *(uint16 *)((uint8 *)&bg1_update_col_wrapped_size + k) = v3;
  *(uint16 *)((uint8 *)&bg1_update_col_unwrapped_size + k) = (v3 ^ 0x3F) + 1;
  prod = Mult8x8(vram_blocks_to_update_y_block & 0xF, 0x40);
  uint16 var935 = vram_blocks_to_update_x_block & 0x1F;
  uint16 v4 = 2 * var935;
  uint16 var933 = prod + v4;
  uint16 v5 = addr_unk_605000;
  if (var935 >= 0x10)
    v5 = addr_unk_6053E0;
  if (k)
    v5 -= size_of_bg2;
  uint16 var937 = v5;
  *(uint16 *)((uint8 *)&bg1_update_col_unwrapped_dst + k) = var933 + v5;
  *(uint16 *)((uint8 *)&bg1_update_col_wrapped_dst + k) = var935 + var935 + var937;
  uint16 v6 = ADDR16_OF_RAM(*bg1_column_update_tilemap_left_halves);
  uint16 v7 = 0;
  if (k) {
    v6 = ADDR16_OF_RAM(*bg2_column_update_tilemap_left_halves);
    v7 = 264;
  }
  uint16 v8 = *(uint16 *)((uint8 *)&bg1_update_col_unwrapped_size + k) + v6;
  *(uint16 *)((uint8 *)&bg1_update_col_wrapped_left_src + k) = v8;
  *(uint16 *)((uint8 *)&bg1_update_col_wrapped_right_src + k) = v8 + 64;
  var937 = v7;
  uint16 t2 = k;
  uint16 v9 = 0;
  uint16 var939 = 16;
  do {
    uint16 var93B = r54[v9 >> 1];
    uint16 v10 = var93B & 0x3FF;
    uint16 v17 = v9;
    uint16 v11 = var937;
    uint16 v12 = var93B & 0xC00;
    if ((var93B & 0xC00) != 0) {
      if (v12 == 1024) {
        uint16 v14 = var937 >> 1;
        bg1_column_update_tilemap_left_halves[v14] = tile_table.tables[v10].top_right ^ 0x4000;
        bg1_column_update_tilemap_right_halves[v14] = tile_table.tables[v10].top_left ^ 0x4000;
        bg1_column_update_tilemap_left_halves[v14 + 1] = tile_table.tables[v10].bottom_right ^ 0x4000;
        bg1_column_update_tilemap_right_halves[v14 + 1] = tile_table.tables[v10].bottom_left ^ 0x4000;
      } else {
        uint16 v15 = var937 >> 1;
        uint16 v16;
        if (v12 == 2048) {
          bg1_column_update_tilemap_left_halves[v15] = tile_table.tables[v10].bottom_left ^ 0x8000;
          bg1_column_update_tilemap_right_halves[v15] = tile_table.tables[v10].bottom_right ^ 0x8000;
          bg1_column_update_tilemap_left_halves[v15 + 1] = tile_table.tables[v10].top_left ^ 0x8000;
          v16 = tile_table.tables[v10].top_right ^ 0x8000;
        } else {
          bg1_column_update_tilemap_left_halves[v15] = tile_table.tables[v10].bottom_right ^ 0xC000;
          bg1_column_update_tilemap_right_halves[v15] = tile_table.tables[v10].bottom_left ^ 0xC000;
          bg1_column_update_tilemap_left_halves[v15 + 1] = tile_table.tables[v10].top_right ^ 0xC000;
          v16 = tile_table.tables[v10].top_left ^ 0xC000;
        }
        bg1_column_update_tilemap_right_halves[v15 + 1] = v16;
      }
    } else {
      uint16 v13 = var937 >> 1;
      bg1_column_update_tilemap_left_halves[v13] = tile_table.tables[v10].top_left;
      bg1_column_update_tilemap_right_halves[v13] = tile_table.tables[v10].top_right;
      bg1_column_update_tilemap_left_halves[v13 + 1] = tile_table.tables[v10].bottom_left;
      bg1_column_update_tilemap_right_halves[v13 + 1] = tile_table.tables[v10].bottom_right;
    }
    var937 = v11 + 4;
    v9 = room_width_in_blocks * 2 + v17;
    --var939;
  } while (var939);
  ++ *(uint16 *)((uint8 *)&bg1_update_col_enable + t2);
}

static void UpdateBackgroundDataRow(void) {  // 0x80AB70
  UpdateLevelOrBackgroundDataRow(0x1c);
}

static void UpdateLevelDataRow(void) {  // 0x80AB75
  UpdateLevelOrBackgroundDataRow(0);
}

static void UpdateLevelOrBackgroundDataRow(uint16 v0) {  // 0x80AB78
  if (irq_enable_mode7)
    return;
  uint16 prod = Mult8x8(blocks_to_update_y_block, room_width_in_blocks);
  uint16 v1 = blocks_to_update_x_block;
  uint16 v2 = 2 * (prod + v1) + 2;
  if (v0)
    v2 -= 27136;
  const uint16 *r54 = (const uint16 *)((uint8*)&ram7F_start + v2);
  uint16 var933 = vram_blocks_to_update_x_block & 0xF;
  *(uint16 *)((uint8 *)&bg1_update_row_unwrapped_size + v0) = 4 * (16 - var933);
  *(uint16 *)((uint8 *)&bg1_update_row_wrapped_size + v0) = 4 * (var933 + 1);
  prod = Mult8x8(vram_blocks_to_update_y_block & 0xF, 0x40);
  uint16 var935 = vram_blocks_to_update_x_block & 0x1F;
  uint16 v3 = 2 * var935;
  var933 = prod + v3;
  uint16 var937 = addr_unk_605400;
  uint16 v4 = addr_unk_605000;
  if (var935 >= 0x10) {
    var937 = addr_unk_605000;
    v4 = addr_unk_6053E0;
  }
  if (v0)
    v4 -= size_of_bg2;
  *(uint16 *)((uint8 *)&bg1_update_row_unwrapped_dst + v0) = var933 + v4;
  uint16 v5 = var937;
  if (v0)
    v5 = var937 - size_of_bg2;
  *(uint16 *)((uint8 *)&bg1_update_row_wrapped_dst + v0) = prod + v5;
  uint16 v6 = ADDR16_OF_RAM(*bg1_column_update_tilemap_top_halves);
  uint16 v7 = 0;
  if (v0) {
    v6 = ADDR16_OF_RAM(*bg2_column_update_tilemap_top_halves);
    v7 = 264;
  }
  uint16 v8 = *(uint16 *)((uint8 *)&bg1_update_row_unwrapped_size + v0) + v6;
  *(uint16 *)((uint8 *)&bg1_update_row_wrapped_top_src + v0) = v8;
  *(uint16 *)((uint8 *)&bg1_update_row_wrapped_bottom_src + v0) = v8 + 68;
  var937 = v7;
  uint16 t2 = v0;
  uint16 v9 = 0;
  uint16 var939 = 17;
  do {
    uint16 var93B = r54[v9 >> 1];
    uint16 v10 = var93B & 0x3FF;
    uint16 v17 = v9;
    uint16 v11 = var937;
    uint16 v12 = var93B & 0xC00;
    if ((var93B & 0xC00) != 0) {
      if (v12 == 1024) {
        uint16 v14 = var937 >> 1;
        bg1_column_update_tilemap_top_halves[v14] = tile_table.tables[v10].top_right ^ 0x4000;
        bg1_column_update_tilemap_top_halves[v14 + 1] = tile_table.tables[v10].top_left ^ 0x4000;
        bg1_column_update_tilemap_bottom_halves[v14] = tile_table.tables[v10].bottom_right ^ 0x4000;
        bg1_column_update_tilemap_bottom_halves[v14 + 1] = tile_table.tables[v10].bottom_left ^ 0x4000;
      } else {
        uint16 v15 = var937 >> 1;
        uint16 v16;
        if (v12 == 2048) {
          bg1_column_update_tilemap_top_halves[v15] = tile_table.tables[v10].bottom_left ^ 0x8000;
          bg1_column_update_tilemap_top_halves[v15 + 1] = tile_table.tables[v10].bottom_right ^ 0x8000;
          bg1_column_update_tilemap_bottom_halves[v15] = tile_table.tables[v10].top_left ^ 0x8000;
          v16 = tile_table.tables[v10].top_right ^ 0x8000;
        } else {
          bg1_column_update_tilemap_top_halves[v15] = tile_table.tables[v10].bottom_right ^ 0xC000;
          bg1_column_update_tilemap_top_halves[v15 + 1] = tile_table.tables[v10].bottom_left ^ 0xC000;
          bg1_column_update_tilemap_bottom_halves[v15] = tile_table.tables[v10].top_right ^ 0xC000;
          v16 = tile_table.tables[v10].top_left ^ 0xC000;
        }
        bg1_column_update_tilemap_bottom_halves[v15 + 1] = v16;
      }
    } else {
      uint16 v13 = var937 >> 1;
      bg1_column_update_tilemap_top_halves[v13] = tile_table.tables[v10].top_left;
      bg1_column_update_tilemap_top_halves[v13 + 1] = tile_table.tables[v10].top_right;
      bg1_column_update_tilemap_bottom_halves[v13] = tile_table.tables[v10].bottom_left;
      bg1_column_update_tilemap_bottom_halves[v13 + 1] = tile_table.tables[v10].bottom_right;
    }
    var937 = v11 + 4;
    v9 = v17 + 2;
    --var939;
  } while (var939);
  ++ *(uint16 *)((uint8 *)&bg1_update_row_enable + t2);
}

void FixDoorsMovingUp(void) {  // 0x80AD1D
  door_transition_frame_counter = 0;
  CalculateBgScrollAndLayerPositionBlocks();
  UpdatePreviousLayerBlocks();
  ++previous_layer1_y_block;
  ++previous_layer2_y_block;
  DoorTransition_Up();
}

static Func_V *const kDoorTransitionSetupFuncs[4] = {  // 0x80AD30
  DoorTransitionScrollingSetup_Right,
  DoorTransitionScrollingSetup_Left,
  DoorTransitionScrollingSetup_Down,
  DoorTransitionScrollingSetup_Up,
};

void DoorTransitionScrollingSetup(void) {
  layer1_x_pos = door_destination_x_pos;
  layer1_y_pos = door_destination_y_pos;
  kDoorTransitionSetupFuncs[door_direction & 3]();
}

static void DoorTransitionScrollingSetup_Right(void) {  // 0x80AD4A
  CalculateLayer2Xpos();
  layer2_x_pos -= 256;
  CalculateLayer2Ypos();
  layer1_x_pos -= 256;
  UpdateBgScrollOffsets();
  CalculateBgScrollAndLayerPositionBlocks();
  UpdatePreviousLayerBlocks();
  --previous_layer1_x_block;
  --previous_layer2_x_block;
  DoorTransition_Right();
}

static void DoorTransitionScrollingSetup_Left(void) {  // 0x80AD74
  CalculateLayer2Xpos();
  layer2_x_pos += 256;
  CalculateLayer2Ypos();
  layer1_x_pos += 256;
  UpdateBgScrollOffsets();
  CalculateBgScrollAndLayerPositionBlocks();
  UpdatePreviousLayerBlocks();
  ++previous_layer1_x_block;
  ++previous_layer2_x_block;
  DoorTransition_Left();
}

static void DoorTransitionScrollingSetup_Down(void) {  // 0x80AD9E
  CalculateLayer2Xpos();
  CalculateLayer2Ypos();
  layer2_y_pos -= 224;
  layer1_y_pos -= 224;
  UpdateBgScrollOffsets();
  CalculateBgScrollAndLayerPositionBlocks();
  UpdatePreviousLayerBlocks();
  --previous_layer1_y_block;
  --previous_layer2_y_block;
  DoorTransition_Down();
}

static void DoorTransitionScrollingSetup_Up(void) {  // 0x80ADC8
  CalculateLayer2Xpos();
  uint16 v1 = layer1_y_pos;
  layer1_y_pos += 31;
  CalculateLayer2Ypos();
  layer2_y_pos += 224;
  layer1_y_pos = v1 + 256;
  UpdateBgScrollOffsets();
  door_destination_y_pos += 32;
  CalculateBgScrollAndLayerPositionBlocks();
  UpdatePreviousLayerBlocks();
  ++previous_layer1_y_block;
  ++previous_layer2_y_block;
  --layer1_y_pos;
  DoorTransition_Up();
}

static void UpdatePreviousLayerBlocks(void) {  // 0x80AE10
  previous_layer1_x_block = layer1_x_block;
  previous_layer2_x_block = layer2_x_block;
  previous_layer1_y_block = layer1_y_block;
  previous_layer2_y_block = layer2_y_block;
}

static void UpdateBgScrollOffsets(void) {  // 0x80AE29
  bg1_x_offset = reg_BG1HOFS - layer1_x_pos;
  bg1_y_offset = reg_BG1VOFS - layer1_y_pos;
  bg2_x_scroll = reg_BG2HOFS - layer1_x_pos;
  bg2_y_scroll = reg_BG2VOFS - layer1_y_pos;
}

static Func_U8 *const kDoorTransitionFuncs[4] = {
  DoorTransition_Right,
  DoorTransition_Left,
  DoorTransition_Down,
  DoorTransition_Up,
};

void Irq_FollowDoorTransition(void) {
  if (kDoorTransitionFuncs[door_direction & 3]()) {
    layer1_x_pos = door_destination_x_pos;
    layer1_y_pos = door_destination_y_pos;
    door_transition_flag |= 0x8000;
  }
}

static uint8 DoorTransition_Right(void) {  // 0x80AE7E
  uint16 v2 = door_transition_frame_counter;
  AddToHiLo(&samus_x_pos, &samus_x_subpos, __PAIR32__(samus_door_transition_speed, samus_door_transition_subspeed));
  samus_prev_x_pos = samus_x_pos;
  layer1_x_pos += 4;
  layer2_x_pos += 4;
  UpdateScrollVarsUpdateMap();
  door_transition_frame_counter = v2 + 1;
  if (v2 != 63)
    return 0;
  UpdateScrollVarsUpdateMap();
  return 1;
}

static uint8 DoorTransition_Left(void) {  // 0x80AEC2
  uint16 v2 = door_transition_frame_counter;
  AddToHiLo(&samus_x_pos, &samus_x_subpos, -IPAIR32(samus_door_transition_speed, samus_door_transition_subspeed));
  samus_prev_x_pos = samus_x_pos;
  layer1_x_pos -= 4;
  layer2_x_pos -= 4;
  UpdateScrollVarsUpdateMap();
  door_transition_frame_counter = v2 + 1;
  return v2 == 63;
}

static uint8 DoorTransition_Down(void) {  // 0x80AF02
  uint16 v6 = door_transition_frame_counter;
  if (door_transition_frame_counter) {
    if (door_transition_frame_counter < 0x39) {
      AddToHiLo(&samus_y_pos, &samus_y_subpos, __PAIR32__(samus_door_transition_speed, samus_door_transition_subspeed));
      samus_prev_y_pos = samus_y_pos;
      layer1_y_pos += 4;
      layer2_y_pos += 4;
      UpdateScrollVarsUpdateMap();
    }
  } else {
    uint16 v5 = reg_BG1VOFS;
    uint16 v4 = reg_BG2VOFS;
    uint16 v3 = layer1_y_pos;
    layer1_y_pos -= 15;
    uint16 v2 = layer2_y_pos;
    layer2_y_pos -= 15;
    CalculateBgScrollAndLayerPositionBlocks();
    UpdatePreviousLayerBlocks();
    --previous_layer1_y_block;
    --previous_layer2_y_block;
    UpdateScrollVarsUpdateMap();
    layer2_y_pos = v2;
    layer1_y_pos = v3;
    reg_BG2VOFS = v4;
    reg_BG1VOFS = v5;
  }
  door_transition_frame_counter = v6 + 1;
  if ((uint16)(v6 + 1) < 0x39)
    return 0;
  UpdateScrollVarsUpdateMap();
  return 1;
}

static uint8 DoorTransition_Up(void) {  // 0x80AF89
  uint16 v6 = door_transition_frame_counter;
  if (door_transition_frame_counter) {
    AddToHiLo(&samus_y_pos, &samus_y_subpos, -IPAIR32(samus_door_transition_speed, samus_door_transition_subspeed));
    samus_prev_y_pos = samus_y_pos;
    layer1_y_pos -= 4;
    layer2_y_pos -= 4;
    if (door_transition_frame_counter >= 5) {
      UpdateScrollVarsUpdateMap();
    } else {
      reg_BG1HOFS = bg1_x_offset + layer1_x_pos;
      reg_BG1VOFS = bg1_y_offset + layer1_y_pos;
      reg_BG2HOFS = bg2_x_scroll + layer2_x_pos;
      reg_BG2VOFS = bg2_y_scroll + layer2_y_pos;
    }
  } else {
    uint16 v5 = reg_BG1VOFS;
    uint16 v4 = reg_BG2VOFS;
    uint16 v3 = layer1_y_pos;
    layer1_y_pos -= 16;
    uint16 v2 = layer2_y_pos;
    layer2_y_pos -= 16;
    CalculateBgScrollAndLayerPositionBlocks();
    UpdatePreviousLayerBlocks();
    ++previous_layer1_y_block;
    ++previous_layer2_y_block;
    UpdateScrollVarsUpdateMap();
    layer2_y_pos = v2;
    layer1_y_pos = v3;
    reg_BG2VOFS = v4;
    reg_BG1VOFS = v5;
  }
  door_transition_frame_counter = v6 + 1;
  return v6 == 56;
}
