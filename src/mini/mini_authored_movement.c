#include "mini_authored_movement.h"

#include <stdlib.h>

#include "ida_types.h"
#include "physics_config.h"
#include "variables.h"

enum {
  kMiniAuthoredSamusXRadius = 6,
  kMiniAuthoredSamusYRadius = 16,
  kMiniAuthoredBallXRadius = 7,
  kMiniAuthoredBallYRadius = 8,
  kMiniAuthoredDefaultRunSpeed = 3,
  kMiniAuthoredDefaultJumpSpeed = 6,
  kMiniAuthoredDefaultGravity = 1,
  kMiniAuthoredMaxFallFrames = 7,
  kMiniAuthoredSlopeSnapPixels = kBlockPixelSize,
  kMiniAuthoredBombSlot = 0,
  kMiniAuthoredBombFuseFrames = 8,
  kMiniAuthoredBombExplosionFrames = 3,
  kMiniAuthoredBombBlastRadiusX = 24,
  kMiniAuthoredBombBlastRadiusY = 32,
};

static int MiniAuthoredJumpSpeed(void);

static bool MiniAuthoredIsBallMovement(uint16 movement_type) {
  return movement_type == kMovementType_04_MorphBallOnGround ||
         movement_type == kMovementType_08_MorphBallFalling;
}

static int MiniAuthoredXRadiusForBallState(bool is_ball) {
  return is_ball ? kMiniAuthoredBallXRadius : kMiniAuthoredSamusXRadius;
}

static int MiniAuthoredYRadiusForBallState(bool is_ball) {
  return is_ball ? kMiniAuthoredBallYRadius : kMiniAuthoredSamusYRadius;
}

static void MiniAuthoredSetBallState(MiniGameState *state, bool is_ball) {
  int old_y_radius = state->samus.y_radius != 0
      ? state->samus.y_radius
      : kMiniAuthoredSamusYRadius;
  int foot_y = state->samus.world_y + old_y_radius - 1;
  state->samus.x_radius = (uint16)MiniAuthoredXRadiusForBallState(is_ball);
  state->samus.y_radius = (uint16)MiniAuthoredYRadiusForBallState(is_ball);
  state->samus.world_y = foot_y - state->samus.y_radius + 1;
}

bool MiniAuthoredMovement_ShouldUseRoom(const MiniRoomInfo *room) {
  return room->room_source == kMiniRoomSource_EditorExport &&
         !room->uses_rom_room &&
         !room->has_editor_room_visuals;
}

bool MiniAuthoredMovement_ShouldUseState(const MiniGameState *state) {
  return state->room.room_source == kMiniRoomSource_EditorExport &&
         !state->room.uses_rom_room &&
         !state->room.has_editor_room_visuals;
}

static bool MiniAuthoredMaterialBlocksSamus(BlockType material) {
  switch (material) {
  case kBlockType_Air:
  case kBlockType_Slope:
  case kBlockType_SpikeAir:
  case kBlockType_SpecialAir:
  case kBlockType_ShootableAir:
  case kBlockType_UnusedAir:
  case kBlockType_BombableAir:
  case kBlockType_Door:
    return false;
  default:
    return true;
  }
}

static bool MiniAuthoredSamusCollidesWithRadius(int world_x, int world_y,
                                                int x_radius, int y_radius) {
  int left = world_x - x_radius;
  int right = world_x + x_radius;
  int top = world_y - y_radius;
  int bottom = world_y + y_radius - 1;
  int xs[2] = { left, right };
  int ys[2] = { top, bottom };
  for (int yi = 0; yi < 2; yi++) {
    for (int xi = 0; xi < 2; xi++) {
      BlockType material = MiniStubs_GetCollisionMaterial(xs[xi] >> kBlockPixelShift,
                                                          ys[yi] >> kBlockPixelShift);
      if (MiniAuthoredMaterialBlocksSamus(material))
        return true;
    }
  }
  return false;
}

static bool MiniAuthoredSamusCollidesAt(const MiniGameState *state, int world_x, int world_y) {
  int x_radius = state->samus.x_radius != 0 ? state->samus.x_radius : kMiniAuthoredSamusXRadius;
  int y_radius = state->samus.y_radius != 0 ? state->samus.y_radius : kMiniAuthoredSamusYRadius;
  return MiniAuthoredSamusCollidesWithRadius(world_x, world_y, x_radius, y_radius);
}

static bool MiniAuthoredSlopeBlockMatches(int block_x, int block_y, uint8 slope_flags) {
  if (MiniStubs_GetCollisionMaterial(block_x, block_y) != kBlockType_Slope)
    return false;
  uint8 bts = MiniStubs_GetBts(block_x, block_y);
  return (bts & (kSlopeBts_MirrorX | kSlopeBts_Ceiling)) == slope_flags;
}

static bool MiniAuthoredSlopeGroundAt(int world_x, int world_y, int *ground_y) {
  int block_x = world_x >> kBlockPixelShift;
  int block_y = world_y >> kBlockPixelShift;
  if (MiniStubs_GetCollisionMaterial(block_x, block_y) != kBlockType_Slope)
    return false;

  uint8 bts = MiniStubs_GetBts(block_x, block_y);
  uint8 slope_flags = bts & (kSlopeBts_MirrorX | kSlopeBts_Ceiling);
  if ((slope_flags & kSlopeBts_Ceiling) != 0)
    return false;

  int segment_left_block = block_x;
  int segment_right_block = block_x;
  while (segment_left_block > 0 &&
         MiniAuthoredSlopeBlockMatches(segment_left_block - 1, block_y, slope_flags)) {
    segment_left_block--;
  }
  while (MiniAuthoredSlopeBlockMatches(segment_right_block + 1, block_y, slope_flags))
    segment_right_block++;

  int segment_left = segment_left_block << kBlockPixelShift;
  int segment_width = (segment_right_block - segment_left_block + 1) * kBlockPixelSize;
  int segment_max_x = segment_width - 1;
  int segment_x = world_x - segment_left;
  int slope_height = kBlockPixelMask;
  int local_y = (slope_flags & kSlopeBts_MirrorX) != 0
      ? (segment_x * slope_height + segment_max_x / 2) / segment_max_x
      : ((segment_max_x - segment_x) * slope_height + segment_max_x / 2) / segment_max_x;
  *ground_y = (block_y << kBlockPixelShift) + local_y;
  return true;
}

static bool MiniAuthoredFindSlopeGroundY(int world_x, int foot_y, int max_down,
                                         int *ground_y) {
  for (int probe_y = foot_y; probe_y <= foot_y + max_down; probe_y++) {
    if (MiniAuthoredSlopeGroundAt(world_x, probe_y, ground_y))
      return true;
  }
  return false;
}

static bool MiniAuthoredTrySnapToSlopeFloor(MiniGameState *state, int max_up, int max_down) {
  int y_radius = state->samus.y_radius != 0 ? state->samus.y_radius : kMiniAuthoredSamusYRadius;
  int foot_y = state->samus.world_y + y_radius - 1;
  int ground_y;
  if (!MiniAuthoredFindSlopeGroundY(state->samus.world_x, foot_y, max_down, &ground_y))
    return false;

  int delta = foot_y - ground_y;
  if (delta > max_up || -delta > max_down)
    return false;

  state->samus.world_y = ground_y - y_radius + 1;
  state->samus.y_velocity = 0;
  state->samus.on_ground = true;
  return true;
}

static bool MiniAuthoredSamusIsGrounded(const MiniGameState *state, int world_x, int world_y) {
  int y_radius = state->samus.y_radius != 0 ? state->samus.y_radius : kMiniAuthoredSamusYRadius;
  if (MiniAuthoredSamusCollidesAt(state, world_x, world_y + 1))
    return true;

  int foot_y = world_y + y_radius - 1;
  int ground_y;
  return MiniAuthoredFindSlopeGroundY(world_x, foot_y, 1, &ground_y) &&
         abs(foot_y - ground_y) <= 1;
}

static bool MiniAuthoredSamusTouchesWall(const MiniGameState *state, int direction) {
  return MiniAuthoredSamusCollidesAt(state, state->samus.world_x + direction,
                                     state->samus.world_y);
}

static bool MiniAuthoredBombIsActive(void) {
  return projectile_type[kMiniAuthoredBombSlot] == kProjectileType_Bomb;
}

static void MiniAuthoredClearBomb(void) {
  projectile_type[kMiniAuthoredBombSlot] = 0;
  projectile_dir[kMiniAuthoredBombSlot] = 0;
  projectile_x_pos[kMiniAuthoredBombSlot] = 0;
  projectile_y_pos[kMiniAuthoredBombSlot] = 0;
  projectile_x_radius[kMiniAuthoredBombSlot] = 0;
  projectile_y_radius[kMiniAuthoredBombSlot] = 0;
  projectile_damage[kMiniAuthoredBombSlot] = 0;
  projectile_bomb_instruction_ptr[kMiniAuthoredBombSlot] = 0;
  projectile_variables[kMiniAuthoredBombSlot] = 0;
  projectile_timers[kMiniAuthoredBombSlot] = 0;
  bomb_timers[kMiniAuthoredBombSlot] = 0;
  bomb_counter = 0;
}

static void MiniAuthoredPlaceBomb(const MiniGameState *state) {
  if (MiniAuthoredBombIsActive())
    return;

  int y_radius = state->samus.y_radius != 0 ? state->samus.y_radius : kMiniAuthoredSamusYRadius;
  int bomb_y = state->samus.world_y + y_radius - 1;
  projectile_type[kMiniAuthoredBombSlot] = kProjectileType_Bomb;
  projectile_dir[kMiniAuthoredBombSlot] = 0;
  projectile_x_pos[kMiniAuthoredBombSlot] = (uint16)state->samus.world_x;
  projectile_y_pos[kMiniAuthoredBombSlot] = (uint16)bomb_y;
  projectile_x_radius[kMiniAuthoredBombSlot] = 8;
  projectile_y_radius[kMiniAuthoredBombSlot] = 8;
  projectile_damage[kMiniAuthoredBombSlot] = 0;
  projectile_bomb_instruction_ptr[kMiniAuthoredBombSlot] = 0;
  projectile_variables[kMiniAuthoredBombSlot] = 0;
  projectile_timers[kMiniAuthoredBombSlot] = 0;
  bomb_timers[kMiniAuthoredBombSlot] = kMiniAuthoredBombFuseFrames;
  bomb_counter = 1;
}

static void MiniAuthoredMaybePlaceBomb(const MiniGameState *state, uint16 buttons) {
  bool bomb_pressed = (state->controls.new_buttons & kButton_X) != 0;
  bool down_held = (buttons & kButton_Down) != 0;
  if (bomb_pressed && down_held)
    MiniAuthoredPlaceBomb(state);
}

static void MiniAuthoredApplyBombBlast(MiniGameState *state) {
  int bomb_x = projectile_x_pos[kMiniAuthoredBombSlot];
  int bomb_y = projectile_y_pos[kMiniAuthoredBombSlot];
  int dx = state->samus.world_x - bomb_x;
  int dy = state->samus.world_y - bomb_y;
  if (abs(dx) > kMiniAuthoredBombBlastRadiusX || abs(dy) > kMiniAuthoredBombBlastRadiusY)
    return;

  state->samus.y_velocity = -MiniAuthoredJumpSpeed();
  state->samus.on_ground = false;
}

static void MiniAuthoredUpdateBomb(MiniGameState *state) {
  if (!MiniAuthoredBombIsActive())
    return;

  if (bomb_timers[kMiniAuthoredBombSlot] != 0) {
    bomb_timers[kMiniAuthoredBombSlot]--;
    if (bomb_timers[kMiniAuthoredBombSlot] != 0)
      return;

    projectile_damage[kMiniAuthoredBombSlot] = 1;
    projectile_timers[kMiniAuthoredBombSlot] = kMiniAuthoredBombExplosionFrames;
    projectile_x_radius[kMiniAuthoredBombSlot] = kMiniAuthoredBombBlastRadiusX;
    projectile_y_radius[kMiniAuthoredBombSlot] = kMiniAuthoredBombBlastRadiusY;
    MiniAuthoredApplyBombBlast(state);
    return;
  }

  if (projectile_timers[kMiniAuthoredBombSlot] != 0) {
    projectile_timers[kMiniAuthoredBombSlot]--;
    if (projectile_timers[kMiniAuthoredBombSlot] != 0)
      return;
  }
  MiniAuthoredClearBomb();
}

static int MiniAuthoredPairToPixels(uint16 whole, uint16 sub, int fallback) {
  int pixels = whole + (sub != 0 ? 1 : 0);
  return pixels > 0 ? pixels : fallback;
}

static int MiniAuthoredRunSpeed(void) {
  return MiniAuthoredPairToPixels(g_physics_params.run_max_speed,
                                  g_physics_params.run_max_speed_sub,
                                  kMiniAuthoredDefaultRunSpeed);
}

static int MiniAuthoredJumpSpeed(void) {
  return MiniAuthoredPairToPixels(g_physics_params.jump_hi_initial_speed[0],
                                  g_physics_params.jump_hi_initial_subspeed[0],
                                  kMiniAuthoredDefaultJumpSpeed);
}

static int MiniAuthoredGravity(void) {
  return MiniAuthoredPairToPixels(g_physics_params.gravity_accel,
                                  g_physics_params.gravity_subaccel,
                                  kMiniAuthoredDefaultGravity);
}

static int MiniAuthoredMaxFallVelocity(void) {
  return MiniAuthoredGravity() * kMiniAuthoredMaxFallFrames;
}

static void MiniAuthoredMoveHorizontal(MiniGameState *state, int velocity) {
  int step = velocity < 0 ? -1 : 1;
  for (int remaining = abs(velocity); remaining != 0; remaining--) {
    int next_x = state->samus.world_x + step;
    if (MiniAuthoredSamusCollidesAt(state, next_x, state->samus.world_y))
      break;
    state->samus.world_x = next_x;
  }
}

static void MiniAuthoredMoveVertical(MiniGameState *state) {
  int start_y_velocity = state->samus.y_velocity;
  int step = state->samus.y_velocity < 0 ? -1 : 1;
  for (int remaining = abs(state->samus.y_velocity); remaining != 0; remaining--) {
    int next_y = state->samus.world_y + step;
    if (MiniAuthoredSamusCollidesAt(state, state->samus.world_x, next_y)) {
      state->samus.y_velocity = 0;
      state->samus.on_ground = step > 0;
      return;
    }
    state->samus.world_y = next_y;
  }
  if (start_y_velocity >= 0 &&
      MiniAuthoredTrySnapToSlopeFloor(state, kMiniAuthoredSlopeSnapPixels,
                                      abs(start_y_velocity) + 1)) {
    return;
  }
  state->samus.on_ground = MiniAuthoredSamusIsGrounded(state, state->samus.world_x,
                                                       state->samus.world_y);
}

static int MiniAuthoredClampInt(int value, int min_value, int max_value) {
  if (value < min_value)
    return min_value;
  if (value > max_value)
    return max_value;
  return value;
}

static void MiniAuthoredFollowCamera(const MiniGameState *state) {
  int max_x = state->room.room_width_blocks * kMiniBlockSize - state->viewport.width;
  int max_y = state->room.room_height_blocks * kMiniBlockSize - state->viewport.height;
  int target_x = state->viewport.width * state->room.camera_target_x_percent / 100;
  int target_y = state->viewport.height * state->room.camera_target_y_percent / 100;
  int camera_x = state->samus.world_x - target_x;
  int camera_y = state->samus.world_y - target_y;
  layer1_x_pos = MiniAuthoredClampInt(camera_x, 0, max_x > 0 ? max_x : 0);
  layer1_y_pos = MiniAuthoredClampInt(camera_y, 0, max_y > 0 ? max_y : 0);
  layer1_x_subpos = 0;
  layer1_y_subpos = 0;
}

static void MiniAuthoredSetCameraForDoorway(const MiniGameState *state,
                                            const MiniDoorwayTransition *doorway) {
  int max_x = state->room.room_width_blocks * kMiniBlockSize - state->viewport.width;
  int max_y = state->room.room_height_blocks * kMiniBlockSize - state->viewport.height;
  layer1_x_pos = MiniAuthoredClampInt(doorway->camera_x, 0, max_x > 0 ? max_x : 0);
  layer1_y_pos = MiniAuthoredClampInt(doorway->camera_y, 0, max_y > 0 ? max_y : 0);
  layer1_x_subpos = 0;
  layer1_y_subpos = 0;
}

static bool MiniAuthoredTryDoorwayTransition(MiniGameState *state) {
  int block_x = state->samus.world_x >> kBlockPixelShift;
  int y_radius = state->samus.y_radius != 0 ? state->samus.y_radius : kMiniAuthoredSamusYRadius;
  int foot_y = state->samus.world_y + y_radius - 1;
  int block_y = foot_y >> kBlockPixelShift;

  if (MiniStubs_GetCollisionMaterial(block_x, block_y) != kBlockType_Door)
    return false;

  int doorway_count = state->room.doorway_count;
  if (doorway_count > kMiniDoorwayTransitionCapacity)
    doorway_count = kMiniDoorwayTransitionCapacity;
  for (int i = 0; i < doorway_count; i++) {
    const MiniDoorwayTransition *doorway = &state->room.doorways[i];
    if (!doorway->active ||
        doorway->source_block_x != block_x ||
        doorway->source_block_y != block_y) {
      continue;
    }

    state->samus.world_x = doorway->destination_x;
    state->samus.world_y = doorway->destination_y;
    state->samus.x_velocity = 0;
    state->samus.y_velocity = 0;
    state->samus.on_ground = MiniAuthoredSamusIsGrounded(state, state->samus.world_x,
                                                         state->samus.world_y);
    MiniAuthoredSetCameraForDoorway(state, doorway);
    return true;
  }
  return false;
}

void MiniAuthoredMovement_InitializeSamusGlobals(void) {
  samus_x_radius = kMiniAuthoredSamusXRadius;
  samus_y_radius = kMiniAuthoredSamusYRadius;
  samus_pose = kPose_01_FaceR_Normal;
  samus_movement_type = kMovementType_00_Standing;
  MiniAuthoredClearBomb();
}

void MiniAuthoredMovement_SyncGrounded(MiniGameState *state) {
  state->samus.on_ground = MiniAuthoredSamusIsGrounded(state, state->samus.world_x,
                                                       state->samus.world_y);
}

void MiniAuthoredMovement_Step(MiniGameState *state) {
  uint16 buttons = state->controls.buttons;
  bool left = (buttons & kButton_Left) != 0;
  bool right = (buttons & kButton_Right) != 0;
  bool jump_pressed = (state->controls.new_buttons & kButton_A) != 0;
  bool shoot_pressed = (state->controls.new_buttons & kButton_X) != 0;
  bool is_ball = MiniAuthoredIsBallMovement(state->samus.movement_type);
  int gravity = MiniAuthoredGravity();
  int max_fall_velocity = MiniAuthoredMaxFallVelocity();
  int run_speed = MiniAuthoredRunSpeed();

  if ((state->controls.new_buttons & kButton_Down) != 0 && !shoot_pressed && state->samus.on_ground)
    is_ball = true;
  if ((state->controls.new_buttons & kButton_Up) != 0 && is_ball) {
    int foot_y = state->samus.world_y + state->samus.y_radius - 1;
    int stand_y = foot_y - kMiniAuthoredSamusYRadius + 1;
    if (!MiniAuthoredSamusCollidesWithRadius(state->samus.world_x, stand_y,
                                             kMiniAuthoredSamusXRadius,
                                             kMiniAuthoredSamusYRadius)) {
      is_ball = false;
    }
  }
  MiniAuthoredSetBallState(state, is_ball);

  state->samus.on_ground = MiniAuthoredSamusIsGrounded(state, state->samus.world_x,
                                                       state->samus.world_y);
  bool touching_left_wall = MiniAuthoredSamusTouchesWall(state, -1);
  bool touching_right_wall = MiniAuthoredSamusTouchesWall(state, 1);
  state->samus.x_velocity = right == left ? 0 : (right ? run_speed : -run_speed);
  if (jump_pressed && state->samus.on_ground && !is_ball) {
    state->samus.y_velocity = -MiniAuthoredJumpSpeed();
    state->samus.on_ground = false;
  } else if (jump_pressed && !is_ball && !state->samus.on_ground &&
             (touching_left_wall || touching_right_wall)) {
    state->samus.y_velocity = -MiniAuthoredJumpSpeed();
    state->samus.x_velocity = touching_right_wall && !touching_left_wall ? -run_speed : run_speed;
    state->samus.on_ground = false;
  } else if (!state->samus.on_ground) {
    state->samus.y_velocity += gravity;
    if (state->samus.y_velocity > max_fall_velocity)
      state->samus.y_velocity = max_fall_velocity;
  } else {
    state->samus.y_velocity = 0;
  }

  MiniAuthoredUpdateBomb(state);
  MiniAuthoredMaybePlaceBomb(state, buttons);
  MiniAuthoredMoveHorizontal(state, state->samus.x_velocity);
  MiniAuthoredMoveVertical(state);
  bool doorway_transitioned = MiniAuthoredTryDoorwayTransition(state);

  if (is_ball) {
    state->samus.movement_type = state->samus.on_ground ? kMovementType_04_MorphBallOnGround
                                                        : kMovementType_08_MorphBallFalling;
    state->samus.pose = state->samus.x_velocity < 0 ? kPose_41_FaceL_Morphball_Ground
                                                    : kPose_1D_FaceR_Morphball_Ground;
  } else if (!state->samus.on_ground) {
    state->samus.movement_type = kMovementType_03_SpinJumping;
    state->samus.pose = state->samus.x_velocity < 0 ? kPose_1A_FaceL_SpinJump
                                                    : kPose_19_FaceR_SpinJump;
  } else if (state->samus.x_velocity < 0) {
    state->samus.movement_type = kMovementType_01_Running;
    state->samus.pose = kPose_0A_MoveL_NoAim;
  } else if (state->samus.x_velocity > 0) {
    state->samus.movement_type = kMovementType_01_Running;
    state->samus.pose = kPose_09_MoveR_NoAim;
  } else {
    state->samus.movement_type = kMovementType_00_Standing;
    state->samus.pose = kPose_01_FaceR_Normal;
  }

  samus_x_radius = state->samus.x_radius;
  samus_y_radius = state->samus.y_radius;
  samus_x_pos = state->samus.world_x;
  samus_y_pos = state->samus.world_y;
  samus_pose = state->samus.pose;
  samus_movement_type = state->samus.movement_type;
  if (!doorway_transitioned)
    MiniAuthoredFollowCamera(state);
}
