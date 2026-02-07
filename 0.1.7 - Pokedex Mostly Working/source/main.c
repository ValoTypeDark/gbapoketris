#include "main.h"
#include "save.h"
#include "pokemon_database.h"
#include "sprite_manager.h"
#include "pokedex_ui.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
int main(void) {
    /* Video – SDK wrapper, not raw register */
    SetMode(MODE_3 | BG2_ON);

    /* VBlank IRQ – absolutely required before any VBlankIntrWait() */
    irqInit();
    irqEnable(IRQ_VBLANK);

    /* RNG seed from hardware registers — timer counters + raw key register
     * gives a different value every boot depending on exact timing */
    {
        volatile u16* key_reg = (volatile u16*)0x04000130;
        u32 seed = (u32)REG_TM0CNT_L ^ ((u32)REG_TM1CNT_L << 8)
                 ^ ((u32)(*key_reg) << 16) ^ 0xDEAD;
        srand(seed);
    }

    /* Zero the entire game struct so every timer / flag starts clean */
    memset(&game, 0, sizeof(GameData));

    /* Persistent data */
    init_save_system();   /* internally calls load_game() */

    /* Cold-start state */
    game.state          = STATE_TITLE;
    game.mode           = MODE_ROOKIE;
    game.level          = 1;
    game.fall_speed     = 30;
    game.menu_selection = 0;

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

    init_board(&game.board);
    init_sprite_system();

    game.state         = STATE_GAMEPLAY;
    game.score         = 0;
    game.lines_cleared = 0;
    game.level         = 1;
    game.fall_speed    = 30;
    game.fall_timer    = 0;

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

    /* Pokemon catch tracking */
    for(i = 0; i < 1349; i++) {
        if(i < TOTAL_POKEMON)
            game.pokemon_catches[i].dex_number = POKEMON_DATABASE[i].dex_number;
        game.pokemon_catches[i].unlocked            = 0;
        game.pokemon_catches[i].unlocked_shiny      = 0;
        game.pokemon_catches[i].catch_count         = 0;
        game.pokemon_catches[i].catch_count_shiny   = 0;
    }

    /* Shiny / streak */
    game.is_shiny                  = 0;
    game.big_clear_streak          = 0;
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
        /* Bonus modes have their own dedicated spawners once implemented;
         * until then they fall through to 0 like Rookie. */
        case MODE_UNOWN:
        case MODE_VIVILLON:
        case MODE_ALCREMIE:
        case MODE_ROOKIE:
        case MODE_BONUS:
        case MODE_BACK:
        case MODE_COUNT:
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

    /* Reset DAS + hold-lock on every new piece */
    game.left_das_timer   = 0;  game.right_das_timer  = 0;
    game.left_das_active  = 0;  game.right_das_active = 0;
    game.left_arr_timer   = 0;  game.right_arr_timer  = 0;
    game.hold_used        = 0;

    /* Pokemon countdown tick */
    if(game.pieces_left > 0) game.pieces_left--;
    if(game.pieces_left == 0) spawn_random_pokemon();

    /* Immediate top-out check */
    if(check_collision(&game.current_piece, &game.board)) {
        game.state = STATE_GAME_OVER;

    }
}

/* ─────────────────────────────────────────────────────────────────────────────
 * Pokemon target & catching
 * ───────────────────────────────────────────────────────────────────────────── */
void spawn_random_pokemon(void) {
    check_for_shiny();

    /* Map game.mode → the POKE_MODE bit for this mode */
    u8 mode_bit;
    switch(game.mode) {
        case MODE_ROOKIE:   mode_bit = POKE_MODE_ROOKIE;    break;
        case MODE_NORMAL:   mode_bit = POKE_MODE_NORMAL;    break;
        case MODE_SUPER:    mode_bit = POKE_MODE_SUPER;     break;
        case MODE_HYPER:    mode_bit = POKE_MODE_HYPER;     break;
        case MODE_MASTER:   mode_bit = POKE_MODE_MASTER;    break;
        case MODE_UNOWN:    mode_bit = POKE_MODE_UNOWN;     break;
        case MODE_VIVILLON: mode_bit = POKE_MODE_VIVILLON;  break;
        case MODE_ALCREMIE: mode_bit = POKE_MODE_ALCREMIE;  break;
        default:            mode_bit = POKE_MODE_ROOKIE;    break;
    }

    /* Count how many Pokemon match this mode */
    int pool_size = 0;
    {
        int i;
        for(i = 0; i < TOTAL_POKEMON; i++)
            if(POKEMON_DATABASE[i].modes & mode_bit) pool_size++;
    }

    if(pool_size == 0) {
        /* Fallback: no Pokemon tagged for this mode — pick any */
        game.current_pokemon = rand() % TOTAL_POKEMON;
    } else {
        /* Pick a random index into the filtered pool */
        int target = rand() % pool_size;
        int i, count = 0;
        for(i = 0; i < TOTAL_POKEMON; i++) {
            if(POKEMON_DATABASE[i].modes & mode_bit) {
                if(count == target) {
                    game.current_pokemon = (u16)i;
                    break;
                }
                count++;
            }
        }
    }

    {
        u8 turns = get_pokemon_turns((int)game.current_pokemon, (u8)game.mode);
        if(turns == 0) turns = 11;   /* safety: never zero */
        game.pieces_left      = turns;
        game.pokemon_duration = turns;
    }
    load_current_pokemon_sprite();
}

void unlock_pokemon(u16 pokemon_index, u8 is_shiny) {
    if(pokemon_index >= 1349) return;

    if(is_shiny) {
        if(!game.pokemon_catches[pokemon_index].unlocked_shiny) {
            game.pokemon_catches[pokemon_index].unlocked_shiny = 1;
            game.new_dex_entries++;
        }
        game.pokemon_catches[pokemon_index].catch_count_shiny++;
        game.shinies_caught_this_game++;
    } else {
        if(!game.pokemon_catches[pokemon_index].unlocked) {
            game.pokemon_catches[pokemon_index].unlocked = 1;
            game.new_dex_entries++;
        }
        game.pokemon_catches[pokemon_index].catch_count++;
        game.pokemon_caught_this_game++;
    }
    save_game();
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
        u32 base = 0;
        switch(count) {
            case 1: base =   40; break;
            case 2: base =  100; break;
            case 3: base =  300; break;
            case 4: base = 1200; break;
            case 5: base = 3000; break;   /* pentomino pentris */
        }
        game.score += base * game.level;
    }

    /* Level up every 10 lines */
    {
        int nl = 1 + (int)game.lines_cleared / 10;
        if(nl > 99) nl = 99;
        if(nl > (int)game.level) { game.level = nl; update_fall_speed(); }
    }

    /* Streak + catch attempt */
    update_big_clear_streak(count);
    try_catch_pokemon(count);

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
    // Modern Tetris speed curve - much faster starting speed
    if       (game.level >= 100) game.fall_speed = 1;   // Ultimate speed
    else if  (game.level >= 90)  game.fall_speed = 1;
    else if  (game.level >= 80)  game.fall_speed = 2;
    else if  (game.level >= 70)  game.fall_speed = 2;
    else if  (game.level >= 60)  game.fall_speed = 3;
    else if  (game.level >= 50)  game.fall_speed = 3;
    else if  (game.level >= 40)  game.fall_speed = 4;
    else if  (game.level >= 30)  game.fall_speed = 5;
    else if  (game.level >= 25)  game.fall_speed = 6;
    else if  (game.level >= 20)  game.fall_speed = 7;
    else if  (game.level >= 15)  game.fall_speed = 8;
    else if  (game.level >= 10)  game.fall_speed = 10;
    else if  (game.level >=  7)  game.fall_speed = 12;
    else if  (game.level >=  5)  game.fall_speed = 15;
    else if  (game.level >=  3)  game.fall_speed = 20;
    else                         game.fall_speed = 30;  // Starting speed: 500ms (was 883ms!)
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

    /* ── TITLE ────────────────────────────────────────────────────────────── */
    if(game.state == STATE_TITLE) {
        if(down & KEY_START) { game.state = STATE_MAIN_MENU; game.menu_selection = 0; }
        return;
    }

    /* ── MAIN MENU ────────────────────────────────────────────────────────── */
    if(game.state == STATE_MAIN_MENU) {
        if(down & KEY_UP)   { if(game.menu_selection > 0) game.menu_selection--; }
        if(down & KEY_DOWN) { if(game.menu_selection < 4) game.menu_selection++; }
        if(down & (KEY_START | KEY_A)) {
            switch(game.menu_selection) {
                case 0: game.state = STATE_MODE_SELECT; break;
                case 1: game.state = STATE_POKEDEX;     pokedex_ui_reset(); break;
                case 2: game.state = STATE_HIGHSCORES;  break;
                case 3: game.state = STATE_OPTIONS;     break;
                case 4: game.state = STATE_CREDITS;     break;
            }
        }
        return;
    }

    /* ── MODE SELECT ──────────────────────────────────────────────────────── */
    if(game.state == STATE_MODE_SELECT) {
        if(down & KEY_LEFT)  { if(game.mode > 0) game.mode--; }
        if(down & KEY_RIGHT) { if(game.mode < 6) game.mode++; }
        if(down & KEY_B)     { game.state = STATE_MAIN_MENU; game.mode = MODE_ROOKIE; }
        if(down & (KEY_START | KEY_A)) {
            if(game.mode == MODE_BACK) {
                game.state = STATE_MAIN_MENU;  game.mode = MODE_ROOKIE;
            } else if(game.mode == MODE_BONUS) {
                /* bonus sub-menu – placeholder, go back for now */
                game.state = STATE_MAIN_MENU;  game.mode = MODE_ROOKIE;
            } else {
                init_game();   /* sets state = STATE_GAMEPLAY */
            }
        }
        return;
    }

    /* ── PAUSE ────────────────────────────────────────────────────────────── */
    if(game.state == STATE_PAUSE) {
        if(down & KEY_START) game.state = STATE_GAMEPLAY;
        return;
    }

    /* ── GAME OVER ────────────────────────────────────────────────────────── */
    if(game.state == STATE_GAME_OVER) {
        if(down & KEY_UP)   { if(game.game_over_selection > 0) game.game_over_selection--; }
        if(down & KEY_DOWN) { if(game.game_over_selection < 1) game.game_over_selection++; }
        if(down & (KEY_START | KEY_A)) {
            if(game.game_over_selection == 0) init_game();   /* restart */
            else { game.state = STATE_MAIN_MENU; game.menu_selection = 0; }
        }
        return;
    }

    
/* ── SUB-SCREENS  (Pokedex / Highscores / Options / Credits) ──────────── */
if(game.state == STATE_POKEDEX) {
    // Pokédex navigation
    pokedex_ui_handle_input(down, held);
    if(down & KEY_B) game.state = STATE_MAIN_MENU;
    return;
}
if(game.state == STATE_HIGHSCORES || game.state == STATE_OPTIONS || game.state == STATE_CREDITS) {
    if(down & KEY_B) game.state = STATE_MAIN_MENU;
    return;
}

/* ── GAMEPLAY ─────────────────────────────────────────────────────────── */
    if(game.state != STATE_GAMEPLAY) return;

    /* Dismiss catch celebration on A or Start (before normal controls) */
    if(game.catch_celebration_active) {
        if(down & (KEY_A | KEY_START)) {
            game.catch_celebration_active = 0;
            spawn_random_pokemon();   /* new target — the caught one is done */
            spawn_next_piece();
        }
        return;   /* eat all other input while celebration is up */
    }

    /* Also eat input during line-clear flash */
    if(game.line_clear_active) return;

    /* Pause toggle */
    if(down & KEY_START) { game.state = STATE_PAUSE; return; }

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

    /* ── Flip (R) ─────────────────────────────────────────────────────────── */
    if(down & KEY_R) { flip_tetromino(&game.current_piece); RESET_LOCK_IF_ACTIVE(); }

    /* ── Hold (L or SELECT) ───────────────────────────────────────────────── */
    if((down & KEY_L) || (down & KEY_SELECT)) {
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
    }

    /* ── Hard drop (UP press) ─────────────────────────────────────────────── */
    if(down & KEY_UP) {
        /* drop until collision */
        while(1) {
            game.current_piece.y++;
            if(check_collision(&game.current_piece, &game.board)) {
                game.current_piece.y--;
                break;
            }
        }
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
            spawn_random_pokemon();   /* new target — the caught one is done */
            spawn_next_piece();
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
        if(!check_collision(&probe, &game.board))
            game.current_piece.y++;
        /* if grounded, lock-delay (above) will handle locking on subsequent frames */
    }
}
