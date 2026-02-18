#include "main.h"
#include "save.h"
#include "pokemon_database.h"
#include "sprite_manager.h"
#include "pokedex_ui.h"
#include "highscores_ui.h"
#include "options_ui.h"
#include "audio.h"
#include "audio_data.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Audio sample macros - pass data pointer and size to audio_play_sfx
#define PLAY_MUSIC_MAINMENU    audio_play_sfx(bg_mainmenu_pcm,   bg_mainmenu_pcm_size)
#define PLAY_MUSIC_GAMEPLAY    audio_play_sfx(bg_gameplay_pcm,   bg_gameplay_pcm_size)
#define PLAY_MUSIC_POKEDEX     audio_play_sfx(bg_pokedex_pcm,    bg_pokedex_pcm_size)
#define PLAY_SFX_CLEAR         audio_play_sfx(clear_pcm,         clear_pcm_size)
#define PLAY_SFX_LEVEL_UP      audio_play_sfx(level_up_pcm,      level_up_pcm_size)
#define PLAY_SFX_POKEMON_CATCH audio_play_sfx(pokemon_catch_pcm, pokemon_catch_pcm_size)
#define PLAY_SFX_SHINY         audio_play_sfx(shiny_pcm,         shiny_pcm_size)
#define PLAY_SFX_MENU_MOVE     audio_play_sfx(menu_move_pcm,     menu_move_pcm_size)
#define PLAY_SFX_MENU_SELECT   audio_play_sfx(menu_select_pcm,   menu_select_pcm_size)
#define PLAY_SFX_BIG_CLEAR     audio_play_sfx(big_clear_pcm,     big_clear_pcm_size)


/* ---------------------------------------------------------------------------
 * Timer control bit defines (some libgba setups don't expose TM_* macros)
 * Timer CNT_H bits:
 *   0-1: prescale (0=1,1=64,2=256,3=1024)
 *   2  : cascade
 *   6  : IRQ enable
 *   7  : enable
 * --------------------------------------------------------------------------- */
#ifndef TM_FREQ_1024
#define TM_FREQ_1024 0x0003
#endif
#ifndef TM_CASCADE
#define TM_CASCADE   0x0004
#endif
#ifndef TM_ENABLE
#define TM_ENABLE    0x0080

/* ---------------------------------------------------------------------------
 * RNG entropy accumulation
 * We keep a running entropy accumulator during menus so the seed depends on
 * real user timing (fixes "same Pokémon on every cold boot" in emulators).
 * --------------------------------------------------------------------------- */
static u32 g_rng_entropy = 0x6D2B79F5; /* non-zero */
static u32 read_timer32(void) {
    return ((u32)REG_TM3CNT_L << 16) | (u32)REG_TM2CNT_L;
}
static void rng_mix_frame(void) {
    /* REG_KEYINPUT: pressed buttons are 0, released are 1 */
    u32 keys = (u32)(~REG_KEYINPUT) & 0x03FF;
    /* Simple xorshift-ish mixer */
    g_rng_entropy ^= (read_timer32() << 1);
    g_rng_entropy ^= ((u32)REG_VCOUNT << 24);
    g_rng_entropy ^= (keys << 12);
    g_rng_entropy ^= (g_rng_entropy >> 17);
    g_rng_entropy *= 0x85EBCA6Bu;
}
static void rng_reseed_now(void) {
    u32 seed = g_rng_entropy ^ read_timer32() ^ ((u32)REG_VCOUNT << 16) ^ 0xA5A5C3u;
    srand(seed);
    /* advance entropy so consecutive reseeds differ even if called same frame */
    g_rng_entropy ^= seed + 0x9E3779B9u;
}


#endif


/* ─────────────────────────────────────────────────────────────────────────────
 * Globals   (graphics.c / sprite_manager.c access these via extern)
 * ───────────────────────────────────────────────────────────────────────────── */
GameData  game;
u16*      video_buffer = (u16*)0x06000000;   /* Mode-3 frame buffer base */

/* ─────────────────────────────────────────────────────────────────────────────
 * Tetromino coordinate table  –  SRS spawn shapes (rotation 0-3)
 *   [piece_type][rotation][block_index][0=dx, 1=dy]
 * ───────────────────────────────────────────────────────────────────────────── */
static const s8 tetromino_coords[TETRO_COUNT][4][4][2] = {
    /* I ─ horizontal spawn */
    { {{-1, 0},{0, 0},{1, 0},{2, 0}},
      {{ 0,-1},{0, 0},{0, 1},{0, 2}},
      {{-1, 0},{0, 0},{1, 0},{2, 0}},
      {{ 0,-1},{0, 0},{0, 1},{0, 2}} },
    /* O */
    { {{0,0},{1,0},{0,1},{1,1}},
      {{0,0},{1,0},{0,1},{1,1}},
      {{0,0},{1,0},{0,1},{1,1}},
      {{0,0},{1,0},{0,1},{1,1}} },
    /* T */
    { {{-1,0},{0,0},{1,0},{0,-1}},
      {{ 0,-1},{0,0},{0,1},{1,0}},
      {{-1,0},{0,0},{1,0},{0,1}},
      {{ 0,-1},{0,0},{0,1},{-1,0}} },
    /* S */
    { {{-1,0},{0,0},{0,-1},{1,-1}},
      {{ 0,-1},{0,0},{1,0},{1,1}},
      {{-1,1},{0,1},{0,0},{1,0}},
      {{-1,-1},{-1,0},{0,0},{0,1}} },
    /* Z */
    { {{-1,-1},{0,-1},{0,0},{1,0}},
      {{ 1,-1},{1,0},{0,0},{0,1}},
      {{-1,0},{0,0},{0,1},{1,1}},
      {{ 0,-1},{0,0},{-1,0},{-1,1}} },
    /* J */
    { {{-1,0},{0,0},{1,0},{1,-1}},
      {{ 0,-1},{0,0},{0,1},{1,1}},
      {{-1,1},{-1,0},{0,0},{1,0}},
      {{-1,-1},{0,-1},{0,0},{0,1}} },
    /* L */
    { {{-1,0},{0,0},{1,0},{-1,-1}},
      {{ 0,-1},{0,0},{0,1},{1,-1}},
      {{-1,0},{0,0},{1,0},{1,1}},
      {{ 0,-1},{0,0},{0,1},{-1,1}} },
};

/* Pokemon-themed colours, indexed by TetrominoType */
static const u16 tetromino_colors[TETRO_COUNT] = {
    RGB15(0,25,31),   /* I  – cyan   */
    RGB15(31,31,0),   /* O  – yellow */
    RGB15(25,0,31),   /* T  – purple */
    RGB15(0,31,0),    /* S  – green  */
    RGB15(31,0,0),    /* Z  – red    */
    RGB15(0,0,31),    /* J  – blue   */
    RGB15(31,15,0)    /* L  – orange */
};

/* ─────────────────────────────────────────────────────────────────────────────
 * SRS wall-kick tables
 *   standard_kicks[8][5][2]  –  J L S Z T
 *   i_kicks[8][5][2]         –  I piece
 *   Index layout:  0 = 0→R,  1 = R→2,  2 = 2→L,  3 = L→0  (CW)
 *                  4 = R→0,  5 = 2→R,  6 = L→2,  7 = 0→L  (CCW)
 * ───────────────────────────────────────────────────────────────────────────── */
static const int standard_kicks[8][5][2] = {
    {{0,0},{-1,0},{-1, 1},{0,-2},{-1,-2}},   /* 0→R */
    {{0,0},{ 1,0},{ 1,-1},{0, 2},{ 1, 2}},   /* R→2 */
    {{0,0},{ 1,0},{ 1, 1},{0,-2},{ 1,-2}},   /* 2→L */
    {{0,0},{-1,0},{-1,-1},{0, 2},{-1, 2}},   /* L→0 */
    {{0,0},{ 1,0},{ 1,-1},{0, 2},{ 1, 2}},   /* R→0 */
    {{0,0},{-1,0},{-1,-1},{0, 2},{-1, 2}},   /* 2→R */
    {{0,0},{-1,0},{-1, 1},{0,-2},{-1,-2}},   /* L→2 */
    {{0,0},{ 1,0},{ 1,-1},{0, 2},{ 1, 2}}    /* 0→L */
};

static const int i_kicks[8][5][2] = {
    {{0,0},{-2, 0},{ 1, 0},{-2,-1},{ 1, 2}},  /* 0→R */
    {{0,0},{-1, 0},{ 2, 0},{-1, 2},{ 2,-1}},  /* R→2 */
    {{0,0},{ 2, 0},{-1, 0},{ 2, 1},{-1,-2}},  /* 2→L */
    {{0,0},{ 1, 0},{-2, 0},{ 1,-2},{-2, 1}},  /* L→0 */
    {{0,0},{ 2, 0},{-1, 0},{ 2, 1},{-1,-2}},  /* R→0 */
    {{0,0},{ 1, 0},{-2, 0},{ 1,-2},{-2, 1}},  /* 2→R */
    {{0,0},{-2, 0},{ 1, 0},{-2,-1},{ 1, 2}},  /* L→2 */
    {{0,0},{-1, 0},{ 2, 0},{-1, 2},{ 2,-1}}   /* 0→L */
};

/* ─────────────────────────────────────────────────────────────────────────────
 * Private-helper forward declarations
 * ───────────────────────────────────────────────────────────────────────────── */
static void update_fall_speed(void);
static void update_big_clear_streak(int lines_cleared);
static void check_for_shiny(void);
static void finish_line_clear(Board* board);
static void init_pentomino(Tetromino* tetro, int pento_type);

/* ─────────────────────────────────────────────────────────────────────────────
 * main()
 * ───────────────────────────────────────────────────────────────────────────── */
/* ─────────────────────────────────────────────────────────────────────────────
 * check_mode_unlocks() - Check Pokemon progress and unlock modes
 * ───────────────────────────────────────────────────────────────────────────── */
void check_mode_unlocks(void) {
    // Count unique Pokemon caught (either form counts)
    int unique_caught = 0;
    for(int i = 0; i < 1349; i++) {
        if(game.pokemon_catches[i].unlocked || game.pokemon_catches[i].unlocked_shiny) {
            unique_caught++;
        }
    }
    
    // HYPER mode: 150 unique Pokemon
    if(unique_caught >= 150) {
        game.mode_unlocked[3] = 1;
    }
    
    // MASTER mode: 250 unique Pokemon
    if(unique_caught >= 250) {
        game.mode_unlocked[4] = 1;
    }
    
    // UNOWN mode: Catch Unown A (ID 201, index 200) - either form
    if(game.pokemon_catches[200].unlocked || game.pokemon_catches[200].unlocked_shiny) {
        game.mode_unlocked[5] = 1;
    }
    
    // VIVILLON mode: Catch Vivillon Archipelago - either form
    if(game.pokemon_catches[665].unlocked || game.pokemon_catches[665].unlocked_shiny) {
        game.mode_unlocked[6] = 1;
    }
    
    // ALCREMIE mode: Catch Alcremie Vanilla Strawberry - either form
    if(game.pokemon_catches[868].unlocked || game.pokemon_catches[868].unlocked_shiny) {
        game.mode_unlocked[7] = 1;
    }
}

/* ─────────────────────────────────────────────────────────────────────────────
 * main()
 * ───────────────────────────────────────────────────────────────────────────── */
int main(void) {
    /* Video – SDK wrapper, not raw register */
    SetMode(MODE_3 | BG2_ON);

    /* VBlank IRQ */
    irqInit();
    irqEnable(IRQ_VBLANK);

    /* Initialize audio */
    audio_init();

    
/* RNG: start a free-running 32-bit timer (TM2 + TM3 cascade).
 * Timer0+Timer1 are reserved for Direct Sound audio.
 * We *don't* srand() here; instead we accumulate entropy during menus and
 * seed when a match begins (init_game). */
{
    REG_TM2CNT_H = 0;
    REG_TM3CNT_H = 0;
    REG_TM2CNT_L = 0;
    REG_TM3CNT_L = 0;
    REG_TM2CNT_H = TM_FREQ_1024 | TM_ENABLE;
    REG_TM3CNT_H = TM_CASCADE  | TM_ENABLE;
}

    /* Zero the entire game struct so every timer / flag starts clean */
    memset(&game, 0, sizeof(GameData));
    
    /* Initialize mode unlocks - will be updated after save loads */
    // Always unlocked
    game.mode_unlocked[0] = 1; // ROOKIE
    game.mode_unlocked[1] = 1; // NORMAL
    game.mode_unlocked[2] = 1; // SUPER
    // Locked by default (will check after save loads)
    game.mode_unlocked[3] = 0; // HYPER
    game.mode_unlocked[4] = 0; // MASTER
    game.mode_unlocked[5] = 0; // UNOWN
    game.mode_unlocked[6] = 0; // VIVILLON
    game.mode_unlocked[7] = 0; // ALCREMIE
    game.mode_unlock_celebration_pending = 0;

    /* Persistent data - deferred to splash screen */
    init_save_system();   /* Just initializes RAM, doesn't load yet */
    /* load_pokemon_progress will be called after load_save_deferred() */

    /* Cold-start state */
    game.state          = STATE_SPLASH;
    game.mode           = MODE_ROOKIE;
    game.level          = 1;
    game.fall_speed     = 30;
    game.menu_selection = 0;
    game.splash_timer   = 0;              
    game.splash_save_done = 0;            

    /* Board + hold/next sentinels (TETRO_COUNT = "empty") */
    init_board(&game.board);
    game.hold_piece.category = PIECE_TETROMINO;
    game.hold_piece.type     = TETRO_COUNT;
    game.next_piece.category = PIECE_TETROMINO;
    game.next_piece.type     = TETRO_COUNT;

    /* Sprite sub-system (OAM hide + cache clear) */
    init_sprite_system();

    /* ── Main loop ── */
    while(1) {
        VBlankIntrWait();        /* sync to 60 fps */
        rng_mix_frame();
        
        /* Update audio (music looping) */
// DISABLED:         audio_update();
        
        // Handle splash screen logic
        if(game.state == STATE_SPLASH) {
            // Only start loading AFTER we've rendered at least one frame
            if(game.splash_timer > 0) {
                // Load save from FLASH (happens once, deferred from boot)
                if(needs_save_loading()) {
                    load_save_deferred();
                    // Now that save is loaded, load Pokemon progress
                    load_pokemon_progress(game.pokemon_catches);
                    
                    // Load mode unlock status from save
                    load_mode_unlocks(game.mode_unlocked);
                    
                    // Check which modes should be unlocked based on Pokemon progress
                    // (This will upgrade old saves that didn't have mode unlocks)
                    check_mode_unlocks();
                }
                
                // Create initial save if needed (only happens if load failed)
                if(!game.splash_save_done && needs_initial_save_creation()) {
                    create_initial_save_blocking();
                    game.splash_save_done = 1;
                }
            }
            
            // Increment timer
            game.splash_timer++;
            
            // Auto-advance after loads complete + 2 seconds minimum
            // (loads will take 120-360 frames anyway)
            if(game.splash_timer >= 120 && !needs_save_loading() && !needs_initial_save_creation()) {
                game.state = STATE_TITLE;
// DISABLED:                 audio_play_music(MUSIC_MAINMENU);  // Start main menu music
            }
        }
        
        handle_input();
        update_game();
        render_game();           /* defined in graphics.c */
    }
    return 0;
}

/* ─────────────────────────────────────────────────────────────────────────────
 * init_game()  –  full reset for a new match
 * ───────────────────────────────────────────────────────────────────────────── */
void init_game(void) {
    int i;

    /* Reseed RNG based on accumulated user-timing entropy */
    rng_reseed_now();

    init_board(&game.board);
    init_sprite_system();

    game.state         = STATE_GAMEPLAY;
// DISABLED:     audio_play_music(MUSIC_GAMEPLAY);  // Start gameplay music
    game.score         = 0;
    game.lines_cleared = 0;
    game.level         = 1;
    game.fall_speed    = 30;
    game.pause_menu_selection = 0;
    game.fall_timer    = 0;
    
    /* Scoring system */
    game.b2b_active    = 0;
    game.b2b_count     = 0;
    game.drop_bonus    = 0;

    /* DAS */
    game.left_das_timer   = 0;  game.right_das_timer  = 0;
    game.left_das_active  = 0;  game.right_das_active = 0;
    game.left_arr_timer   = 0;  game.right_arr_timer  = 0;

    /* Hold – sentinel = empty */
    game.hold_used           = 0;
    game.hold_piece.type     = TETRO_COUNT;
    game.hold_piece.category = PIECE_TETROMINO;

    /* Next – sentinel; first spawn_next_piece will fill it */
    game.next_piece.type     = TETRO_COUNT;
    game.next_piece.category = PIECE_TETROMINO;

    /* Pokemon catch tracking: set dex_number mapping only; progress is loaded from save */
    for(i = 0; i < TOTAL_POKEMON; i++) {
        game.pokemon_catches[i].dex_number = POKEMON_DATABASE[i].dex_number;
    }
/* Shiny / streak */
    game.is_shiny                  = 0;
    game.big_clear_streak          = 0;
    game.special_spawn_pending     = 0;
    game.tier_up_active            = 0;
    game.pokemon_caught_this_game  = 0;
    game.shinies_caught_this_game  = 0;
    game.new_dex_entries           = 0;

    /* First Pokemon target */
    spawn_random_pokemon();

    /* Two calls: sentinel→current + generate next, then next→current + generate next */
    spawn_next_piece();
    spawn_next_piece();

    /* Particles & effects */
    game.screen_shake_active = 0;
    game.screen_shake_timer  = 0;
    game.shake_offset_x      = 0;
    game.shake_offset_y      = 0;
    game.shake_intensity     = 0;

    /* Line-clear animation */
    game.line_clear_active  = 0;
    game.line_clear_timer   = 0;
    game.cleared_line_count = 0;

    /* Catch celebration */
    game.catch_celebration_active = 0;
    game.catch_celebration_timer  = 0;
    game.is_new_catch             = 0;

    /* Lock delay */
    game.lock_delay_timer  = 0;
    game.lock_delay_active = 0;
    game.lock_resets_used  = 0;
    game.is_grounded       = 0;

    /* Game-over sub-menu */
    game.game_over_selection = 0;
}

/* ─────────────────────────────────────────────────────────────────────────────
 * Board
 * ───────────────────────────────────────────────────────────────────────────── */
void init_board(Board* board) {
    int i, j;
    for(i = 0; i < BOARD_HEIGHT; i++)
        for(j = 0; j < BOARD_WIDTH; j++) {
            board->filled[i][j] = 0;
            board->grid[i][j]   = BLACK;
        }
}

/* ─────────────────────────────────────────────────────────────────────────────
 * Piece initialisation
 * ───────────────────────────────────────────────────────────────────────────── */
void init_tetromino(Tetromino* tetro, TetrominoType type) {
    int i;
    tetro->type        = type;
    tetro->category    = PIECE_TETROMINO;
    tetro->x           = BOARD_WIDTH / 2;
    tetro->y           = 0;
    tetro->rotation    = 0;
    tetro->color       = tetromino_colors[type];
    tetro->block_count = 4;
    tetro->is_flipped  = 0;

    /* Load actual SRS coordinates – NOT the dummy {0,i} that was here before */
    for(i = 0; i < 4; i++) {
        tetro->blocks[i][0] = tetromino_coords[type][0][i][0];
        tetro->blocks[i][1] = tetromino_coords[type][0][i][1];
    }
}

static void init_pentomino(Tetromino* tetro, int pento_type) {
    int i;
    tetro->type        = pento_type;
    tetro->category    = PIECE_PENTOMINO;
    tetro->x           = BOARD_WIDTH / 2;
    tetro->y           = 0;
    tetro->rotation    = 0;
    tetro->color       = PENTOMINO_COLORS[pento_type];
    tetro->block_count = 5;
    tetro->is_flipped  = 0;

    {
        const s8* raw = get_pentomino_shape(pento_type, 0);
        for(i = 0; i < 5; i++) {
            tetro->blocks[i][0] = raw[i*2];
            tetro->blocks[i][1] = raw[i*2 + 1];
        }
    }
}

/* ─────────────────────────────────────────────────────────────────────────────
 * Collision & placement
 * ───────────────────────────────────────────────────────────────────────────── */
int check_collision(Tetromino* tetro, Board* board) {
    int i;
    for(i = 0; i < tetro->block_count; i++) {
        int bx = tetro->x + tetro->blocks[i][0];
        int by = tetro->y + tetro->blocks[i][1];
        if(bx < 0 || bx >= BOARD_WIDTH || by >= BOARD_HEIGHT) return 1;
        if(by >= 0 && board->filled[by][bx])                  return 1;
    }
    return 0;
}

void place_tetromino(Tetromino* tetro, Board* board) {
    int i;
    for(i = 0; i < tetro->block_count; i++) {
        int bx = tetro->x + tetro->blocks[i][0];
        int by = tetro->y + tetro->blocks[i][1];
        if(by >= 0 && by < BOARD_HEIGHT && bx >= 0 && bx < BOARD_WIDTH) {
            board->filled[by][bx] = 1;
            board->grid[by][bx]   = tetro->color;
        }
    }
}

/* ─────────────────────────────────────────────────────────────────────────────
 * Rotation  (full SRS for tetrominoes, 5-position kick for pentominoes)
 * ───────────────────────────────────────────────────────────────────────────── */
void rotate_tetromino(Tetromino* tetro, int direction) {
    // Handle Unown rotation with wall kicks
    if(tetro->category == PIECE_UNOWN) {
        int new_rot = (tetro->rotation + direction + 4) % 4;
        
        // Create temporary piece with new rotation
        Tetromino temp = *tetro;
        temp.rotation = new_rot;
        
        // Load blocks for new rotation from piece data
        const UnownPieceData* piece = &UNOWN_PIECES[tetro->unown];
        for(int i = 0; i < temp.block_count; i++) {
            temp.blocks[i][0] = piece->blocks[new_rot][i][0];
            temp.blocks[i][1] = piece->blocks[new_rot][i][1];
        }
        
        // Try rotation with wall kicks (same as pentominos)
        static const int unown_kicks[5][2] = {{0,0},{-1,0},{1,0},{0,-1},{0,1}};
        for(int k = 0; k < 5; k++) {
            temp.x = tetro->x + unown_kicks[k][0];
            temp.y = tetro->y + unown_kicks[k][1];
            if(!check_collision(&temp, &game.board)) {
                *tetro = temp;
                return;
            }
        }
        // All kicks failed - rotation cancelled
        return;
    }
    
    int i, k;
    int old_rot = tetro->rotation;
    int new_rot = (old_rot + direction + 4) % 4;

    Tetromino temp = *tetro;
    temp.rotation  = new_rot;

    /* ── Load new-rotation coordinates ── */
    if(tetro->category == PIECE_PENTOMINO) {
        const s8* raw = get_pentomino_shape(tetro->type, new_rot);
        for(i = 0; i < 5; i++) {
            temp.blocks[i][0] = raw[i*2];
            temp.blocks[i][1] = raw[i*2 + 1];
        }
    } else {
        for(i = 0; i < 4; i++) {
            temp.blocks[i][0] = tetromino_coords[tetro->type][new_rot][i][0];
            temp.blocks[i][1] = tetromino_coords[tetro->type][new_rot][i][1];
        }
    }

    /* Re-apply horizontal flip if the piece was previously flipped */
    if(tetro->is_flipped) {
        int mn = 127, mx = -128;
        for(i = 0; i < temp.block_count; i++) {
            if(temp.blocks[i][0] < mn) mn = temp.blocks[i][0];
            if(temp.blocks[i][0] > mx) mx = temp.blocks[i][0];
        }
        for(i = 0; i < temp.block_count; i++)
            temp.blocks[i][0] = mx - (temp.blocks[i][0] - mn);
    }

    /* ── Wall kicks ── */
    if(tetro->category == PIECE_TETROMINO) {
        /* O piece is rotationally symmetric – no kick needed */
        if(tetro->type == TETRO_O) { *tetro = temp; return; }

        /*
         * Kick-table row selection:
         *   CW  (direction == +1):  row = old_rot          (0,1,2,3)
         *   CCW (direction == -1):  row = 4 + new_rot      (4,5,6,7)
         */
        int kick_idx = (direction == 1) ? old_rot : (4 + new_rot);
        const int (*kicks)[2] = (tetro->type == TETRO_I)
            ? (const int (*)[2])i_kicks[kick_idx]
            : (const int (*)[2])standard_kicks[kick_idx];

        for(k = 0; k < 5; k++) {
            temp.x = tetro->x + kicks[k][0];
            temp.y = tetro->y + kicks[k][1];
            if(!check_collision(&temp, &game.board)) {
                *tetro = temp;   /* accept first valid kick */
                return;
            }
        }
        /* All 5 kicks failed – rotation cancelled silently */
    } else {
        /* Pentomino – simple 5-position kick */
        static const int pento_kicks[5][2] = {{0,0},{-1,0},{1,0},{0,-1},{0,1}};
        for(k = 0; k < 5; k++) {
            temp.x = tetro->x + pento_kicks[k][0];
            temp.y = tetro->y + pento_kicks[k][1];
            if(!check_collision(&temp, &game.board)) {
                *tetro = temp;
                return;
            }
        }
    }
}

/* ─────────────────────────────────────────────────────────────────────────────
 * Flip  (horizontal mirror with 9 kick attempts)
 * ───────────────────────────────────────────────────────────────────────────── */
void flip_tetromino(Tetromino* tetro) {
    int i, k;

    /* Pieces that look the same flipped – skip */
    if(tetro->category == PIECE_TETROMINO &&
       (tetro->type == TETRO_O || tetro->type == TETRO_I ||
        tetro->type == TETRO_T))
        return;

    /* Mirror around the bounding-box centre */
    int mn = 127, mx = -128;
    for(i = 0; i < tetro->block_count; i++) {
        if(tetro->blocks[i][0] < mn) mn = tetro->blocks[i][0];
        if(tetro->blocks[i][0] > mx) mx = tetro->blocks[i][0];
    }

    Tetromino temp = *tetro;
    for(i = 0; i < tetro->block_count; i++) {
        temp.blocks[i][0] = mx - (tetro->blocks[i][0] - mn);
        temp.blocks[i][1] = tetro->blocks[i][1];
    }
    temp.is_flipped = !tetro->is_flipped;

    /* Try current position first, then 8 offsets */
    static const int flip_kicks[9][2] = {
        {0,0},{-1,0},{1,0},{-2,0},{2,0},{0,-1},{0,1},{-1,-1},{1,-1}
    };
    for(k = 0; k < 9; k++) {
        temp.x = tetro->x + flip_kicks[k][0];
        temp.y = tetro->y + flip_kicks[k][1];
        if(!check_collision(&temp, &game.board)) {
            *tetro = temp;
            return;
        }
    }
    /* All kicks failed – flip cancelled */
}

/* ─────────────────────────────────────────────────────────────────────────────
 * Piece spawning
 * ───────────────────────────────────────────────────────────────────────────── */
static void spawn_random_tetromino(Tetromino* piece) {
    init_tetromino(piece, (TetrominoType)(rand() % TETRO_COUNT));
}

static void spawn_random_pentomino(Tetromino* piece) {
    init_pentomino(piece, rand() % PENTOMINO_COUNT);
}

/* Mode-aware spawn – pentomino probability scales with mode + level */
static void spawn_piece_for_mode(Tetromino* piece, GameMode mode, int level) {
    // MODE_UNOWN: Special Unown piece spawning
    if(mode == MODE_UNOWN) {
        // Get level-based Unown pool
        u8 unown_pool[28];
        u8 pool_size = 0;
        get_unown_pool_for_level(level, unown_pool, &pool_size);
        
        // Pick random Unown from available pool
        UnownType unown_type = unown_pool[rand() % pool_size];
        
        // Initialize as Unown piece
        piece->category = PIECE_UNOWN;
        piece->unown = unown_type;
        piece->block_count = UNOWN_PIECES[unown_type].block_count;
        piece->color = UNOWN_PIECES[unown_type].color;
        piece->rotation = 0;
        piece->x = BOARD_WIDTH / 2;
        piece->y = 0;
        piece->is_flipped = 0;
        
        // Copy blocks for rotation 0
        for(int i = 0; i < piece->block_count; i++) {
            piece->blocks[i][0] = UNOWN_PIECES[unown_type].blocks[0][i][0];
            piece->blocks[i][1] = UNOWN_PIECES[unown_type].blocks[0][i][1];
        }
        
        return;
    }
    
    // MODE_VIVILLON and MODE_ALCREMIE: Use Super mode mechanics
    if(mode == MODE_VIVILLON || mode == MODE_ALCREMIE) {
        mode = MODE_SUPER;
    }
    
    int pento_pct = 0;   /* 0-100 */

    switch(mode) {
        case MODE_MASTER:
            pento_pct = 100;
            break;
        case MODE_SUPER:
        case MODE_HYPER:
            pento_pct = 20 + 5 * level;
            if(pento_pct > 90) pento_pct = 90;
            break;
        case MODE_NORMAL:
            if(level >= 8) {
                pento_pct = 5 + (level - 8);
                if(pento_pct > 20) pento_pct = 20;
            }
            break;
        case MODE_ROOKIE:
        case MODE_BONUS:
        case MODE_BACK:
        case MODE_COUNT:
            pento_pct = 0;
            break;
        default:
            pento_pct = 0;
            break;
    }

    if((rand() % 100) < pento_pct)
        spawn_random_pentomino(piece);
    else
        spawn_random_tetromino(piece);
}

void spawn_next_piece(void) {
    /* Promote next → current  (or spawn directly if next is still sentinel) */
    if(game.next_piece.type != TETRO_COUNT) {
        game.current_piece   = game.next_piece;
        game.current_piece.x = BOARD_WIDTH / 2;
        game.current_piece.y = 0;
    } else {
        spawn_piece_for_mode(&game.current_piece, game.mode, game.level);
    }

    /* Generate a fresh next piece */
    spawn_piece_for_mode(&game.next_piece, game.mode, game.level);

    /* Reset DAS + hold-lock + drop bonus on every new piece */
    game.left_das_timer   = 0;  game.right_das_timer  = 0;
    game.left_das_active  = 0;  game.right_das_active = 0;
    game.left_arr_timer   = 0;  game.right_arr_timer  = 0;
    game.hold_used        = 0;

    /* Pokemon countdown tick */
    if(game.pieces_left > 0) game.pieces_left--;
    if(game.pieces_left == 0) {
        game.tier_up_active = 0;  // Clear tier-up flag before spawning new Pokemon
        spawn_random_pokemon();
    }

    /* Immediate top-out check */
    if(check_collision(&game.current_piece, &game.board)) {
        game.state = STATE_GAME_OVER;
        // DON'T start save here - it will start on first frame of game over screen
    }
}

/* ─────────────────────────────────────────────────────────────────────────────
 * Pokemon target & catching
 * ───────────────────────────────────────────────────────────────────────────── */
void spawn_random_pokemon(void) {
    int i;
    check_for_shiny();

    // STEP 4: Determine which mode pool to use
    u8 target_mode_bit;
    u8 use_tier_up = 0;
    u8 use_special_only = 0;
    
    // Check if this is a special spawn (5-line clear bonus)
    if(game.special_spawn_pending) {
        // 50% chance to pull from next tier up
        if((rand() % 100) < 50) {
            use_tier_up = 1;
            game.tier_up_active = 1;  // ADD THIS LINE - Track that tier-up happened
        }
        
        // Determine target mode based on current mode
        switch(game.mode) {
            case MODE_ROOKIE:
                target_mode_bit = use_tier_up ? POKE_MODE_NORMAL : POKE_MODE_ROOKIE;
                break;
            case MODE_NORMAL:
                target_mode_bit = use_tier_up ? POKE_MODE_SUPER : POKE_MODE_NORMAL;
                break;
            case MODE_SUPER:
                target_mode_bit = use_tier_up ? POKE_MODE_HYPER : POKE_MODE_SUPER;
                break;
            case MODE_HYPER:
                target_mode_bit = use_tier_up ? POKE_MODE_MASTER : POKE_MODE_HYPER;
                break;
            case MODE_MASTER:
                // Master mode: 50% chance for special Pokemon instead of tier up
                if(use_tier_up) {
                    use_special_only = 1;
                    target_mode_bit = POKE_MODE_MASTER;
                } else {
                    target_mode_bit = POKE_MODE_MASTER;
                }
                break;
            case MODE_UNOWN:
            case MODE_VIVILLON:
            case MODE_ALCREMIE:
                // Bonus modes: use their own pool
                target_mode_bit = (game.mode == MODE_UNOWN) ? POKE_MODE_UNOWN :
                                  (game.mode == MODE_VIVILLON) ? POKE_MODE_VIVILLON :
                                  POKE_MODE_ALCREMIE;
                break;
            default:
                target_mode_bit = POKE_MODE_ROOKIE;
                break;
        }
    } else {
        // Normal spawn - use current mode
        switch(game.mode) {
            case MODE_ROOKIE:   target_mode_bit = POKE_MODE_ROOKIE;    break;
            case MODE_NORMAL:   target_mode_bit = POKE_MODE_NORMAL;    break;
            case MODE_SUPER:    target_mode_bit = POKE_MODE_SUPER;     break;
            case MODE_HYPER:    target_mode_bit = POKE_MODE_HYPER;     break;
            case MODE_MASTER:   target_mode_bit = POKE_MODE_MASTER;    break;
            case MODE_UNOWN:    target_mode_bit = POKE_MODE_UNOWN;     break;
            case MODE_VIVILLON: target_mode_bit = POKE_MODE_VIVILLON;  break;
            case MODE_ALCREMIE: target_mode_bit = POKE_MODE_ALCREMIE;  break;
            default:            target_mode_bit = POKE_MODE_ROOKIE;    break;
        }
    }

    // Build pool of eligible Pokemon
    int pool_size = 0;
    for(i = 0; i < TOTAL_POKEMON; i++) {
        const PokemonData* pdata = &POKEMON_DATABASE[i];
        
        // Check mode compatibility
        if(!(pdata->modes & target_mode_bit)) continue;
        
        // Check if this mode has non-zero turns
        if(get_pokemon_turns(i, (u8)game.mode) == 0) continue;
        
        // Special spawn filters
        if(game.special_spawn_pending) {
            // Must be uncaught (check both normal and shiny)
            if(game.pokemon_catches[i].unlocked || game.pokemon_catches[i].unlocked_shiny) {
                continue;
            }
            
            // If Master mode tier-up, only allow special Pokemon
            if(use_special_only) {
                if(pdata->special != 1) continue;
            }
        }
        
        pool_size++;
    }

    // Fallback if no uncaught Pokemon in pool
    if(pool_size == 0 && game.special_spawn_pending) {
        // Fall back to caught Pokemon from target tier
        for(i = 0; i < TOTAL_POKEMON; i++) {
            const PokemonData* pdata = &POKEMON_DATABASE[i];
            if((pdata->modes & target_mode_bit) && get_pokemon_turns(i, (u8)game.mode) > 0) {
                if(use_special_only && pdata->special != 1) continue;
                pool_size++;
            }
        }
    }

    // Final fallback: any Pokemon with non-zero turns
    if(pool_size == 0) {
        for(i = 0; i < TOTAL_POKEMON; i++) {
            if(get_pokemon_turns(i, (u8)game.mode) > 0) pool_size++;
        }
    }

    // Absolute fallback
    if(pool_size == 0) {
        game.current_pokemon = (u16)(rand() % TOTAL_POKEMON);
    } else {
        // Pick random from pool
        int target = rand() % pool_size;
        int count = 0;
        u8 picked = 0;
        
        // First pass: try with all filters
        for(i = 0; i < TOTAL_POKEMON; i++) {
            const PokemonData* pdata = &POKEMON_DATABASE[i];
            
            if(!(pdata->modes & target_mode_bit)) continue;
            if(get_pokemon_turns(i, (u8)game.mode) == 0) continue;
            
            if(game.special_spawn_pending) {
                if(game.pokemon_catches[i].unlocked || game.pokemon_catches[i].unlocked_shiny) {
                    continue;
                }
                if(use_special_only && pdata->special != 1) continue;
            }
            
            if(count++ == target) {
                game.current_pokemon = (u16)i;
                picked = 1;
                break;
            }
        }
        
        // Fallback scan if needed
        if(!picked && game.special_spawn_pending) {
            count = 0;
            for(i = 0; i < TOTAL_POKEMON; i++) {
                const PokemonData* pdata = &POKEMON_DATABASE[i];
                if((pdata->modes & target_mode_bit) && get_pokemon_turns(i, (u8)game.mode) > 0) {
                    if(use_special_only && pdata->special != 1) continue;
                    if(count++ == target) {
                        game.current_pokemon = (u16)i;
                        picked = 1;
                        break;
                    }
                }
            }
        }
        
        // Final fallback
        if(!picked) {
            count = 0;
            for(i = 0; i < TOTAL_POKEMON; i++) {
                if(get_pokemon_turns(i, (u8)game.mode) > 0) {
                    if(count++ == target) {
                        game.current_pokemon = (u16)i;
                        break;
                    }
                }
            }
        }
    }

    // Clear the special spawn flag (but keep tier_up_active until Pokemon changes)
    game.special_spawn_pending = 0;
    // Note: tier_up_active stays set and will be cleared when Pokemon despawns

    // Set Pokemon duration
    {
        u8 turns = get_pokemon_turns((int)game.current_pokemon, (u8)game.mode);
        if(turns == 0) turns = 11;
        game.pieces_left = turns;
        game.pokemon_duration = turns;
    }

    load_current_pokemon_sprite();
}

void unlock_pokemon(u16 pokemon_index, u8 is_shiny) {
    if(pokemon_index >= 1349) return;

    u8 was_new_unlock = 0;
    
    if(is_shiny) {
        if(!game.pokemon_catches[pokemon_index].unlocked_shiny) {
            game.pokemon_catches[pokemon_index].unlocked_shiny = 1;
            game.new_dex_entries++;
            was_new_unlock = 1;
        }
        game.pokemon_catches[pokemon_index].catch_count_shiny++;
        game.shinies_caught_this_game++;
    } else {
        if(!game.pokemon_catches[pokemon_index].unlocked) {
            game.pokemon_catches[pokemon_index].unlocked = 1;
            game.new_dex_entries++;
            was_new_unlock = 1;
        }
        game.pokemon_catches[pokemon_index].catch_count++;
        game.pokemon_caught_this_game++;
    }
    
    // Check if this catch unlocks any modes (both shiny and non-shiny count)
    if(was_new_unlock) {
        // Count total unique Pokemon (either form counts)
        int unique_caught = 0;
        for(int i = 0; i < 1349; i++) {
            if(game.pokemon_catches[i].unlocked || game.pokemon_catches[i].unlocked_shiny) {
                unique_caught++;
            }
        }
        
        // Check milestone unlocks
        if(unique_caught == 150 && !game.mode_unlocked[3]) {
            game.mode_unlocked[3] = 1;
            game.mode_unlock_celebration_pending = 4; // HYPER = index 3, but we use 4 for display
        }
        else if(unique_caught == 250 && !game.mode_unlocked[4]) {
            game.mode_unlocked[4] = 1;
            game.mode_unlock_celebration_pending = 5; // MASTER = index 4, but we use 5 for display
        }
        
        // Check specific Pokemon unlocks (either form counts)
        if(pokemon_index == 200 && !game.mode_unlocked[5]) {  // Unown A
            game.mode_unlocked[5] = 1;
            game.mode_unlock_celebration_pending = 6; // UNOWN mode
        }
        else if(pokemon_index == 665 && !game.mode_unlocked[6]) {  // Vivillon Archipelago
            game.mode_unlocked[6] = 1;
            game.mode_unlock_celebration_pending = 7; // VIVILLON mode
        }
        else if(pokemon_index == 868 && !game.mode_unlocked[7]) {  // Alcremie Vanilla Strawberry
            game.mode_unlocked[7] = 1;
            game.mode_unlock_celebration_pending = 8; // ALCREMIE mode
        }
    }
    // NO SAVE HERE - deferred to game over screen
}

void try_catch_pokemon(int lines_cleared) {
    if(lines_cleared < 4) return;
    if(game.current_pokemon >= 1349) return;

    u16 idx   = game.current_pokemon;
    u8  shiny = game.is_shiny;
    u8  was_new = shiny ? !game.pokemon_catches[idx].unlocked_shiny
                        : !game.pokemon_catches[idx].unlocked;

    unlock_pokemon(idx, shiny);

    game.is_new_catch                = was_new;
    game.catch_celebration_active    = 1;
    game.catch_celebration_timer     = 0;
    game.caught_pokemon_id           = idx;
    
    // Play Pokemon catch sound
// DISABLED:     audio_play_sfx(SFX_POKEMON_CATCH);
}

/* ─────────────────────────────────────────────────────────────────────────────
 * Line clearing
 * ───────────────────────────────────────────────────────────────────────────── */
void clear_lines(Board* board) {
    int row;
    game.cleared_line_count = 0;

    for(row = 0; row < BOARD_HEIGHT; row++) {
        int col, full = 1;
        for(col = 0; col < BOARD_WIDTH; col++)
            if(!board->filled[row][col]) { full = 0; break; }
        if(full)
            game.cleared_lines[game.cleared_line_count++] = row;
    }

    if(game.cleared_line_count > 0) {
        game.line_clear_active = 1;
        game.line_clear_timer  = 0;
    }
}

/* Called after the 30-frame flash – removes rows, scores, triggers effects */
static void finish_line_clear(Board* board) {
    if(!game.line_clear_active) return;

    int i, j, k;
    int count = game.cleared_line_count;

    /* Remove each cleared row (shift everything above down) */
    for(i = 0; i < count; i++) {
        int line = game.cleared_lines[i];
        for(j = line; j > 0; j--)
            for(k = 0; k < BOARD_WIDTH; k++) {
                board->grid[j][k]   = board->grid[j-1][k];
                board->filled[j][k] = board->filled[j-1][k];
            }
        for(k = 0; k < BOARD_WIDTH; k++) {
            board->grid[0][k]   = BLACK;
            board->filled[0][k] = 0;
        }
        /* Rows that were above this one shift down – adjust stored indices */
        for(j = i+1; j < count; j++)
            if(game.cleared_lines[j] < line) game.cleared_lines[j]++;
    }

    /* ── Scoring ── */
    game.lines_cleared += count;

    {
        // Base scores for line clears
        u32 base = 0;
        switch(count) {
            case 1: base =   40; break;   // Single
            case 2: base =  100; break;   // Double
            case 3: base =  300; break;   // Triple
            case 4: base = 1200; break;   // Tetris
            case 5: base = 2000; break;   // Pentomino (changed from 3000)
        }
        
        // Back-to-Back (B2B) multiplier for 4+ line clears
        if(count >= 4) {
            if(game.b2b_active) {
                // Apply 1.5x multiplier (50% bonus)
                base = (base * 3) / 2;  // base * 1.5
                game.b2b_count++;
            } else {
                // Start B2B chain
                game.b2b_active = 1;
                game.b2b_count = 1;
            }
        } else if(count > 0 && count < 4) {
            // Break B2B chain on small clears
            game.b2b_active = 0;
            game.b2b_count = 0;
        }
        
        // Check for Perfect Clear (all blocks removed)
        int is_perfect_clear = 1;
        for(int y = 0; y < BOARD_HEIGHT && is_perfect_clear; y++) {
            for(int x = 0; x < BOARD_WIDTH; x++) {
                if(board->filled[y][x]) {
                    is_perfect_clear = 0;
                    break;
                }
            }
        }
        
        if(is_perfect_clear && count > 0) {
            // Perfect Clear: 10x multiplier
            base *= 10;
            // Perfect Clear maintains B2B chain (considered a "difficult" clear)
            if(!game.b2b_active) {
                game.b2b_active = 1;
                game.b2b_count = 1;
            }
        }
        
        // Apply level multiplier: base × (level + 1)
        u32 line_clear_score = base * (game.level + 1);
        
        // Add to score
        game.score += line_clear_score;
    }

    /* Level up every 10 lines */
    int leveled_up = 0;
    {
        int nl = 1 + (int)game.lines_cleared / 10;
        if(nl > 99) nl = 99;
        if(nl > (int)game.level) {
            game.level = nl;
            update_fall_speed();
            leveled_up = 1;
        }
    }

    /* Streak + catch attempt */
    update_big_clear_streak(count);
    try_catch_pokemon(count);

    /* Play SFX - catch takes priority, then level up, then normal clear */
    if(game.catch_celebration_active) {
        /* SFX_POKEMON_CATCH already played inside try_catch_pokemon() */
    } else if(leveled_up) {
// DISABLED:         audio_play_sfx(SFX_LEVEL_UP);
    } else if(count > 0) {
// DISABLED:         audio_play_sfx(SFX_CLEAR);
    }

    /* Particles + shake on Tetris (4) or Pentris (5) —
     * skip when a catch celebration is about to play (overlay covers them) */
    if(count >= 4 && !game.catch_celebration_active) {
        start_screen_shake(count >= 5 ? 5 : 4);
    }

    /* Reset animation state */
    game.line_clear_active  = 0;
    game.cleared_line_count = 0;

    /* Next piece – unless catch celebration is about to play */
    if(!game.catch_celebration_active)
        spawn_next_piece();
}

/* ─────────────────────────────────────────────────────────────────────────────
 * Fall-speed curve  (Game Boy Tetris style)
 * ───────────────────────────────────────────────────────────────────────────── */
static void update_fall_speed(void) {
    /* Modern-ish gravity curve.
       fall_speed is in frames-per-row (60fps). */
    if      (game.level >= 22) game.fall_speed = 2;
    else if (game.level >= 18) game.fall_speed = 3;
    else if (game.level >= 15) game.fall_speed = 4;
    else if (game.level >= 12) game.fall_speed = 5;
    else if (game.level >= 10) game.fall_speed = 6;
    else if (game.level >=  9) game.fall_speed = 7;
    else if (game.level >=  8) game.fall_speed = 8;
    else if (game.level >=  7) game.fall_speed = 9;
    else if (game.level >=  6) game.fall_speed = 10;
    else if (game.level >=  5) game.fall_speed = 11;
    else if (game.level >=  4) game.fall_speed = 12;
    else if (game.level >=  3) game.fall_speed = 13;
    else if (game.level >=  2) game.fall_speed = 14;
    else                       game.fall_speed = 15; /* Level 1: ~250ms per row */
}


/* ─────────────────────────────────────────────────────────────────────────────
 * Colour helper
 * ───────────────────────────────────────────────────────────────────────────── */
u16 get_tetromino_color(TetrominoType type) {
    return tetromino_colors[type];
}

/* ─────────────────────────────────────────────────────────────────────────────
 * Screen shake
 * ───────────────────────────────────────────────────────────────────────────── */
void start_screen_shake(u8 intensity) {
    game.screen_shake_active = 1;
    game.screen_shake_timer  = 12;
    game.shake_intensity     = intensity;
}

void update_screen_shake(void) {
    if(!game.screen_shake_active) return;
    if(--game.screen_shake_timer == 0) {
        game.screen_shake_active = 0;
        game.shake_offset_x      = 0;
        game.shake_offset_y      = 0;
    } else {
        game.shake_offset_x = (rand() % (game.shake_intensity*2+1)) - game.shake_intensity;
        game.shake_offset_y = (rand() % (game.shake_intensity*2+1)) - game.shake_intensity;
    }
}

/* ─────────────────────────────────────────────────────────────────────────────
 * Catch celebration  (timer is ticked in update_game; extend effects here)
 * ───────────────────────────────────────────────────────────────────────────── */
void update_catch_celebration(void) {
    /* placeholder – add mid-celebration sparkles here later */
}

/* ─────────────────────────────────────────────────────────────────────────────
 * Dead declarations in main.h – stubs so the linker is happy
 * ───────────────────────────────────────────────────────────────────────────── */
void init_pokemon_data(void)  { /* data lives in pokemon_database.c */ }
u16  get_random_pokemon(void) { return rand() % TOTAL_POKEMON; }
void vsync(void)              { VBlankIntrWait(); }

/* show_game_over_screen / handle_game_over_input – declared in main.h but never
 * called.  Tiny stubs so linking doesn't fail. */
void show_game_over_screen(void)   { /* handled by show_game_over() in graphics.c */ }
void handle_game_over_input(void)  { /* handled inside handle_input() STATE_GAME_OVER */ }

/* ─────────────────────────────────────────────────────────────────────────────
 * Shiny-streak helpers
 * ───────────────────────────────────────────────────────────────────────────── */
static void update_big_clear_streak(int lines_cleared) {
    if(lines_cleared >= 4)  game.big_clear_streak++;
    else if(lines_cleared > 0) game.big_clear_streak = 0;
    
    // STEP 3: Trigger special spawn on 5-line clear
    if(lines_cleared == 5) {
        game.special_spawn_pending = 1;
    }
}

static void check_for_shiny(void) {
    if(game.big_clear_streak >= 10) {
        game.is_shiny         = 1;
        game.big_clear_streak = 0;
    } else {
        game.is_shiny = 0;
    }
}

/* ─────────────────────────────────────────────────────────────────────────────
 * Sprite display helpers (called from update_game each frame)
 * ───────────────────────────────────────────────────────────────────────────── */
static void update_sprite_display(void) {
    /* Sprite blit moved into render_game() (graphics.c) so it runs
     * AFTER clear_screen(BLACK).  Only tick the cache LRU counter here. */
    update_sprite_system();
}

/* ─────────────────────────────────────────────────────────────────────────────
 * Macro: reset lock-delay when the piece moves / rotates while grounded
 * ───────────────────────────────────────────────────────────────────────────── */
#define RESET_LOCK_IF_ACTIVE() do { \
    if(game.lock_delay_active && game.lock_resets_used < MAX_LOCK_RESETS) { \
        game.lock_delay_timer = 0; \
        game.lock_resets_used++; \
    } \
} while(0)

/* ─────────────────────────────────────────────────────────────────────────────
 * INPUT  –  full 10-state machine, SDK edge-detection
 * ───────────────────────────────────────────────────────────────────────────── */
void handle_input(void) {
    scanKeys();
    u16 down = keysDown();   /* pressed this frame */
    u16 held = keysHeld();   /* held (includes this frame) */

    /* ── SPLASH ───────────────────────────────────────────────────────────── */
    if(game.state == STATE_SPLASH) {
        // Splash shows for exactly 2 seconds (120 frames), no skipping
        return;
    }

    /* ── TITLE (with integrated horizontal menu) ──────────────────────────── */
    if(game.state == STATE_TITLE) {
        // CHEAT CODES - Check for button combos
        
        // CHEAT 1: L + R + SELECT + A = Unlock entire Pokedex
        if((held & KEY_L) && (held & KEY_R) && (held & KEY_SELECT) && (held & KEY_A)) {
            // Unlock all Pokemon (set to caught 1 time each)
            for(int i = 0; i < 1349; i++) {
                game.pokemon_catches[i].unlocked = 1;
                if(game.pokemon_catches[i].catch_count == 0) {
                    game.pokemon_catches[i].catch_count = 1;
                }
            }
            
            // Save the unlocked Pokemon data
            save_pokemon_progress_deferred(game.pokemon_catches);
            
            // Continue to normal menu navigation below
        }
        
        // CHEAT 2: L + R + SELECT + B = Unlock all shinies
        if((held & KEY_L) && (held & KEY_R) && (held & KEY_SELECT) && (held & KEY_B)) {
            // Unlock all shiny Pokemon
            for(int i = 0; i < 1349; i++) {
                game.pokemon_catches[i].unlocked_shiny = 1;
                if(game.pokemon_catches[i].catch_count_shiny == 0) {
                    game.pokemon_catches[i].catch_count_shiny = 1;
                }
            }
            
            // Save the unlocked shiny Pokemon data
            save_pokemon_progress_deferred(game.pokemon_catches);
        }
        
        // Horizontal navigation with LEFT/RIGHT
        if(down & KEY_LEFT)  { 
            if(game.menu_selection > 0) {
                game.menu_selection--;
                audio_play_sfx(menu_move_pcm, menu_move_pcm_size);  // Menu move sound
            }
        }
        if(down & KEY_RIGHT) { 
            if(game.menu_selection < 4) {
                game.menu_selection++;
                audio_play_sfx(menu_move_pcm, menu_move_pcm_size);  // Menu move sound
            }
        }
        
        // Activate selected menu item with A or START
        if(down & (KEY_START | KEY_A)) {
            audio_play_sfx(menu_select_pcm, menu_select_pcm_size);  // Menu select sound
            switch(game.menu_selection) {
                case 0: game.state = STATE_MODE_SELECT; break;
                case 1: 
                    game.state = STATE_POKEDEX;
                    pokedex_ui_reset();
// DISABLED:                     audio_play_music(MUSIC_POKEDEX);  // Pokedex music
                    break;
                case 2: 
                    game.state = STATE_HIGHSCORES;
// DISABLED:                     audio_play_music(MUSIC_POKEDEX);  // Highscores music
                    break;
                case 3: 
                    game.state = STATE_OPTIONS;
                    options_ui_reset();
// DISABLED:                     audio_play_music(MUSIC_POKEDEX);  // Options music
                    break;
                case 4: 
                    game.state = STATE_CREDITS;
// DISABLED:                     audio_play_music(MUSIC_POKEDEX);  // Credits music
                    break;
            }
        }
        return;
    }

    /* ── MAIN MENU (now unused - integrated into title screen) ───────────── */
    if(game.state == STATE_MAIN_MENU) {
        // Redirect to title screen (backward compatibility)
        game.state = STATE_TITLE;
        return;
    }

    /* ── MODE SELECT ──────────────────────────────────────────────────────── */
    if(game.state == STATE_MODE_SELECT) {
        // Navigation with wrapping - show ALL modes (including locked)
        if(down & KEY_LEFT) {
            if(game.menu_selection > 0) game.menu_selection--;
            else game.menu_selection = 6; // Wrap to BACK
            audio_play_sfx(menu_move_pcm, menu_move_pcm_size);  // Menu move sound
        }
        if(down & KEY_RIGHT) {
            if(game.menu_selection < 6) game.menu_selection++;
            else game.menu_selection = 0; // Wrap to ROOKIE
            audio_play_sfx(menu_move_pcm, menu_move_pcm_size);  // Menu move sound
        }
        if(down & KEY_B) { 
            game.state = STATE_TITLE; 
            game.menu_selection = 0;
// DISABLED:             audio_play_music(MUSIC_MAINMENU);  // Return to main menu music
        }
        if(down & (KEY_START | KEY_A)) {
            if(game.menu_selection == 6) {
                // Selected BACK
                audio_play_sfx(menu_select_pcm, menu_select_pcm_size);  // Menu select sound
                game.state = STATE_TITLE;
                game.menu_selection = 0;
// DISABLED:                 audio_play_music(MUSIC_MAINMENU);  // Return to main menu music
            } else if(game.menu_selection == 5) {
                // Selected BONUS - go to bonus submenu (always unlocked)
                audio_play_sfx(menu_select_pcm, menu_select_pcm_size);  // Menu select sound
                game.menu_selection = 0; // Reset for submenu
                game.state = STATE_BONUS_SELECT;
            } else {
                // Selected a main mode (0-4) - check if unlocked before starting
                if(game.mode_unlocked[game.menu_selection]) {
                    audio_play_sfx(menu_select_pcm, menu_select_pcm_size);  // Menu select sound
                    game.mode = (GameMode)game.menu_selection;
                    init_game();   /* sets state = STATE_GAMEPLAY */
                }
                // If locked, pressing A/START does nothing (lock icon shows)
            }
        }
        return;
    }

    /* ── BONUS MODE SELECT ──────────────────────────────────────────────── */
    if(game.state == STATE_BONUS_SELECT) {
        // Navigation with wrapping - show ALL bonus modes (including locked)
        if(down & KEY_LEFT) {
            if(game.menu_selection > 0) game.menu_selection--;
            else game.menu_selection = 3; // Wrap to BACK
            audio_play_sfx(menu_move_pcm, menu_move_pcm_size);  // Menu move sound
        }
        if(down & KEY_RIGHT) {
            if(game.menu_selection < 3) game.menu_selection++;
            else game.menu_selection = 0; // Wrap to first
            audio_play_sfx(menu_move_pcm, menu_move_pcm_size);  // Menu move sound
        }
        if(down & (KEY_START | KEY_A)) {
            if(game.menu_selection == 3) {
                // Selected BACK - return to main mode select
                audio_play_sfx(menu_select_pcm, menu_select_pcm_size);  // Menu select sound
                game.menu_selection = 5; // Set to BONUS position
                game.state = STATE_MODE_SELECT;
            } else {
                // Selected a bonus mode - check if unlocked before starting
                if(game.mode_unlocked[5 + game.menu_selection]) {
                    audio_play_sfx(menu_select_pcm, menu_select_pcm_size);  // Menu select sound
                    GameMode bonus_modes[3] = {MODE_UNOWN, MODE_VIVILLON, MODE_ALCREMIE};
                    game.mode = bonus_modes[game.menu_selection];
                    init_game();
                }
                // If locked, pressing A/START does nothing (lock icon shows)
            }
        }
        if(down & KEY_B) {
            // B button - go back to main mode select
            game.menu_selection = 5; // Set to BONUS position
            game.state = STATE_MODE_SELECT;
        }
        return;
    }

    /* ── PAUSE ────────────────────────────────────────────────────────────── */
    if(game.state == STATE_PAUSE) {
        // Navigate with LEFT/RIGHT or UP/DOWN
        if(down & (KEY_LEFT | KEY_UP)) {
            if(game.pause_menu_selection > 0) {
                game.pause_menu_selection--;
                audio_play_sfx(menu_move_pcm, menu_move_pcm_size);  // Menu move sound
            }
        }
        if(down & (KEY_RIGHT | KEY_DOWN)) {
            if(game.pause_menu_selection < 1) {
                game.pause_menu_selection++;
                audio_play_sfx(menu_move_pcm, menu_move_pcm_size);  // Menu move sound
            }
        }
        
        // Select with A or START
        if(down & (KEY_A | KEY_START)) {
            audio_play_sfx(menu_select_pcm, menu_select_pcm_size);  // Menu select sound
            if(game.pause_menu_selection == 0) {
                // Resume
                game.state = STATE_GAMEPLAY;
// DISABLED:                 audio_resume_music();  // Resume gameplay music
            } else {
                // Quit to title
                game.state = STATE_TITLE;
                game.menu_selection = 0;
// DISABLED:                 audio_play_music(MUSIC_MAINMENU);  // Return to main menu music
            }
        }
        
        // B also resumes (quick unpause)
        if(down & KEY_B) {
            game.state = STATE_GAMEPLAY;
// DISABLED:             audio_resume_music();  // Resume gameplay music
        }
        
        return;
    }

    /* ── GAME OVER ────────────────────────────────────────────────────────── */
    if(game.state == STATE_GAME_OVER) {
        // If mode unlock celebration is showing, dismiss it on any button
        if(game.mode_unlock_celebration_pending > 0) {
            if(down & (KEY_A | KEY_B | KEY_START | KEY_SELECT)) {
                game.mode_unlock_celebration_pending = 0;
            }
            return; // Block all other input while celebration shows
        }
        
        // Block ALL input while save is in progress
        int save_in_progress = save_game_async_in_progress();
        if(save_in_progress) return;
        
        // After save completes, allow menu navigation with LEFT/RIGHT
        // (since options are on the same horizontal line)
        if(down & KEY_LEFT) { 
            if(game.game_over_selection > 0) {
                game.game_over_selection--;
                audio_play_sfx(menu_move_pcm, menu_move_pcm_size);  // Menu move sound
            }
        }
        if(down & KEY_RIGHT) { 
            if(game.game_over_selection < 1) {
                game.game_over_selection++;
                audio_play_sfx(menu_move_pcm, menu_move_pcm_size);  // Menu move sound
            }
        }
        if(down & (KEY_START | KEY_A)) {
            audio_play_sfx(menu_select_pcm, menu_select_pcm_size);  // Menu select sound
            if(game.game_over_selection == 0) {
                init_game();   /* restart - music starts in init_game */
            } else { 
                game.state = STATE_TITLE;
                game.menu_selection = 0;
// DISABLED:                 audio_play_music(MUSIC_MAINMENU);  // Return to main menu music
            }
        }
        return;
    }

    
/* ── SUB-SCREENS  (Pokedex / Highscores / Options / Credits) ──────────── */
if(game.state == STATE_POKEDEX) {
    // Pokédex navigation handled by pokedex_ui module
    pokedex_ui_handle_input(down, held);
    if(down & KEY_B) {
        game.state = STATE_TITLE;
// DISABLED:         audio_play_music(MUSIC_MAINMENU);  // Return to main menu music
    }
    return;
}
if(game.state == STATE_HIGHSCORES) {
    highscores_ui_handle_input(down, held);
    if(down & KEY_B) {
        game.state = STATE_TITLE;
// DISABLED:         audio_play_music(MUSIC_MAINMENU);  // Return to main menu music
    }
    return;
}
if(game.state == STATE_OPTIONS) {
    options_ui_handle_input(down, held);
    if(down & KEY_B) {
        game.state = STATE_TITLE;
// DISABLED:         audio_play_music(MUSIC_MAINMENU);  // Return to main menu music
    }
    return;
}
if(game.state == STATE_CREDITS) {
    if(down & KEY_B) {
        game.state = STATE_TITLE;
// DISABLED:         audio_play_music(MUSIC_MAINMENU);  // Return to main menu music
    }
    return;
}

/* ── GAMEPLAY ─────────────────────────────────────────────────────────── */
    if(game.state != STATE_GAMEPLAY) return;

    /* Dismiss catch celebration on A or Start (before normal controls) */
    if(game.catch_celebration_active) {
        if(down & (KEY_A | KEY_START)) {
            game.catch_celebration_active = 0;
            spawn_random_pokemon();   /* new target — sets game.is_shiny */
            spawn_next_piece();
            /* Play shiny sound after catch sound has finished */
            if(game.is_shiny) {
// DISABLED:                 audio_play_sfx(SFX_SHINY);
            }
        }
        return;   /* eat all other input while celebration is up */
    }

    /* Also eat input during line-clear flash */
    if(game.line_clear_active) return;

    /* Pause toggle */
    if(down & KEY_START) { 
        game.state = STATE_PAUSE;
// DISABLED:         audio_pause_music();  // Pause gameplay music
        return;
    }

    /* ── Left movement + DAS ──────────────────────────────────────────────── */
    if(down & KEY_LEFT) {
        /* initial tap */
        Tetromino t = game.current_piece; t.x--;
        if(!check_collision(&t, &game.board)) { game.current_piece = t; RESET_LOCK_IF_ACTIVE(); }
        /* arm DAS timers from zero */
        game.left_das_timer  = 0;
        game.left_das_active = 0;
        game.left_arr_timer  = 0;
    }
    if(held & KEY_LEFT) {
        if(!game.left_das_active) {
            /* counting up to DAS_DELAY before auto-repeat begins */
            if(++game.left_das_timer >= DAS_DELAY) {
                game.left_das_active = 1;
                game.left_arr_timer  = 0;
            }
        } else {
            /* auto-repeat at ARR rate */
            if(++game.left_arr_timer >= ARR) {
                game.left_arr_timer = 0;
                Tetromino t = game.current_piece; t.x--;
                if(!check_collision(&t, &game.board)) { game.current_piece = t; RESET_LOCK_IF_ACTIVE(); }
            }
        }
    } else {
        game.left_das_timer  = 0;
        game.left_das_active = 0;
        game.left_arr_timer  = 0;
    }

    /* ── Right movement + DAS ─────────────────────────────────────────────── */
    if(down & KEY_RIGHT) {
        Tetromino t = game.current_piece; t.x++;
        if(!check_collision(&t, &game.board)) { game.current_piece = t; RESET_LOCK_IF_ACTIVE(); }
        game.right_das_timer  = 0;
        game.right_das_active = 0;
        game.right_arr_timer  = 0;
    }
    if(held & KEY_RIGHT) {
        if(!game.right_das_active) {
            if(++game.right_das_timer >= DAS_DELAY) {
                game.right_das_active = 1;
                game.right_arr_timer  = 0;
            }
        } else {
            if(++game.right_arr_timer >= ARR) {
                game.right_arr_timer = 0;
                Tetromino t = game.current_piece; t.x++;
                if(!check_collision(&t, &game.board)) { game.current_piece = t; RESET_LOCK_IF_ACTIVE(); }
            }
        }
    } else {
        game.right_das_timer  = 0;
        game.right_das_active = 0;
        game.right_arr_timer  = 0;
    }

    /* ── Rotate CW (A) / CCW (B) ─────────────────────────────────────────── */
    if(down & KEY_A) { rotate_tetromino(&game.current_piece,  1); RESET_LOCK_IF_ACTIVE(); }
    if(down & KEY_B) { rotate_tetromino(&game.current_piece, -1); RESET_LOCK_IF_ACTIVE(); }

    /* ── L/R buttons with control swap support ───────────────────────────── */
    // Get control swap setting (0 = L=Hold/R=Flip, 1 = L=Flip/R=Hold)
    u8 swap = get_control_swap();
    
    /* ── Flip button (R by default, or L if swapped) ─────────────────────── */
    int flip_pressed = swap ? (down & KEY_L) : (down & KEY_R);
    if(flip_pressed) {
        flip_tetromino(&game.current_piece);
        RESET_LOCK_IF_ACTIVE();
    }

    /* ── Hold button (L by default, or R if swapped; SELECT always works) ── */
    int hold_pressed = swap ? (down & KEY_R) : (down & KEY_L);
    if(hold_pressed || (down & KEY_SELECT)) {
        if(!game.hold_used) {
            if(game.hold_piece.type == TETRO_COUNT) {
                /* nothing held yet – stash current, spawn fresh */
                game.hold_piece = game.current_piece;
                game.hold_piece.x = BOARD_WIDTH / 2;
                game.hold_piece.y = 0;
                spawn_next_piece();
            } else {
                /* swap current ↔ hold */
                Tetromino tmp        = game.current_piece;
                game.current_piece   = game.hold_piece;
                game.hold_piece      = tmp;
                game.current_piece.x = BOARD_WIDTH / 2;
                game.current_piece.y = 0;
                game.hold_piece.x    = 0;
                game.hold_piece.y    = 0;
            }
            game.hold_used = 1;
        }
    }

    /* ── Soft drop (DOWN held) ────────────────────────────────────────────── */
    if(held & KEY_DOWN) {
        game.fall_timer = game.fall_speed;   /* force an immediate gravity tick */
        // Award 1 point per row dropped (will be applied on next gravity tick)
    }

    /* ── Hard drop (UP press) ─────────────────────────────────────────────── */
    if(down & KEY_UP) {
        /* drop until collision, counting rows */
        int rows_dropped = 0;
        while(1) {
            game.current_piece.y++;
            if(check_collision(&game.current_piece, &game.board)) {
                game.current_piece.y--;
                break;
            }
            rows_dropped++;
        }
        /* Award 2 points per row dropped - add immediately to score */
        u32 hard_drop_points = rows_dropped * 2;
        game.score += hard_drop_points;
        
        /* lock immediately */
        place_tetromino(&game.current_piece, &game.board);
        clear_lines(&game.board);
        game.hold_used         = 0;
        game.lock_delay_active = 0;
        game.lock_delay_timer  = 0;
        game.lock_resets_used  = 0;
        if(!game.line_clear_active) spawn_next_piece();
    }
}

/* ─────────────────────────────────────────────────────────────────────────────
 * UPDATE  –  per-frame game logic (only runs in STATE_GAMEPLAY)
 * ───────────────────────────────────────────────────────────────────────────── */
void update_game(void) {
    if(game.state != STATE_GAMEPLAY) return;

    /* ── Sprite display (OAM + cache tick) ──────────────────────────────── */
    update_sprite_display();

    /* ── Line-clear flash animation ───────────────────────────────────────── */
    if(game.line_clear_active) {
        if(++game.line_clear_timer >= 18)
            finish_line_clear(&game.board);
        return;   /* everything else paused while flashing */
    }

    /* ── Catch celebration ────────────────────────────────────────────────── */
    if(game.catch_celebration_active) {
        update_catch_celebration();
        if(++game.catch_celebration_timer >= 120) {
            game.catch_celebration_active = 0;
            spawn_random_pokemon();   /* new target — sets game.is_shiny */
            spawn_next_piece();
            /* Play shiny sound now, after catch sound has finished */
            if(game.is_shiny) {
// DISABLED:                 audio_play_sfx(SFX_SHINY);
            }
        }
        return;   /* gameplay paused during celebration */
    }

    /* ── Per-frame effects ────────────────────────────────────────────────── */
    update_screen_shake();

    /* ── Lock delay ───────────────────────────────────────────────────────── */
    {
        Tetromino probe = game.current_piece;
        probe.y++;
        int grounded = check_collision(&probe, &game.board);

        if(grounded) {
            if(!game.lock_delay_active) {
                game.lock_delay_active = 1;
                game.lock_delay_timer  = 0;
            } else if(++game.lock_delay_timer >= LOCK_DELAY_DURATION ||
                      game.lock_resets_used  >= MAX_LOCK_RESETS) {
                /* lock the piece */
                place_tetromino(&game.current_piece, &game.board);
                clear_lines(&game.board);
                game.hold_used         = 0;
                game.lock_delay_active = 0;
                game.lock_delay_timer  = 0;
                game.lock_resets_used  = 0;
                if(!game.line_clear_active) spawn_next_piece();
                return;
            }
        } else {
            /* piece left the ground – cancel lock delay */
            game.lock_delay_active = 0;
            game.lock_delay_timer  = 0;
        }
    }

    /* ── Gravity ──────────────────────────────────────────────────────────── */
    if(++game.fall_timer >= game.fall_speed) {
        game.fall_timer = 0;
        Tetromino probe = game.current_piece;
        probe.y++;
        if(!check_collision(&probe, &game.board)) {
            game.current_piece.y++;
            // Award soft drop bonus if DOWN is held (1 point per row)
            if(keysHeld() & KEY_DOWN) {
                game.score += 1;  // Add immediately to score
            }
        }
        /* if grounded, lock-delay (above) will handle locking on subsequent frames */
    }
}