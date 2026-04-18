// PLM block/door/gate setup handlers.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"

#define kDowardGatePlmListPtrs ((uint16*)RomFixedPtr(0x84c70a))
#define kDowardGateLeftBlockBts ((uint16*)RomFixedPtr(0x84c71a))
#define kDowardGateRightBlockBts ((uint16*)RomFixedPtr(0x84c72a))
#define kUpwardGatePlmListPtrs ((uint16*)RomFixedPtr(0x84c764))
#define kUpwardGateLeftBlockBts ((uint16*)RomFixedPtr(0x84c774))
#define kUpwardGateRightBlockBts ((uint16*)RomFixedPtr(0x84c784))
#define kGrayDoorPreInstrs ((uint16*)RomFixedPtr(0x84be4b))

static Func_Y_V *const kPlmSetup_QuicksandSurface[4] = {  // 0x84B408
  PlmSetup_QuicksandSurface_0,
  PlmSetup_QuicksandSurface_1,
  PlmSetup_QuicksandSurface_2,
  PlmSetup_QuicksandSurface_0,
};

static const uint16 g_word_84B48B[2] = { 0x200, 0x200 };
static const uint16 g_word_84B48F[2] = { 0x120, 0x100 };
static const uint16 g_word_84B493[2] = { 0x280, 0x380 };


uint8 PlmSetup_BTS_Brinstar_0x80_Floorplant(uint16 j) {  // 0x84B0DC
  if (((samus_y_radius + samus_y_pos - 1) & 0xF) == 15) {
    int v1 = plm_block_indices[j >> 1] >> 1;
    level_data[v1] &= 0x8FFF;
    int v2 = j >> 1;
    plm_variable[v2] = samus_x_pos;
    plm_variables[v2] = samus_y_pos - 1;
  } else {
    plm_header_ptr[j >> 1] = 0;
  }
  return 0;
}

uint8 PlmSetup_BTS_Brinstar_0x81_Ceilingplant(uint16 j) {  // 0x84B113
  if (((samus_y_pos - samus_y_radius) & 0xF) != 0) {
    plm_header_ptr[j >> 1] = 0;
  } else {
    int v1 = plm_block_indices[j >> 1] >> 1;
    level_data[v1] &= 0x8FFF;
    int v2 = j >> 1;
    plm_variable[v2] = samus_x_pos;
    plm_variables[v2] = samus_y_pos + 1;
  }
  return 0;
}

uint8 PlmSetup_B638_Rightwards_Extension(uint16 j) {  // 0x84B33A
  WriteLevelDataBlockTypeAndBts(plm_block_indices[j >> 1], 0x50FF);
  DeletePlm(j);
  return 0;
}

uint8 PlmSetup_B63F_Leftwards_Extension(uint16 j) {  // 0x84B345
  WriteLevelDataBlockTypeAndBts(plm_block_indices[j >> 1], 0x5001);
  DeletePlm(j);
  return 0;
}

uint8 PlmSetup_B643_Downwards_Extension(uint16 j) {  // 0x84B350
  WriteLevelDataBlockTypeAndBts(plm_block_indices[j >> 1], 0xD0FF);
  DeletePlm(j);
  return 0;
}

uint8 PlmSetup_B647_Upwards_Extension(uint16 j) {  // 0x84B35B
  WriteLevelDataBlockTypeAndBts(plm_block_indices[j >> 1], 0xD001);
  DeletePlm(j);
  return 0;
}

void SkipDebugDrawInstructionsForScrollPlms(uint16 j) {  // 0x84B366
  plm_instr_list_ptrs[j >> 1] += 4;
}

uint8 PlmSetup_B703_ScrollPLM(uint16 j) {  // 0x84B371
  int v1 = j >> 1;
  WriteLevelDataBlockTypeAndBts(plm_block_indices[v1], 0x3046);
  plm_variable[v1] = 0;
  SkipDebugDrawInstructionsForScrollPlms(j);
  return 0;
}

uint8 PlmSetup_B707_SolidScrollPLM(uint16 j) {  // 0x84B382
  int v1 = j >> 1;
  WriteLevelDataBlockTypeAndBts(plm_block_indices[v1], 0xB046);
  plm_variable[v1] = 0;
  SkipDebugDrawInstructionsForScrollPlms(j);
  return 0;
}

uint8 PlmSetup_B6FF_ScrollBlockTouch(uint16 j) {  // 0x84B393
  int v1 = j >> 1;
  uint16 v2 = plm_block_indices[v1];
  plm_block_indices[v1] = 0;
  uint16 v3 = 78;
  while (v2 != plm_block_indices[v3 >> 1]) {
    v3 -= 2;
    if ((v3 & 0x8000) != 0) {
      while (1)
        ;
    }
  }
  int v4 = v3 >> 1;
  if ((plm_variable[v4] & 0x8000) == 0) {
    plm_variable[v4] = 0x8000;
    ++plm_instr_list_ptrs[v4];
    ++plm_instr_list_ptrs[v4];
    plm_instruction_timer[v4] = 1;
  }
  return 0;
}

uint8 PlmSetup_DeactivatePlm(uint16 j) {  // 0x84B3C1
  int v1 = plm_block_indices[j >> 1] >> 1;
  level_data[v1] &= 0x8FFF;
  return 0;
}

uint8 PlmSetup_ReturnCarryClear(uint16 j) {  // 0x84B3D0
  return 0;
}

uint8 PlmSetup_ReturnCarrySet(uint16 j) {  // 0x84B3D2
  return 1;
}

uint8 PlmSetup_D094_EnemyBreakableBlock(uint16 j) {  // 0x84B3D4
  int v1 = plm_block_indices[j >> 1] >> 1;
  level_data[v1] &= 0xFFF;
  return 0;
}

uint8 UNUSED_sub_84B3E3(uint16 j) {  // 0x84B3E3
  assert(0);
  return 0;
}

uint8 PlmSetup_B70F_IcePhysics(uint16 j) {  // 0x84B3EB
  if (((samus_y_radius + samus_y_pos - 1) & 0xF) == 7 || ((samus_y_radius + samus_y_pos - 1) & 0xF) == 15) {
    *(uint16 *)&samus_x_decel_mult = 16;
  }
  return 0;
}

uint8 PlmSetup_QuicksandSurface(uint16 j) {

  samus_has_momentum_flag = 0;
  speed_boost_counter = 0;
  samus_echoes_sound_flag = 0;
  samus_x_extra_run_subspeed = 0;
  samus_x_extra_run_speed = 0;
  samus_x_base_subspeed &= ~0x8000;
  samus_x_base_speed = 0;
  uint16 v1 = 0;
  if ((equipped_items & 0x20) != 0)
    v1 = 2;
  if (!inside_block_reaction_samus_point)
    kPlmSetup_QuicksandSurface[samus_y_dir & 3](v1);
  return 0;
}

void PlmSetup_QuicksandSurface_0(uint16 j) {  // 0x84B447
  samus_y_subspeed = 0;
  samus_y_speed = 0;
  extra_samus_y_subdisplacement = 0;
  extra_samus_y_displacement = 0;
  *(uint16 *)((uint8 *)&extra_samus_y_subdisplacement + 1) = g_word_84B48F[j >> 1];
}

void PlmSetup_QuicksandSurface_1(uint16 j) {  // 0x84B45A
  int v1 = j >> 1;
  if (g_word_84B493[v1] < *(uint16 *)((uint8 *)&samus_y_subspeed + 1)) {
    samus_y_subspeed = 0;
    samus_y_speed = 0;
    *(uint16 *)((uint8 *)&samus_y_subspeed + 1) = g_word_84B493[v1];
  }
  extra_samus_y_subdisplacement = 0;
  extra_samus_y_displacement = 0;
  *(uint16 *)((uint8 *)&extra_samus_y_subdisplacement + 1) = g_word_84B48B[v1];
}

void PlmSetup_QuicksandSurface_2(uint16 j) {  // 0x84B47B
  extra_samus_y_subdisplacement = 0;
  extra_samus_y_displacement = 0;
  *(uint16 *)((uint8 *)&extra_samus_y_subdisplacement + 1) = g_word_84B48B[j >> 1];
  autojump_timer = 0;
}

uint8 PlmSetup_B71F_SubmergingQuicksand(uint16 j) {  // 0x84B497
  autojump_timer = 0;
  extra_samus_y_subdisplacement = 0x2000;
  extra_samus_y_displacement = 1;
  return 0;
}

uint8 PlmSetup_B723_SandfallsSlow(uint16 j) {  // 0x84B4A8
  extra_samus_y_subdisplacement = 0x4000;
  extra_samus_y_displacement = 1;
  return 0;
}

uint8 PlmSetup_B727_SandFallsFast(uint16 j) {  // 0x84B4B6
  extra_samus_y_subdisplacement = -16384;
  extra_samus_y_displacement = 1;
  return 0;
}

uint8 PlmSetup_QuicksandSurfaceB(uint16 j) {
  if ((samus_collision_direction & 2) == 0)
    return 0;

  if (cur_coll_amt32 == NULL) {
    printf("ERROR: PlmSetup_QuicksandSurfaceB is broken!!\n");
    Unreachable();
    return 0;
  }

  switch (samus_y_dir & 3) {
  case 0:
  case 3:
    if ((samus_collision_direction & 0xF) == 3) {
      if (samus_contact_damage_index == 1) {
        *cur_coll_amt32 = 0;
        return 1;
      } else {
        if ((uint16)(*cur_coll_amt32 >> 8) > 0x30)
          *cur_coll_amt32 = (*cur_coll_amt32 & 0xff0000ff) | 0x3000;// R19_ = 0x30;
        flag_samus_in_quicksand++;
      }
    }
    break;
  case 2:
    if (samus_contact_damage_index == 1) {
      *cur_coll_amt32 = 0;
      return 1;
    } else {
      flag_samus_in_quicksand++;
      return 0;
    }
  }
  return 0;
}

uint8 PlmSetup_B737_SubmergingQuicksand(uint16 j) {  // 0x84B541
  samus_y_subspeed = 0;
  samus_y_speed = 0;
  samus_y_subaccel = 0;
  samus_y_accel = 0;
  return 0;
}

uint8 PlmSetup_B73B_B73F_SandFalls(uint16 j) {  // 0x84B54F
  return 0;
}

uint8 PlmSetup_C806_LeftGreenGateTrigger(uint16 j) {  // 0x84C54D
  if ((projectile_type[projectile_index >> 1] & 0xFFF) == 512)
    return sub_84C63F(j);
  QueueSfx2_Max6(0x57);
  plm_header_ptr[j >> 1] = 0;
  return 0;
}

uint8 PlmSetup_C80A_RightGreenGateTrigger(uint16 j) {  // 0x84C56C
  if ((projectile_type[projectile_index >> 1] & 0xFFF) == 512)
    return sub_84C647(j);
  QueueSfx2_Max6(0x57);
  plm_header_ptr[j >> 1] = 0;
  return 0;
}

uint8 PlmSetup_C80E_LeftRedGateTrigger(uint16 j) {  // 0x84C58B
  int16 v1;

  v1 = projectile_type[projectile_index >> 1] & 0xFFF;
  if (v1 == 256 || v1 == 512)
    return sub_84C63F(j);
  QueueSfx2_Max6(0x57);
  plm_header_ptr[j >> 1] = 0;
  return 0;
}

uint8 PlmSetup_C812_RightRedGateTrigger(uint16 j) {  // 0x84C5AF
  int16 v1;

  v1 = projectile_type[projectile_index >> 1] & 0xFFF;
  if (v1 == 256 || v1 == 512)
    return sub_84C647(j);
  QueueSfx2_Max6(0x57);
  plm_header_ptr[j >> 1] = 0;
  return 0;
}

uint8 PlmSetup_C822_RightYellowGateTrigger(uint16 j) {  // 0x84C5F1
  if ((projectile_type[projectile_index >> 1] & 0xFFF) != 768)
    return sub_84C647(j);
  QueueSfx2_Max6(0x57);
  plm_header_ptr[j >> 1] = 0;
  return 0;
}

uint8 PlmSetup_C81E_LeftYellowGateTrigger(uint16 j) {  // 0x84C5D3
  if ((projectile_type[projectile_index >> 1] & 0xFFF) == 768)
    return sub_84C63F(j);
  QueueSfx2_Max6(0x57);
  int v1 = j >> 1;
  plm_header_ptr[v1] = 0;
  return PlmSetup_C822_RightYellowGateTrigger(j);
}

uint8 PlmSetup_C81A_RightBlueGateTrigger(uint16 j) {  // 0x84C627
  if ((projectile_type[projectile_index >> 1] & 0xFFF) != 768)
    return sub_84C647(j);
  plm_header_ptr[j >> 1] = 0;
  return 0;
}

uint8 PlmSetup_C816_LeftBlueGateTrigger(uint16 j) {  // 0x84C610
  if ((projectile_type[projectile_index >> 1] & 0xFFF) != 768)
    return sub_84C63F(j);
  plm_header_ptr[j >> 1] = 0;
  return PlmSetup_C81A_RightBlueGateTrigger(j);
}

uint8 sub_84C63F(uint16 j) {  // 0x84C63F
  return sub_84C64C(j, plm_block_indices[j >> 1] + 2);
}

uint8 sub_84C647(uint16 j) {  // 0x84C647
  return sub_84C64C(j, plm_block_indices[j >> 1] - 2);
}

uint8 sub_84C64C(uint16 j, uint16 a) {  // 0x84C64C
  uint16 v2 = 78;
  while (a != plm_block_indices[v2 >> 1]) {
    v2 -= 2;
    if ((v2 & 0x8000) != 0)
      goto LABEL_7;
  }
  int v3;
  v3 = v2 >> 1;
  if (!plm_timers[v3])
    ++plm_timers[v3];
LABEL_7:
  plm_header_ptr[j >> 1] = 0;
  return 1;
}

void SetBts0x10FiveStepsDown(uint16 j) {  // 0x84C66A
  uint16 v1 = SetBtsTo0x10AdvanceRow(plm_block_indices[j >> 1] >> 1);
  uint16 v2 = SetBtsTo0x10AdvanceRow(v1);
  uint16 v3 = SetBtsTo0x10AdvanceRow(v2);
  uint16 v4 = SetBtsTo0x10AdvanceRow(v3);
  SetBtsTo0x10AdvanceRow(v4);
}

uint16 SetBtsTo0x10AdvanceRow(uint16 k) {  // 0x84C67F
  *(uint16 *)&BTS[k] = (BTS[k + 1] << 8) | 0x10;
  return room_width_in_blocks + k;
}

void SetBts0x10FiveStepsUp(uint16 j) {  // 0x84C694
  uint16 v1 = SetBtsTo0x10AdvanceRowUp(plm_block_indices[j >> 1] >> 1);
  uint16 v2 = SetBtsTo0x10AdvanceRowUp(v1);
  uint16 v3 = SetBtsTo0x10AdvanceRowUp(v2);
  uint16 v4 = SetBtsTo0x10AdvanceRowUp(v3);
  SetBtsTo0x10AdvanceRowUp(v4);
}

uint16 SetBtsTo0x10AdvanceRowUp(uint16 k) {  // 0x84C6A9
  *(uint16 *)&BTS[k] = (BTS[k + 1] << 8) | 0x10;
  return k - room_width_in_blocks;
}

uint8 PlmSetup_C82A_DownwardsClosedGate(uint16 j) {  // 0x84C6BE
  SpawnEprojWithRoomGfx(0xE659, 0);
  SetBts0x10FiveStepsDown(j);
  return 0;
}

uint8 PlmSetup_C832_UpwardsClosedGate(uint16 j) {  // 0x84C6CB
  SpawnEprojWithRoomGfx(0xE675, 0);
  SetBts0x10FiveStepsUp(j);
  return 0;
}

uint8 PlmSetup_C826_DownwardsOpenGate(uint16 j) {  // 0x84C6D8
  SetBts0x10FiveStepsDown(j);
  return 0;
}

uint8 PlmSetup_C82E_UpwardsOpenGate(uint16 j) {  // 0x84C6DC
  SetBts0x10FiveStepsUp(j);
  return 0;
}

uint8 PlmSetup_C836_DownwardsGateShootblock(uint16 j) {  // 0x84C6E0
  int v1 = j >> 1;
  plm_instr_list_ptrs[v1] = kDowardGatePlmListPtrs[plm_room_arguments[v1] >> 1];
  uint16 v2 = kDowardGateLeftBlockBts[plm_room_arguments[v1] >> 1];
  if (v2)
    WriteLevelDataBlockTypeAndBts(plm_block_indices[v1] - 2, v2);
  uint16 v3 = kDowardGateRightBlockBts[plm_room_arguments[v1] >> 1];
  if (v3)
    WriteLevelDataBlockTypeAndBts(plm_block_indices[v1] + 2, v3);
  return 0;
}

uint8 PlmSetup_C73A_UpwardsGateShootblock(uint16 j) {  // 0x84C73A
  int v1 = j >> 1;
  plm_instr_list_ptrs[v1] = kUpwardGatePlmListPtrs[plm_room_arguments[v1] >> 1];
  uint16 v2 = kUpwardGateLeftBlockBts[plm_room_arguments[v1] >> 1];
  if (v2)
    WriteLevelDataBlockTypeAndBts(plm_block_indices[v1] - 2, v2);
  uint16 v3 = kUpwardGateRightBlockBts[plm_room_arguments[v1] >> 1];
  if (v3)
    WriteLevelDataBlockTypeAndBts(plm_block_indices[v1] + 2, v3);
  return 0;
}

uint8 PlmSetup_C794_GreyDoor(uint16 j) {  // 0x84C794
  int v1 = j >> 1;
  plm_variable[v1] = (uint16)(HIBYTE(plm_room_arguments[v1]) & 0x7C) >> 1;
  plm_room_arguments[v1] &= ~0x7C00;
  WriteLevelDataBlockTypeAndBts(plm_block_indices[v1], 0xC044);
  return 0;
}

uint8 PlmSetup_Door_Colored(uint16 j) {  // 0x84C7B1
  WriteLevelDataBlockTypeAndBts(plm_block_indices[j >> 1], 0xC044);
  return 0;
}

uint8 PlmSetup_Door_Blue(uint16 j) {  // 0x84C7BB
//  if (sign16(projectile_index))
//    printf("BUG: projectile_index invalid\n");
  if (!sign16(projectile_index) && (projectile_type[projectile_index >> 1] & 0xF00) == 768) {
    plm_header_ptr[j >> 1] = 0;
  } else {
    uint16 v1 = plm_block_indices[j >> 1];
    level_data[v1 >> 1] = level_data[v1 >> 1] & 0xFFF | 0x8000;
  }
  return 0;
}

uint8 PlmSetup_Door_Strange(uint16 j) {  // 0x84C7E2
  int v1 = j >> 1;
  uint16 v2 = plm_block_indices[v1];
  plm_block_indices[v1] = 0;
  uint16 v3 = 78;
  while (v2 != plm_block_indices[v3 >> 1]) {
    v3 -= 2;
    if ((v3 & 0x8000) != 0)
      return 0;
  }
  plm_timers[v3 >> 1] = projectile_type[(int16)projectile_index >> 1] & 0x1FFF | 0x8000;
  return 0;
}

uint8 PlmSetup_D028_D02C_Unused(uint16 j) {  // 0x84CDC2
  if (samus_pose == kPose_81_FaceR_Screwattack || samus_pose == kPose_82_FaceL_Screwattack) {
    int v1 = j >> 1;
    int v2 = plm_block_indices[v1] >> 1;
    uint16 v3 = level_data[v2];
    plm_variable[v1] = v3;
    level_data[v2] = v3 & 0xFFF;
    return 0;
  } else {
    plm_header_ptr[j >> 1] = 0;
    return 1;
  }
}

uint8 PlmSetup_RespawningSpeedBoostBlock(uint16 j) {  // 0x84CDEA
  if ((speed_boost_counter & 0xF00) == 1024
      || samus_pose == kPose_C9_FaceR_Shinespark_Horiz
      || samus_pose == kPose_CA_FaceL_Shinespark_Horiz
      || samus_pose == kPose_CB_FaceR_Shinespark_Vert
      || samus_pose == kPose_CC_FaceL_Shinespark_Vert
      || samus_pose == kPose_CD_FaceR_Shinespark_Diag
      || samus_pose == kPose_CE_FaceL_Shinespark_Diag) {
    int v1 = j >> 1;
    int v2 = plm_block_indices[v1] >> 1;
    uint16 v3 = level_data[v2] & 0xF000 | 0xB6;
    plm_variable[v1] = v3;
    level_data[v2] = v3 & 0xFFF;
    return 0;
  } else {
    plm_header_ptr[j >> 1] = 0;
    return 1;
  }
}

uint8 PlmSetup_RespawningCrumbleBlock(uint16 j) {  // 0x84CE37
  if ((samus_collision_direction & 0xF) == 3) {
    int v1 = j >> 1;
    int v2 = plm_block_indices[v1] >> 1;
    uint16 v3 = level_data[v2] & 0xF000 | 0xBC;
    plm_variable[v1] = v3;
    level_data[v2] = v3 & 0x8FFF;
    plm_instruction_timer[v1] = 4;
  } else {
    plm_header_ptr[j >> 1] = 0;
  }
  return 1;
}

uint8 PlmSetup_RespawningShotBlock(uint16 j) {  // 0x84CE6B
  int v1 = j >> 1;
  int v2 = plm_block_indices[v1] >> 1;
  uint16 v3 = level_data[v2] & 0xF000 | 0x52;
  plm_variable[v1] = v3;
  level_data[v2] = v3 & 0x8FFF;
  return 0;
}

uint8 PlmSetup_RespawningBombBlock(uint16 j) {  // 0x84CE83
  if ((speed_boost_counter & 0xF00) == 1024
      || samus_pose == kPose_81_FaceR_Screwattack
      || samus_pose == kPose_82_FaceL_Screwattack
      || samus_pose == kPose_C9_FaceR_Shinespark_Horiz
      || samus_pose == kPose_CA_FaceL_Shinespark_Horiz
      || samus_pose == kPose_CB_FaceR_Shinespark_Vert
      || samus_pose == kPose_CC_FaceL_Shinespark_Vert
      || samus_pose == kPose_CD_FaceR_Shinespark_Diag
      || samus_pose == kPose_CE_FaceL_Shinespark_Diag) {
    int v2 = j >> 1;
    int v3 = plm_block_indices[v2] >> 1;
    uint16 v4 = level_data[v3] & 0xF000 | 0x58;
    plm_variable[v2] = v4;
    level_data[v3] = v4 & 0xFFF;
    return 0;
  } else {
    plm_header_ptr[j >> 1] = 0;
    return 1;
  }
}

uint8 PlmSetup_RespawningBombBlock2(uint16 j) {  // 0x84CEDA
  int16 v1;

  v1 = projectile_type[projectile_index >> 1] & 0xF00;
  if (v1 == 1280) {
    int v5 = j >> 1;
    plm_instr_list_ptrs[v5] += 3;
    int v6 = plm_block_indices[v5] >> 1;
    uint16 v7 = level_data[v6] & 0xF000 | 0x58;
    plm_variable[v5] = v7;
    level_data[v6] = v7 & 0x8FFF;
  } else if (v1 == 768) {
    int v2 = j >> 1;
    int v3 = plm_block_indices[v2] >> 1;
    uint16 v4 = level_data[v3] & 0xF000 | 0x58;
    plm_variable[v2] = v4;
    level_data[v3] = v4 & 0x8FFF;
  } else {
    plm_header_ptr[j >> 1] = 0;
  }
  return 0;
}

uint8 PlmSetup_RespawningPowerBombBlock(uint16 j) {  // 0x84CF2E
  int16 v1;

  v1 = projectile_type[projectile_index >> 1] & 0xF00;
  if (v1 == 1280) {
    plm_instr_list_ptrs[j >> 1] = addr_kPlmInstrList_C91C;
  } else if (v1 == 768) {
    int v2 = j >> 1;
    int v3 = plm_block_indices[v2] >> 1;
    uint16 v4 = level_data[v3] & 0xF000 | 0x57;
    plm_variable[v2] = v4;
    level_data[v3] = v4 & 0x8FFF;
  } else {
    plm_header_ptr[j >> 1] = 0;
  }
  return 0;
}

uint8 PlmSetup_D08C_SuperMissileBlockRespawning(uint16 j) {  // 0x84CF67
  int16 v1;

  v1 = projectile_type[projectile_index >> 1] & 0xF00;
  if (v1 == 1280) {
    plm_instr_list_ptrs[j >> 1] = addr_kPlmInstrList_C922;
  } else if (v1 == 512) {
    int v2 = j >> 1;
    int v3 = plm_block_indices[v2] >> 1;
    uint16 v4 = level_data[v3] & 0xF000 | 0x9F;
    plm_variable[v2] = v4;
    level_data[v3] = v4 & 0x8FFF;
  } else {
    plm_header_ptr[j >> 1] = 0;
  }
  return 0;
}

uint8 PlmSetup_D08C_CrumbleBlock(uint16 j) {  // 0x84CFA0
  if ((projectile_type[projectile_index >> 1] & 0xF00) != 1280)
    plm_header_ptr[j >> 1] = 0;
  return 0;
}

uint8 PlmSetup_D0DC_BreakableGrappleBlock(uint16 j) {  // 0x84CFB5
  int v1 = j >> 1;
  uint16 v2 = plm_block_indices[v1];
  plm_variable[v1] = level_data[v2 >> 1];
  *(uint16 *)&BTS[v2 >> 1] = BTS[(v2 >> 1) + 1] << 8;
  return 0x41;
}

uint8 PlmSetup_D0D8_SetVFlag(uint16 j) {  // 0x84CFCD
  return 0x41;
}

uint8 PlmSetup_D0D8_ClearVflag(uint16 j) {  // 0x84CFD1
  return 1;
}

uint8 PlmSetup_D0E8_GiveSamusDamage(uint16 j) {  // 0x84CFD5
  ++samus_periodic_damage;
  return 0x41;
}

uint8 PlmSetup_D113_LowerNorfairChozoRoomPlug(uint16 j) {  // 0x84D108
  uint16 a = 0; // a undefined
  level_data[plm_block_indices[j >> 1] >> 1] = a & 0xFFF;
  return 0;
}

uint8 PlmSetup_D127(uint16 j) {  // 0x84D117
  WriteLevelDataBlockTypeAndBts(plm_block_indices[j >> 1], 0xC000);
  return 0;
}

uint8 PlmSetup_D138(uint16 j) {  // 0x84D12B
  WriteLevelDataBlockTypeAndBts(plm_block_indices[j >> 1], 0xE000);
  return 0;
}

