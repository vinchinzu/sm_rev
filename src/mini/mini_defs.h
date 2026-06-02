#ifndef SM_MINI_DEFS_H_
#define SM_MINI_DEFS_H_

enum {
  kMiniCreTilesByteOffset = 0x5000,
  kMiniCreBg2TileBase = kMiniCreTilesByteOffset / 32,
  kMiniDefaultFrames = 180,
  kMiniGameWidth = 256,
  kMiniGameHeight = 224,
  kMiniWindowWidth = 1440,
  kMiniWindowHeight = 900,
  kMiniFrameDelayMs = 16,
  kMiniMaxPlayers = 2,
};

#endif  // SM_MINI_DEFS_H_
