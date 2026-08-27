// Enemy AI - Fire geyser — peeled from Bank $A6
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"

static const uint16 g_word_A68DBB = 5;

static const uint16 g_word_A68DBD = 0xa;

static const uint16 g_word_A68DBF = 0xf;

static const uint16 g_word_A68DC1 = 0x14;

static const uint16 g_word_A68DC3 = 0x19;

static const uint16 g_word_A68DC5 = 0x1e;

static const uint16 g_word_A68DC7 = 0x23;

static const uint16 g_word_A68DC9 = 0x28;

static const uint16 g_word_A68DCB = 0x2d;

static const uint16 g_word_A68DCD = 0x32;

static const uint16 g_word_A68DCF = 0x37;

static const uint16 g_word_A68DD1 = 0x3c;

static const uint16 g_word_A68DD3 = 0x41;

static const uint16 g_word_A68DD5 = 0x46;

static const uint16 g_word_A68DD7 = 0x4b;

static const uint16 g_word_A68DD9 = 0x50;

static const uint16 g_word_A68DDB = 0x55;

static const uint16 g_word_A68DDD = 0x5a;

static const uint16 g_word_A68DDF = 0x5f;

static const uint16 g_word_A68DE1 = 0x64;

static const uint16 g_word_A68DE3 = 0x69;

static const uint16 g_word_A68DE5 = 0x6e;

static const uint16 g_word_A68DE7 = 0x18;

static const uint16 g_word_A68DE9 = 0x18;

static const uint16 g_word_A68DEB = 0x18;

static const uint16 g_word_A68DED = 0x18;

static const uint16 g_word_A68DEF = 0x18;

static const uint16 g_word_A68DF1 = 0x18;

static const uint16 g_word_A68DF3 = 0x18;

static const uint16 g_word_A68DF5 = 0x18;

static const uint16 g_word_A68DF7 = 0x18;

static const uint16 g_word_A68DF9 = 0x18;

static const uint16 g_word_A68DFB = 0x18;

static const uint16 g_word_A68DFD = 0x18;

static const uint16 g_word_A68DFF = 0x18;

static const uint16 g_word_A68E01 = 0x18;

static const uint16 g_word_A68E03 = 0x18;

static const uint16 g_word_A68E05 = 0x18;

static const uint16 g_word_A68E07 = 0x18;

static const uint16 g_word_A68E09 = 0x18;

static const uint16 g_word_A68E0B = 0x14;

static const uint16 g_word_A68E0D = 0x10;

static const uint16 g_word_A68E0F = 0xc;

static const uint16 g_word_A68E11 = 8;

const uint16 *FireGeyser_Instr_1(uint16 k, const uint16 *jp) {  // 0xA68DAF
  QueueSfx2_Max6(0x61);
  return jp;
}

const uint16 *FireGeyser_Instr_2(uint16 k, const uint16 *jp) {  // 0xA68E13
  uint16 v2 = Get_FireGeyser(cur_enemy_index)->fgr_var_D - g_word_A68DBB;
  Enemy_FireGeyser *E = Get_FireGeyser(cur_enemy_index + 64);
  E->base.y_pos = v2;
  E->base.y_height = g_word_A68DE7;
  E->base.x_width = 8;
  return jp;
}

const uint16 *FireGeyser_Instr_3(uint16 k, const uint16 *jp) {  // 0xA68E2D
  uint16 v2 = Get_FireGeyser(cur_enemy_index)->fgr_var_D - g_word_A68DBD;
  Enemy_FireGeyser *E = Get_FireGeyser(cur_enemy_index + 64);
  E->base.y_pos = v2;
  E->base.y_height = g_word_A68DE9;
  return jp;
}

const uint16 *FireGeyser_Instr_4(uint16 k, const uint16 *jp) {  // 0xA68E41
  uint16 v2 = Get_FireGeyser(cur_enemy_index)->fgr_var_D - g_word_A68DBF;
  Enemy_FireGeyser *E = Get_FireGeyser(cur_enemy_index + 64);
  E->base.y_pos = v2;
  E->base.y_height = g_word_A68DEB;
  return jp;
}

const uint16 *FireGeyser_Instr_5(uint16 k, const uint16 *jp) {  // 0xA68E55
  uint16 v2 = Get_FireGeyser(cur_enemy_index)->fgr_var_D - g_word_A68DC1;
  Enemy_FireGeyser *E = Get_FireGeyser(cur_enemy_index + 64);
  E->base.y_pos = v2;
  E->base.y_height = g_word_A68DED;
  return jp;
}

const uint16 *FireGeyser_Instr_6(uint16 k, const uint16 *jp) {  // 0xA68E69
  uint16 v2 = Get_FireGeyser(cur_enemy_index)->fgr_var_D - g_word_A68DC3;
  Enemy_FireGeyser *E = Get_FireGeyser(cur_enemy_index + 64);
  E->base.y_pos = v2;
  E->base.y_height = g_word_A68DEF;
  return jp;
}

const uint16 *FireGeyser_Instr_7(uint16 k, const uint16 *jp) {  // 0xA68E7D
  uint16 v2 = Get_FireGeyser(cur_enemy_index)->fgr_var_D - g_word_A68DC5;
  Enemy_FireGeyser *E = Get_FireGeyser(cur_enemy_index + 64);
  E->base.y_pos = v2;
  E->base.y_height = g_word_A68DF1;
  return jp;
}

const uint16 *FireGeyser_Instr_8(uint16 k, const uint16 *jp) {  // 0xA68E91
  uint16 v2 = Get_FireGeyser(cur_enemy_index)->fgr_var_D - g_word_A68DC7;
  Enemy_FireGeyser *E = Get_FireGeyser(cur_enemy_index + 64);
  E->base.y_pos = v2;
  E->base.y_height = g_word_A68DF3;
  return jp;
}

const uint16 *FireGeyser_Instr_9(uint16 k, const uint16 *jp) {  // 0xA68EA5
  uint16 v2 = Get_FireGeyser(cur_enemy_index)->fgr_var_D - g_word_A68DC9;
  Enemy_FireGeyser *E = Get_FireGeyser(cur_enemy_index + 64);
  E->base.y_pos = v2;
  E->base.y_height = g_word_A68DF5;
  return jp;
}

const uint16 *FireGeyser_Instr_10(uint16 k, const uint16 *jp) {  // 0xA68EB9
  uint16 v2 = Get_FireGeyser(cur_enemy_index)->fgr_var_D - g_word_A68DCB;
  Enemy_FireGeyser *E = Get_FireGeyser(cur_enemy_index + 64);
  E->base.y_pos = v2;
  E->base.y_height = g_word_A68DF7;
  return jp;
}

const uint16 *FireGeyser_Instr_11(uint16 k, const uint16 *jp) {  // 0xA68ECD
  uint16 v2 = Get_FireGeyser(cur_enemy_index)->fgr_var_D - g_word_A68DCD;
  Enemy_FireGeyser *E = Get_FireGeyser(cur_enemy_index + 64);
  E->base.y_pos = v2;
  E->base.y_height = g_word_A68DF9;
  return jp;
}

const uint16 *FireGeyser_Instr_12(uint16 k, const uint16 *jp) {  // 0xA68EE1
  uint16 v2 = Get_FireGeyser(cur_enemy_index)->fgr_var_D - g_word_A68DCF;
  Enemy_FireGeyser *E = Get_FireGeyser(cur_enemy_index + 64);
  E->base.y_pos = v2;
  E->base.y_height = g_word_A68DFB;
  return jp;
}

const uint16 *FireGeyser_Instr_13(uint16 k, const uint16 *jp) {  // 0xA68EF5
  uint16 v2 = Get_FireGeyser(cur_enemy_index)->fgr_var_D - g_word_A68DD1;
  Enemy_FireGeyser *E = Get_FireGeyser(cur_enemy_index + 64);
  E->base.y_pos = v2;
  E->base.y_height = g_word_A68DFD;
  return jp;
}

const uint16 *FireGeyser_Instr_14(uint16 k, const uint16 *jp) {  // 0xA68F09
  uint16 v2 = Get_FireGeyser(cur_enemy_index)->fgr_var_D - g_word_A68DD3;
  Enemy_FireGeyser *E = Get_FireGeyser(cur_enemy_index + 64);
  E->base.y_pos = v2;
  E->base.y_height = g_word_A68DFF;
  return jp;
}

const uint16 *FireGeyser_Instr_15(uint16 k, const uint16 *jp) {  // 0xA68F1D
  uint16 v2 = Get_FireGeyser(cur_enemy_index)->fgr_var_D - g_word_A68DD5;
  Enemy_FireGeyser *E = Get_FireGeyser(cur_enemy_index + 64);
  E->base.y_pos = v2;
  E->base.y_height = g_word_A68E01;
  return jp;
}

const uint16 *FireGeyser_Instr_16(uint16 k, const uint16 *jp) {  // 0xA68F31
  uint16 v2 = Get_FireGeyser(cur_enemy_index)->fgr_var_D - g_word_A68DD7;
  Enemy_FireGeyser *E = Get_FireGeyser(cur_enemy_index + 64);
  E->base.y_pos = v2;
  E->base.y_height = g_word_A68E03;
  return jp;
}

const uint16 *FireGeyser_Instr_17(uint16 k, const uint16 *jp) {  // 0xA68F45
  uint16 v2 = Get_FireGeyser(cur_enemy_index)->fgr_var_D - g_word_A68DD9;
  Enemy_FireGeyser *E = Get_FireGeyser(cur_enemy_index + 64);
  E->base.y_pos = v2;
  E->base.y_height = g_word_A68E05;
  return jp;
}

const uint16 *FireGeyser_Instr_18(uint16 k, const uint16 *jp) {  // 0xA68F59
  uint16 v2 = Get_FireGeyser(cur_enemy_index)->fgr_var_D - g_word_A68DDB;
  Enemy_FireGeyser *E = Get_FireGeyser(cur_enemy_index + 64);
  E->base.y_pos = v2;
  E->base.y_height = g_word_A68E07;
  return jp;
}

const uint16 *FireGeyser_Instr_19(uint16 k, const uint16 *jp) {  // 0xA68F6D
  uint16 v2 = Get_FireGeyser(cur_enemy_index)->fgr_var_D - g_word_A68DDD;
  Enemy_FireGeyser *E = Get_FireGeyser(cur_enemy_index + 64);
  E->base.y_pos = v2;
  E->base.y_height = g_word_A68E09;
  return jp;
}

const uint16 *FireGeyser_Instr_20(uint16 k, const uint16 *jp) {  // 0xA68F81
  uint16 v2 = Get_FireGeyser(cur_enemy_index)->fgr_var_D - g_word_A68DDF;
  Enemy_FireGeyser *E = Get_FireGeyser(cur_enemy_index + 64);
  E->base.y_pos = v2;
  E->base.y_height = g_word_A68E0B;
  return jp;
}

const uint16 *FireGeyser_Instr_21(uint16 k, const uint16 *jp) {  // 0xA68F95
  uint16 v2 = Get_FireGeyser(cur_enemy_index)->fgr_var_D - g_word_A68DE1;
  Enemy_FireGeyser *E = Get_FireGeyser(cur_enemy_index + 64);
  E->base.y_pos = v2;
  E->base.y_height = g_word_A68E0D;
  return jp;
}

const uint16 *FireGeyser_Instr_22(uint16 k, const uint16 *jp) {  // 0xA68FA9
  uint16 v2 = Get_FireGeyser(cur_enemy_index)->fgr_var_D - g_word_A68DE3;
  Enemy_FireGeyser *E = Get_FireGeyser(cur_enemy_index + 64);
  E->base.y_pos = v2;
  E->base.y_height = g_word_A68E0F;
  return jp;
}

const uint16 *FireGeyser_Instr_23(uint16 k, const uint16 *jp) {  // 0xA68FBD
  uint16 v2 = Get_FireGeyser(cur_enemy_index)->fgr_var_D - g_word_A68DE5;
  Enemy_FireGeyser *E = Get_FireGeyser(cur_enemy_index + 64);
  E->base.y_pos = v2;
  E->base.y_height = g_word_A68E11;
  return jp;
}

const uint16 *FireGeyser_Instr_24(uint16 k, const uint16 *jp) {  // 0xA68FD1
  Enemy_FireGeyser *E = Get_FireGeyser(cur_enemy_index);
  E->fgr_var_C = 1;
  Enemy_FireGeyser *E1 = Get_FireGeyser(cur_enemy_index + 64);
  E1->base.x_width = 0;
  E1->base.y_height = 0;
  E->base.y_pos = E->fgr_var_D;
  E->base.properties |= kEnemyProps_Invisible;
  E1->base.properties |= 0x400;
  return jp;
}

void FireGeyser_Init(void) {  // 0xA68FFC
  Enemy_FireGeyser *E = Get_FireGeyser(cur_enemy_index);
  E->base.current_instruction = addr_kFireGeyser_Ilist_8DA9;
  if (!E->fgr_parameter_2) {
    E->base.current_instruction = addr_kFireGeyser_Ilist_8D1B;
    E->fgr_var_A = FUNC16(FireGeyser_Func_1);
    E->fgr_var_D = E->base.y_pos;
    E->base.x_width = 0;
  }
}

void CallFireGeyserFunc(uint32 ea) {
  switch (ea) {
  case fnFireGeyser_Func_1: FireGeyser_Func_1(); return;
  case fnFireGeyser_Func_2: FireGeyser_Func_2(); return;
  default: Unreachable();
  }
}

void FireGeyser_Main(void) {  // 0xA69023
  Enemy_FireGeyser *E = Get_FireGeyser(cur_enemy_index);
  if (!E->fgr_parameter_2)
    CallFireGeyserFunc(E->fgr_var_A | 0xA60000);
}

void FireGeyser_Func_1(void) {  // 0xA6902F
  Enemy_FireGeyser *E = Get_FireGeyser(cur_enemy_index);
  if ((--E->fgr_var_B & 0x8000) != 0) {
    E->fgr_var_A = FUNC16(FireGeyser_Func_2);
    E->fgr_var_C = 0;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
    E->base.current_instruction = addr_kFireGeyser_Ilist_8D1B;
    E->base.properties &= ~kEnemyProps_Invisible;
    Enemy_FireGeyser *E1 = Get_FireGeyser(cur_enemy_index + 64);
    E1->base.properties &= ~0x400;
  }
}

void FireGeyser_Func_2(void) {  // 0xA69062
  Enemy_FireGeyser *E = Get_FireGeyser(cur_enemy_index);
  if (E->fgr_var_C) {
    E->fgr_var_B = E->fgr_parameter_1;
    E->base.properties |= kEnemyProps_Invisible;
    E->fgr_var_A = FUNC16(FireGeyser_Func_1);
  }
}
