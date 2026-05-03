#include "mini_editor_bridge.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "third_party/cJSON.h"

#include "block_reaction.h"

enum {
  kMiniLandingSiteCameraX = 1024,
  kMiniLandingSiteCameraY = 976,
  kMiniLandingSiteSamusX = 1153,
  kMiniLandingSiteSamusY = 1088,
};

static const char *g_room_export_path;
static char g_resolved_room_export_path[512];
static char g_base_path[512];
static char g_git_common_base_path[512];

static const char *const kDefaultRoomExportCandidates[] = {
  "assets/mini/landing_site.room.json",
  "assets/local_mini/room_91F8.json",
  "../super_metroid_editor/export/sm_nav/rooms/room_91F8.json",
};

static void MiniEditorRoom_Reset(MiniEditorRoom *room) {
  memset(room, 0, sizeof(*room));
}

static bool MiniPathExists(const char *path) {
  FILE *f = fopen(path, "rb");
  if (f == NULL)
    return false;
  fclose(f);
  return true;
}

static bool MiniPathIsAbsolute(const char *path) {
  return path != NULL && path[0] == '/';
}

static void MiniTrimTrailingSlashes(char *path) {
  size_t len = strlen(path);
  while (len > 1 && path[len - 1] == '/') {
    path[len - 1] = '\0';
    len--;
  }
}

static void MiniJoinPath(char *dst, size_t dst_size, const char *base, const char *path) {
  if (dst_size == 0)
    return;
  if (path == NULL || path[0] == '\0') {
    dst[0] = '\0';
  } else if (MiniPathIsAbsolute(path) || base == NULL || base[0] == '\0') {
    snprintf(dst, dst_size, "%s", path);
  } else {
    snprintf(dst, dst_size, "%s/%s", base, path);
  }
}

static void MiniStripLineEnd(char *line) {
  size_t len = strlen(line);
  while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
    line[len - 1] = '\0';
    len--;
  }
}

static bool MiniReadFirstLine(const char *path, char *dst, size_t dst_size) {
  if (dst_size == 0)
    return false;
  dst[0] = '\0';
  FILE *f = fopen(path, "r");
  if (f == NULL)
    return false;
  bool ok = fgets(dst, (int)dst_size, f) != NULL;
  fclose(f);
  if (ok)
    MiniStripLineEnd(dst);
  return ok;
}

static void MiniCopyDirname(char *dst, size_t dst_size, const char *path) {
  const char *slash = strrchr(path, '/');
  if (slash == NULL) {
    snprintf(dst, dst_size, ".");
    return;
  }
  snprintf(dst, dst_size, "%.*s", (int)(slash - path), path);
}

static const char *MiniSkipGitdirPrefix(const char *line) {
  static const char kGitdirPrefix[] = "gitdir:";
  size_t prefix_len = sizeof(kGitdirPrefix) - 1;
  if (strncmp(line, kGitdirPrefix, prefix_len) != 0)
    return NULL;
  line += prefix_len;
  while (*line == ' ' || *line == '\t')
    line++;
  return *line != '\0' ? line : NULL;
}

static bool MiniReadCheckoutGitDir(const char *checkout_path, char *dst, size_t dst_size) {
  char dot_git[512];
  char dot_git_head[512];
  char git_file_line[512];
  MiniJoinPath(dot_git, sizeof(dot_git), checkout_path, ".git");
  MiniJoinPath(dot_git_head, sizeof(dot_git_head), dot_git, "HEAD");
  if (MiniPathExists(dot_git_head)) {
    snprintf(dst, dst_size, "%s", dot_git);
    return true;
  }

  if (!MiniReadFirstLine(dot_git, git_file_line, sizeof(git_file_line)))
    return false;
  const char *gitdir = MiniSkipGitdirPrefix(git_file_line);
  if (gitdir == NULL)
    return false;
  MiniJoinPath(dst, dst_size, checkout_path, gitdir);
  return dst[0] != '\0';
}

static bool MiniCanonicalizeExistingPath(const char *path, char *dst, size_t dst_size) {
#ifdef PATH_MAX
  char canonical[PATH_MAX];
#else
  char canonical[4096];
#endif
  if (realpath(path, canonical) == NULL)
    return false;
  snprintf(dst, dst_size, "%s", canonical);
  return dst[0] != '\0';
}

static bool MiniDeriveGitCommonBasePath(const char *checkout_path, char *dst, size_t dst_size) {
  char git_dir[512];
  char common_dir_file[512];
  char common_dir[512];
  char common_dir_line[512];
  char canonical_common_dir[512];
  if (!MiniReadCheckoutGitDir(checkout_path, git_dir, sizeof(git_dir)))
    return false;

  MiniJoinPath(common_dir_file, sizeof(common_dir_file), git_dir, "commondir");
  if (MiniReadFirstLine(common_dir_file, common_dir_line, sizeof(common_dir_line))) {
    MiniJoinPath(common_dir, sizeof(common_dir), git_dir, common_dir_line);
  } else {
    snprintf(common_dir, sizeof(common_dir), "%s", git_dir);
  }

  if (!MiniCanonicalizeExistingPath(common_dir, canonical_common_dir, sizeof(canonical_common_dir)))
    return false;
  MiniCopyDirname(dst, dst_size, canonical_common_dir);
  MiniTrimTrailingSlashes(dst);
  return dst[0] != '\0';
}

static bool MiniResolveSearchCandidate(const char *candidate, char *dst, size_t dst_size) {
  if (candidate == NULL || candidate[0] == '\0' || dst_size == 0)
    return false;
  if (MiniPathIsAbsolute(candidate)) {
    snprintf(dst, dst_size, "%s", candidate);
    return true;
  }
  if (MiniPathExists(candidate)) {
    snprintf(dst, dst_size, "%s", candidate);
    return true;
  }
  if (g_base_path[0] != '\0') {
    snprintf(dst, dst_size, "%s/%s", g_base_path, candidate);
    if (MiniPathExists(dst))
      return true;
  }
  if (g_git_common_base_path[0] != '\0') {
    snprintf(dst, dst_size, "%s/%s", g_git_common_base_path, candidate);
    if (MiniPathExists(dst))
      return true;
  }
  return false;
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

static bool MiniMaterialNameToBlockType(const char *name, BlockType *block_type) {
  if (name == NULL)
    return false;
  if (strcmp(name, "air") == 0 || strcmp(name, "empty") == 0) {
    *block_type = kBlockType_Air;
  } else if (strcmp(name, "slope") == 0) {
    *block_type = kBlockType_Slope;
  } else if (strcmp(name, "spike_air") == 0) {
    *block_type = kBlockType_SpikeAir;
  } else if (strcmp(name, "special_air") == 0) {
    *block_type = kBlockType_SpecialAir;
  } else if (strcmp(name, "shootable_air") == 0) {
    *block_type = kBlockType_ShootableAir;
  } else if (strcmp(name, "horizontal_extension") == 0) {
    *block_type = kBlockType_HorizontalExtension;
  } else if (strcmp(name, "unused_air") == 0) {
    *block_type = kBlockType_UnusedAir;
  } else if (strcmp(name, "bombable_air") == 0) {
    *block_type = kBlockType_BombableAir;
  } else if (strcmp(name, "solid") == 0 || strcmp(name, "wall") == 0 ||
             strcmp(name, "floor") == 0) {
    *block_type = kBlockType_Solid;
  } else if (strcmp(name, "door") == 0) {
    *block_type = kBlockType_Door;
  } else if (strcmp(name, "spike_block") == 0 || strcmp(name, "hazard") == 0 ||
             strcmp(name, "spike") == 0) {
    *block_type = kBlockType_SpikeBlock;
  } else if (strcmp(name, "special_block") == 0) {
    *block_type = kBlockType_SpecialBlock;
  } else if (strcmp(name, "shootable_block") == 0) {
    *block_type = kBlockType_ShootableBlock;
  } else if (strcmp(name, "vertical_extension") == 0) {
    *block_type = kBlockType_VerticalExtension;
  } else if (strcmp(name, "grapple_block") == 0) {
    *block_type = kBlockType_GrappleBlock;
  } else if (strcmp(name, "bombable_block") == 0) {
    *block_type = kBlockType_BombableBlock;
  } else {
    return false;
  }
  return true;
}

static bool MiniParseMaterialGridRow(cJSON *row, int width, uint8 *dst) {
  if (!cJSON_IsArray(row) || cJSON_GetArraySize(row) != width)
    return false;
  for (int x = 0; x < width; x++) {
    cJSON *value = cJSON_GetArrayItem(row, x);
    BlockType block_type;
    if (!cJSON_IsString(value) ||
        !MiniMaterialNameToBlockType(value->valuestring, &block_type)) {
      return false;
    }
    dst[x] = (uint8)BlockTypeIndexFromTile((uint16)block_type);
  }
  return true;
}

static bool MiniParseMaterialGrid(cJSON *grid, int width, int height, uint8 *dst) {
  if (!cJSON_IsArray(grid) || cJSON_GetArraySize(grid) != height)
    return false;
  for (int y = 0; y < height; y++) {
    if (!MiniParseMaterialGridRow(cJSON_GetArrayItem(grid, y), width, dst + y * width))
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

static bool MiniParseCameraFollow(cJSON *camera_follow, MiniEditorRoom *room) {
  if (!cJSON_IsObject(camera_follow))
    return false;
  int target_x_percent, target_y_percent;
  if (!MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(camera_follow, "targetXPercent"),
                      &target_x_percent) ||
      !MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(camera_follow, "targetYPercent"),
                      &target_y_percent)) {
    return false;
  }
  if (target_x_percent < 0 || target_x_percent > 100 ||
      target_y_percent < 0 || target_y_percent > 100) {
    return false;
  }
  room->camera_target_x_percent = target_x_percent;
  room->camera_target_y_percent = target_y_percent;
  return true;
}

static bool MiniParseDoorwayTransition(cJSON *node, int width_blocks, int height_blocks,
                                       MiniDoorwayTransition *doorway) {
  if (!cJSON_IsObject(node))
    return false;
  int source_block_x, source_block_y, destination_x, destination_y, camera_x, camera_y;
  if (!MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(node, "blockX"), &source_block_x) ||
      !MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(node, "blockY"), &source_block_y) ||
      !MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(node, "targetX"), &destination_x) ||
      !MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(node, "targetY"), &destination_y) ||
      !MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(node, "cameraX"), &camera_x) ||
      !MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(node, "cameraY"), &camera_y)) {
    return false;
  }

  if ((unsigned)source_block_x >= (unsigned)width_blocks ||
      (unsigned)source_block_y >= (unsigned)height_blocks ||
      destination_x < 0 || destination_x >= width_blocks * kMiniBlockSize ||
      destination_y < 0 || destination_y >= height_blocks * kMiniBlockSize ||
      camera_x < 0 || camera_y < 0) {
    return false;
  }

  *doorway = (MiniDoorwayTransition){
    .active = true,
    .source_block_x = source_block_x,
    .source_block_y = source_block_y,
    .destination_x = destination_x,
    .destination_y = destination_y,
    .camera_x = camera_x,
    .camera_y = camera_y,
  };
  return true;
}

static bool MiniParseDoorwayTransitions(cJSON *doorways, int width_blocks, int height_blocks,
                                        MiniEditorRoom *room) {
  if (!cJSON_IsArray(doorways))
    return false;
  int count = cJSON_GetArraySize(doorways);
  if (count < 0 || count > kMiniDoorwayTransitionCapacity)
    return false;
  for (int i = 0; i < count; i++) {
    if (!MiniParseDoorwayTransition(cJSON_GetArrayItem(doorways, i), width_blocks, height_blocks,
                                    &room->doorways[i])) {
      return false;
    }
  }
  room->doorway_count = count;
  return true;
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

static bool MiniMaybeLoadSamusRenderedSprites(const char *room_path, cJSON *samus_assets, MiniEditorRoom *room) {
  cJSON *rendered = cJSON_GetObjectItemCaseSensitive(samus_assets, "renderedSprites");
  if (!cJSON_IsObject(rendered))
    return true;

  char rgba_path[512];
  int frame_width = 0, frame_height = 0;
  if (!MiniCopyJsonString(cJSON_GetObjectItemCaseSensitive(rendered, "rgbaPath"),
                          rgba_path, sizeof(rgba_path)) ||
      !MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(rendered, "frameWidth"), &frame_width) ||
      !MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(rendered, "frameHeight"), &frame_height) ||
      frame_width <= 0 || frame_height <= 0 || frame_width > 256 || frame_height > 256) {
    return false;
  }

  cJSON *frames = cJSON_GetObjectItemCaseSensitive(rendered, "frames");
  if (!cJSON_IsArray(frames))
    return false;
  int frame_count = cJSON_GetArraySize(frames);
  if (frame_count <= 0)
    return false;

  char resolved_rgba[512];
  if (!MiniResolveRelativePath(room_path, rgba_path, resolved_rgba, sizeof(resolved_rgba)))
    return false;
  if (!MiniReadWholeBinaryFile(resolved_rgba, &room->samus_rendered_sprite_rgba,
                               &room->samus_rendered_sprite_rgba_size)) {
    return false;
  }

  room->samus_rendered_frames =
      (MiniEditorSamusRenderedFrame *)calloc((size_t)frame_count, sizeof(*room->samus_rendered_frames));
  if (room->samus_rendered_frames == NULL)
    return false;

  size_t frame_size = (size_t)frame_width * (size_t)frame_height * 4;
  for (int i = 0; i < frame_count; i++) {
    cJSON *frame = cJSON_GetArrayItem(frames, i);
    int pose, anim_frame, data_offset, origin_x, origin_y;
    if (!cJSON_IsObject(frame) ||
        !MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(frame, "pose"), &pose) ||
        !MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(frame, "animFrame"), &anim_frame) ||
        !MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(frame, "dataOffset"), &data_offset) ||
        !MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(frame, "originX"), &origin_x) ||
        !MiniGetJsonInt(cJSON_GetObjectItemCaseSensitive(frame, "originY"), &origin_y) ||
        pose < 0 || pose > 0xFFFF || anim_frame < 0 || anim_frame > 0xFFFF ||
        data_offset < 0 || origin_x < -32768 || origin_x > 32767 ||
        origin_y < -32768 || origin_y > 32767) {
      return false;
    }
    if ((size_t)data_offset + frame_size > room->samus_rendered_sprite_rgba_size)
      return false;
    room->samus_rendered_frames[i] = (MiniEditorSamusRenderedFrame){
      .pose = (uint16)pose,
      .anim_frame = (uint16)anim_frame,
      .data_offset = (uint32)data_offset,
      .origin_x = (int16)origin_x,
      .origin_y = (int16)origin_y,
    };
  }

  room->samus_rendered_frame_count = frame_count;
  room->samus_rendered_frame_width = frame_width;
  room->samus_rendered_frame_height = frame_height;
  room->has_samus_rendered_sprites = true;
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
  return MiniMaybeLoadSamusRenderedSprites(room_path, samus_assets, room);
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
  room->camera_target_x_percent = kMiniCameraFollowDefaultTargetPercent;
  room->camera_target_y_percent = kMiniCameraFollowDefaultTargetPercent;

  cJSON *tileset_meta = cJSON_GetObjectItemCaseSensitive(root, "tilesetMeta");
  cJSON *background_assets = cJSON_GetObjectItemCaseSensitive(root, "backgroundAssets");
  cJSON *samus_assets = cJSON_GetObjectItemCaseSensitive(root, "samusAssets");
  cJSON *scroll = cJSON_GetObjectItemCaseSensitive(root, "scroll");
  cJSON *camera = cJSON_GetObjectItemCaseSensitive(root, "camera");
  cJSON *camera_follow = cJSON_GetObjectItemCaseSensitive(root, "cameraFollow");
  cJSON *block_words = cJSON_GetObjectItemCaseSensitive(root, "blockWords");
  cJSON *materials = cJSON_GetObjectItemCaseSensitive(root, "materials");
  cJSON *collision = cJSON_GetObjectItemCaseSensitive(root, "collision");
  cJSON *bts = cJSON_GetObjectItemCaseSensitive(root, "bts");
  cJSON *room_sprites = cJSON_GetObjectItemCaseSensitive(root, "roomSprites");
  cJSON *doorways = cJSON_GetObjectItemCaseSensitive(root, "doorways");

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

  if (ok && cJSON_IsArray(materials))
    ok = MiniParseMaterialGrid(materials, width_blocks, height_blocks, room->collision_types);
  else if (ok)
    ok = MiniParseGrid(collision, width_blocks, height_blocks, room->collision_types);
  if (ok && cJSON_IsArray(bts))
    ok = MiniParseGrid(bts, width_blocks, height_blocks, room->bts);
  if (ok && cJSON_IsArray(block_words))
    ok = MiniParseWordGrid(block_words, width_blocks, height_blocks, room->block_words);
  if (ok && cJSON_IsObject(camera))
    ok = MiniParseCamera(camera, room);
  if (ok && cJSON_IsObject(camera_follow))
    ok = MiniParseCameraFollow(camera_follow, room);
  if (ok && cJSON_IsArray(doorways))
    ok = MiniParseDoorwayTransitions(doorways, width_blocks, height_blocks, room);
  if (ok && cJSON_IsObject(scroll))
    ok = MiniParseScroll(scroll, room);
  if (ok && room->scroll_values != NULL && screen_count > 256)
    ok = false;
  if (ok && !cJSON_IsArray(block_words)) {
    for (size_t i = 0; i < block_count; i++)
      room->block_words[i] = BlockTileWithTypeIndex(0, room->collision_types[i]);
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

void MiniEditorBridge_SetBasePath(const char *path) {
  char *sdl_base = NULL;

  g_base_path[0] = '\0';
  g_git_common_base_path[0] = '\0';
  if (path != NULL && path[0] != '\0')
    MiniCopyDirname(g_base_path, sizeof(g_base_path), path);

  sdl_base = SDL_GetBasePath();
  if (sdl_base != NULL && sdl_base[0] != '\0')
    snprintf(g_base_path, sizeof(g_base_path), "%s", sdl_base);
  SDL_free(sdl_base);

  MiniTrimTrailingSlashes(g_base_path);
  if (g_base_path[0] != '\0')
    MiniDeriveGitCommonBasePath(g_base_path, g_git_common_base_path, sizeof(g_git_common_base_path));
}

const char *MiniEditorBridge_GetResolvedPath(void) {
  return g_resolved_room_export_path[0] != '\0' ? g_resolved_room_export_path : NULL;
}

bool MiniEditorBridge_LoadRoom(MiniEditorRoom *room) {
  char resolved_path[512];
  MiniEditorRoom_Reset(room);
  if (g_room_export_path != NULL) {
    const char *path = g_room_export_path;
    if (MiniResolveSearchCandidate(g_room_export_path, resolved_path, sizeof(resolved_path)))
      path = resolved_path;
    return MiniParseRoomJson(path, room);
  }

  for (size_t i = 0; i < sizeof(kDefaultRoomExportCandidates) / sizeof(kDefaultRoomExportCandidates[0]); i++) {
    if (MiniResolveSearchCandidate(kDefaultRoomExportCandidates[i], resolved_path, sizeof(resolved_path)) &&
        MiniParseRoomJson(resolved_path, room)) {
      return true;
    }
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
  free(room->samus_rendered_sprite_rgba);
  free(room->samus_rendered_frames);
  free(room->room_sprites);
  MiniEditorRoom_Reset(room);
}
