#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "mini/mini_game.h"
#include "mini/mini_predict.h"
#include "mini/mini_defs.h"
#include "third_party/cJSON.h"

// retro_rl wire format: standard SNES button bit order
// Wire format from retro_rl#1: Left=0x40, Right=0x80, etc.
#define WIRE_BUTTON_B       0x01
#define WIRE_BUTTON_Y       0x02
#define WIRE_BUTTON_SELECT  0x04
#define WIRE_BUTTON_START   0x08
#define WIRE_BUTTON_UP      0x10
#define WIRE_BUTTON_DOWN    0x20
#define WIRE_BUTTON_LEFT    0x40
#define WIRE_BUTTON_RIGHT   0x80
#define WIRE_BUTTON_A       0x100
#define WIRE_BUTTON_X       0x200
#define WIRE_BUTTON_L       0x400
#define WIRE_BUTTON_R       0x800

// Internal mini button format (from ida_types.h)
#define MINI_BUTTON_B       0x8000
#define MINI_BUTTON_Y       0x4000
#define MINI_BUTTON_SELECT  0x2000
#define MINI_BUTTON_START   0x1000
#define MINI_BUTTON_UP      0x800
#define MINI_BUTTON_DOWN    0x400
#define MINI_BUTTON_LEFT    0x200
#define MINI_BUTTON_RIGHT   0x100
#define MINI_BUTTON_A       0x80
#define MINI_BUTTON_X       0x40
#define MINI_BUTTON_L       0x20
#define MINI_BUTTON_R       0x10

static uint16_t wire_to_mini_buttons(uint16_t wire) {
  uint16_t mini = 0;
  if (wire & WIRE_BUTTON_B)      mini |= MINI_BUTTON_B;
  if (wire & WIRE_BUTTON_Y)      mini |= MINI_BUTTON_Y;
  if (wire & WIRE_BUTTON_SELECT) mini |= MINI_BUTTON_SELECT;
  if (wire & WIRE_BUTTON_START)  mini |= MINI_BUTTON_START;
  if (wire & WIRE_BUTTON_UP)     mini |= MINI_BUTTON_UP;
  if (wire & WIRE_BUTTON_DOWN)   mini |= MINI_BUTTON_DOWN;
  if (wire & WIRE_BUTTON_LEFT)   mini |= MINI_BUTTON_LEFT;
  if (wire & WIRE_BUTTON_RIGHT)  mini |= MINI_BUTTON_RIGHT;
  if (wire & WIRE_BUTTON_A)      mini |= MINI_BUTTON_A;
  if (wire & WIRE_BUTTON_X)      mini |= MINI_BUTTON_X;
  if (wire & WIRE_BUTTON_L)      mini |= MINI_BUTTON_L;
  if (wire & WIRE_BUTTON_R)      mini |= MINI_BUTTON_R;
  return mini;
}

static char* read_stdin(void) {
  size_t capacity = 4096;
  size_t size = 0;
  char *buffer = malloc(capacity);
  if (!buffer) return NULL;

  int c;
  while ((c = fgetc(stdin)) != EOF) {
    if (size + 1 >= capacity) {
      capacity *= 2;
      char *new_buffer = realloc(buffer, capacity);
      if (!new_buffer) {
        free(buffer);
        return NULL;
      }
      buffer = new_buffer;
    }
    buffer[size++] = (char)c;
  }
  buffer[size] = '\0';
  return buffer;
}

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  char *json_input = read_stdin();
  if (!json_input) {
    fprintf(stderr, "{\"error\":\"failed to read stdin\"}\n");
    return 1;
  }

  cJSON *root = cJSON_Parse(json_input);
  free(json_input);
  if (!root) {
    fprintf(stderr, "{\"error\":\"invalid JSON input\"}\n");
    return 1;
  }

  // Parse input sequence
  cJSON *inputs_json = cJSON_GetObjectItem(root, "inputs");
  if (!inputs_json || !cJSON_IsArray(inputs_json)) {
    cJSON_Delete(root);
    fprintf(stderr, "{\"error\":\"missing or invalid 'inputs' array\"}\n");
    return 1;
  }

  size_t input_count = cJSON_GetArraySize(inputs_json);
  if (input_count == 0) {
    cJSON_Delete(root);
    fprintf(stderr, "{\"error\":\"empty inputs array\"}\n");
    return 1;
  }

  uint16_t *mini_buttons = malloc(input_count * sizeof(uint16_t));
  if (!mini_buttons) {
    cJSON_Delete(root);
    fprintf(stderr, "{\"error\":\"allocation failed\"}\n");
    return 1;
  }

  for (size_t i = 0; i < input_count; i++) {
    cJSON *input_item = cJSON_GetArrayItem(inputs_json, (int)i);
    if (!cJSON_IsNumber(input_item)) {
      free(mini_buttons);
      cJSON_Delete(root);
      fprintf(stderr, "{\"error\":\"invalid input at index %zu\"}\n", i);
      return 1;
    }
    uint16_t wire_buttons = (uint16_t)input_item->valueint;
    mini_buttons[i] = wire_to_mini_buttons(wire_buttons);
  }

  // Optional: parse initial state snapshot if provided
  cJSON *state_json = cJSON_GetObjectItem(root, "state");
  void *snapshot = NULL;
  size_t snapshot_size = 0;
  if (state_json && cJSON_IsString(state_json)) {
    // TODO: decode base64 or hex state if needed
    // For now, always start from fresh state
  }

  cJSON_Delete(root);

  // Run prediction
  MiniPrediction *prediction = MiniPrediction_Create(input_count);
  if (!prediction) {
    free(mini_buttons);
    fprintf(stderr, "{\"error\":\"MiniPrediction_Create failed\"}\n");
    return 1;
  }

  bool success = MiniPredict(
    prediction,
    snapshot,
    snapshot_size,
    mini_buttons,
    input_count,
    kMiniGameWidth,
    kMiniGameHeight
  );

  free(mini_buttons);

  if (!success) {
    MiniPrediction_Destroy(prediction);
    fprintf(stderr, "{\"error\":\"MiniPredict failed\"}\n");
    return 1;
  }

  // Output Trajectory JSON in snake_case (retro_rl wire format)
  printf("{\"trajectory\":[");
  for (size_t i = 0; i < prediction->frame_count; i++) {
    const MiniTrajectoryFrame *frame = &prediction->frames[i];
    if (i > 0) printf(",");
    printf("\n  {");
    printf("\"frame\":%d,", frame->frame);
    printf("\"samus_x\":%d,", frame->world_x);
    printf("\"samus_x_sub\":%d,", frame->x_subpos);
    printf("\"samus_y\":%d,", frame->world_y);
    printf("\"samus_y_sub\":%d,", frame->y_subpos);
    printf("\"velocity_x\":%d,", frame->x_velocity);
    printf("\"velocity_y\":%d,", frame->y_velocity);
    printf("\"velocity_x_extra\":%d,", frame->x_extra_run_speed);
    printf("\"velocity_y_speed\":%d,", frame->y_speed);
    printf("\"pose\":%u,", frame->pose);
    printf("\"movement_type\":%u,", frame->movement_type);
    printf("\"on_ground\":%s,", frame->on_ground ? "true" : "false");
    printf("\"room_id\":0,");  // TODO: populate when room system is integrated
    printf("\"state_hash\":%llu,", (unsigned long long)frame->state_hash);
    printf("\"enemies\":[]");  // Empty for now, wire format placeholder
    printf("}");
  }
  printf("\n]}\n");

  MiniPrediction_Destroy(prediction);
  return 0;
}
