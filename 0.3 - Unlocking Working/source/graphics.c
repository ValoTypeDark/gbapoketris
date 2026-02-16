#include "main.h"
#include "pokemon_database.h"
#include "sprite_manager.h"
#include "save.h"
#include "pokedex_ui.h"
#include "highscores_ui.h"
#include "options_ui.h"
#include "gameplay_backgrounds.h"
#include "shiny_star.h"
#include <stdio.h>
#include <string.h>
#include "font_pokemongame8.h"
#include "font_pokemon_game10.h"
#include "font_pokemon_solid14.h"
#include "font_pokemon_classic6.h"
#include "title_logo.h"

// Menu image includes
#include "mode_rookie.h"
#include "mode_normal.h"
#include "mode_super.h"
#include "mode_hyper.h"
#include "mode_master.h"
#include "mode_bonus.h"
#include "mode_back.h"
#include "bonus_ruins.h"
#include "bonus_gardens.h"
#include "bonus_cafe.h"
#include "bonus_back.h"

// Menu background
#include "menu_bg.h"

// Splash screen
#include "splash_screen.h"

// Lock icon for locked modes
#include "lock_icon.h"

extern GameData game;

// Forward declarations
void show_catch_celebration(void);
void show_mode_unlock_celebration(void);

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

// Draw a menu image with transparency support
void draw_menu_image(int x, int y, const u16* image, int width, int height) {
    for(int iy = 0; iy < height; iy++) {
        for(int ix = 0; ix < width; ix++) {
            int px = x + ix;
            int py = y + iy;
            
            if(px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                u16 color = image[iy * width + ix];
                
                // Skip transparent pixels (0x0000)
                if(color != 0x0000) {
                    back_buffer[py * SCREEN_WIDTH + px] = color;
                }
            }
        }
    }
}

// Draw the menu background (blue diagonal stripes)
void draw_menu_background(void) {
    // Fast copy: copy entire background bitmap to back buffer
    u32* dst32 = (u32*)back_buffer;
    const u32* src32 = (const u32*)menu_bg;
    
    // 38400 pixels = 19200 u32 copies
    for(int i = 0; i < 19200; i++) {
        dst32[i] = src32[i];
    }
}

// Helper function: Draw sprite as solid-color silhouette
// Uses frequency analysis to detect background color (most sprites don't use index 0 as transparent)
static void draw_sprite_silhouette(int dest_x, int dest_y, SpriteData* sprite,
                                   int dest_w, int dest_h, u16 color) {
    if(!sprite || !sprite->is_loaded) return;

    const u8* tiles = sprite->tiles;
    int src_w = sprite->width;
    int src_h = sprite->height;
    int tiles_per_row = src_w / 8;

    // Detect most common palette index (this is the background)
    u16 freq[256] = {0};
    for(int sy = 0; sy < src_h; sy++) {
        int tile_row = sy >> 3;
        int pix_row = sy & 7;
        for(int sx = 0; sx < src_w; sx++) {
            int tile_col = sx >> 3;
            int pix_col = sx & 7;
            int tile_index = tile_row * tiles_per_row + tile_col;
            u8 idx = tiles[(tile_index << 6) + (pix_row << 3) + pix_col];
            if(freq[idx] < 0xFFFF) freq[idx]++;
        }
    }
    
    // Find most common index = background
    u8 bg_index = 0;
    u16 best = 0;
    for(int i = 0; i < 256; i++) {
        if(freq[i] > best) {
            best = freq[i];
            bg_index = (u8)i;
        }
    }

    // Pre-compute source pixel mappings
    u8 src_x_map[64];
    u8 src_y_map[64];
    for(int i = 0; i < dest_w; i++) src_x_map[i] = (u8)((i * src_w) / dest_w);
    for(int i = 0; i < dest_h; i++) src_y_map[i] = (u8)((i * src_h) / dest_h);

    // Clamp to screen bounds
    int dy_start = 0, dy_end = dest_h;
    if(dest_y < 0) dy_start = -dest_y;
    if(dest_y + dest_h > SCREEN_HEIGHT) dy_end = SCREEN_HEIGHT - dest_y;

    int dx_start = 0, dx_end = dest_w;
    if(dest_x < 0) dx_start = -dest_x;
    if(dest_x + dest_w > SCREEN_WIDTH) dx_end = SCREEN_WIDTH - dest_x;

    // Draw silhouette (all non-background pixels become solid color)
    for(int dy = dy_start; dy < dy_end; dy++) {
        int py = dest_y + dy;
        int src_y = src_y_map[dy];
        int tile_row = src_y >> 3;
        int pix_row = src_y & 7;
        int row_base = py * SCREEN_WIDTH;

        for(int dx = dx_start; dx < dx_end; dx++) {
            int src_x = src_x_map[dx];
            int tile_col = src_x >> 3;
            int pix_col = src_x & 7;
            int tile_index = tile_row * tiles_per_row + tile_col;
            u8 pal_index = tiles[(tile_index << 6) + (pix_row << 3) + pix_col];
            
            if(pal_index == bg_index) continue;  // Skip background pixels
            
            back_buffer[row_base + dest_x + dx] = color;
        }
    }
}

// Helper function: Draw shiny star indicator (16x16)
static void draw_shiny_star(int x, int y) {
    for(int dy = 0; dy < 16; dy++) {
        for(int dx = 0; dx < 16; dx++) {
            int px = x + dx;
            int py = y + dy;
            if(px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                u16 color = shiny_star_data[dy * 16 + dx];
                if(color != 0) {  // Skip transparent pixels
                    back_buffer[py * SCREEN_WIDTH + px] = color;
                }
            }
        }
    }
}

void show_splash_screen(void) {
    // Draw ValoTypeDark Presents splash screen image
    // Fast copy: copy entire splash screen bitmap to back buffer
    u32* dst32 = (u32*)back_buffer;
    const u32* src32 = (const u32*)splash_screen;
    
    // 38400 pixels = 19200 u32 copies
    for(int i = 0; i < 19200; i++) {
        dst32[i] = src32[i];
    }
}

void render_game(void) {
    if(game.state == STATE_SPLASH) {
        show_splash_screen();
    }
    // Draw everything to back buffer
    else if(game.state == STATE_GAMEPLAY || game.state == STATE_PAUSE) {
        // Draw mode-specific background bitmap (includes UI boxes)
        draw_gameplay_background(game.mode);
        
        // Calculate board position with screen shake
        int board_draw_x = BOARD_X + game.shake_offset_x;
        int board_draw_y = BOARD_Y + game.shake_offset_y;
        
        // Board background is part of the bitmap - no need to draw it
        // Just draw the blocks directly on the background
        
        // Draw faint grid lines on board (before blocks)
        u16 grid_color = RGB15(15, 15, 15);  // Brighter white grid
        for(int gx = 0; gx <= BOARD_WIDTH; gx++) {
            int line_x = BOARD_X + gx * BLOCK_SIZE;
            for(int gy = 0; gy < BOARD_HEIGHT * BLOCK_SIZE; gy++) {
                int py = BOARD_Y + gy;
                if(line_x >= 0 && line_x < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                    back_buffer[py * SCREEN_WIDTH + line_x] = grid_color;
                }
            }
        }
        for(int gy = 0; gy <= BOARD_HEIGHT; gy++) {
            int line_y = BOARD_Y + gy * BLOCK_SIZE;
            for(int gx = 0; gx < BOARD_WIDTH * BLOCK_SIZE; gx++) {
                int px = BOARD_X + gx;
                if(px >= 0 && px < SCREEN_WIDTH && line_y >= 0 && line_y < SCREEN_HEIGHT) {
                    back_buffer[line_y * SCREEN_WIDTH + px] = grid_color;
                }
            }
        }
        
        // Precompute which rows are flashing during a line clear (avoids per-cell scans)
        int i, j;
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
        
        // Board border removed - not needed with bitmap backgrounds
        // The background image already defines the play area visually
        
        // Draw current piece with shake
        if(game.state == STATE_GAMEPLAY) {
            draw_tetromino(&game.current_piece, board_draw_x, board_draw_y);
        }
        
        
        // LEFT SIDE UI - Only numbers (labels are in background)
        // Positions: x=6 (was 10, moved left 4px), y adjusted down 2px
        draw_number(6, 17, game.score, BLACK);          // Score (was: x=10, y=15)
        draw_number(6, 47, game.lines_cleared, BLACK);  // Lines (was: x=10, y=45)
        draw_number(6, 77, game.level, BLACK);          // Level (was: x=10, y=75)
        
        // Next piece section - RIGHT box (measured from background)
        // Box: (195, 21) to (225, 51), center at (210, 36)
        int next_x = 210;  // Exact center X of NEXT box
        int next_y = 36;   // Exact center Y of NEXT box
        
        // Draw next piece if there is one
        if(game.next_piece.category == PIECE_TETROMINO && game.next_piece.type != TETRO_COUNT) {
            // Draw at smaller scale in the next box
            for(int i = 0; i < game.next_piece.block_count; i++) {
                int px = next_x + game.next_piece.blocks[i][0] * 6;
                int py = next_y + game.next_piece.blocks[i][1] * 6;
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
                int px = next_x + game.next_piece.blocks[i][0] * 6;
                int py = next_y + game.next_piece.blocks[i][1] * 6;
                for(int dy = 0; dy < 5; dy++) {
                    for(int dx = 0; dx < 5; dx++) {
                        if(px + dx >= 0 && px + dx < SCREEN_WIDTH && py + dy >= 0 && py + dy < SCREEN_HEIGHT) {
                            back_buffer[(py + dy) * SCREEN_WIDTH + (px + dx)] = game.next_piece.color;
                        }
                    }
                }
            }
        }
        
        // Hold piece section - LEFT box (measured from background)
        // Box: (147, 21) to (177, 51), center at (162, 36)
        int hold_x = 162;  // Exact center X of HOLD box
        int hold_y = 36;   // Exact center Y of HOLD box
        
        // Draw held piece if there is one
        if(game.hold_piece.category == PIECE_TETROMINO && game.hold_piece.type != TETRO_COUNT) {
            // Draw at smaller scale in the hold box
            for(int i = 0; i < game.hold_piece.block_count; i++) {
                int px = hold_x + game.hold_piece.blocks[i][0] * 6;
                int py = hold_y + game.hold_piece.blocks[i][1] * 6;
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
                int px = hold_x + game.hold_piece.blocks[i][0] * 6;
                int py = hold_y + game.hold_piece.blocks[i][1] * 6;
                for(int dy = 0; dy < 5; dy++) {
                    for(int dx = 0; dx < 5; dx++) {
                        if(px + dx >= 0 && px + dx < SCREEN_WIDTH && py + dy >= 0 && py + dy < SCREEN_HEIGHT) {
                            back_buffer[(py + dy) * SCREEN_WIDTH + (px + dx)] = game.hold_piece.color;
                        }
                    }
                }
            }
        }

        
        // Pokemon sprite area - positioned with exact measurements
        // Sprite box: (171, 70) to (202, 100), center at (186, 85)
        // Using 32x32 sprite centered in box
        int poke_x = 170;  // Top-left X for 32x32 sprite (186 - 16)
        int poke_y = 69;   // Top-left Y for 32x32 sprite (85 - 16)
        
        // Check if Pokemon has been caught
        const PokemonData* pdata = get_pokemon_data(game.current_pokemon);
        int is_caught = 0;
        if(pdata) {
            is_caught = is_pokemon_unlocked(pdata->dex_number);
        }
        
        // Draw Pokemon sprite/silhouette inside the box
        {
            // Force sprite load to ensure it's always available (fixes re-spawn issue)
            SpriteData* sprite = load_current_pokemon_sprite();
            if(sprite && sprite->is_loaded) {
                if(is_caught) {
                    /* Pokemon is caught - show actual sprite */
                    display_sprite_bg(poke_x, poke_y, sprite, 32, 32);
                } else {
                    /* Pokemon not caught - draw silhouette in mode color */
                    u16 mode_color;
                    switch(game.mode) {
                        case MODE_ROOKIE:   mode_color = RGB15(22, 18, 3);  break;  // Gold
                        case MODE_NORMAL:   mode_color = RGB15(3, 7, 18);   break;  // Blue
                        case MODE_SUPER:    mode_color = RGB15(3, 18, 7);   break;  // Green
                        case MODE_HYPER:    mode_color = RGB15(18, 3, 22);  break;  // Purple
                        case MODE_MASTER:   mode_color = RGB15(18, 3, 3);   break;  // Red
                        default:            mode_color = RGB15(10, 10, 10); break;  // Gray fallback
                    }
                    
                    /* Draw silhouette using helper function */
                    draw_sprite_silhouette(poke_x, poke_y, sprite, 32, 32, mode_color);
                }
                
                /* Shiny star indicator - show regardless of caught status */
                if(game.is_shiny) {
                    draw_shiny_star(poke_x + 18, poke_y - 2);
                }
            }
        }

        // Pokemon name — show "???" if not caught, actual name if caught
        // Mode-colored with black outline
        {
            const PokemonData* pdata = get_pokemon_data(game.current_pokemon);
            if(pdata) {
                /* Check if caught */
                int is_caught = is_pokemon_unlocked(pdata->dex_number);
                
                /* Build display name */
                char upper[32];
                if(is_caught) {
                    /* Show actual name in uppercase, strip parentheses */
                    const char* src = pdata->name;
                    int k = 0;
                    
                    /* Copy until we hit '(' or end of string */
                    while(k < 31 && src[k] && src[k] != '(') {
                        char c = src[k];
                        upper[k] = (c >= 'a' && c <= 'z') ? (c - 32) : c;
                        k++;
                    }
                    
                    /* Trim trailing space if we stopped at '(' */
                    while(k > 0 && upper[k-1] == ' ') k--;
                    
                    upper[k] = '\0';
                } else {
                    /* Show ??? */
                    strcpy(upper, "???");
                }

                /* Get mode color */
                u16 mode_color;
                switch(game.mode) {
                    case MODE_ROOKIE:   mode_color = RGB15(22, 18, 3);  break;  // Gold
                    case MODE_NORMAL:   mode_color = RGB15(3, 7, 18);   break;  // Blue
                    case MODE_SUPER:    mode_color = RGB15(3, 18, 7);   break;  // Green
                    case MODE_HYPER:    mode_color = RGB15(18, 3, 22);  break;  // Purple
                    case MODE_MASTER:   mode_color = RGB15(18, 3, 3);   break;  // Red
                    default:            mode_color = GREEN;             break;  // Fallback
                }

                int name_len = 0;
                while(upper[name_len]) name_len++;
                
                /* pokemongame8 is 7px wide; use 6px spacing for tightness */
                int text_w  = name_len * 6;
                /* Centre around Pokemon display area (186 is center X) */
                int name_x  = 186 - text_w / 2;
                int name_y  = 107;  // Between sprite (ends at ~101) and REMAINING text (starts at ~124)
                
                /* Draw black outline (8 directions) */
                int i2;
                for(i2 = 0; i2 < name_len; i2++) {
                    int cx = name_x + i2 * 6;
                    // Draw outline in all 8 directions
                    draw_char_pokemongame8(cx - 1, name_y - 1, upper[i2], BLACK, back_buffer, SCREEN_WIDTH);
                    draw_char_pokemongame8(cx,     name_y - 1, upper[i2], BLACK, back_buffer, SCREEN_WIDTH);
                    draw_char_pokemongame8(cx + 1, name_y - 1, upper[i2], BLACK, back_buffer, SCREEN_WIDTH);
                    draw_char_pokemongame8(cx - 1, name_y,     upper[i2], BLACK, back_buffer, SCREEN_WIDTH);
                    draw_char_pokemongame8(cx + 1, name_y,     upper[i2], BLACK, back_buffer, SCREEN_WIDTH);
                    draw_char_pokemongame8(cx - 1, name_y + 1, upper[i2], BLACK, back_buffer, SCREEN_WIDTH);
                    draw_char_pokemongame8(cx,     name_y + 1, upper[i2], BLACK, back_buffer, SCREEN_WIDTH);
                    draw_char_pokemongame8(cx + 1, name_y + 1, upper[i2], BLACK, back_buffer, SCREEN_WIDTH);
                }
                
                /* Draw main text in mode color on top */
                for(i2 = 0; i2 < name_len; i2++) {
                    draw_char_pokemongame8(name_x + i2 * 6, name_y, upper[i2],
                                           mode_color, back_buffer, SCREEN_WIDTH);
                }
                
                /* Progress bar: Pieces Remaining - tighter spacing, moved left 2px */
                int progress_y = name_y + 12;
                char pieces_text[32];
                sprintf(pieces_text, "REMAINING: %d", game.pieces_left);
                
                /* Draw with tighter spacing (6px instead of 8px) */
                int text_x = 138;  // Moved left 2px from 140
                int spacing = 6;   // Tighter spacing
                for(int i = 0; pieces_text[i] != '\0'; i++) {
                    char c = pieces_text[i];
                    if(c >= 'a' && c <= 'z') c = c - 32;  // Uppercase
                    draw_char_pokemongame8(text_x + (i * spacing), progress_y, c, WHITE, back_buffer, SCREEN_WIDTH);
                }
                
                /* Progress bar visual */
                int bar_x = 140;
                int bar_y = progress_y + 10;  // Back to original spacing
                int bar_width = 90;
                int bar_height = 6;
                
                /* Draw bar background (empty) */
                for(int by = bar_y; by < bar_y + bar_height; by++) {
                    for(int bx = bar_x; bx < bar_x + bar_width; bx++) {
                        if(bx >= 0 && bx < SCREEN_WIDTH && by >= 0 && by < SCREEN_HEIGHT) {
                            back_buffer[by * SCREEN_WIDTH + bx] = RGB15(5, 5, 5);
                        }
                    }
                }
                
                /* Draw filled portion based on pieces_left */
                int filled_width = (game.pieces_left * bar_width) / game.pokemon_duration;
                for(int by = bar_y; by < bar_y + bar_height; by++) {
                    for(int bx = bar_x; bx < bar_x + filled_width; bx++) {
                        if(bx >= 0 && bx < SCREEN_WIDTH && by >= 0 && by < SCREEN_HEIGHT) {
                            back_buffer[by * SCREEN_WIDTH + bx] = mode_color;
                        }
                    }
                }
                
                /* Draw bar border */
                for(int bx = bar_x; bx < bar_x + bar_width; bx++) {
                    if(bx >= 0 && bx < SCREEN_WIDTH) {
                        if(bar_y >= 0 && bar_y < SCREEN_HEIGHT)
                            back_buffer[bar_y * SCREEN_WIDTH + bx] = WHITE;
                        if(bar_y + bar_height - 1 >= 0 && bar_y + bar_height - 1 < SCREEN_HEIGHT)
                            back_buffer[(bar_y + bar_height - 1) * SCREEN_WIDTH + bx] = WHITE;
                    }
                }
                for(int by = bar_y; by < bar_y + bar_height; by++) {
                    if(by >= 0 && by < SCREEN_HEIGHT) {
                        if(bar_x >= 0 && bar_x < SCREEN_WIDTH)
                            back_buffer[by * SCREEN_WIDTH + bar_x] = WHITE;
                        if(bar_x + bar_width - 1 >= 0 && bar_x + bar_width - 1 < SCREEN_WIDTH)
                            back_buffer[by * SCREEN_WIDTH + bar_x + bar_width - 1] = WHITE;
                    }
                }
                
                /* Big Clear Indicator - only show if streak >= 1 */
                /* Same style as REMAINING text but with mode color */
                if(game.big_clear_streak >= 1) {
                    int big_clear_y = bar_y + 10;
                    char big_clear_text[32];
                    sprintf(big_clear_text, "BIG CLEARS: %d/10", game.big_clear_streak);
                    
                    /* Draw with same tighter spacing as REMAINING (6px instead of 8px) */
                    int text_x = 138;  // Same position as REMAINING
                    int spacing = 6;   // Same tight spacing
                    for(int i = 0; big_clear_text[i] != '\0'; i++) {
                        char c = big_clear_text[i];
                        if(c >= 'a' && c <= 'z') c = c - 32;  // Uppercase
                        draw_char_pokemongame8(text_x + (i * spacing), big_clear_y, c, mode_color, back_buffer, SCREEN_WIDTH);
                    }
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
    else if(game.state == STATE_MODE_SELECT) {
        show_mode_select();
    }
    else if(game.state == STATE_BONUS_SELECT) {
        show_bonus_select();
    }
    else if(game.state == STATE_GAME_OVER) {
        // Show the screen FIRST
        show_game_over();
        
        // Show mode unlock celebration if pending (overlaid on game over screen)
        if(game.mode_unlock_celebration_pending > 0) {
            show_mode_unlock_celebration();
        }
        
        // Track frames for save initialization
        static u8 game_over_save_started = 0;
        static u8 game_over_frames = 0;
        
        game_over_frames++;
        
        // Start save on SECOND FRAME (after screen is visible and user sees it)
        // This way, "NOW SAVING..." shows DURING the actual save operation
        if(game_over_frames == 2 && !game_over_save_started && !save_game_async_in_progress()) {
            // Queue Pokemon data for async save
            save_pokemon_progress_deferred(game.pokemon_catches);
            
            // Save mode unlock status
            save_mode_unlocks(game.mode_unlocked);
            
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

// Helper function to draw the logo bitmap
static void draw_logo_bitmap(int offset_x, int offset_y) {
    int logo_w = TITLE_LOGO_WIDTH;
    int logo_h = TITLE_LOGO_HEIGHT;
    
    for(int y = 0; y < logo_h; y++) {
        int screen_y = offset_y + y;
        if(screen_y < 0 || screen_y >= SCREEN_HEIGHT) continue;
        
        int row_offset = y * logo_w;
        int screen_row = screen_y * SCREEN_WIDTH;
        
        for(int x = 0; x < logo_w; x++) {
            int screen_x = offset_x + x;
            if(screen_x < 0 || screen_x >= SCREEN_WIDTH) continue;
            
            u16 color = title_logoBitmap[row_offset + x];
            
            // Skip pure black pixels (treat as transparent background)
            if(color != 0x0000) {
                back_buffer[screen_row + screen_x] = color;
            }
        }
    }
}

void show_title_screen(void) {
    // Draw blue diagonal stripe background
    draw_menu_background();
    
    // Draw logo centered at top
    int logo_x = (SCREEN_WIDTH - TITLE_LOGO_WIDTH) / 2;  // Center horizontally
    int logo_y = 5;  // Small margin from top
    draw_logo_bitmap(logo_x, logo_y);
    
    // Carousel-style menu - show only current selection
    const char* menu_items[] = {
        "PLAY",
        "POKEDEX",
        "HIGHSCORES",
        "OPTIONS",
        "CREDITS"
    };
    
    int num_items = 5;
    int menu_y = 147;  // Near bottom of screen
    
    // Get current menu item
    const char* current_item = menu_items[game.menu_selection];
    
    // Calculate text width for centering (using 8-pixel font for better visibility)
    int text_len = 0;
    while(current_item[text_len] != '\0') text_len++;
    int text_width = text_len * 8;  // 8px per char
    int text_x = (SCREEN_WIDTH - text_width) / 2;  // Center the text
    
    // Draw current menu item in yellow using regular 8px font
    draw_text(text_x, menu_y, current_item, POKEMON_YELLOW);
    
    // Draw directional arrows
    // Left arrow (if not at first item)
    if(game.menu_selection > 0) {
        draw_text_menu(10, menu_y - 2, "<<", WHITE);
    }
    
    // Right arrow (if not at last item)
    if(game.menu_selection < num_items - 1) {
        draw_text_menu(SCREEN_WIDTH - 30, menu_y - 2, ">>", WHITE);
    }
}

void show_mode_select(void) {
    // Draw blue diagonal stripe background
    draw_menu_background();
    
    // Array of mode images
    const u16* mode_images[7] = {
        mode_rookie, mode_normal, mode_super,
        mode_hyper, mode_master, mode_bonus, mode_back
    };
    
    int current_mode = game.menu_selection;
    
    // Draw the mode image (centered at x=20, y=35)
    int img_x = 20; // (240 - 200) / 2
    int img_y = 35;
    draw_menu_image(img_x, img_y, mode_images[current_mode], 200, 85);
    
    // Draw lock icon if mode is locked (skip BONUS and BACK)
    if(current_mode < 5 && !game.mode_unlocked[current_mode]) {
        // Center lock icon on the image
        int lock_x = img_x + (200 - LOCK_ICON_WIDTH) / 2;
        int lock_y = img_y + (85 - LOCK_ICON_HEIGHT) / 2;
        draw_menu_image(lock_x, lock_y, lock_icon, LOCK_ICON_WIDTH, LOCK_ICON_HEIGHT);
    }
    
    // Draw arrows (adjusted for new image position)
    if(current_mode > 0) {
        // Left arrow
        draw_text_large_outlined_custom_spacing(10, 55, "<", POKEMON_BLUE, POKEMON_YELLOW, 14);
    }
    
    if(current_mode < 6) {
        // Right arrow
        draw_text_large_outlined_custom_spacing(215, 55, ">", POKEMON_BLUE, POKEMON_YELLOW, 14);
    }
    
    // Show dots indicator at bottom (moved down to y=135)
    int dots_y = 135;
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

void show_bonus_select(void) {
    // Draw blue diagonal stripe background
    draw_menu_background();
    
    // Array of bonus mode images
    const u16* bonus_images[4] = {
        bonus_ruins, bonus_gardens, bonus_cafe, bonus_back
    };
    
    int current_mode = game.menu_selection;
    if(current_mode < 0) current_mode = 0;
    if(current_mode > 3) current_mode = 3;
    
    // Draw the bonus mode image (centered at x=20, y=35)
    int img_x = 20; // (240 - 200) / 2
    int img_y = 35;
    draw_menu_image(img_x, img_y, bonus_images[current_mode], 200, 85);
    
    // Draw lock icon if bonus mode is locked (skip BACK)
    if(current_mode < 3 && !game.mode_unlocked[5 + current_mode]) {
        // Center lock icon on the image
        int lock_x = img_x + (200 - LOCK_ICON_WIDTH) / 2;
        int lock_y = img_y + (85 - LOCK_ICON_HEIGHT) / 2;
        draw_menu_image(lock_x, lock_y, lock_icon, LOCK_ICON_WIDTH, LOCK_ICON_HEIGHT);
    }
    
    // Draw arrows (matching mode select)
    if(current_mode > 0) {
        // Left arrow
        draw_text_large_outlined_custom_spacing(10, 55, "<", POKEMON_BLUE, POKEMON_YELLOW, 14);
    }
    
    if(current_mode < 3) {
        // Right arrow
        draw_text_large_outlined_custom_spacing(215, 55, ">", POKEMON_BLUE, POKEMON_YELLOW, 14);
    }
    
    // Show dots indicator at bottom (moved down to y=135)
    int dots_y = 135;
    int dots_spacing = 8;
    int total_width = 4 * dots_spacing;
    int dots_start_x = SCREEN_WIDTH/2 - total_width/2;
    
    for(int i = 0; i < 4; i++) {
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
    // Celebration box (wider to fit sprite + text)
    int box_width = 200;
    int box_height = 100;
    int box_x = (SCREEN_WIDTH - box_width) / 2;
    int box_y = (SCREEN_HEIGHT - box_height) / 2;
    
    // Draw box background (solid dark)
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
    
    // Draw Pokemon sprite on left side
    int sprite_x = box_x + 10;
    int sprite_y = box_y + 25;
    int sprite_size = 50;
    
    if(game.caught_pokemon_id < TOTAL_POKEMON) {
        const PokemonData* pdata = get_pokemon_data(game.caught_pokemon_id);
        const char* sprite_file = game.is_shiny ? pdata->sprite_shiny : pdata->sprite_filename;
        SpriteData* sprite = load_sprite(sprite_file);
        if(sprite) {
            display_sprite_bg(sprite_x, sprite_y, sprite, sprite_size, sprite_size);
            
            /* Show shiny star if shiny */
            if(game.is_shiny) {
                draw_shiny_star(sprite_x + 35, sprite_y - 5);
            }
        }
    }
    
    // Text on right side - all uppercase
    int text_x = box_x + 70;  // Right of sprite
    {
        int text_w;
        const char* header;
        u16 header_color;

        if(game.is_new_catch) {
            header       = "NEW POKEMON!";
            header_color = POKEMON_YELLOW;
        } else {
            header       = "POKEMON CAUGHT!";
            header_color = WHITE;
        }

        /* Header line: draw_text_menu uses 10px per character */
        int len = 0;
        const char* p = header;
        while(*p) { len++; p++; }
        text_w = len * 10;
        draw_text_menu(text_x + (130 - text_w) / 2, box_y + 20, header, header_color);

        /* Pokemon name: uppercase, draw_text uses 8px per character */
        if(game.caught_pokemon_id < TOTAL_POKEMON) {
            const PokemonData* pdata = get_pokemon_data(game.caught_pokemon_id);
            
            // Convert to uppercase
            char upper[32];
            int i;
            for(i = 0; i < 31 && pdata->name[i]; i++) {
                char c = pdata->name[i];
                upper[i] = (c >= 'a' && c <= 'z') ? (c - 32) : c;
            }
            upper[i] = '\0';
            
            len = i;
            text_w = len * 8;
            draw_text(text_x + (130 - text_w) / 2, box_y + 45, upper, WHITE);
        }

        /* "PRESS A" hint: uppercase, draw_text uses 8px per character */
        text_w = 7 * 8;
        draw_text(text_x + (130 - text_w) / 2, box_y + 72, "PRESS A", GREEN);
    }
}

// Static variables for game over state (shared between functions)
static u8 game_over_save_phase_active = 1;
static u8 game_over_post_save_delay = 0;

void reset_game_over_state(void) {
    game_over_save_phase_active = 1;  // Reset to saving phase
    game_over_post_save_delay = 0;    // Reset delay counter
}

void show_mode_unlock_celebration(void) {
    // Mode unlock celebration box (same style as catch celebration)
    int box_width = 200;
    int box_height = 80;
    int box_x = (SCREEN_WIDTH - box_width) / 2;
    int box_y = (SCREEN_HEIGHT - box_height) / 2;
    
    // Draw box background (solid dark)
    for(int y = box_y; y < box_y + box_height; y++) {
        for(int x = box_x; x < box_x + box_width; x++) {
            back_buffer[y * SCREEN_WIDTH + x] = RGB15(5, 5, 5);
        }
    }
    
    // Draw border (yellow/gold)
    for(int x = box_x; x < box_x + box_width; x++) {
        if(x >= 0 && x < SCREEN_WIDTH) {
            if(box_y >= 0 && box_y < SCREEN_HEIGHT) {
                back_buffer[box_y * SCREEN_WIDTH + x] = POKEMON_YELLOW;
            }
            if(box_y + box_height - 1 >= 0 && box_y + box_height - 1 < SCREEN_HEIGHT) {
                back_buffer[(box_y + box_height - 1) * SCREEN_WIDTH + x] = POKEMON_YELLOW;
            }
        }
    }
    for(int y = box_y; y < box_y + box_height; y++) {
        if(y >= 0 && y < SCREEN_HEIGHT) {
            if(box_x >= 0 && box_x < SCREEN_WIDTH) {
                back_buffer[y * SCREEN_WIDTH + box_x] = POKEMON_YELLOW;
            }
            if(box_x + box_width - 1 >= 0 && box_x + box_width - 1 < SCREEN_WIDTH) {
                back_buffer[y * SCREEN_WIDTH + (box_x + box_width - 1)] = POKEMON_YELLOW;
            }
        }
    }
    
    // Mode names for unlock messages
    const char* mode_names[] = {
        "", "", "", "", // 0-3 unused
        "HYPER MODE",   // 4
        "MASTER MODE",  // 5
        "RUINS OF ALPH", // 6 (UNOWN)
        "GLOBAL GARDENS", // 7 (VIVILLON)
        "SWEET DREAMS"  // 8 (ALCREMIE)
    };
    
    const char* unlock_msg = "UNLOCKED!";
    
    if(game.mode_unlock_celebration_pending >= 4 && game.mode_unlock_celebration_pending <= 8) {
        const char* mode_name = mode_names[game.mode_unlock_celebration_pending];
        
        // Draw mode name (centered)
        int name_len = 0;
        while(mode_name[name_len] != '\0') name_len++;
        int name_width = name_len * 8;
        int name_x = box_x + (box_width - name_width) / 2;
        draw_text(name_x, box_y + 20, mode_name, POKEMON_YELLOW);
        
        // Draw "UNLOCKED!" (centered)
        int msg_len = 9; // "UNLOCKED!"
        int msg_width = msg_len * 8;
        int msg_x = box_x + (box_width - msg_width) / 2;
        draw_text(msg_x, box_y + 45, unlock_msg, WHITE);
    }
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
    // Options screen is rendered by the dedicated options_ui module
    options_ui_draw();
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
