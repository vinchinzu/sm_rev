// Enemies
#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "enemy_types.h"

#define g_off_A0C2DA ((uint16*)RomFixedPtr(0xa0c2da))
#define CHECK_locret_A0C434(Ek) (byte_A0C435[Ek] & 0x80 ? -1 : 0)
#define g_word_A0C49F ((uint16*)RomFixedPtr(0xa0c49f))
#define kAlignYPos_Tab0 ((uint8*)RomFixedPtr(0x948b2b))

typedef struct EnemyBlockCollInfo {
  int32 ebci_r18_r20;
  uint16 ebci_r24;
  uint16 ebci_r26;
  uint16 ebci_r28;
  uint16 ebci_r30;
  uint16 ebci_r32;
} EnemyBlockCollInfo;

typedef uint8 Func_EnemyBlockCollInfo_U8(EnemyBlockCollInfo *ebci);

static uint8 EnemyBlockCollReact_Vert(EnemyBlockCollInfo *ebci, uint16 k);
static uint8 EnemyBlockCollReact_Horiz(EnemyBlockCollInfo *ebci, uint16 k);
static uint8 Enemy_MoveRight_IgnoreSlopes_Inner(uint16 k, int32 amount32, uint16 r32);
static uint8 EnemyBlockCollHorizReact_Slope_NonSquare(EnemyBlockCollInfo *ebci);
static uint8 EnemyBlockCollHorizReact_Slope_Square(EnemyBlockCollInfo *ebci, uint16 k, uint16 a);
static uint8 EnemyBlockCollVertReact_Slope_NonSquare(EnemyBlockCollInfo *ebci);
static uint8 EnemyBlockCollVertReact_Slope_Square(EnemyBlockCollInfo *ebci, uint16 a, uint16 k);

static uint8 ClearCarry_13(EnemyBlockCollInfo *ebci) {  // 0xA0C2BC
  return 0;
}

static uint8 SetCarry_4(EnemyBlockCollInfo *ebci) {  // 0xA0C2BE
  return 1;
}

static uint8 EnemyBlockCollReact_Spike(EnemyBlockCollInfo *ebci) {  // 0xA0C2C0
  uint16 v0 = g_off_A0C2DA[BTS[cur_block_index] & 0x7F];
  if (!v0)
    return 1;
  SpawnPLM(v0);
  return 0;
}

static uint8 EnemyBlockCollHorizReact_Slope(EnemyBlockCollInfo *ebci) {  // 0xA0C2FA
  if ((BTS[cur_block_index] & 0x1F) >= 5) {
    current_slope_bts = BTS[cur_block_index];
    return EnemyBlockCollHorizReact_Slope_NonSquare(ebci);
  } else {
    return EnemyBlockCollHorizReact_Slope_Square(ebci, cur_block_index, BTS[cur_block_index] & 0x1F);
  }
}

static uint8 EnemyBlockCollVertReact_Slope(EnemyBlockCollInfo *ebci) {  // 0xA0C319
  if ((BTS[cur_block_index] & 0x1F) >= 5)
    return EnemyBlockCollVertReact_Slope_NonSquare(ebci);
  else
    return EnemyBlockCollVertReact_Slope_Square(ebci, BTS[cur_block_index] & 0x1F, cur_block_index);
}

static const uint8 byte_A0C435[20] = {  // 0xA0C32E
     0,    1, 0x82, 0x83,
     0, 0x81,    2, 0x83,
     0,    1,    2, 0x83,
     0, 0x81, 0x82, 0x83,
  0x80, 0x81, 0x82, 0x83,
};

static uint8 EnemyBlockCollHorizReact_Slope_Square(EnemyBlockCollInfo *ebci, uint16 k, uint16 a) {
  EnemyData *E = gEnemyData(cur_enemy_index);

  uint16 temp_collision_DD4 = 4 * a;
  uint16 temp_collision_DD6 = BTS[k] >> 6;
  uint16 i = 4 * a + (temp_collision_DD6 ^ ((ebci->ebci_r26 & 8) >> 3));
  if (!ebci->ebci_r28) {
    if (((LOBYTE(E->y_height) + LOBYTE(E->y_pos) - 1) & 8) == 0)
      return CHECK_locret_A0C434(i) < 0;
    goto LABEL_7;
  }
  if (ebci->ebci_r28 != ebci->ebci_r30 || ((E->y_pos - E->y_height) & 8) == 0) {
LABEL_7:
    if (CHECK_locret_A0C434(i) < 0)
      return 1;
  }
  return CHECK_locret_A0C434(i ^ 2) < 0;
}

static uint8 EnemyBlockCollVertReact_Slope_Square(EnemyBlockCollInfo *ebci, uint16 a, uint16 k) {  // 0xA0C3B2
  EnemyData *E = gEnemyData(cur_enemy_index);
  uint16 temp_collision_DD4 = 4 * a;
  uint16 temp_collision_DD6 = BTS[k] >> 6;
  uint16 i = 4 * a + (temp_collision_DD6 ^ ((ebci->ebci_r26 & 8) >> 2));
  if (!ebci->ebci_r28) {
    if (((LOBYTE(E->x_width) + LOBYTE(E->x_pos) - 1) & 8) == 0)
      return CHECK_locret_A0C434(i) < 0;
    goto LABEL_7;
  }
  if (ebci->ebci_r28 != ebci->ebci_r30 || ((E->x_pos - E->x_width) & 8) == 0) {
LABEL_7:
    if (CHECK_locret_A0C434(i) < 0)
      return 1;
  }
  return CHECK_locret_A0C434(i ^ 1) < 0;
}

static uint8 EnemyBlockCollHorizReact_Slope_NonSquare(EnemyBlockCollInfo *ebci) {  // 0xA0C449
  if ((ebci->ebci_r32 & 0x8000) == 0)
    return (ebci->ebci_r32 & 0x4000) != 0;
  int i = 2 * (current_slope_bts & 0x1F);
  if (ebci->ebci_r18_r20 >= 0) {
    ebci->ebci_r18_r20 = (ebci->ebci_r18_r20 >> 8) * g_word_A0C49F[i + 1];
  } else {
    ebci->ebci_r18_r20 = -(-(int16)(ebci->ebci_r18_r20 >> 8) * g_word_A0C49F[i + 1]);
  }
  return 0;
}

static uint8 EnemyBlockCollVertReact_Slope_NonSquare(EnemyBlockCollInfo *ebci) {  // 0xA0C51F
  int16 v3;
  int16 v5;
  int16 v6;
  uint16 v7;
  int16 v8;
  int16 v11;
  int16 v12;
  int16 x_pos;
  uint16 v14;
  int16 v15;
  uint16 mod = SnesModulus(cur_block_index, room_width_in_blocks);
  EnemyData *E = gEnemyData(cur_enemy_index);
  uint16 v1 = cur_block_index;

  if (ebci->ebci_r18_r20 < 0) {
    v11 = E->x_pos >> 4;
    if (v11 != mod)
      return 0;
    uint16 temp_collision_DD4 = (ebci->ebci_r24 - E->y_height) & 0xF ^ 0xF;
    uint16 temp_collision_DD6 = 16 * (BTS[v1] & 0x1F);
    v12 = BTS[v1] << 8;
    if (v12 < 0
        && ((v12 & 0x4000) != 0 ? (x_pos = E->x_pos ^ 0xF) : (x_pos = E->x_pos),
        (v14 = temp_collision_DD6 + (x_pos & 0xF),
        v15 = (kAlignYPos_Tab0[v14] & 0x1F) - temp_collision_DD4 - 1, v15 <= 0))) {
      E->y_pos = ebci->ebci_r24 - v15;
      E->y_subpos = 0;
      return 1;
    } else {
      return 0;
    }
  } else {
    v3 = E->x_pos >> 4;
    if (v3 != mod)
      return 0;
    uint16 temp_collision_DD4 = (E->y_height + ebci->ebci_r24 - 1) & 0xF;
    uint16 temp_collision_DD6 = 16 * (BTS[v1] & 0x1F);
    v5 = BTS[v1] << 8;
    if (v5 >= 0
      && ((v5 & 0x4000) != 0 ? (v6 = E->x_pos ^ 0xF) : (v6 = E->x_pos),
      (v7 = temp_collision_DD6 + (v6 & 0xF),
      v8 = (kAlignYPos_Tab0[v7] & 0x1F) - temp_collision_DD4 - 1,
      (kAlignYPos_Tab0[v7] & 0x1F) - temp_collision_DD4 == 1) || v8 < 0)) {
      E->y_pos = ebci->ebci_r24 + v8;
      E->y_subpos = -1;
      return 1;
    } else {
      return 0;
    }
  }
}

static uint8 EnemyBlockCollReact_HorizExt(EnemyBlockCollInfo *ebci) {  // 0xA0C619
  uint8 t = BTS[cur_block_index];
  if (t) {
    cur_block_index += (int8)t;
    return 0xff;
  }
  return 0;
}

static uint8 EnemyBlockCollReact_VertExt(EnemyBlockCollInfo *ebci) {  // 0xA0C64F
  if (BTS[cur_block_index]) {
    cur_block_index += (int8)BTS[cur_block_index] * room_width_in_blocks;
    return 0xff;
  }
  return 0;
}

uint8 Enemy_MoveRight_SlopesAsWalls(uint16 k, int32 amt) {  // 0xA0C69D
  return Enemy_MoveRight_IgnoreSlopes_Inner(k, amt, 0x4000);
}

uint8 Enemy_MoveRight_ProcessSlopes(uint16 k, int32 amt) {  // 0xA0C6A4
  return Enemy_MoveRight_IgnoreSlopes_Inner(k, amt, 0x8000);
}

uint8 Enemy_MoveRight_IgnoreSlopes(uint16 k, int32 amt) {  // 0xA0C6AB
  return Enemy_MoveRight_IgnoreSlopes_Inner(k, amt, 0);
}

// amount32 is r20:r18
static uint8 Enemy_MoveRight_IgnoreSlopes_Inner(uint16 k, int32 amount32, uint16 r32) {  // 0xA0C6AD
  if (!amount32)
    return 0;
  EnemyData *E = gEnemyData(k);
  uint16 r28 = (uint16)(E->y_height + E->y_pos - 1 - ((E->y_pos - E->y_height) & 0xFFF0)) >> 4;
  uint16 r30 = r28;
  uint16 prod = Mult8x8((uint16)(E->y_pos - E->y_height) >> 4, room_width_in_blocks);
  uint32 new_pos = amount32 + __PAIR32__(E->x_pos, E->x_subpos);
  uint16 v5;
  uint16 r24 = new_pos >> 16;  // read by EnemyBlockCollVertReact_Slope_NonSquare
  if (sign32(amount32))
    v5 = (new_pos >> 16) - E->x_width;
  else
    v5 = E->x_width + (new_pos >> 16) - 1;
  EnemyBlockCollInfo ebci = { .ebci_r18_r20 = amount32, .ebci_r24 = r24, .ebci_r26 = v5, .ebci_r28 = r28, .ebci_r30 = r30, .ebci_r32 = r32};
  uint16 v6 = 2 * (prod + (v5 >> 4));
  while (!(EnemyBlockCollReact_Horiz(&ebci, v6) & 1)) {
    v6 += room_width_in_blocks * 2;
    if ((--ebci.ebci_r28 & 0x8000) != 0) {
      AddToHiLo(&E->x_pos, &E->x_subpos, ebci.ebci_r18_r20);
      return 0;
    }
  }
  if (sign32(amount32)) {
    uint16 v12 = E->x_width + 1 + (ebci.ebci_r26 | 0xF);
    if (v12 <= E->x_pos)
      E->x_pos = v12;
    E->x_subpos = 0;
    return 1;
  } else {
    uint16 v10 = (ebci.ebci_r26 & 0xFFF0) - E->x_width;
    if (v10 >= E->x_pos)
      E->x_pos = v10;
    E->x_subpos = -1;
    return 1;
  }
}

uint8 Enemy_MoveDown(uint16 k, int32 amount32) {  // 0xA0C786
  int16 v6;
  uint16 v5;
  if (!amount32)
    return 0;
  EnemyData *E = gEnemyData(k);
  uint16 r28 = (uint16)(E->x_width + E->x_pos - 1 - ((E->x_pos - E->x_width) & 0xFFF0)) >> 4;
  uint16 r30 = r28;
  uint32 new_pos = amount32 + __PAIR32__(E->y_pos, E->y_subpos);
  uint16 r24 = new_pos >> 16;  // read by EnemyBlockCollVertReact_Slope_NonSquare
  if (sign32(amount32))
    v5 = (new_pos >> 16) - E->y_height;
  else
    v5 = E->y_height + (new_pos >> 16) - 1;
  uint16 prod = Mult8x8(v5 >> 4, room_width_in_blocks);
  v6 = (uint16)(E->x_pos - E->x_width) >> 4;
  EnemyBlockCollInfo ebci = { .ebci_r18_r20 = amount32, .ebci_r24 = r24, .ebci_r26 = v5, .ebci_r28 = r28, .ebci_r30 = r30, .ebci_r32 = 0 };
  for (int i = 2 * (prod + v6); !(EnemyBlockCollReact_Vert(&ebci, i) & 1); i += 2) {
    if ((--ebci.ebci_r28 & 0x8000) != 0) {
      E->y_subpos = new_pos, E->y_pos = new_pos >> 16;
      return 0;
    }
  }
  if (sign32(amount32)) {
    uint16 v13 = E->y_height + 1 + (ebci.ebci_r26 | 0xF);
    if (v13 <= E->y_pos)
      E->y_pos = v13;
    E->y_subpos = 0;
    return 1;
  } else {
    uint16 v10 = (ebci.ebci_r26 & 0xFFF0) - E->y_height;
    if (v10 >= E->y_pos)
      E->y_pos = v10;
    E->y_subpos = -1;
    return 1;
  }
}

static Func_EnemyBlockCollInfo_U8 *const off_A0C859[16] = {  // 0xA0C845
  ClearCarry_13,
  EnemyBlockCollHorizReact_Slope,
  ClearCarry_13,
  ClearCarry_13,
  ClearCarry_13,
  EnemyBlockCollReact_HorizExt,
  ClearCarry_13,
  ClearCarry_13,
  SetCarry_4,
  SetCarry_4,
  EnemyBlockCollReact_Spike,
  SetCarry_4,
  SetCarry_4,
  EnemyBlockCollReact_VertExt,
  SetCarry_4,
  SetCarry_4,
};

static uint8 EnemyBlockCollReact_Horiz(EnemyBlockCollInfo *ebci, uint16 k) {
  cur_block_index = k >> 1;
  uint8 rv = 0;
  do {
    rv = off_A0C859[(level_data[cur_block_index] & 0xf000) >> 12](ebci);
  } while (rv & 0x80);
  return rv;
}

static Func_EnemyBlockCollInfo_U8 *const off_A0C88D[16] = {  // 0xA0C879
  ClearCarry_13,
  EnemyBlockCollVertReact_Slope,
  ClearCarry_13,
  ClearCarry_13,
  ClearCarry_13,
  EnemyBlockCollReact_HorizExt,
  ClearCarry_13,
  ClearCarry_13,
  SetCarry_4,
  SetCarry_4,
  EnemyBlockCollReact_Spike,
  SetCarry_4,
  SetCarry_4,
  EnemyBlockCollReact_VertExt,
  SetCarry_4,
  SetCarry_4,
};

static uint8 EnemyBlockCollReact_Vert(EnemyBlockCollInfo *ebci, uint16 k) {
  cur_block_index = k >> 1;
  uint8 rv = 0;
  do {
    rv = off_A0C88D[(level_data[cur_block_index] & 0xf000) >> 12](ebci);
  } while (rv & 0x80);
  return rv;
}

void CalculateBlockContainingPixelPos(uint16 xpos, uint16 ypos) {
  uint16 prod = Mult8x8(ypos >> 4, room_width_in_blocks);
  cur_block_index = prod + (xpos >> 4);
}

uint8 EnemyFunc_C8AD(uint16 k) {  // 0xA0C8AD
  uint8 result = 0;

  EnemyData *E = gEnemyData(k);
  CalculateBlockContainingPixelPos(E->x_pos, E->y_pos + E->y_height - 1);
  if ((level_data[cur_block_index] & 0xF000) == 4096 && (BTS[cur_block_index] & 0x1F) >= 5) {
    result = 1;
    uint16 temp_collision_DD4 = (E->y_height + E->y_pos - 1) & 0xF;
    uint16 temp_collision_DD6 = 16 * (BTS[cur_block_index] & 0x1F);
    if ((BTS[cur_block_index] & 0x80) == 0) {
      uint16 j = (BTS[cur_block_index] & 0x40) != 0 ? E->x_pos ^ 0xF : E->x_pos;
      int16 v4 = (kAlignYPos_Tab0[temp_collision_DD6 + (j & 0xF)] & 0x1F) - temp_collision_DD4 - 1;
      if (v4 < 0)
        E->y_pos += v4;
    }
  }
  CalculateBlockContainingPixelPos(E->x_pos, E->y_pos - E->y_height);
  if ((level_data[cur_block_index] & 0xF000) == 4096 && (BTS[cur_block_index] & 0x1F) >= 5) {
    result = 1;
    uint16 temp_collision_DD4 = (E->y_pos - E->y_height) & 0xF ^ 0xF;
    uint16 temp_collision_DD6 = 16 * (BTS[cur_block_index] & 0x1F);
    if (BTS[cur_block_index] & 0x80) {
      uint16 j = (BTS[cur_block_index] & 0x40) != 0 ? E->x_pos ^ 0xF : E->x_pos;
      int16 v7 = (kAlignYPos_Tab0[temp_collision_DD6 + (j & 0xF)] & 0x1F) - temp_collision_DD4 - 1;
      if (v7 < 0)
        E->y_pos -= v7;
    }
  }
  return result;
}
