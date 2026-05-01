// Torizo finish-explosion instruction handlers.
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "torizo_config.h"

static void Torizo_SpawnFinishExplosionBurst(uint16 projectile_id, uint16 init_param, uint16 count) {
  for (uint16 i = 0; i < count; i++)
    SpawnEprojWithRoomGfx(projectile_id, init_param);
}

const uint16 *Torizo_Instr_30(uint16 k, const uint16 *jp) {  // 0xAAC303
  Torizo_SpawnFinishExplosionBurst(addr_kEproj_BombTorizoLowHealthExplode, jp[0],
                                   TorizoConfig_FinishExplosionBodyBurstCount());

  Enemy_Torizo *E = Get_Torizo(k);
  uint16 flash_frames = TorizoConfig_FinishExplosionFlashFrames();
  E->base.current_instruction = INSTR_ADDR_TO_PTR(k, jp + 1);
  E->base.flash_timer = flash_frames;
  E->base.instruction_timer = flash_frames;
  return 0;
}

const uint16 *Torizo_Instr_34(uint16 k, const uint16 *jp) {  // 0xAAC32F
  Torizo_SpawnFinishExplosionBurst(addr_kEproj_BombTorizoDeathExplosion, 0,
                                   TorizoConfig_FinishExplosionFinalBurstCount());

  Enemy_Torizo *E = Get_Torizo(k);
  E->base.current_instruction = INSTR_ADDR_TO_PTR(k, jp);
  E->base.flash_timer = 1;
  E->base.instruction_timer = 1;
  return 0;
}
