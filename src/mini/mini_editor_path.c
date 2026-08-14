#include "mini_editor_path.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

#include <SDL.h>

static const char *g_room_export_path;
static char g_resolved_room_export_path[512];
static char g_base_path[512];
static char g_git_common_base_path[512];

static const char *const kDefaultRoomExportCandidates[] = {
  "assets/mini/landing_site.room.json",
  "assets/local_mini/room_91F8.json",
  "../super_metroid_editor/export/sm_nav/rooms/room_91F8.json",
};

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

bool MiniEditorPath_Exists(const char *path) {
  FILE *f = fopen(path, "rb");
  if (f == NULL)
    return false;
  fclose(f);
  return true;
}

void MiniEditorPath_CopyDirname(char *dst, size_t dst_size, const char *path) {
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
  if (MiniEditorPath_Exists(dot_git_head)) {
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
  MiniEditorPath_CopyDirname(dst, dst_size, canonical_common_dir);
  MiniTrimTrailingSlashes(dst);
  return dst[0] != '\0';
}

bool MiniEditorPath_ResolveSearchCandidate(const char *candidate, char *dst, size_t dst_size) {
  if (candidate == NULL || candidate[0] == '\0' || dst_size == 0)
    return false;
  if (MiniPathIsAbsolute(candidate)) {
    snprintf(dst, dst_size, "%s", candidate);
    return true;
  }
  if (MiniEditorPath_Exists(candidate)) {
    snprintf(dst, dst_size, "%s", candidate);
    return true;
  }
  if (g_base_path[0] != '\0') {
    snprintf(dst, dst_size, "%s/%s", g_base_path, candidate);
    if (MiniEditorPath_Exists(dst))
      return true;
  }
  if (g_git_common_base_path[0] != '\0') {
    snprintf(dst, dst_size, "%s/%s", g_git_common_base_path, candidate);
    if (MiniEditorPath_Exists(dst))
      return true;
  }
  return false;
}

bool MiniEditorPath_ResolveRelative(const char *base_path, const char *asset_path,
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

bool MiniEditorPath_ResolveDefaultRoom(char *dst, size_t dst_size) {
  for (size_t i = 0; i < sizeof(kDefaultRoomExportCandidates) / sizeof(kDefaultRoomExportCandidates[0]); i++) {
    if (MiniEditorPath_ResolveSearchCandidate(kDefaultRoomExportCandidates[i], dst, dst_size))
      return true;
  }
  return false;
}

void MiniEditorPath_SetRoomExportPath(const char *path) {
  g_room_export_path = (path != NULL && path[0] != '\0') ? path : NULL;
  g_resolved_room_export_path[0] = '\0';
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

void MiniEditorPath_SetBasePath(const char *path) {
  char *sdl_base = NULL;

  g_base_path[0] = '\0';
  g_git_common_base_path[0] = '\0';
  if (path != NULL && path[0] != '\0')
    MiniEditorPath_CopyDirname(g_base_path, sizeof(g_base_path), path);

  sdl_base = SDL_GetBasePath();
  if (sdl_base != NULL && sdl_base[0] != '\0')
    snprintf(g_base_path, sizeof(g_base_path), "%s", sdl_base);
  SDL_free(sdl_base);

  MiniTrimTrailingSlashes(g_base_path);
  if (g_base_path[0] != '\0')
    MiniDeriveGitCommonBasePath(g_base_path, g_git_common_base_path, sizeof(g_git_common_base_path));
}

const char *MiniEditorPath_ExportPath(void) {
  return g_room_export_path;
}
