// PLM pre-instruction handlers.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"

void PlmPreInstr_Empty2(void) {  // 0x8484E6
  ;
}

void PlmPreInstr_PositionSamusAndInvincible(uint16 k) {  // 0x84AC89
  int v1 = k >> 1;
  samus_x_pos = plm_variable[v1];
  samus_y_pos = plm_variables[v1];
  samus_invincibility_timer |= 0x10;
}

void PlmPreInstr_B7EB_DecTimerEnableSoundsDeletePlm(uint16 k) {  // 0x84B7DD
  int v2 = k >> 1;
  if (plm_timers[v2]-- == 1) {
    debug_disable_sounds = 0;
    plm_header_ptr[v2] = 0;
  }
}

void PlmPreInstr_WakeAndLavaIfBoosterCollected(uint16 k) {  // 0x84B7EF
  if ((collected_items & 0x2000) != 0) {
    if ((fx_target_y_pos & 0x8000) != 0) {
      plm_header_ptr[k >> 1] = 0;
    } else {
      fx_y_vel = -128;
      int v1 = k >> 1;
      plm_instruction_timer[v1] = 1;
      ++plm_instr_list_ptrs[v1];
      ++plm_instr_list_ptrs[v1];
      plm_timers[v1] = 0;
    }
  } else {
    fx_target_y_pos = -1;
    fx_y_vel = 0;
    fx_timer = 0;
    earthquake_timer = 0;
    plm_header_ptr[k >> 1] = 0;
  }
}

void PlmPreInstr_WakePLMAndStartFxMotionSamusFarLeft(uint16 k) {  // 0x84B82A
  if (samus_x_pos <= 0xAE0) {
    fx_timer = 1;
    int v1 = k >> 1;
    plm_instruction_timer[v1] = 1;
    ++plm_instr_list_ptrs[v1];
    ++plm_instr_list_ptrs[v1];
    plm_timers[v1] = 0;
  }
}

void PlmPreInstr_AdvanceLavaSamusMovesLeft(uint16 k) {  // 0x84B846
  static const uint16 g_word_84B876[10] = {
     0x72b,  0x1bf,
    0xff50,  0x50a,
     0x167, 0xff20,
     0x244,  0x100,
    0xff20, 0x8000,
  };


  int v1 = k >> 1;
  uint16 v2 = plm_timers[v1];
  int v3 = v2 >> 1;
  uint16 v4 = g_word_84B876[v3];
  if ((v4 & 0x8000) != 0) {
    SetEventHappened(0x15);
  } else if (v4 >= samus_x_pos) {
    if (g_word_84B876[v3 + 1] < fx_base_y_pos)
      fx_base_y_pos = g_word_84B876[v3 + 1];
    fx_y_vel = g_word_84B876[v3 + 2];
    plm_timers[v1] = v2 + 6;
  }
}

void PlmPreInstr_ShaktoolsRoom(uint16 k) {  // 0x84B8B0
  if (power_bomb_explosion_status) {
    *(uint16 *)scrolls = 257;
    *(uint16 *)&scrolls[2] = 257;
  }
  if (samus_x_pos > 0x348) {
    SetEventHappened(0xD);
    plm_header_ptr[k >> 1] = 0;
  }
}

uint8 WakePlmIfSamusIsBelowAndRightOfTarget(uint16 k, uint16 x_r18, uint16 y_r20) {  // 0x84B8FD
  if (x_r18 < samus_x_pos && y_r20 < samus_y_pos) {
    int v2 = k >> 1;
    plm_instr_list_ptrs[v2] += 2;
    plm_instruction_timer[v2] = 1;
    return false;
  }
  return true;
}

void PlmPreInstr_OldTourianEscapeShaftEscape(uint16 k) {  // 0x84B927
  if (!WakePlmIfSamusIsBelowAndRightOfTarget(k, 240, 2080))
    SpawnEprojWithRoomGfx(0xB4B1, 0);
}

void PlmPreInstr_EscapeRoomBeforeOldTourianEscapeShaft(uint16 k) {  // 0x84B948
  if (!WakePlmIfSamusIsBelowAndRightOfTarget(k, 240, 1344)) {
    fx_y_vel = -104;
    fx_timer = 16;
  }
}

void PlmPreInstr_WakePlmIfTriggered(uint16 k) {  // 0x84BB52
  int v1 = k >> 1;
  if (plm_timers[v1]) {
    ++plm_instr_list_ptrs[v1];
    ++plm_instr_list_ptrs[v1];
    plm_instruction_timer[v1] = 1;
    plm_pre_instrs[v1] = addr_locret_84BB6A;
  }
}

void PlmPreInstr_WakePlmIfTriggeredOrSamusBelowPlm(uint16 k) {  // 0x84BB6B
  CalculatePlmBlockCoords(k);
  if (samus_x_pos >> 4 == plm_x_block && (uint16)((samus_y_pos >> 4) - plm_y_block) < 5
      || plm_timers[k >> 1]) {
    int v1 = k >> 1;
    ++plm_instr_list_ptrs[v1];
    ++plm_instr_list_ptrs[v1];
    plm_instruction_timer[v1] = 1;
    plm_pre_instrs[v1] = FUNC16(PlmPreInstr_Empty5);
  }
}

void PlmPreInstr_WakePlmIfTriggeredOrSamusAbovePlm(uint16 k) {  // 0x84BBA4
  CalculatePlmBlockCoords(k);
  if (samus_x_pos >> 4 == plm_x_block && (uint16)((samus_y_pos >> 4) - plm_y_block) >= 0xFFFC
      || plm_timers[k >> 1]) {
    int v1 = k >> 1;
    ++plm_instr_list_ptrs[v1];
    ++plm_instr_list_ptrs[v1];
    plm_instruction_timer[v1] = 1;
    plm_pre_instrs[v1] = addr_locret_84BBDC;
  }
}

void PlmPreInstr_GoToLinkInstrIfShot(uint16 k) {  // 0x84BD0F
  int v2 = k >> 1;
  if (plm_timers[v2]) {
    plm_timers[v2] = 0;
    plm_instr_list_ptrs[v2] = plm_instruction_list_link_reg[v2];
    plm_instruction_timer[v2] = 1;
  }
}

void PlmPreInstr_GoToLinkInstrIfShotWithPowerBomb(uint16 k) {  // 0x84BD26
  int v2 = k >> 1;
  uint16 v3 = plm_timers[v2];
  if (v3) {
    if ((v3 & 0xF00) == 768) {
      plm_timers[v2] = 0;
      plm_instr_list_ptrs[v2] = plm_instruction_list_link_reg[v2];
      plm_instruction_timer[v2] = 1;
      return;
    }
    QueueSfx2_Max6(0x57);
  }
  plm_timers[v2] = 0;
}

void PlmPreInstr_GoToLinkInstrIfShotWithAnyMissile(uint16 k) {  // 0x84BD50
  int16 v3;

  uint16 v2 = plm_timers[k >> 1];
  if (v2) {
    v3 = v2 & 0xF00;
    if (v3 == 512) {
      plm_variables[k >> 1] = 119;
      goto LABEL_4;
    }
    if (v3 == 256) {
LABEL_4:;
      int v4 = k >> 1;
      plm_timers[v4] = 0;
      plm_instr_list_ptrs[v4] = plm_instruction_list_link_reg[v4];
      plm_instruction_timer[v4] = 1;
      return;
    }
    QueueSfx2_Max6(0x57);
  }
  plm_timers[k >> 1] = 0;
}

void PlmPreInstr_GoToLinkInstrIfShotWithSuperMissile(uint16 k) {  // 0x84BD88
  int v2 = k >> 1;
  uint16 v3 = plm_timers[v2];
  if (v3) {
    if ((v3 & 0xF00) == 512) {
      plm_timers[v2] = 0;
      plm_instr_list_ptrs[v2] = plm_instruction_list_link_reg[v2];
      plm_instruction_timer[v2] = 1;
      return;
    }
    QueueSfx2_Max6(0x57);
  }
  plm_timers[v2] = 0;
}

void PlmPreInstr_GoToLinkInstruction(uint16 k) {  // 0x84BDB2
  int v2 = k >> 1;
  plm_timers[v2] = 0;
  plm_instr_list_ptrs[v2] = plm_instruction_list_link_reg[v2];
  plm_instruction_timer[v2] = 1;
}

void PlmPreInstr_PlayDudSound(uint16 k) {  // 0x84BE1C
  int v2 = k >> 1;
  if (plm_timers[v2])
    QueueSfx2_Max6(0x57);
  plm_timers[v2] = 0;
}

void PlmPreInstr_GotoLinkIfBoss1Dead(uint16 k) {  // 0x84BDD4
  if (CheckBossBitForCurArea(1) & 1)
    PlmPreInstr_GoToLinkInstruction(k);
  else
    PlmPreInstr_PlayDudSound(k);
}

void PlmPreInstr_GotoLinkIfMiniBossDead(uint16 k) {  // 0x84BDE3
  if (CheckBossBitForCurArea(2) & 1)
    PlmPreInstr_GoToLinkInstruction(k);
  else
    PlmPreInstr_PlayDudSound(k);
}

void PlmPreInstr_GotoLinkIfTorizoDead(uint16 k) {  // 0x84BDF2
  if (CheckBossBitForCurArea(4) & 1)
    PlmPreInstr_GoToLinkInstruction(k);
  else
    PlmPreInstr_PlayDudSound(k);
}

void PlmPreInstr_GotoLinkIfEnemyDeathQuotaOk(uint16 k) {  // 0x84BE01
  if (num_enemies_killed_in_room < num_enemy_deaths_left_to_clear) {
    PlmPreInstr_PlayDudSound(k);
  } else {
    SetEventHappened(0);
    PlmPreInstr_GoToLinkInstruction(k);
  }
}

void PlmPreInstr_GotoLinkIfTourianStatueFinishedProcessing(uint16 k) {  // 0x84BE1F
  if ((tourian_entrance_statue_finished & 0x8000) == 0)
    PlmPreInstr_PlayDudSound(k);
  else
    PlmPreInstr_GoToLinkInstruction(k);
}

void PlmPreInstr_GotoLinkIfCrittersEscaped(uint16 k) {  // 0x84BE30
  if (CheckEventHappened(0xF) & 1)
    PlmPreInstr_GoToLinkInstruction(k);
  else
    PlmPreInstr_PlayDudSound(k);
}

void PlmPreInstr_DeletePlmAndSpawnTriggerIfBlockDestroyed(uint16 k) {  // 0x84D15C
  uint16 prod = 8 * (uint8)room_width_in_blocks;
  uint16 v1 = 2 * (prod + 4);
  if (level_data[v1 >> 1] == 255) {
    WriteLevelDataBlockTypeAndBts(v1, 0xB083);
    plm_header_ptr[plm_id >> 1] = 0;
  }
}

void PlmPreInstr_IncrementRoomArgIfShotBySuperMissile(uint16 k) {  // 0x84D1E6
  int v1 = k >> 1;
  uint16 v2 = plm_timers[v1];
  if ((v2 & 0xF00) == 512 || (v2 & 0xF00) == 256)
    plm_room_arguments[v1]++;
  plm_timers[v1] = 0;
}

void PlmPreInstr_WakePlmIfSamusHasBombs(uint16 k) {  // 0x84D33B
  if ((collected_items & 0x1000) != 0) {
    int v1 = k >> 1;
    plm_instruction_timer[v1] = 1;
    plm_instr_list_ptrs[v1] += 2;
    plm_pre_instrs[v1] = FUNC16(nullsub_351);
  }
}

void PlmPreInstr_WakeOnKeyPress(uint16 k) {  // 0x84D4BF
  if ((joypad1_newkeys & (uint16)(kButton_B | kButton_Y | kButton_Left | kButton_Right | kButton_A | kButton_X)) != 0) {
    int v2 = k >> 1;
    plm_instruction_timer[v2] = 1;
    ++plm_instr_list_ptrs[v2];
    ++plm_instr_list_ptrs[v2];
  }
}

void PlmPreInstr_WakePlmIfRoomArgumentDoorIsSet(uint16 k) {  // 0x84D753
  int v2 = k >> 1;
  int idx = PrepareBitAccess(plm_room_arguments[v2]);
  if ((bitmask & opened_door_bit_array[idx]) != 0) {
    int v3 = k >> 1;
    plm_pre_instrs[v3] = addr_locret_84D779;
    plm_instr_list_ptrs[v3] = plm_instruction_list_link_reg[v3];
    plm_instruction_timer[v3] = 1;
  }
}

void sub_84D7EF(uint16 k) {  // 0x84D7EF
  uint16 v1 = room_width_in_blocks * 2 + k;
  WriteLevelDataBlockTypeAndBts(v1, 0xD0FF);
  uint16 v2 = room_width_in_blocks * 2 + v1;
  WriteLevelDataBlockTypeAndBts(v2, 0xD0FE);
  WriteLevelDataBlockTypeAndBts(room_width_in_blocks * 2 + v2, 0xD0FD);
}

void PlmPreInstr_SetMetroidsClearState_Ev0x10(uint16 k) {  // 0x84DADE
  if (num_enemies_killed_in_room >= num_enemy_deaths_left_to_clear)
    SetEventHappened(0x10);
}

void PlmPreInstr_SetMetroidsClearState_Ev0x11(uint16 k) {  // 0x84DAEE
  if (num_enemies_killed_in_room >= num_enemy_deaths_left_to_clear)
    SetEventHappened(0x11);
}

void PlmPreInstr_SetMetroidsClearState_Ev0x12(uint16 k) {  // 0x84DAFE
  if (num_enemies_killed_in_room >= num_enemy_deaths_left_to_clear)
    SetEventHappened(0x12);
}

void PlmPreInstr_SetMetroidsClearState_Ev0x13(uint16 k) {  // 0x84DB0E
  if (num_enemies_killed_in_room >= num_enemy_deaths_left_to_clear)
    SetEventHappened(0x13);
}

void PlmPreInstr_GotoLinkIfShotWithSuperMissile(uint16 k) {  // 0x84DB64
  int16 v1;

  v1 = plm_timers[k >> 1] & 0xF00;
  if (v1 == 512) {
    plm_room_arguments[k >> 1] = 119;
  } else if (v1 != 256) {
    return;
  }
  int v2 = k >> 1;
  plm_timers[v2] = 0;
  plm_instr_list_ptrs[v2] = plm_instruction_list_link_reg[v2];
  plm_instruction_timer[v2] = 1;
}

void PlmPreInstr_GotoLinkIfTriggered(uint16 k) {  // 0x84DF89
  int v1 = k >> 1;
  if (LOBYTE(plm_timers[v1]) == 255) {
    plm_pre_instrs[v1] = FUNC16(PlmPreInstr_nullsub_301);
    plm_instr_list_ptrs[v1] = plm_instruction_list_link_reg[v1];
    plm_instruction_timer[v1] = 1;
  }
}

void PlmPreInstr_WakeIfTriggered(uint16 k) {  // 0x84DFE6
  int v1 = k >> 1;
  uint16 v2 = plm_timers[v1];
  if (v2 != 768 && (uint8)v2 == 255) {
    plm_timers[v1] = 0;
    ++plm_instr_list_ptrs[v1];
    ++plm_instr_list_ptrs[v1];
    plm_instruction_timer[v1] = 1;
  }
}

const uint8 *PlmInstr_E63B(const uint8 *plmp, uint16 k) {  // 0x84E63B
  fx_y_vel = -32;
  return plmp;
}

