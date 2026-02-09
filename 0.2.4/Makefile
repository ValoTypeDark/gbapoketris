#---------------------------------------------------------------------------------
# Pokemon Tetris GBA - Makefile
# Supports parallel compilation (-j flag) and response file linking
#---------------------------------------------------------------------------------

TARGET := pokemon_tetris

BUILD    := build
SOURCES  := source
INCLUDES := include
SPRITES  := gfx/sprites

PREFIX   := arm-none-eabi-
CC       := $(PREFIX)gcc
OBJCOPY  := $(PREFIX)objcopy

ARCH     := -mthumb -mthumb-interwork
CFLAGS   := -Wall -O2 \
            -mcpu=arm7tdmi -mtune=arm7tdmi \
            -fomit-frame-pointer -ffast-math \
            $(ARCH) \
            -I$(INCLUDES) \
            -I$(DEVKITPRO)/libgba/include

LDFLAGS  := $(ARCH) -specs=gba.specs
LIBS     := -L$(DEVKITPRO)/libgba/lib -lgba

#---------------------------------------------------------------------------------
# File lists
#---------------------------------------------------------------------------------

# Source .c files -> .o in build/src/
SRC_CFILES   := $(wildcard $(SOURCES)/*.c)
SRC_OFILES   := $(SRC_CFILES:$(SOURCES)/%.c=$(BUILD)/src/%.o)

# Sprite .c files -> .o in build/spr/
SPR_CFILES   := $(wildcard $(SPRITES)/*.c)
SPR_OFILES   := $(SPR_CFILES:$(SPRITES)/%.c=$(BUILD)/spr/%.o)

ALL_OFILES   := $(SRC_OFILES) $(SPR_OFILES)

#---------------------------------------------------------------------------------
# Targets
#---------------------------------------------------------------------------------

.PHONY: all clean

all: $(TARGET).gba
	@echo ""
	@echo "✓ Build complete!"
	@echo ""
	@echo "ROM: $(TARGET).gba"
	@ls -lh $(TARGET).gba

clean:
	@echo "Cleaning..."
	@rm -rf $(BUILD)
	@rm -f $(TARGET).elf $(TARGET).gba
	@echo "✓ Clean complete"

#---------------------------------------------------------------------------------
# ROM creation with SRAM save type
#---------------------------------------------------------------------------------

$(TARGET).gba: $(TARGET).elf
	@echo "Creating GBA ROM..."
	@$(OBJCOPY) -O binary $< $@
	@-gbafix -t"POKETETRIS" -c"PKTM" -m"01" -r2 -p $@ 2>/dev/null || true
	@echo "✓ ROM fixed with SRAM (32KB) save type"

#---------------------------------------------------------------------------------
# Linking (response file avoids "argument list too long")
#---------------------------------------------------------------------------------

$(TARGET).elf: $(ALL_OFILES)
	@echo "Linking $(words $(ALL_OFILES)) object files..."
	@printf '%s\n' $(ALL_OFILES) > $(BUILD)/objects.txt
	@$(CC) $(LDFLAGS) @$(BUILD)/objects.txt $(LIBS) -o $@
	@echo "✓ Link complete"

#---------------------------------------------------------------------------------
# Compilation rules
#---------------------------------------------------------------------------------

# Source files
$(BUILD)/src/%.o: $(SOURCES)/%.c
	@mkdir -p $(BUILD)/src
	@echo "  src: $(<F)"
	@$(CC) $(CFLAGS) -c $< -o $@

# Sprite files (quiet — only summary printed by make's job output)
$(BUILD)/spr/%.o: $(SPRITES)/%.c
	@mkdir -p $(BUILD)/spr
	@$(CC) $(CFLAGS) -c $< -o $@

#---------------------------------------------------------------------------------
# Usage:
#   make            - single core build
#   make -j4        - 4 parallel jobs (use number of your CPU cores)
#   make -j         - use ALL cores
#   make clean      - remove all build files
#---------------------------------------------------------------------------------

#---------------------------------------------------------------------------------
# Save Type Reference (gbafix -r flag):
#   -r0  EEPROM      512 bytes    (Pokemon Ruby/Sapphire)
#   -r1  EEPROM_V124 8KB          (Pokemon Emerald)
#   -r2  SRAM        32KB         (Pokemon Tetris - THIS GAME)
#   -r3  FLASH1M     128KB        
#   -r4  FLASH       64KB
#
# We use -r2 (SRAM 32KB) which is standard for GBA games.
# Modern flash carts use FLASH chips that emulate SRAM (no battery needed).
# Our SaveData is ~8KB, fits perfectly in 32KB SRAM.
#---------------------------------------------------------------------------------
