#ifndef AUDIO_H
#define AUDIO_H

#include <gba_base.h>

// Sound effect IDs
typedef enum {
    SFX_MENU_MOVE = 0,
    SFX_CLEAR,
    SFX_BIG_CLEAR,
    SFX_LEVEL_UP,
    SFX_POKEMON_CATCH,
    SFX_SHINY,
    SFX_COUNT
} SoundEffect;

// Initialize audio system
void audio_init(void);

// Play a sound effect
void audio_play_sfx(SoundEffect sfx);

// Stop all sounds
void audio_stop_all(void);

// Update audio (call once per frame)
void audio_update(void);

// Music control (for future implementation)
void audio_play_music(int track_id);
void audio_stop_music(void);
void audio_pause_music(void);
void audio_resume_music(void);

// Volume control (0-10)
void audio_set_music_volume(u8 volume);
void audio_set_sfx_volume(u8 volume);

#endif // AUDIO_H
