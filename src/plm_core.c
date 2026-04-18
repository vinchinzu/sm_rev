// PLM core lifecycle (spawn, enable, disable, clear, handler loop, instructions).
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"


#define kGoldenTorizoPalette1 ((uint16*)RomFixedPtr(0x848032))
#define kGoldenTorizoPalette2 ((uint16*)RomFixedPtr(0x848132))
#define kXrayBlockDrawingInstrs ((uint16*)RomFixedPtr(0x84839d))
#define kGrayDoorPreInstrs ((uint16*)RomFixedPtr(0x84be4b))
#define off_84E05F ((uint16*)RomFixedPtr(0x84e05f))
#define off_84E077 ((uint16*)RomFixedPtr(0x84e077))
#define kPlmVramAddresses ((uint16*)RomFixedPtr(0x8487cd))
#define kPlmTileDataOffs ((uint16*)RomFixedPtr(0x8487d5))
#define kPlmStartingTileNumber ((uint16*)RomFixedPtr(0x8487dd))

void SetGoldenTorizoPalette(uint16 a) {  // 0x848000
  int16 v1;

  v1 = HIBYTE(a) & 0x78;
  if ((v1 & 0x40) != 0)
    v1 = 56;
  uint16 v2 = (4 * v1) | 0x1E;
  for (int i = 30; i >= 0; i -= 2) {
    int v4 = v2 >> 1;
    int v5 = i >> 1;
    palette_buffer[v5 + 160] = kGoldenTorizoPalette2[v4];
    palette_buffer[v5 + 144] = kGoldenTorizoPalette1[v4];
    v2 -= 2;
  }
}

void LoadRoomPlmGfx(void) {  // 0x848232
  plm_item_gfx_index = 0;
  int v0 = 0;
  do {
    if (plm_item_gfx_ptrs[v0 >> 1]) // bugfix
      PlmInstr_LoadItemPlmGfx(RomPtr_84(plm_item_gfx_ptrs[v0 >> 1]), v0);
    v0 += 2;
  } while (v0 != 8);
}

void ClearSoundsWhenGoingThroughDoor(void) {  // 0x848250
  CallSomeSamusCode(0x1D);
}

void UNUSED_sub_848258(void) {  // 0x848258
  if (samus_movement_type << 8 == 768 || samus_movement_type << 8 == 5120)
    QueueSfx1_Max15(0x32);
}

void PlaySpinJumpSoundIfSpinJumping(void) {  // 0x848270
  CallSomeSamusCode(0x1C);
}

void UNUSED_sub_848278(void) {  // 0x848278
  if (samus_movement_type << 8 == 768 || samus_movement_type << 8 == 5120)
    QueueSfx1_Max15(0x30);
}

void CalculatePlmBlockCoords(uint16 k) {  // 0x848290
  uint16 t = plm_block_indices[k >> 1] >> 1;
  plm_y_block = SnesDivide(t, room_width_in_blocks);
  plm_x_block = SnesModulus(t, room_width_in_blocks);
}

void WriteLevelDataBlockTypeAndBts(uint16 k, uint16 a) {  // 0x8482B4
  uint8 *v2 = (uint8*)&ram7F_start + k;
  v2[3] = HIBYTE(a) | v2[3] & 0xF;
  BTS[k >> 1] = a;
}

void WriteRowOfLevelDataBlockAndBTS(uint16 k, uint16 arg0, uint16 arg1, uint16 arg2) {  // 0x8482D6
  uint16 v2 = plm_block_indices[k >> 1];
  uint16 v8 = v2 >> 1;
  for (int i = 0; i < arg2; i++) {
    level_data[(v2 + i * 2) >> 1] = arg0;
    BTS[v8 + i] = arg1;
  }
}

void LoadXrayBlocks(void) {  // 0x84831A
  for (int i = 78; i >= 0; i -= 2) {
    int v1 = i >> 1;
    if (plm_header_ptr[v1] >= FUNC16(PlmPreInstr_GotoLinkIfTriggered)) {
      uint16 k = i;
      uint16 v2 = item_bit_array[PrepareBitAccess(plm_room_arguments[v1])];
      i = k;
      if ((bitmask & v2) == 0) {
        CalculatePlmBlockCoords(k);
        const uint8 *v3 = RomPtr_84(kXrayBlockDrawingInstrs[plm_variables[k >> 1] >> 1]);
        LoadBlockToXrayTilemap(GET_WORD(v3 + 2) & 0xFFF, plm_x_block, plm_y_block);
        i = k;
      }
    }
  }
  RoomDefRoomstate *RS = get_RoomDefRoomstate(roomdefroomstate_ptr);
  if (RS->xray_special_casing_ptr) {
    const XraySpecialCasing *p = (const XraySpecialCasing *)RomPtr_8F(RS->xray_special_casing_ptr);
    for (; p->x_block || p->y_block; p++)
      LoadBlockToXrayTilemap(p->level_data_block, p->x_block, p->y_block);
  }
}

void EnablePLMs(void) {  // 0x8483AD
  plm_flag |= 0x8000;
}

void DisablePLMs(void) {  // 0x8483B8
  plm_flag &= ~0x8000;
}

void ClearPLMs(void) {  // 0x8483C3
  for (int i = 78; i >= 0; i -= 2)
    plm_header_ptr[i >> 1] = 0;
  plm_item_gfx_index = 0;
}

void SpawnHardcodedPlm(SpawnHardcodedPlmArgs p) {  // 0x8483D7
  SpawnHardcodedPlmArgs *pp = &p;
  
  uint16 v1 = 78;
  while (plm_header_ptr[v1 >> 1]) {
    v1 -= 2;
    if ((v1 & 0x8000) != 0) {
      return;
    }
  }
  uint16 prod = Mult8x8(pp->field_1, room_width_in_blocks);
  plm_block_indices[v1 >> 1] = 2 * (prod + pp->field_0);
  uint16 v3 = pp->field_2;
  plm_header_ptr[v1 >> 1] = v3;
  int v4 = v1 >> 1;
  plm_room_arguments[v4] = 0;
  plm_variables[v4] = 0;
  plm_pre_instrs[v4] = 0x8469;
  PlmHeader_Size4 *PH = get_PlmHeader_Size4(v3);
  plm_instr_list_ptrs[v4] = PH->instr_list_ptr;
  plm_instruction_timer[v4] = 1;
  plm_instruction_draw_ptr[v4] = addr_kDefaultPlmDrawInstruction;
  plm_timers[v4] = 0;
  plm_id = v1;
  CallPlmHeaderFunc(PH->func_ptr | 0x840000, v1);
}

void SpawnRoomPLM(uint16 k) {  // 0x84846A
  RoomPlmEntry *rpe;
  int16 x_block;

  uint16 v1 = 78;
  while (plm_header_ptr[v1 >> 1]) {
    v1 -= 2;
    if ((v1 & 0x8000) != 0)
      return;
  }
  rpe = (k == 0x12) ? door_closing_plm_entry : get_RoomPlmEntry(k);
  uint16 prod = Mult8x8(rpe->y_block, room_width_in_blocks);
  x_block = rpe->x_block;
  int v4 = v1 >> 1;
  plm_block_indices[v4] = 2 * (prod + x_block);
  plm_room_arguments[v4] = rpe->plm_room_argument;
  uint16 pp = rpe->plm_header_ptr_;
  plm_header_ptr[v4] = rpe->plm_header_ptr_;
  plm_variables[v4] = 0;
  plm_pre_instrs[v4] = FUNC16(PlmPreInstr_Empty2);
  PlmHeader_Size4 *PH = get_PlmHeader_Size4(pp);
  plm_instr_list_ptrs[v4] = PH->instr_list_ptr;
  plm_instruction_timer[v4] = 1;
  plm_instruction_draw_ptr[v4] = addr_kDefaultPlmDrawInstruction;
  plm_timers[v4] = 0;
  plm_id = v1;
  
  CallPlmHeaderFunc(PH->func_ptr | 0x840000, v1);
}


// returns bit 0 set if carry, 0x40 if ovf
uint8 SpawnPLM(uint16 a) {  // 0x8484E7
  uint16 v1 = 78;
  while (plm_header_ptr[v1 >> 1]) {
    v1 -= 2;
    if ((v1 & 0x8000) != 0)
      return 0;
  }
  int v3 = v1 >> 1;
  plm_block_indices[v3] = 2 * cur_block_index;
  plm_header_ptr[v3] = a;
  plm_pre_instrs[v3] = FUNC16(PlmPreInstr_Empty3);
  PlmHeader_Size4 *PH = get_PlmHeader_Size4(a);
  plm_instr_list_ptrs[v3] = PH->instr_list_ptr;
  plm_instruction_timer[v3] = 1;
  plm_instruction_draw_ptr[v3] = addr_kDefaultPlmDrawInstruction;
  plm_timers[v3] = 0;
  plm_room_arguments[v3] = 0;
  plm_variables[v3] = 0;
  plm_id = v1;
  return CallPlmHeaderFunc(PH->func_ptr | 0x840000, v1);
}

CoroutineRet PlmHandler_Async(void) {  // 0x8485B4
  COROUTINE_BEGIN(coroutine_state_2, 0);
  if ((plm_flag & 0x8000) == 0)
    return kCoroutineNone;
  plm_draw_tilemap_index = 0;
  for (plm_id = 78; (plm_id & 0x8000) == 0; plm_id -= 2) {
    if (!plm_header_ptr[plm_id >> 1])
      continue;
    CallPlmPreInstr(plm_pre_instrs[plm_id >> 1] | 0x840000, plm_id);
    if (--plm_instruction_timer[plm_id >> 1])
      continue;
    const uint8 *base = RomBankBase(0x84), *v5;
    v5 = base + plm_instr_list_ptrs[plm_id >> 1];
    while (1) {
      if ((GET_WORD(v5) & 0x8000) == 0)
        break;
      v5 = CallPlmInstr(GET_WORD(v5) | 0x840000, v5 + 2, plm_id);
      if (!v5)
        goto NEXT_PLM;
      if ((uintptr_t)v5 < 0x10000)
        v5 = base + (uintptr_t)v5;
      // If the plm handler wanted to display a message, then display it.
      if (queued_message_box_index != 0) {
        plm_instr_list_ptrs[plm_id >> 1] = v5 - base;
        COROUTINE_AWAIT(1, DisplayMessageBox_Async(queued_message_box_index));
        queued_message_box_index = 0;
        base = RomBankBase(0x84);
        v5 = base + plm_instr_list_ptrs[plm_id >> 1];
      }
    }
    int v7;
    v7 = plm_id >> 1;
    plm_instruction_timer[v7] = GET_WORD(v5);
    plm_instruction_draw_ptr[v7] = GET_WORD(v5 + 2);
    plm_instr_list_ptrs[v7] = v5 + 4 - base;
    ProcessPlmDrawInstruction(plm_id);
    CalculatePlmBlockCoords(plm_id);
    DrawPlm(plm_id);
NEXT_PLM:;
  }
  plm_id = 0;
  COROUTINE_END(0);
}

const uint8 *PlmInstr_Sleep(const uint8 *plmp, uint16 k) {  // 0x8486B4
  plm_instr_list_ptrs[k >> 1] = plmp - RomBankBase(0x84) - 2;
  return 0;
}

const uint8 *PlmInstr_Delete(const uint8 *plmp, uint16 k) {  // 0x8486BC
  plm_header_ptr[k >> 1] = 0;
  return 0;
}

const uint8 *PlmInstr_PreInstr(const uint8 *plmp, uint16 k) {  // 0x8486C1
  plm_pre_instrs[k >> 1] = GET_WORD(plmp);
  return plmp + 2;
}

const uint8 *PlmInstr_ClearPreInstr(const uint8 *plmp, uint16 k) {  // 0x8486CA
  plm_pre_instrs[k >> 1] = FUNC16(PlmPreInstr_Empty);
  return plmp;
}

const uint8 *PlmInstr_CallFunction(const uint8 *plmp, uint16 k) {  // 0x84870B
  CallPlmInstrFunc(Load24((LongPtr *)plmp));
  return plmp + 3;
}

const uint8 *PlmInstr_Goto(const uint8 *plmp, uint16 k) {  // 0x848724
  return INSTRB_RETURN_ADDR(GET_WORD(plmp));
}


const uint8 *PlmInstr_DecrementAndBranchNonzero(const uint8 *plmp, uint16 k) {  // 0x84873F
  if (plm_timers[k >> 1]-- == 1)
    return plmp + 2;
  else
    return PlmInstr_Goto(plmp, k);
}

const uint8 *PlmInstr_SetTimer(const uint8 *plmp, uint16 k) {  // 0x84874E
  *((uint8 *)plm_timers + k) = *plmp;
  return plmp + 1;
}

const uint8 *PlmInstr_LoadItemPlmGfx(const uint8 *plmp, uint16 k) {  // 0x848764
  uint16 v2 = plm_item_gfx_index;
  plm_variables[k >> 1] = plm_item_gfx_index;
  plm_item_gfx_index = (v2 + 2) & 6;
  int v3 = v2 >> 1;
  int r18 = kPlmVramAddresses[v3];
  int r20 = kPlmTileDataOffs[v3];
  int R22 = kPlmStartingTileNumber[v3];
  plm_item_gfx_ptrs[v3] = plmp - RomBankBase(0x84);
  int v4 = vram_write_queue_tail;
  VramWriteEntry *v5 = gVramWriteEntry(vram_write_queue_tail);
  v5->size = 256;
  v5->src.addr = GET_WORD(plmp);
  v5->src.bank = 0x89;
  v5->vram_dst = r18;
  vram_write_queue_tail = v4 + 7;
  plmp += 2;
  int v7 = r20;
  int R24 = r20 + 16;
  do {
    *(uint16 *)((uint8 *)&tile_table.tables[0].top_left + v7) = R22 + (plmp[0] << 10);
    ++R22;
    plmp++;
    v7 += 2;
  } while (v7 != R24);
  return plmp;
}

const uint8 *PlmInstr_CopyFromRamToVram(const uint8 *plmp, uint16 k) {  // 0x8487E5
  uint16 v2 = vram_write_queue_tail;
  VramWriteEntry *v4 = gVramWriteEntry(vram_write_queue_tail);
  v4->size = GET_WORD(plmp);
  v4->src.addr = GET_WORD(plmp + 2);
  v4->src.bank = plmp[4];
  v4->vram_dst = GET_WORD(plmp + 5);
  vram_write_queue_tail = v2 + 7;
  return plmp + 7;
}

const uint8 *PlmInstr_GotoIfBossBitSet(const uint8 *plmp, uint16 k) {  // 0x84880E
  if (CheckBossBitForCurArea(plmp[0]))
    return PlmInstr_Goto(plmp + 1, k);
  else
    return plmp + 3;
}

const uint8 *PlmInstr_GotoIfEventSet(const uint8 *plmp, uint16 k) {  // 0x84882D
  if (CheckEventHappened(GET_WORD(plmp)))
    return PlmInstr_Goto(plmp + 2, k);
  else
    return plmp + 4;
}

const uint8 *PlmInstr_SetEvent(const uint8 *plmp, uint16 k) {  // 0x84883E
  SetEventHappened(GET_WORD(plmp));
  return plmp + 2;
}

const uint8 *PlmInstr_GotoIfChozoSet(const uint8 *plmp, uint16 k) {  // 0x848848
  uint16 v3 = plm_room_arguments[k >> 1];
  int idx = PrepareBitAccess(v3);
  if (!sign16(v3) && ((bitmask & room_chozo_bits[idx]) != 0))
    return PlmInstr_Goto(plmp, k);
  else
    return plmp + 2;
}

const uint8 *PlmInstr_SetRoomChozoBit(const uint8 *plmp, uint16 k) {  // 0x848865
  uint16 v3 = plm_room_arguments[k >> 1];
  if (!sign16(v3)) {
    int idx = PrepareBitAccess(v3);
    room_chozo_bits[idx] |= bitmask;
  }
  return plmp;
}

const uint8 *PlmInstr_GotoIfItemBitSet(const uint8 *plmp, uint16 k) {  // 0x84887C
  uint16 v2 = plm_room_arguments[k >> 1];
  int idx = PrepareBitAccess(v2);
  if (!sign16(v2) && (bitmask & item_bit_array[idx]) != 0)
    return PlmInstr_Goto(plmp, k);
  else
    return plmp + 2;
}

const uint8 *PlmInstr_SetItemBit(const uint8 *plmp, uint16 k) {  // 0x848899
  uint16 v2 = plm_room_arguments[k >> 1];
  if (!sign16(v2)) {
    uint16 v3 = PrepareBitAccess(v2);
    item_bit_array[v3] |= bitmask;
  }
  return plmp;
}

const uint8 *PlmInstr_PickupBeamAndShowMessage(const uint8 *plmp, uint16 k) {  // 0x8488B0
  uint16 t = GET_WORD(plmp);
  collected_beams |= t;
  equipped_beams |= t;
  equipped_beams &= ~((t << 1) & 8);
  equipped_beams &= ~((t >> 1) & 4);
  UpdateBeamTilesAndPalette();
  PlayRoomMusicTrackAfterAFrames(0x168);
  DisplayMessageBox(plmp[2]);
  return plmp + 3;
}

const uint8 *PlmInstr_PickupEquipmentAndShowMessage(const uint8 *plmp, uint16 k) {  // 0x8488F3
  uint16 t = GET_WORD(plmp);
  equipped_items |= t;
  collected_items |= t;
  PlayRoomMusicTrackAfterAFrames(0x168);
  DisplayMessageBox(plmp[2]);
  return plmp + 3;
}

const uint8 *PlmInstr_PickupEquipmentAddGrappleShowMessage(const uint8 *plmp, uint16 k) {  // 0x84891A
  uint16 t = GET_WORD(plmp);
  equipped_items |= t;
  collected_items |= t;
  AddGrappleToHudTilemap();
  PlayRoomMusicTrackAfterAFrames(0x168);
  DisplayMessageBox(5);
  return plmp + 2;
}

const uint8 *PlmInstr_PickupEquipmentAddXrayShowMessage(const uint8 *plmp, uint16 k) {  // 0x848941
  uint16 t = GET_WORD(plmp);
  equipped_items |= t;
  collected_items |= t;
  AddXrayToHudTilemap();
  PlayRoomMusicTrackAfterAFrames(0x168);
  DisplayMessageBox(6);
  return plmp + 2;
}

const uint8 *PlmInstr_CollectHealthEnergyTank(const uint8 *plmp, uint16 k) {  // 0x848968
  samus_max_health += GET_WORD(plmp);
  samus_health = samus_max_health;
  PlayRoomMusicTrackAfterAFrames(0x168);
  DisplayMessageBox(1);
  return plmp + 2;
}

const uint8 *PlmInstr_CollectHealthReserveTank(const uint8 *plmp, uint16 k) {  // 0x848986
  samus_max_reserve_health += GET_WORD(plmp);
  if (!reserve_health_mode)
    ++reserve_health_mode;
  PlayRoomMusicTrackAfterAFrames(0x168);
  DisplayMessageBox(0x19);
  return plmp + 2;
}

const uint8 *PlmInstr_CollectAmmoMissileTank(const uint8 *plmp, uint16 k) {  // 0x8489A9
  uint16 t = GET_WORD(plmp);
  samus_max_missiles += t;
  samus_missiles += t;
  AddMissilesToHudTilemap();
  PlayRoomMusicTrackAfterAFrames(0x168);
  DisplayMessageBox(2);
  return plmp + 2;
}

const uint8 *PlmInstr_CollectAmmoSuperMissileTank(const uint8 *plmp, uint16 k) {  // 0x8489D2
  uint16 t = GET_WORD(plmp);
  samus_max_super_missiles += t;
  samus_super_missiles += t;
  AddSuperMissilesToHudTilemap();
  PlayRoomMusicTrackAfterAFrames(0x168);
  DisplayMessageBox(3);
  return plmp + 2;
}

const uint8 *PlmInstr_CollectAmmoPowerBombTank(const uint8 *plmp, uint16 k) {  // 0x8489FB
  uint16 t = GET_WORD(plmp);
  samus_max_power_bombs += t;
  samus_power_bombs += t;
  AddPowerBombsToHudTilemap();
  PlayRoomMusicTrackAfterAFrames(0x168);
  DisplayMessageBox(4);
  return plmp + 2;
}

const uint8 *PlmInstr_SetLinkReg(const uint8 *plmp, uint16 k) {  // 0x848A24
  plm_instruction_list_link_reg[k >> 1] = GET_WORD(plmp);
  return plmp + 2;
}

const uint8 *PlmInstr_Call(const uint8 *plmp, uint16 k) {  // 0x848A2E
  plm_instruction_list_link_reg[k >> 1] = plmp - RomBankBase(0x84) + 2;
  return INSTRB_RETURN_ADDR(GET_WORD(plmp));
}

const uint8 *PlmInstr_Return(const uint8 *plmp, uint16 k) {  // 0x848A3A
  return INSTRB_RETURN_ADDR(plm_instruction_list_link_reg[k >> 1]);
}

const uint8 *PlmInstr_GotoIfDoorBitSet(const uint8 *plmp, uint16 k) {  // 0x848A72
  uint16 v2 = plm_room_arguments[k >> 1];
  if (sign16(v2))
    return plmp + 2;
  int idx = PrepareBitAccess(v2);
  if (!sign16(v2) && (bitmask & opened_door_bit_array[idx]) != 0)
    return INSTRB_RETURN_ADDR(GET_WORD(plmp));
  else
    return plmp + 2;
}

const uint8 *PlmInstr_IncrementDoorHitCounterAndJGE(const uint8 *plmp, uint16 k) {  // 0x848A91
  int v2 = k >> 1;
  if ((uint8)++plm_variables[v2] < plmp[0])
    return plmp + 3;
  uint16 v5 = plm_room_arguments[v2];
  if (!sign16(v5)) {
    int idx = PrepareBitAccess(v5);
    opened_door_bit_array[idx] |= bitmask;
    plm_room_arguments[v2] = WORD(opened_door_bit_array[idx]) | 0x8000;
  }
  plm_pre_instrs[v2] = addr_PlmPreInstr_Empty4;
  return PlmInstr_Goto(plmp + 1, k);
}

const uint8 *PlmInstr_IncrementArgumentAndJGE(const uint8 *plmp, uint16 k) {  // 0x848ACD
  uint8 v2 = *((uint8 *)plm_room_arguments + k) + 1;
  if (v2 >= plmp[0]) {
    int v4 = k >> 1;
    plm_room_arguments[v4] = -1;
    plm_pre_instrs[v4] = addr_locret_848AE0;
    return PlmInstr_Goto(plmp + 1, k);
  } else {
    plm_room_arguments[k >> 1] = v2;
    return plmp + 3;
  }
}

const uint8 *PlmInstr_SetBTS(const uint8 *plmp, uint16 k) {  // 0x848AF1
  BTS[plm_block_indices[k >> 1] >> 1] = plmp[0];
  return plmp + 1;
}

const uint8 *PlmInstr_DrawPlmBlock(const uint8 *plmp, uint16 k) {  // 0x848B17
  int v2 = k >> 1;
  uint16 v3 = plm_variable[v2];
  level_data[plm_block_indices[v2] >> 1] = v3;
  custom_draw_instr_plm_block = v3;
  custom_draw_instr_num_blocks = 1;
  custom_draw_instr_zero_terminator = 0;
  plm_instruction_timer[v2] = 1;
  plm_instruction_draw_ptr[v2] = ADDR16_OF_RAM(custom_draw_instr_num_blocks);
  plm_instr_list_ptrs[v2] = plmp - RomBankBase(0x84);
  ProcessPlmDrawInstruction(k);
  uint16 v4 = plm_id;
  CalculatePlmBlockCoords(plm_id);
  DrawPlm(v4);
  return 0;
}

const uint8 *PlmInstr_ProcessAirScrollUpdate(const uint8 *plmp, uint16 k) {  // 0x848B55
  plm_variable[k >> 1] = 0;
  const uint8 *v5 = RomPtr_8F(plm_room_arguments[k >> 1]);
  while (1) {
    if (v5[0] & 0x80)
      break;
    scrolls[v5[0]] = v5[1];
    v5 += 2;
  }
  int v7 = plm_block_indices[k >> 1] >> 1;
  level_data[v7] = level_data[v7] & 0xFFF | 0x3000;
  return plmp;
}

const uint8 *PlmInstr_ProcessSolidScrollUpdate(const uint8 *plmp, uint16 k) {  // 0x848B93
  plm_variable[k >> 1] = 0;
  const uint8 *v5 = RomPtr_8F(plm_room_arguments[k >> 1]);
  while (1) {
    if (v5[0] & 0x80)
      break;
    scrolls[v5[0]] = v5[1];
    v5 += 2;
  }
  int v7 = plm_block_indices[k >> 1] >> 1;
  level_data[v7] = level_data[v7] & 0xFFF | 0xB000;
  return plmp;
}

const uint8 *PlmInstr_QueueMusic(const uint8 *plmp, uint16 k) {  // 0x848BD1
  QueueMusic_Delayed8(plmp[0]);
  return plmp + 1;
}

const uint8 *PlmInstr_ClearMusicQueueAndQueueTrack(const uint8 *plmp, uint16 k) {  // 0x848BDD
  for (int i = 14; i >= 0; i -= 2) {
    int v3 = i >> 1;
    music_queue_track[v3] = 0;
    music_queue_delay[v3] = 0;
  }
  music_queue_read_pos = music_queue_write_pos;
  music_timer = 0;
  music_entry = 0;
  QueueMusic_Delayed8(plmp[0]);
  return plmp + 1;
}

const uint8 *PlmInstr_QueueSfx1_Max6(const uint8 *plmp, uint16 k) {  // 0x848C07
  QueueSfx1_Max6(plmp[0]);
  return plmp + 1;
}

const uint8 *PlmInstr_QueueSfx2_Max6(const uint8 *plmp, uint16 k) {  // 0x848C10
  QueueSfx2_Max6(plmp[0]);
  return plmp + 1;
}

const uint8 *PlmInstr_QueueSfx3_Max6(const uint8 *plmp, uint16 k) {  // 0x848C19
  QueueSfx3_Max6(plmp[0]);
  return plmp + 1;
}

const uint8 *PlmInstr_QueueSfx1_Max15(const uint8 *plmp, uint16 k) {  // 0x848C22
  QueueSfx1_Max15(plmp[0]);
  return plmp + 1;
}

const uint8 *PlmInstr_QueueSfx2_Max15(const uint8 *plmp, uint16 k) {  // 0x848C2B
  QueueSfx2_Max15(plmp[0]);
  return plmp + 1;
}

const uint8 *PlmInstr_QueueSfx3_Max15(const uint8 *plmp, uint16 k) {  // 0x848C34
  QueueSfx3_Max15(plmp[0]);
  return plmp + 1;
}

const uint8 *PlmInstr_QueueSfx1_Max3(const uint8 *plmp, uint16 k) {  // 0x848C3D
  QueueSfx1_Max3(plmp[0]);
  return plmp + 1;
}

const uint8 *PlmInstr_QueueSfx2_Max3(const uint8 *plmp, uint16 k) {  // 0x848C46
  QueueSfx2_Max3(plmp[0]);
  return plmp + 1;
}

const uint8 *PlmInstr_QueueSfx_Max3(const uint8 *plmp, uint16 k) {  // 0x848C4F
  QueueSfx3_Max3(plmp[0]);
  return plmp + 1;
}

const uint8 *PlmInstr_QueueSfx1_Max9(const uint8 *plmp, uint16 k) {  // 0x848C58
  QueueSfx1_Max9(plmp[0]);
  return plmp + 1;
}

const uint8 *PlmInstr_QueueSfx2_Max9(const uint8 *plmp, uint16 k) {  // 0x848C61
  QueueSfx2_Max9(plmp[0]);
  return plmp + 1;
}

const uint8 *PlmInstr_QueueSfx3_Max9(const uint8 *plmp, uint16 k) {  // 0x848C6A
  QueueSfx3_Max9(plmp[0]);
  return plmp + 1;
}

const uint8 *PlmInstr_QueueSfx1_Max1(const uint8 *plmp, uint16 k) {  // 0x848C73
  QueueSfx1_Max1(plmp[0]);
  return plmp + 1;
}

const uint8 *PlmInstr_QueueSfx2_Max1(const uint8 *plmp, uint16 k) {  // 0x848C7C
  QueueSfx2_Max1(plmp[0]);
  return plmp + 1;
}

const uint8 *PlmInstr_QueueSfx3_Max1(const uint8 *plmp, uint16 k) {  // 0x848C85
  QueueSfx3_Max1(plmp[0]);
  return plmp + 1;
}

const uint8 *PlmInstr_ActivateMapStation(const uint8 *plmp, uint16 k) {  // 0x848C8F
  *(uint16 *)&map_station_byte_array[area_index] |= 0xFF;
  DisplayMessageBox(0x14);
  has_area_map = 1;
  return plmp;
}

const uint8 *PlmInstr_ActivateEnergyStation(const uint8 *plmp, uint16 k) {  // 0x848CAF
  if (samus_max_health != samus_health) {
    DisplayMessageBox(0x15);
    samus_health = samus_max_health;
  }
  CallSomeSamusCode(1);
  return plmp;
}

const uint8 *PlmInstr_ActivateMissileStation(const uint8 *plmp, uint16 k) {  // 0x848CD0
  if (samus_max_missiles != samus_missiles) {
    DisplayMessageBox(0x16);
    samus_missiles = samus_max_missiles;
  }
  CallSomeSamusCode(1);
  return plmp;
}

const uint8 *PlmInstr_ActivateSaveStationAndGotoIfNo(const uint8 *plmp, uint16 k) {  // 0x848CF1
  int r = DisplayMessageBox_Poll(23);
  if (r < 0)
    return plmp - 2; // restart plm instr
  if (r == 2)
    return INSTRB_RETURN_ADDR(GET_WORD(plmp));
  SpawnEprojWithRoomGfx(addr_kEproj_SaveStationElectricity, 0);
  load_station_index = plm_room_arguments[plm_id >> 1] & 7;
  PrepareBitAccess(load_station_index);
  used_save_stations_and_elevators[2 * area_index] |= bitmask;
  SaveToSram(selected_save_slot);
  return plmp + 2;
}

const uint8 *PlmInstr_GotoIfSamusNear(const uint8 *plmp, uint16 k) {  // 0x848D41
  CalculatePlmBlockCoords(k);
  uint8 v2 = abs16((samus_x_pos >> 4) - plm_x_block);
  uint8 v4 = abs16((samus_y_pos >> 4) - plm_y_block);
  if (v2 <= plmp[0] && v4 <= plmp[1])
    return INSTRB_RETURN_ADDR(GET_WORD(plmp + 2));
  else
    return plmp + 4;
}


const uint8 *PlmInstr_MovePlmDownOneBlock(const uint8 *plmp, uint16 k) {  // 0x84AB00
  plm_block_indices[k >> 1] += 2 * room_width_in_blocks;
  return plmp;
}


const uint8 *PlmInstr_Scroll_0_1_Blue(const uint8 *plmp, uint16 k) {  // 0x84AB51
  *(uint16 *)scrolls = 257;
  return plmp;
}

const uint8 *PlmInstr_MovePlmDownOneBlock_0(const uint8 *plmp, uint16 k) {  // 0x84AB59
  plm_block_indices[k >> 1] += 2 * room_width_in_blocks;
  return plmp;
}

const uint8 *PlmInstr_ABD6(const uint8 *plmp, uint16 k) {  // 0x84ABD6
  plm_block_indices[k >> 1] += 2;
  return plmp;
}


const uint8 *PlmInstr_DealDamage_2(const uint8 *plmp, uint16 k) {  // 0x84AC9D
  samus_periodic_damage += 2;
  return plmp;
}

const uint8 *PlmInstr_GiveInvincibility(const uint8 *plmp, uint16 k) {  // 0x84ACB1
  samus_invincibility_timer = 48;
  return plmp;
}

const uint8 *PlmInstr_Draw0x38FramesOfRightTreadmill(const uint8 *plmp, uint16 k) {  // 0x84AD43
  WriteRowOfLevelDataBlockAndBTS(k, 0x30ff, 0x8, 0x38);
  return plmp;
}

const uint8 *PlmInstr_Draw0x38FramesOfLeftTreadmill(const uint8 *plmp, uint16 k) {  // 0x84AD58
  WriteRowOfLevelDataBlockAndBTS(k, 0x30ff, 0x9, 0x38);
  return plmp;
}

const uint8 *PlmInstr_GotoIfSamusHealthFull(const uint8 *plmp, uint16 k) {  // 0x84AE35
  if (samus_max_health != samus_health)
    return plmp + 2;
  CallSomeSamusCode(1);
  return INSTRB_RETURN_ADDR(GET_WORD(plmp));
}

const uint8 *PlmInstr_GotoIfMissilesFull(const uint8 *plmp, uint16 k) {  // 0x84AEBF
  if (samus_max_missiles != samus_missiles)
    return plmp + 2;
  CallSomeSamusCode(1);
  return INSTRB_RETURN_ADDR(GET_WORD(plmp));
}

const uint8 *PlmInstr_PlaceSamusOnSaveStation(const uint8 *plmp, uint16 k) {  // 0x84B00E
  samus_x_pos = (samus_x_pos + 8) & 0xFFF0;
  MakeSamusFaceForward();
  return plmp;
}

const uint8 *PlmInstr_DisplayGameSavedMessageBox(const uint8 *plmp, uint16 k) {  // 0x84B024
  DisplayMessageBox(0x18);
  return plmp;
}

const uint8 *PlmInstr_EnableMovementAndSetSaveStationUsed(const uint8 *plmp, uint16 k) {  // 0x84B030
  CallSomeSamusCode(1);
  save_station_lockout_flag = 1;
  return plmp;
}

























































const uint8 *PlmInstr_SetCrittersEscapedEvent(const uint8 *plmp, uint16 k) {  // 0x84B9B9
  SetEventHappened(0xF);
  return plmp;
}



const uint8 *PlmInstr_JumpIfSamusHasNoBombs(const uint8 *plmp, uint16 k) {  // 0x84BA6F
  if ((collected_items & 0x1000) != 0)
    return plmp + 2;
  else
    return INSTRB_RETURN_ADDR(GET_WORD(plmp));
}


const uint8 *PlmInstr_MovePlmRight4Blocks(const uint8 *plmp, uint16 k) {  // 0x84BB25
  plm_block_indices[k >> 1] += 8;
  return plmp;
}




const uint8 *PlmInstr_ClearTrigger(const uint8 *plmp, uint16 k) {  // 0x84BBDD
  plm_timers[k >> 1] = 0;
  return plmp;
}

const uint8 *PlmInstr_SpawnEproj(const uint8 *plmp, uint16 k) {  // 0x84BBE1
  SpawnEprojWithRoomGfx(GET_WORD(plmp), 0);
  return plmp + 2;
}

const uint8 *PlmInstr_WakeEprojAtPlmPos(const uint8 *plmp, uint16 k) {  // 0x84BBF0
  int i;

  uint16 v2 = plm_block_indices[k >> 1];
  for (i = 34; i >= 0; i -= 2) {
    if (v2 == eproj_E[i >> 1])
      break;
  }
  int v4 = i >> 1;
  eproj_instr_timers[v4] = 1;
  eproj_instr_list_ptr[v4] += 2;
  return plmp + 2;
}













const uint8 *PlmInstr_SetGreyDoorPreInstr(const uint8 *plmp, uint16 k) {  // 0x84BE3F
  plm_pre_instrs[k >> 1] = kGrayDoorPreInstrs[plm_variable[k >> 1] >> 1];
  return plmp;
}


























const uint8 *PlmInstr_SetBtsTo1(const uint8 *plmp, uint16 k) {  // 0x84CD93
  BTS[plm_block_indices[k >> 1] >> 1] = 1;
  return plmp;
}

















const uint8 *PlmInstr_FxBaseYPos_0x2D2(const uint8 *plmp, uint16 k) {  // 0x84D155
  fx_base_y_pos = 722;
  return plmp;
}




const uint8 *PlmInstr_GotoIfRoomArgLess(const uint8 *plmp, uint16 k) {  // 0x84D2F9
  if (plm_room_arguments[k >> 1] >= GET_WORD(plmp))
    return plmp + 4;
  else
    return INSTRB_RETURN_ADDR(GET_WORD(plmp + 2));
}

const uint8 *PlmInstr_SpawnFourMotherBrainGlass(const uint8 *plmp, uint16 k) {  // 0x84D30B
  QueueSfx3_Max15(0x2E);
  SpawnMotherBrainGlassShatteringShard(GET_WORD(plmp + 0));
  SpawnMotherBrainGlassShatteringShard(GET_WORD(plmp + 2));
  SpawnMotherBrainGlassShatteringShard(GET_WORD(plmp + 4));
  SpawnMotherBrainGlassShatteringShard(GET_WORD(plmp + 6));
  return plmp + 8;
}



const uint8 *PlmInstr_SpawnTorizoStatueBreaking(const uint8 *plmp, uint16 k) {  // 0x84D357
  SpawnEprojWithRoomGfx(0xA993, GET_WORD(plmp));
  return plmp + 2;
}

const uint8 *PlmInstr_QueueSong1MusicTrack(const uint8 *plmp, uint16 k) {  // 0x84D3C7
  QueueMusic_Delayed8(6);
  return plmp;
}

const uint8 *PlmInstr_TransferWreckedShipChozoSpikesToSlopes(const uint8 *plmp, uint16 k) {  // 0x84D3D7
  WriteLevelDataBlockTypeAndBts(0x1608, 0x1012);
  WriteLevelDataBlockTypeAndBts(0x160A, 0x1013);
  return plmp;
}

const uint8 *PlmInstr_TransferWreckedShipSlopesToChozoSpikes(const uint8 *plmp, uint16 k) {  // 0x84D3F4
  WriteLevelDataBlockTypeAndBts(0x1608, 0xA000);
  WriteLevelDataBlockTypeAndBts(0x160A, 0xA000);
  return plmp;
}


const uint8 *PlmInstr_EnableWaterPhysics(const uint8 *plmp, uint16 k) {  // 0x84D525
  fx_liquid_options &= ~4;
  return plmp;
}

const uint8 *PlmInstr_SpawnN00bTubeCrackEproj(const uint8 *plmp, uint16 k) {  // 0x84D52C
  SpawnEprojWithRoomGfx(0xD904, 0);
  return plmp;
}

const uint8 *PlmInstr_DiagonalEarthquake(const uint8 *plmp, uint16 k) {  // 0x84D536
  earthquake_type = 11;
  earthquake_timer = 64;
  return plmp;
}

const uint8 *PlmInstr_Spawn10shardsAnd6n00bs(const uint8 *plmp, uint16 k) {  // 0x84D543
  SpawnEprojWithRoomGfx(addr_kEproj_N00bTubeShards, 0);
  SpawnEprojWithRoomGfx(addr_kEproj_N00bTubeShards, 2);
  SpawnEprojWithRoomGfx(addr_kEproj_N00bTubeShards, 4);
  SpawnEprojWithRoomGfx(addr_kEproj_N00bTubeShards, 6);
  SpawnEprojWithRoomGfx(addr_kEproj_N00bTubeShards, 8);
  SpawnEprojWithRoomGfx(addr_kEproj_N00bTubeShards, 0xA);
  SpawnEprojWithRoomGfx(addr_kEproj_N00bTubeShards, 0xC);
  SpawnEprojWithRoomGfx(addr_kEproj_N00bTubeShards, 0xE);
  SpawnEprojWithRoomGfx(addr_kEproj_N00bTubeShards, 0x10);
  SpawnEprojWithRoomGfx(addr_kEproj_N00bTubeShards, 0x12);
  SpawnEprojWithRoomGfx(addr_kEproj_N00bTubeReleasedAirBubbles, 0);
  SpawnEprojWithRoomGfx(addr_kEproj_N00bTubeReleasedAirBubbles, 2);
  SpawnEprojWithRoomGfx(addr_kEproj_N00bTubeReleasedAirBubbles, 4);
  SpawnEprojWithRoomGfx(addr_kEproj_N00bTubeReleasedAirBubbles, 6);
  SpawnEprojWithRoomGfx(addr_kEproj_N00bTubeReleasedAirBubbles, 8);
  SpawnEprojWithRoomGfx(addr_kEproj_N00bTubeReleasedAirBubbles, 0xA);
  return plmp;
}

const uint8 *PlmInstr_DisableSamusControls(const uint8 *plmp, uint16 k) {  // 0x84D5E6
  CallSomeSamusCode(0);
  return plmp;
}

const uint8 *PlmInstr_EnableSamusControls(const uint8 *plmp, uint16 k) {  // 0x84D5EE
  CallSomeSamusCode(1);
  return plmp;
}










const uint8 *PlmInstr_ShootEyeDoorProjectileWithProjectileArg(const uint8 *plmp, uint16 k) {  // 0x84D77A
  SpawnEprojWithRoomGfx(addr_kEproj_EyeDoorProjectile, GET_WORD(plmp));
  QueueSfx2_Max6(0x4C);
  return plmp + 2;
}

const uint8 *PlmInstr_SpawnEyeDoorSweatEproj(const uint8 *plmp, uint16 k) {  // 0x84D790
  SpawnEprojWithRoomGfx(addr_kEproj_EyeDoorSweat, GET_WORD(plmp));
  return plmp + 2;
}

const uint8 *PlmInstr_SpawnTwoEyeDoorSmoke(const uint8 *plmp, uint16 k) {  // 0x84D79F
  SpawnEprojWithRoomGfx(0xE517, 0x30A);
  SpawnEprojWithRoomGfx(0xE517, 0x30A);
  return plmp;
}

const uint8 *PlmInstr_SpawnEyeDoorSmokeProjectile(const uint8 *plmp, uint16 k) {  // 0x84D7B6
  SpawnEprojWithRoomGfx(addr_kEproj_EyeDoorSmoke, 0xB);
  return plmp;
}

const uint8 *PlmInstr_MoveUpAndMakeBlueDoorFacingRight(const uint8 *plmp, uint16 k) {  // 0x84D7C3
  int v2 = k >> 1;
  uint16 blk = (plm_block_indices[v2] -= room_width_in_blocks * 2);
  WriteLevelDataBlockTypeAndBts(blk, 0xC041);
  sub_84D7EF(blk);
  return plmp;
}

const uint8 *PlmInstr_MoveUpAndMakeBlueDoorFacingLeft(const uint8 *plmp, uint16 k) {  // 0x84D7DA
  int v2 = k >> 1;
  uint16 blk = (plm_block_indices[v2] -= room_width_in_blocks * 2);
  WriteLevelDataBlockTypeAndBts(blk, 0xC040);
  sub_84D7EF(blk);
  return plmp;
}











const uint8 *PlmInstr_DamageDraygonTurret(const uint8 *plmp, uint16 k) {  // 0x84DB8E
  SetPlmVarPtr(k, 1);
  uint16 v2 = plm_block_indices[k >> 1];
  WriteLevelDataBlockTypeAndBts(v2, 0xA003);
  WriteLevelDataBlockTypeAndBts(room_width_in_blocks * 2 + v2, 0xA003);
  return plmp;
}

const uint8 *PlmInstr_DamageDraygonTurretFacingDownRight(const uint8 *plmp, uint16 k) {  // 0x84DBB8
  SetPlmVarPtr(k, 1);
  uint16 v2 = plm_block_indices[k >> 1];
  WriteLevelDataBlockTypeAndBts(v2, 0xA003);
  WriteLevelDataBlockTypeAndBts(v2 + 2, 0xA003);
  uint16 v3 = plm_block_indices[plm_id >> 1];
  uint16 v4 = room_width_in_blocks * 2 + v3;
  WriteLevelDataBlockTypeAndBts(v4, 0xA003);
  WriteLevelDataBlockTypeAndBts(v4 + 2, 0);
  return plmp;
}

const uint8 *PlmInstr_DamageDraygonTurretFacingUpRight(const uint8 *plmp, uint16 k) {  // 0x84DBF7
  SetPlmVarPtr(k, 1);
  uint16 v2 = plm_block_indices[k >> 1];
  WriteLevelDataBlockTypeAndBts(v2, 0xA003);
  WriteLevelDataBlockTypeAndBts(v2 + 2, 0);
  uint16 v4 = room_width_in_blocks * 2 + v2;
  WriteLevelDataBlockTypeAndBts(v4, 0xA003);
  WriteLevelDataBlockTypeAndBts(v4 + 2, 0xA003);
  return plmp;
}

const uint8 *PlmInstr_DamageDraygonTurret2(const uint8 *plmp, uint16 k) {  // 0x84DC36
  SetPlmVarPtr(k, 1);
  uint16 v2 = plm_block_indices[k >> 1];
  WriteLevelDataBlockTypeAndBts(v2, 0xA003);
  WriteLevelDataBlockTypeAndBts(room_width_in_blocks * 2 + v2, 0xA003);
  return plmp;
}

const uint8 *PlmInstr_DamageDraygonTurretFacingDownLeft(const uint8 *plmp, uint16 k) {  // 0x84DC60
  SetPlmVarPtr(k, 1);
  uint16 v2 = plm_block_indices[k >> 1];
  WriteLevelDataBlockTypeAndBts(v2, 0xA003);
  WriteLevelDataBlockTypeAndBts(v2 - 2, 0xA003);
  uint16 v4 = room_width_in_blocks * 2 + v2;
  WriteLevelDataBlockTypeAndBts(v4, 0xA003);
  WriteLevelDataBlockTypeAndBts(v4 - 2, 0);
  return plmp;
}

const uint8 *PlmInstr_DamageDraygonTurretFacingUpLeft(const uint8 *plmp, uint16 k) {  // 0x84DC9F
  SetPlmVarPtr(k, 1);
  uint16 v2 = plm_block_indices[k >> 1];
  WriteLevelDataBlockTypeAndBts(v2, 0xA003);
  WriteLevelDataBlockTypeAndBts(v2 - 2, 0);
  uint16 v4 = v2 + room_width_in_blocks * 2;
  WriteLevelDataBlockTypeAndBts(v4, 0xA003);
  WriteLevelDataBlockTypeAndBts(v4 - 2, 0xA003);
  return plmp;
}








const uint8 *PlmInstr_DrawItemFrame_Common(const uint8 *plmp, uint16 k) {  // 0x84E07F
  int v2 = k >> 1;
  plm_instruction_timer[v2] = 4;
  plm_instr_list_ptrs[v2] = plmp - RomBankBase(0x84);
  ProcessPlmDrawInstruction(k);
  uint16 v3 = plm_id;
  CalculatePlmBlockCoords(plm_id);
  DrawPlm(v3);
  return 0;
}

const uint8 *PlmInstr_DrawItemFrame0(const uint8 *plmp, uint16 k) {  // 0x84E04F
  plm_instruction_draw_ptr[k >> 1] = off_84E05F[plm_variables[k >> 1] >> 1];
  return PlmInstr_DrawItemFrame_Common(plmp, k);
}

const uint8 *PlmInstr_DrawItemFrame1(const uint8 *plmp, uint16 k) {  // 0x84E067
  plm_instruction_draw_ptr[k >> 1] = off_84E077[plm_variables[k >> 1] >> 1];
  return PlmInstr_DrawItemFrame_Common(plmp, k);
}

const uint8 *PlmInstr_ClearChargeBeamCounter(const uint8 *plmp, uint16 k) {  // 0x84E29D
  flare_counter = 0;
  return plmp;
}















