#include "main.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Global game data
GameData game;
u16* video_buffer = (u16*)0x06000000; // VRAM

// Tetromino shape definitions (I, O, T, S, Z, J, L)
// Tetromino coordinate definitions (matching Python game)
// Format: [piece][rotation][block][x, y]
const s8 tetromino_coords[TETRO_COUNT][4][4][2] = {
    // I piece - rotates around center
    {
        {{-1,0}, {0,0}, {1,0}, {2,0}},      // Horizontal
        {{0,-1}, {0,0}, {0,1}, {0,2}},      // Vertical
        {{-1,0}, {0,0}, {1,0}, {2,0}},      // Horizontal
        {{0,-1}, {0,0}, {0,1}, {0,2}},      // Vertical
    },
    // O piece - 2x2 square, doesn't move when rotated
    {
        {{0,0}, {1,0}, {0,1}, {1,1}},
        {{0,0}, {1,0}, {0,1}, {1,1}},
        {{0,0}, {1,0}, {0,1}, {1,1}},
        {{0,0}, {1,0}, {0,1}, {1,1}},
    },
    // T piece
    {
        {{-1,0}, {0,0}, {1,0}, {0,-1}},     // Spawn
        {{0,-1}, {0,0}, {0,1}, {1,0}},      // 90° CW
        {{-1,0}, {0,0}, {1,0}, {0,1}},      // 180°
        {{0,-1}, {0,0}, {0,1}, {-1,0}},     // 270° CW
    },
    // S piece
    {
        {{-1,0}, {0,0}, {0,-1}, {1,-1}},
        {{0,-1}, {0,0}, {1,0}, {1,1}},
        {{-1,1}, {0,1}, {0,0}, {1,0}},
        {{-1,-1}, {-1,0}, {0,0}, {0,1}},
    },
    // Z piece
    {
        {{-1,-1}, {0,-1}, {0,0}, {1,0}},
        {{1,-1}, {1,0}, {0,0}, {0,1}},
        {{-1,0}, {0,0}, {0,1}, {1,1}},
        {{0,-1}, {0,0}, {-1,0}, {-1,1}},
    },
    // J piece
    {
        {{-1,0}, {0,0}, {1,0}, {1,-1}},
        {{0,-1}, {0,0}, {0,1}, {1,1}},
        {{-1,1}, {-1,0}, {0,0}, {1,0}},
        {{-1,-1}, {0,-1}, {0,0}, {0,1}},
    },
    // L piece
    {
        {{-1,0}, {0,0}, {1,0}, {-1,-1}},
        {{0,-1}, {0,0}, {0,1}, {1,-1}},
        {{-1,0}, {0,0}, {1,0}, {1,1}},
        {{0,-1}, {0,0}, {0,1}, {-1,1}},
    },
};

// OLD grid-based definitions (keeping for reference, but unused)
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
    game.state = STATE_TITLE;  // Start at title screen
    game.mode = MODE_ROOKIE;   // Default to Rookie mode
    game.level = 1;
    game.fall_speed = 53;  // Game Boy Tetris starting speed (~0.88s)
    game.menu_selection = 0;  // First menu item
    
    // Initialize board
    init_board(&game.board);
    
    // Initialize hold piece as empty
    game.hold_piece.category = PIECE_TETROMINO;
    game.hold_piece.type = TETRO_COUNT; // Use invalid type to indicate empty
    game.hold_used = 0;
    
    // Initialize next piece as empty (will be set on first spawn)
    game.next_piece.category = PIECE_TETROMINO;
    game.next_piece.type = TETRO_COUNT;
    
    // Initialize Pokemon data
    init_pokemon_data();
    
    // Show title screen
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
    int i;
    tetro->type = type;
    tetro->category = PIECE_TETROMINO;
    tetro->x = BOARD_WIDTH / 2;  // Center spawn
    tetro->y = 0;
    tetro->rotation = 0;
    tetro->color = tetromino_colors[type];
    tetro->block_count = 4;
    tetro->is_flipped = 0;  // Start unflipped
    
    // Copy coordinates for spawn rotation
    for(i = 0; i < 4; i++) {
        tetro->blocks[i][0] = tetromino_coords[type][0][i][0];  // x
        tetro->blocks[i][1] = tetromino_coords[type][0][i][1];  // y
    }
}

void init_pentomino(Tetromino* tetro, int pento_type) {
    int i;
    tetro->type = pento_type;
    tetro->category = PIECE_PENTOMINO;
    tetro->x = BOARD_WIDTH / 2;  // Center spawn
    tetro->y = 0;
    tetro->rotation = 0;
    tetro->color = PENTOMINO_COLORS[pento_type];
    tetro->block_count = 5;
    tetro->is_flipped = 0;  // Start unflipped
    
    // Get pentomino coordinates for spawn rotation
    const s8* shape_data = get_pentomino_shape(pento_type, 0);
    // shape_data points to [5][2] array, access as 2D array
    const s8 (*coords)[2] = (const s8 (*)[2])shape_data;
    for(i = 0; i < 5; i++) {
        tetro->blocks[i][0] = coords[i][0];  // x
        tetro->blocks[i][1] = coords[i][1];  // y
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
    int i, k;
    int old_rotation = tetro->rotation;
    int new_rotation = old_rotation + direction;
    
    // Wrap rotation
    if(new_rotation < 0) new_rotation = 3;
    if(new_rotation > 3) new_rotation = 0;
    
    // Create temporary tetromino for testing
    Tetromino temp = *tetro;
    temp.rotation = new_rotation;
    
    // Load new coordinates based on piece type
    if(tetro->category == PIECE_PENTOMINO) {
        // Load pentomino coordinates
        const s8* shape_data = get_pentomino_shape(tetro->type, new_rotation);
        const s8 (*coords)[2] = (const s8 (*)[2])shape_data;
        for(i = 0; i < 5; i++) {
            temp.blocks[i][0] = coords[i][0];  // x
            temp.blocks[i][1] = coords[i][1];  // y
        }
    } else {
        // Load tetromino coordinates
        for(i = 0; i < 4; i++) {
            temp.blocks[i][0] = tetromino_coords[tetro->type][new_rotation][i][0];
            temp.blocks[i][1] = tetromino_coords[tetro->type][new_rotation][i][1];
        }
    }
    
    // If piece was flipped, apply flip to the new rotation
    if(tetro->is_flipped) {
        // Find bounding box of new rotation
        int min_x = 127, max_x = -128;
        for(i = 0; i < temp.block_count; i++) {
            if(temp.blocks[i][0] < min_x) min_x = temp.blocks[i][0];
            if(temp.blocks[i][0] > max_x) max_x = temp.blocks[i][0];
        }
        
        // Flip all coordinates
        for(i = 0; i < temp.block_count; i++) {
            temp.blocks[i][0] = max_x - (temp.blocks[i][0] - min_x);
        }
    }
    
    // Try wall kicks for tetrominos
    if(tetro->category == PIECE_TETROMINO) {
        // Determine kick table index
        int kick_index;
        if(direction == 1) {
            kick_index = old_rotation;
        } else {
            kick_index = 4 + new_rotation;
        }
        
        // Choose appropriate kick table
        const int (*kicks)[2];
        int num_kicks = 5;
        
        if(tetro->type == TETRO_I) {
            kicks = (const int (*)[2])i_kicks[kick_index];
        } else if(tetro->type == TETRO_O) {
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
                *tetro = temp;
                return;
            }
        }
    } else {
        // Pentomino - simple kicks
        int simple_kicks[5][2] = {{0,0}, {-1,0}, {1,0}, {0,-1}, {0,1}};
        for(k = 0; k < 5; k++) {
            temp.x = tetro->x + simple_kicks[k][0];
            temp.y = tetro->y + simple_kicks[k][1];
            
            if(!check_collision(&temp, &game.board)) {
                *tetro = temp;
                return;
            }
        }
    }
    
    // No kicks worked, rotation fails
}

void flip_tetromino(Tetromino* tetro) {
    // Don't flip symmetrical pieces (O and I for tetros)
    if(tetro->category == PIECE_TETROMINO) {
        if(tetro->type == TETRO_O || tetro->type == TETRO_I) {
            return;
        }
    }
    
    int i, k;
    
    // Find bounding box
    int min_x = 127, max_x = -128;
    for(i = 0; i < tetro->block_count; i++) {
        if(tetro->blocks[i][0] < min_x) min_x = tetro->blocks[i][0];
        if(tetro->blocks[i][0] > max_x) max_x = tetro->blocks[i][0];
    }
    
    // Create flipped coordinates within bounding box
    Tetromino temp = *tetro;
    for(i = 0; i < tetro->block_count; i++) {
        // Flip horizontally: new_x = max_x - (x - min_x)
        temp.blocks[i][0] = max_x - (tetro->blocks[i][0] - min_x);
        temp.blocks[i][1] = tetro->blocks[i][1];  // y stays same
    }
    
    // Toggle flip state
    temp.is_flipped = !tetro->is_flipped;
    
    // Check if flip is valid at current position
    if(!check_collision(&temp, &game.board)) {
        *tetro = temp;
        return;
    }
    
    // Try kick offsets
    int kick_offsets[9][2] = {
        {0, 0}, {-1, 0}, {1, 0}, {-2, 0}, {2, 0},
        {0, -1}, {0, 1}, {-1, -1}, {1, -1}
    };
    
    for(k = 0; k < 9; k++) {
        temp.x = tetro->x + kick_offsets[k][0];
        temp.y = tetro->y + kick_offsets[k][1];
        
        if(!check_collision(&temp, &game.board)) {
            *tetro = temp;
            return;
        }
    }
    // Flip failed - piece stays as-is
}

int check_collision(Tetromino* tetro, Board* board) {
    int i;
    for(i = 0; i < tetro->block_count; i++) {
        int board_x = tetro->x + tetro->blocks[i][0];
        int board_y = tetro->y + tetro->blocks[i][1];
        
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
    return 0; // No collision
}

void place_tetromino(Tetromino* tetro, Board* board) {
    int i;
    for(i = 0; i < tetro->block_count; i++) {
        int board_x = tetro->x + tetro->blocks[i][0];
        int board_y = tetro->y + tetro->blocks[i][1];
        
        if(board_y >= 0 && board_y < BOARD_HEIGHT &&
           board_x >= 0 && board_x < BOARD_WIDTH) {
            board->filled[board_y][board_x] = 1;
            board->grid[board_y][board_x] = tetro->color;
        }
    }
}

// Helper: Spawn a random tetromino
void spawn_random_tetromino(Tetromino* piece) {
    TetrominoType type = rand() % TETRO_COUNT;
    init_tetromino(piece, type);
}

// Helper: Spawn a random pentomino
void spawn_random_pentomino(Tetromino* piece) {
    int pento_type = rand() % PENTOMINO_COUNT;
    init_pentomino(piece, pento_type);
}

// Spawn piece based on mode and level
void spawn_piece_for_mode(Tetromino* piece, GameMode mode, int level) {
    float pento_chance = 0.0f;
    
    // Determine pentomino spawn chance based on mode
    if(mode == MODE_MASTER) {
        // Master mode: 100% pentominos
        pento_chance = 1.0f;
    } else if(mode == MODE_NORMAL) {
        // Normal mode: pentominos from level 8, 5-20% chance
        if(level >= 8) {
            pento_chance = 0.05f + 0.01f * (level - 8);
            if(pento_chance > 0.20f) pento_chance = 0.20f;
        }
    } else if(mode == MODE_SUPER || mode == MODE_HYPER) {
        // Super/Hyper: 20-90% pentominos (increases with level)
        pento_chance = 0.20f + 0.05f * level;
        if(pento_chance > 0.90f) pento_chance = 0.90f;
    }
    // Rookie mode: pento_chance stays 0.0f (tetrominos only)
    
    // Spawn pentomino or tetromino based on chance
    int random_val = rand() % 100;
    if(random_val < (int)(pento_chance * 100)) {
        spawn_random_pentomino(piece);
    } else {
        spawn_random_tetromino(piece);
    }
}

void spawn_next_piece(void) {
    // If next piece exists, use it
    if(game.next_piece.category != PIECE_TETROMINO || game.next_piece.type != TETRO_COUNT) {
        game.current_piece = game.next_piece;
        game.current_piece.x = BOARD_WIDTH / 2;  // Center spawn
        game.current_piece.y = 0;
    } else {
        // First piece, spawn based on mode
        spawn_piece_for_mode(&game.current_piece, game.mode, game.level);
    }
    
    // Generate new next piece based on mode and level
    spawn_piece_for_mode(&game.next_piece, game.mode, game.level);
    
    // Reset DAS when piece spawns
    game.left_das_timer = 0;
    game.right_das_timer = 0;
    game.left_das_active = 0;
    game.right_das_active = 0;
    game.left_arr_timer = 0;
    game.right_arr_timer = 0;
    
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
        // DAS (Delayed Auto Shift) for left/right movement
        // Handle initial press
        if(keys_down & KEY_LEFT) {
            Tetromino temp = game.current_piece;
            temp.x--;
            if(!check_collision(&temp, &game.board)) {
                game.current_piece = temp;
                // Reset lock delay on successful movement
                if(game.lock_delay_active && game.lock_resets_used < MAX_LOCK_RESETS) {
                    game.lock_delay_timer = 0;
                    game.lock_resets_used++;
                }
            }
            // Reset DAS for left
            game.left_das_timer = 0;
            game.left_das_active = 0;
            game.left_arr_timer = 0;
        }
        
        if(keys_down & KEY_RIGHT) {
            Tetromino temp = game.current_piece;
            temp.x++;
            if(!check_collision(&temp, &game.board)) {
                game.current_piece = temp;
                // Reset lock delay on successful movement
                if(game.lock_delay_active && game.lock_resets_used < MAX_LOCK_RESETS) {
                    game.lock_delay_timer = 0;
                    game.lock_resets_used++;
                }
            }
            // Reset DAS for right
            game.right_das_timer = 0;
            game.right_das_active = 0;
            game.right_arr_timer = 0;
        }
        
        // Handle held keys with DAS
        if(keys_held & KEY_LEFT) {
            if(!game.left_das_active) {
                // Still in DAS delay period
                game.left_das_timer++;
                if(game.left_das_timer >= DAS_DELAY) {
                    game.left_das_active = 1;
                    game.left_arr_timer = 0;
                }
            } else {
                // DAS is active, handle auto-repeat
                game.left_arr_timer++;
                if(game.left_arr_timer >= ARR) {
                    game.left_arr_timer = 0;
                    Tetromino temp = game.current_piece;
                    temp.x--;
                    if(!check_collision(&temp, &game.board)) {
                        game.current_piece = temp;
                        // Reset lock delay on successful DAS movement
                        if(game.lock_delay_active && game.lock_resets_used < MAX_LOCK_RESETS) {
                            game.lock_delay_timer = 0;
                            game.lock_resets_used++;
                        }
                    }
                }
            }
        } else {
            // Reset left DAS when key released
            game.left_das_timer = 0;
            game.left_das_active = 0;
            game.left_arr_timer = 0;
        }
        
        if(keys_held & KEY_RIGHT) {
            if(!game.right_das_active) {
                // Still in DAS delay period
                game.right_das_timer++;
                if(game.right_das_timer >= DAS_DELAY) {
                    game.right_das_active = 1;
                    game.right_arr_timer = 0;
                }
            } else {
                // DAS is active, handle auto-repeat
                game.right_arr_timer++;
                if(game.right_arr_timer >= ARR) {
                    game.right_arr_timer = 0;
                    Tetromino temp = game.current_piece;
                    temp.x++;
                    if(!check_collision(&temp, &game.board)) {
                        game.current_piece = temp;
                        // Reset lock delay on successful DAS movement
                        if(game.lock_delay_active && game.lock_resets_used < MAX_LOCK_RESETS) {
                            game.lock_delay_timer = 0;
                            game.lock_resets_used++;
                        }
                    }
                }
            }
        } else {
            // Reset right DAS when key released
            game.right_das_timer = 0;
            game.right_das_active = 0;
            game.right_arr_timer = 0;
        }
        
        // Rotate clockwise
        if(keys_down & KEY_A) {
            rotate_tetromino(&game.current_piece, 1);
            // Reset lock delay on rotation
            if(game.lock_delay_active && game.lock_resets_used < MAX_LOCK_RESETS) {
                game.lock_delay_timer = 0;
                game.lock_resets_used++;
            }
        }
        
        // Rotate counter-clockwise  
        if(keys_down & KEY_B) {
            rotate_tetromino(&game.current_piece, -1);
            // Reset lock delay on rotation
            if(game.lock_delay_active && game.lock_resets_used < MAX_LOCK_RESETS) {
                game.lock_delay_timer = 0;
                game.lock_resets_used++;
            }
        }
        
        // Flip piece (R button)
        if(keys_down & KEY_R) {
            flip_tetromino(&game.current_piece);
            // Reset lock delay on flip
            if(game.lock_delay_active && game.lock_resets_used < MAX_LOCK_RESETS) {
                game.lock_delay_timer = 0;
                game.lock_resets_used++;
            }
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
    else if(game.state == STATE_TITLE) {
        // Title screen - press START to go to main menu
        if(keys_down & KEY_START) {
            game.state = STATE_MAIN_MENU;
            game.menu_selection = 0;
        }
    }
    else if(game.state == STATE_MAIN_MENU) {
        // Menu navigation
        if(keys_down & KEY_UP) {
            if(game.menu_selection > 0) {
                game.menu_selection--;
            }
        }
        if(keys_down & KEY_DOWN) {
            if(game.menu_selection < 4) { // 5 menu items (0-4)
                game.menu_selection++;
            }
        }
        
        // Menu selection
        if(keys_down & KEY_START || keys_down & KEY_A) {
            switch(game.menu_selection) {
                case 0: // Play
                    game.state = STATE_MODE_SELECT;
                    break;
                case 1: // Pokedex
                    game.state = STATE_POKEDEX;
                    break;
                case 2: // Highscores
                    game.state = STATE_HIGHSCORES;
                    break;
                case 3: // Options
                    game.state = STATE_OPTIONS;
                    break;
                case 4: // Credits
                    game.state = STATE_CREDITS;
                    break;
            }
        }
    }
    else if(game.state == STATE_MODE_SELECT) {
        // Navigate modes with LEFT/RIGHT
        if(keys_down & KEY_LEFT) {
            if(game.mode > 0) {
                game.mode--;
            }
        }
        if(keys_down & KEY_RIGHT) {
            if(game.mode < 6) { // 7 modes total (0-6)
                game.mode++;
            }
        }
        
        // Go back to main menu
        if(keys_down & KEY_B) {
            game.state = STATE_MAIN_MENU;
            game.mode = MODE_ROOKIE; // Reset to first mode
        }
        
        // Select mode
        if(keys_down & KEY_START || keys_down & KEY_A) {
            if(game.mode == MODE_BACK) {
                // Back to main menu
                game.state = STATE_MAIN_MENU;
                game.mode = MODE_ROOKIE;
            } else if(game.mode == MODE_BONUS) {
                // TODO: Go to bonus mode submenu
                // For now, just return to menu
                game.state = STATE_MAIN_MENU;
                game.mode = MODE_ROOKIE;
            } else {
                // Start game with selected difficulty
                game.state = STATE_GAMEPLAY;
                init_board(&game.board);
                game.score = 0;
                game.lines_cleared = 0;
                game.level = 1;
                
                // Game Boy Tetris starting speed
                game.fall_speed = 53;  // ~0.88 seconds
                
                game.hold_used = 0;
                game.hold_piece.type = TETRO_COUNT;
                spawn_next_piece();
            }
        }
    }
    else if(game.state == STATE_POKEDEX || 
            game.state == STATE_HIGHSCORES || 
            game.state == STATE_OPTIONS || 
            game.state == STATE_CREDITS) {
        // Go back to main menu from any of these screens
        if(keys_down & KEY_B) {
            game.state = STATE_MAIN_MENU;
        }
    }
    else if(game.state == STATE_PAUSE) {
        if(keys_down & KEY_START) {
            game.state = STATE_GAMEPLAY;
        }
    }
    else if(game.state == STATE_GAME_OVER) {
        if(keys_down & KEY_START) {
            game.state = STATE_MAIN_MENU;
            init_game();
        }
    }
}

void update_game(void) {
    if(game.state != STATE_GAMEPLAY) {
        return;
    }
    
    // Check if piece is currently grounded (can't move down)
    Tetromino ground_check = game.current_piece;
    ground_check.y++;
    u8 is_grounded = check_collision(&ground_check, &game.board);
    
    // If piece is grounded, activate/update lock delay
    if(is_grounded) {
        if(!game.lock_delay_active) {
            // Start lock delay
            game.lock_delay_active = 1;
            game.lock_delay_timer = 0;
        } else {
            // Update lock delay timer
            game.lock_delay_timer++;
            
            // Check if lock delay has expired or max resets reached
            if(game.lock_delay_timer >= LOCK_DELAY_DURATION || 
               game.lock_resets_used >= MAX_LOCK_RESETS) {
                // Lock the piece
                place_tetromino(&game.current_piece, &game.board);
                clear_lines(&game.board);
                game.hold_used = 0;
                spawn_next_piece();
                
                // Reset lock delay state
                game.lock_delay_active = 0;
                game.lock_delay_timer = 0;
                game.lock_resets_used = 0;
                return; // Exit early since we spawned a new piece
            }
        }
    } else {
        // Piece is not grounded - stop lock delay
        game.lock_delay_active = 0;
        game.lock_delay_timer = 0;
    }
    
    // Update fall timer
    game.fall_timer++;
    
    if(game.fall_timer >= game.fall_speed) {
        game.fall_timer = 0;
        
        // Try to move piece down (only if not grounded)
        if(!is_grounded) {
            game.current_piece.y++;
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
        int new_level = 1 + game.lines_cleared / 10;
        if(new_level > 99) new_level = 99;  // Cap at 99
        
        if(new_level > game.level) {
            game.level = new_level;
            
            // Game Boy Tetris speed curve with continued progression after 20
            // Speed continues to increase every 10 levels after 20
            if(game.level >= 90) {
                game.fall_speed = 2;   // Levels 90-99 (extreme!)
            } else if(game.level >= 80) {
                game.fall_speed = 2;   // Levels 80-89
            } else if(game.level >= 70) {
                game.fall_speed = 3;   // Levels 70-79
            } else if(game.level >= 60) {
                game.fall_speed = 3;   // Levels 60-69
            } else if(game.level >= 50) {
                game.fall_speed = 4;   // Levels 50-59
            } else if(game.level >= 40) {
                game.fall_speed = 4;   // Levels 40-49
            } else if(game.level >= 30) {
                game.fall_speed = 5;   // Levels 30-39
            } else if(game.level >= 20) {
                game.fall_speed = 6;   // Levels 20-29
            } else if(game.level >= 16) {
                game.fall_speed = 8;   // Levels 16-19
            } else if(game.level >= 13) {
                game.fall_speed = 10;  // Levels 13-15
            } else if(game.level >= 10) {
                game.fall_speed = 12;  // Levels 10-12
            } else if(game.level >= 7) {
                game.fall_speed = 16;  // Levels 7-9
            } else if(game.level >= 4) {
                game.fall_speed = 20;  // Levels 4-6
            } else if(game.level >= 1) {
                game.fall_speed = 53;  // Levels 1-3 (starting speed ~0.88s)
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
