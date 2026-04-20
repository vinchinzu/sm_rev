// HDMA core runtime extracted from bank 88: layer blending, HDMA object
// lifecycle, and the generic HDMA instruction/pre-instruction dispatch.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"

void CallHdmaobjPreInstr(uint32 ea, uint16 k);
const uint8 *CallHdmaobjInstr(uint32 ea, uint16 k, const uint8 *j);

static uint16 nullsub_9(uint16 j) {  // 0x888074
  return j;
}

void InitializeLayerBlending(void) {  // 0x888075
  reg_W12SEL = 0;
  reg_W34SEL = 0;
  reg_WOBJSEL = 0;
  reg_TM = 19;
  reg_TS = 4;
  reg_TMW = 0;
  reg_TSW = 0;
  next_gameplay_CGWSEL = 2;
  next_gameplay_CGADSUB = 51;
}

static uint16 LayerBlendFunc_4_PhantoonIntro(uint16 j) {  // 0x888090
  reg_TM = 17;
  reg_TS = 4;
  return j;
}

static uint16 LayerBlendFunc_6(uint16 j) {  // 0x888099
  reg_TS = 20;
  next_gameplay_CGADSUB = 39;
  return j;
}

static uint16 LayerBlendFunc_8(uint16 j) {  // 0x8880A2
  reg_TS = 20;
  next_gameplay_CGADSUB = 34;
  return j;
}

static uint16 LayerBlendFunc_A(uint16 j) {  // 0x8880AB
  next_gameplay_CGADSUB = 50;
  return j;
}

static uint16 LayerBlendFunc_C(uint16 j) {  // 0x8880B0
  next_gameplay_CGWSEL = 0;
  next_gameplay_CGADSUB = -94;
  return j;
}

static uint16 nullsub_10(uint16 j) {  // 0x8880B7
  return j;
}

static uint16 LayerBlendFunc_10(uint16 j) {  // 0x8880B8
  reg_W34SEL = 2;
  reg_WOBJSEL = 32;
  reg_TSW = 4;
  return j;
}

static uint16 LayerBlendFunc_14(uint16 j) {  // 0x8880C5
  next_gameplay_CGADSUB = -77;
  return j;
}

static uint16 LayerBlendFunc_16(uint16 j) {  // 0x8880CA
  LOBYTE(j) = 4;
  reg_TM = 17;
  reg_TS = 6;
  next_gameplay_CGADSUB = -79;
  return j;
}

static uint16 LayerBlendFunc_1A(uint16 j) {  // 0x8880D9
  LOBYTE(j) = 4;
  reg_TM = 21;
  reg_TS = 2;
  next_gameplay_CGADSUB = 53;
  return j;
}

static uint16 LayerBlendFunc_1C(uint16 j) {  // 0x8880E8
  reg_TM = 21;
  reg_TS = 2;
  next_gameplay_CGADSUB = 85;
  return j;
}

static uint16 LayerBlendFunc_18(uint16 j) {  // 0x8880F5
  j = 2;
  if ((reg_NMITIMEN & 0x30) == 0x30) {
    next_gameplay_CGADSUB = 36;
    reg_TS = 19;
    reg_TM = 4;
  }
  return j;
}

static uint16 nullsub_11(uint16 j) {  // 0x88810C
  return j;
}

static uint16 LayerBlendFunc_26(uint16 j) {  // 0x88810D
  next_gameplay_CGADSUB = 119;
  return j;
}

static uint16 LayerBlendFunc_28(uint16 j) {  // 0x888112
  next_gameplay_CGWSEL = 0;
  next_gameplay_CGADSUB = -77;
  if ((fx_layer_blending_config_c & 0x8000) == 0) {
    reg_COLDATA[0] = 37;
    reg_COLDATA[1] = 64;
    reg_COLDATA[2] = 0x80;
  }
  return j;
}

static uint16 LayerBlendFunc_2A(uint16 j) {  // 0x88812A
  next_gameplay_CGWSEL = 0;
  next_gameplay_CGADSUB = -77;
  if ((fx_layer_blending_config_c & 0x8000) == 0) {
    reg_COLDATA[0] = 38;
    reg_COLDATA[1] = 66;
    reg_COLDATA[2] = 0x80;
  }
  return j;
}

static uint16 LayerBlendFunc_2C(uint16 j) {  // 0x888142
  next_gameplay_CGWSEL = 0;
  return j;
}

static uint16 LayerBlendFunc_2E(uint16 j) {  // 0x888145
  next_gameplay_CGADSUB = -77;
  return j;
}

static uint16 LayerBlendFunc_32(uint16 j) {  // 0x88814A
  reg_TS = 68;
  next_gameplay_CGADSUB = -78;
  return j;
}

static uint16 LayerBlendFunc_34(uint16 j) {  // 0x888153
  j = 6;
  return j;
}

static uint16 LayerBlendFunc_24(uint16 j) {  // 0x888156
  reg_W12SEL = 0;
  reg_W34SEL = 2;
  reg_WOBJSEL = 32;
  reg_TM = 19;
  reg_TS = 4;
  reg_TMW = 19;
  reg_TSW = 4;
  next_gameplay_CGWSEL = 16;
  next_gameplay_CGADSUB = 51;
  return j;
}

void HandleLayerBlendingXrayCanShowBlocks(void) {  // 0x88817B
  reg_W12SEL = -56;
  reg_W34SEL = 8;
  reg_WOBJSEL = 0x80;
  reg_TM = 19;
  reg_TS = 4;
  reg_TMW = 3;
  reg_TSW = 4;
  next_gameplay_CGWSEL = 34;
  next_gameplay_CGADSUB = next_gameplay_CGADSUB & 0x80 | 0x73;
}

void HandleLayerBlendingXrayCantShowBlocks(void) {  // 0x8881A4
  reg_W12SEL = 0;
  reg_W34SEL = 8;
  reg_WOBJSEL = 0x80;
  reg_TM = 19;
  reg_TS = 4;
  reg_TMW = 3;
  reg_TSW = 4;
  next_gameplay_CGWSEL = 34;
  next_gameplay_CGADSUB = next_gameplay_CGADSUB & 0x80 | 0x61;
  if (room_ptr == addr_kRoom_cefb)
    reg_TM = 17;
}

void HandleLayerBlendingXrayFirefleaRoom(void) {  // 0x8881DB
  reg_W12SEL = 0;
  reg_W34SEL = 8;
  reg_WOBJSEL = 0x80;
  reg_TM = 19;
  reg_TS = 0;
  reg_TMW = 3;
  reg_TSW = 4;
  next_gameplay_CGWSEL = 32;
  next_gameplay_CGADSUB = -77;
}

static void LayerBlendPowerBombFunc_0(void) {  // 0x888219
  reg_W12SEL = 0;
  reg_W34SEL = 8;
  reg_WOBJSEL = 0x80;
  next_gameplay_CGWSEL = 2;
  next_gameplay_CGADSUB = 55;
  reg_TMW = 0;
  reg_TSW = 4;
  reg_TM = 19;
  reg_TS = 4;
}

static void LayerBlendPowerBombFunc_4(void) {  // 0x88823E
  reg_W12SEL = 0x80;
  reg_W34SEL = 8;
  reg_WOBJSEL = 0x80;
  next_gameplay_CGWSEL = 2;
  next_gameplay_CGADSUB = 55;
  reg_TMW = 0;
  reg_TSW = 6;
  reg_TM = 17;
  reg_TS = 6;
}

static void LayerBlendPowerBombFunc_6(void) {  // 0x888263
  reg_W12SEL = 0;
  reg_W34SEL = 8;
  reg_WOBJSEL = 0x80;
  next_gameplay_CGWSEL = 2;
  next_gameplay_CGADSUB = 49;
  reg_TMW = 0;
  reg_TSW = 4;
  reg_TM = 19;
  reg_TS = 4;
}

static Func_Y_Y *const kLayerBlendFuncTable[27] = {
  nullsub_9,
  nullsub_9,
  LayerBlendFunc_4_PhantoonIntro,
  LayerBlendFunc_6,
  LayerBlendFunc_8,
  LayerBlendFunc_A,
  LayerBlendFunc_C,
  nullsub_10,
  LayerBlendFunc_10,
  LayerBlendFunc_10,
  LayerBlendFunc_14,
  LayerBlendFunc_16,
  LayerBlendFunc_18,
  LayerBlendFunc_1A,
  LayerBlendFunc_1C,
  LayerBlendFunc_18,
  nullsub_11,
  LayerBlendFunc_14,
  LayerBlendFunc_24,
  LayerBlendFunc_26,
  LayerBlendFunc_28,
  LayerBlendFunc_2A,
  LayerBlendFunc_2C,
  LayerBlendFunc_2E,
  LayerBlendFunc_18,
  LayerBlendFunc_32,
  LayerBlendFunc_34,
};

void LayerBlendingHandler(void) {  // 0x888000
  uint16 v0 = 0;
  uint8 v1 = fx_layer_blending_config_c;
  if ((uint8)fx_layer_blending_config_c) {
    InitializeLayerBlending();
    v0 = kLayerBlendFuncTable[v1 >> 1](v0);
  }
  if ((fx_layer_blending_config_c & 0x8000) == 0) {
    if ((fx_layer_blending_config_c & 0x4000) != 0) {
      HandleLayerBlendingXrayCanShowBlocks();
    } else if ((fx_layer_blending_config_c & 0x2000) != 0) {
      HandleLayerBlendingXrayCantShowBlocks();
    } else if ((fx_layer_blending_config_c & 0x1000) != 0) {
      HandleLayerBlendingXrayFirefleaRoom();
    }
  } else {
    HandleLayerBlendingPowerBomb(v0);
  }
}

static Func_V *const kLayerBlendPowerBombFuncs[4] = {  // 0x8881FE
  LayerBlendPowerBombFunc_0,
  LayerBlendPowerBombFunc_0,
  LayerBlendPowerBombFunc_4,
  LayerBlendPowerBombFunc_6,
};

void HandleLayerBlendingPowerBomb(uint16 j) {
  if (room_ptr == addr_kRoom_a66a)
    j = 6;
  kLayerBlendPowerBombFuncs[j >> 1]();
}

void EnableHdmaObjects(void) {  // 0x888288
  hdma_objects_enable_flag |= 0x8000;
}

void DisableHdmaObjects(void) {  // 0x888293
  hdma_objects_enable_flag &= ~0x8000;
}

void WaitUntilEndOfVblankAndClearHdma(void) {  // 0x88829E
// Patched away wait, it will wait until the very start of the next frame until it disabled hdma
// but we run the frames instantly so no need.
  WriteReg(MDMAEN, 0);
  WriteReg(HDMAEN, 0);
  sub_8882AC();
}

void sub_8882AC(void) {  // 0x8882AC
  reg_HDMAEN = 0;
  for (int i = 10; i >= 0; i -= 2)
    hdma_object_channels_bitmask[i >> 1] = 0;
}

void InitializeSpecialEffectsForNewRoom(void) {  // 0x8882C1
  earthquake_sfx_index = 0;
  earthquake_sfx_timer = 0;
  if (room_ptr == addr_kRoom_9804
      || room_ptr == addr_kRoom_96ba
      || room_ptr == addr_kRoom_b32e
      || room_ptr == addr_kRoom_b457
      || room_ptr == addr_kRoom_dd58
      || room_ptr == addr_kRoom_dede) {
    earthquake_sfx_timer = -1;
  }
  debug_disable_minimap = 0;
  for (int i = 32; i != 0x80; i += 16) {
    WriteReg((SnesRegs)(i + 17152), 0);
    WriteReg((SnesRegs)(i + 17153), 0x13);
    WriteReg((SnesRegs)(i + 17154), 0);
    WriteReg((SnesRegs)(i + 17155), 0);
    WriteReg((SnesRegs)(i + 17156), 0);
    WriteReg((SnesRegs)(i + 17157), 0);
    WriteReg((SnesRegs)(i + 17158), 0);
    WriteReg((SnesRegs)(i + 17160), 0);
    WriteReg((SnesRegs)(i + 17161), 0);
  }
  fx_y_subpos = 0;
  fx_y_pos = -1;
  lava_acid_y_subpos = 0;
  lava_acid_y_pos = -1;
  hud_bg3_xpos = 0;
  hud_bg3_ypos = 0;
  bg3_xpos = 0;
  bg3_ypos = 0;
  irq_enable_mode7 = 0;
  camera_distance_index = 0;
  tourian_entrance_statue_animstate = 0;
  tourian_entrance_statue_finished = 0;
  earthquake_timer = 0;
  phantom_related_layer_flag = 0;
  power_bomb_explosion_status = 0;
  power_bomb_flag = 0;
  power_bomb_explosion_radius = 0;
  fx_tilemap_ptr = 0;
  fx_type = 0;
  fx_base_y_subpos = 0x8000;
  fx_base_y_pos = 0;
  fx_target_y_pos = 0;
  fx_y_vel = 0;
  fx_liquid_options = 0;
  fx_timer = 0;
  tide_phase = 0;
  fx_y_suboffset = 0;
  fx_y_offset = 0;
  fx_layer_blending_config_a = 2;
  reg_BG3HOFS = 0;
  reg_BG3VOFS = 0;
  layer2_x_pos = 0;
  layer2_y_pos = 0;
  room_loading_irq_handler = 0;
  *(VoidP *)((uint8 *)&pause_hook.addr + 1) = -30720;
  *(VoidP *)((uint8 *)&unpause_hook.addr + 1) = -30720;
  pause_hook.addr = FUNC16(PauseHook_Empty);
  unpause_hook.addr = FUNC16(PauseHook_Empty);
  WriteReg(WMADDL, 0xF0);
  WriteReg(WMADDM, 0xFF);
  WriteReg(WMADDH, 1);
  reg_HDMAEN = 0;
  reg_COLDATA[0] = 32;
  reg_COLDATA[1] = 64;
  reg_COLDATA[2] = 0x80;
  reg_MOSAIC = 0;
  reg_TM = 19;
  reg_CGWSEL = 0;
  reg_CGADSUB = 0;
  reg_BG12NBA = 0;
  reg_BG34NBA = 4;
  reg_BG2SC = 73;
  reg_BG3SC = 90;
  gameplay_BG3SC = 90;
}

static uint16 SpawnHdmaObjectInner(uint16 k, uint16 *p, uint16 r18, uint16 r20, uint16 r24) {  // 0x888477
  int v2 = k >> 1;
  hdma_object_pre_instructions[v2] = FUNC16(nullsub_293);
  hdma_object_pre_instruction_bank[v2] = 136;
  hdma_object_instruction_list_pointers[v2] = p[1];
  hdma_object_instruction_timers[v2] = 1;
  hdma_object_timers[v2] = 0;
  hdma_object_A[v2] = 0;
  hdma_object_B[v2] = 0;
  hdma_object_C[v2] = 0;
  hdma_object_D[v2] = 0;
  hdma_object_channels_bitmask[v2] = swap16(r18);
  hdma_object_bank_slot[v2] = r24 | r20;
  WriteRegWord((SnesRegs)(r20 + DMAP0), *p);
  return k;
}

uint16 SpawnHdmaObject(uint8 db, const void *p) {  // 0x888435
  uint16 r18 = 1024, r20 = 32;
  uint16 v3 = 0;
  for (;;) {
    if (!hdma_object_channels_bitmask[v3 >> 1])
      return SpawnHdmaObjectInner(v3, (uint16 *)p, r18, r20, db << 8);
    bool v4 = r18 >> 15;
    r18 *= 2;
    if (v4)
      return -1;
    r20 += 16;
    v3 += 2;
    if (v3 == 12)
      return -1;
  }
}

void SpawnHdmaObjectToSlot0xA(uint8 db, const void *p) {  // 0x88840A
  SpawnHdmaObjectInner(0xa, (uint16 *)p, 0x8000, 0x70, db << 8);
}

void HdmaObjectHandler(void) {  // 0x8884B9
  int8 v1;

  HandleMusicQueue();
  if (!time_is_frozen_flag && (power_bomb_explosion_status & 0x4000) != 0) {
    power_bomb_explosion_status = 0x8000;
    static const SpawnHdmaObject_Args unk_8884D5 = { 0x40, 0x28, 0x8ace };
    static const SpawnHdmaObject_Args unk_8884DD = { 0x40, 0x29, 0x8b80 };
    SpawnHdmaObject(0x88, &unk_8884D5);
    SpawnHdmaObject(0x88, &unk_8884DD);
  }
  fx_layer_blending_config_c = fx_layer_blending_config_a;
  if ((hdma_objects_enable_flag & 0x8000) != 0) {
    reg_HDMAEN = 0;
    int i = 0;
    do {
      hdma_object_index = i;
      v1 = *((uint8 *)hdma_object_channels_bitmask + i);
      if (v1) {
        reg_HDMAEN |= v1;
        HdmaobjInstructionHandler(i);
        i = hdma_object_index;
      }
      i += 2;
    } while (i != 12);
    LayerBlendingHandler();
  }
}

void HdmaobjInstructionHandler(uint8 k) {  // 0x88851C
  int kh = k >> 1;
  CallHdmaobjPreInstr(hdma_object_pre_instruction_bank[kh] << 16 | hdma_object_pre_instructions[kh], k);
  if (hdma_object_instruction_timers[kh]-- == 1) {
    const uint8 *base = RomBankBase(*((uint8 *)hdma_object_bank_slot + k + 1));
    const uint8 *p = base + hdma_object_instruction_list_pointers[kh];
    while (GET_WORD(p) & 0x8000) {
      p = CallHdmaobjInstr(GET_WORD(p) | 0x880000, k, p + 2);
      if ((uintptr_t)p < 0x10000) {
        if (!p)
          return;
        p = base + (uintptr_t)p;
      }
    }
    hdma_object_instruction_timers[kh] = GET_WORD(p);
    hdma_object_table_pointers[kh] = GET_WORD(p + 2);
    hdma_object_instruction_list_pointers[kh] = p + 4 - base;
  }
}

const uint8 *HdmaobjInstr_Delete(uint16 k, const uint8 *hdp) {  // 0x888569
  hdma_object_channels_bitmask[k >> 1] = 0;
  return 0;
}

const uint8 *HdmaobjInstr_SetPreInstr(uint16 k, const uint8 *hdp) {  // 0x888570
  hdma_object_pre_instructions[k >> 1] = GET_WORD(hdp);
  *((uint8 *)hdma_object_pre_instruction_bank + k) = hdp[2];
  return hdp + 3;
}

const uint8 *HdmaobjInstr_ClearPreInstr(uint16 k, const uint8 *hdp) {  // 0x888584
  hdma_object_pre_instructions[k >> 1] = addr_locret_88858A;
  return hdp;
}

void CallHdmaobjPreInstr(uint32 ea, uint16 k) {
  switch (ea) {
  case fnnullsub_56: return;
  case fnnullsub_293: return;
  case fnnullsub_309: return;
  case fnHdmaobjPreInstr_XraySetup: HdmaobjPreInstr_XraySetup(k); return;
  case fnHdmaobjPreInstr_Xray: HdmaobjPreInstr_Xray(k); return;
  case fnHdmaobjPreInstr_XrayFunc0_NoBeam: HdmaobjPreInstr_XrayFunc0_NoBeam(k); return;
  case fnHdmaobjPreInstr_XrayFunc1_BeamWidening: HdmaobjPreInstr_XrayFunc1_BeamWidening(k); return;
  case fnHdmaobjPreInstr_XrayFunc2_FullBeam: HdmaobjPreInstr_XrayFunc2_FullBeam(k); return;
  case fnHdmaobjPreInstr_XrayFunc3_DeactivateBeam: HdmaobjPreInstr_XrayFunc3_DeactivateBeam(k); return;
  case fnHdmaobjPreInstr_XrayFunc4_DeactivateBeam: HdmaobjPreInstr_XrayFunc4_DeactivateBeam(k); return;
  case fnHdmaobjPreInstr_XrayFunc5_DeactivateBeam: HdmaobjPreInstr_XrayFunc5_DeactivateBeam(k); return;
  case fnHdmaobjPreInstr_PowerBombExplode_SetWindowConf: HdmaobjPreInstr_PowerBombExplode_SetWindowConf(k); return;
  case fnHdmaobjPreInstr_PowerBombExplode_Stage5_Afterglow: HdmaobjPreInstr_PowerBombExplode_Stage5_Afterglow(k); return;
  case fnHdmaobjPreInstr_PowerBombExplode_ExplosionYellow: HdmaobjPreInstr_PowerBombExplode_ExplosionYellow(k); return;
  case fnHdmaobjPreInstr_PowerBombExplode_ExplosionWhite: HdmaobjPreInstr_PowerBombExplode_ExplosionWhite(k); return;
  case fnHdmaobjPreInstr_PowerBombExplode_PreExplosionWhite: HdmaobjPreInstr_PowerBombExplode_PreExplosionWhite(k); return;
  case fnHdmaobjPreInstr_PowerBombExplode_PreExplosionYellow: HdmaobjPreInstr_PowerBombExplode_PreExplosionYellow(k); return;
  case fnHdmaobjPreInstr_CrystalFlash_CustomLayerBlend: HdmaobjPreInstr_CrystalFlash_CustomLayerBlend(k); return;
  case fnHdmaobjPreInstr_CrystalFlash_Stage2_AfterGlow: HdmaobjPreInstr_CrystalFlash_Stage2_AfterGlow(k); return;
  case fnHdmaobjPreInstr_CrystalFlash_Stage1_Explosion: HdmaobjPreInstr_CrystalFlash_Stage1_Explosion(k); return;
  case fnHdmaobjPreInstr_FxType22_BG3Yscroll: HdmaobjPreInstr_FxType22_BG3Yscroll(k); return;
  case fnHdmaobjPreInstr_BG3Xscroll: HdmaobjPreInstr_BG3Xscroll(k); return;
  case fnHdmaobjPreInstr_SkyLandBG2Xscroll: HdmaobjPreInstr_SkyLandBG2Xscroll(k); return;
  case fnHdmaobjPreInstr_SkyLandBG2Xscroll2: HdmaobjPreInstr_SkyLandBG2Xscroll2(k); return;
  case fnHdmaobjPreInstr_SkyLandBG2XscrollInner: HdmaobjPreInstr_SkyLandBG2XscrollInner(k); return;
  case fnHdmaobjPreInstr_FirefleaBG3XScroll: HdmaobjPreInstr_FirefleaBG3XScroll(k); return;
  case fnHdmaobjPreInstr_LavaAcidBG3YScroll: HdmaobjPreInstr_LavaAcidBG3YScroll(k); return;
  case fnHdmaobjPreInstr_LavaAcidBG2YScroll: HdmaobjPreInstr_LavaAcidBG2YScroll(k); return;
  case fnHdmaobjPreInstr_WaterBG3XScroll: HdmaobjPreInstr_WaterBG3XScroll(k); return;
  case fnHdmaobjPreInstr_WaterBG2XScroll: HdmaobjPreInstr_WaterBG2XScroll(k); return;
  case fnHdmaobjPreInstr_WaterBG2XScroll_Func2: HdmaobjPreInstr_WaterBG2XScroll_Func2(k); return;
  case fnHdmaobjPreInstr_WaterBG2XScroll_Func1: HdmaobjPreInstr_WaterBG2XScroll_Func1(k); return;
  case fnHdmaobjPreInstr_RainBg3Scroll: HdmaobjPreInstr_RainBg3Scroll(k); return;
  case fnHdmaobjPreInstr_SporesBG3Xscroll: HdmaobjPreInstr_SporesBG3Xscroll(k); return;
  case fnHdmaobjPreInstr_FogBG3Scroll: HdmaobjPreInstr_FogBG3Scroll(k); return;
  case fnHdmaobjPreInstr_CheckLotsOfEventsHappened: HdmaobjPreInstr_CheckLotsOfEventsHappened(k); return;
  case fnHdmaobjPreInstr_DC23: HdmaobjPreInstr_DC23(k); return;
  case fnHdmaobjPreInstr_DC69: HdmaobjPreInstr_DC69(k); return;
  case fnHdmaobjPreInstr_DCBA: HdmaobjPreInstr_DCBA(k); return;
  case fnHdmaobjPreInstr_BombTorizoHazeColorMathBgColor: HdmaobjPreInstr_BombTorizoHazeColorMathBgColor(k); return;
  case fnHdmaobjPreInstr_HazeColorMathSubscreen_CeresRidleyAlive: HdmaobjPreInstr_HazeColorMathSubscreen_CeresRidleyAlive(k); return;
  case fnHdmaobjPreInstr_HazeColorMathSubscreen_CeresRidleyDead: HdmaobjPreInstr_HazeColorMathSubscreen_CeresRidleyDead(k); return;
  case fnHdmaobjPreInstr_HazeColorMathSubscreen_FadingIn: HdmaobjPreInstr_HazeColorMathSubscreen_FadingIn(k); return;
  case fnHdmaobjPreInstr_HazeColorMathSubscreen_FadedIn: HdmaobjPreInstr_HazeColorMathSubscreen_FadedIn(k); return;
  case fnHdmaobjPreInstr_HazeColorMathSubscreen_FadingOut: HdmaobjPreInstr_HazeColorMathSubscreen_FadingOut(k); return;
  case fnHdmaobjPreInstr_DF94: HdmaobjPreInstr_DF94(k); return;
  case fnHdmaobjPreInstr_VariaSuitPickup: HdmaobjPreInstr_VariaSuitPickup(k); return;
  case fnHdmaobjPreInstr_GravitySuitPickup: HdmaobjPreInstr_GravitySuitPickup(k); return;
  case fnHdmaobjPreInstr_E449: HdmaobjPreInstr_E449(k); return;
  case fnHdmaobjPreInstr_E567: HdmaobjPreInstr_E567(k); return;
  case fnHdmaobjPreInstr_E7BC: HdmaobjPreInstr_E7BC(k); return;
  case fnHdmaobjPreInstr_E9E6: HdmaobjPreInstr_E9E6(k); return;
  case fnHdmaobjPreInstr_EA3C: HdmaobjPreInstr_EA3C(k); return;
  case fnHdmaobjPreInstr_EACB: HdmaobjPreInstr_EACB(k); return;
  case fnHdmaobjPreInstr_Backdrop_TitleSequenceGradient: HdmaobjPreInstr_Backdrop_TitleSequenceGradient(k); return;
  case fnHdmaobjPreInstr_ColorMathControlB_TitleGradient: HdmaobjPreInstr_ColorMathControlB_TitleGradient(k); return;
  case fnHdmaobjPreInstr_IntroCutsceneCrossfade: HdmaobjPreInstr_IntroCutsceneCrossfade(k); return;
  case fnnullsub_357: return;
  case fnHdmaobjPreInstr_ECB6: HdmaobjPreInstr_ECB6(k); return;
  default: Unreachable();
  }
}

static void CallHdmaobjInstrFunc(uint32 ea, uint16 k) {
  switch (ea) {
  case fnHdmaobj_PreExplodeWhite: Hdmaobj_PreExplodeWhite(); return;
  case fnHdmaobj_PreExplodeYellow: Hdmaobj_PreExplodeYellow(); return;
  case fnHdmaobj_ExplodeYellow: Hdmaobj_ExplodeYellow(); return;
  case fnHdmaobj_ExplodeWhite: Hdmaobj_ExplodeWhite(); return;
  case fnHdmaobj_CleanUpTryCrystalFlash: Hdmaobj_CleanUpTryCrystalFlash(k); return;
  case fnCrystalFlashSetupPart1: CrystalFlashSetupPart1(); return;
  case fnCrystalFlashSetupPart2: CrystalFlashSetupPart2(); return;
  case fnCrystalFlashCleanup: CrystalFlashCleanup(k); return;
  case fnnullsub_113: return;
  case fnnullsub_114: return;
  case fnnullsub_357: return;
  case fnInitializeRainbowBeam: InitializeRainbowBeam(); return;
  case fnXray_SetupStage1_FreezeTimeBackup: Xray_SetupStage1_FreezeTimeBackup(k); return;
  case fnXray_SetupStage2_ReadBg1_2ndScreen: Xray_SetupStage2_ReadBg1_2ndScreen(); return;
  case fnXray_SetupStage3_ReadBg1_1stScreen: Xray_SetupStage3_ReadBg1_1stScreen(); return;
  case fnXray_SetupStage4: Xray_SetupStage4(); return;
  case fnXray_SetupStage5: Xray_SetupStage5(); return;
  case fnXray_SetupStage6: Xray_SetupStage6(); return;
  case fnXray_SetupStage7: Xray_SetupStage7(); return;
  case fnXray_SetupStage8_SetBackdropColor: Xray_SetupStage8_SetBackdropColor(); return;
  case fnInitializeSuitPickupHdma: InitializeSuitPickupHdma(); return;
  default: Unreachable();
  }
}

const uint8 *HdmaobjInstr_CallFarFunc(uint16 k, const uint8 *hdp) {  // 0x8885B4
  CallHdmaobjInstrFunc(Load24((LongPtr *)hdp), k);
  return hdp + 3;
}

const uint8 *CallHdmaobjInstr(uint32 ea, uint16 k, const uint8 *j) {
  switch (ea) {
  case fnnullsub_112: return j;
  case fnHdmaobjInstr_Delete: return HdmaobjInstr_Delete(k, j);
  case fnHdmaobjInstr_SetPreInstr: return HdmaobjInstr_SetPreInstr(k, j);
  case fnHdmaobjInstr_ClearPreInstr: return HdmaobjInstr_ClearPreInstr(k, j);
  case fnHdmaobjInstr_CallFarFunc: return HdmaobjInstr_CallFarFunc(k, j);
  case fnHdmaobjInstr_Goto: return HdmaobjInstr_Goto(k, j);
  case fnHdmaobjInstr_GotoRel: return HdmaobjInstr_GotoRel(k, j);
  case fnHdmaobjInstr_DecrementAndGoto: return HdmaobjInstr_DecrementAndGoto(k, j);
  case fnHdmaobjInstr_DecrementAndGotoRel: return HdmaobjInstr_DecrementAndGotoRel(k, j);
  case fnHdmaobjInstr_SetTimer: return HdmaobjInstr_SetTimer(k, j);
  case fnHdmaobjInstr_SetHdmaControl: return HdmaobjInstr_SetHdmaControl(k, j);
  case fnHdmaobjInstr_SetHdmaTarget: return HdmaobjInstr_SetHdmaTarget(k, j);
  case fnHdmaobjInstr_SetHdmaTablePtr: return HdmaobjInstr_SetHdmaTablePtr(k, j);
  case fnHdmaobjInstr_SetHdmaTableBank: return HdmaobjInstr_SetHdmaTableBank(k, j);
  case fnHdmaobjInstr_SetIndirectHdmaDataBank: return HdmaobjInstr_SetIndirectHdmaDataBank(k, j);
  case fnHdmaobjInstr_Sleep: return HdmaobjInstr_Sleep(k, j);
  case fnHdmaobjInstr_SetFlagB: return HdmaobjInstr_SetFlagB(k, j);
  case fnHdmaobjInstr_SetFlagB_Copy: return HdmaobjInstr_SetFlagB_Copy(k, j);
  case fnHdmaobjInstr_SetFlagB_Copy2: return HdmaobjInstr_SetFlagB_Copy2(k, j);
  case fnHdmaobjInstr_SetFlagB_Copy3: return HdmaobjInstr_SetFlagB_Copy3(k, j);
  case fnHdmaobjInstr_SetVideoMode1: return HdmaobjInstr_SetVideoMode1(k, j);
  case fnHdmaobjInstr_1938_RandomNumber: return HdmaobjInstr_1938_RandomNumber(k, j);
  case fnHdmaobjInstr_GotoIfEventHappened: return HdmaobjInstr_GotoIfEventHappened(k, j);
  case fnHdmaobjInstr_E4BD: return HdmaobjInstr_E4BD(k, j);
  case fnHdmaobjInstr_InitMorphBallEyeBeamHdma: return HdmaobjInstr_InitMorphBallEyeBeamHdma(k, j);
  case fnHdmaobjInstr_EC9F_ClearVars: return HdmaobjInstr_EC9F_ClearVars(k, j);
  case fnHdmaobjInstr_B3A9: return HdmaobjInstr_B3A9(k, j);
  case fnHdmaobjInsr_ConfigTitleSequenceGradientHDMA: return HdmaobjInsr_ConfigTitleSequenceGradientHDMA(k, j);
  case fnsub_88D916: sub_88D916(); return j;
  default: Unreachable(); return NULL;
  }
}

const uint8 *HdmaobjInstr_Goto(uint16 k, const uint8 *hdp) {  // 0x8885EC
  return INSTRB_RETURN_ADDR(GET_WORD(hdp));
}

const uint8 *HdmaobjInstr_GotoRel(uint16 k, const uint8 *hdp) {  // 0x8885F1
  return hdp + (int8)*hdp;
}

const uint8 *HdmaobjInstr_DecrementAndGoto(uint16 k, const uint8 *hdp) {  // 0x888607
  int v2 = k >> 1;
  if (hdma_object_timers[v2]-- == 1)
    return hdp + 2;
  else
    return INSTRB_RETURN_ADDR(GET_WORD(hdp));
}

const uint8 *HdmaobjInstr_DecrementAndGotoRel(uint16 k, const uint8 *hdp) {  // 0x88860F
  int v2 = k >> 1;
  if (hdma_object_timers[v2]-- == 1)
    return hdp + 1;
  else
    return hdp + (int8)*hdp;
}

const uint8 *HdmaobjInstr_SetTimer(uint16 k, const uint8 *hdp) {  // 0x888616
  *((uint8 *)hdma_object_timers + k) = *hdp;
  return hdp + 1;
}

const uint8 *HdmaobjInstr_SetHdmaControl(uint16 k, const uint8 *hdp) {  // 0x888622
  WriteReg((SnesRegs)(LOBYTE(hdma_object_bank_slot[k >> 1]) + DMAP0), *hdp);
  return hdp + 1;
}

const uint8 *HdmaobjInstr_SetHdmaTarget(uint16 k, const uint8 *hdp) {  // 0x888637
  WriteReg((SnesRegs)(LOBYTE(hdma_object_bank_slot[k >> 1]) + BBAD0), *hdp);
  return hdp + 1;
}

const uint8 *HdmaobjInstr_SetHdmaTablePtr(uint16 k, const uint8 *hdp) {  // 0x88864C
  hdma_object_table_pointers[k >> 1] = GET_WORD(hdp);
  return hdp + 2;
}

const uint8 *HdmaobjInstr_SetHdmaTableBank(uint16 k, const uint8 *hdp) {  // 0x888655
  WriteReg((SnesRegs)(LOBYTE(hdma_object_bank_slot[k >> 1]) + A1B0), *hdp);
  return hdp + 1;
}

const uint8 *HdmaobjInstr_SetIndirectHdmaDataBank(uint16 k, const uint8 *hdp) {  // 0x88866A
  WriteReg((SnesRegs)(LOBYTE(hdma_object_bank_slot[k >> 1]) + DAS00), *hdp);
  return hdp + 1;
}

const uint8 *HdmaobjInstr_Sleep(uint16 k, const uint8 *hdp) {  // 0x888682
  const uint8 *base = RomBankBase(*((uint8 *)hdma_object_bank_slot + k + 1));
  hdma_object_instruction_list_pointers[k >> 1] = hdp - base - 2;
  return 0;
}
