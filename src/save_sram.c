// Save-slot SRAM and map packing helpers extracted from Bank 81.

#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"

#define kOffsetToSaveSlot ((uint16*)RomFixedPtr(0x81812b))
#define kPackedBytesPerArea_Count ((uint8*)RomFixedPtr(0x818131))
#define kPackedBytesPerArea_PackedOffs ((uint16*)RomFixedPtr(0x818138))
#define kPackedBytesPerArea_UnpackedOffs ((uint16*)RomFixedPtr(0x8182d6))

static void UnpackMapFromSave(void) {  // 0x8182E4
  for (int i = 1792; i >= 0; i -= 2)
    explored_map_tiles_saved[i >> 1] = 0;
  for (int i = 0; i < 6; i++) {
    int n = kPackedBytesPerArea_Count[i];
    const uint8 *r0 = RomPtr_81(kPackedBytesPerArea_UnpackedOffs[i]);
    int v3 = kPackedBytesPerArea_PackedOffs[i];
    uint8 *unpacked = (uint8 *)explored_map_tiles_saved;
    do {
      uint16 v4 = i * 256 + *r0++;
      unpacked[v4] = compressed_map_data[v3++];
    } while (--n);
  }
}

static void PackMapToSave(void) {  // 0x81834B
  for (int i = 0; i < 6; i++) {
    int n = kPackedBytesPerArea_Count[i];
    const uint8 *r0 = RomPtr_81(kPackedBytesPerArea_UnpackedOffs[i]);
    int v1 = kPackedBytesPerArea_PackedOffs[i];
    uint8 *unpacked = (uint8 *)explored_map_tiles_saved;
    do {
      int v3 = i * 256 + *r0++;
      compressed_map_data[v1++] = unpacked[v3];
    } while (--n);
  }
}

void SoftReset(void) {
  game_state = 0xffff;
}

void SaveToSram(uint16 a) {  // 0x818000
  uint16 v7;
  uint16 v11;

  uint16 r20 = 0;
  uint16 r18 = 2 * (a & 3);
  for (int i = 94; i >= 0; i -= 2)
    player_data_saved[i >> 1] = *(uint16 *)((uint8 *)&equipped_items + i);
  uint16 v3 = area_index * 256;
  uint16 v4 = 0;
  do {
    explored_map_tiles_saved[v3 >> 1] = *(uint16 *)&map_tiles_explored[v4];
    v4 += 2;
    v3 += 2;
  } while ((int16)(v4 - 256) < 0);
  PackMapToSave();
  sram_save_station_index = load_station_index;
  sram_area_index = area_index;
  uint16 v5 = kOffsetToSaveSlot[r18 >> 1];
  uint16 *v6 = player_data_saved;
  do {
    v7 = *v6++;
    *(uint16 *)(&g_sram[2 * (v5 >> 1)]) = v7;
    r20 += v7;
    v5 += 2;
  } while (v6 != plm_instruction_timer);
  uint16 v8 = r18;
  uint16 v9 = r20;
  int v10 = r18 >> 1;
  *(uint16 *)(&g_sram[2 * v10]) = r20;
  *(uint16 *)(&g_sram[2 * v10 + 0x1FF0]) = v9;
  v11 = ~v9;
  *(uint16 *)(&g_sram[2 * v10 + 8]) = v11;
  *(uint16 *)(&g_sram[2 * v10 + 0x1FF8]) = v11;

  RtlWriteSram();
}

uint8 LoadFromSram(uint16 a) {  // 0x818085
  uint16 r20 = 0;
  uint16 r18 = 2 * (a & 3);
  uint16 v1 = kOffsetToSaveSlot[a & 3];
  uint16 *v2 = player_data_saved;
  do {
    uint16 v3 = kSramChecksum[v1 >> 1];
    *v2++ = v3;
    r20 += v3;
    v1 += 2;
  } while (v2 != plm_instruction_timer);
  int v4 = r18 >> 1;
  if (r20 == kSramChecksum[v4] && (r20 ^ 0xffff) == kSramChecksumInverted[v4]
      || r20 == kSramChecksumUpper[v4] && (r20 ^ 0xffff) == kSramChecksumInvertedUpper[v4]) {
    for (int i = 94; i >= 0; i -= 2)
      *(uint16 *)((uint8 *)&equipped_items + i) = player_data_saved[i >> 1];
    UnpackMapFromSave();
    load_station_index = sram_save_station_index;
    area_index = sram_area_index;
    return 0;
  } else {
    r20 = 0;
    uint16 v7 = kOffsetToSaveSlot[r18 >> 1];
    uint16 v8 = ADDR16_OF_RAM(*player_data_saved);
    uint16 v9 = 0;
    do {
      kSramChecksum[v7 >> 1] = v9;
      v9 += r20;
      r20 = v9;
      v7 += 2;
      v8 += 2;
    } while (v8 != ADDR16_OF_RAM(*plm_instruction_timer)); // 0xDE1C
    load_station_index = 0;
    area_index = 0;
    return 1;
  }
}
