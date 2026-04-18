// Room FX loading helpers extracted from Bank $89.

#include "ida_types.h"
#include "variables.h"
#include "funcs.h"

#define kFxPaletteBlendColors ((uint16*)RomFixedPtr(0x89aa02))
#define kFxTypeTilemapPtrs ((uint16*)RomFixedPtr(0x83abf0))
#define kAreaPalFxListPointers ((uint16*)RomFixedPtr(0x83ac46))
#define kAreaAnimtilesListPtrs ((uint16*)RomFixedPtr(0x83ac56))

enum {
  kFxBlendPaletteIndex = 25,
  kFxBlendPaletteCount = 3,
};

static void FxTypeFunc_Null(void) {}

static Func_V *const kFxTypeFuncPtrs[23] = {  // 0x89AB82
  FxTypeFunc_Null,
  FxTypeFunc_2_Lava,
  FxTypeFunc_4_Acid,
  FxTypeFunc_6_Water,
  FxTypeFunc_8_Spores,
  FxTypeFunc_A_Rain,
  FxTypeFunc_C,
  FxTypeFunc_Null,
  FxTypeFunc_Null,
  FxTypeFunc_Null,
  FxTypeFunc_Null,
  FxTypeFunc_Null,
  FxTypeFunc_Null,
  FxTypeFunc_Null,
  FxTypeFunc_Null,
  FxTypeFunc_Null,
  FxTypeFunc_20,
  FxTypeFunc_22_ScrollingSky,
  FxTypeFunc_24,
  FxTypeFunc_26_TourianEntranceStatue,
  FxTypeFunc_28_CeresRidley,
  FxTypeFunc_CeresElevator,
  FxTypeFunc_2C_Haze,
};

static void LoadFxBaseState(const FxDef *fx) {
  fx_base_y_pos = fx->base_y_pos;
  fx_target_y_pos = fx->target_y_pos;
  fx_y_vel = fx->y_vel;
  fx_timer = fx->timer;
  fx_layer_blending_config_a = fx->default_layer_blend;
  fx_layer_blending_config_b = fx->layer3_layer_blend;
  fx_liquid_options = fx->fx_liquid_options_;
}

static void LoadFxBlendPalette(const FxDef *fx, uint16 *palette_target) {
  if (fx->palette_blend) {
    uint16 palette_index = fx->palette_blend >> 1;
    for (int i = 0; i < kFxBlendPaletteCount; i++)
      palette_target[kFxBlendPaletteIndex + i] = kFxPaletteBlendColors[palette_index + i];
  } else {
    palette_target[kFxBlendPaletteIndex + 2] = 0;
  }
}

static void SpawnAreaPaletteFxObjects(uint8 bitset) {
  const uint16 *area_palettes = (const uint16 *)RomPtr_83(kAreaPalFxListPointers[area_index]);
  for (int i = 0; i < 8; i++, bitset >>= 1) {
    if (bitset & 1)
      SpawnPalfxObject(area_palettes[i]);
  }
}

static void SpawnAreaAnimtiles(uint8 bitset) {
  const uint16 *area_animtiles = (const uint16 *)RomPtr_83(kAreaAnimtilesListPtrs[area_index]);
  for (int i = 0; i < 8; i++, bitset >>= 1) {
    if (bitset & 1)
      SpawnAnimtiles(area_animtiles[i]);
  }
}

static FxDef *FindFxDefForCurrentDoor(void) {
  uint16 entry_ptr = room_layer3_asm_ptr;
  if (!entry_ptr)
    return NULL;

  for (;; entry_ptr += sizeof(FxDef)) {
    FxDef *fx = get_FxDef(entry_ptr);
    if (!fx->door_ptr || fx->door_ptr == door_def_ptr)
      return fx;
    if (fx->door_ptr == 0xFFFF)
      return NULL;
  }
}

void LoadFxEntry(uint16 index) {  // 0x89AB02
  FxDef *fx = get_FxDef(room_layer3_asm_ptr + sizeof(FxDef) * (index & 7));
  LoadFxBaseState(fx);
  LoadFxBlendPalette(fx, palette_buffer);
}

void LoadFXHeader(void) {
  FxDef *fx = FindFxDefForCurrentDoor();
  if (!fx)
    return;

  LoadFxBaseState(fx);
  LoadFxBlendPalette(fx, target_palettes);

  fx_type = fx->type;
  if (fx->type) {
    fx_tilemap_ptr = kFxTypeTilemapPtrs[fx->type >> 1];
    kFxTypeFuncPtrs[fx->type >> 1]();
  }
  if (fx->palette_fx_bitset)
    SpawnAreaPaletteFxObjects(fx->palette_fx_bitset);
  if (fx->animtiles_bitset)
    SpawnAreaAnimtiles(fx->animtiles_bitset);
}
