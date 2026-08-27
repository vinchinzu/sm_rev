// Enemy AI - Ridley boss + explosion — peeled from Bank $A6
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"

#define g_word_A6E46A ((uint16*)RomFixedPtr(0xa6e46a))
#define g_word_A6B60D ((uint16*)RomFixedPtr(0xa6b60d))
#define g_word_A6B63B ((uint16*)RomFixedPtr(0xa6b63b))
#define g_word_A6B6C8 ((uint16*)RomFixedPtr(0xa6b6c8))
#define g_word_A6B9D5 ((uint16*)RomFixedPtr(0xa6b9d5))
#define g_word_A6B9DB ((uint16*)RomFixedPtr(0xa6b9db))
#define g_off_A6B965 ((uint16*)RomFixedPtr(0xa6b965))
#define g_off_A6B96D ((uint16*)RomFixedPtr(0xa6b96d))
#define g_word_A6B9D5 ((uint16*)RomFixedPtr(0xa6b9d5))
#define g_word_A6B9DB ((uint16*)RomFixedPtr(0xa6b9db))
#define g_word_A6B94D ((uint16*)RomFixedPtr(0xa6b94d))
#define g_word_A6B959 ((uint16*)RomFixedPtr(0xa6b959))
#define g_word_A6BB48 ((uint16*)RomFixedPtr(0xa6bb48))
#define g_word_A6BB4E ((uint16*)RomFixedPtr(0xa6bb4e))
#define g_word_A6C1DF ((uint16*)RomFixedPtr(0xa6c1df))
#define g_off_A6C7BA ((uint16*)RomFixedPtr(0xa6c7ba))
#define g_word_A6C804 ((uint16*)RomFixedPtr(0xa6c804))
#define g_off_A6C808 ((uint16*)RomFixedPtr(0xa6c808))
#define g_word_A6C836 ((uint16*)RomFixedPtr(0xa6c836))
#define g_off_A6C83A ((uint16*)RomFixedPtr(0xa6c83a))
#define g_word_A6C868 ((uint16*)RomFixedPtr(0xa6c868))
#define g_off_A6C86C ((uint16*)RomFixedPtr(0xa6c86c))
#define g_word_A6C89A ((uint16*)RomFixedPtr(0xa6c89a))
#define g_off_A6C89E ((uint16*)RomFixedPtr(0xa6c89e))
#define g_word_A6C8CC ((uint16*)RomFixedPtr(0xa6c8cc))
#define g_off_A6C8D0 ((uint16*)RomFixedPtr(0xa6c8d0))
#define g_word_A6C6CE ((uint16*)RomFixedPtr(0xa6c6ce))
#define g_off_A6C6E6 ((uint16*)RomFixedPtr(0xa6c6e6))
#define g_word_A6CC12 ((uint16*)RomFixedPtr(0xa6cc12))
#define g_word_A6CC18 ((uint16*)RomFixedPtr(0xa6cc18))
#define g_off_A6DB02 ((uint16*)RomFixedPtr(0xa6db02))
#define kRidley_Ilist_DCBA ((uint16*)RomFixedPtr(0xa6dcba))

static uint8 Ridley_Func_40_Carry();
static uint8 Ridley_Func_40_Sign();

static const uint16 g_word_A6B288 = 8;

static const uint16 g_word_A6BBEB[3] = { 0x40, 0, 0xd0 };

static const uint16 g_word_A6BC62[3] = { 0xb0, 0, 0x50 };

static const int16 g_word_A6C66E[20] = { -24, -24, -20, 20, 16, -30, 30, -3, 14, -13, -2, 18, -2, -32, -31, 8, -4, -10, 19, 19 };

static const int16 g_word_A6CF54[3] = { 0x20, 0, -0x20 };

static const uint8 g_byte_A6D61F[16] = { 0x10, 0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 9, 8, 7, 6, 5, 4, 3, 2, 1 };

static const uint8 g_byte_A6D712[16] = { 0x10, 0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 9, 8, 7, 6, 5, 4, 3, 2, 1 };

static const uint16 g_word_A6D9ED[8] = { 0xc, 0xe, 0x10, 0x12, 0x1c, 0x20, 0x28, 0x30 };

void CallRidleyFunc(uint32 ea) {
  switch (ea) {
  case fnCeresRidley_Func_2: CeresRidley_Func_2(); return;
  case fnCeresRidley_Func_3: CeresRidley_Func_3(); return;
  case fnCeresRidley_A377: CeresRidley_A377(); return;
  case fnCeresRidley_Func_4: CeresRidley_Func_4(); return;
  case fnCeresRidley_Func_5: CeresRidley_Func_5(); return;
  case fnCeresRidley_Func_6: CeresRidley_Func_6(); return;
  case fnCeresRidley_Func_7: CeresRidley_Func_7(); return;
  case fnCeresRidley_Func_9: CeresRidley_Func_9(); return;
  case fnCeresRidley_Func_10: CeresRidley_Func_10(); return;
  case fnCeresRidley_Func_11: CeresRidley_Func_11(); return;
  case fnCeresRidley_Func_13: CeresRidley_Func_13(); return;
  case fnCeresRidley_A7F9: CeresRidley_A7F9(); return;
  case fnCeresRidley_Func_14: CeresRidley_Func_14(); return;
  case fnCeresRidley_Func_15: CeresRidley_Func_15(); return;
  case fnCeresRidley_Func_16: CeresRidley_Func_16(); return;
  case fnCeresRidley_Func_17: CeresRidley_Func_17(); return;
  case fnCeresRidley_Func_18: CeresRidley_Func_18(); return;
  case fnCeresRidley_Func_19: CeresRidley_Func_19(); return;
  case fnCeresRidley_Func_20: CeresRidley_Func_20(); return;
  case fnCeresRidley_Func_21: CeresRidley_Func_21(); return;
  case fnCeresRidley_Func_22: CeresRidley_Func_22(); return;
  case fnCeresRidley_Func_23: CeresRidley_Func_23(); return;
  case fnCeresRidley_Func_24: CeresRidley_Func_24(); return;
  case fnnullsub_233: return;
  case fnCeresRidley_Func_26: CeresRidley_Func_26(); return;
  case fnRidley_Func_3: Ridley_Func_3(); return;
  case fnRidley_Func_3b: Ridley_Func_3b(); return;
  case fnRidley_Func_5: Ridley_Func_5(); return;
  case fnRidley_Func_6: Ridley_Func_6(); return;
  case fnRidley_Func_8: Ridley_Func_8(); return;
  case fnRidley_B455: Ridley_B455(); return;
  case fnRidley_Func_9: Ridley_Func_9(); return;
  case fnRidley_Func_10: Ridley_Func_10(); return;
  case fnRidley_Func_11: Ridley_Func_11(); return;
  case fnRidley_Func_12: Ridley_Func_12(); return;
  case fnRidley_Func_13: Ridley_Func_13(); return;
  case fnRidley_Func_14: Ridley_Func_14(); return;
  case fnRidley_Func_15: Ridley_Func_15(); return;
  case fnRidley_Func_16: Ridley_Func_16(); return;
  case fnRidley_Func_19: Ridley_Func_19(); return;
  case fnRidley_Func_20: Ridley_Func_20(); return;
  case fnRidley_Func_21: Ridley_Func_21(); return;
  case fnRidley_Func_22: Ridley_Func_22(); return;
  case fnRidley_Func_33: Ridley_Func_33(); return;
  case fnRidley_Func_33b: Ridley_Func_33b(); return;
  case fnRidley_Func_34: Ridley_Func_34(); return;
  case fnRidley_Func_35: Ridley_Func_35(); return;
  case fnRidley_Func_36: Ridley_Func_36(); return;
  case fnRidley_Func_43: Ridley_Func_43(); return;
  case fnRidley_Func_44: Ridley_Func_44(); return;
  case fnRidley_Func_45: Ridley_Func_45(); return;
  case fnRidley_Func_46: Ridley_Func_46(); return;
  case fnRidley_Func_47: Ridley_Func_47(); return;
  case fnRidley_Func_54: Ridley_Func_54(); return;
  case fnRidley_Func_63: Ridley_Func_63(); return;
  case fnRidley_C53E: Ridley_C53E(); return;
  case fnRidley_C551: Ridley_C551(); return;
  case fnRidley_Func_64: Ridley_Func_64(); return;
  case fnRidley_Func_65: Ridley_Func_65(); return;
  case fnRidley_Func_66: Ridley_Func_66(); return;
  case fnRidley_Func_67: Ridley_Func_67(); return;
  case fnnullsub_349: return;
  default: Unreachable();
  }
}

void Ridley_A2DC(void) {  // 0xA6A2DC
  Ridley_Func_49();
  if (Get_CeresRidley(0)->cry_var_02) {
    mov24(&enemy_gfx_drawn_hook, 0xA6A2F2);
  } else {
    Ridley_A2F2();
  }
}

void Ridley_A2F2(void) {  // 0xA6A2F2
  if (!ceres_status)
    DrawBabyMetroid_0();
  Enemy_CeresRidley *E = Get_CeresRidley(0x40);
  if (E->cry_var_B) {
    // bug
    static const int16 g_word_A6A321[4] = { 0, -1024, -4, -1 };
    DrawSpritemap(0xA6, addr_kCeresRidley_Sprmap_A329, 
      E->base.x_pos + g_word_A6A321[earthquake_timer & 3], E->base.y_pos, 1024);
  }
}

void Ridley_Main(void) {  // 0xA6B227
  int16 v1;

  Enemy_Ridley *E = Get_Ridley(0);
  v1 = E->ridley_var_24 - 4;
  if (v1 < 0)
    v1 = 0;
  E->ridley_var_24 = v1;
  Ridley_Func_39();
  Ridley_Func_42();
  CallRidleyFunc(E->ridley_var_A | 0xA60000);
  if (E->ridley_var_02) {
    Ridley_Func_102();
    Ridley_Func_112();
    Ridley_Func_115();
    Ridley_Func_70();
    Ridley_Func_129();
    sub_A6DB2A();
    Ridley_Func_120();
    Ridley_Func_118();
    if (E->ridley_var_1B)
      Ridley_Func_30();
  }
  Ridley_Func_100();
}

void Ridley_Func_1(void) {  // 0xA6B26F
  Enemy_Ridley *E = Get_Ridley(0);
  if (!(E->ridley_var_1B | E->ridley_var_1E)) {
    if (Ridley_Func_26(4, 4) & 1)
      Ridley_Func_37();
  }
}

void Ridley_Func_2(void) {  // 0xA6B28A
  Enemy_Ridley *E = Get_Ridley(0);
  E->ridley_var_24 = 0;
  E->base.frame_counter = 1;
  Ridley_Hurt();
}

void Ridley_Hurt(void) {  // 0xA6B297
  Enemy_Ridley *E = Get_Ridley(0);
  if ((E->base.frame_counter & 1) == 0) {
    Ridley_Func_39();
    Ridley_Func_42();
    CallRidleyFunc(E->ridley_var_A | 0xA60000);
    if (!Get_Ridley(0)->ridley_var_02) {
LABEL_7:
      if (Get_Ridley(0)->ridley_var_1B)
        Ridley_Func_30();
      return;
    }
    Ridley_Func_112();
    Ridley_Func_115();
    Ridley_Func_70();
  }
  Ridley_Func_129();
  Ridley_Func_102();
  sub_A6DB2A();
  Ridley_Func_120();
  Ridley_Func_100();
  Ridley_Func_118();
  uint16 v2 = E->ridley_var_24 + 1;
  if ((int16)(v2 - g_word_A6B288) >= 0)
    v2 = g_word_A6B288;
  E->ridley_var_24 = v2;
  if ((int16)(E->ridley_var_24 - g_word_A6B288) >= 0)
    goto LABEL_7;
}

void Ridley_Func_3(void) {  // 0xA6B2F3
  Rect16U rect = { 64, 256, 8, 8 };
  Ridley_Func_104_0(0, 14, rect.x, rect.y);
  if (!Shitroid_Func_2(0, rect)) {
    Enemy_Ridley *E = Get_Ridley(0);
    E->ridley_var_01 = 1;
    E->ridley_var_A = FUNC16(Ridley_Func_3b);
    Ridley_Func_3b();
  }
}

void Ridley_Func_3b(void) {  // 0xA6B321
  uint16 r18 = Ridley_Func_4();
  int v0 = 2 * (NextRandom() & 7);
  uint16 v1 = *(uint16 *)&RomPtr_A6(r18)[v0];
  Enemy_Ridley *E = Get_Ridley(0);
  E->ridley_var_A = v1;
  CallRidleyFunc(E->ridley_var_A | 0xA60000);
}

uint16 Ridley_Func_4(void) {  // 0xA6B335
  uint16 r18;
  if (samus_movement_type == 3) {
    r18 = addr_off_A6B3CC;
  } else {
    Enemy_Ridley *E = Get_Ridley(0);
    uint16 health = E->base.health;
    if (health) {
      if (sign16(health - 14400)) {
        r18 = addr_off_A6B38C;
      } else if (Ridley_Func_41() & 1) {
        r18 = addr_off_A6B3BC;
      } else if ((Ridley_Func_40_Carry() & 1) != 0) {
        r18 = addr_off_A6B3AC;
      } else {
        r18 = sign16(E->base.health - 9000) ? addr_off_A6B39C : addr_off_A6B38C;
      }
    } else {
      r18 = addr_off_A6B3DC;
      ++E->ridley_var_25;
    }
  }
  return r18;
}

void Ridley_Func_5(void) {  // 0xA6B3EC
  Enemy_Ridley *E = Get_Ridley(0);
  E->ridley_var_A = FUNC16(Ridley_Func_6);
  E->ridley_var_F = 128;
  Ridley_Func_6();
}

void Ridley_Func_6(void) {  // 0xA6B3F8
  Enemy_Ridley *E = Get_Ridley(0);
  if ((--E->ridley_var_F & 0x8000) != 0)
    goto LABEL_5;
  uint16 v2;
  v2 = 192;
  if (E->ridley_var_10)
    v2 = 96;
  Rect16U rect = { v2, 256, 8, 8 };
  Ridley_Func_104_0(0, Ridley_Func_7(), rect.x, rect.y);
  if (!Shitroid_Func_2(0, rect)) {
LABEL_5:
    E->ridley_var_A = FUNC16(Ridley_Func_3b);
  }
}

uint16 Ridley_Func_7(void) {  // 0xA6B42E
  static const uint16 g_word_A6B439[4] = { 4, 8, 0xa, 0xc };
  return g_word_A6B439[Get_Ridley(0)->ridley_var_12];
}

void Ridley_Func_8(void) {  // 0xA6B441
  Enemy_Ridley *E = Get_Ridley(0);
  E->ridley_var_A = addr_loc_A6B455;
  E->ridley_var_00 = 10;
  E->ridley_var_0A = 0;
  Ridley_B455();
}

void Ridley_B455(void) {  // 0xA6B455
  Enemy_Ridley *E = Get_Ridley(0);
  uint16 v1 = 192;
  if (E->ridley_var_10)
    v1 = 64;
  Rect16U rect = { v1, 128, 8, 8 };
  Ridley_Func_104_0(0, 1, rect.x, rect.y);
  if (!Shitroid_Func_2(0, rect)) {
    E->ridley_var_A = FUNC16(Ridley_Func_9);
    E->ridley_var_00 = 32;
    E->ridley_var_0A = 0;
  }
}

void Ridley_Func_9(void) {  // 0xA6B493
  Enemy_Ridley *E = Get_Ridley(0);
  if (E->ridley_var_10) {
    Ridley_Func_111(32, 512, 1152);
  } else {
    Ridley_Func_111(-32, -512, 1152);
  }
  uint16 ridley_var_00 = E->ridley_var_00;
  if (ridley_var_00) {
    E->ridley_var_00 = ridley_var_00 - 1;
  } else {
    E->ridley_var_A = FUNC16(Ridley_Func_10);
    E->ridley_var_00 = 20;
  }
}

void Ridley_Func_10(void) {  // 0xA6B4D1
  Enemy_Ridley *E = Get_Ridley(0);
  if (E->ridley_var_10)
    Ridley_Func_111(320, 0x4000, 1280);
  else
    Ridley_Func_111(-320, -16384, 1280);
  uint16 ridley_var_00 = E->ridley_var_00;
  if (ridley_var_00) {
    E->ridley_var_00 = ridley_var_00 - 1;
  } else {
    E->ridley_var_A = FUNC16(Ridley_Func_11);
    E->ridley_var_00 = 16;
    tilemap_stuff[2] = 1;
  }
}

void Ridley_Func_11(void) {  // 0xA6B516
  Enemy_Ridley *E = Get_Ridley(0);
  if (E->ridley_var_10)
    Ridley_Func_111(512, 30720, 768);
  else
    Ridley_Func_111(-512, -30720, 768);
  uint16 ridley_var_00 = E->ridley_var_00;
  if (ridley_var_00) {
    E->ridley_var_00 = ridley_var_00 - 1;
  } else {
    E->ridley_var_A = FUNC16(Ridley_Func_12);
    E->ridley_var_00 = 32;
  }
}

void Ridley_Func_12(void) {  // 0xA6B554
  Enemy_Ridley *E = Get_Ridley(0);
  if (E->ridley_var_10) {
    Ridley_Func_111(1024, 30720, 768); 
  } else {
    Ridley_Func_111(-1024, -30720, 768);
  }
  uint16 ridley_var_00 = E->ridley_var_00;
  if (ridley_var_00) {
    E->ridley_var_00 = ridley_var_00 - 1;
  } else {
    E->ridley_var_A = FUNC16(Ridley_Func_13);
    E->ridley_var_00 = 32;
    Ridley_Func_114();
  }
}

void Ridley_Func_13(void) {  // 0xA6B594
  Ridley_Func_111(0, 0x8000, 448);
  Enemy_Ridley *E = Get_Ridley(0);
  uint16 ridley_var_00 = E->ridley_var_00;
  if (ridley_var_00) {
    E->ridley_var_00 = ridley_var_00 - 1;
  } else {
    E->ridley_var_A = Ridley_Func_40_Carry() ? FUNC16(Ridley_Func_33) : FUNC16(Ridley_Func_3b);
  }
}

void Ridley_Func_14(void) {  // 0xA6B5C4
  tilemap_stuff[15] = 11;
  tilemap_stuff[9] = 384;
  Enemy_Ridley *E = Get_Ridley(0);
  E->ridley_var_A = FUNC16(Ridley_Func_15);
  E->ridley_var_F = (random_number & 0x1F) + 32;
  Ridley_Func_15();
}

void Ridley_Func_15(void) {  // 0xA6B5E5
  Enemy_Ridley *E = Get_Ridley(0);
  if (Ridley_Func_17(g_word_A6B60D[E->ridley_var_10]) & 1) {
    Ridley_Func_18();
  } else if ((--E->ridley_var_F & 0x8000) != 0) {
    E->ridley_var_A = FUNC16(Ridley_Func_16);
    E->ridley_var_F = 128;
    Ridley_Func_114();
  }
}

void Ridley_Func_16(void) {  // 0xA6B613
  Enemy_Ridley *E = Get_Ridley(0);
  if (Ridley_Func_17(g_word_A6B63B[E->ridley_var_10]) & 1) {
    Ridley_Func_18();
  } else if ((--E->ridley_var_F & 0x8000) != 0) {
    E->ridley_var_A = FUNC16(Ridley_Func_15);
    E->ridley_var_F = 128;
    Ridley_Func_114();
  }
}

uint8 Ridley_Func_17(uint16 r18) {  // 0xA6B641
  uint16 v0 = samus_y_pos;
  if (!sign16(samus_y_pos - 352))
    v0 = 352;
  Ridley_Func_104_0(0, Ridley_Func_7(), r18, v0);
  tilemap_stuff[2] = 1;
  if (samus_movement_type != 3)
    return 1;
  if ((uint8)random_number >= 0x80) {
    Enemy_Ridley *E = Get_Ridley(0);
    if (!E->ridley_var_0F && E->ridley_var_10 != 1)
      Ridley_Func_99(addr_kRidley_Ilist_E73A);
  }
  return 0;
}

void Ridley_Func_18(void) {  // 0xA6B68B
  tilemap_stuff[9] = 240;
  tilemap_stuff[15] = 16;
  tilemap_stuff[0] = 1;
  Get_Ridley(0)->ridley_var_A = FUNC16(Ridley_Func_19);
}

void Ridley_Func_19(void) {  // 0xA6B6A7
  Enemy_Ridley *E = Get_Ridley(0);
  if (sign16(E->base.y_pos - 288)) {
    Ridley_Func_114();
    E->ridley_var_A = FUNC16(Ridley_Func_20);
    E->ridley_var_F = 32;
    Ridley_Func_20();
  } else {
    Ridley_Func_104_0(0, 0, g_word_A6B6C8[E->ridley_var_10], 288);
  }
}

void Ridley_Func_20(void) {  // 0xA6B6DD
  Enemy_Ridley *E = Get_Ridley(0);
  Ridley_Func_104_0(0, 0, E->base.x_pos, 288);
  if ((--E->ridley_var_F & 0x8000) != 0) {
    Ridley_Func_71();
    Ridley_Func_29();
    E->ridley_var_A = FUNC16(Ridley_Func_21);
    E->ridley_var_F = (random_number & 0x3F) + 128;
  }
}

void Ridley_Func_21(void) {  // 0xA6B70E
  int16 v1;

  if (Ridley_Func_25() & 1) {
    Ridley_Func_24();
    Ridley_Func_27();
  } else {
    Enemy_Ridley *E = Get_Ridley(0);
    v1 = tilemap_stuff[6] + E->ridley_var_C;
    if (!sign16(v1 - 1536))
      v1 = 1536;
    E->ridley_var_C = v1;
    if (Ridley_Func_23() & 1) {
      eproj_spawn_pt = (Point16U) { tilemap_stuff[82], tilemap_stuff[83] + 12 };
      SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 9);
      QueueSfx2_Max6(0x76);
      earthquake_type = 13;
      earthquake_timer = 4;
      Ridley_Func_28();
      Ridley_Func_29();
      Ridley_Func_93(0x2026, addr_word_A6D37C);
      tilemap_stuff[20] = 3072;
      tilemap_stuff[30] = 3072;
      tilemap_stuff[40] = 3072;
      tilemap_stuff[50] = 3072;
      tilemap_stuff[60] = 3072;
      tilemap_stuff[70] = 3072;
      tilemap_stuff[80] = 3072;
      tilemap_stuff[0] = 4;
      uint16 v2 = E->ridley_var_26 + 1;
      if (!sign16(E->ridley_var_26 - 1)) {
        if (E->ridley_var_10 != 1)
          Ridley_Func_99(addr_kRidley_Ilist_E73A);
        v2 = 0;
      }
      E->ridley_var_26 = v2;
      E->ridley_var_A = FUNC16(Ridley_Func_22);
    }
  }
}

void Ridley_Func_22(void) {  // 0xA6B7B9
  Enemy_Ridley *E = Get_Ridley(0);
  bool v1;
  if (!(Ridley_Func_41() & 1)
      || (v1 = (int16)(E->ridley_var_F - 1) < 0, --E->ridley_var_F, v1)) {
    Ridley_Func_24();
    E->ridley_var_A = FUNC16(Ridley_Func_3b);
  } else {
    uint16 ridley_var_C = E->ridley_var_C;
    v1 = (int16)(tilemap_stuff[7] + ridley_var_C) < 0;
    E->ridley_var_C = tilemap_stuff[7] + ridley_var_C;
    if (!v1) {
      E->ridley_var_C = 0;
      E->ridley_var_A = FUNC16(Ridley_Func_21);
    }
  }
}

uint8 Ridley_Func_23(void) {  // 0xA6B7E7
  uint8 v0 = Ridley_Func_103(tilemap_stuff[82], tilemap_stuff[83] + 16) & 1;
  if (!v0) {
    v0 = Ridley_Func_103(tilemap_stuff[72], tilemap_stuff[73] + 18) & 1;
    if (!v0) {
      v0 = Ridley_Func_103(tilemap_stuff[62], tilemap_stuff[63] + 18) & 1;
      if (!v0) {
        v0 = Ridley_Func_103(tilemap_stuff[52], tilemap_stuff[53] + 18) & 1;
        if (!v0)
          return Ridley_Func_103(tilemap_stuff[42], tilemap_stuff[43] + 18) & 1;
      }
    }
  }
  return v0;
}

void Ridley_Func_24(void) {  // 0xA6B84D
  tilemap_stuff[0] = 1;
  tilemap_stuff[10] = 1;
}

uint8 Ridley_Func_25(void) {  // 0xA6B859
  if ((Ridley_Func_40_Carry() & 1) != 0)
    return Ridley_Func_26(4, 4) & 1;
  else
    return 0;
}

uint8 Ridley_Func_26(uint16 k, uint16 j) {  // 0xA6B865
  Enemy_Ridley *E = Get_Ridley(0);
  Rect16U rect = {
    E->base.x_pos + g_word_A6B9D5[E->ridley_var_10], E->base.y_pos + g_word_A6B9DB[E->ridley_var_1D >> 1],
    k, j
  };
  return Ridley_Func_124(rect);
}

void Ridley_Func_27(void) {  // 0xA6B889
  int16 ridley_var_C;

  Enemy_Ridley *E = Get_Ridley(0);
  ridley_var_C = E->ridley_var_C;
  if (ridley_var_C >= 0)
    ridley_var_C = -ridley_var_C;
  if (!sign16(ridley_var_C + 512))
    ridley_var_C = -512;
  E->ridley_var_C = ridley_var_C;
  Ridley_Func_24();
  E->ridley_var_A = FUNC16(Ridley_Func_33b);
  Ridley_Func_33b();
}

void Ridley_Func_28(void) {  // 0xA6B8A9
  Enemy_Ridley *E = Get_Ridley(0);
  if (!E->ridley_var_B)
    E->ridley_var_B = E->base.x_pos & 0x80 ? -0xc0 : 0xc0;
  if (((E->ridley_var_B ^ (E->base.x_pos - samus_x_pos)) & 0x8000) == 0) {
    uint16 x_pos;
    x_pos = E->base.x_pos;
    if ((int16)(x_pos - E->ridley_var_22) >= 0) {
      if ((int16)(x_pos - E->ridley_var_23) < 0) {
        if (random_number < 0x555)
          return;
        goto LABEL_7;
      }
      goto LABEL_15;
    }
LABEL_13:
    if ((E->ridley_var_B & 0x8000) == 0)
      return;
LABEL_7:
    E->ridley_var_B = -E->ridley_var_B;
    return;
  }
  uint16 v4;
  v4 = E->base.x_pos;
  if ((int16)(v4 - E->ridley_var_22) < 0)
    goto LABEL_13;
  if ((int16)(v4 - E->ridley_var_23) >= 0) {
LABEL_15:
    if ((E->ridley_var_B & 0x8000) != 0)
      return;
    goto LABEL_7;
  }
  if (random_number < 0x555)
    goto LABEL_7;
}

void Ridley_Func_29(void) {  // 0xA6B90F
  int v0 = random_number & 3;
  uint16 r18 = g_off_A6B965[v0], r20 = g_off_A6B96D[v0];
  Enemy_Ridley *E = Get_Ridley(0);
  uint16 v2 = 2 * (E->ridley_var_12 + 2);
  int v3 = v2 >> 1;
  tilemap_stuff[7] = g_word_A6B94D[v3];
  tilemap_stuff[6] = g_word_A6B959[v3];
  E->ridley_var_C = *(uint16 *)&RomPtr_A6(r20)[v2];
  const uint8 *v4 = RomPtr_A6(r18);
  uint16 t = *(uint16 *)&v4[v2];
  E->ridley_var_B = sign16(E->ridley_var_B) ? -t : t;
}

void Ridley_Func_30(void) {  // 0xA6B9E1
  Enemy_Ridley *E = Get_Ridley(0);
  uint16 ridley_var_14 = E->ridley_var_14, v3, v6;
  if (ridley_var_14) {
    int16 v2 = abs16(ridley_var_14) - 4;
    if (v2 >= 0)
      v3 = sign16(ridley_var_14) ? -v2 : v2;
    else
      v3 = 0;
    E->ridley_var_14 = v3;
  }
  uint16 ridley_var_15 = E->ridley_var_15;
  if (ridley_var_15) {
    int16 v5 = abs16(ridley_var_15) - 4;
    if (v5 >= 0)
      v6 = sign16(ridley_var_15) ? -v5 : v5;
    else
      v6 = 0;
    E->ridley_var_15 = v6;
  }
  samus_x_pos = E->ridley_var_14 + E->base.x_pos + g_word_A6B9D5[E->ridley_var_10];
  samus_y_pos = E->ridley_var_15 + E->base.y_pos + g_word_A6B9DB[E->ridley_var_1D >> 1];
}

void Ridley_Func_31(void) {  // 0xA6BA54
  Enemy_Ridley *E = Get_Ridley(0);
  E->ridley_var_14 = samus_x_pos - (E->base.x_pos + g_word_A6B9D5[E->ridley_var_10]);
  E->ridley_var_15 = samus_y_pos - (E->base.y_pos + g_word_A6B9DB[E->ridley_var_1D >> 1]);
}

void Ridley_Func_32(void) {  // 0xA6BA85
  Enemy_Ridley *E = Get_Ridley(0);
  if (sign16(E->ridley_var_25 - 10)) {
    if (power_bomb_flag) {
      E->ridley_var_A = FUNC16(Ridley_Func_43);
      Ridley_Func_43();
    } else {
      E->ridley_var_A = FUNC16(Ridley_Func_5);
      tilemap_stuff[2] = 1;
    }
  } else {
    Ridley_Func_126();
    E->ridley_var_A = FUNC16(Ridley_C53E);
    Ridley_C53E();
  }
}

void Ridley_Func_33(void) {  // 0xA6BAB7
  Enemy_Ridley *E = Get_Ridley(0);
  uint16 v1;
  int16 v2;
  uint16 r18, r20;

  if ((Ridley_Func_40_Carry() & 1) != 0
      && (!E->ridley_var_1F)
      && ((r18 = g_word_A6BB48[E->ridley_var_10], r20 = E->base.x_pos - samus_x_pos,
           ((r18 ^ r20) & 0x8000) == 0)
          || (v1 = abs16(r20), sign16(v1 - 32)))
      && (int16)(E->base.y_pos + 35 - samus_y_pos) < 0) {
    v2 = -16;
    if (E->ridley_var_10)
      v2 = 16;
    Ridley_Func_104_0(0, g_word_A6BB4E[E->ridley_var_12], samus_x_pos + v2, samus_y_pos - 4);
    Rect16U rect = { E->base.x_pos + g_word_A6B9D5[E->ridley_var_10], E->base.y_pos + 35, 8, 12 };
    if (Ridley_Func_124(rect)) {
      E->ridley_var_C = -E->ridley_var_C;
      if (E->base.health) {
        if (power_bomb_flag) {
          Ridley_Func_37();
          E->ridley_var_A = FUNC16(Ridley_Func_43);
          Ridley_Func_43();
        } else {
          Ridley_Func_33b();
        }
      } else {
        if (!E->ridley_var_1B)
          Ridley_Func_37();
        Ridley_Func_126();
        E->ridley_var_A = FUNC16(Ridley_Func_63);
        Ridley_Func_63();
      }
    }
  } else {
    Ridley_Func_32();
  }
}

void Ridley_Func_33b(void) {  // 0xA6BB8F
  int16 v1;

  Enemy_Ridley *E = Get_Ridley(0);
  E->ridley_var_17 = g_word_A6BBEB[E->ridley_var_10];
  v1 = E->base.y_pos - 64;
  if (sign16(E->base.y_pos - 320))
    v1 = 256;
  E->ridley_var_18 = v1;
  if (!E->ridley_var_1B)
    Ridley_Func_37();
  E->ridley_var_A = FUNC16(Ridley_Func_34);
  E->ridley_var_F = 32;
  Ridley_Func_34();
}

void Ridley_Func_34(void) {  // 0xA6BBC4
  Enemy_Ridley *E = Get_Ridley(0);
  Ridley_Func_104_0(0, 0, E->ridley_var_17, E->ridley_var_18);
  if ((--E->ridley_var_F & 0x8000) != 0) {
    E->ridley_var_A = FUNC16(Ridley_Func_35);
    E->ridley_var_F = 32;
  }
}

void Ridley_Func_35(void) {  // 0xA6BBF1
  Enemy_Ridley *E = Get_Ridley(0);
  if ((--E->ridley_var_F & 0x8000) != 0) {
    tilemap_stuff[15] = 8;
    tilemap_stuff[9] = 240;
    Ridley_Func_38();
    E->ridley_var_A = FUNC16(Ridley_Func_36);
    E->ridley_var_F = 64;
  } else {
    Ridley_Func_104_0(0, 0, E->ridley_var_17, 256);
  }
}

void Ridley_Func_36(void) {  // 0xA6BC2E
  Enemy_Ridley *E = Get_Ridley(0);
  if ((--E->ridley_var_F & 0x8000) != 0) {
    tilemap_stuff[15] = 16;
    tilemap_stuff[9] = 240;
    E->ridley_var_A = FUNC16(Ridley_Func_3b);
  } else {
    Ridley_Func_104_0(0, 0, g_word_A6BC62[E->ridley_var_10], 224);
  }
}

void Ridley_Func_37(void) {  // 0xA6BC68
  Ridley_Func_31();
  Enemy_Ridley *E = Get_Ridley(0);
  E->base.properties |= kEnemyProps_Intangible;
  E->ridley_var_1B = 1;
  CallSomeSamusCode(0);
  Ridley_Func_119(1);
}

void Ridley_Func_38(void) {  // 0xA6BC84
  tilemap_stuff[2] = 1;
  tilemap_stuff[0] = 1;
  Enemy_Ridley *E = Get_Ridley(0);
  if ((E->ridley_var_01 & 0x8000) == 0) {
    E->ridley_var_1E = Ridley_Func_40_Sign() ? 6 : 10;
  }
  E->ridley_var_1B = 0;
  CallSomeSamusCode(1);
  Ridley_Func_119(0);
}

void Ridley_Func_39(void) {  // 0xA6BCB4
  int16 ridley_var_01;

  Enemy_Ridley *E = Get_Ridley(0);
  ridley_var_01 = E->ridley_var_01;
  if (ridley_var_01 >= 0) {
    if (ridley_var_01) {
      if (Ridley_Func_121() & 1) {
        E->base.properties |= kEnemyProps_Intangible;
        return;
      }
      E->base.properties &= ~kEnemyProps_Intangible;
    }
    uint16 ridley_var_1E = E->ridley_var_1E;
    if (ridley_var_1E) {
      uint16 v3 = ridley_var_1E - 1;
      E->ridley_var_1E = v3;
      if (!v3 && (E->ridley_var_01 & 0x8000) == 0)
        E->base.properties &= ~kEnemyProps_Intangible;
    }
  }
}

static const uint8 byte_A6BD04[28] = {
  0x80, 0x80, 0x80,    0, 0xff, 0x80, 0x80, 0xff, 0xff, 0xff, 0x80,    0, 0, 0x80, 0x80, 0x80,
  0x80, 0xff, 0xff, 0xff, 0x80, 0x80,    0, 0x80, 0x80,    0,    0, 0x80,
};

static uint8 Ridley_Func_40_Carry(void) {
  return (byte_A6BD04[samus_movement_type] & 0x80) != 0;
}

static uint8 Ridley_Func_40_Sign(void) {
  return (byte_A6BD04[samus_movement_type] & 0x40) != 0;
}

uint8 Ridley_Func_41(void) {  // 0xA6BD20
  return sign16(samus_y_pos - 352) == 0;
}

void Ridley_Func_42(void) {  // 0xA6BD2C
  int16 ridley_var_01;

  Enemy_Ridley *E = Get_Ridley(0);
  ridley_var_01 = E->ridley_var_01;
  if (ridley_var_01 > 0 && ridley_var_01 != 2 && power_bomb_flag && !E->ridley_var_1B) {
    Ridley_Func_24();
    E->ridley_var_A = FUNC16(Ridley_Func_33);
  }
}

void Ridley_Func_43(void) {  // 0xA6BD4E
  if (power_bomb_flag) {
    Get_Ridley(0)->ridley_var_01 = 2;
    uint16 r18 = sign16(power_bomb_explosion_x_pos - 128) ? 192 : 80;
    uint16 r20 = sign16(power_bomb_explosion_y_pos - 256) ? 384 : 192;
    Ridley_Func_104_0(0, Ridley_Func_7(), r18, r20);
  } else {
    Enemy_Ridley *E = Get_Ridley(0);
    E->ridley_var_01 = 1;
    E->ridley_var_A = E->ridley_var_1B ? FUNC16(Ridley_Func_33b) : FUNC16(Ridley_Func_3b);
  }
}

void Ridley_Func_44(void) {  // 0xA6BD9A
  Ridley_Func_106(0, 1, 192, 128);
  Enemy_Ridley *E = Get_Ridley(0);
  if (!sign16(E->base.x_pos - 192))
    E->ridley_var_A = FUNC16(Ridley_Func_45);
}

void Ridley_Func_45(void) {  // 0xA6BDBC
  Enemy_Ridley *E = Get_Ridley(0);
  E->ridley_var_20 = -192;
  Ridley_Func_106(0, 1, 192, -128);
  if (sign16(E->base.y_pos - 32)) {
    E->ridley_var_40 = FUNC16(Ridley_Func_52);
    E->ridley_var_A = FUNC16(Ridley_Func_46);
    E->ridley_var_F = 21;
  }
}

void Ridley_Func_46(void) {  // 0xA6BDF2
  Enemy_Ridley *E = Get_Ridley(0);
  if ((--E->ridley_var_F & 0x8000) != 0) {
    Ridley_Func_99(addr_kRidley_Ilist_E658);
    E->ridley_var_A = FUNC16(Ridley_Func_47);
    Ridley_Func_47();
  }
}

void Ridley_Func_47(void) {  // 0xA6BE03
  Enemy_Ridley *E = Get_Ridley(0);
  Ridley_Func_104_0(0, 12, E->ridley_var_42 - 10, E->ridley_var_44 - 56);
  Rect16U rect = { E->base.x_pos + 14, E->base.y_pos + 66, 4, 4 };
  if (Ridley_Func_48(rect) & 1) {
    E->ridley_var_1B = 1;
    Ridley_Func_119(1);
    E->ridley_var_C = -512;
    E->ridley_var_40 = FUNC16(Ridley_Func_51);
    E->ridley_var_A = FUNC16(CeresRidley_Func_22);
  }
}

uint8 Ridley_Func_48(Rect16U rect) {  // 0xA6BE61
  Enemy_Ridley *E = Get_Ridley(0);
  uint16 v1 = abs16(E->ridley_var_42 - rect.x);
  bool v2 = v1 < 4;
  uint16 v3 = v1 - 4;
  uint8 result = 0;
  if (v2 || v3 < rect.w) {
    uint16 v4 = abs16(E->ridley_var_44 - rect.y);
    v2 = v4 < 4;
    uint16 v5 = v4 - 4;
    if (v2 || v5 < rect.h)
      return 1;
  }
  return result;
}

void CallRidleyFunc_var40(uint32 ea) {
  switch (ea) {
  case fnRidley_Func_50: Ridley_Func_50(); return;
  case fnRidley_Func_51: Ridley_Func_51(); return;  // 0xa6beb3
  case fnRidley_Func_52: Ridley_Func_52(); return;  // 0xa6beca
  case fnRidley_Func_53: Ridley_Func_53(); return;  // 0xa6bedc
  case fnnullsub_348: return;  // 0xa6bf19
  default: Unreachable();
  }
}

void Ridley_Func_49(void) {  // 0xA6BE93
  CallRidleyFunc_var40(Get_Ridley(0)->ridley_var_40 | 0xA60000);
}

void Ridley_Func_50(void) {  // 0xA6BE9C
  Enemy_Ridley *E = Get_Ridley(0);
  E->ridley_var_42 = E->base.x_pos - 16;
  E->ridley_var_44 = E->base.y_pos + 22;
}

void Ridley_Func_51(void) {  // 0xA6BEB3
  Enemy_Ridley *E = Get_Ridley(0);
  E->ridley_var_42 = E->base.x_pos + 14;
  E->ridley_var_44 = E->base.y_pos + 66;
}

void Ridley_Func_52(void) {  // 0xA6BECA
  Enemy_Ridley *E = Get_Ridley(0);
  E->ridley_var_45 = 0;
  E->ridley_var_46 = 0;
  E->ridley_var_40 = FUNC16(Ridley_Func_53);
  Ridley_Func_53();
}

void Ridley_Func_53(void) {  // 0xA6BEDC
  Enemy_Ridley *E = Get_Ridley(0);
  uint16 v1 = E->ridley_var_46 + 8;
  E->ridley_var_46 = v1;
  int t = HIBYTE(E->ridley_var_43) + (v1&0xff);
  HIBYTE(E->ridley_var_43) = t;
  E->ridley_var_44 += (int8)(v1 >> 8) + (t >> 8);
  if (!sign16(E->ridley_var_44 - 192)) {
    E->ridley_var_44 = 192;
    E->ridley_var_40 = addr_locret_A6BF19;
  }
}

void Ridley_Func_54(void) {  // 0xA6C04E
  uint16 ridley_var_F = Get_Ridley(0)->ridley_var_F;
  if (ridley_var_F) {
    switch (ridley_var_F) {
    case 2:
      Ridley_C08E();
      break;
    case 4:
      Ridley_C09F();
      break;
    case 6:
      Ridley_Func_56();
      break;
    case 8:
      Ridley_Func_57();
      break;
    case 0xA:
      Ridley_C104();
      break;
    case 0xC:
      Ridley_Func_58();
      break;
    default:
      Unreachable();
      while (1)
        ;
    }
  } else {
    Ridley_Func_55();
  }
}

void Ridley_Func_55(void) {  // 0xA6C062
  palette_buffer[97] = palette_buffer[1];
  palette_buffer[99] = palette_buffer[3];
  palette_buffer[81] = palette_buffer[17];
  palette_buffer[83] = palette_buffer[19];
  Enemy_Ridley *E = Get_Ridley(0);
  E->ridley_var_E = -15157;
  ++E->ridley_var_F;
  ++E->ridley_var_F;
  Ridley_C08E();
}

void Ridley_C08E(void) {  // 0xA6C08E
  if (ProcessEscapeTimerTileTransfers() & 1) {
    Enemy_Ridley *E = Get_Ridley(0);
    E->ridley_var_E = -15106;
    ++E->ridley_var_F;
    ++E->ridley_var_F;
    Ridley_C09F();
  }
}

void Ridley_C09F(void) {  // 0xA6C09F
  if (ProcessEscapeTimerTileTransfers() & 1) {
    Enemy_Ridley *E = Get_Ridley(0);
    ++E->ridley_var_F;
    ++E->ridley_var_F;
    Ridley_Func_59();
    E->ridley_var_E = 128;
    QueueMusic_Delayed8(7);
  }
}

void Ridley_Func_56(void) {  // 0xA6C0BB
  Ridley_Func_61();
  Enemy_Ridley *E = Get_Ridley(0);
  if (E->ridley_var_E-- == 1) {
    E->ridley_var_3B = -15280;
    E->ridley_var_3C = 0;
    E->ridley_var_3D = 0;
    E->ridley_var_3E = 0;
    E->ridley_var_3F = 0;
    E->ridley_var_E = 32;
    if (!japanese_text_flag) {
      ++E->ridley_var_F;
      ++E->ridley_var_F;
    }
    ++E->ridley_var_F;
    ++E->ridley_var_F;
  }
}

void Ridley_Func_57(void) {  // 0xA6C0F5
  Enemy_Ridley *E = Get_Ridley(0);
  if (E->ridley_var_E-- == 1) {
    ++E->ridley_var_F;
    ++E->ridley_var_F;
    Ridley_Func_62();
  }
  Ridley_C104();
}

void Ridley_C104(void) {  // 0xA6C104
  Ridley_Func_61();
  if (HandleTypewriterText_Ext(0x3582) & 1) {
    Enemy_Ridley *E = Get_Ridley(0);
    ++E->ridley_var_F;
    ++E->ridley_var_F;
  }
}

void Ridley_Func_58(void) {  // 0xA6C117
  Ridley_Func_61();
  CeresRidley_Func_2();
  Enemy_Ridley *E = Get_Ridley(0);
  E->ridley_var_F = 0;
  E->ridley_var_A = FUNC16(CeresRidley_Func_26);
  timer_status = 1;
  ceres_status = 2;
  SetBossBitForCurArea(1);
}

void Ridley_Func_59(void) {  // 0xA6C136
  VramWriteEntry *v2;

  uint16 v0 = vram_write_queue_tail;
  const uint8 *v1 = RomPtr_A6(addr_byte_A6C15D);
  v2 = gVramWriteEntry(vram_write_queue_tail);
  v2->size = GET_WORD(v1);
  *(VoidP *)((uint8 *)&v2->src.addr + 1) = GET_WORD(v1 + 3);
  v2->src.addr = GET_WORD(v1 + 2);
  v2->vram_dst = GET_WORD(v1 + 5);
  vram_write_queue_tail = v0 + 7;
}

void Ridley_Func_60(void) {  // 0xA6C176
  int16 v1;

  Enemy_Ridley *E = Get_Ridley(0);
  v1 = E->ridley_var_39 + 1;
  if (!sign16(E->ridley_var_39 - 31))
    v1 = -31;
  E->ridley_var_39 = v1;
  palette_buffer[11] = abs16(E->ridley_var_39) & 0x1F;
}

void Ridley_Func_61(void) {  // 0xA6C19C
  if (!(door_transition_flag_enemies | palette_change_num) && (nmi_frame_counter_word & 3) == 0) {
    Enemy_Ridley *E = Get_Ridley(0);
    uint16 v1 = E->ridley_var_38 + 1;
    if (v1 >= 0x10)
      v1 = 0;
    E->ridley_var_38 = v1;
    int v2 = E->ridley_var_38 + 2 * v1;
    palette_buffer[97] = g_word_A6C1DF[v2];
    palette_buffer[98] = g_word_A6C1DF[v2 + 1];
    palette_buffer[99] = g_word_A6C1DF[v2 + 2];
  }
}

void Ridley_Func_62(void) {  // 0xA6C383
  int i;
  VramWriteEntry *v3;

  uint16 v0 = addr_stru_A6C3B8;
  for (i = vram_write_queue_tail; ; i += 7) {
    const uint8 *v2 = RomPtr_A6(v0);
    if (!GET_WORD(v2))
      break;
    v3 = gVramWriteEntry(i);
    v3->size = GET_WORD(v2);
    *(VoidP *)((uint8 *)&v3->src.addr + 1) = GET_WORD(v2 + 3);
    v3->src.addr = GET_WORD(v2 + 2);
    v3->vram_dst = GET_WORD(v2 + 5);
    v0 += 7;
  }
  vram_write_queue_tail = i;
}

void Ridley_Func_63(void) {  // 0xA6C538
  if (!(Ridley_Func_68() & 1))
    Ridley_C53E();
}

void Ridley_C53E(void) {  // 0xA6C53E
  Ridley_Func_99(addr_kRidley_Ilist_E6C8);
  Enemy_Ridley *E = Get_Ridley(0);
  E->ridley_var_A = FUNC16(Ridley_C551);
  E->ridley_var_F = 32;
}

void Ridley_C551(void) {  // 0xA6C551
  Ridley_Func_68();
  Enemy_Ridley *E = Get_Ridley(0);
  if ((--E->ridley_var_F & 0x8000) != 0) {
    E->ridley_var_B = 0;
    E->ridley_var_C = 0;
    fx_target_y_pos = 528;
    fx_y_vel = 64;
    fx_timer = 1;
    E->ridley_var_27 = 0;
    E->ridley_var_28 = 0;
    E->ridley_var_A = FUNC16(Ridley_Func_64);
    E->ridley_var_F = 160;
    Ridley_Func_64();
  }
}

void Ridley_Func_64(void) {  // 0xA6C588
  Ridley_Func_69();
  Enemy_Ridley *E = Get_Ridley(0);
  if ((--E->ridley_var_F & 0x8000) != 0) {
    if (E->ridley_var_1B)
      Ridley_Func_38();
    E->ridley_var_A = FUNC16(Ridley_Func_65);
    E->ridley_var_B = 0;
    E->ridley_var_C = 0;
    RidleysExplosion_Func_2();
  }
}

void Ridley_Func_65(void) {  // 0xA6C5A8
  Ridley_Func_69();
  Enemy_Ridley *E = Get_Ridley(0);
  E->ridley_var_02 = 0;
  E->base.properties |= kEnemyProps_Invisible;
  E->ridley_var_A = FUNC16(Ridley_Func_66);
  E->ridley_var_F = 32;
}

void Ridley_Func_66(void) {  // 0xA6C5C8
  Enemy_Ridley *E = Get_Ridley(0);
  if ((--E->ridley_var_F & 0x8000) != 0) {
    E->ridley_var_A = FUNC16(Ridley_Func_67);
    E->ridley_var_F = 256;
  }
}

void Ridley_Func_67(void) {  // 0xA6C5DA
  Enemy_Ridley *E = Get_Ridley(0);
  if ((--E->ridley_var_F & 0x8000) != 0) {
    uint16 k = 0;
    printf("Warning: X undefined\n");
    SetBossBitForCurArea(1);
    Enemy_ItemDrop_Ridley(k);
    QueueMusic_Delayed8(3);
    E->base.properties |= kEnemyProps_Deleted;
    E->ridley_var_A = addr_locret_A6C600;
  }
}

uint8 Ridley_Func_68(void) {  // 0xA6C601
  Rect16U rect = { 128, 328, 4, 4 };
  Ridley_Func_104(0, 0, 0x10, rect.x, rect.y);
  return Shitroid_Func_2(0, rect);
}

void Ridley_Func_69(void) {  // 0xA6C623
  int16 v1;

  Enemy_Ridley *E = Get_Ridley(0);
  v1 = E->ridley_var_27 - 1;
  if (v1 < 0) {
    E->ridley_var_27 = 4;
    uint16 v2 = E->ridley_var_28 + 1;
    if (!sign16(E->ridley_var_28 - 9))
      v2 = 0;
    E->ridley_var_28 = v2;
    int v3 = (uint16)(4 * v2) >> 1;
    eproj_spawn_pt = (Point16U){ E->base.x_pos + g_word_A6C66E[v3], E->base.y_pos + g_word_A6C66E[v3 + 1] };
    SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 3);
    QueueSfx2_Max3(0x24);
  } else {
    E->ridley_var_27 = v1;
  }
}

void RidleysExplosion_Init(void) {  // 0xA6C696
  Enemy_Ridley *E = Get_Ridley(cur_enemy_index);
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->base.vram_tiles_index = 0;
  E->base.palette_index = 3584;
  uint16 ridley_parameter_1 = E->ridley_parameter_1;
  E->ridley_var_F = g_word_A6C6CE[ridley_parameter_1 >> 1];
  uint16 Random = NextRandom();
  E->ridley_var_B = sign16(Random) ? -(Random & 0x130) : (Random & 0x130);
  E->ridley_var_C = 0;
  if (ridley_parameter_1) {
    switch (ridley_parameter_1) {
    case 2:
      E->base.x_pos = tilemap_stuff[32];
      E->base.y_pos = tilemap_stuff[33];
      E->base.current_instruction = addr_kRidleysExplosion_Ilist_CA47;
      break;
    case 4:
      E->base.x_pos = tilemap_stuff[42];
      E->base.y_pos = tilemap_stuff[43];
      E->base.current_instruction = addr_kRidleysExplosion_Ilist_CA4D;
      break;
    case 6:
      E->base.x_pos = tilemap_stuff[52];
      E->base.y_pos = tilemap_stuff[53];
      E->base.current_instruction = addr_kRidleysExplosion_Ilist_CA4D;
      break;
    case 8:
      E->base.x_pos = tilemap_stuff[62];
      E->base.y_pos = tilemap_stuff[63];
      E->base.current_instruction = addr_kRidleysExplosion_Ilist_CA53;
      break;
    case 0xA:
      E->base.x_pos = tilemap_stuff[72];
      E->base.y_pos = tilemap_stuff[73];
      E->base.current_instruction = addr_kRidleysExplosion_Ilist_CA53;
      break;
    case 0xC:
      E->base.x_pos = tilemap_stuff[82];
      E->base.y_pos = tilemap_stuff[83];
      E->base.current_instruction = g_off_A6C7BA[(uint8)((LOBYTE(tilemap_stuff[71]) + LOBYTE(tilemap_stuff[81]) + 8) & 0xF0) >> 4];
      break;
    case 0xE: {
      uint16 v10 = 0;
      Enemy_Ridley *E0 = Get_Ridley(0);
      if (E0->ridley_var_10)
        v10 = 2;
      int v13 = v10 >> 1;
      uint16 v14 = E0->base.x_pos + g_word_A6C804[v13];
      E->base.x_pos = v14;
      E->base.y_pos = E0->base.y_pos;
      E->base.current_instruction = g_off_A6C808[v13];
      break;
    }
    case 0x10: {
      uint16 v16 = 0;
      Enemy_Ridley *E0 = Get_Ridley(0);
      if (E0->ridley_var_10)
        v16 = 2;
      int v19 = v16 >> 1;
      uint16 v20 = E0->base.x_pos + g_word_A6C836[v19];
      E->base.x_pos = v20;
      E->base.y_pos = E0->base.y_pos + 22;
      E->base.current_instruction = g_off_A6C83A[v19];
      break;
    }
    case 0x12: {
      uint16 v22 = 0;
      Enemy_Ridley *E0 = Get_Ridley(0);
      if (E0->ridley_var_10)
        v22 = 2;
      int v25 = v22 >> 1;
      uint16 v26 = E0->base.x_pos + g_word_A6C868[v25];
      E->base.x_pos = v26;
      E->base.y_pos = E0->base.y_pos - 24;
      E->base.current_instruction = g_off_A6C86C[v25];
      break;
    }
    case 0x14: {
      uint16 v28 = 0;
      Enemy_Ridley *E0 = Get_Ridley(0);
      if (E0->ridley_var_10)
        v28 = 2;
      int v31 = v28 >> 1;
      uint16 v32 = E0->base.x_pos + g_word_A6C89A[v31];
      E->base.x_pos = v32;
      E->base.y_pos = E0->base.y_pos;
      E->base.current_instruction = g_off_A6C89E[v31];
      break;
    }
    case 0x16: {
      uint16 v34 = 0;
      Enemy_Ridley *E0 = Get_Ridley(0);
      if (E0->ridley_var_10)
        v34 = 2;
      int v37 = v34 >> 1;
      uint16 v38 = E0->base.x_pos + g_word_A6C8CC[v37];
      E->base.x_pos = v38;
      E->base.y_pos = E0->base.y_pos + 7;
      E->base.current_instruction = g_off_A6C8D0[v37];
      break;
    }
    default:
      Unreachable();
      while (1)
        ;
    }
  } else {
    E->base.x_pos = tilemap_stuff[22];
    E->base.y_pos = tilemap_stuff[23];
    E->base.current_instruction = addr_kRidleysExplosion_Ilist_CA47;
  }
}

void RidleysExplosion_Main(void) {  // 0xA6C8D4
  int16 v2;

  RidleysExplosion_Func_1(cur_enemy_index);
  Enemy_RidleysExplosion *E = Get_RidleysExplosion(cur_enemy_index);
  v2 = abs16(E->ren_var_B) - 4;
  if (v2 < 0)
    v2 = 0;
  E->ren_var_B = sign16(E->ren_var_B) ? -v2 : v2;
  E->ren_var_C += 4;
  MoveEnemyWithVelocity();
  if ((--E->ren_var_F & 0x8000) != 0)
    EnemyDeathAnimation(cur_enemy_index, 0);
}

void RidleysExplosion_Func_1(uint16 k) {  // 0xA6C913
  Enemy_RidleysExplosion *E = Get_RidleysExplosion(k);
  uint16 v2 = E->base.frame_counter & 1, v3;
  E->base.frame_counter = v2;
  if (v2)
    v3 = E->base.properties | kEnemyProps_Invisible;
  else
    v3 = E->base.properties & ~kEnemyProps_Invisible;
  E->base.properties = v3;
}

void RidleysExplosion_Func_2(void) {  // 0xA6C932
  SpawnEnemy(0xA6, addr_stru_A6C9E7);
  SpawnEnemy(0xA6, addr_stru_A6C9D7);
  SpawnEnemy(0xA6, addr_stru_A6C9C7);
  SpawnEnemy(0xA6, addr_stru_A6C9B7);
  SpawnEnemy(0xA6, addr_stru_A6C9A7);
  SpawnEnemy(0xA6, addr_stru_A6C997);
  SpawnEnemy(0xA6, addr_stru_A6C987);
  SpawnEnemy(0xA6, addr_stru_A6C9F7);
  SpawnEnemy(0xA6, addr_stru_A6CA07);
  SpawnEnemy(0xA6, addr_stru_A6CA27);
  SpawnEnemy(0xA6, addr_stru_A6CA17);
  SpawnEnemy(0xA6, addr_stru_A6CA37);
}

void locret_A6CB20(void) {}

static Func_V *const off_A6CB21[9] = { locret_A6CB20, Ridley_CBC0, Ridley_Func_71, Ridley_Func_72, Ridley_CBC7, Ridley_CBCE, Ridley_Func_73, Ridley_Func_74, Ridley_CBD5 };

void Ridley_Func_70(void) {  // 0xA6CAF5
  Ridley_Func_91();
  off_A6CB21[tilemap_stuff[0]]();
  Ridley_Func_86();
  Ridley_Func_87();
  Enemy_Ridley *E = Get_Ridley(0);
  if (!(E->ridley_var_1B | (uint16)(samus_invincibility_timer | E->base.properties & kEnemyProps_Intangible)))
    Ridley_Func_127();
}

void Ridley_Func_71(void) {  // 0xA6CB33
  tilemap_stuff[10] = 8;
  Ridley_Func_75();
  tilemap_stuff[0] = 3;
}

void Ridley_Func_72(void) {  // 0xA6CB45
  tilemap_stuff[10] = 8;
  Ridley_Func_75();
}

void Ridley_Func_73(void) {  // 0xA6CB4E
  tilemap_stuff[10] = 3;
  Ridley_Func_75();
  tilemap_stuff[0] = 6;
}

void Ridley_Func_74(void) {  // 0xA6CB60
  tilemap_stuff[10] = 2;
  Ridley_Func_75();
  tilemap_stuff[0] = 7;
}

void Ridley_Func_75(void) {  // 0xA6CB72
  if (Get_Ridley(0)->ridley_var_10) {
    tilemap_stuff[13] = 0x4000;
    tilemap_stuff[14] = -1;
  } else {
    tilemap_stuff[14] = 0x4000;
    tilemap_stuff[13] = -1;
  }
  Ridley_CBC7();
  if (!(tilemap_stuff[76] | (uint16)(tilemap_stuff[66] | tilemap_stuff[56] | tilemap_stuff[46] | tilemap_stuff[36] | tilemap_stuff[26] | tilemap_stuff[16])))
    tilemap_stuff[0] = 4;
}

void Ridley_CBC0(void) {  // 0xA6CBC0
  Ridley_Func_79();
  Ridley_CBDC();
}

void Ridley_CBC7(void) {  // 0xA6CBC7
  Ridley_Func_83();
  Ridley_CBDC();
}

void Ridley_CBCE(void) {  // 0xA6CBCE
  Ridley_Func_84();
  Ridley_CBDC();
}

void Ridley_CBD5(void) {  // 0xA6CBD5
  Ridley_Func_81();
  Ridley_CBDC();
}

void Ridley_CBDC(void) {  // 0xA6CBDC
  uint16 v0 = 7;
  uint16 v1 = 0;
  do {
    Ridley_Func_88(v1);
    v1 += 20;
  } while (v0-- != 1);
}

void Ridley_Func_77(void) {  // 0xA6CBFE
  int v0 = Get_Ridley(0)->ridley_var_10;
  tilemap_stuff[11] = g_word_A6CC12[v0];
  tilemap_stuff[12] = g_word_A6CC18[v0];
}

uint8 Ridley_Func_78(void) {  // 0xA6CC1E
  return (tilemap_stuff[76] & (uint16)(tilemap_stuff[66] & tilemap_stuff[56] & tilemap_stuff[46] & tilemap_stuff[36] & tilemap_stuff[26] & tilemap_stuff[16])) != 0;
}

void Ridley_Func_79(void) {  // 0xA6CC39
  Ridley_Func_77();
  if (Ridley_Func_78()) {
    if (Ridley_Func_80_DoubleRet())
      return;
  }
  if (!(tilemap_stuff[76] | (uint16)(tilemap_stuff[66] | tilemap_stuff[56] | tilemap_stuff[46] | tilemap_stuff[36] | tilemap_stuff[26] | tilemap_stuff[16]))) {
    tilemap_stuff[16] = 0x8000;
    tilemap_stuff[13] = -1;
    tilemap_stuff[14] = -1;
    uint16 v0 = 2;
    if (samus_x_pos < 0x70)
      v0 = 1;
    tilemap_stuff[10] = v0;
  }
}

uint8 Ridley_Func_80_DoubleRet(void) {  // 0xA6CC7D
  Enemy_Ridley *E = Get_Ridley(0);
  bool retval = false;
  uint16 v1;
  if (tilemap_stuff[2] && (tilemap_stuff[13] & tilemap_stuff[14] & 0x8000) != 0)
    goto LABEL_7;
  if (!tilemap_stuff[1])
    goto LABEL_9;
  if ((uint8)random_number >= 0xF0) {
LABEL_7:
    v1 = tilemap_stuff[2] - 1;
    goto LABEL_8;
  }
  if (abs16(samus_x_pos - E->base.x_pos) < 0x80) {
    v1 = 0;
LABEL_8:
    Ridley_Func_89(v1);
    retval = true;
  }
LABEL_9:
  tilemap_stuff[2] = 0;
  return retval;
}

bool Ridley_Func_82_DoubleRet();

void Ridley_Func_81(void) {  // 0xA6CCBD
  Ridley_Func_77();
  if (Ridley_Func_78() & 1) {
    if (Ridley_Func_82_DoubleRet())
      return;
  }
  if (!(tilemap_stuff[76] | (uint16)(tilemap_stuff[66] | tilemap_stuff[56] | tilemap_stuff[46] | tilemap_stuff[36] | tilemap_stuff[26] | tilemap_stuff[16]))) {
    tilemap_stuff[16] = 0x8000;
    tilemap_stuff[13] = -1;
    tilemap_stuff[14] = -1;
    uint16 v0 = 2;
    if (samus_x_pos < 0x70)
      v0 = 1;
    tilemap_stuff[10] = v0;
  }
}

bool Ridley_Func_82_DoubleRet(void) {
  if (tilemap_stuff[2]) {
    if ((tilemap_stuff[13] & tilemap_stuff[14] & 0x8000) != 0) {
      Ridley_Func_89(1);
      tilemap_stuff[2] = 0;
      return true;
    }
  }
  return false;
}

void Ridley_Func_83(void) {  // 0xA6CD24
  Enemy_Ridley *E = Get_Ridley(0);
  Ridley_Func_77();
  if (Ridley_Func_78() & 1
      && ((uint8)random_number >= 0xF0
          || (abs16(samus_x_pos - E->base.x_pos) < 0x80))
      && (tilemap_stuff[13] & tilemap_stuff[14] & 0x8000) != 0) {
    tilemap_stuff[13] = 16128;
    tilemap_stuff[10] = 8;
  } else if (!(tilemap_stuff[76] | (uint16)(tilemap_stuff[66] | tilemap_stuff[56] | tilemap_stuff[46] | tilemap_stuff[36] | tilemap_stuff[26] | tilemap_stuff[16]))) {
    if ((Get_Ridley(0)->ridley_var_C & 0x8000) == 0)
      tilemap_stuff[0] = 5;
    tilemap_stuff[13] = -1;
    tilemap_stuff[14] = -1;
    uint16 v1 = tilemap_stuff[8];
    if (!tilemap_stuff[8] || (v1 = tilemap_stuff[8] - 1, (tilemap_stuff[8] = v1) != 0)) {
      tilemap_stuff[10] = v1;
    } else {
      tilemap_stuff[16] = 0x8000;
      Ridley_Func_85();
      tilemap_stuff[10] = 8;
    }
  }
}

void Ridley_Func_84(void) {  // 0xA6CDAA
  Enemy_Ridley *E = Get_Ridley(0);
  Ridley_Func_77();
  if (Ridley_Func_78() & 1
      && ((uint8)random_number >= 0xF0
          || (abs16(samus_x_pos - E->base.x_pos) < 0x80))
      && (tilemap_stuff[13] & tilemap_stuff[14] & 0x8000) != 0) {
    tilemap_stuff[13] = 16128;
    tilemap_stuff[10] = 8;
  } else if (!(tilemap_stuff[76] | (uint16)(tilemap_stuff[66] | tilemap_stuff[56] | tilemap_stuff[46] | tilemap_stuff[36] | tilemap_stuff[26] | tilemap_stuff[16]))) {
    if ((E->ridley_var_C & 0x8000) == 0) {
      tilemap_stuff[0] = 6;
      tilemap_stuff[20] = 2560;
      tilemap_stuff[30] = 2560;
      tilemap_stuff[40] = 2560;
      tilemap_stuff[50] = 2560;
      tilemap_stuff[60] = 2560;
      tilemap_stuff[70] = 2560;
      tilemap_stuff[80] = 2560;
      tilemap_stuff[21] = 0x4000;
      tilemap_stuff[31] = 0x4000;
      tilemap_stuff[41] = 0x4000;
      tilemap_stuff[51] = 0x4000;
      tilemap_stuff[61] = 0x4000;
      tilemap_stuff[71] = 0x4000;
      tilemap_stuff[81] = 0x4000;
      tilemap_stuff[16] = 0x8000;
    }
    tilemap_stuff[13] = -1;
    tilemap_stuff[14] = -1;
    uint16 v1 = tilemap_stuff[8];
    if (tilemap_stuff[8]) {
      v1 = tilemap_stuff[8] - 1;
      tilemap_stuff[8] = v1;
      if (!v1) {
        tilemap_stuff[16] = 0x8000;
        Ridley_Func_85();
        v1 = 8;
      }
    }
    tilemap_stuff[10] = v1;
  }
}

void Ridley_Func_85(void) {  // 0xA6CE65
  tilemap_stuff[21] = LOBYTE(tilemap_stuff[21]) | 0x4000;
  tilemap_stuff[31] = LOBYTE(tilemap_stuff[31]) | 0x4000;
  tilemap_stuff[41] = LOBYTE(tilemap_stuff[41]) | 0x4000;
  tilemap_stuff[51] = LOBYTE(tilemap_stuff[51]) | 0x4000;
  tilemap_stuff[61] = tilemap_stuff[21] & 0x4FF | 0x4000;
  tilemap_stuff[71] = LOBYTE(tilemap_stuff[71]) | 0x4000;
  tilemap_stuff[81] = LOBYTE(tilemap_stuff[81]) | 0x4000;
}

void Ridley_Func_86(void) {  // 0xA6CEBA
  int16 v1;

  Enemy_Ridley *E = Get_Ridley(0);
  tilemap_stuff[23] = tilemap_stuff[25] + E->base.y_pos + 16;
  tilemap_stuff[33] = tilemap_stuff[35] + tilemap_stuff[23];
  tilemap_stuff[43] = tilemap_stuff[45] + tilemap_stuff[35] + tilemap_stuff[23];
  tilemap_stuff[53] = tilemap_stuff[55] + tilemap_stuff[43];
  tilemap_stuff[63] = tilemap_stuff[65] + tilemap_stuff[55] + tilemap_stuff[43];
  tilemap_stuff[73] = tilemap_stuff[75] + tilemap_stuff[63];
  tilemap_stuff[83] = tilemap_stuff[85] + tilemap_stuff[75] + tilemap_stuff[63];
  v1 = E->ridley_var_10 - 1;
  if (E->ridley_var_10 != 1)
    v1 = tilemap_stuff[24] + g_word_A6CF54[E->ridley_var_10];
  tilemap_stuff[22] = E->base.x_pos + v1;
  if (E->ridley_var_10 == 1) {
    tilemap_stuff[32] = E->base.x_pos;
    tilemap_stuff[42] = tilemap_stuff[32];
    tilemap_stuff[52] = tilemap_stuff[32];
    tilemap_stuff[62] = tilemap_stuff[32];
    tilemap_stuff[72] = tilemap_stuff[32];
    tilemap_stuff[82] = tilemap_stuff[32];
  } else {
    tilemap_stuff[32] = tilemap_stuff[34] + tilemap_stuff[22];
    tilemap_stuff[42] = tilemap_stuff[44] + tilemap_stuff[34] + tilemap_stuff[22];
    tilemap_stuff[52] = tilemap_stuff[54] + tilemap_stuff[42];
    tilemap_stuff[62] = tilemap_stuff[64] + tilemap_stuff[54] + tilemap_stuff[42];
    tilemap_stuff[72] = tilemap_stuff[74] + tilemap_stuff[62];
    tilemap_stuff[82] = tilemap_stuff[84] + tilemap_stuff[74] + tilemap_stuff[62];
  }
}

void Ridley_Func_87(void) {  // 0xA6CF5A
  if (tilemap_stuff[30]) {
    if ((int16)(tilemap_stuff[30] - tilemap_stuff[29]) < 0)
      tilemap_stuff[30] = 0;
    uint16 v0 = tilemap_stuff[9] + tilemap_stuff[29];
    if (!sign16(tilemap_stuff[9] + tilemap_stuff[29] - 6144))
      v0 = 6144;
    tilemap_stuff[29] = v0;
  } else if ((int16)(2048 - tilemap_stuff[29]) < 0) {
    tilemap_stuff[29] -= 128;
  }
  if (tilemap_stuff[40]) {
    if ((int16)(tilemap_stuff[40] - tilemap_stuff[39]) < 0)
      tilemap_stuff[40] = 0;
    uint16 v1 = tilemap_stuff[9] + tilemap_stuff[39];
    if (!sign16(tilemap_stuff[9] + tilemap_stuff[39] - 6144))
      v1 = 6144;
    tilemap_stuff[39] = v1;
  } else if ((int16)(2048 - tilemap_stuff[39]) < 0) {
    tilemap_stuff[39] -= 128;
  }
  if (tilemap_stuff[50]) {
    if ((int16)(tilemap_stuff[50] - tilemap_stuff[49]) < 0)
      tilemap_stuff[50] = 0;
    uint16 v2 = tilemap_stuff[9] + tilemap_stuff[49];
    if (!sign16(tilemap_stuff[9] + tilemap_stuff[49] - 5632))
      v2 = 5632;
    tilemap_stuff[49] = v2;
  } else if ((int16)(2048 - tilemap_stuff[49]) < 0) {
    tilemap_stuff[49] -= 128;
  }
  if (tilemap_stuff[60]) {
    if ((int16)(tilemap_stuff[60] - tilemap_stuff[59]) < 0)
      tilemap_stuff[60] = 0;
    uint16 v3 = tilemap_stuff[9] + tilemap_stuff[59];
    if (!sign16(tilemap_stuff[9] + tilemap_stuff[59] - 5632))
      v3 = 5632;
    tilemap_stuff[59] = v3;
  } else if ((int16)(2048 - tilemap_stuff[59]) < 0) {
    tilemap_stuff[59] -= 128;
  }
  if (tilemap_stuff[70]) {
    if ((int16)(tilemap_stuff[70] - tilemap_stuff[69]) < 0)
      tilemap_stuff[70] = 0;
    uint16 v4 = tilemap_stuff[9] + tilemap_stuff[69];
    if (!sign16(tilemap_stuff[9] + tilemap_stuff[69] - 4608))
      v4 = 4608;
    tilemap_stuff[69] = v4;
  } else if ((int16)(2048 - tilemap_stuff[69]) < 0) {
    tilemap_stuff[69] -= 128;
  }
  if (tilemap_stuff[80]) {
    if ((int16)(tilemap_stuff[80] - tilemap_stuff[79]) < 0)
      tilemap_stuff[80] = 0;
    uint16 v5 = tilemap_stuff[9] + tilemap_stuff[79];
    if (!sign16(tilemap_stuff[9] + tilemap_stuff[79] - 1280))
      v5 = 1280;
    tilemap_stuff[79] = v5;
  } else if ((int16)(1280 - tilemap_stuff[79]) < 0) {
    tilemap_stuff[79] -= 128;
  }
}

void Ridley_Func_88(uint16 k) {  // 0xA6D09F
  int16 v6;
  uint16 v2;
  int v1 = k >> 1;
  if ((tilemap_stuff[v1 + 16] & 0x8000) != 0) {
    int v4 = k >> 1;
    uint16 v5 = tilemap_stuff[v4 + 17];
    if (v5 < tilemap_stuff[15]) {
      tilemap_stuff[(k >> 1) + 17] += tilemap_stuff[10];
      return;
    }
    if (v5 != 0xFFFF) {
      tilemap_stuff[v4 + 17] = -1;
      tilemap_stuff[v4 + 26] = 0x8000;
      tilemap_stuff[v4 + 28] = tilemap_stuff[v4 + 18];
    }
    if ((tilemap_stuff[v4 + 18] & 0x8000) == 0) {
      if ((tilemap_stuff[14] & 0x8000) != 0) {
        v2 = tilemap_stuff[10] + tilemap_stuff[v4 + 21];
        if ((int16)(v2 - tilemap_stuff[12]) >= 0) {
LABEL_15:
          tilemap_stuff[v4 + 18] = 0x8000;
          v2 = tilemap_stuff[12];
        }
      } else {
        tilemap_stuff[v4 + 20] = 3072;
        v2 = tilemap_stuff[10] + tilemap_stuff[v4 + 21];
        if ((int16)(v2 - tilemap_stuff[14]) >= 0) {
          if (!k || !tilemap_stuff[v4 + 6]) {
            tilemap_stuff[v4 + 21] = tilemap_stuff[14];
LABEL_3:;
            int v3 = k >> 1;
            tilemap_stuff[v3 + 16] = 0;
            tilemap_stuff[v3 + 17] = 0;
            tilemap_stuff[v3 + 18] ^= 0x8000;
            return;
          }
          goto LABEL_15;
        }
      }
LABEL_24:
      tilemap_stuff[v4 + 21] = v2;
      goto LABEL_25;
    }
    if ((tilemap_stuff[13] & 0x8000) != 0) {
      v6 = tilemap_stuff[v4 + 21] - tilemap_stuff[10] - 1;
      if ((int16)(v6 - tilemap_stuff[11]) < 0) {
LABEL_22:
        tilemap_stuff[v4 + 18] = 0;
        v2 = tilemap_stuff[11];
        goto LABEL_24;
      }
    } else {
      tilemap_stuff[v4 + 20] = 3072;
      v6 = tilemap_stuff[v4 + 21] - tilemap_stuff[10] - 1;
      if ((int16)(v6 - tilemap_stuff[13]) < 0) {
        if (!k || !tilemap_stuff[v4 + 6]) {
          tilemap_stuff[v4 + 21] = tilemap_stuff[13];
          goto LABEL_3;
        }
        goto LABEL_22;
      }
    }
    v2 = v6 + 1;
    goto LABEL_24;
  }
  v2 = tilemap_stuff[v1 + 21];
LABEL_25:;
  uint16 r18 = (uint8)v2;
  if (k)
    r18 = (uint8)(LOBYTE(tilemap_stuff[(k >> 1) + 11]) + r18);
  int v7 = k >> 1;
  tilemap_stuff[v7 + 24] = ComputeSinMult(HIBYTE(tilemap_stuff[v7 + 19]), r18);
  tilemap_stuff[v7 + 25] = ComputeCosMult(HIBYTE(tilemap_stuff[v7 + 19]), r18);
}

void Ridley_Func_89(uint16 a) {  // 0xA6D19D
  uint16 v1, r18, r20;
  int8 v2; // t0

  v2 = a;
  LOBYTE(v1) = 0;
  HIBYTE(v1) = v2;
  tilemap_stuff[5] = v1;
  Enemy_Ridley *E = Get_Ridley(0);
  if (E->ridley_var_10 != 1 && E->ridley_var_01) {
    Point16U pt;
    if (Ridley_Func_90(&pt)) {
      r18 = pt.x - tilemap_stuff[22];
      r20 = pt.y - tilemap_stuff[23];
    } else {
      r18 = samus_x_pos - tilemap_stuff[22];
      r20 = samus_y_pos + 24 - tilemap_stuff[23];
    }
    r18 = (uint8)-(CalculateAngleFromXY(r18, r20) + 0x80);
    if (E->ridley_var_10) {
      uint16 v7 = r18;
      if (r18 >= 0x18 && r18 < 0xE8)
        v7 = 24;
      uint16 v8 = tilemap_stuff[5] + v7 + 0x4000;
      if (v8 >= tilemap_stuff[21]) {
        tilemap_stuff[14] = v8;
        tilemap_stuff[10] = 8;
      }
    } else {
      uint16 v5 = r18;
      if (r18 >= 0x18 && r18 < 0xE8)
        v5 = 232;
      uint16 v6 = v5 + 16128 - tilemap_stuff[5];
      if (v6 < tilemap_stuff[21]) {
        tilemap_stuff[13] = v6;
        tilemap_stuff[10] = 8;
      }
    }
  }
}

uint8 Ridley_Func_90(Point16U *out) {  // 0xA6D242
  Rect16U rect = { tilemap_stuff[82], tilemap_stuff[83], 64, 64 };

  if (!projectile_counter)
    return 0;
  uint16 v1 = 0;
  while (1) {
    int v2 = v1 >> 1;
    if ((HIBYTE(projectile_type[v2]) & 0xF) == 1 || (HIBYTE(projectile_type[v2]) & 0xF) == 2) {
      uint16 v3 = abs16(projectile_x_pos[v2] - rect.x);
      bool v4 = v3 < projectile_x_radius[v2];
      uint16 v5 = v3 - projectile_x_radius[v2];
      if (v4 || v5 < rect.w) {
        uint16 v6 = abs16(projectile_y_pos[v2] - rect.y);
        v4 = v6 < projectile_y_radius[v2];
        uint16 v7 = v6 - projectile_y_radius[v2];
        if (v4 || v7 < rect.h)
          break;
      }
    }
    v1 += 2;
    if ((int16)(v1 - 10) >= 0)
      return 0;
  }
  int v8 = v1 >> 1;
  out->x = projectile_x_pos[v8];
  out->y = projectile_y_pos[v8];
  return 1;
}

void Ridley_Func_91(void) {  // 0xA6D2AA
  Enemy_Ridley *E = Get_Ridley(0);
  if (!sign16(tilemap_stuff[10] - 8)) {
    uint16 v1 = E->ridley_var_0E + 1;
    E->ridley_var_0E = v1;
    if (sign16(v1 - 16))
      return;
    if (!E->ridley_var_0F)
      QueueSfx3_Max6(0x21);
  }
  E->ridley_var_0E = 0;
}

void Ridley_Func_92(void) {  // 0xA6D2D6
  tilemap_stuff[10] = 1;
  tilemap_stuff[11] = 16368;
  tilemap_stuff[12] = 16448;
  tilemap_stuff[13] = -1;
  tilemap_stuff[14] = -1;
  tilemap_stuff[9] = 240;
  tilemap_stuff[15] = 16;
  tilemap_stuff[17] = 17;
  tilemap_stuff[27] = 17;
  tilemap_stuff[37] = 17;
  tilemap_stuff[47] = 17;
  tilemap_stuff[57] = 17;
  tilemap_stuff[67] = 17;
  tilemap_stuff[77] = 17;
  Ridley_Func_93(0x2024, addr_word_A6D36E);
  Ridley_Func_93(0x2026, addr_word_A6D37C);
  Ridley_Func_93(0x202A, addr_word_A6D38A);
  Ridley_Func_93(0x202C, addr_word_A6D398);
  Ridley_Func_93(0x202E, addr_word_A6D3A6);
  tilemap_stuff[16] = 0;
  tilemap_stuff[26] = 0;
  tilemap_stuff[36] = 0;
  tilemap_stuff[46] = 0;
  tilemap_stuff[56] = 0;
  tilemap_stuff[66] = 0;
  tilemap_stuff[76] = 0;
}

void Ridley_Func_93(uint16 j, uint16 k) {  // 0xA6D3B4
  uint16 *dst = (uint16*)(g_ram + j);
  const uint16 *src = (const uint16 *)RomPtr_A6(k);
  int v3 = 7;
  do {
    *dst = *src;
    dst += 10, src++;
  } while (--v3);
}

void Ridley_Func_94(void) {  // 0xA6D3D4
  Ridley_Func_D3DC(0);
}

void Ridley_Func_95(void) {  // 0xA6D3D9
  Ridley_Func_D3DC(0x8000);
}

void Ridley_Func_D3DC(uint16 a) {  // 0xA6D3DC
  tilemap_stuff[16] = a;
  tilemap_stuff[26] = a;
  tilemap_stuff[36] = a;
  tilemap_stuff[46] = a;
  tilemap_stuff[56] = a;
  tilemap_stuff[66] = a;
  tilemap_stuff[76] = a;
}

void Ridley_Func_96(void) {  // 0xA6D3F9
  int16 v0;

  Ridley_Func_93(0x2026, addr_word_A6D37C);
  v0 = 7;
  uint16 v1 = 0;
  do {
    int v2 = v1 >> 1;
    tilemap_stuff[v2 + 21] = 0x8000 - tilemap_stuff[v2 + 21];
    tilemap_stuff[v2 + 18] |= 0x8000;
    v1 += 20;
    --v0;
  } while (v0);
}

uint8 Ridley_Func_97(void) {  // 0xA6D431
  return (tilemap_stuff[76] & (uint16)(tilemap_stuff[66] & tilemap_stuff[56] & tilemap_stuff[46] & tilemap_stuff[36] & tilemap_stuff[26] & tilemap_stuff[16])) != 0;
}

void Ridley_Func_98(void) {  // 0xA6D453
  uint16 enemy_ptr = Get_Ridley(cur_enemy_index)->base.enemy_ptr;
  const uint8 *v1 = RomPtr_A0(enemy_ptr);
  Samus_DealDamage(SuitDamageDivision(GET_WORD(v1 + 6)));
}

void Ridley_Func_99(uint16 a) {  // 0xA6D467
  Enemy_Ridley *E = Get_Ridley(0);
  E->base.current_instruction = a;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
}

void Ridley_Func_100(void) {  // 0xA6D474
  int16 v0;

  v0 = 0;
  Enemy_Ridley *E = Get_Ridley(0);
  uint16 health = E->base.health;
  if (health < 0x2328) {
    v0 = 1;
    if (health < 0x1518) {
      v0 = 2;
      if (health < 0x708)
        v0 = 3;
    }
  }
  E->ridley_var_12 = v0;
  if ((int16)(v0 - 1) >= 0)
    Ridley_D495(v0 - 1);
}

void Ridley_D495(uint16 r18) {  // 0xA6D495
  if (!palette_change_num)
    WriteColorsToPalette(0x1E2, 0xa6, 28 * r18 - 7062, 0xE);
}

void Ridley_Func_101(void) {  // 0xA6D4B5
  Enemy_Ridley *E = Get_Ridley(0);
  if (E->ridley_var_01 && E->ridley_var_0D >= 0x32)
    Ridley_D495((E->ridley_var_0D >= 0x46) ? 2 : 0);
}

void Ridley_Func_102(void) {  // 0xA6D4DA
  int16 v0;

  v0 = 3584;
  Enemy_Ridley *E = Get_Ridley(0);
  if (E->base.flash_timer >= 2 && ((random_enemy_counter + 1) & 2) != 0)
    v0 = 0;
  E->ridley_var_0C = v0;
}

uint8 Ridley_Func_103(uint16 k, uint16 j) {  // 0xA6D4F9
  uint16 prod = Mult8x8(j >> 4, room_width_in_blocks);
  return (level_data[prod + (k >> 4)] & 0xF000) != 0;
}

void Ridley_Func_104_0(uint16 k, uint16 j, uint16 r18, uint16 r20) {  // 0xA6D523
  Ridley_Func_104(k, j, 0, r18, r20);
}

void Ridley_Func_104(uint16 k, uint16 j, uint16 a, uint16 r18, uint16 r20) {  // 0xA6D526
  int16 v4;
  int16 v6;
  int16 v8;
  int16 v9;
  int16 v11;
  int16 ridley_var_C;
  int16 v17;

  uint16 R26 = a;
  uint16 r24 = g_byte_A6D61F[j];
  Ridley_Func_105(k, r18, r24, a);
  Enemy_Ridley *E = Get_Ridley(k);
  v4 = E->base.y_pos - r20;
  if (v4) {
    if (v4 >= 0) {
      uint16 RegWord = SnesDivide(v4, r24);
      if (!RegWord)
        RegWord = 1;
      uint16 r22 = RegWord;
      ridley_var_C = E->ridley_var_C;
      bool v14 = 0;
      if (ridley_var_C >= 0) {
        uint16 v15 = ridley_var_C - R26;
        v15 -= 8;
        uint16 v16 = r22;
        ridley_var_C = v15 - v16;
      }
      v17 = ridley_var_C - r22;
      if (sign16(v17 + 1280))
        v17 = -1280;
      E->ridley_var_C = v17;
    } else {
      uint16 v5 = SnesDivide(r20 - E->base.y_pos, r24);
      if (!v5)
        v5 = 1;
      uint16 r22 = v5;
      v6 = E->ridley_var_C;
      bool v7 = 0;
      if (v6 < 0) {
        v8 = R26 + v6;
        v8 += 8;
        v9 = v8;
        v6 = r22 + v9;
      }
      v11 = r22 + v6;
      if (!sign16(v11 - 1280))
        v11 = 1280;
      E->ridley_var_C = v11;
    }
  }
}

void Ridley_Func_105(uint16 k, uint16 r18, uint16 r24, uint16 r26) {  // 0xA6D5A9
  int16 v2;
  int16 v4;
  int16 v6;
  int16 v7;
  int16 v9;
  int16 ridley_var_B;
  int16 v15;

  Enemy_Ridley *E = Get_Ridley(k);
  v2 = E->base.x_pos - r18;
  if (v2) {
    if (v2 >= 0) {
      uint16 RegWord = SnesDivide(v2, r24);
      if (!RegWord)
        RegWord = 1;
      uint16 r22 = RegWord;
      ridley_var_B = E->ridley_var_B;
      bool v12 = 0;
      if (ridley_var_B >= 0) {
        uint16 v13 = ridley_var_B - r26;
        v13 -= 8;
        uint16 v14 = r22;
        ridley_var_B = v13 - v14;
      }
      v15 = ridley_var_B - r22;
      if (sign16(v15 + 1280))
        v15 = -1280;
      E->ridley_var_B = v15;
    } else {
      uint16 v3 = SnesDivide(r18 - E->base.x_pos, r24);
      if (!v3)
        v3 = 1;
      uint16 r22 = v3;
      v4 = E->ridley_var_B;
      if (v4 < 0) {
        v6 = r26 + v4;
        v6 += 8;
        v7 = v6;
        v4 = r22 + v7;
      }
      v9 = r22 + v4;
      if (!sign16(v9 - 1280))
        v9 = 1280;
      E->ridley_var_B = v9;
    }
  }
}

void Ridley_Func_106(uint16 k, uint16 j, uint16 r18, uint16 r20) {  // 0xA6D62F
  int16 v3;
  int16 v5;
  int16 v6;
  int16 ridley_var_C;
  int16 v9;

  uint16 r24 = g_byte_A6D712[j];
  Ridley_Func_107(k, r18, r24);
  Enemy_Ridley *E = Get_Ridley(k);
  v3 = E->base.y_pos - r20;
  if (v3) {
    if (v3 >= 0) {
      uint16 RegWord = SnesDivide(v3, r24);
      if (!RegWord)
        RegWord = 1;
      uint16 r22 = RegWord;
      ridley_var_C = E->ridley_var_C;
      if (ridley_var_C >= 0)
        ridley_var_C = ridley_var_C - r22 - r22;
      v9 = ridley_var_C - r22;
      if (sign16(v9 + 1280))
        v9 = -1280;
      E->ridley_var_C = v9;
    } else {
      uint16 v4 = SnesDivide(r20 - E->base.y_pos, r24);
      if (!v4)
        v4 = 1;
      uint16 r22 = v4;
      v5 = E->ridley_var_C;
      if (v5 < 0)
        v5 += r22 + r22;
      v6 = r22 + v5;
      if (!sign16(v6 - 1280))
        v6 = 1280;
      E->ridley_var_C = v6;
    }
  }
}

void Ridley_Func_107(uint16 k, uint16 r18, uint16 r24) {  // 0xA6D6A6
  Enemy_Ridley *E = Get_Ridley(k);
  int16 v2 = E->base.x_pos - r18;
  if (!v2)
    return;
  if (v2 >= 0) {
    uint16 RegWord = SnesDivide(v2, r24);
    uint16 r22 = RegWord = RegWord ? RegWord : 1;
    int16 s = E->ridley_var_B;
    if (s >= 0)
      s -= 2 * RegWord;
    s -= RegWord;
    E->ridley_var_B = sign16(s + 1280) ? -1280 : s;
  } else {
    uint16 v3 = SnesDivide(r18 - E->base.x_pos, r24);
    uint16 r22 = v3 ? v3 : 1;
    int16 s = E->ridley_var_B;
    if (s < 0)
      s += 2 * r22;
    s += r22;
    E->ridley_var_B = !sign16(s - 1280) ? 1280 : s;
  }
}

void Ridley_Func_111(uint16 r18, uint16 r20, uint16 r22) {  // 0xA6D800
  Enemy_Ridley *E = Get_Ridley(0);
  if (r22 != E->ridley_var_0B) {
    if ((int16)(r22 - E->ridley_var_0B) >= 0) {
      uint16 v2 = E->ridley_var_0B + 32;
      if (!sign16(v2 - r22))
        v2 = r22;
      E->ridley_var_0B = v2;
    } else {
      uint16 v1 = E->ridley_var_0B - 32;
      if (sign16(v1 - r22))
        v1 = r22;
      E->ridley_var_0B = v1;
    }
  }
  uint16 v3;
  if ((r18 & 0x8000) != 0) {
    v3 = E->ridley_var_0A + r18;
    if (sign16(v3 - r20))
      LABEL_13:
    v3 = r20;
  } else {
    v3 = E->ridley_var_0A + r18;
    if (!sign16(v3 - r20))
      goto LABEL_13;
  }
  E->ridley_var_0A = v3;
  r18 = HIBYTE(v3);
  E->ridley_var_B = Math_MultBySin(E->ridley_var_0B, r18);
  E->ridley_var_C = Math_MultByCos(E->ridley_var_0B, r18);
}

void Ridley_Func_112(void) {  // 0xA6D86B
  Enemy_Ridley *Ridley;
  uint16 v1;
  Enemy_Ridley *v2;
  uint16 ridley_var_23;
  uint16 ridley_var_21;

  Ridley = Get_Ridley(0);
  Ridley->ridley_var_1F = 0;
  v1 = cur_enemy_index;
  v2 = Get_Ridley(cur_enemy_index);

  int with_carry = HIBYTE(v2->base.x_subpos) + LOBYTE(v2->ridley_var_B);
  HIBYTE(v2->base.x_subpos) = with_carry;
  ridley_var_23 = v2->base.x_pos + (int8)(v2->ridley_var_B >> 8) + (with_carry >> 8);
  if ((int16)(ridley_var_23 - Ridley->ridley_var_22) >= 0) {
    if ((int16)(ridley_var_23 - Ridley->ridley_var_23) >= 0) {
      v2->ridley_var_B = 0;
      Ridley->ridley_var_1F = 2;
      ridley_var_23 = Ridley->ridley_var_23;
    }
    v2->base.x_pos = ridley_var_23;
  } else {
    Ridley_Func_113(v1);
    v2->base.x_pos = Ridley->ridley_var_22;
    v2->ridley_var_B = 0;
    Ridley->ridley_var_1F = 1;
  }

  with_carry = HIBYTE(v2->base.y_subpos) + LOBYTE(v2->ridley_var_C);
  HIBYTE(v2->base.y_subpos) = with_carry;

  ridley_var_21 = v2->base.y_pos + (int8)(v2->ridley_var_C >> 8) + (with_carry >> 8);
  if ((int16)(ridley_var_21 - Ridley->ridley_var_20) >= 0) {
    if ((int16)(ridley_var_21 - Ridley->ridley_var_21) >= 0) {
      v2->ridley_var_C = 0;
      Ridley->ridley_var_1F = 8;
      ridley_var_21 = Ridley->ridley_var_21;
    }
    v2->base.y_pos = ridley_var_21;
  } else {
    v2->base.y_pos = Ridley->ridley_var_20;
    v2->ridley_var_C = 0;
    Ridley->ridley_var_1F = 4;
  }
}

void Ridley_Func_113(uint16 k) {  // 0xA6D914
  if (area_index != 2) {
    Enemy_Ridley *E = Get_Ridley(k);
    uint16 r18 = abs16(E->ridley_var_B);
    uint16 v2 = abs16(E->ridley_var_C);
    if (v2 < r18)
      v2 = r18;
    if (v2 >= 0x280) {
      uint16 v3;
      if (area_index == 2)
        v3 = 24;
      else
        v3 = 33;
      earthquake_type = v3;
      earthquake_timer = 12;
    }
  }
}

void Ridley_Func_114(void) {  // 0xA6D955
  Enemy_Ridley *E = Get_Ridley(0);
  uint16 ridley_var_10 = E->ridley_var_10, v2;
  if (!ridley_var_10) {
    if (*(int16 *)((uint8 *)&E->base.enemy_ptr + 1) < 0)
      return;
    v2 = addr_kRidley_Ilist_E6F0;
    goto LABEL_7;
  }
  if (ridley_var_10 != 1 && *(int16 *)((uint8 *)&E->base.enemy_ptr + 1) < 0) {
    v2 = addr_kRidley_Ilist_E706;
LABEL_7:
    E->base.current_instruction = v2;
    E->base.instruction_timer = 2;
    E->base.timer = 0;
  }
}

void Ridley_Func_115(void) {  // 0xA6D97D
  int16 v1;

  Ridley_Func_116();
  Enemy_Ridley *E = Get_Ridley(0);
  v1 = E->ridley_var_09 - E->ridley_var_08;
  E->ridley_var_09 = v1;
  if (v1 < 0) {
    E->ridley_var_09 = 32;
    uint16 v2 = E->ridley_var_07 + 1;
    if (v2 >= 0xA)
      v2 = 0;
    E->ridley_var_07 = v2;
  }
}

void Ridley_Func_116(void) {  // 0xA6D9A8
  Enemy_Ridley *E = Get_Ridley(0);
  uint16 r18 = abs16(E->ridley_var_B);
  uint16 v1 = r18 + abs16(E->ridley_var_C);
  if (v1) {
    uint16 v2 = v1 - r18;
    if (sign16(v2 - r18))
      v2 = r18;
    int v5 = (((4 * v2) & 0xF00) >> 8);
    if (v5 >= 7)
      v5 = 7;
    uint16 v6 = g_word_A6D9ED[v5];
    if ((E->ridley_var_C & 0x8000) == 0)
      v6 >>= 1;
    E->ridley_var_08 = v6;
  } else {
    E->ridley_var_08 = 0;
  }
}

void Ridley_Func_117(void) {  // 0xA6D9FD
  Enemy_Ridley *E = Get_Ridley(0);
  E->ridley_var_05 = addr_word_A6DA71;
  E->ridley_var_06 = 1;
}

void Ridley_Func_118(void) {  // 0xA6DA0C
  VramWriteEntry *v7;

  Enemy_Ridley *E = Get_Ridley(0);
  uint16 v1 = E->ridley_var_06 - 1;
  E->ridley_var_06 = v1;
  if (!v1) {
    int i;
    uint16 v3, *v4;
    for (i = E->ridley_var_05; ; E->ridley_var_05 = i) {
      v3 = i;
      v4 = (uint16 *)RomPtr_A6(i);
      i = *v4;
      if ((*v4 & 0x8000) == 0)
        break;
    }
    E->ridley_var_06 = i;
    uint16 v6 = vram_write_queue_tail;
    v7 = gVramWriteEntry(vram_write_queue_tail);
    *(VoidP *)((uint8 *)&v7->src.addr + 1) = -20480;
    *(VoidP *)((uint8 *)&v7[1].src.addr + 1) = -20480;
    v7->src.addr = v4[1];
    v7[1].src.addr = v4[2];
    v7->vram_dst = 29216;
    v7[1].vram_dst = 29472;
    v7->size = 64;
    v7[1].size = 64;
    vram_write_queue_tail = v6 + 14;
    gVramWriteEntry(v6 + 14)->size = 0;
    E->ridley_var_05 = v3 + 6;
  }
}

void Ridley_Func_119(uint8 carry) {  // 0xA6DA8B
  VramWriteEntry *v3;

  uint16 v1 = addr_off_A6DAD0;
  if (carry)
    v1 = addr_off_A6DAD4;
  uint16 v2 = vram_write_queue_tail;
  v3 = gVramWriteEntry(vram_write_queue_tail);
  *(VoidP *)((uint8 *)&v3->src.addr + 1) = -20480;
  *(VoidP *)((uint8 *)&v3[1].src.addr + 1) = -20480;
  const uint8 *v4 = RomPtr_A6(v1);
  v3->src.addr = GET_WORD(v4);
  v3[1].src.addr = GET_WORD(v4 + 2);
  v3->vram_dst = 31424;
  v3[1].vram_dst = 31680;
  v3->size = 128;
  v3[1].size = 128;
  vram_write_queue_tail = v2 + 14;
  gVramWriteEntry(v2 + 14)->size = 0;
}

void Ridley_Func_120(void) {  // 0xA6DAD8
  int16 ridley_var_10;

  Enemy_Ridley *E = Get_Ridley(0);
  ridley_var_10 = E->ridley_var_10;
  if (ridley_var_10) {
    if (ridley_var_10 == 1)
      return;
    ridley_var_10 = 10;
  }
  uint16 v2 = g_off_A6DB02[E->ridley_var_07 + ridley_var_10];
  sub_A6DC13(v2, E->base.x_pos, E->base.y_pos, E->ridley_var_0C);
}

void sub_A6DB2A(void) {  // 0xA6DB2A
  if ((gEnemyData(0)->properties & kEnemyProps_Invisible) == 0) {
    sub_A6DBC2(kRidley_Ilist_DCBA[(uint8)((LOBYTE(tilemap_stuff[71]) + LOBYTE(tilemap_stuff[81]) + 8) & 0xF0) >> 4],
        tilemap_stuff[82], tilemap_stuff[83]);
    sub_A6DBC2(0xDC9E, tilemap_stuff[72], tilemap_stuff[73]);
    sub_A6DBC2(0xDC9E, tilemap_stuff[62], tilemap_stuff[63]);
    sub_A6DBC2(0xDC97, tilemap_stuff[52], tilemap_stuff[53]);
    sub_A6DBC2(0xDC97, tilemap_stuff[42], tilemap_stuff[43]);
    sub_A6DBC2(0xDC90, tilemap_stuff[32], tilemap_stuff[33]);
    sub_A6DBC2(0xDC90, tilemap_stuff[22], tilemap_stuff[23]);
  }
}

void sub_A6DBC2(uint16 j, uint16 x, uint16 y) {  // 0xA6DBC2
  sub_A6DC13(j, x, y, Get_Ridley(0)->ridley_var_0C);
}

void sub_A6DC13(uint16 j, uint16 r18, uint16 r20, uint16 r22) {  // 0xA6DC13
  const uint8 *p = RomPtr_A6(j);
  int n = GET_WORD(p);
  p += 2;
  int idx = oam_next_ptr;
  do {
    int16 v9 = r20 + (int8)p[2] - layer1_y_pos;
    if (v9 >= 0 && sign16(v9 - 224)) {
      uint16 x = r18 + GET_WORD(p) - layer1_x_pos;
      OamEnt *v11 = gOamEnt(idx);
      v11->xcoord = x;
      v11->ycoord = v9;
      *(uint16 *)&v11->charnum = r22 | GET_WORD(p + 3);
      oam_ext[idx >> 5] |= (((x & 0x100) >> 8) | (*(int16 *)p < 0) * 2) << (2 * ((idx >> 2) & 7));
      idx = (idx + 4) & 0x1FF;
    }
    p += 5;
  } while (--n);
  oam_next_ptr = idx;
}

uint8 Ridley_Func_121(void) {  // 0xA6DE7A
  int16 y_pos;
  int16 v2;
  int16 x_pos;
  int16 v4;

  Enemy_Ridley *E = Get_Ridley(0);
  y_pos = E->base.y_pos;
  uint8 result = 1;
  if (y_pos >= 0) {
    v2 = y_pos + 32 - layer1_y_pos;
    if (v2 >= 0) {
      if (sign16(v2 - 288)) {
        x_pos = E->base.x_pos;
        if (x_pos >= 0) {
          v4 = x_pos + 32 - layer1_x_pos;
          if (v4 >= 0) {
            if (sign16(v4 - 320))
              return 0;
          }
        }
      }
    }
  }
  return result;
}

uint16 Ridley_Func_122(Rect16U rect) {  // 0xA6DEA6
  int16 v2;

  if (!projectile_counter)
    return 0xffff;
  uint16 result = 0;
  while (1) {
    int v1 = result >> 1;
    v2 = projectile_type[v1];
    if (v2 < 0) {
      if (sign16((HIBYTE(v2) & 0xF) - 3)) {
        uint16 v3 = abs16(projectile_x_pos[v1] - rect.x);
        bool v4 = v3 < projectile_x_radius[v1];
        uint16 v5 = v3 - projectile_x_radius[v1];
        if (v4 || v5 < rect.w) {
          uint16 v6 = abs16(projectile_y_pos[v1] - rect.y);
          v4 = v6 < projectile_y_radius[v1];
          uint16 v7 = v6 - projectile_y_radius[v1];
          if (v4 || v7 < rect.h)
            break;
        }
      }
    }
    result += 2;
    if ((int16)(result - 10) >= 0)
      return 0xffff;
  }
  int v8 = result >> 1;
  projectile_x_pos[v8] = rect.x;
  projectile_y_pos[v8] = rect.y;
  projectile_dir[v8] |= 0x10;
  return result;
}

void Ridley_Func_123(uint16 j) {  // 0xA6DF08
  int v1 = j >> 1;
  uint16 v2;
  if ((projectile_dir[v1] & 0xF) == 7) {
    v2 = 1;
  } else if ((projectile_dir[v1] & 0xF) == 2) {
    v2 = 8;
  } else {
    v2 = 5;
  }
  projectile_dir[v1] = v2;
}

uint8 Ridley_Func_124(Rect16U rect) {  // 0xA6DF29
  uint16 v0 = abs16(samus_x_pos - rect.x);
  bool v1 = v0 < samus_x_radius;
  uint16 v2 = v0 - samus_x_radius;
  if (v1 || v2 < rect.w) {
    uint16 v3 = abs16(samus_y_pos - rect.y);
    v1 = v3 < samus_y_radius;
    uint16 v4 = v3 - samus_y_radius;
    if (v1 || v4 < rect.h)
      return 1;
  }
  return 0;
}

void sub_A6DF59(void) {  // 0xA6DF59
  NormalEnemyTouchAiSkipDeathAnim_CurEnemy();
}

void sub_A6DF60(void) {  // 0xA6DF60
  Ridley_Func_125();
}

void Ridley_Func_125(void) {  // 0xA6DF66
  Ridley_Func_98();
  samus_invincibility_timer = 96;
  samus_knockback_timer = 5;
  knockback_x_dir = (int16)(samus_x_pos - Get_Ridley(cur_enemy_index)->base.x_pos) >= 0;
}

void Ridley_Shot(void) {  // 0xA6DF8A
  int16 v0;

  if (area_index == 2) {
    NormalEnemyShotAiSkipDeathAnim_CurEnemy();
  } else {
    v0 = 13;
    Enemy_Ridley *E = Get_Ridley(0);
    uint16 flash_timer = E->base.flash_timer;
    if (flash_timer) {
      if (flash_timer & 1)
        v0 = 14;
    }
    E->base.flash_timer = v0;
    ++E->ridley_var_0D;
  }
}

void Ridley_Powerbomb(void) {  // 0xA6DFB2
  NormalEnemyPowerBombAiSkipDeathAnim_CurEnemy();
}

void Ridley_Func_126(void) {  // 0xA6DFB7
  Enemy_Ridley *E = Get_Ridley(0);
  if (!E->base.health && (E->ridley_var_01 & 0x8000) == 0) {
    E->ridley_var_01 = -1;
    E->base.properties |= kEnemyProps_Intangible;
    E->ridley_var_A = FUNC16(Ridley_Func_63);
  }
}

void Ridley_Func_127(void) {  // 0xA6DFD9
  Rect16U rect = { tilemap_stuff[82], tilemap_stuff[83], 14, 14 };
  if (Ridley_Func_124(rect)) {
    Enemy_Ridley *E = Get_Ridley(0);
    Samus_DealDamage(SuitDamageDivision(E->ridley_var_1C));
    samus_invincibility_timer = 96;
    samus_knockback_timer = 5;
    knockback_x_dir = (int16)(samus_x_pos - tilemap_stuff[82]) >= 0;
  }
}

void Ridley_Func_128(void) {  // 0xA6E01B
  Enemy_Ridley *E = Get_Ridley(cur_enemy_index);
  uint16 r18 = (uint8)-CalculateAngleFromXY(projectile_x_pos[0] - E->base.x_pos, projectile_y_pos[0] - E->base.y_pos);
  uint16 v1 = 4 * projectile_damage[collision_detection_index];
  if (v1 >= 0x300)
    v1 = 768;
  uint16 a = v1;
  r18 = Math_MultBySin(v1, r18);
  if (((E->ridley_var_B ^ r18) & 0x8000) != 0)
    E->ridley_var_B += r18;
  r18 = Math_MultByCos(a, r18);
  if (((E->ridley_var_C ^ r18) & 0x8000) != 0)
    E->ridley_var_C += r18;
}

void Ridley_Func_129(void) {  // 0xA6E088
  if ((Get_Ridley(0)->base.properties & kEnemyProps_Intangible) == 0) {
    uint16 v0 = Ridley_Func_122((Rect16U) { tilemap_stuff[82], tilemap_stuff[83], 14, 14});
    if (sign16(v0)) {
      v0 = Ridley_Func_122((Rect16U) { tilemap_stuff[72], tilemap_stuff[73], 10, 10 });
      if (sign16(v0))
        return;
    }
    int v1 = v0 >> 1;
    eproj_spawn_pt = (Point16U){ projectile_x_pos[v1], projectile_y_pos[v1] };
    uint16 v2 = 12;
    if ((HIBYTE(projectile_type[v1]) & 0xF) == 1) {
      QueueSfx1_Max6(0x3D);
      v2 = 6;
    }
    SpawnEprojWithRoomGfx(0xE509, v2);
  }
}

const uint16 *Ridley_Instr_5(uint16 k, const uint16 *jp) {  // 0xA6E4BE
  Get_Ridley(0)->ridley_var_0F = 89;
  QueueSfx2_Max6(0x59);
  return jp;
}

const uint16 *Ridley_Instr_6(uint16 k, const uint16 *jp) {  // 0xA6E4CA
  Get_Ridley(0)->ridley_var_0F = 0;
  return jp;
}

const uint16 *Ridley_Instr_10(uint16 k, const uint16 *jp) {  // 0xA6E4D2
  if (area_index == 2 || !sign16(samus_health - 30))
    return jp + 1;
  Get_Ridley(0)->ridley_var_00 = 8;
  return INSTR_RETURN_ADDR(jp[0]);
}

const uint16 *Ridley_Instr_4(uint16 k, const uint16 *jp) {  // 0xA6E4EE
  if (Get_Ridley(0)->ridley_var_1B)
    return INSTR_RETURN_ADDR(jp[0]);
  else
    return INSTR_RETURN_ADDR(jp[1]);
}

const uint16 *Ridley_Instr_3(uint16 k, const uint16 *jp) {  // 0xA6E4F8
  if (Get_Ridley(0)->ridley_var_1B)
    return jp + 1;
  else
    return INSTR_RETURN_ADDR(jp[0]);
}

const uint16 *Ridley_Instr_2(uint16 k, const uint16 *jp) {  // 0xA6E501
  samus_prev_x_pos = samus_x_pos;
  samus_prev_y_pos = samus_y_pos;
  Get_Ridley(0)->ridley_var_1D = jp[0];
  return jp + 1;
}

const uint16 *Ridley_Instr_1(uint16 k, const uint16 *jp) {  // 0xA6E517
  if (Get_Ridley(0)->ridley_var_10)
    return INSTR_RETURN_ADDR(jp[0]);
  else
    return jp + 1;
}

const uint16 *Ridley_Instr_14(uint16 k, const uint16 *jp) {  // 0xA6E51F
  Enemy_Ridley *E = Get_Ridley(0);
  E->base.x_pos += jp[0];
  E->base.y_pos += jp[1];
  return jp + 2;
}

const uint16 *Ridley_Instr_9(uint16 k, const uint16 *jp) {  // 0xA6E71C
  Get_Ridley(0)->ridley_var_10 = 0;
  Ridley_Func_96();
  return jp;
}

const uint16 *Ridley_Instr_7(uint16 k, const uint16 *jp) {  // 0xA6E727
  Get_Ridley(0)->ridley_var_10 = 1;
  return jp;
}

const uint16 *Ridley_Instr_8(uint16 k, const uint16 *jp) {  // 0xA6E72F
  Get_Ridley(0)->ridley_var_10 = 2;
  Ridley_Func_96();
  return jp;
}

void Ridley_Func_131(uint16 k) {  // 0xA6E828
  Ridley_Func_132(k, 0);
  Ridley_Func_132(k, 1);
  Ridley_Func_132(k, 2);
  Ridley_Func_132(k, 3);
}

void Ridley_Func_132(uint16 k, uint16 a) {  // 0xA6E840
  Get_Ridley(0)->ridley_parameter_1 = a;
  SpawnEprojWithGfx(a, k, addr_stru_869634);
}

const uint16 *Ridley_Instr_11(uint16 k, const uint16 *jp) {  // 0xA6E84D
  uint16 r18;
  Enemy_Ridley *E = Get_Ridley(0);
  if (E->ridley_var_10) {
    uint16 v4 = (uint8)-(CalculateAngleFromXY(samus_x_pos - (E->base.x_pos + 25), samus_y_pos - (E->base.y_pos - 43)) + 0x80);
    if (v4 < 0x50) {
      if (v4 >= 0x15)
        goto LABEL_13;
    } else if (v4 < 0xC0) {
      v4 = 80;
LABEL_13:
      r18 = v4;
      goto LABEL_14;
    }
    v4 = 21;
    goto LABEL_13;
  }
  uint16 v3;
  v3 = (uint8)-(CalculateAngleFromXY(samus_x_pos - (E->base.x_pos - 25), samus_y_pos - (E->base.y_pos - 43)) + 0x80);
  if (v3 >= 0xB0) {
    if (v3 < 0xEB)
      goto LABEL_7;
LABEL_6:
    v3 = 235;
    goto LABEL_7;
  }
  if (v3 < 0x40)
    goto LABEL_6;
  v3 = 176;
LABEL_7:
  r18 = v3;
LABEL_14:
  E->ridley_var_19 = Math_MultBySin(0x500, r18);
  E->ridley_var_1A = Math_MultByCos(0x500, r18);
  return jp;
}

const uint16 *Ridley_Instr_12(uint16 k, const uint16 *jp) {  // 0xA6E904
  return Ridley_E90C(k, jp, 0);
}

const uint16 *Ridley_Instr_13(uint16 k, const uint16 *jp) {  // 0xA6E909
  return Ridley_E90C(k, jp, 0xe);
}

const uint16 *Ridley_E90C(uint16 k, const uint16 *jp, uint16 a) {  // 0xA6E90C
  eproj_unk1995 = a;
  Enemy_Ridley *E = Get_Ridley(0);
  SpawnEprojWithRoomGfx(0x9642, E->ridley_var_10);
  return jp;
}

const uint16 *Ridley_Instr_15(uint16 k, const uint16 *jp) {  // 0xA6E969
  Enemy_Ridley *E = Get_Ridley(0);
  E->ridley_var_A = FUNC16(CeresRidley_Func_9);
  E->ridley_var_C = -352;
  return jp;
}

const uint16 *Ridley_Instr_16(uint16 k, const uint16 *jp) {  // 0xA6E976
  Enemy_Ridley *E = Get_Ridley(0);
  E->ridley_var_A = FUNC16(Ridley_Func_3);
  E->ridley_var_C = -352;
  return jp;
}
