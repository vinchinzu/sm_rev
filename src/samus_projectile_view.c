#include "samus_projectile_view.h"

#include <string.h>

#include "ida_types.h"
#include "variables.h"

bool SamusProjectile_IsBeamType(uint16 type) {
  return type != 0 && (type & kProjectileType_TypeMask) == 0;
}

bool SamusProjectile_IsBasicBeamType(uint16 type) {
  return SamusProjectile_IsBeamType(type) && (type & 0x3F) == 0;
}

static bool SamusProjectile_ReadSlot(int slot, SamusProjectileView *view) {
  if ((unsigned)slot >= kSamusProjectileSlotCount)
    return false;

  bool active = projectile_type[slot] != 0
             || projectile_damage[slot] != 0
             || projectile_bomb_instruction_ptr[slot] != 0;
  if (!active)
    return false;

  uint16 type = projectile_type[slot];
  *view = (SamusProjectileView){
    .active = true,
    .is_beam = SamusProjectile_IsBeamType(type),
    .is_basic_beam = SamusProjectile_IsBasicBeamType(type),
    .slot_index = (uint16)slot,
    .slot_offset = (uint16)(slot * 2),
    .type = type,
    .direction = projectile_dir[slot],
    .x_pos = projectile_x_pos[slot],
    .y_pos = projectile_y_pos[slot],
    .x_radius = projectile_x_radius[slot],
    .y_radius = projectile_y_radius[slot],
    .damage = projectile_damage[slot],
    .instruction_ptr = projectile_bomb_instruction_ptr[slot],
  };
  return true;
}

int SamusProjectile_GetActiveViews(SamusProjectileView *views, int capacity) {
  int count = 0;
  if (views != NULL && capacity > 0)
    memset(views, 0, (size_t)capacity * sizeof(*views));

  for (int slot = 0; slot < kSamusProjectileSlotCount; slot++) {
    SamusProjectileView view;
    if (!SamusProjectile_ReadSlot(slot, &view))
      continue;
    if (views != NULL && count < capacity)
      views[count] = view;
    count++;
  }
  return count;
}

int SamusProjectile_CountActive(void) {
  return SamusProjectile_GetActiveViews(NULL, 0);
}

bool SamusProjectile_GetFirstActive(SamusProjectileView *view) {
  if (view == NULL)
    return false;

  for (int slot = 0; slot < kSamusProjectileSlotCount; slot++) {
    if (SamusProjectile_ReadSlot(slot, view))
      return true;
  }
  memset(view, 0, sizeof(*view));
  return false;
}
