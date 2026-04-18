#ifndef BLOCK_REACTION_H_
#define BLOCK_REACTION_H_

#include "types.h"

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
