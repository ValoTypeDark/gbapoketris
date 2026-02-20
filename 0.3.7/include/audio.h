#ifndef AUDIO_H
#define AUDIO_H

#include <gba_types.h>

void audio_init(void);
void audio_play_sfx(const s8* data, u32 size);

#endif
