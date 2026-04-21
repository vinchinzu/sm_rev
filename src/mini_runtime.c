#include <stdio.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <SDL.h>

#include "ida_types.h"
#include "features.h"
#include "funcs.h"
#include "mini_game.h"
#include "mini_runtime.h"
#include "samus_asset_bridge.h"
#include "stubs_mini.h"
#include "variables.h"

#if CURRENT_BUILD != BUILD_MINI
#error "mini_runtime.c must be compiled with CURRENT_BUILD=BUILD_MINI"
#endif

typedef struct MiniScriptFrame {
  uint16 buttons;
  bool quit_requested;
} MiniScriptFrame;

typedef struct MiniInputScript {
  MiniScriptFrame *frames;
  int count;
} MiniInputScript;

enum {
  kMiniEditorAirMetatile = 0xFF,
};

static int MiniBgTileBase(int layer) {
  uint16 word_addr = layer == 1 ? (reg_BG12NBA & 0xF) << 12
                                : (reg_BG12NBA & 0xF0) << 8;
  return word_addr >> 4;
}

static void MiniFreeInputScript(MiniInputScript *script) {
  free(script->frames);
  script->frames = NULL;
  script->count = 0;
}

static void MiniUppercase(char *text) {
  for (; *text; text++)
    *text = (char)toupper((unsigned char)*text);
}

static bool MiniParseInputToken(const char *token, MiniScriptFrame *frame) {
  if (!strcmp(token, ".") || !strcmp(token, "WAIT") || !strcmp(token, "NONE"))
    return true;
  if (!strcmp(token, "L") || !strcmp(token, "LEFT")) {
    frame->buttons |= kButton_Left;
  } else if (!strcmp(token, "R") || !strcmp(token, "RIGHT")) {
    frame->buttons |= kButton_Right;
  } else if (!strcmp(token, "U") || !strcmp(token, "UP")) {
    frame->buttons |= kButton_Up;
  } else if (!strcmp(token, "D") || !strcmp(token, "DOWN")) {
    frame->buttons |= kButton_Down;
  } else if (!strcmp(token, "J") || !strcmp(token, "JUMP")) {
    frame->buttons |= kButton_A;
  } else if (!strcmp(token, "RUN") || !strcmp(token, "B")) {
    frame->buttons |= kButton_B;
  } else if (!strcmp(token, "SHOOT") || !strcmp(token, "X")) {
    frame->buttons |= kButton_X;
  } else if (!strcmp(token, "ITEM") || !strcmp(token, "Y")) {
    frame->buttons |= kButton_Y;
  } else if (!strcmp(token, "AIMUP") || !strcmp(token, "RU") || !strcmp(token, "RSHOULDER")) {
    frame->buttons |= kButton_R;
  } else if (!strcmp(token, "AIMDOWN") || !strcmp(token, "LU") || !strcmp(token, "LSHOULDER")) {
    frame->buttons |= kButton_L;
  } else if (!strcmp(token, "QUIT")) {
    frame->quit_requested = true;
  } else {
    return false;
  }
  return true;
}

static bool MiniLoadInputScript(const char *path, MiniInputScript *script) {
  FILE *f = fopen(path, "r");
  if (f == NULL) {
    fprintf(stderr, "mini: could not open input script %s\n", path);
    return false;
  }

  script->frames = NULL;
  script->count = 0;
  int capacity = 0;
  char line[512];
  while (fgets(line, sizeof(line), f) != NULL) {
    char *comment = strchr(line, '#');
    if (comment != NULL)
      *comment = '\0';

    MiniScriptFrame frame = {0};
    char *saveptr = NULL;
    for (char *token = strtok_r(line, " ,+\t\r\n", &saveptr);
         token != NULL;
         token = strtok_r(NULL, " ,+\t\r\n", &saveptr)) {
      MiniUppercase(token);
      if (!MiniParseInputToken(token, &frame)) {
        fprintf(stderr, "mini: unknown input token '%s' in %s\n", token, path);
        fclose(f);
        MiniFreeInputScript(script);
        return false;
      }
    }

    if (script->count == capacity) {
      capacity = capacity ? capacity * 2 : 64;
      MiniScriptFrame *frames = realloc(script->frames, (size_t)capacity * sizeof(*frames));
      if (frames == NULL) {
        fclose(f);
        MiniFreeInputScript(script);
        return false;
      }
      script->frames = frames;
    }
    script->frames[script->count++] = frame;
  }

  fclose(f);
  return true;
}

static void PrintResult(const MiniOptions *options, const MiniGameState *state) {
  printf("{\"build\":\"mini\",\"headless\":%s,\"frames\":%d,"
         "\"no_enemies\":true,\"no_bosses\":true,\"no_rooms\":%s,"
         "\"room_ptr\":%u,\"room_width\":%d,\"room_height\":%d,"
         "\"room_source\":\"%s\",\"room_visuals\":\"%s\",\"room_handle\":\"%s\","
         "\"samus_suit\":\"%s\","
         "\"rom_room\":%s,"
         "\"samus_x\":%d,\"samus_y\":%d}\n",
         options->headless ? "true" : "false", state->frame,
         state->has_room ? "false" : "true",
         state->room_id,
         state->room_width_blocks * kMiniBlockSize,
         state->room_height_blocks * kMiniBlockSize,
         MiniStubs_RoomSourceName(state->room_source),
         state->uses_rom_room ? "rom" : (state->has_editor_room_visuals ? "editor_tileset" : "placeholder"),
         state->room_handle,
         MiniStubs_SamusSuitName(state->samus_suit),
         state->uses_rom_room ? "true" : "false",
         state->samus_x, state->samus_y);
}

static uint32_t MiniConvertBgr555(uint16 color) {
  uint32_t r = (color & 0x1F) * 255 / 31;
  uint32_t g = ((color >> 5) & 0x1F) * 255 / 31;
  uint32_t b = ((color >> 10) & 0x1F) * 255 / 31;
  return 0xFF000000u | (r << 16) | (g << 8) | b;
}

static uint32_t MiniBlendColor(uint32_t a, uint32_t b, int numer, int denom) {
  uint32_t ar = (a >> 16) & 0xFF, ag = (a >> 8) & 0xFF, ab = a & 0xFF;
  uint32_t br = (b >> 16) & 0xFF, bg = (b >> 8) & 0xFF, bb = b & 0xFF;
  uint32_t r = (ar * (uint32_t)(denom - numer) + br * (uint32_t)numer) / (uint32_t)denom;
  uint32_t g = (ag * (uint32_t)(denom - numer) + bg * (uint32_t)numer) / (uint32_t)denom;
  uint32_t bl = (ab * (uint32_t)(denom - numer) + bb * (uint32_t)numer) / (uint32_t)denom;
  return 0xFF000000u | (r << 16) | (g << 8) | bl;
}

static void MiniFillRect(uint32_t *pixels, int pitch_pixels, int left, int top,
                         int width, int height, uint32_t color) {
  for (int py = 0; py < height; py++) {
    int y = top + py;
    if ((unsigned)y >= kMiniGameHeight)
      continue;
    for (int px = 0; px < width; px++) {
      int x = left + px;
      if ((unsigned)x >= kMiniGameWidth)
        continue;
      pixels[y * pitch_pixels + x] = color;
    }
  }
}

static uint8 MiniDecode4bppPixel(const uint8 *tiles, int tile_index, int x, int y) {
  const uint8 *tile = tiles + tile_index * 32;
  uint8 mask = 0x80 >> x;
  uint8 p0 = (tile[y * 2] & mask) != 0;
  uint8 p1 = (tile[y * 2 + 1] & mask) != 0;
  uint8 p2 = (tile[16 + y * 2] & mask) != 0;
  uint8 p3 = (tile[16 + y * 2 + 1] & mask) != 0;
  return p0 | (p1 << 1) | (p2 << 2) | (p3 << 3);
}

static void MiniRenderTile(uint32_t *pixels, int pitch_pixels, const uint8 *tiles,
                           const uint16 *palette, int tile_base, uint16 tile_attr,
                           int dst_x, int dst_y) {
  int tile_index = tile_base + (tile_attr & 0x3FF);
  int palette_base = ((tile_attr >> 10) & 7) * 16;
  bool x_flip = (tile_attr & 0x4000) != 0;
  bool y_flip = (tile_attr & 0x8000) != 0;
  for (int py = 0; py < 8; py++) {
    int sy = y_flip ? 7 - py : py;
    int out_y = dst_y + py;
    if ((unsigned)out_y >= kMiniGameHeight)
      continue;
    for (int px = 0; px < 8; px++) {
      int sx = x_flip ? 7 - px : px;
      int out_x = dst_x + px;
      if ((unsigned)out_x >= kMiniGameWidth)
        continue;
      uint8 color_index = MiniDecode4bppPixel(tiles, tile_index, sx, sy);
      if (color_index == 0)
        continue;
      pixels[out_y * pitch_pixels + out_x] = MiniConvertBgr555(palette[palette_base + color_index]);
    }
  }
}

static uint16 MiniReadVramWord(const uint8 *vram, uint16 word_addr) {
  size_t offset = (size_t)(word_addr & 0x7FFF) << 1;
  return GET_WORD(vram + offset);
}

static uint16 MiniGetBgTilemapWord(const uint8 *vram, uint8 bg_sc, int tile_x, int tile_y) {
  uint16 addr = (bg_sc & 0xFC) << 8;
  addr += ((tile_y & 31) << 5) | (tile_x & 31);
  if ((tile_x & 32) && (bg_sc & 1))
    addr += 0x400;
  if ((tile_y & 32) && (bg_sc & 2))
    addr += (bg_sc & 1) ? 0x800 : 0x400;
  return MiniReadVramWord(vram, addr);
}

static void MiniRenderBgLayer(uint32_t *pixels, int pitch_pixels, const uint8 *vram,
                              uint8 bg_sc, int tile_base, int scroll_x, int scroll_y) {
  int first_tile_x = scroll_x / 8;
  int first_tile_y = scroll_y / 8;
  int last_tile_x = (scroll_x + kMiniGameWidth + 7) / 8;
  int last_tile_y = (scroll_y + kMiniGameHeight + 7) / 8;
  for (int tile_y = first_tile_y; tile_y <= last_tile_y; tile_y++) {
    for (int tile_x = first_tile_x; tile_x <= last_tile_x; tile_x++) {
      uint16 tile_attr = MiniGetBgTilemapWord(vram, bg_sc, tile_x, tile_y);
      int screen_x = tile_x * 8 - scroll_x;
      int screen_y = tile_y * 8 - scroll_y;
      MiniRenderTile(pixels, pitch_pixels, vram, target_palettes, tile_base, tile_attr, screen_x, screen_y);
    }
  }
}

static void MiniRenderEditorRoomTiles(uint32_t *pixels, int pitch_pixels, const MiniGameState *state) {
  MiniEditorTilesetView view;
  MiniStubs_GetEditorTilesetView(&view);
  if (!view.loaded)
    return;

  int first_block_x = state->camera_x / kMiniBlockSize;
  int first_block_y = state->camera_y / kMiniBlockSize;
  int last_block_x = (state->camera_x + kMiniGameWidth + kMiniBlockSize - 1) / kMiniBlockSize;
  int last_block_y = (state->camera_y + kMiniGameHeight + kMiniBlockSize - 1) / kMiniBlockSize;
  for (int block_y = first_block_y; block_y <= last_block_y; block_y++) {
    for (int block_x = first_block_x; block_x <= last_block_x; block_x++) {
      uint16 level = MiniStubs_GetLevelBlock(block_x, block_y);
      int metatile_index = level & 0x3FF;
      if ((unsigned)metatile_index >= 1024)
        continue;
      if (metatile_index == kMiniEditorAirMetatile)
        continue;
      bool metatile_hflip = (level & 0x400) != 0;
      bool metatile_vflip = (level & 0x800) != 0;
      const uint16 *metatile = view.metatile_words + metatile_index * 4;
      int screen_left = block_x * kMiniBlockSize - state->camera_x;
      int screen_top = block_y * kMiniBlockSize - state->camera_y;
      for (int quadrant = 0; quadrant < 4; quadrant++) {
        int src_quadrant = quadrant;
        if (metatile_hflip)
          src_quadrant ^= 1;
        if (metatile_vflip)
          src_quadrant ^= 2;
        uint16 tile_attr = metatile[src_quadrant];
        if (metatile_hflip)
          tile_attr ^= 0x4000;
        if (metatile_vflip)
          tile_attr ^= 0x8000;
        MiniRenderTile(
            pixels, pitch_pixels, view.tiles4bpp, view.palette, 0, tile_attr,
            screen_left + ((quadrant & 1) ? 8 : 0),
            screen_top + ((quadrant & 2) ? 8 : 0));
      }
    }
  }
}

static int MiniComputeEditorLayer2Pos(int layer1_pos, uint8 scroll_mode) {
  if (scroll_mode == 0)
    return layer1_pos;
  if (scroll_mode == 1)
    return 0;
  int t = scroll_mode & 0xFE;
  return t * (layer1_pos >> 8) + ((t * (layer1_pos & 0xFF)) >> 8);
}

static void MiniRenderEditorBg2(uint32_t *pixels, int pitch_pixels, const MiniGameState *state,
                                const MiniEditorTilesetView *tileset_view) {
  MiniEditorBg2View bg2_view;
  MiniStubs_GetEditorBg2View(&bg2_view);
  if (bg2_view.uses_rom_vram) {
    MiniRenderBgLayer(pixels, pitch_pixels, MiniStubs_GetVram(), reg_BG2SC, MiniBgTileBase(2),
                      reg_BG2HOFS, reg_BG2VOFS);
    return;
  }
  if (!bg2_view.loaded || bg2_view.tilemap_words == NULL || !tileset_view->loaded)
    return;
  (void)state;

  int scroll_x = reg_BG2HOFS;
  int scroll_y = reg_BG2VOFS;
  int first_tile_x = scroll_x / 8;
  int first_tile_y = scroll_y / 8;
  int last_tile_x = (scroll_x + kMiniGameWidth + 7) / 8;
  int last_tile_y = (scroll_y + kMiniGameHeight + 7) / 8;
  for (int tile_y = first_tile_y; tile_y <= last_tile_y; tile_y++) {
    int wrapped_y = tile_y & 31;
    for (int tile_x = first_tile_x; tile_x <= last_tile_x; tile_x++) {
      int wrapped_x = tile_x & 63;
      uint16 tile_attr = bg2_view.tilemap_words[wrapped_y * 64 + wrapped_x];
      int screen_x = tile_x * 8 - scroll_x;
      int screen_y = tile_y * 8 - scroll_y;
      MiniRenderTile(pixels, pitch_pixels, tileset_view->tiles4bpp, tileset_view->palette,
                     0, tile_attr, screen_x, screen_y);
    }
  }
}

static void MiniRenderObjTileWithPalette(uint32_t *pixels, int pitch_pixels, const uint8 *tiles,
                                         size_t tiles_size, const uint16 *palette, int tile_index,
                                         bool x_flip, bool y_flip, int dst_x, int dst_y) {
  size_t tile_addr = (size_t)tile_index * 32;
  if (palette == NULL || tile_addr > tiles_size || tiles_size - tile_addr < 32)
    return;
  const uint8 *tile = tiles + tile_addr;
  for (int py = 0; py < 8; py++) {
    int sy = y_flip ? 7 - py : py;
    int out_y = dst_y + py;
    if ((unsigned)out_y >= kMiniGameHeight)
      continue;
    for (int px = 0; px < 8; px++) {
      int sx = x_flip ? 7 - px : px;
      int out_x = dst_x + px;
      if ((unsigned)out_x >= kMiniGameWidth)
        continue;
      uint8 color_index = MiniDecode4bppPixel(tile, 0, sx, sy);
      if (color_index == 0)
        continue;
      pixels[out_y * pitch_pixels + out_x] = MiniConvertBgr555(palette[color_index]);
    }
  }
}

static void MiniRenderEditorRoomSprites(uint32_t *pixels, int pitch_pixels, const MiniGameState *state) {
  enum { kMiniEditorRoomSpriteTileBase = 0x100 };
  const MiniEditorRoomSpriteView *sprites = NULL;
  int count = MiniStubs_GetEditorRoomSpriteViews(&sprites);
  if (count <= 0 || sprites == NULL)
    return;

  for (int i = count - 1; i >= 0; i--) {
    const MiniEditorRoomSpriteView *sprite = &sprites[i];
    if (sprite->tile_data == NULL || sprite->palette == NULL || sprite->entries == NULL)
      continue;
    for (int j = 0; j < sprite->entry_count; j++) {
      const MiniEditorRoomSpriteOamView *entry = &sprite->entries[j];
      int base_x = sprite->x_pos - state->camera_x + entry->x_offset;
      int base_y = sprite->y_pos - state->camera_y + entry->y_offset;
      int tile_index = (entry->tile_num & 0x1FF) - kMiniEditorRoomSpriteTileBase;
      if (tile_index < 0)
        continue;
      if (entry->is_16x16) {
        static const int kTileDx[4] = {0, 8, 0, 8};
        static const int kTileDy[4] = {0, 0, 8, 8};
        static const int kTileAdd[4] = {0, 1, 16, 17};
        for (int part = 0; part < 4; part++) {
          int draw_part = part;
          if (entry->h_flip)
            draw_part ^= 1;
          if (entry->v_flip)
            draw_part ^= 2;
          MiniRenderObjTileWithPalette(
              pixels, pitch_pixels, sprite->tile_data, sprite->tile_data_size, sprite->palette,
              tile_index + kTileAdd[draw_part], entry->h_flip, entry->v_flip,
              base_x + kTileDx[part], base_y + kTileDy[part]);
        }
      } else {
        MiniRenderObjTileWithPalette(
            pixels, pitch_pixels, sprite->tile_data, sprite->tile_data_size, sprite->palette,
            tile_index, entry->h_flip, entry->v_flip, base_x, base_y);
      }
    }
  }
}

static void MiniRenderRoom(uint32_t *pixels, int pitch_pixels, const MiniGameState *state) {
  if (!state->uses_rom_room) {
    MiniEditorTilesetView tileset_view;
    MiniStubs_GetEditorTilesetView(&tileset_view);
    uint32_t sky_top = tileset_view.loaded ? MiniConvertBgr555(tileset_view.palette[0]) : MiniConvertBgr555(0x0C24);
    uint32_t sky_bottom = MiniConvertBgr555(0x1C03);
    for (int y = 0; y < kMiniGameHeight; y++) {
      uint32_t sky = MiniBlendColor(sky_top, sky_bottom, y, kMiniGameHeight - 1);
      for (int x = 0; x < kMiniGameWidth; x++)
        pixels[y * pitch_pixels + x] = sky;
    }

    MiniRenderEditorBg2(pixels, pitch_pixels, state, &tileset_view);

    if (state->has_editor_room_visuals) {
      MiniRenderEditorRoomTiles(pixels, pitch_pixels, state);
      return;
    }

    static const uint16 kCollisionPalette[16] = {
      0x0012, 0x2D06, 0x1C63, 0x03E0,
      0x7C00, 0x0210, 0x4210, 0x56B5,
      0x5F5A, 0x03FF, 0x0010, 0x2D6B,
      0x001F, 0x03FF, 0x7C1F, 0x4631,
    };
    int first_block_x = state->camera_x / kMiniBlockSize;
    int first_block_y = state->camera_y / kMiniBlockSize;
    int last_block_x = (state->camera_x + kMiniGameWidth + kMiniBlockSize - 1) / kMiniBlockSize;
    int last_block_y = (state->camera_y + kMiniGameHeight + kMiniBlockSize - 1) / kMiniBlockSize;
    for (int block_y = first_block_y; block_y <= last_block_y; block_y++) {
      for (int block_x = first_block_x; block_x <= last_block_x; block_x++) {
        uint16 level = MiniStubs_GetLevelBlock(block_x, block_y);
        uint8 collision_type = level >> 12;
        if (collision_type == 0)
          continue;
        uint8 bts = MiniStubs_GetBts(block_x, block_y);
        uint32_t color = MiniConvertBgr555(kCollisionPalette[collision_type & 0xF]);
        uint32_t shade = MiniBlendColor(color, 0xFF000000u, 1, 4);
        uint32_t hilite = MiniBlendColor(color, 0xFFFFFFFFu, 1, 5);
        int screen_left = block_x * kMiniBlockSize - state->camera_x;
        int screen_top = block_y * kMiniBlockSize - state->camera_y;
        MiniFillRect(pixels, pitch_pixels, screen_left, screen_top, kMiniBlockSize, kMiniBlockSize, color);
        MiniFillRect(pixels, pitch_pixels, screen_left, screen_top, kMiniBlockSize, 1, hilite);
        MiniFillRect(pixels, pitch_pixels, screen_left, screen_top + kMiniBlockSize - 1, kMiniBlockSize, 1, shade);
        MiniFillRect(pixels, pitch_pixels, screen_left, screen_top, 1, kMiniBlockSize, hilite);
        MiniFillRect(pixels, pitch_pixels, screen_left + kMiniBlockSize - 1, screen_top, 1, kMiniBlockSize, shade);
        for (int py = 1; py < kMiniBlockSize - 1; py++) {
          int out_y = screen_top + py;
          if ((unsigned)out_y >= kMiniGameHeight)
            continue;
          for (int px = 1; px < kMiniBlockSize - 1; px++) {
            int out_x = screen_left + px;
            if ((unsigned)out_x >= kMiniGameWidth)
              continue;
            bool draw = false;
            switch (collision_type) {
            case 1:
              draw = ((px + py + bts) & 7) == 0;
              break;
            case 8:
              draw = ((px + (15 - py) + (bts >> 1)) & 7) <= 1;
              break;
            case 9:
              draw = px == py || px == 15 - py || px == 7 || py == 7;
              break;
            default:
              draw = (((px ^ py) + bts) & 7) == 0;
              break;
            }
            if (draw)
              pixels[out_y * pitch_pixels + out_x] = shade;
          }
        }
      }
    }
    return;
  }

  (void)state;
  const uint8 *vram = MiniStubs_GetVram();
  uint32_t clear = MiniConvertBgr555(target_palettes[0]);
  for (int y = 0; y < kMiniGameHeight; y++) {
    for (int x = 0; x < kMiniGameWidth; x++)
      pixels[y * pitch_pixels + x] = clear;
  }
  MiniRenderBgLayer(pixels, pitch_pixels, vram, reg_BG2SC, MiniBgTileBase(2), reg_BG2HOFS, reg_BG2VOFS);
  MiniRenderBgLayer(pixels, pitch_pixels, vram, reg_BG1SC, MiniBgTileBase(1), reg_BG1HOFS, reg_BG1VOFS);
}

static void MiniTransferSamusHalfToObjTiles(uint16 tile_src, uint16 top_vram_addr, uint16 bottom_vram_addr) {
  SamusTileAnimationTileDefs *td = (SamusTileAnimationTileDefs *)SamusAssetBridge_GetBank92(tile_src);
  if (td == NULL)
    return;
  uint32 src_addr = td->src.addr | ((uint32)td->src.bank << 16);
  const uint8 *src = SamusAssetBridge_GetData(src_addr, td->part1_size + td->part2_size);
  if (src == NULL)
    return;
  uint8 *vram = MiniStubs_GetVram();
  memcpy(vram + ((size_t)top_vram_addr << 1), src, td->part1_size);
  if (td->part2_size != 0)
    memcpy(vram + ((size_t)bottom_vram_addr << 1), src + td->part1_size, td->part2_size);
}

static void MiniPrepareSamusObjTiles(void) {
  if ((uint8)nmi_copy_samus_halves != 0)
    MiniTransferSamusHalfToObjTiles(nmi_copy_samus_top_half_src, 0x6000, 0x6100);
  if (HIBYTE(nmi_copy_samus_halves) != 0)
    MiniTransferSamusHalfToObjTiles(nmi_copy_samus_bottom_half_src, 0x6080, 0x6180);
}

static int MiniObjSpriteSizePixels(void) {
  static const uint8 kObjSizeTable[8][2] = {
    {8, 16},
    {8, 32},
    {8, 64},
    {16, 32},
    {16, 64},
    {32, 64},
    {16, 32},
    {16, 32},
  };
  return kObjSizeTable[(reg_OBSEL >> 5) & 7][1];
}

static int MiniObjTileAddr(bool name_select) {
  int obj_tile_addr1 = (reg_OBSEL & 7) << 14;
  int obj_tile_addr2 = obj_tile_addr1 + (((reg_OBSEL & 0x18) + 8) << 10);
  return name_select ? obj_tile_addr2 : obj_tile_addr1;
}

static int MiniObjTileOffset(uint16 charnum, int tile_x, int tile_y) {
  int tile = charnum & 0xFF;
  return ((((tile >> 4) + tile_y) << 4) | (((tile & 0xF) + tile_x) & 0xF)) * 32;
}

static void MiniRenderObjTile(uint32_t *pixels, int pitch_pixels, const uint8 *tiles,
                              int tile_addr, int palette_base, bool x_flip, bool y_flip,
                              int dst_x, int dst_y) {
  if ((unsigned)tile_addr > 0x10000 - 32)
    return;
  const uint8 *tile = tiles + tile_addr;
  for (int py = 0; py < 8; py++) {
    int sy = y_flip ? 7 - py : py;
    int out_y = dst_y + py;
    if ((unsigned)out_y >= kMiniGameHeight)
      continue;
    for (int px = 0; px < 8; px++) {
      int sx = x_flip ? 7 - px : px;
      int out_x = dst_x + px;
      if ((unsigned)out_x >= kMiniGameWidth)
        continue;
      uint8 color_index = MiniDecode4bppPixel(tile, 0, sx, sy);
      if (color_index == 0)
        continue;
      pixels[out_y * pitch_pixels + out_x] = MiniConvertBgr555(target_palettes[palette_base + color_index]);
    }
  }
}

static void MiniRenderSamus(uint32_t *pixels, int pitch_pixels) {
  const uint8 *vram = MiniStubs_GetVram();
  memset(oam_ext, 0, sizeof(uint16) * 16);
  oam_next_ptr = 0;
  nmi_copy_samus_halves = 0;
  Samus_DrawWhenNotAnimatingOrDying();
  MiniPrepareSamusObjTiles();

  int large_size = MiniObjSpriteSizePixels();
  int small_size = large_size / 2;
  for (int idx = 0; idx < oam_next_ptr; idx += 4) {
    uint16 ext = (oam_ext[idx >> 5] >> (2 * ((idx >> 2) & 7))) & 3;
    OamEnt *oam = gOamEnt(idx);
    uint16 attr = *(uint16 *)&oam->charnum;
    int charnum = attr & 0x1FF;
    int palette_base = 128 + ((attr >> 9) & 7) * 16;
    bool x_flip = (attr & 0x4000) != 0;
    bool y_flip = (attr & 0x8000) != 0;
    int size = (ext & 2) ? large_size : small_size;
    int tiles_per_side = size / 8;
    int sprite_x = oam->xcoord | ((ext & 1) << 8);
    int sprite_y = oam->ycoord;
    int obj_tile_base = MiniObjTileAddr((charnum & 0x100) != 0);
    if (sprite_x >= 256)
      sprite_x -= 512;
    if (sprite_y >= 224)
      sprite_y -= 256;
    if (sprite_x <= -size || sprite_x >= kMiniGameWidth || sprite_y <= -size || sprite_y >= kMiniGameHeight)
      continue;

    for (int ty = 0; ty < tiles_per_side; ty++) {
      for (int tx = 0; tx < tiles_per_side; tx++) {
        int tile_x = x_flip ? (tiles_per_side - 1 - tx) : tx;
        int tile_y = y_flip ? (tiles_per_side - 1 - ty) : ty;
        int tile_addr = obj_tile_base + MiniObjTileOffset(charnum, tile_x, tile_y);
        int dst_x = sprite_x + tx * 8;
        int dst_y = sprite_y + ty * 8;
        MiniRenderObjTile(pixels, pitch_pixels, vram, tile_addr, palette_base,
                          x_flip, y_flip, dst_x, dst_y);
      }
    }
  }
}

static void MiniRenderRoomSprites(uint32_t *pixels, int pitch_pixels) {
  const MiniRoomSprite *sprites = NULL;
  int count = MiniStubs_GetRoomSprites(&sprites);
  if (count <= 0 || sprites == NULL)
    return;

  memset(oam_ext, 0, sizeof(uint16) * 16);
  oam_next_ptr = 0;
  for (int i = 0; i < count; i++) {
    const MiniRoomSprite *sprite = &sprites[i];
    if (!sprite->active)
      continue;
    DrawSpritemapWithBaseTile(sprite->bank,
                              sprite->spritemap,
                              sprite->x_pos - layer1_x_pos,
                              sprite->y_pos - layer1_y_pos,
                              sprite->palette_index,
                              sprite->vram_tiles_index);
  }

  const uint8 *vram = MiniStubs_GetVram();
  int large_size = MiniObjSpriteSizePixels();
  int small_size = large_size / 2;
  for (int idx = 0; idx < oam_next_ptr; idx += 4) {
    uint16 ext = (oam_ext[idx >> 5] >> (2 * ((idx >> 2) & 7))) & 3;
    OamEnt *oam = gOamEnt(idx);
    uint16 attr = *(uint16 *)&oam->charnum;
    int charnum = attr & 0x1FF;
    int palette_base = 128 + ((attr >> 9) & 7) * 16;
    bool x_flip = (attr & 0x4000) != 0;
    bool y_flip = (attr & 0x8000) != 0;
    int size = (ext & 2) ? large_size : small_size;
    int tiles_per_side = size / 8;
    int sprite_x = oam->xcoord | ((ext & 1) << 8);
    int sprite_y = oam->ycoord;
    int obj_tile_base = MiniObjTileAddr((charnum & 0x100) != 0);
    if (sprite_x >= 256)
      sprite_x -= 512;
    if (sprite_y >= 224)
      sprite_y -= 256;
    if (sprite_x <= -size || sprite_x >= kMiniGameWidth || sprite_y <= -size || sprite_y >= kMiniGameHeight)
      continue;
    for (int ty = 0; ty < tiles_per_side; ty++) {
      for (int tx = 0; tx < tiles_per_side; tx++) {
        int tile_x = x_flip ? (tiles_per_side - 1 - tx) : tx;
        int tile_y = y_flip ? (tiles_per_side - 1 - ty) : ty;
        int tile_addr = obj_tile_base + MiniObjTileOffset(charnum, tile_x, tile_y);
        MiniRenderObjTile(pixels, pitch_pixels, vram, tile_addr, palette_base,
                          x_flip, y_flip, sprite_x + tx * 8, sprite_y + ty * 8);
      }
    }
  }
}

static void MiniRenderFrameToPixels(uint32_t *pixels, int pitch_pixels, const MiniGameState *state) {
  MiniRenderRoom(pixels, pitch_pixels, state);
  if (state->uses_rom_room)
    MiniRenderRoomSprites(pixels, pitch_pixels);
  else
    MiniRenderEditorRoomSprites(pixels, pitch_pixels, state);
  MiniRenderSamus(pixels, pitch_pixels);
}

static bool MiniSaveScreenshot(const char *path, const MiniGameState *state) {
  uint32_t pixels[kMiniGameWidth * kMiniGameHeight];
  MiniRenderFrameToPixels(pixels, kMiniGameWidth, state);
  SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormatFrom(
      pixels, kMiniGameWidth, kMiniGameHeight, 32,
      kMiniGameWidth * (int)sizeof(uint32_t), SDL_PIXELFORMAT_ARGB8888);
  if (surface == NULL) {
    fprintf(stderr, "SDL_CreateRGBSurfaceWithFormatFrom failed: %s\n", SDL_GetError());
    return false;
  }
  bool ok = SDL_SaveBMP(surface, path) == 0;
  if (!ok)
    fprintf(stderr, "SDL_SaveBMP failed: %s\n", SDL_GetError());
  SDL_FreeSurface(surface);
  return ok;
}

static void RenderFrame(SDL_Renderer *renderer, SDL_Texture *frame_texture, const MiniGameState *state) {
  void *pixels_void = NULL;
  int pitch_bytes = 0;
  if (SDL_LockTexture(frame_texture, NULL, &pixels_void, &pitch_bytes) != 0)
    return;
  uint32_t *pixels = (uint32_t *)pixels_void;
  int pitch_pixels = pitch_bytes / (int)sizeof(uint32_t);
  MiniRenderFrameToPixels(pixels, pitch_pixels, state);
  SDL_UnlockTexture(frame_texture);

  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, frame_texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

static void RunFrames(MiniGameState *state, const MiniOptions *options, SDL_Renderer *renderer,
                      SDL_Texture *frame_texture, SDL_GameController *controller) {
  MiniInputScript script = {0};
  if (options->input_script_path != NULL && !MiniLoadInputScript(options->input_script_path, &script))
    return;

  int frame_limit = (options->headless || options->frames_explicit) ? options->frames : INT_MAX;
  for (int i = 0; i < frame_limit && !state->quit_requested; i++) {
    MiniInputState input = {0};

    if (i < script.count) {
      input.buttons = script.frames[i].buttons;
      input.quit_requested = script.frames[i].quit_requested;
    }

    if (renderer != NULL && options->input_script_path == NULL) {
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
          input.quit_requested = true;
      }
      SDL_PumpEvents();
      const Uint8 *keys = SDL_GetKeyboardState(NULL);
      if (keys[SDL_SCANCODE_UP])
        input.buttons |= kButton_Up;
      if (keys[SDL_SCANCODE_DOWN])
        input.buttons |= kButton_Down;
      if (keys[SDL_SCANCODE_LEFT])
        input.buttons |= kButton_Left;
      if (keys[SDL_SCANCODE_RIGHT])
        input.buttons |= kButton_Right;
      if (keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_Z])
        input.buttons |= kButton_A;
      if (keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT] || keys[SDL_SCANCODE_C])
        input.buttons |= kButton_B;
      if (keys[SDL_SCANCODE_X])
        input.buttons |= kButton_X;
      if (keys[SDL_SCANCODE_A])
        input.buttons |= kButton_Y;
      if (keys[SDL_SCANCODE_S])
        input.buttons |= kButton_L;
      if (keys[SDL_SCANCODE_D])
        input.buttons |= kButton_R;
      if (controller != NULL && SDL_GameControllerGetAttached(controller)) {
        Sint16 axis = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
        Sint16 axis_y = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
        if (axis_y <= -16000 || SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP))
          input.buttons |= kButton_Up;
        if (axis_y >= 16000 || SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN))
          input.buttons |= kButton_Down;
        if (axis <= -16000 || SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT))
          input.buttons |= kButton_Left;
        if (axis >= 16000 || SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
          input.buttons |= kButton_Right;
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A))
          input.buttons |= kButton_A;
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B))
          input.buttons |= kButton_B;
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X))
          input.buttons |= kButton_X;
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_Y))
          input.buttons |= kButton_Y;
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER))
          input.buttons |= kButton_L;
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER))
          input.buttons |= kButton_R;
      }
    }

    MiniUpdate(state, &input);

    if (renderer != NULL && frame_texture != NULL) {
      RenderFrame(renderer, frame_texture, state);
      SDL_Delay(kMiniFrameDelayMs);
    }
  }

  MiniFreeInputScript(&script);
}

static int RunHeadless(const MiniOptions *options) {
  MiniGameState state;
  MiniStubs_SetRoomExportPath(options->room_export_path);
  MiniGameState_Init(&state, kMiniGameWidth, kMiniGameHeight);
  RunFrames(&state, options, NULL, NULL, NULL);
  if (options->screenshot_path != NULL && !MiniSaveScreenshot(options->screenshot_path, &state))
    return 1;
  PrintResult(options, &state);
  return 0;
}

static int RunWindowed(const MiniOptions *options) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
    fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Window *window = SDL_CreateWindow("sm_rev mini shell",
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        kMiniWindowWidth,
                                        kMiniWindowHeight,
                                        SDL_WINDOW_SHOWN);
  if (window == NULL) {
    fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == NULL) {
    fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }
  SDL_RenderSetLogicalSize(renderer, kMiniGameWidth, kMiniGameHeight);
  SDL_Texture *frame_texture = SDL_CreateTexture(renderer,
                                                 SDL_PIXELFORMAT_ARGB8888,
                                                 SDL_TEXTUREACCESS_STREAMING,
                                                 kMiniGameWidth,
                                                 kMiniGameHeight);
  if (frame_texture == NULL) {
    fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  SDL_GameController *controller = NULL;
  for (int i = 0; i < SDL_NumJoysticks(); i++) {
    if (SDL_IsGameController(i)) {
      controller = SDL_GameControllerOpen(i);
      if (controller != NULL)
        break;
    }
  }

  MiniGameState state;
  MiniStubs_SetRoomExportPath(options->room_export_path);
  MiniGameState_Init(&state, kMiniGameWidth, kMiniGameHeight);
  RunFrames(&state, options, renderer, frame_texture, controller);
  if (options->screenshot_path != NULL && !MiniSaveScreenshot(options->screenshot_path, &state)) {
    if (controller != NULL)
      SDL_GameControllerClose(controller);
    SDL_DestroyTexture(frame_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  if (controller != NULL)
    SDL_GameControllerClose(controller);
  SDL_DestroyTexture(frame_texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  PrintResult(options, &state);
  return 0;
}

int MiniRun(const MiniOptions *options) {
  return options->headless ? RunHeadless(options) : RunWindowed(options);
}
