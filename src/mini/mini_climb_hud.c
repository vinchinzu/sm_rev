#include "mini_climb_hud.h"

#include <stdio.h>

#include "ida_types.h"
#include "mini_climb_endless.h"
#include "mini_game.h"
#include "mini_renderer.h"
#include "variables.h"

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

  int total_centiseconds = (MiniClimbEndless_RunFrames(state) * 100) / 60;
  int centiseconds = total_centiseconds % 100;
  int total_seconds = total_centiseconds / 100;
  int seconds = total_seconds % 60;
  int minutes = total_seconds / 60;
  if (minutes > 99)
    minutes = 99;

  int ascent = MiniClimbEndless_AscentPixels(state);
  if (ascent > 9999)
    ascent = 9999;
  int floors = MiniClimbEndless_VirtualFloors(state);
  if (floors > 999)
    floors = 999;
  int energy = samus_health;
  if (energy > 999)
    energy = 999;

  char hud_line[32];
  snprintf(hud_line, sizeof(hud_line), "%02d:%02d.%02d|+%04d|%03d|%03d",
           minutes, seconds, centiseconds, ascent, floors, energy);

  int text_width = MiniClimbHudTextWidth(hud_line);
  int panel_width = text_width + kMiniClimbHudPadX * 2;
  int panel_height = kMiniClimbHudGlyphHeight + kMiniClimbHudPadY * 2;
  enum { kHudMargin = 2, kHudLowEnergy = 30 };
  int panel_x = kHudMargin;
  int panel_y = kHudMargin;
  int text_x = panel_x + kMiniClimbHudPadX;
  int text_y = panel_y + kMiniClimbHudPadY;

  bool in_danger = MiniClimbEndless_SamusInLava(state) || samus_health < kHudLowEnergy;
  uint32_t panel = in_danger ? 0xC0401012u : 0xC010151Au;
  uint32_t text = MiniRenderer_ConvertBgr555(in_danger ? 0x2D7F : 0x03FF);
  MiniRenderer_FillRect(pixels, pitch_pixels, panel_x, panel_y, panel_width, panel_height, panel);
  MiniClimbHudRenderText(pixels, pitch_pixels, text_x, text_y, hud_line, text);

  // Session line: best ascent and deaths, shown once a run has ended.
  if (MiniClimbEndless_Deaths(state) > 0) {
    int best = MiniClimbEndless_BestAscentPixels(state);
    if (best > 9999)
      best = 9999;
    int deaths = MiniClimbEndless_Deaths(state);
    if (deaths > 99)
      deaths = 99;
    char session_line[16];
    snprintf(session_line, sizeof(session_line), "+%04d|%02d", best, deaths);
    int session_y = panel_y + panel_height + 1;
    int session_width = MiniClimbHudTextWidth(session_line) + kMiniClimbHudPadX * 2;
    MiniRenderer_FillRect(pixels, pitch_pixels, panel_x, session_y,
                          session_width, panel_height, 0xC010151Au);
    MiniClimbHudRenderText(pixels, pitch_pixels, panel_x + kMiniClimbHudPadX,
                           session_y + kMiniClimbHudPadY, session_line,
                           MiniRenderer_ConvertBgr555(0x4631));
  }
}
