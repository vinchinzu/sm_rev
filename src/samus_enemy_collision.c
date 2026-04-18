#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "enemy_types.h"

typedef struct PositionAndWidth {
  uint16 x_pos;
  uint16 x_subpos;
  uint16 y_pos;
  uint16 y_subpos;
  uint16 width;
  uint16 height;
} PositionAndWidth;

CheckEnemyColl_Result Samus_CheckSolidEnemyColl(int32 amt) {  // 0xA0A8F0
  int16 distance_to_collision;
  uint16 delta_pos = amt >> 16, delta_subpos = amt;
  uint16 enemy_edge;
  PositionAndWidth next_pos;

  if (!interactive_enemy_indexes_write_ptr)
    return (CheckEnemyColl_Result){0, amt};

  switch (samus_collision_direction & 3) {
  case 0: {
    next_pos.x_pos = samus_x_pos - delta_pos;
    bool subpos_aligned = samus_x_subpos == delta_subpos;
    if (samus_x_subpos < delta_subpos)
      subpos_aligned = next_pos.x_pos-- == 1;
    if (!subpos_aligned)
      --next_pos.x_pos;
    next_pos.y_pos = samus_y_pos;
    next_pos.y_subpos = samus_y_subpos;
    break;
  }
  case 1: {
    next_pos.x_pos = samus_x_pos + delta_pos;
    bool subpos_aligned = samus_x_subpos + delta_subpos == 0;
    if (__CFADD__uint16(samus_x_subpos, delta_subpos))
      subpos_aligned = next_pos.x_pos++ == 0xFFFF;
    if (!subpos_aligned)
      ++next_pos.x_pos;
    next_pos.y_pos = samus_y_pos;
    next_pos.y_subpos = samus_y_subpos;
    break;
  }
  case 2: {
    next_pos.y_pos = samus_y_pos - delta_pos;
    bool subpos_aligned = samus_y_subpos == delta_subpos;
    if (samus_y_subpos < delta_subpos)
      subpos_aligned = next_pos.y_pos-- == 1;
    if (!subpos_aligned)
      --next_pos.y_pos;
    next_pos.x_pos = samus_x_pos;
    next_pos.x_subpos = samus_x_subpos;
    break;
  }
  case 3: {
    next_pos.y_pos = samus_y_pos + delta_pos;
    bool subpos_aligned = samus_y_subpos + delta_subpos == 0;
    if (__CFADD__uint16(samus_y_subpos, delta_subpos))
      subpos_aligned = next_pos.y_pos++ == 0xFFFF;
    if (!subpos_aligned)
      ++next_pos.y_pos;
    next_pos.x_pos = samus_x_pos;
    next_pos.x_subpos = samus_x_subpos;
    break;
  }
  default:
    Unreachable();
  }

  next_pos.width = samus_x_radius;
  next_pos.height = samus_y_radius;
  collision_detection_index = 0;
  for (int i = 0;; i++) {
    uint16 enemy_index = interactive_enemy_indexes[i];
    if (enemy_index == 0xFFFF)
      break;

    collision_detection_index = enemy_index;
    EnemyData *enemy = gEnemyData(enemy_index);
    if (!enemy->frozen_timer && (enemy->properties & 0x8000) == 0)
      continue;

    uint16 x_delta = abs16(enemy->x_pos - next_pos.x_pos);
    bool x_overlaps = x_delta < enemy->x_width;
    uint16 x_gap = x_delta - enemy->x_width;
    if (!x_overlaps && x_gap >= next_pos.width)
      continue;

    uint16 y_delta = abs16(enemy->y_pos - next_pos.y_pos);
    bool y_overlaps = y_delta < enemy->y_height;
    uint16 y_gap = y_delta - enemy->y_height;
    if (!y_overlaps && y_gap >= next_pos.height)
      continue;

    switch (samus_collision_direction & 3) {
    case 0:
      enemy_edge = enemy->x_width + enemy->x_pos;
      distance_to_collision = samus_x_pos - samus_x_radius - enemy_edge;
      if (samus_x_pos - samus_x_radius == enemy_edge)
        goto hit_now;
      if (distance_to_collision >= 0)
        goto hit_with_gap;
      break;
    case 1:
      enemy_edge = samus_x_radius + samus_x_pos;
      distance_to_collision = enemy->x_pos - enemy->x_width - enemy_edge;
      if (!distance_to_collision)
        goto hit_now;
      if (distance_to_collision >= 0)
        goto hit_with_gap;
      break;
    case 2:
      enemy_edge = enemy->y_height + enemy->y_pos;
      distance_to_collision = samus_y_pos - samus_y_radius - enemy_edge;
      if (samus_y_pos - samus_y_radius == enemy_edge)
        goto hit_now;
      if (distance_to_collision >= 0)
        goto hit_with_gap;
      break;
    case 3:
      enemy_edge = samus_y_radius + samus_y_pos;
      distance_to_collision = enemy->y_pos - enemy->y_height - enemy_edge;
      if (!distance_to_collision)
        goto hit_now;
      if (distance_to_collision >= 0)
        goto hit_with_gap;
      break;
    default:
      Unreachable();
    }
  }

  return (CheckEnemyColl_Result){0, amt};

hit_now:
  samus_y_subpos = 0;
  enemy_index_colliding_dirs[samus_collision_direction & 3] = collision_detection_index;
  return (CheckEnemyColl_Result){-1, 0};

hit_with_gap:
  enemy_index_colliding_dirs[samus_collision_direction & 3] = collision_detection_index;
  return (CheckEnemyColl_Result){-1, INT16_SHL16(distance_to_collision)};
}
