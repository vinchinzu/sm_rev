#ifndef SM_MINI_AUDIO_H_
#define SM_MINI_AUDIO_H_

#include "types.h"

enum {
  kMiniAudioFreq = 44100,
  kMiniAudioChannels = 2,
  kMiniAudioSamples = 2048,
};

// Kernel-side APU bridge (no SDL). Hosts that render audio from another
// thread must install lock hooks; headless hosts can leave them unset.
void MiniAudio_SetLockHooks(void (*lock)(void), void (*unlock)(void));
bool MiniAudio_EnsureSpcPlayer(void);
void MiniAudio_ResetRoomMusicBoot(void);
void MiniAudio_BootRoomMusic(void);
void MiniAudio_TickQueues(bool music_already_ticked);

#endif  // SM_MINI_AUDIO_H_
