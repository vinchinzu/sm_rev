// Enemy AI - Spore Spawn boss runtime — peeled from Bank $A5

#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "enemy_types.h"


#define g_word_A5E379 ((uint16*)RomFixedPtr(0xa5e379))
#define kSporeSpawn_Palette ((uint16*)RomFixedPtr(0xa5e359))


void sub_A5E9F5(void) {  // 0xA5E9F5
  if ((nmi_frame_counter_word & 0xF) == 0) {
    NextRandom();
    uint16 x = (random_number & 0x3F) + 96;
    uint16 y = ((uint16)(random_number & 0xF00) >> 8) + 480;
    CreateSpriteAtPos(x, y, 21, 0);
  }
}

void SporeSpawn_Init(void) {  // 0xA5EA2A
  uint16 v0 = 0;
  for (int i = 0; i != 32; i += 2) {
    target_palettes[(i >> 1) + 240] = kSporeSpawn_Palette[v0 >> 1];
    v0 += 2;
  }
  SpawnEprojWithGfx(0, cur_enemy_index, addr_kEproj_SporeSpawnsStalk);
  SpawnEprojWithGfx(1, cur_enemy_index, addr_kEproj_SporeSpawnsStalk);
  SpawnEprojWithGfx(2, cur_enemy_index, addr_kEproj_SporeSpawnsStalk);
  SpawnEprojWithGfx(3, cur_enemy_index, addr_kEproj_SporeSpawnsStalk);
  Enemy_SporeSpawn *E = Get_SporeSpawn(cur_enemy_index);
  Enemy_SporeSpawn *E0 = Get_SporeSpawn(0);
  E->ssn_var_04 = E->base.x_pos;
  E->ssn_var_05 = E->base.y_pos - 72;
  E->ssn_var_C = E->base.x_pos;
  E->ssn_var_D = E->base.y_pos;
  E->ssn_var_F = 0;
  if ((boss_bits_for_area[area_index] & 2) != 0) {
    E0->base.current_instruction = addr_kDraygon_Ilist_E6B9;
    E0->ssn_var_A = FUNC16(nullsub_223);
    E0->base.properties |= 0x8000;
    SporeSpawn_Func_5();
    SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x07, 0x1e, 0xb793 });
    scrolling_finished_hook = 0;
  } else {
    E0->base.current_instruction = addr_kDraygon_Ilist_E6C7;
    flag_process_all_enemies = -1;
    uint16 v5 = cur_enemy_index;
    E->ssn_var_A = FUNC16(nullsub_223);
    scrolling_finished_hook = FUNC16(Samus_ScrollFinishedHook_SporeSpawnFight);
    E->base.y_pos -= 128;
    SpawnEprojWithGfx(0, v5, addr_kEproj_SporeSpawners);
    SpawnEprojWithGfx(1, v5, addr_kEproj_SporeSpawners);
    SpawnEprojWithGfx(2, v5, addr_kEproj_SporeSpawners);
    SpawnEprojWithGfx(3, v5, addr_kEproj_SporeSpawners);
    SporeSpawn_Func_5();
  }
}

void CallSporeSpawnFunc(uint32 ea) {
  switch (ea) {
  case fnnullsub_223: return;
  case fnSporeSpawn_Func_1: SporeSpawn_Func_1(); return;
  case fnSporeSpawn_Func_2: SporeSpawn_Func_2(cur_enemy_index); return;
  case fnSporeSpawn_Func_3: SporeSpawn_Func_3(); return;
  case fnSporeSpawn_Func_4: SporeSpawn_Func_4(); return;
  default: Unreachable();
  }
}

void SporeSpawn_Main(void) {  // 0xA5EB13
  Enemy_SporeSpawn *E = Get_SporeSpawn(cur_enemy_index);
  CallSporeSpawnFunc(E->ssn_var_A | 0xA50000);
}

void SporeSpawn_Func_1(void) {  // 0xA5EB1B
  SporeSpawn_Func_5();
  Enemy_SporeSpawn *E = Get_SporeSpawn(cur_enemy_index);
  uint16 v1 = E->base.y_pos + 1;
  E->base.y_pos = v1;
  if (!sign16(v1 - 624)) {
    E->base.current_instruction = addr_kDraygon_Ilist_E6D5;
    E->base.instruction_timer = 1;
  }
  Enemy_SporeSpawn *E0 = Get_SporeSpawn(0);
  E0->ssn_var_0B = 48;
  E0->ssn_var_0C = 1;
  E0->ssn_var_0A = 192;
}

void SporeSpawn_Func_2(uint16 k) {  // 0xA5EB52
  SporeSpawn_Func_5();
  Enemy_SporeSpawn *E0 = Get_SporeSpawn(0);
  Enemy_SporeSpawn *E = Get_SporeSpawn(k);
  E->base.x_pos = E->ssn_var_C + CosineMult8bit(E0->ssn_var_0A, E0->ssn_var_0B);
  E->base.y_pos = E->ssn_var_D + SineMult8bit(2 * (E0->ssn_var_0A - 64), E0->ssn_var_0B - 16);
  E0->ssn_var_0A = (uint8)(LOBYTE(E0->ssn_var_0C) + E0->ssn_var_0A);
}

void SporeSpawn_Func_3(void) {  // 0xA5EB9B
  Enemy_SporeSpawn *E0 = Get_SporeSpawn(0);
  E0->ssn_var_43 = (uint8)(64 - CalculateAngleFromXY(128 - E0->base.x_pos, 624 - E0->base.y_pos));
  Enemy_SporeSpawn *E = Get_SporeSpawn(cur_enemy_index);
  Point32 pt = ConvertAngleToXy(LOBYTE(E0->ssn_var_43), 1);
  E->ssn_var_28 = pt.x >> 16;
  E->ssn_var_29 = pt.x;
  E->ssn_var_2A = pt.y >> 16;
  E->ssn_var_2B = pt.y;
}

void SporeSpawn_Func_4(void) {  // 0xA5EBEE
  Enemy_SporeSpawn *E = Get_SporeSpawn(cur_enemy_index);
  Enemy_SporeSpawn *E0 = Get_SporeSpawn(0);
  uint16 varE20 = LOBYTE(E0->ssn_var_43);
  EnemyFunc_B691(varE20, (Point32) { __PAIR32__(E->ssn_var_28, E->ssn_var_29), __PAIR32__(E->ssn_var_2A, E->ssn_var_2B) });
  uint16 v2 = Abs16(E0->base.x_pos - 128);
  if (sign16(v2 - 8)) {
    uint16 v3 = Abs16(E0->base.y_pos - 624);
    if (sign16(v3 - 8))
      E0->ssn_var_A = FUNC16(nullsub_223);
  }
  SporeSpawn_Func_5();
  sub_A5E9F5();
}

void SporeSpawn_Func_5(void) {  // 0xA5EC49
  Enemy_SporeSpawn *E = Get_SporeSpawn(0);
  uint16 v1 = E->base.x_pos - E->ssn_var_04, v2;
  uint16 r18, r20, r22;
  if ((v1 & 0x8000) == 0) {
    r18 = v1 >> 1, r20 = v1 >> 2;
    r22 = (v1 >> 1) + (v1 >> 2);
    eproj_x_pos[14] = 128;
    eproj_x_pos[15] = E->ssn_var_04 + (v1 >> 2);
    E->ssn_var_20 = eproj_x_pos[15];
    eproj_x_pos[16] = E->ssn_var_04 + r18;
    E->ssn_var_21 = eproj_x_pos[16];
    v2 = E->ssn_var_04 + r22;
  } else {
    r18 = (uint16)(E->ssn_var_04 - E->base.x_pos) >> 1;
    r20 = (uint16)-v1 >> 2;
    r22 = r18 + r20;
    eproj_x_pos[14] = 128;
    eproj_x_pos[15] = E->ssn_var_04 - r20;
    E->ssn_var_20 = eproj_x_pos[15];
    eproj_x_pos[16] = E->ssn_var_04 - r18;
    E->ssn_var_21 = eproj_x_pos[16];
    v2 = E->ssn_var_04 - r22;
  }
  eproj_x_pos[17] = v2;
  E->ssn_var_22 = v2;
  uint16 v3 = E->base.y_pos - 40 - E->ssn_var_05, v4;
  if ((v3 & 0x8000) == 0) {
    r18 = v3 >> 1, r20 = v3 >> 2;
    r22 = (v3 >> 1) + (v3 >> 2);
    eproj_y_pos[14] = 560;
    eproj_y_pos[15] = E->ssn_var_05 + (v3 >> 2);
    E->ssn_var_23 = eproj_y_pos[15];
    eproj_y_pos[16] = E->ssn_var_05 + r18;
    E->ssn_var_24 = eproj_y_pos[16];
    v4 = E->ssn_var_05 + r22;
  } else {
    r18 = (uint16)(E->ssn_var_05 - (E->base.y_pos - 40)) >> 1;
    r20 = (uint16)-v3 >> 2;
    r22 = r18 + r20;
    eproj_y_pos[14] = 560;
    eproj_y_pos[15] = E->ssn_var_05 - r20;
    E->ssn_var_23 = eproj_y_pos[15];
    eproj_y_pos[16] = E->ssn_var_05 - r18;
    E->ssn_var_24 = eproj_y_pos[16];
    v4 = E->ssn_var_05 - r22;
  }
  eproj_y_pos[17] = v4;
  E->ssn_var_25 = v4;
}

void SporeSpawn_Shot(void) {  // 0xA5ED5A
  int16 v4;

  uint16 v0 = 2 * collision_detection_index;
  uint16 v1 = projectile_type[collision_detection_index];
  if ((v1 & 0x700) != 0 || (v1 & 0x10) != 0) {
    EnemyFunc_A6B4_UsedBySporeSpawn();
    v0 = cur_enemy_index;
    Enemy_SporeSpawn *EK = Get_SporeSpawn(v0);
    if (EK->base.flash_timer) {
      Enemy_SporeSpawn *E = Get_SporeSpawn(0);
      EK->ssn_var_A = FUNC16(SporeSpawn_Func_2);
      v4 = 2;
      if (sign16(EK->base.health - 400)) {
        if ((E->ssn_var_0C & 0x8000) != 0)
          v4 = -2;
        E->ssn_var_0C = v4;
      }
      if (!E->ssn_var_2F) {
        E->ssn_var_0C = -E->ssn_var_0C;
        E->ssn_var_2F = 1;
        EK->base.current_instruction = addr_stru_A5E729;
        EK->base.instruction_timer = 1;
        uint16 v7 = 96;
        uint16 health = EK->base.health;
        if (!sign16(health - 70)) {
          v7 = 64;
          if (!sign16(health - 410)) {
            v7 = 32;
            if (!sign16(health - 770))
              v7 = 0;
          }
        }
        if (health != E->ssn_var_40) {
          E->ssn_var_40 = health;
          SporeSpawn_Func_7(v7);
        }
      }
    }
    SporeSpawn_Func_6();
  }
}

void SporeSpawn_Touch(void) {  // 0xA5EDEC
  NormalEnemyTouchAiSkipDeathAnim_CurEnemy();
  SporeSpawn_Func_6();
}

void SporeSpawn_Func_6(void) {  // 0xA5EDF3
  if (!Get_SporeSpawn(cur_enemy_index)->base.health) {
    Enemy_SporeSpawn *E = Get_SporeSpawn(cur_enemy_index);
    Enemy_SporeSpawn *E0 = Get_SporeSpawn(0);
    E0->ssn_var_0E = 0;
    E->base.invincibility_timer = 0;
    E->base.flash_timer = 0;
    E->base.ai_handler_bits = 0;
    E->base.properties |= kEnemyProps_Tangible;
    for (int i = 26; i >= 0; i -= 2)
      *(uint16 *)((uint8 *)eproj_id + (uint16)i) = 0;
    E0->base.current_instruction = addr_kDraygon_Ilist_E77D;
    E0->base.instruction_timer = 1;
    *(uint16 *)&boss_bits_for_area[area_index] |= 2;
    scrolling_finished_hook = 0;
    SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x07, 0x1e, 0xb78f });
  }
}

void SporeSpawn_Func_7(uint16 a) {  // 0xA5EE4A
  uint16 v1 = a;
  for (int i = 0; i != 32; i += 2) {
    palette_buffer[(i >> 1) + 144] = g_word_A5E379[v1 >> 1];
    v1 += 2;
  }
}
