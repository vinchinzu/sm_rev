#include "mini_editor_bridge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "third_party/cJSON.h"

enum {
  kMiniLandingSiteCameraX = 1024,
  kMiniLandingSiteCameraY = 976,
  kMiniLandingSiteSamusX = 1153,
  kMiniLandingSiteSamusY = 1088,
};

static const char *g_room_export_path;
static char g_resolved_room_export_path[512];

static const char *const kDefaultRoomExportCandidates[] = {
  "assets/mini/landing_site.room.json",
  "assets/local_mini/room_91F8.json",
  "../super_metroid_editor/export/sm_nav/rooms/room_91F8.json",
};

static void MiniEditorRoom_Reset(MiniEditorRoom *room) {
  memset(room, 0, sizeof(*room));
}

static bool MiniReadFile(const char *path, char **out_data) {
  *out_data = NULL;
  FILE *f = fopen(path, "rb");
  if (f == NULL)
    return false;
  if (fseek(f, 0, SEEK_END) != 0) {
    fclose(f);
    return false;
  }
  long size = ftell(f);
  if (size <= 0) {
    fclose(f);
    return false;
  }
  rewind(f);
  char *data = (char *)malloc((size_t)size + 1);
  if (data == NULL) {
    fclose(f);
    return false;
  }
  bool ok = fread(data, 1, (size_t)size, f) == (size_t)size;
  fclose(f);
  if (!ok) {
    free(data);
    return false;
  }
  data[size] = '\0';
  *out_data = data;
  return true;
}

static bool MiniReadExactFile(const char *path, void *dst, size_t size) {
  FILE *f = fopen(path, "rb");
  if (f == NULL)
    return false;
  bool ok = fread(dst, 1, size, f) == size;
  if (ok) {
    unsigned char extra_byte;
    ok = fread(&extra_byte, 1, 1, f) == 0;
  }
  fclose(f);
  return ok;
}

static bool MiniReadWholeBinaryFile(const char *path, uint8 **out_data, size_t *out_size) {
  *out_data = NULL;
  *out_size = 0;
  FILE *f = fopen(path, "rb");
  if (f == NULL)
    return false;
  if (fseek(f, 0, SEEK_END) != 0) {
    fclose(f);
    return false;
  }
  long size = ftell(f);
  if (size <= 0) {
    fclose(f);
    return false;
  }
  rewind(f);
  uint8 *data = (uint8 *)malloc((size_t)size);
  if (data == NULL) {
    fclose(f);
    return false;
  }
  bool ok = fread(data, 1, (size_t)size, f) == (size_t)size;
  fclose(f);
  if (!ok) {
    free(data);
    return false;
  }
  *out_data = data;
  *out_size = (size_t)size;
  return true;
}

static bool MiniCopyJsonString(cJSON *node, char *dst, size_t dst_size) {
  if (!cJSON_IsString(node) || node->valuestring == NULL || dst_size == 0)
    return false;
  snprintf(dst, dst_size, "%s", node->valuestring);
  return true;
}

static bool MiniJsonStringEquals(cJSON *node, const char *value) {
  return cJSON_IsString(node) && node->valuestring != NULL && strcmp(node->valuestring, value) == 0;
}

static bool MiniGetJsonInt(cJSON *node, int *value) {
  if (!cJSON_IsNumber(node))
    return false;
  *value = node->valueint;
  return true;
}

static bool MiniParseGridRow(cJSON *row, int width, uint8 *dst) {
  if (!cJSON_IsArray(row) || cJSON_GetArraySize(row) != width)
    return false;
  for (int x = 0; x < width; x++) {
    cJSON *value = cJSON_GetArrayItem(row, x);
    if (!cJSON_IsNumber(value))
      return false;
    dst[x] = (uint8)value->valueint;
  }
  return true;
}

static bool MiniParseGrid(cJSON *grid, int width, int height, uint8 *dst) {
  if (!cJSON_IsArray(grid) || cJSON_GetArraySize(grid) != height)
    return false;
  for (int y = 0; y < height; y++) {
    if (!MiniParseGridRow(cJSON_GetArrayItem(grid, y), width, dst + y * width))
      return false;
  }
  return true;
}

static bool MiniParseWordGridRow(cJSON *row, int width, uint16 *dst) {
  if (!cJSON_IsArray(row) || cJSON_GetArraySize(row) != width)
    return false;
  for (int x = 0; x < width; x++) {
    cJSON *value = cJSON_GetArrayItem(row, x);
    if (!cJSON_IsNumber(value) || value->valueint < 0 || value->valueint > 0xFFFF)
      return false;
    dst[x] = (uint16)value->valueint;
  }
  return true;
}

static bool MiniParseWordGrid(cJSON *grid, int width, int height, uint16 *dst) {
  if (!cJSON_IsArray(grid) || cJSON_GetArraySize(grid) != height)
    return false;
  for (int y = 0; y < height; y++) {
    if (!MiniParseWordGridRow(cJSON_GetArrayItem(grid, y), width, dst + y * width))
      return false;
  }
  return true;
}

static bool MiniParseCamera(cJSON *camera, MiniEditorRoom *room) {
  if (!cJSON_IsObject(camera))
    return false;
  return MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(camera, "cameraX"), &room->camera_x) &&
         MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(camera, "cameraY"), &room->camera_y) &&
         MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(camera, "spawnX"), &room->spawn_x) &&
         MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(camera, "spawnY"), &room->spawn_y);
}

static bool MiniResolveRelativePath(const char *base_path, const char *asset_path,
                                    char *dst, size_t dst_size) {
  if (asset_path == NULL || asset_path[0] == '\0' || dst_size == 0)
    return false;
  if (asset_path[0] == '/') {
    snprintf(dst, dst_size, "%s", asset_path);
    return true;
  }
  const char *slash = strrchr(base_path, '/');
  if (slash == NULL) {
    snprintf(dst, dst_size, "%s", asset_path);
    return true;
  }
  size_t base_len = (size_t)(slash - base_path);
  snprintf(dst, dst_size, "%.*s/%s", (int)base_len, base_path, asset_path);
  return true;
}

static bool MiniLoadWordFile(const char *path, uint16 *dst, size_t count) {
  uint8 *bytes = (uint8 *)malloc(count * 2);
  if (bytes == NULL)
    return false;
  bool ok = MiniReadExactFile(path, bytes, count * 2);
  if (ok) {
    for (size_t i = 0; i < count; i++)
      dst[i] = (uint16)(bytes[i * 2] | (bytes[i * 2 + 1] << 8));
  }
  free(bytes);
  return ok;
}

static MiniEditorSamusSuit MiniParseSamusSuit(cJSON *node) {
  if (MiniJsonStringEquals(node, "gravity"))
    return kMiniEditorSamusSuit_Gravity;
  if (MiniJsonStringEquals(node, "varia"))
    return kMiniEditorSamusSuit_Varia;
  return kMiniEditorSamusSuit_Power;
}

static bool MiniMaybeLoadTilesetAssets(const char *room_path, cJSON *tileset_meta, MiniEditorRoom *room) {
  cJSON *assets = cJSON_GetObjectItemCaseSensitive(tileset_meta, "assets");
  if (!cJSON_IsObject(assets))
    return true;

  char tiles_path[512], metatile_path[512], palette_path[512];
  if (!MiniCopyJsonString(cJSON_GetObjectItemCaseSensitive(assets, "tiles4bppPath"), tiles_path, sizeof(tiles_path)) ||
      !MiniCopyJsonString(cJSON_GetObjectItemCaseSensitive(assets, "metatileWordsPath"), metatile_path, sizeof(metatile_path)) ||
      !MiniCopyJsonString(cJSON_GetObjectItemCaseSensitive(assets, "palettePath"), palette_path, sizeof(palette_path))) {
    return false;
  }

  char resolved_tiles[512], resolved_metatiles[512], resolved_palette[512];
  if (!MiniResolveRelativePath(room_path, tiles_path, resolved_tiles, sizeof(resolved_tiles)) ||
      !MiniResolveRelativePath(room_path, metatile_path, resolved_metatiles, sizeof(resolved_metatiles)) ||
      !MiniResolveRelativePath(room_path, palette_path, resolved_palette, sizeof(resolved_palette))) {
    return false;
  }

  room->tiles4bpp = (uint8 *)malloc(kMiniEditorBridgeTiles4bppSize);
  room->metatile_words = (uint16 *)malloc(sizeof(uint16) * kMiniEditorBridgeMetatileWordCount);
  room->palette = (uint16 *)malloc(sizeof(uint16) * kMiniEditorBridgePaletteCount);
  if (room->tiles4bpp == NULL || room->metatile_words == NULL || room->palette == NULL)
    return false;

  if (!MiniReadExactFile(resolved_tiles, room->tiles4bpp, kMiniEditorBridgeTiles4bppSize) ||
      !MiniLoadWordFile(resolved_metatiles, room->metatile_words, kMiniEditorBridgeMetatileWordCount) ||
      !MiniLoadWordFile(resolved_palette, room->palette, kMiniEditorBridgePaletteCount)) {
    return false;
  }

  room->has_tileset_assets = true;
  return true;
}

static bool MiniMaybeLoadBackgroundAssets(const char *room_path, cJSON *background_assets,
                                         MiniEditorRoom *room) {
  if (!cJSON_IsObject(background_assets))
    return true;

  cJSON *variants = cJSON_GetObjectItemCaseSensitive(background_assets, "variants");
  if (!cJSON_IsArray(variants) || cJSON_GetArraySize(variants) <= 0)
    return true;

  const char *preferred_key = NULL;
  cJSON *default_variant_key = cJSON_GetObjectItemCaseSensitive(background_assets, "defaultVariantKey");
  if (cJSON_IsString(default_variant_key) && default_variant_key->valuestring != NULL)
    preferred_key = default_variant_key->valuestring;

  cJSON *selected = NULL;
  int variant_count = cJSON_GetArraySize(variants);
  for (int i = 0; i < variant_count; i++) {
    cJSON *variant = cJSON_GetArrayItem(variants, i);
    cJSON *key = cJSON_GetObjectItemCaseSensitive(variant, "key");
    if (preferred_key != NULL && MiniJsonStringEquals(key, preferred_key)) {
      selected = variant;
      break;
    }
    if (selected == NULL)
      selected = variant;
  }
  if (!cJSON_IsObject(selected))
    return true;

  char variant_key[sizeof(room->bg2_variant_key)];
  char tilemap_path[512];
  if (!MiniCopyJsonString(cJSON_GetObjectItemCaseSensitive(selected, "key"),
                          variant_key, sizeof(variant_key)) ||
      !MiniCopyJsonString(cJSON_GetObjectItemCaseSensitive(selected, "tilemapWordsPath"),
                          tilemap_path, sizeof(tilemap_path))) {
    return false;
  }

  char resolved_tilemap[512];
  if (!MiniResolveRelativePath(room_path, tilemap_path, resolved_tilemap, sizeof(resolved_tilemap)))
    return false;

  room->bg2_tilemap_words = (uint16 *)malloc(sizeof(uint16) * kMiniEditorBridgeBg2TilemapWordCount);
  if (room->bg2_tilemap_words == NULL)
    return false;
  if (!MiniLoadWordFile(resolved_tilemap, room->bg2_tilemap_words, kMiniEditorBridgeBg2TilemapWordCount))
    return false;

  snprintf(room->bg2_variant_key, sizeof(room->bg2_variant_key), "%s", variant_key);
  room->has_bg2_assets = true;
  return true;
}

static bool MiniMaybeLoadSamusPaletteAssets(const char *room_path, cJSON *samus_assets, MiniEditorRoom *room) {
  cJSON *palette_assets = cJSON_GetObjectItemCaseSensitive(samus_assets, "paletteAssets");
  room->initial_suit = MiniParseSamusSuit(cJSON_GetObjectItemCaseSensitive(samus_assets, "initialSuit"));
  if (!cJSON_IsObject(palette_assets))
    return true;

  char power_path[512], varia_path[512], gravity_path[512];
  if (!MiniCopyJsonString(cJSON_GetObjectItemCaseSensitive(palette_assets, "powerPath"),
                          power_path, sizeof(power_path)) ||
      !MiniCopyJsonString(cJSON_GetObjectItemCaseSensitive(palette_assets, "variaPath"),
                          varia_path, sizeof(varia_path)) ||
      !MiniCopyJsonString(cJSON_GetObjectItemCaseSensitive(palette_assets, "gravityPath"),
                          gravity_path, sizeof(gravity_path))) {
    return false;
  }

  char resolved_power[512], resolved_varia[512], resolved_gravity[512];
  if (!MiniResolveRelativePath(room_path, power_path, resolved_power, sizeof(resolved_power)) ||
      !MiniResolveRelativePath(room_path, varia_path, resolved_varia, sizeof(resolved_varia)) ||
      !MiniResolveRelativePath(room_path, gravity_path, resolved_gravity, sizeof(resolved_gravity))) {
    return false;
  }

  if (!MiniLoadWordFile(resolved_power, room->samus_palette_power, kMiniEditorBridgeSamusPaletteCount) ||
      !MiniLoadWordFile(resolved_varia, room->samus_palette_varia, kMiniEditorBridgeSamusPaletteCount) ||
      !MiniLoadWordFile(resolved_gravity, room->samus_palette_gravity, kMiniEditorBridgeSamusPaletteCount)) {
    return false;
  }

  room->has_samus_palette_assets = true;
  return true;
}

static bool MiniMaybeLoadSamusAssets(const char *room_path, cJSON *samus_assets, MiniEditorRoom *room) {
  if (!cJSON_IsObject(samus_assets))
    return true;
  if (!MiniMaybeLoadSamusPaletteAssets(room_path, samus_assets, room))
    return false;

  char bank92_path[512], data_path[512];
  if (!MiniCopyJsonString(cJSON_GetObjectItemCaseSensitive(samus_assets, "bank92Path"),
                          bank92_path, sizeof(bank92_path)) ||
      !MiniCopyJsonString(cJSON_GetObjectItemCaseSensitive(samus_assets, "dataPath"),
                          data_path, sizeof(data_path))) {
    return false;
  }

  cJSON *ranges = cJSON_GetObjectItemCaseSensitive(samus_assets, "ranges");
  if (!cJSON_IsArray(ranges))
    return false;
  int range_count = cJSON_GetArraySize(ranges);
  if (range_count <= 0)
    return false;

  char resolved_bank92[512], resolved_data[512];
  if (!MiniResolveRelativePath(room_path, bank92_path, resolved_bank92, sizeof(resolved_bank92)) ||
      !MiniResolveRelativePath(room_path, data_path, resolved_data, sizeof(resolved_data))) {
    return false;
  }

  room->samus_bank92 = (uint8 *)malloc(kMiniEditorBridgeSamusBank92Size);
  room->samus_ranges = (MiniEditorSamusAssetRange *)calloc((size_t)range_count, sizeof(*room->samus_ranges));
  if (room->samus_bank92 == NULL || room->samus_ranges == NULL)
    return false;
  if (!MiniReadExactFile(resolved_bank92, room->samus_bank92, kMiniEditorBridgeSamusBank92Size))
    return false;
  if (!MiniReadWholeBinaryFile(resolved_data, &room->samus_data, &room->samus_data_size))
    return false;

  for (int i = 0; i < range_count; i++) {
    cJSON *range = cJSON_GetArrayItem(ranges, i);
    int snes_address, size, data_offset;
    if (!cJSON_IsObject(range) ||
        !MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(range, "snesAddress"), &snes_address) ||
        !MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(range, "size"), &size) ||
        !MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(range, "dataOffset"), &data_offset) ||
        snes_address < 0 || size <= 0 || data_offset < 0) {
      return false;
    }
    room->samus_ranges[i] = (MiniEditorSamusAssetRange){
      .snes_address = (uint32)snes_address,
      .size = (uint32)size,
      .data_offset = (uint32)data_offset,
    };
    if ((size_t)data_offset + (size_t)size > room->samus_data_size)
      return false;
  }

  room->samus_range_count = range_count;
  room->has_samus_assets = true;
  return true;
}

static bool MiniParseRoomSpriteOamEntries(cJSON *entries, MiniEditorRoomSprite *sprite) {
  if (!cJSON_IsArray(entries))
    return false;
  int count = cJSON_GetArraySize(entries);
  if (count <= 0)
    return false;
  sprite->entries = (MiniEditorRoomSpriteOamEntry *)calloc((size_t)count, sizeof(*sprite->entries));
  if (sprite->entries == NULL)
    return false;
  sprite->entry_count = count;
  for (int i = 0; i < count; i++) {
    cJSON *entry = cJSON_GetArrayItem(entries, i);
    int x_offset, y_offset, tile_num, palette_row;
    cJSON *h_flip = cJSON_GetObjectItemCaseSensitive(entry, "hFlip");
    cJSON *v_flip = cJSON_GetObjectItemCaseSensitive(entry, "vFlip");
    cJSON *is_16x16 = cJSON_GetObjectItemCaseSensitive(entry, "is16x16");
    if (!cJSON_IsObject(entry) ||
        !MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(entry, "xOffset"), &x_offset) ||
        !MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(entry, "yOffset"), &y_offset) ||
        !MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(entry, "tileNum"), &tile_num) ||
        !MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(entry, "paletteRow"), &palette_row) ||
        !cJSON_IsBool(h_flip) ||
        !cJSON_IsBool(v_flip) ||
        !cJSON_IsBool(is_16x16)) {
      return false;
    }
    sprite->entries[i] = (MiniEditorRoomSpriteOamEntry){
      .x_offset = (int16)x_offset,
      .y_offset = (int16)y_offset,
      .tile_num = (uint16)tile_num,
      .palette_row = (uint8)palette_row,
      .h_flip = cJSON_IsTrue(h_flip),
      .v_flip = cJSON_IsTrue(v_flip),
      .is_16x16 = cJSON_IsTrue(is_16x16),
    };
  }
  return true;
}

static bool MiniMaybeLoadRoomSprites(const char *room_path, cJSON *room_sprites, MiniEditorRoom *room) {
  if (!cJSON_IsArray(room_sprites))
    return true;
  int sprite_count = cJSON_GetArraySize(room_sprites);
  if (sprite_count <= 0)
    return true;
  room->room_sprites = (MiniEditorRoomSprite *)calloc((size_t)sprite_count, sizeof(*room->room_sprites));
  if (room->room_sprites == NULL)
    return false;
  room->room_sprite_count = sprite_count;
  for (int i = 0; i < sprite_count; i++) {
    cJSON *sprite_node = cJSON_GetArrayItem(room_sprites, i);
    MiniEditorRoomSprite *sprite = &room->room_sprites[i];
    int species_id, pixel_x, pixel_y;
    char tile_path[512], palette_path[512];
    if (!cJSON_IsObject(sprite_node) ||
        !MiniCopyJsonString(cJSON_GetObjectItemCaseSensitive(sprite_node, "key"),
                            sprite->key, sizeof(sprite->key)) ||
        !MiniCopyJsonString(cJSON_GetObjectItemCaseSensitive(sprite_node, "label"),
                            sprite->label, sizeof(sprite->label)) ||
        !MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(sprite_node, "speciesId"), &species_id) ||
        !MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(sprite_node, "pixelX"), &pixel_x) ||
        !MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(sprite_node, "pixelY"), &pixel_y) ||
        !MiniCopyJsonString(cJSON_GetObjectItemCaseSensitive(sprite_node, "tileDataPath"),
                            tile_path, sizeof(tile_path)) ||
        !MiniCopyJsonString(cJSON_GetObjectItemCaseSensitive(sprite_node, "palettePath"),
                            palette_path, sizeof(palette_path)) ||
        !MiniParseRoomSpriteOamEntries(cJSON_GetObjectItemCaseSensitive(sprite_node, "entries"), sprite)) {
      return false;
    }
    sprite->species_id = (uint16)species_id;
    sprite->x_pos = pixel_x;
    sprite->y_pos = pixel_y;

    char resolved_tiles[512], resolved_palette[512];
    if (!MiniResolveRelativePath(room_path, tile_path, resolved_tiles, sizeof(resolved_tiles)) ||
        !MiniResolveRelativePath(room_path, palette_path, resolved_palette, sizeof(resolved_palette))) {
      return false;
    }
    if (!MiniReadWholeBinaryFile(resolved_tiles, &sprite->tile_data, &sprite->tile_data_size) ||
        !MiniLoadWordFile(resolved_palette, sprite->palette, kMiniEditorBridgeRoomSpritePaletteCount)) {
      return false;
    }
  }
  return true;
}

static bool MiniParseScroll(cJSON *scroll, MiniEditorRoom *room) {
  if (!cJSON_IsObject(scroll))
    return false;
  size_t screen_count = (size_t)room->width_screens * room->height_screens;
  room->scroll_values = (uint8 *)calloc(screen_count, sizeof(uint8));
  if (room->scroll_values == NULL)
    return false;
  if (!MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(scroll, "upScroller"), &room->export_up_scroller) ||
      !MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(scroll, "downScroller"), &room->export_down_scroller) ||
      !MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(scroll, "bgScrolling"), &room->export_bg_scrolling) ||
      !MiniParseGrid(cJSON_GetObjectItemCaseSensitive(scroll, "screens"),
                     room->width_screens, room->height_screens, room->scroll_values)) {
    return false;
  }
  return true;
}

static void MiniAssignLandingSiteDefaults(MiniEditorRoom *room) {
  if (room->room_id != kMiniEditorBridgeRoomId_LandingSite && strcmp(room->handle, "landingSite") != 0)
    return;
  room->camera_x = kMiniLandingSiteCameraX;
  room->camera_y = kMiniLandingSiteCameraY;
  room->spawn_x = kMiniLandingSiteSamusX;
  room->spawn_y = kMiniLandingSiteSamusY;
}

static bool MiniParseRoomJson(const char *path, MiniEditorRoom *room) {
  char *file_data = NULL;
  if (!MiniReadFile(path, &file_data))
    return false;

  cJSON *root = cJSON_Parse(file_data);
  free(file_data);
  if (root == NULL)
    return false;

  MiniEditorRoom_Reset(room);
  int room_id, width_blocks, height_blocks, width_screens = 0, height_screens = 0;
  bool ok =
      MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(root, "roomId"), &room_id) &&
      MiniCopyJsonString(cJSON_GetObjectItemCaseSensitive(root, "handle"), room->handle, sizeof(room->handle)) &&
      MiniCopyJsonString(cJSON_GetObjectItemCaseSensitive(root, "name"), room->name, sizeof(room->name)) &&
      MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(root, "widthBlocks"), &width_blocks) &&
      MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(root, "heightBlocks"), &height_blocks);
  if (!ok || room_id < 0 || room_id > 0xFFFF || width_blocks <= 0 || height_blocks <= 0) {
    cJSON_Delete(root);
    return false;
  }
  if ((width_blocks & 15) != 0 || (height_blocks & 15) != 0) {
    cJSON_Delete(root);
    return false;
  }
  MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(root, "widthScreens"), &width_screens);
  MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(root, "heightScreens"), &height_screens);
  if (width_screens == 0)
    width_screens = width_blocks / 16;
  if (height_screens == 0)
    height_screens = height_blocks / 16;
  if (width_screens != width_blocks / 16 || height_screens != height_blocks / 16) {
    cJSON_Delete(root);
    return false;
  }

  size_t block_count = (size_t)width_blocks * height_blocks;
  size_t screen_count = (size_t)width_screens * height_screens;
  room->block_words = (uint16 *)calloc(block_count, sizeof(uint16));
  room->collision_types = (uint8 *)calloc(block_count, sizeof(uint8));
  room->bts = (uint8 *)calloc(block_count, sizeof(uint8));
  if (room->block_words == NULL || room->collision_types == NULL || room->bts == NULL) {
    cJSON_Delete(root);
    MiniEditorBridge_FreeRoom(room);
    return false;
  }

  room->room_id = (uint16)room_id;
  room->width_screens = width_screens;
  room->height_screens = height_screens;
  room->width_blocks = width_blocks;
  room->height_blocks = height_blocks;
  room->tileset = -1;
  room->cre_bitflag = 0;
  room->export_up_scroller = 112;
  room->export_down_scroller = 160;
  room->export_bg_scrolling = 0;
  room->initial_suit = kMiniEditorSamusSuit_Power;

  cJSON *tileset_meta = cJSON_GetObjectItemCaseSensitive(root, "tilesetMeta");
  cJSON *background_assets = cJSON_GetObjectItemCaseSensitive(root, "backgroundAssets");
  cJSON *samus_assets = cJSON_GetObjectItemCaseSensitive(root, "samusAssets");
  cJSON *scroll = cJSON_GetObjectItemCaseSensitive(root, "scroll");
  cJSON *camera = cJSON_GetObjectItemCaseSensitive(root, "camera");
  cJSON *block_words = cJSON_GetObjectItemCaseSensitive(root, "blockWords");
  cJSON *room_sprites = cJSON_GetObjectItemCaseSensitive(root, "roomSprites");

  MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(root, "tileset"), &room->tileset);
  if (cJSON_IsObject(tileset_meta)) {
    MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(tileset_meta, "creBitflag"), &room->cre_bitflag);
    if (!MiniMaybeLoadTilesetAssets(path, tileset_meta, room))
      fprintf(stderr, "mini: failed to load editor tileset assets for %s; using placeholder renderer\n", path);
  }
  if (cJSON_IsObject(background_assets)) {
    if (!MiniMaybeLoadBackgroundAssets(path, background_assets, room))
      fprintf(stderr, "mini: failed to load editor background assets for %s; using solid sky fallback\n", path);
  }
  if (cJSON_IsObject(samus_assets)) {
    if (!MiniMaybeLoadSamusAssets(path, samus_assets, room))
      fprintf(stderr, "mini: failed to load editor samus assets for %s; falling back to ROM-backed Samus assets\n", path);
  }
  if (cJSON_IsArray(room_sprites)) {
    if (!MiniMaybeLoadRoomSprites(path, room_sprites, room))
      fprintf(stderr, "mini: failed to load editor room sprite assets for %s; continuing without room sprites\n", path);
  }

  ok = MiniParseGrid(cJSON_GetObjectItemCaseSensitive(root, "collision"), width_blocks, height_blocks, room->collision_types) &&
       MiniParseGrid(cJSON_GetObjectItemCaseSensitive(root, "bts"), width_blocks, height_blocks, room->bts);
  if (ok && cJSON_IsArray(block_words))
    ok = MiniParseWordGrid(block_words, width_blocks, height_blocks, room->block_words);
  if (ok && cJSON_IsObject(camera))
    ok = MiniParseCamera(camera, room);
  if (ok && cJSON_IsObject(scroll))
    ok = MiniParseScroll(scroll, room);
  if (ok && room->scroll_values != NULL && screen_count > 256)
    ok = false;
  if (ok && !cJSON_IsArray(block_words)) {
    for (size_t i = 0; i < block_count; i++)
      room->block_words[i] = (uint16)room->collision_types[i] << 12;
  }
  if (ok && !cJSON_IsObject(camera))
    MiniAssignLandingSiteDefaults(room);
  cJSON_Delete(root);
  if (!ok) {
    MiniEditorBridge_FreeRoom(room);
    return false;
  }

  snprintf(g_resolved_room_export_path, sizeof(g_resolved_room_export_path), "%s", path);
  return true;
}

void MiniEditorBridge_SetRoomExportPath(const char *path) {
  g_room_export_path = (path != NULL && path[0] != '\0') ? path : NULL;
  g_resolved_room_export_path[0] = '\0';
}

const char *MiniEditorBridge_GetResolvedPath(void) {
  return g_resolved_room_export_path[0] != '\0' ? g_resolved_room_export_path : NULL;
}

bool MiniEditorBridge_LoadRoom(MiniEditorRoom *room) {
  MiniEditorRoom_Reset(room);
  if (g_room_export_path != NULL)
    return MiniParseRoomJson(g_room_export_path, room);

  for (size_t i = 0; i < sizeof(kDefaultRoomExportCandidates) / sizeof(kDefaultRoomExportCandidates[0]); i++) {
    if (MiniParseRoomJson(kDefaultRoomExportCandidates[i], room))
      return true;
  }
  return false;
}

void MiniEditorBridge_FreeRoom(MiniEditorRoom *room) {
  if (room->room_sprites != NULL) {
    for (int i = 0; i < room->room_sprite_count; i++) {
      free(room->room_sprites[i].tile_data);
      free(room->room_sprites[i].entries);
    }
  }
  free(room->block_words);
  free(room->collision_types);
  free(room->bts);
  free(room->scroll_values);
  free(room->tiles4bpp);
  free(room->metatile_words);
  free(room->palette);
  free(room->bg2_tilemap_words);
  free(room->samus_bank92);
  free(room->samus_data);
  free(room->samus_ranges);
  free(room->room_sprites);
  MiniEditorRoom_Reset(room);
}
