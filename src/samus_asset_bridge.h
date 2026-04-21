#ifndef SM_SAMUS_ASSET_BRIDGE_H_
#define SM_SAMUS_ASSET_BRIDGE_H_

#include <stddef.h>

#include "types.h"

enum {
  kSamusAssetBridgeBank92Size = 0x8000,
};

typedef struct SamusAssetBridgeRange {
  uint32 snes_address;
  uint32 size;
  uint32 data_offset;
} SamusAssetBridgeRange;

void SamusAssetBridge_Reset(void);
bool SamusAssetBridge_Install(const uint8 *bank92, size_t bank92_size,
                              const uint8 *data, size_t data_size,
                              const SamusAssetBridgeRange *ranges, int range_count);
bool SamusAssetBridge_IsLoaded(void);
const uint8 *SamusAssetBridge_GetBank92(uint16 addr);
const uint8 *SamusAssetBridge_GetData(uint32 snes_address, size_t size);

#endif  // SM_SAMUS_ASSET_BRIDGE_H_
