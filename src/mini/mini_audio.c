#include "mini_audio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "funcs.h"
#include "sm_rtl.h"
#include "spc_player.h"
#include "variables.h"

enum {
  kMiniAudioFreq = 44100,
  kMiniAudioChannels = 2,
  kMiniAudioSamples = 2048,
};

static SDL_mutex *g_mini_audio_mutex;
static SDL_AudioDeviceID g_mini_audio_device;
static int g_mini_audio_frames_per_block;
static uint8 *g_mini_audio_buffer;
static uint8 *g_mini_audio_buffer_cur;
static uint8 *g_mini_audio_buffer_end;
static bool g_mini_audio_booted_room_music;

void RtlApuLock(void) {
  if (g_mini_audio_mutex != NULL)
    SDL_LockMutex(g_mini_audio_mutex);
}

void RtlApuUnlock(void) {
  if (g_mini_audio_mutex != NULL)
    SDL_UnlockMutex(g_mini_audio_mutex);
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

static bool MiniAudio_EnsureSpcPlayer(void) {
  if (g_spc_player != NULL)
    return true;
  g_spc_player = SpcPlayer_Create();
  if (g_spc_player == NULL)
    return false;
  SpcPlayer_Initialize(g_spc_player);
  g_use_my_apu_code = true;
  return true;
}

static void MiniAudio_Callback(void *userdata, uint8 *stream, int len) {
  (void)userdata;
  if (stream == NULL || len <= 0)
    return;
  memset(stream, 0, (size_t)len);
  if (g_mini_audio_buffer == NULL || g_mini_audio_frames_per_block <= 0)
    return;

  uint8 *out = stream;
  int remaining = len;
  while (remaining > 0) {
    if (g_mini_audio_buffer_cur == g_mini_audio_buffer_end) {
      RtlRenderAudio((int16 *)g_mini_audio_buffer, g_mini_audio_frames_per_block,
                     kMiniAudioChannels);
      g_mini_audio_buffer_cur = g_mini_audio_buffer;
      g_mini_audio_buffer_end =
          g_mini_audio_buffer +
          g_mini_audio_frames_per_block * kMiniAudioChannels * (int)sizeof(int16);
    }

    int available = (int)(g_mini_audio_buffer_end - g_mini_audio_buffer_cur);
    int copy = available < remaining ? available : remaining;
    memcpy(out, g_mini_audio_buffer_cur, (size_t)copy);
    out += copy;
    g_mini_audio_buffer_cur += copy;
    remaining -= copy;
  }
}

bool MiniAudio_Init(void) {
  if (!MiniAudio_EnsureSpcPlayer()) {
    fprintf(stderr, "mini: audio disabled: failed to create SPC player\n");
    return false;
  }
  if (g_mini_audio_mutex == NULL) {
    g_mini_audio_mutex = SDL_CreateMutex();
    if (g_mini_audio_mutex == NULL) {
      fprintf(stderr, "mini: audio disabled: SDL_CreateMutex failed: %s\n", SDL_GetError());
      return false;
    }
  }
  if (g_mini_audio_device != 0)
    return true;

  SDL_AudioSpec want = {0};
  SDL_AudioSpec have = {0};
  want.freq = kMiniAudioFreq;
  want.format = AUDIO_S16;
  want.channels = kMiniAudioChannels;
  want.samples = kMiniAudioSamples;
  want.callback = MiniAudio_Callback;

  g_mini_audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
  if (g_mini_audio_device == 0) {
    fprintf(stderr, "mini: audio disabled: SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
    return false;
  }

  g_mini_audio_frames_per_block = (534 * have.freq) / 32000;
  g_mini_audio_buffer = (uint8 *)malloc(
      (size_t)g_mini_audio_frames_per_block * kMiniAudioChannels * sizeof(int16));
  if (g_mini_audio_buffer == NULL) {
    fprintf(stderr, "mini: audio disabled: failed to allocate audio buffer\n");
    SDL_CloseAudioDevice(g_mini_audio_device);
    g_mini_audio_device = 0;
    return false;
  }
  g_mini_audio_buffer_cur = g_mini_audio_buffer;
  g_mini_audio_buffer_end = g_mini_audio_buffer;
  SDL_PauseAudioDevice(g_mini_audio_device, 0);
  return true;
}

void MiniAudio_Shutdown(void) {
  if (g_mini_audio_device != 0) {
    SDL_PauseAudioDevice(g_mini_audio_device, 1);
    SDL_CloseAudioDevice(g_mini_audio_device);
    g_mini_audio_device = 0;
  }
  free(g_mini_audio_buffer);
  g_mini_audio_buffer = NULL;
  g_mini_audio_buffer_cur = NULL;
  g_mini_audio_buffer_end = NULL;
  g_mini_audio_frames_per_block = 0;
  if (g_mini_audio_mutex != NULL) {
    SDL_DestroyMutex(g_mini_audio_mutex);
    g_mini_audio_mutex = NULL;
  }
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
