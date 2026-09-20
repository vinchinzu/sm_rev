#include "pico_ls_room.h"

#include "pico_ls_assets.h"

#include "assets/pico_ls_room.inc"

_Static_assert((int)kPicoLsRoomSpawnX == (int)kPicoLsRoomSpawnXPacked,
               "display spawn X must match the packed gameplay room");
_Static_assert((int)kPicoLsRoomSpawnY == (int)kPicoLsRoomSpawnYPacked,
               "display spawn Y must match the packed gameplay room");
_Static_assert((int)kPicoLsRoomSpawnXPacked / (int)kPicoLsRoomBlockPx ==
                   (int)kPicoLsRoomSpawnBlockX,
               "spawn block X moved");
_Static_assert((int)kPicoLsRoomSpawnYPacked / (int)kPicoLsRoomBlockPx ==
                   (int)kPicoLsRoomSpawnBlockY,
               "spawn block Y moved");
_Static_assert(sizeof(kPicoLsRoomLevelData) / sizeof(kPicoLsRoomLevelData[0]) ==
                   (size_t)kPicoLsRoomWidthBlocks * (size_t)kPicoLsRoomHeightBlocks,
               "level words must be width * height");
_Static_assert(sizeof(kPicoLsRoomBts) ==
                   (size_t)kPicoLsRoomWidthBlocks * (size_t)kPicoLsRoomHeightBlocks,
               "bts must be width * height");
_Static_assert(sizeof(kPicoLsRoomScrolls) ==
                   (size_t)kPicoLsRoomWidthScreens * (size_t)kPicoLsRoomHeightScreens,
               "scrolls must be one byte per screen");

static const MiniBakedRoom kPicoLsBakedRoom = {
    .room_id = (uint16)kPicoLsRoomIdPacked,
    .handle = "landingSite",
    .name = "Landing Site",
    .width_blocks = kPicoLsRoomWidthBlocks,
    .height_blocks = kPicoLsRoomHeightBlocks,
    .block_words = kPicoLsRoomLevelData,
    .bts = kPicoLsRoomBts,
    .scroll_values = kPicoLsRoomScrolls,
    .export_up_scroller = kPicoLsRoomUpScroller,
    .export_down_scroller = kPicoLsRoomDownScroller,
    .export_bg_scrolling = kPicoLsRoomBgScrolling,
    .camera_x = kPicoLsRoomCameraXPacked,
    .camera_y = kPicoLsRoomCameraYPacked,
    .spawn_x = kPicoLsRoomSpawnXPacked,
    .spawn_y = kPicoLsRoomSpawnYPacked,
    .camera_target_x_percent = 50,
    .camera_target_y_percent = 50,
};

bool PicoLsRoom_Install(void) {
  MiniStubs_SetBakedRoom(&kPicoLsBakedRoom);
  return MiniStubs_GetBakedRoom() == &kPicoLsBakedRoom;
}

void PicoLsRoom_Uninstall(void) {
  MiniStubs_SetBakedRoom(NULL);
}

const MiniBakedRoom *PicoLsRoom_Baked(void) {
  return &kPicoLsBakedRoom;
}

size_t PicoLsRoom_PackedSize(void) {
  return sizeof(kPicoLsRoomLevelData) + sizeof(kPicoLsRoomBts) +
         sizeof(kPicoLsRoomScrolls);
}
