#include "options_ui.h"
#include "main.h"
#include "save.h"
#include <stdio.h>
#include <string.h>

// External functions from graphics.c
extern u16 back_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];
extern void draw_text(int x, int y, const char* text, u16 color);
extern void draw_text_large_custom_spacing(int x, int y, const char* text, u16 color, int char_spacing);

extern GameData game;

// Color scheme matching Pokedex
#define OPT_BG_COLOR      RGB15(0,0,10)  // Dark blue background
#define OPT_PANEL_DARK    RGB15(0,0,6)   // Darker panel
#define OPT_PANEL_MID     RGB15(0,0,8)   // Mid-tone panel
#define OPT_BORDER        RGB15(8,8,12)  // Border color
#define OPT_HILITE        RGB15(31,25,5) // Pokemon yellow highlight
#define OPT_TEXT          WHITE
#define OPT_TEXT_DIM      GRAY
#define OPT_TEXT_ACCENT   POKEMON_YELLOW

// Layout
#define OPT_HEADER_H      18
#define OPT_PANEL_X       20
#define OPT_PANEL_Y       30
#define OPT_PANEL_W       200
#define OPT_PANEL_H       100

// State
static u8 s_selected = 0;  // Which option is selected (0-2: Music, SFX, Controls)
static int s_dirty = 1;

// Helper functions
static inline void put_pixel(int x, int y, u16 color) {
    if((unsigned)x >= SCREEN_WIDTH || (unsigned)y >= SCREEN_HEIGHT) return;
    back_buffer[y * SCREEN_WIDTH + x] = color;
}

static void fill_rect(int x, int y, int w, int h, u16 color) {
    if(w <= 0 || h <= 0) return;
    int x2 = x + w;
    int y2 = y + h;

    if(x < 0) x = 0;
    if(y < 0) y = 0;
    if(x2 > SCREEN_WIDTH) x2 = SCREEN_WIDTH;
    if(y2 > SCREEN_HEIGHT) y2 = SCREEN_HEIGHT;

    for(int yy = y; yy < y2; yy++) {
        u16* row = &back_buffer[yy * SCREEN_WIDTH + x];
        for(int xx = x; xx < x2; xx++) {
            *row++ = color;
        }
    }
}

static void draw_hline(int x, int y, int w, u16 color) {
    for(int i = 0; i < w; i++) put_pixel(x+i, y, color);
}

static void draw_rect(int x, int y, int w, int h, u16 color) {
    draw_hline(x, y, w, color);
    draw_hline(x, y+h-1, w, color);
    for(int i = 0; i < h; i++) {
        put_pixel(x, y+i, color);
        put_pixel(x+w-1, y+i, color);
    }
}

void options_ui_reset(void) {
    s_selected = 0;
    s_dirty = 1;
    // Control swap setting is now stored in save system, no need for local copy
}

void options_ui_handle_input(u16 down, u16 held) {
    (void)held;
    
    if(down & KEY_UP) {
        if(s_selected > 0) s_selected--;
        s_dirty = 1;
    }
    
    if(down & KEY_DOWN) {
        if(s_selected < 2) s_selected++;  // 0=Music, 1=SFX, 2=Controls
        s_dirty = 1;
    }
    
    // LEFT/RIGHT to change control mapping (only on Controls option)
    if(s_selected == 2) {
        if(down & (KEY_LEFT | KEY_RIGHT | KEY_A)) {
            // Toggle and save to save system
            u8 current = get_control_swap();
            set_control_swap(!current);  // This saves to FLASH automatically
            s_dirty = 1;
        }
    }
}

void options_ui_draw(void) {
    if(!s_dirty) return;
    s_dirty = 0;
    
    // Background
    fill_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, OPT_BG_COLOR);
    
    // Header bar
    fill_rect(0, 0, SCREEN_WIDTH, OPT_HEADER_H, OPT_PANEL_DARK);
    draw_hline(0, OPT_HEADER_H, SCREEN_WIDTH, OPT_BORDER);
    
    // Title (matching Pokedex style)
    draw_text_large_custom_spacing(10, 2, "OPTIONS", POKEMON_YELLOW, 12);
    
    // Main panel
    fill_rect(OPT_PANEL_X-2, OPT_PANEL_Y-2, OPT_PANEL_W+4, OPT_PANEL_H+4, OPT_PANEL_DARK);
    fill_rect(OPT_PANEL_X,   OPT_PANEL_Y,   OPT_PANEL_W,   OPT_PANEL_H,   OPT_PANEL_MID);
    draw_rect(OPT_PANEL_X-2, OPT_PANEL_Y-2, OPT_PANEL_W+4, OPT_PANEL_H+4, OPT_BORDER);
    
    // Options list
    int y_start = OPT_PANEL_Y + 15;
    int spacing = 25;
    
    // Music option
    u16 music_color = (s_selected == 0) ? OPT_HILITE : OPT_TEXT;
    draw_text(OPT_PANEL_X + 15, y_start, "MUSIC:", music_color);
    draw_text(OPT_PANEL_X + 110, y_start, "ON", music_color);
    if(s_selected == 0) {
        draw_text(OPT_PANEL_X + 5, y_start, ">", OPT_HILITE);
    }
    
    // SFX option  
    u16 sfx_color = (s_selected == 1) ? OPT_HILITE : OPT_TEXT;
    draw_text(OPT_PANEL_X + 15, y_start + spacing, "SFX:", sfx_color);
    draw_text(OPT_PANEL_X + 110, y_start + spacing, "ON", sfx_color);
    if(s_selected == 1) {
        draw_text(OPT_PANEL_X + 5, y_start + spacing, ">", OPT_HILITE);
    }
    
    // Controls option
    u16 ctrl_color = (s_selected == 2) ? OPT_HILITE : OPT_TEXT;
    draw_text(OPT_PANEL_X + 15, y_start + spacing*2, "CONTROLS:", ctrl_color);
    
    // Show current mapping from save system
    u8 swap = get_control_swap();
    const char* mapping = swap ? "L=FLIP/R=HOLD" : "L=HOLD/R=FLIP";
    draw_text(OPT_PANEL_X + 15, y_start + spacing*2 + 12, mapping, ctrl_color);
    
    if(s_selected == 2) {
        draw_text(OPT_PANEL_X + 5, y_start + spacing*2, ">", OPT_HILITE);
        // Show change hint
        draw_text(OPT_PANEL_X + 15, y_start + spacing*2 + 24, "(PRESS A TO SWAP)", OPT_TEXT_DIM);
    }
    
    // Footer hint
    draw_text(20, 145, "PRESS B TO GO BACK", OPT_TEXT_DIM);
}
