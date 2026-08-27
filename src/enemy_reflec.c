// Enemy AI - Reflec — peeled from Bank $A3
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

#define g_word_A3DABC ((uint16*)RomFixedPtr(0xa3dabc))
#define g_off_A3DC0B ((uint16*)RomFixedPtr(0xa3dc0b))
#define g_word_A3DCAE ((uint16*)RomFixedPtr(0xa3dcae))
#define g_off_A3DCA6 ((uint16*)RomFixedPtr(0xa3dca6))

void Reflec_Func_1(void) {  // 0xA3DB0C
  if (!door_transition_flag_enemies && !--variables_for_enemy_graphics_drawn_hook[2]) {
    variables_for_enemy_graphics_drawn_hook[2] = 16;
    uint16 v0 = variables_for_enemy_graphics_drawn_hook[0];
    uint16 v1 = 8 * variables_for_enemy_graphics_drawn_hook[1];
    int n = 4;
    do {
      palette_buffer[(v0 >> 1) + 137] = g_word_A3DABC[v1 >> 1];
      v1 += 2;
      v0 += 2;
    } while (--n);
    variables_for_enemy_graphics_drawn_hook[1] = (LOBYTE(variables_for_enemy_graphics_drawn_hook[1]) + 1) & 7;
  }
}

const uint16 *Reflec_Instr_1(uint16 k, const uint16 *jp) {  // 0xA3DBC8
  Get_Reflec(k)->reflec_parameter_2 = jp[0];
  return jp + 1;
}

void Reflec_Init(void) {  // 0xA3DBD3
  Enemy_Reflec *E = Get_Reflec(cur_enemy_index);
  E->base.properties |= kEnemyProps_BlockPlasmaBeam;
  E->base.current_instruction = g_off_A3DC0B[E->reflec_parameter_1];
  enemy_gfx_drawn_hook.addr = FUNC16(Reflec_Func_1);
  *(uint16 *)&enemy_gfx_drawn_hook.bank = 163;
  variables_for_enemy_graphics_drawn_hook[0] = ((16 * E->base.palette_index) & 0xFF00) >> 8;
  variables_for_enemy_graphics_drawn_hook[2] = 16;
}

void Reflec_Shot(void) {
  uint16 v0 = 2 * collision_detection_index;
  Enemy_Reflec *EK = Get_Reflec(cur_enemy_index);
  EK->base.invincibility_timer = 10;
  uint16 v2 = 32 * EK->reflec_parameter_2 + 2 * (projectile_dir[v0 >> 1] & 0xF);
  uint16 varE32 = v2;
  int v3 = v2 >> 1;
  if (g_word_A3DCAE[v3] == 0x8000) {
    projectile_dir[v0 >> 1] |= 0x10;
    printf("Possible bug. What is X?\n");
    Enemy_Reflec *ET = Get_Reflec(v2);
    if (ET->base.health) {
      NormalEnemyShotAiSkipDeathAnim_CurEnemy();
      if (!ET->base.health) {
        ET->base.current_instruction = g_off_A3DCA6[ET->reflec_parameter_2];
        ET->base.instruction_timer = 1;
        ET->base.timer = 0;
      }
    }
  } else {
    int16 v4 = g_word_A3DCAE[v3];
    if (v4 < 0)
      v4 = -g_word_A3DCAE[varE32 >> 1];
    int v5 = v0 >> 1;
    projectile_dir[v5] = v4;
    projectile_type[v5] &= ~0x8000;
    ProjectileReflection(v0);
    QueueSfx2_Max6(0x57);
  }
}
