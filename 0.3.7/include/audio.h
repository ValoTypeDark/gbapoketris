#ifndef AUDIO_H
#define AUDIO_H

#include <gba_types.h>

void audio_init(void);
void audio_play_sfx(const s8* data, u32 size);
void audio_play_music(const s8* data, u32 size);
void audio_stop_music(void);
void audio_update(void);    // Call once per VBlank to handle music looping

#endif
