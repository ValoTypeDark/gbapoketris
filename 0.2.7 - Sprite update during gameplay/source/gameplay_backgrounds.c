// Gameplay backgrounds - bitmap implementation
#include "gameplay_backgrounds.h"

extern u16 back_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];

void draw_gameplay_background(GameMode mode) {
    const u16* bg_data = NULL;
    
    // Select the appropriate background based on mode
    switch(mode) {
#ifdef INCLUDE_ROOKIE_BG
        case MODE_ROOKIE:
            bg_data = bg_rookie;
            break;
#endif
            
#ifdef INCLUDE_NORMAL_BG
        case MODE_NORMAL:
            bg_data = bg_normal;
            break;
#endif
            
#ifdef INCLUDE_SUPER_BG
        case MODE_SUPER:
            bg_data = bg_super;
            break;
#endif
            
#ifdef INCLUDE_HYPER_BG
        case MODE_HYPER:
            bg_data = bg_hyper;
            break;
#endif
            
#ifdef INCLUDE_MASTER_BG
        case MODE_MASTER:
            bg_data = bg_master;
            break;
#endif
            
        default:
            // Fallback: clear to black if background not included
            for(int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
                back_buffer[i] = RGB15(0, 0, 0);
            }
            return;
    }
    
    // Fast copy: copy entire background bitmap to back buffer
    // This is a direct memory copy - very fast on GBA
    if(bg_data) {
        // Use 32-bit copies for speed (copy 2 pixels at a time)
        u32* dst32 = (u32*)back_buffer;
        const u32* src32 = (const u32*)bg_data;
        
        // 38400 pixels = 19200 u32 copies
        for(int i = 0; i < 19200; i++) {
            dst32[i] = src32[i];
        }
    }
}
