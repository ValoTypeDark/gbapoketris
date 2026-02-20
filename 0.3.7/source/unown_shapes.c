#include "unown_shapes.h"
#include <string.h>

// Size classifications
const u8 SMALL_UNOWN[8] = {UNOWN_I, UNOWN_J, UNOWN_L, UNOWN_O, UNOWN_R, UNOWN_S, UNOWN_T, UNOWN_Z};
const u8 MEDIUM_UNOWN[10] = {UNOWN_B, UNOWN_C, UNOWN_D, UNOWN_M, UNOWN_N, UNOWN_P, UNOWN_U, UNOWN_W, UNOWN_X, UNOWN_Y};
const u8 LARGE_UNOWN[5] = {UNOWN_A, UNOWN_F, UNOWN_K, UNOWN_Q, UNOWN_V};
const u8 VERY_LARGE_UNOWN[3] = {UNOWN_E, UNOWN_G, UNOWN_H};
const u8 SPECIAL_UNOWN[2] = {UNOWN_EXCLAMATION, UNOWN_QUESTION};

// Unown piece data (all 28 pieces with rotations)
const UnownPieceData UNOWN_PIECES[UNOWN_COUNT] = {
    // UNOWN_A (6 blocks)
    {
        .blocks = {
            {{0,1}, {-1,0}, {0,0}, {1,0}, {-1,-1}, {1,-1}, {0,0}},  // Rot 0
            {{-1,0}, {0,1}, {0,0}, {0,-1}, {1,1}, {1,-1}, {0,0}},   // Rot 1
            {{0,-1}, {1,0}, {0,0}, {-1,0}, {1,1}, {-1,1}, {0,0}},   // Rot 2
            {{1,0}, {0,-1}, {0,0}, {0,1}, {-1,-1}, {-1,1}, {0,0}}   // Rot 3
        },
        .block_count = 6,
        .color = 0x56AB,  // RGB15(21,21,10) - Saddle brown
        .name = "unown_a"
    },
    
    // UNOWN_B (5 blocks) - Manual rotations with 2x2 core + tail
    {
        .blocks = {
            {{0,-1}, {0,0}, {1,0}, {0,1}, {1,1}, {0,0}, {0,0}},   // Rot 0 (tail at top)
            {{0,0}, {1,0}, {0,1}, {1,1}, {-1,1}, {0,0}, {0,0}},   // Rot 1 (tail at left)
            {{0,0}, {1,0}, {0,1}, {1,1}, {1,2}, {0,0}, {0,0}},    // Rot 2 (tail at bottom)
            {{0,0}, {1,0}, {0,1}, {1,1}, {2,0}, {0,0}, {0,0}}     // Rot 3 (tail at right)
        },
        .block_count = 5,
        .color = 0x6301,  // RGB15(1,24,12) - Dark khaki
        .name = "unown_b"
    },
    
    // UNOWN_C (5 blocks)
    {
        .blocks = {
            {{-1,1}, {0,1}, {-1,0}, {-1,-1}, {0,-1}, {0,0}, {0,0}},  // Rot 0
            {{-1,-1}, {-1,0}, {0,-1}, {1,-1}, {1,0}, {0,0}, {0,0}},  // Rot 1
            {{1,-1}, {0,-1}, {1,0}, {1,1}, {0,1}, {0,0}, {0,0}},     // Rot 2
            {{1,1}, {1,0}, {0,1}, {-1,1}, {-1,0}, {0,0}, {0,0}}      // Rot 3
        },
        .block_count = 5,
        .color = 0x7B9B,  // RGB15(27,23,15) - Wheat
        .name = "unown_c"
    },
    
    // UNOWN_D (5 blocks) - Manual rotations with 2x2 core + tail
    {
        .blocks = {
            {{1,-1}, {0,0}, {1,0}, {0,1}, {1,1}, {0,0}, {0,0}},   // Rot 0 (tail at top-right)
            {{0,0}, {1,0}, {0,1}, {1,1}, {-1,0}, {0,0}, {0,0}},   // Rot 1 (tail at left)
            {{0,0}, {1,0}, {0,1}, {1,1}, {0,2}, {0,0}, {0,0}},    // Rot 2 (tail at bottom-left)
            {{0,0}, {1,0}, {0,1}, {1,1}, {2,1}, {0,0}, {0,0}}     // Rot 3 (tail at right)
        },
        .block_count = 5,
        .color = 0x5B2D,  // RGB15(13,22,11) - Peach puff
        .name = "unown_d"
    },
    
    // UNOWN_E (7 blocks)
    {
        .blocks = {
            {{0,-1}, {1,-1}, {0,0}, {1,0}, {0,1}, {0,2}, {1,2}},   // Rot 0
            {{-1,0}, {-1,1}, {0,0}, {0,1}, {1,0}, {2,0}, {2,1}},   // Rot 1
            {{0,1}, {1,1}, {0,0}, {1,0}, {0,-1}, {0,-2}, {1,-2}},  // Rot 2
            {{1,0}, {1,1}, {0,0}, {0,1}, {-1,0}, {-2,0}, {-2,1}}   // Rot 3
        },
        .block_count = 7,
        .color = 0x4AD8,  // RGB15(24,21,9) - Dark olive green
        .name = "unown_e"
    },
    
    // UNOWN_F (6 blocks)
    {
        .blocks = {
            {{0,-1}, {1,-1}, {0,0}, {0,1}, {1,1}, {0,2}},   // Rot 0
            {{-1,0}, {-1,1}, {0,0}, {1,0}, {1,1}, {2,0}},   // Rot 1
            {{0,1}, {1,1}, {0,0}, {0,-1}, {1,-1}, {0,-2}},  // Rot 2
            {{1,0}, {1,1}, {0,0}, {-1,0}, {-1,1}, {-2,0}}   // Rot 3
        },
        .block_count = 6,
        .color = 0x50A1,  // RGB15(1,20,10) - Dark goldenrod
        .name = "unown_f"
    },
    
    // UNOWN_G (7 blocks) - Manual rotations with core + tail
    {
        .blocks = {
            {{0,0}, {1,0}, {0,1}, {1,1}, {1,2}, {0,0}, {0,0}},    // Rot 0 (tail at bottom)
            {{0,0}, {1,0}, {0,1}, {1,1}, {2,0}, {0,0}, {0,0}},    // Rot 1 (tail at right)
            {{0,-1}, {0,0}, {1,0}, {0,1}, {1,1}, {0,0}, {0,0}},   // Rot 2 (tail at top)
            {{0,0}, {1,0}, {0,1}, {1,1}, {-1,1}, {0,0}, {0,0}}    // Rot 3 (tail at left)
        },
        .block_count = 5,
        .color = 0x52B4,  // RGB15(20,21,10) - Olive drab
        .name = "unown_g"
    },
    
    // UNOWN_H (7 blocks)
    {
        .blocks = {
            {{-1,-1}, {1,-1}, {-1,0}, {0,0}, {1,0}, {-1,1}, {1,1}},   // Rot 0
            {{-1,-1}, {-1,1}, {0,-1}, {0,0}, {0,1}, {1,-1}, {1,1}},   // Rot 1
            {{-1,-1}, {1,-1}, {-1,0}, {0,0}, {1,0}, {-1,1}, {1,1}},   // Rot 2
            {{-1,-1}, {-1,1}, {0,-1}, {0,0}, {0,1}, {1,-1}, {1,1}}    // Rot 3
        },
        .block_count = 7,
        .color = 0x6B4A,  // RGB15(10,26,13) - Tan
        .name = "unown_h"
    },
    
    // UNOWN_I (4 blocks) - Manual rotations matching standard I-piece
    {
        .blocks = {
            {{-1,0}, {0,0}, {1,0}, {2,0}, {0,0}, {0,0}, {0,0}},    // Rot 0 (horizontal)
            {{1,-1}, {1,0}, {1,1}, {1,2}, {0,0}, {0,0}, {0,0}},    // Rot 1 (vertical)
            {{-1,0}, {0,0}, {1,0}, {2,0}, {0,0}, {0,0}, {0,0}},    // Rot 2 (horizontal)
            {{1,-1}, {1,0}, {1,1}, {1,2}, {0,0}, {0,0}, {0,0}}     // Rot 3 (vertical)
        },
        .block_count = 4,
        .color = 0x7BCF,  // RGB15(15,29,15) - Burlywood
        .name = "unown_i"
    },
    
    // UNOWN_J (4 blocks)
    {
        .blocks = {
            {{0,-1}, {0,0}, {-1,1}, {0,1}, {0,0}, {0,0}, {0,0}},   // Rot 0
            {{-1,0}, {0,0}, {1,-1}, {1,0}, {0,0}, {0,0}, {0,0}},   // Rot 1
            {{0,1}, {0,0}, {1,-1}, {0,-1}, {0,0}, {0,0}, {0,0}},   // Rot 2
            {{1,0}, {0,0}, {-1,1}, {-1,0}, {0,0}, {0,0}, {0,0}}    // Rot 3
        },
        .block_count = 4,
        .color = 0x4D6C,  // RGB15(12,26,9) - Dark salmon
        .name = "unown_j"
    },
    
    // UNOWN_K (6 blocks)
    {
        .blocks = {
            {{-1,-1}, {1,-1}, {-1,0}, {0,0}, {-1,1}, {1,1}},   // Rot 0
            {{-1,-1}, {-1,1}, {0,-1}, {0,0}, {1,-1}, {1,1}},   // Rot 1
            {{-1,-1}, {1,-1}, {-1,0}, {0,0}, {-1,1}, {1,1}},   // Rot 2
            {{-1,-1}, {-1,1}, {0,-1}, {0,0}, {1,-1}, {1,1}}    // Rot 3
        },
        .block_count = 6,
        .color = 0x5AEB,  // RGB15(11,23,11) - Rosy brown
        .name = "unown_k"
    },
    
    // UNOWN_L (4 blocks)
    {
        .blocks = {
            {{0,-1}, {0,0}, {0,1}, {1,1}, {0,0}, {0,0}, {0,0}},   // Rot 0
            {{-1,0}, {0,0}, {1,0}, {1,1}, {0,0}, {0,0}, {0,0}},   // Rot 1
            {{0,1}, {0,0}, {0,-1}, {-1,-1}, {0,0}, {0,0}, {0,0}}, // Rot 2
            {{1,0}, {0,0}, {-1,0}, {-1,-1}, {0,0}, {0,0}, {0,0}}  // Rot 3
        },
        .block_count = 4,
        .color = 0x7B95,  // RGB15(21,28,15) - Peach puff
        .name = "unown_l"
    },
    
    // UNOWN_M (5 blocks)
    {
        .blocks = {
            {{0,-1}, {1,-1}, {-1,0}, {0,0}, {-1,1}, {0,0}, {0,0}},   // Rot 0
            {{-1,0}, {-1,1}, {0,-1}, {0,0}, {1,-1}, {0,0}, {0,0}},   // Rot 1
            {{0,1}, {-1,1}, {1,0}, {0,0}, {1,-1}, {0,0}, {0,0}},     // Rot 2
            {{1,0}, {1,-1}, {0,1}, {0,0}, {-1,1}, {0,0}, {0,0}}      // Rot 3
        },
        .block_count = 5,
        .color = 0x6318,  // RGB15(24,24,12) - Light wood
        .name = "unown_m"
    },
    
    // UNOWN_N (5 blocks)
    {
        .blocks = {
            {{-1,0}, {0,0}, {1,0}, {-1,1}, {1,1}, {0,0}, {0,0}},   // Rot 0
            {{0,-1}, {0,0}, {0,1}, {1,-1}, {1,1}, {0,0}, {0,0}},   // Rot 1
            {{-1,0}, {0,0}, {1,0}, {-1,-1}, {1,-1}, {0,0}, {0,0}}, // Rot 2
            {{0,-1}, {0,0}, {0,1}, {-1,-1}, {-1,1}, {0,0}, {0,0}}  // Rot 3
        },
        .block_count = 5,
        .color = 0x56AB,  // RGB15(21,21,10) - Saddle brown
        .name = "unown_n"
    },
    
    // UNOWN_O (4 blocks) - No rotation (like standard O-piece)
    {
        .blocks = {
            {{0,0}, {1,0}, {0,1}, {1,1}, {0,0}, {0,0}, {0,0}},   // Rot 0
            {{0,0}, {1,0}, {0,1}, {1,1}, {0,0}, {0,0}, {0,0}},   // Rot 1 (same)
            {{0,0}, {1,0}, {0,1}, {1,1}, {0,0}, {0,0}, {0,0}},   // Rot 2 (same)
            {{0,0}, {1,0}, {0,1}, {1,1}, {0,0}, {0,0}, {0,0}}    // Rot 3 (same)
        },
        .block_count = 4,
        .color = 0x7B9B,  // RGB15(27,23,15) - Wheat
        .name = "unown_o"
    },
    
    // UNOWN_P (5 blocks) - Manual rotations with 2x2 core + tail
    {
        .blocks = {
            {{0,0}, {1,0}, {0,1}, {1,1}, {0,2}, {0,0}, {0,0}},    // Rot 0 (tail at bottom-left)
            {{0,0}, {1,0}, {0,1}, {1,1}, {-1,0}, {0,0}, {0,0}},   // Rot 1 (tail at left)
            {{0,0}, {1,0}, {0,1}, {1,1}, {1,-1}, {0,0}, {0,0}},   // Rot 2 (tail at top-right)
            {{0,0}, {1,0}, {0,1}, {1,1}, {2,1}, {0,0}, {0,0}}     // Rot 3 (tail at right)
        },
        .block_count = 5,
        .color = 0x5B2D,  // RGB15(13,22,11) - Peach puff
        .name = "unown_p"
    },
    
    // UNOWN_Q (6 blocks)
    {
        .blocks = {
            {{-1,-1}, {0,-1}, {-1,0}, {0,0}, {0,1}, {1,1}},   // Rot 0
            {{-1,-1}, {-1,0}, {0,-1}, {0,0}, {1,0}, {1,1}},   // Rot 1
            {{1,1}, {0,1}, {1,0}, {0,0}, {0,-1}, {-1,-1}},    // Rot 2
            {{1,1}, {1,0}, {0,1}, {0,0}, {-1,0}, {-1,-1}}     // Rot 3
        },
        .block_count = 6,
        .color = 0x4AD8,  // RGB15(24,21,9) - Dark olive green
        .name = "unown_q"
    },
    
    // UNOWN_R (4 blocks)
    {
        .blocks = {
            {{0,-1}, {1,-1}, {0,0}, {0,1}, {0,0}, {0,0}, {0,0}},   // Rot 0
            {{-1,0}, {-1,1}, {0,0}, {1,0}, {0,0}, {0,0}, {0,0}},   // Rot 1
            {{0,1}, {-1,1}, {0,0}, {0,-1}, {0,0}, {0,0}, {0,0}},   // Rot 2
            {{1,0}, {1,-1}, {0,0}, {-1,0}, {0,0}, {0,0}, {0,0}}    // Rot 3
        },
        .block_count = 4,
        .color = 0x50A1,  // RGB15(1,20,10) - Dark goldenrod
        .name = "unown_r"
    },
    
    // UNOWN_S (4 blocks) - Manual rotations matching standard S-piece
    {
        .blocks = {
            {{0,0}, {1,0}, {-1,1}, {0,1}, {0,0}, {0,0}, {0,0}},   // Rot 0
            {{0,-1}, {0,0}, {1,0}, {1,1}, {0,0}, {0,0}, {0,0}},   // Rot 1
            {{0,0}, {1,0}, {-1,1}, {0,1}, {0,0}, {0,0}, {0,0}},   // Rot 2 (same as 0)
            {{0,-1}, {0,0}, {1,0}, {1,1}, {0,0}, {0,0}, {0,0}}    // Rot 3 (same as 1)
        },
        .block_count = 4,
        .color = 0x52B4,  // RGB15(20,21,10) - Olive drab
        .name = "unown_s"
    },
    
    // UNOWN_T (4 blocks)
    {
        .blocks = {
            {{-1,0}, {0,0}, {1,0}, {0,1}, {0,0}, {0,0}, {0,0}},   // Rot 0
            {{0,-1}, {0,0}, {0,1}, {1,0}, {0,0}, {0,0}, {0,0}},   // Rot 1
            {{-1,0}, {0,0}, {1,0}, {0,-1}, {0,0}, {0,0}, {0,0}},  // Rot 2
            {{0,-1}, {0,0}, {0,1}, {-1,0}, {0,0}, {0,0}, {0,0}}   // Rot 3
        },
        .block_count = 4,
        .color = 0x6B4A,  // RGB15(10,26,13) - Tan
        .name = "unown_t"
    },
    
    // UNOWN_U (5 blocks)
    {
        .blocks = {
            {{-1,0}, {1,0}, {-1,1}, {0,1}, {1,1}, {0,0}, {0,0}},   // Rot 0
            {{0,-1}, {0,1}, {1,-1}, {1,0}, {1,1}, {0,0}, {0,0}},   // Rot 1
            {{-1,0}, {1,0}, {-1,-1}, {0,-1}, {1,-1}, {0,0}, {0,0}}, // Rot 2
            {{0,-1}, {0,1}, {-1,-1}, {-1,0}, {-1,1}, {0,0}, {0,0}}  // Rot 3
        },
        .block_count = 5,
        .color = 0x7BCF,  // RGB15(15,29,15) - Burlywood
        .name = "unown_u"
    },
    
    // UNOWN_V (6 blocks)
    {
        .blocks = {
            {{-1,-1}, {1,-1}, {-1,0}, {1,0}, {-1,1}, {0,1}},   // Rot 0
            {{-1,-1}, {-1,1}, {0,-1}, {0,1}, {1,-1}, {1,0}},   // Rot 1
            {{1,1}, {-1,1}, {1,0}, {-1,0}, {1,-1}, {0,-1}},    // Rot 2
            {{1,1}, {1,-1}, {0,1}, {0,-1}, {-1,1}, {-1,0}}     // Rot 3
        },
        .block_count = 6,
        .color = 0x4D6C,  // RGB15(12,26,9) - Dark salmon
        .name = "unown_v"
    },
    
    // UNOWN_W (5 blocks)
    {
        .blocks = {
            {{-1,-1}, {-1,0}, {0,0}, {0,1}, {1,1}, {0,0}, {0,0}},   // Rot 0
            {{-1,1}, {0,1}, {0,0}, {1,0}, {1,-1}, {0,0}, {0,0}},    // Rot 1
            {{1,1}, {1,0}, {0,0}, {0,-1}, {-1,-1}, {0,0}, {0,0}},   // Rot 2
            {{1,-1}, {0,-1}, {0,0}, {-1,0}, {-1,1}, {0,0}, {0,0}}   // Rot 3
        },
        .block_count = 5,
        .color = 0x5AEB,  // RGB15(11,23,11) - Rosy brown
        .name = "unown_w"
    },
    
    // UNOWN_X (5 blocks)
    {
        .blocks = {
            {{0,-1}, {-1,0}, {0,0}, {1,0}, {0,1}, {0,0}, {0,0}},   // Rot 0
            {{-1,0}, {0,-1}, {0,0}, {0,1}, {1,0}, {0,0}, {0,0}},   // Rot 1
            {{0,1}, {1,0}, {0,0}, {-1,0}, {0,-1}, {0,0}, {0,0}},   // Rot 2
            {{1,0}, {0,1}, {0,0}, {0,-1}, {-1,0}, {0,0}, {0,0}}    // Rot 3
        },
        .block_count = 5,
        .color = 0x7B95,  // RGB15(21,28,15) - Peach puff
        .name = "unown_x"
    },
    
    // UNOWN_Y (5 blocks)
    {
        .blocks = {
            {{0,-1}, {-1,0}, {0,0}, {1,0}, {0,1}, {0,0}, {0,0}},   // Rot 0
            {{-1,0}, {0,-1}, {0,0}, {0,1}, {1,0}, {0,0}, {0,0}},   // Rot 1
            {{0,1}, {1,0}, {0,0}, {-1,0}, {0,-1}, {0,0}, {0,0}},   // Rot 2
            {{1,0}, {0,1}, {0,0}, {0,-1}, {-1,0}, {0,0}, {0,0}}    // Rot 3
        },
        .block_count = 5,
        .color = 0x6318,  // RGB15(24,24,12) - Light wood
        .name = "unown_y"
    },
    
    // UNOWN_Z (4 blocks) - Manual rotations matching standard Z-piece
    {
        .blocks = {
            {{-1,0}, {0,0}, {0,1}, {1,1}, {0,0}, {0,0}, {0,0}},   // Rot 0
            {{0,-1}, {0,0}, {-1,0}, {-1,1}, {0,0}, {0,0}, {0,0}}, // Rot 1
            {{-1,0}, {0,0}, {0,1}, {1,1}, {0,0}, {0,0}, {0,0}},   // Rot 2 (same as 0)
            {{0,-1}, {0,0}, {-1,0}, {-1,1}, {0,0}, {0,0}, {0,0}}  // Rot 3 (same as 1)
        },
        .block_count = 4,
        .color = 0x56AB,  // RGB15(21,21,10) - Saddle brown
        .name = "unown_z"
    },
    
    // UNOWN_EXCLAMATION (3 blocks)
    {
        .blocks = {
            {{0,-1}, {0,0}, {0,2}, {0,0}, {0,0}, {0,0}, {0,0}},   // Rot 0
            {{-1,0}, {0,0}, {2,0}, {0,0}, {0,0}, {0,0}, {0,0}},   // Rot 1
            {{0,1}, {0,0}, {0,-2}, {0,0}, {0,0}, {0,0}, {0,0}},   // Rot 2
            {{1,0}, {0,0}, {-2,0}, {0,0}, {0,0}, {0,0}, {0,0}}    // Rot 3
        },
        .block_count = 3,
        .color = 0x54DA,  // RGB15(26,25,10) - Goldenrod
        .name = "unown_exclamation"
    },
    
    // UNOWN_QUESTION (5 blocks)
    {
        .blocks = {
            {{0,-2}, {1,-2}, {1,-1}, {0,0}, {0,2}, {0,0}, {0,0}},   // Rot 0
            {{-2,0}, {-2,1}, {-1,1}, {0,0}, {2,0}, {0,0}, {0,0}},   // Rot 1
            {{0,2}, {-1,2}, {-1,1}, {0,0}, {0,-2}, {0,0}, {0,0}},   // Rot 2
            {{2,0}, {2,-1}, {1,-1}, {0,0}, {-2,0}, {0,0}, {0,0}}    // Rot 3
        },
        .block_count = 5,
        .color = 0x4C2B,  // RGB15(11,16,9) - Dark goldenrod
        .name = "unown_question"
    }
};

// Get level-based Unown pool
void get_unown_pool_for_level(u8 level, u8* pool, u8* pool_size) {
    u8 count = 0;
    
    // Small pieces (always available)
    for(u8 i = 0; i < 8; i++) {
        pool[count++] = SMALL_UNOWN[i];
    }
    
    // Medium pieces from level 3+
    if(level >= 3) {
        for(u8 i = 0; i < 10; i++) {
            pool[count++] = MEDIUM_UNOWN[i];
        }
    }
    
    // Large pieces from level 6+
    if(level >= 6) {
        for(u8 i = 0; i < 5; i++) {
            pool[count++] = LARGE_UNOWN[i];
        }
    }
    
    // Very large pieces from level 10+
    if(level >= 10) {
        for(u8 i = 0; i < 3; i++) {
            pool[count++] = VERY_LARGE_UNOWN[i];
        }
    }
    
    // Special pieces from level 13+
    if(level >= 13) {
        for(u8 i = 0; i < 2; i++) {
            pool[count++] = SPECIAL_UNOWN[i];
        }
    }
    
    *pool_size = count;
}

// Get Pokemon name for Unown piece
const char* get_unown_pokemon_name(UnownType type) {
    static const char* names[UNOWN_COUNT] = {
        "Unown (A)", "Unown (B)", "Unown (C)", "Unown (D)", "Unown (E)",
        "Unown (F)", "Unown (G)", "Unown (H)", "Unown (I)", "Unown (J)",
        "Unown (K)", "Unown (L)", "Unown (M)", "Unown (N)", "Unown (O)",
        "Unown (P)", "Unown (Q)", "Unown (R)", "Unown (S)", "Unown (T)",
        "Unown (U)", "Unown (V)", "Unown (W)", "Unown (X)", "Unown (Y)",
        "Unown (Z)", "Unown (!)", "Unown (?)"
    };
    
    if(type >= UNOWN_COUNT) return "Unknown";
    return names[type];
}
