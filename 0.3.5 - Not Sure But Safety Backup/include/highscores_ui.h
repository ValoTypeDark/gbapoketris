#ifndef HIGHSCORES_UI_H
#define HIGHSCORES_UI_H

#include <gba_types.h>

// Reset highscores UI state (cursor/page). Does NOT touch saved data.
void highscores_ui_reset(void);

// Handle input on highscores screen.
void highscores_ui_handle_input(u16 down, u16 held);

// Draw highscores screen (Mode 3 back buffer). Uses internal dirty redraw.
void highscores_ui_draw(void);

#endif
