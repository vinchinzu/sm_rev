// Bank $88 power-bomb and crystal-flash HDMA effects.

#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"

#define g_byte_88A206 ((uint8*)RomFixedPtr(0x88a206))
#define g_byte_88A286 ((uint8*)RomFixedPtr(0x88a286))
#define kPowerBombExplosionColors ((uint8*)RomFixedPtr(0x888d85))
#define g_byte_889079 ((uint8*)RomFixedPtr(0x889079))

static const uint16 kPowerBombPreExplosionRadiusSpeed = 0x3000;
static const uint16 kPowerBombPreExplosionRadiusAccel = 0x80;
static const uint16 g_word_888B96 = 3;
static const uint16 kPowerBombExplosionRadiusSpeed = 0;
static const uint16 kPowerBombExplosionRadiusAccel = 0x30;

static uint16 k_out;

void SpawnPowerBombExplosion(void) {  // 0x888AA4
  if (time_is_frozen_flag) {
    power_bomb_explosion_status = 0x4000;
  } else {
    QueueSfx1_Max15(1);
    power_bomb_explosion_status = 0x8000;

    static const SpawnHdmaObject_Args unk_888ABA = { 0x40, 0x28, 0x8ace };
    static const SpawnHdmaObject_Args unk_888AC2 = { 0x40, 0x29, 0x8b80 };
    SpawnHdmaObject(0x88, &unk_888ABA);
    SpawnHdmaObject(0x88, &unk_888AC2);
  }
}

void Hdmaobj_PreExplodeWhite(void) {  // 0x888B14
  offscreen_power_bomb_left_hdma = -1;
  offscreen_power_bomb_right_hdma = 0;
  power_bomb_pre_explosion_flash_radius = 1024;
  power_bomb_pre_explosion_radius_speed = kPowerBombPreExplosionRadiusSpeed;
}

void Hdmaobj_PreExplodeYellow(void) {  // 0x888B32
  pre_scaled_power_bomb_explosion_shape_def_ptr = addr_byte_889F06;
}

void Hdmaobj_ExplodeYellow(void) {  // 0x888B39
  power_bomb_explosion_radius = 1024;
  power_bomb_pre_explosion_radius_speed = kPowerBombExplosionRadiusSpeed;
}

void Hdmaobj_ExplodeWhite(void) {  // 0x888B47
  pre_scaled_power_bomb_explosion_shape_def_ptr = addr_kPowerBombExplosionShapeDef0;
}

void Hdmaobj_CleanUpTryCrystalFlash(uint16 v0) {  // 0x888B4E
  if (samus_x_pos != power_bomb_explosion_x_pos
      || samus_y_pos != power_bomb_explosion_y_pos
      || Hdmaobj_CrystalFlash() & 1) {
    power_bomb_flag = 0;
  }
  power_bomb_explosion_status = 0;
  int v1 = v0 >> 1;
  hdma_object_channels_bitmask[v1] = 0;
  hdma_object_channels_bitmask[v1 + 1] = 0;
  power_bomb_pre_explosion_flash_radius = 0;
  power_bomb_explosion_radius = 0;
  CallSomeSamusCode(0x1E);
}

void HdmaobjPreInstr_PowerBombExplode_SetWindowConf(uint16 k) {  // 0x888B8F
  fx_layer_blending_config_c |= 0x8000;
}

void HdmaobjPreInstr_PowerBombExplode_Stage5_Afterglow(uint16 k) {  // 0x888B98
  if ((power_bomb_explosion_status & 0x8000) != 0) {
    int v1 = (uint8)k >> 1;
    if ((--hdma_object_timers[v1] & 0x8000) != 0) {
      if ((*((uint8 *)hdma_object_D + (uint8)k))-- == 1) {
        hdma_object_instruction_timers[v1] = 1;
        hdma_object_instruction_list_pointers[v1] += 2;
      } else {
        if ((reg_COLDATA[0] & 0x1F) != 0)
          reg_COLDATA[0] = ((reg_COLDATA[0] & 0x1F) - 1) | 0x20;
        if ((reg_COLDATA[1] & 0x1F) != 0)
          reg_COLDATA[1] = ((reg_COLDATA[1] & 0x1F) - 1) | 0x40;
        if ((reg_COLDATA[2] & 0x1F) != 0)
          reg_COLDATA[2] = ((reg_COLDATA[2] & 0x1F) - 1) | 0x80;
        *((uint8 *)hdma_object_timers + (uint8)k) = g_word_888B96;
      }
    }
  }
}

void CalculatePowerBombHdma_LeftOfScreen(uint16 k, const uint8 *j) {  // 0x888BEA
  do {
    uint8 w = *j;
    uint8 c = power_bomb_explosion_x_pos_plus_0x100;
    if (__CFADD__uint8(w, c)) {
      power_bomb_explosion_right_hdma[k] = w + c;
      power_bomb_explosion_left_hdma[k] = 0;
    } else {
      power_bomb_explosion_right_hdma[k] = 0;
      power_bomb_explosion_left_hdma[k] = 1;
    }
    ++j;
    ++k;
  } while (k != 192);
}

void CalculatePowerBombHdma_OnScreen(uint16 k, const uint8 *j) {  // 0x888C12
  do {
    uint8 w = *j;
    if (!w)
      break;
    uint8 c = power_bomb_explosion_x_pos_plus_0x100;
    uint8 right = __CFADD__uint8(c, w) ? 255 : c + w;
    uint8 left = (c >= w) ? c - w : 0;
    power_bomb_explosion_left_hdma[k] = left;
    power_bomb_explosion_right_hdma[k] = right;
    ++j;
    ++k;
  } while (k != 192);
}

void CalculatePowerBombHdma_RightOfScreen(uint16 k, const uint8 *j) {  // 0x888C3A
  do {
    uint8 w = *j;
    if ((uint8)power_bomb_explosion_x_pos_plus_0x100 < w) {
      power_bomb_explosion_left_hdma[k] = power_bomb_explosion_x_pos_plus_0x100 - w;
      power_bomb_explosion_right_hdma[k] = -1;
    } else {
      power_bomb_explosion_left_hdma[k] = -1;
      power_bomb_explosion_right_hdma[k] = -2;
    }
    ++j;
    ++k;
  } while (k != 192);
}

void CalculatePowerBombHdmaObjectTablePtrs(uint16 k) {  // 0x888C62
  uint16 v1;
  if ((uint16)(power_bomb_explosion_x_pos - layer1_x_pos + 256) >= 0x300
      || (power_bomb_explosion_x_pos_plus_0x100 = power_bomb_explosion_x_pos - layer1_x_pos + 256,
          v1 = power_bomb_explosion_y_pos - layer1_y_pos + 256,
          v1 >= 0x300)) {
    v1 = 0;
  }
  power_bomb_explosion_y_pos_rsub_0x1ff = (v1 ^ 0x3FF) - 256;
  if ((power_bomb_explosion_radius & 0xFF00) == 0)
    power_bomb_explosion_y_pos_rsub_0x1ff = 0;
  hdma_object_table_pointers[(k >> 1) + 0] = 3 * power_bomb_explosion_y_pos_rsub_0x1ff + addr_kIndirectHdmaTable_PowerBombExplodeLeft;
  hdma_object_table_pointers[(k >> 1) + 1] = 3 * power_bomb_explosion_y_pos_rsub_0x1ff + addr_kIndirectHdmaTable_PowerBombExplodeRight;
}

uint16 CalculatePowerBombHdmaScaled_LeftOfScreen(uint16 k, uint16 j, uint8 multval) {  // 0x888CC6
  int8 v2;
  uint8 v6, v7;
  do {
    uint8 r20 = Mult8x8(multval, g_byte_88A206[(uint8)j + 32]) >> 8;
    v2 = power_bomb_explosion_x_pos_plus_0x100;
    uint8 Reg = Mult8x8(multval, g_byte_88A206[(uint8)j]) >> 8;
    bool v4 = __CFADD__uint8(Reg, v2);
    uint8 v5 = Reg + v2;
    if (v4) {
      v6 = v5;
      v7 = 0;
    } else {
      v6 = 0;
      v7 = -1;
    }
    while (1) {
      power_bomb_explosion_left_hdma[(uint8)k] = v7;
      power_bomb_explosion_right_hdma[(uint8)k] = v6;
      if ((uint8)k == r20)
        break;
      LOBYTE(k) = k - 1;
    }
    LOBYTE(j) = j + 1;
  } while ((j & 0x80) == 0);
  k_out = LOBYTE(k);
  return v6 << 8 | v7;
}

uint16 CalculatePowerBombHdmaScaled_OnScreen(uint16 k, uint16 j, uint8 multval) {  // 0x888D04
  int8 v2;
  int8 v5;
  uint8 v6, v9;
  do {
    uint8 r20 = Mult8x8(multval, g_byte_88A206[(uint8)j + 32]) >> 8;
    v2 = power_bomb_explosion_x_pos_plus_0x100;
    uint8 Reg = Mult8x8(multval, g_byte_88A206[(uint8)j]) >> 8;
    bool v4 = __CFADD__uint8(Reg, v2);
    v5 = Reg + v2;
    if (v4)
      v5 = -1;
    v6 = v5;
    uint8 v7 = power_bomb_explosion_x_pos_plus_0x100;
    uint8 v8 = Reg;
    v4 = v7 < v8;
    v9 = v7 - v8;
    if (v4)
      v9 = 0;
    while (1) {
      power_bomb_explosion_left_hdma[(uint8)k] = v9;
      power_bomb_explosion_right_hdma[(uint8)k] = v6;
      if ((uint8)k == r20)
        break;
      LOBYTE(k) = k - 1;
    }
    LOBYTE(j) = j + 1;
  } while ((j & 0x80) == 0);
  k_out = LOBYTE(k);
  return v6 << 8 | v9;
}

uint16 CalculatePowerBombHdmaScaled_RightOfScreen(uint16 k, uint16 j, uint8 multval) {  // 0x888D46
  int8 v5;
  int8 v6; // ah
  int8 v7;
  int8 v10; // t2
  uint8 v8, v9;
  do {
    uint8 r20 = Mult8x8(multval, g_byte_88A206[(uint8)j + 32]) >> 8;
    uint8 v2 = power_bomb_explosion_x_pos_plus_0x100;
    uint8 Reg = Mult8x8(multval, g_byte_88A206[(uint8)j]) >> 8;
    bool v4 = v2 < Reg;
    v5 = v2 - Reg;
    if (v4) {
      v6 = v5;
      v7 = -1;
    } else {
      v6 = -1;
      v7 = 0;
    }
    v10 = v7;
    v8 = v6;
    v9 = v10;
    while (1) {
      power_bomb_explosion_left_hdma[(uint8)k] = v8;
      power_bomb_explosion_right_hdma[(uint8)k] = v9;
      if ((uint8)k == r20)
        break;
      LOBYTE(k) = k - 1;
    }
    LOBYTE(j) = j + 1;
  } while ((j & 0x80) == 0);
  k_out = LOBYTE(k);
  return v9 << 8 | v8;
}

void HdmaobjPreInstr_PowerBombExplode_ExplosionYellow(uint16 k) {  // 0x888DE9
  uint16 v2;

  if ((power_bomb_explosion_status & 0x8000) == 0)
    return;
  CalculatePowerBombHdmaObjectTablePtrs(k);
  int kk = Mult8x8(GET_HIBYTE(power_bomb_explosion_radius), g_byte_88A286[0]) >> 8;

  if (GET_HIBYTE(power_bomb_explosion_x_pos_plus_0x100)) {
    if (GET_HIBYTE(power_bomb_explosion_x_pos_plus_0x100) == 1)
      v2 = CalculatePowerBombHdmaScaled_OnScreen(kk, 96, GET_HIBYTE(power_bomb_explosion_radius));
    else
      v2 = CalculatePowerBombHdmaScaled_RightOfScreen(kk, 96, GET_HIBYTE(power_bomb_explosion_radius));
  } else {
    v2 = CalculatePowerBombHdmaScaled_LeftOfScreen(kk, 96, GET_HIBYTE(power_bomb_explosion_radius));
  }
  int i = k_out;
  do {
    power_bomb_explosion_left_hdma[i] = v2;
    power_bomb_explosion_right_hdma[i] = GET_HIBYTE(v2);
  } while ((--i & 0x80) == 0);
  for (int v3 = kk + 1; v3 != 0xc0; v3++) {
    power_bomb_explosion_left_hdma[v3] = 255;
    power_bomb_explosion_right_hdma[v3] = 0;
  }
  int t = 3 * (GET_HIBYTE(power_bomb_explosion_radius) >> 3);
  reg_COLDATA[0] = kPowerBombExplosionColors[t + 0] | 0x20;
  reg_COLDATA[1] = kPowerBombExplosionColors[t + 1] | 0x40;
  reg_COLDATA[2] = kPowerBombExplosionColors[t + 2] | 0x80;
  power_bomb_explosion_radius += power_bomb_pre_explosion_radius_speed;
  if (power_bomb_explosion_radius < 0x8600) {
    power_bomb_pre_explosion_radius_speed += kPowerBombExplosionRadiusAccel;
  } else {
    int v7 = k >> 1;
    hdma_object_instruction_timers[v7] = 1;
    hdma_object_instruction_list_pointers[v7] += 2;
    hdma_object_timers[v7] = 0;
  }
}

void HdmaobjPreInstr_PowerBombExplode_ExplosionWhite(uint16 k) {  // 0x888EB2
  if ((power_bomb_explosion_status & 0x8000) == 0)
    return;

  CalculatePowerBombHdmaObjectTablePtrs(k);
  const uint8 *v1 = RomPtr_88(pre_scaled_power_bomb_explosion_shape_def_ptr);
  if (GET_HIBYTE(power_bomb_explosion_x_pos_plus_0x100)) {
    if (GET_HIBYTE(power_bomb_explosion_x_pos_plus_0x100) == 1)
      CalculatePowerBombHdma_OnScreen(0, v1);
    else
      CalculatePowerBombHdma_RightOfScreen(0, v1);
  } else {
    CalculatePowerBombHdma_LeftOfScreen(0, v1);
  }
  int t = 3 * (GET_HIBYTE(power_bomb_explosion_radius) >> 3);
  reg_COLDATA[0] = kPowerBombExplosionColors[t] | 0x20;
  reg_COLDATA[1] = kPowerBombExplosionColors[t + 1] | 0x40;
  reg_COLDATA[2] = kPowerBombExplosionColors[t + 2] | 0x80;
  pre_scaled_power_bomb_explosion_shape_def_ptr += 192;
  if (pre_scaled_power_bomb_explosion_shape_def_ptr == addr_byte_889F06) {
    int v3 = k >> 1;
    hdma_object_instruction_timers[v3] = 1;
    hdma_object_instruction_list_pointers[v3] += 2;
    hdma_object_timers[v3] = 0;
    hdma_object_D[v3] = 32;
  }
  if (power_bomb_pre_explosion_radius_speed + power_bomb_explosion_radius < 0x10000) {
    power_bomb_explosion_radius += power_bomb_pre_explosion_radius_speed;
    power_bomb_pre_explosion_radius_speed += kPowerBombExplosionRadiusAccel;
  }
}

void CalculatePowerBombHdmaTablePointers(uint16 v0) {  // 0x888F56
  uint16 v1;
  if ((power_bomb_explosion_status & 0x8000) != 0) {
    if ((uint16)(power_bomb_explosion_x_pos - layer1_x_pos + 256) >= 0x300
        || (power_bomb_explosion_x_pos_plus_0x100 = power_bomb_explosion_x_pos - layer1_x_pos + 256,
            v1 = power_bomb_explosion_y_pos - layer1_y_pos + 256,
            v1 >= 0x300)) {
      v1 = 0;
    }
    power_bomb_explosion_y_pos_rsub_0x1ff = (v1 ^ 0x3FF) - 256;
    if ((power_bomb_pre_explosion_flash_radius & 0xFF00) == 0)
      power_bomb_explosion_y_pos_rsub_0x1ff = 0;
    int v2 = v0 >> 1;
    hdma_object_table_pointers[v2] = 3 * power_bomb_explosion_y_pos_rsub_0x1ff + addr_kIndirectHdmaTable_PowerBombExplodeLeft;
    hdma_object_table_pointers[v2 + 1] = 3 * power_bomb_explosion_y_pos_rsub_0x1ff + addr_kIndirectHdmaTable_PowerBombExplodeRight;
  }
}

void HdmaobjPreInstr_PowerBombExplode_PreExplosionWhite(uint16 k) {  // 0x8890DF
  if ((power_bomb_explosion_status & 0x8000) == 0)
    return;
  CalculatePowerBombHdmaTablePointers(k);
  uint16 v1 = 96, v2;
  int kk = Mult8x8(HIBYTE(power_bomb_pre_explosion_flash_radius), g_byte_88A286[0]) >> 8;
  if (HIBYTE(power_bomb_explosion_x_pos_plus_0x100)) {
    if (HIBYTE(power_bomb_explosion_x_pos_plus_0x100) == 1)
      v2 = CalculatePowerBombHdmaScaled_OnScreen(kk, v1, HIBYTE(power_bomb_pre_explosion_flash_radius));
    else
      v2 = CalculatePowerBombHdmaScaled_RightOfScreen(kk, v1, HIBYTE(power_bomb_pre_explosion_flash_radius));
  } else {
    v2 = CalculatePowerBombHdmaScaled_LeftOfScreen(kk, v1, HIBYTE(power_bomb_pre_explosion_flash_radius));
  }
  uint8 i = k_out;
  do {
    power_bomb_explosion_left_hdma[i] = v2;
    power_bomb_explosion_right_hdma[i] = HIBYTE(v2);
    i--;
  } while ((i & 0x80) == 0);
  for (uint8 v3 = kk + 1; v3 != 0xc0; v3++) {
    power_bomb_explosion_left_hdma[v3] = 255;
    power_bomb_explosion_right_hdma[v3] = 0;
  }
  int t = (HIBYTE(power_bomb_pre_explosion_flash_radius) >> 3) & 0xF;
  reg_COLDATA[0] = g_byte_889079[t * 3 + 0] | 0x20;
  reg_COLDATA[1] = g_byte_889079[t * 3 + 1] | 0x40;
  reg_COLDATA[2] = g_byte_889079[t * 3 + 2] | 0x80;
  power_bomb_pre_explosion_flash_radius += power_bomb_pre_explosion_radius_speed;
  if (power_bomb_pre_explosion_flash_radius < 0x9200) {
    power_bomb_pre_explosion_radius_speed -= kPowerBombPreExplosionRadiusAccel;
  } else {
    hdma_object_instruction_timers[k >> 1] = 1;
    hdma_object_instruction_list_pointers[k >> 1] += 2;
    hdma_object_timers[k >> 1] = 0;
  }
}

void HdmaobjPreInstr_PowerBombExplode_PreExplosionYellow(uint16 k) {  // 0x8891A8
  if ((power_bomb_explosion_status & 0x8000) == 0)
    return;
  CalculatePowerBombHdmaTablePointers(k);
  const uint8 *v1 = RomPtr_88(pre_scaled_power_bomb_explosion_shape_def_ptr);
  if (GET_HIBYTE(power_bomb_explosion_x_pos_plus_0x100)) {
    if (GET_HIBYTE(power_bomb_explosion_x_pos_plus_0x100) == 1)
      CalculatePowerBombHdma_OnScreen(0, v1);
    else
      CalculatePowerBombHdma_RightOfScreen(0, v1);
  } else {
    CalculatePowerBombHdma_LeftOfScreen(0, v1);
  }
  int t = (GET_HIBYTE(power_bomb_pre_explosion_flash_radius) >> 3) & 0xF;
  reg_COLDATA[0] = g_byte_889079[3 * t] | 0x20;
  reg_COLDATA[1] = g_byte_889079[3 * t + 1] | 0x40;
  reg_COLDATA[2] = g_byte_889079[3 * t + 2] | 0x80;
  pre_scaled_power_bomb_explosion_shape_def_ptr += 192;
  if (pre_scaled_power_bomb_explosion_shape_def_ptr == 0xA206) {
    hdma_object_instruction_timers[k >> 1] = 1;
    hdma_object_instruction_list_pointers[k >> 1] += 2;
    hdma_object_timers[k >> 1] = 0;
  }
  if (power_bomb_pre_explosion_radius_speed + power_bomb_pre_explosion_flash_radius < 0x10000) {
    power_bomb_pre_explosion_flash_radius += power_bomb_pre_explosion_radius_speed;
    power_bomb_pre_explosion_radius_speed -= kPowerBombPreExplosionRadiusAccel;
  }
}

void SpawnCrystalFlashHdmaObjs(void) {  // 0x88A2A6
  power_bomb_explosion_status = FUNC16(LayerBlendingHandler);
  static const SpawnHdmaObject_Args unk_88A2B0 = { 0x40, 0x28, 0xa2bd };
  static const SpawnHdmaObject_Args unk_88A2B8 = { 0x40, 0x29, 0xa32a };
  SpawnHdmaObject(0x88, &unk_88A2B0);
  SpawnHdmaObject(0x88, &unk_88A2B8);
}

void CrystalFlashSetupPart1(void) {  // 0x88A2E4
  offscreen_power_bomb_left_hdma = -1;
  offscreen_power_bomb_right_hdma = 0;
  power_bomb_pre_explosion_flash_radius = 1024;
  power_bomb_pre_explosion_radius_speed = kPowerBombPreExplosionRadiusSpeed;
  QueueSfx1_Max6(1);
}

void CrystalFlashSetupPart2(void) {  // 0x88A309
  power_bomb_explosion_radius = 1024;
  power_bomb_pre_explosion_radius_speed = kPowerBombExplosionRadiusSpeed;
}

void CrystalFlashCleanup(uint16 k) {  // 0x88A317
  uint16 v0 = k;

  power_bomb_flag = 0;
  power_bomb_explosion_status = 0;
  int v1 = v0 >> 1;
  hdma_object_channels_bitmask[v1] = 0;
  hdma_object_channels_bitmask[v1 + 1] = 0;
  power_bomb_pre_explosion_flash_radius = 0;
  power_bomb_explosion_radius = 0;
}

void HdmaobjPreInstr_CrystalFlash_CustomLayerBlend(uint16 k) {  // 0x88A339
  reg_W12SEL = 0;
  reg_W34SEL = 8;
  reg_WOBJSEL = 0x80;
  next_gameplay_CGWSEL = 2;
  next_gameplay_CGADSUB = 51;
  reg_TMW = 0;
  reg_TSW = 4;
}

void HdmaobjPreInstr_CrystalFlash_Stage2_AfterGlow(uint16 k) {  // 0x88A35D
  if ((power_bomb_explosion_status & 0x8000) != 0) {
    int v1 = k >> 1;
    if ((--hdma_object_timers[v1] & 0x8000) != 0) {
      if (((reg_COLDATA[2] | reg_COLDATA[1] | reg_COLDATA[0]) & 0x1F) != 0) {
        if ((reg_COLDATA[0] & 0x1F) != 0)
          reg_COLDATA[0] = ((reg_COLDATA[0] & 0x1F) - 1) | 0x20;
        if ((reg_COLDATA[1] & 0x1F) != 0)
          reg_COLDATA[1] = ((reg_COLDATA[1] & 0x1F) - 1) | 0x40;
        if ((reg_COLDATA[2] & 0x1F) != 0)
          reg_COLDATA[2] = ((reg_COLDATA[2] & 0x1F) - 1) | 0x80;
        *((uint8 *)hdma_object_timers + k) = g_word_888B96;
      } else {
        hdma_object_instruction_timers[v1] = 1;
        hdma_object_instruction_list_pointers[v1] += 2;
      }
    }
  }
}

void CalculateCrystalFlashHdmaObjectTablePtrs(uint16 k) {  // 0x88A42F
  uint16 v1;
  if ((uint16)(power_bomb_explosion_x_pos - layer1_x_pos + 256) >= 0x300
      || (power_bomb_explosion_x_pos_plus_0x100 = power_bomb_explosion_x_pos - layer1_x_pos + 256,
          v1 = power_bomb_explosion_y_pos - layer1_y_pos + 256,
          v1 >= 0x300)) {
    v1 = 0;
  }
  power_bomb_explosion_y_pos_rsub_0x1ff = (v1 ^ 0x3FF) - 256;
  if ((power_bomb_explosion_radius & 0xFF00) == 0)
    power_bomb_explosion_y_pos_rsub_0x1ff = 0;
  hdma_object_table_pointers[(k >> 1)] = 3 * power_bomb_explosion_y_pos_rsub_0x1ff + addr_kIndirectHdmaTable_PowerBombExplodeLeft;
  hdma_object_table_pointers[(k >> 1) + 1] = 3 * power_bomb_explosion_y_pos_rsub_0x1ff + addr_kIndirectHdmaTable_PowerBombExplodeRight;
}

uint16 CalculateCrystalFlashHdmaDataTablesScaled_LeftOfScreen(uint16 k, uint16 j) {  // 0x88A493
  uint8 right, left;
  do {
    uint8 r20 = Mult8x8(GET_HIBYTE(power_bomb_explosion_radius), g_byte_88A206[(uint8)j + 32]) >> 8;
    uint8 w = Mult8x8(GET_HIBYTE(power_bomb_explosion_radius), g_byte_88A206[(uint8)j]) >> 8;
    uint8 c = power_bomb_explosion_x_pos_plus_0x100;
    left = (w + c >= 256) ? 0 : 255;
    right = (w + c >= 256) ? w + c : 0;
    while (1) {
      power_bomb_explosion_left_hdma[(uint8)k] = left;
      power_bomb_explosion_right_hdma[(uint8)k] = right;
      if ((uint8)k == r20)
        break;
      k--;
    }
    j++;  // note ++
  } while ((j & 0x80) == 0);
  k_out = LOBYTE(k);
  return right << 8 | left;
}

uint16 CalculateCrystalFlashHdmaDataTablesScaled_OnScreen(uint16 k, uint16 j) {  // 0x88A4D1
  uint8 right, left;
  do {
    uint8 r20 = Mult8x8(GET_HIBYTE(power_bomb_explosion_radius), g_byte_88A206[(uint8)j + 32]) >> 8;
    uint8 w = Mult8x8(GET_HIBYTE(power_bomb_explosion_radius), g_byte_88A206[(uint8)j]) >> 8;
    uint8 c = power_bomb_explosion_x_pos_plus_0x100;
    right = c + w < 256 ? c + w : 255;
    left = c - w >= 0 ? c - w : 0;
    while (1) {
      power_bomb_explosion_left_hdma[(uint8)k] = left;
      power_bomb_explosion_right_hdma[(uint8)k] = right;
      if ((uint8)k == r20)
        break;
      k--;
    }
    j++;  // note ++
  } while ((j & 0x80) == 0);
  k_out = LOBYTE(k);
  return right << 8 | left;
}

uint16 CalculateCrystalFlashHdmaDataTablesScaled_RightOfScreen(uint16 k, uint16 j) {  // 0x88A513
  uint8 left, right;
  do {
    uint8 r20 = Mult8x8(GET_HIBYTE(power_bomb_explosion_radius), g_byte_88A206[(uint8)j + 32]) >> 8;
    uint8 w = Mult8x8(GET_HIBYTE(power_bomb_explosion_radius), g_byte_88A206[(uint8)j]) >> 8;
    uint8 c = power_bomb_explosion_x_pos_plus_0x100;
    right = (c < w) ? 255 : 0;
    left = (c < w) ? c - w : 255;
    while (1) {
      power_bomb_explosion_left_hdma[(uint8)k] = left;
      power_bomb_explosion_right_hdma[(uint8)k] = right;
      if ((uint8)k == r20)
        break;
      k--;
    }
    j++;  // note ++
  } while ((j & 0x80) == 0);
  k_out = LOBYTE(k);
  return right << 8 | left;
}

void HdmaobjPreInstr_CrystalFlash_Stage1_Explosion(uint16 k) {  // 0x88A552
  if ((power_bomb_explosion_status & 0x8000) == 0)
    return;
  CalculateCrystalFlashHdmaObjectTablePtrs(k);

  uint16 v2;
  int kk = Mult8x8(GET_HIBYTE(power_bomb_explosion_radius), g_byte_88A286[0]) >> 8;
  if (GET_HIBYTE(power_bomb_explosion_x_pos_plus_0x100)) {
    if (GET_HIBYTE(power_bomb_explosion_x_pos_plus_0x100) == 1)
      v2 = CalculateCrystalFlashHdmaDataTablesScaled_OnScreen(kk, 96);
    else
      v2 = CalculateCrystalFlashHdmaDataTablesScaled_RightOfScreen(kk, 96);
  } else {
    v2 = CalculateCrystalFlashHdmaDataTablesScaled_LeftOfScreen(kk, 96);
  }
  int i = k_out;
  do {
    power_bomb_explosion_left_hdma[(uint8)i] = v2;
    power_bomb_explosion_right_hdma[(uint8)i] = GET_HIBYTE(v2);
    i--;
  } while ((i & 0x80) == 0);
  for (uint8 v3 = kk + 1; v3 != 0xc0; v3++) {
    power_bomb_explosion_left_hdma[v3] = 255;
    power_bomb_explosion_right_hdma[v3] = 0;
  }
  int pos = (HIBYTE(power_bomb_explosion_radius) >> 3) * 3;
  reg_COLDATA[0] = kPowerBombExplosionColors[pos + 0] | 0x20;
  reg_COLDATA[1] = kPowerBombExplosionColors[pos + 1] | 0x40;
  reg_COLDATA[2] = kPowerBombExplosionColors[pos + 2] | 0x80;
  power_bomb_explosion_radius += power_bomb_pre_explosion_radius_speed;
  if (power_bomb_explosion_radius < 0x2000) {
    power_bomb_pre_explosion_radius_speed += kPowerBombExplosionRadiusAccel;
  } else {
    hdma_object_instruction_timers[k >> 1] = 1;
    hdma_object_instruction_list_pointers[k >> 1] += 2;
    hdma_object_timers[k >> 1] = 0;
  }
}
