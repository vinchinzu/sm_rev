// Room setup callbacks extracted from Bank $8F.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"

enum {
  kRoomMainAsmVar0 = 0,
  kRoomMainAsmVar2 = 2,
  kEarthquake_LightHorizontal = 18,
  kEarthquake_MediumHorizontal = 21,
};

static inline void WriteRoomMainAsmWord(uint16 offset, uint16 value) {
  *(uint16 *)(room_main_asm_variables + offset) = value;
}

static inline void StartIndefiniteEarthquake(uint16 type) {
  earthquake_type = type;
  earthquake_timer = -1;
}

static inline void InitializeRoomMainEarthquake(uint16 type) {
  earthquake_type = type;
  WriteRoomMainAsmWord(kRoomMainAsmVar2, type);
  WriteRoomMainAsmWord(kRoomMainAsmVar0, 0);
  earthquake_timer = -1;
}

static void RoomSetup_AfterSavingAnimals(void) {  // 0x8F9194
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x3d, 0x0b, 0xbb30 });
  StartIndefiniteEarthquake(24);
}

static void RoomSetup_AutoDestroyWallAfterEscape(void) {  // 0x8F91A9
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x10, 0x87, 0xb964 });
}

static void RoomSetup_TurnWallIntoShotblocks(void) {  // 0x8F91B2
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x0f, 0x0a, 0xb9ed });
}

static void RoomSetup_ShakeDuringEscape(void) {  // 0x8F91BD
  StartIndefiniteEarthquake(6);
  FxTypeFunc_20();
}

static void RoomSetup_ScrollingSkyLand(void) {  // 0x8F91C9
  FxTypeFunc_20();
}

static void RoomSetup_ScrollingSkyOcean(void) {  // 0x8F91CE
  RoomSetupAsm_ScrollingSkyOcean();
}

static void RoomSetup_RunStatueUnlockAnims(void) {  // 0x8F91D7
  SpawnAnimtiles(addr_kAnimtiles_TourianStatue_Kraid);
  SpawnAnimtiles(addr_kAnimtiles_TourianStatue_Phantoon);
  SpawnAnimtiles(addr_kAnimtiles_TourianStatue_Draygon);
  SpawnAnimtiles(addr_kAnimtiles_TourianStatue_Ridley);
}

static void RoomSetup_PrePhantoonRoom(void) {  // 0x8FC8C8
  SpawnEprojWithRoomGfx(addr_kEproj_PrePhantomRoom, 0);
}

static void RoomSetup_ShaktoolRoomPlm(void) {  // 0x8FC8D3
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x00, 0x00, 0xb8eb });
}

static void RoomSetup_DraygonPauseHooks(void) {  // 0x8FC8DD
  mov24(&pause_hook, fnPauseHook_DraygonRoom);
  mov24(&unpause_hook, fnUnpauseHook_DraygonRoom);
}

void PauseHook_DraygonRoom(void) {  // 0x8FC8F6
  irqhandler_next_handler = 4;
}

CoroutineRet UnpauseHook_DraygonRoom(void) {  // 0x8FC8FC
  if (hdma_object_channels_bitmask[1] == 8)
    irqhandler_next_handler = 12;
  return kCoroutineNone;
}

static void RoomSetup_SetCollectedMap(void) {  // 0x8FC90A
  uint16 v0 = *(uint16 *)&map_station_byte_array[area_index] | 1;
  *(uint16 *)&map_station_byte_array[area_index] = v0;
  has_area_map = v0;
}

static void RoomSetup_SetZebesTimebombEvent(void) {  // 0x8FC91F
  SetEventHappened(0xE);
  StartIndefiniteEarthquake(kEarthquake_LightHorizontal);
}

static void RoomSetup_LightHorizRoomShake(void) {  // 0x8FC933
  InitializeRoomMainEarthquake(kEarthquake_LightHorizontal);
}

static void RoomSetup_MediumHorizRoomShake(void) {  // 0x8FC946
  StartIndefiniteEarthquake(kEarthquake_MediumHorizontal);
}

static void RoomSetup_Escape4MediumHorizRoomShake(void) {  // 0x8FC953
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x10, 0x10, 0xb968 });
  InitializeRoomMainEarthquake(kEarthquake_MediumHorizontal);
}

static void RoomSetup_CeresDoorSolid(void) {  // 0x8FC96E
  SpawnHardcodedPlm((SpawnHardcodedPlmArgs) { 0x0f, 0x26, 0xba48 });
  FxTypeFunc_2C_Haze();
}

static void RoomSetup_CeresColorMathHdma(void) {  // 0x8FC976
  FxTypeFunc_2C_Haze();
}

static void RoomSetup_CeresColorMathHdmaBgBase(void) {  // 0x8FC97B
  reg_BG12NBA = 102;
  FxTypeFunc_2C_Haze();
  hdma_data_table_in_ceres = 9;
}

static void CallRoomSetupCode(uint32 ea) {
  switch (ea) {
  case fnRoomSetup_AfterSavingAnimals: RoomSetup_AfterSavingAnimals(); return;
  case fnRoomSetup_AutoDestroyWallAfterEscape: RoomSetup_AutoDestroyWallAfterEscape(); return;
  case fnRoomSetup_TurnWallIntoShotblocks: RoomSetup_TurnWallIntoShotblocks(); return;
  case fnRoomSetup_ShakeDuringEscape: RoomSetup_ShakeDuringEscape(); return;
  case fnRoomSetup_ScrollingSkyLand: RoomSetup_ScrollingSkyLand(); return;
  case fnRoomSetup_ScrollingSkyOcean: RoomSetup_ScrollingSkyOcean(); return;
  case fnRoomSetup_RunStatueUnlockAnims: RoomSetup_RunStatueUnlockAnims(); return;
  case fnRoomCode_8FC8C8: RoomSetup_PrePhantoonRoom(); return;
  case fnRoomCode_SetupShaktoolRoomPlm: RoomSetup_ShaktoolRoomPlm(); return;
  case fnRoomCode_SetPauseCodeForDraygon: RoomSetup_DraygonPauseHooks(); return;
  case fnRoomCode_SetCollectedMap: RoomSetup_SetCollectedMap(); return;
  case fnnullsub_132:
  case fnnullsub_133:
  case fnnullsub_134:
  case fnnullsub_135:
  case fnnullsub_136:
  case fnnullsub_137:
  case fnnullsub_138:
  case fnnullsub_139:
  case fnnullsub_140:
  case fnnullsub_141:
  case fnnullsub_142:
  case fnnullsub_143:
  case fnnullsub_144:
  case fnnullsub_145:
  case fnnullsub_146:
  case fnnullsub_147: return;
  case fnRoomCode_SetZebesTimebombEvent: RoomSetup_SetZebesTimebombEvent(); return;
  case fnRoomCode_SetLightHorizRoomShake: RoomSetup_LightHorizRoomShake(); return;
  case fnRoomCode_SetMediumHorizRoomShake: RoomSetup_MediumHorizRoomShake(); return;
  case fnRoomCode_Escape4_SetMediumHorizRoomShake: RoomSetup_Escape4MediumHorizRoomShake(); return;
  case fnRoomCode_SetCeresDoorSolid: RoomSetup_CeresDoorSolid(); return;
  case fnRoomCode_CeresColorMathHdma: RoomSetup_CeresColorMathHdma(); return;
  case fnRoomCode_CeresColorMathHdma_BgBase: RoomSetup_CeresColorMathHdmaBgBase(); return;
  default: Unreachable();
  }
}

void RunRoomSetupCode(void) {  // 0x8FE88F
  RoomDefRoomstate *room_state = get_RoomDefRoomstate(roomdefroomstate_ptr);
  if (room_state->room_setup_code)
    CallRoomSetupCode(room_state->room_setup_code | 0x8F0000);
}
