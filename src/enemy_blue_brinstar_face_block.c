// Enemy AI - Blue Brinstar Face Block — peeled from Bank $A8

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

enum {
  kCollectedItems_MorphBall = 0x04,
  kBlueBrinstarFaceBlockBank = 168,
  kBlueBrinstarFaceBlockCycleTimer = 16,
  kBlueBrinstarFaceBlockPaletteBase = 137,
  kBlueBrinstarFaceBlockPaletteColors = 4,
  kBlueBrinstarFaceBlockPaletteFrames = 7,
  kProjectileDir_Hit = 0x10,
};

static const uint16 kBlueBrinstarFaceBlockGlowColors[8][4] = {
  { 0x001f, 0x0012, 0x000a, 0x002b },
  { 0x051f, 0x0096, 0x0011, 0x0007 },
  { 0x0a3f, 0x013b, 0x0018, 0x000d },
  { 0x0f3f, 0x01bf, 0x001f, 0x0012 },
  { 0x0f3f, 0x01bf, 0x001f, 0x0012 },
  { 0x0a3f, 0x013b, 0x0018, 0x000d },
  { 0x051f, 0x0096, 0x0011, 0x0007 },
  { 0x001f, 0x0012, 0x000a, 0x002b },
};

void BlueBrinstarFaceBlock_Init(void) {  // 0xA8E82E
  EnemyData *E = gEnemyData(cur_enemy_index);
  E->current_instruction = addr_kBlueBrinstarFaceBlock_Ilist_E828;
  uint16 gfx_hook = FUNC16(BlueBrinstarFaceBlock_Func_1);
  if ((collected_items & kCollectedItems_MorphBall) == 0)
    gfx_hook = FUNC16(nullsub_170_A8);
  enemy_gfx_drawn_hook.addr = gfx_hook;
  *(uint16 *)&enemy_gfx_drawn_hook.bank = kBlueBrinstarFaceBlockBank;
  variables_for_enemy_graphics_drawn_hook[0] = ((16 * E->palette_index) & 0xFF00) >> 8;
  variables_for_enemy_graphics_drawn_hook[2] = kBlueBrinstarFaceBlockCycleTimer;
  E->parameter_2 = ((E->parameter_2 & 1) >> 2) | ((E->parameter_2 & 1) << 15);
}

void BlueBrinstarFaceBlock_Func_1(void) {  // 0xA8E86E
  if (!door_transition_flag_enemies && !--variables_for_enemy_graphics_drawn_hook[2]) {
    variables_for_enemy_graphics_drawn_hook[2] = kBlueBrinstarFaceBlockCycleTimer;
    uint16 pal_off = variables_for_enemy_graphics_drawn_hook[0] >> 1;
    uint16 frame = variables_for_enemy_graphics_drawn_hook[1] & kBlueBrinstarFaceBlockPaletteFrames;
    for (int i = 0; i < kBlueBrinstarFaceBlockPaletteColors; i++)
      palette_buffer[pal_off + kBlueBrinstarFaceBlockPaletteBase + i] = kBlueBrinstarFaceBlockGlowColors[frame][i];
    variables_for_enemy_graphics_drawn_hook[1] = (LOBYTE(variables_for_enemy_graphics_drawn_hook[1]) + 1) & kBlueBrinstarFaceBlockPaletteFrames;
  }
}

void BlueBrinstarFaceBlock_Main(void) {  // 0xA8E8AE
  if (collected_items & kCollectedItems_MorphBall) {
    enemy_gfx_drawn_hook.addr = FUNC16(BlueBrinstarFaceBlock_Func_1);
    EnemyData *E = gEnemyData(cur_enemy_index);
    variables_for_enemy_graphics_drawn_hook[0] = ((16 * E->palette_index) & 0xFF00) >> 8;
    if (!E->ai_var_A) {
      uint16 samus_delta_y = GetSamusEnemyDelta_Y(cur_enemy_index);
      if ((int16)(Abs16(samus_delta_y) - E->parameter_1) < 0) {
        uint16 samus_delta_x = GetSamusEnemyDelta_X(cur_enemy_index);
        E->ai_var_B = samus_delta_x;
        if ((int16)(Abs16(samus_delta_x) - E->parameter_1) < 0 && sign16(E->ai_var_B) != E->parameter_2) {
          uint16 ilist = addr_kBlueBrinstarFaceBlock_Ilist_E80C;
          if (!sign16(E->ai_var_B))
            ilist = addr_kBlueBrinstarFaceBlock_Ilist_E81A;
          E->current_instruction = ilist;
          E->instruction_timer = 1;
          E->ai_var_A = 1;
          variables_for_enemy_graphics_drawn_hook[2] = kBlueBrinstarFaceBlockCycleTimer;
        }
      }
    }
  }
}

void BlueBrinstarFaceBlock_Shot(void) {  // 0xA8E91D
  projectile_dir[collision_detection_index] &= ~kProjectileDir_Hit;
}
