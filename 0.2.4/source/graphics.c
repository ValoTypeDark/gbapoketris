#include "main.h"
#include "pokemon_database.h"
#include "sprite_manager.h"
#include "save.h"
#include "pokedex_ui.h"
#include "highscores_ui.h"
#include <stdio.h>
#include <string.h>
#include "font_pokemongame8.h"
#include "font_pokemon_game10.h"
#include "font_pokemon_solid14.h"
#include "font_pokemon_classic6.h"

extern GameData game;

// Forward declarations
void show_catch_celebration(void);

extern u16* video_buffer;

// Double buffer - draw to this, then copy to VRAM
// Put in EWRAM (not IWRAM) since it's large (76KB)
u16 back_buffer[SCREEN_WIDTH * SCREEN_HEIGHT] __attribute__((section(".ewram")));
static inline void put_pixel_backbuffer(int x, int y, u16 color) {
    if ((unsigned)x >= SCREEN_WIDTH || (unsigned)y >= SCREEN_HEIGHT) return;
    back_buffer[y * SCREEN_WIDTH + x] = color;
}


// DMA copy helper
static inline void dma_copy(const void* src, void* dst, u32 count) {
    #define DMA3SAD  *(volatile u32*)0x040000D4
    #define DMA3DAD  *(volatile u32*)0x040000D8
    #define DMA3CNT  *(volatile u32*)0x040000DC
    
    DMA3SAD = (u32)src;
    DMA3DAD = (u32)dst;
    DMA3CNT = (count / 4) | DMA_ENABLE | DMA32;
}

// Flip buffers - copy back buffer to screen
static void flip_buffer(void) {
    dma_copy(back_buffer, video_buffer, sizeof(back_buffer));
}

// Simple font data (5x7 pixel font, very basic)
// This is a simplified ASCII font for GBA
// Simple 5x7 bitmap font data for ASCII characters
// Each character is 5 bytes (5 columns), each byte represents a column
/*
// OLD 5x7 BITMAP FONTS - REPLACED BY POKEMON FONTS
// Kept for reference only
static const u8 font_data[][5] = {
    // Space (32)
    {0x00, 0x00, 0x00, 0x00, 0x00},
    // ! (33)
    {0x00, 0x00, 0x5F, 0x00, 0x00},
    // " (34)
    {0x00, 0x07, 0x00, 0x07, 0x00},
    // # (35)
    {0x14, 0x7F, 0x14, 0x7F, 0x14},
    // $ (36)
    {0x24, 0x2A, 0x7F, 0x2A, 0x12},
    // % (37)
    {0x23, 0x13, 0x08, 0x64, 0x62},
    // & (38)
    {0x36, 0x49, 0x55, 0x22, 0x50},
    // ' (39)
    {0x00, 0x05, 0x03, 0x00, 0x00},
    // ( (40)
    {0x00, 0x1C, 0x22, 0x41, 0x00},
    // ) (41)
    {0x00, 0x41, 0x22, 0x1C, 0x00},
    // * (42)
    {0x14, 0x08, 0x3E, 0x08, 0x14},
    // + (43)
    {0x08, 0x08, 0x3E, 0x08, 0x08},
    // , (44)
    {0x00, 0x50, 0x30, 0x00, 0x00},
    // - (45)
    {0x08, 0x08, 0x08, 0x08, 0x08},
    // . (46)
    {0x00, 0x60, 0x60, 0x00, 0x00},
    // / (47)
    {0x20, 0x10, 0x08, 0x04, 0x02},
    // 0 (48)
    {0x3E, 0x51, 0x49, 0x45, 0x3E},
    // 1 (49)
    {0x00, 0x42, 0x7F, 0x40, 0x00},
    // 2 (50)
    {0x42, 0x61, 0x51, 0x49, 0x46},
    // 3 (51)
    {0x21, 0x41, 0x45, 0x4B, 0x31},
    // 4 (52)
    {0x18, 0x14, 0x12, 0x7F, 0x10},
    // 5 (53)
    {0x27, 0x45, 0x45, 0x45, 0x39},
    // 6 (54)
    {0x3C, 0x4A, 0x49, 0x49, 0x30},
    // 7 (55)
    {0x01, 0x71, 0x09, 0x05, 0x03},
    // 8 (56)
    {0x36, 0x49, 0x49, 0x49, 0x36},
    // 9 (57)
    {0x06, 0x49, 0x49, 0x29, 0x1E},
    // : (58)
    {0x00, 0x36, 0x36, 0x00, 0x00},
    // ; (59)
    {0x00, 0x56, 0x36, 0x00, 0x00},
    // < (60)
    {0x08, 0x14, 0x22, 0x41, 0x00},
    // = (61)
    {0x14, 0x14, 0x14, 0x14, 0x14},
    // > (62)
    {0x00, 0x41, 0x22, 0x14, 0x08},
    // ? (63)
    {0x02, 0x01, 0x51, 0x09, 0x06},
    // @ (64)
    {0x32, 0x49, 0x79, 0x41, 0x3E},
    // A (65)
    {0x7E, 0x11, 0x11, 0x11, 0x7E},
    // B (66)
    {0x7F, 0x49, 0x49, 0x49, 0x36},
    // C (67)
    {0x3E, 0x41, 0x41, 0x41, 0x22},
    // D (68)
    {0x7F, 0x41, 0x41, 0x22, 0x1C},
    // E (69)
    {0x7F, 0x49, 0x49, 0x49, 0x41},
    // F (70)
    {0x7F, 0x09, 0x09, 0x09, 0x01},
    // G (71)
    {0x3E, 0x41, 0x49, 0x49, 0x7A},
    // H (72)
    {0x7F, 0x08, 0x08, 0x08, 0x7F},
    // I (73)
    {0x00, 0x41, 0x7F, 0x41, 0x00},
    // J (74)
    {0x20, 0x40, 0x41, 0x3F, 0x01},
    // K (75)
    {0x7F, 0x08, 0x14, 0x22, 0x41},
    // L (76)
    {0x7F, 0x40, 0x40, 0x40, 0x40},
    // M (77)
    {0x7F, 0x02, 0x0C, 0x02, 0x7F},
    // N (78)
    {0x7F, 0x04, 0x08, 0x10, 0x7F},
    // O (79)
    {0x3E, 0x41, 0x41, 0x41, 0x3E},
    // P (80)
    {0x7F, 0x09, 0x09, 0x09, 0x06},
    // Q (81)
    {0x3E, 0x41, 0x51, 0x21, 0x5E},
    // R (82)
    {0x7F, 0x09, 0x19, 0x29, 0x46},
    // S (83)
    {0x46, 0x49, 0x49, 0x49, 0x31},
    // T (84)
    {0x01, 0x01, 0x7F, 0x01, 0x01},
    // U (85)
    {0x3F, 0x40, 0x40, 0x40, 0x3F},
    // V (86)
    {0x1F, 0x20, 0x40, 0x20, 0x1F},
    // W (87)
    {0x3F, 0x40, 0x38, 0x40, 0x3F},
    // X (88)
    {0x63, 0x14, 0x08, 0x14, 0x63},
    // Y (89)
    {0x07, 0x08, 0x70, 0x08, 0x07},
    // Z (90)
    {0x61, 0x51, 0x49, 0x45, 0x43},
};

// Large 8x8 font for titles and menus (double size, bold)
// This creates a bolder, more visible font for important text
static const u8 font_large_data[][5] = {
    // Space through Z, same data but will be drawn 2x2 pixels per font pixel
    // (Using same data, just scaled up in drawing)
    // Space (32)
    {0x00, 0x00, 0x00, 0x00, 0x00},
    // ! through Z - same as small font
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x08, 0x14, 0x22, 0x41, 0x00}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // @
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
};
*/

// Font type enum - kept for compatibility
typedef enum {
    FONT_SMALL = 0,
    FONT_LARGE = 1
} FontSize;

// Current font - kept for compatibility
static FontSize current_font = FONT_SMALL;

void draw_char(int x, int y, char c, u16 color) {
    // Use Pokemon Game 10pt font for in-game UI (better readability)
    draw_char_pokemon_game10(x, y, c, color, back_buffer, SCREEN_WIDTH);
}

void draw_text(int x, int y, const char* text, u16 color) {
    int i = 0;
    int spacing = 8;  // Tighter spacing: 8px instead of 10px (9px wide font + small gap)
    while(text[i] != '\0') {
        draw_char(x + (i * spacing), y, text[i], color);
        i++;
    }
}

// Pokemon Game 8pt font (used for dense lists like the Pokédex list)
void draw_text_game8(int x, int y, const char* text, u16 color) {
    int i = 0;
    // Slightly tighter spacing to reduce the "gappy" look.
    const int spacing = 6;
    if (!text) return;
    while (text[i] != '\0') {
        char c = text[i];
        // Force ALL CAPS for consistent UI.
        if (c >= 'a' && c <= 'z') c = (char)(c - 32);
        draw_char_pokemongame8(x + (i * spacing), y, c, color, back_buffer, SCREEN_WIDTH);
        i++;
    }
}


// Draw Pokemon Game8 text clipped to a rectangle, with optional horizontal pixel offset (for marquee scrolling).
// x_offset_px shifts text LEFT when positive.
void draw_text_game8_clipped(int x, int y, const char* text, u16 color,
                             int clip_x, int clip_y, int clip_w, int clip_h,
                             int x_offset_px)
{
    if (!text) return;

    const int spacing = 6;   // match draw_text_game8
    const int char_w  = 7;   // conservative width for culling
    const int char_h  = 8;   // game8 font height
    const int clip_r  = clip_x + clip_w;
    const int clip_b  = clip_y + clip_h;

    // Outside vertical clip
    if (y + char_h <= clip_y || y >= clip_b) return;

    int cx = x - x_offset_px;

    for (int i = 0; text[i] != '\0'; i++) {
        char c = text[i];
        // Force ALL CAPS
        if (c >= 'a' && c <= 'z') c = (char)(c - 32);

        // Fast horizontal cull
        if (cx + char_w <= clip_x) { cx += spacing; continue; }
        if (cx >= clip_r) break;

        draw_char_pokemongame8(cx, y, c, color, back_buffer, SCREEN_WIDTH);
        cx += spacing;
    }
}

// --- 6x6 "Pokemon Classic" font (font_pokemon_classic6) ---
static inline void draw_char6(int x, int y, char c, u16 color) {
    if (c < 32 || c > 126) return;
    const u8* rows = font_pokemon_classic6_data[c - 32];
    // Each row is a bitmask; we draw 6 pixels wide. Assume bit 5 is leftmost.
    for (int ry = 0; ry < FONT_POKEMON_CLASSIC6_HEIGHT; ry++) {
        u8 mask = rows[ry];
        for (int rx = 0; rx < FONT_POKEMON_CLASSIC6_WIDTH; rx++) {
            if (mask & (1 << (5 - rx))) {
                put_pixel_backbuffer(x + rx, y + ry, color);
            }
        }
    }
}


void draw_text6(int x, int y, const char* text, u16 color) {
    // Uppercase-only rendering using the 6x6 classic font.
    // Lowercase glyphs in this font set can look wrong, so we force A-Z.
    int cx = x;
    int cy = y;
    const int spacing = FONT_POKEMON_CLASSIC6_WIDTH;   // 6
    const int line_h  = FONT_POKEMON_CLASSIC6_HEIGHT;  // 6

    if (!text) return;

    for (const char* pch = text; *pch; ++pch) {
        char c = *pch;
        if (c == '\n') {
            cx = x;
            cy += line_h;
            continue;
        }
        if (c == ' ') {
            cx += spacing;
            continue;
        }
        if (c >= 'a' && c <= 'z') c = (char)(c - 32);
        draw_char6(cx, cy, c, color);
        cx += spacing;
    }
}


void draw_text6_clipped(int x, int y, const char* text, u16 color,
                        int clip_x, int clip_y, int clip_w, int clip_h) {
    // Clip-aware text draw for the 6x6 font.
    // Stops horizontally when outside clip, and stops vertically when we exceed clip bounds.
    int cx = x;
    int cy = y;
    const int spacing = FONT_POKEMON_CLASSIC6_WIDTH;
    const int line_h  = FONT_POKEMON_CLASSIC6_HEIGHT;

    if (!text) return;

    const int clip_r = clip_x + clip_w;
    const int clip_b = clip_y + clip_h;

    for (const char* pch = text; *pch; ++pch) {
        char c = *pch;

        if (c == '\n') {
            cx = x;
            cy += line_h;
            if (cy + line_h > clip_b) break;
            continue;
        }

        // Skip drawing lines above clip
        if (cy + line_h <= clip_y) {
            if (c == '\n') {
                cx = x;
                cy += line_h;
            }
            continue;
        }

        if (c == ' ') {
            cx += spacing;
            if (cx >= clip_r) {
                cx = x;
                cy += line_h;
                if (cy + line_h > clip_b) break;
            }
            continue;
        }

        if (c >= 'a' && c <= 'z') c = (char)(c - 32);

        if (cx + spacing <= clip_x) {
            cx += spacing;
            continue;
        }
        if (cx >= clip_r) {
            cx = x;
            cy += line_h;
            if (cy + line_h > clip_b) break;
            continue;
        }

        // Per-glyph soft clip: only draw if within clip box.
        // draw_char6 does not clip, so we clip at character cell level.
        if (cx + spacing > clip_x && cx < clip_r && cy + line_h > clip_y && cy < clip_b) {
            draw_char6(cx, cy, c, color);
        }

        cx += spacing;
    }
}





void draw_text_menu(int x, int y, const char* text, u16 color) {
    // Use Pokemon Game 10pt font for menus (medium size, clear)
    int i = 0;
    int spacing = 10;  // Pokemon game 10pt font spacing
    while(text[i] != '\0') {
        draw_char_pokemon_game10(x + (i * spacing), y, text[i], color, back_buffer, SCREEN_WIDTH);
        i++;
    }
}

void draw_text_large(int x, int y, const char* text, u16 color) {
    // Use Pokemon Solid 14pt font for titles (largest, bold)
    int i = 0;
    int spacing = 16;  // Pokemon solid font spacing
    while(text[i] != '\0') {
        draw_char_pokemon_solid14(x + (i * spacing), y, text[i], color, back_buffer, SCREEN_WIDTH);
        i++;
    }
}

// Draw large 14pt text with custom character spacing (no outline)
void draw_text_large_custom_spacing(int x, int y, const char* text, u16 color, int char_spacing) {
    int i = 0;
    if (!text) return;
    while(text[i] != '\0') {
        draw_char_pokemon_solid14(x + (i * char_spacing), y, text[i], color, back_buffer, SCREEN_WIDTH);
        i++;
    }
}

void draw_text_large_outlined(int x, int y, const char* text, u16 outline_color, u16 fill_color) {
    // Draw outline by rendering text in 8 directions
    int offsets[8][2] = {
        {-1, -1}, {0, -1}, {1, -1},  // Top row
        {-1,  0},          {1,  0},  // Middle row (skip center)
        {-1,  1}, {0,  1}, {1,  1}   // Bottom row
    };
    
    // Draw outline
    for(int i = 0; i < 8; i++) {
        draw_text_large(x + offsets[i][0], y + offsets[i][1], text, outline_color);
    }
    
    // Draw fill color on top
    draw_text_large(x, y, text, fill_color);
}

void draw_text_large_outlined_custom_spacing(int x, int y, const char* text, u16 outline_color, u16 fill_color, int char_spacing) {
    // Draw text with custom character spacing and outline
    int i = 0;
    while(text[i] != '\0') {
        int char_x = x + (i * char_spacing);
        
        // Draw outline in 8 directions for this character
        draw_char_pokemon_solid14(char_x - 1, y - 1, text[i], outline_color, back_buffer, SCREEN_WIDTH);
        draw_char_pokemon_solid14(char_x, y - 1, text[i], outline_color, back_buffer, SCREEN_WIDTH);
        draw_char_pokemon_solid14(char_x + 1, y - 1, text[i], outline_color, back_buffer, SCREEN_WIDTH);
        draw_char_pokemon_solid14(char_x - 1, y, text[i], outline_color, back_buffer, SCREEN_WIDTH);
        draw_char_pokemon_solid14(char_x + 1, y, text[i], outline_color, back_buffer, SCREEN_WIDTH);
        draw_char_pokemon_solid14(char_x - 1, y + 1, text[i], outline_color, back_buffer, SCREEN_WIDTH);
        draw_char_pokemon_solid14(char_x, y + 1, text[i], outline_color, back_buffer, SCREEN_WIDTH);
        draw_char_pokemon_solid14(char_x + 1, y + 1, text[i], outline_color, back_buffer, SCREEN_WIDTH);
        
        // Draw fill color on top
        draw_char_pokemon_solid14(char_x, y, text[i], fill_color, back_buffer, SCREEN_WIDTH);
        
        i++;
    }
}

// Font control functions (kept for compatibility)
void set_font_size(int size) {
    current_font = (FontSize)size;
}

int get_font_size(void) {
    return (int)current_font;
}

void draw_number(int x, int y, u32 number, u16 color) {
    char buffer[16];
    sprintf(buffer, "%lu", number);
    draw_text(x, y, buffer, color);
}

void draw_block(int x, int y, u16 color) {
    /* Guard: skip entirely if fully off-screen */
    if(x + BLOCK_SIZE <= 0 || x >= SCREEN_WIDTH ||
       y + BLOCK_SIZE <= 0 || y >= SCREEN_HEIGHT) return;

    u16 border = color >> 1;   /* darkened border colour */
    int i, j;

    /* Top border row */
    if(y >= 0 && y < SCREEN_HEIGHT) {
        int row = y * SCREEN_WIDTH;
        int x0 = x < 0 ? 0 : x;
        int x1 = (x + BLOCK_SIZE) > SCREEN_WIDTH ? SCREEN_WIDTH : (x + BLOCK_SIZE);
        for(j = x0; j < x1; j++) back_buffer[row + j] = border;
    }
    /* Bottom border row */
    {
        int by = y + BLOCK_SIZE - 1;
        if(by >= 0 && by < SCREEN_HEIGHT) {
            int row = by * SCREEN_WIDTH;
            int x0 = x < 0 ? 0 : x;
            int x1 = (x + BLOCK_SIZE) > SCREEN_WIDTH ? SCREEN_WIDTH : (x + BLOCK_SIZE);
            for(j = x0; j < x1; j++) back_buffer[row + j] = border;
        }
    }
    /* Interior rows (top+1 … bottom-1): left border, fill, right border */
    {
        int y0 = y + 1;  if(y0 < 0) y0 = 0;
        int y1 = y + BLOCK_SIZE - 1;  if(y1 > SCREEN_HEIGHT) y1 = SCREEN_HEIGHT;
        int lx = x;                    /* left border col */
        int rx = x + BLOCK_SIZE - 1;   /* right border col */
        int fx0 = x + 1;              /* fill start */
        int fx1 = x + BLOCK_SIZE - 1; /* fill end (exclusive) */
        /* clamp fill range */
        if(fx0 < 0) fx0 = 0;
        if(fx1 > SCREEN_WIDTH) fx1 = SCREEN_WIDTH;

        for(i = y0; i < y1; i++) {
            int row = i * SCREEN_WIDTH;
            /* left border pixel */
            if(lx >= 0 && lx < SCREEN_WIDTH)
                back_buffer[row + lx] = border;
            /* interior fill — tight loop, no branches */
            for(j = fx0; j < fx1; j++)
                back_buffer[row + j] = color;
            /* right border pixel */
            if(rx >= 0 && rx < SCREEN_WIDTH)
                back_buffer[row + rx] = border;
        }
    }
}

void draw_tetromino(Tetromino* tetro, int offset_x, int offset_y) {
    int i;
    for(i = 0; i < tetro->block_count; i++) {
        int x = offset_x + (tetro->x + tetro->blocks[i][0]) * BLOCK_SIZE;
        int y = offset_y + (tetro->y + tetro->blocks[i][1]) * BLOCK_SIZE;
        draw_block(x, y, tetro->color);
    }
}

void render_board(Board* board) {
    int i, j;
    
    // Draw board background to back buffer

    // Precompute which rows are clearing (avoids per-cell scans)
    u8 clearing_row[BOARD_HEIGHT];
    for(int k = 0; k < BOARD_HEIGHT; k++) clearing_row[k] = 0;
    if(game.line_clear_active) {
        for(int k = 0; k < game.cleared_line_count; k++) {
            int y = game.cleared_lines[k];
            if((unsigned)y < (unsigned)BOARD_HEIGHT) clearing_row[y] = 1;
        }
    }

    for(i = 0; i < BOARD_HEIGHT * BLOCK_SIZE; i++) {
        for(j = 0; j < BOARD_WIDTH * BLOCK_SIZE; j++) {
            int px = BOARD_X + j;
            int py = BOARD_Y + i;
            back_buffer[py * SCREEN_WIDTH + px] = RGB15(2,2,2); // Dark gray
        }
    }
    
    // Draw placed blocks
    for(i = 0; i < BOARD_HEIGHT; i++) {
        for(j = 0; j < BOARD_WIDTH; j++) {
            if(board->filled[i][j]) {
                int x = BOARD_X + j * BLOCK_SIZE;
                int y = BOARD_Y + i * BLOCK_SIZE;
                
                // Check if this line is being cleared
                int is_clearing = (game.line_clear_active && clearing_row[i]);
                if(is_clearing) {
                    // Flash animation: alternate between white and original color
                    // Flash every 3 frames (3 full cycles over 18 frames)
                    if((game.line_clear_timer / 3) % 2 == 0) {
                        draw_block(x, y, WHITE);  // Flash white
                    } else {
                        draw_block(x, y, board->grid[i][j]);  // Original color
                    }
                } else {
                    draw_block(x, y, board->grid[i][j]);
                }
            }
        }
    }
    
    // Draw board border
    // Draw vertical borders (left and right)
    for(i = 0; i < BOARD_HEIGHT * BLOCK_SIZE + 2; i++) {
        int y = BOARD_Y - 1 + i;
        if(y >= 0 && y < SCREEN_HEIGHT) {
            // Left border
            if(BOARD_X - 1 >= 0) {
                back_buffer[y * SCREEN_WIDTH + (BOARD_X - 1)] = WHITE;
            }
            // Right border
            if(BOARD_X + BOARD_WIDTH * BLOCK_SIZE < SCREEN_WIDTH) {
                back_buffer[y * SCREEN_WIDTH + (BOARD_X + BOARD_WIDTH * BLOCK_SIZE)] = WHITE;
            }
        }
    }
    // Draw horizontal borders (top and bottom)
    for(j = 0; j < BOARD_WIDTH * BLOCK_SIZE + 2; j++) {
        if(BOARD_X - 1 + j < SCREEN_WIDTH && BOARD_X - 1 + j >= 0) {
            // Top border
            if(BOARD_Y - 1 >= 0) {
                back_buffer[(BOARD_Y - 1) * SCREEN_WIDTH + (BOARD_X - 1 + j)] = WHITE;
            }
            // Bottom border
            if(BOARD_Y + BOARD_HEIGHT * BLOCK_SIZE < SCREEN_HEIGHT) {
                back_buffer[(BOARD_Y + BOARD_HEIGHT * BLOCK_SIZE) * SCREEN_WIDTH + (BOARD_X - 1 + j)] = WHITE;
            }
        }
    }
}

void clear_screen(u16 color) {
    int i;
    u32* buf32 = (u32*)back_buffer;
    u32 color32 = (color << 16) | color;
    
    // Clear back buffer (faster with 32-bit writes)
    for(i = 0; i < (SCREEN_WIDTH * SCREEN_HEIGHT) / 2; i++) {
        buf32[i] = color32;
    }
}

void show_splash_screen(void) {
    clear_screen(BLACK);  // Black background
    
    // "VALOTYPEDARK" - using large Pokemon Solid font
    // Calculate center position
    int title_len = 12;  // "VALOTYPEDARK" = 12 chars
    int title_width = title_len * 14;  // 14px spacing for large font
    int title_x = SCREEN_WIDTH/2 - title_width/2;
    int title_y = 60;
    
    draw_text_large(title_x, title_y, "VALOTYPEDARK", POKEMON_YELLOW);
    
    // "presents" - using menu font (10pt)
    int sub_len = 8;  // "presents" = 8 chars
    int sub_width = sub_len * 10;  // 10px per char for menu font
    int sub_x = SCREEN_WIDTH/2 - sub_width/2;
    int sub_y = 95;
    
    draw_text_menu(sub_x, sub_y, "presents", WHITE);
    
    // Show status message
    if(needs_save_loading()) {
        // "Loading..." = 10 chars × 8px = 80px wide
        int msg_x = (SCREEN_WIDTH - 80) / 2;
        draw_text(msg_x, 130, "Loading...", GRAY);
    } else if(!game.splash_save_done && needs_initial_save_creation()) {
        // "Initializing..." = 17 chars × 8px = 136px wide
        int msg_x = (SCREEN_WIDTH - 136) / 2;
        draw_text(msg_x, 130, "Initializing...", GRAY);
    }
}

void render_game(void) {
        if(game.state == STATE_SPLASH) {
        show_splash_screen();
    }
    // Draw everything to back buffer
    if(game.state == STATE_GAMEPLAY || game.state == STATE_PAUSE) {
        // Clear back buffer
        clear_screen(BLACK);
        
        // Calculate board position with screen shake
        int board_draw_x = BOARD_X + game.shake_offset_x;
        int board_draw_y = BOARD_Y + game.shake_offset_y;
        
        // Draw board background with shake offset (loop-clamped, no per-pixel branch)
        int i, j;
        {
            int bg_x0 = board_draw_x;
            int bg_y0 = board_draw_y;
            int bg_x1 = board_draw_x + BOARD_WIDTH  * BLOCK_SIZE;
            int bg_y1 = board_draw_y + BOARD_HEIGHT * BLOCK_SIZE;
            /* clamp to screen */
            if(bg_x0 < 0)              bg_x0 = 0;
            if(bg_y0 < 0)              bg_y0 = 0;
            if(bg_x1 > SCREEN_WIDTH)   bg_x1 = SCREEN_WIDTH;
            if(bg_y1 > SCREEN_HEIGHT)  bg_y1 = SCREEN_HEIGHT;
            u16 dark = RGB15(2,2,2);
            for(i = bg_y0; i < bg_y1; i++) {
                int row = i * SCREEN_WIDTH;
                for(j = bg_x0; j < bg_x1; j++) {
                    back_buffer[row + j] = dark;
                }
            }
        }
        
        
        // Precompute which rows are flashing during a line clear (avoids per-cell scans)
        u8 clearing_row[BOARD_HEIGHT];
        if(game.line_clear_active) {
            for(int r = 0; r < BOARD_HEIGHT; r++) clearing_row[r] = 0;
            for(int k = 0; k < game.cleared_line_count; k++) {
                int ry = game.cleared_lines[k];
                if((unsigned)ry < (unsigned)BOARD_HEIGHT) clearing_row[ry] = 1;
            }
        }

// Draw placed blocks with shake
        for(i = 0; i < BOARD_HEIGHT; i++) {
            for(j = 0; j < BOARD_WIDTH; j++) {
                if(game.board.filled[i][j]) {
                    int x = board_draw_x + j * BLOCK_SIZE;
                    int y = board_draw_y + i * BLOCK_SIZE;
                    
                    // Check if this line is being cleared
                int is_clearing = (game.line_clear_active && clearing_row[i]);
                if(is_clearing) {
                        // Flash animation
                        if((game.line_clear_timer / 3) % 2 == 0) {
                            draw_block(x, y, WHITE);
                        } else {
                            draw_block(x, y, game.board.grid[i][j]);
                        }
                    } else {
                        draw_block(x, y, game.board.grid[i][j]);
                    }
                }
            }
        }
        
        // Draw board border with shake
        for(i = 0; i < BOARD_HEIGHT * BLOCK_SIZE + 2; i++) {
            int y = board_draw_y - 1 + i;
            if(y >= 0 && y < SCREEN_HEIGHT) {
                int left_x = board_draw_x - 1;
                int right_x = board_draw_x + BOARD_WIDTH * BLOCK_SIZE;
                if(left_x >= 0 && left_x < SCREEN_WIDTH) {
                    back_buffer[y * SCREEN_WIDTH + left_x] = WHITE;
                }
                if(right_x >= 0 && right_x < SCREEN_WIDTH) {
                    back_buffer[y * SCREEN_WIDTH + right_x] = WHITE;
                }
            }
        }
        for(j = 0; j < BOARD_WIDTH * BLOCK_SIZE + 2; j++) {
            int top_y = board_draw_y - 1;
            int bottom_y = board_draw_y + BOARD_HEIGHT * BLOCK_SIZE;
            int x = board_draw_x - 1 + j;
            if(x >= 0 && x < SCREEN_WIDTH) {
                if(top_y >= 0 && top_y < SCREEN_HEIGHT) {
                    back_buffer[top_y * SCREEN_WIDTH + x] = WHITE;
                }
                if(bottom_y >= 0 && bottom_y < SCREEN_HEIGHT) {
                    back_buffer[bottom_y * SCREEN_WIDTH + x] = WHITE;
                }
            }
        }
        
        // Draw current piece with shake
        if(game.state == STATE_GAMEPLAY) {
            draw_tetromino(&game.current_piece, board_draw_x, board_draw_y);
        }
        
        
        // LEFT SIDE UI (x=5 to x=75) - NO SHAKE on UI
        // Score section
        draw_text(5, 10, "SCORE", WHITE);
        draw_number(5, 20, game.score, YELLOW);
        
        // Lines section
        draw_text(5, 40, "LINES", WHITE);
        draw_number(5, 50, game.lines_cleared, YELLOW);
        
        // Level section
        draw_text(5, 70, "LEVEL", WHITE);
        draw_number(5, 80, game.level, YELLOW);
        
        // Hold piece section (bottom left)
        draw_text(8, 110, "HOLD", WHITE);
        // Draw hold piece box - larger to match 8x8 blocks (32x32)
        int hold_x = 5;
        int hold_y = 120;
        for(int i = 0; i < 33; i++) {
            back_buffer[(hold_y) * SCREEN_WIDTH + (hold_x + i)] = GRAY;
            back_buffer[(hold_y + 32) * SCREEN_WIDTH + (hold_x + i)] = GRAY;
        }
        for(int i = 0; i < 33; i++) {
            back_buffer[(hold_y + i) * SCREEN_WIDTH + (hold_x)] = GRAY;
            back_buffer[(hold_y + i) * SCREEN_WIDTH + (hold_x + 32)] = GRAY;
        }
        // Draw held piece if there is one
        if(game.hold_piece.category == PIECE_TETROMINO && game.hold_piece.type != TETRO_COUNT) {
            // Draw at smaller scale in the hold box
            for(int i = 0; i < game.hold_piece.block_count; i++) {
                int px = hold_x + 16 + game.hold_piece.blocks[i][0] * 6;
                int py = hold_y + 16 + game.hold_piece.blocks[i][1] * 6;
                for(int dy = 0; dy < 5; dy++) {
                    for(int dx = 0; dx < 5; dx++) {
                        if(px + dx >= 0 && px + dx < SCREEN_WIDTH && py + dy >= 0 && py + dy < SCREEN_HEIGHT) {
                            back_buffer[(py + dy) * SCREEN_WIDTH + (px + dx)] = game.hold_piece.color;
                        }
                    }
                }
            }
        } else if(game.hold_piece.category == PIECE_PENTOMINO) {
            // Pentomino in hold
            for(int i = 0; i < game.hold_piece.block_count; i++) {
                int px = hold_x + 16 + game.hold_piece.blocks[i][0] * 6;
                int py = hold_y + 16 + game.hold_piece.blocks[i][1] * 6;
                for(int dy = 0; dy < 5; dy++) {
                    for(int dx = 0; dx < 5; dx++) {
                        if(px + dx >= 0 && px + dx < SCREEN_WIDTH && py + dy >= 0 && py + dy < SCREEN_HEIGHT) {
                            back_buffer[(py + dy) * SCREEN_WIDTH + (px + dx)] = game.hold_piece.color;
                        }
                    }
                }
            }
        }
        
        // RIGHT SIDE UI (board ends at x=130, panel is x=135 to x=240 = 105px wide)
        // Panel center is at x = 135 + 105/2 = 187.5 ≈ 188
        
        // Next piece section
        // "NEXT" = 4 chars * 8 pixels = 32 pixels, center at 188-16 = 172
        draw_text(172, 10, "NEXT", WHITE);
        // Draw next piece box (32x32) - centered at 188-16 = 172
        int next_x = 172;
        int next_y = 20;
        for(int i = 0; i < 33; i++) {
            back_buffer[(next_y) * SCREEN_WIDTH + (next_x + i)] = GRAY;
            back_buffer[(next_y + 32) * SCREEN_WIDTH + (next_x + i)] = GRAY;
        }
        for(int i = 0; i < 33; i++) {
            back_buffer[(next_y + i) * SCREEN_WIDTH + (next_x)] = GRAY;
            back_buffer[(next_y + i) * SCREEN_WIDTH + (next_x + 32)] = GRAY;
        }
        // Draw next piece if there is one
        if(game.next_piece.category == PIECE_TETROMINO && game.next_piece.type != TETRO_COUNT) {
            // Draw at smaller scale in the next box
            for(int i = 0; i < game.next_piece.block_count; i++) {
                int px = next_x + 16 + game.next_piece.blocks[i][0] * 6;
                int py = next_y + 16 + game.next_piece.blocks[i][1] * 6;
                for(int dy = 0; dy < 5; dy++) {
                    for(int dx = 0; dx < 5; dx++) {
                        if(px + dx >= 0 && px + dx < SCREEN_WIDTH && py + dy >= 0 && py + dy < SCREEN_HEIGHT) {
                            back_buffer[(py + dy) * SCREEN_WIDTH + (px + dx)] = game.next_piece.color;
                        }
                    }
                }
            }
        } else if(game.next_piece.category == PIECE_PENTOMINO) {
            // Pentomino in next
            for(int i = 0; i < game.next_piece.block_count; i++) {
                int px = next_x + 16 + game.next_piece.blocks[i][0] * 6;
                int py = next_y + 16 + game.next_piece.blocks[i][1] * 6;
                for(int dy = 0; dy < 5; dy++) {
                    for(int dx = 0; dx < 5; dx++) {
                        if(px + dx >= 0 && px + dx < SCREEN_WIDTH && py + dy >= 0 && py + dy < SCREEN_HEIGHT) {
                            back_buffer[(py + dy) * SCREEN_WIDTH + (px + dx)] = game.next_piece.color;
                        }
                    }
                }
            }
        }
        
        // Pokemon sprite area
        // "POKEMON" = 7 chars * 8 pixels = 56 pixels, center at 188-28 = 160
        draw_text(160, 70, "POKEMON", WHITE);
        // Draw Pokemon box (32x32) - centered at 188-16 = 172
        int poke_x = 172;
        int poke_y = 80;
        for(int i = 0; i < 33; i++) {
            back_buffer[(poke_y) * SCREEN_WIDTH + (poke_x + i)] = WHITE;
            back_buffer[(poke_y + 32) * SCREEN_WIDTH + (poke_x + i)] = WHITE;
        }
        for(int i = 0; i < 33; i++) {
            back_buffer[(poke_y + i) * SCREEN_WIDTH + (poke_x)] = WHITE;
            back_buffer[(poke_y + i) * SCREEN_WIDTH + (poke_x + 32)] = WHITE;
        }
        // Draw Pokemon sprite inside the box (software blit into back_buffer)
        {
            SpriteData* sprite = load_current_pokemon_sprite();
            if(sprite) {
                /* Box interior: (173,81) size 30x30 */
                display_sprite_bg(173, 81, sprite, 30, 30);
                /* Shiny star indicator next to box */
                if(game.is_shiny) {
                    draw_text(205, 80, "*", YELLOW);
                }
            }
        }

        // Pokemon name — uppercase, small font, centred under box
        {
            const PokemonData* pdata = get_pokemon_data(game.current_pokemon);
            if(pdata) {
                /* Build an all-caps copy of the name */
                char upper[32];
                const char* src = pdata->name;
                int k;
                for(k = 0; k < 31 && src[k]; k++)
                    upper[k] = (src[k] >= 'a' && src[k] <= 'z') ? (src[k] - 32) : src[k];
                upper[k] = '\0';

                int name_len = k;
                /* pokemongame8 is 7px wide; use 6px spacing for tightness */
                int text_w  = name_len * 6;
                /* Centre around panel midpoint x=188 */
                int name_x  = 188 - text_w / 2;
                int i2;
                for(i2 = 0; i2 < name_len; i2++) {
                    draw_char_pokemongame8(name_x + i2 * 6, 118, upper[i2],
                                           GREEN, back_buffer, SCREEN_WIDTH);
                }
            }
        }
        
        // Draw "PAUSE" overlay if paused
        if(game.state == STATE_PAUSE) {
            draw_text(SCREEN_WIDTH/2 - 18, SCREEN_HEIGHT/2 - 10, "PAUSED", RED);
            draw_text(SCREEN_WIDTH/2 - 42, SCREEN_HEIGHT/2 + 5, "START RESUME", WHITE);
        }
        
        // Draw catch celebration overlay (on top of everything)
        if(game.catch_celebration_active) {
            show_catch_celebration();
        }
    }
    else if(game.state == STATE_TITLE) {
        show_title_screen();
    }
    else if(game.state == STATE_MAIN_MENU) {
        show_main_menu();
    }
    else if(game.state == STATE_MODE_SELECT) {
        show_mode_select();
    }
    else if(game.state == STATE_GAME_OVER) {
        // Show the screen FIRST
        show_game_over();
        
        // Track frames for save initialization
        static u8 game_over_save_started = 0;
        static u8 game_over_frames = 0;
        
        game_over_frames++;
        
        // Start save on SECOND FRAME (after screen is visible and user sees it)
        // This way, "NOW SAVING..." shows DURING the actual save operation
        if(game_over_frames == 2 && !game_over_save_started && !save_game_async_in_progress()) {
            // Queue Pokemon data for async save
            save_pokemon_progress_deferred(game.pokemon_catches);
            
            // Add highscore (updates current_save in RAM)
            add_high_score(game.mode, game.score);
            
            // Start async save (writes to FLASH over many frames)
            save_game_async_begin();
            game_over_save_started = 1;
        }
        
        // Process save (96 bytes per frame)
        if(save_game_async_in_progress()) {
            (void)save_game_async_step(96);
        }
        
        // Reset flags when leaving game over screen
        if(game.state != STATE_GAME_OVER) {
            game_over_save_started = 0;
            game_over_frames = 0;
            // Reset the save_phase flag in show_game_over by calling it with reset context
            extern void reset_game_over_state(void);
            reset_game_over_state();
        }
    }
    else if(game.state == STATE_POKEDEX) {
        // Update handles input AND decides if redraw is needed
        if(pokedex_ui_update()) {
            pokedex_ui_draw();
        }
    }
    else if(game.state == STATE_HIGHSCORES) {
        show_highscores();
    }
    else if(game.state == STATE_OPTIONS) {
        show_options();
    }
    else if(game.state == STATE_CREDITS) {
        show_credits();
    }
    
    // Flip: copy back buffer to screen (during VBlank, no tearing!)
    flip_buffer();
}

void show_title_screen(void) {
    clear_screen(RGB15(0,0,10)); // Dark blue background
    
    // Draw "PoKeMoN TETRIS" with tighter spacing (12px instead of 16px)
    // "PoKeMoN TETRIS" = 13 chars * 12 pixels = 156 pixels wide
    int title_x = SCREEN_WIDTH/2 - 78;  // Center the title
    draw_text_large_outlined_custom_spacing(title_x, 30, "PoKeMoN TETRIS", POKEMON_BLUE, POKEMON_YELLOW, 12);
    
    // Use menu font for subtitle (10pt)
    // "GBA EDITION" = 11 chars * 10 pixels = 110 pixels wide  
    draw_text_menu(SCREEN_WIDTH/2 - 55, 60, "GBA EDITION", WHITE);
    
    // "PRESS START" = 11 chars * 10 pixels = 110 pixels wide
    draw_text_menu(SCREEN_WIDTH/2 - 55, 95, "PRESS START", GREEN);
    
    // Draw a simple Pokeball graphic
    int center_x = SCREEN_WIDTH / 2;
    int center_y = 125;
    int radius = 10;
    int i, j;
    
    for(i = -radius; i <= radius; i++) {
        for(j = -radius; j <= radius; j++) {
            if(i * i + j * j <= radius * radius) {
                int px = center_x + j;
                int py = center_y + i;
                if(px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                    if(i < 0) {
                        back_buffer[py * SCREEN_WIDTH + px] = RED;
                    } else if(i > 1) {
                        back_buffer[py * SCREEN_WIDTH + px] = WHITE;
                    } else {
                        back_buffer[py * SCREEN_WIDTH + px] = BLACK;
                    }
                }
            }
        }
    }
    
    // Center circle
    for(i = -3; i <= 3; i++) {
        for(j = -3; j <= 3; j++) {
            if(i * i + j * j <= 9) {
                int px = center_x + j;
                int py = center_y + i;
                if(px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                    back_buffer[py * SCREEN_WIDTH + px] = WHITE;
                }
            }
        }
    }
    
    // Inner circle
    for(i = -1; i <= 1; i++) {
        for(j = -1; j <= 1; j++) {
            if(i * i + j * j <= 1) {
                int px = center_x + j;
                int py = center_y + i;
                if(px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                    back_buffer[py * SCREEN_WIDTH + px] = BLACK;
                }
            }
        }
    }
}

void show_main_menu(void) {
    clear_screen(RGB15(0,0,10)); // Dark blue background
    
    // Menu items (all caps, no title)
    const char* menu_items[5] = {
        "PLAY",
        "POKEDEX", 
        "HIGHSCORES",
        "OPTIONS",
        "CREDITS"
    };
    
    int menu_y_start = 50;  // Start higher since no title
    int menu_spacing = 16;  // Tighter spacing
    
    // Draw menu items
    for(int i = 0; i < 5; i++) {
        int y = menu_y_start + (i * menu_spacing);
        u16 color = (i == game.menu_selection) ? POKEMON_YELLOW : WHITE;
        
        // Calculate text width and center it (using 8px spacing for draw_text)
        int text_len = 0;
        while(menu_items[i][text_len] != '\0') text_len++;
        int text_width = text_len * 8; // 8px per char with tight spacing
        int x = SCREEN_WIDTH/2 - text_width/2;
        
        // Selection arrow
        if(i == game.menu_selection) {
            draw_text(x - 15, y, ">", POKEMON_YELLOW);
        }
        
        draw_text(x, y, menu_items[i], color);
    }
}

void show_mode_select(void) {
    clear_screen(RGB15(0,0,10)); // Match main menu dark blue background
    
    // Mode names (all caps)
    const char* mode_names[7] = {
        "ROOKIE",
        "NORMAL",
        "SUPER",
        "HYPER",
        "MASTER",
        "BONUS MODES",
        "BACK"
    };
    
    // Mode descriptions (correct from Python game)
    const char* mode_desc_line1[7] = {
        "TETROMINOS ONLY",
        "PENTOMINOS FROM",
        "HIGH PENTOMINO",
        "VERY HIGH PENTOMINO",
        "PENTOMINOS ONLY",
        "SPECIAL GAME",
        "TO MAIN"
    };
    
    const char* mode_desc_line2[7] = {
        "",
        "LEVEL 8",
        "SPAWN RATE",
        "SPAWN RATE",
        "",
        "MODES",
        "MENU"
    };
    
    int current_mode = game.mode;
    
    // Draw mode name in large Pokemon Solid font (centered)
    int text_len = 0;
    while(mode_names[current_mode][text_len] != '\0') text_len++;
    int text_width = text_len * 15; // ~15px per char for Pokemon Solid with spacing=14
    int center_x = SCREEN_WIDTH/2 - text_width/2;
    
    // Use large outlined Pokemon font for mode name
    draw_text_large_outlined_custom_spacing(center_x, 50, mode_names[current_mode], POKEMON_BLUE, POKEMON_YELLOW, 14);
    
    // Draw description line 1 (centered)
    if(mode_desc_line1[current_mode][0] != '\0') {
        int desc1_len = 0;
        while(mode_desc_line1[current_mode][desc1_len] != '\0') desc1_len++;
        int desc1_width = desc1_len * 8;
        draw_text(SCREEN_WIDTH/2 - desc1_width/2, 75, mode_desc_line1[current_mode], WHITE);
    }
    
    // Draw description line 2 (centered)
    if(mode_desc_line2[current_mode][0] != '\0') {
        int desc2_len = 0;
        while(mode_desc_line2[current_mode][desc2_len] != '\0') desc2_len++;
        int desc2_width = desc2_len * 8;
        draw_text(SCREEN_WIDTH/2 - desc2_width/2, 85, mode_desc_line2[current_mode], WHITE);
    }
    
    // Draw simple arrows (no text)
    if(current_mode > 0) {
        // Left arrow - use Pokemon Solid font
        draw_text_large_outlined_custom_spacing(10, 55, "<", POKEMON_BLUE, POKEMON_YELLOW, 14);
    }
    
    if(current_mode < 6) {
        // Right arrow - use Pokemon Solid font
        draw_text_large_outlined_custom_spacing(215, 55, ">", POKEMON_BLUE, POKEMON_YELLOW, 14);
    }
    
    // Show dots indicator at bottom (which mode you're on)
    int dots_y = 110;
    int dots_spacing = 8;
    int total_width = 7 * dots_spacing;
    int dots_start_x = SCREEN_WIDTH/2 - total_width/2;
    
    for(int i = 0; i < 7; i++) {
        int dot_x = dots_start_x + (i * dots_spacing);
        u16 dot_color = (i == current_mode) ? POKEMON_YELLOW : GRAY;
        
        // Draw a small dot (3x3 pixels)
        for(int dy = 0; dy < 3; dy++) {
            for(int dx = 0; dx < 3; dx++) {
                int px = dot_x + dx;
                int py = dots_y + dy;
                if(px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                    back_buffer[py * SCREEN_WIDTH + px] = dot_color;
                }
            }
        }
    }
}

void show_pause_menu(void) {
    // Already handled in render_game
}

void show_catch_celebration(void) {
    // Celebration box (centered)
    int box_width = 160;
    int box_height = 100;
    int box_x = (SCREEN_WIDTH - box_width) / 2;
    int box_y = (SCREEN_HEIGHT - box_height) / 2;
    
    // Draw box background (solid dark — no need for full-screen darken,
    // the rest of the screen is already black from clear_screen)
    int y, x;
    for(y = box_y; y < box_y + box_height; y++) {
        for(x = box_x; x < box_x + box_width; x++) {
            back_buffer[y * SCREEN_WIDTH + x] = RGB15(5, 5, 5);
        }
    }
    
    // Draw border (yellow/gold)
    for(x = box_x; x < box_x + box_width; x++) {
        if(x >= 0 && x < SCREEN_WIDTH) {
            if(box_y >= 0 && box_y < SCREEN_HEIGHT) {
                back_buffer[box_y * SCREEN_WIDTH + x] = POKEMON_YELLOW;
            }
            if(box_y + box_height - 1 >= 0 && box_y + box_height - 1 < SCREEN_HEIGHT) {
                back_buffer[(box_y + box_height - 1) * SCREEN_WIDTH + x] = POKEMON_YELLOW;
            }
        }
    }
    for(y = box_y; y < box_y + box_height; y++) {
        if(y >= 0 && y < SCREEN_HEIGHT) {
            if(box_x >= 0 && box_x < SCREEN_WIDTH) {
                back_buffer[y * SCREEN_WIDTH + box_x] = POKEMON_YELLOW;
            }
            if(box_x + box_width - 1 >= 0 && box_x + box_width - 1 < SCREEN_WIDTH) {
                back_buffer[y * SCREEN_WIDTH + (box_x + box_width - 1)] = POKEMON_YELLOW;
            }
        }
    }
    
    // Text — all centred within the box using string-length × char-width
    {
        int text_w;
        const char* header;
        u16        header_color;

        if(game.is_new_catch) {
            header       = "NEW POKEMON!";
            header_color = POKEMON_YELLOW;
        } else {
            header       = "POKEMON CAUGHT!";
            header_color = WHITE;
        }

        /* Header line: draw_text_menu uses 10px per character */
        {
            int len = 0;
            const char* p = header;
            while(*p) { len++; p++; }
            text_w = len * 10;
            draw_text_menu(box_x + (box_width - text_w) / 2, box_y + 12, header, header_color);
        }

        /* Pokemon name: draw_text uses 8px per character */
        if(game.caught_pokemon_id < TOTAL_POKEMON) {
            const PokemonData* pdata = get_pokemon_data(game.caught_pokemon_id);
            int len = 0;
            const char* p = pdata->name;
            while(*p) { len++; p++; }
            text_w = len * 8;
            draw_text(box_x + (box_width - text_w) / 2, box_y + 38, pdata->name, WHITE);
        }

        /* "Press A" hint: draw_text uses 8px per character, "Press A" = 7 chars = 56px */
        text_w = 7 * 8;   /* 56 */
        draw_text(box_x + (box_width - text_w) / 2, box_y + 72, "Press A", GREEN);
    }
}

// Static variables for game over state (shared between functions)
static u8 game_over_save_phase_active = 1;
static u8 game_over_post_save_delay = 0;

void reset_game_over_state(void) {
    game_over_save_phase_active = 1;  // Reset to saving phase
    game_over_post_save_delay = 0;    // Reset delay counter
}

void show_game_over(void) {
    clear_screen(RGB15(0,0,10)); // Blue background to match main screens
    
    // Reset flag when save completes
    if(!save_game_async_in_progress() && game_over_save_phase_active) {
        // Small delay after save completes before showing menu
        game_over_post_save_delay++;
        if(game_over_post_save_delay > 10) {  // Wait 10 frames after save completes
            game_over_save_phase_active = 0;
            game_over_post_save_delay = 0;
        }
    }
    
    // Check if this is a new highscore
    int is_new_highscore = is_high_score(game.mode, game.score);
    
    // "GAME OVER" title - large font at top, centered
    // "GAME OVER" = 9 chars * 16px spacing = 144px wide
    int title_x = (SCREEN_WIDTH - 144) / 2;  // Center: (240 - 144) / 2 = 48
    int title_y = 20;
    draw_text_large(title_x, title_y, "GAME OVER", RED);
    
    // NEW HIGHSCORE message if applicable
    if(is_new_highscore) {
        // "NEW HIGHSCORE!" centered below title
        // "NEW HIGHSCORE!" = 14 chars * 10px = 140px
        int hs_x = (SCREEN_WIDTH - 140) / 2;
        draw_text_menu(hs_x, title_y + 22, "NEW HIGHSCORE!", POKEMON_YELLOW);
    }
    
    // Stats section - using small font (draw_text = 8px per char)
    int stats_y = is_new_highscore ? 65 : 55;  // Shift down if showing highscore message
    int label_x = 30;
    int value_x = 110;  // Align values
    
    // Points: (small font)
    draw_text(label_x, stats_y, "Points:", WHITE);
    draw_number(value_x, stats_y, game.score, YELLOW);
    
    // Lines: (small font)
    draw_text(label_x, stats_y + 15, "Lines:", WHITE);
    draw_number(value_x, stats_y + 15, game.lines_cleared, YELLOW);
    
    // Caught: (small font)
    draw_text(label_x, stats_y + 30, "Caught:", WHITE);
    draw_number(value_x, stats_y + 30, game.pokemon_caught_this_game, YELLOW);
    
    // New: (small font)
    draw_text(label_x, stats_y + 45, "New:", WHITE);
    draw_number(value_x, stats_y + 45, game.new_dex_entries, GREEN);
    
    int bottom_y = 130;  // Moved down from 118 to give more space
    
    if(game_over_save_phase_active) {
        // "NOW SAVING..." message - medium font (draw_text_menu), gray color
        // "NOW SAVING..." = 13 chars * 10px = 130px wide
        int saving_x = (SCREEN_WIDTH - 130) / 2;  // Center: (240 - 130) / 2 = 55
        draw_text_menu(saving_x, bottom_y, "NOW SAVING...", GRAY);
    } else {
        // Show "Restart" and "Menu" options on same line
        // Using medium font (draw_text_menu = 10px per char)
        
        int restart_x = 40;   // "Restart" position
        int menu_x = 150;     // "Menu" position (same line, well-spaced)
        
        // RESTART option
        if(game.game_over_selection == 0) {
            // Selected - yellow
            draw_text_menu(restart_x, bottom_y, "Restart", POKEMON_YELLOW);
        } else {
            draw_text_menu(restart_x, bottom_y, "Restart", WHITE);
        }
        
        // MENU option
        if(game.game_over_selection == 1) {
            // Selected - yellow
            draw_text_menu(menu_x, bottom_y, "Menu", POKEMON_YELLOW);
        } else {
            draw_text_menu(menu_x, bottom_y, "Menu", WHITE);
        }
        
        // Reset save phase when user leaves game over screen
        // (This is detected by checking if we're still in game over state next frame)
    }
}

void show_pokedex(void) {
    // Full Pokédex UI (rendered to back_buffer in Mode 3)
    pokedex_ui_draw();
}

void show_highscores(void) {
    // Highscores screen is rendered by the dedicated highscores_ui module.
    highscores_ui_draw();
}

void show_options(void) {
    clear_screen(BLACK);
    
    draw_text_menu(SCREEN_WIDTH/2 - 40, 20, "OPTIONS", POKEMON_YELLOW);
    
    draw_text(30, 60, "Music:     ON", WHITE);
    draw_text(30, 75, "SFX:       ON", WHITE);
    draw_text(30, 90, "Controls:  GBA", WHITE);
    
    draw_text(30, 135, "Press B to go back", GRAY);
}

void show_credits(void) {
    clear_screen(BLACK);
    
    draw_text_menu(SCREEN_WIDTH/2 - 40, 20, "CREDITS", POKEMON_YELLOW);
    
    draw_text(20, 50, "POKEMON TETRIS GBA", WHITE);
    draw_text(20, 65, "VERSION 1.0", WHITE);
    
    draw_text(20, 90, "GAME DESIGN:", CYAN);
    draw_text(20, 100, "VALOTYPEDARK", WHITE);
    
    draw_text(20, 120, "PROGRAMMING:", CYAN);
    draw_text(20, 130, "VALOTYPEDARK", WHITE);
    
    draw_text(20, 145, "PRESS B TO GO BACK", GRAY);
}
