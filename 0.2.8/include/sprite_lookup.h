// Auto-generated sprite lookup table
// Generated from Pokemon database
// DO NOT EDIT MANUALLY

#ifndef SPRITE_LOOKUP_H
#define SPRITE_LOOKUP_H

#include <gba.h>

// Sprite lookup entry
typedef struct {
    u16 dex_number;
    const char* suffix;  // Form suffix (NULL for base form)
    u8 is_shiny;
    const unsigned int* tiles;
    const unsigned short* palette;
} SpriteLookupEntry;

// Sprite lookup table
#define SPRITE_LOOKUP_SIZE 2711
extern const SpriteLookupEntry SPRITE_LOOKUP[];

#endif // SPRITE_LOOKUP_H
