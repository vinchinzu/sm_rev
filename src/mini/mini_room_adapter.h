#ifndef SM_MINI_ROOM_ADAPTER_H_
#define SM_MINI_ROOM_ADAPTER_H_

#include <stddef.h>

#include "block_reaction.h"
#include "types.h"

enum {
  kMiniBlockSize = kBlockPixelSize,
  kMiniSolidBlock = kBlockType_Solid,
  kMiniRoomHandleCapacity = 32,
  kMiniRoomNameCapacity = 64,
  kMiniDoorwayTransitionCapacity = 8,
  kMiniCameraFollowDefaultTargetPercent = 50,
};

typedef enum MiniRoomSource {
  kMiniRoomSource_Fallback = 0,
  kMiniRoomSource_EditorExport = 1,
  kMiniRoomSource_RomSave = 2,
  kMiniRoomSource_RomDemo = 3,
} MiniRoomSource;

typedef enum MiniSamusSuit {
  kMiniSamusSuit_Power = 0,
  kMiniSamusSuit_Varia = 1,
  kMiniSamusSuit_Gravity = 2,
} MiniSamusSuit;

typedef struct MiniDoorwayTransition {
  bool active;
  int source_block_x;
  int source_block_y;
  int destination_x;
  int destination_y;
  int camera_x;
  int camera_y;
} MiniDoorwayTransition;

typedef struct MiniRoomInfo {
  bool has_room;
  bool uses_rom_room;
  bool booted_from_save_slot;
  bool has_editor_room_visuals;
  bool uses_original_gameplay_runtime;
  bool has_original_enemies;
  bool has_original_plms;
  MiniSamusSuit samus_suit;
  uint16 room_id;
  char room_handle[kMiniRoomHandleCapacity];
  char room_name[kMiniRoomNameCapacity];
  MiniRoomSource room_source;
  int room_left;
  int room_top;
  int room_right;
  int room_bottom;
  int room_width_blocks;
  int room_height_blocks;
  int camera_x;
  int camera_y;
  int spawn_x;
  int spawn_y;
  int camera_target_x_percent;
  int camera_target_y_percent;
  int doorway_count;
  MiniDoorwayTransition doorways[kMiniDoorwayTransitionCapacity];
} MiniRoomInfo;

typedef struct MiniCollisionMapView {
  int block_size;
  int width_blocks;
  int height_blocks;
  int world_left;
  int world_top;
  int world_right;
  int world_bottom;
} MiniCollisionMapView;

typedef struct MiniStubsSnapshot {
  int world_left;
  int world_right;
  int world_ceiling;
  int world_floor;
  bool explicit_room_export_path;
  MiniRoomInfo room_info;
} MiniStubsSnapshot;

void MiniStubs_SetRoomExportPath(const char *path);
void MiniStubs_ConfigureWorld(int viewport_width, int viewport_height);
void MiniStubs_GetRoomInfo(MiniRoomInfo *info);
void MiniStubs_GetCollisionMapView(MiniCollisionMapView *view);
void MiniStubs_SaveSnapshot(MiniStubsSnapshot *snapshot);
void MiniStubs_LoadSnapshot(const MiniStubsSnapshot *snapshot);
uint16 MiniStubs_GetLevelBlock(int block_x, int block_y);
BlockType MiniStubs_GetCollisionMaterial(int block_x, int block_y);
uint8 MiniStubs_GetBts(int block_x, int block_y);
int MiniStubs_GetFloorY(void);
const char *MiniStubs_RoomSourceName(MiniRoomSource source);
const char *MiniStubs_SamusSuitName(MiniSamusSuit suit);
void MiniStubs_ClampCameraToRoom(void);

#endif  // SM_MINI_ROOM_ADAPTER_H_
