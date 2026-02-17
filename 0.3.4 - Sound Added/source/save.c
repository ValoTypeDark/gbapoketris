#define SAVE_VERSION 2
#include "save.h"
#include "pokemon_database.h"
#include <string.h>

// SRAM is defined in libgba as 0x0E000000
// We'll use it as a volatile pointer
#define SRAM_PTR ((volatile u8*)SRAM)

// Save buffers in EWRAM
__attribute__((section(".ewram")))
static SaveData current_save;

__attribute__((section(".ewram")))
static SaveData pending_save;

// State tracking
static u32 async_save_offset = 0;
static u8 async_save_active = 0;
static u8 save_needs_loading = 1;
static u8 save_needs_creation = 0;

static int dex_to_db_index(u16 dex_number) {
    for(int i = 0; i < TOTAL_POKEMON; i++) {
        if(POKEMON_DATABASE[i].dex_number == dex_number) return i;
    }
    return -1;
}

static void sram_read(void* dest, u32 offset, u32 size) {
    u8* dst = (u8*)dest;
    for(u32 i = 0; i < size; i++) {
        dst[i] = SRAM_PTR[offset + i];
    }
}

static void sram_write(u32 offset, const void* src, u32 size) {
    const u8* s = (const u8*)src;
    for(u32 i = 0; i < size; i++) {
        SRAM_PTR[offset + i] = s[i];
    }
}

u32 calculate_checksum(SaveData* data) {
    u32 checksum = 0;
    u8* bytes = (u8*)data;
    u32 size = sizeof(SaveData) - sizeof(u32);
    
    for(u32 i = 0; i < size; i++) {
        checksum ^= bytes[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    return checksum;
}

void init_save_system(void) {
    save_needs_loading = 1;
    save_needs_creation = 0;
    async_save_active = 0;
}

int needs_save_loading(void) { return save_needs_loading; }
int needs_initial_save_creation(void) { return save_needs_creation; }

void load_save_deferred(void) {
    if(!save_needs_loading) return;
    if(!load_game()) {
        reset_save_data();
        save_needs_creation = 1;
    }
    save_needs_loading = 0;
}

void create_initial_save_blocking(void) {
    if(save_needs_creation) {
        save_game();
        save_needs_creation = 0;
    }
}

static void ensure_save_file_exists(void) {
    if(save_needs_creation) {
        save_game();
        save_needs_creation = 0;
    }
}

void reset_save_data(void) {
    memset(&current_save, 0, sizeof(SaveData));
    current_save.magic = SAVE_MAGIC;
    current_save.version = SAVE_VERSION;
    current_save.music_volume = 7;
    current_save.sfx_volume = 7;
    current_save.difficulty = 1;
    current_save.control_swap = 0;  // Default: L=Hold, R=Flip
    
    // Initialize mode unlocks - first 3 modes unlocked by default
    current_save.mode_unlocked[0] = 1; // ROOKIE
    current_save.mode_unlocked[1] = 1; // NORMAL
    current_save.mode_unlocked[2] = 1; // SUPER
    current_save.mode_unlocked[3] = 0; // HYPER (locked)
    current_save.mode_unlocked[4] = 0; // MASTER (locked)
    current_save.mode_unlocked[5] = 0; // UNOWN (locked)
    current_save.mode_unlocked[6] = 0; // VIVILLON (locked)
    current_save.mode_unlocked[7] = 0; // ALCREMIE (locked)
    
    for(int i = 0; i < MODE_COUNT; i++) {
        for(int j = 0; j < 5; j++) {
            current_save.high_scores[i][j] = 0;
        }
    }
    current_save.checksum = calculate_checksum(&current_save);
}

int verify_save_data(void) {
    SaveData temp;
    sram_read(&temp, 0, sizeof(SaveData));
    if(temp.magic != SAVE_MAGIC) return 0;
    if(temp.version != SAVE_VERSION) return 0;
    return (temp.checksum == calculate_checksum(&temp));
}

int save_game(void) {
    current_save.checksum = calculate_checksum(&current_save);
    sram_write(0, &current_save, sizeof(SaveData));
    if(!verify_save_data()) return 0;
    save_needs_creation = 0;
    return 1;
}

int load_game(void) {
    SaveData temp;
    sram_read(&temp, 0, sizeof(SaveData));
    if(temp.magic != SAVE_MAGIC) return 0;
    if(temp.version != SAVE_VERSION) return 0;
    if(temp.checksum != calculate_checksum(&temp)) return 0;
    memcpy(&current_save, &temp, sizeof(SaveData));
    return 1;
}

void load_pokemon_progress(PokemonCatch* out_catches) {
    if(!out_catches) return;
    for(int i = 0; i < TOTAL_POKEMON; i++) {
        out_catches[i].dex_number = POKEMON_DATABASE[i].dex_number;
        out_catches[i].unlocked = current_save.pokemon_unlocked[i];
        out_catches[i].unlocked_shiny = current_save.pokemon_unlocked_shiny[i];
        out_catches[i].catch_count = current_save.catch_count[i];
        out_catches[i].catch_count_shiny = current_save.catch_count_shiny[i];
    }
}

void save_pokemon_progress(const PokemonCatch* in_catches) {
    if(!in_catches) return;
    ensure_save_file_exists();
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

void save_pokemon_progress_deferred(const PokemonCatch* in_catches) {
    if(!in_catches) return;
    current_save.total_pokemon_unlocked = 0;
    current_save.total_pokemon_unlocked_shiny = 0;
    pending_save.total_pokemon_unlocked = 0;
    pending_save.total_pokemon_unlocked_shiny = 0;
    for(int i = 0; i < TOTAL_POKEMON; i++) {
        u8 u = in_catches[i].unlocked ? 1 : 0;
        u8 us = in_catches[i].unlocked_shiny ? 1 : 0;
        current_save.pokemon_unlocked[i] = u;
        current_save.pokemon_unlocked_shiny[i] = us;
        current_save.catch_count[i] = in_catches[i].catch_count;
        current_save.catch_count_shiny[i] = in_catches[i].catch_count_shiny;
        pending_save.pokemon_unlocked[i] = u;
        pending_save.pokemon_unlocked_shiny[i] = us;
        pending_save.catch_count[i] = in_catches[i].catch_count;
        pending_save.catch_count_shiny[i] = in_catches[i].catch_count_shiny;
        if(u) { current_save.total_pokemon_unlocked++; pending_save.total_pokemon_unlocked++; }
        if(us) { current_save.total_pokemon_unlocked_shiny++; pending_save.total_pokemon_unlocked_shiny++; }
    }
}

void save_game_async_begin(void) {
    ensure_save_file_exists();
    memcpy(pending_save.high_scores, current_save.high_scores, sizeof(pending_save.high_scores));
    pending_save.magic = current_save.magic;
    pending_save.version = current_save.version;
    pending_save.total_games_played = current_save.total_games_played;
    pending_save.total_lines_cleared = current_save.total_lines_cleared;
    pending_save.total_pieces_placed = current_save.total_pieces_placed;
    pending_save.playtime_seconds = current_save.playtime_seconds;
    pending_save.music_volume = current_save.music_volume;
    pending_save.sfx_volume = current_save.sfx_volume;
    pending_save.difficulty = current_save.difficulty;
    pending_save.control_swap = current_save.control_swap;
    
    // Copy mode unlock status
    memcpy(pending_save.mode_unlocked, current_save.mode_unlocked, sizeof(current_save.mode_unlocked));
    
    pending_save.checksum = calculate_checksum(&pending_save);
    async_save_offset = 0;
    async_save_active = 1;
}

int save_game_async_step(u32 bytes_per_frame) {
    if(!async_save_active) return 0;
    u32 total = sizeof(SaveData);
    u32 remaining = total - async_save_offset;
    u32 chunk = (remaining < bytes_per_frame) ? remaining : bytes_per_frame;
    sram_write(async_save_offset, ((const u8*)&pending_save) + async_save_offset, chunk);
    async_save_offset += chunk;
    if(async_save_offset >= total) {
        async_save_active = 0;
        memcpy(&current_save, &pending_save, sizeof(SaveData));
        return 0;
    }
    return 1;
}

int save_game_async_in_progress(void) { return async_save_active; }

void add_high_score(GameMode mode, u32 score) {
    if(mode >= MODE_COUNT) return;
    for(int i = 0; i < 5; i++) {
        if(score > current_save.high_scores[mode][i]) {
            for(int j = 4; j > i; j--) {
                current_save.high_scores[mode][j] = current_save.high_scores[mode][j-1];
            }
            current_save.high_scores[mode][i] = score;
            break;
        }
    }
}

int is_high_score(GameMode mode, u32 score) {
    return (mode < MODE_COUNT) && (score > current_save.high_scores[mode][4]);
}

u32 get_high_score(GameMode mode, int rank) {
    if(mode >= MODE_COUNT || rank < 0 || rank >= 5) return 0;
    return current_save.high_scores[mode][rank];
}

void update_statistics(u32 lines, u32 pieces, u32 time) {
    current_save.total_games_played++;
    current_save.total_lines_cleared += lines;
    current_save.total_pieces_placed += pieces;
    current_save.playtime_seconds += time;
}

void get_statistics(u32* games, u32* lines, u32* pieces, u32* time) {
    if(games) *games = current_save.total_games_played;
    if(lines) *lines = current_save.total_lines_cleared;
    if(pieces) *pieces = current_save.total_pieces_placed;
    if(time) *time = current_save.playtime_seconds;
}

u8 get_music_volume(void) { return current_save.music_volume; }
void set_music_volume(u8 v) { current_save.music_volume = (v > 10) ? 10 : v; }
u8 get_sfx_volume(void) { return current_save.sfx_volume; }
void set_sfx_volume(u8 v) { current_save.sfx_volume = (v > 10) ? 10 : v; }
u8 get_difficulty(void) { return current_save.difficulty; }
void set_difficulty(u8 d) { current_save.difficulty = (d > 2) ? 2 : d; }
void unlock_mode(GameMode mode) { 
    if(mode < 8) current_save.mode_unlocked[mode] = 1; 
}

int is_mode_unlocked(GameMode mode) { 
    return (mode < 8) && current_save.mode_unlocked[mode]; 
}

void unlock_pokemon_save(u16 dex_number) {
    int i = dex_to_db_index(dex_number);
    if(i < 0) return;
    if(!current_save.pokemon_unlocked[i]) {
        current_save.pokemon_unlocked[i] = 1;
        current_save.total_pokemon_unlocked++;
        save_game();
    }
}

int is_pokemon_unlocked(u16 dex_number) {
    int i = dex_to_db_index(dex_number);
    return (i >= 0) && current_save.pokemon_unlocked[i];
}

u16 get_unlocked_count(void) {
    return current_save.total_pokemon_unlocked;
}

// Control swap accessors
u8 get_control_swap(void) {
    return current_save.control_swap;
}

void set_control_swap(u8 swap) {
    if(swap <= 1) {
        current_save.control_swap = swap;
        save_game();
    }
}

// Mode unlock sync functions
void load_mode_unlocks(u8* out_mode_unlocked) {
    if(!out_mode_unlocked) return;
    memcpy(out_mode_unlocked, current_save.mode_unlocked, 8);
}

void save_mode_unlocks(const u8* in_mode_unlocked) {
    if(!in_mode_unlocked) return;
    memcpy(current_save.mode_unlocked, in_mode_unlocked, 8);
}
