// PLM set-piece room handlers (stations, escapes, Mother Brain, Draygon, etc.).
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"

#define off_84DB28 ((uint16*)RomFixedPtr(0x84db28))
#define off_84E05F ((uint16*)RomFixedPtr(0x84e05f))
#define off_84E077 ((uint16*)RomFixedPtr(0x84e077))

uint8 PlmSetup_CrumbleBotwoonWall(uint16 j) {  // 0x84AB28
  plm_instruction_timer[j >> 1] = 64;
  return 0;
}

uint8 PlmSetup_SetrupWreckedShipEntrance(uint16 j) {  // 0x84B04A
  int16 v2;

  uint16 v1 = plm_block_indices[j >> 1];
  v2 = 56;
  do {
    level_data[v1 >> 1] = 255;
    v1 += 2;
    --v2;
  } while (v2);
  return 0;
}

uint8 ActivateStationIfSamusCannonLinedUp(uint16 a, uint16 j) {  // 0x84B146
  uint16 v2 = 78;
  while (a != plm_block_indices[v2 >> 1]) {
    v2 -= 2;
    if ((v2 & 0x8000) != 0)
      goto LABEL_7;
  }
  CalculatePlmBlockCoords(plm_id);
  if (((uint16)(16 * plm_y_block) | 0xB) == samus_y_pos) {
    int v3 = v2 >> 1;
    plm_instr_list_ptrs[v3] = plm_instruction_list_link_reg[v3];
    plm_instruction_timer[v3] = 1;
    CallSomeSamusCode(6);
    return 1;
  }
LABEL_7:
  plm_header_ptr[j >> 1] = 0;
  return 1;
}

uint8 PlmSetup_B6D3_MapStation(uint16 j) {  // 0x84B18B
  int v1 = j >> 1;
  int v2 = plm_block_indices[v1] >> 1;
  level_data[v2] = level_data[v2] & 0xFFF | 0x8000;
  if (map_station_byte_array[area_index]) {
    plm_instr_list_ptrs[v1] = addr_word_84AD76;
  } else {
    WriteLevelDataBlockTypeAndBts(plm_block_indices[v1] + 2, 0xB047);
    WriteLevelDataBlockTypeAndBts(plm_block_indices[v1] - 4, 0xB048);
  }
  return 0;
}

uint8 PlmSetup_Bts47_MapStationRightAccess(uint16 j) {  // 0x84B1C8
  if ((samus_collision_direction & 0xF) == 0 && samus_pose == kPose_8A_FaceL_Ranintowall && (samus_pose_x_dir & 4) != 0)
    return ActivateStationIfSamusCannonLinedUp(plm_block_indices[j >> 1] - 2, j);
  plm_header_ptr[j >> 1] = 0;
  return 1;
}

uint8 PlmSetup_Bts4_MapStationLeftAccess(uint16 j) {  // 0x84B1F0
  if ((samus_collision_direction & 0xF) == 1 && samus_pose == kPose_89_FaceR_Ranintowall && (samus_pose_x_dir & 8) != 0)
    return ActivateStationIfSamusCannonLinedUp(plm_block_indices[j >> 1] + 4, j);
  plm_header_ptr[j >> 1] = 0;
  return 1;
}

uint8 PlmSetup_PlmB6DF_EnergyStation(uint16 j) {  // 0x84B21D
  int v1 = j >> 1;
  int v2 = plm_block_indices[v1] >> 1;
  level_data[v2] = level_data[v2] & 0xFFF | 0x8000;
  WriteLevelDataBlockTypeAndBts(plm_block_indices[v1] + 2, 0xB049);
  WriteLevelDataBlockTypeAndBts(plm_block_indices[v1] - 2, FUNC16(PlmSetup_SetrupWreckedShipEntrance));
  return 0;
}

uint8 PlmSetup_PlmB6EB_EnergyStation(uint16 j) {  // 0x84B245
  int v1 = j >> 1;
  int v2 = plm_block_indices[v1] >> 1;
  level_data[v2] = level_data[v2] & 0xFFF | 0x8000;
  WriteLevelDataBlockTypeAndBts(plm_block_indices[v1] + 2, 0xB04B);
  WriteLevelDataBlockTypeAndBts(plm_block_indices[v1] - 2, 0xB04C);
  return 0;
}

uint8 PlmSetup_B6E3_EnergyStationRightAccess(uint16 j) {  // 0x84B26D
  if ((samus_collision_direction & 0xF) != 0
      || samus_pose != kPose_8A_FaceL_Ranintowall
      || (samus_pose_x_dir & 4) == 0
      || samus_health == samus_max_health) {
    plm_header_ptr[j >> 1] = 0;
    return 1;
  } else {
    return ActivateStationIfSamusCannonLinedUp(plm_block_indices[j >> 1] - 2, j);
  }
}

uint8 PlmSetup_B6E7_EnergyStationLeftAccess(uint16 j) {  // 0x84B29D
  if ((samus_collision_direction & 0xF) == 1
      && samus_pose == kPose_89_FaceR_Ranintowall
      && (samus_pose_x_dir & 8) != 0
      && samus_health != samus_max_health) {
    return ActivateStationIfSamusCannonLinedUp(plm_block_indices[j >> 1] + 2, j);
  }
  plm_header_ptr[j >> 1] = 0;
  return 1;
}

uint8 PlmSetup_B6EF_MissileStationRightAccess(uint16 j) {  // 0x84B2D0
  if ((samus_collision_direction & 0xF) != 0
      || samus_pose != kPose_8A_FaceL_Ranintowall
      || (samus_pose_x_dir & 4) == 0
      || samus_missiles == samus_max_missiles) {
    plm_header_ptr[j >> 1] = 0;
    return 1;
  }
  return ActivateStationIfSamusCannonLinedUp(plm_block_indices[j >> 1] - 2, j);
}

uint8 PlmSetup_B6F3_MissileStationLeftAccess(uint16 j) {  // 0x84B300
  if ((samus_collision_direction & 0xF) == 1
      && samus_pose == kPose_89_FaceR_Ranintowall
      && (samus_pose_x_dir & 8) != 0
      && samus_missiles != samus_max_missiles) {
    return ActivateStationIfSamusCannonLinedUp(plm_block_indices[j >> 1] + 2, j);
  } else {
    plm_header_ptr[j >> 1] = 0;
    return 1;
  }
}

void DeletePlm(uint16 j) {  // 0x84B333
  plm_header_ptr[j >> 1] = 0;
}

uint8 PlmSetup_ClearShitroidInvisibleWall(uint16 j) {  // 0x84B551
  int16 v2;

  uint16 v1 = plm_block_indices[j >> 1];
  v2 = 10;
  do {
    level_data[v1 >> 1] &= 0xFFF;
    v1 += room_width_in_blocks * 2;
    --v2;
  } while (v2);
  return 0;
}

uint8 PlmSetup_B767_ClearShitroidInvisibleWall(uint16 j) {  // 0x84B56F
  int16 v2;

  uint16 v1 = plm_block_indices[j >> 1];
  v2 = 10;
  do {
    level_data[v1 >> 1] = level_data[v1 >> 1] & 0xFFF | 0x8000;
    v1 += room_width_in_blocks * 2;
    --v2;
  } while (v2);
  return 0;
}

uint8 PlmSetup_B76B_SaveStationTrigger(uint16 j) {  // 0x84B590
  if (!power_bomb_explosion_status
      && (samus_pose == kPose_01_FaceR_Normal || samus_pose == kPose_02_FaceL_Normal)
      && !save_station_lockout_flag
      && (samus_collision_direction & 0xF) == 3) {
    CalculatePlmBlockCoords(j);
    if ((uint16)(samus_x_pos - 8) >> 4 == plm_x_block) {
      int v1 = j >> 1;
      uint16 v2 = plm_block_indices[v1];
      plm_block_indices[v1] = 0;
      plm_header_ptr[v1] = 0;
      uint16 v3 = 78;
      while (v2 != plm_block_indices[v3 >> 1]) {
        v3 -= 2;
        if ((v3 & 0x8000) != 0)
          return 1;
      }
      int v4 = v3 >> 1;
      ++plm_instr_list_ptrs[v4];
      ++plm_instr_list_ptrs[v4];
      plm_instruction_timer[v4] = 1;
    }
  }
  return 1;
}

uint8 PlmSetup_B76F_SaveStation(uint16 j) {  // 0x84B5EE
  WriteLevelDataBlockTypeAndBts(plm_block_indices[j >> 1], 0xB04D);
  return 0;
}

uint8 PlmSetup_MotherBrainRoomEscapeDoor(uint16 j) {  // 0x84B5F8
  uint16 v1 = plm_block_indices[j >> 1];
  WriteLevelDataBlockTypeAndBts(v1, 0x9001);
  uint16 v2 = room_width_in_blocks * 2 + v1;
  WriteLevelDataBlockTypeAndBts(v2, 0xD0FF);
  uint16 v3 = room_width_in_blocks * 2 + v2;
  WriteLevelDataBlockTypeAndBts(v3, 0xD0FF);
  WriteLevelDataBlockTypeAndBts(
    room_width_in_blocks * 2 + v3,
    0xD0FF);
  return 0;
}

uint8 PlmSetup_B7EB_EnableSoundsIn32Frames(uint16 j) {  // 0x84B7C3
  uint16 v1;
  if (area_index == 6)
    v1 = 32;
  else
    v1 = 240;
  int v2 = j >> 1;
  plm_timers[v2] = v1;
  plm_pre_instrs[v2] = FUNC16(PlmPreInstr_B7EB_DecTimerEnableSoundsDeletePlm);
  return 0;
}

uint8 PlmSetup_SpeedBoosterEscape(uint16 j) {  // 0x84B89C
  if (CheckEventHappened(0x15))
    plm_header_ptr[j >> 1] = 0;
  return 0;
}

uint8 PlmSetup_ShaktoolsRoom(uint16 j) {  // 0x84B8DC
  *(uint16 *)scrolls = 1;
  *(uint16 *)&scrolls[2] = 0;
  return 0;
}

uint8 PlmSetup_B974(uint16 j) {  // 0x84B96C
  // Implemented in BlockReact_ShootableAir
  return 0;
}

uint8 PlmSetup_B9C1_CrittersEscapeBlock(uint16 j) {  // 0x84B978
  if (projectile_type[projectile_index >> 1]) {
    int v1 = j >> 1;
    int v2 = plm_block_indices[v1] >> 1;
    uint16 v3 = level_data[v2] & 0xF000 | 0x9F;
    plm_variable[v1] = v3;
    level_data[v2] = v3 & 0x8FFF;
  } else {
    plm_header_ptr[j >> 1] = 0;
  }
  return 0;
}

uint8 PlmSetup_B9ED_CrittersEscapeBlock(uint16 j) {  // 0x84B9C5
  uint16 v1 = plm_block_indices[j >> 1];
  WriteLevelDataBlockTypeAndBts(v1, 0xC04F);
  uint16 v2 = v1 + room_width_in_blocks * 2;
  WriteLevelDataBlockTypeAndBts(v2, 0xD0FF);
  WriteLevelDataBlockTypeAndBts(v2 + room_width_in_blocks * 2, 0xD0FF);
  return 0;
}

uint8 sub_84B9F1(uint16 j) {  // 0x84B9F1
  uint16 v1 = plm_block_indices[j >> 1];
  level_data[v1 >> 1] = level_data[v1 >> 1] & 0xFFF | 0x8000;
  uint16 v2 = room_width_in_blocks * 2 + v1;
  level_data[v2 >> 1] = level_data[v2 >> 1] & 0xFFF | 0x8000;
  uint16 v3 = room_width_in_blocks + room_width_in_blocks + v2;
  level_data[v3 >> 1] = level_data[v3 >> 1] & 0xFFF | 0x8000;
  int v4 = (uint16)(room_width_in_blocks * 2 + v3) >> 1;
  level_data[v4] = level_data[v4] & 0xFFF | 0x8000;
  return 0;
}

uint8 PlmSetup_BB30_CrateriaMainstreetEscape(uint16 j) {  // 0x84BB09
  if (!CheckEventHappened(0xF))
    plm_header_ptr[j >> 1] = 0;
  return 0;
}

uint8 PlmSetup_D6DA_LowerNorfairChozoHandTrigger(uint16 j) {  // 0x84D18F
  if ((collected_items & 0x200) != 0
      && (samus_collision_direction & 0xF) == 3
      && (samus_pose == kPose_1D_FaceR_Morphball_Ground
          || samus_pose == kPose_79_FaceR_Springball_Ground
          || samus_pose == kPose_7A_FaceL_Springball_Ground)) {
    SetEventHappened(0xC);
    enemy_data[0].parameter_1 = 1;
    int v2 = plm_block_indices[j >> 1] >> 1;
    level_data[v2] &= 0xFFF;
    CallSomeSamusCode(0);
    SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x0c, 0x1d, 0xd113 });
  }
  plm_header_ptr[j >> 1] = 0;
  return 1;
}

void SpawnMotherBrainGlassShatteringShard(uint16 a) {  // 0x84D331
  SpawnEprojWithRoomGfx(0xCEFC, a);
}

uint8 PlmSetup_MotherBrainGlass(uint16 j) {  // 0x84D5F6
  int v1 = j >> 1;
  plm_room_arguments[v1] = 0;
  WriteLevelDataBlockTypeAndBts(plm_block_indices[v1], 0x8044);
  return 0;
}

uint8 PlmSetup_DeletePlmIfAreaTorizoDead(uint16 j) {  // 0x84D606
  if (CheckBossBitForCurArea(4) & 1)
    plm_header_ptr[j >> 1] = 0;
  return 0;
}

uint8 PlmSetup_MakeBllockChozoHandTrigger(uint16 j) {  // 0x84D616
  WriteLevelDataBlockTypeAndBts(plm_block_indices[j >> 1], 0xB080);
  return 0;
}

uint8 PlmSetup_D6F2_WreckedShipChozoHandTrigger(uint16 j) {  // 0x84D620
  if (CheckBossBitForCurArea(1) & 1
      && (samus_collision_direction & 0xF) == 3
      && (samus_pose == kPose_1D_FaceR_Morphball_Ground
          || samus_pose == kPose_79_FaceR_Springball_Ground
          || samus_pose == kPose_7A_FaceL_Springball_Ground)) {
    enemy_data[0].parameter_1 = 1;
    *(uint16 *)&scrolls[7] = 514;
    *(uint16 *)&scrolls[13] = 257;
    int v1 = plm_block_indices[j >> 1] >> 1;
    level_data[v1] &= 0xFFF;
    CallSomeSamusCode(0);
    SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x17, 0x1d, 0xd6f8 });
  }
  plm_header_ptr[j >> 1] = 0;
  return 1;
}

uint8 PlmSetup_D700_MakePlmAirBlock_Unused(uint16 j) {  // 0x84D67F
  WriteLevelDataBlockTypeAndBts(plm_block_indices[j >> 1], 0x44);
  return 0;
}

uint8 PlmSetup_D704_AlteranateLowerNorfairChozoHand_Unused(uint16 j) {  // 0x84D689
  WriteLevelDataBlockTypeAndBts(plm_block_indices[j >> 1], 0x8044);
  return 0;
}

uint8 PlmSetup_D708_LowerNorfairChozoBlockUnused(uint16 j) {  // 0x84D693
  int v1 = j >> 1;
  WriteLevelDataBlockTypeAndBts(plm_block_indices[v1], 0x8044);
  WriteLevelDataBlockTypeAndBts(plm_block_indices[v1] + 2, 0x50FF);
  WriteLevelDataBlockTypeAndBts(
    room_width_in_blocks
    + room_width_in_blocks
    + plm_block_indices[v1],
    0xD0FF);
  WriteLevelDataBlockTypeAndBts(
    room_width_in_blocks
    + room_width_in_blocks
    + plm_block_indices[v1]
    + 2,
    0xD0FF);
  return 0;
}

uint8 PlmSetup_D70C_NoobTube(uint16 j) {  // 0x84D6CC
  WriteLevelDataBlockTypeAndBts(plm_block_indices[j >> 1], 0x8044);
  return 0;
}

uint8 PlmSetup_EyeDoorEye(uint16 j) {  // 0x84DA8C
  int idx = PrepareBitAccess(plm_room_arguments[j >> 1]);
  if ((bitmask & opened_door_bit_array[idx]) == 0) {
    int v1 = j >> 1;
    WriteLevelDataBlockTypeAndBts(plm_block_indices[v1], 0xC044);
    WriteLevelDataBlockTypeAndBts(2 * room_width_in_blocks + plm_block_indices[v1], 0xD0FF);
  }
  return 0;
}

uint8 PlmSetup_EyeDoor(uint16 j) {  // 0x84DAB9
  int idx = PrepareBitAccess(plm_room_arguments[j >> 1]);
  if ((bitmask & opened_door_bit_array[idx]) == 0)
    WriteLevelDataBlockTypeAndBts(plm_block_indices[j >> 1], 0xA000);
  return 0;
}

uint8 PlmSetup_SetMetroidRequiredClearState(uint16 j) {  // 0x84DB1E
  plm_pre_instrs[j >> 1] = off_84DB28[plm_room_arguments[j >> 1] >> 1];
  return 0;
}

void SetPlmVarPtr(uint16 k, uint16 a) {
  *(uint16 *)&g_ram[plm_variable[k >> 1]] = a;
}

uint8 PlmSetup_DraygonCannonFacingRight(uint16 j) {  // 0x84DE94
  int v1 = j >> 1;
  plm_variable[v1] = plm_room_arguments[v1];
  plm_room_arguments[v1] = 0;
  uint16 v2 = plm_block_indices[v1];
  WriteLevelDataBlockTypeAndBts(v2, 0xC044);
  WriteLevelDataBlockTypeAndBts(room_width_in_blocks * 2 + v2, 0xD0FF);
  return 0;
}

uint8 PlmSetup_DraygonCannonFacingDiagonalRight(uint16 j) {  // 0x84DEB9
  int v1 = j >> 1;
  plm_variable[v1] = plm_room_arguments[v1];
  plm_room_arguments[v1] = 0;
  uint16 v2 = plm_block_indices[v1];
  WriteLevelDataBlockTypeAndBts(v2, 0xC044);
  WriteLevelDataBlockTypeAndBts(v2 + 2, 0x50FF);
  uint16 v3 = room_width_in_blocks * 2 + v2;
  WriteLevelDataBlockTypeAndBts(v3, 0xD0FF);
  WriteLevelDataBlockTypeAndBts(v3 + 2, 0xD0FF);
  return 0;
}

uint8 PlmSetup_DraygonCannonFacingLeft(uint16 j) {  // 0x84DEF0
  int v1 = j >> 1;
  plm_variable[v1] = plm_room_arguments[v1];
  plm_room_arguments[v1] = 0;
  uint16 v2 = plm_block_indices[v1];
  WriteLevelDataBlockTypeAndBts(v2, 0xC044);
  WriteLevelDataBlockTypeAndBts(room_width_in_blocks * 2 + v2, 0xD0FF);
  return 0;
}

uint8 PlmSetup_DraygonCannonFacingDiagonalLeft(uint16 j) {  // 0x84DF15
  int v1 = j >> 1;
  plm_variable[v1] = plm_room_arguments[v1];
  plm_room_arguments[v1] = 0;
  uint16 v2 = plm_block_indices[v1];
  WriteLevelDataBlockTypeAndBts(v2, 0xC044);
  WriteLevelDataBlockTypeAndBts(v2 - 2, 0x5001);
  uint16 v3 = room_width_in_blocks * 2 + v2;
  WriteLevelDataBlockTypeAndBts(v3, 0xD0FF);
  WriteLevelDataBlockTypeAndBts(v3 - 2, 0xD0FF);
  return 0;
}

uint8 PlmSetup_DraygonCannon(uint16 j) {  // 0x84DF4C
  int v1 = j >> 1;
  plm_variable[v1] = plm_room_arguments[v1];
  plm_room_arguments[v1] = 3;
  return 0;
}

uint8 sub_84EE4D(uint16 j) {  // 0x84EE4D
  return sub_84EE5F(j, 8);
}

uint8 sub_84EE52(uint16 j) {  // 0x84EE52
  return sub_84EE5F(j, 0xA);
}

uint8 sub_84EE57(uint16 j) {  // 0x84EE57
  return sub_84EE5F(j, 0xC);
}

uint8 sub_84EE5C(uint16 j) {  // 0x84EE5C
  return sub_84EE5F(j, 0xE);
}

uint8 sub_84EE5F(uint16 j, uint16 a) {  // 0x84EE5F
  plm_variables[j >> 1] = a;
  return sub_84EE64(j);
}

uint8 sub_84EE64(uint16 j) {  // 0x84EE64
  WriteLevelDataBlockTypeAndBts(plm_block_indices[j >> 1], 0x45);
  ++global_number_of_items_loaded_ctr;
  return 0;
}

uint8 sub_84EE77(uint16 j) {  // 0x84EE77
  return sub_84EE89(j, 8);
}

uint8 sub_84EE7C(uint16 j) {  // 0x84EE7C
  return sub_84EE89(j, 0xA);
}

uint8 sub_84EE81(uint16 j) {  // 0x84EE81
  return sub_84EE89(j, 0xC);
}

uint8 sub_84EE86(uint16 j) {  // 0x84EE86
  return sub_84EE89(j, 0xE);
}

uint8 sub_84EE89(uint16 j, uint16 a) {  // 0x84EE89
  plm_variables[j >> 1] = a;
  return sub_84EE8E(j);
}

uint8 sub_84EE8E(uint16 j) {  // 0x84EE8E
  int v1 = j >> 1;
  WriteLevelDataBlockTypeAndBts(plm_block_indices[v1], 0xC045);
  plm_variable[v1] = level_data[plm_block_indices[v1] >> 1];
  ++global_number_of_items_loaded_ctr;
  return 0;
}

uint8 sub_84EEAB(uint16 v0) {  // 0x84EEAB
  int i;

  if (time_is_frozen_flag) {
    plm_header_ptr[v0 >> 1] = 0;
  } else {
    int v1 = v0 >> 1;
    uint16 v2 = plm_block_indices[v1];
    plm_block_indices[v1] = 0;
    for (i = 78; i >= 0; i -= 2) {
      if (v2 == plm_block_indices[i >> 1])
        break;
    }
    plm_timers[i >> 1] = 255;
  }
  return 0;
}

