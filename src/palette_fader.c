#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "sm_82_data.h"

uint16 CalculateNthTransitionColorComponentFromXtoY(uint16 a, uint16 k, uint16 j) {  // 0x82DAA6
  if (!a)
    return k;
  uint16 v4 = a - 1;
  if (v4 == palette_change_denom)
    return j;
  uint16 RegWord = SnesDivide(abs16(j - k) << 8, palette_change_denom - (v4 + 1) + 1);
  uint16 r18 = sign16(j - k) ? -RegWord : RegWord;
  return (uint16)(r18 + (k << 8)) >> 8;
}

uint16 CalculateNthTransitionColorFromXtoY(uint16 a, uint16 k, uint16 j) {  // 0x82DA4A
  return CalculateNthTransitionColorComponentFromXtoY(a, k & 0x1F, j & 0x1F) |
         CalculateNthTransitionColorComponentFromXtoY(a, (k >> 5) & 0x1F, (j >> 5) & 0x1F) << 5 |
         CalculateNthTransitionColorComponentFromXtoY(a, (k >> 10) & 0x1F, (j >> 10) & 0x1F) << 10;
}

uint8 AdvancePaletteFadeForAllPalettes(void) {  // 0x82DA02
  if ((uint16)(palette_change_denom + 1) >= palette_change_num) {
    for (int i = 0; i < 0x200; i += 2) {
      g_word_7EC404 = i;
      int v2 = i >> 1;
      if (target_palettes[v2] != palette_buffer[v2]) {
        uint16 v3 = CalculateNthTransitionColorFromXtoY(palette_change_num, palette_buffer[v2], target_palettes[v2]);
        i = g_word_7EC404;
        palette_buffer[g_word_7EC404 >> 1] = v3;
      }
    }
    ++palette_change_num;
    return 0;
  } else {
    palette_change_num = 0;
    return 1;
  }
}

uint8 AdvancePaletteFadeForAllPalettes_0xc(void) {  // 0x82D961
  palette_change_denom = 12;
  return AdvancePaletteFadeForAllPalettes();
}

uint8 AdvancePaletteFade_BgPalette6(void) {  // 0x82D96C
  palette_change_denom = 12;
  if (palette_change_num <= 0xD) {
    for (int i = 192; i < 0xE0; i += 2)
      palette_buffer[i >> 1] = CalculateNthTransitionColorFromXtoY(
        palette_change_num,
        palette_buffer[i >> 1],
        target_palettes[i >> 1]);
    ++palette_change_num;
    return 0;
  } else {
    palette_change_num = 0;
    return 1;
  }
}

uint8 AdvanceGradualColorChangeOfPalette(uint16 k, uint16 j) {  // 0x82D9B8
  palette_change_denom = 15;
  if ((int16)(15 - palette_change_num) >= 0) {
    uint16 R34 = j;
    do {
      palette_buffer[k >> 1] = CalculateNthTransitionColorFromXtoY(
        palette_change_num + 1,
        palette_buffer[k >> 1],
        target_palettes[k >> 1]);
      k += 2;
    } while (k < R34);
    ++palette_change_num;
    return 0;
  } else {
    palette_change_num = 0;
    return 1;
  }
}

void sub_82DB41(void) {  // 0x82DB41
  uint16 v0 = g_word_7EC404;
  do {
    int v1 = v0 >> 1;
    if (target_palettes[v1] != palette_buffer[v1]) {
      uint16 v2 = CalculateNthTransitionColorFromXtoY(palette_change_num, palette_buffer[v1], target_palettes[v1]);
      v0 = g_word_7EC404;
      palette_buffer[g_word_7EC404 >> 1] = v2;
    }
    v0 += 2;
    g_word_7EC404 = v0;
  } while ((v0 & 0x1F) != 0);
}

uint8 sub_82DB0C(uint16 a) {  // 0x82DB0C
  if ((uint16)(palette_change_denom + 1) >= palette_change_num) {
    g_word_7EC404 = 0;
    while (a) {
      if (!(a & 1))
        g_word_7EC404 += 32;
      else
        sub_82DB41();
      a >>= 1;
    }
    ++palette_change_num;
    return 0;
  } else {
    palette_change_num = 0;
    return 1;
  }
}

uint8 sub_82DAF7(uint16 a) {  // 0x82DAF7
  palette_change_denom = 12;
  return sub_82DB0C(a);
}

void LoadColorsForSpritesBeamsAndEnemies(void) {  // 0x82E139
  uint16 j;
  uint16 k;

  for (int i = 30; i >= 0; i -= 2)
    target_palettes[(i >> 1) + 208] = kInitialPalette[(i >> 1) + 208];
  for (j = 30; (j & 0x8000) == 0; j -= 2)
    target_palettes[(j >> 1) + 224] = palette_buffer[(j >> 1) + 224];
  for (k = 30; (k & 0x8000) == 0; k -= 2)
    target_palettes[(k >> 1) + 128] = kCommonSpritesPalette1[k >> 1];
}
