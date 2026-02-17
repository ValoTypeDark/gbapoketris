#ifndef UNOWN_SHAPES_H
#define UNOWN_SHAPES_H

#include <gba_types.h>

// Unown piece types (28 total: A-Z, !, ?)
typedef enum {
    UNOWN_A = 0,
    UNOWN_B,
    UNOWN_C,
    UNOWN_D,
    UNOWN_E,
    UNOWN_F,
    UNOWN_G,
    UNOWN_H,
    UNOWN_I,
    UNOWN_J,
    UNOWN_K,
    UNOWN_L,
    UNOWN_M,
    UNOWN_N,
    UNOWN_O,
    UNOWN_P,
    UNOWN_Q,
    UNOWN_R,
    UNOWN_S,
    UNOWN_T,
    UNOWN_U,
    UNOWN_V,
    UNOWN_W,
    UNOWN_X,
    UNOWN_Y,
    UNOWN_Z,
    UNOWN_EXCLAMATION,
    UNOWN_QUESTION,
    UNOWN_COUNT
} UnownType;

// Unown piece data structure
typedef struct {
    s8 blocks[4][7][2];  // [rotation][block][x,y]
    u8 block_count;      // 3-7 blocks per piece
    u16 color;           // Earthy stone color
    const char* name;    // "unown_a", etc.
} UnownPieceData;

// Size classifications
extern const u8 SMALL_UNOWN[8];      // 4 blocks
extern const u8 MEDIUM_UNOWN[10];    // 5 blocks
extern const u8 LARGE_UNOWN[5];      // 6 blocks
extern const u8 VERY_LARGE_UNOWN[3]; // 7 blocks
extern const u8 SPECIAL_UNOWN[2];    // Special pieces

extern const UnownPieceData UNOWN_PIECES[UNOWN_COUNT];

// Get Unown piece pool for current level
void get_unown_pool_for_level(u8 level, u8* pool, u8* pool_size);

// Get Pokemon name for Unown piece
const char* get_unown_pokemon_name(UnownType type);

#endif // UNOWN_SHAPES_H
