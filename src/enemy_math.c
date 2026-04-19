// Enemies
#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "enemy_types.h"

#define kSine8bit ((uint8*)RomFixedPtr(0xa0b143))
#define kEquationForQuarterCircle ((uint16*)RomFixedPtr(0xa0b7ee))

static uint32 SineMult8bitInner(uint16 magnitude, uint16 angle) {  // 0xA0B0DA
  uint16 mult = Mult8x8(kSine8bit[angle & 0x7F], magnitude);
  uint16 hi = mult >> 8;
  uint16 lo = mult << 8;
  if ((angle & 0x80) != 0) {
    hi = -hi;
    lo = -lo;
  }
  return __PAIR32__(hi, lo);
}

static uint8 CalculateAngleFromXY_0(uint16 div) {  // 0xA0C112
  return (div >> 3) + 64;
}

static uint8 CalculateAngleFromXY_1(uint16 div) {  // 0xA0C120
  return 0x80 - (div >> 3);
}

static uint8 CalculateAngleFromXY_2(uint16 div) {  // 0xA0C132
  return div >> 3;
}

static uint8 CalculateAngleFromXY_3(uint16 div) {  // 0xA0C13C
  return 64 - (div >> 3);
}

static uint8 CalculateAngleFromXY_4(uint16 div) {  // 0xA0C14E
  return (div >> 3) + 0x80;
}

static uint8 CalculateAngleFromXY_5(uint16 div) {  // 0xA0C15C
  return -64 - (div >> 3);
}

static uint8 CalculateAngleFromXY_6(uint16 div) {  // 0xA0C16E
  return (div >> 3) - 64;
}

static uint8 CalculateAngleFromXY_7(uint16 div) {  // 0xA0C17C
  return 0 - (div >> 3);
}

typedef uint8 CalculateAngleFromXYFunc(uint16 div);

static CalculateAngleFromXYFunc *const kAngleFromXYFuncsLo[4] = {
  CalculateAngleFromXY_0, CalculateAngleFromXY_3, CalculateAngleFromXY_5, CalculateAngleFromXY_6,
};

static CalculateAngleFromXYFunc *const kAngleFromXYFuncsHi[4] = {
  CalculateAngleFromXY_1, CalculateAngleFromXY_2, CalculateAngleFromXY_4, CalculateAngleFromXY_7,
};

uint16 CheckIfEnemyTouchesSamus(uint16 k) {  // 0xA0ABE7
  EnemyData *E = gEnemyData(k);
  uint16 dx = abs16(samus_x_pos - E->x_pos);
  bool overlaps_x = dx < samus_x_radius;
  uint16 x_gap = dx - samus_x_radius;
  if (!overlaps_x && x_gap >= E->x_width)
    return 0;
  if ((int16)(samus_y_pos + 3 - E->y_pos) < 0) {
    uint16 dy = E->y_pos - (samus_y_pos + 3);
    uint16 y_gap = dy - samus_y_radius;
    if (dy < samus_y_radius || y_gap == E->y_height || y_gap < E->y_height)
      return -1;
  }
  return 0;
}

uint16 EnemyFunc_AC67(uint16 k) {  // 0xA0AC67
  EnemyData *E = gEnemyData(k);
  uint16 dx = abs16(samus_x_pos - E->x_pos);
  bool overlaps_x = dx < samus_x_radius;
  uint16 x_gap = dx - samus_x_radius;
  if (!overlaps_x && x_gap >= E->x_width && x_gap >= 8)
    return 0;
  uint16 dy = abs16(samus_y_pos - E->y_pos);
  bool overlaps_y = dy < samus_y_radius;
  uint16 y_gap = dy - samus_y_radius;
  return overlaps_y || y_gap < E->y_height ? -1 : 0;
}

PairU16 EnemyFunc_ACA8(Point16U base_pt, Point16U samus_pt) {  // 0xA0ACA8
  uint16 dx_signed = samus_pt.x - base_pt.x;
  uint16 dx = abs16(dx_signed);
  if (sign16(dx - 255)) {
    uint16 dy_signed = samus_pt.y - base_pt.y;
    uint16 dy = abs16(dy_signed);
    if (sign16(dy - 255)) {
      uint16 angle = CalculateAngleFromXY(dx, dy);
      uint16 dist = abs16(SineMult8bit(angle, dx)) + abs16(CosineMult8bit(angle, dy));
      uint16 samus_angle = CalculateAngleFromXY(dx_signed, dy_signed);
      return (PairU16){dist, samus_angle};
    }
  }
  return (PairU16){dx, 0};
}

uint16 CheckIfEnemyIsOnScreen(void) {  // 0xA0AD70
  EnemyData *E = gEnemyData(cur_enemy_index);
  return (int16)(E->x_pos - layer1_x_pos) < 0 || (int16)(layer1_x_pos + 256 - E->x_pos) < 0 ||
      (int16)(E->y_pos - layer1_y_pos) < 0 || (int16)(layer1_y_pos + 256 - E->y_pos) < 0;
}

uint16 EnemyFunc_ADA3(uint16 a) {  // 0xA0ADA3
  EnemyData *E = gEnemyData(cur_enemy_index);
  return (int16)(a + E->x_pos - layer1_x_pos) < 0 || (int16)(a + layer1_x_pos + 256 - E->x_pos) < 0 ||
      (int16)(a + E->y_pos - layer1_y_pos) < 0 || (int16)(a + layer1_y_pos + 256 - E->y_pos) < 0;
}

uint16 EnemyWithNormalSpritesIsOffScreen(void) {  // 0xA0ADE7
  EnemyData *E = gEnemyData(cur_enemy_index);
  return (int16)(E->x_width + E->x_pos - layer1_x_pos) < 0 || (int16)(E->x_width + layer1_x_pos + 256 - E->x_pos) < 0 ||
      (int16)(E->y_pos + 8 - layer1_y_pos) < 0 || (int16)(layer1_y_pos + 248 - E->y_pos) < 0;
}

uint16 DetermineDirectionOfSamusFromEnemy(void) {  // 0xA0AE29
  if (IsSamusWithinEnemy_Y(cur_enemy_index, 0x20)) {
    if ((GetSamusEnemyDelta_X(cur_enemy_index) & 0x8000) != 0)
      return 7;
    return 2;
  } else if (IsSamusWithinEnemy_X(cur_enemy_index, 0x20)) {
    if ((GetSamusEnemyDelta_Y(cur_enemy_index) & 0x8000) != 0)
      return 0;
    return 4;
  } else if ((GetSamusEnemyDelta_X(cur_enemy_index) & 0x8000) != 0) {
    if ((GetSamusEnemyDelta_Y(cur_enemy_index) & 0x8000) != 0)
      return 8;
    return 6;
  } else {
    if ((GetSamusEnemyDelta_Y(cur_enemy_index) & 0x8000) != 0)
      return 1;
    return 3;
  }
}

uint16 GetSamusEnemyDelta_Y(uint16 k) {  // 0xA0AEDD
  return samus_y_pos - gEnemyData(k)->y_pos;
}

uint16 GetSamusEnemyDelta_X(uint16 k) {  // 0xA0AEE5
  return samus_x_pos - gEnemyData(k)->x_pos;
}

uint16 IsSamusWithinEnemy_Y(uint16 k, uint16 a) {  // 0xA0AEED
  return (int16)(SubtractThenAbs16(gEnemyData(k)->y_pos, samus_y_pos) - a) < 0;
}

uint16 IsSamusWithinEnemy_X(uint16 k, uint16 a) {  // 0xA0AF0B
  return (int16)(SubtractThenAbs16(gEnemyData(k)->x_pos, samus_x_pos) - a) < 0;
}

uint16 SignExtend8(uint16 a) {  // 0xA0AFEA
  return (a & 0x80) ? a | 0xff00 : a;
}

uint16 Mult32(uint16 a) {  // 0xA0B002
  return 32 * a;
}

uint16 Abs16(uint16 a) {  // 0xA0B067
  return ((a & 0x8000) != 0) ? -a : a;
}

uint16 SubtractThenAbs16(uint16 k, uint16 j) {  // 0xA0B07D
  return Abs16(j - k);
}

uint16 CosineMult8bit(uint16 a, uint16 magnitude) {  // 0xA0B0B2
  return SineMult8bitInner(magnitude, (uint8)(a + 64)) >> 16;
}

uint16 SineMult8bit(uint16 a, uint16 magnitude) {  // 0xA0B0C6
  return SineMult8bitInner(magnitude, (uint8)(a + 0x80)) >> 16;
}

uint32 CosineMult8bitFull(uint16 a, uint16 magnitude) {
  return SineMult8bitInner(magnitude, (uint8)(a + 64));
}

uint32 SineMult8bitFull(uint16 a, uint16 magnitude) {
  return SineMult8bitInner(magnitude, (uint8)(a + 0x80));
}

Point32 ConvertAngleToXy(uint16 angle, uint16 magnitude) {  // 0xA0B643
  uint32 x = kEquationForQuarterCircle[(angle + 0x40) & 0x7F] * magnitude;
  uint32 y = kEquationForQuarterCircle[(angle + 0x80) & 0x7F] * magnitude;
  return (Point32){x, y};
}

void EnemyFunc_B691(uint16 angle, Point32 pt) {  // 0xA0B691
  EnemyData *E = gEnemyData(cur_enemy_index);
  if (((angle + 64) & 0x80) != 0)
    AddToHiLo(&E->x_pos, &E->x_subpos, -(int32)pt.x);
  else
    AddToHiLo(&E->x_pos, &E->x_subpos, pt.x);
  if (((angle + 128) & 0x80) != 0)
    AddToHiLo(&E->y_pos, &E->y_subpos, -(int32)pt.y);
  else
    AddToHiLo(&E->y_pos, &E->y_subpos, pt.y);
}

uint32 EnemyFunc_Divide(uint32 a, uint32 b) {  // 0xA0B761
  return a ? b / a : 0;
}

void EnemyFunc_B7A1(void) {  // 0xA0B7A1
  uint16 samus_y_delta = samus_y_pos - samus_prev_y_pos;
  if (!sign16(Abs16(samus_y_delta) - 12))
    samus_prev_y_pos = samus_y_pos + ((samus_y_delta & 0x8000) == 0 ? 12 : -12);
  uint16 samus_x_delta = samus_x_pos - samus_prev_x_pos;
  if (!sign16(Abs16(samus_x_delta) - 12))
    samus_prev_x_pos = samus_x_pos + ((samus_x_delta & 0x8000) == 0 ? 12 : -12);
}

uint8 CompareDistToSamus_X(uint16 k, uint16 a) {  // 0xA0BB9B
  EnemyData *E = gEnemyData(k);
  return abs16(samus_x_pos - E->x_pos) >= a;
}

uint8 CompareDistToSamus_Y(uint16 k, uint16 a) {  // 0xA0BBAD
  EnemyData *E = gEnemyData(k);
  return abs16(samus_y_pos - E->y_pos) >= a;
}

uint8 EnemyFunc_BBBF(uint16 k, int32 amt) {  // 0xA0BBBF
  EnemyData *E = gEnemyData(k);
  uint16 rows_left = (E->y_pos - E->y_height) & 0xFFF0;
  rows_left = (uint16)(E->y_height + E->y_pos - 1 - rows_left) >> 4;
  uint16 prod = Mult8x8((uint16)(E->y_pos - E->y_height) >> 4, room_width_in_blocks);
  uint16 next_x = (amt + __PAIR32__(E->x_pos, E->x_subpos)) >> 16;
  uint16 edge_x;
  if (sign32(amt))
    edge_x = next_x - E->x_width;
  else
    edge_x = E->x_width + next_x - 1;
  for (uint16 block = 2 * (prod + (edge_x >> 4)); (level_data[block >> 1] & 0x8000) == 0; block += room_width_in_blocks * 2) {
    if ((--rows_left & 0x8000) != 0)
      return 0;
  }
  return 1;
}

uint8 EnemyFunc_BC76(uint16 k, int32 amt) {  // 0xA0BC76
  EnemyData *E = gEnemyData(k);
  uint16 cols_left = (E->x_pos - E->x_width) & 0xFFF0;
  cols_left = (uint16)(E->x_width + E->x_pos - 1 - cols_left) >> 4;
  uint16 next_y = (amt + __PAIR32__(E->y_pos, E->y_subpos)) >> 16;
  uint16 edge_y;
  if (sign32(amt))
    edge_y = next_y - E->y_height;
  else
    edge_y = E->y_height + next_y - 1;
  uint16 prod = Mult8x8(edge_y >> 4, room_width_in_blocks);
  int16 block_x = (uint16)(E->x_pos - E->x_width) >> 4;
  for (int block = 2 * (prod + block_x); (level_data[block >> 1] & 0x8000) == 0; block += 2) {
    if ((--cols_left & 0x8000) != 0)
      return 0;
  }
  return 1;
}

uint8 EnemyFunc_BF8A(uint16 k, uint16 a, int32 amt) {  // 0xA0BF8A
  EnemyData *E = gEnemyData(k);
  uint16 cols_left = (E->x_pos - E->x_width) & 0xFFF0;
  cols_left = (uint16)(E->x_width + E->x_pos - 1 - cols_left) >> 4;
  uint16 edge_y;
  if (a & 1) {
    uint16 next_y = (amt + __PAIR32__(E->y_pos, E->y_subpos)) >> 16;
    edge_y = E->y_height + next_y - 1;
  } else {
    uint16 next_y = (__PAIR32__(E->y_pos, E->y_subpos) - amt) >> 16;
    edge_y = next_y - E->y_height;
  }
  uint16 prod = Mult8x8(edge_y >> 4, room_width_in_blocks);
  int16 block_x = (uint16)(E->x_pos - E->x_width) >> 4;
  for (int block = 2 * (prod + block_x); (level_data[block >> 1] & 0x8000) == 0; block += 2) {
    if ((--cols_left & 0x8000) != 0)
      return 0;
  }
  return 1;
}

uint16 CalculateAngleOfSamusFromEproj(uint16 k) {  // 0xA0C04E
  int i = k >> 1;
  return CalculateAngleFromXY(samus_x_pos - eproj_x_pos[i], samus_y_pos - eproj_y_pos[i]);
}

uint16 CalculateAngleOfSamusFromEnemy(uint16 k) {  // 0xA0C066
  EnemyData *E = gEnemyData(k);
  return CalculateAngleFromXY(samus_x_pos - E->x_pos, samus_y_pos - E->y_pos);
}

uint16 CalculateAngleOfEnemyXfromEnemyY(uint16 k, uint16 j) {  // 0xA0C096
  EnemyData *Ek = gEnemyData(k);
  EnemyData *Ej = gEnemyData(j);
  return CalculateAngleFromXY(Ek->x_pos - Ej->x_pos, Ek->y_pos - Ej->y_pos);
}

uint16 CalculateAngleFromXY(uint16 x_r18, uint16 y_r20) {  // 0xA0C0B1
  int idx = 0;
  if ((x_r18 & 0x8000) != 0) {
    idx += 2;
    x_r18 = -x_r18;
  }
  if ((y_r20 & 0x8000) != 0) {
    idx += 1;
    y_r20 = -y_r20;
  }
  if (y_r20 < x_r18) {
    uint16 div = SnesDivide(y_r20 << 8, x_r18);
    return kAngleFromXYFuncsLo[idx](div);
  }
  uint16 div = SnesDivide(x_r18 << 8, y_r20);
  return kAngleFromXYFuncsHi[idx](div);
}

uint8 IsEnemyLeavingScreen(uint16 k) {  // 0xA0C18E
  EnemyData *E = gEnemyData(k);
  int16 x_pos = E->x_pos;
  if (x_pos >= 0) {
    int16 x_with_radius = E->x_width + x_pos - layer1_x_pos;
    if (x_with_radius >= 0 && (int16)(x_with_radius - 256 - E->x_width) < 0)
      return 0;
  }
  return 1;
}
