#include "mini_room_fx.h"

#include "ida_types.h"
#include "mini_climb_endless.h"
#include "mini_run_mode.h"
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

static int MiniRoomFx_SmallWaveOffset(int y, int frame, int amplitude) {
  static const int8_t kWave[16] = {0, 1, 1, 0, 0, -1, -1, 0, 0, 1, 1, 0, 0, -1, -1, 0};
  return kWave[((y >> 1) + frame) & 15] * amplitude;
}

int MiniRoomFx_EditorBg2DriftX(const MiniGameState *state) {
  return MiniRoomFx_IsLandingSite(state) ? state->frame >> 2 : 0;
}

int MiniRoomFx_EditorBg2ScrollXForScanline(const MiniGameState *state, int screen_y, int base_scroll_x) {
  if (MiniRoomFx_IsLandingSite(state)) {
    if ((unsigned)screen_y < 32) {
      int row_scroll = g_word_7E9E80[(screen_y >> 1) & 15];
      int reference_scroll = g_word_7E9E80[0];
      return base_scroll_x + row_scroll - reference_scroll;
    }
    return base_scroll_x;
  }

  switch (fx_type) {
  case 2:
  case 4:
  case 6:
    return base_scroll_x + MiniRoomFx_SmallWaveOffset(screen_y, state != NULL ? state->frame >> 2 : 0, 1);
  case 0xA:
    return base_scroll_x + (((screen_y + (state != NULL ? state->frame : 0)) >> 3) & 1);
  case 0x2C:
    return base_scroll_x + MiniRoomFx_SmallWaveOffset(screen_y, state != NULL ? state->frame >> 3 : 0, 1);
  default:
    return base_scroll_x;
  }
}

static void MiniRoomFx_RenderSkyLandOverlay(uint32_t *pixels, int pitch_pixels, const MiniGameState *state) {
  (void)state;
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

static void MiniRoomFx_RenderLiquidOverlay(uint32_t *pixels, int pitch_pixels, const MiniGameState *state) {
  if (MiniRunMode_IsClimbEndless() || MiniRoomFx_IsLandingSite(state))
    return;
  if (fx_type != 2 && fx_type != 4 && fx_type != 6)
    return;

  int surface_y = (int)fx_y_pos - (state != NULL ? state->camera_y : layer1_y_pos);
  if (surface_y < 0)
    surface_y = 0;
  if (surface_y >= kMiniGameHeight)
    return;

  uint32_t liquid_color = fx_type == 2 ? MiniRoomFx_ConvertBgr555(0x001F)
                         : fx_type == 4 ? MiniRoomFx_ConvertBgr555(0x03E0)
                                        : MiniRoomFx_ConvertBgr555(0x4210);
  uint32_t highlight = MiniRoomFx_ConvertBgr555(0x7FFF);
  for (int y = surface_y; y < kMiniGameHeight; y++) {
    int wave = MiniRoomFx_SmallWaveOffset(y, state != NULL ? state->frame >> 2 : 0, 2);
    for (int x = 0; x < kMiniGameWidth; x++) {
      uint32_t base = pixels[y * pitch_pixels + x];
      uint32_t tint = (((x + wave + y) & 31) < 2) ? highlight : liquid_color;
      pixels[y * pitch_pixels + x] = MiniRoomFx_BlendColor(base, tint, 1, 5);
    }
  }
}

static void MiniRoomFx_RenderRainOverlay(uint32_t *pixels, int pitch_pixels, const MiniGameState *state) {
  if (fx_type != 0xA)
    return;

  uint32_t rain = MiniRoomFx_ConvertBgr555(0x6B5A);
  int frame = state != NULL ? state->frame : 0;
  for (int y = 0; y < kMiniGameHeight; y++) {
    for (int x = ((frame + y * 3) & 15); x < kMiniGameWidth; x += 32) {
      for (int streak = 0; streak < 4; streak++) {
        int px = x + streak;
        int py = y + streak * 2;
        if ((unsigned)px >= kMiniGameWidth || (unsigned)py >= kMiniGameHeight)
          continue;
        uint32_t base = pixels[py * pitch_pixels + px];
        pixels[py * pitch_pixels + px] = MiniRoomFx_BlendColor(base, rain, 1, 3);
      }
    }
  }
}

static void MiniRoomFx_RenderHazeOverlay(uint32_t *pixels, int pitch_pixels, const MiniGameState *state) {
  if (fx_type != 0x2C && fx_layer_blending_config_c != 44)
    return;

  uint32_t haze = MiniRoomFx_ConvertBgr555(0x5294);
  int frame = state != NULL ? state->frame : 0;
  for (int y = 0; y < kMiniGameHeight; y++) {
    int shimmer = MiniRoomFx_SmallWaveOffset(y, frame >> 3, 1);
    for (int x = 0; x < kMiniGameWidth; x++) {
      if (((x + shimmer + y) & 7) == 0)
        continue;
      uint32_t base = pixels[y * pitch_pixels + x];
      pixels[y * pitch_pixels + x] = MiniRoomFx_BlendColor(base, haze, 1, 8);
    }
  }
}

static void MiniRoomFx_RenderClimbHeatTint(uint32_t *pixels, int pitch_pixels, int tier) {
  if (tier <= 0)
    return;
  uint32_t heat = 0xFFFF6020u;
  for (int y = 0; y < kMiniGameHeight; y++) {
    for (int x = 0; x < kMiniGameWidth; x++) {
      uint32_t base = pixels[y * pitch_pixels + x];
      pixels[y * pitch_pixels + x] = MiniRoomFx_BlendColor(base, heat, tier, 48);
    }
  }
}

static void MiniRoomFx_RenderClimbLavaWarning(uint32_t *pixels, int pitch_pixels,
                                              int gap_below_screen, int frame) {
  enum { kWarningRange = 240 };
  if (gap_below_screen >= kWarningRange)
    return;
  // Pulse faster and brighter the closer the lava gets to the screen edge.
  int closeness = kWarningRange - gap_below_screen;
  int pulse_period = gap_below_screen < 80 ? 8 : 16;
  if (((frame / pulse_period) & 1) == 0)
    return;
  uint32_t warning = 0xFFFF2810u;
  int numer = 1 + closeness / 60;
  for (int y = kMiniGameHeight - 4; y < kMiniGameHeight; y++) {
    for (int x = 0; x < kMiniGameWidth; x++) {
      uint32_t base = pixels[y * pitch_pixels + x];
      pixels[y * pitch_pixels + x] = MiniRoomFx_BlendColor(base, warning, numer, 5);
    }
  }
}

void MiniRoomFx_RenderClimbLavaOverlay(uint32_t *pixels, int pitch_pixels, const MiniGameState *state) {
  if (state == NULL || !MiniRunMode_IsClimbEndless())
    return;

  int frame = state->frame;
  MiniRoomFx_RenderClimbHeatTint(pixels, pitch_pixels, MiniClimbEndless_DifficultyTier(state));
  if (!state->climb.lava_enabled)
    return;

  int surface_y = state->climb.lava_floor_y - state->viewport.camera_y;
  if (surface_y >= kMiniGameHeight) {
    MiniRoomFx_RenderClimbLavaWarning(pixels, pitch_pixels, surface_y - kMiniGameHeight, frame);
    return;
  }

  uint32_t glow = 0xFFFF9030u;
  uint32_t crest = 0xFFFFE080u;
  uint32_t body = 0xFFE03808u;
  uint32_t deep = 0xFF801000u;
  int start_y = surface_y < 0 ? 0 : surface_y;
  for (int y = start_y; y < kMiniGameHeight; y++) {
    int depth = y - surface_y;
    int wave = MiniRoomFx_SmallWaveOffset(y, frame >> 2, 2);
    for (int x = 0; x < kMiniGameWidth; x++) {
      uint32_t base = pixels[y * pitch_pixels + x];
      int phase = x + state->viewport.camera_x + wave;
      uint32_t tint;
      int numer;
      if (depth < 2) {
        tint = crest;
        numer = 4;
      } else {
        bool bubble = ((phase * 5 + (y << 2) + frame * 3) % 97) < 3;
        tint = bubble ? crest : (depth > 48 ? deep : body);
        numer = depth > 16 ? 4 : 3;
      }
      pixels[y * pitch_pixels + x] = MiniRoomFx_BlendColor(base, tint, numer, 5);
    }
  }
  // Soft glow line floating just above the surface.
  if (surface_y - 1 >= 0 && surface_y - 1 < kMiniGameHeight) {
    int y = surface_y - 1;
    for (int x = 0; x < kMiniGameWidth; x++) {
      uint32_t base = pixels[y * pitch_pixels + x];
      pixels[y * pitch_pixels + x] = MiniRoomFx_BlendColor(base, glow, 1, 3);
    }
  }
}

void MiniRoomFx_RenderEditorOverlay(uint32_t *pixels, int pitch_pixels, const MiniGameState *state) {
  if (MiniRoomFx_IsLandingSite(state))
    MiniRoomFx_RenderSkyLandOverlay(pixels, pitch_pixels, state);
  MiniRoomFx_RenderLiquidOverlay(pixels, pitch_pixels, state);
  MiniRoomFx_RenderRainOverlay(pixels, pitch_pixels, state);
  MiniRoomFx_RenderHazeOverlay(pixels, pitch_pixels, state);
}
