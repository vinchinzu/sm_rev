#include "mini_input_script.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ida_types.h"

static void MiniUppercase(char *text) {
  for (; *text; text++)
    *text = (char)toupper((unsigned char)*text);
}

static bool MiniParseButtonToken(const char *token, uint16 *buttons, bool *quit_requested) {
  if (!strcmp(token, ".") || !strcmp(token, "WAIT") || !strcmp(token, "NONE"))
    return true;
  if (!strcmp(token, "L") || !strcmp(token, "LEFT")) {
    *buttons |= kButton_Left;
  } else if (!strcmp(token, "R") || !strcmp(token, "RIGHT")) {
    *buttons |= kButton_Right;
  } else if (!strcmp(token, "U") || !strcmp(token, "UP")) {
    *buttons |= kButton_Up;
  } else if (!strcmp(token, "D") || !strcmp(token, "DOWN")) {
    *buttons |= kButton_Down;
  } else if (!strcmp(token, "J") || !strcmp(token, "JUMP")) {
    *buttons |= kButton_A;
  } else if (!strcmp(token, "RUN") || !strcmp(token, "B")) {
    *buttons |= kButton_B;
  } else if (!strcmp(token, "SHOOT") || !strcmp(token, "X")) {
    *buttons |= kButton_X;
  } else if (!strcmp(token, "ITEM") || !strcmp(token, "Y")) {
    *buttons |= kButton_Y;
  } else if (!strcmp(token, "AIMUP") || !strcmp(token, "RU") || !strcmp(token, "RSHOULDER")) {
    *buttons |= kButton_R;
  } else if (!strcmp(token, "AIMDOWN") || !strcmp(token, "LU") || !strcmp(token, "LSHOULDER")) {
    *buttons |= kButton_L;
  } else if (!strcmp(token, "QUIT")) {
    *quit_requested = true;
  } else {
    return false;
  }
  return true;
}

static bool MiniParseInputToken(const char *token, MiniScriptFrame *frame) {
  int player_index = 0;
  const char *button_token = token;
  if (!strncmp(token, "P1:", 3) || !strncmp(token, "P2:", 3)) {
    player_index = token[1] == '2' ? 1 : 0;
    button_token = token + 3;
    if (*button_token == '\0')
      return false;
  }

  uint16 buttons = 0;
  bool quit_requested = false;
  if (!MiniParseButtonToken(button_token, &buttons, &quit_requested))
    return false;

  frame->player_buttons[player_index] |= buttons;
  frame->buttons = frame->player_buttons[0];
  if (player_index + 1 > frame->player_count)
    frame->player_count = player_index + 1;
  if (quit_requested)
    frame->quit_requested = true;
  return true;
}

void MiniInputScript_Clear(MiniInputScript *script) {
  free(script->frames);
  script->frames = NULL;
  script->count = 0;
}

bool MiniInputScript_Load(MiniInputScript *script, const char *path) {
  FILE *f = fopen(path, "r");
  if (f == NULL) {
    fprintf(stderr, "mini: could not open input script %s\n", path);
    return false;
  }

  MiniInputScript_Clear(script);
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
        MiniInputScript_Clear(script);
        return false;
      }
    }
    frame.buttons = frame.player_buttons[0];
    if (frame.player_count == 0)
      frame.player_count = 1;

    if (script->count == capacity) {
      capacity = capacity ? capacity * 2 : 64;
      MiniScriptFrame *frames = realloc(script->frames, (size_t)capacity * sizeof(*frames));
      if (frames == NULL) {
        fclose(f);
        MiniInputScript_Clear(script);
        return false;
      }
      script->frames = frames;
    }
    script->frames[script->count++] = frame;
  }

  fclose(f);
  return true;
}

void MiniInputScript_ApplyFrame(const MiniInputScript *script, int frame_index, MiniScriptFrame *frame) {
  if (frame_index < script->count)
    *frame = script->frames[frame_index];
}
