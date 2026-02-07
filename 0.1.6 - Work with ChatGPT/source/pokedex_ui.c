#include "pokedex_ui.h"
#include "main.h"
#include "pokemon_database.h"
#include "sprite_manager.h"
#include "font_pokemon_classic6.h"
#include <stdio.h>
#include <string.h>

// These are defined in graphics.c
extern u16 back_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];
extern void draw_text(int x, int y, const char* text, u16 color);
extern void draw_text6(int x, int y, const char* text, u16 color);
extern void draw_text6_clipped(int x, int y, const char* text, u16 color, int clip_x, int clip_y, int clip_w, int clip_h);
extern void draw_text_large(int x, int y, const char* text, u16 color);
extern void draw_text_large_custom_spacing(int x, int y, const char* text, u16 color, int char_spacing);
extern void draw_text_game8(int x, int y, const char* text, u16 color);

extern GameData game;

// ─────────────────────────────────────────────────────────────────────────────
// UI layout constants
// ─────────────────────────────────────────────────────────────────────────────
#define PDX_BG_COLOR      RGB15(0,0,10) // match title screen background
#define PDX_PANEL_DARK    RGB15(0,0,6)
#define PDX_PANEL_MID     RGB15(0,0,8)
#define PDX_SPRITE_BG     RGB15(3,3,12) // lighter blue so silhouettes show
#define PDX_BORDER        RGB15(8,8,12)
#define PDX_HILITE        RGB15(31,25,5) // Pokemon yellow-ish
#define PDX_TEXT          WHITE
#define PDX_TEXT_DIM      GRAY
#define PDX_TEXT_ACCENT   POKEMON_YELLOW

// Lighter sprite-box background so silhouettes read clearly
#define PDX_SPRITE_BG     RGB15(3,3,12)

#define LIST_X        8
#define LIST_Y        24
#define LIST_W        116
#define LIST_H        132
#define LIST_ROWS     10
#define ROW_H         12

#define DETAIL_X      130
#define DETAIL_Y      14
#define DETAIL_W      102
#define DETAIL_H      142

#define SPRITE_W      64
#define SPRITE_H      64
#define SPRITE_X      (DETAIL_X + (DETAIL_W/2) - (SPRITE_W/2))
#define SPRITE_Y      (DETAIL_Y + 10)

#define SWAP_FRAMES   210  // ~3.5 seconds at 60fps

// Key repeat (simple)
#define REPEAT_DELAY  12
#define REPEAT_RATE   3

// ─────────────────────────────────────────────────────────────────────────────
// UI state
// ─────────────────────────────────────────────────────────────────────────────
typedef enum {
    PDX_MODE_LIST = 0,
    PDX_MODE_JUMP,
    PDX_MODE_LETTER
} PokedexInputMode;

static PokedexInputMode s_mode = PDX_MODE_LIST;

// Jump-to-number (4 digits: #0000-#9999)
static u8 s_jump_digits[4] = {0,0,0,1};
static int s_jump_pos = 3; // active digit 0..3

// Letter search
static char s_letter = 'A';

static int s_selected = 0;     // 0..TOTAL_POKEMON-1
static int s_scroll   = 0;     // first visible row index
static int s_hold_frames = 0;
static int s_hold_dir = 0;     // -1 up, +1 down, 0 none

static int s_swap_timer = 0;
static int s_show_shiny = 0;   // toggles when both caught
// Dex text vertical scrolling (auto)
static int s_dex_scroll_px = 0;
static int s_dex_scroll_max = 0;
static int s_dex_scroll_delay = 0;
static int s_frame_counter = 0;


static int s_last_selected = -1;

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

// Render a sprite as a solid-color silhouette (non-transparent pixels only)
static void display_sprite_bg_silhouette(int dest_x, int dest_y, SpriteData* sprite,
                                        int dest_w, int dest_h, u16 color) {
    if(!sprite || !sprite->is_loaded) return;

    const u8* tiles = sprite->tiles;
    int src_w = sprite->width;
    int src_h = sprite->height;
    int tiles_per_row = src_w / 8;

    // Many of the grit exports in this project do NOT guarantee that palette index 0 is the transparent background.
    // To reliably create a silhouette, detect the most common palette index across the sprite and treat it as "background".
    u16 freq[256] = {0};
    int total_px = src_w * src_h;
    (void)total_px;
    for(int sy=0; sy<src_h; sy++) {
        int tile_row = sy >> 3;
        int pix_row  = sy & 7;
        for(int sx=0; sx<src_w; sx++) {
            int tile_col = sx >> 3;
            int pix_col  = sx & 7;
            int tile_index = tile_row * tiles_per_row + tile_col;
            u8 idx = tiles[(tile_index << 6) + (pix_row << 3) + pix_col];
            if(freq[idx] < 0xFFFF) freq[idx]++;
        }
    }
    u8 bg_index = 0;
    u16 best = 0;
    for(int i=0;i<256;i++) {
        if(freq[i] > best) { best = freq[i]; bg_index = (u8)i; }
    }

    u8 src_x_map[64];
    u8 src_y_map[64];
    for(int i=0;i<dest_w;i++) src_x_map[i] = (u8)((i * src_w) / dest_w);
    for(int i=0;i<dest_h;i++) src_y_map[i] = (u8)((i * src_h) / dest_h);

    int dy_start = 0, dy_end = dest_h;
    if(dest_y < 0) dy_start = -dest_y;
    if(dest_y + dest_h > SCREEN_HEIGHT) dy_end = SCREEN_HEIGHT - dest_y;

    int dx_start = 0, dx_end = dest_w;
    if(dest_x < 0) dx_start = -dest_x;
    if(dest_x + dest_w > SCREEN_WIDTH) dx_end = SCREEN_WIDTH - dest_x;

    for(int dy=dy_start; dy<dy_end; dy++) {
        int py = dest_y + dy;
        int src_y = src_y_map[dy];
        int tile_row = src_y >> 3;
        int pix_row = src_y & 7;
        int row_base = py * SCREEN_WIDTH;

        for(int dx=dx_start; dx<dx_end; dx++) {
            int src_x = src_x_map[dx];
            int tile_col = src_x >> 3;
            int pix_col = src_x & 7;
            int tile_index = tile_row * tiles_per_row + tile_col;
            u8 pal_index = tiles[(tile_index << 6) + (pix_row << 3) + pix_col];
            if(pal_index == bg_index) continue;
            back_buffer[row_base + dest_x + dx] = color;
        }
    }
}



static void clear_screen(u16 color) {
    fill_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, color);
}

// ─────────────────────────────────────────────────────────────────────────────
// UI helpers
// ─────────────────────────────────────────────────────────────────────────────
static int clampi(int v, int lo, int hi) {
    if(v < lo) return lo;
    if(v > hi) return hi;
    return v;
}

static void ensure_visible(void) {
    if(s_selected < s_scroll) s_scroll = s_selected;
    if(s_selected >= s_scroll + LIST_ROWS) s_scroll = s_selected - (LIST_ROWS - 1);

    int max_scroll = TOTAL_POKEMON - LIST_ROWS;
    if(max_scroll < 0) max_scroll = 0;
    s_scroll = clampi(s_scroll, 0, max_scroll);
}

static void move_selection(int delta) {
    int new_sel = s_selected + delta;
    new_sel = clampi(new_sel, 0, TOTAL_POKEMON - 1);
    if(new_sel != s_selected) {
        s_selected = new_sel;
        ensure_visible();
        s_swap_timer = 0;
        s_show_shiny = 0;
    }
}

// Find the first entry index that matches a displayed Dex number (e.g. 3 matches base + forms)
static int find_first_by_dex(u16 dex_num) {
    for(int i=0;i<TOTAL_POKEMON;i++) {
        if(POKEMON_DATABASE[i].dex_number == dex_num) return i;
    }
    return -1;
}

// Find first entry index whose name starts with the given letter (case-insensitive)
static int find_first_by_letter(char letter) {
    char u = (letter >= 'a' && letter <= 'z') ? (char)(letter - 32) : letter;
    for(int i=0;i<TOTAL_POKEMON;i++) {
        const char* n = POKEMON_DATABASE[i].name;
        if(!n || !n[0]) continue;
        char c = n[0];
        if(c >= 'a' && c <= 'z') c = (char)(c - 32);
        if(c == u) return i;
    }
    return -1;
}

// count caught entries (either normal or shiny)
static int count_caught_any(void) {
    int c = 0;
    for(int i=0;i<TOTAL_POKEMON;i++) {
        if(game.pokemon_catches[i].unlocked || game.pokemon_catches[i].unlocked_shiny) c++;
    }
    return c;
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────
void pokedex_ui_reset(void) {
    s_selected = 0;
    s_scroll = 0;
    s_hold_frames = 0;
    s_hold_dir = 0;
    s_swap_timer = 0;
    s_show_shiny = 0;
    s_last_selected = -1;
    s_mode = PDX_MODE_LIST;
    s_jump_digits[0]=0; s_jump_digits[1]=0; s_jump_digits[2]=0; s_jump_digits[3]=1;
    s_jump_pos = 3;
    s_letter = 'A';
}

void pokedex_ui_handle_input(u16 down, u16 held) {
    // ── Jump-to-number mode (START)
    if(s_mode == PDX_MODE_JUMP) {
        if(down & KEY_B) { s_mode = PDX_MODE_LIST; return; }

        if(down & KEY_LEFT)  { if(s_jump_pos > 0) s_jump_pos--; }
        if(down & KEY_RIGHT) { if(s_jump_pos < 3) s_jump_pos++; }

        if(down & KEY_UP) {
            s_jump_digits[s_jump_pos] = (u8)((s_jump_digits[s_jump_pos] + 1) % 10);
        }
        if(down & KEY_DOWN) {
            s_jump_digits[s_jump_pos] = (u8)((s_jump_digits[s_jump_pos] + 9) % 10);
        }

        if(down & KEY_A) {
            u16 dex = (u16)(s_jump_digits[0]*1000 + s_jump_digits[1]*100 + s_jump_digits[2]*10 + s_jump_digits[3]);
            int idx = find_first_by_dex(dex);
            if(idx >= 0) {
                s_selected = idx;
                ensure_visible();
                s_swap_timer = 0;
                s_show_shiny = 0;
            }
            s_mode = PDX_MODE_LIST;
        }
        return;
    }

    // ── Letter search mode (SELECT)
    if(s_mode == PDX_MODE_LETTER) {
        if(down & KEY_B) { s_mode = PDX_MODE_LIST; return; }

        if(down & KEY_UP) {
            if(s_letter == 'A') s_letter = 'Z'; else s_letter--;
        }
        if(down & KEY_DOWN) {
            if(s_letter == 'Z') s_letter = 'A'; else s_letter++;
        }

        if(down & KEY_A) {
            int idx = find_first_by_letter(s_letter);
            if(idx >= 0) {
                s_selected = idx;
                ensure_visible();
                s_swap_timer = 0;
                s_show_shiny = 0;
            }
            s_mode = PDX_MODE_LIST;
        }
        return;
    }

    // ── Enter overlays
    if(down & KEY_START) {
        // preload digits from current displayed dex number
        u16 d = POKEMON_DATABASE[s_selected].dex_number;
        s_jump_digits[0] = (u8)((d / 1000) % 10);
        s_jump_digits[1] = (u8)((d / 100) % 10);
        s_jump_digits[2] = (u8)((d / 10) % 10);
        s_jump_digits[3] = (u8)(d % 10);
        s_jump_pos = 3;
        s_mode = PDX_MODE_JUMP;
        return;
    }
    if(down & KEY_SELECT) {
        // set starting letter from current name (if alpha)
        char c = POKEMON_DATABASE[s_selected].name[0];
        if(c >= 'a' && c <= 'z') c -= 32;
        if(c >= 'A' && c <= 'Z') s_letter = c;
        s_mode = PDX_MODE_LETTER;
        return;
    }

    // Page jump
    if(down & KEY_L) move_selection(-LIST_ROWS);
    if(down & KEY_R) move_selection(+LIST_ROWS);

    // Immediate presses
    if(down & KEY_UP)   { move_selection(-1); s_hold_dir = -1; s_hold_frames = 0; }
    if(down & KEY_DOWN) { move_selection(+1); s_hold_dir = +1; s_hold_frames = 0; }

    // Handle held repeat for up/down
    int dir = 0;
    if(held & KEY_UP) dir = -1;
    else if(held & KEY_DOWN) dir = +1;

    if(dir == 0) {
        s_hold_dir = 0;
        s_hold_frames = 0;
        return;
    }

    if(dir != s_hold_dir) {
        s_hold_dir = dir;
        s_hold_frames = 0;
        return;
    }

    s_hold_frames++;

    if(s_hold_frames == REPEAT_DELAY) {
        move_selection(dir);
    } else if(s_hold_frames > REPEAT_DELAY) {
        if(((s_hold_frames - REPEAT_DELAY) % REPEAT_RATE) == 0) {
            move_selection(dir);
        }
    }
}


// ─────────────────────────────────────────────────────────────────────────────
// Rendering
// ─────────────────────────────────────────────────────────────────────────────
static void draw_panels(void) {
    clear_screen(PDX_BG_COLOR);

    // Header bar
    fill_rect(0, 0, SCREEN_WIDTH, 18, PDX_PANEL_DARK);
    draw_hline(0, 18, SCREEN_WIDTH, PDX_BORDER);

    // Title (match title screen font + color)
    // Title uses the original 14pt font in yellow, but with tighter spacing
    draw_text_large_custom_spacing(10, 2, "PoKeDeX", POKEMON_YELLOW, 12);

    // Caught count
    char buf[32];
    int caught = count_caught_any();
    snprintf(buf, sizeof(buf), "Caught: %d/%d", caught, TOTAL_POKEMON);
    draw_text6(120, 5, buf, WHITE);

    // Left list panel
    fill_rect(LIST_X-2, LIST_Y-2, LIST_W+4, LIST_H+4, PDX_PANEL_DARK);
    fill_rect(LIST_X,   LIST_Y,   LIST_W,   LIST_H,   PDX_PANEL_MID);
    draw_rect(LIST_X-2, LIST_Y-2, LIST_W+4, LIST_H+4, PDX_BORDER);

    // Right detail panel
    fill_rect(DETAIL_X-2, DETAIL_Y-2, DETAIL_W+4, DETAIL_H+4, PDX_PANEL_DARK);
    fill_rect(DETAIL_X,   DETAIL_Y,   DETAIL_W,   DETAIL_H,   PDX_PANEL_MID);
    draw_rect(DETAIL_X-2, DETAIL_Y-2, DETAIL_W+4, DETAIL_H+4, PDX_BORDER);

}

static void draw_list(void) {
    // Rows
    int base_y = LIST_Y + 4;
    for(int row=0; row<LIST_ROWS; row++) {
        int idx = s_scroll + row;
        if(idx >= TOTAL_POKEMON) break;

        int y = base_y + row * ROW_H;

        // Highlight
        if(idx == s_selected) {
            fill_rect(LIST_X+1, y-1, LIST_W-2, ROW_H, RGB15(2,2,12));
            draw_rect(LIST_X+1, y-1, LIST_W-2, ROW_H, PDX_HILITE);
        }

        const PokemonData* p = &POKEMON_DATABASE[idx];
        int caught = (game.pokemon_catches[idx].unlocked || game.pokemon_catches[idx].unlocked_shiny);

        char line[40];
        if(caught) {
            // "0001 Bulbasaur"
            snprintf(line, sizeof(line), "%04d %s", (int)p->dex_number, p->name);
            draw_text_game8(LIST_X+6, y+1, line, PDX_TEXT);
        } else {
            snprintf(line, sizeof(line), "%04d ????", (int)p->dex_number);
            draw_text_game8(LIST_X+6, y+1, line, PDX_TEXT_DIM);
        }
    }

    // Scrollbar (tiny)
    {
        int track_x = LIST_X + LIST_W - 6;
        int track_y = LIST_Y + 4;
        int track_w = 4;
        int track_h = LIST_H - 8;
        // Track
        fill_rect(track_x, track_y, track_w, track_h, RGB15(1,1,6));
        draw_rect(track_x, track_y, track_w, track_h, PDX_BORDER);

        int total = TOTAL_POKEMON;
        int visible = LIST_ROWS;
        int max_scroll = total - visible;
        if(max_scroll < 1) max_scroll = 1;

        int thumb_h = (track_h * visible) / total;
        if(thumb_h < 6) thumb_h = 6;
        if(thumb_h > track_h) thumb_h = track_h;

        int thumb_y = track_y + ((track_h - thumb_h) * s_scroll) / max_scroll;
        fill_rect(track_x+1, thumb_y, track_w-2, thumb_h, RGB15(4,4,10));
        draw_rect(track_x+1, thumb_y, track_w-2, thumb_h, PDX_HILITE);
    }
}


// Measure wrapped text height (6x7 font) for auto-scrolling
static int measure_wrapped_text6_height(int w, const char* text) {
    if(!text) return 0;
    const int cw = FONT_POKEMON_CLASSIC6_WIDTH;
    const int ch = FONT_POKEMON_CLASSIC6_HEIGHT;
    int max_cols = w / cw;
    if(max_cols < 1) max_cols = 1;

    int lines = 1;
    int col = 0;

    const char* p = text;
    while(*p) {
        if(*p == '\n') {
            lines++;
            col = 0;
            p++;
            continue;
        }
        if(*p == ' ') {
            // collapse spaces
            while(*p == ' ') p++;
            // add one space if room
            if(col < max_cols) col++;
            continue;
        }
        // word length
        int wl = 0;
        while(p[wl] && p[wl] != ' ' && p[wl] != '\n') wl++;

        if(col != 0 && col + wl > max_cols) {
            lines++;
            col = 0;
        }
        // place word
        int take = wl;
        if(take > max_cols) take = max_cols; // very long word safety
        col += take;

        p += wl;
        // trailing space handled next loop
    }
    return lines * ch;
}

// Draw wrapped dex text inside a bounding box, with vertical pixel scroll and hard clipping.
static void draw_wrapped_text6_clipped_scrolling(int x, int y, int w, int h, const char* text, u16 color, int scroll_px) {
    if(!text) return;

    const int cw = FONT_POKEMON_CLASSIC6_WIDTH;
    const int ch = FONT_POKEMON_CLASSIC6_HEIGHT;

    int max_cols = w / cw;
    if(max_cols < 1) max_cols = 1;

    int clip_x = x;
    int clip_y = y;
    int clip_w = w;
    int clip_h = h;

    int cx = x;
    int cy = y - scroll_px;

    // Greedy word-wrap (spaces)
    const char* p = text;
    while(*p) {
        // Skip leading spaces/newlines
        while(*p == ' ') p++;

        if(*p == '\n') {
            p++;
            cx = x;
            cy += ch;
            continue;
        }

        // Extract next word
        const char* word_start = p;
        int word_len = 0;
        while(p[word_len] && p[word_len] != ' ' && p[word_len] != '\n') word_len++;

        // If word doesn't fit on this line, move to next line (unless at line start)
        int cur_col = (cx - x) / cw;
        if(cur_col != 0 && cur_col + word_len > max_cols) {
            cx = x;
            cy += ch;
        }

        // Stop if next line is entirely below clip
        if(cy >= clip_y + clip_h) break;

        // Render the word
        for(int i=0;i<word_len;i++) {
            char c = word_start[i];
            if(c >= 'a' && c <= 'z') c = (char)(c - 32);

            // Hard stop at line width
            if(cx + cw > x + w) break;

            // Only draw if within vertical clip band
            if(cy + ch > clip_y && cy < clip_y + clip_h) {
                draw_text6_clipped(cx, cy, (char[]){c,0}, color, clip_x, clip_y, clip_w, clip_h);
            }
            cx += cw;
        }

        p += word_len;

        // Add a single space after a word if next char is space
        if(*p == ' ') {
            if(cx + cw <= x + w) cx += cw;
            while(*p == ' ') p++;
        } else if(*p == '\n') {
            // newline handled in loop start
        }
    }
}
static void draw_detail(void) {
    const PokemonData* p = &POKEMON_DATABASE[s_selected];
    int caught_n = game.pokemon_catches[s_selected].unlocked ? 1 : 0;
    int caught_s = game.pokemon_catches[s_selected].unlocked_shiny ? 1 : 0;

    // Sprite frame
    int frame_x = SPRITE_X - 4;
    int frame_y = SPRITE_Y - 4;
    fill_rect(frame_x, frame_y, SPRITE_W+8, SPRITE_H+8, PDX_PANEL_DARK);
    draw_rect(frame_x, frame_y, SPRITE_W+8, SPRITE_H+8, PDX_BORDER);

    // Lighter background inside the sprite box so silhouettes are visible
    fill_rect(SPRITE_X, SPRITE_Y, SPRITE_W, SPRITE_H, PDX_SPRITE_BG);

    // Decide which sprite to show
    const char* sprite_file = NULL;
    if(caught_n && caught_s) {
        sprite_file = s_show_shiny ? p->sprite_shiny : p->sprite_filename;
    } else if(caught_s) {
        sprite_file = p->sprite_shiny;
    } else if(caught_n) {
        sprite_file = p->sprite_filename;
    }

    if(sprite_file) {
        SpriteData* spr = load_sprite(sprite_file);
        if(spr) {
            display_sprite_bg(SPRITE_X, SPRITE_Y, spr, SPRITE_W, SPRITE_H);
        }
    } else {
        // Not caught - silhouette (uses normal sprite shape if available)
        const char* base_file = p->sprite_filename;
        SpriteData* spr = load_sprite(base_file);
        if(spr) {
            display_sprite_bg_silhouette(SPRITE_X, SPRITE_Y, spr, SPRITE_W, SPRITE_H, RGB15(0,0,0));
        } else {
            draw_text6(SPRITE_X+10, SPRITE_Y+28, "????", PDX_TEXT_DIM);
        }
    }

    // Name + number
    char header[48];
    snprintf(header, sizeof(header), "%s", p->name);
    draw_text6_clipped(DETAIL_X+6, SPRITE_Y + SPRITE_H + 8, header, PDX_TEXT, DETAIL_X+6, DETAIL_Y, DETAIL_W-12, DETAIL_H);

    // Types
    char types[48];
    if(p->type2 && p->type2[0]) {
        snprintf(types, sizeof(types), "Type: %s/%s", p->type1, p->type2);
    } else {
        snprintf(types, sizeof(types), "Type: %s", p->type1);
    }
    draw_text6_clipped(DETAIL_X+6, SPRITE_Y + SPRITE_H + 20, types, PDX_TEXT_ACCENT, DETAIL_X+6, DETAIL_Y, DETAIL_W-12, DETAIL_H);

    // Caught status
    char status[64];
    // Display caught flags as counts: 0/1 for Normal and Shiny
    int n_count = caught_n ? 1 : 0;
    int s_count = caught_s ? 1 : 0;
    snprintf(status, sizeof(status), "Caught: %d (Normal) | %d (Shiny)", n_count, s_count);
    draw_text6_clipped(DETAIL_X+6, SPRITE_Y + SPRITE_H + 32, status, (caught_n || caught_s) ? PDX_TEXT : PDX_TEXT_DIM, DETAIL_X+6, DETAIL_Y, DETAIL_W-12, DETAIL_H);

// Dex flavor text (wrapped)
    int text_x = DETAIL_X + 6;
    int text_y = SPRITE_Y + SPRITE_H + 44;
    int text_w = DETAIL_W - 12;
    int text_h = (DETAIL_Y + DETAIL_H) - text_y - 6;
    int total_h = measure_wrapped_text6_height(text_w, p->dex_text);
    s_dex_scroll_max = total_h - text_h;
    if(s_dex_scroll_max < 0) s_dex_scroll_max = 0;
    // draw subtle divider
    draw_hline(DETAIL_X+4, text_y-6, DETAIL_W-8, PDX_BORDER);

    draw_wrapped_text6_clipped_scrolling(text_x, text_y, text_w, text_h, p->dex_text, WHITE, s_dex_scroll_px);
}


static void draw_overlay(void) {
    if(s_mode == PDX_MODE_LIST) return;

    int box_w = 160;
    int box_h = 54;
    int box_x = (SCREEN_WIDTH - box_w) / 2;
    int box_y = 52;

    fill_rect(box_x, box_y, box_w, box_h, RGB15(0,0,6));
    draw_rect(box_x, box_y, box_w, box_h, PDX_BORDER);

    if(s_mode == PDX_MODE_JUMP) {
        draw_text6(box_x + 8, box_y + 8, "Jump to Dex", PDX_TEXT);
        draw_text6(box_x + 8, box_y + 22, "D-Pad: edit  A:Go  B:Cancel", PDX_TEXT_DIM);

        char num[8];
        snprintf(num, sizeof(num), "%c%c%c%c",
                 (char)('0' + s_jump_digits[0]),
                 (char)('0' + s_jump_digits[1]),
                 (char)('0' + s_jump_digits[2]),
                 (char)('0' + s_jump_digits[3]));
        draw_text6(box_x + 62, box_y + 36, num, PDX_TEXT);

        // underline current digit
        int underline_x = box_x + 62 + 8 + (s_jump_pos * 6); // approx fixed width
        draw_hline(underline_x, box_y + 46, 6, PDX_HILITE);
    } else if(s_mode == PDX_MODE_LETTER) {
        draw_text6(box_x + 8, box_y + 8, "Search by letter", PDX_TEXT);
        draw_text6(box_x + 8, box_y + 22, "Up/Down: choose  A:Go  B:Cancel", PDX_TEXT_DIM);

        char l[4] = {s_letter, 0, 0, 0};
        draw_text6(box_x + (box_w/2) - 3, box_y + 36, l, PDX_HILITE);
    }
}

void pokedex_ui_draw(void) {
    // Update swap timer when both caught
    int caught_n = game.pokemon_catches[s_selected].unlocked ? 1 : 0;
    int caught_s = game.pokemon_catches[s_selected].unlocked_shiny ? 1 : 0;

    if(s_selected != s_last_selected) {
        s_last_selected = s_selected;
        s_swap_timer = 0;
        s_show_shiny = 0;

        // reset dex text scroll when selection changes
        s_dex_scroll_px = 0;
        s_dex_scroll_delay = 0;
    }

    if(caught_n && caught_s) {
        s_swap_timer++;
        if(s_swap_timer >= SWAP_FRAMES) {
            s_swap_timer = 0;
            s_show_shiny = !s_show_shiny;
        }
    } else {
        s_swap_timer = 0;
        s_show_shiny = 0;
    }

    
    // Dex text auto-scroll (only when not in jump/letter overlay)
    s_frame_counter++;
    if(s_mode == PDX_MODE_LIST && s_dex_scroll_max > 0) {
        // Delay before starting to scroll
        if(s_dex_scroll_delay < 120) {
            s_dex_scroll_delay++;
        } else {
            // Scroll 1px every 2 frames for smoothness
            if((s_frame_counter & 1) == 0) {
                s_dex_scroll_px++;
                if(s_dex_scroll_px > s_dex_scroll_max + 10) { // small pause margin at end
                    s_dex_scroll_px = 0;
                    s_dex_scroll_delay = 0;
                }
            }
        }
    } else {
        s_dex_scroll_px = 0;
        s_dex_scroll_delay = 0;
    }

draw_panels();
    draw_list();
    draw_detail();
    draw_overlay();
}
