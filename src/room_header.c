// Shared room metadata loaders extracted from room_transition.c.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"

#define kStateHeaderTileSets ((uint16*)RomFixedPtr(0x8fe7a7))

void LoadRoomHeader(void) {  // 0x82DE6F
  RoomDefHeader *RoomDefHeader = get_RoomDefHeader(room_ptr);
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
