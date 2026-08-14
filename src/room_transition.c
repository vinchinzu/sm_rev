// Room load, library backgrounds, and StartGameplay.
// Door-transition game states live in door_transition.c.

#include "block_reaction.h"
#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"

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
    level_data[i >> 1] = kBlockType_Solid;

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
      CopyToVramNow(vram_update_dst, Load24(&ED->tile_data), vram_update_size);
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

void LoaadDesinationRoomCreBitset(void) {  // 0x82DDF1
  uint16 room_definition_ptr = get_DoorDef(door_def_ptr)->room_definition_ptr;
  previous_cre_bitset = cre_bitset;
  cre_bitset = get_RoomDefHeader(room_definition_ptr)->cre_bitset_;
}

void LoadLevelScrollAndCre(void) {  // 0x82EA73
  int16 rdf_scroll_ptr;
  uint16 m;
  int8 v10;
  int8 v11;
  int8 v12;

  for (int i = 6398; i >= 0; i -= 2) {
    level_data[i >> 1] = kBlockType_Solid;
    level_data[(i >> 1) + 3200 * 1] = kBlockType_Solid;
    level_data[(i >> 1) + 3200 * 2] = kBlockType_Solid;
    level_data[(i >> 1) + 3200 * 3] = kBlockType_Solid;
  }

  DecompressToMem(Load24(&room_compr_level_data_ptr), (uint8 *)&ram7F_start);

  uint16 size = ram7F_start;
  memmove(custom_background, (uint8*)level_data + size + (size >> 1), size);
  memmove(BTS, (uint8 *)level_data + size, size >> 1);

  if (area_index == 6) {
    DecompressToMem(Load24(&tileset_tile_table_pointer), (uint8*)&tile_table);
  } else {
    if ((cre_bitset & 2) != 0) {
      DecompressToMem(0xb9a09d, (uint8*)&tile_table);
    }
    DecompressToMem(Load24(&tileset_tile_table_pointer), tile_table_cre_hi);
  }
  RoomDefRoomstate *RD = get_RoomDefRoomstate(roomdefroomstate_ptr);
  rdf_scroll_ptr = RD->rdf_scroll_ptr;
  if (rdf_scroll_ptr >= 0) {
    uint16 r18 = RD->rdf_scroll_ptr;
    uint8 r20 = room_height_in_scrolls - 1;
    uint8 v8 = 2;
    uint8 v9 = 0;
    v10 = 0;
    do {
      if (v10 == r20)
        v8 = r18 + 1;
      v12 = v10;
      v11 = 0;
      do {
        scrolls[v9++] = v8;
        ++v11;
      } while (v11 != (uint8)room_width_in_scrolls);
      v10 = v12 + 1;
    } while (v12 + 1 != (uint8)room_height_in_scrolls);
  } else {
    for (m = 0; m != 50; m += 2) {
      *(uint16 *)&scrolls[m] = *(uint16 *)RomPtr_8F(rdf_scroll_ptr);
      rdf_scroll_ptr += 2;
    }
  }
}

void CreatePlmsExecuteDoorAsmRoomSetup(void) {  // 0x82EB6C
  RoomDefRoomstate *RoomDefRoomstate;
  RoomDefRoomstate = get_RoomDefRoomstate(roomdefroomstate_ptr);
  if (RoomDefRoomstate->room_plm_header_ptr) {
    for (int i = RoomDefRoomstate->room_plm_header_ptr; get_RoomPlmEntry(i)->plm_header_ptr_; i += 6)
      SpawnRoomPLM(i);
  }
  RunDoorSetupCode();
  RunRoomSetupCode();
  if (elevator_flags)
    elevator_status = 2;
}

CoroutineRet StartGameplay_Async(void) {  // 0x80A07B
  COROUTINE_BEGIN(coroutine_state_2, 0)
  WriteRegWord(MDMAEN, 0);
  scrolling_finished_hook = 0;
  music_data_index = 0;
  music_track_index = 0;
  timer_status = 0;
  ResetSoundQueues();
  debug_disable_sounds = -1;
  DisableNMI();
  DisableIrqInterrupts();
  LoadDestinationRoomThings();
  COROUTINE_AWAIT(1, Play20FramesOfMusic_Async());
  ClearAnimtiles();
  WaitUntilEndOfVblankAndClearHdma();
  InitializeSpecialEffectsForNewRoom();
  ClearPLMs();
  ClearEprojs();
  ClearPaletteFXObjects();
  UpdateBeamTilesAndPalette();
  LoadColorsForSpritesBeamsAndEnemies();
  LoadEnemies();
  LoadRoomMusic();
  COROUTINE_AWAIT(2, Play20FramesOfMusic_Async());
  UpdateMusicTrackIndex();
  NullFunc();
  ClearBG2Tilemap();
  LoadLevelDataAndOtherThings();
  LoadFXHeader();
  LoadLibraryBackground();
  CalculateLayer2Xpos();
  CalculateLayer2Ypos();
  bg2_x_scroll = layer2_x_pos;
  bg2_y_scroll = layer2_y_pos;
  CalculateBgScrolls();
  DisplayViewablePartOfRoom();
  EnableNMI();
  irqhandler_next_handler = (room_loading_irq_handler == 0) ? 4 : room_loading_irq_handler;
  EnableIrqInterrupts();
  COROUTINE_AWAIT(3, Play20FramesOfMusic_Async());
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x08, 0x08, 0xb7eb });
  door_transition_function = FUNC16(DoorTransition_FadeInScreenAndFinish);
  COROUTINE_END(0);
}

CoroutineRet Play20FramesOfMusic_Async(void) {  // 0x80A12B
  COROUTINE_BEGIN(coroutine_state_3, 0)
  EnableNMI();
  for(my_counter = 0; my_counter != 20; my_counter++) {
    HandleMusicQueue();
    COROUTINE_AWAIT(1, WaitForNMI_Async());
  }
  DisableNMI();
  COROUTINE_END(0);
}

void ResumeGameplay(void) {  // 0x80A149
  WriteRegWord(MDMAEN, 0);
  DisableNMI();
  DisableIrqInterrupts();
  LoadCRETilesTilesetTilesAndPalette();
  LoadLibraryBackground();
  DisplayViewablePartOfRoom();
  LoadRoomPlmGfx();
  EnableNMI();
  EnableIrqInterrupts();
}
