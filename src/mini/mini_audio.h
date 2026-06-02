#ifndef SM_MINI_AUDIO_H_
#define SM_MINI_AUDIO_H_

#include "types.h"

bool MiniAudio_Init(void);
void MiniAudio_Shutdown(void);
void MiniAudio_BootRoomMusic(void);
void MiniAudio_TickQueues(bool music_already_ticked);

#endif  // SM_MINI_AUDIO_H_
