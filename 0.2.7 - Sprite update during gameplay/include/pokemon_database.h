#ifndef POKEMON_DATABASE_H
#define POKEMON_DATABASE_H

#include <gba.h>

// Pokemon data structure
typedef struct {
    u16 dex_number;
    const char* name;
    u8 modes;           // Bitmask: bit 0=Rookie, 1=Normal, 2=Super, 3=Hyper, 4=Master, 5=Unown, 6=Vivillon, 7=Alcremie
    u8 turns_rookie;    // Pieces for Rookie mode (0 if not available)
    u8 turns_normal;    // Pieces for Normal mode (0 if not available)
    u8 turns_super;     // Pieces for Super mode (0 if not available)
    u8 turns_hyper;     // Pieces for Hyper mode (0 if not available)
    u8 turns_master;    // Pieces for Master mode (0 if not available)
    u8 turns_unown;     // Pieces for Unown bonus mode (0 if not available)
    u8 turns_vivillon;  // Pieces for Vivillon bonus mode (0 if not available)
    u8 turns_alcremie;  // Pieces for Alcremie bonus mode (0 if not available)
    u8 special;         // Special/Legendary flag (for +1 mechanic)
    const char* sprite_filename;  // Sprite path (e.g. "0001.png" or "0003a.png" for Mega)
    const char* sprite_shiny;     // Shiny sprite path (e.g. "0001_shiny.png" or "0003a_shiny.png")
    const char* type1;  // Primary type
    const char* type2;  // Secondary type (NULL if none)
    const char* height; // Height (e.g. "0.7 m")
    const char* weight; // Weight (e.g. "6.9 kg")
    const char* dex_text; // Pokedex description
} PokemonData;

// Mode bit flags (match GameMode enum in main.h)
#define POKE_MODE_ROOKIE   0x01   // MODE_ROOKIE = 0
#define POKE_MODE_NORMAL   0x02   // MODE_NORMAL = 1
#define POKE_MODE_SUPER    0x04   // MODE_SUPER = 2
#define POKE_MODE_HYPER    0x08   // MODE_HYPER = 3
#define POKE_MODE_MASTER   0x10   // MODE_MASTER = 4
#define POKE_MODE_UNOWN    0x20   // MODE_UNOWN = 5 (Bonus)
#define POKE_MODE_VIVILLON 0x40   // MODE_VIVILLON = 6 (Bonus)
#define POKE_MODE_ALCREMIE 0x80   // MODE_ALCREMIE = 7 (Bonus)
#define POKE_MODE_ALL      0x1F   // All standard modes (excludes bonus modes)

// Total number of Pokemon in database
#define TOTAL_POKEMON 1349

// Pokemon database (defined in pokemon_database.c)
extern const PokemonData POKEMON_DATABASE[TOTAL_POKEMON];

// Helper function to get Pokemon data by index
const PokemonData* get_pokemon_data(int index);

// Get turn count for a specific mode
u8 get_pokemon_turns(int index, u8 mode);

#endif // POKEMON_DATABASE_H
