#include "audio.h"
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
    
    mm_sfxhand handle;
    
    switch(sfx) {
        case SFX_MENU_MOVE:
            mmEffect(SFX_MENUMOVE);
            break;
            
        case SFX_CLEAR:
            mmEffect(SFX_CLEAR);
            break;
            
        case SFX_BIG_CLEAR:
            mmEffect(SFX_BIGCLEAR);
            break;
            
        case SFX_LEVEL_UP:
            mmEffect(SFX_LEVELUP);
            break;
            
        case SFX_POKEMON_CATCH:
            mmEffect(SFX_POKEMONCATCH);
            break;
            
        case SFX_SHINY:
            mmEffect(SFX_SHINY);
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
