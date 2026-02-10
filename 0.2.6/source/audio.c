#include "audio.h"
#include "main.h"
#include "save.h"
#include <maxmod.h>
#include "soundbank.h"
#include "soundbank_bin.h"

// Volume scaling (0-10 from save system -> 0-1024 for MaxMod)
static u16 music_volume = 1024;
static u16 sfx_volume = 1024;

void audio_init(void) {
    // Initialize MaxMod with 8 channels
    mmInitDefault((mm_addr)soundbank_bin, 8);
    
    // Load volume settings from save system
    u8 music_vol = get_music_volume();  // 0-10
    u8 sfx_vol = get_sfx_volume();      // 0-10
    
    // Convert 0-10 to 0-1024 (MaxMod's volume range)
    music_volume = (music_vol * 1024) / 10;
    sfx_volume = (sfx_vol * 1024) / 10;
    
    // Set initial volumes
    mmSetModuleVolume(music_volume);
    mmSetJingleVolume(sfx_volume);
}

void audio_play_sfx(SoundEffect sfx) {
    // Check if SFX are enabled (volume > 0)
    if(sfx_volume == 0) return;
    
    // MaxMod effect structure
    mm_sound_effect sound;
    sound.rate = 1024;  // Normal playback rate
    sound.handle = 0;   // Auto-assign channel
    sound.volume = (sfx_volume * 255) / 1024;  // Convert to 0-255
    sound.panning = 128; // Center
    
    switch(sfx) {
        case SFX_MENU_MOVE:
            sound.id = SFX_MENU_MOVE_GBA;
            mmEffectEx(&sound);
            break;
            
        case SFX_CLEAR:
            sound.id = SFX_CLEAR_GBA;
            mmEffectEx(&sound);
            break;
            
        case SFX_BIG_CLEAR:
            sound.id = SFX_BIG_CLEAR_GBA;
            mmEffectEx(&sound);
            break;
            
        case SFX_LEVEL_UP:
            sound.id = SFX_LEVEL_UP_GBA;
            mmEffectEx(&sound);
            break;
            
        case SFX_POKEMON_CATCH:
            sound.id = SFX_POKEMON_CATCH_GBA;
            mmEffectEx(&sound);
            break;
            
        case SFX_SHINY:
            sound.id = SFX_SHINY_GBA;
            mmEffectEx(&sound);
            break;
            
        default:
            break;
    }
}

void audio_stop_all(void) {
    mmEffectCancelAll();
}

void audio_update(void) {
    // MaxMod frame update (call once per frame)
    mmFrame();
}

void audio_play_music(int track_id) {
    if(music_volume == 0) return;
    
    // For future music implementation
    // mmStart(track_id, MM_PLAY_LOOP);
}

void audio_stop_music(void) {
    mmStop();
}

void audio_pause_music(void) {
    mmPause();
}

void audio_resume_music(void) {
    mmResume();
}

void audio_set_music_volume(u8 volume) {
    // Clamp to 0-10
    if(volume > 10) volume = 10;
    
    // Convert to MaxMod range (0-1024)
    music_volume = (volume * 1024) / 10;
    
    // Apply to MaxMod
    mmSetModuleVolume(music_volume);
    
    // Save to save system
    set_music_volume(volume);
}

void audio_set_sfx_volume(u8 volume) {
    // Clamp to 0-10
    if(volume > 10) volume = 10;
    
    // Convert to MaxMod range (0-1024)
    sfx_volume = (volume * 1024) / 10;
    
    // Apply to MaxMod
    mmSetJingleVolume(sfx_volume);
    
    // Save to save system
    set_sfx_volume(volume);
}
