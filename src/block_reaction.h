#ifndef BLOCK_REACTION_H_
#define BLOCK_REACTION_H_

#include "types.h"

enum {
  kBlockTypeMask = 0xF000,
  kBlockTypeShift = 12,
  kBlockTileAttributeMask = 0x0FFF,
  kBlockTileMetatileMask = 0x03FF,
  kBlockTileHFlip = 0x0400,
  kBlockTileVFlip = 0x0800,
  kBlockPixelShift = 4,
  kBlockPixelSize = 16,
  kBlockPixelMask = kBlockPixelSize - 1,

  kSlopeBts_ShapeMask = 0x1F,
  kSlopeBts_MirrorX = 0x40,
  kSlopeBts_Ceiling = 0x80,
  kSlopeBts_FirstAlignedShape = 5,
};

typedef enum BlockType {
  kBlockType_Air = 0x0000,
  kBlockType_Slope = 0x1000,
  kBlockType_SpikeAir = 0x2000,
  kBlockType_SpecialAir = 0x3000,
  kBlockType_ShootableAir = 0x4000,
  kBlockType_HorizontalExtension = 0x5000,
  kBlockType_UnusedAir = 0x6000,
  kBlockType_BombableAir = 0x7000,
  kBlockType_Solid = 0x8000,
  kBlockType_Door = 0x9000,
  kBlockType_SpikeBlock = 0xA000,
  kBlockType_SpecialBlock = 0xB000,
  kBlockType_ShootableBlock = 0xC000,
  kBlockType_VerticalExtension = 0xD000,
  kBlockType_GrappleBlock = 0xE000,
  kBlockType_BombableBlock = 0xF000,
} BlockType;

static inline uint16 BlockTypeFromTile(uint16 tile) {
  return tile & kBlockTypeMask;
}

static inline uint16 BlockTypeIndexFromTile(uint16 tile) {
  return (tile & kBlockTypeMask) >> kBlockTypeShift;
}

static inline uint16 BlockTypeFromIndex(uint16 type_index) {
  return (type_index << kBlockTypeShift) & kBlockTypeMask;
}

static inline uint16 BlockTileAttributes(uint16 tile) {
  return tile & kBlockTileAttributeMask;
}

static inline uint16 BlockTileMetatileIndex(uint16 tile) {
  return tile & kBlockTileMetatileMask;
}

static inline bool BlockTileHasHFlip(uint16 tile) {
  return (tile & kBlockTileHFlip) != 0;
}

static inline bool BlockTileHasVFlip(uint16 tile) {
  return (tile & kBlockTileVFlip) != 0;
}

static inline uint16 BlockTileWithType(uint16 tile, BlockType block_type) {
  return BlockTileAttributes(tile) | ((uint16)block_type & kBlockTypeMask);
}

static inline uint16 BlockTileWithTypeIndex(uint16 tile, uint16 type_index) {
  return BlockTileWithType(tile, (BlockType)BlockTypeFromIndex(type_index));
}

static inline uint16 BlockTileMakeAir(uint16 tile) {
  return BlockTileWithType(tile, kBlockType_Air);
}

static inline uint16 BlockTileMakeSolid(uint16 tile) {
  return BlockTileWithType(tile, kBlockType_Solid);
}

typedef struct CollInfo {
  int32 ci_r18_r20;
  uint16 ci_r24;
  uint16 ci_r26;
  uint16 ci_r28;
  uint16 ci_r30;
  uint16 ci_r32;
  uint16 ci_r38;
  uint16 ci_r40;
} CollInfo;

typedef uint8 Func_CollInfo_U8(CollInfo *ci);

uint8 BlockReact_HorizExt(CollInfo *ci);
uint8 BlockReact_VertExt(CollInfo *ci);
uint8 BlockReact_ShootableAir(CollInfo *ci);
uint8 BlockReact_Shootable(CollInfo *ci);
uint8 BlockReact_BombableAir(CollInfo *ci);
uint8 BlockReact_BombableBlock(CollInfo *ci);

#endif
