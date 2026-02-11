#include "sprite_manager.h"
#include "main.h"
#include "pokemon_database.h"
#include "sprite_lookup.h"
#include <string.h>
#include <stdio.h>

// Global sprite cache
static SpriteCacheEntry sprite_cache[SPRITE_CACHE_SIZE];
static u32 frame_counter = 0;

// External game variable
extern GameData game;

// Parse sprite filename to extract dex number and suffix
static void parse_sprite_filename(const char* filename, u16* dex_num, char* suffix, u8* is_shiny) {
    // Parse: "0025.png" -> dex=25, suffix="", shiny=0
    // Parse: "0003a.png" -> dex=3, suffix="a", shiny=0
    // Parse: "0869ab_shiny.png" -> dex=869, suffix="ab", shiny=1
    
    int num = 0;
    int i = 0;
    
    // Extract number
    while(filename[i] >= '0' && filename[i] <= '9') {
        num = num * 10 + (filename[i] - '0');
        i++;
    }
    *dex_num = num;
    
    // Extract suffix (letters after number)
    int j = 0;
    while(filename[i] >= 'a' && filename[i] <= 'z') {
        suffix[j++] = filename[i++];
    }
    suffix[j] = '\0';
    
    // Check for _shiny
    *is_shiny = (strstr(filename, "_shiny") != NULL) ? 1 : 0;
}

// Find sprite by dex number and form
static const SpriteLookupEntry* find_sprite_by_dex(u16 dex_number, const char* suffix, u8 is_shiny) {
    int i;
    for(i = 0; i < SPRITE_LOOKUP_SIZE; i++) {
        if(SPRITE_LOOKUP[i].dex_number == dex_number &&
           SPRITE_LOOKUP[i].is_shiny == is_shiny) {
            // Check suffix match
            if(suffix == NULL && SPRITE_LOOKUP[i].suffix == NULL) {
                return &SPRITE_LOOKUP[i];
            }
            if(suffix && SPRITE_LOOKUP[i].suffix && 
               strcmp(suffix, SPRITE_LOOKUP[i].suffix) == 0) {
                return &SPRITE_LOOKUP[i];
            }
        }
    }
    return NULL;
}

// Initialize sprite system
void init_sprite_system(void) {
    int i;
    
    // Clear cache
    for(i = 0; i < SPRITE_CACHE_SIZE; i++) {
        sprite_cache[i].filename[0] = '\0';
        sprite_cache[i].data.is_loaded = 0;
        sprite_cache[i].last_used = 0;
    }
    
    frame_counter = 0;
    
    // Mode 3 bitmap mode — OAM sprites are not used.
    // Sprites are software-rendered into the back_buffer via display_sprite_bg().
}

// Find sprite in cache
static SpriteCacheEntry* find_in_cache(const char* filename) {
    int i;
    for(i = 0; i < SPRITE_CACHE_SIZE; i++) {
        if(sprite_cache[i].data.is_loaded && 
           strcmp(sprite_cache[i].filename, filename) == 0) {
            sprite_cache[i].last_used = frame_counter;
            return &sprite_cache[i];
        }
    }
    return NULL;
}

// Find empty cache slot (or least recently used)
static SpriteCacheEntry* find_cache_slot(void) {
    int i;
    
    // First, try to find empty slot
    for(i = 0; i < SPRITE_CACHE_SIZE; i++) {
        if(!sprite_cache[i].data.is_loaded) {
            return &sprite_cache[i];
        }
    }
    
    // No empty slots - find least recently used
    int lru_index = 0;
    u32 oldest_time = sprite_cache[0].last_used;
    
    for(i = 1; i < SPRITE_CACHE_SIZE; i++) {
        if(sprite_cache[i].last_used < oldest_time) {
            oldest_time = sprite_cache[i].last_used;
            lru_index = i;
        }
    }
    
    return &sprite_cache[lru_index];
}

// Load sprite by filename
SpriteData* load_sprite(const char* filename) {
    if(!filename || filename[0] == '\0') {
        return NULL;
    }
    
    // Check cache first
    SpriteCacheEntry* cached = find_in_cache(filename);
    if(cached) {
        return &cached->data;
    }
    
    // Parse filename
    u16 dex_num;
    char suffix[8];
    u8 is_shiny;
    parse_sprite_filename(filename, &dex_num, suffix, &is_shiny);
    
    // Find sprite data
    const SpriteLookupEntry* entry = find_sprite_by_dex(dex_num, suffix[0] ? suffix : NULL, is_shiny);
    if(!entry) {
        return NULL;  // Sprite not found
    }
    
    // Get cache slot
    SpriteCacheEntry* slot = find_cache_slot();
    
    // Fill sprite data
    slot->data.tiles = (const u8*)entry->tiles;
    slot->data.palette = entry->palette;
    slot->data.width = 64;
    slot->data.height = 64;
    slot->data.tile_count = 64;  // 8x8 tiles for 64x64
    slot->data.palette_size = 256;
    slot->data.is_loaded = 1;
    slot->data.vram_offset = 0;
    
    // Copy filename
    strncpy(slot->filename, filename, 31);
    slot->filename[31] = '\0';
    slot->last_used = frame_counter;
    
    return &slot->data;
}

// Load sprite for current Pokemon (uses Pokemon database directly!)
SpriteData* load_current_pokemon_sprite(void) {
    const char* filename = get_pokemon_sprite_filename(
        game.current_pokemon, 
        game.is_shiny
    );
    
    return load_sprite(filename);
}

// Display sprite using OAM — NO-OP in Mode 3.
// Kept as a stub so any remaining call sites link cleanly.
// Use display_sprite_bg() instead.
void display_sprite_oam(int x, int y, SpriteData* sprite, int oam_index) {
    (void)x; (void)y; (void)sprite; (void)oam_index;
}

// Clear sprite from OAM (no-op in Mode 3, kept for API compat)
void clear_sprite_oam(int oam_index) {
    (void)oam_index;
}

/* ── Software sprite blit into back_buffer ─────────────────────────────────
 * Decodes 8bpp GBA-tiled sprite data and draws it scaled into the
 * back_buffer at (dest_x, dest_y) with size (dest_w × dest_h).
 * Palette index 0 is treated as transparent.
 *
 * Tile layout (grit 8bpp, 1D mapping):
 *   Each 8×8 tile = 64 bytes, one byte per pixel (palette index).
 *   For a 64×64 sprite there are 64 tiles laid out in row-major order:
 *     tile_index = (tile_row * tiles_per_row) + tile_col
 *   tiles_per_row = sprite->width / 8
 */
void display_sprite_bg(int dest_x, int dest_y, SpriteData* sprite,
                        int dest_w, int dest_h) {
    if(!sprite || !sprite->is_loaded) return;

    extern u16 back_buffer[];   /* defined in graphics.c (EWRAM) */

    const u8*  tiles   = sprite->tiles;
    const u16* palette = sprite->palette;
    int src_w          = sprite->width;   /* 64 */
    int src_h          = sprite->height;  /* 64 */
    int tiles_per_row  = src_w / 8;       /* 8  */

    /* Pre-compute source pixel indices — kills all divides from the inner loop.
     * dest_w / dest_h are small (≤64) so these fit on the stack easily. */
    u8 src_x_map[64];
    u8 src_y_map[64];
    {
        int i;
        for(i = 0; i < dest_w; i++) src_x_map[i] = (u8)((i * src_w) / dest_w);
        for(i = 0; i < dest_h; i++) src_y_map[i] = (u8)((i * src_h) / dest_h);
    }

    /* Clamp vertical range to screen */
    int dy_start = 0, dy_end = dest_h;
    if(dest_y < 0)                  dy_start = -dest_y;
    if(dest_y + dest_h > SCREEN_HEIGHT) dy_end = SCREEN_HEIGHT - dest_y;

    /* Clamp horizontal range to screen */
    int dx_start = 0, dx_end = dest_w;
    if(dest_x < 0)                  dx_start = -dest_x;
    if(dest_x + dest_w > SCREEN_WIDTH)  dx_end = SCREEN_WIDTH - dest_x;

    int dy, dx;
    for(dy = dy_start; dy < dy_end; dy++) {
        int py = dest_y + dy;
        int src_y    = src_y_map[dy];
        int tile_row = src_y >> 3;          /* / 8 */
        int pix_row  = src_y & 7;           /* % 8 */
        int row_base = py * SCREEN_WIDTH;

        for(dx = dx_start; dx < dx_end; dx++) {
            int src_x    = src_x_map[dx];
            int tile_col = src_x >> 3;
            int pix_col  = src_x & 7;

            int tile_index = tile_row * tiles_per_row + tile_col;
            u8  pal_index  = tiles[(tile_index << 6) + (pix_row << 3) + pix_col];

            if(pal_index == 0) continue;   /* transparent */

            back_buffer[row_base + dest_x + dx] = palette[pal_index];
        }
    }
}

// Update sprite system
void update_sprite_system(void) {
    frame_counter++;
}

// Get sprite filename from Pokemon data (uses database!)
const char* get_pokemon_sprite_filename(u16 pokemon_index, u8 is_shiny) {
    if(pokemon_index >= TOTAL_POKEMON) {
        return "0001.png";
    }
    
    const PokemonData* pdata = get_pokemon_data(pokemon_index);
    
    if(is_shiny) {
        return pdata->sprite_shiny;
    } else {
        return pdata->sprite_filename;
    }
}
