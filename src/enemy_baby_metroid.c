// Enemy AI - Baby Metroid (Ridley cling) — peeled from Bank $A6
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"

enum {
  kSfx3_BabyMetroidCryCeres = 0x24,
  kBabyMetroidPaletteDest = 0x162,
  kBabyMetroidPaletteColors = 0xF,
};

static int BabyMetroid_DBCB_DoubleRetEx(uint16 a);

void DrawBabyMetroid_0(void) {  // 0xA6BF1A
  int spritemap = BabyMetroid_DBCB_DoubleRetEx(ADDR16_OF_RAM(*enemy_ram7800) + 6);
  if (spritemap < 0)
    return;
  Enemy_Ridley *E = Get_Ridley(0);

  sub_A6DC13(spritemap, E->ridley_var_42, E->ridley_var_44, 0);
}

uint16 BabyMetroid_Instr_2(uint16 k) {  // 0xA6BFC9
  if (!Get_Ridley(0)->ridley_var_46 && (random_number & 1) != 0)
    return BabyMetroid_Goto(k);
  QueueSfx3_Max6(kSfx3_BabyMetroidCryCeres);
  return k + 2;
}

uint16 BabyMetroid_Instr_3(uint16 k) {  // 0xA6BFE1
  uint16 pal_src = *(uint16 *)RomPtr_A6(k);
  WriteColorsToPalette(kBabyMetroidPaletteDest, 0xa6, pal_src, kBabyMetroidPaletteColors);
  return k + 2;
}

uint16 BabyMetroid_Instr_1(uint16 k) {  // 0xA6BFF2
  if (Get_Ridley(0)->ridley_var_46)
    return BabyMetroid_Goto(k);
  else
    return k + 2;
}

uint16 BabyMetroid_Goto(uint16 k) {  // 0xA6BFF8
  return *(uint16 *)RomPtr_A6(k);
}

uint16 CallBabyMetroidInstr(uint32 ea, uint16 k) {
  switch (ea) {
  case fnBabyMetroid_Instr_2: return BabyMetroid_Instr_2(k);
  case fnBabyMetroid_Instr_3: return BabyMetroid_Instr_3(k);
  case fnBabyMetroid_Instr_1: return BabyMetroid_Instr_1(k);
  case fnBabyMetroid_Instr_4: return BabyMetroid_Goto(k);
  default: return Unreachable();
  }
}

typedef struct BabyMetroidExecState {
  uint16 ip;
  uint16 timer;
} BabyMetroidExecState;

static int BabyMetroid_DBCB_DoubleRetEx(uint16 a) {
  BabyMetroidExecState *st = (BabyMetroidExecState *)&g_ram[a];

  if (!sign16(st->ip))
    return -1;
  uint16 ip = st->ip;
  const uint16 *instr = (uint16 *)RomPtr_A6(ip);
  if (!sign16(instr[0])) {
    if (st->timer != instr[0]) {
      st->timer++;
      return instr[1];
    }
    ip += 4;
    instr = (uint16 *)RomPtr_A6(ip);
  }
  while (sign16(instr[0])) {
    ip = CallBabyMetroidInstr(instr[0] | 0xA60000, ip + 2);
    instr = (uint16 *)RomPtr_A6(ip);
  }
  st->timer = 1;
  st->ip = ip;
  return instr[1];
}
