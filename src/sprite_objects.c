// Sprite object runtime: spawn/handle/draw/clear and VM instruction helpers.
// Extracted from sm_b4.c.

#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "enemy_types.h"
#include "sm_rtl.h"

// The sprite object table holds 32 slots. sprite_object_index iterates as a
// byte offset (i >> 1 = slot), so the last valid byte offset is 62.
enum {
  kSpriteObject_LastByteOffset = 62,
};

// Offscreen cull bounds (in pixels, relative to layer1 scroll).
enum {
  kSpriteObject_CullMargin = 16,
  kSpriteObject_CullRight = 272,
  kSpriteObject_CullBottom = 272,
};

// Palette word layout: low 9 bits = palette index, bits 9..11 = flip/priority.
enum {
  kSpriteObject_PaletteIndexMask = 0x1FF,
  kSpriteObject_PaletteFlagsMask = 0xE00,
};

// Instruction VM constants.
enum {
  kSpriteObject_NoSlot = 0xFFFF,
  kSpriteObject_HoldForever = 0x7FFF,  // latched timer so the instr re-fires every frame
};

// ROM bank used to resolve sprite instruction list pointers.
#define kSpriteObject_RomBank 0xB4

// Pointer table of per-ilist-id instruction-list addresses in bank B4.
#define kCreateSprite_Ilists ((uint16*)RomFixedPtr(0xb4bda8))

// Clearing all per-sprite arrays at once: they are laid out contiguously
// (instr list, timer, palette, x/y pos, x/y subpos, disable flag), each 128
// bytes. Writing through sprite_instr_list_ptrs past its own length zeroes the
// adjacent arrays too. Preserves original behavior from bank B4.
enum {
  kSpriteObject_ClearAllLastByteOffset = 1022,
};

uint16 CreateSpriteAtPos(uint16 x_r18, uint16 y_r20, uint16 ilist_r22, uint16 pal_r24) {  // 0xB4BC26
  int v0 = kSpriteObject_LastByteOffset;
  while (sprite_instr_list_ptrs[v0 >> 1]) {
    v0 -= 2;
    if (sign16(v0))
      return kSpriteObject_NoSlot;
  }
  int v1 = v0 >> 1;
  sprite_palettes[v1] = 0;
  sprite_x_subpos[v1] = 0;
  sprite_y_subpos[v1] = 0;
  sprite_disable_flag[v1] = 0;
  sprite_x_pos[v1] = x_r18;
  sprite_y_pos[v1] = y_r20;
  sprite_palettes[v1] = pal_r24;
  uint16 v2 = kCreateSprite_Ilists[ilist_r22];
  sprite_instr_list_ptrs[v1] = v2;
  sprite_instr_timer[v1] = *(uint16 *)RomPtr_B4(v2);
  return v0;
}

void CallSpriteObjectInstr(uint32 ea) {
  switch (ea) {
  case fnSpriteObject_Instr_RepeatLast: SpriteObject_Instr_RepeatLast(); return;  // 0xb4bcf0
  case fnSpriteObject_Instr_Terminate: SpriteObject_Instr_Terminate(); return;  // 0xb4bd07
  case fnSpriteObject_Instr_Goto: SpriteObject_Instr_Goto(); return;  // 0xb4bd12
  default: Unreachable();
  }
}

void HandleSpriteObjects(void) {  // 0xB4BC82
  uint16 v1;

  if (debug_time_frozen_for_enemies | time_is_frozen_flag)
    return;

  sprite_object_index = kSpriteObject_LastByteOffset;
  do {
    int v0 = sprite_object_index >> 1;
    if (!sprite_instr_list_ptrs[v0] || (sprite_disable_flag[v0] & 1) != 0)
      continue;
    v1 = sprite_instr_timer[v0];
    if (sign16(v1)) {
BREAKLABEL:
      CallSpriteObjectInstr((kSpriteObject_RomBank << 16) | v1);
      continue;
    }
    sprite_instr_timer[v0] = v1 - 1;
    if (v1 == 1) {
      uint16 v3 = sprite_instr_list_ptrs[v0] + 4;
      sprite_instr_list_ptrs[v0] = v3;
      v1 = *(uint16 *)RomPtr_B4(v3);
      if (sign16(v1))
        goto BREAKLABEL;
      sprite_instr_timer[sprite_object_index >> 1] = v1;
    }
  } while (!sign16(sprite_object_index -= 2));
}

void SpriteObject_Instr_RepeatLast(void) {  // 0xB4BCF0
  int v2 = sprite_object_index >> 1;
  sprite_instr_list_ptrs[v2] -= 4;
  sprite_instr_timer[v2] = kSpriteObject_HoldForever;
}

void SpriteObject_Instr_Terminate(void) {  // 0xB4BD07
  sprite_instr_list_ptrs[sprite_object_index >> 1] = 0;
}

void SpriteObject_Instr_Goto(void) {  // 0xB4BD12
  uint16 v2 = GET_WORD(RomPtr_B4(sprite_instr_list_ptrs[sprite_object_index >> 1]) + 2);
  sprite_instr_list_ptrs[sprite_object_index >> 1] = v2;
  sprite_instr_timer[sprite_object_index >> 1] = GET_WORD(RomPtr_B4(v2));
}

void DrawSpriteObjects(void) {  // 0xB4BD32
  for (int i = kSpriteObject_LastByteOffset; i >= 0; i -= 2) {
    int v1 = i >> 1;
    if (!sprite_instr_list_ptrs[v1])
      continue;
    uint16 x = sprite_x_pos[v1] - layer1_x_pos;
    if ((int16)(x + kSpriteObject_CullMargin) < 0)
      continue;
    if (!sign16(x - kSpriteObject_CullRight))
      continue;
    int16 y = sprite_y_pos[v1] - layer1_y_pos;
    if (y < 0)
      continue;
    if (!sign16(y - kSpriteObject_CullBottom))
      continue;
    uint16 pal_flags = sprite_palettes[v1] & kSpriteObject_PaletteFlagsMask;
    uint16 pal_index = sprite_palettes[v1] & kSpriteObject_PaletteIndexMask;
    const uint8 *v3 = RomPtr_B4(sprite_instr_list_ptrs[v1]);
    DrawSpritemapWithBaseTile(kSpriteObject_RomBank, GET_WORD(v3 + 2), x, y, pal_flags, pal_index);
  }
}

void ClearSpriteObjects(void) {  // 0xB4BD97
  int v0 = kSpriteObject_ClearAllLastByteOffset;
  do {
    sprite_instr_list_ptrs[v0 >> 1] = 0;
    v0 -= 2;
  } while (v0);
}
