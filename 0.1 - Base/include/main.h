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

// Screen dimensions
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 160

// Game constants
#define BOARD_WIDTH   10
#define BOARD_HEIGHT  20
#define BLOCK_SIZE    6   // Each tetromino block is 6x6 pixels on GBA

// Board position on screen
#define BOARD_X       80
#define BOARD_Y       10

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
    STATE_MENU = 0,
    STATE_MODE_SELECT,
    STATE_GAMEPLAY,
    STATE_PAUSE,
    STATE_GAME_OVER,
    STATE_POKEDEX,
    STATE_OPTIONS
} GameState;

// Game modes
typedef enum {
    MODE_CLASSIC = 0,
    MODE_PENTOMINO,
    MODE_UNOWN,
    MODE_VIVILLON,
    MODE_ALCREMIE,
    MODE_COUNT
} GameMode;

// Tetromino structure
typedef struct {
    TetrominoType type;
    int x, y;           // Position on board
    int rotation;       // 0-3
    u16 color;          // Pokemon-themed color
    u8 shape[4][4];     // 4x4 grid for tetromino shape
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
void show_main_menu(void);
void show_mode_select(void);
void show_pause_menu(void);
void show_game_over(void);

// Pokemon functions
void init_pokemon_data(void);
void unlock_pokemon(u16 dex_number);
u16 get_random_pokemon(void);

// Utility functions
u16 get_tetromino_color(TetrominoType type);
void vsync(void);

#endif // MAIN_H
