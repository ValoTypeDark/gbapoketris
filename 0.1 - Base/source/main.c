#include "main.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Global game data
GameData game;
u16* video_buffer = (u16*)0x06000000; // VRAM

// Tetromino shape definitions (I, O, T, S, Z, J, L)
const u8 tetromino_shapes[TETRO_COUNT][4][4][4] = {
    // I piece
    {
        {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},
        {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}},
        {{0,0,0,0},{0,0,0,0},{1,1,1,1},{0,0,0,0}},
        {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}}
    },
    // O piece
    {
        {{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}},
        {{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}},
        {{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}},
        {{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}}
    },
    // T piece
    {
        {{0,0,0,0},{0,1,0,0},{1,1,1,0},{0,0,0,0}},
        {{0,0,0,0},{0,1,0,0},{0,1,1,0},{0,1,0,0}},
        {{0,0,0,0},{0,0,0,0},{1,1,1,0},{0,1,0,0}},
        {{0,0,0,0},{0,1,0,0},{1,1,0,0},{0,1,0,0}}
    },
    // S piece
    {
        {{0,0,0,0},{0,1,1,0},{1,1,0,0},{0,0,0,0}},
        {{0,0,0,0},{0,1,0,0},{0,1,1,0},{0,0,1,0}},
        {{0,0,0,0},{0,0,0,0},{0,1,1,0},{1,1,0,0}},
        {{0,0,0,0},{1,0,0,0},{1,1,0,0},{0,1,0,0}}
    },
    // Z piece
    {
        {{0,0,0,0},{1,1,0,0},{0,1,1,0},{0,0,0,0}},
        {{0,0,0,0},{0,0,1,0},{0,1,1,0},{0,1,0,0}},
        {{0,0,0,0},{0,0,0,0},{1,1,0,0},{0,1,1,0}},
        {{0,0,0,0},{0,1,0,0},{1,1,0,0},{1,0,0,0}}
    },
    // J piece
    {
        {{0,0,0,0},{1,0,0,0},{1,1,1,0},{0,0,0,0}},
        {{0,0,0,0},{0,1,1,0},{0,1,0,0},{0,1,0,0}},
        {{0,0,0,0},{0,0,0,0},{1,1,1,0},{0,0,1,0}},
        {{0,0,0,0},{0,1,0,0},{0,1,0,0},{1,1,0,0}}
    },
    // L piece
    {
        {{0,0,0,0},{0,0,1,0},{1,1,1,0},{0,0,0,0}},
        {{0,0,0,0},{0,1,0,0},{0,1,0,0},{0,1,1,0}},
        {{0,0,0,0},{0,0,0,0},{1,1,1,0},{1,0,0,0}},
        {{0,0,0,0},{1,1,0,0},{0,1,0,0},{0,1,0,0}}
    }
};

// Tetromino colors (Pokemon-themed)
const u16 tetromino_colors[TETRO_COUNT] = {
    RGB15(0,25,31),   // I - Cyan (Water type)
    RGB15(31,31,0),   // O - Yellow (Electric type)
    RGB15(25,0,31),   // T - Purple (Psychic type)
    RGB15(0,31,0),    // S - Green (Grass type)
    RGB15(31,0,0),    // Z - Red (Fire type)
    RGB15(0,0,31),    // J - Blue (Water type)
    RGB15(31,15,0)    // L - Orange (Fighting type)
};

int main(void) {
    // Set video mode to Mode 3 (240x160, 16-bit color bitmap)
    SetMode(MODE_3 | BG2_ON);
    
    // Initialize interrupts
    irqInit();
    irqEnable(IRQ_VBLANK);
    
    // Initialize game
    init_game();
    
    // Main game loop
    while(1) {
        VBlankIntrWait();
        
        handle_input();
        update_game();
        render_game();
    }
    
    return 0;
}

void init_game(void) {
    // Clear game data
    memset(&game, 0, sizeof(GameData));
    
    // Set initial state
    game.state = STATE_MENU;
    game.mode = MODE_CLASSIC;
    game.level = 1;
    game.fall_speed = 48; // Start slow (48 frames = ~0.8 seconds at 60fps)
    
    // Initialize board
    init_board(&game.board);
    
    // Initialize hold piece as empty
    game.hold_piece.type = TETRO_COUNT; // Use invalid type to indicate empty
    game.hold_used = 0;
    
    // Initialize next piece as empty (will be set on first spawn)
    game.next_piece.type = TETRO_COUNT;
    
    // Initialize Pokemon data
    init_pokemon_data();
    
    // Show main menu
    show_main_menu();
}

void init_board(Board* board) {
    int i, j;
    for(i = 0; i < BOARD_HEIGHT; i++) {
        for(j = 0; j < BOARD_WIDTH; j++) {
            board->filled[i][j] = 0;
            board->grid[i][j] = BLACK;
        }
    }
}

void init_tetromino(Tetromino* tetro, TetrominoType type) {
    int i, j;
    tetro->type = type;
    tetro->x = BOARD_WIDTH / 2 - 2;
    tetro->y = 0;
    tetro->rotation = 0;
    tetro->color = tetromino_colors[type];
    
    // Copy shape data
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            tetro->shape[i][j] = tetromino_shapes[type][0][i][j];
        }
    }
}

// SRS Wall Kick Data
// Kick offsets for standard pieces (J, L, S, Z, T)
static const int standard_kicks[8][5][2] = {
    // 0->1 (0 to R)
    {{0,0}, {-1,0}, {-1,1}, {0,-2}, {-1,-2}},
    // 1->2 (R to 2)
    {{0,0}, {1,0}, {1,-1}, {0,2}, {1,2}},
    // 2->3 (2 to L)
    {{0,0}, {1,0}, {1,1}, {0,-2}, {1,-2}},
    // 3->0 (L to 0)
    {{0,0}, {-1,0}, {-1,-1}, {0,2}, {-1,2}},
    // 1->0 (R to 0)
    {{0,0}, {1,0}, {1,1}, {0,-2}, {1,-2}},
    // 2->1 (2 to R)
    {{0,0}, {-1,0}, {-1,-1}, {0,2}, {-1,2}},
    // 3->2 (L to 2)
    {{0,0}, {-1,0}, {-1,1}, {0,-2}, {-1,-2}},
    // 0->3 (0 to L)
    {{0,0}, {1,0}, {1,-1}, {0,2}, {1,2}}
};

// I-piece special kicks
static const int i_kicks[8][5][2] = {
    // 0->1
    {{0,0}, {-2,0}, {1,0}, {-2,-1}, {1,2}},
    // 1->2
    {{0,0}, {-1,0}, {2,0}, {-1,2}, {2,-1}},
    // 2->3
    {{0,0}, {2,0}, {-1,0}, {2,1}, {-1,-2}},
    // 3->0
    {{0,0}, {1,0}, {-2,0}, {1,-2}, {-2,1}},
    // 1->0
    {{0,0}, {2,0}, {-1,0}, {2,1}, {-1,-2}},
    // 2->1
    {{0,0}, {1,0}, {-2,0}, {1,-2}, {-2,1}},
    // 3->2
    {{0,0}, {-2,0}, {1,0}, {-2,-1}, {1,2}},
    // 0->3
    {{0,0}, {-1,0}, {2,0}, {-1,2}, {2,-1}}
};

void rotate_tetromino(Tetromino* tetro, int direction) {
    int i, j, k;
    int old_rotation = tetro->rotation;
    int new_rotation = old_rotation + direction;
    
    // Wrap rotation
    if(new_rotation < 0) new_rotation = 3;
    if(new_rotation > 3) new_rotation = 0;
    
    // Create temporary tetromino for testing
    Tetromino temp = *tetro;
    temp.rotation = new_rotation;
    
    // Copy new shape
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            temp.shape[i][j] = tetromino_shapes[tetro->type][new_rotation][i][j];
        }
    }
    
    // Determine kick table index
    int kick_index;
    if(direction == 1) {
        // Clockwise
        kick_index = old_rotation;  // 0->1, 1->2, 2->3, 3->0
    } else {
        // Counter-clockwise
        kick_index = 4 + new_rotation;  // 1->0, 2->1, 3->2, 0->3
    }
    
    // Choose appropriate kick table
    const int (*kicks)[2];
    int num_kicks = 5;
    
    if(tetro->type == TETRO_I) {
        kicks = (const int (*)[2])i_kicks[kick_index];
    } else if(tetro->type == TETRO_O) {
        // O-piece doesn't need kicks, just accept rotation
        *tetro = temp;
        return;
    } else {
        kicks = (const int (*)[2])standard_kicks[kick_index];
    }
    
    // Try each kick offset
    for(k = 0; k < num_kicks; k++) {
        temp.x = tetro->x + kicks[k][0];
        temp.y = tetro->y + kicks[k][1];
        
        if(!check_collision(&temp, &game.board)) {
            // This kick worked!
            *tetro = temp;
            return;
        }
    }
    
    // No kicks worked, rotation fails (piece stays in original state)
}

void flip_tetromino(Tetromino* tetro) {
    // Don't flip symmetrical pieces (O and I)
    if(tetro->type == TETRO_O || tetro->type == TETRO_I) {
        return;
    }
    
    int i, j;
    
    // Find the bounding box of the current shape
    int min_x = 4, max_x = -1;
    int has_blocks = 0;
    
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            if(tetro->shape[i][j]) {
                has_blocks = 1;
                if(j < min_x) min_x = j;
                if(j > max_x) max_x = j;
            }
        }
    }
    
    if(!has_blocks) return; // No blocks to flip
    
    // Create flipped shape by flipping horizontally within the bounding box
    u8 flipped_shape[4][4];
    
    // Clear the new shape
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            flipped_shape[i][j] = 0;
        }
    }
    
    // Flip each block within the bounding box
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            if(tetro->shape[i][j]) {
                // Flip horizontally: new_x = max_x - (x - min_x)
                int new_j = max_x - (j - min_x);
                flipped_shape[i][new_j] = 1;
            }
        }
    }
    
    // Create temporary tetromino with flipped shape
    Tetromino temp = *tetro;
    
    // Copy flipped shape
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            temp.shape[i][j] = flipped_shape[i][j];
        }
    }
    
    // Check if flip is valid at current position
    if(!check_collision(&temp, &game.board)) {
        // Flip succeeded without kicks
        *tetro = temp;
        return;
    }
    
    // Try kick offsets to make flip valid
    int kick_offsets[9][2] = {
        {0, 0},   // No shift (already tried above but include for consistency)
        {-1, 0},  // Left
        {1, 0},   // Right
        {-2, 0},  // Left 2
        {2, 0},   // Right 2
        {0, -1},  // Up
        {0, 1},   // Down
        {-1, -1}, // Diagonal up-left
        {1, -1}   // Diagonal up-right
    };
    
    int k;
    for(k = 0; k < 9; k++) {
        temp.x = tetro->x + kick_offsets[k][0];
        temp.y = tetro->y + kick_offsets[k][1];
        
        if(!check_collision(&temp, &game.board)) {
            // Flip succeeded with this kick!
            *tetro = temp;
            return;
        }
    }
    
    // No kicks worked, flip fails (piece stays as-is)
}

int check_collision(Tetromino* tetro, Board* board) {
    int i, j;
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            if(tetro->shape[i][j]) {
                int board_x = tetro->x + j;
                int board_y = tetro->y + i;
                
                // Check boundaries
                if(board_x < 0 || board_x >= BOARD_WIDTH ||
                   board_y >= BOARD_HEIGHT) {
                    return 1; // Collision
                }
                
                // Check if position is filled (but not if above board)
                if(board_y >= 0 && board->filled[board_y][board_x]) {
                    return 1; // Collision
                }
            }
        }
    }
    return 0; // No collision
}

void place_tetromino(Tetromino* tetro, Board* board) {
    int i, j;
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            if(tetro->shape[i][j]) {
                int board_x = tetro->x + j;
                int board_y = tetro->y + i;
                
                if(board_y >= 0 && board_y < BOARD_HEIGHT &&
                   board_x >= 0 && board_x < BOARD_WIDTH) {
                    board->filled[board_y][board_x] = 1;
                    board->grid[board_y][board_x] = tetro->color;
                }
            }
        }
    }
}

void spawn_next_piece(void) {
    // If next piece exists, use it
    if(game.next_piece.type != TETRO_COUNT) {
        game.current_piece = game.next_piece;
        game.current_piece.x = BOARD_WIDTH / 2 - 2;
        game.current_piece.y = 0;
    } else {
        // First piece, spawn random
        TetrominoType type = rand() % TETRO_COUNT;
        init_tetromino(&game.current_piece, type);
    }
    
    // Generate new next piece
    TetrominoType next_type = rand() % TETRO_COUNT;
    init_tetromino(&game.next_piece, next_type);
    
    // Check for game over
    if(check_collision(&game.current_piece, &game.board)) {
        game.state = STATE_GAME_OVER;
    }
}

void handle_input(void) {
    scanKeys();
    u16 keys_down = keysDown();
    u16 keys_held = keysHeld();
    
    if(game.state == STATE_GAMEPLAY) {
        // Move left
        if(keys_down & KEY_LEFT) {
            Tetromino temp = game.current_piece;
            temp.x--;
            if(!check_collision(&temp, &game.board)) {
                game.current_piece = temp;
            }
        }
        
        // Move right
        if(keys_down & KEY_RIGHT) {
            Tetromino temp = game.current_piece;
            temp.x++;
            if(!check_collision(&temp, &game.board)) {
                game.current_piece = temp;
            }
        }
        
        // Rotate clockwise
        if(keys_down & KEY_A) {
            rotate_tetromino(&game.current_piece, 1);
        }
        
        // Rotate counter-clockwise  
        if(keys_down & KEY_B) {
            rotate_tetromino(&game.current_piece, -1);
        }
        
        // Flip piece (R button)
        if(keys_down & KEY_R) {
            flip_tetromino(&game.current_piece);
        }
        
        // Hold piece (L button or SELECT)
        if((keys_down & KEY_L) || (keys_down & KEY_SELECT)) {
            if(!game.hold_used) {
                if(game.hold_piece.type == TETRO_COUNT) {
                    // No piece held yet, just store current and spawn new
                    game.hold_piece = game.current_piece;
                    game.hold_piece.x = BOARD_WIDTH / 2 - 2;
                    game.hold_piece.y = 0;
                    spawn_next_piece();
                } else {
                    // Swap current and held piece
                    Tetromino temp = game.current_piece;
                    game.current_piece = game.hold_piece;
                    game.hold_piece = temp;
                    
                    // Reset position of swapped-in piece
                    game.current_piece.x = BOARD_WIDTH / 2 - 2;
                    game.current_piece.y = 0;
                    
                    // Store position of held piece
                    game.hold_piece.x = 0;
                    game.hold_piece.y = 0;
                }
                game.hold_used = 1; // Can't hold again until piece is placed
            }
        }
        
        // Soft drop
        if(keys_held & KEY_DOWN) {
            game.fall_timer = game.fall_speed; // Force drop
        }
        
        // Hard drop
        if(keys_down & KEY_UP) {
            while(!check_collision(&game.current_piece, &game.board)) {
                game.current_piece.y++;
            }
            game.current_piece.y--;
            place_tetromino(&game.current_piece, &game.board);
            clear_lines(&game.board);
            game.hold_used = 0; // Can hold again after placing
            spawn_next_piece();
        }
        
        // Pause
        if(keys_down & KEY_START) {
            game.state = STATE_PAUSE;
        }
    }
    else if(game.state == STATE_MENU) {
        if(keys_down & KEY_START) {
            game.state = STATE_GAMEPLAY;
            spawn_next_piece();
        }
    }
    else if(game.state == STATE_PAUSE) {
        if(keys_down & KEY_START) {
            game.state = STATE_GAMEPLAY;
        }
    }
}

void update_game(void) {
    if(game.state != STATE_GAMEPLAY) {
        return;
    }
    
    // Update fall timer
    game.fall_timer++;
    
    if(game.fall_timer >= game.fall_speed) {
        game.fall_timer = 0;
        
        // Try to move piece down
        Tetromino temp = game.current_piece;
        temp.y++;
        
        if(!check_collision(&temp, &game.board)) {
            game.current_piece = temp;
        }
        else {
            // Piece has landed
            place_tetromino(&game.current_piece, &game.board);
            clear_lines(&game.board);
            game.hold_used = 0; // Can hold again after placing
            spawn_next_piece();
        }
    }
}

void clear_lines(Board* board) {
    int i, j, k;
    int lines_cleared = 0;
    
    for(i = BOARD_HEIGHT - 1; i >= 0; i--) {
        int full = 1;
        for(j = 0; j < BOARD_WIDTH; j++) {
            if(!board->filled[i][j]) {
                full = 0;
                break;
            }
        }
        
        if(full) {
            lines_cleared++;
            
            // Move all lines above down
            for(k = i; k > 0; k--) {
                for(j = 0; j < BOARD_WIDTH; j++) {
                    board->filled[k][j] = board->filled[k-1][j];
                    board->grid[k][j] = board->grid[k-1][j];
                }
            }
            
            // Clear top line
            for(j = 0; j < BOARD_WIDTH; j++) {
                board->filled[0][j] = 0;
                board->grid[0][j] = BLACK;
            }
            
            i++; // Check this line again
        }
    }
    
    if(lines_cleared > 0) {
        game.lines_cleared += lines_cleared;
        game.score += lines_cleared * lines_cleared * 100 * game.level;
        
        // Level up every 10 lines
        if(game.lines_cleared / 10 > game.level - 1) {
            game.level++;
            if(game.fall_speed > 10) {
                game.fall_speed -= 3; // Speed up
            }
        }
    }
}

u16 get_tetromino_color(TetrominoType type) {
    return tetromino_colors[type];
}

void vsync(void) {
    VBlankIntrWait();
}

void init_pokemon_data(void) {
    // Initialize with limited Pokemon for GBA memory constraints
    game.pokemon_count = 10; // Start with just 10 Pokemon
    
    // Example Pokemon (simplified)
    game.pokemon_list[0].dex_number = 1;
    strcpy(game.pokemon_list[0].name, "Bulbasaur");
    game.pokemon_list[0].unlocked = 0;
    
    // Add more Pokemon as needed...
}

void unlock_pokemon(u16 dex_number) {
    int i;
    for(i = 0; i < game.pokemon_count; i++) {
        if(game.pokemon_list[i].dex_number == dex_number) {
            game.pokemon_list[i].unlocked = 1;
            break;
        }
    }
}

u16 get_random_pokemon(void) {
    return rand() % game.pokemon_count;
}
