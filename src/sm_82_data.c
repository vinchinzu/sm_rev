#include "sm_82_data.h"

const uint16 kMap_Criteria_SavePoints[16] = { 0x0168, 0x0108, 0x0148, 0x0188, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

const Buttons word_82F575[9] = { 0x0080, 0x0040, 0x0020, 0x0010, 0x0100, 0x0200, 0x0400, 0x0800, 0x1000 };

const uint16 g_word_82F149[4] = { 0x0001, 0x0002, 0x0004, 0x0008 };
const uint16 g_word_82F151[4] = { 0x0010, 0x0020, 0x0040, 0x0080 };

const uint16 word_82F204[16] = {
  0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
  0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000, 0x8000
};

const uint16 g_word_82F6AD[6] = { 0, 1, 2, 3, 4, 5 };

const uint16 kDrawMenuSelectionMissile_Enable[4] = { 0x0080, 0x0100, 0x0200, 0x0400 };
const uint16 kDrawMenuSelectionMissile_SpriteMap[4] = { 0x0001, 0x0002, 0x0003, 0x0004 };

const uint16 kEquipmentScreenWireframeCmp[3] = { 0x0101, 0x0001, 0x0000 };
const uint16 kEquipmentScreenWireframePtrs[3] = { 0xb251, 0xb231, 0xb211 };

// Backup variables for pausing are handled by macros in variables.h
uint16 oam_next_ptr_backup;

// ChangePaletteValues is a common helper that was in sm_82.c
void ChangePaletteValues(uint16 *tilemap, uint16 palette_mask, uint16 count) {
  for (int i = 0; i < count; ++i)
    tilemap[i] = (tilemap[i] & ~0x1C00) | palette_mask;
}
