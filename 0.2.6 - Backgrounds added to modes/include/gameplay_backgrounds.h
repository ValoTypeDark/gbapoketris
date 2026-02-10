// Gameplay backgrounds - your custom PNG backgrounds converted to GBA format
#ifndef GAMEPLAY_BACKGROUNDS_H
#define GAMEPLAY_BACKGROUNDS_H

#include <gba.h>
#include "main.h"

// Configuration: Which backgrounds to include
// Comment out modes you don't want to save ROM space
// Each background is ~75KB
#define INCLUDE_ROOKIE_BG    1
#define INCLUDE_NORMAL_BG    1
#define INCLUDE_SUPER_BG     1
#define INCLUDE_HYPER_BG     1
#define INCLUDE_MASTER_BG    1

// Background bitmap declarations (only if enabled)
#ifdef INCLUDE_ROOKIE_BG
#include "bg_rookie.h"
#endif

#ifdef INCLUDE_NORMAL_BG
#include "bg_normal.h"
#endif

#ifdef INCLUDE_SUPER_BG
#include "bg_super.h"
#endif

#ifdef INCLUDE_HYPER_BG
#include "bg_hyper.h"
#endif

#ifdef INCLUDE_MASTER_BG
#include "bg_master.h"
#endif

// Draw the appropriate background for the current mode
void draw_gameplay_background(GameMode mode);

#endif // GAMEPLAY_BACKGROUNDS_H
