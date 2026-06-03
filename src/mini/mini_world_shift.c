#include "mini_world_shift.h"

#include "ida_types.h"
#include "samus_env.h"
#include "variables.h"

enum {
  kMiniWorldShiftAtmosphericGfxCount = 4,
  kMiniWorldShiftProjectileCount = 10,
  kMiniWorldShiftProjectileTrailCount = 18,
  kMiniWorldShiftEprojSlotCount = 143,
};

static void MiniWorldShift_AddUint16(uint16 *field, int shift_y) {
  *field = (uint16)(*field + shift_y);
}

static void MiniWorldShift_AddUint16Array(uint16 *fields, int count, int shift_y) {
  for (int i = 0; i < count; i++)
    fields[i] = (uint16)(fields[i] + shift_y);
}

static void MiniWorldShift_ScrollLayerAndSamus(int shift_y) {
  MiniWorldShift_AddUint16(&layer1_y_pos, shift_y);
  MiniWorldShift_AddUint16(&ideal_layer1_ypos, shift_y);
  MiniWorldShift_AddUint16(&samus_y_pos, shift_y);
  MiniWorldShift_AddUint16(&samus_prev_y_pos, shift_y);
  MiniWorldShift_AddUint16(&samus_spritemap_y_pos, shift_y);
  MiniWorldShift_AddUint16Array(speed_echo_ypos, kMiniWorldShiftAtmosphericGfxCount, shift_y);
  MiniWorldShift_AddUint16Array(atmospheric_gfx_y_pos, kMiniWorldShiftAtmosphericGfxCount, shift_y);
}

static void MiniWorldShift_ScrollProjectiles(int shift_y) {
  MiniWorldShift_AddUint16Array(projectile_y_pos, kMiniWorldShiftProjectileCount, shift_y);
  MiniWorldShift_AddUint16Array(projectiletrail_left_y_pos, kMiniWorldShiftProjectileTrailCount, shift_y);
  MiniWorldShift_AddUint16Array(projectiletrail_right_y_pos, kMiniWorldShiftProjectileTrailCount, shift_y);
  MiniWorldShift_AddUint16(&power_bomb_explosion_y_pos, shift_y);
  MiniWorldShift_AddUint16(&power_bomb_explosion_y_pos_rsub_0x1ff, shift_y);
  MiniWorldShift_AddUint16(&grapple_beam_end_y_pos, shift_y);
  MiniWorldShift_AddUint16(&y_pos_of_start_of_grapple_beam, shift_y);
  MiniWorldShift_AddUint16(&y_pos_of_start_of_grapple_beam_prevframe, shift_y);
}

static void MiniWorldShift_ScrollEnemies(int shift_y) {
  for (int slot = 0; slot < num_enemies_in_room; slot++) {
    EnemyData *enemy = &enemy_data[slot];
    if (enemy->enemy_ptr != 0)
      enemy->y_pos = (uint16)(enemy->y_pos + shift_y);
  }
}

static void MiniWorldShift_ScrollEprojs(int shift_y) {
  for (int slot = 0; slot < kMiniWorldShiftEprojSlotCount; slot++) {
    if (eproj_flags[slot] == 0)
      continue;
    eproj_y_pos[slot] = (uint16)(eproj_y_pos[slot] + shift_y);
  }
}

static void MiniWorldShift_ScrollLiquidSurfaces(int shift_y) {
  if ((fx_y_pos & kLiquidYPos_Disabled) == 0)
    fx_y_pos = (uint16)(fx_y_pos + shift_y);
  if ((lava_acid_y_pos & kLiquidYPos_Disabled) == 0)
    lava_acid_y_pos = (uint16)(lava_acid_y_pos + shift_y);
}

void MiniWorldShift_ApplyY(int shift_y) {
  if (shift_y == 0)
    return;
  MiniWorldShift_ScrollLayerAndSamus(shift_y);
  MiniWorldShift_ScrollProjectiles(shift_y);
  MiniWorldShift_ScrollEnemies(shift_y);
  MiniWorldShift_ScrollEprojs(shift_y);
  MiniWorldShift_ScrollLiquidSurfaces(shift_y);
}
