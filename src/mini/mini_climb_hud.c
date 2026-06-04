#include "mini_climb_hud.h"

#include <stdio.h>

#include "mini_climb_endless.h"
#include "mini_game.h"
#include "mini_renderer.h"

enum {
  kMiniClimbHudGlyphScale = 1,
  kMiniClimbHudGlyphWidth = 3,
  kMiniClimbHudGlyphHeight = 5,
  kMiniClimbHudCharAdvance = 4,
  kMiniClimbHudSpaceAdvance = 3,
  kMiniClimbHudPadX = 2,
  kMiniClimbHudPadY = 1,
};

static uint8 MiniClimbHudGlyphRows(char c, int row) {
  static const uint8 kDigits[10][5] = {
    {7, 5, 5, 5, 7},
    {2, 6, 2, 2, 7},
    {7, 1, 7, 4, 7},
    {7, 1, 7, 1, 7},
    {5, 5, 7, 1, 1},
    {7, 4, 7, 1, 7},
    {7, 4, 7, 5, 7},
    {7, 1, 1, 2, 2},
    {7, 5, 7, 5, 7},
    {7, 5, 7, 1, 7},
  };
  if (c >= '0' && c <= '9')
    return kDigits[c - '0'][row];
  switch (c) {
  case '+': return row == 2 ? 7 : 0;
  case ':': return row == 1 || row == 3 ? 2 : 0;
  case '.': return row == 4 ? 2 : 0;
  case '|': return row == 1 || row == 2 || row == 3 ? 2 : 0;
  default: return 0;
  }
}

static int MiniClimbHudTextWidth(const char *text) {
  int width = 0;
  for (const char *p = text; *p != '\0'; p++) {
    if (*p == ' ')
      width += kMiniClimbHudSpaceAdvance;
    else
      width += kMiniClimbHudCharAdvance;
  }
  return width;
}

static void MiniClimbHudRenderText(uint32_t *pixels, int pitch_pixels, int x, int y,
                                   const char *text, uint32_t color) {
  int cursor_x = x;
  for (const char *p = text; *p != '\0'; p++) {
    if (*p == ' ') {
      cursor_x += kMiniClimbHudSpaceAdvance;
      continue;
    }
    for (int gy = 0; gy < kMiniClimbHudGlyphHeight; gy++) {
      uint8 bits = MiniClimbHudGlyphRows(*p, gy);
      for (int gx = 0; gx < kMiniClimbHudGlyphWidth; gx++) {
        if ((bits & (1 << (kMiniClimbHudGlyphWidth - 1 - gx))) == 0)
          continue;
        MiniRenderer_FillRect(pixels, pitch_pixels,
                              cursor_x + gx * kMiniClimbHudGlyphScale,
                              y + gy * kMiniClimbHudGlyphScale,
                              kMiniClimbHudGlyphScale, kMiniClimbHudGlyphScale, color);
      }
    }
    cursor_x += kMiniClimbHudCharAdvance;
  }
}

void MiniClimbHud_Render(uint32_t *pixels, int pitch_pixels, const MiniGameState *state) {
  if (!MiniClimbEndless_IsActive())
    return;

  int total_centiseconds = (state->frame * 100) / 60;
  int centiseconds = total_centiseconds % 100;
  int total_seconds = total_centiseconds / 100;
  int seconds = total_seconds % 60;
  int minutes = total_seconds / 60;
  if (minutes > 99)
    minutes = 99;

  int ascent = MiniClimbEndless_AscentPixels();
  if (ascent > 9999)
    ascent = 9999;
  int floors = MiniClimbEndless_VirtualFloors();
  if (floors > 999)
    floors = 999;

  char hud_line[24];
  snprintf(hud_line, sizeof(hud_line), "%02d:%02d.%02d|+%04d|%03d",
           minutes, seconds, centiseconds, ascent, floors);

  int text_width = MiniClimbHudTextWidth(hud_line);
  int panel_width = text_width + kMiniClimbHudPadX * 2;
  int panel_height = kMiniClimbHudGlyphHeight + kMiniClimbHudPadY * 2;
  enum { kHudMargin = 2 };
  int panel_x = kHudMargin;
  int panel_y = kHudMargin;
  int text_x = panel_x + kMiniClimbHudPadX;
  int text_y = panel_y + kMiniClimbHudPadY;

  uint32_t panel = 0xC010151Au;
  uint32_t text = MiniRenderer_ConvertBgr555(0x03FF);
  MiniRenderer_FillRect(pixels, pitch_pixels, panel_x, panel_y, panel_width, panel_height, panel);
  MiniClimbHudRenderText(pixels, pitch_pixels, text_x, text_y, hud_line, text);
}
