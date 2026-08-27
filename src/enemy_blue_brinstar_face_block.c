// Enemy AI - Blue Brinstar Face Block — peeled from Bank $A8

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

#define g_word_A8E7CC ((uint16*)RomFixedPtr(0xa8e7cc))

void BlueBrinstarFaceBlock_Init(void) {  // 0xA8E82E
  EnemyData *v0 = gEnemyData(cur_enemy_index);
  v0->current_instruction = addr_kBlueBrinstarFaceBlock_Ilist_E828;
  uint16 v1 = FUNC16(BlueBrinstarFaceBlock_Func_1);
  if ((collected_items & 4) == 0)
    v1 = FUNC16(nullsub_170_A8);
  enemy_gfx_drawn_hook.addr = v1;
  *(uint16 *)&enemy_gfx_drawn_hook.bank = 168;
  variables_for_enemy_graphics_drawn_hook[0] = ((16 * v0->palette_index) & 0xFF00) >> 8;
  variables_for_enemy_graphics_drawn_hook[2] = 16;
  v0->parameter_2 = ((v0->parameter_2 & 1) >> 2) | ((v0->parameter_2 & 1) << 15);
}

void BlueBrinstarFaceBlock_Func_1(void) {  // 0xA8E86E
  if (!door_transition_flag_enemies && !--variables_for_enemy_graphics_drawn_hook[2]) {
    variables_for_enemy_graphics_drawn_hook[2] = 16;
    uint16 v0 = variables_for_enemy_graphics_drawn_hook[0];
    uint16 v1 = 8 * variables_for_enemy_graphics_drawn_hook[1];
    int n = 4;
    do {
      palette_buffer[(v0 >> 1) + 137] = g_word_A8E7CC[v1 >> 1];
      v1 += 2;
      v0 += 2;
    } while (--n);
    variables_for_enemy_graphics_drawn_hook[1] = (LOBYTE(variables_for_enemy_graphics_drawn_hook[1]) + 1) & 7;
  }
}

void BlueBrinstarFaceBlock_Main(void) {  // 0xA8E8AE
  if ((collected_items & 4) != 0) {
    enemy_gfx_drawn_hook.addr = FUNC16(BlueBrinstarFaceBlock_Func_1);
    EnemyData *v1 = gEnemyData(cur_enemy_index);
    variables_for_enemy_graphics_drawn_hook[0] = ((16 * v1->palette_index) & 0xFF00) >> 8;
    if (!v1->ai_var_A) {
      uint16 SamusEnemyDelta_Y = GetSamusEnemyDelta_Y(cur_enemy_index);
      if ((int16)(Abs16(SamusEnemyDelta_Y) - v1->parameter_1) < 0) {
        uint16 SamusEnemyDelta_X = GetSamusEnemyDelta_X(cur_enemy_index);
        v1->ai_var_B = SamusEnemyDelta_X;
        if ((int16)(Abs16(SamusEnemyDelta_X) - v1->parameter_1) < 0 && (v1->ai_var_B & 0x8000) != v1->parameter_2) {
          uint16 v4 = addr_kBlueBrinstarFaceBlock_Ilist_E80C;
          if ((v1->ai_var_B & 0x8000) == 0)
            v4 = addr_kBlueBrinstarFaceBlock_Ilist_E81A;
          v1->current_instruction = v4;
          v1->instruction_timer = 1;
          v1->ai_var_A = 1;
          variables_for_enemy_graphics_drawn_hook[2] = 16;
        }
      }
    }
  }
}

void BlueBrinstarFaceBlock_Shot(void) {  // 0xA8E91D
  projectile_dir[collision_detection_index] &= ~0x10;
}

