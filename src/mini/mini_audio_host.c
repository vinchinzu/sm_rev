// Host-side SDL audio device for the mini runtime. Kept out of
// libsm_rev_mini_kernel.a so headless hosts (rollback test, Rust host,
// browser bridge) link the kernel without SDL.
#include "mini_audio_host.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "mini_audio.h"
#include "sm_rtl.h"

static SDL_mutex *g_mini_audio_mutex;
static SDL_AudioDeviceID g_mini_audio_device;
static int g_mini_audio_frames_per_block;
static uint8 *g_mini_audio_buffer;
static uint8 *g_mini_audio_buffer_cur;
static uint8 *g_mini_audio_buffer_end;

static void MiniAudioHost_Lock(void) {
  SDL_LockMutex(g_mini_audio_mutex);
}

static void MiniAudioHost_Unlock(void) {
  SDL_UnlockMutex(g_mini_audio_mutex);
}

static void MiniAudioHost_Callback(void *userdata, uint8 *stream, int len) {
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
  MiniAudio_SetLockHooks(MiniAudioHost_Lock, MiniAudioHost_Unlock);
  if (g_mini_audio_device != 0)
    return true;

  SDL_AudioSpec want = {0};
  SDL_AudioSpec have = {0};
  want.freq = kMiniAudioFreq;
  want.format = AUDIO_S16;
  want.channels = kMiniAudioChannels;
  want.samples = kMiniAudioSamples;
  want.callback = MiniAudioHost_Callback;

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
  MiniAudio_SetLockHooks(NULL, NULL);
  if (g_mini_audio_mutex != NULL) {
    SDL_DestroyMutex(g_mini_audio_mutex);
    g_mini_audio_mutex = NULL;
  }
  MiniAudio_ResetRoomMusicBoot();
}
