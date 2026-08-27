// Enemy AI - Evir Projectile — peeled from Bank $A8

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

void CallMiniDraygonFunc(uint32 ea);

void EvirProjectile_Init(void) {  // 0xA888B0
  Enemy_MiniDraygon *E = Get_MiniDraygon(cur_enemy_index);
  E->mdn_var_02 = addr_kMiniDraygon_Ilist_876F;
  MiniDraygon_Func_12();
  int v2 = cur_enemy_index >> 1;
  E->base.palette_index = enemy_drawing_queue[v2 + 73];
  E->base.vram_tiles_index = enemy_drawing_queue[v2 + 74];
  MiniDraygon_Func_3();
  E->mdn_var_01 = 0;
  E->mdn_var_0C = 0;
  E->mdn_var_0B = 0;
  E->mdn_var_F = 0;
  E->mdn_var_C = FUNC16(MiniDraygon_Func_8);
}

void EvirProjectile_Main(void) {  // 0xA8899E
  Enemy_MiniDraygon *E = Get_MiniDraygon(cur_enemy_index);
  if (!E->base.frozen_timer) {
    if (E->mdn_var_0B) {
      E->mdn_var_02 = addr_kMiniDraygon_Ilist_876F;
      MiniDraygon_Func_12();
    } else if (E->mdn_var_0C) {
      E->mdn_var_02 = addr_kMiniDraygon_Ilist_8775;
      MiniDraygon_Func_12();
    } else {
      MiniDraygon_Func_7();
    }
  }
  CallMiniDraygonFunc(E->mdn_var_C | 0xA80000);
}

