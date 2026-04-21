// Shared enemy tileset / VRAM staging helpers extracted from enemy_main.c.

#include <string.h>

#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "enemy_types.h"

#define kStandardSpriteTiles ((uint16*)RomFixedPtr(0x9ad200))

void LoadEnemyGfxIndexes(uint16 k, uint16 j) {  // 0xA08BF3
  EnemyData *E = gEnemyData(j);
  EnemySpawnData *ES = gEnemySpawnData(j);
  uint16 e = room_enemy_tilesets_ptr;
  uint16 f = 0;
  while (1) {
    EnemyTileset *ET = get_EnemyTileset(e);
    if (get_EnemyPopulation(0xa1, k)->enemy_ptr == ET->enemy_def) {
      E->palette_index = ES->palette_index = (ET->vram_dst & 0xF) << 9;
      E->vram_tiles_index = ES->vram_tiles_index = f;
      return;
    }
    if (ET->enemy_def == 0xFFFF) {
      E->vram_tiles_index = ES->vram_tiles_index = 0;
      E->palette_index = ES->palette_index = 2560;
      return;
    }
    f += get_EnemyDef_A2(ET->enemy_def)->tile_data_size >> 5;
    e += 4;
  }
}

void LoadEnemyTileData(void) {  // 0xA08C6C
  for (int i = 510; i >= 0; i -= 2)
    gEnemySpawnData(i)->some_flag = kStandardSpriteTiles[(i >> 1) + 3072];
  if (enemy_tile_load_data_write_pos) {
    uint16 v1 = 0;
    EnemyTileLoadData *load_data = enemy_tile_load_data;
    do {
      uint16 v2 = load_data->tile_data_ptr.addr;
      uint16 r18 = load_data->tile_data_size + v2;
      uint16 v3 = load_data->offset_into_ram;
      uint8 db = load_data->tile_data_ptr.bank;
      do {
        memcpy((uint8 *)enemy_spawn_data + v3, RomPtrWithBank(db, v2), 8);
        v3 += 8;
        v2 += 8;
      } while (v2 != r18);
      load_data++;
      v1 += 7;
    } while (v1 != enemy_tile_load_data_write_pos);
    enemy_tile_load_data_write_pos = 0;
  }
}

void TransferEnemyTilesToVramAndInit(void) {  // 0xA08CD7
  uint16 v0 = enemy_tile_vram_src;
  if (!enemy_tile_vram_src) {
    v0 = ADDR16_OF_RAM(*enemy_spawn_data);
    enemy_tile_vram_src = ADDR16_OF_RAM(*enemy_spawn_data);
    enemy_tile_vram_dst = 0x6c00;
  }
  if (v0 != 0xFFFF) {
    if (v0 == 0xFFFE) {
      InitializeEnemies();
      enemy_tile_vram_src = -1;
    } else if (v0 == 0x9800) {
      enemy_tile_vram_src = -2;
    } else {
      uint16 v1 = vram_write_queue_tail;
      VramWriteEntry *v2 = gVramWriteEntry(vram_write_queue_tail);
      v2->size = 2048;
      uint16 v3 = enemy_tile_vram_src;
      v2->src.addr = enemy_tile_vram_src;
      enemy_tile_vram_src = v3 + 2048;
      *(uint16 *)&v2->src.bank = 126;
      uint16 v4 = enemy_tile_vram_dst;
      v2->vram_dst = enemy_tile_vram_dst;
      enemy_tile_vram_dst = v4 + 1024;
      vram_write_queue_tail = v1 + 7;
    }
  }
}

void ProcessEnemyTilesets(void) {  // 0xA08D64
  enemy_tile_load_data_write_pos = 0;
  uint16 r30 = 2048;
  enemy_def_ptr[0] = 0;
  enemy_def_ptr[1] = 0;
  enemy_def_ptr[2] = 0;
  enemy_def_ptr[3] = 0;
  enemy_gfxdata_tiles_index[0] = 0;
  enemy_gfxdata_tiles_index[1] = 0;
  enemy_gfxdata_tiles_index[2] = 0;
  enemy_gfxdata_tiles_index[3] = 0;
  enemy_gfxdata_vram_ptr[0] = 0;
  enemy_gfxdata_vram_ptr[1] = 0;
  enemy_gfxdata_vram_ptr[2] = 0;
  enemy_gfxdata_vram_ptr[3] = 0;
  uint16 enemy_gfx_data_write_ptr = 0;
  uint16 next_enemy_tiles_index = 0;
  EnemyTileset *ET = get_EnemyTileset(room_enemy_tilesets_ptr);
  EnemyTileLoadData *LD = enemy_tile_load_data;
  for (; ET->enemy_def != 0xffff; ET++) {
    EnemyDef *ED = get_EnemyDef_A2(ET->enemy_def);
    memcpy(&target_palettes[(LOBYTE(ET->vram_dst) + 8) * 16],
           RomPtrWithBank(ED->bank, ED->palette_ptr), 32);
    LD->tile_data_size = ED->tile_data_size & 0x7FFF;
    LD->tile_data_ptr = ED->tile_data;
    uint16 v10 = r30;
    if ((ED->tile_data_size & 0x8000) != 0)
      v10 = (uint16)(ET->vram_dst & 0x3000) >> 3;
    LD->offset_into_ram = v10;
    enemy_tile_load_data_write_pos += 7;
    LD++;
    uint16 v11 = enemy_gfx_data_write_ptr;
    enemy_gfxdata_tiles_index[enemy_gfx_data_write_ptr >> 1] = next_enemy_tiles_index;
    enemy_def_ptr[v11 >> 1] = ET->enemy_def;
    enemy_gfxdata_vram_ptr[v11 >> 1] = ET->vram_dst;
    enemy_gfx_data_write_ptr += 2;
    next_enemy_tiles_index += ED->tile_data_size >> 5;
    r30 += ED->tile_data_size;
  }
}
