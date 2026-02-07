#ifndef SAVE_H
#define SAVE_H

#include "main.h"
#include "pokemon_database.h"

// Save data magic number - "PKMN"
#define SAVE_VERSION 2
#define SAVE_MAGIC 0x504B4D4E

// SRAM address - use the one from gba_base.h
// #define SRAM ((volatile u8*)0x0E000000)  // Already defined in gba_base.h

// Save data structure
typedef struct {
    u32 magic;                    // Magic number for validation
    u16 version;                  // Save format version
    
    // Pokemon progress data
    u8  pokemon_unlocked[TOTAL_POKEMON];        // 1 if seen/caught (normal)
    u8  pokemon_unlocked_shiny[TOTAL_POKEMON];  // 1 if shiny caught
    u16 catch_count[TOTAL_POKEMON];             // total catches
    u16 catch_count_shiny[TOTAL_POKEMON];       // shiny catches
    u16 total_pokemon_unlocked;                 // count of normal-unlocked entries
    u16 total_pokemon_unlocked_shiny;           // count of shiny-unlocked entries

    // High scores per mode
    u32 high_scores[MODE_COUNT][5]; // Top 5 scores per mode
    
    // Statistics
    u32 total_games_played;
    u32 total_lines_cleared;
    u32 total_pieces_placed;
    u32 playtime_seconds;
    
    // Settings
    u8 music_volume;              // 0-10
    u8 sfx_volume;                // 0-10
    u8 difficulty;                // 0=Easy, 1=Normal, 2=Hard
    
    // Unlocked content
    u8 modes_unlocked;            // Bitflags for modes
    
    u32 checksum;                 // Data integrity check
} SaveData;

// Function declarations
void init_save_system(void);
int save_game(void);
int load_game(void);
void reset_save_data(void);
int verify_save_data(void);
u32 calculate_checksum(SaveData* data);

// Pokemon unlock functions

// Pokemon progress sync (Game <-> Save)
void load_pokemon_progress(PokemonCatch* out_catches);
void save_pokemon_progress(const PokemonCatch* in_catches);
void unlock_pokemon_save(u16 dex_number);
int is_pokemon_unlocked(u16 dex_number);
u16 get_unlocked_count(void);

// High score functions
void add_high_score(GameMode mode, u32 score);
int is_high_score(GameMode mode, u32 score);
u32 get_high_score(GameMode mode, int rank); // 0-4

// Statistics functions
void update_statistics(u32 lines, u32 pieces, u32 time);
void get_statistics(u32* games, u32* lines, u32* pieces, u32* time);

#endif // SAVE_H
