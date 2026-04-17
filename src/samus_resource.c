#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "sm_rtl.h"

void Samus_RestoreHealth(uint16 a) {  // 0x91DF12
  uint16 v1 = a + samus_health;
  samus_health = v1;
  if ((int16)(v1 - samus_max_health) >= 0) {
    uint16 v2 = samus_reserve_health + v1 - samus_max_health;
    if ((int16)(v2 - samus_max_reserve_health) >= 0)
      v2 = samus_max_reserve_health;
    samus_reserve_health = v2;
    if (v2) {
      if (!reserve_health_mode)
        reserve_health_mode = 1;
    }
    samus_health = samus_max_health;
  }
}

void Samus_DealDamage(uint16 a) {  // 0x91DF51
  if ((a & 0x8000) == 0) {
    if (a != 300 && !time_is_frozen_flag) {
      samus_health -= a;
      if ((samus_health & 0x8000) != 0)
        samus_health = 0;
    }
  } else {
    InvalidInterrupt_Crash();
  }
}

void Samus_RestoreMissiles(uint16 a) {  // 0x91DF80
  uint16 v1 = samus_missiles + a;
  samus_missiles = v1;
  if ((int16)(v1 - samus_max_missiles) >= 0) {
    if (sign16(samus_max_missiles - 99)) {
      samus_reserve_missiles += v1 - samus_max_missiles;
      if ((int16)(samus_reserve_missiles - samus_max_missiles) >= 0)
        samus_reserve_missiles = samus_max_missiles;
    } else {
      samus_reserve_missiles += v1 - samus_max_missiles;
      if (!sign16(samus_reserve_missiles - 99))
        samus_reserve_missiles = 99;
    }
    samus_missiles = samus_max_missiles;
  }
}

void Samus_RestoreSuperMissiles(uint16 a) {  // 0x91DFD3
  uint16 v1 = samus_super_missiles + a;
  samus_super_missiles = v1;
  if ((int16)(v1 - samus_max_super_missiles) >= 0 && v1 != samus_max_super_missiles)
    samus_super_missiles = samus_max_super_missiles;
}

void Samus_RestorePowerBombs(uint16 a) {  // 0x91DFF0
  uint16 v1 = samus_power_bombs + a;
  samus_power_bombs = v1;
  if ((int16)(v1 - samus_max_power_bombs) >= 0 && v1 != samus_max_power_bombs)
    samus_power_bombs = samus_max_power_bombs;
}
