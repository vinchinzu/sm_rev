// Enemy AI - Metroid
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"

#define g_word_A3EAD6 ((uint16*)RomFixedPtr(0xa3ead6))
#define g_word_A3EA3F ((uint16*)RomFixedPtr(0xa3ea3f))

static Func_Y_V *const kMetroidHandlers[4] = {
  Metroid_Func_1, Metroid_Func_2, Metroid_Func_3, Metroid_Func_4,
};

void Metroid_Init(void) {  // 0xA3EA4F
  Enemy_Metroid *E = Get_Metroid(cur_enemy_index);
  E->base.current_instruction = addr_kMetroid_Ilist_E9CF;
  E->metroid_var_00 = CreateSpriteAtPos(E->base.x_pos, E->base.y_pos, 50, E->base.vram_tiles_index | E->base.palette_index);
  E->metroid_var_01 = CreateSpriteAtPos(E->base.x_pos, E->base.y_pos, 52, E->base.vram_tiles_index | E->base.palette_index);
  E->metroid_var_02 = 0;
}

const uint16 *Metroid_Instr_2(uint16 k, const uint16 *jp) {  // 0xA3EAA5
  QueueSfx2_Max6(0x50);
  return jp;
}

const uint16 *Metroid_Instr_1(uint16 k, const uint16 *jp) {  // 0xA3EAB1
  int v2 = NextRandom() & 7;
  QueueSfx2_Max6(g_word_A3EAD6[v2]);
  return jp;
}

void Metroid_Frozen(void) {  // 0xA3EAE6
  Enemy_NormalFrozenAI_A3();
  Enemy_Metroid *E = Get_Metroid(cur_enemy_index);
  if (E->metroid_var_E) {
    --E->metroid_var_E;
    E->base.flash_timer = 2;
  }
  int v1 = E->metroid_var_00 >> 1;
  sprite_palettes[v1] = 3072;
  sprite_disable_flag[v1] = 1;
  sprite_instr_list_ptrs[v1] = addr_kSpriteObject_Ilist_C3BA;
  int v2 = E->metroid_var_01 >> 1;
  sprite_palettes[v2] = 3072;
  sprite_disable_flag[v2] = 1;
  sprite_instr_list_ptrs[v2] = addr_kSpriteObject_Ilist_C4B6;
}

void Metroid_Hurt(void) {  // 0xA3EB33
  Enemy_Metroid *E = Get_Metroid(cur_enemy_index);
  if ((E->base.flash_timer & 2) != 0) {
    uint16 r18 = E->base.palette_index;
    int v1 = E->metroid_var_00 >> 1;
    sprite_palettes[v1] = r18 | sprite_palettes[v1] & 0xF1FF;
    int v2 = E->metroid_var_01 >> 1;
    sprite_palettes[v2] = r18 | sprite_palettes[v2] & 0xF1FF;
  } else {
    int v3 = E->metroid_var_00 >> 1;
    sprite_palettes[v3] &= 0xF1FF;
    uint16 metroid_var_01 = E->metroid_var_01;
    sprite_palettes[metroid_var_01 >> 1] &= 0xF1FF;
  }
}

void Metroid_Main(void) {  // 0xA3EB98
  Enemy_Metroid *E = Get_Metroid(cur_enemy_index);
  kMetroidHandlers[E->metroid_var_F](samus_y_pos - 8);
  uint16 r24 = E->base.vram_tiles_index | E->base.palette_index;
  int v2 = E->metroid_var_00 >> 1;
  sprite_x_pos[v2] = E->base.x_pos;
  sprite_y_pos[v2] = E->base.y_pos;
  sprite_palettes[v2] = r24;
  sprite_disable_flag[v2] = 0;
  int v4 = E->metroid_var_01 >> 1;
  sprite_x_pos[v4] = E->base.x_pos;
  sprite_y_pos[v4] = E->base.y_pos;
  sprite_palettes[v4] = r24;
  sprite_disable_flag[v4] = 0;
}

void Metroid_Func_1(uint16 varE32) {  // 0xA3EC11
  int16 v3;
  int16 v4;
  int16 v8;
  int16 v9;

  Enemy_Metroid *E = Get_Metroid(cur_enemy_index);
  int32 t = INT16_SHL8((int16)(E->base.y_pos - varE32) >> 2);
  uint16 r18 = t, r20 = t >> 16;
  uint16 metroid_var_C = E->metroid_var_C;
  bool v2 = metroid_var_C < r18;
  E->metroid_var_C = metroid_var_C - r18;
  v3 = E->metroid_var_D - (v2 + r20);
  E->metroid_var_D = v3;
  if (v3 < 0) {
    if ((uint16)v3 >= 0xFFFD)
      goto LABEL_9;
    v4 = -3;
  } else {
    if ((uint16)v3 < 3)
      goto LABEL_9;
    v4 = 3;
  }
  E->metroid_var_D = v4;
  E->metroid_var_C = 0;
LABEL_9:
  if (Enemy_MoveDown(cur_enemy_index, __PAIR32__(E->metroid_var_D, E->metroid_var_C))) {
    E->metroid_var_C = 0;
    E->metroid_var_D = 0;
  }
  t = INT16_SHL8((int16)(E->base.x_pos - samus_x_pos) >> 2);
  r18 = t, r20 = t >> 16;
  uint16 metroid_var_A = E->metroid_var_A;
  v2 = metroid_var_A < r18;
  E->metroid_var_A = metroid_var_A - r18;
  v8 = E->metroid_var_B - (v2 + r20);
  E->metroid_var_B = v8;
  if (v8 < 0) {
    if ((uint16)v8 >= 0xFFFD)
      goto LABEL_19;
    v9 = -3;
  } else {
    if ((uint16)v8 < 3)
      goto LABEL_19;
    v9 = 3;
  }
  E->metroid_var_B = v9;
  E->metroid_var_A = 0;
LABEL_19:
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, __PAIR32__(E->metroid_var_B, E->metroid_var_A))) {
    E->metroid_var_A = 0;
    E->metroid_var_B = 0;
  }
}

void Metroid_Func_2(uint16 varE32) {  // 0xA3ECDC
  int16 v1;
  int16 v5;
  uint16 v2, v6;
  uint16 r18 = 0, r20 = 0;
  Enemy_Metroid *E = Get_Metroid(cur_enemy_index);
  LOBYTE(v1) = (uint16)(varE32 - E->base.y_pos) >> 8;
  HIBYTE(v1) = varE32 - LOBYTE(E->base.y_pos);
  r20 = (uint16)(v1 & 0xFF00) >> 3;
  if ((r20 & 0x1000) != 0)
    r20 |= 0xE000;
  if ((r20 & 0x8000) != 0) {
    if (r20 >= 0xFFFD)
      goto LABEL_9;
    v2 = -3;
  } else {
    if (r20 < 3)
      goto LABEL_9;
    v2 = 3;
  }
  r20 = v2;
  r18 = 0;
LABEL_9:
  E->metroid_var_C = r18;
  E->metroid_var_D = r20;
  if (Enemy_MoveDown(cur_enemy_index, __PAIR32__(r20, r18))) {
    E->metroid_var_C = 0;
    E->metroid_var_D = 0;
  }
  r18 = 0, r20 = 0;
  LOBYTE(v5) = (uint16)(samus_x_pos - E->base.x_pos) >> 8;
  HIBYTE(v5) = samus_x_pos - LOBYTE(E->base.x_pos);
  r20 = (uint16)(v5 & 0xFF00) >> 3;
  if ((r20 & 0x1000) != 0)
    r20 |= 0xE000;
  if ((r20 & 0x8000) != 0) {
    if (r20 >= 0xFFFD)
      goto LABEL_19;
    v6 = -3;
  } else {
    if (r20 < 3)
      goto LABEL_19;
    v6 = 3;
  }
  r20 = v6;
  r18 = 0;
LABEL_19:
  E->metroid_var_A = r18;
  E->metroid_var_B = r20;
  if (Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, __PAIR32__(r20, r18))) {
    E->metroid_var_A = 0;
    E->metroid_var_B = 0;
  }
}

void Metroid_Func_3(uint16 varE32) {  // 0xA3ED8F
  Enemy_Metroid *E = Get_Metroid(cur_enemy_index);
  E->base.x_pos = samus_x_pos;
  E->base.y_pos = varE32;
  E->metroid_var_A = 0;
  E->metroid_var_B = 0;
  E->metroid_var_C = 0;
  E->metroid_var_D = 0;
}

void Metroid_Func_4(uint16 varE32) {  // 0xA3EDAB
  Enemy_Metroid *E = Get_Metroid(cur_enemy_index);
  int v1 = E->metroid_var_E & 3;
  E->base.x_pos += g_word_A3EA3F[v1];
  E->base.y_pos += g_word_A3EA3F[v1 + 4];
  E->metroid_var_A = 0;
  E->metroid_var_B = 0;
  E->metroid_var_C = 0;
  E->metroid_var_D = 0;
  if (E->metroid_var_E-- == 1) {
    E->metroid_var_F = 0;
    E->base.current_instruction = addr_kMetroid_Ilist_E9CF;
    E->base.instruction_timer = 1;
  }
}

void Metroid_Touch(void) {  // 0xA3EDEB
  Enemy_Metroid *E = Get_Metroid(cur_enemy_index);
  uint16 varE32 = samus_y_pos - 8;
  if (samus_contact_damage_index) {
    if (E->metroid_var_F != 2) {
      E->metroid_var_A = 0;
      E->metroid_var_C = 0;
      int16 v2 = E->base.x_pos - samus_x_pos;
      E->metroid_var_B = (v2 < 0) ? -256 : 0;
      *(uint16 *)((uint8 *)&E->metroid_var_A + 1) = v2 << 6;
      int16 v4 = E->base.y_pos - varE32;
      E->metroid_var_D = (v4 < 0) ? -256 : 0;
      *(uint16 *)((uint8 *)&E->metroid_var_C + 1) = v4 << 6;
      E->metroid_var_F = 0;
      E->base.current_instruction = addr_kMetroid_Ilist_E9CF;
      E->base.instruction_timer = 1;
    }
  } else {
    uint16 v5 = cur_enemy_index;
    if (E->metroid_var_F != 3) {
      if ((random_enemy_counter & 7) == 7 && !sign16(samus_health - 30))
        QueueSfx3_Max6(0x2D);
      Metroid_Func_5(v5);
    }
    if (E->metroid_var_F < 2) {
      uint16 v7 = 1;
      if (abs16(E->base.x_pos - samus_x_pos) < 8 && abs16(E->base.y_pos - varE32) < 8) {
        v7 = 2;
        CallSomeSamusCode(0x12);
      }
      E->metroid_var_F = v7;
      if (v7 == 2) {
        E->base.current_instruction = addr_kMetroid_Ilist_EA25;
        E->base.instruction_timer = 1;
      }
    }
  }
}

void Metroid_Func_5(uint16 k) {  // 0xA3EECE
  uint16 v1;
  uint16 varE32 = samus_y_pos - 8;
  if ((equipped_items & 0x20) != 0) {
    v1 = 12288;
  } else if (equipped_items & 1) {
    v1 = 24576;
  } else {
    v1 = -16384;
  }
  uint16 r18 = v1;
  Enemy_Metroid *E = Get_Metroid(k);
  uint16 metroid_var_02 = E->metroid_var_02;
  bool v4 = metroid_var_02 < r18;
  E->metroid_var_02 = metroid_var_02 - r18;
  if (v4)
    Samus_DealDamage(1);
}

void Metroid_Shot(void) {  // 0xA3EF07
  uint16 v0 = 2 * collision_detection_index;
  Enemy_Metroid *E = Get_Metroid(cur_enemy_index);
  if (E->base.frozen_timer) {
    uint16 v3 = projectile_type[v0 >> 1] & 0xF00;
    if (v3 == 256 || v3 == 512) {
      special_death_item_drop_x_origin_pos = E->base.x_pos;
      special_death_item_drop_y_origin_pos = E->base.y_pos;
      Enemy_NormalShotAI_SkipSomeParts_A3();
      if (!E->base.health) {
        E->metroid_var_B = 0;
        EnemyDeathAnimation(cur_enemy_index, 4);
        CallSomeSamusCode(0x13);
        sprite_instr_list_ptrs[E->metroid_var_00 >> 1] = 0;
        uint16 metroid_var_01 = E->metroid_var_01;
        sprite_instr_list_ptrs[metroid_var_01 >> 1] = 0;
        Enemy_ItemDrop_Metroid(metroid_var_01);
      }
    }
  } else {
    if (E->metroid_var_F == 2) {
      if ((projectile_type[v0 >> 1] & 0xF00) == 1280) {
        E->metroid_var_E = 4;
        E->metroid_var_F = 3;
        E->base.current_instruction = addr_kMetroid_Ilist_E9CF;
        E->base.instruction_timer = 1;
        CallSomeSamusCode(0x13);
      }
    } else {
      E->metroid_var_A = 0;
      E->metroid_var_C = 0;
      int16 v7 = E->base.x_pos - projectile_x_pos[0];
      E->metroid_var_B = (v7 < 0) ? -256 : 0;
      *(uint16 *)((uint8 *)&E->metroid_var_A + 1) = 32 * v7;
      int16 v9 = E->base.y_pos - projectile_y_pos[0];
      E->metroid_var_D = (v9 < 0) ? -256 : 0;
      *(uint16 *)((uint8 *)&E->metroid_var_C + 1) = 32 * v9;
      E->metroid_var_F = 0;
      E->base.current_instruction = addr_kMetroid_Ilist_E9CF;
      E->base.instruction_timer = 1;
      int v10 = collision_detection_index;
      if ((projectile_type[v10] & 2) != 0) {
        QueueSfx3_Max6(0xA);
        uint16 r18 = projectile_damage[v10];
        E->base.flash_timer = 4;
        uint16 metroid_parameter_2 = E->metroid_parameter_2;
        bool v13 = metroid_parameter_2 < r18;
        uint16 v12 = metroid_parameter_2 - r18;
        v13 = !v13;
        if (!v12 || !v13) {
          E->metroid_parameter_2 = 0;
          E->base.frozen_timer = 400;
          E->base.ai_handler_bits |= 4;
          return;
        }
        E->metroid_parameter_2 = v12;
      }
      QueueSfx2_Max6(0x5A);
    }
  }
}

void Metroid_Powerbomb(uint16 k) {  // 0xA3F042
  NormalEnemyPowerBombAi();
  if (!Get_Metroid(k)->base.health) {
    CallSomeSamusCode(0x13);
    sprite_instr_list_ptrs[Get_Metroid(cur_enemy_index)->metroid_var_00 >> 1] = 0;
    sprite_instr_list_ptrs[Get_Metroid(cur_enemy_index)->metroid_var_01 >> 1] = 0;
  }
}
