// Enemy AI - Dead Torizo/Sidehopper/Zoomer/Skree/Ripper props and corpse-rotting — peeled from Bank $A9

#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "enemy_types.h"
#include "sm_rtl.h"

#define g_word_A9D583 ((uint16*)RomFixedPtr(0xa9d583))
#define g_word_A9D549 ((uint16*)RomFixedPtr(0xa9d549))
#define g_word_A9D67C ((uint16*)RomFixedPtr(0xa9d67c))
#define g_word_A9D69C ((uint16*)RomFixedPtr(0xa9d69c))
#define kDeadTorizo_TileData ((uint16*)RomFixedPtr(0xb7a800))
#define g_off_A9D86A ((uint16*)RomFixedPtr(0xa9d86a))
#define g_off_A9D870 ((uint16*)RomFixedPtr(0xa9d870))
#define g_off_A9D897 ((uint16*)RomFixedPtr(0xa9d897))
#define g_off_A9D89B ((uint16*)RomFixedPtr(0xa9d89b))
#define g_off_A9D8C0 ((uint16*)RomFixedPtr(0xa9d8c0))
#define g_off_A9D8C6 ((uint16*)RomFixedPtr(0xa9d8c6))
#define g_word_A9D951 ((uint16*)RomFixedPtr(0xa9d951))
#define g_word_A9D959 ((uint16*)RomFixedPtr(0xa9d959))
#define TILEMAP_ADDR(x) ((uint8*)tilemap_stuff + (x))
#define kDeadMonsters_TileData RomPtr_B7(addr_kDeadMonsters_TileData)


void CallCorpseRottingMove(uint32 ea, uint16 k, uint16 j) {
  switch (ea) {
  case fnTorizo_CorpseRottingCopyFunc: Torizo_CorpseRottingCopyFunc(k, j); return;
  case fnSidehopper_CorpseRottingCopyFunc_0: Sidehopper_CorpseRottingCopyFunc_0(k, j); return;
  case fnSidehopper_CorpseRottingCopyFunc_2: Sidehopper_CorpseRottingCopyFunc_2(k, j); return;
  case fnZoomer_CorpseRottingCopyFunc_0: Zoomer_CorpseRottingCopyFunc_0(k, j); return;
  case fnZoomer_CorpseRottingCopyFunc_2: Zoomer_CorpseRottingCopyFunc_2(k, j); return;
  case fnZoomer_CorpseRottingCopyFunc_4: Zoomer_CorpseRottingCopyFunc_4(k, j); return;
  case fnRipper_CorpseRottingCopyFunc_0: Ripper_CorpseRottingCopyFunc_0(k, j); return;
  case fnRipper_CorpseRottingCopyFunc_2: Ripper_CorpseRottingCopyFunc_2(k, j); return;
  case fnSkree_CorpseRottingCopyFunc_0: Skree_CorpseRottingCopyFunc_0(k, j); return;
  case fnSkree_CorpseRottingCopyFunc_2: Skree_CorpseRottingCopyFunc_2(k, j); return;
  case fnSkree_CorpseRottingCopyFunc_4: Skree_CorpseRottingCopyFunc_4(k, j); return;
  case fnMotherBrain_CorpseRottingCopyFunc: MotherBrain_CorpseRottingCopyFunc(k, j); return;

  case fnTorizo_CorpseRottingMoveFunc: Torizo_CorpseRottingMoveFunc(k, j); return;
  case fnSidehopper_CorpseRottingMoveFunc_0: Sidehopper_CorpseRottingMoveFunc_0(k, j); return;
  case fnSidehopper_CorpseRottingMoveFunc_2: Sidehopper_CorpseRottingMoveFunc_2(k, j); return;
  case fnZoomer_CorpseRottingMoveFunc_0: Zoomer_CorpseRottingMoveFunc_0(k, j); return;
  case fnZoomer_CorpseRottingMoveFunc_2: Zoomer_CorpseRottingMoveFunc_2(k, j); return;
  case fnZoomer_CorpseRottingMoveFunc_4: Zoomer_CorpseRottingMoveFunc_4(k, j); return;
  case fnRipper_CorpseRottingMoveFunc_0: Ripper_CorpseRottingMoveFunc_0(k, j); return;
  case fnRipper_CorpseRottingMoveFunc_2: Ripper_CorpseRottingMoveFunc_2(k, j); return;
  case fnSkree_CorpseRottingMoveFunc_0: Skree_CorpseRottingMoveFunc_0(k, j); return;
  case fnSkree_CorpseRottingMoveFunc_2: Skree_CorpseRottingMoveFunc_2(k, j); return;
  case fnSkree_CorpseRottingMoveFunc_4: Skree_CorpseRottingMoveFunc_4(k, j); return;
  case fnMotherBrain_CorpseRottingMoveFunc: MotherBrain_CorpseRottingMoveFunc(k, j); return;
  default: Unreachable();
  }
}
void CallCorpseRottingInit(uint32 ea) {
  switch (ea) {
  case fnTorizo_CorpseRottingInitFunc: Torizo_CorpseRottingInitFunc(); return;
  case fnSidehopper_CorpseRottingInitFunc_0: Sidehopper_CorpseRottingInitFunc_0(); return;
  case fnSidehopper_CorpseRottingInitFunc_2: Sidehopper_CorpseRottingInitFunc_2(); return;
  case fnZoomer_CorpseRottingInitFunc_0: Zoomer_CorpseRottingInitFunc_0(); return;
  case fnZoomer_CorpseRottingInitFunc_2: Zoomer_CorpseRottingInitFunc_2(); return;
  case fnZoomer_CorpseRottingInitFunc_4: Zoomer_CorpseRottingInitFunc_4(); return;
  case fnRipper_CorpseRottingInitFunc_0: Ripper_CorpseRottingInitFunc_0(); return;
  case fnRipper_CorpseRottingInitFunc_2: Ripper_CorpseRottingInitFunc_2(); return;
  case fnSkree_CorpseRottingInitFunc_0: Skree_CorpseRottingInitFunc_0(); return;
  case fnSkree_CorpseRottingInitFunc_2: Skree_CorpseRottingInitFunc_2(); return;
  case fnSkree_CorpseRottingInitFunc_4: Skree_CorpseRottingInitFunc_4(); return;
  case fnMotherBrain_CorpseRottingInitFunc: MotherBrain_CorpseRottingInitFunc(); return;
  default: Unreachable();
  }
}
void CallCorpseRottingFinish(uint32 ea) {
  switch (ea) {
  case fnMotherBrain_CorpseRottingFinished: MotherBrain_CorpseRottingFinished(); return;
  case fnDeadTorizo_CorpseRottingFinished: DeadTorizo_CorpseRottingFinished(); return;
  case fnCorpseRottingRotEntryFinishedHook: CorpseRottingRotEntryFinishedHook(); return;
  default: Unreachable();
  }
}

void DeadTorizo_Init(void) {  // 0xA9D308
  for (int i = 4094; i >= 0; i -= 2)
    tilemap_stuff[i >> 1] = 0;
  Enemy_DeadTorizo *E = Get_DeadTorizo(0);
  E->dto_var_A = FUNC16(DeadTorizo_WaitForSamusColl);
  E->base.properties |= 0xA000;
  E->base.current_instruction = addr_kDeadTorizo_Ilist_D6DC;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->base.palette_index = 512;
  E->dto_var_B = 0;
  E->dto_var_C = 8;
  E->dto_var_04 = 0;
  E->dto_var_02 = 15;
  E->dto_var_03 = 0;
  InitializeEnemyCorpseRotting(0, addr_stru_A9DD58);
}

void CallDeadTorizoFuncA(uint32 ea) {
  switch (ea) {
  case fnDeadTorizo_WaitForSamusColl: DeadTorizo_WaitForSamusColl(cur_enemy_index); return;
  case fnDeadTorizo_Rotting: DeadTorizo_Rotting(); return;
  case fnDeadTorizo_PreRotDelay: DeadTorizo_PreRotDelay(); return;
  case fnnullsub_361: return;
  default: Unreachable();
  }
}

void DeadTorizo_Main(void) {  // 0xA9D368
  Enemy_DeadTorizo *E = Get_DeadTorizo(0);
  if ((E->base.properties & kEnemyProps_Intangible) == 0 && DeadTorizo_Func_0() & 1) {
    E->base.properties |= kEnemyProps_Intangible;
    E->dto_var_A = FUNC16(DeadTorizo_Rotting);
  }
  mov24(&enemy_gfx_drawn_hook, 0xA9D39A);
  CallDeadTorizoFuncA(E->dto_var_A | 0xA90000);
  DeadTorizo_Func_1();
}

void DeadTorizo_MainGfxHook(void) {  // 0xA9D39A
  MotherBrain_AddSpritemapToOam(addr_kDeadTorizo_Sprmap_D761, 296, 187, 0);
}

void DeadTorizo_WaitForSamusColl(uint16 k) {  // 0xA9D3AD
  if (k == enemy_index_colliding_dirs[0]
      || k == enemy_index_colliding_dirs[1]
      || k == enemy_index_colliding_dirs[2]
      || k == enemy_index_colliding_dirs[3]) {
    Get_DeadTorizo(0)->dto_var_A = FUNC16(DeadTorizo_PreRotDelay);
  }
}

void DeadTorizo_PreRotDelay(void) {  // 0xA9D3C8
  Enemy_DeadTorizo *E = Get_DeadTorizo(0);
  uint16 v1 = E->dto_var_04 + 1;
  E->dto_var_04 = v1;
  if (v1 >= 0x10) {
    E->base.properties |= kEnemyProps_Intangible;
    E->dto_var_A = FUNC16(DeadTorizo_Rotting);
    DeadTorizo_Rotting();
  }
}

void DeadTorizo_Rotting(void) {  // 0xA9D3E6
  Enemy_DeadTorizo *E = Get_DeadTorizo(0);
  uint16 v1 = E->dto_var_03 + 1;
  E->dto_var_03 = v1;
  if (v1 >= 0xF) {
    E->dto_var_03 = 0;
    uint16 dto_var_02 = E->dto_var_02;
    if (dto_var_02) {
      DeadTorizo_CopyLineOfSandHeapTileData(dto_var_02);
      --E->dto_var_02;
    }
  }
  ++E->dto_var_C;
  MoveEnemyWithVelocity();
  if (!(ProcessCorpseRotting(0) & 1))
    E->dto_var_A = FUNC16(nullsub_361);
}

void DeadTorizo_Powerbomb(void) {  // 0xA9D42A
  if ((Get_DeadTorizo(0)->base.properties & kEnemyProps_Intangible) == 0)
    DeadTorizo_Shot();
}

void DeadTorizo_Shot(void) {  // 0xA9D433
  Enemy_DeadTorizo *E = Get_DeadTorizo(0);
  E->base.properties |= kEnemyProps_Intangible;
  E->dto_var_A = FUNC16(DeadTorizo_Rotting);
}

uint8 DeadTorizo_Func_0(void) {  // 0xA9D443
  int16 v5;
  uint16 v3, v4;
  Enemy_DeadTorizo *E = Get_DeadTorizo(0);
  uint16 r18 = E->base.x_pos, r20 = E->base.y_pos;
  const uint8 *p = RomPtr_A9(addr_kDeadTorizo_Hitbox_D77C);
  int R22 = GET_WORD(p);
  uint16 R24;
  for (p += 2; R22; p += 8, R22--) {
    if ((int16)(samus_y_pos - r20) >= 0) {
      R24 = samus_y_pos - r20;
      v3 = GET_WORD(p + 6);
    } else {
      R24 = r20 - samus_y_pos;
      v3 = GET_WORD(p + 2);
    }
    if ((int16)(samus_y_radius + abs16(v3) - R24) >= 0) {
      if ((int16)(samus_x_pos - r18) >= 0) {
        R24 = samus_x_pos - r18;
        v4 = GET_WORD(p + 4);
      } else {
        R24 = r18 - samus_x_pos;
        v4 = GET_WORD(p);
      }
      v5 = samus_x_radius + abs16(v4) - R24;
      if (v5 >= 0) {
        if (sign16(v5 - 4))
          v5 = 4;
        extra_samus_x_displacement = v5;
        extra_samus_y_displacement = 4;
        extra_samus_x_subdisplacement = 0;
        extra_samus_y_subdisplacement = 0;
        return 1;
      }
    }
  }
  return 0;
}

void DeadTorizo_Func_1(void) {  // 0xA9D4CF
  VramWriteEntry *v5;
  VramWriteEntry *v10;
  Enemy_DeadTorizo *E = Get_DeadTorizo(0);
  uint16 v1 = E->dto_var_00 + 1;
  E->dto_var_00 = v1;
  uint16 v3 = vram_write_queue_tail;
  if (v1 & 1) {
    uint16 v8 = 0;
    uint16 v9 = g_word_A9D583[0];
    do {
      v10 = gVramWriteEntry(v3);
      v10->size = v9;
      int v11 = v8 >> 1;
      *(VoidP *)((uint8 *)&v10->src.addr + 1) = g_word_A9D583[v11 + 1];
      v10->src.addr = g_word_A9D583[v11 + 2];
      v10->vram_dst = g_word_A9D583[v11 + 3];
      v3 += 7;
      v8 += 8;
      v9 = g_word_A9D583[v8 >> 1];
    } while (v9);
  } else {
    uint16 v2 = 0;
    uint16 v4 = g_word_A9D549[0];
    do {
      v5 = gVramWriteEntry(v3);
      v5->size = v4;
      int v6 = v2 >> 1;
      *(VoidP *)((uint8 *)&v5->src.addr + 1) = g_word_A9D549[v6 + 1];
      v5->src.addr = g_word_A9D549[v6 + 2];
      v5->vram_dst = g_word_A9D549[v6 + 3];
      v3 += 7;
      v2 += 8;
      v4 = g_word_A9D549[v2 >> 1];
    } while (v4);
  }
  Get_DeadEnemy(0)->dey_var_22 = 0;
  vram_write_queue_tail = v3;
}

void DeadTorizo_CorpseRottingFinished(void) {  // 0xA9D5BD
  eproj_spawn_pt = (Point16U){ (random_number & 0x1F) + 272, 188 };
  SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 0xA);
  if ((random_enemy_counter & 7) == 0)
    QueueSfx2_Max6(0x10);
}

void DeadTorizo_CopyLineOfSandHeapTileData(uint16 a) {  // 0xA9D5EA
  uint16 v1 = g_word_A9D67C[a];
  int v2 = g_word_A9D69C[a] >> 1;
  *(uint16 *)(&g_byte_7E9500 + v1) = kDeadTorizo_TileData[v2];
  *(uint16 *)(&g_byte_7E9510 + v1) = kDeadTorizo_TileData[v2 + 8];
  *(uint16 *)(&g_byte_7E9520 + v1) = kDeadTorizo_TileData[v2 + 16];
  *(uint16 *)(&g_byte_7E9530 + v1) = kDeadTorizo_TileData[v2 + 24];
  *(uint16 *)(&g_byte_7E9540 + v1) = kDeadTorizo_TileData[v2 + 32];
  *(uint16 *)(&g_byte_7E9550 + v1) = kDeadTorizo_TileData[v2 + 40];
  *(uint16 *)(&g_byte_7E9560 + v1) = kDeadTorizo_TileData[v2 + 48];
  *(uint16 *)(&g_byte_7E9570 + v1) = kDeadTorizo_TileData[v2 + 56];
  *(uint16 *)(&g_byte_7E9580 + v1) = kDeadTorizo_TileData[v2 + 64];
  *(uint16 *)(&g_byte_7E9590 + v1) = kDeadTorizo_TileData[v2 + 72];
  *(uint16 *)(&g_byte_7E95A0 + v1) = kDeadTorizo_TileData[v2 + 80];
  *(uint16 *)(&g_byte_7E95B0 + v1) = kDeadTorizo_TileData[v2 + 88];
  *(uint16 *)(&g_byte_7E95C0 + v1) = kDeadTorizo_TileData[v2 + 96];
  *(uint16 *)(&g_byte_7E95D0 + v1) = kDeadTorizo_TileData[v2 + 104];
  *(uint16 *)(&g_byte_7E95E0 + v1) = kDeadTorizo_TileData[v2 + 112];
  *(uint16 *)(&g_byte_7E95F0 + v1) = kDeadTorizo_TileData[v2 + 120];
  *(uint16 *)(&g_byte_7E9600 + v1) = kDeadTorizo_TileData[v2 + 128];
  *(uint16 *)(&g_byte_7E9610 + v1) = kDeadTorizo_TileData[v2 + 136];
}

void DeadSidehopper_Init(void) {  // 0xA9D7B6
  uint16 dsr_parameter_1 = Get_DeadSidehopper(cur_enemy_index)->dsr_parameter_1;
  if (dsr_parameter_1) {
    if (dsr_parameter_1 != 2) {
      Unreachable();
    }
    DeadSidehopper_Init_1();
  } else {
    DeadSidehopper_Init_0();
  }
}

void DeadSidehopper_Init_0(void) {  // 0xA9D7C4
  Enemy_DeadSidehopper *E = Get_DeadSidehopper(cur_enemy_index);
  E->base.properties = E->base.properties & 0x77FF | kEnemyProps_ProcessedOffscreen;
  if ((Get_DeadSidehopper(0)->base.properties & kEnemyProps_Invisible) != 0)
    E->base.properties |= kEnemyProps_Deleted;
  E->dsr_var_08 = 0;
  E->dsr_var_0A = 96;
  E->dsr_var_0B = 256;
  E->base.x_pos = 488;
  E->base.y_pos = 184;
  E->dsr_var_A = FUNC16(DeadSidehopper_Alive_WaitForActivate);
  E->base.palette_index = 512;
  E->base.y_height = 21;
  Enemy_SetInstrList(cur_enemy_index, addr_kDeadMonsters_Ilist_ECE3);
  InitializeEnemyCorpseRotting(cur_enemy_index, addr_stru_A9DD68);
}

void DeadSidehopper_Init_1(void) {  // 0xA9D825
  Enemy_DeadSidehopper *E = Get_DeadSidehopper(cur_enemy_index);
  E->dsr_var_08 = -1;
  E->dsr_var_A = FUNC16(DeadSidehopper_WaitForSamusColl);
  E->base.palette_index = 3584;
  Enemy_SetInstrList(cur_enemy_index, addr_kDeadMonsters_Ilist_ECEF);
  InitializeEnemyCorpseRotting(cur_enemy_index, addr_stru_A9DD78);
}

void DeadZoomer_Init(void) {  // 0xA9D849
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  v1->palette_index = 3584;
  v1->ai_var_A = FUNC16(DeadZoomer_WaitForSamusColl);
  int v2 = v1->parameter_1 >> 1;
  Enemy_SetInstrList(cur_enemy_index, g_off_A9D86A[v2]);
  InitializeEnemyCorpseRotting(cur_enemy_index, g_off_A9D870[v2]);
}

void DeadRipper_Init(void) {  // 0xA9D876
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  v1->palette_index = 3584;
  v1->ai_var_A = FUNC16(DeadRipper_WaitForSamusColl);
  int v2 = v1->parameter_1 >> 1;
  Enemy_SetInstrList(cur_enemy_index, g_off_A9D897[v2]);
  InitializeEnemyCorpseRotting(cur_enemy_index, g_off_A9D89B[v2]);
}

void DeadSkree_Init(void) {  // 0xA9D89F
  EnemyData *v1 = gEnemyData(cur_enemy_index);
  v1->palette_index = 3584;
  v1->ai_var_A = FUNC16(DeadSkree_WaitForSamusColl);
  int v2 = v1->parameter_1 >> 1;
  Enemy_SetInstrList(cur_enemy_index, g_off_A9D8C0[v2]);
  InitializeEnemyCorpseRotting(cur_enemy_index, g_off_A9D8C6[v2]);
}

void DeadSidehopper_Powerbomb(void) {  // 0xA9D8CC
  if (Get_DeadSidehopper(cur_enemy_index)->dsr_var_08 >= 8)
    DeadSidehopper_Shot();
  else
    DeadSidehopper_Main();
}

void CallDeadSidehopperFunc(uint32 ea, uint16 k) {
  switch (ea) {
  case fnDeadSidehopper_Activated: DeadSidehopper_Activated(k); return;
  case fnDeadSidehopper_Alive_WaitForActivate: DeadSidehopper_Alive_WaitForActivate(k); return;

  case fnDeadSidehopper_WaitForSamusColl: DeadSidehopper_WaitForSamusColl(k); return;
  case fnDeadZoomer_WaitForSamusColl: DeadZoomer_WaitForSamusColl(k); return;
  case fnDeadSkree_WaitForSamusColl: DeadSkree_WaitForSamusColl(k); return;
  case fnDeadRipper_WaitForSamusColl: DeadRipper_WaitForSamusColl(k); return;

  case fnDeadSidehopper_PreRotDelay: DeadSidehopper_PreRotDelay(k); return;
  case fnDeadZoomer_PreRotDelay: DeadZoomer_PreRotDelay(k); return;
  case fnDeadSkree_PreRotDelay: DeadSkree_PreRotDelay(k); return;
  case fnDeadRipper_PreRotDelay: DeadRipper_PreRotDelay(k); return;

  case fnDeadSidehopper_Rotting: DeadSidehopper_Rotting(k); return;
  case fnDeadZoomer_Rotting: DeadZoomer_Rotting(k); return;
  case fnDeadSkree_Rotting: DeadSkree_Rotting(k); return;
  case fnDeadRipper_Rotting: DeadRipper_Rotting(k); return;

  case fnDeadMonsters_Func_1: DeadMonsters_Func_1(k); return;
  case fnDeadMonsters_Func_2: DeadMonsters_Func_2(k); return;
  case fnDeadMonsters_Func_5: DeadMonsters_Func_5(k); return;

  case fnnullsub_362: return;
  case fnnullsub_268: return;

  default: Unreachable();
  }
}
void DeadSidehopper_Main(void) {  // 0xA9D8DB
  Enemy_DeadSidehopper *E = Get_DeadSidehopper(cur_enemy_index);
  CallDeadSidehopperFunc(E->dsr_var_A | 0xA90000, cur_enemy_index);
}

void DeadSidehopper_Alive_WaitForActivate(uint16 k) {  // 0xA9D8E2
  if (sign16(layer1_x_pos - 513)) {
    Get_DeadSidehopper(k)->dsr_var_A = FUNC16(DeadSidehopper_Activated);
    DeadSidehopper_Activated(k);
  }
}

void DeadSidehopper_Activated(uint16 k) {  // 0xA9D8F1
  if (DeadMonsters_Func_3(k) & 1) {
    Enemy_DeadSidehopper *E = Get_DeadSidehopper(k);
    E->dsr_var_06 = (E->dsr_var_06 + 1) & 3;
    Enemy_SetInstrList(k, addr_kDeadMonsters_Ilist_ECAC);
    E->dsr_var_A = FUNC16(nullsub_362);
  }
}

void DeadMonsters_Func_1(uint16 k) {  // 0xA9D910
  Enemy_DeadMonsters *E = Get_DeadMonsters(k);
  E->dms_var_A = FUNC16(DeadMonsters_Func_2);
  E->dms_var_F = 64;
}

void DeadMonsters_Func_2(uint16 k) {  // 0xA9D91D
  Enemy_DeadMonsters *E = Get_DeadMonsters(k);
  if ((--E->dms_var_F & 0x8000) != 0) {
    if (E->dms_var_08) {
      E->dms_var_A = FUNC16(DeadMonsters_Func_5);
    } else {
      E->dms_var_A = FUNC16(DeadSidehopper_Activated);
      Enemy_SetInstrList(k, addr_kDeadMonsters_Ilist_ECE3);
      int v3 = E->dms_var_06;
      E->dms_var_0B = g_word_A9D951[v3];
      E->dms_var_0A = g_word_A9D959[v3];
    }
  }
}

uint8 DeadMonsters_Func_3(uint16 k) {  // 0xA9D961
  Enemy_DeadMonsters *E = Get_DeadMonsters(k);
  DeadMonsters_Func_4(k, E->dms_var_0A);
  uint16 v2 = 32;
  if ((E->dms_var_0B & 0x8000) == 0)
    v2 = 128;
  uint16 v3 = E->dms_var_0B + v2;
  E->dms_var_0B = v3;
  if (sign16(E->base.x_pos - 544)) {
    int full = HIBYTE(E->base.y_subpos) + LOBYTE(v3);
    HIBYTE(E->base.y_subpos) = full;
    uint16 v12 = E->base.y_pos + (int8)(v3 >> 8) + (full >> 8);
    E->base.y_pos = v12;
    return sign16(v12 - 184) == 0;
  } else {
    return Enemy_MoveDown(k, INT16_SHL8(v3));
  }
}

void DeadMonsters_Func_4(uint16 k, uint16 a) {  // 0xA9D9C7
  Enemy_DeadMonsters *E = Get_DeadMonsters(k);
  if (sign16(E->base.x_pos - 544)) {
    int full = HIBYTE(E->base.x_subpos) + LOBYTE(a);
    HIBYTE(E->base.x_subpos) = full;
    E->base.x_pos += (int8)(a >> 8) + (full >> 8);
  } else {
    Enemy_MoveRight_IgnoreSlopes(k, INT16_SHL8(a));
  }
}

void DeadMonsters_Func_5(uint16 k) {  // 0xA9DA08
  Enemy_DeadMonsters *E = Get_DeadMonsters(k);
  uint16 v2 = E->dms_var_07 + 1;
  E->dms_var_07 = v2;
  if (v2 >= 8) {
    E->dms_var_07 = 0;
    WriteColorsToPalette(
      0x122,
      0xa9, 32 * (E->dms_var_08 - 1) - 0x1434,
      0xF);
    uint16 v3 = cur_enemy_index;
    uint16 v5 = E->dms_var_08 + 1;
    E->dms_var_08 = v5;
    if (v5 >= 8) {
      Enemy_SetInstrList(v3, addr_kDeadMonsters_Ilist_ECE9);
      E->dms_var_A = FUNC16(DeadSidehopper_WaitForSamusColl);
      E->base.properties |= kEnemyProps_SolidToSamus;
      E->base.y_height = 12;
    }
  }
}

void DeadSidehopper_WaitForSamusColl(uint16 k) {  // 0xA9DA64
  DeadMonsters_WaitForSamusColl(k, FUNC16(DeadSidehopper_PreRotDelay));
}

void DeadZoomer_WaitForSamusColl(uint16 k) {  // 0xA9DA69
  DeadMonsters_WaitForSamusColl(k, FUNC16(DeadZoomer_PreRotDelay));
}

void DeadSkree_WaitForSamusColl(uint16 k) {  // 0xA9DA6E
  DeadMonsters_WaitForSamusColl(k, FUNC16(DeadSkree_PreRotDelay));
}

void DeadRipper_WaitForSamusColl(uint16 k) {  // 0xA9DA73
  DeadMonsters_WaitForSamusColl(k, FUNC16(DeadRipper_PreRotDelay));
}

void DeadMonsters_WaitForSamusColl(uint16 k, uint16 j) {  // 0xA9DA76
  if (k == enemy_index_colliding_dirs[0]
      || k == enemy_index_colliding_dirs[1]
      || k == enemy_index_colliding_dirs[2]
      || k == enemy_index_colliding_dirs[3]) {
    Get_DeadMonsters(k)->dms_var_A = j;
  }
}

void DeadSidehopper_PreRotDelay(uint16 k) {  // 0xA9DA8F
  DeadMonsters_PreRotDelay_Common(k, FUNC16(DeadSidehopper_Rotting));
}

void DeadZoomer_PreRotDelay(uint16 k) {  // 0xA9DA94
  DeadMonsters_PreRotDelay_Common(k, FUNC16(DeadZoomer_Rotting));
}

void DeadRipper_PreRotDelay(uint16 k) {  // 0xA9DA99
  DeadMonsters_PreRotDelay_Common(k, FUNC16(DeadRipper_Rotting));
}

void DeadSkree_PreRotDelay(uint16 k) {  // 0xA9DA9E
  DeadMonsters_PreRotDelay_Common(k, FUNC16(DeadSkree_Rotting));
}

void DeadMonsters_PreRotDelay_Common(uint16 k, uint16 j) {  // 0xA9DAA1
  Enemy_DeadMonsters *E = Get_DeadMonsters(k);
  if (++E->dms_var_B >= 0x10) {
    E->dms_var_A = j;
    E->base.properties |= kEnemyProps_Intangible;
  }
}

void DeadSidehopper_Rotting(uint16 k) {  // 0xA9DABA
  uint8 v1 = ProcessCorpseRotting(k);
  if (!(v1 & 1))
    Get_DeadMonsters(cur_enemy_index)->dms_var_A = FUNC16(DeadSidehopper_WaitForSamusColl);
  uint16 dms_var_53 = Get_DeadMonsters(cur_enemy_index)->dms_var_53;
  ProcessCorpseRottingVramTransfers(dms_var_53);
}

void DeadZoomer_Rotting(uint16 k) {  // 0xA9DAD0
  uint8 v1 = ProcessCorpseRotting(k);
  if (!(v1 & 1))
    Get_DeadMonsters(cur_enemy_index)->dms_var_A = FUNC16(nullsub_268);
  uint16 dms_var_53 = Get_DeadMonsters(cur_enemy_index)->dms_var_53;
  ProcessCorpseRottingVramTransfers(dms_var_53);
}

void DeadRipper_Rotting(uint16 k) {  // 0xA9DAE6
  uint8 v1 = ProcessCorpseRotting(k);
  if (!(v1 & 1))
    Get_DeadMonsters(cur_enemy_index)->dms_var_A = FUNC16(nullsub_268);
  uint16 dms_var_53 = Get_DeadMonsters(cur_enemy_index)->dms_var_53;
  ProcessCorpseRottingVramTransfers(dms_var_53);
}

void DeadSkree_Rotting(uint16 k) {  // 0xA9DAFC
  uint8 v1 = ProcessCorpseRotting(k);
  if (!(v1 & 1))
    Get_DeadMonsters(cur_enemy_index)->dms_var_A = FUNC16(nullsub_268);
  uint16 dms_var_53 = Get_DeadMonsters(cur_enemy_index)->dms_var_53;
  ProcessCorpseRottingVramTransfers(dms_var_53);
}

uint8 ProcessCorpseRotting(uint16 k) {  // 0xA9DB12
  int16 *v5;
  int16 v7;
  int16 v8;

  Enemy_DeadMonsters *EK = Get_DeadMonsters(k);
  Enemy_DeadMonsters *E0 = Get_DeadMonsters(0);
  E0->dms_var_45 = EK->dms_var_57;
  E0->dms_var_46 = EK->dms_var_58;
  E0->dms_var_47 = EK->dms_var_59;
  E0->dms_var_48 = EK->dms_var_5A;
  E0->dms_var_49 = EK->dms_var_5B;
  E0->dms_var_44 = EK->dms_var_56;
  E0->dms_var_42 = EK->dms_var_54;
  E0->dms_var_43 = EK->dms_var_55;
  uint16 dms_var_52 = EK->dms_var_52;
  uint16 v4 = 0;
  uint16 v15;
  uint16 *v6;
  while (1) {
    v15 = v4;
    v5 = (int16 *)(g_ram + dms_var_52);
    v6 = (uint16 *)v5;
    if (*v5 >= 0)
      break;
LABEL_12:
    dms_var_52 += 4;
    v4 = v15 + 1;
    E0 = Get_DeadMonsters(0);
    if ((int16)(v15 + 1 - E0->dms_var_45) >= 0)
      return (uint16)(v15 + 1) >= E0->dms_var_45;
  }
  v7 = v5[1];
  if (v7) {
    v8 = v7 - 1;
    v5[1] = v8;
    if ((uint16)v8 < 4) {
      E0 = Get_DeadMonsters(0);
      uint16 r18 = E0->dms_var_42;
      if (v15 >= E0->dms_var_47)
        r18 = E0->dms_var_43;

      CopyMoveCorpseRottingRotEntry((const uint16 *)RomPtr_A9(E0->dms_var_44), *v6, r18);
    }
    goto LABEL_12;
  }
  Enemy_DeadMonsters *E = Get_DeadMonsters(0);
  uint16 r18 = E->dms_var_43;

  uint16 *v11 = (uint16 *)(g_ram + dms_var_52);
  CopyMoveCorpseRottingRotEntry((const uint16 *)RomPtr_A9(E->dms_var_44), *v11, r18);
  uint16 v12 = *v11 + 2;
  if (v12 < E->dms_var_46) {
    *(uint16 *)v11 = v12;
    goto LABEL_12;
  }
  CallCorpseRottingFinish(E->dms_var_49 | 0xA90000);
  if (v15 < Get_DeadMonsters(0)->dms_var_46) {
    *(uint16 *)v11 = -1;
    goto LABEL_12;
  }
  return 0;
}

void CopyMoveCorpseRottingRotEntry(const uint16 *r20, uint16 a, uint16 r18) {  // 0xA9DBE0
  Enemy_DeadMonsters *E = Get_DeadMonsters(0);
  E->dms_var_41 = a;
  uint16 v1 = (uint16)(a & 0xFFF8) >> 2;
  uint16 v2 = a & 7;
  uint16 v6 = r20[v1 >> 1] + 2 * v2;
  if (v2 >= 6) {
    CallCorpseRottingMove(r18 | 0xA90000, E->dms_var_48 + v6, v6);
  } else {
    CallCorpseRottingMove(r18 | 0xA90000, v6, v6);
  }
}

void CorpseRottingRotEntryFinishedHook(void) {  // 0xA9DC08
  EnemyData *v0 = gEnemyData(cur_enemy_index);
  eproj_spawn_pt = (Point16U){ v0->x_pos + (random_number & 0x1A) - 14, v0->y_pos + 16 };
  SpawnEprojWithRoomGfx(addr_kEproj_DustCloudExplosion, 0xA);
  if ((random_enemy_counter & 7) == 0)
    QueueSfx2_Max3(0x10);
}

void InitializeCorpseRottingDataTable(uint16 *table, uint16 a) {  // 0xA9DC40
  int16 v2;
  v2 = a - 1;
  uint16 r18 = 0;
  do {
    table[0] = v2;
    table[1] = r18;
    table += 2;
    r18 += 2;
    --v2;
  } while (v2 >= 0);
}

void InitializeEnemyCorpseRotting(uint16 k, uint16 j) {  // 0xA9DC5F
  const uint8 *v2 = RomPtr_A9(j);
  Enemy_DeadMonsters *E = Get_DeadMonsters(k);
  E->dms_var_52 = GET_WORD(v2);
  E->dms_var_53 = GET_WORD(v2 + 2);
  E->dms_var_54 = GET_WORD(v2 + 4);
  E->dms_var_55 = GET_WORD(v2 + 6);
  E->dms_var_5B = GET_WORD(v2 + 14);
  uint16 v4 = GET_WORD(v2 + 12);
  E->dms_var_56 = v4;
  E->dms_var_5A = *((uint16 *)RomPtr_A9(v4) + 1) - 12;
  uint16 *v5 = (uint16 *)v2;
  uint16 v6 = v5[4];
  E->dms_var_57 = v6--;
  E->dms_var_58 = v6;
  E->dms_var_59 = v6 - 1;
  InitializeCorpseRottingDataTable((uint16*)(g_ram + *v5), v5[4]);
  CallCorpseRottingInit(v5[5] | 0xA90000);
}

void ProcessCorpseRottingVramTransfers(uint16 k) {  // 0xA9DCB9
  const uint8 *p = RomPtr_A9(k);
  uint16 v1 = vram_write_queue_tail;
  uint16 v2 = *(uint16 *)p;
  do {
    VramWriteEntry *v3 = gVramWriteEntry(v1);
    v3->size = v2;
    *(VoidP *)((uint8 *)&v3->src.addr + 1) = *((uint16 *)p + 1);
    v3->src.addr = *((uint16 *)p + 2);
    v3->vram_dst = *((uint16 *)p + 3);
    v1 += 7;
    p += 8;
    v2 = *(uint16 *)p;
  } while (v2);
  Get_DeadEnemy(0)->dey_var_22 = 0;
  vram_write_queue_tail = v1;
}

void DeadZoomer_Powerbomb(void) {  // 0xA9DCED
  if ((gEnemyData(cur_enemy_index)->properties & kEnemyProps_Intangible) == 0)
    DeadZoomer_Shot();
}

void DeadZoomer_Shot(void) {  // 0xA9DCF8
  DeadSidehopper_DD34(FUNC16(DeadZoomer_Rotting));
}

void DeadRipper_Powerbomb(void) {  // 0xA9DCFD
  if ((gEnemyData(cur_enemy_index)->properties & kEnemyProps_Intangible) == 0)
    DeadRipper_Shot();
}

void DeadRipper_Shot(void) {  // 0xA9DD08
  DeadSidehopper_DD34(FUNC16(DeadRipper_Rotting));
}

void DeadSkree_Powerbomb(void) {  // 0xA9DD0D
  if ((gEnemyData(cur_enemy_index)->properties & kEnemyProps_Intangible) == 0)
    DeadSkree_Shot();
}

void DeadSkree_Shot(void) {  // 0xA9DD18
  DeadSidehopper_DD34(FUNC16(DeadSkree_Rotting));
}

void DeadSidehopper_Shot(void) {  // 0xA9DD1D
  Enemy_DeadSidehopper *E = Get_DeadSidehopper(cur_enemy_index);
  if ((E->base.properties & kEnemyProps_Intangible) != 0 || E->dsr_var_08 < 8)
    ;
  else
    DeadSidehopper_DD31();
}

void DeadSidehopper_DD31(void) {  // 0xA9DD31
  DeadSidehopper_DD34(FUNC16(DeadSidehopper_Rotting));
}

void DeadSidehopper_DD34(uint16 a) {  // 0xA9DD34
  Enemy_DeadSidehopper *E = Get_DeadSidehopper(cur_enemy_index);
  E->dsr_var_A = a;
  E->base.properties |= kEnemyProps_ProcessedOffscreen | kEnemyProps_Intangible;
}

void DeadSidehopper_Touch(void) {  // 0xA9DD44
  if (Get_DeadSidehopper(cur_enemy_index)->dsr_var_08 < 8)
    NormalEnemyTouchAiSkipDeathAnim_CurEnemy();
  else
    DeadSidehopper_DD31();
}


void Torizo_CorpseRottingInitFunc(void) {  // 0xA9DE18
  const uint8 *p = RomPtr_B7(addr_kDeadTorizo_TileData);
  MemCpy(TILEMAP_ADDR(0x060), p + 288, 0xC0);
  MemCpy(TILEMAP_ADDR(0x1A0), p + 800, 0xC0);
  MemCpy(TILEMAP_ADDR(0x2C0), p + 1280, 0x100);
  MemCpy(TILEMAP_ADDR(0x400), p + 1792, 0x100);
  MemCpy(TILEMAP_ADDR(0x540), p + 2304, 0x100);
  MemCpy(TILEMAP_ADDR(0x680), p + 2816, 0x100);
  MemCpy(TILEMAP_ADDR(0x7C0), p + 3328, 0x100);
  MemCpy(TILEMAP_ADDR(0x900), p + 3840, 0x100);
  MemCpy(TILEMAP_ADDR(0xA40), p + 4352, 0x100);
  MemCpy(TILEMAP_ADDR(0xB60), p + 4832, 0x120);
  MemCpy(TILEMAP_ADDR(0xC80), p + 5312, 0x140);
  MemCpy(TILEMAP_ADDR(0xDC0), p + 5824, 0x140);
}


void Sidehopper_CorpseRottingInitFunc_0(void) {  // 0xA9DEC1
  const uint8 *p = kDeadMonsters_TileData;
  MemCpy(TILEMAP_ADDR(0x040), p + 64, 0x60);
  MemCpy(TILEMAP_ADDR(0x0A0), p + 512, 0xA0);
  MemCpy(TILEMAP_ADDR(0x140), p + 1024, 0xA0);
  MemCpy(TILEMAP_ADDR(0x1E0), p + 1536, 0xA0);
  MemCpy(TILEMAP_ADDR(0x280), p + 2048, 0xA0);
}

void Sidehopper_CorpseRottingInitFunc_2(void) {  // 0xA9DF08
  const uint8 *p = kDeadMonsters_TileData;
  MemCpy(TILEMAP_ADDR(0x320), p + 288, 0x40);
  MemCpy(TILEMAP_ADDR(0x3C0), p + 800, 0xA0);
  MemCpy(TILEMAP_ADDR(0x460), p + 1312, 0xA0);
  MemCpy(TILEMAP_ADDR(0x500), p + 1824, 0xA0);
  MemCpy(TILEMAP_ADDR(0x5A0), p + 2336, 0xA0);
}

void Zoomer_CorpseRottingInitFunc_0(void) {  // 0xA9DF4F
  const uint8 *p = kDeadMonsters_TileData;
  MemCpy(TILEMAP_ADDR(0x940), p + 2656, 0x60);
  MemCpy(TILEMAP_ADDR(0x9A0), p + 3168, 0x60);
}

void Zoomer_CorpseRottingInitFunc_2(void) {  // 0xA9DF6C
  const uint8 *p = kDeadMonsters_TileData;
  MemCpy(TILEMAP_ADDR(0xA00), p + 2752, 0x60);
  MemCpy(TILEMAP_ADDR(0xA60), p + 3264, 0x60);
}

void Zoomer_CorpseRottingInitFunc_4(void) {  // 0xA9DF89
  const uint8 *p = kDeadMonsters_TileData;
  MemCpy(TILEMAP_ADDR(0xAC0), p + 2848, 0x60);
  MemCpy(TILEMAP_ADDR(0xB20), p + 3360, 0x60);
}

void Ripper_CorpseRottingInitFunc_0(void) {  // 0xA9DFA6
  const uint8 *p = kDeadMonsters_TileData;
  MemCpy(TILEMAP_ADDR(0xB80), p + 2560, 0x60);
  MemCpy(TILEMAP_ADDR(0xBE0), p + 3072, 0x60);
}

void Ripper_CorpseRottingInitFunc_2(void) {  // 0xA9DFC3
  const uint8 *p = kDeadMonsters_TileData;
  MemCpy(TILEMAP_ADDR(0xC40), p + 2944, 0x60);
  MemCpy(TILEMAP_ADDR(0xCA0), p + 3456, 0x60);
}

void Skree_CorpseRottingInitFunc_0(void) {  // 0xA9DFE0
  const uint8 *p = kDeadMonsters_TileData;
  MemCpy(TILEMAP_ADDR(0x640), p + 672, 0x40);
  MemCpy(TILEMAP_ADDR(0x680), p + 1184, 0x40);
  MemCpy(TILEMAP_ADDR(0x6C0), p + 1696, 0x40);
  MemCpy(TILEMAP_ADDR(0x700), p + 2208, 0x40);
}

void Skree_CorpseRottingInitFunc_2(void) {  // 0xA9E019
  const uint8 *p = kDeadMonsters_TileData;
  MemCpy(TILEMAP_ADDR(0x740), p + 224, 0x40);
  MemCpy(TILEMAP_ADDR(0x780), p + 736, 0x40);
  MemCpy(TILEMAP_ADDR(0x7C0), p + 1248, 0x40);
  MemCpy(TILEMAP_ADDR(0x800), p + 1760, 0x40);
}

void Skree_CorpseRottingInitFunc_4(void) {  // 0xA9E052
  const uint8 *p = kDeadMonsters_TileData;
  MemCpy(TILEMAP_ADDR(0x840), p + 448, 0x40);
  MemCpy(TILEMAP_ADDR(0x880), p + 960, 0x40);
  MemCpy(TILEMAP_ADDR(0x8C0), p + 1472, 0x40);
  MemCpy(TILEMAP_ADDR(0x900), p + 1984, 0x40);
}

void Torizo_CorpseRottingMoveFunc(uint16 j, uint16 k) {  // 0xA9E272
  Enemy_DeadMonsters *E = Get_DeadMonsters(0);
  if (E->dms_var_41 >= 0x50) {
    if (sign16(E->dms_var_41 - 94)) {
      int v3 = k >> 1;
      int v4 = j >> 1;
      tilemap_stuff[v4 + 1] = tilemap_stuff[v3];
      tilemap_stuff[v4 + 9] = tilemap_stuff[v3 + 8];
    }
    int v5 = k >> 1;
    tilemap_stuff[v5] = 0;
    tilemap_stuff[v5 + 8] = 0;
  }
  if (E->dms_var_41 >= 0x48) {
    if (sign16(E->dms_var_41 - 94)) {
      int v6 = k >> 1;
      int v7 = j >> 1;
      tilemap_stuff[v7 + 17] = tilemap_stuff[v6 + 16];
      tilemap_stuff[v7 + 25] = tilemap_stuff[v6 + 24];
    }
    int v8 = k >> 1;
    tilemap_stuff[v8 + 16] = 0;
    tilemap_stuff[v8 + 24] = 0;
  }
  if (E->dms_var_41 >= 0x10) {
    if (sign16(E->dms_var_41 - 94)) {
      int v9 = k >> 1;
      int v10 = j >> 1;
      tilemap_stuff[v10 + 33] = tilemap_stuff[v9 + 32];
      tilemap_stuff[v10 + 41] = tilemap_stuff[v9 + 40];
    }
    int v11 = k >> 1;
    tilemap_stuff[v11 + 32] = 0;
    tilemap_stuff[v11 + 40] = 0;
  }
  if (sign16(E->dms_var_41 - 94)) {
    int v12 = k >> 1;
    int v13 = j >> 1;
    tilemap_stuff[v13 + 49] = tilemap_stuff[v12 + 48];
    tilemap_stuff[v13 + 57] = tilemap_stuff[v12 + 56];
  }
  int v14 = k >> 1;
  tilemap_stuff[v14 + 48] = 0;
  tilemap_stuff[v14 + 56] = 0;
  if (sign16(E->dms_var_41 - 94)) {
    int v15 = j >> 1;
    tilemap_stuff[v15 + 65] = tilemap_stuff[v14 + 64];
    tilemap_stuff[v15 + 73] = tilemap_stuff[v14 + 72];
  }
  tilemap_stuff[v14 + 64] = 0;
  tilemap_stuff[v14 + 72] = 0;
  if (sign16(E->dms_var_41 - 94)) {
    int v16 = j >> 1;
    tilemap_stuff[v16 + 81] = tilemap_stuff[v14 + 80];
    tilemap_stuff[v16 + 89] = tilemap_stuff[v14 + 88];
  }
  tilemap_stuff[v14 + 80] = 0;
  tilemap_stuff[v14 + 88] = 0;
  if (sign16(E->dms_var_41 - 94)) {
    int v17 = j >> 1;
    tilemap_stuff[v17 + 97] = tilemap_stuff[v14 + 96];
    tilemap_stuff[v17 + 105] = tilemap_stuff[v14 + 104];
  }
  tilemap_stuff[v14 + 96] = 0;
  tilemap_stuff[v14 + 104] = 0;
  if (sign16(E->dms_var_41 - 94)) {
    int v18 = j >> 1;
    tilemap_stuff[v18 + 113] = tilemap_stuff[v14 + 112];
    tilemap_stuff[v18 + 121] = tilemap_stuff[v14 + 120];
  }
  tilemap_stuff[v14 + 112] = 0;
  tilemap_stuff[v14 + 120] = 0;
  if (sign16(E->dms_var_41 - 94)) {
    int v19 = j >> 1;
    tilemap_stuff[v19 + 129] = tilemap_stuff[v14 + 128];
    tilemap_stuff[v19 + 137] = tilemap_stuff[v14 + 136];
  }
  tilemap_stuff[v14 + 128] = 0;
  tilemap_stuff[v14 + 136] = 0;
  if (E->dms_var_41 >= 0x10) {
    if (sign16(E->dms_var_41 - 94)) {
      int v20 = j >> 1;
      tilemap_stuff[v20 + 145] = tilemap_stuff[v14 + 144];
      tilemap_stuff[v20 + 153] = tilemap_stuff[v14 + 152];
    }
    tilemap_stuff[v14 + 144] = 0;
    tilemap_stuff[v14 + 152] = 0;
  }
}

void Torizo_CorpseRottingCopyFunc(uint16 j, uint16 k) {  // 0xA9E38B
  Enemy_DeadMonsters *E = Get_DeadMonsters(0);
  if (E->dms_var_41 >= 0x50 && sign16(E->dms_var_41 - 94)) {
    int v3 = k >> 1;
    int v4 = j >> 1;
    tilemap_stuff[v4 + 1] = tilemap_stuff[v3];
    tilemap_stuff[v4 + 9] = tilemap_stuff[v3 + 8];
  }
  if (E->dms_var_41 >= 0x48 && sign16(E->dms_var_41 - 94)) {
    int v5 = k >> 1;
    int v6 = j >> 1;
    tilemap_stuff[v6 + 17] = tilemap_stuff[v5 + 16];
    tilemap_stuff[v6 + 25] = tilemap_stuff[v5 + 24];
  }
  if (E->dms_var_41 >= 0x10 && sign16(E->dms_var_41 - 94)) {
    int v7 = k >> 1;
    int v8 = j >> 1;
    tilemap_stuff[v8 + 33] = tilemap_stuff[v7 + 32];
    tilemap_stuff[v8 + 41] = tilemap_stuff[v7 + 40];
  }
  if (sign16(E->dms_var_41 - 94)) {
    int v9 = k >> 1;
    int v10 = j >> 1;
    tilemap_stuff[v10 + 49] = tilemap_stuff[v9 + 48];
    tilemap_stuff[v10 + 57] = tilemap_stuff[v9 + 56];
  }
  if (sign16(E->dms_var_41 - 94)) {
    int v11 = k >> 1;
    int v12 = j >> 1;
    tilemap_stuff[v12 + 65] = tilemap_stuff[v11 + 64];
    tilemap_stuff[v12 + 73] = tilemap_stuff[v11 + 72];
  }
  if (sign16(E->dms_var_41 - 94)) {
    int v13 = k >> 1;
    int v14 = j >> 1;
    tilemap_stuff[v14 + 81] = tilemap_stuff[v13 + 80];
    tilemap_stuff[v14 + 89] = tilemap_stuff[v13 + 88];
  }
  if (sign16(E->dms_var_41 - 94)) {
    int v15 = k >> 1;
    int v16 = j >> 1;
    tilemap_stuff[v16 + 97] = tilemap_stuff[v15 + 96];
    tilemap_stuff[v16 + 105] = tilemap_stuff[v15 + 104];
  }
  if (sign16(E->dms_var_41 - 94)) {
    int v17 = k >> 1;
    int v18 = j >> 1;
    tilemap_stuff[v18 + 113] = tilemap_stuff[v17 + 112];
    tilemap_stuff[v18 + 121] = tilemap_stuff[v17 + 120];
  }
  if (sign16(E->dms_var_41 - 94)) {
    int v19 = k >> 1;
    int v20 = j >> 1;
    tilemap_stuff[v20 + 129] = tilemap_stuff[v19 + 128];
    tilemap_stuff[v20 + 137] = tilemap_stuff[v19 + 136];
  }
  if (E->dms_var_41 >= 0x10) {
    if (sign16(E->dms_var_41 - 94)) {
      int v21 = k >> 1;
      int v22 = j >> 1;
      tilemap_stuff[v22 + 145] = tilemap_stuff[v21 + 144];
      tilemap_stuff[v22 + 153] = tilemap_stuff[v21 + 152];
    }
  }
}

void Sidehopper_CorpseRottingMoveFunc_0(uint16 j, uint16 k) {  // 0xA9E468
  Enemy_DeadMonsters *E = Get_DeadMonsters(0);
  if (E->dms_var_41 >= 8) {
    if (sign16(E->dms_var_41 - 38)) {
      int v3 = k >> 1;
      int v4 = j >> 1;
      tilemap_stuff[v4 + 1] = tilemap_stuff[v3];
      tilemap_stuff[v4 + 9] = tilemap_stuff[v3 + 8];
    }
    int v5 = k >> 1;
    tilemap_stuff[v5] = 0;
    tilemap_stuff[v5 + 8] = 0;
  }
  if (E->dms_var_41 >= 8) {
    if (sign16(E->dms_var_41 - 38)) {
      int v6 = k >> 1;
      int v7 = j >> 1;
      tilemap_stuff[v7 + 17] = tilemap_stuff[v6 + 16];
      tilemap_stuff[v7 + 25] = tilemap_stuff[v6 + 24];
    }
    int v8 = k >> 1;
    tilemap_stuff[v8 + 16] = 0;
    tilemap_stuff[v8 + 24] = 0;
  }
  if (sign16(E->dms_var_41 - 38)) {
    int v9 = k >> 1;
    int v10 = j >> 1;
    tilemap_stuff[v10 + 33] = tilemap_stuff[v9 + 32];
    tilemap_stuff[v10 + 41] = tilemap_stuff[v9 + 40];
  }
  int v11 = k >> 1;
  tilemap_stuff[v11 + 32] = 0;
  tilemap_stuff[v11 + 40] = 0;
  if (sign16(E->dms_var_41 - 38)) {
    int v12 = j >> 1;
    tilemap_stuff[v12 + 49] = tilemap_stuff[v11 + 48];
    tilemap_stuff[v12 + 57] = tilemap_stuff[v11 + 56];
  }
  tilemap_stuff[v11 + 48] = 0;
  tilemap_stuff[v11 + 56] = 0;
  if (sign16(E->dms_var_41 - 38)) {
    int v13 = j >> 1;
    tilemap_stuff[v13 + 65] = tilemap_stuff[v11 + 64];
    tilemap_stuff[v13 + 73] = tilemap_stuff[v11 + 72];
  }
  tilemap_stuff[v11 + 64] = 0;
  tilemap_stuff[v11 + 72] = 0;
}

void Sidehopper_CorpseRottingCopyFunc_0(uint16 j, uint16 k) {  // 0xA9E4F5
  Enemy_DeadMonsters *E = Get_DeadMonsters(0);
  if (E->dms_var_41 >= 8 && sign16(E->dms_var_41 - 38)) {
    int v3 = k >> 1;
    int v4 = j >> 1;
    tilemap_stuff[v4 + 1] = tilemap_stuff[v3];
    tilemap_stuff[v4 + 9] = tilemap_stuff[v3 + 8];
  }
  if (E->dms_var_41 >= 8 && sign16(E->dms_var_41 - 38)) {
    int v5 = k >> 1;
    int v6 = j >> 1;
    tilemap_stuff[v6 + 17] = tilemap_stuff[v5 + 16];
    tilemap_stuff[v6 + 25] = tilemap_stuff[v5 + 24];
  }
  if (sign16(E->dms_var_41 - 38)) {
    int v7 = k >> 1;
    int v8 = j >> 1;
    tilemap_stuff[v8 + 33] = tilemap_stuff[v7 + 32];
    tilemap_stuff[v8 + 41] = tilemap_stuff[v7 + 40];
  }
  if (sign16(E->dms_var_41 - 38)) {
    int v9 = k >> 1;
    int v10 = j >> 1;
    tilemap_stuff[v10 + 49] = tilemap_stuff[v9 + 48];
    tilemap_stuff[v10 + 57] = tilemap_stuff[v9 + 56];
  }
  if (sign16(E->dms_var_41 - 38)) {
    int v11 = k >> 1;
    int v12 = j >> 1;
    tilemap_stuff[v12 + 65] = tilemap_stuff[v11 + 64];
    tilemap_stuff[v12 + 73] = tilemap_stuff[v11 + 72];
  }
}

void Sidehopper_CorpseRottingMoveFunc_2(uint16 j, uint16 k) {  // 0xA9E564
  Enemy_DeadMonsters *E = Get_DeadMonsters(0);
  if (sign16(E->dms_var_41 - 38)) {
    int v3 = k >> 1;
    int v4 = j >> 1;
    tilemap_stuff[v4 + 401] = tilemap_stuff[v3 + 400];
    tilemap_stuff[v4 + 409] = tilemap_stuff[v3 + 408];
  }
  int v5 = k >> 1;
  tilemap_stuff[v5 + 400] = 0;
  tilemap_stuff[v5 + 408] = 0;
  if (sign16(E->dms_var_41 - 38)) {
    int v6 = j >> 1;
    tilemap_stuff[v6 + 417] = tilemap_stuff[v5 + 416];
    tilemap_stuff[v6 + 425] = tilemap_stuff[v5 + 424];
  }
  tilemap_stuff[v5 + 416] = 0;
  tilemap_stuff[v5 + 424] = 0;
  if (E->dms_var_41 >= 8) {
    if (sign16(E->dms_var_41 - 38)) {
      int v7 = j >> 1;
      tilemap_stuff[v7 + 433] = tilemap_stuff[v5 + 432];
      tilemap_stuff[v7 + 441] = tilemap_stuff[v5 + 440];
    }
    tilemap_stuff[v5 + 432] = 0;
    tilemap_stuff[v5 + 440] = 0;
  }
  if (E->dms_var_41 >= 8) {
    if (sign16(E->dms_var_41 - 38)) {
      int v8 = j >> 1;
      tilemap_stuff[v8 + 449] = tilemap_stuff[v5 + 448];
      tilemap_stuff[v8 + 457] = tilemap_stuff[v5 + 456];
    }
    tilemap_stuff[v5 + 448] = 0;
    tilemap_stuff[v5 + 456] = 0;
  }
  if (E->dms_var_41 >= 8) {
    if (sign16(E->dms_var_41 - 38)) {
      int v9 = j >> 1;
      tilemap_stuff[v9 + 465] = tilemap_stuff[v5 + 464];
      tilemap_stuff[v9 + 473] = tilemap_stuff[v5 + 472];
    }
    tilemap_stuff[v5 + 464] = 0;
    tilemap_stuff[v5 + 472] = 0;
  }
}

void Sidehopper_CorpseRottingCopyFunc_2(uint16 j, uint16 k) {  // 0xA9E5F6
  Enemy_DeadMonsters *E = Get_DeadMonsters(0);
  if (sign16(E->dms_var_41 - 38)) {
    int v3 = k >> 1;
    int v4 = j >> 1;
    tilemap_stuff[v4 + 401] = tilemap_stuff[v3 + 400];
    tilemap_stuff[v4 + 409] = tilemap_stuff[v3 + 408];
  }
  if (sign16(E->dms_var_41 - 38)) {
    int v5 = k >> 1;
    int v6 = j >> 1;
    tilemap_stuff[v6 + 417] = tilemap_stuff[v5 + 416];
    tilemap_stuff[v6 + 425] = tilemap_stuff[v5 + 424];
  }
  if (E->dms_var_41 >= 8 && sign16(E->dms_var_41 - 38)) {
    int v7 = k >> 1;
    int v8 = j >> 1;
    tilemap_stuff[v8 + 433] = tilemap_stuff[v7 + 432];
    tilemap_stuff[v8 + 441] = tilemap_stuff[v7 + 440];
  }
  if (E->dms_var_41 >= 8 && sign16(E->dms_var_41 - 38)) {
    int v9 = k >> 1;
    int v10 = j >> 1;
    tilemap_stuff[v10 + 449] = tilemap_stuff[v9 + 448];
    tilemap_stuff[v10 + 457] = tilemap_stuff[v9 + 456];
  }
  if (E->dms_var_41 >= 8) {
    if (sign16(E->dms_var_41 - 38)) {
      int v11 = k >> 1;
      int v12 = j >> 1;
      tilemap_stuff[v12 + 465] = tilemap_stuff[v11 + 464];
      tilemap_stuff[v12 + 473] = tilemap_stuff[v11 + 472];
    }
  }
}

void Zoomer_CorpseRottingMoveFunc_0(uint16 j, uint16 k) {  // 0xA9E66A
  Enemy_DeadMonsters *E = Get_DeadMonsters(0);
  if (sign16(E->dms_var_41 - 14)) {
    int v3 = k >> 1;
    int v4 = j >> 1;
    tilemap_stuff[v4 + 1185] = tilemap_stuff[v3 + 1184];
    tilemap_stuff[v4 + 1193] = tilemap_stuff[v3 + 1192];
  }
  int v5 = k >> 1;
  tilemap_stuff[v5 + 1184] = 0;
  tilemap_stuff[v5 + 1192] = 0;
  if (sign16(E->dms_var_41 - 14)) {
    int v6 = j >> 1;
    tilemap_stuff[v6 + 1201] = tilemap_stuff[v5 + 1200];
    tilemap_stuff[v6 + 1209] = tilemap_stuff[v5 + 1208];
  }
  tilemap_stuff[v5 + 1200] = 0;
  tilemap_stuff[v5 + 1208] = 0;
  if (sign16(E->dms_var_41 - 14)) {
    int v7 = j >> 1;
    tilemap_stuff[v7 + 1217] = tilemap_stuff[v5 + 1216];
    tilemap_stuff[v7 + 1225] = tilemap_stuff[v5 + 1224];
  }
  tilemap_stuff[v5 + 1216] = 0;
  tilemap_stuff[v5 + 1224] = 0;
}

void Zoomer_CorpseRottingCopyFunc_0(uint16 j, uint16 k) {  // 0xA9E6B9
  Enemy_DeadMonsters *E = Get_DeadMonsters(0);
  if (sign16(E->dms_var_41 - 14)) {
    int v3 = k >> 1;
    int v4 = j >> 1;
    tilemap_stuff[v4 + 1185] = tilemap_stuff[v3 + 1184];
    tilemap_stuff[v4 + 1193] = tilemap_stuff[v3 + 1192];
  }
  if (sign16(E->dms_var_41 - 14)) {
    int v5 = k >> 1;
    int v6 = j >> 1;
    tilemap_stuff[v6 + 1201] = tilemap_stuff[v5 + 1200];
    tilemap_stuff[v6 + 1209] = tilemap_stuff[v5 + 1208];
  }
  if (sign16(E->dms_var_41 - 14)) {
    int v7 = k >> 1;
    int v8 = j >> 1;
    tilemap_stuff[v8 + 1217] = tilemap_stuff[v7 + 1216];
    tilemap_stuff[v8 + 1225] = tilemap_stuff[v7 + 1224];
  }
}

void Zoomer_CorpseRottingMoveFunc_2(uint16 j, uint16 k) {  // 0xA9E6F6
  Enemy_DeadMonsters *E = Get_DeadMonsters(0);
  if (sign16(E->dms_var_41 - 14)) {
    int v3 = k >> 1;
    int v4 = j >> 1;
    tilemap_stuff[v4 + 1281] = tilemap_stuff[v3 + 1280];
    tilemap_stuff[v4 + 1289] = tilemap_stuff[v3 + 1288];
  }
  int v5 = k >> 1;
  tilemap_stuff[v5 + 1280] = 0;
  tilemap_stuff[v5 + 1288] = 0;
  if (sign16(E->dms_var_41 - 14)) {
    int v6 = j >> 1;
    tilemap_stuff[v6 + 1297] = tilemap_stuff[v5 + 1296];
    tilemap_stuff[v6 + 1305] = tilemap_stuff[v5 + 1304];
  }
  tilemap_stuff[v5 + 1296] = 0;
  tilemap_stuff[v5 + 1304] = 0;
  if (sign16(E->dms_var_41 - 14)) {
    int v7 = j >> 1;
    tilemap_stuff[v7 + 1313] = tilemap_stuff[v5 + 1312];
    tilemap_stuff[v7 + 1321] = tilemap_stuff[v5 + 1320];
  }
  tilemap_stuff[v5 + 1312] = 0;
  tilemap_stuff[v5 + 1320] = 0;
}

void Zoomer_CorpseRottingCopyFunc_2(uint16 j, uint16 k) {  // 0xA9E745
  Enemy_DeadMonsters *E = Get_DeadMonsters(0);
  if (sign16(E->dms_var_41 - 14)) {
    int v3 = k >> 1;
    int v4 = j >> 1;
    tilemap_stuff[v4 + 1281] = tilemap_stuff[v3 + 1280];
    tilemap_stuff[v4 + 1289] = tilemap_stuff[v3 + 1288];
  }
  if (sign16(E->dms_var_41 - 14)) {
    int v5 = k >> 1;
    int v6 = j >> 1;
    tilemap_stuff[v6 + 1297] = tilemap_stuff[v5 + 1296];
    tilemap_stuff[v6 + 1305] = tilemap_stuff[v5 + 1304];
  }
  if (sign16(E->dms_var_41 - 14)) {
    int v7 = k >> 1;
    int v8 = j >> 1;
    tilemap_stuff[v8 + 1313] = tilemap_stuff[v7 + 1312];
    tilemap_stuff[v8 + 1321] = tilemap_stuff[v7 + 1320];
  }
}

void Zoomer_CorpseRottingMoveFunc_4(uint16 j, uint16 k) {  // 0xA9E782
  Enemy_DeadMonsters *E = Get_DeadMonsters(0);
  if (sign16(E->dms_var_41 - 14)) {
    int v3 = k >> 1;
    int v4 = j >> 1;
    tilemap_stuff[v4 + 1377] = tilemap_stuff[v3 + 1376];
    tilemap_stuff[v4 + 1385] = tilemap_stuff[v3 + 1384];
  }
  int v5 = k >> 1;
  tilemap_stuff[v5 + 1376] = 0;
  tilemap_stuff[v5 + 1384] = 0;
  if (sign16(E->dms_var_41 - 14)) {
    int v6 = j >> 1;
    tilemap_stuff[v6 + 1393] = tilemap_stuff[v5 + 1392];
    tilemap_stuff[v6 + 1401] = tilemap_stuff[v5 + 1400];
  }
  tilemap_stuff[v5 + 1392] = 0;
  tilemap_stuff[v5 + 1400] = 0;
  if (sign16(E->dms_var_41 - 14)) {
    int v7 = j >> 1;
    tilemap_stuff[v7 + 1409] = tilemap_stuff[v5 + 1408];
    tilemap_stuff[v7 + 1417] = tilemap_stuff[v5 + 1416];
  }
  tilemap_stuff[v5 + 1408] = 0;
  tilemap_stuff[v5 + 1416] = 0;
}

void Zoomer_CorpseRottingCopyFunc_4(uint16 j, uint16 k) {  // 0xA9E7D1
  Enemy_DeadMonsters *E = Get_DeadMonsters(0);
  if (sign16(E->dms_var_41 - 14)) {
    int v3 = k >> 1;
    int v4 = j >> 1;
    tilemap_stuff[v4 + 1377] = tilemap_stuff[v3 + 1376];
    tilemap_stuff[v4 + 1385] = tilemap_stuff[v3 + 1384];
  }
  if (sign16(E->dms_var_41 - 14)) {
    int v5 = k >> 1;
    int v6 = j >> 1;
    tilemap_stuff[v6 + 1393] = tilemap_stuff[v5 + 1392];
    tilemap_stuff[v6 + 1401] = tilemap_stuff[v5 + 1400];
  }
  if (sign16(E->dms_var_41 - 14)) {
    int v7 = k >> 1;
    int v8 = j >> 1;
    tilemap_stuff[v8 + 1409] = tilemap_stuff[v7 + 1408];
    tilemap_stuff[v8 + 1417] = tilemap_stuff[v7 + 1416];
  }
}

void Ripper_CorpseRottingMoveFunc_0(uint16 j, uint16 k) {  // 0xA9E80E
  Enemy_DeadMonsters *E = Get_DeadMonsters(0);
  if (sign16(E->dms_var_41 - 14)) {
    int v3 = k >> 1;
    int v4 = j >> 1;
    tilemap_stuff[v4 + 1473] = tilemap_stuff[v3 + 1472];
    tilemap_stuff[v4 + 1481] = tilemap_stuff[v3 + 1480];
  }
  int v5 = k >> 1;
  tilemap_stuff[v5 + 1472] = 0;
  tilemap_stuff[v5 + 1480] = 0;
  if (sign16(E->dms_var_41 - 14)) {
    int v6 = j >> 1;
    tilemap_stuff[v6 + 1489] = tilemap_stuff[v5 + 1488];
    tilemap_stuff[v6 + 1497] = tilemap_stuff[v5 + 1496];
  }
  tilemap_stuff[v5 + 1488] = 0;
  tilemap_stuff[v5 + 1496] = 0;
  if (sign16(E->dms_var_41 - 14)) {
    int v7 = j >> 1;
    tilemap_stuff[v7 + 1505] = tilemap_stuff[v5 + 1504];
    tilemap_stuff[v7 + 1513] = tilemap_stuff[v5 + 1512];
  }
  tilemap_stuff[v5 + 1504] = 0;
  tilemap_stuff[v5 + 1512] = 0;
}

void Ripper_CorpseRottingCopyFunc_0(uint16 j, uint16 k) {  // 0xA9E85D
  Enemy_DeadMonsters *E = Get_DeadMonsters(0);
  if (sign16(E->dms_var_41 - 14)) {
    int v3 = k >> 1;
    int v4 = j >> 1;
    tilemap_stuff[v4 + 1473] = tilemap_stuff[v3 + 1472];
    tilemap_stuff[v4 + 1481] = tilemap_stuff[v3 + 1480];
  }
  if (sign16(E->dms_var_41 - 14)) {
    int v5 = k >> 1;
    int v6 = j >> 1;
    tilemap_stuff[v6 + 1489] = tilemap_stuff[v5 + 1488];
    tilemap_stuff[v6 + 1497] = tilemap_stuff[v5 + 1496];
  }
  if (sign16(E->dms_var_41 - 14)) {
    int v7 = k >> 1;
    int v8 = j >> 1;
    tilemap_stuff[v8 + 1505] = tilemap_stuff[v7 + 1504];
    tilemap_stuff[v8 + 1513] = tilemap_stuff[v7 + 1512];
  }
}

void Ripper_CorpseRottingMoveFunc_2(uint16 j, uint16 k) {  // 0xA9E89A
  Enemy_DeadMonsters *E = Get_DeadMonsters(0);
  if (sign16(E->dms_var_41 - 14)) {
    int v3 = k >> 1;
    int v4 = j >> 1;
    tilemap_stuff[v4 + 1569] = tilemap_stuff[v3 + 1568];
    tilemap_stuff[v4 + 1577] = tilemap_stuff[v3 + 1576];
  }
  int v5 = k >> 1;
  tilemap_stuff[v5 + 1568] = 0;
  tilemap_stuff[v5 + 1576] = 0;
  if (sign16(E->dms_var_41 - 14)) {
    int v6 = j >> 1;
    tilemap_stuff[v6 + 1585] = tilemap_stuff[v5 + 1584];
    tilemap_stuff[v6 + 1593] = tilemap_stuff[v5 + 1592];
  }
  tilemap_stuff[v5 + 1584] = 0;
  tilemap_stuff[v5 + 1592] = 0;
  if (sign16(E->dms_var_41 - 14)) {
    int v7 = j >> 1;
    tilemap_stuff[v7 + 1601] = tilemap_stuff[v5 + 1600];
    tilemap_stuff[v7 + 1609] = tilemap_stuff[v5 + 1608];
  }
  tilemap_stuff[v5 + 1600] = 0;
  tilemap_stuff[v5 + 1608] = 0;
}

void Ripper_CorpseRottingCopyFunc_2(uint16 j, uint16 k) {  // 0xA9E8E9
  Enemy_DeadMonsters *E = Get_DeadMonsters(0);
  if (sign16(E->dms_var_41 - 14)) {
    int v3 = k >> 1;
    int v4 = j >> 1;
    tilemap_stuff[v4 + 1569] = tilemap_stuff[v3 + 1568];
    tilemap_stuff[v4 + 1577] = tilemap_stuff[v3 + 1576];
  }
  if (sign16(E->dms_var_41 - 14)) {
    int v5 = k >> 1;
    int v6 = j >> 1;
    tilemap_stuff[v6 + 1585] = tilemap_stuff[v5 + 1584];
    tilemap_stuff[v6 + 1593] = tilemap_stuff[v5 + 1592];
  }
  if (sign16(E->dms_var_41 - 14)) {
    int v7 = k >> 1;
    int v8 = j >> 1;
    tilemap_stuff[v8 + 1601] = tilemap_stuff[v7 + 1600];
    tilemap_stuff[v8 + 1609] = tilemap_stuff[v7 + 1608];
  }
}

void Skree_CorpseRottingMoveFunc_0(uint16 j, uint16 k) {  // 0xA9E926
  Enemy_DeadMonsters *E = Get_DeadMonsters(0);
  if (sign16(E->dms_var_41 - 30)) {
    int v3 = k >> 1;
    int v4 = j >> 1;
    tilemap_stuff[v4 + 801] = tilemap_stuff[v3 + 800];
    tilemap_stuff[v4 + 809] = tilemap_stuff[v3 + 808];
  }
  int v5 = k >> 1;
  tilemap_stuff[v5 + 800] = 0;
  tilemap_stuff[v5 + 808] = 0;
  if (sign16(E->dms_var_41 - 30)) {
    int v6 = j >> 1;
    tilemap_stuff[v6 + 817] = tilemap_stuff[v5 + 816];
    tilemap_stuff[v6 + 825] = tilemap_stuff[v5 + 824];
  }
  tilemap_stuff[v5 + 816] = 0;
  tilemap_stuff[v5 + 824] = 0;
}

void Skree_CorpseRottingCopyFunc_0(uint16 j, uint16 k) {  // 0xA9E95B
  Enemy_DeadMonsters *E = Get_DeadMonsters(0);
  if (sign16(E->dms_var_41 - 30)) {
    int v3 = k >> 1;
    int v4 = j >> 1;
    tilemap_stuff[v4 + 801] = tilemap_stuff[v3 + 800];
    tilemap_stuff[v4 + 809] = tilemap_stuff[v3 + 808];
  }
  if (sign16(E->dms_var_41 - 30)) {
    int v5 = k >> 1;
    int v6 = j >> 1;
    tilemap_stuff[v6 + 817] = tilemap_stuff[v5 + 816];
    tilemap_stuff[v6 + 825] = tilemap_stuff[v5 + 824];
  }
}

void Skree_CorpseRottingMoveFunc_2(uint16 j, uint16 k) {  // 0xA9E984
  Enemy_DeadMonsters *E = Get_DeadMonsters(0);
  if (sign16(E->dms_var_41 - 30)) {
    int v3 = k >> 1;
    int v4 = j >> 1;
    tilemap_stuff[v4 + 929] = tilemap_stuff[v3 + 928];
    tilemap_stuff[v4 + 937] = tilemap_stuff[v3 + 936];
  }
  int v5 = k >> 1;
  tilemap_stuff[v5 + 928] = 0;
  tilemap_stuff[v5 + 936] = 0;
  if (sign16(E->dms_var_41 - 30)) {
    int v6 = j >> 1;
    tilemap_stuff[v6 + 945] = tilemap_stuff[v5 + 944];
    tilemap_stuff[v6 + 953] = tilemap_stuff[v5 + 952];
  }
  tilemap_stuff[v5 + 944] = 0;
  tilemap_stuff[v5 + 952] = 0;
}

void Skree_CorpseRottingCopyFunc_2(uint16 j, uint16 k) {  // 0xA9E9B9
  Enemy_DeadMonsters *E = Get_DeadMonsters(0);
  if (sign16(E->dms_var_41 - 30)) {
    int v3 = k >> 1;
    int v4 = j >> 1;
    tilemap_stuff[v4 + 929] = tilemap_stuff[v3 + 928];
    tilemap_stuff[v4 + 937] = tilemap_stuff[v3 + 936];
  }
  if (sign16(E->dms_var_41 - 30)) {
    int v5 = k >> 1;
    int v6 = j >> 1;
    tilemap_stuff[v6 + 945] = tilemap_stuff[v5 + 944];
    tilemap_stuff[v6 + 953] = tilemap_stuff[v5 + 952];
  }
}

void Skree_CorpseRottingMoveFunc_4(uint16 j, uint16 k) {  // 0xA9E9E2
  Enemy_DeadMonsters *E = Get_DeadMonsters(0);
  if (sign16(E->dms_var_41 - 30)) {
    int v3 = k >> 1;
    int v4 = j >> 1;
    tilemap_stuff[v4 + 1057] = tilemap_stuff[v3 + 1056];
    tilemap_stuff[v4 + 1065] = tilemap_stuff[v3 + 1064];
  }
  int v5 = k >> 1;
  tilemap_stuff[v5 + 1056] = 0;
  tilemap_stuff[v5 + 1064] = 0;
  if (sign16(E->dms_var_41 - 30)) {
    int v6 = j >> 1;
    tilemap_stuff[v6 + 1073] = tilemap_stuff[v5 + 1072];
    tilemap_stuff[v6 + 1081] = tilemap_stuff[v5 + 1080];
  }
  tilemap_stuff[v5 + 1072] = 0;
  tilemap_stuff[v5 + 1080] = 0;
}

void Skree_CorpseRottingCopyFunc_4(uint16 j, uint16 k) {  // 0xA9EA17
  Enemy_DeadMonsters *E = Get_DeadMonsters(0);
  if (sign16(E->dms_var_41 - 30)) {
    int v3 = k >> 1;
    int v4 = j >> 1;
    tilemap_stuff[v4 + 1057] = tilemap_stuff[v3 + 1056];
    tilemap_stuff[v4 + 1065] = tilemap_stuff[v3 + 1064];
  }
  if (sign16(E->dms_var_41 - 30)) {
    int v5 = k >> 1;
    int v6 = j >> 1;
    tilemap_stuff[v6 + 1073] = tilemap_stuff[v5 + 1072];
    tilemap_stuff[v6 + 1081] = tilemap_stuff[v5 + 1080];
  }
}

const uint16 *sub_A9ECD0(uint16 k, const uint16 *jp) {  // 0xA9ECD0
  uint16 v2 = FUNC16(DeadMonsters_Func_1);
  Enemy_DeadMonsters *E = Get_DeadMonsters(k);
  if (E->dms_var_08)
    v2 = FUNC16(DeadMonsters_Func_5);
  E->dms_var_A = v2;
  return jp;
}
