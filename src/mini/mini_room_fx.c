#include "mini_room_fx.h"

#include "ida_types.h"
#include "mini_defs.h"
#include "mini_editor_bridge.h"
#include "variables.h"

static bool MiniRoomFx_IsLandingSite(const MiniGameState *state) {
  return state != NULL && state->room_id == kMiniEditorBridgeRoomId_LandingSite;
}

static uint32_t MiniRoomFx_ConvertBgr555(uint16 color) {
  uint32_t r = (color & 0x1F) * 255 / 31;
  uint32_t g = ((color >> 5) & 0x1F) * 255 / 31;
  uint32_t b = ((color >> 10) & 0x1F) * 255 / 31;
  return 0xFF000000u | (r << 16) | (g << 8) | b;
}

static uint32_t MiniRoomFx_BlendColor(uint32_t a, uint32_t b, int numer, int denom) {
  uint32_t ar = (a >> 16) & 0xFF, ag = (a >> 8) & 0xFF, ab = a & 0xFF;
  uint32_t br = (b >> 16) & 0xFF, bg = (b >> 8) & 0xFF, bb = b & 0xFF;
  uint32_t r = (ar * (uint32_t)(denom - numer) + br * (uint32_t)numer) / (uint32_t)denom;
  uint32_t g = (ag * (uint32_t)(denom - numer) + bg * (uint32_t)numer) / (uint32_t)denom;
  uint32_t bl = (ab * (uint32_t)(denom - numer) + bb * (uint32_t)numer) / (uint32_t)denom;
  return 0xFF000000u | (r << 16) | (g << 8) | bl;
}

static int MiniRoomFx_LandingSiteBg2ScanlineScrollX(int y) {
  if ((unsigned)y < 32)
    return g_word_7E9E80[(y >> 1) & 15];
  return reg_BG2HOFS + (nmi_frame_counter_word >> 2);
}

int MiniRoomFx_EditorBg2DriftX(const MiniGameState *state) {
  return MiniRoomFx_IsLandingSite(state) ? state->frame >> 2 : 0;
}

void MiniRoomFx_RenderEditorOverlay(uint32_t *pixels, int pitch_pixels, const MiniGameState *state) {
  if (!MiniRoomFx_IsLandingSite(state))
    return;

  uint32_t cloud_light = MiniRoomFx_ConvertBgr555(0x2D6B);
  uint32_t cloud_dark = MiniRoomFx_ConvertBgr555(0x1C63);
  for (int y = 0; y < 80; y++) {
    int scroll_x = MiniRoomFx_LandingSiteBg2ScanlineScrollX(y);
    uint32_t cloud_color = y < 32 ? cloud_light : cloud_dark;
    for (int x = 0; x < kMiniGameWidth; x++) {
      int phase = ((x + scroll_x) >> 3) + (y >> 1);
      bool draw = ((phase & 15) < (y < 32 ? 5 : 4)) || ((((x + scroll_x) >> 4) + y) & 31) == 0;
      if (!draw)
        continue;
      uint32_t base = pixels[y * pitch_pixels + x];
      pixels[y * pitch_pixels + x] = MiniRoomFx_BlendColor(base, cloud_color, y < 32 ? 1 : 2, 4);
    }
  }
}
