#include "mini_editor_camera.h"

#include "funcs.h"
#include "mini_run_mode.h"
#include "variables.h"

static int MiniEditorCamera_Clamp(int value, int min_value, int max_value) {
  if (value < min_value)
    return min_value;
  if (value > max_value)
    return max_value;
  return value;
}

bool MiniEditorCamera_ShouldUseState(const MiniGameState *state) {
  if (state == NULL)
    return false;
  return state->room.room_source == kMiniRoomSource_EditorExport &&
         !state->room.uses_rom_room &&
         state->room.has_editor_room_visuals;
}

void MiniEditorCamera_Follow(MiniGameState *state) {
  int max_x = state->room.room_width_blocks * kMiniBlockSize - state->viewport.width;
  int max_y = state->room.room_height_blocks * kMiniBlockSize - state->viewport.height;
  int target_x = state->viewport.width * state->room.camera_target_x_percent / 100;
  int camera_x = (int)samus_x_pos - target_x;
  int camera_y;
  if (MiniRunMode_IsClimbEndless()) {
    camera_y = (int)samus_y_pos - down_scroller;
  } else {
    int target_y = state->viewport.height * state->room.camera_target_y_percent / 100;
    camera_y = (int)samus_y_pos - target_y;
  }

  layer1_x_pos = (uint16)MiniEditorCamera_Clamp(camera_x, 0, max_x > 0 ? max_x : 0);
  layer1_y_pos = (uint16)MiniEditorCamera_Clamp(camera_y, 0, max_y > 0 ? max_y : 0);
  ideal_layer1_xpos = layer1_x_pos;
  ideal_layer1_ypos = layer1_y_pos;
  layer1_x_subpos = 0;
  layer1_y_subpos = 0;
  CalculateLayer2Xpos();
  CalculateLayer2Ypos();
  CalculateBgScrolls();
}
