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
// At 60 FPS: 167ms = ~10 frames, 33ms = ~2 frames
#define DAS_DELAY  10    // Frames before auto-repeat starts (167ms @ 60fps)
#define ARR        1     // Frames between repeated moves (16ms @ 60fps) - Very fast sliding!

// Lock delay system constants
// At 60 FPS: 450ms = ~27 frames
#define LOCK_DELAY_DURATION  27   // Frames before piece locks (450ms @ 60fps)
#define MAX_LOCK_RESETS      15   // Maximum resets before forcing lock

// Tetromino types
typedef enum {
    TETRO_I = 0,
    TETRO_O,
    TETRO_T,
    TETRO_S,
    TETRO_Z,
    TETRO_J,
    TETRO_L,
    TETRO_COUNT
} TetrominoType;

// Game states
typedef enum {
    STATE_TITLE = 0,        // Title screen "Press Start"
    STATE_MAIN_MENU,        // Main menu with options
    STATE_MODE_SELECT,      // Game mode selection
    STATE_GAMEPLAY,         // Active gameplay
    STATE_PAUSE,            // Paused game
    STATE_GAME_OVER,        // Game over screen
    STATE_POKEDEX,          // Pokedex viewer
    STATE_HIGHSCORES,       // Highscore list
    STATE_OPTIONS,          // Options menu
    STATE_CREDITS           // Credits screen
} GameState;

// Game modes
typedef enum {
    MODE_ROOKIE = 0,      // Easy mode
    MODE_NORMAL,          // Standard
    MODE_SUPER,           // Fast
    MODE_HYPER,           // Very fast
    MODE_MASTER,          // Expert
    MODE_BONUS,           // Bonus modes menu
    MODE_BACK,            // Back to main menu
    MODE_COUNT
} GameMode;

// Piece categories
typedef enum {
    PIECE_TETROMINO,    // 4-block piece
    PIECE_PENTOMINO     // 5-block piece
} PieceCategory;

// Tetromino structure (now stores coordinates like Python, not grid)
typedef struct {
    TetrominoType type; // Type within category (0-6 for tetro, 0-17 for pento)
    PieceCategory category; // Tetromino or pentomino
    int x, y;           // Position on board (center point)
    int rotation;       // 0-3
    u16 color;          // Pokemon-themed color
    u8 block_count;     // 4 or 5
    s8 blocks[5][2];    // Coordinate list: [block_index][x, y] (like Python)
    u8 is_flipped;      // Is this piece currently flipped?
} Tetromino;

// Board structure
typedef struct {
    u16 grid[BOARD_HEIGHT][BOARD_WIDTH];  // Colors of placed blocks
    u8 filled[BOARD_HEIGHT][BOARD_WIDTH]; // 0 = empty, 1 = filled
} Board;

// Pokemon data structure (simplified for GBA)
typedef struct {
    u16 dex_number;
    u16 sprite_id;      // Index into sprite data
    u8 unlocked;
    char name[12];      // Shortened for GBA memory
} Pokemon;

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
    u16 level;
    u32 fall_timer;
    u32 fall_speed;     // Frames per drop
    u16 current_pokemon; // Current Pokemon index
    Pokemon pokemon_list[50]; // Limited Pokemon for GBA
    u8 pokemon_count;
    u8 menu_selection;  // Current menu selection (0-4)
    
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
} GameData;

// Function declarations
void init_game(void);
void update_game(void);
void render_game(void);
void handle_input(void);

// Tetromino functions
void init_tetromino(Tetromino* tetro, TetrominoType type);
void rotate_tetromino(Tetromino* tetro, int direction);
void flip_tetromino(Tetromino* tetro);
int check_collision(Tetromino* tetro, Board* board);
void place_tetromino(Tetromino* tetro, Board* board);
void spawn_next_piece(void);

// Board functions
void init_board(Board* board);
void clear_lines(Board* board);
void render_board(Board* board);

// Drawing functions
void draw_block(int x, int y, u16 color);
void draw_tetromino(Tetromino* tetro, int offset_x, int offset_y);
void draw_text(int x, int y, const char* text, u16 color);
void draw_text_menu(int x, int y, const char* text, u16 color);
void draw_text_large(int x, int y, const char* text, u16 color);
void draw_text_large_outlined(int x, int y, const char* text, u16 color, u16 outline_color);
void draw_number(int x, int y, u32 number, u16 color);

// Font control (kept for compatibility)
void set_font_size(int size);
int get_font_size(void);

// Menu functions
void show_title_screen(void);
void show_main_menu(void);
void show_mode_select(void);
void show_pause_menu(void);
void show_game_over(void);
void show_pokedex(void);
void show_highscores(void);
void show_options(void);
void show_credits(void);

// Pokemon functions
void init_pokemon_data(void);
void unlock_pokemon(u16 dex_number);
u16 get_random_pokemon(void);

// Utility functions
u16 get_tetromino_color(TetrominoType type);
void vsync(void);

#endif // MAIN_H
