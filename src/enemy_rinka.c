// Rinka enemy runtime extracted from sm_a2.c.
#include "ida_types.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "variables.h"
#include "enemy_ai_canon.h"

enum {
  kRinkaSpawnSlotCount = 11,
  kRinkaFireSpeed = 0x120,
};

typedef struct RinkaSpawnSlot {
  uint16 x_pos;
  uint16 y_pos;
  uint16 slot;
} RinkaSpawnSlot;
_Static_assert(sizeof(RinkaSpawnSlot) == 6, "Rinka spawn slot is 6 bytes");

static const RinkaSpawnSlot kRinkaSpawnSlots[kRinkaSpawnSlotCount] = {
  { 0x03e7, 0x0026, 0x0002 },
  { 0x03e7, 0x00a6, 0x0004 },
  { 0x0337, 0x0036, 0x0006 },
  { 0x0337, 0x00a6, 0x0008 },
  { 0x0277, 0x001c, 0x000a },
  { 0x0277, 0x00b6, 0x000c },
  { 0x01b7, 0x0036, 0x000e },
  { 0x01b7, 0x00a6, 0x0010 },
  { 0x00f7, 0x001c, 0x0012 },
  { 0x00f7, 0x00b6, 0x0014 },
  { 0x0080, 0x00a8, 0x0016 },
};

static uint16 *RinkaSpawnAvailabilityFlag(uint16 slot) {
  return &gRam8000_Default(slot)[31].var_3F;
}

static bool RinkaSpawnSlotBusy(uint16 slot) {
  return (*RinkaSpawnAvailabilityFlag(slot) & 1) != 0;
}

static void RinkaOccupySpawnSlot(uint16 k, EnemySpawnData *ES, const RinkaSpawnSlot *spawn) {
  Enemy_Rinka *E = Get_Rinka(k);
  ES->x_pos = spawn->x_pos;
  E->base.x_pos = spawn->x_pos;
  ES->y_pos = spawn->y_pos;
  E->base.y_pos = spawn->y_pos;
  *RinkaSpawnAvailabilityFlag(spawn->slot) = (uint16)-1;
  E->rinka_var_D = spawn->slot;
}

void Rinka_Init(void) {  // 0xA2B602
  Enemy_Rinka *E = Get_Rinka(cur_enemy_index);
  if (E->rinka_parameter_1) {
    Rinka_1(cur_enemy_index);
    E->base.properties = (E->base.properties | kEnemyProps_ProcessInstructions | kEnemyProps_ProcessedOffscreen | kEnemyProps_Intangible) & ~kEnemyProps_RespawnIfKilled;
  } else {
    E->base.properties = (E->base.properties | kEnemyProps_RespawnIfKilled | kEnemyProps_ProcessInstructions | kEnemyProps_Intangible) & ~kEnemyProps_ProcessedOffscreen;
  }
  E->base.palette_index = 1024;
  Rinka_Init3(cur_enemy_index);
}

void Rinka_Init2(uint16 k) {  // 0xA2B63E
  Enemy_Rinka *E = Get_Rinka(k);
  if (E->rinka_parameter_1)
    Rinka_1(k);
  EnemySpawnData *ES = gEnemySpawnData(k);
  E->base.x_pos = ES->x_pos;
  E->base.y_pos = ES->y_pos;
  Rinka_Init3(k);
}

void Rinka_Init3(uint16 k) {  // 0xA2B654
  Enemy_Rinka *E = Get_Rinka(k);
  E->rinka_var_A = FUNC16(Rinka_5);
  E->rinka_var_F = 26;
  E->rinka_var_B = 0;
  E->rinka_var_C = 0;
  if (E->rinka_parameter_1) {
    if (Get_Rinka(0)->rinka_var_1D) {
      E->base.properties |= kEnemyProps_Deleted;
    } else {
      E->base.current_instruction = addr_kRinka_Ilist_BA0C;
      E->base.instruction_timer = 1;
      E->base.timer = 0;
    }
  } else {
    E->base.current_instruction = addr_kRinka_Ilist_B9E0;
    E->base.instruction_timer = 1;
    E->base.timer = 0;
  }
}

void Rinka_1(uint16 k) {  // 0xA2B69B
  EnemySpawnData *ES = gEnemySpawnData(k);
  uint16 previous_slot;
  if (!Rinka_10(ES->x_pos, ES->y_pos) &&
      (previous_slot = Rinka_2(k), !RinkaSpawnSlotBusy(previous_slot))) {
    Get_Rinka(k)->rinka_var_D = previous_slot;
    *RinkaSpawnAvailabilityFlag(previous_slot) = (uint16)-1;
    return;
  }

  for (int i = 0; i < kRinkaSpawnSlotCount; i++) {
    const RinkaSpawnSlot *spawn = &kRinkaSpawnSlots[i];
    if (!Rinka_10(spawn->x_pos, spawn->y_pos) && !RinkaSpawnSlotBusy(spawn->slot)) {
      RinkaOccupySpawnSlot(k, ES, spawn);
      return;
    }
  }

  for (int i = 0; i < kRinkaSpawnSlotCount; i++) {
    const RinkaSpawnSlot *spawn = &kRinkaSpawnSlots[i];
    if (!RinkaSpawnSlotBusy(spawn->slot)) {
      RinkaOccupySpawnSlot(k, ES, spawn);
      return;
    }
  }
}

uint16 Rinka_2(uint16 k) {  // 0xA2B79D
  EnemySpawnData *ES = gEnemySpawnData(k);
  for (int i = 0; i < kRinkaSpawnSlotCount; i++) {
    if (kRinkaSpawnSlots[i].x_pos == ES->x_pos && kRinkaSpawnSlots[i].y_pos == ES->y_pos)
      return kRinkaSpawnSlots[i].slot;
  }
  return kRinkaSpawnSlots[0].slot;
}

void CallRinkaFunc(uint32 ea, uint16 k) {
  switch (ea) {
  case fnRinka_3: Rinka_3(k); return;
  case fnRinka_4: Rinka_4(k); return;
  case fnRinka_5: Rinka_5(k); return;
  case fnRinka_B85B: Rinka_B85B(k); return;
  default: Unreachable();
  }
}

void Rinka_Main(void) {  // 0xA2B7C4
  Enemy_Rinka *E = Get_Rinka(cur_enemy_index);
  if (E->rinka_parameter_1 && Get_Rinka(0)->rinka_var_1D) {
    Rinka_6(cur_enemy_index);
    Rinka_8(cur_enemy_index);
    //printf("A unknown\n");
    RinkasDeathAnimation(0);
  } else {
    CallRinkaFunc(E->rinka_var_A | 0xA20000, cur_enemy_index);
  }
}

void Rinka_3(uint16 k) {  // 0xA2B7DF
  Enemy_Rinka *E = Get_Rinka(k);
  if (sign16(--E->rinka_var_F)) {
    E->rinka_var_A = FUNC16(Rinka_B85B);
    if (E->rinka_parameter_1)
      E->base.properties &= ~kEnemyProps_Intangible;
    else
      E->base.properties = (E->base.properties | kEnemyProps_ProcessedOffscreen) & ~kEnemyProps_Intangible;
    uint16 r18 = (uint8)-(CalculateAngleFromXY(samus_x_pos - E->base.x_pos, samus_y_pos - E->base.y_pos) + 0x80);
    E->rinka_var_B = Math_MultBySin(kRinkaFireSpeed, r18);
    E->rinka_var_C = Math_MultByCos(kRinkaFireSpeed, r18);
  }
}

void Rinka_4(uint16 k) {  // 0xA2B844
  Enemy_Rinka *E = Get_Rinka(k);
  if (sign16(--E->rinka_var_F)) {
    E->base.health = 10;
    Rinka_Init2(k);
  }
}

void Rinka_5(uint16 k) {  // 0xA2B852
  if (Rinka_9(k) & 1)
    Rinka_B865(k);
}

void Rinka_B85B(uint16 k) {  // 0xA2B85B
  MoveEnemyWithVelocity();
  if (Rinka_9(k) & 1)
    Rinka_B865(k);
}

void Rinka_B865(uint16 k) {  // 0xA2B865
  if (Get_Rinka(k)->rinka_parameter_1 && (Rinka_8(k), Get_Rinka(0)->rinka_var_1D)) {
    Rinka_6(k);
    DeleteEnemyAndConnectedEnemies();
  } else {
    Rinka_6(k);
    Rinka_Init2(k);
  }
}

void Rinka_6(uint16 k) {  // 0xA2B880
  Enemy_Rinka *E = Get_Rinka(k);
  if (E->rinka_parameter_1 && (E->base.properties & kEnemyProps_Invisible) == 0) {
    Enemy_Rinka *E0 = Get_Rinka(0);
    int16 count = E0->rinka_var_1E - 1;
    if (count < 0)
      count = 0;
    E0->rinka_var_1E = count;
  }
}

void Rinka_7(uint16 k) {  // 0xA2B89C
  Enemy_Rinka *E = Get_Rinka(k);
  if ((random_enemy_counter & 3) == E->rinka_parameter_1)
    E->base.properties &= ~kEnemyProps_Intangible;
  else
    E->base.properties |= kEnemyProps_Intangible;
}

void Rinka_8(uint16 k) {  // 0xA2B8BB
  Enemy_Rinka *E = Get_Rinka(k);
  if (E->rinka_parameter_1) {
    uint16 rinka_var_D = E->rinka_var_D;
    if (rinka_var_D) {
      *RinkaSpawnAvailabilityFlag(rinka_var_D) = 0;
      E->rinka_var_D = 0;
    }
  }
}

uint8 Rinka_9(uint16 k) {  // 0xA2B8D3
  Enemy_Rinka *E = Get_Rinka(k);
  int16 y_pos = E->base.y_pos;
  if (y_pos < 0)
    return 1;
  int16 y_on_screen = y_pos + 16 - layer1_y_pos;
  if (y_on_screen < 0 || !sign16(y_on_screen - 256))
    return 1;
  int16 x_pos = E->base.x_pos;
  if (x_pos < 0)
    return 1;
  int16 x_on_screen = x_pos + 16 - layer1_x_pos;
  if (x_on_screen < 0 || !sign16(x_on_screen - 288))
    return 1;
  return 0;
}

bool Rinka_10(uint16 r18, uint16 r20) {  // 0xA2B8FF
  return sign16(r20) || (int16)(r20 - layer1_y_pos) < 0 || !sign16(r20 - layer1_y_pos - 224)
       || sign16(r20) || (int16)(r18 - layer1_x_pos) < 0 || !sign16(r18 - layer1_x_pos - 256);
}

void Rinka_Frozen(uint16 k) {  // 0xA2B929
  if (Rinka_9(k) & 1)
    Get_Rinka(k)->base.frozen_timer = 0;
  NormalEnemyFrozenAI();
  if (Get_Rinka(0)->rinka_var_1D) {
    Rinka_6(k);
    Rinka_8(k);
//    printf("A undefined!\n");
    RinkasDeathAnimation(0);
  }
}

void Rinka_Touch(void) {  // 0xA2B947
  NormalEnemyTouchAiSkipDeathAnim_CurEnemy();
  Rinka_B960(cur_enemy_index);
}

void Rinka_Shot(void) {  // 0xA2B94D
  NormalEnemyShotAiSkipDeathAnim_CurEnemy();
  Rinka_B960(cur_enemy_index);
}

void Rinka_Powerbomb(uint16 k) {  // 0xA2B953
  if ((Get_Rinka(k)->base.properties & kEnemyProps_Invisible) == 0) {
    NormalEnemyPowerBombAiSkipDeathAnim_CurEnemy();
    Rinka_B960(k);
  }
}

void Rinka_B960(uint16 k) {  // 0xA2B960
  Enemy_Rinka *E = Get_Rinka(k);
  if (!E->base.health) {
    Rinka_6(k);
    Rinka_8(k);
    if (E->rinka_parameter_1) {
      E->base.properties |= kEnemyProps_Intangible | kEnemyProps_Invisible;
      eproj_spawn_pt = (Point16U){ E->base.x_pos, E->base.y_pos };
      SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 3);
      E->rinka_var_A = FUNC16(Rinka_4);
      E->rinka_var_F = 1;
    } else {
      RinkasDeathAnimation(0);
    }
  }
}

uint16 Rinka_Instr_B9A2(uint16 k, uint16 j) {  // 0xA2B9A2
  Enemy_Rinka *E = Get_Rinka(0);
  if (sign16(E->rinka_var_1E - 3))
    return j + 2;
  else
    return *(uint16 *)RomPtr_A2(j);
}

const uint16 *Rinka_Instr_B9B3(uint16 k, const uint16 *jp) {  // 0xA2B9B3
  Enemy_Rinka *E = Get_Rinka(k);
  E->base.properties |= kEnemyProps_Intangible | kEnemyProps_Invisible;
  return jp;
}

const uint16 *Rinka_Instr_B9BD(uint16 k, const uint16 *jp) {  // 0xA2B9BD
  Enemy_Rinka *E = Get_Rinka(k);
  E->base.properties |= kEnemyProps_ProcessedOffscreen | kEnemyProps_Intangible | kEnemyProps_Invisible;
  return jp;
}

const uint16 *Rinka_Instr_B9C7(uint16 k, const uint16 *jp) {  // 0xA2B9C7
  Enemy_Rinka *E = Get_Rinka(k);
  E->base.properties &= ~(kEnemyProps_Intangible | kEnemyProps_Invisible);
  E->rinka_var_A = FUNC16(Rinka_3);
  Enemy_Rinka *E0 = Get_Rinka(0);
  ++E0->rinka_var_1E;
  return jp;
}
