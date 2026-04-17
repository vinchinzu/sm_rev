#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"

static Func_Y_V *const kSfxHandlers[5] = {  // 0x8289EF
  SfxHandlers_0_SendToApu,
  SfxHandlers_1_WaitForAck,
  SfxHandlers_2_ClearRequest,
  SfxHandlers_3_WaitForAck,
  SfxHandlers_4_Reset,
};

void HandleSoundEffects(void) {
  if ((int8)(sound_handler_downtime - 1) >= 0) {
    LOBYTE(sound_handler_downtime) = sound_handler_downtime - 1;
    RtlApuWrite(APUI01, 0);
    sfx_cur[0] = 0;
    RtlApuWrite(APUI02, 0);
    sfx_cur[1] = 0;
    RtlApuWrite(APUI03, 0);
    sfx_cur[2] = 0;
  } else {
    for (int i = 0; i < 3; ++i)
      kSfxHandlers[sfx_state[i]](i);
  }
}

void SfxHandlers_0_SendToApu(uint16 j) {  // 0x828A2C
  if (sfx_readpos[j] != sfx_writepos[j]) {
    uint8 v2 = sfx_readpos[j] + j * 16;
    uint8 v3 = sfx1_queue[v2];
    RtlApuWrite((SnesRegs)(j + APUI01), v3);
    sfx_cur[j] = v3;
    sfx_readpos[j] = (v2 + 1) & 0xF;
    ++sfx_state[j];
  }
}

void SfxHandlers_1_WaitForAck(uint16 j) {  // 0x828A55
  // Modified from original
  // uint8 v1 = sfx_cur[j];
  ++sfx_state[j];
  sfx_clear_delay[j] = 6;
}

void SfxHandlers_2_ClearRequest(uint16 j) {  // 0x828A6C
  // Modified from original
  if (sfx_clear_delay[j]-- == 1) {
    sfx_cur[j] = 0;
    ++sfx_state[j];
  }
}

void SfxHandlers_3_WaitForAck(uint16 j) {  // 0x828A7C
  // Modified from original
  // uint8 v1 = sfx_cur[j];
  sfx_state[j] = 0;
  SfxHandlers_0_SendToApu(j);
}

void SfxHandlers_4_Reset(uint16 j) {  // 0x828A90
  if (sfx_clear_delay[j]-- == 1)
    sfx_state[j] = 0;
}

void ResetSoundQueues(void) {  // 0x828A9A
  *(uint16 *)sfx_readpos = 0;
  *(uint16 *)&sfx_readpos[2] = 0;
  *(uint16 *)&sfx_writepos[1] = 0;
  *(uint16 *)sfx_state = 0;
  sfx_state[2] = 0;
}

void QueueSamusMovementSfx(void) {  // 0x82BE2F
  if ((speed_boost_counter & 0xFF00) == 1024)
    QueueSfx3_Max6(0x2B);
  if (!sign16(flare_counter - 16))
    QueueSfx1_Max6(0x41);
  CallSomeSamusCode(0x14);
}

