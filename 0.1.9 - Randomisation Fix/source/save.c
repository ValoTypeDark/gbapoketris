#define SAVE_VERSION 2
#include "save.h"
#include "pokemon_database.h"
#include <string.h>

// Current save data in RAM
static SaveData current_save;

static int dex_to_db_index(u16 dex_number) {
    for(int i = 0; i < TOTAL_POKEMON; i++) {
        if(POKEMON_DATABASE[i].dex_number == dex_number) return i;
    }
    return -1;
}

// Helper function to read from SRAM
static void sram_read(void* dest, u32 offset, u32 size) {
    volatile u8* sram = (volatile u8*)(SRAM + offset);
    u8* dst = (u8*)dest;
    u32 i;
    
    for(i = 0; i < size; i++) {
        dst[i] = sram[i];
    }
}

// Helper function to write to SRAM
static void sram_write(u32 offset, const void* src, u32 size) {
    volatile u8* sram = (volatile u8*)(SRAM + offset);
    const u8* source = (const u8*)src;
    u32 i;
    
    for(i = 0; i < size; i++) {
        sram[i] = source[i];
    }
}

// Calculate checksum using simple XOR method
u32 calculate_checksum(SaveData* data) {
    u32 checksum = 0;
    u32 i;
    u8* bytes = (u8*)data;
    u32 size = sizeof(SaveData) - sizeof(u32); // Exclude checksum field
    
    for(i = 0; i < size; i++) {
        checksum ^= bytes[i];
        checksum = (checksum << 1) | (checksum >> 31); // Rotate left
    }
    
    return checksum;
}

void init_save_system(void) {
    // Try to load existing save
    if(!load_game()) {
        // No valid save found, create new one
        reset_save_data();
        save_game();
    }
}

void reset_save_data(void) {
    int i, j;
    
    memset(&current_save, 0, sizeof(SaveData));
    
    current_save.magic = SAVE_MAGIC;
    current_save.version = SAVE_VERSION;
    
    // Initialize settings to defaults
    current_save.music_volume = 7;
    current_save.sfx_volume = 7;
    current_save.difficulty = 1; // Normal
    
    // Rookie mode is always unlocked
    current_save.modes_unlocked = 1 << MODE_ROOKIE;
    
    // Clear high scores
    for(i = 0; i < MODE_COUNT; i++) {
        for(j = 0; j < 5; j++) {
            current_save.high_scores[i][j] = 0;
        }
    }
    
    current_save.checksum = calculate_checksum(&current_save);
}

int verify_save_data(void) {
    SaveData temp;
    
    // Read from SRAM
    sram_read(&temp, 0, sizeof(SaveData));
    
    // Check magic number
    if(temp.magic != SAVE_MAGIC) {
        return 0; // Invalid save
    }
    

    // Check save version
    if(temp.version != SAVE_VERSION) {
        return 0; // Unsupported save version
    }
    // Verify checksum
    u32 stored_checksum = temp.checksum;
    u32 calculated_checksum = calculate_checksum(&temp);
    
    if(stored_checksum != calculated_checksum) {
        return 0; // Corrupted save
    }
    
    return 1; // Valid save
}

int save_game(void) {
    // Update checksum
    current_save.checksum = calculate_checksum(&current_save);
    
    // Write to SRAM
    sram_write(0, &current_save, sizeof(SaveData));
    
    // Verify write
    if(!verify_save_data()) {
        return 0; // Save failed
    }
    
    return 1; // Success
}

int load_game(void) {
    // Verify save data first
    if(!verify_save_data()) {
        return 0; // No valid save
    }
    
    // Load from SRAM
    sram_read(&current_save, 0, sizeof(SaveData));
    
    return 1; // Success
}

// Pokemon unlock functions
void unlock_pokemon_save(u16 dex_number) {
    int index = dex_to_db_index(dex_number);
    if(index < 0) return;
    if(!current_save.pokemon_unlocked[index]) {
        current_save.pokemon_unlocked[index] = 1;
        current_save.total_pokemon_unlocked++;
        save_game();
    }
}


int is_pokemon_unlocked(u16 dex_number) {
    int index = dex_to_db_index(dex_number);
    if(index < 0) return 0;
    return current_save.pokemon_unlocked[index] ? 1 : 0;
}


u16 get_unlocked_count(void) {
    return current_save.total_pokemon_unlocked;
}



// Pokemon progress sync (Game <-> Save)
void load_pokemon_progress(PokemonCatch* out_catches) {
    if(!out_catches) return;
    for(int i = 0; i < TOTAL_POKEMON; i++) {
        out_catches[i].dex_number = POKEMON_DATABASE[i].dex_number;
        out_catches[i].unlocked = current_save.pokemon_unlocked[i] ? 1 : 0;
        out_catches[i].unlocked_shiny = current_save.pokemon_unlocked_shiny[i] ? 1 : 0;
        out_catches[i].catch_count = current_save.catch_count[i];
        out_catches[i].catch_count_shiny = current_save.catch_count_shiny[i];
    }
}

void save_pokemon_progress(const PokemonCatch* in_catches) {
    if(!in_catches) return;
    current_save.total_pokemon_unlocked = 0;
    current_save.total_pokemon_unlocked_shiny = 0;
    for(int i = 0; i < TOTAL_POKEMON; i++) {
        current_save.pokemon_unlocked[i] = in_catches[i].unlocked ? 1 : 0;
        current_save.pokemon_unlocked_shiny[i] = in_catches[i].unlocked_shiny ? 1 : 0;
        current_save.catch_count[i] = in_catches[i].catch_count;
        current_save.catch_count_shiny[i] = in_catches[i].catch_count_shiny;
        if(current_save.pokemon_unlocked[i]) current_save.total_pokemon_unlocked++;
        if(current_save.pokemon_unlocked_shiny[i]) current_save.total_pokemon_unlocked_shiny++;
    }
    save_game();
}

// High score functions
void add_high_score(GameMode mode, u32 score) {
    int i, j;
    
    if(mode >= MODE_COUNT) return;
    
    // Find insertion position
    for(i = 0; i < 5; i++) {
        if(score > current_save.high_scores[mode][i]) {
            // Shift scores down
            for(j = 4; j > i; j--) {
                current_save.high_scores[mode][j] = current_save.high_scores[mode][j-1];
            }
            
            // Insert new score
            current_save.high_scores[mode][i] = score;
            
            // Save
            save_game();
            break;
        }
    }
}

int is_high_score(GameMode mode, u32 score) {
    if(mode >= MODE_COUNT) return 0;
    
    // Check if score beats 5th place
    return score > current_save.high_scores[mode][4];
}

u32 get_high_score(GameMode mode, int rank) {
    if(mode >= MODE_COUNT || rank < 0 || rank >= 5) {
        return 0;
    }
    
    return current_save.high_scores[mode][rank];
}

// Statistics functions
void update_statistics(u32 lines, u32 pieces, u32 time) {
    current_save.total_games_played++;
    current_save.total_lines_cleared += lines;
    current_save.total_pieces_placed += pieces;
    current_save.playtime_seconds += time;
    
    // Save updated stats
    save_game();
}

void get_statistics(u32* games, u32* lines, u32* pieces, u32* time) {
    if(games) *games = current_save.total_games_played;
    if(lines) *lines = current_save.total_lines_cleared;
    if(pieces) *pieces = current_save.total_pieces_placed;
    if(time) *time = current_save.playtime_seconds;
}

// Settings accessors
u8 get_music_volume(void) {
    return current_save.music_volume;
}

void set_music_volume(u8 volume) {
    if(volume <= 10) {
        current_save.music_volume = volume;
        save_game();
    }
}

u8 get_sfx_volume(void) {
    return current_save.sfx_volume;
}

void set_sfx_volume(u8 volume) {
    if(volume <= 10) {
        current_save.sfx_volume = volume;
        save_game();
    }
}

u8 get_difficulty(void) {
    return current_save.difficulty;
}

void set_difficulty(u8 difficulty) {
    if(difficulty <= 2) {
        current_save.difficulty = difficulty;
        save_game();
    }
}

// Mode unlock functions
void unlock_mode(GameMode mode) {
    if(mode < MODE_COUNT) {
        current_save.modes_unlocked |= (1 << mode);
        save_game();
    }
}

int is_mode_unlocked(GameMode mode) {
    if(mode >= MODE_COUNT) return 0;
    return (current_save.modes_unlocked & (1 << mode)) != 0;
}
