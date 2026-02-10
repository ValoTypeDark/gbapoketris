#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "main.h"

// Rendering functions
void render_game(void);
void show_splash_screen(void);
void show_title_screen(void);
void show_mode_select(void);
void show_game_over(void);
void show_highscores(void);
void show_options(void);
void show_credits(void);

// Drawing functions
void draw_block(int x, int y, u16 color);
void draw_tetromino(Tetromino* tetro, int offset_x, int offset_y);
void render_board(Board* board);
void show_catch_celebration(void);

#endif // GRAPHICS_H
