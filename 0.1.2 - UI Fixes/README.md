# Pokemon Tetris GBA - Ready to Build!

## Quick Build

Just run:
```bash
make
```

That's it! The ROM will be `pokemon_tetris.gba`

## Requirements

- DevKitPro with devkitARM
- DEVKITPRO environment variable set

Check with:
```bash
echo $DEVKITPRO
arm-none-eabi-gcc --version
```

## Build Commands

```bash
make           # Build the ROM
make clean     # Clean build files
```

## Test

```bash
mgba pokemon_tetris.gba
```

## Project Structure

```
gba_tetris_final/
├── Makefile          # Simple, working Makefile
├── source/
│   ├── main.c        # Game logic
│   ├── graphics.c    # Rendering (with double buffering)
│   └── save.c        # Save system
└── include/
    ├── main.h        # Main header
    └── save.h        # Save header
```

## Features

✅ Full Tetris gameplay
✅ 7 standard tetrominos
✅ Score, lines, levels
✅ Double buffered (no flicker!)
✅ Pause functionality
✅ SRAM save system

## Controls

- D-Pad: Move left/right/down
- A: Rotate clockwise
- B: Rotate counter-clockwise
- Up: Hard drop
- Start: Pause

## Troubleshooting

If build fails:
1. Check DEVKITPRO is set: `echo $DEVKITPRO`
2. Make sure devkitARM is in PATH
3. Try: `make clean && make`

## File Sizes

- Source: ~27 KB
- Compiled ROM: ~200-300 KB
- RAM usage: ~150 KB

Enjoy your GBA Tetris! 🎮
