#include "mini/mini_editor_path.h"

#include <stdio.h>
#include <string.h>

static const char *g_room_export_path;
static char g_resolved_room_export_path[512];
static char g_base_path[512];

void MiniEditorPath_SetRoomExportPath(const char *path) {
  g_room_export_path = (path != NULL && path[0] != '\0') ? path : NULL;
  g_resolved_room_export_path[0] = '\0';
}

void MiniEditorPath_SetBasePath(const char *path) {
  g_base_path[0] = '\0';
  if (path != NULL && path[0] != '\0')
    MiniEditorPath_CopyDirname(g_base_path, sizeof(g_base_path), path);
}

void MiniEditorPath_SetResolvedPath(const char *path) {
  if (path == NULL || path[0] == '\0') {
    g_resolved_room_export_path[0] = '\0';
    return;
  }
  snprintf(g_resolved_room_export_path, sizeof(g_resolved_room_export_path), "%s", path);
}

const char *MiniEditorPath_GetResolvedPath(void) {
  return g_resolved_room_export_path[0] != '\0' ? g_resolved_room_export_path : NULL;
}

const char *MiniEditorPath_ExportPath(void) {
  return g_room_export_path;
}

bool MiniEditorPath_Exists(const char *path) {
  (void)path;
  /* No host filesystem on-device; MiniCreate uses the fallback room. */
  return false;
}

bool MiniEditorPath_ResolveSearchCandidate(const char *candidate, char *dst, size_t dst_size) {
  (void)candidate;
  if (dst != NULL && dst_size > 0)
    dst[0] = '\0';
  return false;
}

bool MiniEditorPath_ResolveRelative(const char *base_path, const char *asset_path,
                                    char *dst, size_t dst_size) {
  (void)base_path;
  (void)asset_path;
  if (dst != NULL && dst_size > 0)
    dst[0] = '\0';
  return false;
}

void MiniEditorPath_CopyDirname(char *dst, size_t dst_size, const char *path) {
  const char *slash;
  if (dst == NULL || dst_size == 0)
    return;
  if (path == NULL) {
    dst[0] = '\0';
    return;
  }
  slash = strrchr(path, '/');
  if (slash == NULL) {
    snprintf(dst, dst_size, ".");
    return;
  }
  snprintf(dst, dst_size, "%.*s", (int)(slash - path), path);
}

bool MiniEditorPath_ResolveDefaultRoom(char *dst, size_t dst_size) {
  if (dst != NULL && dst_size > 0)
    dst[0] = '\0';
  return false;
}
