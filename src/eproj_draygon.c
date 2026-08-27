// Draygon enemy-projectile family split out of sm_86.c.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "eproj_internal.h"

const uint8 *EprojInstr_SpawnEnemyDropsWithDraygonsEyeDrops(uint16 k, const uint8 *epjp) {  // 0x868C68
  int v1 = k >> 1;
  eproj_spawn_pt = (Point16U) { eproj_x_pos[v1], eproj_y_pos[v1] };
  SpawnEnemyDrops(addr_kEnemyDef_DE7F, k, 0);
  return epjp;
}

const uint8 *EprojInstr_SetPreInstrA(uint16 k, const uint8 *epjp) {  // 0x868CF6
  eproj_pre_instr[k >> 1] = FUNC16(EprojPreInstr_DraygonsTurret_8DFF);
  return epjp;
}

const uint8 *EprojInstr_SetPreInstrB(uint16 k, const uint8 *epjp) {  // 0x868CFD
  eproj_pre_instr[k >> 1] = FUNC16(EprojPreInstr_8DCA);
  return epjp;
}

void EprojInit_DraygonsGunk(uint16 j) {  // 0x868D04
  int v1 = j >> 1;
  eproj_x_pos[v1] = eproj_spawn_pt.x;
  eproj_y_pos[v1] = eproj_spawn_pt.y;
  g_word_7E97DC[v1] = eproj_unk1995;
  Point32 pt = ConvertAngleToXy(eproj_unk1995, eproj_init_param_1);
  eproj_x_vel[v1] = pt.x >> 16;
  eproj_E[v1] = pt.x;
  eproj_y_vel[v1] = pt.y >> 16;
  eproj_F[v1] = pt.y;
  eproj_gfx_idx[v1] = 1024;
}

void EprojInit_DraygonsWallTurretProjs(uint16 j) {  // 0x868D40
  Eproj_AngleToSamus(j, eproj_spawn_pt.x, eproj_spawn_pt.y);
  int v1 = j >> 1;
  eproj_gfx_idx[v1] = 2560;
  eproj_pre_instr[v1] = FUNC16(nullsub_84);
}

void EprojPowerBombCollision(uint16 k) {  // 0x868D5C
  uint16 r18 = HIBYTE(power_bomb_explosion_radius);
  if (HIBYTE(power_bomb_explosion_radius)) {
    uint16 r20 = (uint16)(r18 + (HIBYTE(power_bomb_explosion_radius) & 1) + (power_bomb_explosion_radius >> 9)) >> 1;
    int v1 = k >> 1;
    if (abs16(power_bomb_explosion_x_pos - eproj_x_pos[v1]) < r18
        && abs16(power_bomb_explosion_y_pos - eproj_y_pos[v1]) < r20) {
      eproj_id[v1] = 0;
      samus_x_speed_divisor = 0;
    }
  }
}

const uint8 *EprojInstr_868D99(uint16 k, const uint8 *epjp) {  // 0x868D99
  EprojPowerBombCollision(k);
  uint16 v2 = samus_x_speed_divisor + 1;
  if (sign16(samus_x_speed_divisor - 5)) {
    ++samus_x_speed_divisor;
    int v3 = k >> 1;
    eproj_F[v3] = v2;
    eproj_E[v3] = 256;
    eproj_pre_instr[v3] = FUNC16(EprojPreInstr_8DCA);
    eproj_properties[v3] = eproj_properties[v3] & 0x5FFF | 0x2000;
    samus_invincibility_timer = 0;
    samus_knockback_timer = 0;
  }
  return epjp;
}

void EprojPreInstr_8DCA(uint16 k) {  // 0x868DCA
  int v1;

  EprojPowerBombCollision(k);
  if (samus_contact_damage_index
      || (v1 = k >> 1,
          eproj_x_pos[v1] = samus_x_pos,
          eproj_y_pos[v1] = samus_y_pos + 4 * eproj_F[v1] - 12,
          --eproj_E[v1],
          !eproj_E[v1])) {
    eproj_id[k >> 1] = 0;
    if ((--samus_x_speed_divisor & 0x8000) != 0)
      samus_x_speed_divisor = 0;
  }
}

void EprojPreInstr_DraygonsTurret_8DFF(uint16 k) {  // 0x868DFF
  int16 v1;

  EprojPowerBombCollision(k);
  Eproj_FuncE73E_MoveXY(k);
  v1 = Eproj_FuncE722(k);
  if (v1)
    eproj_id[k >> 1] = 0;
}

void EprojPreInstr_DraygonsGunk_8E0F(uint16 k) {  // 0x868E0F
  uint16 v3;

  EprojPowerBombCollision(k);
  Eproj_FuncE73E_MoveXY(k);
  int v1 = k >> 1;
  uint16 v2 = abs16(samus_x_pos - eproj_x_pos[v1]);
  if (sign16(v2 - 16) && (v3 = abs16(samus_y_pos - eproj_y_pos[v1]), sign16(v3 - 20))) {
    eproj_instr_list_ptr[v1] = 0x8C38;
    eproj_instr_timers[v1] = 1;
  } else {
    if (Eproj_FuncE722(k))
      eproj_id[v1] = 0;
  }
}
