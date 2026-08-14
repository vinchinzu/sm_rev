#ifndef SM_MINI_EDITOR_PATH_H_
#define SM_MINI_EDITOR_PATH_H_

#include <stddef.h>

#include "types.h"

void MiniEditorPath_SetRoomExportPath(const char *path);
void MiniEditorPath_SetBasePath(const char *path);
void MiniEditorPath_SetResolvedPath(const char *path);
const char *MiniEditorPath_GetResolvedPath(void);
const char *MiniEditorPath_ExportPath(void);
bool MiniEditorPath_Exists(const char *path);
bool MiniEditorPath_ResolveSearchCandidate(const char *candidate, char *dst, size_t dst_size);
bool MiniEditorPath_ResolveRelative(const char *base_path, const char *asset_path,
                                    char *dst, size_t dst_size);
void MiniEditorPath_CopyDirname(char *dst, size_t dst_size, const char *path);
bool MiniEditorPath_ResolveDefaultRoom(char *dst, size_t dst_size);

#endif  // SM_MINI_EDITOR_PATH_H_
