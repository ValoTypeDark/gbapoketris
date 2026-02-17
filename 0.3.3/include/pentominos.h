// Pentomino definitions header
#ifndef PENTOMINOS_H
#define PENTOMINOS_H

#include <gba_types.h>

// 18 pentomino types
#define PENTOMINO_COUNT 18

// Get pentomino shape data (returns pointer to 5 blocks of [x,y] coords)
const s8* get_pentomino_shape(int type, int rotation);

// Pentomino colors
extern const u16 PENTOMINO_COLORS[18];

#endif // PENTOMINOS_H
