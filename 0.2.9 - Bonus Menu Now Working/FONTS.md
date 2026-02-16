# Pokemon Tetris GBA - Font System

## Three Font Sizes for Different Purposes

Your game now uses three Pokemon-themed fonts, each optimized for its purpose:

### 1. **Pokemon Solid 14pt** (Title Font)
- **Size:** 15×20 pixels
- **Usage:** Main title screen "POKEMON TETRIS"
- **Function:** `draw_text_large(x, y, text, color)`
- **Spacing:** 16 pixels per character
- **Data Type:** u16 (15 pixels wide)

### 2. **Pokemon Game 10pt** (Menu Font)
- **Size:** 9×13 pixels
- **Usage:** Menu options, subtitles, game over screen
- **Function:** `draw_text_menu(x, y, text, color)`
- **Spacing:** 10 pixels per character
- **Data Type:** u16 (9 pixels wide)

### 3. **Pokemon Game 8pt** (Game UI Font)
- **Size:** 7×10 pixels
- **Usage:** In-game UI (Score, Lines, Level, etc.)
- **Function:** `draw_text(x, y, text, color)`
- **Spacing:** 8 pixels per character
- **Data Type:** u8 (7 pixels wide)

## Font Usage Examples

```c
// Title screen
draw_text_large(x, y, "POKEMON TETRIS", YELLOW);

// Menu items
draw_text_menu(x, y, "PRESS START", GREEN);
draw_text_menu(x, y, "SELECT MODE", YELLOW);

// In-game UI
draw_text(x, y, "SCORE", WHITE);
draw_text(x, y, "LINES", WHITE);
draw_text(x, y, "LEVEL", WHITE);
```

## Memory Usage

- Pokemon Solid 14pt: ~3,800 bytes (95 chars × 20 rows × 2 bytes)
- Pokemon Game 10pt: ~2,470 bytes (95 chars × 13 rows × 2 bytes)
- Pokemon Game 8pt: ~950 bytes (95 chars × 10 rows × 1 byte)
- **Total font data:** ~7,220 bytes

## Font Files

All font files are in the appropriate directories:
- **Headers:** `include/font_pokemongame8.h`, `font_pokemon_game10.h`, `font_pokemon_solid14.h`
- **Source:** `source/font_pokemongame8.c`, `font_pokemon_game10.c`, `font_pokemon_solid14.c`

## Technical Details

- Fonts wider than 8 pixels use `u16` data type
- Fonts 8 pixels or narrower use `u8` data type
- All fonts support printable ASCII (32-126)
- Inline helper functions for optimal performance
- Direct pixel manipulation via frame buffer
