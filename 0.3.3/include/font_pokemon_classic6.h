#ifndef FONT_POKEMON_CLASSIC6_H
#define FONT_POKEMON_CLASSIC6_H

#include <gba_types.h>

#define FONT_POKEMON_CLASSIC6_WIDTH 6
#define FONT_POKEMON_CLASSIC6_HEIGHT 6

extern const u8 font_pokemon_classic6_data[][7];

// Helper function to draw a character
static inline void draw_char_pokemon_classic6(int x, int y, char c, u16 color, u16* buffer, int buffer_width) {
    if(c < 32 || c >= 127) c = 32;
    const u8* glyph = font_pokemon_classic6_data[c - 32];
    for(int row = 0; row < FONT_POKEMON_CLASSIC6_HEIGHT; row++) {
        u8 row_data = glyph[row];
        for(int col = 0; col < FONT_POKEMON_CLASSIC6_WIDTH; col++) {
            if(row_data & (1 << (FONT_POKEMON_CLASSIC6_WIDTH - 1 - col))) {
                int px = x + col;
                int py = y + row;
                if(px >= 0 && px < buffer_width && py >= 0 && py < 160) {
                    buffer[py * buffer_width + px] = color;
                }
            }
        }
    }
}

#endif