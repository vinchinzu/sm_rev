// Enemies
#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "enemy_types.h"

void Enemy_ItemDrop_MiniKraid(uint16 k) {  // 0xA0B8EE
  int n = 4;
  do {
    eproj_spawn_pt.x = special_death_item_drop_x_origin_pos + (NextRandom() & 0x1F) - 16;
    eproj_spawn_pt.y = special_death_item_drop_y_origin_pos + ((random_number & 0x1F00) >> 8) - 16;
    SpawnEnemyDrops(addr_kEnemyDef_E0FF, k, 0);
  } while (--n);
}

void Enemy_ItemDrop_LowerNorfairSpacePirate(uint16 k) {  // 0xA0B92B
  int n = 5;
  do {
    eproj_spawn_pt.x = special_death_item_drop_x_origin_pos + (NextRandom() & 0x1F) - 16;
    eproj_spawn_pt.y = special_death_item_drop_y_origin_pos + ((random_number & 0x1F00) >> 8) - 16;
    SpawnEnemyDrops(addr_kEnemyDef_F593, k, 0);
  } while (--n);
}

void Enemy_ItemDrop_Metroid(uint16 k) {  // 0xA0B968
  int n = 5;
  do {
    eproj_spawn_pt.x = special_death_item_drop_x_origin_pos + (NextRandom() & 0x1F) - 16;
    eproj_spawn_pt.y = special_death_item_drop_y_origin_pos + ((random_number & 0x1F00) >> 8) - 16;
    SpawnEnemyDrops(addr_kEnemyDef_DD7F, k, 0);
  } while (--n);
}

void Enemy_ItemDrop_Ridley(uint16 k) {  // 0xA0B9A5
  int n = 16;
  do {
    eproj_spawn_pt.x = (NextRandom() & 0x7F) + 64;
    eproj_spawn_pt.y = ((random_number & 0x3F00) >> 8) + 320;
    SpawnEnemyDrops(addr_kEnemyDef_E17F, k, 0);
  } while (--n);
}

void Enemy_ItemDrop_Crocomire(uint16 k) {  // 0xA0B9D8
  int n = 16;
  do {
    eproj_spawn_pt.x = (NextRandom() & 0x7F) + 576;
    eproj_spawn_pt.y = ((random_number & 0x3F00) >> 8) + 96;
    SpawnEnemyDrops(addr_kEnemyDef_DDBF, k, 0);
  } while (--n);
}

void Enemy_ItemDrop_Phantoon(uint16 k) {  // 0xA0BA0B
  int n = 16;
  do {
    eproj_spawn_pt.x = (NextRandom() & 0x7F) + 64;
    eproj_spawn_pt.y = ((random_number & 0x3F00) >> 8) + 96;
    SpawnEnemyDrops(addr_kEnemyDef_E4BF, k, 0);
  } while (--n);
}

void Enemy_ItemDrop_Botwoon(uint16 k) {  // 0xA0BA3E
  int n = 16;
  do {
    eproj_spawn_pt.x = (NextRandom() & 0x7F) + 64;
    eproj_spawn_pt.y = ((random_number & 0x3F00) >> 8) + 128;
    SpawnEnemyDrops(addr_kEnemyDef_F293, k, 0);
  } while (--n);
}

void Enemy_ItemDrop_Kraid(uint16 k) {  // 0xA0BA71
  int n = 16;
  do {
    eproj_spawn_pt.x = (uint8)NextRandom() + 128;
    eproj_spawn_pt.y = ((random_number & 0x3F00) >> 8) + 352;
    SpawnEnemyDrops(addr_kEnemyDef_E2BF, k, 0);
  } while (--n);
}

void Enemy_ItemDrop_BombTorizo(uint16 k) {  // 0xA0BAA4
  int n = 16;
  do {
    eproj_spawn_pt.x = (NextRandom() & 0x7F) + 64;
    eproj_spawn_pt.y = ((random_number & 0x3F00) >> 8) + 96;
    SpawnEnemyDrops(addr_kEnemyDef_EEFF, k, 0);
  } while (--n);
}

void Enemy_ItemDrop_GoldenTorizo(uint16 k) {  // 0xA0BAD7
  int n = 16;
  do {
    eproj_spawn_pt.x = (uint8)NextRandom() + 128;
    eproj_spawn_pt.y = ((random_number & 0x3F00) >> 8) + 288;
    SpawnEnemyDrops(addr_kEnemyDef_EEFF, k, 0);
  } while (--n);
}

void Enemy_ItemDrop_SporeSpawn(uint16 k) {  // 0xA0BB0A
  int n = 16;
  do {
    eproj_spawn_pt.x = (NextRandom() & 0x7F) + 64;
    eproj_spawn_pt.y = ((random_number & 0x3F00) >> 8) + 528;
    SpawnEnemyDrops(addr_kEnemyDef_DF3F, k, 0);
  } while (--n);
}

void Enemy_ItemDrop_Draygon(uint16 k) {  // 0xA0BB3D
  int n = 16;
  do {
    eproj_spawn_pt.x = (uint8)NextRandom() + 128;
    eproj_spawn_pt.y = ((random_number & 0x3F00) >> 8) + 352;
    SpawnEnemyDrops(addr_kEnemyDef_DE3F, k, 0);
  } while (--n);
}
