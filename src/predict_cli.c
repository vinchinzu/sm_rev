#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "mini/mini_game.h"
#include "mini/mini_predict.h"
#include "mini/mini_defs.h"
#include "third_party/cJSON.h"

// retro_rl wire format: standard SNES button bit order
// Button order: B=0, Y=1, Select=2, Start=3, Up=4, Down=5, Left=6, Right=7, A=8, X=9, L=10, R=11
#define WIRE_BUTTON_B       0x01    // bit 0
#define WIRE_BUTTON_Y       0x02    // bit 1
#define WIRE_BUTTON_SELECT  0x04    // bit 2
#define WIRE_BUTTON_START   0x08    // bit 3
#define WIRE_BUTTON_UP      0x10    // bit 4
#define WIRE_BUTTON_DOWN    0x20    // bit 5
#define WIRE_BUTTON_LEFT    0x40    // bit 6
#define WIRE_BUTTON_RIGHT   0x80    // bit 7
#define WIRE_BUTTON_A       0x100   // bit 8
#define WIRE_BUTTON_X       0x200   // bit 9
#define WIRE_BUTTON_L       0x400   // bit 10
#define WIRE_BUTTON_R       0x800   // bit 11

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
  // Handle --version (SmRevClient availability check, no stdin required)
  if (argc > 1 && strcmp(argv[1], "--version") == 0) {
    printf("sm_rev_predict 0.1.0-mini-baseline\n");
    return 0;
  }
  
  // SmRevClient invokes as: [SM_REV_PATH, "predict"]
  // Accept optional "predict" arg for compatibility (no-op)
  if (argc > 1 && strcmp(argv[1], "predict") == 0) {
    // Expected usage, continue
  } else if (argc > 1) {
    fprintf(stderr, "{\"error\":\"unknown command '%s' (expected 'predict', '--version', or no args)\"}\n", argv[1]);
    return 1;
  }

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

  // Parse input sequence (wire format: [{"buttons": int}, ...])
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
  uint16_t *wire_buttons = malloc(input_count * sizeof(uint16_t));
  if (!mini_buttons || !wire_buttons) {
    free(mini_buttons);
    free(wire_buttons);
    cJSON_Delete(root);
    fprintf(stderr, "{\"error\":\"allocation failed\"}\n");
    return 1;
  }

  for (size_t i = 0; i < input_count; i++) {
    cJSON *input_item = cJSON_GetArrayItem(inputs_json, (int)i);
    if (!cJSON_IsObject(input_item)) {
      free(mini_buttons);
      free(wire_buttons);
      cJSON_Delete(root);
      fprintf(stderr, "{\"error\":\"invalid input at index %zu (expected object)\"}\n", i);
      return 1;
    }
    cJSON *buttons_json = cJSON_GetObjectItem(input_item, "buttons");
    if (!buttons_json || !cJSON_IsNumber(buttons_json)) {
      free(mini_buttons);
      free(wire_buttons);
      cJSON_Delete(root);
      fprintf(stderr, "{\"error\":\"missing 'buttons' field at input index %zu\"}\n", i);
      return 1;
    }
    wire_buttons[i] = (uint16_t)buttons_json->valueint;
    mini_buttons[i] = wire_to_mini_buttons(wire_buttons[i]);
  }

  // Parse optional start state (wire format: SimState.to_dict())
  // LIMITATION: Mini can only load from binary snapshots (MiniStateSnapshot), not from
  // arbitrary SimState JSON. The start state is echoed back for wire compatibility, but
  // prediction currently begins from a fixed initial Mini room state.
  // To predict from arbitrary states, caller must provide a pre-saved binary snapshot.
  cJSON *start_json = cJSON_GetObjectItem(root, "start");
  // Keep start_json reference valid by not deleting root yet

  // Run prediction
  MiniPrediction *prediction = MiniPrediction_Create(input_count);
  if (!prediction) {
    free(mini_buttons);
    free(wire_buttons);
    fprintf(stderr, "{\"error\":\"MiniPrediction_Create failed\"}\n");
    return 1;
  }

  bool success = MiniPredict(
    prediction,
    NULL,  // TODO: pass snapshot when state loading is implemented
    0,
    mini_buttons,
    input_count,
    kMiniGameWidth,
    kMiniGameHeight
  );

  free(mini_buttons);

  if (!success) {
    free(wire_buttons);
    MiniPrediction_Destroy(prediction);
    fprintf(stderr, "{\"error\":\"MiniPredict failed\"}\n");
    return 1;
  }

  // Output Trajectory.to_dict() (retro_rl wire format)
  printf("{");
  
  // start: SimState.to_dict() - echo requested start state for wire compatibility
  // LIMITATION: Mini cannot hydrate from this JSON; prediction starts from fixed initial state
  if (start_json) {
    // Echo the provided start state
    char *start_str = cJSON_PrintUnformatted(start_json);
    printf("\"start\":%s,", start_str);
    free(start_str);
  } else {
    // No start provided, use first trajectory frame as placeholder
    const MiniTrajectoryFrame *first = prediction->frame_count > 0 ? &prediction->frames[0] : NULL;
    if (first) {
      printf("\"start\":{");
      printf("\"frame\":0,");
      printf("\"room_id\":%d,", first->room_id);
      printf("\"samus_x\":%d,", first->samus_x);
      printf("\"samus_y\":%d,", first->samus_y);
      printf("\"samus_x_sub\":%d,", first->samus_x_sub);
      printf("\"samus_y_sub\":%d,", first->samus_y_sub);
      printf("\"velocity_x\":%d,", first->velocity_x);
      printf("\"velocity_y\":%d,", first->velocity_y);
      printf("\"velocity_x_sub\":%d,", first->velocity_x_sub);
      printf("\"velocity_y_sub\":%d,", first->velocity_y_sub);
      printf("\"momentum_x\":%d,", first->momentum_x);
      printf("\"momentum_x_sub\":%d,", first->momentum_x_sub);
      printf("\"pose\":%u,", first->pose);
      printf("\"facing\":%u,", first->facing);
      printf("\"movement_type\":%u,", first->movement_type);
      printf("\"speed_counter\":%u,", first->speed_counter);
      printf("\"speed_flag\":%u,", first->speed_flag);
      printf("\"shinespark_timer\":%u", first->shinespark_timer);
      printf("},");
    } else {
      printf("\"start\":null,");
    }
  }

  // frames: [TrajectoryFrame.to_dict(), ...]
  printf("\"frames\":[");
  for (size_t i = 0; i < prediction->frame_count; i++) {
    const MiniTrajectoryFrame *frame = &prediction->frames[i];
    if (i > 0) printf(",");
    printf("{");
    printf("\"frame\":%d,", frame->frame);
    printf("\"room_id\":%d,", frame->room_id);
    printf("\"samus_x\":%d,", frame->samus_x);
    printf("\"samus_y\":%d,", frame->samus_y);
    printf("\"samus_x_sub\":%d,", frame->samus_x_sub);
    printf("\"samus_y_sub\":%d,", frame->samus_y_sub);
    printf("\"velocity_x\":%d,", frame->velocity_x);
    printf("\"velocity_y\":%d,", frame->velocity_y);
    printf("\"velocity_x_sub\":%d,", frame->velocity_x_sub);
    printf("\"velocity_y_sub\":%d,", frame->velocity_y_sub);
    printf("\"momentum_x\":%d,", frame->momentum_x);
    printf("\"momentum_x_sub\":%d,", frame->momentum_x_sub);
    printf("\"pose\":%u,", frame->pose);
    printf("\"facing\":%u,", frame->facing);
    printf("\"movement_type\":%u,", frame->movement_type);
    printf("\"speed_counter\":%u,", frame->speed_counter);
    printf("\"speed_flag\":%u,", frame->speed_flag);
    printf("\"shinespark_timer\":%u", frame->shinespark_timer);
    // Omit enemies when empty (per wire format spec)
    printf("}");
  }
  printf("],");

  // predictor: string name
  printf("\"predictor\":\"sm_rev\",");

  // inputs: [{"buttons": int}, ...]
  printf("\"inputs\":[");
  for (size_t i = 0; i < input_count; i++) {
    if (i > 0) printf(",");
    printf("{\"buttons\":%u}", wire_buttons[i]);
  }
  printf("]");

  free(wire_buttons);
  printf("}\n");

  MiniPrediction_Destroy(prediction);
  cJSON_Delete(root);  // Now safe to delete after using start_json
  return 0;
}
