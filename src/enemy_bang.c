// Enemy AI - Bang — peeled from Bank $A3
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

#define g_word_A3BA84 ((uint16*)RomFixedPtr(0xa3ba84))
#define g_word_A3BA94 ((uint16*)RomFixedPtr(0xa3ba94))
#define g_word_A3BC4A ((uint16*)RomFixedPtr(0xa3bc4a))
#define g_word_A3BC6A ((uint16*)RomFixedPtr(0xa3bc6a))
#define g_off_A3B722 ((uint16*)RomFixedPtr(0xa3b722))

const uint16 *Bang_Instr_1(uint16 k, const uint16 *jp) {  // 0xA3BA78
  QueueSfx2_Max6(0x56);
  return jp;
}

const uint16 *Bang_Instr_2(uint16 k, const uint16 *jp) {  // 0xA3BAA8
  Get_Bang(cur_enemy_index)->bang_var_22 = 1;
  return jp;
}

void Bang_Init(void) {  // 0xA3BAB3
  Enemy_Bang *E = Get_Bang(cur_enemy_index);
  E->bang_var_B = E->base.palette_index;
  E->base.properties |= kEnemyProps_BlockPlasmaBeam;
  E->bang_var_F = FUNC16(Bang_Func_6);
  E->bang_var_00 = 16;
  E->bang_var_01 = 0;
  E->bang_var_02 = 0;
  E->bang_var_20 = 0;
  E->bang_var_21 = 0;
  E->bang_var_22 = 0;
  E->bang_var_0B = g_word_A3BA84[LOBYTE(E->bang_parameter_2)];
  int v1 = 2 * HIBYTE(E->bang_parameter_2);
  uint16 v2 = g_word_A3BC6A[v1];
  E->bang_var_0C = v2;
  E->bang_var_0D = v2;
  E->bang_var_0E = g_word_A3BC6A[v1 + 1];
  if (!E->base.current_instruction)
    E->bang_var_F = FUNC16(Bang_Func_7);
  E->base.current_instruction = addr_kBang_Ilist_B75E;
}

void Bang_Main(void) {  // 0xA3BB25
  Enemy_Bang *E = Get_Bang(cur_enemy_index);
  Call(E->bang_parameter_1 | 0xA30000);
}

void Bang_Func_1(void) {  // 0xA3BB2B
  uint16 v0 = Get_Bang(cur_enemy_index + 64)->bang_var_20 + 10;
  Get_Bang(cur_enemy_index)->bang_var_20 = v0;
  Bang_Func_18();
  Enemy_Bang *E = Get_Bang(cur_enemy_index);
  E->base.properties |= kEnemyProps_Intangible;
}

void Bang_Func_2(void) {  // 0xA3BB4A
  uint16 v0 = Get_Bang(cur_enemy_index + 1984)->bang_var_00 + 20;
  Enemy_Bang *E = Get_Bang(cur_enemy_index);
  E->bang_var_20 = v0;
  Bang_Func_18();
  E->base.properties |= kEnemyProps_Intangible;
}

void Bang_Func_3(void) {  // 0xA3BB66
  int16 bang_var_B;

  Bang_Func_5();
  Enemy_Bang *E0 = Get_Bang(cur_enemy_index);
  uint16 x_pos = E0->base.x_pos;
  int v3 = cur_enemy_index >> 1;
  enemy_drawing_queue[v3 + 91] = x_pos;
  Enemy_Bang *E1 = Get_Bang(cur_enemy_index + 64);
  E1->base.x_pos = x_pos;
  uint16 y_pos = E0->base.y_pos;
  enemy_drawing_queue[v3 + 93] = y_pos;
  E1->base.y_pos = y_pos;
  bang_var_B = E0->bang_var_B;
  if ((E0->bang_var_20 & 1) != 0)
    bang_var_B = 3072;
  E0->base.palette_index = bang_var_B;
  Bang_Func_18();
  if (E0->bang_var_22) {
    E0->bang_var_22 = 0;
    if (E0->bang_var_20 == 9) {
      E0->base.invincibility_timer = 16;
      E0->base.properties |= kEnemyProps_Intangible;
      uint16 v7 = DetermineDirectionOfSamusFromEnemy();
      uint16 v8 = Bang_Func_4(v7);
      EnemyDeathAnimation(cur_enemy_index, v8);
      uint16 v9 = cur_enemy_index;
      Enemy_Bang *E = Get_Bang(cur_enemy_index + 64);
      E->base.properties |= 0x200;
      enemy_drawing_queue[(v9 >> 1) + 97] |= 0x200;
    } else {
      ++E0->bang_var_20;
      Bang_Func_18();
    }
  }
}

uint16 Bang_Func_4(uint16 a) {  // 0xA3BBEB
  int i;

  if (!sign16(projectile_counter - 5))
    return 1;
  for (i = 0; projectile_damage[i >> 1]; i += 2)
    ;
  Enemy_Bang *E = Get_Bang(cur_enemy_index);
  int v3 = i >> 1;
  projectile_x_pos[v3] = E->base.x_pos;
  projectile_y_pos[v3] = E->base.y_pos;
  projectile_dir[v3] = a;
  projectile_type[v3] = equipped_beams & 0xF | 0x10;
  ++projectile_counter;
  ProjectileReflection(i);
  projectile_damage[v3] = E->bang_var_E;
  QueueSfx1_Max6(g_word_A3BC4A[projectile_type[v3] & 0xF]);
  return 0;
}

void Bang_Func_5(void) {  // 0xA3BC9E
  Enemy_Bang *E = Get_Bang(cur_enemy_index);
  EnemyRunPreInstr(E->bang_var_F);
}

void Bang_Func_6(uint16 k) {  // 0xA3BCA5
  Enemy_Bang *E = Get_Bang(cur_enemy_index);
  uint16 v1 = E->bang_var_00 - 1;
  E->bang_var_00 = v1;
  if (!v1) {
    E->bang_var_00 = 16;
    E->bang_var_F = FUNC16(Bang_Func_8);
  }
}

void Bang_Func_7(uint16 k) {  // 0xA3BCC1
}

void Bang_Func_8(uint16 k) {  // 0xA3BCC5
  uint8 v1 = CalculateAngleOfSamusFromEnemy(cur_enemy_index) - 64;
  Enemy_Bang *E = Get_Bang(cur_enemy_index);
  E->bang_var_01 = v1;
  E->bang_var_F = FUNC16(Bang_Func_10);
  E->bang_var_07 = 0;
  E->bang_var_08 = 0;
  E->bang_var_09 = 0;
  E->bang_var_0A = 0;
}

void Bang_Func_9(void) {  // 0xA3BCF1
  uint8 v1 = CalculateAngleOfSamusFromEnemy(cur_enemy_index) - 64;
  Enemy_Bang *E = Get_Bang(cur_enemy_index);
  E->bang_var_02 = v1;
  uint16 v3 = SignExtend8(v1 - E->bang_var_01);
  uint16 v4 = Abs16(v3);
  if (!sign16(v4 - 48))
    E->bang_var_F = FUNC16(Bang_Func_11);
}

void Bang_Func_10(uint16 k) {  // 0xA3BD1C
  Bang_Func_14();
  Bang_Func_15();
  Bang_Func_12();
  Bang_Func_9();
}

void Bang_Func_11(uint16 k) {  // 0xA3BD2C
  Bang_Func_14();
  Bang_Func_15();
  Bang_Func_13();
  Enemy_Bang *E = Get_Bang(cur_enemy_index);
  if (E->bang_var_0A || (int16)E->bang_var_09 <= 0)
    E->bang_var_F = FUNC16(Bang_Func_6);
}

void Bang_Func_12(void) {  // 0xA3BD4F
  Enemy_Bang *E = Get_Bang(cur_enemy_index);
  int16 v1 = E->bang_var_0C - 1;
  E->bang_var_0C = v1;
  if (v1 < 0) {
    E->bang_var_0C = E->bang_var_0D;
    if ((int16)(E->bang_var_0A - E->bang_var_0B) < 0) {
      E->bang_var_09 += 22;
      E->bang_var_0A += E->bang_var_09;
    }
  }
}

void Bang_Func_13(void) {  // 0xA3BD89
  Enemy_Bang *E = Get_Bang(cur_enemy_index);
  int16 v1 = E->bang_var_0C - 1;
  E->bang_var_0C = v1;
  if (v1 < 0) {
    E->bang_var_0C = E->bang_var_0E;
    E->bang_var_09 -= 22;
    E->bang_var_0A -= E->bang_var_09;
  }
}

void Bang_Func_14(void) {  // 0xA3BDB9
  Enemy_Bang *E = Get_Bang(cur_enemy_index);
  uint16 r18 = kSine16bit[(uint8)(E->bang_var_01 + 64)];
  if ((r18 & 0x8000) != 0)
    E->bang_var_07 = 1;
  uint32 t = ((uint16)Abs16(r18) >> 8) * E->bang_var_0A;
  if (E->bang_var_07)
    t = -(int32)t;
  t = t + E->base.x_subpos;
  E->base.x_subpos = t, E->base.x_pos = t >> 16;
}

void Bang_Func_15(void) {  // 0xA3BE1C
  Enemy_Bang *E = Get_Bang(cur_enemy_index);
  uint16 r18 = kSine16bit[(uint8)E->bang_var_01];
  if ((r18 & 0x8000) != 0)
    E->bang_var_08 = 1;
  uint32 t = ((uint16)Abs16(r18) >> 8) * E->bang_var_0A;
  if (E->bang_var_08)
    t = -(int32)t;
  t = t + E->base.y_subpos;
  E->base.y_subpos = t, E->base.y_pos = t >> 16;
}

void Bang_Func_18(void) {  // 0xA3BEDA
  Enemy_Bang *E = Get_Bang(cur_enemy_index);
  uint16 bang_var_20 = E->bang_var_20;
  if (bang_var_20 != E->bang_var_21) {
    E->bang_var_21 = bang_var_20;
    E->base.current_instruction = g_off_A3B722[bang_var_20];
    E->base.instruction_timer = 1;
    E->base.timer = 0;
  }
}

void Bang_Shot(void) {  // 0xA3BEFD
  Enemy_Bang *E = Get_Bang(cur_enemy_index);
  if (E->bang_var_F != 0xBCC1) {
    E->bang_var_01 = g_word_A3BA94[projectile_dir[collision_detection_index] & 0xF];
    E->bang_var_F = FUNC16(Bang_Func_11);
    E->bang_var_07 = 0;
    E->bang_var_08 = 0;
    E->bang_var_09 = 256;
    E->bang_var_0A = 1536;
  }
  if (E->bang_var_20 != 9) {
    ++E->bang_var_20;
    Bang_Func_18();
    int v1 = collision_detection_index;
    E->bang_var_E += projectile_damage[v1];
    projectile_dir[v1] |= 0x10;
    if (E->bang_var_20 == 9)
      E->bang_var_D = 1;
  }
}
