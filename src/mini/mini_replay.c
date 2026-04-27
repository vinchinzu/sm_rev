#include "mini_replay.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mini_content_scope.h"
#include "stubs_mini.h"
#include "third_party/cJSON.h"

enum {
  kMiniReplayVersion = 1,
  kMiniReplayMaxFrames = 1000000,
};

static const char kMiniReplayFormat[] = "sm_rev_mini_replay";

void MiniReplayFrames_Clear(MiniReplayFrames *frames) {
  free(frames->frames);
  frames->frames = NULL;
  frames->count = 0;
  frames->capacity = 0;
}

bool MiniReplayFrames_Append(MiniReplayFrames *frames, MiniScriptFrame frame) {
  if (frames->count == frames->capacity) {
    int capacity = frames->capacity ? frames->capacity * 2 : 64;
    MiniScriptFrame *grown = (MiniScriptFrame *)realloc(
        frames->frames, (size_t)capacity * sizeof(*frames->frames));
    if (grown == NULL)
      return false;
    frames->frames = grown;
    frames->capacity = capacity;
  }
  frames->frames[frames->count++] = frame;
  return true;
}

void MiniReplay_Clear(MiniReplayArtifact *artifact) {
  MiniReplayFrames_Clear(&artifact->inputs);
  memset(artifact, 0, sizeof(*artifact));
}

static bool MiniReplay_ReadFile(const char *path, char **out_data) {
  *out_data = NULL;
  FILE *f = fopen(path, "rb");
  if (f == NULL) {
    fprintf(stderr, "mini: could not open replay artifact %s\n", path);
    return false;
  }
  if (fseek(f, 0, SEEK_END) != 0) {
    fclose(f);
    return false;
  }
  long size = ftell(f);
  if (size <= 0 || size > 64 * 1024 * 1024) {
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

static cJSON *MiniReplay_GetObject(cJSON *parent, const char *name) {
  cJSON *item = cJSON_GetObjectItemCaseSensitive(parent, name);
  return cJSON_IsObject(item) ? item : NULL;
}

static bool MiniReplay_GetInt(cJSON *parent, const char *name, int *value) {
  cJSON *item = cJSON_GetObjectItemCaseSensitive(parent, name);
  if (!cJSON_IsNumber(item))
    return false;
  int parsed = item->valueint;
  if ((double)parsed != item->valuedouble)
    return false;
  *value = parsed;
  return true;
}

static bool MiniReplay_GetString(cJSON *parent, const char *name,
                                 char *dst, size_t dst_size) {
  cJSON *item = cJSON_GetObjectItemCaseSensitive(parent, name);
  if (!cJSON_IsString(item) || item->valuestring == NULL || dst_size == 0)
    return false;
  snprintf(dst, dst_size, "%s", item->valuestring);
  return true;
}

static bool MiniReplay_GetOptionalString(cJSON *parent, const char *name,
                                         char *dst, size_t dst_size) {
  cJSON *item = cJSON_GetObjectItemCaseSensitive(parent, name);
  if (item == NULL) {
    if (dst_size != 0)
      dst[0] = '\0';
    return true;
  }
  if (!cJSON_IsString(item) || item->valuestring == NULL || dst_size == 0)
    return false;
  snprintf(dst, dst_size, "%s", item->valuestring);
  return true;
}

static bool MiniReplay_ParseHash(cJSON *parent, const char *name, uint64 *value) {
  char text[32];
  if (!MiniReplay_GetString(parent, name, text, sizeof(text)))
    return false;
  const char *digits = text;
  if (text[0] == '0' && (text[1] == 'x' || text[1] == 'X'))
    digits = text + 2;
  if (digits[0] == '\0')
    return false;
  errno = 0;
  char *end = NULL;
  unsigned long long parsed = strtoull(digits, &end, 16);
  if (errno != 0 || end == NULL || *end != '\0')
    return false;
  *value = (uint64)parsed;
  return true;
}

static bool MiniReplay_ParseInputs(MiniReplayArtifact *artifact, cJSON *root) {
  cJSON *inputs = cJSON_GetObjectItemCaseSensitive(root, "inputs");
  if (!cJSON_IsArray(inputs) || cJSON_GetArraySize(inputs) != artifact->frames)
    return false;

  for (int i = 0; i < artifact->frames; i++) {
    cJSON *item = cJSON_GetArrayItem(inputs, i);
    int frame_index = 0;
    int buttons = 0;
    if (!cJSON_IsObject(item) ||
        !MiniReplay_GetInt(item, "frame", &frame_index) ||
        !MiniReplay_GetInt(item, "buttons", &buttons) ||
        frame_index != i || buttons < 0 || buttons > 0xFFFF)
      return false;

    cJSON *quit = cJSON_GetObjectItemCaseSensitive(item, "quit");
    if (!cJSON_IsBool(quit))
      return false;

    MiniScriptFrame frame = {
      .buttons = (uint16)buttons,
      .quit_requested = cJSON_IsTrue(quit),
    };
    if (!MiniReplayFrames_Append(&artifact->inputs, frame))
      return false;
  }

  return true;
}

bool MiniReplay_Load(MiniReplayArtifact *artifact, const char *path) {
  char *data = NULL;
  MiniReplay_Clear(artifact);
  if (!MiniReplay_ReadFile(path, &data))
    return false;

  cJSON *root = cJSON_Parse(data);
  free(data);
  if (root == NULL) {
    fprintf(stderr, "mini: replay artifact is not valid JSON: %s\n", path);
    return false;
  }

  char format[64];
  cJSON *room = MiniReplay_GetObject(root, "room");
  bool ok = cJSON_IsObject(root) &&
            MiniReplay_GetString(root, "format", format, sizeof(format)) &&
            strcmp(format, kMiniReplayFormat) == 0 &&
            MiniReplay_GetInt(root, "version", &artifact->version) &&
            artifact->version == kMiniReplayVersion &&
            MiniReplay_GetInt(root, "frames", &artifact->frames) &&
            artifact->frames >= 0 && artifact->frames <= kMiniReplayMaxFrames &&
            MiniReplay_ParseHash(root, "initial_hash", &artifact->initial_hash) &&
            MiniReplay_ParseHash(root, "final_hash", &artifact->final_hash) &&
            MiniReplay_GetObject(root, "viewport") != NULL &&
            room != NULL;

  cJSON *viewport = MiniReplay_GetObject(root, "viewport");
  if (ok) {
    ok = MiniReplay_GetInt(viewport, "width", &artifact->viewport_width) &&
         MiniReplay_GetInt(viewport, "height", &artifact->viewport_height) &&
         MiniReplay_GetOptionalString(room, "export_path", artifact->room_export_path,
                                      sizeof(artifact->room_export_path)) &&
         MiniReplay_ParseInputs(artifact, root);
  }

  cJSON_Delete(root);
  if (!ok) {
    fprintf(stderr, "mini: replay artifact has an unsupported shape: %s\n", path);
    MiniReplay_Clear(artifact);
  }
  return ok;
}

static void MiniReplay_WriteJsonString(FILE *f, const char *text) {
  fputc('"', f);
  if (text != NULL) {
    for (const unsigned char *p = (const unsigned char *)text; *p != '\0'; p++) {
      switch (*p) {
      case '\\':
        fputs("\\\\", f);
        break;
      case '"':
        fputs("\\\"", f);
        break;
      case '\b':
        fputs("\\b", f);
        break;
      case '\f':
        fputs("\\f", f);
        break;
      case '\n':
        fputs("\\n", f);
        break;
      case '\r':
        fputs("\\r", f);
        break;
      case '\t':
        fputs("\\t", f);
        break;
      default:
        if (*p < 0x20)
          fprintf(f, "\\u%04x", *p);
        else
          fputc(*p, f);
        break;
      }
    }
  }
  fputc('"', f);
}

static void MiniReplay_WriteHash(FILE *f, uint64 hash) {
  fprintf(f, "\"0x%016llx\"", (unsigned long long)hash);
}

bool MiniReplay_Write(const char *path, const MiniReplayWriteInfo *info,
                      const MiniReplayFrames *inputs) {
  if (path == NULL || info == NULL || info->final_state == NULL ||
      inputs == NULL || inputs->count != info->frames ||
      info->frames < 0 || info->frames > kMiniReplayMaxFrames)
    return false;

  MiniRoomInfo room;
  MiniStubs_GetRoomInfo(&room);

  FILE *f = fopen(path, "wb");
  if (f == NULL) {
    fprintf(stderr, "mini: could not write replay artifact %s\n", path);
    return false;
  }

  const MiniGameState *state = info->final_state;
  fputs("{\n  \"format\": ", f);
  MiniReplay_WriteJsonString(f, kMiniReplayFormat);
  fprintf(f, ",\n  \"version\": %d,\n  \"frames\": %d,\n", kMiniReplayVersion, info->frames);
  fprintf(f, "  \"viewport\": {\"width\": %d, \"height\": %d},\n",
          info->viewport_width, info->viewport_height);
  fputs("  \"content_scope\": ", f);
  MiniReplay_WriteJsonString(f, info->content_scope != NULL ? info->content_scope
                                                           : MiniContentScope_Name());
  fputs(",\n  \"background\": ", f);
  MiniReplay_WriteJsonString(f, info->background != NULL ? info->background : "");
  fputs(",\n  \"room\": {\n", f);
  fprintf(f, "    \"id\": %u,\n", state->room_id);
  fputs("    \"source\": ", f);
  MiniReplay_WriteJsonString(f, MiniStubs_RoomSourceName(state->room_source));
  fputs(",\n    \"handle\": ", f);
  MiniReplay_WriteJsonString(f, state->room_handle);
  fputs(",\n    \"name\": ", f);
  MiniReplay_WriteJsonString(f, state->room_name);
  fputs(",\n    \"export_path\": ", f);
  MiniReplay_WriteJsonString(f, info->room_export_path != NULL ? info->room_export_path : "");
  fprintf(f, ",\n    \"uses_rom_room\": %s,\n", state->uses_rom_room ? "true" : "false");
  fprintf(f, "    \"original_runtime\": %s,\n",
          state->uses_original_gameplay_runtime ? "true" : "false");
  fprintf(f, "    \"bounds\": {\"left\": %d, \"top\": %d, \"right\": %d, \"bottom\": %d},\n",
          room.room_left, room.room_top, room.room_right, room.room_bottom);
  fprintf(f, "    \"spawn\": {\"x\": %d, \"y\": %d}\n", room.spawn_x, room.spawn_y);
  fputs("  },\n  \"initial_hash\": ", f);
  MiniReplay_WriteHash(f, info->initial_hash);
  fputs(",\n  \"final_hash\": ", f);
  MiniReplay_WriteHash(f, info->final_hash);
  fputs(",\n  \"checkpoints\": [\n    {\"label\": \"initial\", \"frame\": 0, \"state_hash\": ", f);
  MiniReplay_WriteHash(f, info->initial_hash);
  fputs("},\n    {\"label\": \"final\", \"frame\": ", f);
  fprintf(f, "%d, \"state_hash\": ", info->frames);
  MiniReplay_WriteHash(f, info->final_hash);
  fputs("}\n  ],\n  \"inputs\": [\n", f);

  for (int i = 0; i < inputs->count; i++) {
    fprintf(f, "    {\"frame\": %d, \"buttons\": %u, \"buttons_hex\": \"0x%04x\", \"quit\": %s}%s\n",
            i,
            inputs->frames[i].buttons,
            inputs->frames[i].buttons,
            inputs->frames[i].quit_requested ? "true" : "false",
            i + 1 == inputs->count ? "" : ",");
  }

  fputs("  ]\n}\n", f);
  bool ok = fclose(f) == 0;
  if (!ok)
    fprintf(stderr, "mini: failed to finish writing replay artifact %s\n", path);
  return ok;
}
