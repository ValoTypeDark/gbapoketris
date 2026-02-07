#ifndef POKEDEX_UI_H
#define POKEDEX_UI_H

#include <gba.h>

// Call when entering the Pokédex screen
void pokedex_ui_reset(void);

// Handle input while in Pokédex (call from handle_input)
void pokedex_ui_handle_input(u16 down, u16 held);

// Render Pokédex UI into the back buffer (called by show_pokedex)
void pokedex_ui_draw(void);

#endif // POKEDEX_UI_H
