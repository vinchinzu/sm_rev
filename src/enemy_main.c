// Enemies
#include <stdio.h>
#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

#define kEnemyLayerToQueuePtr ((uint16*)RomFixedPtr(0xa0b133))

static void DebugLogLoadedEnemies(void) {
  if (!g_debug_flag)
    return;
  static uint16 last_room_ptr = 0xffff;
  static uint16 last_state_ptr = 0xffff;
  static uint16 last_enemy_pop = 0xffff;
  static uint16 last_num_enemies = 0xffff;
  static uint16 last_enemy0 = 0xffff;
  static uint16 last_enemy1 = 0xffff;
  static uint16 last_enemy2 = 0xffff;
  static uint16 last_enemy3 = 0xffff;
  uint16 enemy0 = gEnemyData(0)->enemy_ptr;
  uint16 enemy1 = gEnemyData(64)->enemy_ptr;
  uint16 enemy2 = gEnemyData(128)->enemy_ptr;
  uint16 enemy3 = gEnemyData(192)->enemy_ptr;
  if (last_room_ptr == room_ptr &&
      last_state_ptr == roomdefroomstate_ptr &&
      last_enemy_pop == room_enemy_population_ptr &&
      last_num_enemies == num_enemies_in_room &&
      last_enemy0 == enemy0 &&
      last_enemy1 == enemy1 &&
      last_enemy2 == enemy2 &&
      last_enemy3 == enemy3)
    return;
  last_room_ptr = room_ptr;
  last_state_ptr = roomdefroomstate_ptr;
  last_enemy_pop = room_enemy_population_ptr;
  last_num_enemies = num_enemies_in_room;
  last_enemy0 = enemy0;
  last_enemy1 = enemy1;
  last_enemy2 = enemy2;
  last_enemy3 = enemy3;
  fprintf(stderr,
          "ROOM enemies room=0x%04X state=0x%04X pop=0x%04X loaded=%u first=%04X,%04X,%04X,%04X\n",
          room_ptr, roomdefroomstate_ptr, room_enemy_population_ptr, num_enemies_in_room,
          enemy0, enemy1, enemy2, enemy3);
}

void Enemy_GrappleReact_NoInteract_A0(void) {  // 0xA08000
  SwitchEnemyAiToMainAi();
}

void Enemy_GrappleReact_SamusLatchesOn_A0(void) {  // 0xA08005
  SamusLatchesOnWithGrapple();
}

void Enemy_GrappleReact_KillEnemy_A0(void) {  // 0xA0800A
  EnemyGrappleDeath();
}

void Enemy_GrappleReact_CancelBeam_A0(void) {  // 0xA0800F
  Enemy_SwitchToFrozenAi();
}

void Enemy_GrappleReact_SamusLatchesNoInvinc_A0(void) {  // 0xA08014
  SamusLatchesOnWithGrappleNoInvinc();
}

void Enemy_GrappleReact_SamusLatchesParalyze_A0(void) {  // 0xA08019
  SamusLatchesOnWithGrappleParalyze();
}

void Enemy_GrappleReact_HurtSamus_A0(void) {  // 0xA0801E
  SamusHurtFromGrapple();
}

void Enemy_NormalTouchAI_A0(void) {  // 0xA08023
  NormalEnemyTouchAi();
}

void Enemy_NormalTouchAI_SkipDeathAnim_A0(void) {  // 0xA08028
  NormalEnemyTouchAiSkipDeathAnim_CurEnemy();
}

void Enemy_NormalShotAI_A0(void) {  // 0xA0802D
  NormalEnemyShotAi();
}

void Enemy_NormalShotAI_SkipSomeParts_A0(void) {  // 0xA08032
  NormalEnemyShotAiSkipDeathAnim_CurEnemy();
}

void Enemy_NormalPowerBombAI_A0(void) {  // 0xA08037
  NormalEnemyPowerBombAi();
}

void Enemy_NormalPowerBombAI_SkipDeathAnim_A0(void) {  // 0xA0803C
  NormalEnemyPowerBombAiSkipDeathAnim_CurEnemy();
}

void Enemy_NormalFrozenAI(void) {  // 0xA08041
  NormalEnemyFrozenAI();
}

void CreateADudShot(void) {  // 0xA08046
  CreateDudShot();
}

void func_nullsub_169(void) {
  ;
}

void func_nullsub_170(void) {
  ;
}

const uint16 *EnemyInstr_SetAiPreInstr(uint16 k, const uint16 *jp) {  // 0xA0806B
  gEnemyData(k)->ai_preinstr = jp[0];
  return jp + 1;
}

const uint16 *EnemyInstr_ClearAiPreInstr(uint16 k, const uint16 *jp) {  // 0xA08074
  gEnemyData(k)->ai_preinstr = FUNC16(nullsub_171);
  return jp;
}

void func_nullsub_171(void) {
  ;
}

const uint16 *EnemyInstr_StopScript(uint16 k, const uint16 *jp) {  // 0xA0807C
  EnemyData *v2 = gEnemyData(k);
  v2->properties |= kEnemyProps_Deleted;
  return 0;
}

const uint16 *EnemyInstr_Goto(uint16 k, const uint16 *jp) {  // 0xA280ED
  return INSTR_RETURN_ADDR(*jp);
}

const uint16 *EnemyInstr_GotoRel(uint16 k, const uint16 *jp) {  // 0xA080F2
  return (const uint16 * )((uint8*)jp + *(int8*)jp);
}

const uint16 *EnemyInstr_DecTimerAndGoto(uint16 k, const uint16 *jp) {  // 0xA08108
  EnemyData *v2 = gEnemyData(k);
  if (v2->timer-- == 1)
    return jp + 1;
  else
    return EnemyInstr_Goto(k, jp);
}

const uint16 *EnemyInstr_DecTimerAndGotoRel(uint16 k, const uint16 *jp) {  // 0xA08118
  EnemyData *v2 = gEnemyData(k);
  if (LOBYTE(v2->timer)-- == 1)
    return (const uint16 *)((uint8 *)jp + 1);
  else
    return EnemyInstr_GotoRel(k, jp);
}

const uint16 *EnemyInstr_SetTimer(uint16 k, const uint16 *jp) {  // 0xA08123
  gEnemyData(k)->timer = *jp;
  return jp + 1;
}

const uint16 *EnemyInstr_Skip2bytes(uint16 k, const uint16 *jp) {  // 0xA0812C
  return jp + 1;
}

const uint16 *EnemyInstr_Sleep(uint16 k, const uint16 *jp) {  // 0xA2812F
  EnemyData *ED = gEnemyData(k);
  const uint8 *base_ptr = RomBankBase(ED->bank);
  ED->current_instruction = (const uint8 *)jp - 2 - base_ptr;
  return 0;
}

const uint16 *EnemyInstr_WaitNframes(uint16 k, const uint16 *jp) {  // 0xA0813A
  EnemyData *ED = gEnemyData(k);
  const uint8 *base_ptr = RomBankBase(ED->bank);
  ED->instruction_timer = jp[0];
  ED->current_instruction = (const uint8 *)jp + 2 - base_ptr;
  return 0;
}

const uint16 *EnemyInstr_CopyToVram(uint16 k, const uint16 *jp) {  // 0xA0814B
  VramWriteEntry *v4;

  uint16 v2 = vram_write_queue_tail;
  uint8 *v3 = (uint8*)jp;
  v4 = gVramWriteEntry(vram_write_queue_tail);
  v4->size = GET_WORD(v3);
  v4->src.addr = GET_WORD(v3 + 2);
  *(VoidP *)((uint8 *)&v4->src.addr + 1) = GET_WORD(v3 + 3);
  v4->vram_dst = GET_WORD(v3 + 5);
  vram_write_queue_tail = v2 + 7;
  return INSTR_INCR_BYTES(jp, 7);
}

const uint16 *EnemyInstr_EnableOffScreenProcessing(uint16 k, const uint16 *jp) {  // 0xA08173
  EnemyData *v2 = gEnemyData(k);
  v2->properties |= kEnemyProps_ProcessedOffscreen;
  return jp;
}

const uint16 *EnemyInstr_DisableOffScreenProcessing(uint16 k, const uint16 *jp) {  // 0xA0817D
  EnemyData *v2 = gEnemyData(k);
  v2->properties &= ~kEnemyProps_ProcessedOffscreen;
  return jp;
}

static const uint16 kRoomShakes[144] = {  // 0xA08687
  1, 0, 0, 0,
  0, 1, 0, 0,
  1, 1, 0, 0,
  2, 0, 0, 0,
  0, 2, 0, 0,
  2, 2, 0, 0,
  3, 0, 0, 0,
  0, 3, 0, 0,
  3, 3, 0, 0,
  1, 0, 1, 0,
  0, 1, 0, 1,
  1, 1, 1, 1,
  2, 0, 2, 0,
  0, 2, 0, 2,
  2, 2, 2, 2,
  3, 0, 3, 0,
  0, 3, 0, 3,
  3, 3, 3, 3,
  1, 0, 1, 0,
  0, 1, 0, 1,
  1, 1, 1, 1,
  2, 0, 2, 0,
  0, 2, 0, 2,
  2, 2, 2, 2,
  3, 0, 3, 0,
  0, 3, 0, 3,
  3, 3, 3, 3,
  0, 0, 1, 0,
  0, 0, 0, 1,
  0, 0, 1, 1,
  0, 0, 2, 0,
  0, 0, 0, 2,
  0, 0, 2, 2,
  0, 0, 3, 0,
  0, 0, 0, 3,
  0, 0, 3, 3,
};

void HandleRoomShaking(void) {

  if (earthquake_timer && !time_is_frozen_flag && sign16(earthquake_type - 36)) {
    int v0 = (8 * earthquake_type) >> 1;
    if ((earthquake_timer & 2) != 0) {
      reg_BG1HOFS -= kRoomShakes[v0];
      reg_BG1VOFS -= kRoomShakes[v0 + 1];
      reg_BG2HOFS -= kRoomShakes[v0 + 2];
      reg_BG2VOFS -= kRoomShakes[v0 + 3];
    } else {
      reg_BG1HOFS += kRoomShakes[v0];
      reg_BG1VOFS += kRoomShakes[v0 + 1];
      reg_BG2HOFS += kRoomShakes[v0 + 2];
      reg_BG2VOFS += kRoomShakes[v0 + 3];
    }
    --earthquake_timer;
    if (!sign16(earthquake_type - 18))
      SetAllEnemiesToShakeFor2Frames();
  }
  ++frame_counter_every_frame;
}

void SetAllEnemiesToShakeFor2Frames(void) {  // 0xA08712
  for (int i = 0; ; i += 2) {
    int v1 = i >> 1;
    if (active_enemy_indexes[v1] == 0xFFFF)
      break;
    gEnemyData(active_enemy_indexes[v1])->shake_timer = 2;
  }
}

void CallEnemyGfxDrawHook(uint32 ea) {
  ea = CanonicalizeEnemyHandler(ea);
  switch (ea) {
  case fnnullsub_170: return;  // 0xa0804c
  case fnReflec_Func_1: Reflec_Func_1(); return;  // 0xa3db0c
  case fnDraygon_Func_36: Draygon_Func_36(); return;  // 0xa59342
  case fnRidley_A2F2: Ridley_A2F2(); return;  // 0xa6a2f2
  case fnNorfairLavaMan_Func_6: NorfairLavaMan_Func_6(); return;  // 0xa8b0b2
  case fnWreckedShipRobot_Func_1: WreckedShipRobot_Func_1(); return;  // 0xa8cc67
  case fnBlueBrinstarFaceBlock_Func_1: BlueBrinstarFaceBlock_Func_1(); return;  // 0xa8e86e
  case fnnullsub_264: return;  // 0xa98786
  case fnMotherBrain_DrawBrainNeck_EnemyGfxDrawHook: MotherBrain_DrawBrainNeck_EnemyGfxDrawHook(); return;  // 0xa987c9
  case fnMotherBrainsBrain_GfxDrawHook: MotherBrainsBrain_GfxDrawHook(); return;  // 0xa987dd
  case fnDeadTorizo_MainGfxHook: DeadTorizo_MainGfxHook(); return;  // 0xa9d39a
  default: Unreachable();
  }
}

void DrawSamusEnemiesAndProjectiles(void) {  // 0xA0884D
  DrawSpriteObjects();
  DrawBombAndProjectileExplosions();
  DrawLowPriorityEprojs();
  for (uint16 phase_varE32 = 0; phase_varE32 != 8; ++phase_varE32) {
    if (phase_varE32 == 3) {
      DrawSamusAndProjectiles();
    } else if (phase_varE32 == 6) {
      DrawHighPriorityEprojs();
    }
    if (enemy_drawing_queue_sizes[phase_varE32]) {
      uint16 varE36 = enemy_drawing_queue_sizes[phase_varE32];
      uint16 varE3A = kEnemyLayerToQueuePtr[phase_varE32];
      uint16 v1 = 0, varE38;
      enemy_drawing_queue_sizes[phase_varE32] = 0;
      do {
        varE38 = v1;
        uint8 *v2 = RomPtr_RAM(varE3A + v1);
        uint16 v3 = GET_WORD(v2);
        *(uint16 *)v2 = 0;
        cur_enemy_index = v3;
        WriteEnemyOams();
        v1 = varE38 + 2;
      } while (varE38 + 2 != varE36);
    }
  }
  CallEnemyGfxDrawHook(Load24(&enemy_gfx_drawn_hook));
}

void RecordEnemySpawnData(uint16 j) {  // 0xA088D0
  EnemySpawnData *v2;
  EnemySpawnData *v5;

  EnemyData *v1 = gEnemyData(j);
  v2 = gEnemySpawnData(j);
  v2->id = v1->enemy_ptr;
  v2->x_pos = v1->x_pos;
  v2->y_pos = v1->y_pos;
  v2->init_param = v1->current_instruction;
  v2->properties = v1->properties;
  v2->extra_properties = v1->extra_properties;
  v2->param_1 = v1->parameter_1;
  v2->param_2 = v1->parameter_2;
  uint16 r18 = 0, r20 = 0, r22 = 0, r24 = 0, R26 = 0, R28 = 0;
  uint16 name_ptr = get_EnemyDef_A2(v1->enemy_ptr)->name_ptr;
  if (name_ptr) {
    const uint16 *v4 = (const uint16 *)RomPtr_B4(name_ptr);
    r18 = v4[0];
    r20 = v4[1];
    r22 = v4[2];
    r24 = v4[3];
    R26 = v4[4];
    R28 = v4[6];
  }
  v5 = gEnemySpawnData(j);
  *(uint16 *)v5->name = r18;
  *(uint16 *)&v5->name[2] = r20;
  *(uint16 *)&v5->name[4] = r22;
  *(uint16 *)&v5->name[6] = r24;
  *(uint16 *)&v5->name[8] = R26;
  *(uint16 *)&v5->name[10] = R28;
}

void LoadEnemies(void) {  // 0xA08A1E
  debug_time_frozen_for_enemies = 0;
  enemy_gfx_drawn_hook.bank = 160;
  enemy_gfx_drawn_hook.addr = FUNC16(nullsub_170);
  enemy_bg2_tilemap_size = 2048;
  UNUSED_word_7E179E = 0;
  UNUSED_word_7E17A0 = 0;
  boss_id = 0;
  ClearEnemyDataAndProcessEnemySet();
  LoadEnemyTileData();
  enemy_tile_vram_src = 0;
  flag_disable_projectile_interaction = 0;
  ClearSpriteObjects();
}

void ClearEnemyDataAndProcessEnemySet(void) {  // 0xA08A6D
  memset(enemy_data, 0, 2048);
  if (*(uint16 *)RomPtr_A1(room_enemy_population_ptr) != 0xFFFF)
    ProcessEnemyTilesets();
}

void InitializeEnemies(void) {  // 0xA08A9E
  memset(enemy_spawn_data, 0, 0x2800);
  num_enemies_in_room = 0;
  num_enemies_killed_in_room = 0;
  flag_process_all_enemies = 0;
  for (int i = 286; i >= 0; i -= 2)
    eproj_flags[i >> 1] = 0;
  for (int j = 34; j >= 0; j -= 2)
    eproj_killed_enemy_index[j >> 1] = -1;
  uint16 v4 = room_enemy_population_ptr;
  if (get_EnemyPopulation(0xa1, room_enemy_population_ptr)->enemy_ptr == 0xFFFF)
    return;
  UNUSED_word_7E0E48 = 0;
  uint16 v5 = 0;
  do {
    LoadEnemyGfxIndexes(v4, v5);
    EnemyPopulation *EP = get_EnemyPopulation(0xa1, v4);
    EnemyDef *ED = get_EnemyDef_A2(EP->enemy_ptr);
    EnemyData *E = gEnemyData(v5);
    E->x_width = ED->x_radius;
    E->y_height = ED->y_radius;
    E->health = ED->health;
    E->layer = ED->layer;
    *(uint16 *)&E->bank = *(uint16 *)&ED->bank;
    if (ED->boss_fight_value)
      boss_id = ED->boss_fight_value;
    E->enemy_ptr = EP->enemy_ptr;
    E->x_pos = EP->x_pos;
    E->y_pos = EP->y_pos;
    E->current_instruction = EP->init_param;
    E->properties = EP->properties;
    E->extra_properties = EP->extra_properties;
    E->parameter_1 = EP->parameter1;
    E->parameter_2 = EP->parameter2;
    E->frame_counter = 0;
    E->timer = 0;
    E->instruction_timer = 1;
    E->frame_counter = 0;
    RecordEnemySpawnData(v5);
    cur_enemy_index = v5;
    CallEnemyAi(ED->bank << 16 | ED->ai_init);
    E->spritemap_pointer = 0;
    if ((E->properties & kEnemyProps_ProcessInstructions) != 0) {
      uint16 v12 = addr_kSpritemap_Nothing_A4;
      if ((E->extra_properties & kEnemyExtraProps_MultiHitbox) != 0)
        v12 = addr_kExtendedSpritemap_Nothing_A4;
      E->spritemap_pointer = v12;
    }
    v5 += 64;
    v4 += 16;
  } while (get_EnemyPopulation(0xa1, v4)->enemy_ptr != 0xFFFF);
  first_free_enemy_index = v5;
  num_enemies_in_room = v5 >> 6;
  num_enemy_deaths_left_to_clear = RomPtr_A1(v4)[2];
  DebugLogLoadedEnemies();
}

void DetermineWhichEnemiesToProcess(void) {  // 0xA08EB6
  ++UNUSED_word_7E0E46;
  cur_enemy_index = 0;
  active_enemy_indexes_write_ptr = 0;
  interactive_enemy_indexes_write_ptr = 0;
  if (flag_process_all_enemies) {
    do {
      uint16 v5 = cur_enemy_index;
      EnemyData *v6 = gEnemyData(cur_enemy_index);
      if (v6->enemy_ptr && v6->enemy_ptr != addr_kEnemyDef_DAFF) {
        if ((v6->properties & kEnemyProps_Deleted) != 0) {
          v6->enemy_ptr = 0;
        } else {
          uint16 v7 = active_enemy_indexes_write_ptr;
          int v8 = active_enemy_indexes_write_ptr >> 1;
          active_enemy_indexes[v8] = cur_enemy_index;
          interactive_enemy_indexes[v8] = v5;
          active_enemy_indexes_write_ptr = v7 + 2;
          if ((v6->properties & kEnemyProps_Intangible) == 0) {
            uint16 v9 = interactive_enemy_indexes_write_ptr;
            interactive_enemy_indexes[interactive_enemy_indexes_write_ptr >> 1] = v5;
            interactive_enemy_indexes_write_ptr = v9 + 2;
          }
        }
      }
      cur_enemy_index += 64;
    } while (sign16(cur_enemy_index - 2048));
    active_enemy_indexes[active_enemy_indexes_write_ptr >> 1] = -1;
    interactive_enemy_indexes[interactive_enemy_indexes_write_ptr >> 1] = -1;
  } else {
    do {
      uint16 v0 = cur_enemy_index;
      EnemyData *v1 = gEnemyData(cur_enemy_index);
      if (v1->enemy_ptr && v1->enemy_ptr != addr_kEnemyDef_DAFF) {
        uint16 properties = v1->properties;
        if ((properties & kEnemyProps_Deleted) != 0) {
          v1->enemy_ptr = 0;
        } else if ((properties & kEnemyProps_ProcessedOffscreen) != 0
                   || (v1->ai_handler_bits & kEnemyAiBits_Frozen) != 0
                   || (int16)(v1->x_width + v1->x_pos - layer1_x_pos) >= 0
                   && (int16)(v1->x_width + layer1_x_pos + 256 - v1->x_pos) >= 0
                   && (int16)(v1->y_pos + 8 - layer1_y_pos) >= 0
                   && (int16)(layer1_y_pos + 248 - v1->y_pos) >= 0) {
          uint16 v3 = active_enemy_indexes_write_ptr;
          active_enemy_indexes[active_enemy_indexes_write_ptr >> 1] = cur_enemy_index;
          active_enemy_indexes_write_ptr = v3 + 2;
          if ((v1->properties & kEnemyProps_Intangible) == 0) {
            uint16 v4 = interactive_enemy_indexes_write_ptr;
            interactive_enemy_indexes[interactive_enemy_indexes_write_ptr >> 1] = v0;
            interactive_enemy_indexes_write_ptr = v4 + 2;
          }
        }
      }
      cur_enemy_index += 64;
    } while (sign16(cur_enemy_index - 2048));
    active_enemy_indexes[active_enemy_indexes_write_ptr >> 1] = -1;
    interactive_enemy_indexes[interactive_enemy_indexes_write_ptr >> 1] = -1;
  }
}

void ProcessEnemyInstructions(void) {  // 0xA0C26A
  EnemyData *E = gEnemyData(cur_enemy_index);
  if ((E->ai_handler_bits & kEnemyAiBits_Frozen) == 0) {
    if (E->instruction_timer-- == 1) {
      assert(E->current_instruction & 0x8000);
      const uint8 *base_ptr = RomBankBase(E->bank);
      const uint16 *pc = (const uint16 *)(base_ptr + E->current_instruction);
      while ((*pc & 0x8000) != 0) {
        pc = CallEnemyInstr(E->bank << 16 | *pc, cur_enemy_index, pc + 1);
        if (!pc)
          return;
        if ((uintptr_t)pc < 0x10000)
          pc = (const uint16 *)(base_ptr + (uintptr_t)pc);
      }
      E->instruction_timer = pc[0];
      E->spritemap_pointer = pc[1];
      E->current_instruction = (uint8 *)pc + 4 - base_ptr;
      E->extra_properties |= kEnemyExtraProps_UpdateGfx;
    } else {
      E->extra_properties &= ~kEnemyExtraProps_UpdateGfx;
    }
  }
}

void EnemyMain(void) {  // 0xA08FD4
  if (first_free_enemy_index) {
    if (enemy_index_to_shake != 0xFFFF) {
      gEnemyData(enemy_index_to_shake)->shake_timer = 64;
      enemy_index_to_shake = -1;
    }
    for (int active_enemy_indexes_index = 0; ; active_enemy_indexes_index += 2) {
      uint16 v1 = active_enemy_indexes[active_enemy_indexes_index >> 1];
      if (v1 == 0xFFFF)
        break;
      cur_enemy_index = v1;
      EnemyData *E = gEnemyData(cur_enemy_index);
      bool skip_ai_and_draw = false;
      bool skip_ai = false;
      if ((E->properties & kEnemyProps_Intangible) == 0) {
        if (E->invincibility_timer) {
          --E->invincibility_timer;
        } else if (!debug_disable_sprite_interact) {
          if (!(debug_time_frozen_for_enemies | time_is_frozen_flag)) {
            EnemyCollisionHandler();
            if (!E->enemy_ptr)
              skip_ai_and_draw = true;
          }
          if (!skip_ai_and_draw && (E->extra_properties & kEnemyExtraProps_DisableEnemyAI) != 0)
            skip_ai = true;
        }
      }
      if (!skip_ai_and_draw) {
        if (!skip_ai) {
          bool update_frame_and_instr = false;
          UNUSED_word_7E17A2 = 0;
          if (!(debug_time_frozen_for_enemies | time_is_frozen_flag)) {
            int16 handler_index = 0;
            uint16 ai_handler_bits = E->ai_handler_bits;
            if (ai_handler_bits) {
              int8 bit_set;
              do {
                ++handler_index;
                bit_set = ai_handler_bits & 1;
                ai_handler_bits >>= 1;
              } while (!bit_set);
            }
            CallEnemyAi(E->bank << 16 | get_EnemyDef_A2(E->enemy_ptr + 2 * handler_index)->main_ai);
            update_frame_and_instr = true;
          } else {
            EnemyDef *ED = get_EnemyDef_A2(E->enemy_ptr);
            if (ED->time_is_frozen_ai) {
              CallEnemyAi(E->bank << 16 | ED->time_is_frozen_ai);
              update_frame_and_instr = true;
            }
          }
          if (update_frame_and_instr && !(debug_time_frozen_for_enemies | time_is_frozen_flag)) {
            ++E->frame_counter;
            if ((E->properties & kEnemyProps_ProcessInstructions) != 0) {
              enemy_processing_stage = 2;
              ProcessEnemyInstructions();
            }
          }
        }
        if ((E->extra_properties & kEnemyExtraProps_DisableEnemyAI) != 0 && (E->flash_timer == 1 || E->frozen_timer == 1)) {
          gEnemySpawnData(cur_enemy_index)->cause_of_death = 0;
          EnemyDeathAnimation(cur_enemy_index, 0);
        }
        if (((E->extra_properties & kEnemyExtraProps_MultiHitbox) != 0 || !EnemyWithNormalSpritesIsOffScreen())
            && (E->properties & (kEnemyProps_Invisible | kEnemyProps_Deleted)) == 0
            && (UNUSED_word_7E17A2 & 1) == 0) {
          DrawOneEnemy();
        }
      }
      if (E->flash_timer && !(debug_time_frozen_for_enemies | time_is_frozen_flag)) {
        if (sign16(--E->flash_timer - 8))
          E->ai_handler_bits &= ~kEnemyAiBits_Hurt;
      }
    }
  }
  HandleSpriteObjects();
  random_enemy_counter++;
  enemy_index_colliding_dirs[0] = -1;
  enemy_index_colliding_dirs[1] = -1;
  enemy_index_colliding_dirs[2] = -1;
  enemy_index_colliding_dirs[3] = -1;
}

void DecrementSamusTimers(void) {  // 0xA09169
  if (samus_invincibility_timer)
    --samus_invincibility_timer;
  if (samus_knockback_timer)
    --samus_knockback_timer;
  if (projectile_invincibility_timer)
    --projectile_invincibility_timer;
  interactive_enemy_indexes[0] = -1;
  active_enemy_indexes[0] = -1;
}

void SpawnEnemyDrops(uint16 a, uint16 k, uint16 varE20) {  // 0xA0920E
  eproj_spawn_varE24 = a;
  SpawnEprojWithGfx(varE20, k, addr_kEproj_Pickup);
}

void DeleteEnemyAndConnectedEnemies(void) {  // 0xA0922B
  EnemyData *v0 = gEnemyData(cur_enemy_index);
  uint16 num_parts = get_EnemyDef_A2(v0->enemy_ptr)->num_parts;
  if (!num_parts)
    num_parts = 1;
  uint16 v2 = num_parts;
  do {
    gEnemyData(cur_enemy_index)->enemy_ptr = 0;
    cur_enemy_index += 64;
    --v2;
  } while (v2);
}

uint16 SpawnEnemy(uint8 db, uint16 k) {  // 0xA09275
  uint16 varE20 = k;
  uint16 cur_enemy_index_backup = cur_enemy_index;
  uint16 enemy_ptr = get_EnemyPopulation(db, k)->enemy_ptr;
  int16 v3 = get_EnemyDef_A2(enemy_ptr)->num_parts - 1;
  if (v3 < 0)
    v3 = 0;
  uint16 varE22 = v3;
  uint16 varE26 = v3;
  uint16 new_enemy_index = 0;
  do {
    uint16 v4 = new_enemy_index;
    while (!gEnemyData(v4)->enemy_ptr) {
      if (!varE22)
        goto add_enemy;
      --varE22;
      v4 += 64;
      if (v4 >= 2048)
        return 0xffff;
    }
    new_enemy_index += 64;
  } while (new_enemy_index < 2048);
  return 0xffff;

add_enemy:
  while (1) {
    EnemyData *E = gEnemyData(new_enemy_index);
    EnemyPopulation *EP = get_EnemyPopulation(db, varE20);
    int v10 = 0;
    if (EP->enemy_ptr == enemy_def_ptr[0] ||
      (v10 = 1, EP->enemy_ptr == enemy_def_ptr[1]) ||
      (v10 = 2, EP->enemy_ptr == enemy_def_ptr[2]) ||
      (v10 = 3, EP->enemy_ptr == enemy_def_ptr[3])) {
      E->vram_tiles_index = enemy_gfxdata_tiles_index[v10];
      E->palette_index = 2 * swap16(enemy_gfxdata_vram_ptr[v10]);
    } else {
      E->vram_tiles_index = 0;
      E->palette_index = 0;
    }
    EnemyDef *ED = get_EnemyDef_A2(EP->enemy_ptr);
    E->x_width = ED->x_radius;
    E->y_height = ED->y_radius;
    E->health = ED->health;
    E->layer = ED->layer;
    *(uint16 *)&E->bank = *(uint16 *)&ED->bank;
    E->enemy_ptr = EP->enemy_ptr;
    E->x_pos = EP->x_pos;
    E->y_pos = EP->y_pos;
    E->current_instruction = EP->init_param;
    E->properties = EP->properties;
    E->extra_properties = EP->extra_properties;
    E->parameter_1 = EP->parameter1;
    E->parameter_2 = EP->parameter2;
    E->frame_counter = 0;
    E->timer = 0;
    E->ai_var_A = 0;
    E->ai_var_B = 0;
    E->ai_var_C = 0;
    E->ai_var_D = 0;
    E->ai_var_E = 0;
    E->ai_preinstr = 0;
    E->instruction_timer = 1;
    E->frame_counter = 0;
    RecordEnemySpawnData(new_enemy_index);
    cur_enemy_index = new_enemy_index;
    if (sign16(ED->ai_init))
      CallEnemyAi(ED->bank << 16 | ED->ai_init);
    if ((E->properties & kEnemyProps_ProcessInstructions) != 0)
      E->spritemap_pointer = addr_kSpritemap_Nothing_A0;
    if (!varE26 || !--varE26) {
      cur_enemy_index = cur_enemy_index_backup;
      return new_enemy_index;
    }
    new_enemy_index += 64;
    varE20 += 16;
  }
}

void DrawOneEnemy(void) {  // 0xA09423
  uint16 varE34 = 2 * gEnemyData(cur_enemy_index)->layer;
  *(uint16 *)RomPtr_RAM(enemy_drawing_queue_sizes[varE34 >> 1] + kEnemyLayerToQueuePtr[varE34 >> 1]) = cur_enemy_index;
  enemy_drawing_queue_sizes[varE34 >> 1] += 2;
}

void WriteEnemyOams(void) {  // 0xA0944A
  VoidP palette_index;
  
  EnemyData *E = gEnemyData(cur_enemy_index);
  EnemySpawnData *ES = gEnemySpawnData(cur_enemy_index);
  uint16 x2 = ES->xpos2 + E->x_pos - layer1_x_pos;
  uint16 y2 = ES->ypos2 + E->y_pos - layer1_y_pos;
  if (E->shake_timer) {
    x2 += ((E->frame_counter & 2) == 0) ? 1 : -1;
    E->shake_timer--;
  }
  uint16 x = x2, y = y2;
  if (E->flash_timer && (random_enemy_counter & 2) != 0) {
    palette_index = 0;
  } else {
    if (E->frozen_timer && (E->frozen_timer >= 0x5A || (E->frozen_timer & 2) != 0))
      palette_index = 3072;
    else
      palette_index = E->palette_index;
  }
  uint16 r3 = palette_index;
  uint16 r0 = E->vram_tiles_index;
  if ((E->extra_properties & kEnemyExtraProps_MultiHitbox) != 0) {
    if ((int16)(E->spritemap_pointer + 0x8000) < 0)
      Unreachable();
    int n = *RomPtrWithBank(E->bank, E->spritemap_pointer);
    uint16 v5 = E->spritemap_pointer + 2;
    do {
      ExtendedSpriteMap *ext = get_ExtendedSpriteMap(E->bank, v5);
      if (*(uint16 *)RomPtrWithBank(E->bank, ext->spritemap) == 0xFFFE) {
        x = x2 + ext->xpos;
        y = y2 + ext->ypos;
        if ((E->extra_properties & kEnemyExtraProps_UpdateGfx) != 0)
          ProcessExtendedTilemap(E->bank, ext->spritemap);
      } else {
        x = x2 + ext->xpos;
        y = y2 + ext->ypos;
        if (((x + 128) & 0xFE00) == 0 && ((y + 128) & 0xFE00) == 0) {
          if (HIBYTE(y))
            DrawSpritemapWithBaseTileOffscreen(E->bank, ext->spritemap, x, y, r3, r0);
          else
            DrawSpritemapWithBaseTile2(E->bank, ext->spritemap, x, y, r3, r0);
        }
      }
      v5 += 8;
    } while (--n);
  } else {
    enemy_processing_stage = 1;
    DrawSpritemapWithBaseTile(E->bank, E->spritemap_pointer, x, y, r3, r0);
  }
}

void NormalEnemyFrozenAI(void) {  // 0xA0957E
  EnemyData *v0 = gEnemyData(cur_enemy_index);
  v0->flash_timer = 0;
  if (!v0->frozen_timer || (--v0->frozen_timer, (equipped_beams & 2) == 0)) {
    v0->ai_handler_bits &= ~kEnemyAiBits_Frozen;
    v0->frozen_timer = v0->ai_handler_bits;
  }
}

void ProcessExtendedTilemap(uint8 db, uint16 r22) {  // 0xA096CA
  const uint8 *p = RomPtrWithBank(db, r22 + 2);
  while (1) {
    uint16 v2 = *(uint16 *)p;
    if (v2 == 0xFFFF)
      break;
    int n = *((uint16 *)p + 1);
    p += 4;
    memcpy(g_ram + v2, p, n * 2);
    p += n * 2;
  }
  ++nmi_flag_bg2_enemy_vram_transfer;
}

void QueueEnemyBG2TilemapTransfers(void) {  // 0xA09726
  VramWriteEntry *v0;

  if (nmi_flag_bg2_enemy_vram_transfer && !(debug_time_frozen_for_enemies | time_is_frozen_flag)) {
    v0 = gVramWriteEntry(vram_write_queue_tail);
    v0->size = enemy_bg2_tilemap_size;
    v0->src.addr = ADDR16_OF_RAM(*tilemap_stuff);
    v0->src.bank = 126;
    v0->vram_dst = addr_unk_604800;
    vram_write_queue_tail += 7;
  }
  nmi_flag_bg2_enemy_vram_transfer = 0;
}
