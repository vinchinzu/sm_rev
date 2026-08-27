// Enemy AI - Ceres Ridley — peeled from Bank $A6
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"

#define g_byte_A6E269 ((uint8*)RomFixedPtr(0xa6e269))
#define g_word_A6E2AA ((uint16*)RomFixedPtr(0xa6e2aa))
#define g_word_A6E30A ((uint16*)RomFixedPtr(0xa6e30a))
#define g_off_A6A4EB ((uint16*)RomFixedPtr(0xa6a4eb))
#define g_off_A6A743 ((uint16*)RomFixedPtr(0xa6a743))
#define g_word_A6AE4D ((uint16*)RomFixedPtr(0xa6ae4d))
#define g_word_A6AF2F ((uint16*)RomFixedPtr(0xa6af2f))
#define g_word_A6B00F ((uint16*)RomFixedPtr(0xa6b00f))
#define g_off_A6ACDA ((uint16*)RomFixedPtr(0xa6acda))
#define g_off_A6AD45 ((uint16*)RomFixedPtr(0xa6ad45))

void CeresRidley_Init(void) {  // 0xA6A0F5
  int16 v4;
  int16 v8;
  Enemy_CeresRidley *E = Get_CeresRidley(0);

  if ((boss_bits_for_area[area_index] & 1) != 0) {
    E->base.properties |= kEnemyProps_Intangible | kEnemyProps_Deleted | kEnemyProps_Invisible;
  } else {
    for (int i = 4094; i >= 0; i -= 2)
      tilemap_stuff[i >> 1] = 0;
    DisableMinimapAndMarkBossRoomAsExplored();
    E->cry_parameter_1 = 0;
    E->cry_parameter_2 = 0;
    Ridley_Func_99(addr_kRidley_Ilist_E538);
    E->base.palette_index = 3584;
    E->cry_var_0C = 3584;
    E->base.extra_properties |= 4;
    E->cry_var_01 = 0;
    E->cry_var_0D = 0;
    earthquake_type = 0;
    earthquake_timer = 0;
    Ridley_Func_92();
    Ridley_Func_117();
    if (area_index == 2) {
      E->base.properties |= kEnemyProps_BlockPlasmaBeam | kEnemyProps_Intangible;
      E->base.x_pos = 96;
      E->base.y_pos = 394;
      E->cry_var_A = FUNC16(CeresRidley_Func_3);
      E->cry_var_B = 0;
      E->cry_var_C = 0;
      E->base.layer = 5;
      E->cry_var_01 = 0;
      E->cry_var_02 = 1;
      E->cry_var_20 = 64;
      E->cry_var_21 = 416;
      E->cry_var_22 = 64;
      E->cry_var_23 = 224;
      E->cry_var_10 = 2;
      E->cry_var_1C = 120;
      WriteColorsToTargetPalette(0xa6, 0x140, addr_word_A6E1CF, 0x20);
      uint16 v3 = 0;
      v4 = 15;
      do {
        int v5 = v3 >> 1;
        target_palettes[v5 + 113] = 0;
        target_palettes[v5 + 241] = 0;
        v3 += 2;
        --v4;
      } while (v4);
    } else {
      E->base.properties |= kEnemyProps_BlockPlasmaBeam | kEnemyProps_Intangible;
      E->base.x_pos = 186;
      E->base.y_pos = 169;
      ceres_status = 0;
      E->cry_var_10 = 0;
      E->cry_var_02 = 0;
      tilemap_stuff[1] = 1;
      E->cry_var_1C = 15;
      E->cry_var_A = FUNC16(CeresRidley_Func_3);
      E->cry_var_B = 0;
      E->cry_var_C = 0;
      E->cry_var_20 = -32;
      E->cry_var_21 = 176;
      E->cry_var_22 = 40;
      E->cry_var_23 = 224;
      WriteColorsToTargetPalette(0xa6, 0x140, addr_word_A6E16F, 0x20);
      uint16 v7 = 482;
      v8 = 15;
      do {
        target_palettes[v7 >> 1] = 0;
        v7 += 2;
        --v8;
      } while (v8);
      E->cry_var_03 = addr_kBabyMetroid_Ilist_BF31;
      E->cry_var_04 = 1;
      gRam8800_Default(0)->var_40 = FUNC16(Ridley_Func_50);
      E->cry_var_07 = 5;
      QueueMusic_Delayed8(0);
    }
  }
}

void CeresRidley_Main(void) {  // 0xA6A288
  Enemy_CeresRidley *E = Get_CeresRidley(0);
  E->base.health = 0x7FFF;
  CallRidleyFunc(E->cry_var_A | 0xA60000);
  if (!ceres_status) {
    if (Get_CeresRidley(0)->cry_var_02) {
      Ridley_Func_102();
      Ridley_Func_112();
      Ridley_Func_115();
      CeresRidley_Func_1();
      Ridley_Func_70();
      Ridley_Func_101();
      Ridley_Func_118();
      sub_A6DB2A();
      Ridley_Func_120();
    }
    Ridley_A2DC();
  }
}

void CeresRidley_Func_1(void) {  // 0xA6A2BD
  if (random_number >= 0xFF00)
    tilemap_stuff[15] = (random_number & 0xF) + 8;
}

void CeresRidley_Hurt(void) {  // 0xA6A2D3
  Ridley_Func_102();
  sub_A6DB2A();
  Ridley_Func_120();
  Ridley_A2DC();
}

void CeresRidley_Func_2(void) {  // 0xA6A354
  Enemy_CeresRidley *E = Get_CeresRidley(0);
  E->cry_var_B = 0;
  E->cry_var_C = 0;
}

void CeresRidley_Func_3(void) {  // 0xA6A35B
  int16 v1;

  if (door_transition_flag_enemies) {
  } else {
    Enemy_CeresRidley *E = Get_CeresRidley(0);
    E->cry_var_A = FUNC16(CeresRidley_A377);
    v1 = 512;
    if (area_index == 2)
      v1 = 170;
    E->cry_var_F = v1;
    CeresRidley_A377();
  }
}

void CeresRidley_A377(void) {  // 0xA6A377
  Enemy_CeresRidley *E = Get_CeresRidley(0);
  if ((--E->cry_var_F & 0x8000) != 0) {
    E->cry_var_A = FUNC16(CeresRidley_Func_4);
    E->cry_var_E = 0;
    E->cry_var_F = 0;
  }
}

void CeresRidley_Func_4(void) {  // 0xA6A389
  int16 v2;

  Enemy_CeresRidley *E = Get_CeresRidley(0);
  if ((E->cry_var_E & 0x8000) == 0) {
    uint16 cry_var_E = E->cry_var_E;
    if (++E->cry_var_F) {
      E->cry_var_F = 0;
      v2 = g_byte_A6E269[cry_var_E];
      if (v2 == 255) {
        E->cry_var_E = 0;
        E->cry_var_A = FUNC16(CeresRidley_Func_5);
        E->cry_var_02 = 1;
      } else {
        E->cry_var_E = cry_var_E + 1;
        int v3 = (uint16)(6 * v2) >> 1;
        palette_buffer[252] = g_word_A6E2AA[v3];
        palette_buffer[253] = g_word_A6E2AA[v3 + 1];
        palette_buffer[254] = g_word_A6E2AA[v3 + 2];
      }
    }
  }
}

void CeresRidley_Func_5(void) {  // 0xA6A3DF
  int16 v2;
  int16 v5;

  Enemy_CeresRidley *E = Get_CeresRidley(0);
  if (++E->cry_var_F >= 2) {
    E->cry_var_F = 0;
    uint16 cry_var_E = E->cry_var_E;
    uint16 r18 = 290, r20 = 482;
    v2 = 11;
    do {
      v5 = v2;
      uint16 v3 = g_word_A6E30A[cry_var_E >> 1];
      palette_buffer[r18 >> 1] = v3;
      palette_buffer[r20 >> 1] = v3;
      cry_var_E += 2;
      r18 += 2;
      r20 += 2;
      v2 = v5 - 1;
    } while (v5 != 1);
    if (cry_var_E >= 0x160) {
      if (area_index == 2)
        E->base.layer = 2;
      E->base.properties &= ~kEnemyProps_Intangible;
      E->cry_var_E = 0;
      E->cry_var_A = FUNC16(CeresRidley_Func_6);
      E->cry_var_F = 4;
      QueueMusic_Delayed8(5);
    } else {
      E->cry_var_E = cry_var_E;
    }
  }
}

void CeresRidley_Func_6(void) {  // 0xA6A455
  Enemy_CeresRidley *E = Get_CeresRidley(0);
  if ((--E->cry_var_F & 0x8000) != 0) {
    Ridley_Func_99(addr_kRidley_Ilist_E690);
    E->cry_var_F = 0;
    E->cry_var_A = FUNC16(CeresRidley_Func_7);
    if (area_index != 2)
      E->cry_var_F = 252;
  }
}

void CeresRidley_Func_7(void) {  // 0xA6A478
  Enemy_CeresRidley *E = Get_CeresRidley(0);

  if (area_index == 2) {
    bool v1 = (--E->cry_var_F & 0x8000) != 0;
    if (!v1)
      return;
    E->cry_var_F = 2;
    uint16 v2 = E->cry_var_E + 1;
    E->cry_var_E = v2;
    if (!(CeresRidley_Func_8(v2 - 1) & 1))
      return;
    fx_target_y_pos = 440;
    fx_y_vel = -96;
    fx_timer = 32;
  } else {
    bool v1 = (--E->cry_var_F & 0x8000) != 0;
    if (!v1)
      return;
  }
  E->cry_var_E = 0;
  Ridley_Func_99(addr_kRidley_Ilist_E91D);
  E->cry_var_08 = 8;
  E->cry_var_09 = 8;
  Ridley_Func_95();
  tilemap_stuff[0] = 1;
  E->cry_var_A = FUNC16(CeresRidley_Func_2);
}

uint8 CeresRidley_Func_8(uint16 a) {  // 0xA6A4D6
  uint16 v1 = g_off_A6A4EB[a];
  if (!v1)
    return 1;
  WriteColorsToPalette(0xE2, 0xa6, v1, 0xE);
  return 0;
}

void CeresRidley_Func_9(void) {  // 0xA6A6AF
  Enemy_CeresRidley *E = Get_CeresRidley(0);
  E->cry_var_C -= 16;
  if (sign16(E->base.y_pos - 112)) {
    E->cry_var_A = FUNC16(CeresRidley_Func_10);
    CeresRidley_Func_10();
  }
}

void CeresRidley_Func_10(void) {  // 0xA6A6C8
  Enemy_CeresRidley *E = Get_CeresRidley(0);
  E->cry_var_C += 20;
  if (sign16(E->base.y_pos - 80)) {
    E->cry_var_A = FUNC16(CeresRidley_Func_11);
    E->cry_var_01 = 1;
  }
}

void CeresRidley_Func_11(void) {  // 0xA6A6E8
  uint16 v1;

  Enemy_CeresRidley *E = Get_CeresRidley(0);
  if (E->cry_var_0D >= 0x64) {
    E->cry_var_01 = 0;
    E->cry_var_A = FUNC16(Ridley_Func_44);
    Ridley_Func_44();
  } else if (sign16(samus_health - 30)) {
    E->cry_var_01 = 0;
    E->cry_var_A = FUNC16(CeresRidley_Func_22);
    CeresRidley_Func_22();
  } else if (!(CeresRidley_Func_12() & 1)
             || (v1 = E->cry_var_00 + 1, E->cry_var_00 = v1, v1 >= 0x7C)) {
    E->cry_var_A = g_off_A6A743[random_number & 0xF];
    E->cry_var_00 = 0;
  }
}

uint8 CeresRidley_Func_12(void) {  // 0xA6A763
  Rect16U rect = { 192, 100, 8, 8 };
  Ridley_Func_106(0, 0, rect.x, rect.y);
  return Shitroid_Func_2(0, rect);
}

void CeresRidley_Func_13(void) {  // 0xA6A782
  Enemy_CeresRidley *E = Get_CeresRidley(0);
  uint16 v1 = abs16(E->cry_var_C);
  if (v1 < 0x80)
    v1 = 128;
  E->cry_var_C = sign16(E->cry_var_C) ? -v1 : v1;
  Ridley_Func_106(0, 0, E->base.x_pos, 88);
  if (sign16(E->base.y_pos - 80)) {
    uint16 v3 = E->cry_var_00 + 1;
    E->cry_var_00 = v3;
    if (v3 >= 0x30)
      E->cry_var_A = FUNC16(CeresRidley_Func_16);
  } else if (sign16(E->base.y_pos - 128)) {
    E->cry_var_17 = E->base.x_pos;
    E->cry_var_18 = E->base.y_pos;
    Ridley_Func_99(addr_kRidley_Ilist_E73A);
    E->cry_var_A = addr_loc_A6A7F9;
    E->cry_var_00 = 224;
    CeresRidley_A7F9();
  }
}

void CeresRidley_A7F9(void) {  // 0xA6A7F9
  uint16 r22 = sign16(random_number) ? -(random_number & 7) : (random_number & 7);
  Enemy_CeresRidley *E = Get_CeresRidley(0);
  uint16 r18 = E->cry_var_17 + r22;
  uint16 r20 = E->cry_var_18 + r22;
  Ridley_Func_106(0, 0, r18, r20);
  int16 v4 = E->cry_var_00 - 1;
  E->cry_var_00 = v4;
  if (v4 < 0) {
    E->cry_var_00 = 0;
    E->cry_var_A = FUNC16(CeresRidley_Func_11);
  }
}

void CeresRidley_Func_14(void) {  // 0xA6A83C
  Ridley_Func_99(addr_kRidley_Ilist_E548);
  Enemy_CeresRidley *E = Get_CeresRidley(0);
  E->cry_var_A = FUNC16(CeresRidley_Func_15);
  E->cry_var_F = 64;
  CeresRidley_Func_15();
}

void CeresRidley_Func_15(void) {  // 0xA6A84E
  bool v2; // sf

  Enemy_CeresRidley *E = Get_CeresRidley(0);
  uint16 r18 = samus_x_pos;
  uint16 v0 = samus_y_pos - 68;
  if (sign16(samus_y_pos - 132))
    v0 = 64;
  uint16 r20 = v0;
  Ridley_Func_106(0, 0xD, r18, r20);
  Rect16U rect = { r18, r20, 2, 2 };
  if (!Shitroid_Func_2(0, rect)
      || (v2 = (int16)(E->cry_var_F - 1) < 0, --E->cry_var_F, v2)) {
    E->cry_var_00 = 0;
    E->cry_var_A = FUNC16(CeresRidley_Func_11);
  }
}

void CeresRidley_Func_16(void) {  // 0xA6A88D
  Enemy_CeresRidley *E = Get_CeresRidley(0);
  E->cry_var_A = FUNC16(CeresRidley_Func_17);
  E->cry_var_F = 10;
  E->cry_var_0A = 0;
  tilemap_stuff[1] = 0;
  CeresRidley_Func_17();
}

void CeresRidley_Func_17(void) {  // 0xA6A8A4
  Ridley_Func_106(0, 1, 192, 80);
  Enemy_CeresRidley *E = Get_CeresRidley(0);
  if (sign16(E->base.y_pos - 96)) {
    E->cry_var_A = FUNC16(CeresRidley_Func_18);
    E->cry_var_F = 10;
    E->cry_var_0A = 0;
  }
}

void CeresRidley_Func_18(void) {  // 0xA6A8D4
  Ridley_Func_111(-32, -1024, 768);
  Enemy_CeresRidley *E = Get_CeresRidley(0);
  if ((--E->cry_var_F & 0x8000) != 0) {
    E->cry_var_A = FUNC16(CeresRidley_Func_19);
    E->cry_var_F = 36;
  }
}

void CeresRidley_Func_19(void) {  // 0xA6A8F8
  Ridley_Func_111(-512, -16384, 768);
  Enemy_CeresRidley *E = Get_CeresRidley(0);
  if ((--E->cry_var_F & 0x8000) != 0) {
    E->cry_var_A = FUNC16(CeresRidley_Func_20);
    E->cry_var_F = 28;
    tilemap_stuff[2] = 1;
  }
}

void CeresRidley_Func_20(void) {  // 0xA6A923
  Ridley_Func_111(-512, -30720, 768);
  Enemy_CeresRidley *E = Get_CeresRidley(0);
  if ((--E->cry_var_F & 0x8000) != 0) {
    E->cry_var_A = FUNC16(CeresRidley_Func_21);
    E->cry_var_F = 1;
  }
}

void CeresRidley_Func_21(void) {  // 0xA6A947
  Ridley_Func_111(-768, -30720, 768);
  Enemy_CeresRidley *E = Get_CeresRidley(0);
  if ((--E->cry_var_F & 0x8000) != 0) {
    E->cry_var_A = FUNC16(CeresRidley_Func_11);
    E->cry_var_00 = 0;
    tilemap_stuff[1] = 1;
  }
}

void CeresRidley_Func_22(void) {  // 0xA6A971
  Enemy_CeresRidley *E = Get_CeresRidley(0);
  E->cry_var_20 = -192;
  Ridley_Func_106(0, 1, 192, -128);
  if (sign16(E->base.y_pos + 128))
    CeresRidley_A994();
}

void CeresRidley_A994(void) {  // 0xA6A994
  Enemy_CeresRidley *E = Get_CeresRidley(0);
  E->cry_var_A = FUNC16(CeresRidley_Func_23);
  E->cry_var_F = 64;
  CeresRidley_Func_23();
}

void CeresRidley_Func_23(void) {  // 0xA6A9A0
  Enemy_CeresRidley *E = Get_CeresRidley(0);
  if ((--E->cry_var_F & 0x8000) != 0) {
    CeresRidley_Func_25();
    E->cry_var_B = 0;
    E->cry_var_C = 0;
    tilemap_stuff[0] = 0;
    E->cry_var_A = FUNC16(CeresRidley_Func_24);
    WriteColorsToPalette(0xA2, 0xa6, addr_word_A6A9E3, 0xF);
    WriteColorsToPalette(0x42, 0xa6, addr_word_A6AA01, 8);
    WriteColorsToPalette(0x1E2, 0xa6, addr_word_A6AA01, 8);
  }
}

void CeresRidley_Func_24(void) {  // 0xA6AA11
  Get_CeresRidley(0)->cry_var_A = FUNC16(nullsub_233);
  ceres_status = 1;
  CeresRidley_Func_27();
}

void CeresRidley_Func_25(void) {  // 0xA6AA20
  SpawnEnemy(0xA6, addr_stru_A6AA2F);
  SpawnEnemy(0xA6, addr_stru_A6AA3F);
}

void CeresRidley_Func_26(void) {  // 0xA6AA50
  Ridley_Func_61();
}

void CeresRidley_Func_27(void) {  // 0xA6AA54
  reg_BGMODE_fake = 7;
  irq_enable_mode7 = 1;
  reg_M7SEL = 0x80;
  reg_M7A = 256;
  reg_M7B = 256;
  reg_M7C = 256;
  reg_M7D = 256;
  reg_M7X = 64;
  reg_M7Y = 64;
  Enemy_CeresRidley *E = Get_CeresRidley(0);
  E->cry_var_32 = 0;
  E->cry_var_33 = 0;
  E->cry_var_0A = 1;
  E->cry_var_34 = -128;
  reg_BG1HOFS = -128;
  E->cry_var_35 = 32;
  reg_BG1VOFS = 32;
  E->cry_var_30 = 2048;
  E->cry_var_31 = 1024;
}

void HandleCeresRidleyGetawayCutscene(void) {  // 0xA6AAAF
  if (!Get_CeresRidley(0)->cry_var_32)
    CeresRidley_Func_28();
}

void CeresRidley_Func_28(void) {  // 0xA6AABD
  Enemy_CeresRidley *E = Get_CeresRidley(0);
  uint16 cry_var_33 = E->cry_var_33;
  E->cry_var_33 = cry_var_33 + 2;
  if (!cry_var_33)
    QueueSfx2_Max6(0x4E);
  if (cry_var_33 == 208) {
    Samus_SetPushedOutOfCeresRidley();
    earthquake_type = 35;
    earthquake_timer = 64;
  }
  int v2 = cry_var_33 >> 1;
  if (g_word_A6AE4D[v2] == 0xFFFF) {
    ++E->cry_var_32;
    reg_BGMODE_fake = 9;
    reg_M7SEL = 0;
    reg_M7A = 0;
    reg_M7B = 0;
    reg_M7C = 0;
    reg_M7D = 0;
    reg_M7X = 0;
    reg_M7Y = 0;
    reg_BG1HOFS = 0;
    reg_BG1VOFS = 0;
    E->cry_var_A = FUNC16(Ridley_Func_54);
    E->cry_var_F = 0;
  } else {
    E->cry_var_30 = g_word_A6AE4D[v2];
    uint16 v3 = g_word_A6AF2F[v2] + E->cry_var_35;
    E->cry_var_35 = v3;
    reg_BG1VOFS = v3;
    uint16 v4 = E->cry_var_34 - g_word_A6B00F[v2];
    E->cry_var_34 = v4;
    reg_BG1HOFS = v4;
    CeresRidley_Func_35(E->cry_var_30);
    E->cry_var_0A += 48;
    CeresRidley_Func_29();
    CeresRidley_Func_33();
    CeresRidley_Func_34();
  }
}

static uint16 CeresMult(uint16 a, uint16 b) {
  uint16 r = abs16(a) * b >> 8;
  return sign16(a) ? -r : r;
}

void CeresRidley_Func_29(void) {  // 0xA6AB5F
  Enemy_CeresRidley *E = Get_CeresRidley(0);
  uint16 r18 = HIBYTE(E->cry_var_0A);
  reg_M7A = CeresMult(CeresRidley_AC30(r18 + 64, 0x100), E->cry_var_30);
  reg_M7B = CeresMult(CeresRidley_AC30(r18, 0x100), E->cry_var_30);
  reg_M7C = CeresMult(-CeresRidley_AC30(r18, 0x100), E->cry_var_30);
  reg_M7D = CeresMult(CeresRidley_AC30(r18 + 64, 0x100), E->cry_var_30);
}

int16 CeresRidley_AC30(uint16 a, int16 r20) {  // 0xA6AC30
  return (int16)kSinCosTable8bit_Sext[(a & 0xff) + 64] * r20 >> 8;
}

void CeresRidley_Func_33(void) {  // 0xA6ACBC
  if ((nmi_frame_counter_word & 3) == 0) {
    Enemy_CeresRidley *E = Get_CeresRidley(0);
    uint16 v1 = (E->cry_var_36 + 1) & 3;
    E->cry_var_36 = v1;
    QueueMode7Transfers(0xA6, g_off_A6ACDA[v1]);
  }
}

void CeresRidley_Func_34(void) {  // 0xA6AD27
  if ((nmi_frame_counter_word & 7) == 0) {
    Enemy_CeresRidley *E = Get_CeresRidley(0);
    uint16 v1 = (E->cry_var_37 + 1) & 1;
    E->cry_var_37 = v1;
    QueueMode7Transfers(0xA6, g_off_A6AD45[v1]);
  }
}

void CeresRidley_Func_35(uint16 a) {  // 0xA6B0EF
  WriteColorsToPalette(0xA2, 0xa6, 32 * HIBYTE(a) - 0x4EF9, 0xF);
}
