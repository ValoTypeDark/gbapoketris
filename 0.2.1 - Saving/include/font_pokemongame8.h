#ifndef FONT_POKEMONGAME8_H
#define FONT_POKEMONGAME8_H

#include <gba_types.h>

#define FONT_POKEMONGAME8_WIDTH 7
#define FONT_POKEMONGAME8_HEIGHT 10

extern const u8 font_pokemongame8_data[][10];

// Helper function to draw a character
static inline void draw_char_pokemongame8(int x, int y, char c, u16 color, u16* buffer, int buffer_width) {
    if(c < 32 || c >= 127) c = 32;
    const u8* glyph = font_pokemongame8_data[c - 32];
    for(int row = 0; row < FONT_POKEMONGAME8_HEIGHT; row++) {
        u8 row_data = glyph[row];
        for(int col = 0; col < FONT_POKEMONGAME8_WIDTH; col++) {
            if(row_data & (1 << (FONT_POKEMONGAME8_WIDTH - 1 - col))) {
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