#include <stdio.h>
#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"

#define kMusicPointers (*(LongPtr*)RomFixedPtr(0x8fe7e1))

enum {
  kMusicQueueLastSlot = 14,
  kMusicQueueMask = 0xE,
  kMinMusicDelay = 8,
  kSfxQueueLen = 16,
};

static Func_Y_V *const kSfxHandlers[5] = {  // 0x8289EF
  SfxHandlers_0_SendToApu,
  SfxHandlers_1_WaitForAck,
  SfxHandlers_2_ClearRequest,
  SfxHandlers_3_WaitForAck,
  SfxHandlers_4_Reset,
};

static void AdvanceMusicQueueReadPos(void) {
  int slot = music_queue_read_pos;
  music_queue_track[slot >> 1] = 0;
  music_queue_delay[slot >> 1] = 0;
  music_queue_read_pos = (slot + 2) & kMusicQueueMask;
}

static void QueueSfx_Internal(uint16 packed_request, uint16 channel, uint8 *queue) {
  sfx_max_queued[channel] = packed_request;
  if (((sfx_writepos[channel] - sfx_readpos[channel]) & (kSfxQueueLen - 1)) >= (uint8)packed_request)
    return;

  if (debug_disable_sounds
      || game_state >= kGameState_40_TransitionToDemo
      || (power_bomb_explosion_status & 0x8000) != 0) {
    return;
  }

  uint8 sfx_id = GET_HIBYTE(packed_request);
  uint8 write_pos = sfx_writepos[channel];
  uint8 next_write_pos = (write_pos + 1) & (kSfxQueueLen - 1);
  if (next_write_pos == sfx_readpos[channel]) {
    if (sfx_id < queue[write_pos])
      queue[write_pos] = sfx_id;
    return;
  }

  queue[write_pos] = sfx_id;
  sfx_writepos[channel] = next_write_pos;
  queue[next_write_pos] = 0;
}

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

uint8 HasQueuedMusic(void) {  // 0x808EF4
  for (int slot = kMusicQueueLastSlot; slot >= 0; slot -= 2) {
    if (music_queue_delay[slot >> 1])
      return 1;
  }
  return 0;
}

void HandleMusicQueue(void) {  // 0x808F0C
  bool timer_expired = music_timer-- == 1;
  if ((music_timer & 0x8000) == 0) {
    if (!timer_expired)
      return;

    if ((music_entry & 0x8000) != 0) {
      if (g_debug_flag) {
        fprintf(stderr,
                "MUSIC apply upload set=0x%02X room=0x%04X state=0x%04X cur=%u/%u queue_r=%u queue_w=%u\n",
                music_entry & 0xff, room_ptr, roomdefroomstate_ptr, music_data_index, music_track_index,
                music_queue_read_pos, music_queue_write_pos);
      }
      music_data_index = (uint8)music_entry;
      cur_music_track = -1;
      APU_UploadBank(Load24((LongPtr *)((uint8 *)&kMusicPointers + (uint8)music_entry)));
      cur_music_track = 0;
      AdvanceMusicQueueReadPos();
      sound_handler_downtime = 8;
      return;
    }

    uint8 track = music_entry & 0x7F;
    if (g_debug_flag) {
      fprintf(stderr,
              "MUSIC apply track=%u room=0x%04X state=0x%04X cur=%u/%u queue_r=%u queue_w=%u\n",
              track, room_ptr, roomdefroomstate_ptr, music_data_index, music_track_index,
              music_queue_read_pos, music_queue_write_pos);
    }
    music_track_index = track;
    RtlApuWrite(APUI00, track);
    cur_music_track = track;
    sound_handler_downtime = 8;
    AdvanceMusicQueueReadPos();
  }

  if (music_queue_read_pos == music_queue_write_pos) {
    music_timer = 0;
  } else {
    int slot = music_queue_read_pos >> 1;
    music_entry = music_queue_track[slot];
    music_timer = music_queue_delay[slot];
  }
}

void QueueMusic_Delayed8(uint16 a) {  // 0x808FC1
  if (game_state < kGameState_40_TransitionToDemo
      && ((music_queue_write_pos + 2) & kMusicQueueMask) != music_queue_read_pos) {
    if (g_debug_flag) {
      fprintf(stderr,
              "MUSIC queue8 raw=0x%04X room=0x%04X state=0x%04X cur=%u/%u queue_r=%u queue_w=%u\n",
              a, room_ptr, roomdefroomstate_ptr, music_data_index, music_track_index,
              music_queue_read_pos, music_queue_write_pos);
    }
    int slot = music_queue_write_pos >> 1;
    music_queue_track[slot] = a;
    music_queue_delay[slot] = kMinMusicDelay;
    music_queue_write_pos = (music_queue_write_pos + 2) & kMusicQueueMask;
  }
}

void QueueMusic_DelayedY(uint16 a, uint16 delay) {  // 0x808FF7
  if (game_state < kGameState_40_TransitionToDemo) {
    if (g_debug_flag) {
      fprintf(stderr,
              "MUSIC queueY raw=0x%04X delay=%u room=0x%04X state=0x%04X cur=%u/%u queue_r=%u queue_w=%u\n",
              a, delay, room_ptr, roomdefroomstate_ptr, music_data_index, music_track_index,
              music_queue_read_pos, music_queue_write_pos);
    }
    int slot = music_queue_write_pos >> 1;
    music_queue_track[slot] = a;
    music_queue_delay[slot] = delay < kMinMusicDelay ? kMinMusicDelay : delay;
    music_queue_write_pos = (music_queue_write_pos + 2) & kMusicQueueMask;
  }
}

void QueueSfx1_Max15(uint16 a) {  // 0x809021
  QueueSfx1_Internal(a << 8 | 15);
}

void QueueSfx1_Max9(uint16 a) {  // 0x80902B
  QueueSfx1_Internal(a << 8 | 9);
}

void QueueSfx1_Max3(uint16 a) {  // 0x809035
  QueueSfx1_Internal(a << 8 | 3);
}

void QueueSfx1_Max1(uint16 a) {  // 0x80903F
  QueueSfx1_Internal(a << 8 | 1);
}

void QueueSfx1_Max6(uint16 a) {  // 0x809049
  QueueSfx1_Internal(a << 8 | 6);
}

void QueueSfx1_Internal(uint16 a) {  // 0x809051
  QueueSfx_Internal(a, 0, sfx1_queue);
}

void QueueSfx2_Max15(uint16 a) {  // 0x8090A3
  QueueSfx2_Internal(a << 8 | 15);
}

void QueueSfx2_Max9(uint16 a) {  // 0x8090AD
  QueueSfx2_Internal(a << 8 | 9);
}

void QueueSfx2_Max3(uint16 a) {  // 0x8090B7
  QueueSfx2_Internal(a << 8 | 3);
}

void QueueSfx2_Max1(uint16 a) {  // 0x8090C1
  QueueSfx2_Internal(a << 8 | 1);
}

void QueueSfx2_Max6(uint16 a) {  // 0x8090CB
  QueueSfx2_Internal(a << 8 | 6);
}

void QueueSfx2_Internal(uint16 a) {  // 0x8090D3
  QueueSfx_Internal(a, 1, sfx2_queue);
}

void QueueSfx3_Max15(uint16 a) {  // 0x809125
  QueueSfx3_Internal(a << 8 | 15);
}

void QueueSfx3_Max9(uint16 a) {  // 0x80912F
  QueueSfx3_Internal(a << 8 | 9);
}

void QueueSfx3_Max3(uint16 a) {  // 0x809139
  QueueSfx3_Internal(a << 8 | 3);
}

void QueueSfx3_Max1(uint16 a) {  // 0x809143
  QueueSfx3_Internal(a << 8 | 1);
}

void QueueSfx3_Max6(uint16 a) {  // 0x80914D
  QueueSfx3_Internal(a << 8 | 6);
}

void QueueSfx3_Internal(uint16 a) {  // 0x809155
  QueueSfx_Internal(a, 2, sfx3_queue);
}

void QueueSamusMovementSfx(void) {  // 0x82BE2F
  if ((speed_boost_counter & 0xFF00) == 1024)
    QueueSfx3_Max6(0x2B);
  if (!sign16(flare_counter - 16))
    QueueSfx1_Max6(0x41);
  CallSomeSamusCode(0x14);
}
