// Room state selection callbacks extracted from Bank $8F.
#include <stdio.h>
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"

static void DebugLogRoomStateSelection(const char *kind, uint16 arg, uint16 state_ptr) {
  if (!g_debug_flag)
    return;
  RoomDefRoomstate *rs = get_RoomDefRoomstate(state_ptr);
  fprintf(stderr,
          "ROOM state-select room=0x%04X area=%u kind=%s arg=0x%04X -> state=0x%04X music=%u/%u enemy_pop=0x%04X enemy_tiles=0x%04X boss=0x%02X event0=0x%02X\n",
          room_ptr, area_index, kind, arg, state_ptr, rs->music_track, rs->music_control,
          rs->enemy_population_ptr_, rs->enemy_tilesets_ptr, boss_bits_for_area[area_index], events_that_happened[0]);
}

uint16 RoomDefStateSelect_Finish(uint16 k) {  // 0x8FE5E6
  roomdefroomstate_ptr = k;
  DebugLogRoomStateSelection("finish", 0, k);
  return 0;
}

uint16 RoomDefStateSelect_TourianBoss01(uint16 k) {  // 0x8FE5FF
  if (!(CheckBossBitForCurArea(1) & 1))
    return k + 2;
  const uint16 *v1 = (const uint16 *)RomPtr_8F(k);
  DebugLogRoomStateSelection("tourian-boss", 1, *v1);
  return RoomDefStateSelect_Finish(*v1);
}

uint16 RoomDefStateSelect_IsEventSet(uint16 k) {  // 0x8FE612
  const uint8 *v1 = RomPtr_8F(k);
  if (CheckEventHappened(*v1)) {
    DebugLogRoomStateSelection("event", *v1, GET_WORD(v1 + 1));
    return RoomDefStateSelect_Finish(GET_WORD(v1 + 1));
  } else {
    return k + 3;
  }
}

uint16 RoomDefStateSelect_IsBossDead(uint16 k) {  // 0x8FE629
  const uint8 *v1 = RomPtr_8F(k);
  if (CheckBossBitForCurArea(*v1) & 1) {
    DebugLogRoomStateSelection("boss", *v1, GET_WORD(v1 + 1));
    return RoomDefStateSelect_Finish(GET_WORD(v1 + 1));
  } else {
    return k + 3;
  }
}

uint16 RoomDefStateSelect_MorphBallMissiles(uint16 k) {  // 0x8FE652
  if ((collected_items & 4) == 0 || !samus_max_missiles)
    return k + 2;
  const uint16 *v1 = (const uint16 *)RomPtr_8F(k);
  DebugLogRoomStateSelection("morph-missiles", 0, *v1);
  return RoomDefStateSelect_Finish(*v1);
}

uint16 RoomDefStateSelect_PowerBombs(uint16 k) {  // 0x8FE669
  if (!samus_max_power_bombs)
    return k + 2;
  const uint16 *v1 = (const uint16 *)RomPtr_8F(k);
  DebugLogRoomStateSelection("power-bombs", 0, *v1);
  return RoomDefStateSelect_Finish(*v1);
}

static uint16 CallRoomDefStateSelect(uint32 ea, uint16 k) {
  switch (ea) {
  case fnRoomDefStateSelect_Finish: return RoomDefStateSelect_Finish(k);
  case fnRoomDefStateSelect_TourianBoss01: return RoomDefStateSelect_TourianBoss01(k);
  case fnRoomDefStateSelect_IsEventSet: return RoomDefStateSelect_IsEventSet(k);
  case fnRoomDefStateSelect_IsBossDead: return RoomDefStateSelect_IsBossDead(k);
  case fnRoomDefStateSelect_MorphBallMissiles: return RoomDefStateSelect_MorphBallMissiles(k);
  case fnRoomDefStateSelect_PowerBombs: return RoomDefStateSelect_PowerBombs(k);
  default: return Unreachable();
  }
}

void HandleRoomDefStateSelect(uint16 k) {  // 0x8FE5D2
  uint16 v1 = k + 11;
  do {
    uint16 event_pointer = get_RoomDefStateSelect_E6E5_Finish(v1)->code_ptr;
    v1 = CallRoomDefStateSelect(event_pointer | 0x8F0000, v1 + 2);
  } while (v1);
}
