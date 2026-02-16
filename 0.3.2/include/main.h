#ifndef MAIN_H
#define MAIN_H

#include <gba_base.h>
#include <gba_video.h>
#include <gba_systemcalls.h>
#include <gba_interrupt.h>
#include <gba_input.h>
#include <gba_sprites.h>
#include <gba_compression.h>
#include <gba_sound.h>

#include "pentominos.h"
#include "unown_shapes.h"

// Screen dimensions
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 160

// Game constants
#define BOARD_WIDTH   10
#define BOARD_HEIGHT  20
#define BLOCK_SIZE    8   // Each tetromino block is 8x8 pixels (increased from 6)

// Board position on screen
// Board is now 80px wide (10×8) and 160px tall (20×8) - fills height!
#define BOARD_X       50
#define BOARD_Y       0

// Preview position
#define PREVIEW_X     180
#define PREVIEW_Y     20

// Colors (GBA uses 15-bit RGB)
#define RGB15(r,g,b)  ((r)|((g)<<5)|((b)<<10))
#define BLACK         RGB15(0,0,0)
#define WHITE         RGB15(31,31,31)
#define GRAY          RGB15(15,15,15)
#define RED           RGB15(31,0,0)
#define GREEN         RGB15(0,31,0)
#define BLUE          RGB15(0,0,31)
#define YELLOW        RGB15(31,31,0)
#define CYAN          RGB15(0,31,31)
#define MAGENTA       RGB15(31,0,31)
#define ORANGE        RGB15(31,15,0)
#define PURPLE        RGB15(15,0,31)

// Pokemon brand colors (converted from hex to RGB15)
// ffc829 -> RGB(255,200,41) -> RGB15(31,25,5)
#define POKEMON_YELLOW RGB15(31,25,5)
// 025597 -> RGB(2,85,151) -> RGB15(0,10,18)
#define POKEMON_BLUE   RGB15(0,10,18)

// DAS (Delayed Auto Shift) timing constants
// OPTION 2: Modern Tetris - Very Responsive
// At 60 FPS: 50ms = ~3 frames, 0ms = instant repeat
#define DAS_DELAY  3     // Frames before auto-repeat starts (~50ms @ 60fps)
#define ARR        0     // Instant sliding when auto-repeat activates

// Lock delay system (prevents instant piece locking)
#define LOCK_DELAY_DURATION  18   // Frames before piece locks (~300ms @ 60fps) [modern-feel]
#define MAX_LOCK_RESETS       8   // Max move/rotate resets while grounded (caps stall time)

// Line clear animation timing
#define LINE_CLEAR_FRAMES 20  // Duration of line clear flash animation

// Game states
typedef enum {
    STATE_SPLASH,      // <-- ADD THIS FIRST
    STATE_TITLE,
    STATE_MAIN_MENU,
    STATE_MODE_SELECT,
    STATE_BONUS_SELECT,  // Bonus mode submenu
    STATE_GAMEPLAY,
    STATE_PAUSE,
    STATE_GAME_OVER,
    STATE_POKEDEX,
    STATE_HIGHSCORES,
    STATE_OPTIONS,
    STATE_CREDITS
} GameState;

// Game modes
typedef enum {
    MODE_ROOKIE = 0,    // 0: Beginner mode
    MODE_NORMAL = 1,    // 1: Standard mode
    MODE_SUPER = 2,     // 2: Advanced mode
    MODE_HYPER = 3,     // 3: Expert mode
    MODE_MASTER = 4,    // 4: Master mode
    MODE_UNOWN = 5,     // 5: Unown bonus mode
    MODE_VIVILLON = 6,  // 6: Vivillon bonus mode
    MODE_ALCREMIE = 7,  // 7: Alcremie bonus mode
    MODE_BONUS = 8,     // 8: Generic bonus placeholder
    MODE_BACK = 9,      // 9: Back button in mode select
    MODE_COUNT          // Total number of modes
} GameMode;

// Tetromino types
typedef enum {
    TETRO_I = 0,
    TETRO_O = 1,
    TETRO_T = 2,
    TETRO_S = 3,
    TETRO_Z = 4,
    TETRO_J = 5,
    TETRO_L = 6,
    TETRO_COUNT = 7
} TetrominoType;

// Pentomino types (12 unique pentominos)
typedef enum {
    PENTO_F = 0,
    PENTO_I = 1,
    PENTO_L = 2,
    PENTO_N = 3,
    PENTO_P = 4,
    PENTO_T = 5,
    PENTO_U = 6,
    PENTO_V = 7,
    PENTO_W = 8,
    PENTO_X = 9,
    PENTO_Y = 10,
    PENTO_Z = 11,
    PENTO_COUNT = 12
} PentominoType;

// Piece category (tetromino vs pentomino vs unown)
typedef enum {
    PIECE_TETROMINO = 0,
    PIECE_PENTOMINO = 1,
    PIECE_UNOWN = 2
} PieceCategory;

// Tetromino structure
typedef struct {
    PieceCategory category;  // Tetromino, Pentomino, or Unown?
    union {
        TetrominoType type;     // For tetrominos
        PentominoType pento;    // For pentominos
        UnownType unown;        // For Unown pieces
    };
    int x, y;
    u8 rotation;  // 0-3 for tetrominos/unown, 0-7 for pentominos (includes flips)
    u16 color;          // Pokemon-themed color
    u8 block_count;     // 3-7 (Unown can have 3-7 blocks)
    s8 blocks[7][2];    // Coordinate list: [block_index][x, y] - increased to 7 for Unown
    u8 is_flipped;      // Is this piece currently flipped?
} Tetromino;

// Board structure
typedef struct {
    u16 grid[BOARD_HEIGHT][BOARD_WIDTH];  // Colors of placed blocks
    u8 filled[BOARD_HEIGHT][BOARD_WIDTH]; // 0 = empty, 1 = filled
} Board;

// Pokemon catch tracking (data comes from pokemon_database.c)
typedef struct {
    u16 dex_number;
    u8 unlocked;           // Has this Pokemon been caught (normal)?
    u8 unlocked_shiny;     // Has shiny form been caught?
    u16 catch_count;       // How many times caught (normal)
    u16 catch_count_shiny; // How many times caught (shiny)
} PokemonCatch;

// Highscore entry
typedef struct {
    u32 score;
    u16 lines;
    u16 level;
    u8 pokemon_caught;
} HighscoreEntry;

// Game state structure
typedef struct {
    GameState state;
    GameMode mode;
    Board board;
    Tetromino current_piece;
    Tetromino next_piece;
    Tetromino hold_piece;
    u8 hold_used;
    u32 score;
    u16 lines_cleared;
    u16 splash_timer;         // Timer for splash screen (frames)
    u8  splash_save_done;     // Has initial save been created?
    u16 level;
    u32 fall_timer;
    u32 fall_speed;     // Frames per drop
    
    // Scoring system
    u8 b2b_active;            // Back-to-back chain active (4+ line clears)
    u16 b2b_count;            // Number of consecutive B2B clears
    u32 drop_bonus;           // Accumulated soft/hard drop points for current piece
    
    u16 current_pokemon; // Current Pokemon index (into POKEMON_DATABASE)
    u16 pieces_left;     // Pieces left before Pokemon disappears
    u16 pokemon_duration; // Total pieces this Pokemon stays for
    u8 is_shiny;         // Is current Pokemon shiny?
    u8 big_clear_streak; // Consecutive 4/5 line clears (resets on 1/2/3 line clear)
    PokemonCatch pokemon_catches[1349]; // Track catches (matches TOTAL_POKEMON)
    u8 menu_selection;  // Current menu selection (0-4)
    u8 game_over_selection; // Game over menu (0=Restart, 1=Menu)
    u8 pokemon_caught_this_game; // Pokemon caught in current session (normal)
    u8 shinies_caught_this_game; // Shiny Pokemon caught in current session
    u8 new_dex_entries; // New Pokedex entries this game
    
    // Highscores - 5 per mode (7 modes total including bonus modes)
    HighscoreEntry highscores[7][5];  // [mode][rank 0-4]
    
    // DAS (Delayed Auto Shift) system for smooth movement
    u16 left_das_timer;   // Timer for left DAS delay
    u16 right_das_timer;  // Timer for right DAS delay
    u8 left_das_active;   // Is left auto-repeat active?
    u8 right_das_active;  // Is right auto-repeat active?
    u16 left_arr_timer;   // Timer for left auto-repeat rate
    u16 right_arr_timer;  // Timer for right auto-repeat rate
    
    // Lock delay system (prevents instant locking, allows piece adjustment)
    u16 lock_delay_timer;  // Timer for lock delay (in frames)
    u8 lock_delay_active;  // Is lock delay active?
    u8 lock_resets_used;   // Number of times lock delay has been reset
    u8 is_grounded;        // Is piece touching ground?
    
    // Line clear animation
    u8 line_clear_active;  // Is line clear animation playing?
    u16 line_clear_timer;  // Animation timer (in frames)
    u8 cleared_lines[5];   // Which lines are being cleared (row indices) - up to 5 for pentominos!
    u8 cleared_line_count; // How many lines are being cleared
    
    // Catch celebration
    u8 catch_celebration_active; // Is catch celebration showing?
    u16 catch_celebration_timer; // Animation timer (in frames)
    u16 caught_pokemon_id;       // Which Pokemon was caught
    u8 is_new_catch;             // Is this a new Pokedex entry?
    
    // Screen shake effects
    u8 screen_shake_active;  // Is screen shake active?
    u16 screen_shake_timer;  // Shake duration timer
    s8 shake_offset_x;       // Current X offset
    s8 shake_offset_y;       // Current Y offset
    u8 shake_intensity;      // Shake strength
    
    // Mode unlock system
    u8 mode_unlocked[8];     // Which modes are unlocked (ROOKIE, NORMAL, SUPER, HYPER, MASTER, UNOWN, VIVILLON, ALCREMIE)
    u8 mode_unlock_celebration_pending; // Mode unlock message to show (0 = none, 1-8 = which mode)
} GameData;

// Global game data
extern GameData game;

// Function declarations
void init_game(void);
void init_board(Board* board);
void handle_input(void);
void update_game(void);
void render_game(void);

// Tetromino functions
void init_tetromino(Tetromino* tetro, TetrominoType type);
void spawn_tetromino(Tetromino* tetro, TetrominoType type);
void rotate_tetromino(Tetromino* tetro, int direction);
void flip_tetromino(Tetromino* tetro);
int check_collision(Tetromino* tetro, Board* board);
void place_tetromino(Tetromino* tetro, Board* board);
void clear_lines(Board* board);

// Pokemon functions
void spawn_random_pokemon(void);
void spawn_next_piece(void);
void try_catch_pokemon(int lines_cleared);
void update_catch_celebration(void);

// Screen shake
void start_screen_shake(u8 intensity);
void update_screen_shake(void);

// Drawing functions (declared in graphics.c)
void draw_block(int x, int y, u16 color);
void draw_tetromino(Tetromino* tetro, int offset_x, int offset_y);
void render_board(Board* board);
void show_catch_celebration(void);

// Menu functions
void show_title_screen(void);
void show_main_menu(void);
void show_mode_select(void);
void show_bonus_select(void);
void show_game_over(void);
void show_pokedex(void);
void show_highscores(void);
void show_options(void);
void show_credits(void);

// Utility
u16 get_tetromino_color(TetrominoType type);

#endif // MAIN_H