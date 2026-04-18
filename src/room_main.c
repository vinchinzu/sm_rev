// Room main callbacks extracted from Bank $8F.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"

enum RoomScrollColor {
  kRoomScroll_Red = 0,
  kRoomScroll_Blue = 1,
  kRoomScroll_Green = 2,
};

enum {
  kRoomMainAsmVar0 = 0,
  kRoomMainAsmVar2 = 2,
  kElevatubeVelocity = 4,
  kElevatubeAcceleration = 6,
  kEarthquake_LightHorizontal = 18,
  kEarthquake_MediumHorizontal = 21,
  kEarthquake_Diagonal = 23,
  kEarthquake_DiagonalStrong = 26,
  kRandomExplosionSfxSmall = 0x24,
  kRandomExplosionSfxLarge = 0x25,
};

static inline uint16 ReadRoomMainAsmWord(uint16 offset) {
  return *(uint16 *)(room_main_asm_variables + offset);
}

static inline int16 ReadRoomMainAsmSignedWord(uint16 offset) {
  return *(int16 *)(room_main_asm_variables + offset);
}

static inline void WriteRoomMainAsmWord(uint16 offset, uint16 value) {
  *(uint16 *)(room_main_asm_variables + offset) = value;
}

static inline uint32 ReadRoomMainAsmLong(void) {
  return *(uint32 *)room_main_asm_variables;
}

static inline void WriteRoomMainAsmLong(uint32 value) {
  *(uint32 *)room_main_asm_variables = value;
}

static void RoomCode_ScrollingSkyLand_(void) {  // 0x8FC116
  RoomCode_ScrollingSkyLand();
}

static void RoomCode_ScrollingSkyOcean_(void) {  // 0x8FC11B
  RoomMainAsm_ScrollingSkyOcean();
}

static void RoomCode_GenRandomExplodes(uint16 x_r18, uint16 y_r20) {  // 0x8FC1A9
  static const uint8 kRandomExplosionSprite[8] = { 3, 3, 9, 12, 12, 18, 18, 21 };
  static const uint8 kRandomExplosionSfx[8] = { kRandomExplosionSfxSmall, 0, 0, kRandomExplosionSfxLarge, 0, 0, 0, 0 };

  uint16 v1 = NextRandom() & 0xF;
  if (v1 < 8) {
    uint16 v2 = kRandomExplosionSfx[v1];
    if (v2)
      QueueSfx2_Max6(v2);
  }
  CreateSpriteAtPos(x_r18, y_r20, kRandomExplosionSprite[v1 & 7], 0);
}

static void RoomCode_GenRandomExplodes_Nonblank(void) {  // 0x8FC131
  if (!time_is_frozen_flag && (nmi_frame_counter_word & 1) == 0) {
    uint16 random = NextRandom();
    uint16 x = layer1_x_pos + (uint8)random;
    uint16 y = layer1_y_pos + HIBYTE(random);
    uint16 prod = Mult8x8(y >> 4, room_width_in_blocks);
    if ((level_data[prod + (x >> 4)] & 0x3FF) != 255)
      RoomCode_GenRandomExplodes(x, y);
  }
}

static void RoomCode_ScrollingSkyLand_Shakes(void) {  // 0x8FC120
  RoomCode_ScrollingSkyLand();
  RoomCode_GenRandomExplodes_Nonblank();
  earthquake_timer |= 0x8000;
}

static void RoomCode_ExplodeShakes(void) {  // 0x8FC124
  RoomCode_GenRandomExplodes_Nonblank();
  earthquake_timer |= 0x8000;
}

static void RoomCode_GenRandomExplodes_4th(void) {  // 0x8FC183
  if (!time_is_frozen_flag && (nmi_frame_counter_word & 3) == 0) {
    uint16 random = NextRandom();
    RoomCode_GenRandomExplodes(layer1_x_pos + (uint8)random, layer1_y_pos + HIBYTE(random));
  }
}

static void RoomCode_ScrollRightDachora(void) {  // 0x8FC1E6
  if (scrolls[11] == kRoomScroll_Green && layer1_y_pos < 0x500 && layer1_x_pos < 0x380)
    layer1_x_pos += (layer1_x_pos >= 0x380) + 3;
}

static void RoomCode_Elevatube(void) {  // 0x8FE2B6
  samus_x_pos = 128;
  samus_x_subpos = 0;
  int32 displacement = INT16_SHL8(ReadRoomMainAsmWord(kElevatubeVelocity));
  uint32 next_position = ReadRoomMainAsmLong() + displacement;
  WriteRoomMainAsmLong(next_position);
  Samus_MoveDown_NoSolidColl(displacement);
  if ((uint16)(ReadRoomMainAsmWord(kElevatubeAcceleration) + ReadRoomMainAsmWord(kElevatubeVelocity) + 3616) < 0x1C41) {
    WriteRoomMainAsmWord(
        kElevatubeVelocity,
        ReadRoomMainAsmWord(kElevatubeVelocity) + ReadRoomMainAsmWord(kElevatubeAcceleration));
  }
}

static void RoomCode_CeresElevatorShaft_(void) {  // 0x8FE51F
  RoomCode_CeresElevatorShaft();
}

static const uint16 kRoomCode_SpawnCeresFallingDebris_Tab[16] = {  // 0x8FE525
   0x50,  0x60,  0x70,  0x80,
   0x90,  0xa0,  0xb0,  0xc0,
   0xd0,  0xe0,  0xf0, 0x110,
  0x130, 0x150, 0x170, 0x190,
};

static void RoomCode_SpawnCeresFallingDebris(void) {
  if (ceres_status && (--*(uint16 *)room_main_asm_variables, ReadRoomMainAsmSignedWord(kRoomMainAsmVar0) < 0)) {
    WriteRoomMainAsmWord(kRoomMainAsmVar0, 8);
    uint16 v0 = (random_number & 0x8000) ? addr_stru_869742 : addr_stru_869734;
    SpawnEprojWithRoomGfx(v0, kRoomCode_SpawnCeresFallingDebris_Tab[random_number & 0xF]);
  }
}

static void RoomCode_HandleCeresRidleyGetaway(void) {  // 0x8FE571
  if (ceres_status & 1)
    HandleCeresRidleyGetawayCutscene();
}

static void RoomCode_ShakeScreenHorizDiag(void) {  // 0x8FE57C
  if (ReadRoomMainAsmWord(kRoomMainAsmVar0)) {
    if (--*(uint16 *)room_main_asm_variables == 0)
      earthquake_type = kEarthquake_LightHorizontal;
  } else if (NextRandom() < 0x200) {
    WriteRoomMainAsmWord(kRoomMainAsmVar0, '*');
    earthquake_type = kEarthquake_Diagonal;
  }
  RoomCode_GenRandomExplodes_4th();
}

static void RoomCode_ShakeScreenHorizDiagStrong(void) {  // 0x8FE5A4
  if (ReadRoomMainAsmWord(kRoomMainAsmVar0)) {
    if (--*(uint16 *)room_main_asm_variables == 0)
      WriteRoomMainAsmWord(kRoomMainAsmVar2, kEarthquake_MediumHorizontal);
  } else if (NextRandom() < 0x180) {
    WriteRoomMainAsmWord(kRoomMainAsmVar0, '*');
    WriteRoomMainAsmWord(kRoomMainAsmVar2, kEarthquake_DiagonalStrong);
  }
  earthquake_type = ReadRoomMainAsmWord(kRoomMainAsmVar2);
  RoomCode_GenRandomExplodes_4th();
}

static void RoomCode_CrocomireShaking(void) {  // 0x8FE8CD
  uint16 ai_var_D;

  if ((enemy_data[0].properties & 0x200) == 0) {
    if (enemy_data[0].ai_var_A == 64) {
      reg_BG1VOFS = enemy_data[1].ai_var_D + bg1_y_offset + layer1_y_pos;
    } else if ((enemy_data[0].ai_var_B & 0x400) != 0) {
      if (sign16(--enemy_data[1].ai_var_D + 7)) {
        ai_var_D = enemy_data[1].ai_var_D - 2 * (enemy_data[1].ai_var_D + 7);
      } else {
        ai_var_D = enemy_data[1].ai_var_D;
      }
      reg_BG1VOFS += ai_var_D;
      reg_BG2VOFS = ai_var_D - 48;
    } else if (enemy_data[0].ai_var_C == 34 && enemy_data[0].ai_var_D) {
      --enemy_data[0].ai_var_D;
      if ((enemy_data[0].ai_var_D & 1) != 0)
        layer1_x_pos -= 4;
      else
        layer1_x_pos += 4;
    }
  }
}

static const int16 kRoomCode_RidleyRoomShaking_X[8] = { 0, 2, 2, 2, 0, -2, -2, -2 };
static const int16 kRoomCode_RidleyRoomShaking_Y[8] = { -2, -2, 0, 2, 2, 2, 0, -2 };

static void RoomCode_RidleyRoomShaking(void) {  // 0x8FE950
  uint16 ai_var_A = enemy_data[4].ai_var_A;
  if (enemy_data[4].ai_var_A) {
    --enemy_data[4].ai_var_A;
    int v1 = ai_var_A - 1;
    reg_BG1HOFS += kRoomCode_RidleyRoomShaking_X[v1];
    reg_BG2HOFS += kRoomCode_RidleyRoomShaking_X[v1];
    reg_BG1VOFS += kRoomCode_RidleyRoomShaking_Y[v1];
    reg_BG2VOFS += kRoomCode_RidleyRoomShaking_Y[v1];
  }
}

static void CallRoomCode(uint32 ea) {
  switch (ea) {
  case fnRoomCode_ScrollingSkyLand_: RoomCode_ScrollingSkyLand_(); return;
  case fnRoomCode_ScrollingSkyOcean_: RoomCode_ScrollingSkyOcean_(); return;
  case fnRoomCode_ScrollingSkyLand_Shakes: RoomCode_ScrollingSkyLand_Shakes(); return;
  case fnRoomCode_ExplodeShakes: RoomCode_ExplodeShakes(); return;
  case fnRoomCode_ScrollRightDachora: RoomCode_ScrollRightDachora(); return;
  case fnRoomCode_Elevatube: RoomCode_Elevatube(); return;
  case fnRoomCode_CeresElevatorShaft_: RoomCode_CeresElevatorShaft_(); return;
  case fnnullsub_148: return;
  case fnRoomCode_SpawnCeresFallingDebris: RoomCode_SpawnCeresFallingDebris(); return;
  case fnRoomCode_HandleCeresRidleyGetaway: RoomCode_HandleCeresRidleyGetaway(); return;
  case fnRoomCode_ShakeScreenHorizDiag: RoomCode_ShakeScreenHorizDiag(); return;
  case fnRoomCode_GenRandomExplodes_4th_: RoomCode_GenRandomExplodes_4th(); return;
  case fnRoomCode_ShakeScreenHorizDiagStrong: RoomCode_ShakeScreenHorizDiagStrong(); return;
  case fnRoomCode_CrocomireShaking: RoomCode_CrocomireShaking(); return;
  case fnRoomCode_RidleyRoomShaking: RoomCode_RidleyRoomShaking(); return;
  default: Unreachable();
  }
}

void RunRoomMainCode(void) {  // 0x8FE8BD
  if (room_main_code_ptr)
    CallRoomCode(room_main_code_ptr | 0x8F0000);
}
