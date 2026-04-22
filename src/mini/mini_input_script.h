#ifndef SM_MINI_INPUT_SCRIPT_H_
#define SM_MINI_INPUT_SCRIPT_H_

#include "types.h"

typedef struct MiniScriptFrame {
  uint16 buttons;
  bool quit_requested;
} MiniScriptFrame;

typedef struct MiniInputScript {
  MiniScriptFrame *frames;
  int count;
} MiniInputScript;

void MiniInputScript_Clear(MiniInputScript *script);
bool MiniInputScript_Load(MiniInputScript *script, const char *path);
void MiniInputScript_ApplyFrame(const MiniInputScript *script, int frame_index, MiniScriptFrame *frame);

#endif  // SM_MINI_INPUT_SCRIPT_H_
