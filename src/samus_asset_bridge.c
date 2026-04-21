#include "samus_asset_bridge.h"

#include <stdlib.h>
#include <string.h>

#include "sm_rtl.h"

static bool g_samus_asset_bridge_loaded;
static uint8 g_samus_bank92[kSamusAssetBridgeBank92Size];
static uint8 *g_samus_data;
static SamusAssetBridgeRange *g_samus_ranges;
static int g_samus_range_count;
static size_t g_samus_data_size;

void SamusAssetBridge_Reset(void) {
  free(g_samus_data);
  free(g_samus_ranges);
  g_samus_data = NULL;
  g_samus_ranges = NULL;
  g_samus_range_count = 0;
  g_samus_data_size = 0;
  g_samus_asset_bridge_loaded = false;
  memset(g_samus_bank92, 0, sizeof(g_samus_bank92));
}

bool SamusAssetBridge_Install(const uint8 *bank92, size_t bank92_size,
                              const uint8 *data, size_t data_size,
                              const SamusAssetBridgeRange *ranges, int range_count) {
  SamusAssetBridge_Reset();
  if (bank92 == NULL || data == NULL || ranges == NULL || range_count <= 0 || data_size == 0)
    return false;
  if (bank92_size != kSamusAssetBridgeBank92Size)
    return false;

  uint8 *copied_data = (uint8 *)malloc(data_size);
  SamusAssetBridgeRange *copied_ranges =
      (SamusAssetBridgeRange *)malloc((size_t)range_count * sizeof(*copied_ranges));
  if (copied_data == NULL || copied_ranges == NULL) {
    free(copied_data);
    free(copied_ranges);
    return false;
  }

  memcpy(g_samus_bank92, bank92, sizeof(g_samus_bank92));
  memcpy(copied_data, data, data_size);
  memcpy(copied_ranges, ranges, (size_t)range_count * sizeof(*copied_ranges));
  g_samus_data = copied_data;
  g_samus_ranges = copied_ranges;
  g_samus_range_count = range_count;
  g_samus_data_size = data_size;
  g_samus_asset_bridge_loaded = true;
  return true;
}

bool SamusAssetBridge_IsLoaded(void) {
  return g_samus_asset_bridge_loaded;
}

const uint8 *SamusAssetBridge_GetBank92(uint16 addr) {
  if (g_samus_asset_bridge_loaded) {
    if (addr < 0x8000)
      return NULL;
    return g_samus_bank92 + (addr - 0x8000);
  }
  return RomPtr_92(addr);
}

const uint8 *SamusAssetBridge_GetData(uint32 snes_address, size_t size) {
  if (g_samus_asset_bridge_loaded) {
    for (int i = 0; i < g_samus_range_count; i++) {
      const SamusAssetBridgeRange *range = &g_samus_ranges[i];
      if (snes_address < range->snes_address)
        continue;
      uint32 delta = snes_address - range->snes_address;
      if (delta > range->size || size > range->size - delta)
        continue;
      if ((size_t)range->data_offset + delta + size > g_samus_data_size)
        return NULL;
      return g_samus_data + range->data_offset + delta;
    }
  }
  return RomPtr(snes_address);
}
