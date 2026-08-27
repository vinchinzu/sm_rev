// Enemy AI - Zebes escape typewriter — peeled from Bank $A6
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"

void SetupZebesEscapeTypewriter(void) {  // 0xA6C23F
  palette_buffer[157] = palette_buffer[125];
  palette_buffer[158] = palette_buffer[126];
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->mbn_var_3B = addr_loc_A9C49C;
  E->mbn_var_3C = 0;
  E->mbn_var_3D = 0;
  E->mbn_var_3E = 0;
  E->mbn_var_3F = 0;
  reg_BG2HOFS = 0;
  reg_BG2VOFS = 0;
}

uint8 ProcessEscapeTimerTileTransfers(void) {  // 0xA6C26E
  VramWriteEntry *v4;

  EnemyData *v0 = gEnemyData(0);
  uint16 ai_var_E = v0->ai_var_E;
  uint16 v2 = vram_write_queue_tail;
  const uint8 *v3 = RomPtr_A6(ai_var_E);
  uint8 result = 1;
  if (GET_WORD(v3)) {
    v4 = gVramWriteEntry(vram_write_queue_tail);
    v4->size = GET_WORD(v3);
    *(VoidP *)((uint8 *)&v4->src.addr + 1) = GET_WORD(v3 + 3);
    v4->src.addr = GET_WORD(v3 + 2);
    v4->vram_dst = GET_WORD(v3 + 5);
    vram_write_queue_tail = v2 + 7;
    v0->ai_var_E = ai_var_E + 7;
    if (*(uint16 *)RomPtr_A6(v0->ai_var_E))
      return 0;
  }
  return result;
}

uint8 HandleTypewriterText_Ext(uint16 a) {  // 0xA6C2A7
  uint16 r18 = a;
  int16 v4;
  VramWriteEntry *v12;

  Enemy_MotherBrain *E = Get_MotherBrain(0);
  uint16 mbn_var_3D = E->mbn_var_3D;
  if (mbn_var_3D) {
    E->mbn_var_3D = mbn_var_3D - 1;
    return 0;
  } else {
    E->mbn_var_3D = E->mbn_var_3E;
    int i, v7;
    for (i = E->mbn_var_3B; ; i = v7 + 2) {
      while (1) {
        v4 = *(uint16 *)RomPtr_A6(i);
        if (!v4)
          return 1;
        if (v4 != 1)
          break;
        uint16 v5 = i + 2;
        uint16 v6 = *(uint16 *)RomPtr_A6(v5);
        E->mbn_var_3E = v6;
        i = v5 + 2;
      }
      if (v4 != 13)
        break;
      v7 = i + 2;
      uint16 v8 = *(uint16 *)RomPtr_A6(v7);
      E->mbn_var_3C = v8;
    }
    v4 = (uint8)v4;
    if ((uint8)v4 == 32) {
      ++E->mbn_var_3C;
      E->mbn_var_3B = i + 1;
      return 0;
    } else {
      if ((uint8)v4 == 33)
        v4 = 91;
      E->mbn_var_3B = i + 1;
      uint16 v11 = vram_write_queue_tail;
      v12 = gVramWriteEntry(vram_write_queue_tail);
      v12->size = 2;
      *(VoidP *)((uint8 *)&v12->src.addr + 1) = 32256;
      E->mbn_var_3A = r18 + v4 - 65;
      v12->src.addr = ADDR16_OF_RAM(*extra_enemy_ram8000) + 52;
      uint16 mbn_var_3C = E->mbn_var_3C;
      v12->vram_dst = mbn_var_3C;
      E->mbn_var_3C = mbn_var_3C + 1;
      vram_write_queue_tail = v11 + 7;
      uint16 v14 = E->mbn_var_3F + 1;
      E->mbn_var_3F = v14;
      if (!sign16(v14 - 2)) {
        E->mbn_var_3F = 0;
        if (area_index == 6)
          QueueSfx2_Max3(0x45);
        else
          QueueSfx3_Max3(0xD);
      }
      return 0;
    }
  }
}
