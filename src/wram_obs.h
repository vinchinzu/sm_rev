#ifndef SM_WRAM_OBS_H_
#define SM_WRAM_OBS_H_

#include <stdio.h>

#include "types.h"
#include "variables.h"

// Residual-relevant WRAM peek. Pixel/subpixel words are the M–E contract:
//   $0AF6 samus_x, $0AF8 samus_x_sub, $0AFA samus_y, $0AFC samus_y_sub
typedef struct WramSamusObs {
  uint16 samus_x;
  uint16 samus_x_sub;
  uint16 samus_y;
  uint16 samus_y_sub;
  uint16 pose;
  uint8 facing;
  uint8 movement_type;
  uint16 room;
  int16 velocity_x;
  uint16 velocity_x_sub;
  int16 velocity_y;
  uint16 velocity_y_sub;
  uint16 energy;
  uint16 time_frames;
  uint16 frame_counter;
} WramSamusObs;

static inline WramSamusObs Wram_PeekSamus(void) {
  WramSamusObs obs;
  obs.samus_x = samus_x_pos;
  obs.samus_x_sub = samus_x_subpos;
  obs.samus_y = samus_y_pos;
  obs.samus_y_sub = samus_y_subpos;
  obs.pose = samus_pose;
  obs.facing = samus_pose_x_dir;
  obs.movement_type = samus_movement_type;
  obs.room = room_ptr;
  obs.velocity_x = (int16)samus_x_extra_run_speed;
  obs.velocity_x_sub = samus_x_extra_run_subspeed;
  obs.velocity_y = (int16)samus_y_speed;
  obs.velocity_y_sub = samus_y_subspeed;
  obs.energy = samus_health;
  obs.time_frames = game_time_frames;
  obs.frame_counter = frame_counter_every_frame;
  return obs;
}

static inline void Wram_WriteJsonl(FILE *f, int frame, const char *source) {
  WramSamusObs obs = Wram_PeekSamus();
  if (f == NULL)
    return;
  fprintf(f,
          "{\"frame\":%d,\"source\":\"%s\","
          "\"samus_x\":%u,\"samus_y\":%u,"
          "\"samus_x_sub\":%u,\"samus_y_sub\":%u,"
          "\"pose\":%u,\"facing\":%u,\"movement_type\":%u,\"room_ptr\":%u,"
          "\"velocity_x\":%d,\"velocity_y\":%d,"
          "\"velocity_x_sub\":%u,\"velocity_y_sub\":%u,"
          "\"energy\":%u,\"game_time_frames\":%u,\"frame_counter\":%u}\n",
          frame, source != NULL ? source : "unknown",
          (unsigned)obs.samus_x, (unsigned)obs.samus_y,
          (unsigned)obs.samus_x_sub, (unsigned)obs.samus_y_sub,
          (unsigned)obs.pose, (unsigned)obs.facing, (unsigned)obs.movement_type,
          (unsigned)obs.room,
          (int)obs.velocity_x, (int)obs.velocity_y,
          (unsigned)obs.velocity_x_sub, (unsigned)obs.velocity_y_sub,
          (unsigned)obs.energy, (unsigned)obs.time_frames,
          (unsigned)obs.frame_counter);
}

#endif  // SM_WRAM_OBS_H_
