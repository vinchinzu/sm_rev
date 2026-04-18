#ifndef SM_82_DATA_H
#define SM_82_DATA_H

#include "types.h"
#include "ida_types.h"

#define kInitialPalette ((uint16*)RomFixedPtr(0x9a8000))
#define kCommonSpritesPalette1 ((uint16*)RomFixedPtr(0x9afc00))

// ROM Pointer Macros (extracted from recovered sm_82.c)
#define kDemoRoomData ((uint16*)RomFixedPtr(0x82876c))
#define kPauseScreenSpriteAnimationData_0 (*(PauseScreenSpriteAnimationData*)RomFixedPtr(0x82c0b2))
#define kPauseScreenSpriteAnimationData_1 (*(PauseScreenSpriteAnimationData*)RomFixedPtr(0x82c0c4))
#define kPauseScreenSpriteAnimationData_2 (*(PauseScreenSpriteAnimationData*)RomFixedPtr(0x82c0d6))
#define kPauseScreenSpriteAnimationData_3 (*(PauseScreenSpriteAnimationData*)RomFixedPtr(0x82c0e8))
#define kPAuseSpritePaletteIndexValues ((uint16*)RomFixedPtr(0x82c0fa))
#define kPausePtsToAnimationSpritemapBaseIds ((uint16*)RomFixedPtr(0x82c1e4))
#define kPauseScreenPalettes ((uint16*)RomFixedPtr(0xb6f000))
#define kPauseLrHighlightAnimData ((uint16*)RomFixedPtr(0x82c10c))
#define kPauseAreaLabelTilemap ((uint16*)RomFixedPtr(0x82965f))
#define kPauseMenuMapTilemaps ((LongPtr*)RomFixedPtr(0x82964a))
#define kPauseMenuMapData ((uint16*)RomFixedPtr(0x829717))
#define kEquipmentTilemaps_Tanks ((uint16*)RomFixedPtr(0x82c088))
#define kEquipmentTilemaps_Weapons ((uint16*)RomFixedPtr(0x82c08c))
#define kEquipmentTilemaps_Suits ((uint16*)RomFixedPtr(0x82c096))
#define kEquipmentTilemaps_Boots ((uint16*)RomFixedPtr(0x82c0a2))
#define kHyperBeamWeaponsTilemaps ((uint16*)RomFixedPtr(0x82c0a8))
#define kEquipmentBitmasks_Weapons ((uint16*)RomFixedPtr(0x82c04c))
#define kEquipmentBitmasks_Suits ((uint16*)RomFixedPtr(0x82c056))
#define kEquipmentBitmasks_Boots ((uint16*)RomFixedPtr(0x82c062))
#define kPauseAnimatedPalette ((uint16*)RomFixedPtr(0x82a987))
#define kPauseReserveTankAnimationData ((uint16*)RomFixedPtr(0x82c165))
#define kEquipmentTilemapOffs_Tanks ((uint16*)RomFixedPtr(0x82c068))
#define kEquipmentTilemapOffs_Weapons ((uint16*)RomFixedPtr(0x82c06c))
#define kEquipmentTilemapOffs_Suits ((uint16*)RomFixedPtr(0x82c076))
#define kEquipmentTilemapOffs_Boots ((uint16*)RomFixedPtr(0x82c082))
extern const uint16 kEquipmentScreenWireframeCmp[3];
extern const uint16 kEquipmentScreenWireframePtrs[3];
#define kEquipmentScreenPtrsToItemXYpos ((uint16*)RomFixedPtr(0x82c18e))
#define kEquipmentPtrsToRamTilemapOffsets ((uint16*)RomFixedPtr(0x82c02c))
#define kEquipmentPtrsToBitmasks ((uint16*)RomFixedPtr(0x82c034))
#define kEquipmentPtrsToBitsets ((uint16*)RomFixedPtr(0x82c03c))
#define kEquipmentPtrsToEquipmentTilemaps ((uint16*)RomFixedPtr(0x82c044))
#define kMapIconDataPointers ((MapIconDataPointers*)RomFixedPtr(0x82c7cb))
#define g_stru_82B9A0 ((MapScrollArrowData*)RomFixedPtr(0x82b9a0))
#define g_stru_82B9AA ((MapScrollArrowData*)RomFixedPtr(0x82b9aa))
#define g_stru_82B9B4 ((MapScrollArrowData*)RomFixedPtr(0x82b9b4))
#define g_stru_82B9BE (*(MapScrollArrowData*)RomFixedPtr(0x82b9be))
#define file_copy_arrow_stuff ((FileCopyArrowStuff*)RomFixedPtr(0x82bb0c))
#define kMapElevatorDests ((uint16*)RomFixedPtr(0x82c74d))
#define kOptionsMenuSpecialPtrs ((uint16*)RomFixedPtr(0x82f0ae))
#define off_82F2ED ((uint16*)RomFixedPtr(0x82f2ed))
#define off_82F54A ((uint16*)RomFixedPtr(0x82f54a))
#define g_word_82F639 ((uint16*)RomFixedPtr(0x82f639))
#define g_off_82F647 ((uint16*)RomFixedPtr(0x82f647))
#define kMenuPalettes ((uint16*)RomFixedPtr(0x8ee400))
#define kStateHeaderTileSets ((uint16*)RomFixedPtr(0x8fe7a7))

// Constants and addresses (offsets)
#define addr_kEquipmentScreenTilemap_Blank 0xb2d4
#define addr_kEquipmentScreenTilemap_AUTO 0xb2cb
#define addr_kEquipmentScreenTilemap_MANUAL 0xb2c2
#define addr_kDoorClosingPlmIds 0xe68a
#define addr_kPauseMenuMapData 0x9717
#define addr_stru_82B9A0 0xb9a0
#define addr_stru_82B9AA 0xb9aa
#define addr_stru_82B9B4 0xb9b4
#define addr_stru_82B9BE 0xb9be
#define addr_off_82F4B6 0xf4b6
#define addr_kMapIconDataPointers 0xc7cb
#define addr_kDummySamusWireframeTilemap 0xb24a

// Data defined in sm_82_data.c (shared/local data)
extern const uint16 kMap_Criteria_SavePoints[16];
extern const Buttons word_82F575[9];
extern const uint16 g_word_82F149[4];
extern const uint16 g_word_82F151[4];
extern const uint16 word_82F204[16];
extern const uint16 g_word_82F6AD[6];
extern const uint16 kDrawMenuSelectionMissile_Enable[4];
extern const uint16 kDrawMenuSelectionMissile_SpriteMap[4];

// Backup variables for pausing are defined as macros in variables.h
extern uint16 oam_next_ptr_backup;

// Helper functions (formerly in sm_82.c)
void ChangePaletteValues(uint16 *tilemap, uint16 palette_mask, uint16 count);

#endif // SM_82_DATA_H
