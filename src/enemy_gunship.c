// Gunship enemy runtime extracted from sm_a2.c: landing-site ship idle,
// save/heal interaction, event-driven departure, and takeoff choreography.

#include "ida_types.h"
#include "enemy_types.h"
#include "variables.h"
#include "funcs.h"

#define g_byte_A2A7CF ((uint8*)RomFixedPtr(0xa2a7cf))
#define g_word_A2A622 ((uint16*)RomFixedPtr(0xa2a622))
#define g_word_A2AC07 ((uint16*)RomFixedPtr(0xa2ac07))
#define g_word_A2AC11 ((uint16*)RomFixedPtr(0xa2ac11))

static uint16 GunshipPrevPieceVramTilesIndex(uint16 enemy_index) {
  return enemy_index >= 64 ? Get_GunshipTop(enemy_index - 64)->base.vram_tiles_index : 0;
}

static uint16 GunshipTopPieceYPos(uint16 enemy_index) {
  return enemy_index >= 128 ? Get_GunshipTop(enemy_index - 128)->base.y_pos : 0;
}

void GunshipTop_Init(void) {  // 0xA2A644
  Enemy_GunshipTop *E = Get_GunshipTop(cur_enemy_index);
  E->base.properties |= kEnemyProps_DisableSamusColl | kEnemyProps_Tangible;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  E->base.current_instruction = addr_kGunshipTop_Ilist_A616;
  E->base.palette_index = 3584;
  if (game_state != 40 || demo_set) {
    if (loading_game_state == kLoadingGameState_22_EscapingCeres) {
      E->base.y_pos = samus_y_pos - 17;
      E->gtp_var_F = FUNC16(GunshipTop_3);
    } else {
      uint16 v3 = E->base.y_pos - 25;
      E->base.y_pos = v3;
      E->gtp_var_E = v3;
      E->gtp_var_F = FUNC16(GunshipTop_8);
    }
  } else {
    samus_y_pos = 1138;
    uint16 v5 = E->base.y_pos - 25;
    E->base.y_pos = v5;
    E->gtp_var_E = v5;
    E->gtp_var_F = FUNC16(GunshipTop_14);
    Get_GunshipTop(0)->gtp_var_A = 144;
  }
  SpawnPalfxObject(addr_stru_8DE1C0);
  Get_GunshipTop(cur_enemy_index)->gtp_var_D = 1;
  Get_GunshipTop(0)->gtp_var_C = 0;
}

void GunshipBottom_Init(void) {  // 0xA2A6D2
  Enemy_GunshipBottom *E = Get_GunshipBottom(cur_enemy_index);
  E->base.properties |= kEnemyProps_DisableSamusColl | kEnemyProps_Tangible;
  E->base.instruction_timer = 1;
  E->base.timer = 0;
  if (E->gbm_parameter_2)
    E->base.current_instruction = addr_kGunshipTop_Ilist_A60E;
  else
    E->base.current_instruction = addr_kGunshipTop_Ilist_A61C;
  E->base.vram_tiles_index = GunshipPrevPieceVramTilesIndex(cur_enemy_index);
  E->base.palette_index = 3584;
  if (E->gbm_parameter_2) {
    E->base.y_pos = GunshipTopPieceYPos(cur_enemy_index) - 1;
    if (game_state == kPose_28_FaceL_Crouch && !demo_set) {
      E->base.instruction_timer = 1;
      E->base.current_instruction = addr_kGunshipTop_Ilist_A5BE;
    }
  } else if (loading_game_state == kGameState_34_CeresGoesBoom) {
    E->base.y_pos = samus_y_pos + 23;
  } else {
    E->base.y_pos += 15;
    E->gbm_var_D = 71;
  }
  E->gbm_var_F = FUNC16(nullsub_187);
}

void GunshipTop_Main(void) {  // 0xA2A759
  Enemy_GunshipTop *E1 = Get_GunshipTop(cur_enemy_index + 64);
  bool v2 = E1->gtp_var_D == 1;
  bool v3 = (--E1->gtp_var_D & 0x8000) != 0;
  if (v2 || v3) {
    QueueSfx2_Max6(0x4D);
    E1->gtp_var_D = 70;
  }
  Enemy_GunshipTop *E0 = Get_GunshipTop(cur_enemy_index);
  if (!sign16(E0->gtp_var_F + 0x56BE) && sign16(E0->gtp_var_F + 0x53E5))
    GunshipTop_1(cur_enemy_index);
  CallEnemyPreInstr(E0->gtp_var_F | 0xA20000);
}

void GunshipTop_1(uint16 k) {  // 0xA2A784
  Enemy_GunshipTop *E0 = Get_GunshipTop(k);
  bool v3 = E0->gtp_var_D == 1;
  bool v4 = (--E0->gtp_var_D & 0x8000) != 0;
  if (v3 || v4) {
    uint16 v5 = 2 * E0->gtp_var_C;
    E0->gtp_var_D = g_byte_A2A7CF[v5];
    uint16 v6 = (int8)g_byte_A2A7CF[v5 + 1];
    E0->base.y_pos += v6;
    Enemy_GunshipTop *E1 = Get_GunshipTop(k + 64);
    E1->base.y_pos += v6;
    Enemy_GunshipTop *E2 = Get_GunshipTop(k + 128);
    E2->base.y_pos += v6;
    E0->gtp_var_C = (E0->gtp_var_C + 1) & 3;
  }
}

void GunshipTop_2(uint16 k) {  // 0xA2A7D8
  samus_y_pos -= 8;
  Enemy_GunshipTop *E2 = Get_GunshipTop(k + 128);
  E2->base.y_pos -= 8;
  Enemy_GunshipTop *E1 = Get_GunshipTop(k + 64);
  E1->base.y_pos -= 8;
  Enemy_GunshipTop *E0 = Get_GunshipTop(k);
  uint16 v4 = E0->base.y_pos - 8;
  E0->base.y_pos = v4;
  if (sign16(v4 - 128))
    E0->gtp_var_F = FUNC16(GunshipTop_3);
}

void GunshipTop_3(uint16 k) {  // 0xA2A80C
  Enemy_GunshipTop *E = Get_GunshipTop(k);
  if (sign16(E->base.y_pos - 768)) {
    AddToHiLo(&samus_y_pos, &samus_y_subpos, 0x48000);
    Enemy_GunshipTop *v3 = Get_GunshipTop(k + 128);
    AddToHiLo(&v3->base.y_pos, &v3->base.y_subpos, 0x48000);
    Enemy_GunshipTop *v5 = Get_GunshipTop(k + 64);
    AddToHiLo(&v5->base.y_pos, &v5->base.y_subpos, 0x48000);
    AddToHiLo(&E->base.y_pos, &E->base.y_subpos, 0x48000);
  } else {
    AddToHiLo(&samus_y_pos, &samus_y_subpos, 0x28000);
    Enemy_GunshipTop *v8 = Get_GunshipTop(k + 128);
    AddToHiLo(&v8->base.y_pos, &v8->base.y_subpos, 0x28000);
    Enemy_GunshipTop *v10 = Get_GunshipTop(k + 64);
    AddToHiLo(&v10->base.y_pos, &v10->base.y_subpos, 0x28000);
    AddToHiLo(&E->base.y_pos, &E->base.y_subpos, 0x28000);
    if (!sign16(E->base.y_pos - 1119)) {
      E->base.y_pos = 1119;
      v10->base.y_pos = 1159;
      v8->base.y_pos = E->base.y_pos - 1;
      E->gtp_var_F = FUNC16(GunshipTop_4);
      E->gtp_var_E = 0;
    }
  }
}

void GunshipTop_4(uint16 k) {  // 0xA2A8D0
  int v2;
  uint16 v5;

  Enemy_GunshipTop *GunshipTop = Get_GunshipTop(k);
  v2 = GunshipTop->gtp_var_E;
  samus_y_pos += g_word_A2A622[v2];
  Enemy_GunshipTop *v3 = Get_GunshipTop(k + 128);
  v3->base.y_pos += g_word_A2A622[v2];
  Enemy_GunshipTop *v4 = Get_GunshipTop(k + 64);
  v4->base.y_pos += g_word_A2A622[v2];
  GunshipTop->base.y_pos += g_word_A2A622[v2];
  v5 = GunshipTop->gtp_var_E + 1;
  GunshipTop->gtp_var_E = v5;
  if (!sign16(v5 - 17)) {
    GunshipTop->gtp_var_F = FUNC16(GunshipTop_5);
    GunshipTop->gtp_var_E = GunshipTop->base.y_pos;
    GunshipTop->gtp_var_D = 1;
    Enemy_GunshipTop *v6 = Get_GunshipTop(0);
    v6->gtp_var_C = 0;
    samus_x_pos = GunshipTop->base.x_pos + 1;
    samus_prev_x_pos = samus_x_pos;
    Enemy_GunshipTop *v7 = Get_GunshipTop(k + 128);
    v7->base.instruction_timer = 1;
    v7->base.current_instruction = addr_kGunshipTop_Ilist_A5BE;
    v6->gtp_var_A = 144;
    QueueSfx3_Max6(0x14);
  }
}

void GunshipTop_5(uint16 k) {  // 0xA2A942
  Enemy_GunshipTop *E = Get_GunshipTop(0);
  bool v2 = E->gtp_var_A == 1;
  bool v3 = (--E->gtp_var_A & 0x8000) != 0;
  if (v2 || v3)
    Get_GunshipTop(k)->gtp_var_F = FUNC16(GunshipTop_6);
}

void GunshipTop_6(uint16 k) {  // 0xA2A950
  Enemy_GunshipTop *E = Get_GunshipTop(k);
  uint16 r18 = E->gtp_var_E - 30;
  if (sign16(--samus_y_pos - r18)) {
    E->gtp_var_F = FUNC16(GunshipTop_7);
    Enemy_GunshipTop *E2 = Get_GunshipTop(k + 128);
    E2->base.instruction_timer = 1;
    E2->base.current_instruction = addr_kGunshipTop_Ilist_A5EE;
    Get_GunshipTop(0)->gtp_var_A = 144;
    QueueSfx3_Max6(0x15);
  }
}

void GunshipTop_7(uint16 k) {  // 0xA2A987
  Enemy_GunshipTop *E = Get_GunshipTop(0);
  bool v2 = E->gtp_var_A == 1;
  bool v3 = (--E->gtp_var_A & 0x8000) != 0;
  if (v2 || v3) {
    Get_GunshipTop(k)->gtp_var_F = FUNC16(GunshipTop_8);
    frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func11);
    frame_handler_beta = FUNC16(Samus_FrameHandlerBeta_Func17);
    loading_game_state = kLoadingGameState_5_Main;
    *(uint16 *)used_save_stations_and_elevators |= 1;
    load_station_index = 0;
    SaveToSram(selected_save_slot);
  }
}

void GunshipTop_8(uint16 k) {  // 0xA2A9BD
  if (game_state == 8 && frame_handler_alfa == FUNC16(Samus_FrameHandlerAlfa_Func11)) {
    Enemy_GunshipTop *E = Get_GunshipTop(k);
    if ((int16)(E->base.x_pos - 8 - samus_x_pos) < 0
        && (int16)(E->base.x_pos + 8 - samus_x_pos) >= 0
        && (int16)(E->base.y_pos - 64 - samus_y_pos) < 0
        && (int16)(E->base.y_pos - samus_y_pos) >= 0
        && !samus_movement_type
        && (joypad1_newkeys & kButton_Down) != 0) {
      E->gtp_var_F = FUNC16(GunshipTop_9);
      if (samus_x_pos != 1152) {
        samus_x_pos = E->base.x_pos;
        samus_prev_x_pos = samus_x_pos;
      }
      MakeSamusFaceForward();
      CallSomeSamusCode(0x1A);
      elevator_status = 0;
      uint16 v3 = E->base.y_pos - 1;
      Enemy_GunshipTop *E2 = Get_GunshipTop(k + 128);
      E2->base.y_pos = v3;
      E2->base.instruction_timer = 1;
      E2->base.current_instruction = addr_kGunshipTop_Ilist_A5BE;
      Get_GunshipTop(0)->gtp_var_A = 144;
      QueueSfx3_Max6(0x14);
    }
  }
}

void GunshipTop_9(uint16 k) {  // 0xA2AA4F
  Enemy_GunshipTop *E = Get_GunshipTop(0);
  bool v2 = E->gtp_var_A == 1;
  bool v3 = (--E->gtp_var_A & 0x8000) != 0;
  if (v2 || v3)
    Get_GunshipTop(k)->gtp_var_F = FUNC16(GunshipTop_10);
}

void GunshipTop_10(uint16 k) {  // 0xA2AA5D
  Enemy_GunshipTop *E = Get_GunshipTop(k);
  uint16 r18 = E->gtp_var_E + 18;
  samus_y_pos += 2;
  if (!sign16(samus_y_pos - r18)) {
    E->gtp_var_F = FUNC16(GunshipTop_11);
    Enemy_GunshipTop *E2 = Get_GunshipTop(k + 128);
    E2->base.instruction_timer = 1;
    E2->base.current_instruction = addr_kGunshipTop_Ilist_A5EE;
    Get_GunshipTop(0)->gtp_var_A = 144;
    QueueSfx3_Max6(0x15);
  }
}

void GunshipTop_11(uint16 k) {  // 0xA2AA94
  Enemy_GunshipTop *E = Get_GunshipTop(0);
  bool v2 = E->gtp_var_A == 1;
  bool v3 = (--E->gtp_var_A & 0x8000) != 0;
  if (v2 || v3)
    Get_GunshipTop(k)->gtp_var_F = FUNC16(GunshipTop_12);
}

void GunshipTop_12(uint16 k) {  // 0xA2AAA2
  if (CheckEventHappened(0xE)) {
    Get_GunshipTop(k)->gtp_var_F = FUNC16(GunshipTop_17);
    Enemy_GunshipTop *E = Get_GunshipTop(k + 64);
    E->gtp_var_F = 0;
    E->gtp_var_E = 0;
    substate = 0;
    suit_pickup_light_beam_pos = 0;
    *(uint16 *)&suit_pickup_color_math_R = 0;
    *(uint16 *)&suit_pickup_color_math_B = 0;
    CallSomeSamusCode(0xA);
  } else {
    Samus_RestoreHealth(2);
    Samus_RestoreMissiles(2);
    Samus_RestoreSuperMissiles(2);
    Samus_RestorePowerBombs(2);
    if ((int16)(samus_reserve_health - samus_max_reserve_health) >= 0
        && (int16)(samus_health - samus_max_health) >= 0
        && (int16)(samus_missiles - samus_max_missiles) >= 0
        && (int16)(samus_super_missiles - samus_max_super_missiles) >= 0
        && (int16)(samus_power_bombs - samus_max_power_bombs) >= 0) {
      Get_GunshipTop(k)->gtp_var_F = FUNC16(GunshipTop_13);
    }
  }
}

void GunshipTop_13(uint16 k) {  // 0xA2AB1F
  int rv = DisplayMessageBox_Poll(0x1c);
  if (rv < 0)
    return;

  if (rv != 2) {
    *(uint16 *)used_save_stations_and_elevators |= 1;
    load_station_index = 0;
    SaveToSram(selected_save_slot);
  }
  Get_GunshipTop(k)->gtp_var_F = FUNC16(GunshipTop_14);
  Enemy_GunshipTop *E = Get_GunshipTop(k + 128);
  E->base.instruction_timer = 1;
  E->base.current_instruction = addr_kGunshipTop_Ilist_A5BE;
  Get_GunshipTop(0)->gtp_var_A = 144;
  QueueSfx3_Max6(0x14);
}

void GunshipTop_14(uint16 k) {  // 0xA2AB60
  Enemy_GunshipTop *E = Get_GunshipTop(0);
  bool v2 = E->gtp_var_A == 1;
  bool v3 = (--E->gtp_var_A & 0x8000) != 0;
  if (v2 || v3)
    Get_GunshipTop(k)->gtp_var_F = FUNC16(GunshipTop_15);
}

void GunshipTop_15(uint16 k) {  // 0xA2AB6E
  Enemy_GunshipTop *E = Get_GunshipTop(k);
  uint16 r18 = E->gtp_var_E - 30;
  samus_y_pos -= 2;
  if (sign16(samus_y_pos - r18)) {
    E->gtp_var_F = FUNC16(GunshipTop_16);
    Enemy_GunshipTop *E2 = Get_GunshipTop(k + 128);
    E2->base.instruction_timer = 1;
    E2->base.current_instruction = addr_kGunshipTop_Ilist_A5EE;
    Get_GunshipTop(0)->gtp_var_A = 144;
    QueueSfx3_Max6(0x15);
  }
}

void GunshipTop_16(uint16 k) {  // 0xA2ABA5
  Enemy_GunshipTop *E = Get_GunshipTop(0);
  bool v2 = E->gtp_var_A == 1;
  bool v3 = (--E->gtp_var_A & 0x8000) != 0;
  if (v2 || v3) {
    Get_GunshipTop(k)->gtp_var_F = FUNC16(GunshipTop_8);
    if (sign16(game_state - kGameState_40_TransitionToDemo)) {
      frame_handler_alfa = FUNC16(Samus_FrameHandlerAlfa_Func11);
      frame_handler_beta = FUNC16(Samus_FrameHandlerBeta_Func17);
    }
  }
}

void GunshipTop_17(uint16 k) {  // 0xA2ABC7
  uint16 v1 = substate;
  uint16 v2 = vram_write_queue_tail;
  gVramWriteEntry(vram_write_queue_tail)->size = 1024;
  v2 += 2;
  int v3 = v1 >> 1;
  gVramWriteEntry(v2)->size = g_word_A2AC07[v3];
  v2 += 2;
  LOBYTE(gVramWriteEntry(v2++)->size) = -108;
  gVramWriteEntry(v2)->size = g_word_A2AC11[v3];
  vram_write_queue_tail = v2 + 2;
  substate += 2;
  if (!sign16(substate - 10)) {
    Get_GunshipTop(k)->gtp_var_F = FUNC16(GunshipTop_18);
    substate = 0;
  }
}

void GunshipTop_18(uint16 k) {  // 0xA2AC1B
  int16 v5;

  Enemy_GunshipTop *E1 = Get_GunshipTop(k + 64);
  uint16 gtp_var_E = E1->gtp_var_E;
  if (sign16(gtp_var_E - 64)) {
    uint16 v3;
    if ((gtp_var_E & 1) != 0)
      v3 = samus_y_pos + 1;
    else
      v3 = samus_y_pos - 1;
    samus_y_pos = v3;
    samus_prev_y_pos = v3;
  } else {
    uint16 v4;
    if ((gtp_var_E & 1) != 0)
      v4 = samus_y_pos + 2;
    else
      v4 = samus_y_pos - 2;
    samus_y_pos = v4;
    samus_prev_y_pos = v4;
  }
  v5 = samus_y_pos - 17;
  Enemy_GunshipTop *E0 = Get_GunshipTop(k);
  E0->base.y_pos = samus_y_pos - 17;
  Get_GunshipTop(k + 128)->base.y_pos = v5 - 1;
  E1->base.y_pos = samus_y_pos + 23;
  uint16 v7 = E1->gtp_var_E + 1;
  E1->gtp_var_E = v7;
  if (sign16(v7 - 128)) {
    if (v7 == 64) {
      SpawnEprojWithRoomGfx(addr_stru_86A379, 0);
      SpawnEprojWithRoomGfx(addr_stru_86A379, 2);
      SpawnEprojWithRoomGfx(addr_stru_86A379, 4);
      SpawnEprojWithRoomGfx(addr_stru_86A379, 6);
      SpawnEprojWithRoomGfx(addr_stru_86A379, 8);
      SpawnEprojWithRoomGfx(addr_stru_86A379, 0xA);
    }
  } else {
    E0->gtp_var_F = FUNC16(GunshipTop_19);
    Get_GunshipTop(0)->gtp_var_A = 0;
  }
}

void GunshipTop_19(uint16 k) {  // 0xA2ACD7
  int16 v1;

  samus_y_pos -= 2;
  v1 = samus_y_pos - 17;
  Enemy_GunshipTop *E0 = Get_GunshipTop(k);
  E0->base.y_pos = samus_y_pos - 17;
  Get_GunshipTop(k + 128)->base.y_pos = v1 - 1;
  Enemy_GunshipTop *E1 = Get_GunshipTop(k + 64);
  E1->base.y_pos = samus_y_pos + 23;
  if (sign16(E0->base.y_pos - 896)) {
    E0->gtp_var_F = FUNC16(GunshipTop_20);
    E1->gtp_var_F = 512;
  }
}

void GunshipTop_20(uint16 k) {  // 0xA2AD0E
  GunshipTop_21(k);
  Enemy_GunshipTop *E = Get_GunshipTop(k);
  if (sign16(E->base.y_pos - 256)) {
    E->gtp_var_F = FUNC16(GunshipTop_21);
    game_state = 38;
    screen_fade_delay = 0;
    screen_fade_counter = 0;
  }
}

void GunshipTop_21(uint16 k) {  // 0xA2AD2D
  Enemy_GunshipTop *E = Get_GunshipTop(k + 64);
  uint16 v3 = E->gtp_var_F + 64;
  E->gtp_var_F = v3;
  if (!sign16((v3 & 0xFF00) - 2560))
    E->gtp_var_F = 2304;
  uint32 v6 = __PAIR32__(samus_y_pos, samus_y_subpos) - __PAIR32__(E->gtp_var_F >> 8, E->gtp_var_F << 8);
  samus_y_subpos = v6;
  samus_y_pos = v6 >> 16;
  uint16 v5 = samus_y_pos - 17;
  Get_GunshipTop(k)->base.y_pos = v5;
  Get_GunshipTop(k + 128)->base.y_pos = v5 - 1;
  E->base.y_pos = samus_y_pos + 23;
}
