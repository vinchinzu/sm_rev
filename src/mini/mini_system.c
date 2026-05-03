#include "mini_system.h"

#include <string.h>

#include "mini_asset_bootstrap.h"
#include "mini_ppu_stub.h"
#include "mini_rom_bootstrap.h"
#include "variables.h"

void MiniSystem_Reset(void) {
  memset(g_ram, 0, sizeof(g_ram));
  MiniPpu_Reset();
  MiniAssetBootstrap_Reset();
  MiniRomBootstrap_Reset();
}
