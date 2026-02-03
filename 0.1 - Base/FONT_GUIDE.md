# Using System Fonts on GBA

## Quick Start

### 1. Find Your Font

**Windows:**
```
C:\Windows\Fonts\
```
Common fonts:
- `arial.ttf` - Arial
- `verdana.ttf` - Verdana
- `cour.ttf` - Courier New
- `comic.ttf` - Comic Sans

**Mac:**
```
/System/Library/Fonts/
```

**Linux:**
```
/usr/share/fonts/
```

### 2. Convert Font to GBA Format

```bash
# Install Pillow if needed
pip install Pillow

# Convert a font
python font_converter.py C:\Windows\Fonts\arial.ttf 8 arial8

# This creates:
# - font_arial8.c (font data)
# - font_arial8.h (header file)
```

### 3. Add to Your GBA Project

```bash
# Copy generated files
cp font_arial8.c gba_tetris_final/source/
cp font_arial8.h gba_tetris_final/include/

# Add to your source files in graphics.c
```

### 4. Use in Code

In `graphics.c`:

```c
#include "font_arial8.h"

// Replace the old draw_char function with:
void draw_char(int x, int y, char c, u16 color) {
    draw_char_arial8(x, y, c, color, back_buffer, SCREEN_WIDTH);
}
```

That's it! Your game now uses Arial font!

## Font Size Recommendations

For GBA's 240×160 screen:

- **UI Text (Score, Lines, Level)**: 8-10pt
- **Menu Titles**: 12-14pt
- **Tiny Text**: 6pt (might be hard to read)

## Creating Multiple Fonts

You can have different fonts for different purposes:

```bash
# Small font for UI
python font_converter.py arial.ttf 8 ui_font

# Large font for titles
python font_converter.py arial.ttf 14 title_font

# Retro font
python font_converter.py C:\Windows\Fonts\cour.ttf 8 retro_font
```

Then use them selectively:

```c
// For score display
draw_char_ui_font(x, y, c, color, back_buffer, 240);

// For menu title
draw_char_title_font(x, y, c, color, back_buffer, 240);
```

## Using Google Fonts or Custom Fonts

1. Download a free font from [Google Fonts](https://fonts.google.com/)
2. Extract the .ttf file
3. Convert it:

```bash
python font_converter.py PressStart2P-Regular.ttf 8 press_start
```

## Advanced: Full Character Sets

By default, the converter includes space through Z (ASCII 32-90).

To include more characters (lowercase, numbers, symbols), modify the script:

```python
# In font_converter.py, change the chars parameter:
chars = ''.join(chr(i) for i in range(32, 127))  # All printable ASCII
```

## Popular Font Choices for GBA

### For Retro Feel:
- **Press Start 2P** (Google Fonts) - Classic pixel font
- **VT323** (Google Fonts) - Terminal style
- **Courier New** (System) - Monospace

### For Readability:
- **Arial** (System) - Clean and clear
- **Verdana** (System) - Designed for screens
- **Tahoma** (System) - Compact but readable

### For Style:
- **Comic Sans** (System) - Playful (perfect for Pokemon!)
- **Impact** (System) - Bold and strong
- **Georgia** (System) - Elegant

## Example Workflow

```bash
# 1. Convert Comic Sans for that Pokemon feel
python font_converter.py C:\Windows\Fonts\comic.ttf 8 pokemon_font

# 2. Copy to project
cp font_pokemon_font.* gba_tetris_final/

# 3. Update graphics.c to use it

# 4. Rebuild
cd gba_tetris_final
make clean
make

# 5. Test!
mgba pokemon_tetris.gba
```

## Memory Considerations

Each font character takes up memory:
- 8×8 font: ~8 bytes per character
- 10×10 font: ~10 bytes per character
- 59 characters (space through Z): ~500-600 bytes per font

This is tiny! You can easily have 5-10 different fonts with no issues.

## Tips

1. **Test sizes**: Convert multiple sizes and see what looks best on GBA
2. **Preview**: The converter shows the dimensions it generates
3. **Anti-aliasing**: GBA only supports 1-bit (on/off), no gray
4. **Spacing**: You might need to adjust spacing in your draw_text function

## Troubleshooting

**Font looks pixelated:**
- Try a larger size (10pt instead of 8pt)
- Try a different font designed for small sizes

**Characters too wide:**
- Use a narrower font (Verdana, Arial Narrow)
- Reduce font size

**Can't find font file:**
- Windows: Open Font Viewer, right-click font, "Show in folder"
- Check exact filename (case-sensitive on Linux/Mac)

---

Now you can use any font installed on your computer in your GBA game! 🎨
