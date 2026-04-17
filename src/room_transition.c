#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "sm_82_data.h"

static Func_Y_Y *const kUpdateBackgroundCommands[8] = {
  UpdateBackgroundCommand_0_Finish,
  UpdateBackgroundCommand_2_TransferToVram,
  UpdateBackgroundCommand_4_Decompression,
  UpdateBackgroundCommand_6_ClearFxTilemap,
  UpdateBackgroundCommand_8_TransferToVramAndSetBg3,
  UpdateBackgroundCommand_A_ClearBg2Tilemap,
  UpdateBackgroundCommand_C_ClearKraidBg2Tilemap,
  UpdateBackgroundCommand_E_DoorDependentTransferToVRAM,
};

static Func_Y_Y *const kLoadLibraryBackgroundFuncs[8] = {  // 0x82E97C
  LoadLibraryBackgroundFunc_0_DONE,
  LoadLibraryBackgroundFunc_2_TransferToVram,
  LoadLibraryBackgroundFunc_4_Decompress,
  LoadLibraryBackgroundFunc_6_ClearFxTilemap,
  LoadLibraryBackgroundFunc_8_TransferToVramAndSetBg3,
  LoadLibraryBackgroundFunc_A_ClearBG2Tilemap,
  LoadLibraryBackgroundFunc_C_ClearKraidLayer2,
  LoadLibraryBackgroundFunc_E_DoorDependentTransferToVram,
};

CoroutineRet CallDoorTransitionFunction_Async(uint32 ea) {
  switch (ea) {
  case fnDoorTransitionFunction_HandleElevator: return DoorTransitionFunction_HandleElevator();
  case fnDoorTransitionFunction_Wait48frames: return DoorTransitionFunction_Wait48frames();
  case fnDoorTransitionFunction_WaitForSoundsToFinish: return DoorTransitionFunction_WaitForSoundsToFinish();
  case fnDoorTransitionFunction_FadeOutScreen: return DoorTransitionFunction_FadeOutScreen();
  case fnDoorTransitionFunction_LoadDoorHeaderEtc: return DoorTransitionFunction_LoadDoorHeaderEtc();
  case fnDoorTransitionFunction_ScrollScreenToAlignment: return DoorTransitionFunction_ScrollScreenToAlignment();
  case fnDoorTransitionFunction_FixDoorsMovingUp: return DoorTransitionFunction_FixDoorsMovingUp();
  case fnDoorTransitionFunction_SetupNewRoom: return DoorTransitionFunction_SetupNewRoom();
  case fnDoorTransitionFunction_SetupScrolling: return DoorTransitionFunction_SetupScrolling();
  case fnDoorTransitionFunction_PlaceSamusLoadTiles: return DoorTransitionFunction_PlaceSamusLoadTiles();
  case fnDoorTransitionFunction_LoadMoreThings_Async: return DoorTransitionFunction_LoadMoreThings_Async();
  case fnDoorTransition_C_HandleAnimTiles: return DoorTransition_C_HandleAnimTiles();
  case fnDoorTransition_WaitForMusicToClear: return DoorTransition_WaitForMusicToClear();
  case fnDoorTransition_HandleTransition: return DoorTransition_HandleTransition();
  case fnDoorTransition_FadeInScreenAndFinish: return DoorTransition_FadeInScreenAndFinish();
  default: return Unreachable();
  }
}

CoroutineRet GameState_9_HitDoorBlock(void) {  // 0x82E169
  return CallDoorTransitionFunction_Async(door_transition_function | 0x820000);
}

CoroutineRet DoorTransitionFunction_HandleElevator(void) {  // 0x82E17D
  if (!elevator_flags) {
    ++game_state;
    return GameState_10_LoadingNextRoom_Async();
  }
  CallSomeSamusCode(0);
  if ((elevator_direction & 0x8000) != 0) {
    ++game_state;
    return GameState_10_LoadingNextRoom_Async();
  }
  downwards_elevator_delay_timer = 48;
  door_transition_function = FUNC16(DoorTransitionFunction_Wait48frames);
  return DoorTransitionFunction_Wait48frames();
}

CoroutineRet DoorTransitionFunction_Wait48frames(void) {  // 0x82E19F
  if ((--downwards_elevator_delay_timer & 0x8000) != 0) {
    ++game_state;
    return GameState_10_LoadingNextRoom_Async();
  } else {
    DetermineWhichEnemiesToProcess();
    EnemyMain();
    DrawSamusEnemiesAndProjectiles();
    EnsureSamusDrawnEachFrame();
  }
  return kCoroutineNone;
}

CoroutineRet GameState_10_LoadingNextRoom_Async(void) {  // 0x82E1B7
  door_transition_flag_enemies = 1;
  door_transition_flag_elevator_zebetites = 1;
  debug_disable_minimap = 0;
  save_station_lockout_flag = 0;
  DetermineWhichEnemiesToProcess();
  EnemyMain();
  DrawSamusEnemiesAndProjectiles();
  EnsureSamusDrawnEachFrame();
  LoadDestinationRoomCreBitset();
  for (int i = 254; i >= 0; i -= 2) {
    int v1 = i >> 1;
    target_palettes[v1] = 0;
    target_palettes[v1 + 128] = 0;
  }
  target_palettes[9] = palette_buffer[9];
  target_palettes[10] = palette_buffer[10];
  target_palettes[13] = palette_buffer[13];
  target_palettes[14] = palette_buffer[14];
  target_palettes[17] = palette_buffer[17];
  target_palettes[18] = palette_buffer[18];
  target_palettes[19] = palette_buffer[19];
  target_palettes[29] = palette_buffer[29];
  if (((previous_cre_bitset | cre_bitset) & 1) == 0) {
    target_palettes[20] = palette_buffer[20];
    target_palettes[21] = palette_buffer[21];
    target_palettes[22] = palette_buffer[22];
    target_palettes[23] = palette_buffer[23];
    target_palettes[28] = palette_buffer[28];
    if (timer_status) {
      target_palettes[209] = palette_buffer[209];
      target_palettes[210] = palette_buffer[210];
      target_palettes[212] = palette_buffer[212];
      target_palettes[221] = palette_buffer[221];
      DrawTimer();
    }
  }
  ClearSoundsWhenGoingThroughDoor();
  QueueSfx2_Max15(0x71);
  debug_disable_sounds = -1;
  door_transition_function = FUNC16(DoorTransitionFunction_WaitForSoundsToFinish);
  ++game_state;
  return kCoroutineNone;
}

CoroutineRet GameState_11_LoadingNextRoom_Async(void) {  // 0x82E288
  COROUTINE_AWAIT_ONLY(CallDoorTransitionFunction_Async(door_transition_function | 0x820000));
  if (timer_status)
    DrawTimer();
  return kCoroutineNone;
}

CoroutineRet DoorTransitionFunction_WaitForSoundsToFinish(void) {  // 0x82E29E
  DetermineWhichEnemiesToProcess();
  EnemyMain();
  DrawSamusEnemiesAndProjectiles();
  EnsureSamusDrawnEachFrame();
  if (((sfx_writepos[0] - sfx_readpos[0]) & 0xF) == 0
      && ((sfx_writepos[1] - sfx_readpos[1]) & 0xF) == 0
      && ((sfx_writepos[2] - sfx_readpos[2]) & 0xF) == 0) {
    door_transition_function = FUNC16(DoorTransitionFunction_FadeOutScreen);
  }
  return kCoroutineNone;
}

CoroutineRet DoorTransitionFunction_FadeOutScreen(void) {  // 0x82E2DB
  if (AdvancePaletteFadeForAllPalettes_0xc()) {
    door_transition_function = FUNC16(DoorTransitionFunction_LoadDoorHeaderEtc);
  } else {
    DetermineWhichEnemiesToProcess();
    EnemyMain();
    DrawSamusEnemiesAndProjectiles();
    EnsureSamusDrawnEachFrame();
  }
  return kCoroutineNone;
}

CoroutineRet DoorTransitionFunction_LoadDoorHeaderEtc(void) {  // 0x82E2F7
  LoadDoorHeader();
  sub_8882AC();
  hdma_objects_enable_flag &= ~0x8000;
  irqhandler_next_handler = 8;
  door_transition_function = FUNC16(DoorTransitionFunction_ScrollScreenToAlignment);
  return kCoroutineNone;
}

CoroutineRet DoorTransitionFunction_ScrollScreenToAlignment(void) {  // 0x82E310
  if ((door_direction & 2) != 0) {
    if ((uint8)layer1_x_pos) {
      if ((layer1_x_pos & 0x80) != 0)
        ++layer1_x_pos;
      else
        --layer1_x_pos;
      goto LABEL_10;
    }
  } else if ((uint8)layer1_y_pos) {
    if ((layer1_y_pos & 0x80) != 0)
      ++layer1_y_pos;
    else
      --layer1_y_pos;
LABEL_10:
    CalculateLayer2PosAndScrollsWhenScrolling();
    return kCoroutineNone;
  }
  CalculateLayer2PosAndScrollsWhenScrolling();
  door_transition_function = FUNC16(DoorTransitionFunction_FixDoorsMovingUp);
  return kCoroutineNone;
}

CoroutineRet DoorTransitionFunction_FixDoorsMovingUp(void) {  // 0x82E353
  if ((door_direction & 3) == 3)
    FixDoorsMovingUp();
  door_transition_function = FUNC16(DoorTransitionFunction_SetupNewRoom);
  return kCoroutineNone;
}

CoroutineRet DoorTransitionFunction_SetupNewRoom(void) {  // 0x82E36E
  SaveMapExploredifElevator();
  LoadRoomHeader();
  LoadStateHeader();
  LoadMapExploredIfElevator();
  InitializeSpecialEffectsForNewRoom();
  LoadLevelDataAndOtherThings();
  door_transition_function = FUNC16(DoorTransitionFunction_SetupScrolling);
  return kCoroutineNone;
}

CoroutineRet DoorTransitionFunction_SetupScrolling(void) {  // 0x82E38E
  reg_BG2HOFS = 0;
  reg_BG2VOFS = 0;
  scrolling_finished_hook = 0;
  if ((door_direction & 3) == 2)
    ++reg_BG1VOFS;
  if ((door_direction & 3) != 3)
    door_transition_frame_counter = 0;
  DoorTransitionScrollingSetup();
  door_transition_function = FUNC16(DoorTransitionFunction_PlaceSamusLoadTiles);
  return kCoroutineNone;
}

CoroutineRet DoorTransitionFunction_PlaceSamusLoadTiles(void) {  // 0x82E3C0
  uint16 v0;

  samus_x_pos = layer1_x_pos + (uint8)samus_x_pos;
  samus_prev_x_pos = samus_x_pos;
  samus_y_pos = layer1_y_pos + (uint8)samus_y_pos;
  samus_prev_y_pos = samus_y_pos;
  door_transition_flag = 0;
  if ((door_direction & 3) == 2)
    v0 = 16;
  else
    v0 = 22;
  irqhandler_next_handler = v0;
  WaitUntilEndOfVblankAndEnableIrq();
  if ((cre_bitset & 2) != 0 && door_def_ptr != addr_kDoorDef_947a) {
    DecompressToVRAM(0xb98000, 0x5000);
  }
  DecompressToMem(Load24(&tileset_tiles_pointer), (uint8*)tilemap_stuff);
  DecompressToMem(Load24(&tileset_compr_palette_ptr), (uint8*)target_palettes);
  CopyToVramNow(0x0000, 0x7e2000, 0x2000);
  CopyToVramNow(0x1000, 0x7e4000, 0x2000);
  CopyToVramNow(0x2000, 0x7e6000, 0x1000);
  if ((cre_bitset & 6) != 0 && door_def_ptr != addr_kDoorDef_947a) {
    CopyToVramNow(0x2800, 0x7e7000, 0x1000);
    CopyToVramNow(0x3000, 0x7e8000, 0x2000);
    CopyToVramNow(0x4000, 0x9ab200, 0x1000);
  }
  if ((door_direction & 3) == 3)
    irqhandler_next_handler = 16;
  door_transition_function = FUNC16(DoorTransitionFunction_LoadMoreThings_Async);
  return kCoroutineNone;
}

CoroutineRet DoorTransitionFunction_LoadMoreThings_Async(void) {
  uint16 bg_data_ptr;

  COROUTINE_BEGIN(coroutine_state_1, 0);

  LoadEnemyGfxToVram();
  LoadRoomMusic();
  ClearEprojs();
  ClearAnimtiles();
  ClearPaletteFXObjects();
  ClearPLMs();
  LoadFXHeader();
  SpawnDoorClosingPLM();
  UpdateBeamTilesAndPalette();
  LoadColorsForSpritesBeamsAndEnemies();
  LoadEnemies();
  InitializeEnemies();
  ResetProjectileData();
  Samus_LoadSuitTargetPalette();
  ClearFxTilemap();
  if (fx_tilemap_ptr)
    CopyToVramNow(0x5BE0, 0x8a0000 | fx_tilemap_ptr, 2112);
  bg_data_ptr = get_RoomDefRoomstate(roomdefroomstate_ptr)->bg_data_ptr;
  if (bg_data_ptr & 0x8000) {
    do {
      int v1 = *(uint16 *)RomPtr_8F(bg_data_ptr) >> 1;
      bg_data_ptr = kUpdateBackgroundCommands[v1](bg_data_ptr + 2);
    } while (bg_data_ptr);
  }
  // Wait until the screen has finished scrolling as a result of the door opening
  while ((door_transition_flag & 0x8000) == 0) {
    COROUTINE_AWAIT(1, WaitForNMI_Async());
  }
  palette_buffer[196] = 15328;
  SpawnBG3ScrollHdmaObject();
  hdma_objects_enable_flag |= 0x8000;
  COROUTINE_AWAIT(2, PlmHandler_Async());

  if ((door_direction & 2) == 0) {
    if ((door_direction & 3) != 0)
      samus_x_pos &= 0xFFF8;
    else
      samus_x_pos |= 7;
  }
  door_transition_function = FUNC16(DoorTransition_C_HandleAnimTiles);
  COROUTINE_END(0);
}

void ClearFxTilemap(void) {  // 0x82E566
  for (int i = 959; i >= 0; --i) {
    ram4000.xray_tilemaps[i] = 6222;
    ram4000.xray_tilemaps[i + 960] = 6222;
  }
  CopyToVramNow(0x5880, 0x7e4000, 0xf00);
}

void ClearBg2Tilemap(void) {  // 0x82E583
  for (int i = 1023; i >= 0; --i) {
    ram4000.xray_tilemaps[i] = 824;
    ram4000.xray_tilemaps[i + 1024] = 824;
  }
  CopyToVramNow(0x4800, 0x7e4000, 0x1000);
}

void ClearKraidBg2Tilemap(void) {  // 0x82E5A0
  for (int i = 1023; i >= 0; --i) {
    ram4000.xray_tilemaps[i] = 824;
    ram4000.xray_tilemaps[i + 1024] = 824;
  }
  CopyToVramNow(0x4000, 0x7e4000, 0x1000);
  CopyToVramNow(0x4800, 0x7e4000, 0x1000);
}

uint16 UpdateBackgroundCommand_0_Finish(uint16 y) {  // 0x82E5D7
  return 0;
}

uint16 UpdateBackgroundCommand_E_DoorDependentTransferToVRAM(uint16 j) {  // 0x82E5D9
  if (door_def_ptr == *(uint16 *)RomPtr_8F(j))
    return UpdateBackgroundCommand_2_TransferToVram(j + 2);
  else
    return j + 9;
}

uint16 UpdateBackgroundCommand_2_TransferToVram(uint16 j) {  // 0x82E5EB
  const uint8 *v1 = RomPtr_8F(j);
  CopyToVramNow(GET_WORD(v1 + 3), Load24((LongPtr *)v1), GET_WORD(v1 + 5));
  return j + 7;
}

uint16 UpdateBackgroundCommand_4_Decompression(uint16 j) {  // 0x82E616
  const uint8 *v1 = RomPtr_8F(j);
  DecompressToMem(Load24((LongPtr *)v1), g_ram + GET_WORD(v1 + 3));
  return j + 5;
}

uint16 UpdateBackgroundCommand_6_ClearFxTilemap(uint16 j) {  // 0x82E637
  ClearFxTilemap();
  return j;
}

uint16 UpdateBackgroundCommand_8_TransferToVramAndSetBg3(uint16 j) {  // 0x82E63E
  j = UpdateBackgroundCommand_2_TransferToVram(j);
  reg_BG34NBA = 2;
  return j;
}

uint16 UpdateBackgroundCommand_A_ClearBg2Tilemap(uint16 j) {  // 0x82E64B
  ClearBg2Tilemap();
  return j;
}

uint16 UpdateBackgroundCommand_C_ClearKraidBg2Tilemap(uint16 j) {  // 0x82E652
  ClearKraidBg2Tilemap();
  return j;
}

CoroutineRet DoorTransition_C_HandleAnimTiles(void) {  // 0x82E659
  AnimtilesHandler();
  door_transition_function = FUNC16(DoorTransition_WaitForMusicToClear);
  return kCoroutineNone;
}

CoroutineRet DoorTransition_WaitForMusicToClear(void) {  // 0x82E664
  if (!HasQueuedMusic()) {
    door_transition_function = FUNC16(DoorTransition_HandleTransition);
    LoadNewMusicTrackIfChanged();
  }
  return kCoroutineNone;
}

CoroutineRet DoorTransition_HandleTransition(void) {  // 0x82E6A2
  if ((samus_x_pos & 0xF0) == 16) {
    samus_x_pos = (samus_x_pos | 0xF) + 8;
  } else if ((samus_x_pos & 0xF0) == 224) {
    samus_x_pos = (samus_x_pos & 0xFFF0) - 8;
  }
  if ((samus_y_pos & 0xF0) == 16)
    samus_y_pos = (samus_y_pos | 0xF) + 8;
  for (int i = 510; i >= 0; i -= 2) {
    *(uint16 *)&mother_brain_indirect_hdma[i] = 0;
    *(uint16 *)(&hdma_window_1_left_pos[0].field_0 + i) = 0;
  }
  uint16 v1 = room_loading_irq_handler;
  if (!room_loading_irq_handler)
    v1 = 4;
  irqhandler_next_handler = v1;
  PointlessFunctionStupidToo();
  if (elevator_flags) {
    if ((elevator_direction & 0x8000) == 0)
      CallSomeSamusCode(7);
    else
      CallSomeSamusCode(0);
  }
  SetLiquidPhysicsType();
  door_transition_function = FUNC16(DoorTransition_FadeInScreenAndFinish);
  *(uint16 *)&reg_INIDISP |= 0x1F;
  return kCoroutineNone;
}

CoroutineRet DoorTransition_FadeInScreenAndFinish(void) {  // 0x82E737
  AnimtilesHandler();
  DetermineWhichEnemiesToProcess();
  EnemyMain();
  EprojRunAll();
  DrawSamusEnemiesAndProjectiles();
  EnsureSamusDrawnEachFrame();
  QueueEnemyBG2TilemapTransfers();
  if (AdvancePaletteFadeForAllPalettes_0xc()) {
    debug_disable_sounds = 0;
    PlaySpinJumpSoundIfSpinJumping();
    door_transition_flag_elevator_zebetites = 0;
    door_transition_flag_enemies = 0;
    game_state = kGameState_8_MainGameplay;
  }
  return kCoroutineNone;
}

void LoadDestinationRoomThings(void) {  // 0x82E76B
  LoadDestinationRoomCreBitset();
  LoadDoorHeader();
  LoadRoomHeader();
  LoadStateHeader();
  LoadCRETilesTilesetTilesAndPalette();
}

void LoadCRETilesTilesetTilesAndPalette(void) {  // 0x82E783
  elevator_flags = 0;
  WriteRegWord(VMAIN, 0x80);
  WriteRegWord(VMADDL, 0x5000 >> 1);
  DecompressToVRAM(0xb98000, 0x5000);
  WriteRegWord(VMADDL, 0);
  DecompressToVRAM(Load24(&tileset_tiles_pointer), 0);
  DecompressToMem(Load24(&tileset_compr_palette_ptr), (uint8*)target_palettes);
}

void LoadLevelDataAndOtherThings(void) {  // 0x82E7D3
  int16 rdf_scroll_ptr;
  uint16 m;
  int8 v10;
  int8 v11;
  uint16 n;

  for (int i = 25598; i >= 0; i -= 2)
    level_data[i >> 1] = 0x8000;
  DecompressToMem(Load24(&room_compr_level_data_ptr), (uint8 *)&ram7F_start);

  uint16 size = ram7F_start;
  memmove(custom_background, (uint8 *)level_data + size + (size >> 1), size);
  memmove(BTS, (uint8 *)level_data + size, size >> 1);

  if (area_index == 6) {
    DecompressToMem(Load24(&tileset_tile_table_pointer), (uint8*)&tile_table);
  } else {
    DecompressToMem(0xb9a09d, (uint8*)&tile_table);
    DecompressToMem(Load24(&tileset_tile_table_pointer), tile_table_cre_hi);
  }
  RoomDefRoomstate *RD = get_RoomDefRoomstate(roomdefroomstate_ptr);
  rdf_scroll_ptr = RD->rdf_scroll_ptr;
  if (rdf_scroll_ptr >= 0) {
    uint16 scrollval = RD->rdf_scroll_ptr;
    uint8 r20 = room_height_in_scrolls - 1;
    uint8 v8 = 2;
    uint8 v9 = 0;
    v10 = 0;
    do {
      if (v10 == r20)
        v8 = scrollval + 1;
      v11 = 0;
      do {
        scrolls[v9++] = v8;
        ++v11;
      } while (v11 != (uint8)room_width_in_scrolls);
      v10++;
    } while (v10 != (uint8)room_height_in_scrolls);
  } else {
    for (m = 0; m != 50; m += 2) {
      *(uint16 *)&scrolls[m] = *(uint16 *)RomPtr_8F(rdf_scroll_ptr);
      rdf_scroll_ptr += 2;
    }
  }
  if (RD->room_plm_header_ptr) {
    for (n = RD->room_plm_header_ptr; get_RoomPlmEntry(n)->plm_header_ptr_; n += 6)
      SpawnRoomPLM(n);
  }
  RunDoorSetupCode();
  RunRoomSetupCode();
  if (elevator_flags)
    elevator_status = 2;
}

void LoadDestinationRoomCreBitset(void) {  // 0x82DDF1
  uint16 room_definition_ptr = get_DoorDef(door_def_ptr)->room_definition_ptr;
  previous_cre_bitset = cre_bitset;
  cre_bitset = get_RoomDefHeader(room_definition_ptr)->cre_bitset_;
}

void LoadDoorHeader(void) {  // 0x82DE12
  DoorDef *DoorDef;
  int16 samus_distance_from_door;

  DoorDef = get_DoorDef(door_def_ptr);
  room_ptr = DoorDef->room_definition_ptr;
  elevator_door_properties_orientation = *(uint16 *)&DoorDef->door_bitflags;
  elevator_flags = elevator_door_properties_orientation & 0x80;
  door_direction = DoorDef->door_orientation;
  door_destination_x_pos = DoorDef->x_pos_in_room << 8;
  door_destination_y_pos = DoorDef->y_pos_in_room << 8;
  samus_distance_from_door = DoorDef->samus_distance_from_door;
  if (samus_distance_from_door < 0) {
    if ((door_direction & 2) != 0)
      samus_distance_from_door = 384;
    else
      samus_distance_from_door = 200;
  }
  uint32 t = (uint32)samus_distance_from_door << 8;
  samus_door_transition_subspeed = t & 0xffff;
  samus_door_transition_speed = t >> 16;
}

void LoadRoomHeader(void) {  // 0x82DE6F
  RoomDefHeader *RoomDefHeader;

  RoomDefHeader = get_RoomDefHeader(room_ptr);
  room_index = RoomDefHeader->semiunique_room_number;
  area_index = RoomDefHeader->area_index_;
  room_x_coordinate_on_map = RoomDefHeader->x_coordinate_on_map;
  room_y_coordinate_on_map = RoomDefHeader->y_coordinate_on_map;
  room_width_in_scrolls = RoomDefHeader->width;
  room_width_in_blocks = 16 * room_width_in_scrolls;
  room_height_in_scrolls = RoomDefHeader->height;
  room_height_in_blocks = 16 * room_height_in_scrolls;
  up_scroller = RoomDefHeader->up_scroller_;
  down_scroller = RoomDefHeader->down_scroller_;
  door_list_pointer = RoomDefHeader->ptr_to_doorout;
  HandleRoomDefStateSelect(room_ptr);
  uint16 prod = Mult8x8(room_width_in_blocks, room_height_in_blocks);
  room_size_in_blocks = 2 * prod;
}

void LoadStateHeader(void) {  // 0x82DEF2
  RoomDefRoomstate *RD = get_RoomDefRoomstate(roomdefroomstate_ptr);
  TileSet *TS = get_TileSet(kStateHeaderTileSets[RD->graphics_set]);
  tileset_tile_table_pointer = TS->tile_table_ptr;
  tileset_tiles_pointer = TS->tiles_ptr;
  tileset_compr_palette_ptr = TS->palette_ptr;
  room_compr_level_data_ptr = RD->compressed_room_map_ptr;
  room_music_data_index = RD->music_track;
  room_music_track_index = RD->music_control;
  room_layer3_asm_ptr = RD->room_layer3_fx_ptr;
  room_enemy_population_ptr = RD->enemy_population_ptr_;
  room_enemy_tilesets_ptr = RD->enemy_tilesets_ptr;
  *(uint16 *)&layer2_scroll_x = RD->vertical_screen_nudge_limit;
  room_main_code_ptr = RD->main_code_ptr;
}

void WaitUntilEndOfVblankAndEnableIrq(void) {  // 0x82DF69
  if ((reg_NMITIMEN & 0x30) != 0x30)
    EnableIrqInterruptsNow();
}

void PointlessFunctionStupidToo(void) {  // 0x82DF80
}

void SaveMapExploredifElevator(void) {  // 0x82DF99
  if ((elevator_door_properties_orientation & 0xF) != 0)
    SetElevatorsAsUsed();
  if ((get_DoorDef(door_def_ptr)->door_bitflags & 0x40) != 0)
    SaveExploredMapTilesToSaved();
}

void LoadMapExploredIfElevator(void) {  // 0x82DFB6
  if ((get_DoorDef(door_def_ptr)->door_bitflags & 0x40) != 0)
    LoadMirrorOfExploredMapTiles();
}

void EnsureSamusDrawnEachFrame(void) {  // 0x82DFC7
  if (!elevator_flags)
    Samus_DrawWhenNotAnimatingOrDying();
}

void LoadEnemyGfxToVram(void) {  // 0x82DFD1
  EnemyDef *ED;

  uint16 dst = 0x7000;
  uint16 v0 = room_enemy_tilesets_ptr;
  if (room_enemy_tilesets_ptr) {
    for (int i = room_enemy_tilesets_ptr; ; v0 = i) {
      uint16 enemy_def = get_EnemyTileset(v0)->enemy_def;
      if (enemy_def == 0xFFFF)
        break;
      ED = get_EnemyDef_A2(enemy_def);
      uint16 vram_update_size, vram_update_dst;
      if ((ED->tile_data_size & 0x8000) != 0) {
        vram_update_size = ED->tile_data_size & 0x7FFF;
        vram_update_dst = ((uint16)(get_EnemyTileset(i)->vram_dst & 0xF000) >> 4) | 0x6000;
      } else {
        vram_update_size = ED->tile_data_size;
        vram_update_dst = dst;
        dst += ED->tile_data_size >> 1;
      }
      DecompressToVRAM(Load24(&ED->tile_data), vram_update_dst);
      i += 4;
    }
  }
}

void LoadRoomMusic(void) {  // 0x82E071
  if (game_state < kGameState_40_TransitionToDemo && room_music_data_index && room_music_data_index != music_data_index) {
    QueueMusic_Delayed8(0);
    QueueMusic_Delayed8(room_music_data_index | 0xFF00);
  }
}

void UpdateMusicTrackIndex(void) {  // 0x82E09B
  if (game_state < kGameState_40_TransitionToDemo && room_music_track_index) {
    uint16 r18 = room_music_data_index << 8;
    r18 |= room_music_track_index;
    uint16 r20 = music_data_index << 8;
    r20 |= music_track_index;
    if (r18 != r20)
      music_track_index = room_music_track_index;
  }
}

void LoadNewMusicTrackIfChanged(void) {  // 0x82E0D5
  if (game_state < 0x28 && room_music_track_index) {
    uint16 r18 = room_music_data_index << 8;
    r18 |= room_music_track_index;
    uint16 r20 = music_data_index << 8;
    r20 |= music_track_index;
    if (r18 != r20)
      QueueMusic_DelayedY(room_music_track_index, 6);
  }
}

void PlayRoomMusicTrackAfterAFrames(uint16 a) {  // 0x82E118
  if (game_state < kGameState_40_TransitionToDemo) {
    QueueMusic_DelayedY(0, a);
    QueueMusic_Delayed8(music_track_index);
  }
}

void NullFunc(void) {  // 0x82E113
}

void SpawnDoorClosingPLM(void) {  // 0x82E8EB
  if (!CheckIfColoredDoorCapSpawned()) {
    const uint16 *v0 = (const uint16 *)RomPtr_8F(2 * door_direction + addr_kDoorClosingPlmIds);
    if (*v0) {
      RoomPlmEntry *rp = door_closing_plm_entry;
      rp->plm_header_ptr_ = *v0;
      *(uint16 *)&rp->x_block = *(uint16 *)&get_DoorDef(door_def_ptr)->x_pos_plm;
      rp->plm_room_argument = 0;
      SpawnRoomPLM(0x12);
    }
  }
}

uint8 CheckIfColoredDoorCapSpawned(void) {  // 0x82E91C
  DoorDef *DD = get_DoorDef(door_def_ptr);
  uint16 v2 = 2 * (Mult8x8(DD->y_pos_plm, room_width_in_blocks) + DD->x_pos_plm);
  uint16 v3 = 78;
  while (v2 != plm_block_indices[v3 >> 1]) {
    v3 -= 2;
    if ((v3 & 0x8000) != 0)
      return 0;
  }
  return 1;
}

uint16 LoadLibraryBackgroundFunc_0_DONE(uint16 j) {  // 0x82E9E5
  return 0;
}

uint16 LoadLibraryBackgroundFunc_E_DoorDependentTransferToVram(uint16 j) {  // 0x82E9E7
  if (door_def_ptr == get_LoadBg_E(j)->field_0)
    return LoadLibraryBackgroundFunc_2_TransferToVram(j + 2);
  else
    return j + 9;
}

uint16 LoadLibraryBackgroundFunc_2_TransferToVram(uint16 j) {  // 0x82E9F9
  uint16 *LoadBg_28 = (uint16 *)RomPtr_8F(j);
  WriteRegWord(VMADDL, *(uint16 *)((uint8 *)LoadBg_28 + 3));
  WriteRegWord(DMAP1, 0x1801);
  WriteRegWord(A1T1L, *LoadBg_28);
  WriteRegWord(A1B1, LoadBg_28[1]);
  WriteRegWord(DAS1L, *(uint16 *)((uint8 *)LoadBg_28 + 5));
  WriteReg(VMAIN, 0x80);
  WriteReg(MDMAEN, 2);
  return j + 7;
}

uint16 LoadLibraryBackgroundFunc_4_Decompress(uint16 j) {  // 0x82EA2D
  const uint8 *p = RomPtr_8F(j);
  DecompressToMem(Load24((LongPtr *)p), g_ram + GET_WORD(p + 3));
  return j + 5;
}

uint16 LoadLibraryBackgroundFunc_6_ClearFxTilemap(uint16 j) {  // 0x82EA4E
  ClearFXTilemap();
  return j;
}

uint16 LoadLibraryBackgroundFunc_A_ClearBG2Tilemap(uint16 j) {  // 0x82EA56
  ClearBG2Tilemap();
  return j;
}

uint16 LoadLibraryBackgroundFunc_C_ClearKraidLayer2(uint16 j) {  // 0x82EA5E
  ClearBG2Tilemap();
  return j;
}

uint16 LoadLibraryBackgroundFunc_8_TransferToVramAndSetBg3(uint16 j) {  // 0x82EA66
  j = LoadLibraryBackgroundFunc_2_TransferToVram(j);
  reg_BG34NBA = 2;
  return j;
}

void LoadLibraryBackground(void) {
  uint16 bg_data_ptr;
  ClearFxTilemap();
  bg_data_ptr = get_RoomDefRoomstate(roomdefroomstate_ptr)->bg_data_ptr;
  if (bg_data_ptr & 0x8000) {
    do {
      uint16 v1 = *(uint16 *)RomPtr_8F(bg_data_ptr);
      bg_data_ptr = kLoadLibraryBackgroundFuncs[v1 >> 1](bg_data_ptr + 2);
    } while (bg_data_ptr);
  }
}
