// MaridiaBeybladeTurtle / MiniMaridiaBeybladeTurtle extracted from sm_a2.c.
#include "ida_types.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "variables.h"
#include "enemy_ai_canon.h"

#define g_word_A28D56 (*(uint16*)RomFixedPtr(0xa28d56))
#define g_word_A28D58 (*(uint16*)RomFixedPtr(0xa28d58))

static const uint16 g_word_A28D50 = 0x30;
static const uint16 g_word_A28D52 = 1;
static const uint16 g_word_A28D5E = 3;
static const uint16 g_word_A28D54 = 0x20;
static const uint16 g_word_A28D60 = 0x1e8;
static const uint16 g_word_A28D62 = 7;
static const uint16 g_word_A28D64 = 0x1e;
static const uint16 g_word_A28D66 = 4;
static const uint16 g_word_A28D68 = 0xfffd;
static const uint16 g_word_A28D6A = 3;

static const int16 g_word_A28E80[48] = {
  -16, -16, -16, -16, -15, -15, -15, -15,
  -15, -14, -13, -13, -12, -11, -10,  -9,
   -8,  -7,  -6,  -5,  -4,  -4,   0,   0,
  -16, -16, -16, -15, -15, -15, -14, -13,
  -12, -11, -10,  -9,  -8,  -7,  -6,  -5,
   -4,  -3,  -3,  -2,   0,   0,   0,   0,
};


void MaridiaBeybladeTurtle_Init(void) {  // 0xA28D6C
  Enemy_MaridiaBeybladeTurtle *E = Get_MaridiaBeybladeTurtle(cur_enemy_index);
  E->base.properties |= kEnemyProps_ProcessInstructions;
  E->base.spritemap_pointer = addr_kSpritemap_Nothing_A2;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->base.y_height = 0;
  E->base.current_instruction = addr_kMaridiaBeybladeTurtle_Ilist_8C44;
  E->mbte_var_A = FUNC16(MaridiaBeybladeTurtle_Func1);
  E->mbte_var_F = g_word_A28D52;
}

void MiniMaridiaBeybladeTurtle_Init(void) {  // 0xA28D9D
  int16 mmbte_parameter_1;

  Enemy_MiniMaridiaBeybladeTurtle *E = Get_MiniMaridiaBeybladeTurtle(cur_enemy_index);
  E->mmbte_var_C = E->base.x_pos;
  E->mmbte_var_D = E->base.y_pos
    - E->base.y_height;
  E->mmbte_var_A = FUNC16(MiniMaridiaBeybladeTurtle_Func1);
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  uint16 v1 = addr_kMaridiaBeybladeTurtle_Ilist_8B80;
  mmbte_parameter_1 = E->mmbte_parameter_1;
  E->mmbte_var_E = mmbte_parameter_1;
  if (mmbte_parameter_1 >= 0)
    v1 = addr_kMaridiaBeybladeTurtle_Ilist_8C72;
  E->base.current_instruction = v1;
}

void CallMaridiaBeybladeTurtleFunc(uint32 ea, uint16 k) {
  switch (ea) {
  case fnMaridiaBeybladeTurtle_Func1: MaridiaBeybladeTurtle_Func1(k); return;
  case fnMaridiaBeybladeTurtle_Func2: MaridiaBeybladeTurtle_Func2(k); return;
  case fnMaridiaBeybladeTurtle_Func3: MaridiaBeybladeTurtle_Func3(k); return;
  case fnMaridiaBeybladeTurtle_Func5: MaridiaBeybladeTurtle_Func5(k); return;
  case fnMaridiaBeybladeTurtle_Func7: MaridiaBeybladeTurtle_Func7(k); return;
  case fnMaridiaBeybladeTurtle_Func8: MaridiaBeybladeTurtle_Func8(k); return;
  case fnMaridiaBeybladeTurtle_Func9: MaridiaBeybladeTurtle_Func9(k); return;
  case fnMaridiaBeybladeTurtle_Func10: MaridiaBeybladeTurtle_Func10(k); return;
  case fnMaridiaBeybladeTurtle_Func11: MaridiaBeybladeTurtle_Func11(k); return;
  case fnnullsub_360: return;
  case fnMiniMaridiaBeybladeTurtle_Func1: MiniMaridiaBeybladeTurtle_Func1(k); return;
  case fnMiniMaridiaBeybladeTurtle_Func2: MiniMaridiaBeybladeTurtle_Func2(k); return;
  case fnMiniMaridiaBeybladeTurtle_Func3: MiniMaridiaBeybladeTurtle_Func3(k); return;
  case fnMiniMaridiaBeybladeTurtle_Func4: MiniMaridiaBeybladeTurtle_Func4(k); return;
  case fnMiniMaridiaBeybladeTurtle_Func6: MiniMaridiaBeybladeTurtle_Func6(k); return;
  case fnMiniMaridiaBeybladeTurtle_Func7: MiniMaridiaBeybladeTurtle_Func7(); return;
  default: Unreachable();
  }
}

void MaridiaBeybladeTurtle_Main(void) {  // 0xA28DD2
  Enemy_MaridiaBeybladeTurtle *E = Get_MaridiaBeybladeTurtle(cur_enemy_index);
  CallMaridiaBeybladeTurtleFunc(E->mbte_var_A | 0xA20000, cur_enemy_index);
}

void MaridiaBeybladeTurtle_Func1(uint16 k) {  // 0xA28DD8
  uint16 palette_index = Get_MaridiaBeybladeTurtle(k)->base.palette_index;
  Get_MaridiaBeybladeTurtle(k + 64)->base.palette_index = palette_index;
  Get_MaridiaBeybladeTurtle(k + 128)->base.palette_index = palette_index;
  Get_MaridiaBeybladeTurtle(k + 192)->base.palette_index = palette_index;
  Get_MaridiaBeybladeTurtle(k + 256)->base.palette_index = palette_index;
  uint16 vram_tiles_index = Get_MaridiaBeybladeTurtle(k)->base.vram_tiles_index;
  Enemy_MaridiaBeybladeTurtle *E1 = Get_MaridiaBeybladeTurtle(k + 64);
  E1->base.vram_tiles_index = vram_tiles_index;
  Enemy_MaridiaBeybladeTurtle *E2 = Get_MaridiaBeybladeTurtle(k + 128);
  E2->base.vram_tiles_index = vram_tiles_index;
  Enemy_MaridiaBeybladeTurtle *E3 = Get_MaridiaBeybladeTurtle(k + 192);
  E3->base.vram_tiles_index = vram_tiles_index;
  Enemy_MaridiaBeybladeTurtle *E4 = Get_MaridiaBeybladeTurtle(k + 256);
  E4->base.vram_tiles_index = vram_tiles_index;
  E1->mbte_var_B = k;
  E2->mbte_var_B = k;
  E3->mbte_var_B = k;
  E4->mbte_var_B = k;
  Get_MaridiaBeybladeTurtle(k)->mbte_var_A = FUNC16(MaridiaBeybladeTurtle_Func2);
}

void MaridiaBeybladeTurtle_Func2(uint16 k) {  // 0xA28E0A
  Enemy_MaridiaBeybladeTurtle *E = Get_MaridiaBeybladeTurtle(k);
  if (E->mbte_var_F) {
    E->base.y_height = 0;
    uint16 xd = E->base.x_pos - samus_x_pos;
    uint16 v2 = abs16(xd);
    if (sign16(v2 - 24)) {
      if (sign16(xd))
        v2 += 24;
      uint16 v3 = cur_enemy_index;
      E->base.y_height = -g_word_A28E80[v2];
      E->base.properties |= kEnemyProps_SolidToSamus;
      if (CheckIfEnemyTouchesSamus(v3)) {
        uint16 r18 = E->base.y_pos - E->base.y_height;
        if ((int16)(samus_y_radius + samus_y_pos - r18) >= 0)
          extra_samus_y_displacement += r18 - (samus_y_radius + samus_y_pos);
      }
    }
  } else {
    E->mbte_var_A = FUNC16(MaridiaBeybladeTurtle_Func3);
    E->base.properties &= ~kEnemyProps_Intangible;
  }
}

void MaridiaBeybladeTurtle_Func3(uint16 k) {  // 0xA28EE0
  MaridiaBeybladeTurtle_Func4();
  if ((nmi_frame_counter_byte & 1) == 0) {
    if (CheckIfEnemyTouchesSamus(k))
      --extra_samus_x_displacement;
    Enemy_MaridiaBeybladeTurtle *E = Get_MaridiaBeybladeTurtle(k);
    --E->base.y_pos;
    E->base.y_height = 16;
    if ((E->base.y_pos & 1) != 0)
      --E->base.x_pos;
    else
      ++E->base.x_pos;
    if (!(Enemy_MoveRight_IgnoreSlopes(k, INT16_SHL16(1)))) {
      E->base.current_instruction = addr_kMaridiaBeybladeTurtle_Ilist_8C4A;
      E->base.instruction_timer = 1;
      //*((uint16 *)RomPtr_A2(k) + 3) = g_word_A28D54; // WTF?
      E->mbte_var_A = addr_locret_A28E09;
    }
  }
}

void MaridiaBeybladeTurtle_Func5(uint16 k) {  // 0xA28F3F
  uint16 v1 = addr_kMaridiaBeybladeTurtle_Ilist_8C1C;
  Enemy_MaridiaBeybladeTurtle *E = Get_MaridiaBeybladeTurtle(k);
  if ((int16)(E->base.x_pos - samus_x_pos) < 0)
    v1 = addr_kMaridiaBeybladeTurtle_Ilist_8D00;
  E->base.current_instruction = v1;
  E->base.instruction_timer = 1;
  E->mbte_var_A = addr_locret_A28E09;
}

void MaridiaBeybladeTurtle_Func6(uint16 k) {  // 0xA28F5F
  if (CheckIfEnemyTouchesSamus(k)) {
    Enemy_MaridiaTurtle *E = Get_MaridiaTurtle(k);
    E->mte_var_A = FUNC16(MaridiaBeybladeTurtle_Func9);
    AddToHiLo(&extra_samus_x_displacement, &extra_samus_x_subdisplacement, -IPAIR32(E->mte_var_E, E->mte_var_03));
    if (sign16(extra_samus_x_displacement + 16))
      extra_samus_x_displacement = -16;
  }
}

void MaridiaBeybladeTurtle_Func7(uint16 k) {  // 0xA28F8D
  MaridiaBeybladeTurtle_Func4();
  if (!(Enemy_MoveDown(k, INT16_SHL16(-1)))) {
    if (CheckIfEnemyTouchesSamus(k))
      --extra_samus_y_displacement;
    Enemy_MaridiaTurtle *E = Get_MaridiaTurtle(k);
    uint16 v2 = E->mte_var_00 - 1;
    E->mte_var_00 = v2;
    if (!v2) {
      uint16 v3 = 0;
      if ((int16)(E->base.x_pos - samus_x_pos) < 0)
        v3 = 4;
      E->mte_var_01 = *(uint16 *)((uint8 *)&g_word_A28D56 + v3);
      E->mte_var_02 = *(uint16 *)((uint8 *)&g_word_A28D58 + v3);
      E->mte_var_E = 0;
      E->mte_var_03 = 0;
      E->mte_var_A = FUNC16(MaridiaBeybladeTurtle_Func8);
    }
  }
}

void MaridiaBeybladeTurtle_Func8(uint16 k) {  // 0xA28FEB
  MaridiaBeybladeTurtle_Func4();
  Enemy_MaridiaTurtle *E = Get_MaridiaTurtle(k);
  if (Enemy_MoveRight_IgnoreSlopes(k, __PAIR32__(E->mte_var_E, E->mte_var_03))) {
    SetHiLo(&E->mte_var_E, &E->mte_var_03, -IPAIR32(E->mte_var_E, E->mte_var_03));
    SetHiLo(&E->mte_var_02, &E->mte_var_01, -IPAIR32(E->mte_var_02, E->mte_var_01));
    earthquake_type = 0;
    earthquake_timer = 16;
    QueueSfx2_Max6(0x1B);
  } else {
    MaridiaBeybladeTurtle_Func6(k);
    AddToHiLo(&E->mte_var_E, &E->mte_var_03, __PAIR32__(E->mte_var_02, E->mte_var_01));
    if ((int16)(abs16(E->mte_var_E) - g_word_A28D5E) >= 0)
      E->mte_var_E = (E->mte_var_E & 0x8000) != 0 ? -g_word_A28D5E : g_word_A28D5E;
  }
}

void MaridiaBeybladeTurtle_Func9(uint16 k) {  // 0xA29083
  Enemy_MaridiaTurtle *E = Get_MaridiaTurtle(k);
  uint16 v2;
  MaridiaBeybladeTurtle_Func4();
  if ((int16)(E->base.y_pos - g_word_A28D60) < 0) {
    E->mte_var_00 = g_word_A28D64;
    v2 = FUNC16(MaridiaBeybladeTurtle_Func10);
  } else {
    if (CheckIfEnemyTouchesSamus(k)) {
      E->base.y_pos -= g_word_A28D62;
      extra_samus_y_displacement -= g_word_A28D62;
      return;
    }
    v2 = FUNC16(MaridiaBeybladeTurtle_Func11);
  }
  E->mte_var_A = v2;
  E->mte_var_07 = 0;
  E->mte_var_04 = 0;
}

void MaridiaBeybladeTurtle_Func10(uint16 k) {  // 0xA290CC
  MaridiaBeybladeTurtle_Func4();
  Enemy_MaridiaTurtle *E = Get_MaridiaTurtle(k);
  uint16 v2 = E->mte_var_00 - 1;
  E->mte_var_00 = v2;
  if (!v2)
    E->mte_var_A = FUNC16(MaridiaBeybladeTurtle_Func11);
}

void MaridiaBeybladeTurtle_Func11(uint16 k) {  // 0xA290E1
  MaridiaBeybladeTurtle_Func4();
  Enemy_MaridiaTurtle *E = Get_MaridiaTurtle(k);
  if ((int16)(Get_MaridiaTurtle(0)->mte_var_04 - g_word_A28D66) < 0)
    AddToHiLo(&E->mte_var_04, &E->mte_var_07, 0x2000);
  if (Enemy_MoveDown(k, INT16_SHL16(E->mte_var_04))) {
    E->base.current_instruction = (E->mte_var_E & 0x8000) == 0 ?
        addr_kMaridiaBeybladeTurtle_Ilist_8D28 : addr_kMaridiaBeybladeTurtle_Ilist_8C4A;
    E->base.instruction_timer = 1;
    E->mte_var_A = addr_locret_A28E09;
  }
}

void MiniMaridiaBeybladeTurtle_Main(void) {  // 0xA2912E
  Enemy_MiniMaridiaTurtle *E = Get_MiniMaridiaTurtle(cur_enemy_index);
  Get_MiniMaridiaTurtle(E->mmte_var_B)->mmte_var_06 = 0;
  CallMaridiaBeybladeTurtleFunc(E->mmte_var_A | 0xA20000, cur_enemy_index);
}

void MiniMaridiaBeybladeTurtle_Func1(uint16 k) {  // 0xA29142
  if (CheckIfEnemyTouchesSamus(k)) {
    Enemy_MiniMaridiaTurtle *E = Get_MiniMaridiaTurtle(k);
    E->mmte_var_A = FUNC16(MiniMaridiaBeybladeTurtle_Func2);
    E->mmte_var_05 = 4;
    uint16 v2 = addr_kMaridiaBeybladeTurtle_Ilist_8C30;
    if ((E->mmte_var_E & 0x8000) == 0)
      v2 = addr_kMaridiaBeybladeTurtle_Ilist_8D14;
    E->base.current_instruction = v2;
    E->base.instruction_timer = 1;
  }
}

void MiniMaridiaBeybladeTurtle_Func2(uint16 k) {  // 0xA2916E
  Enemy_MiniMaridiaTurtle *E = Get_MiniMaridiaTurtle(k);
  if (CheckIfEnemyTouchesSamus(k)) {
    E->mmte_var_05 = 4;
  } else {
    uint16 v2 = E->mmte_var_05 - 1;
    E->mmte_var_05 = v2;
    if (!v2) {
      E->mmte_var_A = FUNC16(MiniMaridiaBeybladeTurtle_Func3);
      E->mmte_var_00 = 60;
    }
  }
}

void MiniMaridiaBeybladeTurtle_Func3(uint16 k) {  // 0xA29198
  Enemy_MiniMaridiaTurtle *E = Get_MiniMaridiaTurtle(k);
  if (CheckIfEnemyTouchesSamus(k)) {
    E->mmte_var_A = FUNC16(MiniMaridiaBeybladeTurtle_Func4);
    E->base.current_instruction = addr_kMaridiaBeybladeTurtle_Ilist_8BD2;
    E->base.instruction_timer = 1;
    E->mmte_var_04 = 1;
    uint16 v2 = g_word_A28D68;
    if ((samus_pose_x_dir & 0xF) == 8)
      v2 = g_word_A28D6A;
    E->mmte_var_E = v2;
  } else {
    uint16 v4 = E->mmte_var_00 - 1;
    E->mmte_var_00 = v4;
    if (!v4) {
      uint16 v5 = addr_kMaridiaBeybladeTurtle_Ilist_8B80;
      if ((E->mmte_var_E & 0x8000) == 0)
        v5 = addr_kMaridiaBeybladeTurtle_Ilist_8C72;
      E->base.current_instruction = v5;
      E->base.instruction_timer = 1;
      E->mmte_var_A = FUNC16(MiniMaridiaBeybladeTurtle_Func1);
    }
  }
}

void MiniMaridiaBeybladeTurtle_Func4(uint16 k) {  // 0xA291F8
  Enemy_MiniMaridiaTurtle *E = Get_MiniMaridiaTurtle(k);
  if (Enemy_MoveRight_IgnoreSlopes(k, __PAIR32__(E->mmte_var_E, 0))) {
    E->mmte_var_E = -E->mmte_var_E;
  } else {
    Enemy_MoveDown(k, INT16_SHL16(E->mmte_var_04));
  }
}

void MiniMaridiaBeybladeTurtle_Func5(uint16 k) {  // 0xA2921D
  uint16 v1 = addr_kMaridiaBeybladeTurtle_Ilist_8B80;
  Enemy_MiniMaridiaBeybladeTurtle *E = Get_MiniMaridiaBeybladeTurtle(k);
  if ((E->mmbte_var_E & 0x8000) == 0)
    v1 = addr_kMaridiaBeybladeTurtle_Ilist_8C72;
  E->base.current_instruction = v1;
  E->base.instruction_timer = 1;
  E->mmbte_var_A = FUNC16(MiniMaridiaBeybladeTurtle_Func1);
}

void MiniMaridiaBeybladeTurtle_Func6(uint16 k) {  // 0xA29239
  if (CheckIfEnemyTouchesSamus(k)) {
    uint16 v1 = addr_kMaridiaBeybladeTurtle_Ilist_8B80;
    Enemy_MiniMaridiaBeybladeTurtle *E = Get_MiniMaridiaBeybladeTurtle(k);
    if ((E->mmbte_var_E & 0x8000) == 0)
      v1 = addr_kMaridiaBeybladeTurtle_Ilist_8C72;
    E->base.current_instruction = v1;
    E->base.instruction_timer = 1;
    E->mmbte_var_A = FUNC16(MiniMaridiaBeybladeTurtle_Func1);
  } else {
    MiniMaridiaBeybladeTurtle_Func4(k);
  }
}

void MiniMaridiaBeybladeTurtle_Func7(void) {  // 0xA2925E
  Enemy_MiniMaridiaTurtle *E = Get_MiniMaridiaTurtle(cur_enemy_index);
  Get_MiniMaridiaTurtle(E->mmte_var_B)->mmte_var_06 = E->base.y_height;
  if (!CheckIfEnemyTouchesSamus(cur_enemy_index))
    Get_MiniMaridiaTurtle(cur_enemy_index)->mmte_var_A = FUNC16(MiniMaridiaBeybladeTurtle_Func1);
}

void MaridiaBeybladeTurtle_Touch(void) {  // 0xA29281
  Enemy_MiniMaridiaTurtle *E = Get_MiniMaridiaTurtle(cur_enemy_index);
  if ((E->base.properties & kEnemyProps_SolidToSamus) == 0) {
    NormalEnemyTouchAi();
    E->mmte_var_A = FUNC16(MaridiaBeybladeTurtle_Func11);
    E->mmte_var_04 = 2;
  }
}

void MiniMaridiaBeybladeTurtle_Touch(void) {  // 0xA2929F
  int16 v2;

  Enemy_MiniMaridiaBeybladeTurtle *E = Get_MiniMaridiaBeybladeTurtle(cur_enemy_index);
  if (E->mmbte_var_A != 0x925E) {
    if ((E->mmbte_var_E & 0x8000) != 0) {
      E->base.current_instruction = addr_kMaridiaBeybladeTurtle_Ilist_8C72;
      v2 = 1;
    } else {
      E->base.current_instruction = addr_kMaridiaBeybladeTurtle_Ilist_8B80;
      v2 = -1;
    }
    E->mmbte_var_E = v2;
    E->base.instruction_timer = 1;
    uint16 v3;
    if ((int16)(E->base.x_pos - samus_x_pos) >= 0)
      v3 = E->base.x_width + samus_x_radius + samus_x_pos;
    else
      v3 = (__PAIR32__(samus_x_pos - samus_x_radius, samus_x_pos)
            - __PAIR32__(E->base.x_width, samus_x_radius)) >> 16;
    E->base.x_pos = v3;
    E->mmbte_var_A = FUNC16(MiniMaridiaBeybladeTurtle_Func1);
    Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, __PAIR32__(E->mmbte_var_E, 0));
    MaridiaBeybladeTurtle_Func21();
  }
}

void MaridiaBeybladeTurtle_Func21(void) {  // 0xA292FF
  uint16 mbte_var_B = Get_MaridiaBeybladeTurtle(cur_enemy_index)->mbte_var_B;
  Enemy_MaridiaBeybladeTurtle *E = Get_MaridiaBeybladeTurtle(mbte_var_B);
  if (E->mbte_var_F)
    --E->mbte_var_F;
}

void MiniMaridiaBeybladeTurtle_Shot(void) {  // 0xA2930F
  NormalEnemyShotAi();
  MaridiaBeybladeTurtle_Func21();
}

void MaridiaBeybladeTurtle_Func4(void) {  // 0xA29315
  Enemy_MaridiaBeybladeTurtle *E = Get_MaridiaBeybladeTurtle(0);
  uint16 r18 = E->base.x_pos - E->base.x_width - 8;
  uint16 r20 = E->base.x_width + E->base.x_pos + 8;
  uint16 r22 = E->base.y_pos - E->base.y_height + 4;
  uint16 r24 = E->base.y_height + E->base.y_pos - 4;
  if (sign16(samus_x_pos - samus_x_radius - 1 - r20)
      && !sign16(samus_x_radius + samus_x_pos - r18)
      && sign16(samus_y_pos - samus_y_radius + 1 - r24)
      && !sign16(samus_y_radius + samus_y_pos - r22)
      && !samus_invincibility_timer) {
    NormalEnemyTouchAi();
  }
}

const uint16 *MaridiaBeybladeTurtle_Instr_9381(uint16 k, const uint16 *jp) {  // 0xA29381
  uint16 R48 = 0;
  if (CheckIfEnemyTouchesSamus(cur_enemy_index)) {
    extra_samus_x_displacement += Get_MaridiaBeybladeTurtle(cur_enemy_index)->mbte_var_E;
    R48 = 1;
  }
  Enemy_MaridiaBeybladeTurtle *E = Get_MaridiaBeybladeTurtle(cur_enemy_index);
  uint16 R50 = E->base.y_pos;
  E->base.y_pos = E->mbte_var_D;
  Enemy_MoveRight_IgnoreSlopes(cur_enemy_index, INT16_SHL16(E->mbte_var_E));
  if (Get_MaridiaBeybladeTurtle(E->mbte_var_B)->mbte_var_A == 0x8E0A) {
    uint16 x_pos = Get_MaridiaBeybladeTurtle(E->mbte_var_B)->base.x_pos;
    uint16 xd = x_pos - E->base.x_pos;
    uint16 v7 = abs16(xd);
    uint16 v8;
    if (sign16(v7 - 24)) {
      if (sign16(xd))
        v7 += 24;
      v8 = g_word_A28E80[v7];
    } else {
      v8 = 1;
    }
    Enemy_MoveDown(cur_enemy_index, INT16_SHL16(v8));
    if (R48)
      extra_samus_y_displacement += Get_MaridiaBeybladeTurtle(cur_enemy_index)->base.y_pos - R50;
  }
  return jp;
}

const uint16 *MaridiaBeybladeTurtle_Instr_9412(uint16 k, const uint16 *jp) {  // 0xA29412
  int16 v4;

  Enemy_MaridiaBeybladeTurtle *E = Get_MaridiaBeybladeTurtle(cur_enemy_index);
  uint16 xd = E->mbte_var_C - E->base.x_pos;
  if ((int16)(abs16(xd) - g_word_A28D50) >= 0) {
    if (!sign16(xd))
      v4 = 1;
    else
      v4 = -1;
    E->mbte_var_E = v4;
  }
  if ((Get_MaridiaBeybladeTurtle(cur_enemy_index)->mbte_var_E & 0x8000) != 0)
    return INSTR_RETURN_ADDR(addr_kMaridiaBeybladeTurtle_Ilist_8B80);
  return INSTR_RETURN_ADDR(addr_kMaridiaBeybladeTurtle_Ilist_8C72);
}

const uint16 *MaridiaBeybladeTurtle_Instr_9447(uint16 k, const uint16 *jp) {  // 0xA29447
  Get_MaridiaBeybladeTurtle(cur_enemy_index)->mbte_var_A = FUNC16(MaridiaBeybladeTurtle_Func5);
  return jp;
}

const uint16 *MaridiaBeybladeTurtle_Instr_9451(uint16 k, const uint16 *jp) {  // 0xA29451
  Enemy_MaridiaTurtle *E = Get_MaridiaTurtle(cur_enemy_index);
  E->mte_var_A = FUNC16(MaridiaBeybladeTurtle_Func7);
  E->mte_var_E = -1;
  E->mte_var_00 = 16;
  return INSTR_RETURN_ADDR(addr_kMaridiaBeybladeTurtle_Ilist_8C02);
}

const uint16 *MaridiaBeybladeTurtle_Instr_946B(uint16 k, const uint16 *jp) {  // 0xA2946B
  Enemy_MaridiaTurtle *E = Get_MaridiaTurtle(cur_enemy_index);
  E->mte_var_A = FUNC16(MaridiaBeybladeTurtle_Func7);
  E->mte_var_E = 1;
  E->mte_var_00 = 16;
  return INSTR_RETURN_ADDR(addr_kMaridiaBeybladeTurtle_Ilist_8C02);
}

const uint16 *MaridiaBeybladeTurtle_Instr_9485(uint16 k, const uint16 *jp) {  // 0xA29485
  if (CheckIfEnemyTouchesSamus(cur_enemy_index)) {
    if ((Get_MaridiaBeybladeTurtle(cur_enemy_index)->mbte_var_E & 0x8000) == 0)
      return INSTR_RETURN_ADDR(addr_kMaridiaBeybladeTurtle_Ilist_8D40);
    else
      return INSTR_RETURN_ADDR(addr_kMaridiaBeybladeTurtle_Ilist_8C62);
  }
  return jp;
}

const uint16 *MaridiaBeybladeTurtle_Instr_94A1(uint16 k, const uint16 *jp) {  // 0xA294A1
  Enemy_MaridiaBeybladeTurtle *E = Get_MaridiaBeybladeTurtle(cur_enemy_index);
  if (CheckIfEnemyTouchesSamus(cur_enemy_index))
    E->mbte_var_A = FUNC16(MiniMaridiaBeybladeTurtle_Func7);
  else
    E->mbte_var_A = FUNC16(MiniMaridiaBeybladeTurtle_Func1);
  if ((E->mbte_var_E & 0x8000) == 0)
    return INSTR_RETURN_ADDR(addr_kMaridiaBeybladeTurtle_Ilist_8C72);
  return INSTR_RETURN_ADDR(addr_kMaridiaBeybladeTurtle_Ilist_8B80);
}

const uint16 *MaridiaBeybladeTurtle_Instr_94C7(uint16 k, const uint16 *jp) {  // 0xA294C7
  Enemy_MaridiaBeybladeTurtle *E = Get_MaridiaBeybladeTurtle(cur_enemy_index);
  E->mbte_var_A = FUNC16(MiniMaridiaBeybladeTurtle_Func6);
  return jp;
}

const uint16 *MaridiaBeybladeTurtle_Instr_94D1(uint16 k, const uint16 *jp) {  // 0xA294D1
  QueueSfx2_Max6(0x3A);
  return jp;
}
