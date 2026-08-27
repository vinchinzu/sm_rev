// Enemy AI - Botwoon boss runtime — peeled from Bank $B3
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

enum {
  kMaridiaAreaIndex = 4,
  kBotwoonBossBit = 2,
  kEvent_ZebesEscape = 0xF,
  kBotwoonHealthPaletteLastStage = 16,
  kBotwoonPaletteBufferEnd = 512,
  kBotwoonEprojProp_Hidden = 0x2000,
  kBotwoonEprojFlag_Visible = 1,
  kBotwoonEprojFlag_Hidden = 2,
  kBotwoonBodyEprojLastOffset = 34,
  kBotwoonBodyEprojFirstOffset = 10,
  kBotwoonPathTerminator = 0xFF80,
  kPlm_ClearBotwoonWall = 0xb797,
  kPlm_CrumbleBotwoonWall = 0xb79b,
  kBotwoonMusic_Defeated = 3,
  kSfx2_BotwoonSpit = 0x7C,
  kSfx2_BotwoonExplosion = 0x24,
};

typedef struct BotwoonHoleHitbox {
  uint16 left;
  uint16 right;
  uint16 top;
  uint16 bottom;
} BotwoonHoleHitbox;

typedef struct BotwoonSpeed {
  uint16 speed;
  uint16 body_travel_time;
} BotwoonSpeed;

typedef struct BotwoonMovementChoice {
  uint16 path_ptr;
  int16 direction;
  uint16 hole_index;
  uint16 unused;
} BotwoonMovementChoice;
_Static_assert(sizeof(BotwoonMovementChoice) == 8, "Botwoon movement choice is 8 bytes");

typedef struct EscapeEtecoonInit {
  uint16 x_pos;
  uint16 y_pos;
  uint16 pre_instr;
  uint16 current_instruction;
  uint16 x_speed;
} EscapeEtecoonInit;

static const uint16 kZebIlists[4] = {
  addr_kBrinstarPipeBug_Ilist_87AB,
  addr_kBrinstarPipeBug_Ilist_87CF,
  addr_kBrinstarPipeBug_Ilist_87EB,
  addr_kBrinstarPipeBug_Ilist_880F,
};
static const uint16 kZebboIlists[4] = {
  addr_kBrinstarPipeBug_Ilist_8A1D,
  addr_kBrinstarPipeBug_Ilist_8A31,
  addr_kBrinstarPipeBug_Ilist_8A45,
  addr_kBrinstarPipeBug_Ilist_8A59,
};

static const uint16 kBotwoonMouthClosedIlists[16] = {
  addr_kBotwoon_Ilist_9381, addr_kBotwoon_Ilist_9379, addr_kBotwoon_Ilist_9371, addr_kBotwoon_Ilist_9369,
  addr_kBotwoon_Ilist_9361, addr_kBotwoon_Ilist_9351, addr_kBotwoon_Ilist_9349, addr_kBotwoon_Ilist_9341,
  addr_kBotwoon_Ilist_9389, addr_kBotwoon_Ilist_9389, addr_kBotwoon_Ilist_9389, addr_kBotwoon_Ilist_9389,
  addr_kBotwoon_Ilist_9389, addr_kBotwoon_Ilist_9389, addr_kBotwoon_Ilist_9389, addr_kBotwoon_Ilist_9389,
};
static const uint16 kBotwoonSpitIlists[8] = {
  addr_kBotwoon_Ilist_941F, addr_kBotwoon_Ilist_940F, addr_kBotwoon_Ilist_93FF, addr_kBotwoon_Ilist_93EF,
  addr_kBotwoon_Ilist_93DF, addr_kBotwoon_Ilist_93BF, addr_kBotwoon_Ilist_93AF, addr_kBotwoon_Ilist_939F,
};

static const BotwoonHoleHitbox kBotwoonHoleHitboxes[4] = {
  { 0x003c, 0x0044, 0x006c, 0x0074 },
  { 0x007c, 0x0084, 0x00ac, 0x00b4 },
  { 0x009c, 0x00a4, 0x005c, 0x0064 },
  { 0x00dc, 0x00e4, 0x008c, 0x0094 },
};
static const BotwoonSpeed kBotwoonSpeed[3] = {
  { 2, 0x18 },
  { 3, 0x10 },
  { 4, 0x0c },
};
static const uint16 kBotwoonSpitRandomMask[3] = { 0xffff, 0x00ff, 0x01ff };
static const uint16 kBotwoonHealthThresForPalChange[8] = {
  0x0bb8, 0x0a41, 0x08ca, 0x0753, 0x05dc, 0x0465, 0x02ee, 0x0177,
};
static const uint16 kBotwoonHealthBasedPalette[8][16] = {
  { 0x0, 0x27e9, 0x1a66, 0x1585, 0xca3, 0x3f9c, 0x2e97, 0x1d72, 0x108e, 0xa5f, 0x9db, 0x956, 0x8d2, 0x82c, 0x7fbd, 0xc05 },
  { 0x2003, 0x27eb, 0x1a88, 0x15a7, 0xcc4, 0x3b5c, 0x2e58, 0x1d53, 0x106f, 0xe3f, 0xdbb, 0xd36, 0xcd2, 0xc2c, 0x77bd, 0xc06 },
  { 0x2003, 0x23ed, 0x168a, 0x11c8, 0x8e5, 0x3afd, 0x2a38, 0x1d34, 0x1071, 0xdff, 0xd9b, 0xd36, 0xcb3, 0xc2d, 0x6fde, 0x807 },
  { 0x0, 0x23ef, 0x16ac, 0x11ea, 0x906, 0x36bd, 0x29f9, 0x1d15, 0x1052, 0x11df, 0x117b, 0x1116, 0x10b3, 0x102d, 0x67de, 0x808 },
  { 0x0, 0x23f2, 0x12cf, 0x11ec, 0x908, 0x327d, 0x25b9, 0x18d6, 0x1453, 0x11bf, 0x115c, 0x10f7, 0x1093, 0x102e, 0x5fde, 0x408 },
  { 0x0, 0x23f4, 0x12f1, 0x120e, 0x929, 0x2e3d, 0x257a, 0x18b7, 0x1434, 0x159f, 0x153c, 0x14d7, 0x1493, 0x142e, 0x57de, 0x409 },
  { 0x0, 0x1ff6, 0xef3, 0xe2f, 0x54a, 0x2dde, 0x215a, 0x1898, 0x1436, 0x155f, 0x151c, 0x14d7, 0x1474, 0x142f, 0x4fff, 0xa },
  { 0x0, 0x1ff8, 0xf15, 0xe51, 0x56b, 0x299e, 0x211b, 0x1879, 0x1417, 0x193f, 0x18fc, 0x18b7, 0x1874, 0x182f, 0x47ff, 0xb },
};
static const int16 kBotwoonSpitEprojParam[3] = { 2, 3, 4 };

static const BotwoonMovementChoice kBotwoonMovement[32] = {
  { 0xa05a, 0, 8, 0 }, { 0xa32a, 0, 0x10, 0 }, { 0xa6bc, 0, 0x18, 0 }, { 0xaa24, 0, 0, 0 },
  { 0xadfe, 0, 0, 0 }, { 0xb16a, 0, 0x10, 0 }, { 0xb556, 0, 0x18, 0 }, { 0xb956, 0, 8, 0 },
  { 0xbc86, 0, 0, 0 }, { 0xc086, 0, 8, 0 }, { 0xc290, 0, 0x18, 0 }, { 0xc690, 0, 0x10, 0 },
  { 0xc9cc, 0, 0, 0 }, { 0xcdcc, 0, 8, 0 }, { 0xd140, 0, 0x10, 0 }, { 0xd4a2, 0, 0x18, 0 },
  { 0xd880, 0, 8, 0 }, { 0xda02, 0, 0x10, 0 }, { 0xdb9c, 0, 0x18, 0 }, { 0xdb9c, 0, 0x18, 0 },
  { 0xda00, -1, 0, 0 }, { 0xdd42, 0, 0x10, 0 }, { 0xde7e, 0, 0x18, 0 }, { 0xde7e, 0, 0x18, 0 },
  { 0xdb9a, -1, 0, 0 }, { 0xde7c, -1, 8, 0 }, { 0xdfe0, 0, 0x18, 0 }, { 0xdfe0, 0, 0x18, 0 },
  { 0xdd40, -1, 0, 0 }, { 0xdfde, -1, 8, 0 }, { 0xe14e, -1, 0x10, 0 }, { 0xe14e, -1, 0x10, 0 },
};

static const EscapeEtecoonInit kEscapeEtecoonInit[3] = {
  { 0x80, 0xc8, FUNC16(EscapeEtecoon_E680), addr_kEscapeEtecoon_Ilist_E556, 0xfe00 },
  { 0xa0, 0xc8, FUNC16(EscapeEtecoon_E680), addr_kEscapeEtecoon_Ilist_E582, 0x0280 },
  { 0xe8, 0xc8, FUNC16(EscapeEtecoon_E670), addr_kEscapeEtecoon_Ilist_E5C6, 0 },
};


void UnusedSpinningTurtleEye_Init(void) {  // 0xB386FB
  EnemyData *E = gEnemyData(cur_enemy_index);
  E->properties |= kEnemyProps_ProcessInstructions;
  E->current_instruction = addr_kUnusedSpinningTurtleEye_Ilist_86A7;
}

void UnusedSpinningTurtleEye_Main(void) {  // 0xB3870E
  ;
}

void BrinstarPipeBug_Init(void) {  // 0xB3883B
  Enemy_PipeBug *PipeBug = Get_PipeBug(cur_enemy_index);
  PipeBug->pbg_var_B = PipeBug->base.x_pos;
  uint16 y_pos = PipeBug->base.y_pos;
  PipeBug->pbg_var_C = y_pos;
  PipeBug->pbg_var_00 = y_pos - 16;
  PipeBug->pbg_var_F = FUNC16(BrinstarPipeBug_PreInstr_1);
  PipeBug->pbg_var_D = 48;
  PipeBug->pbg_var_E = 0;
  PipeBug->pbg_var_01 = 0;
  PipeBug->base.current_instruction = addr_kBrinstarPipeBug_Ilist_87AB;
  if (PipeBug->pbg_parameter_1)
    PipeBug->base.current_instruction = addr_kBrinstarPipeBug_Ilist_8A1D;
}

void BrinstarPipeBug_Main(void) {  // 0xB3887A
  Enemy_PipeBug *PipeBug = Get_PipeBug(cur_enemy_index);
  EnemyRunPreInstr(PipeBug->pbg_var_F);
}

void BrinstarPipeBug_PreInstr_1(uint16 k) {  // 0xB38880
  if (!CheckIfEnemyIsOnScreen())
    Get_PipeBug(cur_enemy_index)->pbg_var_F = FUNC16(BrinstarPipeBug_PreInstr_2);
}

void BrinstarPipeBug_PreInstr_2(uint16 k) {  // 0xB38890
  int16 SamusEnemyDelta_Y;

  SamusEnemyDelta_Y = GetSamusEnemyDelta_Y(k);
  if (SamusEnemyDelta_Y < 0 && !sign16(SamusEnemyDelta_Y + 96)) {
    Enemy_PipeBug *PipeBug = Get_PipeBug(k);

    uint16 a = GetSamusEnemyDelta_X(k);
    PipeBug->pbg_var_A = PipeBug->pbg_var_A & 0x7fff | sign16(a);

    if (abs16(PipeBug->base.x_pos - samus_x_pos) < 0x40) {
      PipeBug->pbg_var_F = FUNC16(BrinstarPipeBug_PreInstr_3);
      PipeBug->base.properties &= ~kEnemyProps_Invisible;
      PipeBug->base.timer = 0;
      uint16 v4;
      if (sign16(PipeBug->pbg_var_A))
        v4 = 0;
      else
        v4 = 2;
      PipeBug->pbg_var_E = v4;
      BrinstarPipeBug_Func_1();
    }
  }
}

void BrinstarPipeBug_PreInstr_3(uint16 k) {  // 0xB388E3
  Enemy_PipeBug *PipeBug = Get_PipeBug(k);
  bool v2 = PipeBug->base.y_subpos-- != 0;
  PipeBug->base.y_pos += v2 - 2;
  if ((int16)(PipeBug->pbg_var_00 - PipeBug->base.y_pos) >= 0 && PipeBug->base.y_pos < samus_y_pos) {
    PipeBug->pbg_var_E |= 1;
    BrinstarPipeBug_Func_1();
    PipeBug->pbg_var_F = FUNC16(BrinstarPipeBug_PreInstr_4);
  }
}

void BrinstarPipeBug_PreInstr_4(uint16 k) {  // 0xB3891C
  Enemy_PipeBug *PipeBug = Get_PipeBug(k);
  if (!sign16(PipeBug->pbg_var_A)) {
    PipeBug->base.x_subpos = PipeBug->base.x_subpos;
    PipeBug->base.x_pos = PipeBug->base.x_pos + 2;
  } else {
    PipeBug->base.x_subpos = PipeBug->base.x_subpos;
    PipeBug->base.x_pos = PipeBug->base.x_pos - 2;
  }
  if (IsEnemyLeavingScreen(k) & 1) {
    PipeBug->base.x_pos = PipeBug->pbg_var_B;
    PipeBug->base.x_subpos = 0;
    uint16 pbg_var_C = PipeBug->pbg_var_C;
    PipeBug->base.y_pos = pbg_var_C;
    PipeBug->base.y_subpos = pbg_var_C;
    PipeBug->pbg_var_E = 0;
    BrinstarPipeBug_Func_1();
    PipeBug->base.properties |= kEnemyProps_Invisible;
    PipeBug->pbg_var_D = 48;
    PipeBug->pbg_var_F = FUNC16(BrinstarPipeBug_PreInstr_5);
  }
}

void BrinstarPipeBug_PreInstr_5(uint16 k) {  // 0xB3897E
  Enemy_PipeBug *E = Get_PipeBug(k);
  if (E->pbg_var_D-- == 1)
    E->pbg_var_F = FUNC16(BrinstarPipeBug_PreInstr_2);
}

void BrinstarPipeBug_Func_1(void) {  // 0xB3898B
  Enemy_PipeBug *PipeBug = Get_PipeBug(cur_enemy_index);
  uint16 pbg_var_E = PipeBug->pbg_var_E;
  if (pbg_var_E != PipeBug->pbg_var_01) {
    PipeBug->pbg_var_01 = pbg_var_E;
    PipeBug->base.current_instruction = PipeBug->pbg_parameter_1 ?
      kZebboIlists[pbg_var_E] : kZebIlists[pbg_var_E];
    PipeBug->base.instruction_timer = 1;
    PipeBug->base.timer = 0;
  }
}

void NorfairPipeBug_Init(void) {  // 0xB38B61
  Enemy_PipeBug *PipeBug = Get_PipeBug(cur_enemy_index);
  PipeBug->pbg_var_D = PipeBug->base.x_pos;
  PipeBug->pbg_var_E = PipeBug->base.y_pos;
  PipeBug->base.properties |= kEnemyProps_Invisible;
  PipeBug->pbg_var_B = 8 * HIBYTE(PipeBug->pbg_parameter_2);
  PipeBug->pbg_var_C = 0;
  PipeBug->base.instruction_timer = 1;
  PipeBug->base.timer = 0;
  PipeBug->base.current_instruction = addr_kNorfairPipeBug_Ilist_8AE1;
  PipeBug->pbg_var_A = FUNC16(NorfairPipeBug_Func_1);
}

void CallBugInstr(uint32 ea, uint16 k) {
  switch (ea) {
  case fnNorfairPipeBug_Func_1: NorfairPipeBug_Func_1(); return;  // 0xb38bcd
  case fnNorfairPipeBug_Func_2: NorfairPipeBug_Func_2(); return;  // 0xb38bff
  case fnNorfairPipeBug_Func_4: NorfairPipeBug_Func_4(); return;  // 0xb38ca6
  case fnNorfairPipeBug_Func_5: NorfairPipeBug_Func_5(k); return;  // 0xb38cff
  case fnNorfairPipeBug_Func_6: NorfairPipeBug_Func_6(); return;  // 0xb38d0c
  case fnNorfairPipeBug_Func_7: NorfairPipeBug_Func_7(); return;  // 0xb38d4e
  case fnNorfairPipeBug_Func_8: NorfairPipeBug_Func_8(); return;  // 0xb38d90
  case fnNorfairPipeBug_Func_9: NorfairPipeBug_Func_9(); return;  // 0xb38dd2
  case fnNorfairPipeBug_Func_10: NorfairPipeBug_Func_10(); return;  // 0xb38e14
  case fnNorfairPipeBug_Func_11: NorfairPipeBug_Func_11(); return;  // 0xb38e35
  case fnNorfairPipeBug_Func_12: NorfairPipeBug_Func_12(); return;  // 0xb38e5a
  case fnBrinstarYellowPipeBug_Func_1: BrinstarYellowPipeBug_Func_1(); return;  // 0xb38fb5
  case fnBrinstarYellowPipeBug_Func_2: BrinstarYellowPipeBug_Func_2(cur_enemy_index); return;  // 0xb38ff5
  case fnBrinstarYellowPipeBug_Func_3: BrinstarYellowPipeBug_Func_3(); return;  // 0xb39028
  case fnBrinstarYellowPipeBug_Func_5: BrinstarYellowPipeBug_Func_5(); return;  // 0xb390bd
  case fnBrinstarYellowPipeBug_Func_7: BrinstarYellowPipeBug_Func_7(); return;  // 0xb3915a
  case fnBrinstarYellowPipeBug_Func_8: BrinstarYellowPipeBug_Func_8(); return;  // 0xb391d8
  default: Unreachable();
  }
}

void NorfairPipeBug_Main(void) {  // 0xB38B9E
  Enemy_PipeBug *PipeBug = Get_PipeBug(cur_enemy_index);
  CallBugInstr(PipeBug->pbg_var_A | 0xB30000, cur_enemy_index);
  NorfairPipeBug_8BA8();
}

void NorfairPipeBug_8BA8(void) {  // 0xB38BA8
  if (CheckIfEnemyIsOnScreen()) {
    Enemy_PipeBug *PipeBug = Get_PipeBug(cur_enemy_index);
    PipeBug->base.properties |= kEnemyProps_Invisible;
    PipeBug->pbg_var_A = FUNC16(NorfairPipeBug_Func_1);
    PipeBug->base.x_pos = PipeBug->pbg_var_D;
    PipeBug->base.y_pos = PipeBug->pbg_var_E;
  }
}

void NorfairPipeBug_Func_1(void) {  // 0xB38BCD
  if (LOBYTE(Get_PipeBug(cur_enemy_index)->pbg_parameter_2)
      && Get_PipeBug(cur_enemy_index + 64)->pbg_var_A == FUNC16(NorfairPipeBug_Func_1)
      && Get_PipeBug(cur_enemy_index + 128)->pbg_var_A == FUNC16(NorfairPipeBug_Func_1)
      && Get_PipeBug(cur_enemy_index + 192)->pbg_var_A == FUNC16(NorfairPipeBug_Func_1)
      && Get_PipeBug(cur_enemy_index + 256)->pbg_var_A == FUNC16(NorfairPipeBug_Func_1)) {
    Get_PipeBug(cur_enemy_index)->pbg_var_A = FUNC16(NorfairPipeBug_Func_2);
  }
}

void NorfairPipeBug_Func_2(void) {  // 0xB38BFF
  int k = cur_enemy_index;
  Enemy_PipeBug *PipeBug = Get_PipeBug(cur_enemy_index);
  if (IsSamusWithinEnemy_X(cur_enemy_index, LOBYTE(PipeBug->pbg_parameter_2))
      && (int16)(PipeBug->base.y_pos - samus_y_pos) >= 0) {
    ++PipeBug->pbg_var_C;
    PipeBug->base.instruction_timer = 1;
    PipeBug->base.timer = 0;
    if (!sign16(GetSamusEnemyDelta_X(k))) {
      Get_PipeBug(k)->base.current_instruction = addr_kNorfairPipeBug_Ilist_8B21;
      Get_PipeBug(k + 64)->base.current_instruction = addr_kNorfairPipeBug_Ilist_8B21;
      Get_PipeBug(k + 128)->base.current_instruction = addr_kNorfairPipeBug_Ilist_8B21;
      Get_PipeBug(k + 192)->base.current_instruction = addr_kNorfairPipeBug_Ilist_8B21;
      Get_PipeBug(k + 256)->base.current_instruction = addr_kNorfairPipeBug_Ilist_8B21;
    } else {
      PipeBug->base.current_instruction = addr_kNorfairPipeBug_Ilist_8AE1;
      Get_PipeBug(k + 64)->base.current_instruction = addr_kNorfairPipeBug_Ilist_8AE1;
      Get_PipeBug(k + 128)->base.current_instruction = addr_kNorfairPipeBug_Ilist_8AE1;
      Get_PipeBug(k + 192)->base.current_instruction = addr_kNorfairPipeBug_Ilist_8AE1;
      Get_PipeBug(k + 256)->base.current_instruction = addr_kNorfairPipeBug_Ilist_8AE1;
    }
    NorfairPipeBug_Func_3(k);
  }
}

void NorfairPipeBug_Func_3(uint16 k) {  // 0xB38C52
  Get_PipeBug(k)->pbg_var_A = FUNC16(NorfairPipeBug_Func_4);
  Get_PipeBug(k + 64)->pbg_var_A = FUNC16(NorfairPipeBug_Func_4);
  Get_PipeBug(k + 128)->pbg_var_A = FUNC16(NorfairPipeBug_Func_4);
  Get_PipeBug(k + 192)->pbg_var_A = FUNC16(NorfairPipeBug_Func_4);
  Get_PipeBug(k + 256)->pbg_var_A = FUNC16(NorfairPipeBug_Func_4);
  Get_PipeBug(k)->pbg_var_02 = 104;
  Get_PipeBug(k + 64)->pbg_var_02 = 96;
  Get_PipeBug(k + 128)->pbg_var_02 = 88;
  Get_PipeBug(k + 192)->pbg_var_02 = 112;
  Get_PipeBug(k + 256)->pbg_var_02 = 120;
  Get_PipeBug(k)->pbg_var_F = FUNC16(NorfairPipeBug_Func_5);
  Get_PipeBug(k + 64)->pbg_var_F = FUNC16(NorfairPipeBug_Func_6);
  Get_PipeBug(k + 128)->pbg_var_F = FUNC16(NorfairPipeBug_Func_7);
  Get_PipeBug(k + 192)->pbg_var_F = FUNC16(NorfairPipeBug_Func_8);
  Get_PipeBug(k + 256)->pbg_var_F = FUNC16(NorfairPipeBug_Func_9);
}

void NorfairPipeBug_Func_4(void) {  // 0xB38CA6
  Enemy_PipeBug *E = Get_PipeBug(cur_enemy_index);
  E->base.properties &= ~kEnemyProps_Invisible;
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(kCommonEnemySpeeds_Linear[66], kCommonEnemySpeeds_Linear[67]));
  if ((int16)(E->base.y_pos - samus_y_pos) < 0) {
    E->pbg_var_A = E->pbg_var_F;
    E->pbg_var_01 = E->base.y_pos;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
    if (!sign16(GetSamusEnemyDelta_X(cur_enemy_index)))
      E->base.current_instruction = addr_kNorfairPipeBug_Ilist_8B21;
    else
      E->base.current_instruction = addr_kNorfairPipeBug_Ilist_8AE1;
  }
}

void NorfairPipeBug_Func_5(uint16 k) {  // 0xB38CFF
  Enemy_PipeBug *E = Get_PipeBug(cur_enemy_index);
  ++E->pbg_var_C;
  E->pbg_var_A = FUNC16(NorfairPipeBug_Func_12);
}

void NorfairPipeBug_Func_6(void) {  // 0xB38D0C
  Enemy_PipeBug *E = Get_PipeBug(cur_enemy_index);
  ++E->pbg_var_C;
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(kCommonEnemySpeeds_Linear[66], kCommonEnemySpeeds_Linear[67]));
  if ((int16)(E->base.y_pos + 16 - E->pbg_var_01) < 0) {
    E->base.y_pos = E->pbg_var_01 - 16;
    E->base.y_subpos = 0;
    E->pbg_var_A = FUNC16(NorfairPipeBug_Func_12);
  }
}

void NorfairPipeBug_Func_7(void) {  // 0xB38D4E
  Enemy_PipeBug *E = Get_PipeBug(cur_enemy_index);
  ++E->pbg_var_C;
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(kCommonEnemySpeeds_Linear[66], kCommonEnemySpeeds_Linear[67]));
  if ((int16)(E->base.y_pos + 32 - E->pbg_var_01) < 0) {
    E->base.y_pos = E->pbg_var_01 - 32;
    E->base.y_subpos = 0;
    E->pbg_var_A = FUNC16(NorfairPipeBug_Func_12);
  }
}

void NorfairPipeBug_Func_8(void) {  // 0xB38D90
  Enemy_PipeBug *E = Get_PipeBug(cur_enemy_index);
  ++E->pbg_var_C;
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(kCommonEnemySpeeds_Linear[64], kCommonEnemySpeeds_Linear[65]));
  if ((int16)(E->base.y_pos - 16 - E->pbg_var_01) >= 0) {
    E->base.y_pos = E->pbg_var_01 + 16;
    E->base.y_subpos = 0;
    E->pbg_var_A = FUNC16(NorfairPipeBug_Func_12);
  }
}

void NorfairPipeBug_Func_9(void) {  // 0xB38DD2
  Enemy_PipeBug *E = Get_PipeBug(cur_enemy_index);
  ++E->pbg_var_C;
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(kCommonEnemySpeeds_Linear[64], kCommonEnemySpeeds_Linear[65]));
  if ((int16)(E->base.y_pos - 32 - E->pbg_var_01) >= 0) {
    E->base.y_pos = E->pbg_var_01 + 32;
    E->base.y_subpos = 0;
    E->pbg_var_A = FUNC16(NorfairPipeBug_Func_12);
  }
}

void NorfairPipeBug_Func_10(void) {  // 0xB38E14
  Enemy_PipeBug *E = Get_PipeBug(cur_enemy_index);
  int v2 = E->pbg_var_B >> 1;
  AddToHiLo(&E->base.x_pos, &E->base.x_subpos, __PAIR32__(kCommonEnemySpeeds_Linear[v2 + 2], kCommonEnemySpeeds_Linear[v2 + 3]));
}

void NorfairPipeBug_Func_11(void) {  // 0xB38E35
  Enemy_PipeBug *E = Get_PipeBug(cur_enemy_index);
  int v2 = E->pbg_var_B >> 1;
  AddToHiLo(&E->base.x_pos, &E->base.x_subpos, __PAIR32__(kCommonEnemySpeeds_Linear[v2], kCommonEnemySpeeds_Linear[v2 + 1]));
}

void sub_B38E56(void) {  // 0xB38E56
  ;
}

void NorfairPipeBug_Func_12(void) {  // 0xB38E5A
  Enemy_PipeBug *E = Get_PipeBug(cur_enemy_index);
  if ((int16)(++E->pbg_var_C - E->pbg_var_02) >= 0) {
    E->pbg_var_C = 0;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
    E->pbg_var_A = FUNC16(NorfairPipeBug_Func_10);
    E->base.current_instruction = addr_kNorfairPipeBug_Ilist_8B05;
    if (!sign16(GetSamusEnemyDelta_X(cur_enemy_index))) {
      E->pbg_var_A = FUNC16(NorfairPipeBug_Func_11);
      E->base.current_instruction = addr_kNorfairPipeBug_Ilist_8B45;
    }
  }
}

void BrinstarYellowPipeBug_Init(void) {  // 0xB38F4C

  Enemy_PipeBug *E = Get_PipeBug(cur_enemy_index);
  E->pbg_var_07 = E->base.x_pos;
  E->pbg_var_08 = E->base.y_pos;
  E->base.x_subpos = 0;
  E->base.y_subpos = 0;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->base.current_instruction = addr_kBrinstarYellowPipeBug_Ilist_8EFC;
  if (!E->pbg_parameter_1)
    E->base.current_instruction = addr_kBrinstarYellowPipeBug_Ilist_8F24;
  int v1 = (8 * E->pbg_parameter_2) >> 1;
  E->pbg_var_01 = kCommonEnemySpeeds_Linear[v1];
  E->pbg_var_00 = kCommonEnemySpeeds_Linear[v1 + 1];
  E->pbg_var_03 = kCommonEnemySpeeds_Linear[v1 + 2];
  E->pbg_var_02 = kCommonEnemySpeeds_Linear[v1 + 3];
  E->pbg_var_A = FUNC16(BrinstarYellowPipeBug_Func_1);
  E->pbg_var_06 = 0;
}

void BrinstarYellowPipeBug_Main(void) {  // 0xB38FAE

  Enemy_PipeBug *E = Get_PipeBug(cur_enemy_index);
  CallBugInstr(E->pbg_var_A | 0xB30000, cur_enemy_index);
}

void BrinstarYellowPipeBug_Func_1(void) {  // 0xB38FB5
  Enemy_PipeBug *E = Get_PipeBug(cur_enemy_index);
  int16 dx = GetSamusEnemyDelta_X(cur_enemy_index);
  bool in_x_range = E->pbg_parameter_1
      ? dx < 0 && !sign16(dx + 192)
      : dx >= 0 && sign16(dx - 192);
  if (in_x_range && IsSamusWithinEnemy_Y(cur_enemy_index, 0x30)) {
    E->base.properties &= ~kEnemyProps_Invisible;
    E->pbg_var_20 = 24;
    E->pbg_var_A = FUNC16(BrinstarYellowPipeBug_Func_2);
  }
}

void BrinstarYellowPipeBug_Func_2(uint16 k) {  // 0xB38FF5

  Enemy_PipeBug *E = Get_PipeBug(k);
  uint16 v2 = E->pbg_var_20 - 1;
  E->pbg_var_20 = v2;
  if (!v2) {
    E->base.instruction_timer = 1;
    E->base.timer = 0;
    E->base.current_instruction = addr_kBrinstarYellowPipeBug_Ilist_8EFC;
    E->pbg_var_A = FUNC16(BrinstarYellowPipeBug_Func_3);
    if (!E->pbg_parameter_1) {
      E->base.current_instruction = addr_kBrinstarYellowPipeBug_Ilist_8F24;
      E->pbg_var_A = FUNC16(BrinstarYellowPipeBug_Func_5);
    }
  }
}

void BrinstarYellowPipeBug_Func_3(void) {  // 0xB39028
  Enemy_PipeBug *E = Get_PipeBug(cur_enemy_index);
  BrinstarYellowPipeBug_Func_4(cur_enemy_index);
  if (CheckIfEnemyIsOnScreen()) {
    E->base.x_pos = E->pbg_var_07;
    E->base.y_pos = E->pbg_var_08;
    E->base.x_subpos = 0;
    E->base.y_subpos = 0;
    E->pbg_var_A = FUNC16(BrinstarYellowPipeBug_Func_1);
    E->pbg_var_06 = 0;
    E->base.properties |= kEnemyProps_Invisible;
  } else {
    if (!E->pbg_var_06) {
      if (sign16(E->base.x_pos - samus_x_pos - 48)) {
        E->pbg_var_A = FUNC16(BrinstarYellowPipeBug_Func_7);
        E->pbg_var_C = 0;
        E->pbg_var_D = 0;
        E->pbg_var_E = 40;
        E->pbg_var_F = 1;
        E->pbg_var_04 = E->base.x_pos;
        E->base.instruction_timer = 1;
        E->base.timer = 0;
        E->base.current_instruction = addr_kBrinstarYellowPipeBug_Ilist_8F10;
      }
    }
  }
}

void BrinstarYellowPipeBug_Func_4(uint16 k) {  // 0xB390A1
  Enemy_PipeBug *E = Get_PipeBug(k);
  AddToHiLo(&E->base.x_pos, &E->base.x_subpos, __PAIR32__(E->pbg_var_03, E->pbg_var_02));
}

void BrinstarYellowPipeBug_Func_5(void) {  // 0xB390BD
  Enemy_PipeBug *E = Get_PipeBug(cur_enemy_index);

  BrinstarYellowPipeBug_Func_6(cur_enemy_index);
  if (CheckIfEnemyIsOnScreen()) {
    E->base.x_pos = E->pbg_var_07;
    E->base.y_pos = E->pbg_var_08;
    E->base.x_subpos = 0;
    E->base.y_subpos = 0;
    E->pbg_var_A = FUNC16(BrinstarYellowPipeBug_Func_1);
    E->pbg_var_06 = 0;
    E->base.properties |= kEnemyProps_Invisible;
  } else {
    if (!E->pbg_var_06) {
      if (sign16(samus_x_pos - E->base.x_pos - 48)) {
        E->pbg_var_A = FUNC16(BrinstarYellowPipeBug_Func_8);
        E->pbg_var_C = 0;
        E->pbg_var_D = 0;
        E->pbg_var_E = 40;
        E->pbg_var_F = 1;
        E->pbg_var_04 = E->base.x_pos;
        E->base.instruction_timer = 1;
        E->base.timer = 0;
        E->base.current_instruction = addr_kBrinstarYellowPipeBug_Ilist_8F38;
      }
    }
  }
}

void BrinstarYellowPipeBug_Func_6(uint16 k) {  // 0xB3913A
  Enemy_PipeBug *E = Get_PipeBug(k);
  AddToHiLo(&E->base.x_pos, &E->base.x_subpos, __PAIR32__(E->pbg_var_01, E->pbg_var_00));
}

void BrinstarYellowPipeBug_Func_7(void) {  // 0xB3915A
  Enemy_PipeBug *E = Get_PipeBug(cur_enemy_index);
  if (CheckIfEnemyIsOnScreen()) {
    E->base.x_pos = E->pbg_var_07;
    E->base.y_pos = E->pbg_var_08;
    E->base.x_subpos = 0;
    E->base.y_subpos = 0;
    E->pbg_var_A = FUNC16(BrinstarYellowPipeBug_Func_1);
    E->pbg_var_06 = 0;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
    E->base.current_instruction = addr_kBrinstarYellowPipeBug_Ilist_8EFC;
    E->base.properties |= kEnemyProps_Invisible;
  } else {
    BrinstarYellowPipeBug_Func_4(cur_enemy_index);
    if (E->pbg_var_F) {
      BrinstarYellowPipeBug_Func_10(cur_enemy_index);
    } else {
      BrinstarYellowPipeBug_Func_9(cur_enemy_index);
      if ((int16)(E->base.y_pos - E->pbg_var_08) < 0) {
        E->pbg_var_06 = 1;
        E->pbg_var_F = 1;
        E->pbg_var_A = FUNC16(BrinstarYellowPipeBug_Func_3);
        E->base.instruction_timer = 1;
        E->base.timer = 0;
        E->base.current_instruction = addr_kBrinstarYellowPipeBug_Ilist_8EFC;
      }
    }
  }
}

void BrinstarYellowPipeBug_Func_8(void) {  // 0xB391D8
  Enemy_PipeBug *E = Get_PipeBug(cur_enemy_index);
  if (CheckIfEnemyIsOnScreen()) {
    E->base.x_pos = E->pbg_var_07;
    E->base.y_pos = E->pbg_var_08;
    E->base.x_subpos = 0;
    E->base.y_subpos = 0;
    E->pbg_var_A = FUNC16(BrinstarYellowPipeBug_Func_1);
    E->pbg_var_06 = 0;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
    E->base.current_instruction = addr_kBrinstarYellowPipeBug_Ilist_8F24;
    E->base.properties |= kEnemyProps_Invisible;
  } else {
    BrinstarYellowPipeBug_Func_6(cur_enemy_index);
    if (E->pbg_var_F) {
      BrinstarYellowPipeBug_Func_10(cur_enemy_index);
    } else {
      BrinstarYellowPipeBug_Func_9(cur_enemy_index);
      if ((int16)(E->pbg_var_08 - E->base.y_pos) >= 0) {
        E->pbg_var_06 = 1;
        E->pbg_var_F = 1;
        E->base.instruction_timer = 1;
        E->base.timer = 0;
        E->base.current_instruction = addr_kBrinstarYellowPipeBug_Ilist_8F24;
        E->pbg_var_A = FUNC16(BrinstarYellowPipeBug_Func_5);
      }
    }
  }
}

void BrinstarYellowPipeBug_Func_9(uint16 k) {  // 0xB39256
  Enemy_PipeBug *E = Get_PipeBug(k);
  ++E->pbg_var_E;
  int v3 = (8 * E->pbg_var_E) >> 1;
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(kCommonEnemySpeeds_Quadratic[v3 + 3], kCommonEnemySpeeds_Quadratic[v3 + 2]));
}

void BrinstarYellowPipeBug_Func_10(uint16 k) {  // 0xB3927A
  Enemy_PipeBug *E = Get_PipeBug(k);
  if (sign16(--E->pbg_var_E)) {
    E->pbg_var_E = 0;
    E->pbg_var_F = 0;
  } else {
    int v4 = (8 * E->pbg_var_E) >> 1;
    AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(kCommonEnemySpeeds_Quadratic[v4 + 1], kCommonEnemySpeeds_Quadratic[v4]));
  }
}

const uint16 *Botwoon_Instr_1(uint16 k, const uint16 *jp) {  // 0xB394C7
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  E->base.x_width = 8;
  E->base.y_height = 16;
  return jp;
}

const uint16 *Botwoon_Instr_2(uint16 k, const uint16 *jp) {  // 0xB394D7
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  E->base.x_width = 12;
  E->base.y_height = 12;
  return jp;
}

const uint16 *Botwoon_Instr_3(uint16 k, const uint16 *jp) {  // 0xB394E7
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  E->base.x_width = 16;
  E->base.y_height = 8;
  return jp;
}

const uint16 *Botwoon_Instr_4(uint16 k, const uint16 *jp) {  // 0xB394F7
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  E->base.x_width = 12;
  E->base.y_height = 12;
  return jp;
}

const uint16 *Botwoon_Instr_5(uint16 k, const uint16 *jp) {  // 0xB39507
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  E->base.x_width = 8;
  E->base.y_height = 16;
  return jp;
}

const uint16 *Botwoon_Instr_6(uint16 k, const uint16 *jp) {  // 0xB39517
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  E->base.x_width = 8;
  E->base.y_height = 16;
  return jp;
}

const uint16 *Botwoon_Instr_7(uint16 k, const uint16 *jp) {  // 0xB39527
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  E->base.x_width = 12;
  E->base.y_height = 12;
  return jp;
}

const uint16 *Botwoon_Instr_8(uint16 k, const uint16 *jp) {  // 0xB39537
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  E->base.x_width = 16;
  E->base.y_height = 8;
  return jp;
}

const uint16 *Botwoon_Instr_9(uint16 k, const uint16 *jp) {  // 0xB39547
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  E->base.x_width = 12;
  E->base.y_height = 12;
  return jp;
}

const uint16 *Botwoon_Instr_10(uint16 k, const uint16 *jp) {  // 0xB39557

  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  E->base.x_width = 8;
  E->base.y_height = 16;
  return jp;
}

const uint16 *Botwoon_Instr_SetSpitting(uint16 k, const uint16 *jp) {  // 0xB39567
  Get_Botwoon(cur_enemy_index)->botwoon_var_5A = 1;
  return jp;
}

const uint16 *Botwoon_Instr_QueueSpitSfx(uint16 k, const uint16 *jp) {  // 0xB39572
  QueueSfx2_Max6(kSfx2_BotwoonSpit);
  return jp;
}

void Botwoon_QueueExplosionSfx(void) {  // 0xB3957B
  QueueSfx2_Max6(kSfx2_BotwoonExplosion);
}

void Botwoon_Init(void) {  // 0xB39583
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  if ((boss_bits_for_area[kMaridiaAreaIndex] & kBotwoonBossBit) != 0) {
    SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x0f, 0x04, kPlm_ClearBotwoonWall });
    *(uint16 *)scrolls = 257;
    E->base.current_instruction = addr_kBotwoon_Ilist_9389;
    E->base.properties |= kEnemyProps_Deleted;
  } else {
    uint16 v3 = 24;
    E->botwoon_var_A = 24;
    bool v5;
    do {
      SpawnEprojWithGfx(v3, cur_enemy_index, addr_kEproj_BotwoonsBody);
      v5 = (int16)(--E->botwoon_var_A - 1) < 0;
      --E->botwoon_var_A;
    } while (!v5);
    E->base.current_instruction = addr_kBotwoon_Ilist_9389;
    E->botwoon_var_3B = addr_kBotwoon_Ilist_9389;
    E->botwoon_var_D = FUNC16(Botwoon_Func_6);
    E->botwoon_var_E = FUNC16(Botwoon_Func_19);
    E->botwoon_var_F = FUNC16(Botwoon_Func_26);
    E->botwoon_var_20 = 256;
    E->botwoon_var_38 = kBotwoonSpeed[0].speed;
    E->botwoon_var_C = kBotwoonSpeed[0].body_travel_time;
    E->botwoon_var_33 = 1;
    E->botwoon_var_34 = 1;
    E->botwoon_var_59 = 1;
    E->botwoon_var_36 = -1;
    E->botwoon_var_37 = 0;
    E->base.properties |= kEnemyProps_Intangible;
    uint16 health = E->base.health;
    E->botwoon_var_5B = health;
    health >>= 1;
    E->botwoon_var_5C = health;
    E->botwoon_var_5D = health >> 1;
    uint16 x_pos = E->base.x_pos;
    E->botwoon_var_50 = x_pos;
    E->botwoon_var_52 = x_pos;
    E->botwoon_var_54 = x_pos;
    E->botwoon_var_56 = x_pos;
    uint16 y_pos = E->base.y_pos;
    E->botwoon_var_51 = y_pos;
    E->botwoon_var_53 = y_pos;
    E->botwoon_var_55 = y_pos;
    E->botwoon_var_57 = y_pos;
    E->botwoon_var_4E = (E->base.palette_index >> 4) + 256;
  }
}

void CallBotwoonFuncVarD(uint32 ea, uint16 k) {
  switch (ea) {
  case fnBotwoon_Func_6: Botwoon_Func_6(); return;
  case fnBotwoon_Func_7: Botwoon_Func_7(); return;
  case fnBotwoon_Func_12: Botwoon_Func_12(); return;
  case fnBotwoon_Func_13: Botwoon_Func_13(); return;
  case fnBotwoon_Func_14: Botwoon_Func_14(); return;
  case fnBotwoon_Func_15: Botwoon_Func_15(); return;
  case fnBotwoon_Func_16: Botwoon_Func_16(); return;
  case fnBotwoon_Func_18: Botwoon_Func_18(k); return;
  default: Unreachable();
  }
}
void CallBotwoonFuncVarE(uint32 ea) {
  switch (ea) {
  case fnBotwoon_Func_19: Botwoon_Func_19(); return;
  case fnBotwoon_Func_32: Botwoon_Func_32(); return;
  case fnBotwoon_Func_33: Botwoon_Func_33(); return;
  default: Unreachable();
  }
}

void Botwoon_Main(void) {  // 0xB39668
  Botwoon_Func_3();
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  CallBotwoonFuncVarD(E->botwoon_var_D | 0xB30000, cur_enemy_index);
  Botwoon_HealthBasedPalHandling();
}

void Botwoon_Func_1(uint16 k) {  // 0xB3967B
  Enemy_Botwoon *E = Get_Botwoon(k);
  if ((kBotwoonSpitRandomMask[E->botwoon_var_3F] & NextRandom()) == 0)
    E->botwoon_var_2E = 1;
}

void Botwoon_Func_2(void) {  // 0xB39696
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  if (!E->botwoon_var_2F && E->botwoon_var_2E && !E->botwoon_var_33 && !E->botwoon_var_21) {
    E->botwoon_var_F = FUNC16(Botwoon_Func_27);
    E->botwoon_var_21 = 24;
  }
  E->botwoon_var_2E = 0;
}

void Botwoon_Func_3(void) {  // 0xB396C6
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  if (E->botwoon_var_2F) {
    if (E->botwoon_var_3E) {
      E->botwoon_var_30 = 1;
      E->botwoon_var_D = FUNC16(Botwoon_Func_14);
      E->botwoon_var_22 = 240;
      E->botwoon_var_2F = 0;
      E->botwoon_var_3E = 0;
    }
  }
}

void Botwoon_Func_4(void) {  // 0xB396F5
  Enemy_Botwoon *E = Get_Botwoon(0);
  E->base.properties |= kEnemyProps_Intangible;
}

void Botwoon_Func_5(void) {  // 0xB396FF
  uint16 eproj_offset = kBotwoonBodyEprojLastOffset;
  do {
    eproj_properties[eproj_offset >> 1] |= kBotwoonEprojProp_Hidden;
    eproj_flags[eproj_offset >> 1] = kBotwoonEprojFlag_Hidden;
    eproj_offset -= 2;
  } while ((int16)(eproj_offset - kBotwoonBodyEprojFirstOffset) >= 0);
}

void Botwoon_HealthBasedPalHandling(void) {  // 0xB3982B
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  uint16 pal_stage = E->botwoon_var_4F;
  if (pal_stage != kBotwoonHealthPaletteLastStage
      && (int16)(E->base.health - kBotwoonHealthThresForPalChange[pal_stage >> 1]) < 0) {
    uint16 dst = E->botwoon_var_4E;
    const uint16 *src = kBotwoonHealthBasedPalette[pal_stage >> 1];
    do {
      palette_buffer[dst >> 1] = *src++;
      dst += 2;
    } while (dst != kBotwoonPaletteBufferEnd);
    E->botwoon_var_4F += 2;
  }
}

void Botwoon_Func_6(void) {  // 0xB39878
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  uint16 v1 = E->botwoon_var_20 - 1;
  E->botwoon_var_20 = v1;
  if (!v1)
    E->botwoon_var_D = FUNC16(Botwoon_Func_7);
}

void Botwoon_Func_7(void) {  // 0xB3989D
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  if (E->botwoon_var_3C) {
    E->botwoon_var_3C = 0;
    uint8 r18 = 0;
    if (!E->botwoon_var_33 && !E->botwoon_var_59 && !E->botwoon_var_3F) {
      r18 = NextRandom() & 0xE;
    }
    E->botwoon_var_59 = 0;
    if (!r18 || r18 == 2 || r18 == 4) {
      Botwoon_Func_8();
    } else {
      if (r18 != 6 && r18 != 8 && r18 != 10 && r18 != 12 && r18 != 14) {
        Unreachable();
      }
      Botwoon_Func_9();
    }
  } else {
    CallBotwoonFuncVarE(E->botwoon_var_E | 0xB30000);
    Botwoon_Func_22();
    Botwoon_Func_23();
    EnemyRunPreInstr(E->botwoon_var_F);
    Botwoon_Func_25();
    Botwoon_Func_24();
    Botwoon_Func_31(cur_enemy_index);
  }
}

void Botwoon_Func_8(void) {  // 0xB398EC
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  E->botwoon_var_D = FUNC16(Botwoon_Func_12);
  E->botwoon_var_E = FUNC16(Botwoon_Func_32);
  E->botwoon_var_21 = 0;
  E->botwoon_var_F = FUNC16(Botwoon_Func_26);
  E->botwoon_var_4B = 0;
  Botwoon_Func_10(cur_enemy_index);
}

void Botwoon_Func_9(void) {  // 0xB39913
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  E->botwoon_var_D = FUNC16(Botwoon_Func_13);
  E->botwoon_var_F = FUNC16(Botwoon_Func_27);
  E->botwoon_var_21 = 48;
  E->base.properties &= ~kEnemyProps_Intangible;
}

void Botwoon_Func_10(uint16 k) {  // 0xB39933
  Botwoon_Func_11(k);
  Enemy_Botwoon *E = Get_Botwoon(k);
  uint16 r18 = E->botwoon_var_33 ? 128 : 0;
  E->botwoon_var_40 = (NextRandom() & 0x18) + r18 + 4 * E->botwoon_var_37;
}

void Botwoon_Func_11(uint16 k) {  // 0xB3995D
  int16 health;

  Enemy_Botwoon *E = Get_Botwoon(k);
  if (!E->botwoon_var_33) {
    E->botwoon_var_3F = 0;
    health = E->base.health;
    if (health > 0 && (int16)(health - E->botwoon_var_5C) < 0) {
      if ((int16)(health - E->botwoon_var_5D) < 0)
        E->botwoon_var_3F = 2;
      else
        E->botwoon_var_3F = 1;
      E->botwoon_var_38 = kBotwoonSpeed[E->botwoon_var_3F].speed;
      E->botwoon_var_C = kBotwoonSpeed[E->botwoon_var_3F].body_travel_time;
    }
  }
}

void Botwoon_Func_12(void) {  // 0xB399A4
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  if (E->botwoon_var_3C) {
    E->botwoon_var_3C = 0;
    E->botwoon_var_D = FUNC16(Botwoon_Func_7);
    E->botwoon_var_E = FUNC16(Botwoon_Func_19);
    if (E->botwoon_var_33)
      E->botwoon_var_35 = 0;
  } else {
    CallBotwoonFuncVarE(E->botwoon_var_E | 0xB30000);
    Botwoon_Func_22();
    Botwoon_Func_23();
    EnemyRunPreInstr(E->botwoon_var_F);
    Botwoon_Func_25();
    Botwoon_Func_24();
  }
}

void Botwoon_Func_13(void) {  // 0xB399E4
  uint16 v3;

  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  if (E->botwoon_var_21) {
    E->botwoon_var_2E = 0;
    EnemyRunPreInstr(E->botwoon_var_F);
  } else {
    E->botwoon_var_3C = 0;
    E->botwoon_var_D = FUNC16(Botwoon_Func_12);
    E->botwoon_var_E = FUNC16(Botwoon_Func_32);
    E->botwoon_var_F = FUNC16(Botwoon_Func_26);
    if (E->botwoon_var_2F || (v3 = NextRandom() & 1, E->botwoon_var_33 = v3, (E->botwoon_var_34 = v3) == 0)) {
      E->botwoon_var_4B = 0;
    } else {
      E->botwoon_var_35 = 0;
      E->botwoon_var_36 = -1;
    }
    Botwoon_Func_10(cur_enemy_index);
  }
}

void Botwoon_Func_14(void) {  // 0xB39A46
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  uint16 v1 = E->botwoon_var_22 + 1;
  E->botwoon_var_22 = v1;
  if (!sign16(v1 - 256))
    E->botwoon_var_D = FUNC16(Botwoon_Func_15);
}

void Botwoon_Func_15(void) {  // 0xB39A5E
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  int v2 = (8 * HIBYTE(E->botwoon_var_28)) >> 1;
  AddToHiLo(&E->base.y_pos, &E->base.y_subpos, __PAIR32__(kCommonEnemySpeeds_Quadratic[v2 + 1], kCommonEnemySpeeds_Quadratic[v2]));
  if (sign16(E->base.y_pos - 200)) {
    E->botwoon_var_28 += 192;
  } else {
    E->base.y_pos = 200;
    E->botwoon_var_D = FUNC16(Botwoon_Func_16);
    eproj_spawn_pt = (Point16U){ E->base.x_pos, E->base.y_pos };
    SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 0x1D);
    Botwoon_QueueExplosionSfx();
    E->base.properties |= kEnemyProps_Invisible | kEnemyProps_Intangible;
  }
}

void Botwoon_Func_16(void) {  // 0xB39ACA
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  if (E->botwoon_var_5F) {
    E->botwoon_var_D = FUNC16(Botwoon_Func_18);
    Botwoon_Func_17(cur_enemy_index);
  }
}

void Botwoon_Func_17(uint16 k) {  // 0xB39ADD
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x0f, 0x04, kPlm_CrumbleBotwoonWall });
  Enemy_ItemDrop_Botwoon(k);
  Enemy_Botwoon *E = Get_Botwoon(k);
  E->botwoon_var_23 = 0;
  E->botwoon_var_24 = 0;
  E->botwoon_var_25 = 0;
}

void Botwoon_Func_18(uint16 k) {  // 0xB39AF9
  Enemy_Botwoon *E = Get_Botwoon(k);
  if (sign16(E->botwoon_var_23 - 192)) {
    if (!sign16(E->botwoon_var_23 - 64)) {
      int16 v2 = E->botwoon_var_24 - 1;
      E->botwoon_var_24 = v2;
      if (v2 < 0) {
        E->botwoon_var_24 = 12;
        uint16 x = (NextRandom() & 0x1F) + 232;
        uint16 y = E->botwoon_var_23 + (NextRandom() & 0x1F) - 8;
        CreateSpriteAtPos(x, y, 29, 2560);
        Botwoon_QueueExplosionSfx();
      }
      int16 v3 = E->botwoon_var_25 - 1;
      E->botwoon_var_25 = v3;
      if (v3 < 0) {
        E->botwoon_var_25 = 4;
        int n = 2;
        do {
          uint16 x = (NextRandom() & 0x3F) + 224;
          uint16 y = Get_Botwoon(k)->botwoon_var_23 + ((NextRandom() & 0x1F) - 8);
          CreateSpriteAtPos(x, y, 9, 2560);
        } while (--n);
      }
    }
    ++E->botwoon_var_23;
  } else {
    E->base.properties |= kEnemyProps_Deleted;
    SetBossBitForCurArea(kBotwoonBossBit);
    QueueMusic_Delayed8(kBotwoonMusic_Defeated);
  }
}

void Botwoon_Func_19(void) {  // 0xB39BB7
  Point16U pt = Botwoon_Func_20(cur_enemy_index);
  uint16 v1 = CalculateAngleFromXY(pt.x, pt.y);
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  E->botwoon_var_39 = v1;
  E->botwoon_var_3A = (uint8)(64 - v1);
  if (E->botwoon_var_33 == E->botwoon_var_34) {
    Botwoon_Func_21();
  } else {
    E->botwoon_var_34 = E->botwoon_var_33;
    E->botwoon_var_3C = 1;
  }
}

Point16U Botwoon_Func_20(uint16 k) {  // 0xB39BF8
  Enemy_Botwoon *E = Get_Botwoon(k);
  const BotwoonHoleHitbox *hole = &kBotwoonHoleHitboxes[E->botwoon_var_37 / 8];
  uint16 v3 = hole->left + 4 - E->base.x_pos;
  uint16 r18 = v3;
  if (sign16(v3 - 256)) {
    if (sign16(v3 + 256))
      r18 = -255;
  } else {
    r18 = 255;
  }
  uint16 v4 = hole->top + 4 - E->base.y_pos;
  uint16 r20 = v4;
  if (sign16(v4 - 256)) {
    if (sign16(v4 + 256))
      r20 = -255;
  } else {
    r20 = 255;
  }
  return (Point16U) { r18, r20 };
}

void Botwoon_Func_21(void) {  // 0xB39C48
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  Point32 pt = ConvertAngleToXy(E->botwoon_var_3A, E->botwoon_var_38);
  EnemyFunc_B691(E->botwoon_var_3A, pt);
}

void Botwoon_Func_22(void) {  // 0xB39C7B
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  uint16 botwoon_var_B = E->botwoon_var_B;
  *(uint16 *)((uint8 *)&kraid_unk9000 + botwoon_var_B) = E->base.x_pos;
  *(uint16 *)((uint8 *)&g_word_7E9002 + botwoon_var_B) = E->base.y_pos;
}

void Botwoon_Func_23(void) {  // 0xB39C90
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  uint16 r18 = (E->botwoon_var_B - E->botwoon_var_C) & 0x3FF;
  int n = 24;
  do {
    Enemy_Botwoon *ET = Get_Botwoon(n);
    uint16 botwoo_var_00 = ET->botwoon_var_00;
    if (E->botwoon_var_36 == r18) {
      uint16 v2 = n;
      bool v4 = ET->botwoon_var_10 == 1;
      ET->botwoon_var_10 ^= 1;
      if (v4) {
        eproj_properties[botwoo_var_00 >> 1] &= ~kBotwoonEprojProp_Hidden;
        eproj_flags[botwoo_var_00 >> 1] = kBotwoonEprojFlag_Visible;
      } else {
        eproj_properties[botwoo_var_00 >> 1] |= kBotwoonEprojProp_Hidden;
        eproj_flags[botwoo_var_00 >> 1] = kBotwoonEprojFlag_Hidden;
      }
      if (!v2) {
        E->botwoon_var_35 = 0;
        E->botwoon_var_3E = 0;
        E->botwoon_var_36 = -1;
        if (!E->botwoon_var_33)
          E->botwoon_var_3E = 1;
      }
    }
    int v7 = botwoo_var_00 >> 1;
    eproj_x_pos[v7] = *(uint16 *)((uint8 *)&kraid_unk9000 + r18);
    eproj_y_pos[v7] = *(uint16 *)((uint8 *)&g_word_7E9002 + r18);
    r18 = (r18 - E->botwoon_var_C) & 0x3FF;
    n -= 2;
  } while (!sign16(n));
}

void Botwoon_Func_24(void) {  // 0xB39D3C
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  E->botwoon_var_B = (E->botwoon_var_B + 4) & 0x3FF;
}

void Botwoon_Func_25(void) {  // 0xB39D4D
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  int n = 24;
  do {
    uint16 r24 = 0;
    Enemy_Botwoon *ERR = Get_Botwoon(n);
    uint16 botwoo_var_00 = ERR->botwoon_var_00;
    if (ERR->botwoon_var_10)
      r24 = 256;
    uint16 x, y;
    if (n == 24) {
      int v4 = botwoo_var_00 >> 1;
      x = E->base.x_pos - eproj_x_pos[v4];
      y = E->base.y_pos - eproj_y_pos[v4];
    } else {
      if (!n)
        r24 += 512;
      int v2 = botwoo_var_00 >> 1;
      x = eproj_x_pos[v2 + 1] - eproj_x_pos[v2];
      y = eproj_y_pos[v2 + 1] - eproj_y_pos[v2];
    }
    eproj_E[botwoo_var_00 >> 1] = 2 * ((uint16)(r24 + CalculateAngleFromXY(x, y)) >> 5);
    n -= 2;
  } while (n >= 0);
}

void Botwoon_Func_26(uint16 k) {  // 0xB39DC0
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  uint16 x = E->base.x_pos - E->botwoon_var_56;
  uint16 y = E->base.y_pos - E->botwoon_var_57;

  uint16 r22 = CalculateAngleFromXY(x, y);
  if (x || y) {  // bugfix: real game overwrote r18
    uint16 v1;
    if (E->botwoon_var_33) {
      E->base.layer = 7;
      E->base.properties |= kEnemyProps_Intangible;
      v1 = addr_kBotwoon_Ilist_9389;
    } else {
      E->base.layer = 2;
      E->base.properties &= ~kEnemyProps_Intangible;
      // Added hysteresis: Compute a weighted average
      E->botwoon_var_45 = (uint8)(E->botwoon_var_45 + (int8)(r22 - E->botwoon_var_45) * 3 / 4);
      v1 = kBotwoonMouthClosedIlists[E->botwoon_var_45 >> 5];
    }
    if (v1 != E->botwoon_var_3B) {
      E->base.current_instruction = v1;
      E->botwoon_var_3B = v1;
      E->base.instruction_timer = 1;
      E->base.timer = 0;
    }
  }
  E->botwoon_var_56 = E->botwoon_var_54;
  E->botwoon_var_57 = E->botwoon_var_55;
  E->botwoon_var_54 = E->botwoon_var_52;
  E->botwoon_var_55 = E->botwoon_var_53;
  E->botwoon_var_52 = E->botwoon_var_50;
  E->botwoon_var_53 = E->botwoon_var_51;
  E->botwoon_var_50 = E->base.x_pos;
  E->botwoon_var_51 = E->base.y_pos;
}

void Botwoon_Func_27(uint16 k) {  // 0xB39E7D
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  E->base.layer = 2;
  uint16 v2 = CalculateAngleOfSamusFromEnemy(cur_enemy_index);
  E->botwoon_var_3D = v2;
  uint16 v3 = kBotwoonSpitIlists[(uint8)(v2 + 16) >> 5];
  E->base.current_instruction = v3;
  E->botwoon_var_3B = v3;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->botwoon_var_3D = (uint8)(64 - E->botwoon_var_3D);
  E->botwoon_var_F = FUNC16(Botwoon_Func_28);
  if (E->botwoon_var_D != FUNC16(Botwoon_Func_13))
    E->botwoon_var_F = FUNC16(Botwoon_Func_29);
  EnemyRunPreInstr(E->botwoon_var_F);
}

void Botwoon_Func_28(uint16 k) {  // 0xB39EE0
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  if (E->botwoon_var_5A) {
    eproj_init_param_3 = E->botwoon_var_3D - 32;
    int n = 5;
    uint16 varE32 = kBotwoonSpitEprojParam[E->botwoon_var_3F];
    do {
      SpawnEprojWithGfx(varE32, cur_enemy_index, addr_kEproj_BotwoonsSpit);
      eproj_init_param_3 += 16;
    } while (--n);
    E->botwoon_var_5A = 0;
    E->botwoon_var_F = FUNC16(Botwoon_Func_30);
  }
}

void Botwoon_Func_29(uint16 k) {  // 0xB39F34
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  eproj_init_param_3 = E->botwoon_var_3D - 16;
  int n = 3;
  uint16 varE32 = kBotwoonSpitEprojParam[E->botwoon_var_3F];
  do {
    SpawnEprojWithGfx(varE32, cur_enemy_index, addr_kEproj_BotwoonsSpit);
    eproj_init_param_3 += 16;
  } while (--n);
  Get_Botwoon(cur_enemy_index)->botwoon_var_F = FUNC16(Botwoon_Func_30);
}

void Botwoon_Func_30(uint16 k) {  // 0xB39F7A
  Enemy_Botwoon *E = Get_Botwoon(k);
  int16 v2 = E->botwoon_var_21 - 1;
  E->botwoon_var_21 = v2;
  if (v2 < 0) {
    E->botwoon_var_21 = 0;
    E->botwoon_var_F = FUNC16(Botwoon_Func_26);
  }
}

void Botwoon_Func_31(uint16 k) {  // 0xB39F93
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);

  if (!Get_Botwoon(k)->botwoon_var_35) {
    int hole = 3;
    do {
      const BotwoonHoleHitbox *box = &kBotwoonHoleHitboxes[hole];
      if ((int16)(E->base.x_pos - box->left) >= 0
          && (int16)(E->base.x_pos - box->right) < 0
          && (int16)(E->base.y_pos - box->top) >= 0
          && (int16)(E->base.y_pos - box->bottom) < 0) {
        E->botwoon_var_35 = 1;
        E->botwoon_var_33 ^= 1;
        E->botwoon_var_36 = E->botwoon_var_B;
        --E->botwoon_var_26;
        break;
      }
      E->botwoon_var_35 = 0;
    } while (--hole >= 0);
  }
}

void Botwoon_Touch(void) {  // 0xB39FFF
  NormalEnemyTouchAiSkipDeathAnim_CurEnemy();
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  if (!E->base.health) {
    E->botwoon_var_2F = 1;
    Botwoon_Func_4();
  }
}

void Botwoon_Shot(void) {  // 0xB3A016
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  E->botwoon_var_4C = E->base.health;
  NormalEnemyShotAiSkipDeathAnim_CurEnemy();
  if (!E->base.health) {
    E->botwoon_var_2F = 1;
    Botwoon_Func_4();
  }
}

void Botwoon_Powerbomb(void) {  // 0xB3A041
  NormalEnemyPowerBombAiSkipDeathAnim_CurEnemy();
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  if (!E->base.health) {
    E->botwoon_var_2F = 1;
    Botwoon_Func_4();
  }
}

void Botwoon_Func_32(void) {  // 0xB3E250
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  E->botwoon_var_E = FUNC16(Botwoon_Func_33);
  E->botwoon_var_3C = 0;
  const BotwoonMovementChoice *choice = &kBotwoonMovement[E->botwoon_var_40 / 8];
  E->botwoon_var_42 = choice->path_ptr;
  E->botwoon_var_37 = choice->hole_index;
  E->botwoon_var_44 = choice->direction;
  if (choice->direction < 0)
    E->botwoon_var_42 -= 4;
  Botwoon_Func_33();
}

void Botwoon_Func_33(void) {  // 0xB3E28C
  uint16 x = 0, y = 0;
  Enemy_Botwoon *E = Get_Botwoon(cur_enemy_index);
  int n = E->botwoon_var_38;
  int step = sign16(E->botwoon_var_44) ? -2 : 2;
  do {
    const uint8 *v6 = RomPtr_B3(E->botwoon_var_42);
    uint16 v7 = SignExtend8(*v6), v8;
    if (v7 == kBotwoonPathTerminator || (x += v7, v8 = SignExtend8(v6[1]), v8 == kBotwoonPathTerminator)) {
      E->botwoon_var_41 = 0;
      E->botwoon_var_3C = 1;
      return;
    }
    y += v8;
    E->botwoon_var_42 += step;
  } while (--n);
  if (sign16(E->botwoon_var_44))
    x = -x, y = -y;
  E->base.x_pos += x;
  E->base.y_pos += y;
}

const uint16 *EscapeEtecoon_Instr_1(uint16 k, const uint16 *jp) {  // 0xB3E545
  if (lava_acid_y_pos >= 0xCE)
    return jp + 1;
  else
    return INSTR_RETURN_ADDR(jp[0]);
}

const uint16 *EscapeEtecoon_Instr_2(uint16 k, const uint16 *jp) {  // 0xB3E610
  Enemy_EscapeEtecoon *E = Get_EscapeEtecoon(k);
  E->base.x_pos += jp[0];
  return jp + 1;
}

void EscapeEtecoon_Main(void) {  // 0xB3E655
  Enemy_EscapeEtecoon *E = Get_EscapeEtecoon(cur_enemy_index);
  EnemyRunPreInstr(E->een_var_F);
}

void EscapeEtecoon_E65C(uint16 k) {  // 0xB3E65C
  Enemy_EscapeEtecoon *E = Get_EscapeEtecoon(k);
  AddToHiLo(&E->base.x_pos, &E->base.x_subpos, 0x38000);
}

void EscapeEtecoon_E670(uint16 k) {  // 0xB3E670
  if (CheckEventHappened(kEvent_ZebesEscape))
    Get_EscapeEtecoon(k)->base.current_instruction = addr_kEscapeEtecoon_Ilist_E5DA;
}

void EscapeEtecoon_E680(uint16 k) {  // 0xB3E680
  Enemy_EscapeEtecoon *E = Get_EscapeEtecoon(k);
  if (Enemy_MoveRight_IgnoreSlopes(k, INT16_SHL8(E->een_var_A))) {
    E->base.instruction_timer = 1;
    bool v3 = sign16(-E->een_var_A);
    E->een_var_A = -E->een_var_A;
    E->base.current_instruction = v3 ?
      addr_kEscapeEtecoon_Ilist_E556 : addr_kEscapeEtecoon_Ilist_E582;
    if (CheckEventHappened(kEvent_ZebesEscape))
      E->base.current_instruction = addr_kEscapeEtecoon_Ilist_E5AE;
  }
  Enemy_MoveDown(k, INT16_SHL16(1));
}

void EscapeEtecoon_Init(void) {  // 0xB3E6CB
  Enemy_EscapeEtecoon *E = Get_EscapeEtecoon(cur_enemy_index);
  if (CheckEventHappened(kEvent_ZebesEscape)) {
    E->base.properties |= kEnemyProps_Deleted;
  } else {
    E->base.properties |= kEnemyProps_ProcessInstructions | kEnemyProps_Intangible | kEnemyProps_SolidToSamus;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
    E->base.palette_index = 0;
    const EscapeEtecoonInit *init = &kEscapeEtecoonInit[E->een_parameter_1 >> 1];
    E->base.x_pos = init->x_pos;
    E->base.y_pos = init->y_pos;
    E->een_var_F = init->pre_instr;
    E->base.current_instruction = init->current_instruction;
    E->een_var_A = init->x_speed;
  }
}

const uint16 *EscapeDachora_Instr_2(uint16 k, const uint16 *jp) {  // 0xB3EAA8
  if (lava_acid_y_pos >= 0xCE)
    return jp + 1;
  else
    return INSTR_RETURN_ADDR(jp[0]);
}

const uint16 *EscapeDachora_Instr_3(uint16 k, const uint16 *jp) {  // 0xB3EAB8
  if (CheckEventHappened(kEvent_ZebesEscape))
    return INSTR_RETURN_ADDR(jp[0]);
  else
    return jp + 1;
}

const uint16 *EscapeDachora_Instr_1(uint16 k, const uint16 *jp) {  // 0xB3EAC9
  Enemy_EscapeDachora *E = Get_EscapeDachora(cur_enemy_index);
  E->base.x_pos -= 6;
  return jp;
}

const uint16 *EscapeDachora_Instr_4(uint16 k, const uint16 *jp) {  // 0xB3EAD7
  Enemy_EscapeDachora *E = Get_EscapeDachora(cur_enemy_index);
  E->base.x_pos += 6;
  return jp;
}

void EscapeDachora_Init(void) {  // 0xB3EAE5
  Enemy_EscapeDachora *E = Get_EscapeDachora(cur_enemy_index);
  if (CheckEventHappened(kEvent_ZebesEscape)) {
    E->base.properties |= kEnemyProps_Deleted;
  } else {
    E->base.properties |= kEnemyProps_ProcessInstructions;
    E->base.spritemap_pointer = addr_kSpritemap_Nothing_B3;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
    E->base.current_instruction = addr_kEscapeDachora_Ilist_E964;
  }
}
