#ifndef SAMUS_PROJECTILE_VIEW_H_
#define SAMUS_PROJECTILE_VIEW_H_

#include "types.h"

enum {
  kSamusProjectileSlotCount = 10,
  kSamusBeamProjectileSlotCount = 5,
};

typedef struct SamusProjectileView {
  bool active;
  bool is_beam;
  bool is_basic_beam;
  uint16 slot_index;
  uint16 slot_offset;
  uint16 type;
  uint16 direction;
  uint16 x_pos;
  uint16 y_pos;
  uint16 x_radius;
  uint16 y_radius;
  uint16 damage;
  uint16 instruction_ptr;
} SamusProjectileView;

bool SamusProjectile_IsBeamType(uint16 type);
bool SamusProjectile_IsBasicBeamType(uint16 type);
int SamusProjectile_GetActiveViews(SamusProjectileView *views, int capacity);
int SamusProjectile_CountActive(void);
bool SamusProjectile_GetFirstActive(SamusProjectileView *view);

#endif  // SAMUS_PROJECTILE_VIEW_H_
