#ifndef OPTIONS_UI_H
#define OPTIONS_UI_H

#include <gba_base.h>

// Options UI functions
void options_ui_reset(void);
void options_ui_handle_input(u16 down, u16 held);
void options_ui_draw(void);

// Note: get_control_swap() and set_control_swap() are now in save.h/save.c

#endif // OPTIONS_UI_H
