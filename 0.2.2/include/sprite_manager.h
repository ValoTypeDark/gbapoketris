#ifndef SPRITE_MANAGER_H
#define SPRITE_MANAGER_H

#include <gba.h>

// Sprite cache configuration
#define SPRITE_CACHE_SIZE 10        // Number of sprites to keep in memory
#define SPRITE_TILE_BASE 0          // Tile base block for sprites
#define SPRITE_PALETTE_BASE 0       // Palette base for sprites

// Sprite data structure
typedef struct {
    const u8* tiles;           // Pointer to tile data
    const u16* palette;        // Pointer to palette data
    u16 width;                 // Sprite width in pixels
    u16 height;                // Sprite height in pixels
    u16 tile_count;            // Number of 8x8 tiles
    u16 palette_size;          // Number of palette entries
    u8 is_loaded;              // Is sprite currently loaded?
    u16 vram_offset;           // Offset in VRAM where tiles are loaded
} SpriteData;

// Sprite cache entry
typedef struct {
    char filename[32];         // Sprite filename (e.g. "0001.png")
    SpriteData data;           // Sprite data
    u32 last_used;             // Frame counter for LRU eviction
} SpriteCacheEntry;

// Initialize sprite system
void init_sprite_system(void);

// Load sprite by filename (e.g. "0001.png" or "0025_shiny.png")
// Returns pointer to sprite data, or NULL if failed
SpriteData* load_sprite(const char* filename);

// Load sprite for current Pokemon
SpriteData* load_current_pokemon_sprite(void);

// Display sprite using OAM (hardware sprites)
void display_sprite_oam(int x, int y, SpriteData* sprite, int oam_index);

// Display sprite on background (software rendering into back_buffer)
// Scales the sprite to fit dest_w x dest_h.  Palette index 0 = transparent.
void display_sprite_bg(int dest_x, int dest_y, SpriteData* sprite,
                        int dest_w, int dest_h);

// Clear sprite from OAM
void clear_sprite_oam(int oam_index);

// Unload least recently used sprites if cache is full
void sprite_cache_cleanup(void);

// Update sprite system (call once per frame)
void update_sprite_system(void);

// Get sprite filename from Pokemon data
const char* get_pokemon_sprite_filename(u16 pokemon_index, u8 is_shiny);

#endif // SPRITE_MANAGER_H
