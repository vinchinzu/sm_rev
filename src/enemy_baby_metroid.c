// Enemy AI - Baby Metroid (Ridley cling) — peeled from Bank $A6
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"

static int BabyMetroid_DBCB_DoubleRetEx(uint16 a);

void DrawBabyMetroid_0(void) {  // 0xA6BF1A
  int v0 = BabyMetroid_DBCB_DoubleRetEx(ADDR16_OF_RAM(*enemy_ram7800) + 6);
  if (v0 < 0)
    return;
  Enemy_Ridley *E = Get_Ridley(0);

  sub_A6DC13(v0, E->ridley_var_42, E->ridley_var_44, 0);
}

uint16 BabyMetroid_Instr_2(uint16 k) {  // 0xA6BFC9
  if (!Get_Ridley(0)->ridley_var_46 && (random_number & 1) != 0)
    return BabyMetroid_Goto(k);
  QueueSfx3_Max6(0x24);
  return k + 2;
}

uint16 BabyMetroid_Instr_3(uint16 k) {  // 0xA6BFE1
  uint16 v1 = *(uint16 *)RomPtr_A6(k);
  WriteColorsToPalette(0x162, 0xa6, v1, 0xF);
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

  if ((st->ip & 0x8000) == 0)
    return -1;  // double ret
  uint16 v2 = st->ip;
  const uint16 *v3 = (uint16 *)RomPtr_A6(v2);
  if (sign16(v3[0]))
    goto LABEL_7;
  if (st->timer != v3[0]) {
    st->timer++;
    return v3[1];
  }
  v2 += 4;
  for (; ; ) {
    v3 = (uint16 *)RomPtr_A6(v2);
    if (!sign16(v3[0]))
      break;
LABEL_7:
    v2 = CallBabyMetroidInstr(v3[0] | 0xA60000, v2 + 2);
  }
  st->timer = 1;
  st->ip = v2;
  return v3[1];
}
