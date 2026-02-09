#include "highscores_ui.h"

#include "main.h"
#include "save.h"

#include <string.h>

// These are defined in graphics.c
extern u16 back_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];
extern void draw_text_large_custom_spacing(int x, int y, const char* text, u16 color, int char_spacing);
extern void draw_text_game8(int x, int y, const char* text, u16 color);


// Fast tiny formatting helpers (avoid snprintf for performance on GBA)
static int append_uint(char* out, unsigned v) {
    // returns chars written (no terminator)
    char tmp[10];
    int n = 0;
    if(v == 0) { out[0] = '0'; return 1; }
    while(v > 0 && n < (int)sizeof(tmp)) { tmp[n++] = (char)('0' + (v % 10)); v /= 10; }
    // reverse
    for(int i = 0; i < n; i++) out[i] = tmp[n - 1 - i];
    return n;
}

static void format_rank_score(char* out, int rank1, int score) {
    // "1-  12345" or "1-  ---"
    int p = 0;
    out[p++] = (char)('0' + rank1); // rank is 1..5
    out[p++] = '-';
    out[p++] = ' ';
    out[p++] = ' ';
    if(score <= 0) {
        out[p++] = '-'; out[p++] = '-'; out[p++] = '-';
    } else {
        p += append_uint(out + p, (unsigned)score);
    }
    out[p] = '\0';
}
// ─────────────────────────────────────────────────────────────────────────────
// Style (matches Pokédex look)
// ─────────────────────────────────────────────────────────────────────────────
#define HS_BG_COLOR      RGB15(0,0,10)
#define HS_PANEL_DARK    RGB15(0,0,6)
#define HS_PANEL_MID     RGB15(0,0,8)
#define HS_BORDER        RGB15(8,8,12)
#define HS_TEXT          WHITE
#define HS_TEXT_DIM      GRAY

// Layout
#define HS_HEADER_H   20
#define HS_LIST_X     8
#define HS_LIST_Y     28
#define HS_LIST_W     116
#define HS_LIST_H     128
#define HS_ROW_H      12

#define HS_RIGHT_X    130
#define HS_RIGHT_Y    28
#define HS_RIGHT_W    102
#define HS_RIGHT_H    128

// Displayed mode pages (NO generic BONUS placeholder)
typedef struct {
    GameMode mode;
    const char* label;
} ModePage;

static const ModePage HS_PAGES[] = {
    { MODE_ROOKIE,   "ROOKIE" },
    { MODE_NORMAL,   "NORMAL" },
    { MODE_SUPER,    "SUPER" },
    { MODE_HYPER,    "HYPER" },
    { MODE_MASTER,   "MASTER" },
    { MODE_UNOWN,    "RUINS OF ALPH" },
    { MODE_VIVILLON, "GLOBAL GARDENS" },
    { MODE_ALCREMIE, "SWEET DREAMS CAFE" },
};

#define HS_PAGE_COUNT ((int)(sizeof(HS_PAGES)/sizeof(HS_PAGES[0])))

// UI state
static int s_page = 0;
static int s_dirty = 1;

// ─────────────────────────────────────────────────────────────────────────────
// Pixel helpers (Mode 3 back buffer)
// ─────────────────────────────────────────────────────────────────────────────
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

static void draw_vline(int x, int y, int h, u16 color) {
    for(int i = 0; i < h; i++) put_pixel(x, y+i, color);
}

static void draw_rect(int x, int y, int w, int h, u16 color) {
    draw_hline(x, y, w, color);
    draw_hline(x, y+h-1, w, color);
    draw_vline(x, y, h, color);
    draw_vline(x+w-1, y, h, color);
}

static u16 darken(u16 c, int amt) {
    int r = (c & 31);
    int g = (c >> 5) & 31;
    int b = (c >> 10) & 31;
    r -= amt; g -= amt; b -= amt;
    if(r < 0) r = 0;
    if(g < 0) g = 0;
    if(b < 0) b = 0;
    return (u16)(r | (g<<5) | (b<<10));
}

static u16 mode_color(GameMode m) {
    switch(m) {
        case MODE_ROOKIE:   return POKEMON_YELLOW;
        case MODE_NORMAL:   return POKEMON_BLUE;
        case MODE_SUPER:    return GREEN;
        case MODE_HYPER:    return PURPLE;
        case MODE_MASTER:   return RED;
        case MODE_UNOWN:    return RGB15(18,10,2);   // brown
        case MODE_VIVILLON: return RGB15(18,28,10);  // lighter green
        case MODE_ALCREMIE: return WHITE;
        default:            return WHITE;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────
void highscores_ui_reset(void) {
    s_page = 0;
    s_dirty = 1;
}

void highscores_ui_handle_input(u16 down, u16 held) {
    (void)held;
    int prev = s_page;

    if(down & KEY_UP)   s_page--;
    if(down & KEY_DOWN) s_page++;
    if(down & KEY_LEFT) s_page--;
    if(down & KEY_RIGHT)s_page++;

    if(s_page < 0) s_page = HS_PAGE_COUNT - 1;
    if(s_page >= HS_PAGE_COUNT) s_page = 0;

    if(s_page != prev) s_dirty = 1;
}

static void draw_background(void) {
    fill_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, HS_BG_COLOR);
    // header divider
    draw_hline(0, HS_HEADER_H, SCREEN_WIDTH, HS_BORDER);
    draw_hline(0, HS_HEADER_H+1, SCREEN_WIDTH, HS_BORDER);
}

static void draw_panels(void) {
    // left list panel
    fill_rect(HS_LIST_X, HS_LIST_Y, HS_LIST_W, HS_LIST_H, HS_PANEL_DARK);
    draw_rect(HS_LIST_X, HS_LIST_Y, HS_LIST_W, HS_LIST_H, HS_BORDER);

    // right scores panel
    fill_rect(HS_RIGHT_X, HS_RIGHT_Y, HS_RIGHT_W, HS_RIGHT_H, HS_PANEL_DARK);
    draw_rect(HS_RIGHT_X, HS_RIGHT_Y, HS_RIGHT_W, HS_RIGHT_H, HS_BORDER);

    // inner separators (subtle)
    fill_rect(HS_LIST_X+1, HS_LIST_Y+1, HS_LIST_W-2, 14, HS_PANEL_MID);
    fill_rect(HS_RIGHT_X+1, HS_RIGHT_Y+1, HS_RIGHT_W-2, 14, HS_PANEL_MID);
}

static void draw_title(void) {
    // Match Pokédex vibe: mixed-case, yellow, custom spacing
    draw_text_large_custom_spacing(8, 2, "HighscorE", POKEMON_YELLOW, 12);
}

static void draw_mode_list(void) {
    int x = HS_LIST_X + 8;
    int y = HS_LIST_Y + 4;
    draw_text_game8(x, y, "MODES", HS_TEXT);

    y += 16;
    for(int i = 0; i < HS_PAGE_COUNT; i++) {
        const ModePage* p = &HS_PAGES[i];
        u16 mc = mode_color(p->mode);
        int row_y = y + i * HS_ROW_H;

        if(i == s_page) {
            // subtle per-mode highlight stripe
            u16 bg = darken(mc, 12);
            fill_rect(HS_LIST_X+2, row_y-1, HS_LIST_W-4, HS_ROW_H, bg);
            draw_text_game8(x, row_y, p->label, mc);
        } else {
            draw_text_game8(x, row_y, p->label, HS_TEXT);
        }
    }
}

static void draw_scores(void) {
    const ModePage* p = &HS_PAGES[s_page];
    u16 mc = mode_color(p->mode);

    int x = HS_RIGHT_X + 10;
    int y = HS_RIGHT_Y + 4;

    draw_text_game8(x, y, "TOP 5", mc);
    y += 18;

    for(int i = 0; i < 5; i++) {
        int score = get_high_score((int)p->mode, i);
        char line[24];
        format_rank_score(line, i+1, score);
        draw_text_game8(x, y + i * 16, line, HS_TEXT);
    }
}

void highscores_ui_draw(void) {
    if(!s_dirty) return;
    s_dirty = 0;

    draw_background();
    draw_title();
    draw_panels();
    draw_mode_list();
    draw_scores();
    // No top "MODE: ..." text, no bottom helper text (by design).
}
