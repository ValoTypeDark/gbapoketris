// Gameplay backgrounds - OPTIMIZED with static caching
#include "gameplay_backgrounds.h"

extern u16 back_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];

// OPTIMIZATION 1: Static cache to avoid redundant mode selection
static u16 bg_cache[SCREEN_WIDTH * SCREEN_HEIGHT] __attribute__((section(".ewram")));
static int cache_valid = 0;
static GameMode cached_mode = MODE_ROOKIE;

void draw_gameplay_background(GameMode mode) {
    // Check if we need to rebuild cache
    if(!cache_valid || cached_mode != mode) {
        const u16* bg_data = NULL;
        
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
#ifdef INCLUDE_UNOWN_BG
            case MODE_UNOWN:
                bg_data = bg_unown_bitmap;
                break;
#endif
#ifdef INCLUDE_VIVILLON_BG
            case MODE_VIVILLON:
                bg_data = bg_vivillon_bitmap;
                break;
#endif
#ifdef INCLUDE_ALCREMIE_BG
            case MODE_ALCREMIE:
                bg_data = bg_alcremie_bitmap;
                break;
#endif
            default:
                for(int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
                    bg_cache[i] = RGB15(0, 0, 0);
                }
                cache_valid = 1;
                cached_mode = mode;
                u32* dst32 = (u32*)back_buffer;
                const u32* src32 = (const u32*)bg_cache;
                for(int i = 0; i < 19200; i++) {
                    dst32[i] = src32[i];
                }
                return;
        }
        
        if(bg_data) {
            u32* dst32 = (u32*)bg_cache;
            const u32* src32 = (const u32*)bg_data;
            for(int i = 0; i < 19200; i++) {
                dst32[i] = src32[i];
            }
        }
        
        cache_valid = 1;
        cached_mode = mode;
    }
    
    // Fast copy from cache to back buffer
    u32* dst32 = (u32*)back_buffer;
    const u32* src32 = (const u32*)bg_cache;
    for(int i = 0; i < 19200; i++) {
        dst32[i] = src32[i];
    }
}