#ifndef SM_MINI_REPLAY_H_
#define SM_MINI_REPLAY_H_

#include "mini_game.h"
#include "mini_input_script.h"
#include "types.h"

enum {
  kMiniReplayRoomExportPathCapacity = 512,
};

typedef struct MiniReplayFrames {
  MiniScriptFrame *frames;
  int count;
  int capacity;
} MiniReplayFrames;

typedef struct MiniReplayArtifact {
  int version;
  int frames;
  int viewport_width;
  int viewport_height;
  uint64 initial_hash;
  uint64 final_hash;
  char room_export_path[kMiniReplayRoomExportPathCapacity];
  MiniReplayFrames inputs;
} MiniReplayArtifact;

typedef struct MiniReplayWriteInfo {
  int frames;
  int viewport_width;
  int viewport_height;
  uint64 initial_hash;
  uint64 final_hash;
  const char *content_scope;
  const char *background;
  const char *room_export_path;
  const MiniGameState *final_state;
} MiniReplayWriteInfo;

void MiniReplayFrames_Clear(MiniReplayFrames *frames);
bool MiniReplayFrames_Append(MiniReplayFrames *frames, MiniScriptFrame frame);

void MiniReplay_Clear(MiniReplayArtifact *artifact);
bool MiniReplay_Load(MiniReplayArtifact *artifact, const char *path);
bool MiniReplay_Write(const char *path, const MiniReplayWriteInfo *info,
                      const MiniReplayFrames *inputs);

#endif  // SM_MINI_REPLAY_H_
