// Kernel-side APU bridge: SPC player ownership, RTL APU shims, and music/sfx
// queue stepping. Must stay free of SDL/host dependencies so it can live in
// libsm_rev_mini_kernel.a; the SDL device half is mini_audio_host.c.
#include "mini_audio.h"

#include <string.h>

#include "funcs.h"
#include "sm_rtl.h"
#include "spc_player.h"
#include "variables.h"

static void (*g_mini_audio_lock)(void);
static void (*g_mini_audio_unlock)(void);
static bool g_mini_audio_booted_room_music;

void MiniAudio_SetLockHooks(void (*lock)(void), void (*unlock)(void)) {
  g_mini_audio_lock = lock;
  g_mini_audio_unlock = unlock;
}

void RtlApuLock(void) {
  if (g_mini_audio_lock != NULL)
    g_mini_audio_lock();
}

void RtlApuUnlock(void) {
  if (g_mini_audio_unlock != NULL)
    g_mini_audio_unlock();
}

void RtlApuWrite(uint32 adr, uint8 val) {
  if (g_spc_player == NULL)
    return;
  RtlApuLock();
  g_spc_player->input_ports[adr & 3] = val;
  RtlApuUnlock();
}

void RtlApuUpload(const uint8 *p) {
  if (g_spc_player == NULL || p == NULL)
    return;
  RtlApuLock();
  SpcPlayer_Upload(g_spc_player, p);
  RtlApuUnlock();
}

void RtlPushApuState(void) {
}

void RtlRenderAudio(int16 *audio_buffer, int samples, int channels) {
  if (audio_buffer == NULL || samples <= 0 || channels != kMiniAudioChannels ||
      g_spc_player == NULL) {
    if (audio_buffer != NULL && samples > 0 && channels > 0)
      memset(audio_buffer, 0, (size_t)samples * (size_t)channels * sizeof(*audio_buffer));
    return;
  }

  RtlApuLock();
  SpcPlayer_GenerateSamples(g_spc_player);
  dsp_getSamples(g_spc_player->dsp, audio_buffer, samples);
  RtlApuUnlock();
}

bool MiniAudio_EnsureSpcPlayer(void) {
  if (g_spc_player != NULL)
    return true;
  g_spc_player = SpcPlayer_Create();
  if (g_spc_player == NULL)
    return false;
  SpcPlayer_Initialize(g_spc_player);
  g_use_my_apu_code = true;
  return true;
}

void MiniAudio_ResetRoomMusicBoot(void) {
  g_mini_audio_booted_room_music = false;
}

void MiniAudio_BootRoomMusic(void) {
  if (g_spc_player == NULL || !g_use_my_apu_code || g_mini_audio_booted_room_music)
    return;
  g_mini_audio_booted_room_music = true;

  APU_UploadBank(0xCF8000);
  RtlApuWrite(APUI00, 0);
  RtlApuWrite(APUI01, 0);
  RtlApuWrite(APUI02, 0);
  RtlApuWrite(APUI03, 0);
  music_timer = 0;
  music_queue_read_pos = 0;
  music_queue_write_pos = 0;
  for (int i = 0; i < 8; i++) {
    music_queue_track[i] = 0;
    music_queue_delay[i] = 0;
  }
  ResetSoundQueues();
  debug_disable_sounds = 0;
  sound_handler_downtime = 0;

  if (room_music_data_index != 0) {
    QueueMusic_Delayed8(0);
    QueueMusic_Delayed8(room_music_data_index | 0xFF00);
    if (room_music_track_index != 0)
      QueueMusic_DelayedY(room_music_track_index, 6);
  }
}

void MiniAudio_TickQueues(bool music_already_ticked) {
  if (!music_already_ticked)
    HandleMusicQueue();
  HandleSoundEffects();
}
