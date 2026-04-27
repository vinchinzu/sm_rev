// Samus projectile slot lifecycle: reset, clear, and kill-position handling.

#include "samus_projectile.h"

#include "funcs.h"
#include "ida_types.h"
#include "sm_rtl.h"
#include "variables.h"

void ResetProjectileData(void) {  // 0x90AD22
  uint16 v0 = 0;
  do {
    int v1 = v0 >> 1;
    projectile_timers[v1] = 0;
    projectile_x_pos[v1] = 0;
    projectile_y_pos[v1] = 0;
    projectile_dir[v1] = 0;
    projectile_bomb_x_speed[v1] = 0;
    projectile_bomb_y_speed[v1] = 0;
    projectile_x_radius[v1] = 0;
    projectile_y_radius[v1] = 0;
    projectile_type[v1] = 0;
    projectile_damage[v1] = 0;
    projectile_bomb_instruction_ptr[v1] = 0;
    projectile_bomb_instruction_timers[v1] = 0;
    projectile_variables[v1] = 0;
    projectile_spritemap_pointers[v1] = 0;
    projectile_bomb_pre_instructions[v1] = FUNC16(ProjPreInstr_Empty);
    v0 += 2;
  } while ((int16)(v0 - 20) < 0);
  bomb_counter = 0;
  cooldown_timer = 0;
  projectile_counter = 0;
  power_bomb_flag = 0;
  if (hud_auto_cancel_flag) {
    hud_item_index = 0;
    samus_auto_cancel_hud_item_index = 0;
  }
  speed_echo_xpos[0] = 0;
  speed_echo_xpos[1] = 0;
  speed_echo_xpos[2] = 0;
  speed_echo_xpos[3] = 0;
  speed_echo_ypos[0] = 0;
  speed_echo_ypos[1] = 0;
  speed_echo_ypos[2] = 0;
  speed_echo_ypos[3] = 0;
  speed_echo_xspeed[0] = 0;
  speed_echo_xspeed[1] = 0;
  speed_echo_xspeed[2] = 0;
  speed_echo_xspeed[3] = 0;
  speed_echoes_index = 0;
  if (samus_special_super_palette_flags) {
    samus_special_super_palette_flags = 0;
    Samus_LoadSuitTargetPalette();
  }
  if (hyper_beam_flag)
    SpawnPalfxObject(addr_stru_8DE1F0);
}

void ClearProjectile(uint16 k) {  // 0x90ADB7
  int v1 = k >> 1;
  projectile_x_pos[v1] = 0;
  projectile_y_pos[v1] = 0;
  projectile_bomb_x_subpos[v1] = 0;
  projectile_bomb_y_subpos[v1] = 0;
  projectile_dir[v1] = 0;
  projectile_bomb_x_speed[v1] = 0;
  projectile_bomb_y_speed[v1] = 0;
  projectile_x_radius[v1] = 0;
  projectile_y_radius[v1] = 0;
  projectile_type[v1] = 0;
  projectile_damage[v1] = 0;
  projectile_bomb_instruction_ptr[v1] = 0;
  projectile_bomb_instruction_timers[v1] = 0;
  projectile_variables[v1] = 0;
  projectile_spritemap_pointers[v1] = 0;
  projectile_bomb_pre_instructions[v1] = FUNC16(ProjPreInstr_Empty);
  if ((int16)(k - 10) >= 0) {
    if ((--bomb_counter & 0x8000) != 0)
      bomb_counter = 0;
  } else if ((--projectile_counter & 0x8000) != 0) {
    projectile_counter = 0;
  }
}

void KillProjectileFunc_0(uint16 j) {  // 0x90AE4E
  projectile_y_pos[j >> 1] -= projectile_y_radius[j >> 1];
}

void KillProjectileFunc_1(uint16 j) {  // 0x90AE59
  int v1 = j >> 1;
  projectile_x_pos[v1] += projectile_x_radius[v1];
  projectile_y_pos[v1] -= projectile_y_radius[v1];
}

void KillProjectileFunc_2(uint16 j) {  // 0x90AE6E
  projectile_x_pos[j >> 1] += projectile_x_radius[j >> 1];
}

void KillProjectileFunc_3(uint16 j) {  // 0x90AE79
  int v1 = j >> 1;
  projectile_x_pos[v1] += projectile_x_radius[v1];
  projectile_y_pos[v1] += projectile_y_radius[v1];
}

void KillProjectileFunc_4(uint16 j) {  // 0x90AE8E
  projectile_y_pos[j >> 1] += projectile_y_radius[j >> 1];
}

void KillProjectileFunc_6(uint16 j) {  // 0x90AE99
  int v1 = j >> 1;
  projectile_x_pos[v1] -= projectile_x_radius[v1];
  projectile_y_pos[v1] += projectile_y_radius[v1];
}

void KillProjectileFunc_7(uint16 j) {  // 0x90AEAE
  projectile_x_pos[j >> 1] -= projectile_x_radius[j >> 1];
}

void KillProjectileFunc_8(uint16 j) {  // 0x90AEB9
  int v1 = j >> 1;
  projectile_x_pos[v1] -= projectile_x_radius[v1];
  projectile_y_pos[v1] -= projectile_y_radius[v1];
}

static Func_Y_V *const kKillProjectileFuncs[10] = {  // 0x90AE06
  KillProjectileFunc_0,
  KillProjectileFunc_1,
  KillProjectileFunc_2,
  KillProjectileFunc_3,
  KillProjectileFunc_4,
  KillProjectileFunc_4,
  KillProjectileFunc_6,
  KillProjectileFunc_7,
  KillProjectileFunc_8,
  KillProjectileFunc_0,
};

void KillProjectile(uint16 k) {
  int v1 = k >> 1;
  if ((projectile_type[v1] & kProjectileType_TypeMask) != 0) {
    if (!sign16((HIBYTE(projectile_type[v1]) & 0xF) - 3)) {
      ClearProjectile(k);
      return;
    }
  } else {
    kKillProjectileFuncs[projectile_dir[v1] & 0xF](k);
  }
  KillProjectileInner(k);
  projectile_bomb_pre_instructions[k >> 1] = FUNC16(ProjPreInstr_Empty);
}
