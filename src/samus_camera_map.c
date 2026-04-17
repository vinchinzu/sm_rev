// Samus camera/minimap helpers: scrolling, camera follow, and HUD minimap
// updates that used to live in the main Samus bank file.

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"

#define kPauseMenuMapData ((uint16*)RomFixedPtr(0x829717))
#define kPauseMenuMapTilemaps ((LongPtr*)RomFixedPtr(0x82964a))

static const uint8 kShr0x80[8] = { 0x80, 0x40, 0x20, 0x10, 8, 4, 2, 1 };
static const uint16 kShr0xFc00[8] = { 0xfc00, 0x7e00, 0x3f00, 0x1f80, 0xfc0, 0x7e0, 0x3f0, 0x1f8 };

static void CallScrollingFinishedHook(uint32 ea) {
  switch (ea) {
  case fnSamus_ScrollFinishedHook_SporeSpawnFight: Samus_ScrollFinishedHook_SporeSpawnFight(); return;
  default: Unreachable();
  }
}

static void MarkMapTileAsExplored(uint16 r18, uint16 r24) {  // 0x90A8A6
  uint8 v0 = (uint16)(r18 & 0xFF00) >> 8;
  uint16 R34 = (room_x_coordinate_on_map + v0) & 0x20;
  uint16 r20 = ((room_x_coordinate_on_map + v0) & 0x1F) >> 3;
  uint16 R22 = room_y_coordinate_on_map + ((r24 & 0xFF00) >> 8) + 1;
  map_tiles_explored[(uint16)(r20 + 4 * (R34 + R22))] |= kShr0x80[(room_x_coordinate_on_map + v0) & 7];
}

void MainScrollingRoutine(void) {  // 0x9094EC
  if (slow_grabble_scrolling_flag) {
    if ((samus_x_pos & 0x8000) != 0)
      goto LABEL_14;
    uint16 v0;
    v0 = samus_x_pos - layer1_x_pos;
    if (samus_x_pos < layer1_x_pos)
      goto LABEL_7;
    if (v0 >= 0xA0) {
      layer1_x_pos += 3;
      goto LABEL_8;
    }
    if (v0 < 0x60)
      LABEL_7:
    layer1_x_pos -= 3;
LABEL_8:
    if ((samus_y_pos & 0x8000) == 0) {
      uint16 v1 = samus_y_pos - layer1_y_pos;
      if (samus_y_pos >= layer1_y_pos) {
        if (v1 >= 0x90) {
          layer1_y_pos += 3;
          goto LABEL_14;
        }
        if (v1 >= 0x70)
          goto LABEL_14;
      }
      layer1_y_pos -= 3;
    }
LABEL_14:
    HandleAutoscrolling_X();
    HandleAutoscrolling_Y();
    goto LABEL_16;
  }
  Samus_CalcDistanceMoved_X();
  Samus_HandleScroll_X();
  Samus_CalcDistanceMoved_Y();
  Samus_HandleScroll_Y();
LABEL_16:
  if (scrolling_finished_hook)
    CallScrollingFinishedHook(scrolling_finished_hook | 0x900000);
  samus_prev_x_pos = samus_x_pos;
  samus_prev_x_subpos = samus_x_subpos;
  samus_prev_y_pos = samus_y_pos;
  samus_prev_y_subpos = samus_y_subpos;
}

void Samus_ScrollFinishedHook_SporeSpawnFight(void) {  // 0x909589
  if (layer1_y_pos <= 0x1D0)
    layer1_y_pos = 464;
}

void Samus_HandleScroll_X(void) {  // 0x9095A0
  static const uint16 kSamus_HandleScroll_X_FaceLeft[4] = { 0xa0, 0x50, 0x20, 0xe0 };
  static const uint16 kSamus_HandleScroll_X_FaceRight[4] = { 0x60, 0x40, 0x20, 0xe0 };
  if (samus_prev_x_pos == samus_x_pos) {
    HandleAutoscrolling_X();
    return;
  }
  if ((knockback_dir || samus_movement_type == 16 || samus_x_accel_mode == 1) ^ (samus_pose_x_dir != 4)) {
    ideal_layer1_xpos = samus_x_pos - kSamus_HandleScroll_X_FaceRight[camera_distance_index >> 1];
  } else {
    ideal_layer1_xpos = samus_x_pos - kSamus_HandleScroll_X_FaceLeft[camera_distance_index >> 1];
  }
  if (ideal_layer1_xpos != layer1_x_pos) {
    if ((int16)(ideal_layer1_xpos - layer1_x_pos) < 0) {
      AddToHiLo(&layer1_x_pos, &layer1_x_subpos, -IPAIR32(absolute_moved_last_frame_x, absolute_moved_last_frame_x_fract));
      HandleScrollingWhenTriggeringScrollLeft();
    } else {
      AddToHiLo(&layer1_x_pos, &layer1_x_subpos, __PAIR32__(absolute_moved_last_frame_x, absolute_moved_last_frame_x_fract));
      HandleScrollingWhenTriggeringScrollRight();
    }
  }
}

void Samus_HandleScroll_Y(void) {  // 0x90964F
  if (samus_prev_y_pos == samus_y_pos) {
    HandleAutoscrolling_Y();
  } else {
    if (samus_y_dir == 1)
      ideal_layer1_ypos = samus_y_pos - down_scroller;
    else
      ideal_layer1_ypos = samus_y_pos - up_scroller;
    if (ideal_layer1_ypos != layer1_y_pos) {
      if ((int16)(ideal_layer1_ypos - layer1_y_pos) < 0) {
        AddToHiLo(&layer1_y_pos, &layer1_y_subpos, -IPAIR32(absolute_moved_last_frame_y, absolute_moved_last_frame_y_fract));
        HandleScrollingWhenTriggeringScrollUp();
      } else {
        AddToHiLo(&layer1_y_pos, &layer1_y_subpos, __PAIR32__(absolute_moved_last_frame_y, absolute_moved_last_frame_y_fract));
        HandleScrollingWhenTriggeringScrollDown();
      }
    }
  }
}

void Samus_CalcDistanceMoved_X(void) {  // 0x9096C0
  if ((int16)(samus_x_pos - samus_prev_x_pos) >= 0) {
    SetHiLo(&absolute_moved_last_frame_x, &absolute_moved_last_frame_x_fract,
      __PAIR32__(samus_x_pos, samus_x_subpos) - __PAIR32__(samus_prev_x_pos, samus_prev_x_subpos) + (1 << 16));
  } else {
    SetHiLo(&absolute_moved_last_frame_x, &absolute_moved_last_frame_x_fract,
      __PAIR32__(samus_prev_x_pos, samus_prev_x_subpos) - __PAIR32__(samus_x_pos, samus_x_subpos) + (1 << 16));
  }
}

void Samus_CalcDistanceMoved_Y(void) {  // 0x9096FF
  if ((int16)(samus_y_pos - samus_prev_y_pos) >= 0) {
    SetHiLo(&absolute_moved_last_frame_y, &absolute_moved_last_frame_y_fract,
      __PAIR32__(samus_y_pos, samus_y_subpos) - __PAIR32__(samus_prev_y_pos, samus_prev_y_subpos) + (1 << 16));
  } else {
    SetHiLo(&absolute_moved_last_frame_y, &absolute_moved_last_frame_y_fract,
      __PAIR32__(samus_prev_y_pos, samus_prev_y_subpos) - __PAIR32__(samus_y_pos, samus_y_subpos) + (1 << 16));
  }
}

void DisableMinimapAndMarkBossRoomAsExplored(void) {  // 0x90A7E2
  debug_disable_minimap = 1;
  uint16 v0 = 0;
  do {
    int v1 = v0 >> 1;
    hud_tilemap[v1 + 26] = 11295;
    hud_tilemap[v1 + 58] = 11295;
    hud_tilemap[v1 + 90] = 11295;
    v0 += 2;
  } while ((int16)(v0 - 10) < 0);
  uint16 v2 = 5;
  while (boss_id != g_stru_90A83A[v2].boss_id_) {
    if ((--v2 & 0x8000) != 0)
      return;
  }
  for (int i = g_stru_90A83A[v2].ptrs; ; i += 4) {
    const uint16 *v4 = (const uint16 *)RomPtr_90(i);
    if ((*v4 & 0x8000) != 0)
      break;
    MarkMapTileAsExplored(v4[0], v4[1]);
  }
}

void InitializeMiniMapBroken(void) {  // 0x90A8EF
  //SetR18_R20(room_x_coordinate_on_map + ((samus_x_pos & 0xFF00) >> 8), r18 >> 3);
  //R22_ = room_y_coordinate_on_map + ((samus_y_pos & 0xFF00) >> 8) + 1;
  //UpdateMinimapInside();
}

void UpdateMinimap(void) {  // 0x90A91B
  int16 v4;
  int16 v10;

  if (debug_disable_minimap || (samus_x_pos >> 4) >= room_width_in_blocks ||
      (samus_y_pos >> 4) >= room_height_in_blocks)
    return;
  uint16 r46 = 0;
  uint8 v0 = (uint16)(samus_x_pos & 0xFF00) >> 8;
  uint16 r34 = (room_x_coordinate_on_map + v0) & 0x20;
  uint16 r18 = (room_x_coordinate_on_map + v0) & 0x1F;
  uint16 v1 = (room_x_coordinate_on_map + v0) & 7;
  uint16 r20 = ((room_x_coordinate_on_map + v0) & 0x1F) >> 3;
  uint16 r22 = room_y_coordinate_on_map + ((samus_y_pos & 0xFF00) >> 8) + 1;
  uint16 v2 = r20 + 4 * (r34 + r22);
  map_tiles_explored[v2] |= kShr0x80[(room_x_coordinate_on_map + v0) & 7];
  uint16 r32 = v1;
  uint16 r30 = v2;
  uint16 v3 = v2 - 4;
  v4 = v1 - 2;
  if ((int16)(v1 - 2) < 0) {
    v4 &= 7;
    --v3;
    ++r46;
  }
  uint16 R50 = v3;
  int r52 = 2 * v4;
  int v6 = r52 >> 1;
  uint16 r24 = kShr0xFc00[v6] & swap16(*(uint16 *)&map_tiles_explored[v3]);
  uint16 r26 = kShr0xFc00[v6] & swap16(*(uint16 *)&map_tiles_explored[v3 + 4]);
  uint16 r28 = kShr0xFc00[v6] & swap16(*(uint16 *)&map_tiles_explored[v3 + 8]);
  const uint8 *r9 = RomPtr_82(kPauseMenuMapData[area_index]);
  const uint8 *r15 = r9;
  r9 += v3;
  uint16 r38 = swap16(GET_WORD(r9));
  r9 += 4;
  uint16 r40 = swap16(GET_WORD(r9));
  r9 += 4;
  uint16 r42 = swap16(GET_WORD(r9));
  if ((R50 & 3) == 3) {
    v10 = r46 ? r52 >> 1 : r32;
    if (!sign16(v10 - 6)) {
      uint8 R48 = r34 ? (R50 - 124) : (R50 + 125);

      uint16 v0 = (uint8)R48;
      uint16 r44 = 0;
      r9 = r15 + (uint8)R48;
      LOBYTE(r44) = map_tiles_explored[(uint8)R48];
      HIBYTE(r44) = *r9;
      if ((uint8)r34 == 32) {
        HIBYTE(r38) = HIBYTE(r44);
        HIBYTE(r24) = r44;
      } else {
        LOBYTE(r38) = HIBYTE(r44);
        LOBYTE(r24) = r44;
      }
      LOBYTE(r44) = map_tiles_explored[v0 + 4];
      r9 += 4;
      HIBYTE(r44) = *r9;
      if ((uint8)r34 == 32) {
        HIBYTE(r40) = HIBYTE(r44);
        HIBYTE(r26) = r44;
      } else {
        LOBYTE(r40) = HIBYTE(r44);
        LOBYTE(r26) = r44;
      }
      LOBYTE(r44) = map_tiles_explored[v0 + 8];
      r9 += 4;
      HIBYTE(r44) = *r9;
      if ((uint8)r34 == 32) {
        HIBYTE(r42) = HIBYTE(r44);
        HIBYTE(r28) = r44;
      } else {
        LOBYTE(r42) = HIBYTE(r44);
        LOBYTE(r28) = r44;
      }
    }
  }
  for (int n = r52 >> 1; n; n--) {
    r24 *= 2;
    r38 *= 2;
    r26 *= 2;
    r40 *= 2;
    r28 *= 2;
    r42 *= 2;
  }
  UpdateMinimapInside(r18, r22, r34, r30, r32, r38, r24, r40, r26, r42, r28);
}

void UpdateMinimapInside(uint16 r18, uint16 r22, uint16 r34, uint16 r30, uint16 r32,
                         uint16 r38, uint16 r24, uint16 r40,
                         uint16 r26, uint16 r42, uint16 r28) {  // 0x90AA43
  uint16 v0;
  int16 v1;
  int16 v5;
  int16 v7;
  int16 v8;

  LOBYTE(v0) = (uint16)(r34 + r22) >> 8;
  HIBYTE(v0) = r34 + r22;
  uint16 t = r18 + (v0 >> 3);
  if (r34 && sign16((t & 0x1F) - 2))
    v1 = t - 1026;
  else
    v1 = t - 34;
  uint16 v2 = 2 * v1;
  const uint16 *r0 = (const uint16 *)RomPtr(Load24(&kPauseMenuMapTilemaps[area_index]));
  const uint16 *r3 = r0 + 32;
  const uint16 *r6 = r0 + 64;
  int n = 5;
  uint16 v3 = 0;
  do {
    bool v4 = r38 >> 15;
    r38 *= 2;
    if (!v4 || (v5 = r0[v2 >> 1], !has_area_map))
      v5 = 31;
    int v6 = v3 >> 1;
    hud_tilemap[v6 + 26] = v5 & 0xC3FF | 0x2C00;

    v4 = r24 >> 15;
    r24 *= 2;
    if (v4)
      hud_tilemap[v6 + 26] = r0[v2 >> 1] & 0xC3FF | 0x2800;

    v4 = r40 >> 15;
    r40 *= 2;
    if (!v4 || (v7 = r3[v2 >> 1], !has_area_map))
      v7 = 31;
    hud_tilemap[v6 + 58] = v7 & 0xC3FF | 0x2C00;

    v4 = r26 >> 15;
    r26 *= 2;
    if (v4) {
      hud_tilemap[v6 + 58] = r3[v2 >> 1] & 0xC3FF | 0x2800;
      if (n == 3 && (hud_tilemap[v6 + 58] & 0x1FF) == 40) {
        *((uint8 *)&music_data_index + r30) |= kShr0x80[r32];
      }
    }

    v4 = r42 >> 15;
    r42 *= 2;
    if (!v4 || (v8 = r6[v2 >> 1], !has_area_map))
      v8 = 31;
    hud_tilemap[v6 + 90] = v8 & 0xC3FF | 0x2C00;

    v4 = r28 >> 15;
    r28 *= 2;
    if (v4)
      hud_tilemap[v6 + 90] = r6[v2 >> 1] & 0xC3FF | 0x2800;

    v3 += 2;
    v2 += 2;
    if ((v2 & 0x3F) == 0)
      v2 += 1984;
  } while (--n);
  if ((nmi_frame_counter_byte & 8) == 0)
    hud_tilemap[60] |= 0x1C00;
}

void SetContactDamageIndexAndUpdateMinimap(void) {  // 0x90E8DC
  samus_contact_damage_index = 0;
  UpdateMinimap();
}
